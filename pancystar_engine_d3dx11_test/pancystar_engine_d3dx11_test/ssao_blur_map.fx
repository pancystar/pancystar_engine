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
Texture2D gNormalDepthMap;
Texture2D gInputImage;
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};
SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};
struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float2 Tex   : TEXCOORD;
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = float4(vin.PosL, 1.0f);
	vout.Tex = vin.Tex;
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

	//先以中心点的遮蔽值作为基础混合值
	float4 center_color = gInputImage.SampleLevel(samInputImage, pin.Tex, 0.0);
	float4 color = gWeights[5] * center_color;
	float totalWeight = gWeights[5];
	//中心点的深度采样
	float4 centerNormalDepth = gNormalDepthMap.SampleLevel(samNormalDepth, pin.Tex, 0.0f);
	for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		if (i == 0)
			continue;
		
		
		float2 tex = pin.Tex + i*texOffset;
		//采集边界点的深度信息用于判断是否应当被贡献
		float4 neighborNormalDepth = gNormalDepthMap.SampleLevel(samNormalDepth, tex, 0.0f);
		//采集边界点的遮蔽值用于计算模糊贡献值
		float4 neighborcolor = gInputImage.SampleLevel(samInputImage, tex, 0.0);
		//计算当前点的遮蔽情况
		float weight = gWeights[i + gBlurRadius];
		/*
		if (abs(center_color.r - neighborcolor.r) > 0.4f)
		{
			//与中心点颜色差异过大的部分将被舍弃
			color += weight*center_color;
			totalWeight += weight;
		}
		*/
		if (abs(dot(neighborNormalDepth.xyz, centerNormalDepth.xyz)) >= 0.8f &&abs(neighborNormalDepth.a - centerNormalDepth.a) <= 0.2f)
		{
			//发现当前点与中心点属于可混合的部分(首先是颜色差不大，然后就是深度差很小说明是同一物体，向量差也小说明是同一面，这种情况下才能够混合)
			color += weight*neighborcolor;
			totalWeight += weight;
		}
		/*
		else
		{
			//发现当前点与中心点属于不可混合的部分，此时不能直接取消混合，因为这样会造成严重的锯齿，对于有ao值的部分我们允许其像外部偏移一些，以消除锯齿现象。
			if (neighborcolor.r < 0.5f)
			{
				color += weight*neighborcolor*2.0;
				totalWeight += weight;
			}
		}*/
		
	}
	return color / totalWeight;
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