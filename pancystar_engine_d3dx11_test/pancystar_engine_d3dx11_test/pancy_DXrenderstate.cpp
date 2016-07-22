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
	return S_OK;
}
void pancy_renderstate::release()
{
	CULL_front->Release();
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