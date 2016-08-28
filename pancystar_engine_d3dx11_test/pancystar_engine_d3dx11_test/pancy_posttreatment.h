#pragma once
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
#include"geometry.h"
//HDR后处理类，在渲染完成后负责处理屏幕空间图像
class render_posttreatment_HDR
{
	//全屏四边形
	ID3D11Buffer                *HDRMap_VB;            //ao图片顶点缓冲区
	ID3D11Buffer                *HDRMap_IB;            //ao图片索引缓冲区
													   //来自原渲染部分的共享变量
	ID3D11Device                *device_pancy;
	ID3D11DeviceContext         *contex_pancy;
	ID3D11RenderTargetView      *rendertarget_input; //前一部分完成的渲染目标
	shader_control              *shader_list;        //shader表
	int                         width, height;       //屏幕宽高
													 //内部变量
	ID3D11UnorderedAccessView   *UAV_HDR_mid;        //HDR的缓冲区，用于中间计算
	ID3D11UnorderedAccessView   *UAV_HDR_final;      //HDR的缓冲区，用于存储结果
	ID3D11ShaderResourceView    *SRV_HDR_map;        //HDR的缓冲区，用于存储map结果
	ID3D11ShaderResourceView    *SRV_HDR_use;        //HDR输入部分，要把屏幕像素转换成非抗锯齿的纹理

	ID3D11ShaderResourceView    *SRV_HDR_save;       //HDR高光存储部分渲染资源，将高光进行存储。
	ID3D11RenderTargetView      *RTV_HDR_save;       //HDR高光存储部分渲染目标，将高光进行存储。

	ID3D11ShaderResourceView    *SRV_HDR_blur1;       //HDR高光模糊渲染资源。
	ID3D11RenderTargetView      *RTV_HDR_blur1;       //HDR高光模糊渲染目标。
	ID3D11ShaderResourceView    *SRV_HDR_blur2;       //HDR高光模糊渲染资源。
	ID3D11RenderTargetView      *RTV_HDR_blur2;       //HDR高光模糊渲染目标。

	D3D11_VIEWPORT              render_viewport;      //视口信息
	ID3D11Buffer*               CPU_read_buffer;
	float                       average_light;
	float                       average_light_last;
	d3d_pancy_basic             *root_state_need;
	int width_rec, height_rec, buffer_num, map_num;
public:
	render_posttreatment_HDR(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, ID3D11RenderTargetView *rendertarget_need, shader_control *shaderlist_need, int width_need, int height_need, d3d_pancy_basic *rec_rootstate);
	HRESULT create();
	void release();
	HRESULT display();
private:
	HRESULT init_buffer();
	HRESULT init_texture();
	HRESULT CreateCPUaccessBuf(int size_need);
	HRESULT build_fullscreen_picturebuff();
	void basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);
	HRESULT count_average_light();
	HRESULT build_preblur_map();
	HRESULT blur_map();
	HRESULT HDR_map();

};