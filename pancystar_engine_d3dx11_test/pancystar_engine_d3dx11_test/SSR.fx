cbuffer PerFrame
{
	float4x4 gViewToTexSpace;      //用于3D重建的纹理级变换
	float4x4 invview_matrix;       //取景变换的逆
	float4x4 view_matrix;          //取景变换
	float3   view_position;        //视点位置
	float4   gFrustumCorners[4];   //3D重建的四个角，用于借助光栅化插值
	float4x4 view_matrix_cube[6];
	float3   center_position;
};
Texture2D gNormalDepthMap;
Texture2D gdepth_map;
Texture2D gcolorMap;
//Texture for pass 2
TextureCube texture_cube;
//TextureCube depth_cube;
TextureCube stencil_cube;
Texture2D ssrcolor_input;
Texture2D mask_input;
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

float2 get_depth(float now_distance, float3 ray_dir, float3 position)
{
	float3 now_3D_point = position + ray_dir * now_distance;
	float3 cube_ray_now = normalize(now_3D_point - center_position);
	float2 rzAstencil = stencil_cube.Sample(samNormalDepth, cube_ray_now);
	float rz = rzAstencil.g;
	rz = 1.0f / (9.996f - rz * 9.996f);
	uint cube_stencil = round(rzAstencil.r);
	float depth_3D_point = mul(float4(now_3D_point, 1.0f), view_matrix_cube[cube_stencil]).z;
	return float2(rz, depth_3D_point);
}

void compute_light_range(
	float st_find,
	float ed_find,
	float3 ray_dir,
	float3 position,
	out float ray_st,
	out float ray_end)
{
	ray_st = st_find;
	ray_end = ed_find;
	[unroll]
	for (int i = 0; i < 5; ++i)
	{
		float length_final_rec = (ray_st + ray_end) / 2.0f;
		float2 check_depth = get_depth(length_final_rec, ray_dir, position);
		if (check_depth.x > 99999.0f || check_depth.x < check_depth.y)
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
}

void compute_light_step(
	float ray_st,
	float ray_end,
	float3 ray_dir,
	float3 position,
	out float end_find)
{
	float step = 1.0f / 5;
	end_find = 0;
	float now_distance;
	float2 check_depth;

	//步长移近一位
	now_distance = ray_end * step * 1;
	check_depth = get_depth(now_distance, ray_dir, position);
	if (check_depth.x < check_depth.y)
	{
		end_find = now_distance;
	}
	else 
	{
		now_distance = ray_end * step * 2;
		check_depth = get_depth(now_distance, ray_dir, position);
		if (check_depth.x < check_depth.y)
		{
			end_find = now_distance;
		}
		else 
		{
			now_distance = ray_end * step * 3;
			check_depth = get_depth(now_distance, ray_dir, position);
			if (check_depth.x < check_depth.y)
			{
				end_find = now_distance;
			}
			else
			{
				now_distance = ray_end * step * 4;
				check_depth = get_depth(now_distance, ray_dir, position);
				if (check_depth.x < check_depth.y)
				{
					end_find = now_distance;
				}
				else
				{
					now_distance = ray_end * step * 5;
					check_depth = get_depth(now_distance, ray_dir, position);
					if (check_depth.x < check_depth.y)
					{
						end_find = now_distance;
					}
				}
			}
		}
	}
	
}

void compute_light_pos(
	float st_find,
	float end_find,
	float3 ray_dir,
	float3 position,
	out float now_distance,
    out float3 now_3D_point)
{
	[unroll]
	for (int i = 0; i < 10; ++i)
	{
		now_distance = (st_find + end_find) / 2.0f;
		float2 check_depth = get_depth(now_distance, ray_dir, position);
		if (check_depth.x < check_depth.y)
		{
			end_find = now_distance;
		}
		else
		{
			st_find = now_distance;
		}
	}
	now_3D_point = position + ray_dir * now_distance;
}


[earlydepthstencil]
pixelOut PS(VertexOut pin) : SV_Target
{
	pixelOut out_color;
	out_color.color_need = float4(0.0f, 0.0f, 0.0f, 99.0f);
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
	float3 ray_dir = reflect(normalize(position), n);
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
	float alpha_fade = pow(saturate(((20.0f - now_distance) / 20.0f)),4);
	float3 normal_test_sample = gNormalDepthMap.Sample(samNormalDepth, answer_sampleloc).xyz;
	float test_dot = dot(normalize(normal_test_sample), ray_dir);
	if (delta_save < 0.00005f)
	{
		float4 outputColor = gcolorMap.Sample(samTex_liner, answer_sampleloc);
		out_color.color_need = outputColor;
		out_color.color_need.a = alpha_fade;
		out_color.mask_need = float4(1.0f, 0.0f,0.0f, 1.0f);
	}
	return out_color;
	//return gcolorMap.Sample(samTex_liner, pin.Tex);
}
float4 PS_cube(VertexOut pin) : SV_Target
{
	float4 mask_color = mask_input.Sample(samNormalDepth, pin.Tex);
	float rec_ifuse = mask_color.r;
	float4 color_ssr_map = ssrcolor_input.Sample(samNormalDepth, pin.Tex);
	if (rec_ifuse > 0.8f) 
	{
		return color_ssr_map;
	}
	//还原点的世界坐标
	float4 normalDepth = gNormalDepthMap.Sample(samNormalDepth, pin.Tex);
	float3 n = normalDepth.xyz;
	float pz = gdepth_map.Sample(samNormalDepth, pin.Tex).r;
	float3 position = (pz / pin.ToFarPlane.z)*pin.ToFarPlane;
	float st_find = 0.0f, end_find = 1000.0f;
	float2 answer_sampleloc;
	position = mul(float4(position, 1.0f), invview_matrix).xyz;
	n = mul(float4(n, 0.0f), invview_matrix).xyz;
	float3 ray_dir = reflect(normalize(position - view_position), n);
	float ray_st = 0.0f, ray_end = 1000.0f;

	compute_light_range(st_find, end_find, ray_dir, position, ray_st, ray_end);
	//按步长寻找第一个交点所在的区域
	compute_light_step(ray_st, ray_end, ray_dir, position, end_find);
	float now_distance;
	float3 now_3D_point;
	compute_light_pos(st_find, end_find, ray_dir, position, now_distance, now_3D_point);

	float3 cube_ray2 = normalize(now_3D_point - center_position);
	//float4 cube_color = texture_cube.SampleLevel(samTex_liner, cube_ray2,0);
	float alpha_fade = pow(saturate(((20.0f - now_distance) / 20.0f)), 4);

	float4 final_color = texture_cube.SampleLevel(samTex_liner, cube_ray2, 0);
	final_color.a = alpha_fade;

	final_color = (1.0f - rec_ifuse) * final_color + rec_ifuse * color_ssr_map;
	return final_color;
}
technique11 draw_ssrmap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
	pass P1
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_cube()));
	}
}
