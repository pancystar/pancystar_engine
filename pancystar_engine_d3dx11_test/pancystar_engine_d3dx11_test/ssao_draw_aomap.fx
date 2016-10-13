cbuffer PerFrame
{
	float4x4 gViewToTexSpace;      //用于3D重建的纹理级变换
	float4   gOffsetVectors[14];   //偏移向量，用于收集AO
	float4   gFrustumCorners[4];   //3D重建的四个角，用于借助光栅化插值
	//随机反射光线的反射半径
	float    gOcclusionRadius = 0.2f;
	//遮挡参数，用于根据遮挡距离计算遮挡权值
	float    gOcclusionFadeStart = 0.2f;
	float    gOcclusionFadeEnd = 2.0f;
	float    gSurfaceEpsilon = 0.05f;
};
// Nonnumeric values cannot be added to a cbuffer.
Texture2D gNormalDepthMap;
Texture2D gdepth_map;
Texture2D gRandomVecMap;
float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;
	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
	}
	return occlusion;
}
SamplerState samTex
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	AddressU = WRAP;
	AddressV = WRAP;
};
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
SamplerState samRandomVec
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(1e5f, 0.0f, 0.0f, 1e5f);
};
struct VertexIn//普通顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 PosH       : SV_POSITION; //渲染管线必要顶点
	float3 ToFarPlane : TEXCOORD0;   //用于3D重建
	float2 Tex        : TEXCOORD1;   //纹理坐标
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	//因为是前一个shader光栅化完毕的像素点，不需要做任何变化
	vout.PosH = float4(vin.pos, 1.0f);
	//把四个顶点设置完毕（四个角点的次序存储在法线的x分量里面）
	vout.ToFarPlane = gFrustumCorners[vin.normal.x].xyz;
	//记录下纹理坐标
	vout.Tex = vin.tex1;
	return vout;
}
//根据两点间的距离对遮挡情况赋予权值

[earlydepthstencil]
float4 PS(VertexOut pin) : SV_Target
{
	//还原点的世界坐标
	float4 normalDepth = gNormalDepthMap.Sample(samNormalDepth, pin.Tex);
	float3 n = normalDepth.xyz;
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	//pz = 0.1f / (1.0f - pz);
	float3 p = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	//获取随机向量
	float3 randVec = 2.0f*gRandomVecMap.SampleLevel(samRandomVec, 4.0f*pin.Tex,0.0f).rgb - 1.0f;
	randVec = normalize(randVec);
	float occlusionSum = 0.0f;
	[unroll]
	for (int i = 0; i < 14; ++i)
	{
		//均分平均向量
		float3 offset = reflect(gOffsetVectors[i].xyz, randVec);
		float flip = sign(dot(offset, n));//仅保留符号
		//根据半径获得随机点
		float3 q = p + flip * gOcclusionRadius * offset;

		//得到这个随机点在normaldepthmap上的坐标
		float4 projQ = mul(float4(q, 1.0f), gViewToTexSpace);
		projQ /= projQ.w;
		//根据坐标在normalmap上找到当前视角能看到的一个随机点
		float rz = gdepth_map.SampleLevel(samNormalDepth, projQ.xy,0.0f).r;
		//还原当前视角能看到的这个点的世界坐标
		float3 r = (rz / q.z) * q;
		//根据距离d = |p.z - r.z|，遮挡点指向测试点的向量与测试点法向量的夹角n*(r-p)共同计算出r点对p点的遮挡贡献
		float distZ = p.z - r.z;
		//线性AO淡化
		float delta = min(max((30.0f - p.z), 1.0f) / (30.0f - 5.0f),1.0f);
		float dp = max(dot(n, normalize(r - p)), 0.0f);
		float occlusion = delta * dp * OcclusionFunction(distZ);

		occlusionSum += occlusion*0.3f;
	}

	occlusionSum /= 14;
	float access = 1.0f - occlusionSum;
	float4 outputColor;
	//高次平滑
	outputColor.r = saturate(pow(access, 4.0f));
	outputColor.g = 0.0f;
	outputColor.b = 0.0f;
	outputColor.a = 0.0f;
	return outputColor;
}
technique11 draw_ssaomap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
