#pragma once
#include<windows.h>
#include<string.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<directxmath.h>
#include<d3dx11effect.h>
#include<DDSTextureLoader.h>
#include<DirectXMesh.h>
using namespace DirectX;
enum CullMode
{
	Cull_CW = 0,
	Cull_CCW = 1,
	Cull_NONE = 2
};
struct pancy_point
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 tex;
};
struct spirit_point
{
	XMFLOAT3 position;
	XMFLOAT2 size;
};
struct point_with_tangent
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT2 tex;
};
struct fire_point
{
	XMFLOAT3 position;
	XMFLOAT3 speed;
	XMFLOAT2 Size;
	float Age;
	unsigned int Type;
};
struct sakura_point
{
	XMFLOAT3 position;
	XMFLOAT3 speed;
	XMFLOAT2 Size;
	float Age;
	unsigned int Type;
	unsigned int texnum;
};
struct tess_point
{
	XMFLOAT3 position;
};
struct HDR_fullscreen
{
	XMFLOAT3 position;
	XMFLOAT2 tex;
};
template<typename T>
class Geometry
{
protected:
	ID3D11Buffer            *vertex_need;       //顶点缓冲区的数据
	ID3D11Buffer	        *index_need;        //索引缓冲区数据
	ID3D11Buffer	        *indexadj_need;        //邻接索引缓冲区数据
	ID3D11Device            *device_pancy;      //d3d设备
	ID3D11DeviceContext     *contex_pancy;      //设备描述表
	ID3DX11EffectTechnique  *teque_pancy;       //绘制路径
	int                     all_vertex;         //几何体的顶点个数
	int                     all_index;          //几何体的索引个数
	bool                    if_init_adj;
public:
	Geometry(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	void get_point_num(int &vertex_number, int &index_number) { vertex_number = all_vertex; index_number = all_index; };
	void get_teque(ID3DX11EffectTechnique *teque_need);
	virtual void show_mesh();
	virtual void show_mesh_adj();
	void release();
	//直接创建几何体，顶点由内置函数init_point()生成。
	virtual HRESULT create_object();
	//根据外部顶点创建几何体。
	virtual HRESULT create_object(T *vertex, UINT *index, bool if_adj);
	//获取缓冲区的数据
	virtual HRESULT get_bufferdata(T *vertex, UINT *index);
protected:
	//根据内存里的顶点信息在显存里开启一份拷贝
	virtual HRESULT init_point(T *vertex, UINT *index);
	//顶点生成函数。
	virtual HRESULT find_point(T *vertex, UINT *index, int &num_vertex, int &num_index) = 0;
	//转换缓存资源使得GPU资源可以map到CPU
	ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Buffer* pGBuffer);
};
//父类
template<typename T>
Geometry<T>::Geometry(ID3D11Device *device_need, ID3D11DeviceContext *contex_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	vertex_need = NULL;
	index_need = NULL;
	indexadj_need = NULL;
	all_vertex = 0;
	all_index = 0;
	if_init_adj = false;
}
template<typename T>
void Geometry<T>::get_teque(ID3DX11EffectTechnique *teque_need)
{
	teque_pancy = teque_need;
}
template<typename T>
HRESULT Geometry<T>::init_point(T *vertex, UINT *index)
{
	D3D11_BUFFER_DESC point_buffer;
	point_buffer.Usage = D3D11_USAGE_IMMUTABLE;            //顶点是gpu只读型
	point_buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;         //缓存类型为顶点缓存
	point_buffer.ByteWidth = all_vertex * sizeof(T); //顶点缓存的大小
	point_buffer.CPUAccessFlags = 0;
	point_buffer.MiscFlags = 0;
	point_buffer.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA resource_vertex;
	resource_vertex.pSysMem = vertex;//指定顶点数据的地址
										  //创建顶点缓冲区
	HRESULT hr = device_pancy->CreateBuffer(&point_buffer, &resource_vertex, &vertex_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"init point error", L"tip", MB_OK);
		return hr;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建索引缓冲区~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//缓冲区格式
	D3D11_BUFFER_DESC index_buffer;
	index_buffer.ByteWidth = all_index*sizeof(UINT);
	index_buffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer.Usage = D3D11_USAGE_IMMUTABLE;
	index_buffer.CPUAccessFlags = 0;
	index_buffer.MiscFlags = 0;
	index_buffer.StructureByteStride = 0;
	//然后给出数据
	D3D11_SUBRESOURCE_DATA resource_index = { 0 };
	resource_index.pSysMem = index;
	//根据描述和数据创建索引缓存
	hr = device_pancy->CreateBuffer(&index_buffer, &resource_index, &index_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"init point error", L"tip", MB_OK);
		return hr;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建邻接索引缓冲区~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (if_init_adj == true)
	{
		//获取邻接数据
		XMFLOAT3 *positions = (XMFLOAT3*)malloc(all_vertex*sizeof(XMFLOAT3));
		for (int i = 0; i < all_vertex; ++i)
		{
			positions[i] = vertex[i].position;
		}
		UINT *rec_adj, *rec_pointrep, *adj_index;
		rec_adj = (UINT*)malloc(all_index * sizeof(UINT));
		rec_pointrep = (UINT*)malloc(all_vertex * sizeof(UINT));
		adj_index = (UINT*)malloc(2 * all_index * sizeof(UINT));
		GenerateAdjacencyAndPointReps(index, all_index / 3, positions, all_vertex, 0.01f, rec_pointrep, rec_adj);
		GenerateGSAdjacency(index, all_index / 3, rec_pointrep, rec_adj, all_vertex, adj_index);
		//绑定数据
		D3D11_SUBRESOURCE_DATA resource_index_adj = { 0 };
		resource_index_adj.pSysMem = adj_index;
		//创建缓冲区
		index_buffer.ByteWidth = 2 * all_index*sizeof(UINT);
		hr = device_pancy->CreateBuffer(&index_buffer, &resource_index_adj, &indexadj_need);
		if (FAILED(hr))
		{
			MessageBox(0, L"init point error", L"tip", MB_OK);
			return hr;
		}
		free(positions);
		free(rec_pointrep);
		free(rec_adj);
		free(adj_index);
	}
	return S_OK;
}
template<typename T>
void Geometry<T>::show_mesh()
{
	UINT stride_need = sizeof(T);     //顶点结构的位宽
	UINT offset_need = 0;                       //顶点结构的首地址偏移
												//顶点缓存，索引缓存，绘图格式
	contex_pancy->IASetVertexBuffers(0, 1, &vertex_need, &stride_need, &offset_need);
	contex_pancy->IASetIndexBuffer(index_need, DXGI_FORMAT_R32_UINT, 0);
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//选定绘制路径
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_pancy->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		teque_pancy->GetPassByIndex(i)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(all_index, 0, 0);
	}
}
template<typename T>
void Geometry<T>::show_mesh_adj()
{
	UINT stride_need = sizeof(T);     //顶点结构的位宽
	UINT offset_need = 0;                       //顶点结构的首地址偏移
												//顶点缓存，索引缓存，绘图格式
	contex_pancy->IASetVertexBuffers(0, 1, &vertex_need, &stride_need, &offset_need);
	contex_pancy->IASetIndexBuffer(indexadj_need, DXGI_FORMAT_R32_UINT, 0);
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ);
	//选定绘制路径
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_pancy->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		teque_pancy->GetPassByIndex(i)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(all_index*2, 0, 0);
	}
}
template<typename T>
void Geometry<T>::release()
{
	if (vertex_need != NULL)
	{
		vertex_need->Release();
	}
	if (index_need != NULL)
	{
		index_need->Release();
	}
	if (indexadj_need != NULL)
	{
		indexadj_need->Release();
	}
}
template<typename T>
HRESULT Geometry<T>::create_object()
{
	if (all_vertex == 0)
	{
		MessageBox(0, L"son class dosent count the number of vertex", L"tip", MB_OK);
		return E_FAIL;
	}
	T *vertex_use = new T[all_vertex + 100];
	UINT   *index_use = new UINT[all_vertex * 6 + 100];  //索引数据
	HRESULT hr;
	hr = find_point(vertex_use, index_use, all_vertex, all_index);
	if (FAILED(hr))
	{
		MessageBox(0, L"create object error when build point", L"tip", MB_OK);
		return hr;
	}
	hr = init_point(vertex_use, index_use);
	if (FAILED(hr))
	{
		MessageBox(0, L"create object error when build buffer", L"tip", MB_OK);
		return hr;
	}
	delete[] vertex_use;
	delete[] index_use;
	return S_OK;
}
template<typename T>
HRESULT Geometry<T>::create_object(T *vertex, UINT *index, bool if_adj)
{
	if_init_adj = if_adj;
	if (vertex == NULL || index <= 0)
	{
		MessageBox(0, L"son class dosent count the number of vertex", L"tip", MB_OK);
		return E_FAIL;
	}
	HRESULT hr = init_point(vertex, index);
	if (FAILED(hr))
	{
		MessageBox(0, L"create object error when build buffer", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
template<typename T>
HRESULT Geometry<T>::get_bufferdata(T *vertex, UINT *index)
{
	if (vertex_need == NULL || index_need == NULL)
	{
		MessageBox(0, L"get vertex/index buffer data error", L"tip", MB_OK);
		return E_FAIL;
	}
	ID3D11Buffer* vertex_rec = NULL;
	ID3D11Buffer* index_rec = NULL;
	vertex_rec = CreateAndCopyToDebugBuf(vertex_need);
	index_rec = CreateAndCopyToDebugBuf(index_need);

	D3D11_MAPPED_SUBRESOURCE vertex_resource;
	D3D11_MAPPED_SUBRESOURCE index_resource;
	HRESULT hr;
	hr = contex_pancy->Map(vertex_rec, 0, D3D11_MAP_READ, 0, &vertex_resource);
	if (FAILED(hr))
	{
		MessageBox(0, L"get vertex buffer map error", L"tip", MB_OK);
		return hr;
	}
	hr = contex_pancy->Map(index_rec, 0, D3D11_MAP_READ, 0, &index_resource);
	if (FAILED(hr))
	{
		MessageBox(0, L"get index buffer map error", L"tip", MB_OK);
		return hr;
	}
	memcpy(static_cast<void*>(vertex), vertex_resource.pData, all_vertex * sizeof(T));
	memcpy(static_cast<void*>(index), index_resource.pData, all_index * sizeof(UINT));
	contex_pancy->Unmap(vertex_rec, 0);
	vertex_rec->Release();
	contex_pancy->Unmap(index_rec, 0);
	index_rec->Release();
	return S_OK;
}
template<typename T>
ID3D11Buffer* Geometry<T>::CreateAndCopyToDebugBuf(ID3D11Buffer* pGBuffer)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	pGBuffer->GetDesc(&bufferDesc);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.BindFlags = 0;
	bufferDesc.MiscFlags = 0;
	ID3D11Buffer* pDebugBuffer = NULL;
	if (SUCCEEDED(device_pancy->CreateBuffer(&bufferDesc, NULL, &pDebugBuffer)))
	{
		contex_pancy->CopyResource(pDebugBuffer, pGBuffer);
	}
	return pDebugBuffer;
}

class mesh_comman : public Geometry<point_with_tangent>
{
public:
	mesh_comman(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, int vertexnum_need, int indexnum_need);
private:
	HRESULT find_point(point_with_tangent *vertex, UINT *index, int &num_vertex, int &num_index);
};

class mesh_mountain : public Geometry<pancy_point>
{
	int width_rec;
	int height_rec;
public:
	mesh_mountain(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, int width_need, int height_need);
private:
	HRESULT find_point(pancy_point *vertex, UINT *index, int &num_vertex, int &num_index);
};
class mesh_cube : public Geometry<pancy_point>
{
public:
	mesh_cube(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
private:
	HRESULT find_point(pancy_point *vertex, UINT *index, int &num_vertex, int &num_index);
};
class mesh_ball : public Geometry<point_with_tangent>
{
	int circle_num;
	int vertex_percircle;
public:
	mesh_ball(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, int circle_num_need, int vertex_percircle_need);
private:
	HRESULT find_point(point_with_tangent *vertex, UINT *index, int &num_vertex, int &num_index);
};
class mesh_heart : public Geometry<pancy_point>
{
	int heart_divide;
	int line_percircle;
public:
	mesh_heart(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, int circle_num_need, int vertex_percircle_need);
private:
	HRESULT find_point(pancy_point *vertex, UINT *index, int &num_vertex, int &num_index);
	double find_f(double a, double b, double z);
	double find_f1(double a, double b, double z);
	double get_z(double x, double y, double ans_start, double balance_check);
	double find_st(double x, double st, double ed, int type, bool if_ans, double ans_check);
	bool check_range(double x, double &st, double &ed);
	XMFLOAT3 count_normal(double x, double y, double z);
	bool check_normal(XMFLOAT3 vec_normal, XMFLOAT3 vec_pos);
};
class mesh_billboard : public Geometry<spirit_point>
{
public:
	mesh_billboard(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	void show_mesh();
private:
	HRESULT find_point(spirit_point *vertex, UINT *index, int &num_vertex, int &num_index);
	HRESULT init_point(spirit_point *vertex, UINT *index);
};
class mesh_cubewithtargent : public Geometry<point_with_tangent>
{
public:
	mesh_cubewithtargent(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
private:
	HRESULT find_point(point_with_tangent *vertex, UINT *index, int &num_vertex, int &num_index);
};
class mesh_square_tessellation : public Geometry<point_with_tangent>
{
public:
	mesh_square_tessellation(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	void show_mesh();
private:
	HRESULT find_point(point_with_tangent *vertex, UINT *index, int &num_vertex, int &num_index);
	HRESULT init_point(point_with_tangent *vertex, UINT *index);
};
