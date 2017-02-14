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
#include"pancy_terrain.h"
#include"pancy_physx.h"


class scene_root
{
protected:
	ID3D11Device              *device_pancy;     //d3d设备
	ID3D11DeviceContext       *contex_pancy;     //设备描述表
	geometry_control          *geometry_lib;     //几何体资源
	shader_control            *shader_lib;       //shader资源
	pancy_renderstate         *renderstate_lib;  //渲染格式
	light_control             *light_list;       //光源
	pancy_input               *user_input;       //输入输出控制
	pancy_camera              *scene_camera;     //虚拟摄像机
	//shadow_basic           *shadowmap_part;
	//ssao_pancy                *ssao_part;
	//d3d_pancy_basic        *engine_state;
	XMFLOAT4X4                view_matrix;
	XMFLOAT4X4                proj_matrix;
	int                       scene_window_width;
	int                       scene_window_height;
	float                     time_game;
	ID3D11ShaderResourceView  *gbuffer_normalspec;
	ID3D11ShaderResourceView  *gbuffer_depth;
	ID3D11ShaderResourceView  *lbuffer_diffuse;
	ID3D11ShaderResourceView  *lbuffer_specular;
	ID3D11ShaderResourceView  *environment_map;
	float                     perspective_near_plane;
	float                     perspective_far_plane;
	float                     perspective_angle;

public:
	scene_root(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state,pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need,int width,int height,float near_plane,float far_plane,float angle_view);
	virtual HRESULT scene_create() = 0;
	void get_gbuffer(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need);
	void get_lbuffer(ID3D11ShaderResourceView *diffuse_need, ID3D11ShaderResourceView *specular_need);
	virtual HRESULT display() = 0;
	virtual HRESULT display_enviroment() = 0;
	virtual HRESULT display_nopost() = 0;
	virtual HRESULT update(float delta_time) = 0;
	virtual HRESULT release() = 0;
	void set_proj_matrix(XMFLOAT4X4 proj_mat_need);
	void reset_proj_matrix();
	void get_environment_map(ID3D11ShaderResourceView  *input) { environment_map = input; };
protected:
	virtual HRESULT camera_move();

};
class scene_engine_test : public scene_root
{
	particle_system<fire_point>           *particle_fire;
public:
	scene_engine_test(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state,pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height, float near_plane, float far_plane, float angle_view);
	HRESULT scene_create();
	HRESULT display();
	HRESULT display_nopost();
	HRESULT display_enviroment();
	HRESULT update(float delta_time);
	HRESULT release();
private:
	void show_ball();
	void show_floor();
	void show_aotestproj();
	void show_castel(LPCSTR techname, LPCSTR technamenormal);
	void show_castel_deffered(LPCSTR techname);
	void show_lightsource();
	void show_fire_particle();
	void show_yuri_animation();
	void show_yuri_animation_deffered();
	void show_billboard();
};

