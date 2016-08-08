#pragma once
#include<windows.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<vector>
#include<d3dx11effect.h>
#include<directxmath.h>
#pragma comment ( lib, "D3D11.lib")
#pragma comment ( lib, "dxgi.lib")
using namespace DirectX;
#define window_width 800
#define window_hight 600
class d3d_pancy_basic
{
protected:
	HWND wind_hwnd;
	UINT wind_width;
	UINT wind_hight;
	UINT                    check_4x_msaa;       //释放支持四倍抗锯齿
	ID3D11Device            *device_pancy;       //d3d设备
	ID3D11DeviceContext     *contex_pancy;       //设备描述表
	D3D_FEATURE_LEVEL       leave_need;          //显卡支持的directx等级
	IDXGISwapChain          *swapchain;          //交换链信息	
    ID3D11RenderTargetView  *m_renderTargetView; //视图变量
	D3D11_VIEWPORT          viewPort;            //视口信息
    ID3D11DepthStencilView  *depthStencilView;   //缓冲区信息

	ID3D11RenderTargetView  *posttreatment_RTV;  //用于后处理的渲染目标
public:
	d3d_pancy_basic(HWND wind_hwnd,UINT wind_width,UINT wind_hight);
	~d3d_pancy_basic();
	virtual void update()   = 0;
	virtual void display()  = 0;
	virtual void release()  = 0;
	bool change_size();
	void restore_rendertarget();
	void set_posttreatment_rendertarget();
protected:	
	HRESULT init(HWND wind_hwnd,UINT wind_width,UINT wind_hight);
	
	template<class T> 
	void safe_release(T t)
	{
		if(t != NULL)
		{
			t->Release();
			t = 0;
		}
	}
};