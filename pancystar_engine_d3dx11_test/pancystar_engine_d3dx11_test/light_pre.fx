/*前向光照效果*/
#include"light_define.fx"
cbuffer perframe
{
	pancy_light_basic   light_need[15];   //聚光灯光源
	float3             position_view;         //视点位置
	int4 num_dir;
};
cbuffer perobject
{
	pancy_material   material_need;    //材质
	float4x4         world_matrix;     //世界变换
	float4x4         normal_matrix;    //法线变换
	float4x4         final_matrix;     //总变换
	float4x4         shadowmap_matrix; //阴影贴图变换
	float4x4         ssao_matrix;      //ssao变换
};
Texture2D        texture_diffuse;  //漫反射贴图
Texture2D        texture_normal;   //法线贴图
Texture2D        texture_shadow;   //阴影贴图
Texture2D        texture_ssao;     //ssao贴图
SamplerState samTex
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	AddressU = WRAP;
	AddressV = WRAP;
};
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
SamplerComparisonState samShadow
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	ComparisonFunc = LESS_EQUAL;
};
BlendState alpha_blend
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
	SrcBlendAlpha = ONE;
	DestBlendAlpha = ZERO;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};
DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};
DepthStencilState NoDepthWrites
{
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
};
float CalcShadowFactor(SamplerComparisonState samShadow, Texture2D shadowMap, float4 shadowPosH)
{
	//归一化
	shadowPosH.xyz /= shadowPosH.w;

	//采集光源投影后的深度
	float depth = shadowPosH.z - 0.0001f;

	//阴影贴图的步长
	const float dx = 1.0f / 1024.0f;
	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		float2 rec_pos = shadowPosH.xy + offsets[i];
		if (rec_pos.x > 1.0f || rec_pos.x < 0.0f || rec_pos.y > 1.0f || rec_pos.y < 0.0f)
		{
			percentLit += 1.0f;
		}
		else
		{
			percentLit += shadowMap.SampleCmpLevelZero(samShadow, rec_pos, depth).r;
		}
	}
	return percentLit /= 9.0f;
}
struct Vertex_IN//含法线贴图顶点
{
	float3	pos 	: POSITION;     //顶点位置
	float3	normal 	: NORMAL;       //顶点法向量
	float3	tangent : TANGENT;      //顶点切向量
	float2  tex1    : TEXCOORD;     //顶点纹理坐标
};
struct VertexOut
{
	float4 position      : SV_POSITION;    //变换后的顶点坐标
	float3 normal        : NORMAL;         //变换后的法向量
	float3 tangent       : TANGENT;        //顶点切向量
	float2 tex           : TEXCOORD;       //纹理坐标
	float3 position_bef  : POSITION;       //变换前的顶点坐标
	float4 pos_shadow    : POSITION1;       //阴影顶点坐标
	float4 pos_ssao      : POSITION2;       //阴影顶点坐标
};
VertexOut VS(Vertex_IN vin)
{
	VertexOut vout;
	vout.position = mul(float4(vin.pos, 1.0f), final_matrix);
	vout.normal = mul(float4(vin.normal, 0.0f), normal_matrix).xyz;
	vout.tangent = mul(float4(vin.tangent, 0.0f), normal_matrix).xyz;
	vout.tex = vin.tex1;
	vout.position_bef = mul(float4(vin.pos, 0.0f), world_matrix).xyz;
	vout.pos_shadow = mul(float4(vin.pos, 1.0f), shadowmap_matrix);
	vout.pos_ssao = mul(float4(vin.pos, 1.0f), ssao_matrix);
	return vout;
}
float4 PS(VertexOut pin) :SV_TARGET
{
	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[i], pin.normal, eye_direct, A1, D1, S1);
	for (int i = 0; i < 2; ++i)
	{
		//方向光(direction light)
		if (light_need[i].type.x == 0)
		{
			compute_dirlight(material_need, light_need[i], pin.normal, eye_direct, A, D, S);
		}
		//点光源(point light)
		else if (light_need[i].type.x == 1)
		{
			compute_pointlight(material_need, light_need[i], pin.position_bef, pin.normal, position_view, A, D, S);
		}
		//聚光灯(spot light)
		else
		{
			compute_spotlight(material_need, light_need[i], pin.position_bef, pin.normal, eye_direct, A, D, S);
		}
		//环境光
		ambient += A;
		//无阴影光
		diffuse += D;
		spec += S;
	}
	float4 final_color = ambient + diffuse + spec;
	return final_color;
}
float4 PS_withtex(VertexOut pin) :SV_TARGET
{
	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[i], pin.normal, eye_direct, A, D, S);

	for (int i = 0; i < 2; ++i)
	{
		//方向光(direction light)
		if (light_need[i].type.x == 0)
		{
			compute_dirlight(material_need, light_need[i], pin.normal, eye_direct, A, D, S);
		}
		//点光源(point light)
		else if (light_need[i].type.x == 1)
		{
			compute_pointlight(material_need, light_need[i], pin.position_bef, pin.normal, position_view, A, D, S);
		}
		//聚光灯(spot light)
		else
		{
			compute_spotlight(material_need, light_need[i], pin.position_bef, pin.normal, eye_direct, A, D, S);
		}
		//环境光
		ambient += A;
		//无阴影光
		diffuse += D;
		spec += S;
	}
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	float4 final_color = tex_color * (ambient + diffuse) + spec;
	return final_color;
}
float4 PS_withshadow(VertexOut pin) :SV_TARGET
{
	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[i], pin.normal, eye_direct, A, D, S);
	float rec_shadow = CalcShadowFactor(samShadow, texture_shadow, pin.pos_shadow);

	for (int i = 0; i < 2; ++i)
	{
		//方向光(direction light)
		if (light_need[i].type.x == 0)
		{
			compute_dirlight(material_need, light_need[i], pin.normal, eye_direct, A, D, S);
		}
		//点光源(point light)
		else if (light_need[i].type.x == 1)
		{
			compute_pointlight(material_need, light_need[i], pin.position_bef, pin.normal, position_view, A, D, S);
		}
		//聚光灯(spot light)
		else
		{
			compute_spotlight(material_need, light_need[i], pin.position_bef, pin.normal, eye_direct, A, D, S);
		}
		//环境光
		ambient += A;
		//无阴影光
		if (light_need[i].type.y == 0)
		{
			diffuse += D;
			spec += S;
		}
		//shadowmap阴影
		else if (light_need[i].type.y == 1)
		{
			diffuse += (0.2f + 0.8f*rec_shadow)*D;
			spec += (0.2f + 0.8f*rec_shadow)*S;
		}
		//shadow volume阴影
		else
		{
			diffuse += D;
			spec += S;
		}
	}
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	float4 final_color = tex_color * (ambient + diffuse) + spec;
	return final_color;
}
float4 PS_withshadownormal(VertexOut pin) :SV_TARGET
{
	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	//求解图片所在自空间->模型所在统一世界空间的变换矩阵
	float3 N = pin.normal;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	float3 normal_map = texture_normal.Sample(samTex, pin.tex).rgb;//从法线贴图中获得法线采样
	normal_map = 2 * normal_map - 1;                               //将向量从图片坐标[0,1]转换至真实坐标[-1,1]  
	normal_map = normalize(mul(normal_map, T2W));                  //切线空间至世界空间
	pin.normal = normal_map;



	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[i], pin.normal, eye_direct, A, D, S);
	float rec_shadow = CalcShadowFactor(samShadow, texture_shadow, pin.pos_shadow);

	for (int i = 0; i < 2; ++i)
	{
		//方向光(direction light)
		if (light_need[i].type.x == 0)
		{
			compute_dirlight(material_need, light_need[i], pin.normal, eye_direct, A, D, S);
		}
		//点光源(point light)
		else if (light_need[i].type.x == 1)
		{
			compute_pointlight(material_need, light_need[i], pin.position_bef, pin.normal, position_view, A, D, S);
		}
		//聚光灯(spot light)
		else
		{
			compute_spotlight(material_need, light_need[i], pin.position_bef, pin.normal, eye_direct, A, D, S);
		}
		//环境光
		ambient += A;
		//无阴影光
		if (light_need[i].type.y == 0)
		{
			diffuse += D;
			spec += S;
		}
		//shadowmap阴影
		else if (light_need[i].type.y == 1)
		{
			diffuse += (0.2f + 0.8f*rec_shadow)*D;
			spec += (0.2f + 0.8f*rec_shadow)*S;
		}
		//shadow volume阴影
		else
		{
			diffuse += D;
			spec += S;
		}
	}
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	float4 final_color = tex_color * (ambient + diffuse) + spec;
	return final_color;
}
float4 PS_withshadowssao(VertexOut pin) :SV_TARGET
{
	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[i], pin.normal, eye_direct, A, D, S);
	float rec_shadow = CalcShadowFactor(samShadow, texture_shadow, pin.pos_shadow);

	pin.pos_ssao /= pin.pos_ssao.w;
	float rec_ssao = texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;
	for (int i = 0; i < 2; ++i)
	{
		//方向光(direction light)
		if (light_need[i].type.x == 0)
		{
			compute_dirlight(material_need, light_need[i], pin.normal, eye_direct, A, D, S);
		}
		//点光源(point light)
		else if (light_need[i].type.x == 1)
		{
			compute_pointlight(material_need, light_need[i], pin.position_bef, pin.normal, position_view, A, D, S);
		}
		//聚光灯(spot light)
		else
		{
			compute_spotlight(material_need, light_need[i], pin.position_bef, pin.normal, eye_direct, A, D, S);
		}
		//环境光
		ambient += A*(0.8f*rec_ssao + 0.2f);
		//无阴影光
		if (light_need[i].type.y == 0) 
		{
			diffuse += D;
			spec += S;
		}
		//shadowmap阴影
		else if (light_need[i].type.y == 1)
		{
			diffuse += (0.2f + 0.8f*rec_shadow)*D;
			spec += (0.2f + 0.8f*rec_shadow)*S;
		}
		//shadow volume阴影
		else 
		{
			diffuse += D;
			spec += S;
		}
	}
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);

	float4 final_color = tex_color * (ambient + diffuse) + spec;
	final_color.a = tex_color.a;
	return final_color;
}
float4 PS_withshadowssaonormal(VertexOut pin) :SV_TARGET
{
	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	//求解图片所在自空间->模型所在统一世界空间的变换矩阵
	float3 N = pin.normal;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	float3 normal_map = texture_normal.Sample(samTex, pin.tex).rgb;//从法线贴图中获得法线采样
	normal_map = 2 * normal_map - 1;                               //将向量从图片坐标[0,1]转换至真实坐标[-1,1]  
	normal_map = normalize(mul(normal_map, T2W));                  //切线空间至世界空间
	pin.normal = normal_map;

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[i], pin.normal, eye_direct, A, D, S);
	float rec_shadow = CalcShadowFactor(samShadow, texture_shadow, pin.pos_shadow);

	pin.pos_ssao /= pin.pos_ssao.w;
	float rec_ssao = texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;

	for (int i = 0; i < 2; ++i)
	{
		//方向光(direction light)
		if (light_need[i].type.x == 0)
		{
			compute_dirlight(material_need, light_need[i], pin.normal, eye_direct, A, D, S);
		}
		//点光源(point light)
		else if (light_need[i].type.x == 1)
		{
			compute_pointlight(material_need, light_need[i], pin.position_bef, pin.normal, position_view, A, D, S);
		}
		//聚光灯(spot light)
		else
		{
			compute_spotlight(material_need, light_need[i], pin.position_bef, pin.normal, eye_direct, A, D, S);
		}
		//环境光
		ambient += A*(0.8f*rec_ssao + 0.2f);
		//无阴影光
		if (light_need[i].type.y == 0)
		{
			diffuse += D;
			spec += S;
		}
		//shadowmap阴影
		else if (light_need[i].type.y == 1)
		{
			diffuse += (0.2f + 0.8f*rec_shadow)*D;
			spec += (0.2f + 0.8f*rec_shadow)*S;
		}
		//shadow volume阴影
		else
		{
			diffuse += D;
			spec += S;
		}
	}
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	float4 final_color = tex_color * (ambient + diffuse) + spec;
	final_color.a = tex_color.a;
	return final_color;
}
float4 PS_alphasave(VertexOut pin) :SV_TARGET
{
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(tex_color.a - 0.9f);

	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	//求解图片所在自空间->模型所在统一世界空间的变换矩阵
	float3 N = pin.normal;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	float3 normal_map = texture_normal.Sample(samTex, pin.tex).rgb;//从法线贴图中获得法线采样
	normal_map = 2 * normal_map - 1;                               //将向量从图片坐标[0,1]转换至真实坐标[-1,1]  
	normal_map = normalize(mul(normal_map, T2W));                  //切线空间至世界空间
	pin.normal = normal_map;

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[i], pin.normal, eye_direct, A, D, S);
	float rec_shadow = CalcShadowFactor(samShadow, texture_shadow, pin.pos_shadow);

	pin.pos_ssao /= pin.pos_ssao.w;
	float rec_ssao = texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;

	for (int i = 0; i < 2; ++i)
	{
		//方向光(direction light)
		if (light_need[i].type.x == 0)
		{
			compute_dirlight(material_need, light_need[i], pin.normal, eye_direct, A, D, S);
		}
		//点光源(point light)
		else if (light_need[i].type.x == 1)
		{
			compute_pointlight(material_need, light_need[i], pin.position_bef, pin.normal, position_view, A, D, S);
		}
		//聚光灯(spot light)
		else
		{
			compute_spotlight(material_need, light_need[i], pin.position_bef, pin.normal, eye_direct, A, D, S);
		}
		//环境光
		ambient += A*(0.8f*rec_ssao + 0.2f);
		//无阴影光
		if (light_need[i].type.y == 0)
		{
			diffuse += D;
			spec += S;
		}
		//shadowmap阴影
		else if (light_need[i].type.y == 1)
		{
			diffuse += (0.2f + 0.8f*rec_shadow)*D;
			spec += (0.2f + 0.8f*rec_shadow)*S;
		}
		//shadow volume阴影
		else
		{
			diffuse += D;
			spec += S;
		}
	}
	float4 final_color = tex_color * (ambient + diffuse) + spec;
	final_color.a = tex_color.a;
	return tex_color;
}
float4 PS_alphatest(VertexOut pin) :SV_TARGET
{
	float4 tex_color = texture_diffuse.Sample(samTex_liner, pin.tex);
	clip(-(tex_color.a - 0.9f));

	pin.normal = normalize(pin.normal);
	pin.tangent = normalize(pin.tangent);
	//求解图片所在自空间->模型所在统一世界空间的变换矩阵
	float3 N = pin.normal;
	float3 T = normalize(pin.tangent - N * pin.tangent * N);
	float3 B = cross(N, T);
	float3x3 T2W = float3x3(T, B, N);
	float3 normal_map = texture_normal.Sample(samTex, pin.tex).rgb;//从法线贴图中获得法线采样
	normal_map = 2 * normal_map - 1;                               //将向量从图片坐标[0,1]转换至真实坐标[-1,1]  
	normal_map = normalize(mul(normal_map, T2W));                  //切线空间至世界空间
	pin.normal = normal_map;

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 eye_direct = normalize(position_view - pin.position_bef.xyz);
	//float3 eye_direct = normalize(position_view);
	float4 A = 0.0f, D = 0.0f, S = 0.0f;
	float4 A1 = 0.0f, D1 = 0.0f, S1 = 0.0f;
	//compute_dirlight(material_need, dir_light_need[i], pin.normal, eye_direct, A, D, S);
	float rec_shadow = CalcShadowFactor(samShadow, texture_shadow, pin.pos_shadow);

	pin.pos_ssao /= pin.pos_ssao.w;
	float rec_ssao = texture_ssao.Sample(samTex_liner, pin.pos_ssao.xy, 0.0f).r;

	for (int i = 0; i < 2; ++i)
	{
		//方向光(direction light)
		if (light_need[i].type.x == 0)
		{
			compute_dirlight(material_need, light_need[i], pin.normal, eye_direct, A, D, S);
		}
		//点光源(point light)
		else if (light_need[i].type.x == 1)
		{
			compute_pointlight(material_need, light_need[i], pin.position_bef, pin.normal, position_view, A, D, S);
		}
		//聚光灯(spot light)
		else
		{
			compute_spotlight(material_need, light_need[i], pin.position_bef, pin.normal, eye_direct, A, D, S);
		}
		//环境光
		ambient += A*(0.8f*rec_ssao + 0.2f);
		//无阴影光
		if (light_need[i].type.y == 0)
		{
			diffuse += D;
			spec += S;
		}
		//shadowmap阴影
		else if (light_need[i].type.y == 1)
		{
			diffuse += (0.2f + 0.8f*rec_shadow)*D;
			spec += (0.2f + 0.8f*rec_shadow)*S;
		}
		//shadow volume阴影
		else
		{
			diffuse += D;
			spec += S;
		}
	}
	float4 final_color = tex_color * (ambient + diffuse) + spec;
	final_color.a = tex_color.a;
	return tex_color;
}
technique11 LightTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
technique11 draw_withtexture
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withtex()));
	}
}
technique11 draw_withshadow
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withshadow()));
	}
}
technique11 draw_withshadownormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withshadownormal()));
	}
}
technique11 draw_withshadowssao
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withshadowssao()));
	}
}
technique11 draw_withshadowssaonormal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withshadowssaonormal()));
	}
}

technique11 draw_hair
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS_alphasave()));
	}
	pass P1
	{
		SetDepthStencilState(NoDepthWrites, 0);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_alphatest()));
		SetBlendState(alpha_blend, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
	}

	/*

	pass P2
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withshadowssaonormal()));
	}
	pass P3
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_withshadowssaonormal()));
	}*/
}