float4x4         final_matrix;     //总变换
float4x4 gBoneTransforms[100];//骨骼变换矩阵
Texture2D        texture_diffuse;  //漫反射贴图
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
RasterizerState Depth
{
	DepthBias = 10000;
	DepthBiasClamp = 0.0f;
	SlopeScaledDepthBias = 1.0f;
};
struct Vertex_IN//含法线贴图顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct Vertex_IN_bone//含法线贴图顶点
{
	float3	pos 	    : POSITION;     //顶点位置
	float3	normal 	    : NORMAL;       //顶点法向量
	float3	tangent     : TANGENT;      //顶点切向量
	uint4   bone_id     : BONEINDICES;  //骨骼ID号
	float4  bone_weight : WEIGHTS;      //骨骼权重
	uint4   texid       : TEXINDICES;   //纹理索引
	float2  tex1        : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 position      : SV_POSITION;    //变换后的顶点坐标
	float2 tex           : TEXCOORD;     //顶点纹理坐标
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.f), final_matrix);
	vout.tex = vin.tex1;
	return vout;
}
VertexOut VS_bone(Vertex_IN_bone vin)
{
	VertexOut vout;
	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.bone_weight.x;
	weights[1] = vin.bone_weight.y;
	weights[2] = vin.bone_weight.z;
	weights[3] = vin.bone_weight.w;
	for (int i = 0; i < 4; ++i)
	{
		// 骨骼变换一般不存在不等缩放的情况，所以可以不做法线的逆转置操作
		posL += weights[i] * mul(float4(vin.pos, 1.0f), gBoneTransforms[vin.bone_id[i]]).xyz;
		normalL += weights[i] * mul(vin.normal, (float3x3)gBoneTransforms[vin.bone_id[i]]);
		tangentL += weights[i] * mul(vin.tangent.xyz, (float3x3)gBoneTransforms[vin.bone_id[i]]);
	}
	//posL = vin.pos;
	//normalL = vin.normal;
	//tangentL = vin.tangent;
	vout.position = mul(float4(posL, 1.0f), final_matrix);
	vout.tex = vin.tex1;
	return vout;
}
float4 PS(VertexOut pin) :SV_TARGET
{ 
	return float4(0.0f,0.0f,0.0f,1.0f);
}
float4 PS_withalpha(VertexOut pin) : SV_Target
{
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.9f);
	return float4(0.0f, 0.0f, 0.0f, 1.0f);;
}
technique11 ShadowTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetRasterizerState(Depth);
	}
}
technique11 ShadowTech_transparent
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withalpha()));
		SetRasterizerState(Depth);
	}
}
technique11 Shadow_skinTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetRasterizerState(Depth);
	}
}
technique11 Shadow_skinTech_transparent
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withalpha()));
		SetRasterizerState(Depth);
	}
}