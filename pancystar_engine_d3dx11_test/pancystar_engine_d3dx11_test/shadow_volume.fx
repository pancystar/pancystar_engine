cbuffer perframe
{
	float3 position_light;   //光源位置
	float3 direction_light;  //光源方向
	float4x4 world_matrix;   //世界变换矩阵
	float4x4 normal_matrix;  //法线变换矩阵
	float4x4 final_matrix;  //法线变换矩阵
};
struct Vertex_IN//含法线贴图顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	uint4   texid   : TEXINDICES;   //纹理索引
	float2  tex     : TEXCOORD;     //顶点纹理坐标
};
struct GSShadowIn
{
	float3 pos          : POSITION;
	float3 norm         : NORMAL;
};
DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};
DepthStencilState VolumeComplexityStencil
{
	DepthEnable = true;
	DepthWriteMask = ZERO;
	DepthFunc = Less;

	// Setup stencil states
	StencilEnable = true;
	StencilReadMask = 0xFFFFFFFF;
	StencilWriteMask = 0xFFFFFFFF;

	BackFaceStencilFunc = Always;
	BackFaceStencilDepthFail = Decr;
	//BackFaceStencilDepthPass = Incr;
	BackFaceStencilPass = Keep;
	BackFaceStencilFail = Keep;

	FrontFaceStencilFunc = Always;
	FrontFaceStencilDepthFail = Incr;
	//FrontFaceStencilDepthPass = Decr;
	FrontFaceStencilPass = Keep;
	FrontFaceStencilFail = Keep;
};
RasterizerState DisableCulling
{
	CullMode = NONE;
};
struct PSShadowIn
{
	float4 pos			: SV_POSITION;
};
void DetectAndProcessSilhouette(float3 N,//中心三角形法线
	GSShadowIn v1,    // 中心三角形与邻接三角形的共享边
	GSShadowIn v2,    // 中心三角形与邻接三角形的共享边
	GSShadowIn vAdj,  // 邻接三角形的孤立顶点
	inout TriangleStream<PSShadowIn> ShadowTriangleStream //输出流
	)
{

	float3 NAdj = cross(v2.pos - vAdj.pos, v1.pos - vAdj.pos);
	N = position_light - v1.pos;
	//if (dot(N, NAdj) < 0)
	//{
		//	return;

		float3 outpos[4];
		float3 extrude1 = normalize(v1.pos - position_light);
		float3 extrude2 = normalize(v2.pos - position_light);

		float bias_offset = 0.1f;
		float shadowvolume_range = 20.0f;
		outpos[0] = v1.pos + bias_offset*extrude1;
		outpos[1] = v1.pos + (shadowvolume_range - bias_offset)*extrude1;
		outpos[2] = v2.pos + bias_offset*extrude2;
		outpos[3] = v2.pos + (shadowvolume_range - bias_offset)*extrude2;

		//创建两个阴影体表面
		PSShadowIn Out;
		[unroll]
		for (int v = 0; v < 4; v++)
		{
			Out.pos = mul(float4(outpos[v], 1.0f), final_matrix);
			ShadowTriangleStream.Append(Out);
		}
		ShadowTriangleStream.RestartStrip();
	//}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~粒子产生shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GSShadowIn StreamOutVS(Vertex_IN vin)
{
	GSShadowIn vout;
	vout.pos = mul(float4(vin.pos, 1.0f), world_matrix).xyz;
	vout.norm = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	return vout;
}

/*                       5
						/\
					   / d\
					 0/_ _ \4
					 /\ a  /\
					/ b\  / c\
				  1/_ _ \/_ _ \3
						 2
pancy:输入三角形为a，其邻接三角形为b,c,d，在gs输入结构中顶点标号如上 参见:https://msdn.microsoft.com/en-us/library/windows/desktop/bb509609(v=vs.85).aspx
*/

[maxvertexcount(18)]
void GSShadowmain(triangleadj GSShadowIn In[6], inout TriangleStream<PSShadowIn> ShadowTriangleStream)
{
	// 计算中心三角形法线方向
	float3 N = cross(In[2].pos - In[0].pos, In[4].pos - In[0].pos);
	//计算光源方向(聚光灯光源)
	float3 lightDir = position_light - In[0].pos;
	// float3(0, -1, -1);
	//;
	//当法线中心面朝向光源时，测试是否有边缘线，并为边缘线生成几何体
	if (dot(N, lightDir) > 0.0f)
	{
		float bias_offset = 0.1f;
		float shadowvolume_range = 20.0f;
		// 检测三条边缘线
		DetectAndProcessSilhouette(lightDir, In[0], In[2], In[1], ShadowTriangleStream);
		DetectAndProcessSilhouette(lightDir, In[2], In[4], In[3], ShadowTriangleStream);
		DetectAndProcessSilhouette(lightDir, In[4], In[0], In[5], ShadowTriangleStream);
		//创建顶部
		PSShadowIn Out;
		for (int i = 0; i < 6; i += 2)
		{
			float3 extrude = normalize(In[i].pos - position_light);

			float3 pos = In[i].pos + bias_offset*extrude;
			Out.pos = mul(float4(pos, 1.0f), final_matrix);
			ShadowTriangleStream.Append(Out);
		}
		ShadowTriangleStream.RestartStrip();

		//创建底部
		for (int k = 4; k >= 0; k -= 2)
		{
			float3 extrude = normalize(In[k].pos - position_light);

			float3 pos = In[k].pos + (shadowvolume_range - bias_offset)*extrude;
			Out.pos = mul(float4(pos, 1.0f), final_matrix);
			ShadowTriangleStream.Append(Out);
		}
		ShadowTriangleStream.RestartStrip();
	}
}
//GeometryShader gsStreamOut = ConstructGSWithSO(CompileShader(gs_5_0, GSShadowmain()),"POSITION.xyz");

float4 PS(PSShadowIn pin) :SV_TARGET
{
	return float4(1.0f,1.0f,1.0f,1.0f);
}
technique11 StreamOutTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, StreamOutVS()));
		//SetGeometryShader(NULL);
		//SetGeometryShader(gsStreamOut);
		SetGeometryShader(CompileShader(gs_5_0, GSShadowmain()));
		// 禁用像素着色器
		//SetPixelShader(NULL);
		//SetDepthStencilState(DisableDepth, 0);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetRasterizerState(DisableCulling);
		SetDepthStencilState(VolumeComplexityStencil, 1);
	}
}