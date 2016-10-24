cbuffer PerFrame
{
	float4x4 final_matrix;      //总变换;
};
struct Vertex_IN//含法线贴图顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct Vertex_IN_bone//含法线贴图顶点
{
	float3	pos 	    : POSITION;     //顶点位置
	float3	normal 	    : NORMAL;       //顶点法向量
	float3	tangent     : TANGENT;      //顶点切向量
	uint4   bone_id     : BONEINDICES;  //骨骼ID号
	float4  bone_weight : WEIGHTS;      //骨骼权重
	uint4   texid       : TEXINDICES;   //纹理索引
	float2  tex1        : TEXCOORD;     //顶点纹理坐标
};
struct Vertex_out 
{
	float4 PosH       : SV_POSITION;
	uint4  texid       : TEXINDICES;
};
Vertex_out VS(Vertex_IN vin)
{
	Vertex_out vout;
	vout.PosH = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.texid = vin.texid;
	return vout;
}
uint4 PS(Vertex_out pin) : SV_Target
{
	uint4 texcolor = pin.texid;
	return texcolor;
}
technique11 draw_clipmap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
