cbuffer PerFrame
{
	float2 move_screen;
};
Texture2D texture_need;
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
struct VertexIn
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
	vout.PosH.xy = move_screen * vin.normal.x + vout.PosH.xy;
	vout.Tex = vin.tex1;
	return vout;
}
float4 PS(VertexOut pin) : SV_Target
{
	float4 texcolor = texture_need.Sample(samTex_liner, pin.Tex);
	clip(texcolor.a - 0.6f);
	return texcolor;
}
technique11 draw_ui
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
