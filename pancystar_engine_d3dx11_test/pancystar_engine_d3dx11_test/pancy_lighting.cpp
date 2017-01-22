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

light_control::light_control(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, int shadow_num_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	max_shadow_num = shadow_num_need;
}
void light_control::release()
{
	ShadowTextureArray->Release();
	shadow_map_resource->Release();
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
	light_input.init_texture(ShadowTextureArray, shadowmap_light_list.size()-1);
}
HRESULT light_control::create(shader_control *shader_need, geometry_control *geometry_lib, pancy_renderstate *renderstate_lib)
{
	shader_lib = shader_need;
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
	*/
	//ID3D11Texture2D* ShadowTextureArray;
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
	int count_light_point = 0, count_light_dir = 0, count_light_spot = 0,count = 0;
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
}
void light_control::draw_shadow()
{
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