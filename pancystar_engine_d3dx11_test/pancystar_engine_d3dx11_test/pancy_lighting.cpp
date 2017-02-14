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
void basic_lighting::set_light_position(float x, float y, float z)
{
	light_data.position = XMFLOAT3(x, y, z);
}
void basic_lighting::set_light_dir(float x, float y, float z)
{
	light_data.dir = XMFLOAT3(x, y, z);
}
void basic_lighting::set_light_decay(float x0, float x1, float x2)
{
	light_data.decay = XMFLOAT3(x0, x1, x2);
}
void basic_lighting::set_light_range(float range_need)
{
	light_data.range = range_need;
}
void basic_lighting::set_light_spottheata(float theta)
{
	light_data.theta = theta;
}
void basic_lighting::set_light_spotstrenth(float spot)
{
	light_data.spot = spot;
}
basic_lighting::basic_lighting(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, geometry_control *geometry_lib_need)
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
	XMFLOAT4 rec_ambient(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 rec_diffuse(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_specular(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT3 rec_dir(-1.0f, -1.0f, 0.0f);
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
	light_data.range = 150.0f;
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
void basic_lighting::set_defferedlight(int light_num)
{
	auto* shader_test = shader_lib->get_shader_defferedlight_lightbuffer();
	shader_test->set_light(light_data, light_num);
}
light_with_shadowmap::light_with_shadowmap(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, geometry_control *geometry_lib_need) : basic_lighting(type_need_light, type_need_shadow, lib_need, device_need, contex_need, render_state, geometry_lib_need)
{
	shadowmap_deal = new shadow_basic(device_need, contex_need, shader_lib);
	cube_range.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	cube_range.Radius = sqrtf(10.0f*10.0f + 10.0f*10.0f);
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
	//contex_pancy->RSSetState(renderstate_lib->get_CULL_front_rs());
	shadowmap_deal->set_renderstate(light_data.position, light_data.dir, cube_range, static_cast<light_type>(light_data.light_type.x));
	//绘制阴影
	//geometry_ResourceView_list *list = geometry_lib->get_model_list();
	//assimpmodel_resource_view *now_rec = list->get_geometry_head();
	for (int i = 0; i < geometry_lib->get_assimp_model_view_num(); ++i)
	{
		//assimpmodel_resource_view *now_rec = list->get_geometry_byindex(1);
		assimpmodel_resource_view *now_rec = geometry_lib->get_assimp_ModelResourceView_by_index(i);
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
		for (int i = 0; i < now_rec->get_geometry_num(); ++i)
		{
			if (now_rec->check_alpha(i))
			{
				material_list rec_mat;
				now_rec->get_texture(&rec_mat, i);
				//设置世界变换矩阵
				shadowmap_deal->set_shaderresource(now_rec->get_world_matrix());
				//设置半透明纹理
				shadowmap_deal->set_transparent_tex(rec_mat.tex_diffuse_resource);
				if (now_rec->check_if_skin() == true)
				{
					shadowmap_deal->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
					now_rec->draw_mesh_part(shadowmap_deal->get_technique_skin_transparent(), i);
				}
				else
				{
					now_rec->draw_mesh_part(shadowmap_deal->get_technique_transparent(), i);
				}
			}
		}
	}
	for (int i = 0; i < geometry_lib->get_BuiltIn_model_view_num(); ++i)
	{
		buildin_geometry_resource_view *now_rec = geometry_lib->get_buildin_GeometryResourceView_by_index(i);
		shadowmap_deal->set_shaderresource(now_rec->get_world_matrix());
		now_rec->draw_full_geometry(shadowmap_deal->get_technique());
	}
	//还原渲染状态
	contex_pancy->RSSetState(0);
	renderstate_lib->set_posttreatment_rendertarget();
}
void light_with_shadowmap::release()
{
	shadowmap_deal->release();
}


sunlight_with_shadowmap::sunlight_with_shadowmap(shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, geometry_control *geometry_lib_need) : basic_lighting(direction_light, shadow_map, lib_need, device_need, contex_need, render_state, geometry_lib_need)
{
	for (int i = 0; i < 20; ++i)
	{
		shadowmap_array[i] = NULL;
	}
}
HRESULT sunlight_with_shadowmap::create(int width_need, int height_need, int shadow_num)
{
	shadow_devide = shadow_num;
	for (int i = 0; i < shadow_devide; ++i)
	{
		shadowmap_array[i] = new shadow_basic(device_pancy, contex_pancy, shader_lib);
	}
	HRESULT hr;
	for (int i = 0; i < shadow_devide; ++i)
	{
		hr = shadowmap_array[i]->create(width_need, height_need);
		if (FAILED(hr))
		{
			return hr;
		}
	}
	return S_OK;
}
void sunlight_with_shadowmap::draw_shadow()
{
	for (int i = 0; i < shadow_devide; ++i)
	{
		draw_shadow_basic(i);
	}
}
void sunlight_with_shadowmap::draw_shadow_basic(int count)
{
	//更新渲染状态
	shadowmap_array[count]->set_renderstate(mat_sunlight_pssm[count]);
	//绘制阴影
	for (int i = 0; i < geometry_lib->get_assimp_model_view_num(); ++i)
	{
		assimpmodel_resource_view *now_rec = geometry_lib->get_assimp_ModelResourceView_by_index(i);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~全部几何体(不透明)的阴影~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//设置世界变换矩阵
		shadowmap_array[count]->set_shaderresource(now_rec->get_world_matrix());
		if (now_rec->check_if_skin() == true)
		{
			shadowmap_array[count]->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
			now_rec->draw_full_geometry(shadowmap_array[count]->get_technique_skin());
		}
		else
		{
			now_rec->draw_full_geometry(shadowmap_array[count]->get_technique());
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~半透明部分的阴影~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		for (int i = 0; i < now_rec->get_geometry_num(); ++i)
		{
			if (now_rec->check_alpha(i))
			{
				material_list rec_mat;
				now_rec->get_texture(&rec_mat, i);
				//设置世界变换矩阵
				shadowmap_array[count]->set_shaderresource(now_rec->get_world_matrix());
				//设置半透明纹理
				shadowmap_array[count]->set_transparent_tex(rec_mat.tex_diffuse_resource);
				if (now_rec->check_if_skin() == true)
				{
					shadowmap_array[count]->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
					now_rec->draw_mesh_part(shadowmap_array[count]->get_technique_skin_transparent(), i);
				}
				else
				{
					now_rec->draw_mesh_part(shadowmap_array[count]->get_technique_transparent(), i);
				}
			}
		}
	}
	for (int i = 0; i < geometry_lib->get_BuiltIn_model_view_num(); ++i)
	{
		buildin_geometry_resource_view *now_rec = geometry_lib->get_buildin_GeometryResourceView_by_index(i);
		shadowmap_array[count]->set_shaderresource(now_rec->get_world_matrix());
		now_rec->draw_full_geometry(shadowmap_array[count]->get_technique());
	}
	//还原渲染状态
	contex_pancy->RSSetState(0);
	renderstate_lib->set_posttreatment_rendertarget();
}
void sunlight_with_shadowmap::release()
{
	for (int i = 0; i < shadow_devide; ++i)
	{
		shadowmap_array[i]->release();
	}
}
void sunlight_with_shadowmap::set_deffered_sunlight()
{
	auto* shader_test = shader_lib->get_shader_defferedlight_lightbuffer();
	shader_test->set_sunlight(light_data);
}
light_with_shadowvolume::light_with_shadowvolume(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, geometry_control *geometry_lib_need) : basic_lighting(type_need_light, type_need_shadow, lib_need, device_need, contex_need, render_state, geometry_lib_need)
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
			//now_rec._Ptr->draw_mesh_part(shadowvolume_deal->get_technique_transparent());
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

light_control::light_control(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_camera *camera_need, int shadow_num_need, float near_plane, float far_plane, float angle_view, int width_need, int height_need)
{
	scene_camera = camera_need;
	device_pancy = device_need;
	contex_pancy = contex_need;
	max_shadow_num = shadow_num_need;
	perspective_near_plane = near_plane;
	perspective_far_plane = far_plane;
	perspective_angle = angle_view;
	wind_width = width_need;
	wind_height = height_need;
	sunlight_divide_num = 0;
	if_sunlight_open = false;
}
void light_control::release()
{
	ShadowTextureArray->Release();
	shadow_map_resource->Release();
	sunlight_pssm_ShadowArray->Release();
	sunlight_pssm_Shadowresource->Release();
	sun_pssmshadow_light->release();
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		rec_shadow_light._Ptr->release();
	}
	for (auto rec_shadow_volume = shadowvalume_light_list.begin(); rec_shadow_volume != shadowvalume_light_list.end(); ++rec_shadow_volume)
	{
		rec_shadow_volume._Ptr->release();
	}
}
void light_control::add_light_without_shadow(basic_lighting light_input)
{
	nonshadow_light_list.push_back(light_input);
}
void light_control::add_light_witn_shadow_map(light_with_shadowmap light_input)
{
	shadowmap_light_list.push_back(light_input);
	light_input.init_texture(ShadowTextureArray, shadowmap_light_list.size() - 1);
}
HRESULT light_control::create(shader_control *shader_need, geometry_control *geometry_lib, pancy_renderstate *renderstate_lib)
{
	geometry_pancy = geometry_lib;
	shader_lib = shader_need;
	sun_pssmshadow_light = new sunlight_with_shadowmap(shader_lib, device_pancy, contex_pancy, renderstate_lib, geometry_lib);
	HRESULT hr;
	/*
	basic_lighting rec_need(point_light, shadow_none, shader_lib, device_pancy, contex_pancy, renderstate_lib, geometry_lib);
	nonshadow_light_list.push_back(rec_need);
	rec_need.set_light_range(5.0f);
	rec_need.set_light_ambient(0.0f,0.0f,0.0f,0.0f);
	rec_need.set_light_diffuse(0.3f, 0.3f, 0.0f,0.0f);
	rec_need.set_light_specular(0.3f, 0.3f, 0.0f, 0.0f);
	rec_need.set_light_decay(0.2f,1.2f,0.0f);
	nonshadow_light_list.push_back(rec_need);
	for (int i = 0; i < 6; ++i)
	{
		rec_need.set_light_position(7.3f, 4.8f, 13.0f - 4.8f*i);
		//nonshadow_light_list.push_back(rec_need);
	}
	for (int i = 0; i < 6; ++i)
	{
		rec_need.set_light_position(-7.3f, 4.8f, 12.85f - 4.8*i);
		//nonshadow_light_list.push_back(rec_need);
	}
	light_with_shadowmap rec_shadow(spot_light, shadow_map, shader_lib, device_pancy, contex_pancy, renderstate_lib, geometry_lib);
	hr = rec_shadow.create(1024, 1024);
	if (FAILED(hr))
	{
		return hr;
	}
	shadowmap_light_list.push_back(rec_shadow);

	//ID3D11Texture2D* ShadowTextureArray;
	*/

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~普通光源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//~~~~~~~~~~~~~~~~~~~~创建阴影图纹理数组资源~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.Height = 1024;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = max_shadow_num;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, NULL, &ShadowTextureArray);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"create shadowmap texarray error", L"tip", MB_OK);
		return hr;
	}
	//~~~~~~~~~~~~~~~~~~~~创建阴影图纹理数组访问器~~~~~~~~~~~~~~~~~~~
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd = {
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
	};
	dsrvd.Texture2DArray.ArraySize = max_shadow_num;
	dsrvd.Texture2DArray.FirstArraySlice = 0;
	dsrvd.Texture2DArray.MipLevels = 1;
	dsrvd.Texture2DArray.MostDetailedMip = 0;
	hr = device_pancy->CreateShaderResourceView(ShadowTextureArray, &dsrvd, &shadow_map_resource);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"create shadowmap texarray error", L"tip", MB_OK);
		return hr;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~太阳光源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//~~~~~~~~~~~~~~~~~~~~创建阴影图纹理数组资源~~~~~~~~~~~~~~~~~~~
	texDesc.Width = 1024;
	texDesc.Height = 1024;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 5;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, NULL, &sunlight_pssm_ShadowArray);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"create sunlight shadowmap texarray error", L"tip", MB_OK);
		return hr;
	}
	//~~~~~~~~~~~~~~~~~~~~创建阴影图纹理数组访问器~~~~~~~~~~~~~~~~~~~
	dsrvd.Texture2DArray.ArraySize = 5;
	dsrvd.Texture2DArray.FirstArraySlice = 0;
	dsrvd.Texture2DArray.MipLevels = 1;
	dsrvd.Texture2DArray.MostDetailedMip = 0;
	hr = device_pancy->CreateShaderResourceView(sunlight_pssm_ShadowArray, &dsrvd, &sunlight_pssm_Shadowresource);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"create shadowmap texarray error", L"tip", MB_OK);
		return hr;
	}

	/*
	int shadow_count = 0;
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		rec_shadow_light._Ptr->init_texture(ShadowTextureArray, shadow_count++);
	}
	//ShadowTextureArray->Release();
	*/
	/*
	DXUT_SetDebugName(m_pCascadedShadowMapVarianceSRVArraySingle, "VSM Cascaded SM Var Array SRV");
	for (int index = 0; index < m_CopyOfCascadeConfig.m_nCascadeLevels; ++index)
	*/
	return S_OK;
}
void light_control::update_and_setlight()
{
	int count_light_point = 0, count_light_dir = 0, count_light_spot = 0, count = 0;
	int count_shadow_point = 0, count_shadow_dir = 0, count_shadow_spot = 0;
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		light_type check_shadow = rec_shadow_light._Ptr->get_light_type();
		if (check_shadow == direction_light)
		{
			rec_shadow_light._Ptr->set_frontlight(count);
			rec_shadow_light._Ptr->set_defferedlight(count);
			count_shadow_dir += 1;
			count += 1;
		}
	}
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{

		light_type check_shadow = rec_shadow_light._Ptr->get_light_type();
		if (check_shadow == spot_light)
		{
			rec_shadow_light._Ptr->set_frontlight(count);
			rec_shadow_light._Ptr->set_defferedlight(count);
			count_shadow_spot += 1;
			count += 1;
		}

	}
	//设置无影光源
	for (auto rec_non_light = nonshadow_light_list.begin(); rec_non_light != nonshadow_light_list.end(); ++rec_non_light)
	{
		rec_non_light._Ptr->set_frontlight(count);
		rec_non_light._Ptr->set_defferedlight(count);
		light_type check_light = rec_non_light._Ptr->get_light_type();
		if (check_light == direction_light)
		{
			count_light_dir += 1;
		}
		else if (check_light == point_light)
		{
			count_light_point += 1;
		}
		else if (check_light == spot_light)
		{
			count_light_spot += 1;
		}
		count += 1;
	}
	//设置shadowvolume光源
	for (auto rec_shadow_volume = shadowvalume_light_list.begin(); rec_shadow_volume != shadowvalume_light_list.end(); ++rec_shadow_volume)
	{
		//rec_shadow_volume._Ptr->set_frontlight(count);
		//rec_shadow_volume._Ptr->set_defferedlight(count);
		//count++;
		//rec_shadow_volume._Ptr->update_view_proj_matrix(view_proj);
	}
	XMFLOAT4X4 mat_shadow[30];
	int shadow_num;
	get_shadow_map_matrix(mat_shadow, shadow_num);
	auto shader_deffered = shader_lib->get_shader_defferedlight_lightbuffer();
	auto shader_pre = shader_lib->get_shader_prelight();
	shader_deffered->set_shadow_matrix(mat_shadow, shadow_num);
	shader_pre->set_shadow_matrix(mat_shadow, shadow_num);
	//shader_deffered->set_shadow_tex(shadow_map_resource);
	XMUINT3 shadownum = XMUINT3(count_shadow_dir, count_shadow_point, count_shadow_spot);
	XMUINT3 lightnum = XMUINT3(count_light_dir, count_light_point, count_light_spot);
	shader_pre->set_light_num(lightnum);
	shader_pre->set_shadow_num(shadownum);
	shader_deffered->set_light_num(lightnum);
	shader_deffered->set_shadow_num(shadownum);
	if (if_sunlight_open)
	{
		//计算太阳光下不同视截体区域对应的投影矩阵
		divide_view_frustum(sunlight_lamda_log, sunlight_divide_num);
		for (int i = 0; i < sunlight_divide_num; ++i)
		{
			sun_pssmshadow_light->update_view_space(mat_sunlight_pssm[i], i);
		}
	}
	XMFLOAT4 rec_depth_devide;
	rec_depth_devide.x = sunlight_pssm_depthdevide[1];
	rec_depth_devide.y = sunlight_pssm_depthdevide[2];
	rec_depth_devide.z = sunlight_pssm_depthdevide[3];
	rec_depth_devide.w = sunlight_pssm_depthdevide[4];
	shader_deffered->set_depth_devide(rec_depth_devide);
	XMUINT3 rec_sunlight_num = XMUINT3(sunlight_divide_num, 0, 0);
	shader_deffered->set_sunlight_num(rec_sunlight_num);
	shader_deffered->set_sunshadow_tex(sunlight_pssm_Shadowresource);
	XMFLOAT4X4 mat_sun_2tex[30];
	for (int i = 0; i < sunlight_divide_num; ++i)
	{
		mat_sun_2tex[i] = sun_pssmshadow_light->get_ViewProjTex_matrix(i);
	}
	shader_deffered->set_sunshadow_matrix(mat_sun_2tex, sunlight_divide_num);
	sun_pssmshadow_light->set_front_sunlight();
	sun_pssmshadow_light->set_deffered_sunlight();

}
void light_control::set_sunlight(XMFLOAT3 light_dir, float lamda_log, int devide_num, int width_need, int height_need)
{
	if_sunlight_open = true;
	sunlight_lamda_log = lamda_log;
	sunlight_divide_num = devide_num - 1;
	sun_pssmshadow_light->create(width_need, height_need, devide_num - 1);
	sun_pssmshadow_light->set_light_diffuse(0.5f,0.5f,0.5f,1.0f);
	XMVECTOR rec_dir = XMLoadFloat3(&light_dir);
	rec_dir = XMVector3Normalize(rec_dir);
	XMStoreFloat3(&sunlight_dir, rec_dir);
	sun_pssmshadow_light->set_light_dir(sunlight_dir.x, sunlight_dir.y, sunlight_dir.z);
	for (int i = 0; i < sunlight_divide_num; ++i)
	{
		sun_pssmshadow_light->init_texture(sunlight_pssm_ShadowArray, i, i);
	}

}


