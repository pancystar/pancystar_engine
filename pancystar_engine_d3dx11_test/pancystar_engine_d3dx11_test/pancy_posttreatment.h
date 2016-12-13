#pragma once
#include"pancy_d3d11_basic.h"
#include"shader_pancy.h"
#include"geometry.h"
#include"PancyCamera.h"
#include"pancy_model_import.h"
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
	pancy_renderstate           *root_state_need;
	int width_rec, height_rec, buffer_num, map_num;
public:
	render_posttreatment_HDR(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, ID3D11RenderTargetView *rendertarget_need, shader_control *shaderlist_need, int width_need, int height_need, pancy_renderstate *rec_rootstate);
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
	ID3D11ShaderResourceView *input_mask_tex;                  //存储渲染结果的纹理资源

	ID3D11RenderTargetView   *reflect_target;             //存储动态屏幕空间反射的渲染目标
	ID3D11ShaderResourceView *reflect_tex;                //存储动态屏幕空间反射的纹理资源

	ID3D11RenderTargetView   *final_reflect_target;       //存储动态屏幕空间反射的渲染目标
	ID3D11ShaderResourceView *final_reflect_tex;          //存储动态屏幕空间反射的纹理资源

	ID3D11RenderTargetView   *blur_reflect_target;        //存储动态屏幕空间反射的渲染目标
	ID3D11ShaderResourceView *blur_reflect_tex;           //存储动态屏幕空间反射的纹理资源

	ID3D11RenderTargetView   *mask_target;                //存储动态屏幕空间反射掩码的渲染目标
	ID3D11ShaderResourceView *mask_tex;                   //存储动态屏幕空间反射掩码的纹理资源

	ID3D11ShaderResourceView *reflect_cubestencil_SRV;    //存储静态cubemapping的纹理资源
	ID3D11RenderTargetView   *reflect_cubestencil_RTV[6]; //存储静态cubemapping的渲染目标

	ID3D11ShaderResourceView *reflect_cube_SRV;           //存储静态cubemapping的纹理资源
	ID3D11RenderTargetView   *reflect_cube_RTV[6];        //存储静态cubemapping的渲染目标

	ID3D11ShaderResourceView *reflect_cubeinput_SRV[6];   //存储静态cubemapping的输入纹理
	ID3D11RenderTargetView   *reflect_cubeinput_RTV[6];   //存储静态cubemapping的输入目标

	ID3D11DepthStencilView   *reflect_DSV[6];             //深度缓冲区目标
	ID3D11ShaderResourceView *reflect_depthcube_SRV[6];      //深度立方贴图

	XMFLOAT4                 FrustumFarCorner[4];         //投影视截体的远截面的四个角点
	D3D11_VIEWPORT           render_viewport;             //视口信息
	D3D11_VIEWPORT           half_render_viewport;        //视口信息

	XMFLOAT3                 center_position;
	XMFLOAT4X4               static_cube_view_matrix[6];  //立方贴图的六个方向的取景变换
	shader_control           *shader_list;                //shader表

	float  width_static_cube;
public:
	render_posttreatment_SSR(pancy_camera *camera_need, pancy_renderstate *renderstate_need, ID3D11Device* device, ID3D11DeviceContext* dc, shader_control *shader_need, geometry_control *geometry_need, int width, int height, float fovy, float farZ);
	void set_normaldepthcolormap(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need);
	HRESULT create();
	void draw_reflect(ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input);
	void draw_static_cube(int count_cube);
	void set_static_cube_rendertarget(int count_cube, XMFLOAT4X4 &mat_project);
	void set_static_cube_view_matrix(int count_cube, XMFLOAT4X4 mat_input);
	void set_static_cube_centerposition(XMFLOAT3 mat_input);
	XMFLOAT3 get_center_position() { return center_position; };
	ID3D11ShaderResourceView *get_cubemap() { return reflect_cube_SRV; };
	void release();
private:
	void set_size(int width, int height, float fovy, float farZ);
	void build_fullscreen_picturebuff();
	void BuildFrustumFarCorners(float fovy, float farZ);
	HRESULT build_texture();
	void build_reflect_map(ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input);
	void blur_map();
	void basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);
	void basic_blur(ID3D11ShaderResourceView *mask,ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz);
	void draw_to_posttarget();
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