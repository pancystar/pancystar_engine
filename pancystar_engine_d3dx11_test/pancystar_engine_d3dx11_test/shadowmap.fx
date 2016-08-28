float4x4         final_matrix;     //总变换
Texture2D        texture_diffuse;  //漫反射贴图
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
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
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
	}
}
technique11 ShadowTech_transparent
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withalpha()));
	}
}