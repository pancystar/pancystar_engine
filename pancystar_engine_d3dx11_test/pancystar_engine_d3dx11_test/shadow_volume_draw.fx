float4x4         final_matrix;     //总变换
struct Vertex_IN
{
	float3	pos 	: POSITION;     //顶点位置
};
struct GeometryOut 
{
	float4 position      : SV_POSITION;    //变换后的顶点坐标
};
GeometryOut VS(Vertex_IN vin)
{
	GeometryOut Out;
	Out.position = mul(float4(vin.pos, 1.0f), final_matrix);
	return Out;
}
/*
[maxvertexcount(3)]
void GSShadowmain(point Vertex_IN In[3], inout TriangleStream<GeometryOut> ShadowTriangleStream)
{
	GeometryOut Out;
	for (int i = 0; i<3; i += 1)
	{
		Out.position = mul(float4(In[i].pos, 1.0f), final_matrix);
		ShadowTriangleStream.Append(Out);
	}
	ShadowTriangleStream.RestartStrip();
}*/
float4 PS(GeometryOut pin) :SV_TARGET
{
	return float4(1.0f,0.0f,0.0f,1.0f);
}
technique11 ShadowTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		//SetGeometryShader(CompileShader(gs_5_0, GSShadowmain()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}