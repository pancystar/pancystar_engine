#pragma once
#include<windows.h>
#include"geometry.h"
#include <assimp/Importer.hpp>      // 导入器在该头文件中定义
#include <assimp/scene.h>           // 读取到的模型数据都放在scene中
#include <assimp/postprocess.h>     // 该头文件中包含后处理的标志位定义
#include <assimp/matrix4x4.h>
#include <assimp/matrix3x3.h>
#include<assimp/Exporter.hpp>
struct material_list
{
	char                       texture_diffuse[128];     //漫反射纹理地址
	char                       texture_normal[128];      //法线贴图纹理地址
	ID3D11ShaderResourceView   *tex_diffuse_resource;    //漫反射纹理
	ID3D11ShaderResourceView   *texture_normal_resource; //法线贴图纹理
	material_list()
	{
		texture_diffuse[0] = '\0';
		texture_normal[0] = '\0';
		tex_diffuse_resource = NULL;
		texture_normal_resource = NULL;
	}
};
template<typename T>
struct mesh_list
{
	Geometry<T> *point_buffer;
	int material_use;
	mesh_list()
	{
		point_buffer = NULL;
		material_use = 0;
	}
};
class assimp_basic 
{
protected:
	ID3D11Device           *device_pancy;     //d3d设备
	ID3D11DeviceContext    *contex_pancy;     //设备描述表
	ID3DX11EffectTechnique *teque_pancy;       //绘制路径
	std::string filename;        //模型文件名
	char rec_texpath[128];       //纹理路径
	Assimp::Importer importer;   //模型导入器
	const aiScene *model_need;   //模型存储类
	material_list *matlist_need; //材质表
	int material_optimization;   //优化后的材质数量
	int mesh_optimization;       //优化后的网格数量
	bool if_alpha_array[10000];    //用于判断是否是透明部分
public:
	assimp_basic(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* filename, char* texture_path);
	HRESULT model_create(bool if_adj, bool if_optimize, int alpha_partnum, int* alpha_part);
	int get_meshnum();
	virtual void get_texture(material_list *texture_need, int i) = 0;
	virtual void release() = 0;
	virtual void draw_part(int i) = 0;
	virtual void draw_mesh() = 0;
	virtual void draw_mesh_adj() = 0;
	HRESULT get_technique(ID3DX11EffectTechnique *teque_need);
protected:
	virtual HRESULT init_mesh(bool if_adj) = 0;
	HRESULT init_texture();
	void remove_texture_path(char rec[]);
	virtual HRESULT combine_vertex_array(int alpha_partnum, int* alpha_part, bool if_adj) = 0;
	virtual HRESULT optimization_mesh(bool if_adj) = 0;//网格优化
	void change_texturedesc_2dds(char rec[]);
	void release_basic();
};





template<typename T>
class model_reader_assimp : public assimp_basic
{
protected:
	//ID3D11Device           *device_pancy;     //d3d设备
	//ID3D11DeviceContext    *contex_pancy;     //设备描述表
	//ID3DX11EffectTechnique *teque_pancy;       //绘制路径
	//std::string filename;        //模型文件名
	//char rec_texpath[128];       //纹理路径
	//Assimp::Importer importer;   //模型导入器
	//const aiScene *model_need;   //模型存储类

	//material_list *matlist_need; //材质表
	mesh_list<T> *mesh_need;        //网格表
	//int material_optimization;   //优化后的材质数量
	//int mesh_optimization;       //优化后的网格数量
	Geometry<T> *mesh_scene;  //存储合并的场景顶点
	//bool if_alpha_array[10000];    //用于判断是否是透明部分
public:
	model_reader_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need,char* filename, char* texture_path);
	//HRESULT model_create(bool if_adj, bool if_optimize,int alpha_partnum,int* alpha_part);
	//int get_meshnum();
	void get_texture(material_list *texture_need, int i);
	void release();
	void draw_part(int i);
	void draw_mesh();
	void draw_mesh_adj();
	//void draw_part(int i);
	//void draw_mesh();
	//void draw_mesh_adj();
	//HRESULT get_technique(ID3DX11EffectTechnique *teque_need);
