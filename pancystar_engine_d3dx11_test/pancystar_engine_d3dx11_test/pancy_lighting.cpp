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
basic_lighting::basic_lighting(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state)
{
	light_source_type = type_need_light;
	shadow_source_type = type_need_shadow;
	shader_lib = lib_need;
	device_pancy = device_need;
	contex_pancy = contex_need;
	renderstate_lib = render_state;
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

light_with_shadowmap::light_with_shadowmap(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state) : basic_lighting(type_need_light, type_need_shadow, lib_need, device_need, contex_need, render_state)
{
	shadowmap_deal = new shadow_basic(device_need, contex_need, shader_lib);
	cube_range.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	cube_range.Radius = sqrtf(1.0f*1.0f + 1.0f*1.0f);
}
HRESULT light_with_shadowmap::create(int width_need,int height_need)
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
	for (auto now_rec = shadowmesh_list.begin(); now_rec != shadowmesh_list.end(); ++now_rec)
	{
		//半透明部分阴影
		if (now_rec._Ptr->check_if_trans() == true)
		{
			//设置世界变换矩阵
			shadowmap_deal->set_shaderresource(now_rec._Ptr->get_world_matrix());
			//设置半透明纹理
			shadowmap_deal->set_transparent_tex(now_rec._Ptr->get_transparent_tex());
			now_rec._Ptr->draw_transparent_part(shadowmap_deal->get_technique_transparent());
		}
		//全部几何体的阴影
		else
		{
			//设置世界变换矩阵
			shadowmap_deal->set_shaderresource(now_rec._Ptr->get_world_matrix());
			
			now_rec._Ptr->draw_full_geometry(shadowmap_deal->get_technique());
		}
	}
	//还原渲染状态
	contex_pancy->RSSetState(0);
}
void light_with_shadowmap::release()
{
	shadowmap_deal->release();
}
void light_with_shadowmap::clear_mesh()
{
	shadowmesh_list.clear();
}
void light_with_shadowmap::add_mesh(geometry_shadow mesh_input)
{
	shadowmesh_list.push_back(mesh_input);
}
