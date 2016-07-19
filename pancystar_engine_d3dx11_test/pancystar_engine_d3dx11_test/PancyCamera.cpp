﻿#include"PancyCamera.h"
pancy_camera::pancy_camera(ID3D11Device *device_need,int weight,int height)
{
	window_weight = weight;
	window_height = height;
	device_d3d = device_need;
	camera_position   = XMFLOAT3(0.f, 2.f, -5.f);
	camera_right      = XMFLOAT3(1.0f, 0.0f, 0.0f);
	camera_up         = XMFLOAT3(0.0f, 1.0f, 0.0f);
	camera_look       = XMFLOAT3(0.0f, 0.0f, 1.0f);
}
void pancy_camera::count_view_matrix(XMMATRIX* view_matrix)
{
	XMVECTOR rec_camera_look;
	XMVECTOR rec_camera_up;
	XMVECTOR rec_camera_right;
	XMVECTOR rec_camera_position;
	//格式转换
	rec_camera_look = XMLoadFloat3(&camera_look);
	rec_camera_up = XMLoadFloat3(&camera_up);
	rec_camera_right = XMLoadFloat3(&camera_right);
	rec_camera_position = XMLoadFloat3(&camera_position);
	//求叉积
	rec_camera_right = XMVector3Normalize(XMVector3Cross(rec_camera_up, rec_camera_look));
	rec_camera_up = XMVector3Normalize(XMVector3Cross(rec_camera_look, rec_camera_right));
	rec_camera_look = XMVector3Normalize(rec_camera_look);
	XMVECTOR rec_ans;
	float x = -XMVectorGetX(XMVector3Dot(rec_camera_right, rec_camera_position));
	float y = -XMVectorGetX(XMVector3Dot(rec_camera_up,rec_camera_position));
	float z = -XMVectorGetX(XMVector3Dot(rec_camera_look,rec_camera_position));
	/*
	填充观察矩阵
	V = 

	Rx     Ux     Dx     0
	Ry     Uy     Dy     0
	Rz     Uz     Dz     0
	-p▪R   -p▪R   -p▪R    1

	*/
	view_matrix->r[0].m128_f32[0] = camera_right.x;
	view_matrix->r[0].m128_f32[1] = camera_up.x;
	view_matrix->r[0].m128_f32[2] = camera_look.x;
	view_matrix->r[0].m128_f32[3] = 0.0f;
	
	view_matrix->r[1].m128_f32[0] = camera_right.y;
	view_matrix->r[1].m128_f32[1] = camera_up.y;
	view_matrix->r[1].m128_f32[2] = camera_look.y;
	view_matrix->r[1].m128_f32[3] = 0.0f;

	view_matrix->r[2].m128_f32[0] = camera_right.z;
	view_matrix->r[2].m128_f32[1] = camera_up.z;
	view_matrix->r[2].m128_f32[2] = camera_look.z;
	view_matrix->r[2].m128_f32[3] = 0.0f;

	view_matrix->r[3].m128_f32[0] = x;
	view_matrix->r[3].m128_f32[1] = y;
	view_matrix->r[3].m128_f32[2] = z;
	view_matrix->r[3].m128_f32[3] = 1.0f;
}
void pancy_camera::rotation_right(float angle)
{
	XMVECTOR rec_camera_look;
	XMVECTOR rec_camera_up;
	XMVECTOR rec_camera_right;
	XMMATRIX trans_rotation;
	rec_camera_look = XMLoadFloat3(&camera_look);
	rec_camera_up = XMLoadFloat3(&camera_up);
	rec_camera_right = XMLoadFloat3(&camera_right);
	trans_rotation = XMMatrixRotationAxis(rec_camera_right, angle);//获取旋转矩阵
	rec_camera_up = XMVector3TransformCoord(rec_camera_up, trans_rotation);//更变摄像机的向上向量
	rec_camera_look = XMVector3TransformCoord(rec_camera_look, trans_rotation);//更变摄像机的向右向量
	XMStoreFloat3(&camera_look, rec_camera_look);
	XMStoreFloat3(&camera_up, rec_camera_up);
	XMStoreFloat3(&camera_right, rec_camera_right);
}
void pancy_camera::rotation_up(float angle)
{
	XMVECTOR rec_camera_look;
	XMVECTOR rec_camera_up;
	XMVECTOR rec_camera_right;
	XMMATRIX trans_rotation;
	rec_camera_look = XMLoadFloat3(&camera_look);
	rec_camera_up = XMLoadFloat3(&camera_up);
	rec_camera_right = XMLoadFloat3(&camera_right);
	trans_rotation = XMMatrixRotationAxis(rec_camera_up, angle);//获取旋转矩阵
	rec_camera_right = XMVector3TransformCoord(rec_camera_right, trans_rotation);//更变摄像机的向右向量
	rec_camera_look = XMVector3TransformCoord(rec_camera_look, trans_rotation);//更变摄像机的观察向量
	XMStoreFloat3(&camera_look, rec_camera_look);
	XMStoreFloat3(&camera_up, rec_camera_up);
	XMStoreFloat3(&camera_right, rec_camera_right);
}
void pancy_camera::rotation_look(float angle)
{
	XMVECTOR rec_camera_look;
	XMVECTOR rec_camera_up;
	XMVECTOR rec_camera_right;
	XMMATRIX trans_rotation;
	rec_camera_look = XMLoadFloat3(&camera_look);
	rec_camera_up = XMLoadFloat3(&camera_up);
	rec_camera_right = XMLoadFloat3(&camera_right);
	trans_rotation = XMMatrixRotationAxis(rec_camera_look, angle);//获取旋转矩阵
	rec_camera_up = XMVector3TransformCoord(rec_camera_up, trans_rotation);//更变摄像机的向上向量
	rec_camera_right = XMVector3TransformCoord(rec_camera_right, trans_rotation);//更变摄像机的向右向量
	XMStoreFloat3(&camera_look, rec_camera_look);
	XMStoreFloat3(&camera_up, rec_camera_up);
	XMStoreFloat3(&camera_right, rec_camera_right);
}
void pancy_camera::walk_front(float distance)
{
	camera_position.x += camera_look.x *distance;
	camera_position.y += camera_look.y *distance;
	camera_position.z += camera_look.z *distance;
}
void pancy_camera::walk_right(float distance)
{
	camera_position.x += camera_right.x *distance;
	camera_position.y += camera_right.y *distance;
	camera_position.z += camera_right.z *distance;
}
void pancy_camera::walk_up(float distance)
{
	camera_position.x += camera_up.x *distance;
	camera_position.y += camera_up.y *distance;
	camera_position.z += camera_up.z *distance;
}
void pancy_camera::get_view_position(XMFLOAT3 *view_pos) 
{
	*view_pos = camera_position;
}
void pancy_camera::get_view_direct(XMFLOAT3 *view_direct)
{
	*view_direct = camera_look;
}