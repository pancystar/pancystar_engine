#pragma once
#include"geometry.h"
#include"pancy_time_basic.h"
#include"PancyCamera.h"
#include"PancyInput.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"engine_shadow.h"
#include"pancy_d3d11_basic.h"
#include"pancy_DXrenderstate.h"
#include"pancy_ssao.h"
#include"pancy_lighting.h"
#include"particle_system.h"
class scene_root
{
protected:
	ID3D11Device              *device_pancy;     //d3d设备
	ID3D11DeviceContext       *contex_pancy;     //设备描述表
	geometry_control          *geometry_lib;     //几何体资源
	shader_control            *shader_lib;       //shader资源
	pancy_renderstate         *renderstate_lib;  //渲染格式
	pancy_input               *user_input;       //输入输出控制
	pancy_camera              *scene_camera;     //虚拟摄像机
	//shadow_basic           *shadowmap_part;
	ssao_pancy                *ssao_part;
	//d3d_pancy_basic        *engine_state;
	XMFLOAT4X4                view_matrix;
	XMFLOAT4X4                proj_matrix;
	int                       scene_window_width;
	int                       scene_window_height;
	float                     time_game;
	ID3D11ShaderResourceView  *gbuffer_normalspec;
	ID3D11ShaderResourceView  *gbuffer_depth;

public:
	scene_root(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state,pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need,int width,int height);
	virtual HRESULT scene_create() = 0;
	void get_gbuffer(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need);
	virtual HRESULT display() = 0;
	virtual HRESULT display_nopost() = 0;
	virtual HRESULT update(float delta_time) = 0;
	virtual HRESULT release() = 0;
protected:
	virtual HRESULT camera_move();

};

class scene_engine_test : public scene_root
{
	vector<basic_lighting>                nonshadow_light_list;
	vector<light_with_shadowmap>          shadowmap_light_list;
	vector<light_with_shadowvolume>       shadowvalume_light_list;
	particle_system<fire_point>           *particle_fire;

public:
	scene_engine_test(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state,pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, int width, int height);
	HRESULT scene_create();
	HRESULT display();
	
	HRESULT display_nopost();
	HRESULT update(float delta_time);
	HRESULT release();
private:
	void show_ball();
	void show_floor();
	void show_aotestproj();
	void show_castel();
	void show_lightsource();
	void draw_shadowmap();
	void draw_ssaomap();
	void show_fire_particle();
	void show_yuri_animation();
	void show_billboard();
};
