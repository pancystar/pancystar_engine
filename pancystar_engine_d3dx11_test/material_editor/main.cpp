#include"PancyCamera.h"
#include"PancyInput.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"pancy_d3d11_basic.h"
#include<SpriteFont.h>
int mouse_position_x;
int mouse_position_y;
struct material_per_part 
{
	XMFLOAT4 diffuse_RGB_material;       //漫反射光强度
	XMFLOAT4 specular_RGB_material;      //镜面反射光强度
	XMFLOAT4 specular_reflect_material;  //反射光系数
};
struct material_object 
{
	int material_num;
	material_per_part *material_data;
};
class GUI_Progress_bar
{
	float            now_percent_diffuse_rgb;
	float            now_percent_specular_rgb;
	float            now_percent_specular_volue;
	XMFLOAT2         st_position_diffusrgb;
	XMFLOAT2         st_position_specular_rgb;
	XMFLOAT2         st_position_specular_volue;
	float range_circle;
	float length_bar;
	
	unique_ptr<DirectX::SpriteFont> font_out;
	unique_ptr<DirectX::SpriteBatch> spriteBatch;
	ID3D11Device            *device_pancy;       //d3d设备
	ID3D11DeviceContext     *contex_pancy;       //设备描述表
	pancy_input             *user_input;          //输入输出控制
	shader_control          *shader_list;         //shader表
	XMFLOAT2         corner_uv[4];
	ID3D11Buffer     *button_VB;          //按钮图片顶点
	ID3D11Buffer     *button_IB;             //图片索引
	D3D11_VIEWPORT   draw_position_diffusrgb;     //绘制位置
	D3D11_VIEWPORT   draw_position_specular_rgb;     //绘制位置
	D3D11_VIEWPORT   draw_position_specular_volue;     //绘制位置
	ID3D11ShaderResourceView *tex_ui;
	bool if_diffusergb_clip;
	bool if_speculargb_clip;
	bool if_specularvolue_clip;
public:
	GUI_Progress_bar(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_input *user_input_need, shader_control *shader_need);
	HRESULT create(wchar_t* texture_path);

