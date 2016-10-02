#include"pancy_lighting.h"
void basic_lighting::set_light_ambient(float red, float green, float blue, float alpha)
{
	light_data.ambient = XMFLOAT4(red, green, green, alpha);
}
void basic_lighting::set_light_diffuse(float red, float green, float blue, float alpha)
{
	light_data.diffuse = XMFLOAT4(red, green, green, alpha);
}
void basic_lighting::set_light_specular(float red, float green, float blue, float alpha)
{
	light_data.specular = XMFLOAT4(red, green, green, alpha);
}
basic_lighting::basic_lighting(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state,geometry_control *geometry_lib_need)
{
	light_source_type = type_need_light;
	shadow_source_type = type_need_shadow;
	shader_lib = lib_need;
	device_pancy = device_need;
	contex_pancy = contex_need;
	renderstate_lib = render_state;
	geometry_lib = geometry_lib_need;
	if (type_need_light == direction_light)
	{
		init_comman_dirlight(type_need_shadow);
	}
	else if (type_need_light == point_light)
	{
		init_comman_pointlight(type_need_shadow);
	}
	else if (type_need_light == spot_light)
	{
		init_comman_spotlight(type_need_shadow);
	}
}
void basic_lighting::init_comman_dirlight(shadow_type type_need_shadow)
{
	XMFLOAT4 rec_ambient(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT3 rec_dir(0.0f, -1.0f, 0.0f);
	light_data.ambient = rec_ambient;
	light_data.diffuse = rec_diffuse;
	light_data.specular = rec_specular;
	light_data.dir = rec_dir;
	light_data.range = 10.0f;
	light_data.light_type.x = direction_light;
	light_data.light_type.y = type_need_shadow;
}
void basic_lighting::init_comman_pointlight(shadow_type type_need_shadow)
{
	XMFLOAT4 rec_ambient1(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT3 rec_decay(0.0f, 0.6f, 0.0f);

	light_data.ambient = rec_ambient1;
	light_data.diffuse = rec_diffuse1;
	light_data.specular = rec_specular1;
	light_data.decay = rec_decay;
	light_data.range = 100.0f;
	light_data.position = XMFLOAT3(0.0f, 15.0f, 0.0f);
	light_data.light_type.x = point_light;
	light_data.light_type.y = type_need_shadow;
}
void basic_lighting::init_comman_spotlight(shadow_type type_need_shadow)
{
	XMFLOAT4 rec_ambient1(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular1(1.0f, 1.0f, 1.0f, 1.0f);

	XMFLOAT3 rec_decay1(0.0f, 0.3f, 0.0f);

	light_data.ambient = rec_ambient1;
	light_data.diffuse = rec_diffuse1;
	light_data.specular = rec_specular1;
	light_data.decay = rec_decay1;
	light_data.range = 100.0f;
	light_data.position = XMFLOAT3(0.0f, 2.5f, 2.5f);
	light_data.dir = XMFLOAT3(0.0f, -1.0f, -1.0f);
	light_data.spot = 12.0f;
	light_data.theta = 3.141592653f / 5.0f;
	light_data.light_type.x = spot_light;
	light_data.light_type.y = type_need_shadow;
}
void basic_lighting::set_frontlight(int light_num)
{
	auto* shader_test = shader_lib->get_shader_prelight();
	shader_test->set_light(light_data, light_num);
}

light_with_shadowmap::light_with_shadowmap(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state,geometry_control *geometry_lib_need) : basic_lighting(type_need_light, type_need_shadow, lib_need, device_need, contex_need, render_state, geometry_lib_need)
{
	shadowmap_deal = new shadow_basic(device_need, contex_need, shader_lib);
	cube_range.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	cube_range.Radius = sqrtf(1.0f*1.0f + 1.0f*1.0f);
}
HRESULT light_with_shadowmap::create(int width_need, int height_need)
{
	return shadowmap_deal->create(width_need, height_need);
}
void light_with_shadowmap::set_shadow_range(XMFLOAT3 center, float range)
{
	cube_range.Center = center;
	cube_range.Radius = range;
}
void light_with_shadowmap::draw_shadow()
{
	//更新渲染状态
	contex_pancy->RSSetState(renderstate_lib->get_CULL_front_rs());
	shadowmap_deal->set_renderstate(light_data.position, light_data.dir, cube_range, spot_light);
	//绘制阴影
	scene_geometry_list *list = geometry_lib->get_model_list();
	geometry_member *now_rec = list->get_geometry_head();
	for (int i = 0; i < list->get_geometry_num(); ++i)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~全部几何体(不透明)的阴影~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//设置世界变换矩阵
		shadowmap_deal->set_shaderresource(now_rec->get_world_matrix());
		if (now_rec->check_if_skin() == true)
		{
			shadowmap_deal->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
			now_rec->draw_full_geometry(shadowmap_deal->get_technique_skin());
		}
		else
		{
			now_rec->draw_full_geometry(shadowmap_deal->get_technique());
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~半透明部分的阴影~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		for (int i = 0; i < now_rec->get_geometry_data()->get_meshnum(); ++i)
		{
			if (now_rec->get_geometry_data()->check_alpha(i))
			{
				material_list rec_mat;
				now_rec->get_geometry_data()->get_texture(&rec_mat,i);
				//设置世界变换矩阵
				shadowmap_deal->set_shaderresource(now_rec->get_world_matrix());
				//设置半透明纹理
				shadowmap_deal->set_transparent_tex(rec_mat.tex_diffuse_resource);
				if (now_rec->check_if_skin() == true)
				{
					shadowmap_deal->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
					now_rec->draw_transparent_part(shadowmap_deal->get_technique_skin_transparent(),i);
				}
				else
				{
					now_rec->draw_transparent_part(shadowmap_deal->get_technique_transparent(),i);
				}
			}
		}
	}
	//还原渲染状态
	contex_pancy->RSSetState(0);
	renderstate_lib->set_posttreatment_rendertarget();
}
void light_with_shadowmap::release()
{
	shadowmap_deal->release();
}

light_with_shadowvolume::light_with_shadowvolume(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state,geometry_control *geometry_lib_need) : basic_lighting(type_need_light, type_need_shadow, lib_need, device_need, contex_need, render_state,geometry_lib_need)
{
	shadowvolume_deal = new pancy_shadow_volume(device_need, contex_need, shader_lib);
}
void light_with_shadowvolume::update_view_proj_matrix(XMFLOAT4X4 mat_need)
{
	shadowvolume_deal->set_view_projmat(mat_need);
}
HRESULT light_with_shadowvolume::create(int vertex_num)
{
	return shadowvolume_deal->create(vertex_num);
}
void light_with_shadowvolume::build_shadow(ID3D11DepthStencilView* depth_input)
{
	//更新渲染状态
	shadowvolume_deal->set_renderstate(depth_input, light_data.position, light_data.dir, spot_light);
	//renderstate_lib->set_posttreatment_rendertarget();
	//绘制阴影
//	for (auto now_rec = shadowmesh_list.begin(); now_rec != shadowmesh_list.end(); ++now_rec)
	//{
		
		//半透明部分阴影
	//	if (now_rec._Ptr->check_if_trans() == true)
	//	{
			//设置世界变换矩阵
	//		shadowvolume_deal->set_shaderresource(now_rec._Ptr->get_world_matrix());
			/*
			//设置半透明纹理
			shadowvolume_deal->set_transparent_tex(now_rec._Ptr->get_transparent_tex());
			*/
			//now_rec._Ptr->draw_transparent_part(shadowvolume_deal->get_technique_transparent());
	//	}
	/*
		//全部几何体的阴影
		else
		{
			//设置世界变换矩阵
			shadowvolume_deal->set_shaderresource(now_rec._Ptr->get_world_matrix());

			now_rec._Ptr->draw_full_geometry_adj(shadowvolume_deal->get_technique());
		}
		*/
	//}
	//还原渲染状态
	contex_pancy->RSSetState(0);
	//renderstate_lib->set_posttreatment_rendertarget(depth_input);
}
void light_with_shadowvolume::release()
{
	shadowvolume_deal->release();
}
void light_with_shadowvolume::draw_shadow_volume()
{
	shadowvolume_deal->draw_SOvertex();
}
