cbuffer PerFrame
{
	float4x4 world_matrix;      //世界变换
	float4x4 normal_matrix;     //法线变换
	float4x4 final_matrix;      //总变换
	float3   position_view;     //视点位置
};
TextureCube texture_cube;
SamplerState samTex
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};
struct Vertex_IN//含法线贴图顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 position      : SV_POSITION;    //变换后的顶点坐标
	float3 normal        : TEXCOORD0;      //变换后的法向量
	float3 position_bef	 : TEXCOORD2;      //变换前的顶点坐标
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.position_bef = normalize(mul(float4(vin.pos, 1.0f), world_matrix)).xyz;
	vout.normal = normalize(mul(float4(vin.normal, 0.0f), normal_matrix)).xyz;
	return vout;
}
float4 PS_reflect(VertexOut pin) :SV_TARGET
{
	float4 tex_color = float4(0.0f,0.0f,0.0f,0.0f);
	float4 color_fog = float4(0.75f, 0.75f, 0.75f, 1.0f);
	float3 view_direct = normalize(position_view - pin.position_bef);
	float3 map_direct = view_direct.xyz;//视线向量

	tex_color = texture_cube.Sample(samTex, map_direct);
	return tex_color;
}
float4 PS_inside(VertexOut pin) :SV_TARGET
{
	float4 tex_color = float4(0.0f,0.0f,0.0f,0.0f);
	float4 color_fog = float4(0.75f, 0.75f, 0.75f, 1.0f);

	float3 view_direct = normalize(pin.position_bef - position_view);

	float3 map_direct = -reflect(view_direct, pin.normal);//视线反射向量
	tex_color = texture_cube.Sample(samTex, map_direct);
	return tex_color;
}
technique11 draw_reflect
{
	Pass p0
	{
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_reflect()));
	}
}
technique11 draw_inside
{
	Pass p0
	{
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_inside()));
	}
}
