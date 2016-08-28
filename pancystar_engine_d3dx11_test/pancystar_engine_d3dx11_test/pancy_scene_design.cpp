#include"pancy_scene_design.h"
class basic_lighting
{
	light_type light_source_type;
	shadow_type shadow_source_type;
	pancy_light_basic light_data;
	shader_control   *shader_lib;       //shader资源
public:
	basic_lighting(light_type type_need_light, shadow_type type_need_shadow,shader_control *lib_need);
	//前向光渲染
	void set_frontlight(int light_num);
	void set_light_ambient(float red, float green, float blue,float alpha);
	void set_light_diffuse(float red, float green, float blue, float alpha);
	void set_light_specular(float red, float green, float blue, float alpha);
private:
	void init_comman_dirlight(shadow_type type_need_shadow);
	void init_comman_pointlight(shadow_type type_need_shadow);
	void init_comman_spotlight(shadow_type type_need_shadow);
};
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
basic_lighting::basic_lighting(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need)
{
	light_source_type = type_need_light;
	shadow_source_type = type_need_shadow;
	shader_lib = lib_need;
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
	light_data.position = XMFLOAT3(0.0f, 5.0f, 5.0f);
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



scene_root::scene_root(d3d_pancy_basic *engine_root, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, int width, int height)
{
	user_input = input_need;
	scene_camera = camera_need;
	shader_lib = lib_need;
	device_pancy = device_need;
	contex_pancy = contex_need;
	scene_window_width = width;
	scene_window_height = height;
	engine_state = engine_root;
	renderstate_lib = render_state;
	//初始化投影以及取景变换矩阵
	XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, scene_window_width*1.0f / scene_window_height, 0.1f, 1000.f);
	XMStoreFloat4x4(&proj_matrix, proj);
	XMMATRIX iden = XMMatrixIdentity();
	XMStoreFloat4x4(&view_matrix, iden);
}
HRESULT scene_root::camera_move()
{
	XMMATRIX view;
	user_input->get_input();
	if (user_input->check_keyboard(DIK_A))
	{
		scene_camera->walk_right(-0.01f);
	}
	if (user_input->check_keyboard(DIK_W))
	{
		scene_camera->walk_front(0.01f);
	}
	if (user_input->check_keyboard(DIK_R))
	{
		scene_camera->walk_up(0.01f);
	}
	if (user_input->check_keyboard(DIK_D))
	{
		scene_camera->walk_right(0.01f);
	}
	if (user_input->check_keyboard(DIK_S))
	{
		scene_camera->walk_front(-0.01f);
	}
	if (user_input->check_keyboard(DIK_F))
	{
		scene_camera->walk_up(-0.01f);
	}
	if (user_input->check_keyboard(DIK_Q))
	{
		scene_camera->rotation_look(0.001f);
	}
	if (user_input->check_keyboard(DIK_E))
	{
		scene_camera->rotation_look(-0.001f);
	}
	if (user_input->check_mouseDown(1))
	{
		scene_camera->rotation_up(user_input->MouseMove_X() * 0.001f);
		scene_camera->rotation_right(user_input->MouseMove_Y() * 0.001f);
	}
	scene_camera->count_view_matrix(&view);
	XMStoreFloat4x4(&view_matrix, view);
	return S_OK;
}

