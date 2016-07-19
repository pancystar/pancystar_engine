#pragma once
#include"geometry.h"
#include"pancy_time_basic.h"
#include"PancyCamera.h"
#include"PancyInput.h"
#include"shader_pancy.h"
#include"pancy_model_import.h"
#include"engine_shadow.h"
#include"pancy_d3d11_basic.h"
class scene_root
{
protected:
	ID3D11Device           *device_pancy;     //d3d设备
	ID3D11DeviceContext    *contex_pancy;     //设备描述表
	shader_control         *shader_lib;       //shader资源
	pancy_input            *user_input;       //输入输出控制
	pancy_camera           *scene_camera;     //虚拟摄像机
	shadow_basic           *shadowmap_part;
	d3d_pancy_basic        *engine_state;
	XMFLOAT4X4             view_matrix;
	XMFLOAT4X4             proj_matrix;
	int                    scene_window_width;
	int                    scene_window_height;

public:
	scene_root(d3d_pancy_basic *engine_root,ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need,int width,int height);
	virtual HRESULT scene_create() = 0;
	virtual HRESULT display() = 0;
	virtual HRESULT update(float delta_time) = 0;
	virtual HRESULT release() = 0;
protected:
	virtual HRESULT camera_move();

};
class scene_engine_test : public scene_root
{
	model_reader_assimp                   *model_yuri;          //yuri模型
	Geometry<point_with_tangent>          *floor_need;          //盒子模型
	Geometry<point_with_tangent>          *ball_need;           //球体模型

	ID3D11ShaderResourceView              *tex_floor;           //地面纹理视图指针
	ID3D11ShaderResourceView              *tex_normal;          //法线贴图
public:
	scene_engine_test(d3d_pancy_basic *engine_root, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, int width, int height);
	HRESULT scene_create();
	HRESULT display();
	HRESULT update(float delta_time);
	HRESULT release();
private:
	void show_ball();
	void show_floor();
	void show_yuri();
	void show_lightsource();
	void draw_shadowmap();
	//	HRESULT get_scene_point(Geometry<point_with_tangent> **rec_geometry, int &all_geometry);
};
