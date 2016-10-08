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
#include<ShellScalingAPI.h>
#pragma comment ( lib, "Shcore.lib")
class Pretreatment_gbuffer 
{
	int                      map_width;
	int                      map_height;
	ID3D11Device             *device_pancy;
	ID3D11DeviceContext      *contex_pancy;
	pancy_renderstate        *renderstate_lib;
	shader_control           *shader_list;                //shader表
	geometry_control         *geometry_lib;               //几何体表
	pancy_camera             *camera_use;                //摄像机

	ID3D11ShaderResourceView *depthmap_tex;               //保存深度信息的纹理资源
	ID3D11DepthStencilView   *depthmap_target;            //用作渲染目标的缓冲区资源

	ID3D11RenderTargetView   *normalspec_target;          //存储法线和镜面反射系数的渲染目标
	ID3D11ShaderResourceView *normalspec_tex;             //存储法线和镜面反射系数的纹理资源

	ID3D11RenderTargetView   *gbuffer_diffuse_target;     //存储漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *gbuffer_diffuse_tex;        //存储漫反射光照效果的纹理资源

	ID3D11RenderTargetView   *gbuffer_specular_target;    //存储漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *gbuffer_specular_tex;       //存储漫反射光照效果的纹理资源

	XMFLOAT4                 FrustumFarCorner[4];         //投影视截体的远截面的四个角点
	D3D11_VIEWPORT           render_viewport;             //视口信息

