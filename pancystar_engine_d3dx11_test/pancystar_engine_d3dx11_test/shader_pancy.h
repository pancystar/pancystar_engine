#pragma once
#include<windows.h>
#include<iostream>
#include<D3D11.h>
#include<assert.h>
#include<d3dx11effect.h>
//#include<d3dx11dbg.h>
#include<directxmath.h>
#include <sstream>
#include <fstream>
#include <vector>
#include<d3dcompiler.h>
using namespace DirectX;
enum light_type 
{
	direction_light = 0,
	point_light     = 1,
	spot_light      = 2
};
struct light_point_handle//为shader中点光源相关的全局变量赋值的句柄集合
{
	ID3DX11EffectVariable *ambient;
	ID3DX11EffectVariable *diffuse;
	ID3DX11EffectVariable *specular;
	ID3DX11EffectVariable *position;
	ID3DX11EffectVariable *decay;
};
struct pancy_light_dir//方向光结构
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
	XMFLOAT3 dir;

	float    range;
};
struct pancy_light_point//点光源结构
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;

	XMFLOAT3 position;
	float    range;

	XMFLOAT3 decay;
};
struct pancy_light_spot//聚光灯结构
{
	XMFLOAT4    ambient;  
	XMFLOAT4    diffuse;
	XMFLOAT4    specular;

	XMFLOAT3    dir;
	float       spot;

	XMFLOAT3    position;
	float       theta; 

	XMFLOAT3    decay;
	float       range;
	
};
struct material_handle//为shader中材质相关的全局变量赋值的句柄集合
{
	ID3DX11EffectVariable *ambient;
	ID3DX11EffectVariable *diffuse;
	ID3DX11EffectVariable *specular;
};
struct pancy_material//材质结构
{
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
};

class shader_basic
{
protected:
	ID3D11Device                          *device_pancy;        //d3d设备
	ID3D11DeviceContext                   *contex_pancy;        //设备描述表
	ID3D11InputLayout                     *input_need;          //定义了shader及其接受输入的格式
	ID3DX11Effect                         *fx_need;             //shader接口
	LPCWSTR                               shader_filename;      //shader文件名
public:
	shader_basic(LPCWSTR filename,ID3D11Device *device_need,ID3D11DeviceContext *contex_need);				 //构造函数，输入shader文件的文件名
	HRESULT shder_create();
	HRESULT get_technique(ID3DX11EffectTechnique** tech_need,LPCSTR tech_name); //获取渲染路径
	virtual void release() = 0;
protected:
	HRESULT combile_shader(LPCWSTR filename);		//shader编译接口
	virtual void init_handle() = 0;                 //注册全局变量句柄
	virtual void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member) = 0;
	HRESULT set_matrix(ID3DX11EffectMatrixVariable *mat_handle, XMFLOAT4X4 *mat_need);
	void release_basic();
	template<class T> 
	void safe_release(T t)
	{
		if(t != NULL)
		{
			t->Release();
			t = 0;
		}
	}
};
class light_pre : public shader_basic 
{
	light_point_handle      pointlight_handle;           //光照信息
	ID3DX11EffectVariable   *view_pos_handle;            //视点位置
	ID3DX11EffectVariable   *material_need;              //材质
	ID3DX11EffectVariable   *light_dir;                  //方向光
	ID3DX11EffectVariable   *light_point;                //点光源
	ID3DX11EffectVariable   *light_spot;                 //聚光灯
	ID3DX11EffectShaderResourceVariable   *texture_diffuse_handle;     //shader中的纹理资源句柄
	ID3DX11EffectShaderResourceVariable   *texture_normal_handle;      //法线贴图纹理
	ID3DX11EffectShaderResourceVariable   *texture_shadow_handle;      //阴影贴图句柄
	ID3DX11EffectShaderResourceVariable   *texture_ssao_handle;        //环境光贴图句柄

	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //全套几何变换句柄
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //世界变换句柄
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //法线变换句柄
	//ID3DX11EffectMatrixVariable           *texture_matrix_handle;    //纹理变换句柄
	ID3DX11EffectMatrixVariable           *shadowmap_matrix_handle;    //shadowmap矩阵变换句柄
	ID3DX11EffectMatrixVariable           *ssao_matrix_handle;         //ssao矩阵变换句柄
public:
	light_pre(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_view_pos(XMFLOAT3 eye_pos);
	HRESULT set_trans_world(XMFLOAT4X4 *mat_need);                          //设置世界变换
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);                            //设置总变换
	HRESULT set_trans_shadow(XMFLOAT4X4 *mat_need);                         //设置阴影变换
	HRESULT set_trans_ssao(XMFLOAT4X4 *mat_need);                           //设置环境光变换
	virtual HRESULT set_material(pancy_material material_in);				//设置材质
	virtual HRESULT set_ssaotex(ID3D11ShaderResourceView *tex_in);			//设置ssaomap
	virtual HRESULT set_shadowtex(ID3D11ShaderResourceView *tex_in);		//设置shadowmap
	virtual HRESULT set_diffusetex(ID3D11ShaderResourceView *tex_in);		//设置漫反射纹理
	virtual HRESULT set_normaltex(ID3D11ShaderResourceView *tex_in);		//设置法线贴图纹理

