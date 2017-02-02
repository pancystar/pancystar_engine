/*
	data 2016.7.7 ：pancystar engine设计程序，用于渲染引擎前向光渲染管线。
	data 2016.7.10：pancystar engine设计程序，用于渲染引擎的场景渲染管理架构。
	data 2016.7.12：pancystar engine设计程序，前向光渲染管线完成。
	data 2016.7.15：pancystar engine设计程序，模型载入系统完成。
	data 2016.7.15: pancystar engine设计程序，mesh合并系统完成。
	data 2016.7.19: pancystar engine设计程序，shadow map系统完成。
	data 2016.7.21: pancystar engine设计程序，shadow map系统简易修缮，添加状态类及normalmap。
	data 2016.7.26: pancystar engine设计程序，ssao系统完成，并在以往的基础上做了修缮。
	data 2016.7.27: pancystar engine设计程序，cubemapping系统完成。
	data 2016.7.28: pancystar engine设计程序，修改了ssao的一些错误。
	data 2016.7.30: pancystar engine设计程序，优化了模型载入系统，合并同一纹理的网格以大幅度减少draw call。
	data 2016.8.1 : pancystar engine设计程序，优化了ssao，为pass1增添了4*MSAA抗锯齿。
	data 2016.8.2 : pancystar engine设计程序，完成了HDR的pass1，为后处理图形得到了平均亮度。
	data 2016.8.4 : pancystar engine设计程序，完成了HDR的pass2，为后处理图形得到了高亮采样。
	data 2016.8.8 : pancystar engine设计程序，完成了全部HDR。
	data 2016.8.10 : pancystar engine设计程序，修缮了HDR CPUmap操作，并将tonemapping函数进行修缮。
	data 2016.8.15: pancystar engine设计程序，添加了毛发渲染。
	data 2016.8.20: pancystar engine设计程序，修缮了毛发的ao及阴影。
	created by pancy_star
*/
#include"geometry.h"
#include"pancy_d3d11_basic.h"
#include"pancy_time_basic.h"
#include"PancyCamera.h"
#include"PancyInput.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"pancy_scene_design.h"
#include"pancy_ssao.h"
#include"pancy_posttreatment.h"
#include"pancy_pretreatment.h"
#include<ShellScalingAPI.h>
#pragma comment ( lib, "Shcore.lib")