protected:
	virtual HRESULT init_mesh(bool if_adj);
	//HRESULT init_texture();
	//void remove_texture_path(char rec[]);
	HRESULT combine_vertex_array(int alpha_partnum, int* alpha_part, bool if_adj);
	HRESULT optimization_mesh(bool if_adj);//网格优化
};
template<typename T>
model_reader_assimp<T>::model_reader_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* pFile, char *texture_path) : assimp_basic(device_need,contex_need,pFile,texture_path)
{
	mesh_need = NULL;
	mesh_scene = NULL;
}
template<typename T>
HRESULT model_reader_assimp<T>::init_mesh(bool if_adj)
{
	T *point_need;
	unsigned int *index_need;
	//创建网格记录表
	mesh_need = new mesh_list<T>[model_need->mNumMeshes];
	mesh_optimization = model_need->mNumMeshes;
	for (int i = 0; i < model_need->mNumMeshes; i++)
	{
		//获取模型的第i个模块
		const aiMesh* paiMesh = model_need->mMeshes[i];
		//获取模型的材质编号
		mesh_need[i].material_use = paiMesh->mMaterialIndex;
		//模型的第i个模块的顶点及索引信息
		mesh_need[i].point_buffer = new mesh_comman<T>(device_pancy, contex_pancy, paiMesh->mNumVertices, paiMesh->mNumFaces * 3);
		point_need = (T*)malloc(paiMesh->mNumVertices * sizeof(T));
		index_need = (unsigned int*)malloc(paiMesh->mNumFaces * 3 * sizeof(unsigned int));
		//顶点缓存
		for (unsigned int j = 0; j < paiMesh->mNumVertices; j++)
		{
			point_need[j].position.x = paiMesh->mVertices[j].x;
			point_need[j].position.y = paiMesh->mVertices[j].y;
			point_need[j].position.z = paiMesh->mVertices[j].z;

			point_need[j].normal.x = paiMesh->mNormals[j].x;
			point_need[j].normal.y = paiMesh->mNormals[j].y;
			point_need[j].normal.z = paiMesh->mNormals[j].z;

			if (paiMesh->HasTextureCoords(0))
			{
				point_need[j].tex.x = paiMesh->mTextureCoords[0][j].x;
				point_need[j].tex.y = 1 - paiMesh->mTextureCoords[0][j].y;
			}
			else
			{
				point_need[j].tex.x = 0.0f;
				point_need[j].tex.y = 0.0f;
			}
			if (paiMesh->mTangents != NULL)
			{
				point_need[j].tangent.x = paiMesh->mTangents[j].x;
				point_need[j].tangent.y = paiMesh->mTangents[j].y;
				point_need[j].tangent.z = paiMesh->mTangents[j].z;
			}
			else
			{
				point_need[j].tangent.x = 0.0f;
				point_need[j].tangent.y = 0.0f;
				point_need[j].tangent.z = 0.0f;
			}
		}
		//索引缓存区
		int count_index = 0;
		for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
		{
			if (paiMesh->mFaces[j].mNumIndices == 3)
			{
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[0];
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[1];
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[2];
			}
			else
			{
				return E_FAIL;
			}
		}
		//根据内存信息创建显存区
		HRESULT hr = mesh_need[i].point_buffer->create_object(point_need, index_need, if_adj);
		if (FAILED(hr))
		{
			MessageBox(0, L"create model mesh error", L"tip", MB_OK);
			return hr;
		}
		//释放内存
		free(point_need);
		point_need = NULL;
		free(index_need);
		index_need = NULL;
	}
	return S_OK;
}
template<typename T>
HRESULT model_reader_assimp<T>::combine_vertex_array(int alpha_partnum, int* alpha_part, bool if_adj)
{
	T *vertex_out;
	UINT *index_out;
	int all_vertex = 0, all_index = 0;
	Geometry<T> *now_meshneed[700];
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		if (if_alpha_array[i] == false)
		{
			now_meshneed[i] = mesh_need[i].point_buffer;
		}
		else
		{
			int a = 0;
		}
	}
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		if (if_alpha_array[i] == false)
		{
			int rec_vertex, rec_index;
			now_meshneed[i]->get_point_num(rec_vertex, rec_index);
			all_vertex += rec_vertex;
			all_index += rec_index;
		}
	}
	vertex_out = (T*)malloc(all_vertex * sizeof(T) + 100);
	index_out = (UINT*)malloc(all_index * sizeof(UINT) + 100);
	//对几何体进行合并操作
	T *now_pointer = vertex_out;
	UINT *now_index_pointer = index_out;
	int vertex_num = 0;
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		if (if_alpha_array[i] == false)
		{
			T *vertex_rec;
			UINT *index_rec;
			int all_vertex = 0, all_index = 0;
			//获取几何体的顶点与索引数目
			now_meshneed[i]->get_point_num(all_vertex, all_index);
			vertex_rec = (T*)malloc(all_vertex * sizeof(T) + 100);
			index_rec = (UINT*)malloc(all_index * sizeof(UINT) + 100);
			//获取几何体的顶点与索引数据
			now_meshneed[i]->get_bufferdata(vertex_rec, index_rec);
			//拷贝顶点
			memcpy(now_pointer, vertex_rec, all_vertex * sizeof(T));
			//修改索引号
			for (int j = 0; j < all_index; ++j)
			{
				index_rec[j] += vertex_num;
			}
			//拷贝索引
			memcpy(now_index_pointer, index_rec, all_index * sizeof(UINT));
			//更新记录
			now_pointer += all_vertex;
			vertex_num += all_vertex;
			now_index_pointer += all_index;
			free(vertex_rec);
			free(index_rec);
		}
	}
	mesh_scene = new mesh_comman<T>(device_pancy, contex_pancy, all_vertex, all_index);
	HRESULT hr = mesh_scene->create_object(vertex_out, index_out, if_adj);
	if (FAILED(hr))
	{
		MessageBox(0, L"combine scene error", L"tip", MB_OK);
		return hr;
	}
	free(vertex_out);
	free(index_out);
	return S_OK;
}
template<typename T>
void model_reader_assimp<T>::get_texture(material_list *texture_need, int i)
{
	*texture_need = matlist_need[mesh_need[i].material_use];
}
template<typename T>
void model_reader_assimp<T>::release()
{
	//释放纹理资源
	if (model_need != NULL)
	{
		for (int i = 0; i < material_optimization; ++i)
		{
			if (matlist_need[i].tex_diffuse_resource != NULL)
			{
				matlist_need[i].tex_diffuse_resource->Release();
				matlist_need[i].tex_diffuse_resource = NULL;
			}
			if (matlist_need[i].texture_normal_resource != NULL)
			{
				matlist_need[i].texture_normal_resource->Release();
				matlist_need[i].texture_normal_resource = NULL;
			}
		}
		//release_basic();
		//释放缓冲区资源
		for (int i = 0; i < mesh_optimization; i++)
		{
			mesh_need[i].point_buffer->release();
		}
		mesh_scene->release();
		//释放表资源
		delete[] mesh_need;
		//释放表资源
		delete[] matlist_need;
		model_need->~aiScene();
	}
}
template<typename T>
HRESULT model_reader_assimp<T>::optimization_mesh(bool if_adj)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~先合并材质~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int hash_UnionFind[1000];
	memset(hash_UnionFind, 0, 1000 * sizeof(int));
	int now_different = 1;//当前已经有的互不相同的材质数量
	hash_UnionFind[0] = 0;
	/*
	material_list rec_need[1000];
	memcpy(rec_need, matlist_need,300*sizeof(material_list));
	*/
	for (int i = 1; i < model_need->mNumMaterials; ++i)
	{
		bool if_change = true;
		for (int j = 0; j < i; ++j)
		{
			if (strcmp(matlist_need[i].texture_diffuse, matlist_need[j].texture_diffuse) == 0 && strcmp(matlist_need[i].texture_normal, matlist_need[j].texture_normal) == 0)
			{
				//当前材质与之前的材质重复，删除之并标记hash
				if (matlist_need[i].tex_diffuse_resource != NULL)
				{
					matlist_need[i].tex_diffuse_resource->Release();
					matlist_need[i].tex_diffuse_resource = NULL;
					matlist_need[i].texture_diffuse[0] = '\0';
				}
				if (matlist_need[i].texture_normal_resource != NULL)
				{
					matlist_need[i].texture_normal_resource->Release();
					matlist_need[i].texture_normal_resource = NULL;
					matlist_need[i].texture_normal[0] = '\0';
				}
				hash_UnionFind[i] = j;
				if_change = false;
				break;
			}
		}
		if (if_change == true)
		{
			//添加一个不同的材质
			matlist_need[now_different] = matlist_need[i];
			hash_UnionFind[i] = now_different;
			now_different += 1;
		}
	}
	material_optimization = now_different;
	mesh_optimization = now_different - 1;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~合并几何体~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int all_vertex = 0, all_index = 0;
	for (int i = 0; i < model_need->mNumMeshes; i++)
	{
		//更改索引号
		mesh_need[i].material_use = hash_UnionFind[mesh_need[i].material_use];
		//计算总顶点数量
		int rec_vertex, rec_index;
		mesh_need[i].point_buffer->get_point_num(rec_vertex, rec_index);
		all_vertex += rec_vertex;
		all_index += rec_index;
	}
	//开启临时存储空间
	T *vertex_rec;
	UINT *index_rec;
	vertex_rec = (T*)malloc(all_vertex * sizeof(T) + 100);
	index_rec = (UINT*)malloc(all_index * sizeof(UINT) + 100);

	//创建临时存储几何体的指针
	mesh_list<T> *rec_optisave;
	rec_optisave = new mesh_list<T>[mesh_optimization + 10];
	//为新的几何体组织顶点和索引数据
	for (int i = 0; i < mesh_optimization; ++i)
	{
		T *now_pointer = vertex_rec;
		UINT *now_index_pointer = index_rec;
		int vertex_num = 0;
		int now_count_vertex = 0;
		int now_count_index = 0;
		for (int j = 0; j < model_need->mNumMeshes; j++)
		{
			//找到所有使用材质相同的网格，并将它们合并
			if (mesh_need[j].material_use == i + 1)
			{
				T *vertex_rec;
				UINT *index_rec;
				int check_vertex = 0, check_index = 0;
				//获取几何体的顶点与索引数目
				mesh_need[j].point_buffer->get_point_num(check_vertex, check_index);
				now_count_vertex += check_vertex;
				now_count_index += check_index;
				vertex_rec = (T*)malloc(check_vertex * sizeof(T) + 100);
				index_rec = (UINT*)malloc(check_index * sizeof(UINT) + 100);
				//获取几何体的顶点与索引数据
				mesh_need[j].point_buffer->get_bufferdata(vertex_rec, index_rec);


				//拷贝顶点
				memcpy(now_pointer, vertex_rec, check_vertex * sizeof(T));
				//修改索引号
				for (int k = 0; k < check_index; ++k)
				{
					index_rec[k] += vertex_num;
				}
				//拷贝索引
				memcpy(now_index_pointer, index_rec, check_index * sizeof(UINT));
				//更新记录
				now_pointer += check_vertex;
				vertex_num += check_vertex;
				now_index_pointer += check_index;
				free(vertex_rec);
				free(index_rec);
			}
		}
		rec_optisave[i].point_buffer = new mesh_comman<T>(device_pancy, contex_pancy, now_count_vertex, now_count_index);
		rec_optisave[i].material_use = i + 1;
		HRESULT hr = rec_optisave[i].point_buffer->create_object(vertex_rec, index_rec, if_adj);
		if (FAILED(hr))
		{
			MessageBox(0, L"combine scene error", L"tip", MB_OK);
			return hr;
		}
	}
	free(vertex_rec);
	free(index_rec);
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		mesh_need[i].point_buffer->release();
	}
	for (int i = 0; i < material_optimization; ++i)
	{
		mesh_need[i] = rec_optisave[i];
	}
	return S_OK;
}
template<typename T>
void model_reader_assimp<T>::draw_part(int i)
{
	mesh_need[i].point_buffer->get_teque(teque_pancy);
	mesh_need[i].point_buffer->show_mesh();
}
template<typename T>
void model_reader_assimp<T>::draw_mesh()
{
	mesh_scene->get_teque(teque_pancy);
	mesh_scene->show_mesh();
}
template<typename T>
void model_reader_assimp<T>::draw_mesh_adj()
{
	mesh_scene->get_teque(teque_pancy);
	mesh_scene->show_mesh_adj();
}

