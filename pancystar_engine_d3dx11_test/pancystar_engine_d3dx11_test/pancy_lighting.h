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
	shader_control         *shader_lib;       //shader��Դ
	ID3D11Device           *device_pancy;     //d3d�豸
	ID3D11DeviceContext    *contex_pancy;     //�豸������
	pancy_renderstate      *renderstate_lib;  //��Ⱦ��ʽ
public:
	basic_lighting(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state);
	//ǰ�����Ⱦ
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
	std::vector<geometry_shadow> shadowmesh_list;
public:
	light_with_shadowmap(light_type type_need_light, shadow_type type_need_shadow, shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state);
	void set_shadow_range(XMFLOAT3 center, float range);
	HRESULT create(int width_need, int height_need);
	void draw_shadow();
	void clear_mesh();
	void add_mesh(geometry_shadow mesh_input);
	ID3D11ShaderResourceView* get_mapresource() { return shadowmap_deal->get_mapresource(); };
	XMFLOAT4X4 get_ViewProjTex_matrix() { return shadowmap_deal->get_ViewProjTex_matrix(); };
	void release();

};