//继承的d3d注册类
class d3d_pancy_1 :public d3d_pancy_basic
{
	pancy_physx              *physics_pancy;
	scene_root               *first_scene_test;
	geometry_control         *geometry_list;       //几何体表
	shader_control           *shader_list;         //shader表
	light_control            *light_list;          //光源表
	time_count               time_need;            //时钟控制
	pancy_input              *test_input;          //输入输出控制
	pancy_camera             *test_camera;         //虚拟摄像机
	float                    time_game;            //游戏时间
	float                    delta_need;
	HINSTANCE                hInstance;
	render_posttreatment_HDR *posttreat_scene;     //场景后处理
	render_posttreatment_SSR *posttreat_reflect;   //反射后处理
	Pretreatment_gbuffer     *pretreat_scene;      //场景预处理
	ssao_pancy               *ssao_part;           //ssao
	float                    perspective_near_plane;
	float                    perspective_far_plane;
	float                    perspective_angle;
public:
	d3d_pancy_1(HWND wind_hwnd, UINT wind_width, UINT wind_hight, HINSTANCE hInstance);
	HRESULT init_create();
	void update();
	void display();
	void release();
private:
	void render_static_enviroment_map(XMFLOAT3 camera_location);
	void display_shadowao(bool if_shadow, bool if_ao);
};
void d3d_pancy_1::release()
{
	ssao_part->release();
	posttreat_reflect->release();
	geometry_list->release();
	render_state->release();
	shader_list->release();
	light_list->release();
	first_scene_test->release();
	swapchain->Release();
	contex_pancy->Release();
	posttreat_scene->release();
	pretreat_scene->release();
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
	perspective_near_plane = 0.1f;
	perspective_far_plane = 300.0f;
	perspective_angle = XM_PI * 0.25f;
	time_need.reset();
	time_game = 0.0f;
	shader_list = new shader_control();
	posttreat_scene = NULL;
	posttreat_reflect = NULL;
	pretreat_scene = NULL;
	hInstance = hInstance_need;
	render_state = NULL;
	//游戏时间
	delta_need = 0.0f;
}
HRESULT d3d_pancy_1::init_create()
{
	HRESULT hr;
	hr = init(wind_hwnd, wind_width, wind_hight);

	if (FAILED(hr))
	{
		MessageBox(0, L"create d3dx11 failed", L"tip", MB_OK);
		return E_FAIL;
	}
	physics_pancy = new pancy_physx(device_pancy, contex_pancy);
	hr = physics_pancy->create();
	if (FAILED(hr))
	{
		return hr;
	}
	test_camera = new pancy_camera(device_pancy, window_width, window_hight);
	test_input = new pancy_input(wind_hwnd, device_pancy, hInstance);
	geometry_list = new geometry_control(device_pancy, contex_pancy,physics_pancy);
	light_list = new light_control(device_pancy, contex_pancy, test_camera,20,perspective_near_plane, perspective_far_plane, perspective_angle,window_width, window_hight);
	hr = shader_list->shader_init(device_pancy, contex_pancy);
	if (FAILED(hr))
	{
		MessageBox(0, L"create shader failed", L"tip", MB_OK);
		return hr;
	}
	hr = geometry_list->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create geometry list failed", L"tip", MB_OK);
		return hr;
	}
	hr = light_list->create(shader_list, geometry_list, render_state);
	if (FAILED(hr))
	{
		return hr;
	}
	first_scene_test = new scene_engine_physicx(device_pancy, contex_pancy, physics_pancy, render_state, test_input, test_camera, shader_list, geometry_list, light_list, wind_width, wind_hight, perspective_near_plane, perspective_far_plane, perspective_angle);
	hr = first_scene_test->scene_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create scene failed", L"tip", MB_OK);
		return hr;
	}
	ssao_part = new ssao_pancy(render_state, device_pancy, contex_pancy, shader_list, geometry_list, window_width, window_hight, perspective_near_plane, perspective_far_plane, perspective_angle);
	hr = ssao_part->basic_create();
	if (FAILED(hr))
	{
		return hr;
	}
	posttreat_scene = new render_posttreatment_HDR(device_pancy, contex_pancy, render_state->get_postrendertarget(), shader_list, wind_width, wind_hight, render_state);
	hr = posttreat_scene->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create posttreat_class failed", L"tip", MB_OK);
		return hr;
	}
	posttreat_reflect = new render_posttreatment_SSR(test_camera, render_state, device_pancy, contex_pancy, shader_list, geometry_list, wind_width, wind_hight, perspective_near_plane, perspective_far_plane, perspective_angle);
	hr = posttreat_reflect->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create posttreat_reflect failed", L"tip", MB_OK);
		return hr;
	}
	pretreat_scene = new Pretreatment_gbuffer(wind_width, wind_hight, device_pancy, contex_pancy, render_state, shader_list, geometry_list, test_camera, light_list, perspective_near_plane, perspective_far_plane, perspective_angle);
	hr = pretreat_scene->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create pretreat_class failed", L"tip", MB_OK);
		return hr;
	}
	render_static_enviroment_map(XMFLOAT3(0.0f, 5.0f, 0.0f));
	return S_OK;
}
void d3d_pancy_1::render_static_enviroment_map(XMFLOAT3 camera_location)
{
	XMFLOAT3 look_vec, up_vec;
	XMFLOAT4X4 proj_matrix;
	XMFLOAT3 up[6] =
	{
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f,-1.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f)
	};
	XMFLOAT3 look[6] =
	{
		XMFLOAT3(1.0f, 0.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f,-1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 1.0f),
		XMFLOAT3(0.0f, 0.0f,-1.0f)
	};
	posttreat_reflect->set_static_cube_centerposition(camera_location);
	for (int i = 0; i < 6; ++i)
	{
		posttreat_reflect->set_static_cube_rendertarget(i, proj_matrix);
		look_vec = look[i];
		up_vec = up[i];
		test_camera->set_camera(look_vec, up_vec, camera_location);
		first_scene_test->set_proj_matrix(proj_matrix);
		XMFLOAT4X4 rec_viewmat;
		test_camera->count_view_matrix(&rec_viewmat);
		posttreat_reflect->set_static_cube_view_matrix(i, rec_viewmat);
		update();
		
		pretreat_scene->display(true);
		first_scene_test->get_gbuffer(pretreat_scene->get_gbuffer_normalspec(), pretreat_scene->get_gbuffer_depth());
		first_scene_test->get_lbuffer(pretreat_scene->get_gbuffer_difusse(), pretreat_scene->get_gbuffer_specular());
		display_shadowao(true, true);
		posttreat_reflect->set_static_cube_rendertarget(i, proj_matrix);
		first_scene_test->display_enviroment();
		posttreat_reflect->draw_static_cube(i);
	}
	pretreat_scene->restore_proj_matrix();
	first_scene_test->reset_proj_matrix();
	test_camera->reset_camera();
}
void d3d_pancy_1::display_shadowao(bool if_shadow, bool if_ao) 
{
	render_state->clear_posttreatmentcrendertarget();
	if (if_ao)
	{
		ssao_part->get_normaldepthmap(pretreat_scene->get_gbuffer_normalspec(), pretreat_scene->get_gbuffer_depth());
		ssao_part->compute_ssaomap();
		ssao_part->blur_ssaomap();
		render_state->set_posttreatment_rendertarget();
		auto shader_deffered = shader_list->get_shader_light_deffered_draw();
		auto shader_pre = shader_list->get_shader_prelight();
		XMFLOAT4X4 view_mat_rec;
		test_camera->count_view_matrix(&view_mat_rec);
		XMMATRIX view_rec = XMLoadFloat4x4(&view_mat_rec);
		XMMATRIX proj_rec = DirectX::XMMatrixPerspectiveFovLH(perspective_angle, wind_width*1.0f / wind_hight*1.0f, perspective_near_plane, perspective_far_plane);
		XMMATRIX T_need(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f
			);
		XMFLOAT4X4 ssao_matrix;
		XMStoreFloat4x4(&ssao_matrix, view_rec*proj_rec*T_need);
		shader_deffered->set_trans_ssao(&ssao_matrix);
		shader_pre->set_trans_ssao(&ssao_matrix);

		shader_deffered->set_ssaotex(ssao_part->get_aomap());
		shader_pre->set_ssaotex(ssao_part->get_aomap());
	}
	if (if_shadow)
	{
		//contex_pancy->RSSetState(render_state->get_CULL_front_rs());
		light_list->draw_shadow();
		auto shader_pre = shader_list->get_shader_prelight();
		shader_pre->set_shadowtex(light_list->get_shadow_map_resource());
		contex_pancy->RSSetState(NULL);
	}
}
void d3d_pancy_1::update()
{
	float delta_time = time_need.get_delta();
	time_game += delta_time;
	delta_need += XM_PI*0.5f*delta_time;
	time_need.refresh();
	if (delta_time > 0.000000001 && delta_time < 1.0f / 30.0f)
	{
		physics_pancy->update(delta_time);
	}
	first_scene_test->update(delta_time);
	return;
}
void d3d_pancy_1::display()
{
	//render_static_enviroment_map(XMFLOAT3(0.0f, 5.0f, 0.0f));

	//初始化gbuffer与lbuffer
	pretreat_scene->display(true);
	render_state->clear_basicrendertarget();
	render_state->clear_posttreatmentcrendertarget();
	//场景设置
	first_scene_test->get_gbuffer(pretreat_scene->get_gbuffer_normalspec(), pretreat_scene->get_gbuffer_depth());
	first_scene_test->get_lbuffer(pretreat_scene->get_gbuffer_difusse(), pretreat_scene->get_gbuffer_specular());
	render_state->set_posttreatment_rendertarget();
	first_scene_test->get_environment_map(posttreat_reflect->get_cubemap());
	//ao与阴影绘制
	display_shadowao(true, true);
	render_state->clear_reflectrendertarget();
	render_state->set_posttreatment_reflect_rendertarget();
	//场景绘制
	first_scene_test->display();
	render_state->set_posttreatment_rendertarget();
	//反射处理
	posttreat_reflect->set_normaldepthcolormap(pretreat_scene->get_gbuffer_normalspec(), pretreat_scene->get_gbuffer_depth());
	posttreat_reflect->draw_reflect(render_state->get_postrendertarget(), render_state->get_reflectrendertarget());
	render_state->restore_rendertarget();
	//HDR处理
	posttreat_scene->display();
	//绘制非后处理目标
	first_scene_test->display_nopost();
	//还原渲染状态
	contex_pancy->RSSetState(0);
	contex_pancy->OMSetDepthStencilState(0, 0);
	//交换到屏幕
	contex_pancy->ClearState();
	HRESULT hr = swapchain->Present(0, 0);
	int a = 0;
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
	//unsigned int x, y;
	//GetDpiForMonitor(NULL, MDT_EFFECTIVE_DPI,&x,&y);

	engine_windows_main *engine_main = new engine_windows_main(hInstance, hPrevInstance, szCmdLine, iCmdShow, window_width, window_hight);
	engine_main->game_create();
	engine_main->game_loop();
	return engine_main->game_end();

}

