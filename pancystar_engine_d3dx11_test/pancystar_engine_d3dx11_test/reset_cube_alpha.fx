Texture2D tex_input;
float3 cube_count;
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
struct VertexIn//普通顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 PosH     : SV_POSITION; //渲染管线必要顶点
	float2 tex      : TEXCOORD;     //顶点纹理坐标
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	//顶点的投影坐标(用于光栅化)
	vout.PosH = float4(vin.pos, 1.0f);
	//定点的纹理坐标(用于msaa采样)
	vout.tex = vin.tex1;
	return vout;
}
//----------------------------------------------------------------------------------
float4 PS(VertexOut IN) : SV_TARGET
{
	float4 color_final = tex_input.SampleLevel(samTex_liner, IN.tex, 0);
	color_final.a = cube_count.r;
    return color_final;
}
technique11 resolove_alpha
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