	HRESULT set_dirlight(pancy_light_dir light_need,int light_num);         //设置一个方向光源
	HRESULT set_pointlight(pancy_light_point light_need, int light_num);    //设置一个点光源
	HRESULT set_spotlight(pancy_light_spot light_need, int light_num);      //设置一个聚光灯光源
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class light_shadow : public shader_basic 
{
	ID3DX11EffectMatrixVariable *project_matrix_handle; //全套几何变换句柄
public:
	light_shadow(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_need);        //设置总变换
	void release();
private:
	void init_handle();                 //注册全局变量句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao_shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_ssaodepthnormal_map : public shader_basic
{
	ID3DX11EffectMatrixVariable           *project_matrix_handle;      //全套几何变换句柄
	ID3DX11EffectMatrixVariable           *world_matrix_handle;        //世界变换句柄
	ID3DX11EffectMatrixVariable           *normal_matrix_handle;       //法线变换句柄
public:
	shader_ssaodepthnormal_map(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_trans_world(XMFLOAT4X4 *mat_world, XMFLOAT4X4 *mat_view);
	HRESULT set_trans_all(XMFLOAT4X4 *mat_final);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao_map_shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_ssaomap : public shader_basic
{
	ID3DX11EffectMatrixVariable* ViewToTexSpace;
	ID3DX11EffectVectorVariable* OffsetVectors;
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectShaderResourceVariable* NormalDepthMap;
	ID3DX11EffectShaderResourceVariable* RandomVecMap;
public:
	shader_ssaomap(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);

	HRESULT set_ViewToTexSpace(XMFLOAT4X4 *mat);
	HRESULT set_OffsetVectors(const XMFLOAT4 v[14]);
	HRESULT set_FrustumCorners(const XMFLOAT4 v[4]);
	HRESULT set_NormalDepthtex(ID3D11ShaderResourceView* srv);
	HRESULT set_randomtex(ID3D11ShaderResourceView* srv);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao_blur_shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class shader_ssaoblur : public shader_basic
{
	ID3DX11EffectScalarVariable* TexelWidth;
	ID3DX11EffectScalarVariable* TexelHeight;

	ID3DX11EffectShaderResourceVariable* NormalDepthMap;
	ID3DX11EffectShaderResourceVariable* InputImage;
public:
	shader_ssaoblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT set_image_size(float width, float height);
	HRESULT set_tex_resource(ID3D11ShaderResourceView* tex_normaldepth, ID3D11ShaderResourceView* tex_aomap);
	void release();
private:
	void init_handle();//注册shader中所有全局变量的句柄
	void set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member);
};
class shader_control
{
	light_pre                  *shader_light_pre;          //前向光照着色器
	light_shadow               *shader_shadowmap;          //阴影着色器
	shader_ssaodepthnormal_map *shader_ssao_depthnormal;   //ssao深度纹理着色器
	shader_ssaomap             *shader_ssao_draw;          //ssao遮蔽图渲染着色器
	shader_ssaoblur            *shader_ssao_blur;          //ssao模糊着色器

	shader_basic *shader_light_deferred;
	shader_basic *shader_cubemap;
	shader_basic *particle_build;
	shader_basic *particle_show;
public:
	shader_control();
	HRESULT shader_init(ID3D11Device *device_pancy, ID3D11DeviceContext *contex_pancy);
	light_pre* get_shader_prelight() { return shader_light_pre; };
	light_shadow* get_shader_shadowmap() { return shader_shadowmap; };
	shader_ssaodepthnormal_map* get_shader_ssaodepthnormal() {return shader_ssao_depthnormal;};
	shader_ssaomap* get_shader_ssaodraw() { return shader_ssao_draw; };
	shader_ssaoblur* get_shader_ssaoblur() { return shader_ssao_blur; };
	void release();
};