struct skin_tree
{
	char bone_ID[128];
	int bone_number;
	XMFLOAT4X4 basic_matrix;
	XMFLOAT4X4 animation_matrix;
	XMFLOAT4X4 now_matrix;
	skin_tree *brother;
	skin_tree *son;
	skin_tree()
	{
		bone_ID[0] = '\0';
		bone_number = -1;
		brother = NULL;
		son = NULL;
		XMStoreFloat4x4(&basic_matrix,XMMatrixIdentity());
		XMStoreFloat4x4(&animation_matrix, XMMatrixIdentity());
		XMStoreFloat4x4(&now_matrix, XMMatrixIdentity());
	}
};
//变换向量
struct vector_animation
{
	float time;               //帧时间
	float main_key[3];        //帧数据
};
//变换四元数
struct quaternion_animation
{
	float time;               //帧时间
	float main_key[4];        //帧数据
};
//变换矩阵
struct matrix_animation
{
	float time;               //帧时间
	float main_key[16];       //帧数据
};
struct animation_data
{
	char bone_name[128];                                //本次变换数据对应的骨骼名称
	skin_tree *bone_point;                              //本次变换数据对应的骨骼的指针

	DWORD number_translation;                           //平移变换的数量
	vector_animation *translation_key;                  //各个平移变换数据