class shader_snakecompute : public shader_basic
{
	ID3DX11EffectShaderResourceVariable      *snakecontrol_input;      //shader中的纹理资源句柄
	ID3DX11EffectUnorderedAccessViewVariable *snakepoint_output;       //shader中的纹理资源句柄
	ID3DX11EffectUnorderedAccessViewVariable *snakecontrol_output;	   //compute_shader计算完毕纹理资源
	ID3DX11EffectMatrixVariable              *Bspline_matrix;          //b样条矩阵
	ID3DX11EffectVariable                    *snake_range;             //蛇体大小
	ID3DX11EffectVariable                    *snake_head;             //蛇体大小
public:
	shader_snakecompute(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_input_buffer(ID3D11ShaderResourceView *buffer_input);
	HRESULT set_piccturerange(int snake_body_num, int devide_num, int radius, int others);
	HRESULT set_snake_head(XMFLOAT3 head_input);
	HRESULT set_output_buffer(ID3D11UnorderedAccessView *buffer_input_need, ID3D11UnorderedAccessView *buffer_output_need);
	void release();
	void dispatch(int snake_num, int snake_devide);
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_snaketesselate : public shader_basic
{
	ID3DX11EffectMatrixVariable                    *final_mat;
public:
	shader_snaketesselate(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);                            //设置总变换
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
struct point_snake
{
	XMFLOAT4 position;
	XMFLOAT4 center_position;
};
struct point_snake_control
{
	XMFLOAT3 position1;
	XMFLOAT3 position2;
	XMFLOAT3 position3;
	XMFLOAT3 position4;
};
class snake_draw
{
	ID3D11Device          *device_pancy;
	ID3D11DeviceContext   *contex_pancy;
	pancy_input           *user_input;       //输入输出控制
	shader_snakecompute   *first_CSshader;
	shader_snaketesselate *second_TLshader;
	int max_snake_length;         //蛇体长度上限
	int snake_length;             //蛇体长度
	int snake_radius;           //蛇体半径
	int devide_num;               //细分数量
	XMFLOAT3 snake_head_position; //蛇头位置
	pancy_camera *snake_head_normal;   //蛇头法线方向
	float time_all;
	ID3D11UnorderedAccessView   *UAV_controlpoint_first;  //控制点的缓冲区1
	ID3D11ShaderResourceView    *SRV_controlpoint_first;  //控制点的缓冲区1
	ID3D11UnorderedAccessView   *UAV_controlpoint_second; //控制点的缓冲区2
	ID3D11ShaderResourceView    *SRV_controlpoint_second; //控制点的缓冲区2

	ID3D11Buffer                *index_buffer_render;
	ID3D11Buffer                *point_buffer_UAV;
	ID3D11Buffer                *CPU_read_buffer;
	ID3D11UnorderedAccessView   *UAV_draw_point_bufeer;   //绘制点的缓冲区
public:
	snake_draw(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_input *input_need,int max_length_need, int devide_num_need);
	HRESULT create();
	void draw(XMFLOAT4X4 view_projmat);
	void update(float time_delta);
	XMFLOAT3 get_head_location() { return snake_head_position; };
	void get_head_rotation(XMFLOAT4X4 *mat) { snake_head_normal->count_invview_matrix(mat); };
	void release();
private:
	HRESULT build_controlbuffer();
	HRESULT build_render_buffer();
	HRESULT CreateCPUaccessBuf();
	void draw_pass(XMFLOAT4X4 view_projmat);
	HRESULT snake_rotate();
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
class scene_engine_snake : public scene_root
{
	snake_draw *test_snake;
	particle_system<fire_point>           *particle_fire;
public:
	scene_engine_snake(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height, float near_plane, float far_plane, float angle_view);
	HRESULT scene_create();
	HRESULT display();
	HRESULT display_nopost();
	HRESULT display_enviroment();
	HRESULT update(float delta_time);
	HRESULT release();
private:
	void show_floor();
	void show_ball();
	void show_snake_animation_deffered();
};

class player_basic
{
	float rec_offset_x, rec_offset_z;
	model_data_type model_type;
	//渲染包
	shader_control *shader_lib;
	//几何属性
	geometry_control *geometry_pancy;
	std::string model_resource;
	std::string model_view_data_name;
	int model_view_data_index;
	//物理属性
	pancy_physx  *physic_pancy;
	physx::PxController *player;
	physx::PxMaterial *mat_force;
	float x_speed, z_speed;
	float rotation_angle;
public:
	player_basic(string model_resource_name, string player_name, geometry_control *geometry_need, pancy_physx  *physic_need, shader_control *shader_need, model_data_type model_type_need);
	HRESULT create();
	virtual void update(float delta_time);
	virtual void display(XMFLOAT4X4 view_proj_matrix);
	void get_now_position(float &x,float &y,float &z);
	void set_speed(float x,float z);
	void get_look_position(float &x,float &y,float &z);
	void change_rotation_angle(float delta_angle);
	void add_offset(float offset_x, float offset_z) { rec_offset_x += offset_x; rec_offset_z += offset_z; };
	void release();
private:
	HRESULT init_physics();
	HRESULT init_geometry_buildin();
	HRESULT init_geometry_assimp();
	HRESULT init_geometry_bounding_box();
	void show_bounding_box(XMFLOAT4X4 view_proj_matrix);
	void show_build_in_model(XMFLOAT4X4 view_proj_matrix);
	void show_assimp_skinmesh_model(XMFLOAT4X4 view_proj_matrix);
	void show_transparent_part(XMFLOAT4X4 view_proj_matrix,int part);
};
class scene_engine_physicx : public scene_root
{
	player_basic *player_main;
	pancy_physx *physics_pancy;
	pancy_terrain_build *terrain_test;
	float camera_height;
public:
	scene_engine_physicx(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_physx *physx_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height, float near_plane, float far_plane, float angle_view);
	HRESULT scene_create();
	HRESULT display();
	HRESULT display_nopost();
	HRESULT display_enviroment();
	HRESULT update(float delta_time);
	HRESULT release();
private:
	void show_floor();
	void show_ball();
	void show_box();
	void set_camera_player();
};
