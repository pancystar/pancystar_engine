cbuffer cbPerFrame
{
	float4 tex_range_color_normal;
};
cbuffer cbSettings
{
	float gWeights[7] =
	{
		0.0289952651318174,0.103818351249974,0.223173360742083,0.288026045752251,0.223173360742083,0.103818351249974,0.0289952651318174
	};
};
cbuffer cbFixed
{
	static const int gBlurRadius = 3;
};
Texture2D gInputImage;
Texture2D normal_tex;
Texture2D depth_tex;
//Texture2D normal_tex;
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(1e5f, 0.0f, 0.0f, 1e5f);
};
SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
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
	float2 Tex        : TEXCOORD1;   //纹理坐标
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = float4(vin.pos, 1.0f);
	vout.Tex = vin.tex1;
	return vout;
}
float4 PS(VertexOut pin, uniform bool gHorizontalBlur) : SV_Target
{
	
	float2 texOffset;
    float2 texoffset_normal;
	if (gHorizontalBlur)
	{
		texOffset = float2(tex_range_color_normal.x, 0.0f);
		texoffset_normal = float2(tex_range_color_normal.z, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, tex_range_color_normal.y);
		texoffset_normal = float2(0.0f, tex_range_color_normal.w);
	}
	//先以中心点的颜色值作为基础混合值
	float4 center_color = gInputImage.SampleLevel(samInputImage, pin.Tex, 0);
	float4 centerNormalDepth = normal_tex.SampleLevel(samNormalDepth, pin.Tex, 0.0f);
	float center_depth = depth_tex.SampleLevel(samNormalDepth, pin.Tex, 0.0f).r;
	//float4 center_normal = normal_tex.SampleLevel(samInputImage, pin.Tex, 0);
	float4 color = gWeights[3] * center_color;
	float total_weight = gWeights[3];
	for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		if (i == 0)
			continue;
		float2 tex = pin.Tex + i*texOffset;
		float2 texnormal = pin.Tex + i*texoffset_normal;
		//采集边界点的颜色
		float4 neighborcolor = gInputImage.SampleLevel(samInputImage, tex, 0);
		float4 neighborNormalDepth = normal_tex.SampleLevel(samNormalDepth, texnormal, 0.0f);
		float neighbor_depth = depth_tex.SampleLevel(samNormalDepth, texnormal, 0.0f).r;
		if (abs(dot(neighborNormalDepth.xyz, centerNormalDepth.xyz)) >= 0.8f &&abs(neighbor_depth - center_depth) <= 0.2f)
		{
			float weight = gWeights[i + gBlurRadius];
			color += weight*neighborcolor;
			total_weight += weight;
		}
		//采集边界点的法线
		//float4 neighbornormal = normal_tex.SampleLevel(samInputImage, texnormal, 0);
		//计算当前点的颜色情况
		
	}
	color /= total_weight;
	return color;
}
technique11 HorzBlur
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(true)));
	}
}

technique11 VertBlur
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(false)));
	}
}