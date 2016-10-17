/*延迟光照效果*/
#include"light_define.fx"
cbuffer perobject
{
	pancy_material   material_need;    //材质
	float4x4         final_matrix;     //总变换
	float4x4         ssao_matrix;      //ssao变换
	float4x4         gBoneTransforms[100];//骨骼变换矩阵
};
Texture2D        texture_light_diffuse;      //漫反射光照贴图
Texture2D        texture_light_specular;     //镜面反射光照贴图
Texture2D        texture_diffuse;            //漫反射贴图
Texture2D        texture_ssao;               //ssao贴图
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
	float2 tex           : TEXCOORD;       //纹理坐标
	float4 pos_ssao      : POSITION1;      //阴影顶点坐标
};
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
	vout.position = mul(float4(posL, 1.0f), final_matrix);
	vout.tex = vin.tex1;
	vout.pos_ssao = mul(float4(posL, 1.0f), ssao_matrix);
	return vout;
}
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.tex = vin.tex1;
	vout.pos_ssao = mul(float4(vin.pos, 1.0f), ssao_matrix);
	return vout;
}
float4 PS(VertexOut pin) :SV_TARGET
{
	pin.pos_ssao /= pin.pos_ssao.w;
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.6f);
	float4 ambient = 0.6f*float4(1.0f, 1.0f, 1.0f, 0.0f) * texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	float4 diffuse = material_need.diffuse * texture_light_diffuse.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);      //漫反射光
	float4 spec = material_need.specular * texture_light_specular.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f);       //镜面反射光
	float4 final_color = tex_color *(ambient + diffuse) + spec;
	final_color.a = tex_color.a;
	
	final_color.rgb *= final_color.a;
	return final_color;
}
technique11 LightTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 LightWithBone
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_bone()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}