	DWORD number_scaling;                               //放缩变换的数量
	vector_animation *scaling_key;                      //各个放缩变换数据

	DWORD number_rotation;                              //旋转变换的数量
	quaternion_animation *rotation_key;                 //各个旋转变换的数据

	DWORD number_transform;                             //混合变换的数量
	matrix_animation *transform_key;                    //各个混合变换的数据	

	struct animation_data *next;                        //下一个变换数据
};
struct animation_set
{
	char  animation_name[128];                          //该动画的名字
	float animation_length;                             //动画的长度
	DWORD number_animation;                             //动画包含的变换数量
	animation_data *head_animition;                     //该动画的数据
	animation_set *next;                                //指向下一个动画的指针
};
class skin_mesh : public model_reader_assimp<point_withskin>
{
	skin_tree *root_skin;
	animation_set *first_animation;
	float time_all;
	int bone_num;
	XMFLOAT4X4 bone_matrix_array[100];
	XMFLOAT4X4 offset_matrix_array[100];
	XMFLOAT4X4 final_matrix_array[100];
	int tree_node_num[100][100];
public:
	skin_mesh(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* filename, char* texture_path);
	void update_root(skin_tree *root, XMFLOAT4X4 matrix_parent);
	void update_mesh_offset(int i);
	void update_mesh_offset();
	void update_animation(float delta_time);
	XMFLOAT4X4* get_bone_matrix(int i, int &num_bone);
	XMFLOAT4X4* get_bone_matrix();
	void release_all();
private:
	HRESULT init_mesh(bool if_adj);
	aiNode *find_skinroot(aiNode *now_node, char root_name[]);
	HRESULT build_skintree(aiNode *now_node, skin_tree *now_root);
	HRESULT build_animation_list();
	bool check_ifsame(char a[], char b[]);
	void set_matrix(XMFLOAT4X4 &out, aiMatrix4x4 *in);
	skin_tree* find_tree(skin_tree* p, char name[]);
	skin_tree* find_tree(skin_tree* p, int num);
	void free_tree(skin_tree *now);
	void update_anim_data(animation_data *now);
	void find_anim_sted(int &st, int &ed, quaternion_animation *input, int num_animation);
	void find_anim_sted(int &st, int &ed, vector_animation *input, int num_animation);
	void Interpolate(quaternion_animation& pOut, quaternion_animation pStart, quaternion_animation pEnd, float pFactor);
	void Interpolate(vector_animation& pOut, vector_animation pStart, vector_animation pEnd, float pFactor);
	void Get_quatMatrix(XMFLOAT4X4 &resMatrix, quaternion_animation& pOut);
};
class geometry_shadow
{
	assimp_basic *model_data;
	ID3D11ShaderResourceView *transparent_resource;
	XMFLOAT4X4 world_matrix;
	XMFLOAT4X4 *bone_matrix;
	bool if_skinmesh;
	bool if_transparent;
	int bone_num;
	int transparent_part;
public:
	geometry_shadow(assimp_basic *model_input, bool if_skin, bool if_trans, int trans_part, XMFLOAT4X4 matrix_need, XMFLOAT4X4 *matrix_bone,int bone_num_need,ID3D11ShaderResourceView *tex_need);
	void draw_full_geometry(ID3DX11EffectTechnique *tech_common);
	void draw_full_geometry_adj(ID3DX11EffectTechnique *tech_common);
	void draw_transparent_part(ID3DX11EffectTechnique *tech_transparent);
	bool check_if_trans() { return if_transparent; };
	bool check_if_skin() { return if_skinmesh; };
	XMFLOAT4X4 get_world_matrix() { return world_matrix; };
	ID3D11ShaderResourceView *get_transparent_tex() { return transparent_resource; };
	XMFLOAT4X4* get_bone_matrix() { return if_skinmesh ? bone_matrix : NULL; };
	int get_bone_num() { return bone_num; };
};
class geometry_control
{
	ID3D11Device        *device_pancy;     //d3d设备
	ID3D11DeviceContext *contex_pancy;     //设备描述表
	model_reader_assimp<point_with_tangent> *yuri_model;
	model_reader_assimp<point_with_tangent> *castel_model;
	skin_mesh           *yuri_animation_model;
	Geometry<point_with_tangent>  *floor_need;          //盒子模型
	Geometry<point_with_tangent>  *sky_need;            //球体模型
	ID3D11ShaderResourceView      *tex_skycube;         //天空盒
	ID3D11ShaderResourceView      *tex_floor;           //地面纹理视图指针
	ID3D11ShaderResourceView      *tex_normal;          //法线贴图

public:
	geometry_control(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	Geometry<point_with_tangent>  *get_floor_geometry() { return floor_need; };
	Geometry<point_with_tangent>  *get_sky_geometry() { return sky_need; };
	ID3D11ShaderResourceView      *get_basic_floor_tex() { return tex_floor; };
	ID3D11ShaderResourceView      *get_floor_normal_tex() { return tex_normal; };
	ID3D11ShaderResourceView      *get_sky_cube_tex() { return tex_skycube; };
	skin_mesh                     *get_yuri_animation() { return yuri_animation_model; };
	model_reader_assimp<point_with_tangent>           *get_yuri() { return yuri_model; }
	model_reader_assimp<point_with_tangent>           *get_castel() { return castel_model; }
	
	HRESULT create();
	void release();
};