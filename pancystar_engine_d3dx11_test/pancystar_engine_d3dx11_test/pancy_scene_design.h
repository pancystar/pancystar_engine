#pragma once
#include <time.h> 
#include<queue>
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
#include"pancy_GUI_basic.h"

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
//角色系统
enum player_shape_physic 
{
	player_shape_box = 1,
	player_shape_capsule = 2
};
enum animal_behavior
{
	animal_walk_around = 0,
	animal_run_target = 1,
	animal_run_around = 2
};
class player_basic
{
protected:
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
	XMFLOAT3 init_position;
	float x_speed, z_speed;
	float rotation_angle;
	float model_scaling;
	bool if_show_bound_box;
	float model_head_radius;
	float model_height;
	float model_length_forward;
	float model_length_slide;
	player_shape_physic player_shape_use;
	float time_animation;
public:
	player_basic(string model_resource_name, string player_name, geometry_control *geometry_need, pancy_physx  *physic_need, shader_control *shader_need, model_data_type model_type_need, XMFLOAT3 st_position,bool bound_box_need);
	HRESULT create(float raidus,float height,float foward,float slide, player_shape_physic player_shape_choose);
	bool check_if_show_bound() { return if_show_bound_box; };
	virtual void update(float delta_time);
	virtual void display(XMFLOAT4X4 view_proj_matrix);
	void get_now_position(float &x,float &y,float &z);
	void set_speed(float x,float z);
	void get_look_position(float &x,float &y,float &z);
	void change_rotation_angle(float delta_angle);
	void reset_rotation_angle(float delta_angle);
	void set_model_scaling(float model_scal_need) { model_scaling = model_scal_need; };
	void add_offset(float offset_x, float offset_z) { rec_offset_x += offset_x; rec_offset_z += offset_z; };
	string get_model_resource_name() { return model_resource; };
	string get_model_view_name() { return model_view_data_name; };
	string get_bound_view_name() { return model_view_data_name + "bounding_box_player"; };
	void release();
protected:
	HRESULT init_physics(player_shape_physic player_shape_choose);
	HRESULT init_geometry_buildin();
	HRESULT init_geometry_assimp();
	HRESULT init_geometry_bounding_box();
	void show_bounding_box(XMFLOAT4X4 view_proj_matrix);
	void show_build_in_model(XMFLOAT4X4 view_proj_matrix);
	virtual void show_assimp_skinmesh_model(XMFLOAT4X4 view_proj_matrix);
	void show_transparent_part(XMFLOAT4X4 view_proj_matrix,int part);
};
struct animal_attribute 
{
	XMFLOAT3 position_center;
	float animation_walk_start;
	float animation_walk_end;
	float animation_run_start;
	float animation_run_end;
	float animation_walk_speed;
	float animation_run_speed;
	int hp_animal;
	int damage_animal;
	float velocity_walk;
	float velocity_run;
	float view_range;
	bool if_active_attack;
	bool if_attack;
	int burn_position_id;
	animal_attribute() 
	{
		animation_walk_start = 0;
		animation_walk_end = 0;
		animation_run_start = 0;
		animation_run_end = 0;
		hp_animal = 0;
		damage_animal = 0;
		velocity_walk = 0;
		velocity_run = 0;
		view_range = 0;
		if_active_attack = false;
		burn_position_id = 0;
	}
	animal_attribute(int hp_need, int damage_need, float walk_speed, float run_speed, float view_range_need, bool if_active_attack_need, int burn_positionID_need) 
	{
		animation_walk_start = 0;
		animation_walk_end = 0;
		animation_run_start = 0;
		animation_run_end = 0;
		hp_animal = hp_need;
		damage_animal = damage_need;
		velocity_walk = walk_speed;
		velocity_run = run_speed;
		view_range = view_range_need;
		if_active_attack = if_active_attack_need;
		burn_position_id = burn_positionID_need;
	}
};
class AI_animal : public player_basic 
{
	float animal_animation_time;
	animal_attribute animal_message;
	animal_behavior animal_now_behavior;
	XMFLOAT3 target;
	XMFLOAT3 speed_animal_dir;
	//XMFLOAT3 now_up_normal;
	//XMFLOAT3 last_position;
	std::queue<XMFLOAT3> real_speed_data;
	int speed_data_size;
	//XMFLOAT3 real_speed_direction;
	bool if_stop;
	float all_time;
	/*
	int hp_animal;
	int damage_animal;
	float velocity_walk;
	float velocity_run;
	float view_range;
	bool if_active_attack;
	bool if_attack;
	int burn_position_id;
	*/
public:
	void update(float delta_time);
	AI_animal(string model_resource_name, string player_name, geometry_control *geometry_need, pancy_physx  *physic_need, shader_control *shader_need, model_data_type model_type_need, animal_attribute animal_data, XMFLOAT3 st_position,bool bound_box_need);
	void set_animation_time(float time);
	//void set_now_up_normal(XMFLOAT3 now_normal) { now_up_normal = now_normal; };
	float get_animation_time() { return animal_animation_time; };
	void get_animation_data(float &animation_st,float &animation_ed,float &animation_speed);
	void check_if_stop();
	void set_position_center(XMFLOAT3 position_center) { animal_message.position_center = position_center; };
	void change_animal_target(XMFLOAT3 animal_target) { target = animal_target; };
private:
	void show_assimp_skinmesh_model(XMFLOAT4X4 view_proj_matrix);
	void update_target();
	//HRESULT get_walk_dir(XMFLOAT3 &walk_dir);
	HRESULT check_walk_dir(XMFLOAT3 walk_dir, XMFLOAT3 now_dir);
	XMFLOAT3 normalize_float(XMFLOAT3 input);
	//void init_animal_data(int hp_need, int damage_need, float walk_speed, float run_speed, float view_range_need, bool if_active_attack_need,int burn_positionID_need);
};
//植被系统
class static_resource_basic
{
	//传入单例
	pancy_physx  *physic_pancy;
	geometry_control *geometry_pancy;
	//识别属性
	std::string resource_view_name;
	int resource_map_ID;
	//基本几何属性
	float scaling;
	XMFLOAT3 position;
	//物理属性
	physx::PxRigidStatic *bound_box;
	physx::PxMaterial *mat_force;
public:
	static_resource_basic(int map_ID, std::string model_resource_view_name, pancy_physx  *physic_need, geometry_control *geometry_need, XMFLOAT3 position_need, float scaling_need);
	HRESULT create();
	void release();
	HRESULT init_data2pipeline();
	int get_num_geometry_view();
private:
	HRESULT init_physics();
	HRESULT init_geometry_bounding_box();
	void show_bounding_box(XMFLOAT4X4 view_proj_matrix);
};
class static_decorate_basic
{
	//传入单例
	pancy_physx  *physic_pancy;
	geometry_control *geometry_pancy;
	//识别属性
	std::string resource_view_name;
	int resource_map_ID;
	//基本几何属性
	float scaling;
	XMFLOAT3 position;
	//物理属性
	physx::PxRigidStatic *bound_box;
	physx::PxMaterial *mat_force;
public:
	static_decorate_basic(int map_ID, std::string model_resource_view_name, pancy_physx  *physic_need, geometry_control *geometry_need, XMFLOAT3 position_need, float scaling_need);
	HRESULT create();
	void release();
	HRESULT init_data2pipeline();
	int get_num_geometry_view();
private:
	HRESULT init_physics();
	HRESULT init_geometry_bounding_box();
	void show_bounding_box(XMFLOAT4X4 view_proj_matrix);
};
class pancy_world_map
{
	std::vector<std::string> resourcedataview_namelist;
	std::vector<std::string> decoratedataview_namelist;
	std::vector<AI_animal*> animaldataview_namelist;
	std::vector<AI_animal> animaldata_list;

