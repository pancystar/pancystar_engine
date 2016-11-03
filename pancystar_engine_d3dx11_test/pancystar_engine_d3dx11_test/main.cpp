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
class render_posttreatment_SSR 
{
	int                      map_width;
	int                      map_height;
	ID3D11Device             *device_pancy;
	ID3D11DeviceContext      *contex_pancy;
	pancy_renderstate        *renderstate_lib;
	geometry_control         *geometry_lib;
	pancy_camera             *camera_use;                 //摄像机
	ID3D11Buffer             *reflectMap_VB;              //ao图片顶点缓冲区
	ID3D11Buffer             *reflectMap_IB;              //ao图片索引缓冲区

	ID3D11ShaderResourceView *normaldepth_tex;            //存储法线和深度的纹理资源
	ID3D11ShaderResourceView *depth_tex;                  //存储法线和深度的纹理资源
	ID3D11ShaderResourceView *color_tex;                  //存储渲染结果的纹理资源

	ID3D11RenderTargetView   *reflect_target;             //存储动态屏幕空间反射的渲染目标
	ID3D11ShaderResourceView *reflect_tex;                //存储动态屏幕空间反射的纹理资源

	ID3D11RenderTargetView   *mask_target;                //存储动态屏幕空间反射掩码的渲染目标
	ID3D11ShaderResourceView *mask_tex;                   //存储动态屏幕空间反射掩码的纹理资源

	ID3D11ShaderResourceView *reflect_cube_SRV;           //存储静态cubemapping的纹理资源
	ID3D11RenderTargetView   *reflect_cube_RTV[6];        //存储静态cubemapping的渲染目标
	ID3D11ShaderResourceView *reflect_cubeinput_SRV[6];   //存储静态cubemapping的输入纹理
	ID3D11RenderTargetView   *reflect_cubeinput_RTV[6];   //存储静态cubemapping的输入目标

	ID3D11DepthStencilView   *reflect_DSV[6];             //深度缓冲区目标
	ID3D11ShaderResourceView *reflect_depthcube_SRV;      //深度立方贴图

	XMFLOAT4                 FrustumFarCorner[4];         //投影视截体的远截面的四个角点
	D3D11_VIEWPORT           render_viewport;             //视口信息

