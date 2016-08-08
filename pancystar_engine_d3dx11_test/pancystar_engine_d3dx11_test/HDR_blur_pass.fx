cbuffer cbPerFrame
{
	float gTexelWidth;
	float gTexelHeight;
};
cbuffer cbSettings
{
	float gWeights[11] =
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
};
cbuffer cbFixed
{
	static const int gBlurRadius = 5;
};
Texture2D gInputImage;
SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
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
	vout.PosH = float4(vin.pos, 1.0f);
	vout.Tex = vin.tex1;
	return vout;
}
float4 PS(VertexOut pin, uniform bool gHorizontalBlur) : SV_Target
{
	float2 texOffset;
	if (gHorizontalBlur)
	{
		texOffset = float2(gTexelWidth, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, gTexelHeight);
	}
	//先以中心点的颜色值作为基础混合值
	float4 center_color = gInputImage.SampleLevel(samInputImage, pin.Tex, 0.0);
	float4 color = gWeights[5] * center_color;
	for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		if (i == 0)
			continue;
		float2 tex = pin.Tex + i*texOffset;
		//采集边界点的颜色
		float4 neighborcolor = gInputImage.SampleLevel(samInputImage, tex, 0.0);
		//计算当前点的颜色情况
		float weight = gWeights[i + gBlurRadius];
		color += weight*neighborcolor;
	}
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