	//地图属性
	float resource_range;//资源占地面积
	float decorate_range;
	//传入单例
	ID3D11DeviceContext *contex_pancy;     //设备描述表
	pancy_renderstate *renderstate_lib;
	geometry_control *geometry_pancy;
	pancy_physx  *physic_pancy;
	shader_control *shader_pancy;
	pancy_terrain_build *terrain_data;
	//装饰品访问表与资源访问表
	//std::vector<static_decorate_type_basic> data_decorate_list;
	//std::vector<static_resource_type_basic> data_resource_list;
	//地图资源实例
	std::map<int, static_decorate_basic> data_decorate_map;
	std::map<int, static_resource_basic> data_resource_map;
public:
	pancy_world_map(ID3D11DeviceContext *contex_need,pancy_renderstate *renderstate_need,geometry_control *geometry_need, pancy_physx  *physic_need, shader_control *shader_need, pancy_terrain_build *terrain_input, float resource_range_need, float decorate_range_need);
	HRESULT create();
	void release();
	void display(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix);
	void update(float delta_time, XMFLOAT3 now_pos_camera, XMFLOAT3 now_lool_camera, float angle_projection);
	HRESULT add_decorate_instance_byname(std::string geometryresourceview_name, int resource_map_ID, float scal_range);
	HRESULT add_resource_instance_byname(std::string geometryresourceview_name, int resource_map_ID, float scal_range);
	HRESULT add_resource_data_view_byname(std::string geometryresource_name, std::string resource_data_view_name);
	HRESULT add_decorate_data_view_byname(std::string geometryresource_name, std::string decorate_data_view_name);
	HRESULT add_animal_data_view_byAIdata(AI_animal* AI_animal_data);
	HRESULT add_animal_data_byname(XMFLOAT3 bound_box_range, std::string geometryresource_name,std::string geometryresourceview_name, int resource_map_ID, float scal_range, animal_attribute animal_data_need);
	HRESULT add_animal_data(AI_animal *animal_data_need);
	void clear_animal_list();
private:
	HRESULT init_decorate_by_view(XMFLOAT3 now_pos_camera, XMFLOAT3 now_lool_camera, float angle_projection,int dataID);
	HRESULT init_decoratedata_to_pipeline(int dataID);
	HRESULT init_resource_by_view(XMFLOAT3 now_pos_camera, XMFLOAT3 now_lool_camera, float angle_projection, int dataID);
	HRESULT init_resourcedata_to_pipeline(int dataID);
	HRESULT find_decoratedata_position_byID(int ID_find, XMFLOAT3 &position_out);
	HRESULT find_resource_position_byID(int ID_find, XMFLOAT3 &position_out);
	HRESULT find_decoratedata_ID_byposition(XMFLOAT3 position_in, int &ID_out);
	HRESULT find_resource_ID_byposition(XMFLOAT3 position_in, int &ID_out);
	void delete_animal_data_view_byname(std::string geometryresourceview_name);
	void get_view_cormer(float angle_view, XMFLOAT3 direction_view, XMFLOAT3 &direction_corner1, XMFLOAT3 &direction_corner2);
	bool check_if_in_view(float angle_view, XMFLOAT3 position_view, XMFLOAT3 direction_view, XMFLOAT3 position_test);
	bool check_if_in_range(XMFLOAT3 position_camera, XMFLOAT3 position_target, float distance_max);
};
class pancy_map_design
{
	std::vector<string> decorate_name_list;
	std::vector<string> resource_name_list;
	std::vector<string> animal_geometry_name_list;
	pancy_world_map *map_control;
public:
	pancy_map_design(pancy_world_map *map_control_need);
	void add_an_animal_type(string name_geometry_resource) { animal_geometry_name_list.push_back(name_geometry_resource); };
	HRESULT create();
	void release();
private:
	HRESULT load_map_fromfile();
	HRESULT build_random_map();
	HRESULT save_map_tofile();
};
//背包系统
class goods_member 
{
	GUI_control  *GUI_engine;
	ID3D11Device *device_pancy;
	string goods_name;
	XMFLOAT3 goods_position;
	int goods_number;
	int tex_ID;
	int pos_ID;
	ID3D11ShaderResourceView *goods_introduce_texture;
public:
	goods_member(GUI_control  *GUI_engine_need,ID3D11Device *device_need,string goods_name_need, int tex_ID_need);
	HRESULT create(wchar_t *introduce_texture);
	void add_a_goods();
	void delete_a_goods();
	int get_resource_num() { return goods_number; };
	void change_position(XMFLOAT3 goods_position_need);
	void change_pos_ID(int goods_position_ID);
	XMFLOAT3 get_goods_position() { return goods_position; };
	ID3D11ShaderResourceView *get_introduce_tex() { return goods_introduce_texture; };
	int      get_tex_id() { return tex_ID; }
	int      get_pos_id() { return pos_ID; }
	void release();
};
class package_design
{
	ID3D11Device *device_pancy;
	GUI_control  *GUI_engine;
	pancy_input  *input_engine;
	bool if_package_open;
	int bag_width;
	int bag_height;
	wchar_t *package_tex_name;
	wchar_t *goods_tex_name;
	wchar_t *number_tex_name;
	int row_length_UI;
	std::map<string,goods_member>  goods_list;
	int goods_render_num;
	XMFLOAT4 position_list[100];
	XMFLOAT4 numpos_list[300];
	XMFLOAT3 packet_position_start;
	XMFLOAT3 packet_position_offset;
	bool if_click;
	bool if_show_introduce;
	string goods_introduce;
	string goods_click;
	//XMFLOAT3 tex_position_list[100];
public:
	package_design(GUI_control  *GUI_engine_need, ID3D11Device *device_need,wchar_t *package_texture_name,int UI_num_per_row,wchar_t *goodstex_name_need, wchar_t *number_tex_name_need);
	HRESULT create();
	HRESULT add_a_goods_type(int tex_id,string goods_name, wchar_t *tex_introduce);
	void add_goodsnum_byname(string goods_name);
	void delete_goodsnum_byname(string goods_name);
	void change_goods_position_byname(string goods_name, XMFLOAT3 position_input);
	void change_goods_position_byname(string goods_name, int position_bag);
	void update(float delta_time);
	void display();
	void release();
	void open_package() { if_package_open = true; };
	void close_package() { if_package_open = false; };
	bool check_if_open() { return if_package_open; };
private:
	void get_pos();
	string get_now_mouse_goods();
	string get_now_pos_goods(int pos_in);
	bool check_mouse_on_instancing(XMFLOAT3 position_bag);
	int count_num_byposition(XMFLOAT4 position_in);
};


class scene_engine_physicx : public scene_root
{
	pancy_world_map *world_map_main;
	player_basic *player_main;
	pancy_physx *physics_pancy;
	pancy_terrain_build *terrain_test;
	pancy_map_design *map_designer;
	float camera_height;
	package_design *player_package;
	bool if_click_bag;
	//GUI_button *test_start;
	//GUI_mouse *mouse_basic;
	//GUI_progressbar *prograss_test;

	GUI_control *gui_list;
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
	void show_grass();
};
