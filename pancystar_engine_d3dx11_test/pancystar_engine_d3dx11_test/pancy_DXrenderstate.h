#pragma once
#include"pancy_d3d11_basic.h"
class pancy_renderstate 
{
	ID3D11Device           *device_pancy;     //d3d设备
	ID3D11DeviceContext    *contex_pancy;     //设备描述表
	//渲染模式
	ID3D11RasterizerState  *CULL_front;       //消隐正面
public:
	pancy_renderstate(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT create();
	void release();
	ID3D11RasterizerState  *get_CULL_front_rs() { return CULL_front; };
private:
	HRESULT init_CULL_front();
};
