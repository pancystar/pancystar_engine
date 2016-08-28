cbuffer perframe
{
	uint4                   input_range;
	float4                  light_average;//平均像素信息(平均亮度，高光分界线，高光最高强度，tonemapping系数)
};
cbuffer always 
{
	float4x4                YUV2RGB;
	float4x4                RGB2YUV;
};
Texture2D               input_tex;
StructuredBuffer<float> input_buffer;
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
struct VertexIn//普通顶点
{
	float3	pos 	: POSITION;     //顶点位置
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
	//因为是前一个shader光栅化完毕的像素点，不需要做任何变化
	vout.PosH = float4(vin.pos, 1.0f);
	//记录下纹理坐标
	vout.Tex = vin.tex1;
	return vout;
}
float4 PS(VertexOut Input) : SV_TARGET
{
	//颜色采样
	float4 input_texcolor = input_tex.Sample(samTex_liner, Input.Tex);
	float4 color_lum = float4(0.0f, 0.0f, 0.0f, 1.0f);
	//RGB->YUV
	float alpha_rec = input_texcolor.w;//保存alpha，之后修改至齐次坐标进行颜色空间变换
	input_texcolor.w = 1.0f;
	color_lum = mul(input_texcolor,RGB2YUV);
	float light_avege_lum = 0.0f;
	for (int i = 0; i < input_range.z; ++i) 
	{
		light_avege_lum += input_buffer[i];
	}
	light_avege_lum = exp(light_avege_lum);
	
	//tonemapping操作(突出高光的归一化公式)
	//color_lum.r *= light_average.w*(color_lum.r / light_average.r);
	color_lum.r *= light_average.w*(color_lum.r / light_avege_lum);
	color_lum.r = color_lum.r*(color_lum.r / light_average.b*light_average.b + 1.0f) / (1 + color_lum.r);
	float4 finalcolor = float4(0.0f,0.0f,0.0f,1.0f);
	//摘选高光部分
	if (color_lum.r > light_average.g)
	{
		//YUV->RGB
		finalcolor = mul(color_lum, YUV2RGB);
	}
	finalcolor.w = alpha_rec;
	return finalcolor;
}
technique11 draw_preblur
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}