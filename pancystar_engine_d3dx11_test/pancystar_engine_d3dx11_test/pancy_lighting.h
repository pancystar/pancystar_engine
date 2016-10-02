#pragma once
#include"geometry.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"engine_shadow.h"
#include"pancy_d3d11_basic.h"
#include"pancy_DXrenderstate.h"
#include<vector>
class basic_lighting
{
protected:
	light_type             light_source_type;
	shadow_type            shadow_source_type;
	pancy_light_basic      light_data;
	shader_control         *shader_lib;       //shader资源
	ID3D11Device           *device_pancy;     //d3d设备
	ID3D11DeviceContext    *contex_pancy;     //设备描述表
	pancy_renderstate      *renderstate_lib;  //渲染格式
	geometry_control       *geometry_lib;
public:
	basic_lighting(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, geometry_control *geometry_lib_need);
	//前向光渲染
	void set_frontlight(int light_num);
	void set_light_ambient(float red, float green, float blue, float alpha);
	void set_light_diffuse(float red, float green, float blue, float alpha);
	void set_light_specular(float red, float green, float blue, float alpha);
protected:
	void init_comman_dirlight(shadow_type type_need_shadow);
	void init_comman_pointlight(shadow_type type_need_shadow);
	void init_comman_spotlight(shadow_type type_need_shadow);
};
class light_with_shadowmap : public basic_lighting
{
	shadow_basic *shadowmap_deal;
	BoundingSphere  cube_range;
public:
	light_with_shadowmap(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, geometry_control *geometry_lib_need);
	void set_shadow_range(XMFLOAT3 center, float range);
	HRESULT create(int width_need, int height_need);
	void draw_shadow();
	ID3D11ShaderResourceView* get_mapresource() { return shadowmap_deal->get_mapresource(); };
	XMFLOAT4X4 get_ViewProjTex_matrix() { return shadowmap_deal->get_ViewProjTex_matrix(); };
	void release();
};
class light_with_shadowvolume : public basic_lighting
{
	pancy_shadow_volume *shadowvolume_deal;
public:
	light_with_shadowvolume(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, geometry_control *geometry_lib_need);
	HRESULT create(int vertex_num);
	void build_shadow(ID3D11DepthStencilView* depth_input);
	void draw_shadow_volume();
	void update_view_proj_matrix(XMFLOAT4X4 mat_need);
	void release();
};