scene_engine_test::scene_engine_test(d3d_pancy_basic *engine_root, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, int width, int height) : scene_root(engine_root, device_need, contex_need, render_state, input_need, camera_need, lib_need, width, height)
{
	floor_need = new mesh_cubewithtargent(device_need, contex_need);
	ball_need = new mesh_ball(device_need, contex_need, 50, 50);
	model_yuri = new model_reader_assimp(device_need, contex_need, "yurimodel\\yuri.obj", "yurimodel\\");
	model_castel = new model_reader_assimp(device_need, contex_need, "castelmodel\\castel.obj", "castelmodel\\");
	shadowmap_part = new shadow_basic(device_need, contex_need, shader_lib);
	ssao_part = new ssao_pancy(device_need, contex_need, shader_lib, scene_window_width, scene_window_height, XM_PI*0.25f, 1000.0f);
	tex_floor = NULL;
	tex_normal = NULL;
	tex_skycube = NULL;
}
HRESULT scene_engine_test::scene_create()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	/*
	pancy_light_basic test_pointL;
	XMFLOAT4 rec_ambient(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT3 rec_dir(0.0f, -1.0f, 0.0f);
	test_pointL.ambient = rec_ambient;
	test_pointL.diffuse = rec_diffuse;
	test_pointL.specular = rec_specular;
	test_pointL.dir = rec_dir;
	test_pointL.range = 10.0f;
	test_pointL.light_type.x = 0;
	test_pointL.light_type.y = 0;
	shader_test->set_light(test_pointL, 0);
	*/
	pancy_light_basic test_point2;
	XMFLOAT4 rec_ambient1(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT3 rec_decay(0.0f, 0.6f, 0.0f);

	test_point2.ambient = rec_ambient1;
	test_point2.diffuse = rec_diffuse1;
	test_point2.specular = rec_specular1;
	test_point2.decay = rec_decay;
	test_point2.range = 100.0f;
	test_point2.position = XMFLOAT3(0.0f, 15.0f, 0.0f);
	test_point2.light_type.x = point_light;
	test_point2.light_type.y = shadow_none;

	shader_test->set_light(test_point2, 0);

	pancy_light_basic test_point3;
	XMFLOAT4 rec_ambient2(0.6f, 0.6f, 0.6f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 1.0f);

	XMFLOAT3 rec_decay1(0.0f, 0.3f, 0.0f);

	test_point3.ambient = rec_ambient1;
	test_point3.diffuse = rec_diffuse1;
	test_point3.specular = rec_specular1;
	test_point3.decay = rec_decay1;
	test_point3.range = 100.0f;
	test_point3.position = XMFLOAT3(0.0f, 5.0f, 5.0f);
	test_point3.dir = XMFLOAT3(0.0f, -1.0f, -1.0f);
	test_point3.spot = 12.0f;
	test_point3.theta = 3.141592653f / 5.0f;
	test_point3.light_type.x = spot_light;
	test_point3.light_type.y = shadow_map;
	shader_test->set_light(test_point3, 1);

	HRESULT hr_need;
	hr_need = floor_need->create_object();
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load object error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = ball_need->create_object();
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load object error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = CreateDDSTextureFromFile(device_pancy, L"floor.dds", 0, &tex_floor, 0, 0);
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load texture file error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = CreateDDSTextureFromFile(device_pancy, L"floor_n.dds", 0, &tex_normal, 0, 0);
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load texture file error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = CreateDDSTextureFromFile(device_pancy, L"Texture_cube.dds", 0, &tex_skycube, 0, 0);
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load texture file error", L"tip", MB_OK);
		return hr_need;
	}
	int alpha_yuri[] = { 3 };
	hr_need = model_yuri->model_create(false,1, alpha_yuri);
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load model file error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = model_castel->model_create(true,0,NULL);
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load model file error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = shadowmap_part->create(1024, 1024);
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load shadowmmap class error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = ssao_part->basic_create();
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load ssao class error", L"tip", MB_OK);
		return hr_need;
	}
	return S_OK;
}
HRESULT scene_engine_test::display()
{
	draw_shadowmap();
	draw_ssaomap();
	
	show_ball();
	show_lightsource();
	show_floor();
	show_castel();
	show_aotestproj();
	show_yuri();
	return S_OK;
}
void scene_engine_test::show_yuri()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need, *teque_normal,*teque_hair;
	shader_test->get_technique(&teque_need, "draw_withshadowssao");
	shader_test->get_technique(&teque_normal, "draw_withshadowssaonormal");
	shader_test->get_technique(&teque_hair, "draw_hair");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);


	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.5);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rotation_world = XMMatrixRotationY(3.141592653f);
	rec_world = scal_world * rotation_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);

	//设定阴影变换以及阴影贴图
	XMFLOAT4X4 shadow_matrix_pre = shadowmap_part->get_ViewProjTex_matrix();
	XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
	shadow_matrix = rec_world * shadow_matrix;
	XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
	shader_test->set_trans_shadow(&shadow_matrix_pre);
	shader_test->set_shadowtex(shadowmap_part->get_mapresource());
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//设定ssao变换及贴图
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMFLOAT4X4 ssao_matrix;
	XMStoreFloat4x4(&ssao_matrix, worldViewProj*T_need);
	shader_test->set_trans_ssao(&ssao_matrix);
	shader_test->set_ssaotex(ssao_part->get_aomap());
	//获取渲染路径并渲染
	//model_yuri->get_technique(teque_need);
	//model_yuri->draw_mesh();
	int yuri_render_order[11] = { 4,5,6,7,8,9,10,3,0,2,1 };
	for (int i = 0; i < 7; ++i)
	{
		//纹理设定
		material_list rec_need;
		model_yuri->get_texture(&rec_need, yuri_render_order[i]);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		if (rec_need.texture_normal_resource != NULL) 
		{
			model_yuri->get_technique(teque_normal);
			shader_test->set_normaltex(rec_need.texture_normal_resource);
		}
		else 
		{
			model_yuri->get_technique(teque_need);
		}
		//shader_test->set_normaltex(tex_normal);
		model_yuri->draw_part(yuri_render_order[i]);
	}		
	//alpha混合设定
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	contex_pancy->OMSetBlendState(renderstate_lib->get_blend_common(), blendFactor, 0xffffffff);
	for (int i = 8; i < model_yuri->get_meshnum(); ++i) 
	{
		//纹理设定
		material_list rec_need;
		model_yuri->get_texture(&rec_need, yuri_render_order[i]);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		if (rec_need.texture_normal_resource != NULL)
		{
			model_yuri->get_technique(teque_normal);
			shader_test->set_normaltex(rec_need.texture_normal_resource);
		}
		else
		{
			model_yuri->get_technique(teque_need);
		}
		model_yuri->draw_part(yuri_render_order[i]);
	}
	//绘制头发
	
	model_yuri->get_technique(teque_hair);
	material_list rec_need;
	model_yuri->get_texture(&rec_need, yuri_render_order[7]);
	shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
	shader_test->set_normaltex(rec_need.texture_normal_resource);
	model_yuri->draw_part(yuri_render_order[7]);
	contex_pancy->OMSetDepthStencilState(NULL,0);


	contex_pancy->OMSetBlendState(0, blendFactor, 0xffffffff);
	
}
void scene_engine_test::show_castel()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need,*teque_normal;
	shader_test->get_technique(&teque_need, "draw_withshadowssao");
	shader_test->get_technique(&teque_normal, "draw_withshadowssaonormal");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f,1.0f, 1.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);


	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);

	//设定阴影变换以及阴影贴图
	XMFLOAT4X4 shadow_matrix_pre = shadowmap_part->get_ViewProjTex_matrix();
	XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
	shadow_matrix = rec_world * shadow_matrix;
	XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
	shader_test->set_trans_shadow(&shadow_matrix_pre);
	shader_test->set_shadowtex(shadowmap_part->get_mapresource());
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//设定ssao变换及贴图
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMFLOAT4X4 ssao_matrix;
	XMStoreFloat4x4(&ssao_matrix, worldViewProj*T_need);
	shader_test->set_trans_ssao(&ssao_matrix);
	shader_test->set_ssaotex(ssao_part->get_aomap());
	//获取渲染路径并渲染
	//model_castel->get_technique(teque_need);
	//model_castel->draw_mesh();
	
	for (int i = 0; i < model_castel->get_meshnum(); ++i)
	{
		//纹理设定
		material_list rec_need;
		model_castel->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		if (rec_need.texture_normal_resource != NULL)
		{
			model_castel->get_technique(teque_normal);
			shader_test->set_normaltex(rec_need.texture_normal_resource);
		}
		else 
		{
			model_castel->get_technique(teque_need);
		}
		//shader_test->set_normaltex(tex_normal);
		model_castel->draw_part(i);
	}
}
void scene_engine_test::show_ball()
{
	contex_pancy->RSSetState(renderstate_lib->get_CULL_front_rs());
	auto* shader_test = shader_lib->get_shader_reflect();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_inside");

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(50.0f, 50.0f, 50.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定立方贴图
	shader_test->set_tex_resource(tex_skycube);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	ball_need->get_teque(teque_need);
	ball_need->show_mesh();
	contex_pancy->RSSetState(NULL);
}
void scene_engine_test::show_lightsource()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 5.0, 5.0);
	scal_world = XMMatrixScaling(0.2f, 0.2f, 0.2f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	floor_need->get_teque(teque_need);
	floor_need->show_mesh();
}
void scene_engine_test::show_floor()
{

	auto* shader_test = shader_lib->get_shader_prelight();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_withshadownormal");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0f, -1.2f, 0.0f);
	scal_world = XMMatrixScaling(15.0f, 0.55f, 15.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);

	//设定阴影变换以及阴影贴图
	XMFLOAT4X4 shadow_matrix_pre = shadowmap_part->get_ViewProjTex_matrix();
	XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
	shadow_matrix = rec_world * shadow_matrix;
	XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
	shader_test->set_trans_shadow(&shadow_matrix_pre);
	shader_test->set_shadowtex(shadowmap_part->get_mapresource());


	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	floor_need->get_teque(teque_need);
	floor_need->show_mesh();
}
void scene_engine_test::show_aotestproj()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_withshadowssao");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(0.6f, 0.6f, 0.6f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0f, 0.0f, -1.1f);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);

	//设定阴影变换以及阴影贴图
	XMFLOAT4X4 shadow_matrix_pre = shadowmap_part->get_ViewProjTex_matrix();
	XMMATRIX shadow_matrix = XMLoadFloat4x4(&shadow_matrix_pre);
	shadow_matrix = rec_world * shadow_matrix;
	XMStoreFloat4x4(&shadow_matrix_pre, shadow_matrix);
	shader_test->set_trans_shadow(&shadow_matrix_pre);
	shader_test->set_shadowtex(shadowmap_part->get_mapresource());


	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//设定ssao变换及贴图
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMFLOAT4X4 ssao_matrix;
	XMStoreFloat4x4(&ssao_matrix, worldViewProj*T_need);
	shader_test->set_trans_ssao(&ssao_matrix);
	shader_test->set_ssaotex(ssao_part->get_aomap());
	floor_need->get_teque(teque_need);
	floor_need->show_mesh();
}
void scene_engine_test::draw_shadowmap()
{
	contex_pancy->RSSetState(renderstate_lib->get_CULL_front_rs());
	XMFLOAT3 position = XMFLOAT3(0.0, 5.0, 5.0);
	XMFLOAT3 dir = XMFLOAT3(0.0, -1.0, -1.0);

	BoundingSphere  cube_range;
	cube_range.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	cube_range.Radius = sqrtf(1.0f*1.0f + 1.0f*1.0f);
	shadowmap_part->set_renderstate(position, dir, cube_range, spot_light);
	//engine_state->set_posttreatment_rendertarget();
	//设定球体世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	XMMATRIX rotation_world;
	//设定yuri世界变换
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.5);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rotation_world = XMMatrixRotationY(3.141592653f);
	rec_world = scal_world * rotation_world* trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shadowmap_part->set_shaderresource(world_matrix);
	model_yuri->get_technique(shadowmap_part->get_technique());
	model_yuri->draw_mesh();
	material_list rec_need;
	model_yuri->get_texture(&rec_need, 3);
	shadowmap_part->set_transparent_tex(rec_need.tex_diffuse_resource);
	model_yuri->draw_part(3);
	//设定ao测试板世界变换
	trans_world = XMMatrixTranslation(0.0f, 0.0f, -1.1f);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shadowmap_part->set_shaderresource(world_matrix);
	floor_need->get_teque(shadowmap_part->get_technique());
	floor_need->show_mesh();

	
	//设定地面世界变换
	trans_world = XMMatrixTranslation(0.0f, -1.2f, 0.0f);
	scal_world = XMMatrixScaling(15.0f, 0.55f, 15.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shadowmap_part->set_shaderresource(world_matrix);
	floor_need->get_teque(shadowmap_part->get_technique());
	floor_need->show_mesh();
	engine_state->set_posttreatment_rendertarget();

	contex_pancy->RSSetState(0);
}
void scene_engine_test::draw_ssaomap()
{
	ssao_part->set_normaldepth_target(NULL);
	//设定球体世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	XMFLOAT4X4 final_matrix;
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	//设定yuri世界变换
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.5);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rotation_world = XMMatrixRotationY(3.141592653f);
	rec_world = scal_world * rotation_world* trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	//设定总变换
	world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	worldViewProj = world_matrix_rec*view*proj;
	XMStoreFloat4x4(&final_matrix, worldViewProj);
	ssao_part->set_normaldepth_mat(world_matrix, view_matrix, final_matrix);
	model_yuri->get_technique(ssao_part->get_technique());
	model_yuri->draw_mesh();

	model_yuri->get_technique(ssao_part->get_technique_transparent());
	material_list rec_need;
	model_yuri->get_texture(&rec_need, 3);
	ssao_part->set_transparent_tex(rec_need.tex_diffuse_resource);
	model_yuri->draw_part(3);
	//model_yuri->get_technique();
	//设定地面世界变换
	trans_world = XMMatrixTranslation(0.0f, -1.2f, 0.0f);
	scal_world = XMMatrixScaling(15.0f, 0.55f, 15.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	//设定总变换
	world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	worldViewProj = world_matrix_rec*view*proj;
	XMStoreFloat4x4(&final_matrix, worldViewProj);
	ssao_part->set_normaldepth_mat(world_matrix, view_matrix, final_matrix);
	floor_need->get_teque(ssao_part->get_technique());
	floor_need->show_mesh();

	//设定ao测试板世界变换
	trans_world = XMMatrixTranslation(0.0f, 0.0f, -1.1f);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	//设定总变换
	world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	worldViewProj = world_matrix_rec*view*proj;
	XMStoreFloat4x4(&final_matrix, worldViewProj);
	ssao_part->set_normaldepth_mat(world_matrix, view_matrix, final_matrix);
	floor_need->get_teque(ssao_part->get_technique());
	floor_need->show_mesh();



	//设定世界变换
	trans_world = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	//设定总变换
	world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	worldViewProj = world_matrix_rec*view*proj;
	XMStoreFloat4x4(&final_matrix, worldViewProj);
	ssao_part->set_normaldepth_mat(world_matrix, view_matrix, final_matrix);
	model_castel->get_technique(ssao_part->get_technique());
	model_castel->draw_mesh();


	ssao_part->compute_ssaomap();
	ssao_part->blur_ssaomap();
	engine_state->set_posttreatment_rendertarget();
	//ssao_part->check_ssaomap();
	//engine_state->set_posttreatment_rendertarget();
}
HRESULT scene_engine_test::update(float delta_time)
{
	HRESULT hr = camera_move();
	if (hr != S_OK)
	{
		MessageBox(0, L"camera system has an error", L"tip", MB_OK);
		return hr;
	}
	XMFLOAT3 eyePos_rec;
	scene_camera->get_view_position(&eyePos_rec);
	auto* shader_test = shader_lib->get_shader_prelight();
	shader_test->set_view_pos(eyePos_rec);

	return S_OK;
}
HRESULT scene_engine_test::release()
{
	floor_need->release();
	tex_floor->Release();
	tex_normal->Release();
	tex_skycube->Release();
	model_yuri->release();
	model_castel->release();
	ball_need->release();
	shadowmap_part->release();
	ssao_part->release();
	return S_OK;
}