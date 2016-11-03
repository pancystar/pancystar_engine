cbuffer PerFrame
{
	float4x4 gViewToTexSpace;      //用于3D重建的纹理级变换
	float4x4 invview_matrix;       //取景变换的逆
	float4x4 view_matrix;          //取景变换
	float3   view_position;        //视点位置
	float4   gFrustumCorners[4];   //3D重建的四个角，用于借助光栅化插值
	float4x4 view_matrix_cube[6];
};
Texture2D gNormalDepthMap;
Texture2D gdepth_map;
Texture2D gcolorMap;
TextureCube texture_cube;
TextureCube depth_cube;
SamplerState samp
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};
SamplerState samTex_liner
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	// Set a very far depth value if sampling outside of the NormalDepth map
	// so we do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(1e5f, 0.0f, 0.0f, 1e5f);
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
	float3 ToFarPlane : TEXCOORD0;   //用于3D重建
	float2 Tex        : TEXCOORD1;   //纹理坐标
};
struct pixelOut
{
	float4 color_need;
	float4 mask_need;
};
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	//因为是前一个shader光栅化完毕的像素点，不需要做任何变化
	vout.PosH = float4(vin.pos, 1.0f);
	//把四个顶点设置完毕（四个角点的次序存储在法线的x分量里面）
	vout.ToFarPlane = gFrustumCorners[vin.normal.x].xyz;
	//记录下纹理坐标
	vout.Tex = vin.tex1;
	return vout;
}
//根据两点间的距离对遮挡情况赋予权值
[earlydepthstencil]
pixelOut PS(VertexOut pin) : SV_Target
{
	pixelOut out_color;
	out_color.color_need = float4(0.0f, 0.0f, 0.0f, 0.0f);
	out_color.mask_need = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//还原点的世界坐标
	float4 normalDepth = gNormalDepthMap.Sample(samNormalDepth, pin.Tex);
	float3 n = normalDepth.xyz;
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	//pz = 0.1f / (1.0f - pz);
	float3 position = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	float step = 1.0f / 20;
	float st_find = 0.0f, end_find = 1000.0f;
	float2 answer_sampleloc;
	//发射反射光线
	//position = mul(float4(position.xyz,1.0f),invview_matrix);
	//float3 ray_dir = reflect(normalize(position), n);
	//float3 cube_ray = reflect(mul(float4(normalize(position), 0.0f), invview_matrix).xyz, mul(float4(n, 0.0f), invview_matrix).xyz);
	//float4 view_pos_need = mul(float4(0.0f, 0.0f, 0.0f, 1.0f), invview_matrix);
	//float3 cube_ray = normalize(mul(float4(position, 1.0f), invview_matrix).xyz - float3(0.0f,5.0f,0.0f));
	float3 ray_dir = reflect(normalize(position), n);
	//float4 cube_color = texture_cube.SampleLevel(samTex_liner, ray_dir,0);
	//return cube_color;
	//return gcolorMap.Sample(samTex_liner, pin.Tex);
	/*
	float rec_step_alpha = dot(-normalize(position), n);
	if (rec_step_alpha > 0.9f)
	{
		return gcolorMap.Sample(samTex_liner, pin.Tex);
	}
	*/
	float ray_st = 0.0f,ray_end = 1000.0f;
	//二分探测射线段的精确长度
	[unroll]
	for (int i = 0; i < 15; ++i)
	{
		float length_final_rec = (ray_st + ray_end) / 2.0f;
		float3 now_3D_point = position + ray_dir * length_final_rec;
		float4 now_2D_position = mul(float4(now_3D_point, 1.0f), gViewToTexSpace);
		now_2D_position /= now_2D_position.w;
		float rz = gdepth_map.SampleLevel(samNormalDepth, now_2D_position.xy, 0.0f).r;
		if (rz > 99999.0f || rz < now_3D_point.z)
		{
			//线段外
			ray_end = length_final_rec;
		}
		else
		{
			//线段内
			ray_st = length_final_rec;
		}
	}
	// *(1 - rec_step_alpha);
	//按步长寻找第一个交点所在的区域
	[unroll]
	for (int i = 1; i <= 20; i++)
	{
		//步长移近一位
		float now_distance = ray_end * step * i;
		float3 now_3D_point = position + ray_dir * now_distance;
		//float3 now_3D_point = mul(float4(position + ray_dir * now_distance,1.0f), view_matrix).xyz;
		float4 now_2D_position = mul(float4(now_3D_point, 1.0f), gViewToTexSpace);
		now_2D_position /= now_2D_position.w;
		float rz = gdepth_map.SampleLevel(samNormalDepth, now_2D_position.xy, 0.0f).r;
		if (rz > 99999.0f)
		{
			step /= 2.0f;
		}
		if (rz < now_3D_point.z)
		{
			end_find = now_distance;
			break;
		}
	}
	//二分精确寻找第一个交点的详细位置
	float delta_save;
	float now_distance;
	float3 now_3D_point;
	[unroll]
	for (int i = 0; i < 25; ++i)
	{
		now_distance = (st_find + end_find) / 2.0f;
		//float3 now_3D_point = mul(float4(position + ray_dir * now_distance, 1.0f), view_matrix).xyz;
		now_3D_point = position + ray_dir * now_distance;
		float4 now_2D_position = mul(float4(now_3D_point, 1.0f), gViewToTexSpace);
		now_2D_position /= now_2D_position.w;
		float rz = gdepth_map.SampleLevel(samNormalDepth, now_2D_position.xy, 0.0f).r;
		answer_sampleloc = now_2D_position.xy;
		if (rz < now_3D_point.z)
		{
			end_find = now_distance;
		}
		else
		{
			st_find = now_distance;
		}
		delta_save = abs(rz - now_3D_point.z);
	}
	float alpha_fade = 0.7f * saturate(pow(((5.0f - now_distance) / 5.0f),3));
	float3 normal_test_sample = gNormalDepthMap.Sample(samNormalDepth, answer_sampleloc).xyz;
	float test_dot = dot(normalize(normal_test_sample), ray_dir);
	out_color.color_need = gcolorMap.Sample(samTex_liner, pin.Tex);
	if (delta_save < 0.00005f)
	{
		//float3 cube_ray = normalize(mul(float4(now_3D_point, 1.0f), invview_matrix).xyz - float3(0.0f, 5.0f, 0.0f));
		//float4 cube_color = depth_cube.SampleLevel(samTex_liner, cube_ray, 0) + texture_cube.SampleLevel(samTex_liner, cube_ray,0);
		//return cube_color;
		//float4 outputColor = (1.0f - alpha_fade) * gcolorMap.Sample(samTex_liner, pin.Tex) + alpha_fade * gcolorMap.Sample(samTex_liner, answer_sampleloc);
		float4 outputColor = gcolorMap.Sample(samTex_liner, answer_sampleloc);
		out_color.color_need = outputColor;
		out_color.mask_need = float4(1.0f, 0.0f, 0.0f, 0.0f);
		//return outputColor;
	}
	
	return out_color;
	//return gcolorMap.Sample(samTex_liner, pin.Tex);
}
float4 PS_cube(VertexOut pin) : SV_Target
{
	//还原点的世界坐标
	float4 normalDepth = gNormalDepthMap.Sample(samNormalDepth, pin.Tex);
	float3 n = normalDepth.xyz;
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	float3 position = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	float step = 1.0f / 10;
	float st_find = 0.0f, end_find = 1000.0f;
	float2 answer_sampleloc;
	position = mul(float4(position, 1.0f), invview_matrix).xyz;
	n = mul(float4(n, 0.0f), invview_matrix).xyz;
	float3 ray_dir = reflect(normalize(position - view_position), n);
	float ray_st = 0.0f,ray_end = 1000.0f;
	//二分探测射线段的精确长度
	[unroll]
	for (int i = 0; i < 5; ++i)
	{
		float length_final_rec = (ray_st + ray_end) / 2.0f;
		float3 now_3D_point = position + ray_dir * length_final_rec;
		float3 cube_ray_now = normalize(now_3D_point - float3(0.0f, 5.0f, 0.0f));
		float rz = depth_cube.SampleLevel(samTex_liner, cube_ray_now, 0).r;
		rz = 1.0f / (9.996f - rz * 9.996f);
		uint cube_stencil = round(texture_cube.SampleLevel(samNormalDepth, cube_ray_now, 0).a);
		float depth_3D_point = mul(float4(now_3D_point, 1.0f), view_matrix_cube[cube_stencil]).z;
		if (rz > 99999.0f || rz < depth_3D_point)
		{
		//线段外
			ray_end = length_final_rec;
		}
		else
		{
			//线段内
			ray_st = length_final_rec;
		}
	}
	//按步长寻找第一个交点所在的区域
	[unroll]
	for (int i = 1; i <= 10; i++)
	{
		//步长移近一位
		float now_distance = ray_end * step * i;
		float3 now_3D_point = position + ray_dir * now_distance;
		float3 cube_ray_now = normalize(now_3D_point - float3(0.0f, 5.0f, 0.0f));
		float rz = depth_cube.SampleLevel(samTex_liner, cube_ray_now, 0);
		rz = 1.0f / (9.996f - rz * 9.996f);
		uint cube_stencil = round(texture_cube.SampleLevel(samNormalDepth, cube_ray_now, 0).a);
		float depth_3D_point = mul(float4(now_3D_point, 1.0f), view_matrix_cube[cube_stencil]).z;
		if (rz > 99999.0f)
		{
			step /= 2.0f;
		}
		if (rz < depth_3D_point)
		{
			end_find = now_distance;
			break;
		}
	}
	//二分精确寻找第一个交点的详细位置
	float delta_save;
	float now_distance;
	float3 now_3D_point;
	[unroll]
	for (int i = 0; i < 15; ++i)
	{
		now_distance = (st_find + end_find) / 2.0f;
		now_3D_point = position + ray_dir * now_distance;
		float3 cube_ray_now = normalize(now_3D_point - float3(0.0f, 5.0f, 0.0f));
		float rz = depth_cube.SampleLevel(samTex_liner, cube_ray_now, 0);
		rz = 1.0f / (9.996f - rz * 9.996f);
		uint cube_stencil = round(texture_cube.SampleLevel(samNormalDepth, cube_ray_now, 0).a);
		float depth_3D_point = mul(float4(now_3D_point, 1.0f), view_matrix_cube[cube_stencil]).z;
		if (rz < depth_3D_point)
		{
			end_find = now_distance;
		}
		else
		{
			st_find = now_distance;
		}
		delta_save = abs(rz - now_3D_point.z);
	}
	float3 cube_ray2 = normalize(now_3D_point - float3(0.0f, 5.0f, 0.0f));
	//float4 cube_color = texture_cube.SampleLevel(samTex_liner, cube_ray2,0);

	float alpha_fade = 0.7f * saturate(pow(((5.0f - now_distance) / 5.0f),3));
	return texture_cube.SampleLevel(samTex_liner, cube_ray2, 0);
}
technique11 draw_ssrmap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_cube()));
	}
}
