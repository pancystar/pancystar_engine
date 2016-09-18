#include"pancy_DXrenderstate.h"
pancy_renderstate::pancy_renderstate(ID3D11Device *device_need, ID3D11DeviceContext *contex_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	CULL_front = NULL;
}
HRESULT pancy_renderstate::create(int wind_width, int wind_height,IDXGISwapChain *swapchain_need)
{
	swapchain = swapchain_need;
	HRESULT hr = init_CULL_front();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = init_common_blend();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = init_CULL_none();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = change_size(wind_width, wind_height);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT pancy_renderstate::change_size(int wind_width, int wind_height) 
{
	HRESULT hr;
	//~~~~~~~~~~~~~~~~~~~~~~~~创建后处理渲染目标~~~~~~~~~~~~~~~~~~~~~~~
	//创建输入资源
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = wind_width;
	texDesc.Height = wind_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 4;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* posttreatment_tex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &posttreatment_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create posttreatment render target tex error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(posttreatment_tex, 0, &posttreatment_RTV);
	if (FAILED(hr))
	{
		MessageBox(0, L"create posttreatment render target view error", L"tip", MB_OK);
		return hr;
	}
	posttreatment_tex->Release();

	//~~~~~~~~~~~~~~~~~~~~~~~创建视图资源
	ID3D11Texture2D *backBuffer = NULL;
	//获取后缓冲区地址 
	hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	//创建视图
	if (FAILED(hr))
	{
		MessageBox(0, L"change size error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView);
	D3D11_TEXTURE2D_DESC check;
	backBuffer->GetDesc(&check);


	if (FAILED(hr))
	{
		MessageBox(0, L"change size error", L"tip", MB_OK);
		return hr;
	}
	//释放后缓冲区引用  
	backBuffer->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~创建深度及模板缓冲区
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.Width = wind_width;
	dsDesc.Height = wind_height;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.SampleDesc.Count = 4;
	dsDesc.SampleDesc.Quality = 0;

	ID3D11Texture2D* depthStencilBuffer;
	device_pancy->CreateTexture2D(&dsDesc, 0, &depthStencilBuffer);
	device_pancy->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView);
	depthStencilBuffer->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~绑定视图信息到渲染管线
	contex_pancy->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~设置视口变换信息
	viewPort.Width = static_cast<FLOAT>(wind_width);
	viewPort.Height = static_cast<FLOAT>(wind_height);
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	contex_pancy->RSSetViewports(1, &viewPort);
	return S_OK;
}
void pancy_renderstate::restore_rendertarget()
{
	contex_pancy->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
	contex_pancy->RSSetViewports(1, &viewPort);
}
void pancy_renderstate::restore_rendertarget(ID3D11DepthStencilView *depthStenci_need)
{
	contex_pancy->OMSetRenderTargets(1, &m_renderTargetView, depthStenci_need);
	contex_pancy->RSSetViewports(1, &viewPort);
}
void pancy_renderstate::clear_basicrendertarget()
{
	XMVECTORF32 color = { 0.75f,0.75f,0.75f,1.0f };
	contex_pancy->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<float*>(&color));
	contex_pancy->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}
void pancy_renderstate::clear_posttreatmentcrendertarget()
{
	XMVECTORF32 color = { 0.75f,0.75f,0.75f,1.0f };
	contex_pancy->ClearRenderTargetView(posttreatment_RTV, reinterpret_cast<float*>(&color));
	//contex_pancy->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}
void pancy_renderstate::set_posttreatment_rendertarget()
{
	contex_pancy->OMSetRenderTargets(1, &posttreatment_RTV, depthStencilView);
	contex_pancy->RSSetViewports(1, &viewPort);
}
void pancy_renderstate::set_posttreatment_rendertarget(ID3D11DepthStencilView *depthStenci_need)
{
	contex_pancy->OMSetRenderTargets(1, &posttreatment_RTV, depthStenci_need);
	contex_pancy->RSSetViewports(1, &viewPort);
}
void pancy_renderstate::release()
{
	CULL_front->Release();
	CULL_none->Release();
	blend_common->Release();
	CULL_front = NULL;

	m_renderTargetView->Release();
	depthStencilView->Release();
	posttreatment_RTV->Release();
}
HRESULT pancy_renderstate::init_CULL_front()
{
	D3D11_RASTERIZER_DESC cull_front_Desc;
	ZeroMemory(&cull_front_Desc, sizeof(D3D11_RASTERIZER_DESC));
	cull_front_Desc.FillMode = D3D11_FILL_SOLID;
	cull_front_Desc.CullMode = D3D11_CULL_FRONT;
	cull_front_Desc.FrontCounterClockwise = false;
	cull_front_Desc.DepthClipEnable = true;
	HRESULT hr = device_pancy->CreateRasterizerState(&cull_front_Desc, &CULL_front);
	if (FAILED(hr))
	{
		MessageBox(0, L"CULL_front mode init fail", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT pancy_renderstate::init_common_blend() 
{
	//开启透明  
	D3D11_BLEND_DESC transDesc;
	//先创建一个混合状态的描述  
	transDesc.AlphaToCoverageEnable = false;
	transDesc.IndependentBlendEnable = false;       
	transDesc.RenderTarget[0].BlendEnable = true;
	transDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	//创建ID3D11BlendState接口  
	HRESULT hr = device_pancy->CreateBlendState(&transDesc, &blend_common);
	if (FAILED(hr)) 
	{
		MessageBox(NULL,L"create commom blend state error",L"tip",MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT pancy_renderstate::init_CULL_none() 
{
	D3D11_RASTERIZER_DESC cull_none_Desc;
	ZeroMemory(&cull_none_Desc, sizeof(D3D11_RASTERIZER_DESC));
	cull_none_Desc.FillMode = D3D11_FILL_SOLID;
	cull_none_Desc.CullMode = D3D11_CULL_NONE;
	cull_none_Desc.FrontCounterClockwise = false;
	cull_none_Desc.DepthClipEnable = true;
	HRESULT hr = device_pancy->CreateRasterizerState(&cull_none_Desc, &CULL_none);
	if (FAILED(hr))
	{
		MessageBox(0, L"CULL_fnone mode init fail", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}