	void update();
	void display();
	void release();
	void set_percent(material_per_part material_in);
	void get_percent(material_per_part &material_in);
private:
	HRESULT init_buffer();
	HRESULT init_texture(wchar_t* texture_path);
	void show_bar(std::string barname, float percent_need,D3D11_VIEWPORT viewport_need);
	void update_bar(XMFLOAT2 &st_position, float &now_percent, bool &now_check);
	bool check_ifin_range(int x, int y, XMFLOAT2 position);
};
GUI_Progress_bar::GUI_Progress_bar(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_input *user_input_need, shader_control *shader_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	user_input = user_input_need;
	shader_list = shader_need;

	range_circle = 15.0f;
	length_bar = 100.0f;
	st_position_diffusrgb.x = 750.0f;
	st_position_diffusrgb.y = 121.0f;

	st_position_specular_rgb.x = 750.0f;
	st_position_specular_rgb.y = 161.0f;

	st_position_specular_volue.x = 750.0f;
	st_position_specular_volue.y = 201.0f;

	now_percent_diffuse_rgb = 0.0f;
	now_percent_specular_rgb = 0.0f;
	now_percent_specular_volue = 0.0f;

	if_diffusergb_clip = false;
	if_speculargb_clip = false;
	if_specularvolue_clip = false;
}
HRESULT GUI_Progress_bar::init_buffer()
{
	//rec.DrawString(,);
	HRESULT hr;
	float button_size = 0.3f *0.5f;
	float line_size = 2.0f *0.7f;
	pancy_point v[8];
	float width = 0.3725f;
	float height = 0.843f - 0.784f;
	//进度条
	v[0].position = XMFLOAT3(-line_size * width, -line_size * height, 0.0f);
	v[1].position = XMFLOAT3(-line_size * width, +line_size * height, 0.0f);
	v[2].position = XMFLOAT3(+line_size * width, +line_size * height, 0.0f);
	v[3].position = XMFLOAT3(+line_size * width, -line_size * height, 0.0f);

	v[0].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[1].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[2].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[3].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);

	v[0].tex = XMFLOAT2(0.0f, 0.843f);
	v[1].tex = XMFLOAT2(0.0f, 0.784f);
	v[2].tex = XMFLOAT2(0.3725f, 0.784f);
	v[3].tex = XMFLOAT2(0.3725f, 0.843f);
	//按钮
	v[4].position = XMFLOAT3(-button_size, -button_size, 0.0f);
	v[5].position = XMFLOAT3(-button_size, +button_size, 0.0f);
	v[6].position = XMFLOAT3(+button_size, +button_size, 0.0f);
	v[7].position = XMFLOAT3(+button_size, -button_size, 0.0f);

	v[4].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[5].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[6].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[7].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);

	v[4].tex = XMFLOAT2(0.588f, 0.9215f);
	v[5].tex = XMFLOAT2(0.588f, 0.745f);
	v[6].tex = XMFLOAT2(0.753f, 0.745f);
	v[7].tex = XMFLOAT2(0.753f, 0.9215f);
	//缓冲区
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(pancy_point) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	hr = device_pancy->CreateBuffer(&vbd, &vinitData, &button_VB);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"create vertex buffer error in GUI", L"tip", MB_OK);
		return hr;
	}
	USHORT indices[] =
	{
		0, 1, 2,
		0, 2, 3,
		4, 5, 6,
		4, 6, 7
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * 12;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	hr = device_pancy->CreateBuffer(&ibd, &iinitData, &button_IB);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"create vertex buffer error in GUI", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT GUI_Progress_bar::init_texture(wchar_t* texture_path)
{
	draw_position_diffusrgb.Width = 200;
	draw_position_diffusrgb.Height = 200;
	draw_position_diffusrgb.MaxDepth = 1.0f;
	draw_position_diffusrgb.MinDepth = 0.0f;
	draw_position_diffusrgb.TopLeftX = window_width - 300;
	draw_position_specular_rgb = draw_position_diffusrgb;
	draw_position_specular_volue = draw_position_diffusrgb;
	draw_position_diffusrgb.TopLeftY = 20;
	draw_position_specular_rgb.TopLeftY = 60;
	draw_position_specular_volue.TopLeftY = 100;
	HRESULT hr_need = CreateDDSTextureFromFileEx(device_pancy, contex_pancy, texture_path, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, false, NULL, &tex_ui);
	if (FAILED(hr_need))
	{
		MessageBox(NULL, L"create texture error in GUI", L"tip", MB_OK);
		return hr_need;
	}
	return S_OK;
}
HRESULT GUI_Progress_bar::create(wchar_t* texture_path)
{
	HRESULT hr = init_buffer();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = init_texture(texture_path);
	if (FAILED(hr))
	{
		return hr;
	}
	font_out.reset(new SpriteFont(device_pancy, L"myfile.spritefont"));
	spriteBatch.reset(new SpriteBatch(contex_pancy));
	return S_OK;
}
void GUI_Progress_bar::release()
{
	button_VB->Release();
	button_IB->Release();
	tex_ui->Release();
	font_out.reset();
	spriteBatch.reset();
}
void GUI_Progress_bar::update_bar(XMFLOAT2 &st_position,float &now_percent,bool &now_check)
{
	if (user_input->check_mouseDown(0))
	{
		XMFLOAT2 now_position = XMFLOAT2(st_position.x + length_bar * now_percent, st_position.y);
		if (if_diffusergb_clip == false && if_speculargb_clip == false&& if_specularvolue_clip == false &&check_ifin_range(mouse_position_x, mouse_position_y, now_position))
		{
			now_check = true;
		}
	}
	if (!user_input->check_mouseDown(0))
	{
		if_diffusergb_clip = false;
		if_speculargb_clip = false;
		if_specularvolue_clip = false;
	}
	if (now_check == true)
	{
		now_percent = (static_cast<float>(mouse_position_x) - 750.0f) / 100.0f;
		if (now_percent < 0.001f)
		{
			now_percent = 0.0f;
		}
		if (now_percent > 1.0f)
		{
			now_percent = 1.0f;
		}
	}
}
void GUI_Progress_bar::update()
{
	update_bar(st_position_diffusrgb, now_percent_diffuse_rgb,if_diffusergb_clip);
	update_bar(st_position_specular_rgb, now_percent_specular_rgb, if_speculargb_clip);
	update_bar(st_position_specular_volue, now_percent_specular_volue, if_specularvolue_clip);
}
bool GUI_Progress_bar::check_ifin_range(int x, int y, XMFLOAT2 position)
{
	float length_x = abs(static_cast<float>(x) - static_cast<float>(position.x)) * abs(static_cast<float>(x) - static_cast<float>(position.x));
	float length_y = abs(static_cast<float>(y) - static_cast<float>(position.y)) * abs(static_cast<float>(y) - static_cast<float>(position.y));
	float length = sqrt(length_x + length_y);
	if (length > range_circle)
	{
		return false;
	}
	return true;
}
void GUI_Progress_bar::show_bar(std::string barname,float percent_need,D3D11_VIEWPORT viewport_need)
{
	//now_percent_diffuse_rgb = 1.0f;
	std::stringstream rec_string;
	rec_string << percent_need;
	std::string s1 = barname + rec_string.str();
	char rec[1000];
	memcpy(rec, s1.c_str(), s1.length() * sizeof(float));
	//转换文件名为unicode
	size_t len = s1.length() + 1;
	size_t converted = 0;
	wchar_t *percent_now;
	percent_now = (wchar_t*)malloc(len*sizeof(wchar_t));
	mbstowcs_s(&converted, percent_now, len, rec, _TRUNCATE);

	contex_pancy->RSSetViewports(1, &viewport_need);
	auto shader_gui = shader_list->get_shader_GUI();
	shader_gui->set_tex(tex_ui);
	shader_gui->set_mov_xy(XMFLOAT2(percent_need - 0.5f, 0.0f));
	spriteBatch->Begin();
	DirectX::FXMVECTOR origin = font_out->MeasureString(percent_now) / 2.0f;
	DirectX::FXMVECTOR m_fontPos = XMLoadFloat2(&XMFLOAT2(90.0f, 80.0f));
	font_out->DrawString(spriteBatch.get(), percent_now, m_fontPos, Colors::White, 0.0f, origin, 0.4f);

	spriteBatch->End();
	contex_pancy->OMSetDepthStencilState(NULL, 0);
	//渲染屏幕空间像素图
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &button_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(button_IB, DXGI_FORMAT_R16_UINT, 0);

	ID3DX11EffectTechnique* tech;
	//选定绘制路径
	shader_gui->get_technique(&tech, "draw_ui");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(12, 0, 0);
	}
	shader_gui->set_tex(NULL);
}
void GUI_Progress_bar::display()
{
	show_bar("diffuseRGB:",now_percent_diffuse_rgb, draw_position_diffusrgb);
	show_bar("specularRGB:",now_percent_specular_rgb, draw_position_specular_rgb);
	show_bar("specularvalue:",now_percent_specular_volue, draw_position_specular_volue);
}
void GUI_Progress_bar::set_percent(material_per_part material_in) 
{
	now_percent_diffuse_rgb = material_in.diffuse_RGB_material.x;
	now_percent_specular_rgb = material_in.specular_RGB_material.x;
	now_percent_specular_volue = material_in.specular_RGB_material.w / 50.0f;
}
void GUI_Progress_bar::get_percent(material_per_part &material_in) 
{
	material_in.diffuse_RGB_material.x = now_percent_diffuse_rgb;
	material_in.diffuse_RGB_material.y = now_percent_diffuse_rgb;
	material_in.diffuse_RGB_material.z = now_percent_diffuse_rgb;

	material_in.specular_RGB_material.x = now_percent_specular_rgb;
	material_in.specular_RGB_material.y = now_percent_specular_rgb;
	material_in.specular_RGB_material.z = now_percent_specular_rgb;

	material_in.specular_RGB_material.w = now_percent_specular_volue * 50.0f;
}
//继承的d3d注册类
class d3d_pancy_1 :public d3d_pancy_basic
{
	float                    delta_need;
	HINSTANCE                hInstance;
	GUI_Progress_bar         *gui_test;
	geometry_control         *geometry_lib;       //几何体表
	shader_control           *shader_lib;         //shader表
	pancy_input              *user_input;          //输入输出控制
	pancy_camera             *scene_camera;         //虚拟摄像机
	XMFLOAT4X4               proj_matrix;
	XMFLOAT4X4               view_matrix;

