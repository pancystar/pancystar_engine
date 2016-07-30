#pragma once
#include<windows.h>
#include"geometry.h"
#include <assimp/Importer.hpp>      // 导入器在该头文件中定义
#include <assimp/scene.h>           // 读取到的模型数据都放在scene中
#include <assimp/postprocess.h>     // 该头文件中包含后处理的标志位定义
#include <assimp/matrix4x4.h>
#include <assimp/matrix3x3.h>
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
struct mesh_list
{
	Geometry<point_with_tangent> *point_buffer;
	int material_use;
	mesh_list()
	{
		point_buffer = NULL;
		material_use = 0;
	}
};
class model_reader_assimp
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
	mesh_list *mesh_need;        //网格表
	int material_optimization;   //优化后的材质数量
	int mesh_optimization;       //优化后的网格数量
	Geometry<point_with_tangent> *mesh_scene;  //存储合并的场景顶点
public:
	model_reader_assimp(ID3D11Device *device_need, ID3D11DeviceContext *contex_need,char* filename, char* texture_path);
	HRESULT model_create(bool if_optimize);
	int get_meshnum();
	void get_texture(material_list *texture_need, int i);
	void release();
	void draw_part(int i);
	void draw_mesh();
	HRESULT get_technique(ID3DX11EffectTechnique *teque_need);
protected:
	virtual HRESULT init_mesh();
	HRESULT init_texture();
	void remove_texture_path(char rec[]);
	HRESULT combine_vertex_array();
	HRESULT optimization_mesh();//网格优化
};