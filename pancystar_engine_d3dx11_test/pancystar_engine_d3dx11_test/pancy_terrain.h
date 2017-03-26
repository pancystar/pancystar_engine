#pragma once
#include<windows.h>
#include<string.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<directxmath.h>
#include<d3dx11effect.h>
#include<DDSTextureLoader.h>
#include<map>
#include"shader_pancy.h"
#include"geometry.h"
#include"pancy_physx.h"
//#include"pancy_model_import.h"
struct pancy_height_data 
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
};
class pancy_terrain_build
{
	/*
	map_first_level_width = 2;
	map_second_level_width = 2;
	共四张一级地图(四张高度图)
	每个一级地图分解成四个二级地图
	——————————————
	|      |      |      |       |
	|      |      |      |       |
	|—————— | —————— |
	|      |      |      |       |
	|      |      |      |       |
	——————————————
	|      |      |      |       |
	|      |      |      |       |
	|—————— | —————— |
	|      |      |      |       |
	|      |      |      |       |
	——————————————
	*/
	ID3D11Device             *device_pancy;           //d3d设备
	ID3D11DeviceContext      *contex_pancy;           //设备描述表
	pancy_physx              *physic_pancy;           //物理引擎
	shader_control           *shader_list;
	pancy_height_data        *height_check_data;      //高度检验数据
	ID3D11ShaderResourceView *height_check_buffer;    //高度检验buffer
	int                      map_first_level_width;   //一级地图的数量(一张高度图代表一个一级地图)
	int                      map_second_level_width;  //二级地图的数量(即每个一级地图分解成几个二级地图进行细分)
	//地图实际宽度信息
	float                    height_terrain_scal;
	int                      depth_map_scal;          //高度图的平平滑等级
	int                      sample_distance;         //高度图的采样距离
	float                    map_width_physics;       //一级地图的物理宽度(地图大小)
	int                      map_sample_width;        //高度纹理的像素宽度
	int                      map_sample_height;       //高度纹理的像素高度
	//地图的采样信息
	ID3D11ShaderResourceView *diffuse_map_atrray;     //漫反射纹理数组
	ID3D11ShaderResourceView *height_map_atrray;      //高度图纹理数组
	ID3D11Buffer             *square_map_point;       //地图顶点缓冲区
	std::vector<LPCWSTR>     height_map_file_name;
	std::vector<LPCWSTR>     diffuse_map_file_name;
	physx::PxMaterial        *terrain_mat_force;
	physx::PxHeightFieldSample* samples;
	int numRows;
	int numCols;
public:
	pancy_terrain_build(pancy_physx* physic_need,ID3D11Device *device_need, ID3D11DeviceContext *contex_need, shader_control *shader_list, int map_number, int map_detail, float map_range, std::vector<LPCWSTR> height_map, std::vector<LPCWSTR> diffuse_map);
	HRESULT create();
	void show_terrain(XMFLOAT4X4 viewproj_mat);
	ID3D11ShaderResourceView *get_heightmap() { return height_map_atrray; };
	ID3D11ShaderResourceView *get_diffusemap() { return diffuse_map_atrray; };
	void show_terrainshape(ID3DX11EffectTechnique* tech);
	HRESULT get_position_ID(int input_ID,float offset_range,XMFLOAT3 &map_position);
	HRESULT get_ID_position(XMFLOAT3 input_position, float offset_range,int &map_ID);
	void release();
private:
	HRESULT build_buffer();
	HRESULT build_texture();
	HRESULT build_physics();
	XMFLOAT3 vector_plus(XMFLOAT3 v1, XMFLOAT3 v2) { return XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); };
	XMFLOAT3 vector_minus(XMFLOAT3 v1, XMFLOAT3 v2) { return XMFLOAT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); };
	XMFLOAT3 vector_mul_num(XMFLOAT3 v1, float num) { return XMFLOAT3(v1.x*num, v1.y *num, v1.z *num); };
};