	ID3D11Buffer             *lightbuffer_VB;             //光照纹理顶点缓冲区
	ID3D11Buffer             *lightbuffer_IB;             //光照纹理索引缓冲区
	XMFLOAT4X4               proj_matrix_gbuffer;         //投影变换
public:
	Pretreatment_gbuffer(int width_need,int height_need,ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *renderstate_need, shader_control *shader_need, geometry_control *geometry_need, pancy_camera *camera_need);
	HRESULT create();
	void display();
	void release();
	ID3D11ShaderResourceView *get_gbuffer_normalspec() { return normalspec_tex; };
	ID3D11ShaderResourceView *get_gbuffer_depth() { return depthmap_tex; };
	ID3D11ShaderResourceView *get_gbuffer_difusse() { return gbuffer_diffuse_tex; };
	ID3D11ShaderResourceView *get_gbuffer_specular() { return gbuffer_specular_tex; };
private:
	void set_size();
	HRESULT init_texture();
	HRESULT init_buffer();
	void set_normalspecdepth_target();
	void set_multirender_target();
	void BuildFrustumFarCorners(float fovy, float farZ);
	void render_gbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix);
	ID3DX11EffectTechnique* get_technique();
	ID3DX11EffectTechnique* get_technique_transparent();
	ID3DX11EffectTechnique* get_technique_skin();
	ID3DX11EffectTechnique* get_technique_skin_transparent();
	template<class T>
	void safe_release(T t)
	{
		if (t != NULL)
		{
			t->Release();
			t = 0;
		}
	}
};
Pretreatment_gbuffer::Pretreatment_gbuffer(int width_need, int height_need,ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *renderstate_need, shader_control *shader_need, geometry_control *geometry_need, pancy_camera *camera_need)
{
	map_width = width_need;
	map_height = height_need;
	device_pancy = device_need;
	contex_pancy = contex_need;
	renderstate_lib = renderstate_need;
	shader_list = shader_need;
	geometry_lib = geometry_need;
	camera_use = camera_need;
	depthmap_tex = NULL;
	depthmap_target = NULL;
	normalspec_target = NULL;
	normalspec_tex = NULL;
	gbuffer_diffuse_target = NULL;
	gbuffer_diffuse_tex = NULL;
	gbuffer_specular_target = NULL;
	gbuffer_specular_tex = NULL;
	lightbuffer_VB = NULL;
	lightbuffer_IB = NULL;
}
HRESULT Pretreatment_gbuffer::init_texture()
{
	HRESULT hr;
	//指定用于存储深度记录的shader图片资源的格式
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//建立CPU上的纹理资源
	ID3D11Texture2D* depthMap = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
	{
		MessageBox(0, L"create texture2D error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	//建立GPU上的两种资源：纹理资源以及渲染目标资源
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device_pancy->CreateDepthStencilView(depthMap, &dsvDesc, &depthmap_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create shader resource view error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = device_pancy->CreateShaderResourceView(depthMap, &srvDesc, &depthmap_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create render target view error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	//释放CPU上的纹理资源
	if (depthMap != NULL)
	{
		depthMap->Release();
	}
	//~~~~~~~~~~~~~~~法线&镜面反射纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//指定纹理格式并创建纹理资源
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* normalspec_buf = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &normalspec_buf);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = device_pancy->CreateShaderResourceView(normalspec_buf, 0, &normalspec_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(normalspec_buf, 0, &normalspec_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	//释放纹理资源
	normalspec_buf->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~光照信息存储纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Texture2D *diffuse_buf = 0,*specular_buf = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &diffuse_buf);
	if (FAILED(hr))
	{
		MessageBox(0, L"create diffuse_buf texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &specular_buf);
	if (FAILED(hr))
	{
		MessageBox(0, L"create specular_buf texture error", L"tip", MB_OK);
		return hr;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = device_pancy->CreateShaderResourceView(diffuse_buf, 0, &gbuffer_diffuse_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(diffuse_buf, 0, &gbuffer_diffuse_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}

	hr = device_pancy->CreateShaderResourceView(specular_buf, 0, &gbuffer_specular_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(specular_buf, 0, &gbuffer_specular_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	diffuse_buf->Release();
	specular_buf->Release();
	return S_OK;
}
HRESULT Pretreatment_gbuffer::init_buffer() 
{
	pancy_point v[4];
	HRESULT hr;
	v[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].position = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v[2].position = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v[3].position = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	// 远截面的四个角点
	v[0].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[1].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[2].normal = XMFLOAT3(2.0f, 0.0f, 0.0f);
	v[3].normal = XMFLOAT3(3.0f, 0.0f, 0.0f);

	v[0].tex = XMFLOAT2(0.0f, 1.0f);
	v[1].tex = XMFLOAT2(0.0f, 0.0f);
	v[2].tex = XMFLOAT2(1.0f, 0.0f);
	v[3].tex = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(pancy_point) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	hr = device_pancy->CreateBuffer(&vbd, &vinitData, &lightbuffer_VB);
	if (FAILED(hr))
	{
		MessageBox(0, L"create buffer error", L"tip", MB_OK);
		return hr;
	}
	USHORT indices[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	hr = device_pancy->CreateBuffer(&ibd, &iinitData, &lightbuffer_IB);
	if (FAILED(hr))
	{
		MessageBox(0, L"create buffer error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void Pretreatment_gbuffer::set_size()
{
	//半屏幕渲染
	render_viewport.TopLeftX = 0.0f;
	render_viewport.TopLeftY = 0.0f;
	render_viewport.Width = static_cast<float>(map_width);
	render_viewport.Height = static_cast<float>(map_height);
	render_viewport.MinDepth = 0.0f;
	render_viewport.MaxDepth = 1.0f;
	float fovy = XM_PI*0.25f;
	float farZ = 300.0f;
	XMStoreFloat4x4(&proj_matrix_gbuffer,DirectX::XMMatrixPerspectiveFovLH(fovy, map_width*1.0f / map_height*1.0f, 0.1f, farZ));
	BuildFrustumFarCorners(fovy, farZ);
}
void Pretreatment_gbuffer::BuildFrustumFarCorners(float fovy, float farZ)
{
	float aspect = (float)map_width / (float)map_height;

	float halfHeight = farZ * tanf(0.5f*fovy);
	float halfWidth = aspect * halfHeight;

	FrustumFarCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	FrustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	FrustumFarCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	FrustumFarCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
}
void Pretreatment_gbuffer::set_normalspecdepth_target()
{
	ID3D11RenderTargetView* renderTargets[1] = { normalspec_target };
	contex_pancy->OMSetRenderTargets(1, renderTargets, depthmap_target);
	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	contex_pancy->ClearRenderTargetView(normalspec_target, clearColor);
	contex_pancy->ClearDepthStencilView(depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void Pretreatment_gbuffer::set_multirender_target() 
{
	ID3D11RenderTargetView* renderTargets[2] = { gbuffer_diffuse_target,gbuffer_specular_target };
	contex_pancy->OMSetRenderTargets(2, renderTargets, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	contex_pancy->ClearRenderTargetView(gbuffer_diffuse_target, clearColor);
	contex_pancy->ClearRenderTargetView(gbuffer_specular_target, clearColor);
	//contex_pancy->ClearDepthStencilView(depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
HRESULT Pretreatment_gbuffer::create()
{
	HRESULT hr;
	hr = init_buffer();
	if (FAILED(hr)) 
	{
		return hr;
	}
	hr = init_texture();
	if (FAILED(hr))
	{
		return hr;
	}
	set_size();
	return S_OK;
}
ID3DX11EffectTechnique* Pretreatment_gbuffer::get_technique()
{
	HRESULT hr;
	//选定绘制路径
	ID3DX11EffectTechnique   *teque_need;          //通用渲染路径
	hr = shader_list->get_shader_gbufferdepthnormal()->get_technique(&teque_need, "NormalDepth");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_need;
}
ID3DX11EffectTechnique* Pretreatment_gbuffer::get_technique_transparent()
{
	HRESULT hr;
	ID3DX11EffectTechnique   *teque_transparent;          //通用渲染路径
														  //选定绘制路径
	hr = shader_list->get_shader_gbufferdepthnormal()->get_technique(&teque_transparent, "NormalDepth_withalpha");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_transparent;
}
ID3DX11EffectTechnique* Pretreatment_gbuffer::get_technique_skin()
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec_point[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION"    ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"      ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT"     ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "BONEINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "WEIGHTS"     ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD"    ,0  ,DXGI_FORMAT_R32G32_FLOAT       ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	int num_member = sizeof(rec_point) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	HRESULT hr;
	//选定绘制路径
	ID3DX11EffectTechnique   *teque_need;          //通用渲染路径
	hr = shader_list->get_shader_gbufferdepthnormal()->get_technique(rec_point, num_member, &teque_need, "NormalDepth_skin");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_need;
}
ID3DX11EffectTechnique* Pretreatment_gbuffer::get_technique_skin_transparent()
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec_point[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION"    ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"      ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT"     ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "BONEINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "WEIGHTS"     ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD"    ,0  ,DXGI_FORMAT_R32G32_FLOAT       ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	int num_member = sizeof(rec_point) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	HRESULT hr;
	ID3DX11EffectTechnique   *teque_transparent;          //通用渲染路径
	hr = shader_list->get_shader_gbufferdepthnormal()->get_technique(rec_point, num_member, &teque_transparent, "NormalDepth_skin_withalpha");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_transparent;
}
void Pretreatment_gbuffer::render_gbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix)
{
	//关闭alpha混合
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	contex_pancy->RSSetViewports(1, &render_viewport);
	set_normalspecdepth_target();
	//绘制环境光遮蔽
	scene_geometry_list *list = geometry_lib->get_model_list();
	geometry_member *now_rec = list->get_geometry_head();
	auto *g_shader = shader_list->get_shader_gbufferdepthnormal();
	for (int i = 0; i < list->get_geometry_num(); ++i)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~全部几何体渲染~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//设置世界变换矩阵
		XMFLOAT4X4 final_matrix;
		XMStoreFloat4x4(&final_matrix, XMLoadFloat4x4(&now_rec->get_world_matrix()) * XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
		g_shader->set_trans_world(&now_rec->get_world_matrix(), &view_matrix);
		g_shader->set_trans_all(&final_matrix);
		//set_normaldepth_mat(now_rec->get_world_matrix(), view_matrix, final_matrix);
		if (now_rec->check_if_skin() == true)
		{
			//设置骨骼矩阵
			g_shader->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
			//set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
			now_rec->draw_full_geometry(get_technique_skin());
		}
		else
		{
			now_rec->draw_full_geometry(get_technique());
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~半透明部分渲染~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		for (int i = 0; i < now_rec->get_geometry_data()->get_meshnum(); ++i)
		{
			if (now_rec->get_geometry_data()->check_alpha(i))
			{
				//设置世界变换矩阵
				//shadowmap_deal->set_shaderresource(now_rec._Ptr->get_world_matrix());
				//设置半透明纹理
				//shadowmap_deal->set_transparent_tex(now_rec._Ptr->get_transparent_tex());
				material_list rec_mat;
				now_rec->get_geometry_data()->get_texture(&rec_mat, i);
				g_shader->set_texture(rec_mat.tex_diffuse_resource);
				//set_transparent_tex(rec_mat.tex_diffuse_resource);
				XMFLOAT4X4 final_matrix;
				XMStoreFloat4x4(&final_matrix, XMLoadFloat4x4(&now_rec->get_world_matrix()) * XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
				//set_normaldepth_mat(now_rec->get_world_matrix(), view_matrix, final_matrix);
				g_shader->set_trans_world(&now_rec->get_world_matrix(), &view_matrix);
				g_shader->set_trans_all(&final_matrix);
				if (now_rec->check_if_skin() == true)
				{
					//set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
					g_shader->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
					now_rec->draw_transparent_part(get_technique_skin_transparent(), i);
				}
				else
				{
					now_rec->draw_transparent_part(get_technique_transparent(), i);
				}
			}
		}
		now_rec = now_rec->get_next_member();
	}
	//还原渲染状态
	contex_pancy->RSSetState(0);
}
void Pretreatment_gbuffer::display()
{
	XMFLOAT4X4 view_matrix_gbuffer;         //取景变换
	camera_use->count_view_matrix(&view_matrix_gbuffer);
	render_gbuffer(view_matrix_gbuffer, proj_matrix_gbuffer);
}
void Pretreatment_gbuffer::release()
{
	safe_release(depthmap_tex);
	safe_release(depthmap_target);
	safe_release(normalspec_target);
	safe_release(normalspec_tex);
	safe_release(gbuffer_diffuse_target);
	safe_release(gbuffer_diffuse_tex);
	safe_release(gbuffer_specular_target);
	safe_release(gbuffer_specular_tex);
	safe_release(lightbuffer_VB);
	safe_release(lightbuffer_IB);
}
//继承的d3d注册类
class d3d_pancy_1 :public d3d_pancy_basic
{
	scene_root               *first_scene_test;
	geometry_control         *geometry_list;       //几何体表
	shader_control           *shader_list;         //shader表
	time_count               time_need;            //时钟控制
	pancy_input              *test_input;          //输入输出控制
    pancy_camera             *test_camera;         //虚拟摄像机
	float                    time_game;            //游戏时间
	float                    delta_need;
	HINSTANCE                hInstance;
	render_posttreatment_HDR *posttreat_scene;     //场景后处理
	Pretreatment_gbuffer     *pretreat_scene;      //场景预处理
public:
	d3d_pancy_1(HWND wind_hwnd, UINT wind_width, UINT wind_hight, HINSTANCE hInstance);
	HRESULT init_create();
	void update();
	void display();
	void release();
};
void d3d_pancy_1::release()
{
	geometry_list->release();
	render_state->release();
	shader_list->release();
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
d3d_pancy_1::d3d_pancy_1(HWND hwnd_need, UINT width_need, UINT hight_need, HINSTANCE hInstance_need) :d3d_pancy_basic(hwnd_need,width_need,hight_need)
{
	time_need.reset();
	time_game = 0.0f;
	shader_list = new shader_control();
	posttreat_scene = NULL;
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
	test_camera = new pancy_camera(device_pancy, window_width, window_hight);
	test_input = new pancy_input(wind_hwnd, device_pancy, hInstance);
	geometry_list = new geometry_control(device_pancy, contex_pancy);
	hr = shader_list->shader_init(device_pancy, contex_pancy);
	if (FAILED(hr)) 
	{
		MessageBox(0,L"create shader failed",L"tip",MB_OK);
		return hr;
	}
	hr = geometry_list->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create geometry list failed", L"tip", MB_OK);
		return hr;
	}
	first_scene_test = new scene_engine_test(device_pancy,contex_pancy, render_state,test_input, test_camera, shader_list, geometry_list,wind_width, wind_hight);
	hr = first_scene_test->scene_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create scene failed", L"tip", MB_OK);
		return hr;
	}
	posttreat_scene = new render_posttreatment_HDR(device_pancy, contex_pancy,render_state->get_postrendertarget(), shader_list, wind_width, wind_hight,render_state);
	hr = posttreat_scene->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create posttreat_class failed", L"tip", MB_OK);
		return hr;
	}
	pretreat_scene = new Pretreatment_gbuffer(wind_width, wind_hight,device_pancy, contex_pancy,render_state,shader_list,geometry_list, test_camera);
	hr = pretreat_scene->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create pretreat_class failed", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void d3d_pancy_1::update()
{
	float delta_time = time_need.get_delta();
	time_game += delta_time;
	delta_need += XM_PI*0.5f*delta_time;
	time_need.refresh();
	first_scene_test->update(delta_time);
	return;
}
void d3d_pancy_1::display()
{
	//初始化
	pretreat_scene->display();
	render_state->clear_basicrendertarget();
	render_state->clear_posttreatmentcrendertarget();
	first_scene_test->get_gbuffer(pretreat_scene->get_gbuffer_normalspec(), pretreat_scene->get_gbuffer_depth());
	//render_state->set_posttreatment_rendertarget();
	first_scene_test->display();

	render_state->restore_rendertarget();
	posttreat_scene->display();
	first_scene_test->display_nopost();
	contex_pancy->RSSetState(0);
	contex_pancy->OMSetDepthStencilState(0, 0);
	//交换到屏幕
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

