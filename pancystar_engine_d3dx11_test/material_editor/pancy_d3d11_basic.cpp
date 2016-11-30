#include"pancy_d3d11_basic.h"
d3d_pancy_basic::d3d_pancy_basic(HWND hwnd_need, UINT width_need, UINT hight_need)
{
	device_pancy = NULL;
	contex_pancy = NULL;
	swapchain = NULL;
	//m_renderTargetView = NULL;
	//depthStencilView = NULL;
	//posttreatment_RTV = NULL;
	wind_hwnd  = hwnd_need;
	wind_width = width_need;
	wind_hight = hight_need;
}
HRESULT d3d_pancy_basic::init(HWND hwnd_need, UINT width_need, UINT hight_need)
{
	UINT create_flag = 0;
	bool if_use_HIGHCARD = true;
	//debug格式下选择返回调试信息
#if defined(DEBUG) || defined(_DEBUG)
	create_flag = D3D11_CREATE_DEVICE_DEBUG;
	if_use_HIGHCARD = false;
#endif
	//~~~~~~~~~~~~~~~~~创建d3d设备以及d3d设备描述表
	HRESULT hr;
	if (if_use_HIGHCARD == true)
	{
		std::vector<IDXGIAdapter1*> vAdapters;
		IDXGIFactory1* factory;
		CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);
		IDXGIAdapter1 * pAdapter = 0;
		DXGI_ADAPTER_DESC1 pancy_star;
		UINT i = 0;
		//HRESULT check_hardweare;
		while (factory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			vAdapters.push_back(pAdapter);
			++i;
		}
		vAdapters[1]->GetDesc1(&pancy_star);
		hr = D3D11CreateDevice(vAdapters[1], D3D_DRIVER_TYPE_UNKNOWN, NULL, create_flag, 0, 0, D3D11_SDK_VERSION, &device_pancy, &leave_need, &contex_pancy);
		int a = 0;
	}
	else
	{
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, create_flag, 0, 0, D3D11_SDK_VERSION, &device_pancy, &leave_need, &contex_pancy);
	}
	if (FAILED(hr))
	{
		MessageBox(hwnd_need, L"d3d设备创建失败", L"提示", MB_OK);
		return false;
	}
	if (leave_need != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(hwnd_need, L"显卡不支持d3d11", L"提示", MB_OK);
		return false;
	}
	//return true;
	//~~~~~~~~~~~~~~~~~~检查是否支持四倍抗锯齿
	device_pancy->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &check_4x_msaa);
	if (create_flag == D3D11_CREATE_DEVICE_DEBUG)
	{
		assert(check_4x_msaa > 0);
	}
	//~~~~~~~~~~~~~~~~~~设置交换链的缓冲区格式信息
	DXGI_SWAP_CHAIN_DESC swapchain_format;//定义缓冲区结构体
	swapchain_format.BufferDesc.Width = width_need;
	swapchain_format.BufferDesc.Height = hight_need;
	swapchain_format.BufferDesc.RefreshRate.Numerator = 60;
	swapchain_format.BufferDesc.RefreshRate.Denominator = 1;
	swapchain_format.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchain_format.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapchain_format.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//设置缓冲区的抗锯齿信息
	if (check_4x_msaa > 0)
	{
		swapchain_format.SampleDesc.Count = 4;
		swapchain_format.SampleDesc.Quality = check_4x_msaa - 1;
	}
	else
	{
		swapchain_format.SampleDesc.Count = 1;
		swapchain_format.SampleDesc.Quality = 0;
	}
	swapchain_format.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//渲染格式为渲染到缓冲区
	swapchain_format.BufferCount = 1;                              //仅使用一个缓冲区作为后台缓存
	swapchain_format.OutputWindow = hwnd_need;                     //输出的窗口句柄
	swapchain_format.Windowed = true;                              //窗口模式
	swapchain_format.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;        //让渲染驱动选择最高效的方法
	swapchain_format.Flags = 0;                                    //是否全屏调整
	//~~~~~~~~~~~~~~~~~~~~~~~创建交换链
	IDXGIDevice *pDxgiDevice(NULL);
	HRESULT hr1 = device_pancy->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDxgiDevice));
	IDXGIAdapter *pDxgiAdapter(NULL);
	hr1 = pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDxgiAdapter));
	IDXGIFactory *pDxgiFactory(NULL);
	hr1 = pDxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDxgiFactory));
	hr1 = pDxgiFactory->CreateSwapChain(device_pancy, &swapchain_format, &swapchain);

	render_state = new pancy_renderstate(device_pancy, contex_pancy);
	hr = render_state->create(width_need, hight_need, swapchain);
	if (FAILED(hr))
	{
		return false;
	}
	//释放接口  
	pDxgiFactory->Release();
	pDxgiAdapter->Release();
	pDxgiDevice->Release();
	return true;
}
d3d_pancy_basic::~d3d_pancy_basic()
{
	//safe_release(device_pancy);
	//safe_release(contex_pancy);
	//safe_release(swapchain);
	//safe_release(m_renderTargetView);
	//safe_release(depthStencilView);
}
/*
bool d3d_pancy_basic::change_size() 
{

	//~~~~~~~~~~~~~~~~~~~~~~~创建视图资源
	ID3D11Texture2D *backBuffer = NULL;
	//获取后缓冲区地址 
	HRESULT hr;
	hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	//创建视图
	if (FAILED(hr))
	{
		MessageBox(wind_hwnd, L"change size error", L"tip", MB_OK);
		return false;
	}
	hr = device_pancy->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView);
	D3D11_TEXTURE2D_DESC check;
	backBuffer->GetDesc(&check);


	if (FAILED(hr))
	{
		MessageBox(wind_hwnd, L"change size error", L"tip", MB_OK);
		return false;
	}
	//释放后缓冲区引用  
	backBuffer->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~创建深度及模板缓冲区
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.Width = wind_width;
	dsDesc.Height = wind_hight;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	if (check_4x_msaa > 0)
	{
		dsDesc.SampleDesc.Count = 4;
		dsDesc.SampleDesc.Quality = check_4x_msaa - 1;
	}
	else
	{
		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
	}
	ID3D11Texture2D* depthStencilBuffer;
	device_pancy->CreateTexture2D(&dsDesc, 0, &depthStencilBuffer);
	device_pancy->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView);
	depthStencilBuffer->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~绑定视图信息到渲染管线
	contex_pancy->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~设置视口变换信息
	viewPort.Width = static_cast<FLOAT>(wind_width);
	viewPort.Height = static_cast<FLOAT>(wind_hight);
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	contex_pancy->RSSetViewports(1, &viewPort);
	return true;
}
void d3d_pancy_basic::restore_rendertarget()
{
	contex_pancy->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
	contex_pancy->RSSetViewports(1, &viewPort);
}
void d3d_pancy_basic::set_posttreatment_rendertarget() 
{
	contex_pancy->OMSetRenderTargets(1, &posttreatment_RTV, depthStencilView);
	contex_pancy->RSSetViewports(1, &viewPort);
}
*/