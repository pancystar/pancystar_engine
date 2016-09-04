#include"pancy_DXrenderstate.h"
pancy_renderstate::pancy_renderstate(ID3D11Device *device_need, ID3D11DeviceContext *contex_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	CULL_front = NULL;
}
HRESULT pancy_renderstate::create()
{
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
	return S_OK;
}
void pancy_renderstate::release()
{
	CULL_front->Release();
	CULL_none->Release();
	blend_common->Release();
	CULL_front = NULL;
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