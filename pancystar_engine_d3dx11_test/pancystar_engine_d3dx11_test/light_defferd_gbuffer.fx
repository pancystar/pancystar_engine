#include"light_define.fx"
cbuffer perobject
{
	pancy_material   material_need;    //材质
	float4x4         world_matrix;     //世界变换
	float4x4         normal_matrix;    //法线变换
	float4x4         final_matrix;     //总变换
};
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
Texture2D        texture_diffuse;  //漫反射贴图
Texture2D        texture_normal;   //法线贴图
struct Vertex_IN//含法线贴图顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 position      : SV_POSITION;    //变换后的顶点坐标
	float3 normal        : NORMAL;         //变换后的法向量
	float3 tangent       : TANGENT;        //顶点切向量
	float2 tex           : TEXCOORD;       //纹理坐标
	float3 position_bef  : POSITION;       //变换前的顶点坐标
};
struct PixelOut_high
{
	float4 normal        : SV_TARGET0;
	float4 diffuse       : SV_TARGET1;
	float4 specular      : SV_TARGET2;
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.normal   = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	vout.tangent  = mul(float4(vin.tangent, 0.0f), normal_matrix).xyz;
	vout.tex      = vin.tex1;
	vout.position_bef = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	return vout;
}
PixelOut_high PS(VertexOut pin)
{
	PixelOut_high pout;
	pin.normal = normalize(pin.normal);
	//填充Gbuffer
	pout.normal = float4(pin.normal, pin.position_bef.z);
	pout.diffuse = texture_diffuse.Sample(samTex_liner, pin.tex) * material_need.diffuse;
	pout.specular = material_need.specular;
	return pout;
}
PixelOut_high PS_wthnormal(VertexOut pin)
{
	PixelOut_high pout;
	pin.normal = normalize(pin.normal);
	//求解图片所在自空间->模型所在统一世界空间的变换矩阵
	float3 N = pin.normal;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	float3 normal_map = texture_normal.Sample(samTex, pin.tex).rgb;//从法线贴图中获得法线采样
	normal_map = 2 * normal_map - 1;                               //将向量从图片坐标[0,1]转换至真实坐标[-1,1]  
	normal_map = normalize(mul(normal_map, T2W));                  //切线空间至世界空间
	pin.normal = normal_map;
	//填充Gbuffer
	pout.normal = float4(pin.normal, pin.position_bef.z);
	pout.diffuse = texture_diffuse.Sample(samTex_liner, pin.tex) * material_need.diffuse;
	pout.specular = material_need.specular;
	return pout;
}
technique11 draw_common
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 draw_withnormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_wthnormal()));
	}
}