	ID3D11Texture2D          *clipTex0;
	ID3D11Texture2D          *CPU_read_buffer;
	ID3D11ShaderResourceView *clip_SRV;
	ID3D11RenderTargetView *clip_RTV;
	ID3D11DepthStencilView   *clip_DSV;
	material_object          material_castel;
	int check_part;
public:
	d3d_pancy_1(HWND wind_hwnd, UINT wind_width, UINT wind_hight, HINSTANCE hInstance);
	HRESULT init_create();
	void update();
	void display();
	void release();
private:
	void show_model();
	int find_clip_model(int pos_x, int pos_y);
	HRESULT camera_move();
	HRESULT create_texture();
	HRESULT CreateCPUaccessBuf();
	void CreateAndCopyToDebugBuf();
};
HRESULT d3d_pancy_1::camera_move()
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
	scene_camera->count_view_matrix(&view_matrix);
	//XMStoreFloat4x4(&view_matrix, view);
	return S_OK;
}
void d3d_pancy_1::release()
{
	clip_DSV->Release();
	CPU_read_buffer->Release();
	clipTex0->Release();
	clip_SRV->Release();
	clip_RTV->Release();
	gui_test->release();
	geometry_lib->release();       //几何体表
	shader_lib->release();         //shader表
	m_renderTargetView->Release();
	swapchain->Release();
	depthStencilView->Release();
	contex_pancy->Release();
	//device_pancy->Release();
#if defined(DEBUG) || defined(_DEBUG)
	ID3D11Debug *d3dDebug;
	HRESULT hr = device_pancy->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	if (SUCCEEDED(hr))
	{
		hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
	if (d3dDebug != nullptr)            d3dDebug->Release();
#endif
	if (device_pancy != nullptr)            device_pancy->Release();

}
d3d_pancy_1::d3d_pancy_1(HWND hwnd_need, UINT width_need, UINT hight_need, HINSTANCE hInstance_need) :d3d_pancy_basic(hwnd_need, width_need, hight_need)
{
	hInstance = hInstance_need;
	shader_lib = new shader_control();
	XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, (window_width-300)*1.0f / window_hight*1.0f, 0.1f, 300.f);
	XMStoreFloat4x4(&proj_matrix, proj);
	//游戏时间
	delta_need = 0.0f;
	check_part = -1;
}
HRESULT d3d_pancy_1::init_create()
{
	check_init = init(wind_hwnd, wind_width, wind_hight);
	if (check_init == false)
	{
		MessageBox(0, L"create d3dx11 failed", L"tip", MB_OK);
		return E_FAIL;
	}

	scene_camera = new pancy_camera(device_pancy, window_width, window_hight);
	user_input = new pancy_input(wind_hwnd, device_pancy, hInstance);
	geometry_lib = new geometry_control(device_pancy, contex_pancy);
	HRESULT hr = shader_lib->shader_init(device_pancy, contex_pancy);
	if (FAILED(hr))
	{
		MessageBox(0, L"create shader failed", L"tip", MB_OK);
		return hr;
	}
	hr = geometry_lib->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create geometry list failed", L"tip", MB_OK);
		return hr;
	}

	gui_test = new GUI_Progress_bar(device_pancy, contex_pancy, user_input, shader_lib);
	hr = gui_test->create(L"dxutcontrols.dds");
	if (FAILED(hr))
	{
		return hr;
	}
	hr = create_texture();
	if (FAILED(hr))
	{
		MessageBox(0, L"create geometry list failed", L"tip", MB_OK);
		return hr;
	}
	auto* model_castel_pack = geometry_lib->get_model_list()->get_geometry_byname("model_castel");
	auto* model_castel = model_castel_pack->get_geometry_data();
	material_castel.material_num = model_castel->get_meshnum();
	material_castel.material_data = new material_per_part[material_castel.material_num];
	for (int i = 0; i < material_castel.material_num; ++i) 
	{
		XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
		XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 6.0f);
		material_castel.material_data[i].diffuse_RGB_material = rec_diffuse2;
		material_castel.material_data[i].specular_RGB_material = rec_specular2;
	}
	return S_OK;
}
void d3d_pancy_1::update()
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新场景摄像机~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HRESULT hr = camera_move();
	XMFLOAT3 eyePos_rec;
	scene_camera->get_view_position(&eyePos_rec);
	auto* shader_test = shader_lib->get_shader_prelight();
	shader_test->set_view_pos(eyePos_rec);
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	//更新castel世界变换
	auto* model_list = geometry_lib->get_model_list();
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	model_list->update_geometry_byname("model_castel", world_matrix, 0.0f);
	//light
	XMFLOAT4 rec_ambient1(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular1(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT3 rec_decay(0.0f, 0.1f, 0.0f);
	pancy_light_basic light_data;
	light_data.ambient = rec_ambient1;
	light_data.diffuse = rec_diffuse1;
	light_data.specular = rec_specular1;
	light_data.decay = rec_decay;
	light_data.range = 150.0f;
	light_data.position = XMFLOAT3(0.0f, 5.0f, 0.0f);
	light_data.light_type.x = point_light;
	light_data.light_type.y = shadow_none;
	shader_test->set_light(light_data, 0);

	gui_test->update();
	auto* model_castel_pack = geometry_lib->get_model_list()->get_geometry_byname("model_castel");
	auto* model_castel = model_castel_pack->get_geometry_data();
	if (check_part >= 0 && check_part < model_castel->get_meshnum()) 
	{
		gui_test->get_percent(material_castel.material_data[check_part]);
	}
	return;
}
void d3d_pancy_1::display()
{
	//初始化
	XMVECTORF32 color = { 0.0f,0.0f,0.0f,1.0f };
	contex_pancy->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<float*>(&color));
	contex_pancy->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	show_model();
	if (user_input->check_mouseDown(0))
	{
		if (mouse_position_x < viewPort.Width && mouse_position_y < viewPort.Height)
		{
			check_part = find_clip_model(mouse_position_x, mouse_position_y);
			gui_test->set_percent(material_castel.material_data[check_part]);
		}
	}
	gui_test->display();
	contex_pancy->RSSetViewports(1, &viewPort);
	//交换到屏幕
	HRESULT hr = swapchain->Present(0, 0);
	int a = 0;
}
void d3d_pancy_1::show_model()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_model_list()->get_geometry_byname("model_castel");
	//几何体的固有属性
	auto* model_castel = model_castel_pack->get_geometry_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need, *teque_normal;
	shader_test->get_technique(&teque_need, "draw_withtexture");
	shader_test->get_technique(&teque_normal, "draw_withtexturenormal");
	//材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	//XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	//XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	//test_Mt.diffuse = rec_diffuse2;
	//test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(rsDesc));
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.DepthClipEnable = true;
	rsDesc.FrontCounterClockwise = false;
	ID3D11RasterizerState *rsState(NULL);
	for (int i = 0; i < model_castel->get_meshnum(); ++i)
	{
		test_Mt.diffuse = material_castel.material_data[i].diffuse_RGB_material;
		test_Mt.specular = material_castel.material_data[i].specular_RGB_material;
		shader_test->set_material(test_Mt);
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
		model_castel->draw_part(i);
	}
}
int d3d_pancy_1::find_clip_model(int pos_x,int pos_y)
{
	//alpha混合设定
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	ID3D11RenderTargetView* renderTargets[1] = { clip_RTV };
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	contex_pancy->OMSetRenderTargets(1, renderTargets, clip_DSV);
	contex_pancy->ClearRenderTargetView(clip_RTV, clearColor);
	contex_pancy->ClearDepthStencilView(clip_DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	contex_pancy->RSSetViewports(1, &viewPort);
	auto *shader_need = shader_lib->get_shader_findclip();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_model_list()->get_geometry_byname("model_castel");
	//几何体的固有属性
	auto* model_castel = model_castel_pack->get_geometry_data();
	//设定世界变换
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_need->set_trans_all(&world_viewrec);
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need, *teque_normal;
	shader_need->get_technique(&teque_need, "draw_clipmap");
	model_castel->get_technique(teque_need);
	model_castel->draw_mesh();
	ID3D11RenderTargetView* renderTargets2[1] = { NULL };
	contex_pancy->OMSetRenderTargets(1, renderTargets2, 0);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
	restore_rendertarget();

	CreateAndCopyToDebugBuf();
	D3D11_TEXTURE2D_DESC texElementDesc;
	CPU_read_buffer->GetDesc(&texElementDesc);
	unsigned int rec_answer;
	for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
	{
		D3D11_MAPPED_SUBRESOURCE mappedTex2D;
		HRESULT hr;
		hr = contex_pancy->Map(CPU_read_buffer, mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
		unsigned int* rec = static_cast<unsigned int*>(mappedTex2D.pData) + (mappedTex2D.RowPitch/4) * pos_y;
		rec_answer = rec[pos_x];
		//contex_pancy->UpdateSubresource(texArray, D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels), 0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);
		contex_pancy->Unmap(CPU_read_buffer, mipLevel);
	}
	return rec_answer;
}
HRESULT d3d_pancy_1::create_texture() 
{
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.Width = viewPort.Width;
	dsDesc.Height = viewPort.Height;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	ID3D11Texture2D* depthStencilBuffer;
	device_pancy->CreateTexture2D(&dsDesc, 0, &depthStencilBuffer);
	device_pancy->CreateDepthStencilView(depthStencilBuffer, 0, &clip_DSV);
	depthStencilBuffer->Release();
	//clip纹理
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.Width = viewPort.Width;
	texDesc.Height = viewPort.Height;
	texDesc.Format = DXGI_FORMAT_R32_UINT;
	HRESULT hr = device_pancy->CreateTexture2D(&texDesc, 0, &clipTex0);
	if (FAILED(hr))
	{
		MessageBox(0, L"create clip map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(clipTex0, 0, &clip_SRV);
	if (FAILED(hr))
	{
		MessageBox(0, L"create clip map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(clipTex0, 0, &clip_RTV);
	if (FAILED(hr))
	{
		MessageBox(0, L"create clip map texture1 error", L"tip", MB_OK);
		return hr;
	}
	CreateCPUaccessBuf();
	return S_OK;
}
void d3d_pancy_1::CreateAndCopyToDebugBuf()
{
	contex_pancy->CopyResource(CPU_read_buffer, clipTex0);
}
HRESULT d3d_pancy_1::CreateCPUaccessBuf()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texDesc.MiscFlags = 0;
	texDesc.Width = viewPort.Width;
	texDesc.Height = viewPort.Height;
	texDesc.Format = DXGI_FORMAT_R32_UINT;
	HRESULT hr = device_pancy->CreateTexture2D(&texDesc, NULL, &CPU_read_buffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"create CPU access read buffer error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
//endl
class engine_windows_main
{
	HWND         hwnd;                                                  //指向windows类的句柄。
	MSG          msg;                                                   //存储消息的结构。
	WNDCLASS     wndclass;
	int          viewport_width;
	int          viewport_height;
	HINSTANCE    hInstance;
	HINSTANCE    hPrevInstance;
	PSTR         szCmdLine;
	int          iCmdShow;
public:
	engine_windows_main(HINSTANCE hInstance_need, HINSTANCE hPrevInstance_need, PSTR szCmdLine_need, int iCmdShow_need, int width, int height);
	HRESULT game_create();
	HRESULT game_loop();
	WPARAM game_end();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};
LRESULT CALLBACK engine_windows_main::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:                // 键盘按下消息
		if (wParam == VK_ESCAPE)    // ESC键
			DestroyWindow(hwnd);    // 销毁窗口, 并发送一条WM_DESTROY消息
		break;
	case WM_MOUSEMOVE:
		mouse_position_x = LOWORD(lParam);
		mouse_position_y = HIWORD(lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);

		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
engine_windows_main::engine_windows_main(HINSTANCE hInstance_need, HINSTANCE hPrevInstance_need, PSTR szCmdLine_need, int iCmdShow_need, int width, int height)
{
	hwnd = NULL;
	hInstance = hInstance_need;
	hPrevInstance = hPrevInstance_need;
	szCmdLine = szCmdLine_need;
	iCmdShow = iCmdShow_need;
	viewport_width = width;
	viewport_height = height;
}
HRESULT engine_windows_main::game_create()
{
	wndclass.style = CS_HREDRAW | CS_VREDRAW;                   //窗口类的类型（此处包括竖直与水平平移或者大小改变时时的刷新）。msdn原文介绍：Redraws the entire window if a movement or size adjustment changes the width of the client area.
	wndclass.lpfnWndProc = WndProc;                                   //确定窗口的回调函数，当窗口获得windows的回调消息时用于处理消息的函数。
	wndclass.cbClsExtra = 0;                                         //为窗口类末尾分配额外的字节。
	wndclass.cbWndExtra = 0;                                         //为窗口类的实例末尾额外分配的字节。
	wndclass.hInstance = hInstance;                                 //创建该窗口类的窗口的句柄。
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);          //窗口类的图标句柄。
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);              //窗口类的光标句柄。
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);     //窗口类的背景画刷句柄。
	wndclass.lpszMenuName = NULL;                                      //窗口类的菜单。
	wndclass.lpszClassName = TEXT("pancystar_engine");                                 //窗口类的名称。

	if (!RegisterClass(&wndclass))                                      //注册窗口类。
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			TEXT("pancystar_engine"), MB_ICONERROR);
		return E_FAIL;
	}
	RECT R = { 0, 0, window_width, window_hight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	hwnd = CreateWindow(TEXT("pancystar_engine"), // window class name创建窗口所用的窗口类的名字。
		TEXT("pancystar_engine"), // window caption所要创建的窗口的标题。
		WS_OVERLAPPEDWINDOW,        // window style所要创建的窗口的类型（这里使用的是一个拥有标准窗口形状的类型，包括了标题，系统菜单，最大化最小化等）。
		CW_USEDEFAULT,              // initial x position窗口的初始位置水平坐标。
		CW_USEDEFAULT,              // initial y position窗口的初始位置垂直坐标。
		width,               // initial x size窗口的水平位置大小。
		height,               // initial y size窗口的垂直位置大小。
		NULL,                       // parent window handle其父窗口的句柄。
		NULL,                       // window menu handle其菜单的句柄。
		hInstance,                  // program instance handle窗口程序的实例句柄。
		NULL);                     // creation parameters创建窗口的指针
	if (hwnd == NULL)
	{
		return E_FAIL;
	}
	ShowWindow(hwnd, SW_SHOW);   // 将窗口显示到桌面上。
	UpdateWindow(hwnd);           // 刷新一遍窗口（直接刷新，不向windows消息循环队列做请示）。
	return S_OK;
}
HRESULT engine_windows_main::game_loop()
{
	//游戏循环
	ZeroMemory(&msg, sizeof(msg));
	d3d_pancy_1 *d3d11_test = new d3d_pancy_1(hwnd, viewport_width, viewport_height, hInstance);
	if (d3d11_test->init_create() == S_OK)
	{
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);//消息转换
				DispatchMessage(&msg);//消息传递给窗口过程函数
				d3d11_test->update();
				d3d11_test->display();
			}
			else
			{
				d3d11_test->update();
				d3d11_test->display();
			}
		}
		d3d11_test->release();
	}

	return S_OK;
}
WPARAM engine_windows_main::game_end()
{
	return msg.wParam;
}

//windows函数的入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	engine_windows_main *engine_main = new engine_windows_main(hInstance, hPrevInstance, szCmdLine, iCmdShow, window_width, window_hight);
	engine_main->game_create();
	engine_main->game_loop();
	return engine_main->game_end();
}