void light_control::divide_view_frustum(float lamda_log, int divide_num)
{
	float C_log, C_uni;
	for (int i = 0; i < divide_num+1; ++i)
	{
		float now_percent = static_cast<float>(i) / static_cast<float>(divide_num);
		C_log = perspective_near_plane * pow((perspective_far_plane / perspective_near_plane), now_percent);
		C_uni = perspective_near_plane + (perspective_far_plane - perspective_near_plane) * now_percent;
		sunlight_pssm_depthdevide[i] = C_log * lamda_log + (1.0f - lamda_log) * C_uni;
		if (i > 0)
		{
			mat_sunlight_pssm[i - 1] = build_matrix_sunlight(sunlight_pssm_depthdevide[0], sunlight_pssm_depthdevide[i], sunlight_dir);
		}
	}

}
XMFLOAT4X4 light_control::build_matrix_sunlight(float near_plane, float far_plane, XMFLOAT3 light_dir)
{
	//视截体的中心点
	XMFLOAT3 center_pos;
	/*
	//直接计算中心点坐标的方法
	XMFLOAT3 view_dir, view_pos;
	scene_camera->get_view_direct(&view_dir);
	XMVECTOR dir_view_vec = XMLoadFloat3(&view_dir);
	scene_camera->get_view_position(&view_pos);
	XMVECTOR now_near_center = XMLoadFloat3(&view_pos) , now_far_center = XMLoadFloat3(&view_pos);
	now_near_center +=  dir_view_vec * near_plane;
	now_far_center += dir_view_vec * far_plane;
	XMVECTOR now_center = (now_near_center + now_far_center) / 2.0f;
	XMStoreFloat3(&center_pos, now_center);
	*/
	//取景变换的逆变换
	XMFLOAT4X4 invview_float4x4;
	scene_camera->count_invview_matrix(&invview_float4x4);
	XMMATRIX invview_mat = XMLoadFloat4x4(&invview_float4x4);
	//将(0,0,(near+far) / 2)进行逆取景变换得到中心点的世界坐标
	XMVECTOR center_view = XMLoadFloat4(&XMFLOAT4(0.0f, 0.0f, (near_plane + far_plane) / 2.0f, 1.0f));
	XMVECTOR center_check = XMVector4Transform(center_view, invview_mat);
	XMStoreFloat3(&center_pos, center_check);
	//近截面的四个角点
	float aspect = static_cast<float>(wind_width) / static_cast<float>(wind_height);
	float halfHeight = near_plane * tanf(0.5f*perspective_angle);
	float halfWidth = aspect * halfHeight;
	XMFLOAT4 FrustumnearCorner[4];
	XMFLOAT4 FrustumFarCorner[4];
	FrustumnearCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, near_plane, 1.0f);
	FrustumnearCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, near_plane, 1.0f);
	FrustumnearCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, near_plane, 1.0f);
	FrustumnearCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, near_plane, 1.0f);
	for (int i = 0; i < 4; ++i)
	{
		//还原到世界坐标系
		XMVECTOR rec_now = XMLoadFloat4(&FrustumnearCorner[i]);
		XMVECTOR rec_ans = XMVector4Transform(rec_now, invview_mat);
		XMStoreFloat4(&FrustumnearCorner[i], rec_ans);
	}
	//远截面的四个角点
	aspect = static_cast<float>(wind_width) / static_cast<float>(wind_height);
	halfHeight = far_plane * tanf(0.5f*perspective_angle);
	halfWidth = aspect * halfHeight;
	FrustumFarCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, far_plane, 1.0f);
	FrustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, far_plane, 1.0f);
	FrustumFarCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, far_plane, 1.0f);
	FrustumFarCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, far_plane, 1.0f);
	for (int i = 0; i < 4; ++i)
	{
		//还原到世界坐标系
		XMVECTOR rec_now = XMLoadFloat4(&FrustumFarCorner[i]);
		XMVECTOR rec_ans = XMVector4Transform(rec_now, invview_mat);
		XMStoreFloat4(&FrustumFarCorner[i], rec_ans);
	}
	//光源视角取景变换
	XMVECTOR lightDir = XMLoadFloat3(&light_dir);
	XMVECTOR lightPos = center_check;
	XMFLOAT3 up_dir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR upDir = XMLoadFloat3(&up_dir);
	XMFLOAT3 check_dir;
	XMStoreFloat3(&check_dir,XMVector3Cross(lightDir, upDir));
	if (abs(check_dir.x) < 0.001f && abs(check_dir.y) < 0.001f && abs(check_dir.z) < 0.001f)
	{
		up_dir = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}
	XMFLOAT4X4 view_mat_pre;
	scene_camera->count_view_matrix(light_dir, up_dir, center_pos, &view_mat_pre);
	XMMATRIX light_view_mat = XMLoadFloat4x4(&view_mat_pre);
	XMVECTOR rec_check_center = XMVector4Transform(center_check, light_view_mat);
	XMFLOAT4 float_check_center;
	XMStoreFloat4(&float_check_center, rec_check_center);

	for (int i = 0; i < 4; ++i)
	{
		//变换到光源视角
		XMVECTOR rec_now = XMLoadFloat4(&FrustumnearCorner[i]);
		XMVECTOR rec_ans = XMVector4Transform(rec_now, light_view_mat);
		XMStoreFloat4(&FrustumnearCorner[i], rec_ans);
	}
	for (int i = 0; i < 4; ++i)
	{
		//变换到光源视角
		XMVECTOR rec_now = XMLoadFloat4(&FrustumFarCorner[i]);
		XMVECTOR rec_ans = XMVector4Transform(rec_now, light_view_mat);
		XMStoreFloat4(&FrustumFarCorner[i], rec_ans);
	}
	XMFLOAT3 min_pos;
	XMFLOAT3 max_pos;
	build_AABB_box(FrustumnearCorner, FrustumFarCorner, min_pos, max_pos);
	//将投影中心拉到合适的位置
	XMFLOAT3 new_center_pos = XMFLOAT3((min_pos.x + max_pos.x) / 2.0f, (min_pos.y + max_pos.y) / 2.0f, min_pos.z);
	new_center_pos.z -= 1.0f;
	//乘以取景变换的逆矩阵得到新的投影中心点在世界坐标系的位置
	XMFLOAT4X4 inv_view_light;
	scene_camera->count_invview_matrix(light_dir, up_dir, center_pos, &inv_view_light);
	XMMATRIX light_invview_mat = XMLoadFloat4x4(&inv_view_light);
	XMVECTOR new_center_vector = XMLoadFloat4(&XMFLOAT4(new_center_pos.x, new_center_pos.y, new_center_pos.z, 1.0f));
	XMVECTOR rec_trans_center = XMVector4Transform(new_center_vector, light_invview_mat);
	XMStoreFloat3(&new_center_pos, rec_trans_center);
	//重新计算取景变换矩阵
	scene_camera->count_view_matrix(light_dir, up_dir, new_center_pos, &view_mat_pre);
	//根据AABB包围盒的长宽高计算投影变换矩阵
	float view_width_need = max_pos.x - min_pos.x;
	float view_height_need = max_pos.y - min_pos.y;
	XMMATRIX mat_orth_view = XMMatrixOrthographicLH(view_width_need, view_height_need, 0.5f, (max_pos.z - min_pos.z) + 1.0f);
	//计算取景*投影矩阵
	light_view_mat = XMLoadFloat4x4(&view_mat_pre);
	XMMATRIX final_matrix = light_view_mat * mat_orth_view;
	XMFLOAT4X4 mat_view_project;
	XMStoreFloat4x4(&mat_view_project, final_matrix);
	return mat_view_project;
}
void light_control::build_AABB_box(XMFLOAT4 near_point[4], XMFLOAT4 far_point[4], XMFLOAT3 &min_pos, XMFLOAT3 &max_pos)
{
	min_pos = XMFLOAT3(99999.0f, 999999.0f, 99999.0f);
	max_pos = XMFLOAT3(-99999.0f, -99999.0f, -99999.0f);
	//包围盒算法求视截体
	for (int i = 0; i < 4; ++i)
	{
		if (near_point[i].x > max_pos.x)
		{
			max_pos.x = near_point[i].x;
		}
		if (near_point[i].x < min_pos.x)
		{
			min_pos.x = near_point[i].x;
		}

		if (near_point[i].y > max_pos.y)
		{
			max_pos.y = near_point[i].y;
		}
		if (near_point[i].y < min_pos.y)
		{
			min_pos.y = near_point[i].y;
		}

		if (near_point[i].z > max_pos.z)
		{
			max_pos.z = near_point[i].z;
		}
		if (near_point[i].z < min_pos.z)
		{
			min_pos.z = near_point[i].z;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (far_point[i].x > max_pos.x)
		{
			max_pos.x = far_point[i].x;
		}
		if (far_point[i].x < min_pos.x)
		{
			min_pos.x = far_point[i].x;
		}

		if (far_point[i].y > max_pos.y)
		{
			max_pos.y = far_point[i].y;
		}
		if (far_point[i].y < min_pos.y)
		{
			min_pos.y = far_point[i].y;
		}

		if (far_point[i].z > max_pos.z)
		{
			max_pos.z = far_point[i].z;
		}
		if (far_point[i].z < min_pos.z)
		{
			min_pos.z = far_point[i].z;
		}
	}
	//规范化视截体的视口
	if (min_pos.x < min_pos.y)
	{
		min_pos.y = min_pos.x;
	}
	else
	{
		min_pos.x = min_pos.y;
	}
	if (max_pos.x > max_pos.y)
	{
		max_pos.y = max_pos.x;
	}
	else
	{
		max_pos.x = max_pos.y;
	}
}
void light_control::draw_shadow()
{
	if (if_sunlight_open)
	{
		sun_pssmshadow_light->draw_shadow();
	}
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		rec_shadow_light._Ptr->draw_shadow();
	}
	for (auto rec_shadow_volume = shadowvalume_light_list.begin(); rec_shadow_volume != shadowvalume_light_list.end(); ++rec_shadow_volume)
	{
		//rec_shadow_volume._Ptr->build_shadow(ssao_part->get_depthstencilmap());
		//rec_shadow_volume._Ptr->draw_shadow_volume();
	}
	contex_pancy->RSSetState(NULL);
}
HRESULT light_control::get_shadow_map_matrix(XMFLOAT4X4* mat_out, int &mat_num_out)
{
	mat_num_out = 0;
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		mat_out[mat_num_out++] = rec_shadow_light._Ptr->get_ViewProjTex_matrix();
	}
	return S_OK;
}