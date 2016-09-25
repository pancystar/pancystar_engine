#include<math.h>
#include"geometry.h"

//心形
mesh_heart::mesh_heart(ID3D11Device *device_need,ID3D11DeviceContext *contex_need,int circle_num_need,int vertex_percircle_need):Geometry(device_need,contex_need)
{
	heart_divide = circle_num_need;
	line_percircle = vertex_percircle_need;
	pancy_point *vertex_need = new pancy_point[circle_num_need*vertex_percircle_need + 100];
	UINT   *index_need = new UINT[circle_num_need*vertex_percircle_need * 6 + 100];  //索引数据
	find_point(vertex_need, index_need,all_vertex,all_index);
	init_point(vertex_need, index_need);
	delete[] vertex_need;
}
double mesh_heart::find_f(double a,double b,double z)//获得f(x,y,z),其中x,y包含在a,b中，z作为输入
{
	return (a + z*z)*(a + z*z)*(a + z*z) - b*z*z*z;
}
double mesh_heart::find_f1(double a,double b,double z)//获得f'(x,y,z),其中x,y包含在a,b中，z作为输入
{
	return 6*z*(z*z + a)*(z*z + a) - 3*b*z*z;
}
double mesh_heart::get_z(double x,double y,double ans_start,double balance_check)//根据给定的x,y以及迭代初值求z值
{
	double a;
	double b;
	a = x*x + (9.0/4.0)*y*y -1;
	b = x*x + (9.0/80.0)*y*y;
	for(int i = 0; i < 200; ++i)//牛顿迭代
	{
		ans_start = ans_start - balance_check*find_f(a,b,ans_start) / find_f1(a,b,ans_start);
	}
	double check = find_f(a,b,ans_start);
	if(check > 0.000000001 || check < -0.000000001)
	{
		return -999.0;//未找到逼近解，返回失败
	}
	else
	{
		return ans_start;//返回找到的解
	}
}
double mesh_heart::find_st(double x,double st,double ed,int type,bool if_ans,double ans_check)//二分锁定函数的定义域
{
	double mid = st + (ed - st) / 2;
	if(type > 80)
	{
		if(if_ans)
		{
			return ans_check;
		}
		else
		{
			return -999;//未找到起始点
		}
	}
	double z_rec1 = get_z(x,mid,-2.0,1.0);
	double z_rec2 = get_z(x,mid,2.0,1.0);
	int check_ifst = 0;
	if(z_rec1 != -999)
	{
		check_ifst += 1;
	}
	if(z_rec2 != -999)
	{
		check_ifst += 1;
	}
	if(check_ifst == 2)
	{
		ans_check = mid;
		return find_st(x,st,mid,type+1,true,ans_check);
	}
	if(check_ifst == 0 || check_ifst == 1)
	{
		return find_st(x,mid,ed,type+1,if_ans,ans_check);
	}
	return 0;
}
bool mesh_heart::check_range(double x,double &st,double &ed)//检测函数的定义域
{	
	double y = find_st(x,-1.5,0,0,false,0);
	st = y;
	ed = -y;
	if(st == -999)
	{
		return false;
	}
	return true;
}
XMFLOAT3 mesh_heart::count_normal(double x,double y,double z)
{
	XMFLOAT3 vec_normal;
	vec_normal.x = -static_cast<float>(((27*y*(x*x + (9*y*y)/4 + z*z - 1)*(x*x + (9*y*y)/4 + z*z - 1))/2 - (9*y*z*z*z)/40)/(3*x*x*z*z + (27*y*y*z*z)/80 - 6*z*(x*x + (9*y*y)/4 + z*z - 1)*(x*x + (9*y*y)/4 + z*z - 1)));
	vec_normal.y = static_cast<float>(1.0);
	vec_normal.z = -static_cast<float>((6*x*(x*x + (9*y*y)/4 + z*z - 1)*(x*x + (9*y*y)/4 + z*z - 1) - 2*x*z*z*z)/(3*x*x*z*z + (27*y*y*z*z)/80 - 6*z*(x*x + (9*y*y)/4 + z*z - 1)*(x*x + (9*y*y)/4 + z*z - 1)));
	XMVECTOR N = XMVectorSet(vec_normal.x,vec_normal.y,vec_normal.z,0.f);
	XMStoreFloat3(&vec_normal,XMVector3Normalize(N));
	return vec_normal;
}
bool mesh_heart::check_normal(XMFLOAT3 vec_normal,XMFLOAT3 vec_pos)
{
	float check = vec_normal.x * vec_pos.x + vec_normal.y * vec_pos.y + vec_normal.z * vec_pos.z;
	if(check > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
HRESULT mesh_heart::find_point(pancy_point *vertex,UINT *index,int &num_vertex,int &num_index)
{
	num_vertex = 0;
	num_index  = 0;
	double st_x = -1.2,ed_x = 1.2;
	line_percircle += 1;
	//填充顶点
	for(int i = 0; i < heart_divide; ++i)
	{
		double st_y,ed_y;
		double x_rec = st_x + ((ed_x - st_x)/static_cast<double>(heart_divide)) * double(i);
		if(check_range(x_rec,st_y,ed_y))
		{
			for(int j = 0; j < line_percircle; j++)
			{
				double y_rec = st_y + ((ed_y-st_y)/static_cast<double>(line_percircle-1)) * (double)j;		
				if(j == line_percircle -1)
				{
					y_rec = ed_y;
				}
				double z_rec1;
				double z_rec2 = get_z(x_rec,y_rec,2.0,1.0);
				double balance_check = 1.0;
				for(int k = 0; k < 10; ++k)
				{
					balance_check =1.0 - k*0.1;
					z_rec1 = get_z(x_rec,y_rec,-2.0,balance_check);
					if(z_rec1 - z_rec2 < -0.00000001 || z_rec1 - z_rec2 > 0.00000001)
					{
						break;
					}
				}
				int st_now = num_vertex+j;
				int ed_now = num_vertex+2*line_percircle-1-j;
				vertex[st_now].position.x = static_cast<float>(y_rec);
				vertex[st_now].position.y = static_cast<float>(z_rec1);
				vertex[st_now].position.z = static_cast<float>(x_rec);
				vertex[st_now].normal = count_normal(x_rec,y_rec,z_rec1);
				if(!check_normal(vertex[st_now].normal,vertex[st_now].position))
				{
					vertex[st_now].normal.x = -vertex[st_now].normal.x;
					vertex[st_now].normal.y = -vertex[st_now].normal.y;
					vertex[st_now].normal.z = -vertex[st_now].normal.z;
				}
				vertex[st_now].tex.x = 0.0f;
				vertex[st_now].tex.y = 0.0f;
				vertex[ed_now].position.x = static_cast<float>(y_rec);
				vertex[ed_now].position.y = static_cast<float>(z_rec2);
				vertex[ed_now].position.z = static_cast<float>(x_rec);
				vertex[ed_now].normal = count_normal(x_rec,y_rec,z_rec2);
				if(!check_normal(vertex[ed_now].normal,vertex[ed_now].position))
				{
					vertex[ed_now].normal.x = -vertex[ed_now].normal.x;
					vertex[ed_now].normal.y = -vertex[ed_now].normal.y;
					vertex[ed_now].normal.z = -vertex[ed_now].normal.z;
				}
				vertex[ed_now].tex.x = 0.0f;
				vertex[ed_now].tex.y = 0.0f;
			}
			num_vertex += 2*line_percircle;
		}
		else
		{
			heart_divide -= 1;
		}
	}
	//填充索引
	for(int i=0; i < heart_divide-1; ++i)  
	{  
		for(int j=0; j < line_percircle*2; ++j)  
		{  
			if(j == line_percircle*2-1)
			{
				index[num_index++] = i * line_percircle*2 + j;  
				index[num_index++] = (i + 1) * line_percircle*2;  
				index[num_index++] = (i + 1) * line_percircle*2 + j;  
				index[num_index++] = i * line_percircle*2 + j;  
				index[num_index++] = i * line_percircle*2;  
				index[num_index++] = (i + 1) * line_percircle*2; 
			}
			else
			{
				index[num_index++] = i * line_percircle*2 + j;  
				index[num_index++] = (i + 1) * line_percircle*2 + j + 1;  
				index[num_index++] = (i + 1) * line_percircle*2 + j;  
				index[num_index++] = i * line_percircle*2 + j;  
				index[num_index++] = i * line_percircle*2 + j + 1;  
				index[num_index++] = (i + 1) * line_percircle*2 + j + 1; 
			} 
		}
	}
	vertex[num_vertex].normal = XMFLOAT3(0.0f,0.0f,-1.0f);
	vertex[num_vertex++].position = XMFLOAT3(0.0f,0.54f,1.13f);
	//加上前后盖信息
	for(int j=0; j < line_percircle*2; ++j)
	{
		if(j == line_percircle*2-1)
		{
			index[num_index++] = num_vertex - 1;
			index[num_index++] = num_vertex - line_percircle*2 + j-1;
			index[num_index++] = num_vertex - line_percircle*2-1;
		}
		else
		{
			index[num_index++] = num_vertex - 1;
			index[num_index++] = num_vertex - line_percircle*2 + j-1;
			index[num_index++] = num_vertex - line_percircle*2 + j;
		}
	}
	vertex[num_vertex].normal = XMFLOAT3(0.0f,0.0f,1.0f);
	vertex[num_vertex++].position = XMFLOAT3(0.0f,0.54f,-1.13f);
	for(int j=0; j < line_percircle*2; ++j)
	{
		if(j == line_percircle*2-1)
		{
			index[num_index++] = num_vertex-1;
			index[num_index++] = 0;
			index[num_index++] = j;
		}
		else
		{
			index[num_index++] = num_vertex-1;
			index[num_index++] = j + 1;
			index[num_index++] = j;
		}
	}
	return S_OK;
}
//球形
mesh_ball::mesh_ball(ID3D11Device *device_need,ID3D11DeviceContext *contex_need,int circle_num_need,int vertex_percircle_need):Geometry(device_need,contex_need)
{
	circle_num = circle_num_need;
	vertex_percircle = vertex_percircle_need;
	all_vertex = circle_num_need*vertex_percircle_need;
	/*
	point_with_tangent *vertex_need = new point_with_tangent[circle_num_need*vertex_percircle_need + 100];
	UINT   *index_need = new UINT[circle_num_need*vertex_percircle_need*6 + 100];  //索引数据
	find_point(vertex_need, index_need,all_vertex,all_index);
	init_point(vertex_need, index_need);
	delete[] vertex_need;
	delete[] index_need;
	*/
}
HRESULT mesh_ball::find_point(point_with_tangent *vertex,UINT *index,int &num_vertex,int &num_index)
{
	num_vertex = 0;
	num_index  = 0;
	float radius = 1.0;
	//填充顶点的位置信息
	for(int i = 0; i < circle_num; ++i)
	{ 
		float phy = XM_PI * static_cast<float>(i+1) / static_cast<float>(circle_num+1); //球面角
		float tmpRadius = radius * sin(phy);//球面每个平面所截的圆半径
		for(int j = 0; j < vertex_percircle; ++j)
		{
			float theta = 2*XM_PI * static_cast<float>(j) / static_cast<float>(vertex_percircle);//圆面角  

			float x = tmpRadius*cos(theta);  
			float y = radius*cos(phy);  
			float z = tmpRadius*sin(theta);  
			//位置坐标
			vertex[num_vertex].position = XMFLOAT3(x,y,z);
			XMVECTOR N = XMVectorSet(x,y,z,0.f);
			XMStoreFloat3(&vertex[num_vertex++].normal,XMVector3Normalize(N));
		}
	}
	//填充顶点的索引信息
	for(int i=0; i < circle_num-1; ++i)  
	{  
		for(int j=0; j < vertex_percircle; ++j)  
		{  
			if(j == vertex_percircle-1)
			{
				index[num_index++] = i * vertex_percircle + j;  
				index[num_index++] = (i + 1) * vertex_percircle;  
				index[num_index++] = (i + 1) * vertex_percircle + j;  
				index[num_index++] = i * vertex_percircle + j;  
				index[num_index++] = i * vertex_percircle;  
				index[num_index++] = (i + 1) * vertex_percircle; 
			}
			else
			{
				index[num_index++] = i * vertex_percircle + j;  
				index[num_index++] = (i + 1) * vertex_percircle + j + 1;  
				index[num_index++] = (i + 1) * vertex_percircle + j;  
				index[num_index++] = i * vertex_percircle + j;  
				index[num_index++] = i * vertex_percircle + j + 1;  
				index[num_index++] = (i + 1) * vertex_percircle + j + 1; 
			} 
		}
	}
	//加上上下盖信息
	vertex[num_vertex].position = XMFLOAT3(0,radius,0);
	vertex[num_vertex++].normal = XMFLOAT3(0,1,0);
	for(int j=0; j < vertex_percircle; ++j)
	{
		if(j == vertex_percircle-1)
		{
			index[num_index++] = num_vertex-1;
			index[num_index++] = 0;
			index[num_index++] = j;
		}
		else
		{
			index[num_index++] = num_vertex-1;
			index[num_index++] = j + 1;
			index[num_index++] = j;
		}
	}
	vertex[num_vertex].position = XMFLOAT3(0,-radius,0);
	vertex[num_vertex++].normal = XMFLOAT3(0,-1,0);
	for(int j=0; j < vertex_percircle; ++j)
	{
		if(j == vertex_percircle-1)
		{
			index[num_index++] = num_vertex-1;
			index[num_index++] = (circle_num-1) * vertex_percircle + j;
			index[num_index++] = (circle_num-1) * vertex_percircle;
		}
		else
		{
			index[num_index++] = num_vertex-1;
			index[num_index++] = (circle_num-1) * vertex_percircle + j;
			index[num_index++] = (circle_num-1) * vertex_percircle + j + 1;
		}
	}
	return S_OK;
}
//立方体
mesh_cube::mesh_cube(ID3D11Device *device_need,ID3D11DeviceContext *contex_need):Geometry(device_need,contex_need)
{
	pancy_point *vertex_need = new pancy_point[100];
	UINT   *index_need = new UINT[100];  //索引数据
	find_point(vertex_need, index_need,all_vertex,all_index);
	init_point(vertex_need, index_need);
	delete[] vertex_need;
	delete[] index_need;
}
HRESULT mesh_cube::find_point(pancy_point *vertex,UINT *index,int &num_vertex,int &num_index)
{
	pancy_point square_test[]=
	{
		{XMFLOAT3(-1.0f,1.0f,-1.0f), XMFLOAT3(-1.0f,1.0f,-1.0f), XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(1.0f,1.0f,-1.0f),  XMFLOAT3(1.0f,1.0f,-1.0f),  XMFLOAT2(1.0f,0.0f)},
		{XMFLOAT3(1.0f,-1.0f,-1.0f), XMFLOAT3(1.0f,-1.0f,-1.0f), XMFLOAT2(1.0f,1.0f)},
		{XMFLOAT3(-1.0f,-1.0f,-1.0f),XMFLOAT3(-1.0f,-1.0f,-1.0f),XMFLOAT2(0.0f,1.0f)},
		{XMFLOAT3(1.0f,1.0f,1.0f),   XMFLOAT3(1.0f,1.0f,1.0f),   XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(-1.0f,1.0f,1.0f),  XMFLOAT3(-1.0f,1.0f,1.0f),  XMFLOAT2(1.0f,0.0f)},
		{XMFLOAT3(-1.0f,-1.0f,1.0f), XMFLOAT3(-1.0f,-1.0f,1.0f), XMFLOAT2(1.0f,1.0f)},
		{XMFLOAT3(1.0f,-1.0f,1.0f),  XMFLOAT3(1.0f,-1.0f,1.0f),  XMFLOAT2(0.0f,1.0f)},
		{XMFLOAT3(-1.0f,1.0f,1.0f),  XMFLOAT3(-1.0f,1.0f,1.0f),  XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(1.0f,1.0f,1.0f),   XMFLOAT3(1.0f,1.0f,1.0f),   XMFLOAT2(1.0f,0.0f)},
		{XMFLOAT3(1.0f,1.0f,-1.0f),  XMFLOAT3(1.0f,1.0f,-1.0f),  XMFLOAT2(1.0f,1.0f)},
		{XMFLOAT3(-1.0f,1.0f,-1.0f), XMFLOAT3(-1.0f,1.0f,-1.0f), XMFLOAT2(0.0f,1.0f)},
		{XMFLOAT3(-1.0f,-1.0f,-1.0f),XMFLOAT3(-1.0f,-1.0f,-1.0f),XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(1.0f,-1.0f,-1.0f), XMFLOAT3(1.0f,-1.0f,-1.0f), XMFLOAT2(1.0f,0.0f)},
		{XMFLOAT3(1.0f,-1.0f,1.0f),  XMFLOAT3(1.0f,-1.0f,1.0f),  XMFLOAT2(1.0f,1.0f)},
		{XMFLOAT3(-1.0f,-1.0f,1.0f), XMFLOAT3(-1.0f,-1.0f,1.0f), XMFLOAT2(0.0f,1.0f)},
		{XMFLOAT3(-1.0f,1.0f,1.0f),  XMFLOAT3(-1.0f,1.0f,1.0f),  XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(-1.0f,1.0f,-1.0f), XMFLOAT3(-1.0f,1.0f,-1.0f), XMFLOAT2(1.0f,0.0f)},
		{XMFLOAT3(-1.0f,-1.0f,-1.0f),XMFLOAT3(-1.0f,-1.0f,-1.0f),XMFLOAT2(1.0f,1.0f)},
		{XMFLOAT3(-1.0f,-1.0f,1.0f), XMFLOAT3(-1.0f,-1.0f,1.0f), XMFLOAT2(0.0f,1.0f)},
		{XMFLOAT3(1.0f,1.0f,-1.0f),  XMFLOAT3(1.0f,1.0f,-1.0f),  XMFLOAT2(0.0f,0.0f)},
		{XMFLOAT3(1.0f,1.0f,1.0f),   XMFLOAT3(1.0f,1.0f,1.0f),   XMFLOAT2(1.0f,0.0f)},
		{XMFLOAT3(1.0f,-1.0f,1.0f),  XMFLOAT3(1.0f,-1.0f,1.0f),  XMFLOAT2(1.0f,1.0f)},
		{XMFLOAT3(1.0f,-1.0f,-1.0f), XMFLOAT3(1.0f,-1.0f,-1.0f), XMFLOAT2(0.0f,1.0f)},
	};
	//创建索引数组。
	num_vertex = sizeof(square_test) / sizeof(pancy_point);
	for(int i = 0; i < num_vertex; ++i)
	{
		vertex[i] = square_test[i];
	}
	UINT indices[] = {0,1,2, 0,2,3, 4,5,6, 4,6,7, 8,9,10, 8,10,11, 12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23};
	num_index = sizeof(indices) / sizeof(UINT);
	for(int i = 0; i < num_index; ++i)
	{
		index[i] = indices[i];
	}
	return S_OK;
}
//立方体(含切线)
mesh_cubewithtargent::mesh_cubewithtargent(ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :Geometry(device_need, contex_need)
{
	all_vertex = 30;
	/*
	point_with_tangent ball_vertex1[100];
	UINT   *index_need = new UINT[100];  //索引数据
	
	find_point(ball_vertex1, index_need, all_vertex, all_index);
	init_point(ball_vertex1, index_need);
	*/
}
HRESULT mesh_cubewithtargent::find_point(point_with_tangent *vertex, UINT *index, int &num_vertex, int &num_index)
{
	point_with_tangent square_test[] =
	{
		{ XMFLOAT3(-1.0, -1.0, -1.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(0.0, 1.0) },
		{ XMFLOAT3(-1.0, 1.0, -1.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(0.0, 0.0) },
		{ XMFLOAT3(1.0, 1.0, -1.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(1.0, 0.0) },
		{ XMFLOAT3(1.0, -1.0, -1.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(1.0, 1.0) },
		{ XMFLOAT3(-1.0, -1.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT2(0.0, 1.0) },
		{ XMFLOAT3(-1.0, 1.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT2(0.0, 0.0) },
		{ XMFLOAT3(-1.0, 1.0, -1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT2(1.0, 0.0) },
		{ XMFLOAT3(-1.0, -1.0, -1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, -1.0), XMFLOAT2(1.0, 1.0) },
		{ XMFLOAT3(1.0, -1.0, 1.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT2(0.0, 1.0) },
		{ XMFLOAT3(1.0, 1.0, 1.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT2(0.0, 0.0) },
		{ XMFLOAT3(-1.0, 1.0, 1.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT2(1.0, 0.0) },
		{ XMFLOAT3(-1.0, -1.0, 1.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT3(-1.0, 0.0, 0.0), XMFLOAT2(1.0, 1.0) },
		{ XMFLOAT3(1.0, -1.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT2(0.0, 1.0) },
		{ XMFLOAT3(1.0, 1.0, -1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT2(0.0, 0.0) },
		{ XMFLOAT3(1.0, 1.0, 1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT2(1.0, 0.0) },
		{ XMFLOAT3(1.0, -1.0, 1.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT3(0.0, 0.0, 1.0), XMFLOAT2(1.0, 1.0) },
		{ XMFLOAT3(-1.0, 1.0, -1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(0.0, 1.0) },
		{ XMFLOAT3(-1.0, 1.0, 1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(0.0, 0.0) },
		{ XMFLOAT3(1.0, 1.0, 1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(1.0, 0.0) },
		{ XMFLOAT3(1.0, 1.0, -1.0), XMFLOAT3(0.0, 1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(1.0, 1.0) },
		{ XMFLOAT3(-1.0, -1.0, 1.0), XMFLOAT3(0.0, -1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(0.0, 1.0) },
		{ XMFLOAT3(-1.0, -1.0, -1.0), XMFLOAT3(0.0, -1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(0.0, 0.0) },
		{ XMFLOAT3(1.0, -1.0, -1.0), XMFLOAT3(0.0, -1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(1.0, 0.0) },
		{ XMFLOAT3(1.0, -1.0, 1.0), XMFLOAT3(0.0, -1.0, 0.0), XMFLOAT3(1.0, 0.0, 0.0), XMFLOAT2(1.0, 1.0) }
	};
	//创建索引数组。
	num_vertex = sizeof(square_test) / sizeof(point_with_tangent);
	for (int i = 0; i < num_vertex; ++i)
	{
		vertex[i] = square_test[i];
	}
	UINT indices[] = { 0,1,2, 0,2,3, 4,5,6, 4,6,7, 8,9,10, 8,10,11, 12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23 };
	num_index = sizeof(indices) / sizeof(UINT);
	for (int i = 0; i < num_index; ++i)
	{
		index[i] = indices[i];
	}
	return S_OK;
}
//山峰
mesh_mountain::mesh_mountain(ID3D11Device *device_need,ID3D11DeviceContext *contex_need,int width_need,int height_need):Geometry(device_need,contex_need)
{
	width_rec = width_need;
	height_rec = height_need;
	all_vertex = width_need*height_need;
	/*
	pancy_point *vertex_need = new pancy_point[width_need*height_need + 100];
	UINT   *index_need = new UINT[width_need*height_need*6 + 100];  //索引数据
	find_point(vertex_need, index_need,all_vertex,all_index);
	init_point(vertex_need,index_need);
	delete[] vertex_need;
	*/
}
HRESULT mesh_mountain::find_point(pancy_point *vertex,UINT *index,int &num_vertex,int &num_index)
{
	float st_x = -20,ed_x = 20;
	float st_z = -20,ed_z = 20;
	num_vertex = 0;
	num_index = 0;
	//填充顶点的位置信息
	for(int i = 0; i < width_rec; ++i)
	{
		for(int j = 0; j < height_rec; ++j)
		{
			vertex[num_vertex].position.x = st_x + static_cast<float>(i)/width_rec * (ed_x - st_x);
			vertex[num_vertex].position.z = st_z + static_cast<float>(j)/height_rec * (ed_z - st_z);
			vertex[num_vertex].position.y = 0.3f*vertex[num_vertex].position.z*sin(vertex[num_vertex].position.x) +  0.3f*vertex[num_vertex].position.x*cos(vertex[num_vertex].position.z);
			if(vertex[num_vertex].position.x > 8.0f || vertex[num_vertex].position.x < -8.0f || vertex[num_vertex].position.z > 7.0f || vertex[num_vertex].position.z < -7.0f)
			{
				vertex[num_vertex].position.y *= 0.1f;
			}
			vertex[num_vertex].tex.x = 0.0f;
			vertex[num_vertex].tex.y = 0.0f;
			float x = -0.03f*vertex[i].position.z*cos(0.1f*vertex[i].position.x) - 0.3f*cos(0.1*vertex[i].position.z);
			float y = 1.0f;
			float z = -0.3f*sin(0.1*vertex[num_vertex].position.x) + 0.03f*vertex[num_vertex].position.x*sin(0.1f*vertex[num_vertex].position.z);
			XMVECTOR N = XMVectorSet(x,y,z,0.0f);
			XMStoreFloat3(&vertex[num_vertex].normal,XMVector3Normalize(N));
			if(vertex[num_vertex].normal.y < 0)
			{
				vertex[num_vertex].normal.x = - vertex[num_vertex].normal.x;
				vertex[num_vertex].normal.y = - vertex[num_vertex].normal.y;
				vertex[num_vertex].normal.z = - vertex[num_vertex].normal.z;
			}
			num_vertex += 1;
		}
	}
	//填充顶点的索引信息
	for(int i=0; i < width_rec-1; ++i)  
	{  
		for(int j=0; j < height_rec-1; ++j)  
		{  
			index[num_index++] = i * height_rec + j;  
			index[num_index++] = (i + 1) * height_rec + j + 1;  
			index[num_index++] = (i + 1) * height_rec + j;  
			index[num_index++] = i * height_rec + j;  
			index[num_index++] = i * height_rec + j + 1;  
			index[num_index++] = (i + 1) * height_rec + j + 1; 
		}
	}
	return S_OK;
}
//公告板
mesh_billboard::mesh_billboard(ID3D11Device *device_need,ID3D11DeviceContext *contex_need):Geometry(device_need,contex_need)
{
	all_vertex = 500;
}
HRESULT mesh_billboard::find_point(spirit_point *vertex,UINT *index,int &num_vertex,int &num_index)
{
	num_index = 0;
	num_vertex = 256;
	for (int i = 0; i < 16; ++i) 
	{
		for (int j = 0; j < 16; ++j) 
		{
			vertex[i * 16 + j].position.x = 0.0f+ 0.5f*j;
			vertex[i * 16 + j].position.y = 0.3f;
			vertex[i * 16 + j].position.z = 4.0f - 0.5f*i;
			vertex[i * 16 + j].size.x = 0.5f;
			vertex[i * 16 + j].size.y = 0.5f;
		}
	}
	return S_OK;
}
void mesh_billboard::show_mesh() 
{
	UINT stride_need = sizeof(spirit_point);     //顶点结构的位宽
	UINT offset_need = 0;                       //顶点结构的首地址偏移
												//顶点缓存，索引缓存，绘图格式
	contex_pancy->IASetVertexBuffers(0, 1, &vertex_need, &stride_need, &offset_need);
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	//选定绘制路径
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_pancy->GetDesc(&techDesc);
	for (UINT i = 0; i<techDesc.Passes; ++i)
	{
		teque_pancy->GetPassByIndex(i)->Apply(0, contex_pancy);
		contex_pancy->Draw(all_vertex, 0);
	}
}
HRESULT mesh_billboard::init_point(spirit_point *vertex,UINT *index)
{
	D3D11_BUFFER_DESC point_buffer;
	point_buffer.Usage = D3D11_USAGE_IMMUTABLE;            //顶点是gpu只读型
	point_buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;         //缓存类型为顶点缓存
	point_buffer.ByteWidth = all_vertex * sizeof(spirit_point); //顶点缓存的大小
	point_buffer.CPUAccessFlags = 0;
	point_buffer.MiscFlags = 0;
	point_buffer.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA resource_vertex;
	resource_vertex.pSysMem = vertex;//指定顶点数据的地址
									 //创建顶点缓冲区
	HRESULT hr = device_pancy->CreateBuffer(&point_buffer, &resource_vertex, &vertex_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"create object error when build point", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
//细分曲面(四边形测试)
mesh_square_tessellation::mesh_square_tessellation(ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :Geometry(device_need, contex_need)
{
}
HRESULT mesh_square_tessellation::find_point(point_with_tangent *vertex, UINT *index, int &num_vertex, int &num_index)
{
	point_with_tangent square_test[] =
	{
		{ XMFLOAT3(-1.0f,1.0f,1.0f), XMFLOAT3(0.0f,1.0f,0.0f), XMFLOAT3(1.0f,0.0f,0.0f), XMFLOAT2(0.0f,0.0f) },
		{ XMFLOAT3(1.0f,1.0f,1.0f),  XMFLOAT3(0.0f,1.0f,0.0f), XMFLOAT3(1.0f,0.0f,0.0f), XMFLOAT2(1.0f,0.0f) },
		{ XMFLOAT3(-1.0f,1.0f,-1.0f),XMFLOAT3(0.0f,1.0f,0.0f), XMFLOAT3(1.0f,0.0f,0.0f), XMFLOAT2(1.0f,1.0f) },
		{ XMFLOAT3(1.0f,1.0f,-1.0f), XMFLOAT3(0.0f,1.0f,0.0f), XMFLOAT3(1.0f,0.0f,0.0f), XMFLOAT2(0.0f,1.0f) },
	};
	for (int i = 0; i < 4; ++i)
	{
		vertex[i] = square_test[i];
	}
	num_vertex = 4;
	num_index = 0;
	return S_OK;
}
HRESULT mesh_square_tessellation::init_point(point_with_tangent *vertex, UINT *index)
{
	D3D11_BUFFER_DESC point_buffer;
	point_buffer.Usage = D3D11_USAGE_IMMUTABLE;            //顶点是gpu只读型
	point_buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;         //缓存类型为顶点缓存
	point_buffer.ByteWidth = all_vertex * sizeof(point_with_tangent); //顶点缓存的大小
	point_buffer.CPUAccessFlags = 0;
	point_buffer.MiscFlags = 0;
	point_buffer.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA resource_vertex;
	resource_vertex.pSysMem = vertex;//指定顶点数据的地址
									 //创建顶点缓冲区
	HRESULT hr = device_pancy->CreateBuffer(&point_buffer, &resource_vertex, &vertex_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"create object error when build point", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void mesh_square_tessellation::show_mesh()
{
	UINT stride_need = sizeof(point_with_tangent);     //顶点结构的位宽
	UINT offset_need = 0;                       //顶点结构的首地址偏移
												//顶点缓存，索引缓存，绘图格式
	contex_pancy->IASetVertexBuffers(0, 1, &vertex_need, &stride_need, &offset_need);
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	//选定绘制路径
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_pancy->GetDesc(&techDesc);
	for (UINT i = 0; i<techDesc.Passes; ++i)
	{
		teque_pancy->GetPassByIndex(i)->Apply(0, contex_pancy);
		contex_pancy->Draw(4, 0);
	}
}