	XMFLOAT4X4               static_cube_view_matrix[6];  //立方贴图的六个方向的取景变换
	shader_control           *shader_list;                //shader表
public:
	render_posttreatment_SSR(pancy_camera *camera_need,pancy_renderstate *renderstate_need, ID3D11Device* device, ID3D11DeviceContext* dc, shader_control *shader_need, geometry_control *geometry_need,int width, int height, float fovy, float farZ);
	void set_normaldepthcolormap(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need);
	HRESULT create();
	void draw_reflect(ID3D11RenderTargetView *rendertarget_input);
	void draw_static_cube(int count_cube);
	void set_static_cube_rendertarget(int count_cube,XMFLOAT4X4 &mat_project);
	void set_static_cube_view_matrix(int count_cube,XMFLOAT4X4 mat_input);
	ID3D11ShaderResourceView *get_cubemap() { return reflect_cube_SRV; };
	void release();
private:
	void set_size(int width, int height, float fovy, float farZ);
	void build_fullscreen_picturebuff();
	void BuildFrustumFarCorners(float fovy, float farZ);
	HRESULT build_texture();
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
render_posttreatment_SSR::render_posttreatment_SSR(pancy_camera *camera_need,pancy_renderstate *renderstate_need, ID3D11Device* device, ID3D11DeviceContext* dc, shader_control *shader_need, geometry_control *geometry_need, int width, int height, float fovy, float farZ)
{
	camera_use = camera_need;
	device_pancy = device;
	contex_pancy = dc;
	set_size(width, height, fovy, farZ);
	shader_list = shader_need;
	renderstate_lib = renderstate_need;
	geometry_lib = geometry_need;
	reflectMap_VB = NULL;
	reflectMap_IB = NULL;
	color_tex = NULL;
	reflect_target = NULL;
	reflect_tex = NULL;
	reflect_cube_SRV = NULL;
	reflect_depthcube_SRV = NULL;
	mask_target = NULL;
	mask_tex = NULL;
	for (int i = 0; i < 6; ++i)
	{
		reflect_cubeinput_RTV[i] = NULL;
		reflect_cubeinput_SRV[i] = NULL;
		reflect_cube_RTV[i] = NULL;
		reflect_DSV[i] = NULL;
	}
	
}
HRESULT render_posttreatment_SSR::create() 
{
	build_fullscreen_picturebuff();
	HRESULT hr;
	//创建纹理
	hr = build_texture();
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT render_posttreatment_SSR::build_texture()
{
	HRESULT hr;
	//创建输入资源
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* inputcolortex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &inputcolortex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_averagetex texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(inputcolortex, 0, &color_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_prepass texture error", L"tip", MB_OK);
		return hr;
	}
	inputcolortex->Release();
	//作为shader resource view的普通纹理(无多重采样)
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
	ID3D11Texture2D* reflecttex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &reflecttex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(reflecttex, 0, &reflect_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(reflecttex, 0, &reflect_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	reflecttex->Release();
	
	ID3D11Texture2D* reflectmasktex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &reflectmasktex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(reflectmasktex, 0, &mask_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(reflectmasktex, 0, &mask_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	reflectmasktex->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~静态cubemap输入资源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.Width = 1024;
	texDesc.Height = 1024;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* tex_cubeinput[6];
	for (int i = 0; i < 6; ++i) 
	{
		hr = device_pancy->CreateTexture2D(&texDesc, 0, &tex_cubeinput[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect cubemap depthtex error", L"tip", MB_OK);
			return hr;
		}
		hr = device_pancy->CreateShaderResourceView(tex_cubeinput[i], 0, &reflect_cubeinput_SRV[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
			return hr;
		}
		hr = device_pancy->CreateRenderTargetView(tex_cubeinput[i], 0, &reflect_cubeinput_RTV[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
			return hr;
		}
		tex_cubeinput[i]->Release();
	}
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~静态cubemap深度缓冲~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width = 1024;
	dsDesc.Height = 1024;
	dsDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	dsDesc.ArraySize = 6;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MipLevels = 0;
	dsDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	ID3D11Texture2D* depthStencilBuffer;
	hr = device_pancy->CreateTexture2D(&dsDesc, 0, &depthStencilBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect cubemap depthtex error", L"tip", MB_OK);
		return hr;
	}
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc =
	{
		DXGI_FORMAT_D32_FLOAT,
		D3D11_DSV_DIMENSION_TEXTURE2DARRAY
	};
	dsvDesc.Texture2DArray.ArraySize = 1;
	dsvDesc.Texture2DArray.MipSlice = 0;
	for (int i = 0; i < 6; ++i) 
	{
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		hr = device_pancy->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, &reflect_DSV[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect cubemap depthbuffer error", L"tip", MB_OK);
			return hr;
		}
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC depthsrvDesc;
	depthsrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	depthsrvDesc.TextureCube.MipLevels = -1;
	depthsrvDesc.TextureCube.MostDetailedMip = 0;
	hr = device_pancy->CreateShaderResourceView(depthStencilBuffer, &depthsrvDesc, &reflect_depthcube_SRV);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect cubemap depthtex error", L"tip", MB_OK);
		return hr;
	}
	depthStencilBuffer->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~静态cubemap纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//渲染目标
	D3D11_TEXTURE2D_DESC cubeMapDesc;
	cubeMapDesc.Width = 1024;  
	cubeMapDesc.Height = 1024;
	cubeMapDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	cubeMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubeMapDesc.ArraySize = 6;
	cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapDesc.CPUAccessFlags = 0;
	cubeMapDesc.MipLevels = 0;
	cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	cubeMapDesc.SampleDesc.Count = 1;
	cubeMapDesc.SampleDesc.Quality = 0;
	//使用以上描述创建纹理
	ID3D11Texture2D *cubeMap(NULL);
	hr = device_pancy->CreateTexture2D(&cubeMapDesc, 0, &cubeMap);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect cubemap texture1 error", L"tip", MB_OK);
		return hr;
	}
	//创建六个rendertarget
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = cubeMapDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;
	for (UINT i = 0; i<6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		hr = device_pancy->CreateRenderTargetView(cubeMap, &rtvDesc, &reflect_cube_RTV[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect cubemap RTV error", L"tip", MB_OK);
			return hr;
		}
	}
	//创建一个SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = cubeMapDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = -1;
	srvDesc.TextureCube.MostDetailedMip = 0;
	hr = device_pancy->CreateShaderResourceView(cubeMap, &srvDesc, &reflect_cube_SRV);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect cubemap SRV error", L"tip", MB_OK);
		return hr;
	}
	cubeMap->Release();
	return S_OK;
}
void render_posttreatment_SSR::set_static_cube_rendertarget(int count_cube, XMFLOAT4X4 &mat_project)
{
	ID3D11RenderTargetView* renderTargets[1] = { reflect_cubeinput_RTV[count_cube] };
	contex_pancy->OMSetRenderTargets(1, renderTargets, reflect_DSV[count_cube]);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
	contex_pancy->ClearRenderTargetView(reflect_cubeinput_RTV[count_cube], clearColor);
	contex_pancy->ClearDepthStencilView(reflect_DSV[count_cube], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, count_cube);
	D3D11_VIEWPORT shadow_map_VP;
	shadow_map_VP.TopLeftX = 0.0f;
	shadow_map_VP.TopLeftY = 0.0f;
	shadow_map_VP.Width = 1024.0f;
	shadow_map_VP.Height = 1024.0f;
	shadow_map_VP.MinDepth = 0.0f;
	shadow_map_VP.MaxDepth = 1.0f;
	contex_pancy->RSSetViewports(1, &shadow_map_VP);
	XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.5f, 1.0f, 0.1f, 300.0f);
	XMStoreFloat4x4(&mat_project, P);
}
void render_posttreatment_SSR::draw_static_cube(int count_cube)
{
	ID3D11RenderTargetView* renderTargets[1] = { reflect_cube_RTV[count_cube] };
	contex_pancy->OMSetRenderTargets(1, renderTargets, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
	contex_pancy->ClearRenderTargetView(reflect_cube_RTV[count_cube], clearColor);
	D3D11_VIEWPORT shadow_map_VP;
	shadow_map_VP.TopLeftX = 0.0f;
	shadow_map_VP.TopLeftY = 0.0f;
	shadow_map_VP.Width = 1024.0f;
	shadow_map_VP.Height = 1024.0f;
	shadow_map_VP.MinDepth = 0.0f;
	shadow_map_VP.MaxDepth = 1.0f;
	auto shader_draw = shader_list->get_shader_cubesave();
	XMFLOAT3 cubecount_vec = XMFLOAT3(static_cast<float>(count_cube),0.0f,0.0f);
	shader_draw->set_cube_count(cubecount_vec);
	shader_draw->set_texture_input(reflect_cubeinput_SRV[count_cube]);
	//渲染屏幕空间像素图
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &reflectMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(reflectMap_IB, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	//选定绘制路径
	shader_draw->get_technique(&tech, "resolove_alpha");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_draw->set_texture_input(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void render_posttreatment_SSR::BuildFrustumFarCorners(float fovy, float farZ)
{
	float aspect = (float)map_width / (float)map_height;

	float halfHeight = farZ * tanf(0.5f*fovy);
	float halfWidth = aspect * halfHeight;

	FrustumFarCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	FrustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	FrustumFarCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	FrustumFarCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
}
void render_posttreatment_SSR::set_size(int width, int height, float fovy, float farZ)
{
	map_width = width;
	map_height = height;
	//半屏幕渲染
	render_viewport.TopLeftX = 0.0f;
	render_viewport.TopLeftY = 0.0f;
	render_viewport.Width = static_cast<float>(width);
	render_viewport.Height = static_cast<float>(height);
	render_viewport.MinDepth = 0.0f;
	render_viewport.MaxDepth = 1.0f;
	BuildFrustumFarCorners(fovy, farZ);
}
void render_posttreatment_SSR::build_fullscreen_picturebuff()
{
	pancy_point v[4];

	v[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].position = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v[2].position = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v[3].position = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	// Store far plane frustum corner indices in Normal.x slot.
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

	device_pancy->CreateBuffer(&vbd, &vinitData, &reflectMap_VB);

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

	device_pancy->CreateBuffer(&ibd, &iinitData, &reflectMap_IB);
}
void render_posttreatment_SSR::set_normaldepthcolormap(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need)
{
	normaldepth_tex = normalspec_need;
	depth_tex = depth_need;
}
void render_posttreatment_SSR::release()
{
	safe_release(mask_tex);
	safe_release(mask_target);
	safe_release(reflectMap_VB);
	safe_release(reflectMap_IB);
	safe_release(reflect_target);
	safe_release(reflect_tex);
	safe_release(color_tex);
	safe_release(reflect_cube_SRV);
	safe_release(reflect_depthcube_SRV);
	for (int i = 0; i < 6; ++i) 
	{
		safe_release(reflect_cubeinput_RTV[i]);
		safe_release(reflect_cubeinput_SRV[i]);
		safe_release(reflect_cube_RTV[i]);
		safe_release(reflect_DSV[i]);
	}
}
void render_posttreatment_SSR::draw_reflect(ID3D11RenderTargetView *rendertarget_input)
{
	ID3D11Resource *rendertargetTex = 0;
	ID3D11Resource *rendertargetTex_singlesample = 0;
	rendertarget_input->GetResource(&rendertargetTex);
	color_tex->GetResource(&rendertargetTex_singlesample);
	//将多重采样纹理转换至非多重纹理
	contex_pancy->ResolveSubresource(rendertargetTex_singlesample, D3D11CalcSubresource(0, 0, 1), rendertargetTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	color_tex->Release();
	color_tex = NULL;
	device_pancy->CreateShaderResourceView(rendertargetTex_singlesample, 0, &color_tex);
	rendertargetTex->Release();
	rendertargetTex_singlesample->Release();
	//绑定渲染目标纹理，不设置深度模缓冲区因为这里不需要
	ID3D11RenderTargetView* renderTargets[2] = { reflect_target,mask_target };
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	//contex_pancy->OMSetRenderTargets(2, renderTargets, 0);
	//contex_pancy->ClearRenderTargetView(reflect_target, clearColor);
	//contex_pancy->RSSetViewports(1, &render_viewport);
	renderstate_lib->clear_posttreatmentcrendertarget();
	//设置渲染状态
	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, map_width*1.0f / map_height*1.0f, 0.1f, 300.0f);
	XMFLOAT4X4 PT;
	XMStoreFloat4x4(&PT, P*T);
	auto *shader_reflectpass = shader_list->get_shader_ssreflect();
	XMFLOAT4X4 invview_mat, view_mat;
	XMFLOAT3 view_pos;
	camera_use->count_invview_matrix(&invview_mat);
	camera_use->count_view_matrix(&view_mat);
	XMMATRIX rec_need = XMLoadFloat4x4(&invview_mat) * XMLoadFloat4x4(&view_mat);

	camera_use->get_view_position(&view_pos);
	shader_reflectpass->set_invview_matrix(&invview_mat);
	shader_reflectpass->set_view_matrix(&view_mat);
	shader_reflectpass->set_view_pos(view_pos);
	shader_reflectpass->set_ViewToTexSpace(&PT);
	shader_reflectpass->set_FrustumCorners(FrustumFarCorner);
	shader_reflectpass->set_NormalDepthtex(normaldepth_tex);
	shader_reflectpass->set_Depthtex(depth_tex);
	shader_reflectpass->set_diffusetex(color_tex);

	shader_reflectpass->set_cubeview_matrix(static_cube_view_matrix,6);
	shader_reflectpass->set_enviroment_depth(reflect_depthcube_SRV);
	shader_reflectpass->set_enviroment_tex(reflect_cube_SRV);
	//渲染屏幕空间像素图
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &reflectMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(reflectMap_IB, DXGI_FORMAT_R16_UINT, 0);

	ID3DX11EffectTechnique* tech;
	//选定绘制路径
	shader_reflectpass->get_technique(&tech, "draw_ssrmap");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_reflectpass->set_NormalDepthtex(NULL);
	shader_reflectpass->set_diffusetex(NULL);
	shader_reflectpass->set_enviroment_tex(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
	renderstate_lib->clear_basicrendertarget();
}
void render_posttreatment_SSR::set_static_cube_view_matrix(int count_cube, XMFLOAT4X4 mat_input)
{
	static_cube_view_matrix[count_cube] = mat_input;
}
//继承的d3d注册类
class d3d_pancy_1 :public d3d_pancy_basic
{
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
	
public:
	d3d_pancy_1(HWND wind_hwnd, UINT wind_width, UINT wind_hight, HINSTANCE hInstance);
	HRESULT init_create();
	void update();
	void display();
	void release();
private:
	void render_static_enviroment_map();
};
void d3d_pancy_1::release()
{
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
	test_camera = new pancy_camera(device_pancy, window_width, window_hight);
	test_input = new pancy_input(wind_hwnd, device_pancy, hInstance);
	geometry_list = new geometry_control(device_pancy, contex_pancy);
	light_list = new light_control(device_pancy, contex_pancy, 20);
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
	first_scene_test = new scene_engine_test(device_pancy, contex_pancy, render_state, test_input, test_camera, shader_list, geometry_list, light_list, wind_width, wind_hight);
	hr = first_scene_test->scene_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create scene failed", L"tip", MB_OK);
		return hr;
	}
	posttreat_scene = new render_posttreatment_HDR(device_pancy, contex_pancy, render_state->get_postrendertarget(), shader_list, wind_width, wind_hight, render_state);
	hr = posttreat_scene->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create posttreat_class failed", L"tip", MB_OK);
		return hr;
	}
	posttreat_reflect = new render_posttreatment_SSR(test_camera,render_state,device_pancy, contex_pancy,  shader_list,geometry_list, wind_width, wind_hight, XM_PI*0.25f, 300.0f);
	hr = posttreat_reflect->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create posttreat_reflect failed", L"tip", MB_OK);
		return hr;
	}
	pretreat_scene = new Pretreatment_gbuffer(wind_width, wind_hight, device_pancy, contex_pancy, render_state, shader_list, geometry_list, test_camera, light_list);
	hr = pretreat_scene->create();
	if (FAILED(hr))
	{
		MessageBox(0, L"create pretreat_class failed", L"tip", MB_OK);
		return hr;
	}
	render_static_enviroment_map();
	return S_OK;
}
void d3d_pancy_1::render_static_enviroment_map()
{
	XMFLOAT3 look_vec, up_vec, position_vec;
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
	position_vec = XMFLOAT3(0.0f,5.0f,0.0f);
	for (int i = 0; i < 6; ++i) 
	{
		posttreat_reflect->set_static_cube_rendertarget(i, proj_matrix);
		look_vec = look[i];
		up_vec = up[i];
		test_camera->set_camera(look_vec, up_vec, position_vec);
		first_scene_test->set_proj_matrix(proj_matrix);
		XMFLOAT4X4 rec_viewmat;
		test_camera->count_view_matrix(&rec_viewmat);
		posttreat_reflect->set_static_cube_view_matrix(i, rec_viewmat);
		update();
		first_scene_test->display_enviroment();
		posttreat_reflect->draw_static_cube(i);
	}
	pretreat_scene->restore_proj_matrix();
	first_scene_test->reset_proj_matrix();
	test_camera->reset_camera();
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
	//render_static_enviroment_map();
	//初始化
	pretreat_scene->display(true);
	render_state->clear_basicrendertarget();
	
	render_state->clear_posttreatmentcrendertarget();
	first_scene_test->get_gbuffer(pretreat_scene->get_gbuffer_normalspec(), pretreat_scene->get_gbuffer_depth());
	first_scene_test->get_lbuffer(pretreat_scene->get_gbuffer_difusse(), pretreat_scene->get_gbuffer_specular());
	//render_state->set_posttreatment_rendertarget();
	//first_scene_test->get_environment_map(posttreat_reflect->get_cubemap());
	first_scene_test->display_shadowao(true,true);
	render_state->set_posttreatment_rendertarget();
	first_scene_test->display();
	render_state->set_posttreatment_rendertarget();
	posttreat_reflect->set_normaldepthcolormap(pretreat_scene->get_gbuffer_normalspec(), pretreat_scene->get_gbuffer_depth());
	posttreat_reflect->draw_reflect(render_state->get_postrendertarget());
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

