#pragma once
#include<D3D11.h>
#include<assert.h>
#include<d3dx11effect.h>
#include<directxmath.h>
#include<string.h>
#include<stdlib.h>
#include<Dinput.h>
#include<iostream>
using namespace DirectX;

#define CameraForFPS 0
#define CameraForFly 1
class pancy_camera
{
    int window_weight;
    int window_height;
	ID3D11Device *device_d3d; //确定待变换的d3d设备接口
	XMFLOAT3 camera_right;    //摄像机的向右方向向量
	XMFLOAT3 camera_look;     //摄像机的观察方向向量
	XMFLOAT3 camera_up;       //摄像机的向上方向向量
	XMFLOAT3 camera_position; //摄像机的所在位置向量
public:
	pancy_camera(ID3D11Device *device_need,int weight,int height);
	void rotation_right(float angle);                    //沿着向右向量旋转
	void rotation_up(float angle);                       //沿着向上向量旋转
	void rotation_look(float angle);                     //沿着观察向量旋转
	void walk_front(float distance);                     //摄像机向前平移
	void walk_right(float distance);                     //摄像机向右平移
	void walk_up(float distance);                        //摄像机向上平移
	void count_view_matrix(XMMATRIX* view_matrix);       //计算取景矩阵
	void pancy_camera::count_view_matrix(XMFLOAT3 rec_look, XMFLOAT3 rec_up, XMFLOAT3 rec_right, XMFLOAT4X4 *matrix);
	void get_view_position(XMFLOAT3 *view_pos);
	void get_view_direct(XMFLOAT3 *view_direct);
private:
    

};