#pragma once
#include"geometry.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"engine_shadow.h"
#include"pancy_d3d11_basic.h"
#include"pancy_DXrenderstate.h"
#include"PancyCamera.h"
#include<vector>
using namespace DirectX;
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
	//延迟光渲染
	void set_defferedlight(int light_num);
	void set_light_ambient(float red, float green, float blue, float alpha);
	void set_light_diffuse(float red, float green, float blue, float alpha);
	void set_light_specular(float red, float green, float blue, float alpha);
	void set_light_position(float x, float y, float z);
	void set_light_dir(float x, float y, float z);
	void set_light_decay(float x0, float x1, float x2);
	void set_light_range(float range_need);
	void set_light_spottheata(float theta);
	void set_light_spotstrenth(float spot);
	shadow_type get_shadow_type() { return shadow_source_type; };
	light_type get_light_type() { return light_source_type; };

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
	HRESULT init_texture(ID3D11Texture2D* depthMap_array, int index_need) { return shadowmap_deal->init_texture(depthMap_array, index_need); };
	void draw_shadow();
	ID3D11ShaderResourceView* get_mapresource() { return shadowmap_deal->get_mapresource(); };
	XMFLOAT4X4 get_ViewProjTex_matrix() { return shadowmap_deal->get_ViewProjTex_matrix(); };
	void release();
};
class sunlight_with_shadowmap : public basic_lighting
{
	int shadow_devide;
	shadow_basic *shadowmap_array[20];
	XMFLOAT4X4 mat_sunlight_pssm[20];
public:
	sunlight_with_shadowmap(shader_control *lib_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, geometry_control *geometry_lib_need);
	HRESULT create(int width_need, int height_need, int shadow_num);
	HRESULT init_texture(ID3D11Texture2D* depthMap_array, int index_need,int count) { return shadowmap_array[count]->init_texture(depthMap_array, index_need); };
	void draw_shadow();
	void update_view_space(XMFLOAT4X4 mat_sunlight_view, int count) { mat_sunlight_pssm[count] = mat_sunlight_view; };
	ID3D11ShaderResourceView* get_mapresource(int count) { return shadowmap_array[count]->get_mapresource(); };
	XMFLOAT4X4 get_ViewProjTex_matrix(int count) { return shadowmap_array[count]->get_ViewProjTex_matrix(); };

	void set_deffered_sunlight();
	void set_front_sunlight() {}
	void release();
private:
	void draw_shadow_basic(int count);
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
class light_control
{
	//基本属性
	ID3D11Device             *device_pancy;     //d3d设备
	ID3D11DeviceContext      *contex_pancy;     //设备描述表
	shader_control           *shader_lib;
	geometry_control         *geometry_pancy;
	//太阳光
	bool                     if_sunlight_open;
	XMFLOAT3                 sunlight_dir;
	pancy_camera             *scene_camera;     //虚拟摄像机
	ID3D11Texture2D          *sunlight_pssm_ShadowArray;     //阳光分级阴影
	ID3D11ShaderResourceView *sunlight_pssm_Shadowresource;
	XMFLOAT4X4               mat_sunlight_pssm[20];
	float sunlight_pssm_depthdevide[20];
	int sunlight_divide_num;
	float sunlight_lamda_log;
	sunlight_with_shadowmap  *sun_pssmshadow_light;
	//普通光源
	int max_shadow_num;
	ID3D11Texture2D          *ShadowTextureArray;      //阴影图数组资源
	ID3D11ShaderResourceView *shadow_map_resource;
	std::vector<basic_lighting>                nonshadow_light_list;
	std::vector<light_with_shadowmap>          shadowmap_light_list;
	std::vector<light_with_shadowvolume>       shadowvalume_light_list;
	//投影视截体信息
	float                    perspective_near_plane;
	float                    perspective_far_plane;
	float                    perspective_angle;
	int                      wind_width;
	int                      wind_height;
	
public:
	light_control(ID3D11Device *device_need,ID3D11DeviceContext *contex_need, pancy_camera *camera_need,int shadow_num_need, float near_plane, float far_plane, float angle_view,int width_need,int height_need);
	void add_light_without_shadow(basic_lighting light_input);
	void add_light_witn_shadow_map(light_with_shadowmap light_input);
	HRESULT create(shader_control *shader_lib, geometry_control *geometry_lib, pancy_renderstate *renderstate_lib);
	std::vector<light_with_shadowmap>* get_lightdata_shadow() { return &shadowmap_light_list; };
	ID3D11ShaderResourceView  *get_shadow_map_resource() { return shadow_map_resource; };
	void release();
	void update_and_setlight();
	void draw_shadow();
	void set_sunlight(XMFLOAT3 light_dir,float lamda_log,int devide_num, int width_need, int height_need);
private:
	HRESULT get_shadow_map_matrix(XMFLOAT4X4* mat_out,int &mat_num_out);
	void divide_view_frustum(float lamda_log, int divide_num);
	XMFLOAT4X4 build_matrix_sunlight(float near_plane,float far_plane, XMFLOAT3 light_dir);
	void build_AABB_box(XMFLOAT4 near_point[4],XMFLOAT4 far_point[4],XMFLOAT3 &min_pos,XMFLOAT3 &max_pos);
};
