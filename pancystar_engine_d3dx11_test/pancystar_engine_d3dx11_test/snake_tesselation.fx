cbuffer perobject
{
	float4x4         final_matrix;     //总变换
};
/*
Texture2D texture_first;         //纹理贴图
Texture2D texture_normal;        //法线贴图
SamplerState samTex
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	AddressU = WRAP;
	AddressV = WRAP;
};
*/
struct patch_tess
{
	float edge_tess[4]: SV_TessFactor;
	float inside_tess[2] : SV_InsideTessFactor;
};
struct Vertex_IN//含法线贴图顶点
{
	float4	pos 	: POSITION;     //顶点位置
	float4	center_pos 	: NORMAL;   //中心点位置
};
struct VertexOut
{
	float4 position   : POSITION;    //变换后的顶点坐标
	float3 center_pos : NORMAL;         //变换后的法向量
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = vin.pos;
	vout.center_pos = vin.center_pos.xyz;
	return vout;
}
patch_tess ConstantHS(InputPatch<VertexOut, 4> patch, uint PatchID:SV_PrimitiveID)
{
	patch_tess pt;
	pt.edge_tess[0] = 5;
	pt.edge_tess[1] = 5;
	pt.edge_tess[2] = 5;
	pt.edge_tess[3] = 5;

	pt.inside_tess[0] = 5;
	pt.inside_tess[1] = 5;

	return pt;
}
struct hull_out
{
	float3 position_need : POSITION;
	float3	center_pos 	    : NORMAL;       //顶点法向量
};
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(25.0f)]
hull_out HS(
	InputPatch<VertexOut, 4> patch,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	hull_out hout;
	hout.position_need = patch[i].position.xyz;
	hout.center_pos = patch[i].center_pos;
	return hout;
}
struct Domin_out
{
	float4 position : SV_POSITION;
	float3	normal 	    : NORMAL;       //顶点法向量
};
[domain("quad")]
Domin_out DS(
	patch_tess patchTess,
	float2 uv : SV_DomainLocation,
	const OutputPatch<hull_out, 4> quard
	)
{
	Domin_out rec;
	float3 v1 = lerp(quard[0].position_need, quard[1].position_need, uv.x);
	float3 v2 = lerp(quard[2].position_need, quard[3].position_need, uv.x);
	float3 p = lerp(v1, v2, uv.y);

	float3 c1 = lerp(quard[0].center_pos, quard[1].center_pos, uv.x);
	float3 c2 = lerp(quard[2].center_pos, quard[3].center_pos, uv.x);
	float3 c = lerp(c1, c2, uv.y);
	float radiu = 0.3f;
	float3 location_need = c + normalize((p - c)) * radiu;
	rec.position = mul(float4(location_need, 1.0f), final_matrix);

	rec.normal = normalize(p - c);
	return rec;
};
float4 PS(Domin_out pin) : SV_Target
{
	float4 final_color;
	float3 normal = normalize(pin.normal);
	float diffuse_angle = abs(dot(float3(0.0f,1.0f,0.0f), normal)); //漫反射夹角
	final_color = float4(0.7f, 0.7f, 0.7f, 1.0f) * diffuse_angle;
	return final_color;
}
technique11 draw_snake
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
/*
struct Vertex_IN//含法线贴图顶点
{
	float4	pos 	: POSITION;     //顶点位置
	float4	center_pos 	: NORMAL;   //中心点位置
};
struct VertexOut
{
	float4 position        : SV_POSITION;    //变换后的顶点坐标
	float3 normal          : NORMAL;         //变换后的法向量
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(vin.pos,final_matrix);
	vout.normal = normalize((vin.pos - vin.center_pos).xyz);
	return vout;
}
float4 PS(VertexOut pin) :SV_TARGET
{
	float4 final_color;
	float3 normal = normalize(pin.normal);
	float diffuse_angle = abs(dot(float3(0.0f,1.0f,0.0f), normal)); //漫反射夹角
	final_color = float4(0.7f, 0.7f, 0.7f, 1.0f) * diffuse_angle;
	return final_color;
}

technique11 draw_snake
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
*/