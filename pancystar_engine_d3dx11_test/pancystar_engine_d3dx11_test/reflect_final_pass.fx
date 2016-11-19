Texture2D gInputImage;
Texture2D gInputReflect;
cbuffer cbPerFrame
{
	float4 tex_range_color_normal;
};
//Texture2D normal_tex;
SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = CLAMP;
	AddressV = CLAMP;
};
struct VertexIn//普通顶点
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
	vout.Tex = vin.tex1;
	return vout;
}
float4 PS(VertexOut pin) : SV_Target
{
	float4 reflect_color = gInputReflect.SampleLevel(samInputImage,pin.Tex,0);
	float reflectance = 0.6f*reflect_color.a;
	//float4 final_color = (1.0f - reflectance) * gInputImage.SampleLevel(samInputImage,pin.Tex,0) + reflectance * reflect_color;
	float accept_range = 0.2f, final_count = 0.0f;
	//自适应的multisample抗锯齿
	float total_count = 0.0f;
	float4 final_blend = (0.0f,0.0f,0.0f,0.0f);
	float2 texOffset_up = pin.Tex + float2(tex_range_color_normal.z, 0.0f);
	float2 texOffset_down = pin.Tex + float2(-tex_range_color_normal.z, 0.0f);
	float2 texOffset_left = pin.Tex + float2(0.0f, tex_range_color_normal.w);
	float2 texOffset_right = pin.Tex + float2(0.0f, -tex_range_color_normal.w);
	//up
	float4 neighbour_color = gInputReflect.SampleLevel(samInputImage, texOffset_up, 0);
	float4 test_color = neighbour_color - reflect_color;
	float check_different = abs(test_color.r) + abs(test_color.g) + abs(test_color.b);
	final_blend += 0.15f * neighbour_color;
	if (check_different > accept_range)
	{
		total_count += 1.0f;
	}
	//down
	neighbour_color = gInputReflect.SampleLevel(samInputImage, texOffset_down, 0);
	test_color = neighbour_color - reflect_color;
	check_different = abs(test_color.r) + abs(test_color.g) + abs(test_color.b);
	final_blend += 0.15f * neighbour_color;
	if (check_different > accept_range)
	{
		total_count += 1.0f;
	}
	//left
	neighbour_color = gInputReflect.SampleLevel(samInputImage, texOffset_left, 0);
	test_color = neighbour_color - reflect_color;
	check_different = abs(test_color.r) + abs(test_color.g) + abs(test_color.b);
	final_blend += 0.15f * neighbour_color;
	if (check_different > accept_range)
	{
		total_count += 1.0f;
	}
	//right
	neighbour_color = gInputReflect.SampleLevel(samInputImage, texOffset_right, 0) - reflect_color;
	test_color = neighbour_color - reflect_color;
	check_different = abs(test_color.r) + abs(test_color.g) + abs(test_color.b);
	final_blend += 0.15f * neighbour_color;
	if (check_different > accept_range)
	{
		total_count += 1.0f;
	}
	//test
	if (total_count > 1.0f)
	{
		reflect_color = 0.4f * reflect_color + final_blend;
	}
	float4 final_color = (1.0f - reflectance) * gInputImage.SampleLevel(samInputImage, pin.Tex, 0) + reflectance * reflect_color;
	//float4 final_color = reflect_color;
	final_color.a = 1.0f;
	return final_color;
}
technique11 blend_reflect
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}