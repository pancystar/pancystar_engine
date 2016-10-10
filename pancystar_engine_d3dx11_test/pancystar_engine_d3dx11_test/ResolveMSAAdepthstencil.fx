Texture2DMS<float> gdepth_map;
float3 proj_desc;//投影参数，用于还原深度信息
struct VertexIn//普通顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float2  tex     : TEXCOORD;     //顶点纹理坐标
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
	vout.tex = vin.tex;
	return vout;
}
//----------------------------------------------------------------------------------
float PS(VertexOut IN) : SV_TARGET
{
	float z = gdepth_map.Load(int2(IN.tex.x,IN.tex.y), 0);
	return 1.0f / (z * proj_desc.x + proj_desc.y);
}
technique11 resolove_msaa
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
