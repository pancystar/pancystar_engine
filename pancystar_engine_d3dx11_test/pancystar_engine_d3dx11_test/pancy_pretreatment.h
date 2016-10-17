#pragma once
#include"geometry.h"
#include"pancy_d3d11_basic.h"
#include"pancy_time_basic.h"
#include"PancyCamera.h"
#include"PancyInput.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"pancy_lighting.h"
class Pretreatment_gbuffer
{
	int                      map_width;
	int                      map_height;
	ID3D11Device             *device_pancy;
	ID3D11DeviceContext      *contex_pancy;
	pancy_renderstate        *renderstate_lib;
	shader_control           *shader_list;                //shader表
	geometry_control         *geometry_lib;               //几何体表
	light_control            *light_list;                 //光源表
	pancy_camera             *camera_use;                 //摄像机

	ID3D11ShaderResourceView *depthmap_tex;               //保存深度信息的纹理资源
	ID3D11DepthStencilView   *depthmap_target;            //用作渲染目标的缓冲区资源

	ID3D11RenderTargetView   *normalspec_target;          //存储法线和镜面反射系数的渲染目标
	ID3D11ShaderResourceView *normalspec_tex;             //存储法线和镜面反射系数的纹理资源

	ID3D11RenderTargetView   *gbuffer_diffuse_target;     //存储漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *gbuffer_diffuse_tex;        //存储漫反射光照效果的纹理资源

	ID3D11RenderTargetView   *gbuffer_specular_target;    //存储漫反射光照效果的渲染目标
	ID3D11ShaderResourceView *gbuffer_specular_tex;       //存储漫反射光照效果的纹理资源

	ID3D11RenderTargetView   *depthmap_single_target;     //存储深度msaa采样后信息的渲染目标
	ID3D11ShaderResourceView *depthmap_single_tex;        //存储深度msaa采样后信息的纹理资源

	XMFLOAT4                 FrustumFarCorner[4];         //投影视截体的远截面的四个角点
	D3D11_VIEWPORT           render_viewport;             //视口信息

	ID3D11Buffer             *depthbuffer_VB;             //深度采样纹理顶点缓冲区
	ID3D11Buffer             *lightbuffer_VB;             //光照纹理顶点缓冲区
	ID3D11Buffer             *lightbuffer_IB;             //光照纹理索引缓冲区
	XMFLOAT4X4               proj_matrix_gbuffer;         //投影变换
public:
	Pretreatment_gbuffer(int width_need, int height_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *renderstate_need, shader_control *shader_need, geometry_control *geometry_need, pancy_camera *camera_need, light_control *light_need);
	HRESULT create();
	//void update();
	void display();
	void release();
	ID3D11ShaderResourceView *get_gbuffer_normalspec() { return normalspec_tex; };
	ID3D11ShaderResourceView *get_gbuffer_depth() { return depthmap_single_tex; };
	ID3D11ShaderResourceView *get_gbuffer_difusse() { return gbuffer_diffuse_tex; };
	ID3D11ShaderResourceView *get_gbuffer_specular() { return gbuffer_specular_tex; };
private:
	void set_size();
	HRESULT init_texture();
	HRESULT init_buffer();
	void set_normalspecdepth_target();
	void set_multirender_target();
	void set_resolvdepth_target();
	void BuildFrustumFarCorners(float fovy, float farZ);
	void render_gbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix);
	void render_lbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 invview_matrix);
	ID3DX11EffectTechnique* get_technique();
	ID3DX11EffectTechnique* get_technique_transparent();
	ID3DX11EffectTechnique* get_technique_normal();
	ID3DX11EffectTechnique* get_technique_skin();
	ID3DX11EffectTechnique* get_technique_skin_transparent();
	ID3DX11EffectTechnique* get_technique_skin_normal();
	void resolve_depth_render(ID3DX11EffectTechnique* tech);
	void light_buffer_render(ID3DX11EffectTechnique* tech);
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