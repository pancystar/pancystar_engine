#include"shader_pancy.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~基础的着色器编译部分~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_basic::shader_basic(LPCWSTR filename,ID3D11Device *device_need,ID3D11DeviceContext *contex_need)
{
	fx_need = NULL;
	device_pancy = device_need;
	contex_pancy = contex_need;
	shader_filename = filename;
}
HRESULT shader_basic::shder_create() 
{
	HRESULT hr = combile_shader(shader_filename);
	if (hr != S_OK) 
	{
		MessageBox(0,L"combile shader error",L"tip",MB_OK);
		return hr;
	}
	init_handle();
	return S_OK;
}
void shader_basic::release_basic()
{
	safe_release(fx_need);
}
HRESULT shader_basic::get_technique(ID3DX11EffectTechnique** tech_need,LPCSTR tech_name)
{
	D3D11_INPUT_ELEMENT_DESC member_point[30];
	UINT num_member;
	set_inputpoint_desc(member_point,&num_member);
	*tech_need = fx_need->GetTechniqueByName(tech_name);
	D3DX11_PASS_DESC pass_shade;
	HRESULT hr2;
	hr2 = (*tech_need)->GetPassByIndex(0)->GetDesc(&pass_shade);
	HRESULT hr = device_pancy->CreateInputLayout(member_point,num_member,pass_shade.pIAInputSignature,pass_shade.IAInputSignatureSize,&input_need);
	if(FAILED(hr))
	{
		MessageBox(NULL, L"CreateInputLayout错误!", L"错误", MB_OK);
		return hr;
	}
	contex_pancy->IASetInputLayout(input_need);
	input_need->Release();
	input_need = NULL;
	return S_OK;
}
HRESULT shader_basic::get_technique(D3D11_INPUT_ELEMENT_DESC member_point[], UINT num_member, ID3DX11EffectTechnique** tech_need, LPCSTR tech_name)
{
	*tech_need = fx_need->GetTechniqueByName(tech_name);
	D3DX11_PASS_DESC pass_shade;
	HRESULT hr2;
	hr2 = (*tech_need)->GetPassByIndex(0)->GetDesc(&pass_shade);
	HRESULT hr = device_pancy->CreateInputLayout(member_point, num_member, pass_shade.pIAInputSignature, pass_shade.IAInputSignatureSize, &input_need);
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	contex_pancy->IASetInputLayout(input_need);
	input_need->Release();
	input_need = NULL;
	return S_OK;
}
HRESULT shader_basic::combile_shader(LPCWSTR filename)
{
	//创建shader
	UINT flag_need(0);
	flag_need |= D3D10_SHADER_SKIP_OPTIMIZATION;
#if defined(DEBUG) || defined(_DEBUG)
	flag_need |= D3D10_SHADER_DEBUG;
#endif
	//两个ID3D10Blob用来存放编译好的shader及错误消息
	ID3D10Blob	*shader(NULL);
	ID3D10Blob	*errMsg(NULL);
	//编译effect
	std::ifstream fin(filename, std::ios::binary);
	if (fin.fail()) 
	{
		MessageBox(0, L"open shader file error", L"tip", MB_OK);
		return E_FAIL;
	}
	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);
	fin.read(&compiledShader[0], size);
	fin.close();
	HRESULT hr = D3DX11CreateEffectFromMemory(&compiledShader[0], size,0,device_pancy,&fx_need);
	if(FAILED(hr))
	{
		MessageBox(NULL,L"CreateEffectFromMemory错误!",L"错误",MB_OK);
		return E_FAIL;
	}
	safe_release(shader);
	//创建输入顶点格式
	return S_OK;
}
HRESULT shader_basic::set_matrix(ID3DX11EffectMatrixVariable *mat_handle, XMFLOAT4X4 *mat_need)
{
	XMMATRIX rec_mat = XMLoadFloat4x4(mat_need);
	HRESULT hr;
	hr = mat_handle->SetMatrix(reinterpret_cast<float*>(&rec_mat));
	return hr;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~前向光照明部分~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
light_pre::light_pre(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{

}
void light_pre::init_handle()
{
	//纹理句柄
	texture_diffuse_handle = fx_need->GetVariableByName("texture_diffuse")->AsShaderResource();  //shader中的纹理资源句柄
	texture_normal_handle = fx_need->GetVariableByName("texture_normal")->AsShaderResource();    //法线贴图纹理
	texture_shadow_handle = fx_need->GetVariableByName("texture_shadow")->AsShaderResource();    //阴影贴图句柄
	texture_ssao_handle = fx_need->GetVariableByName("texture_ssao")->AsShaderResource();        //环境光贴图句柄
																								 //几何变换句柄
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();         //全套几何变换句柄
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();           //世界变换句柄
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();         //法线变换句柄
																							//texture_matrix_handle = fx_need->GetVariableByName("position_view")->AsMatrix();      //纹理变换句柄
	shadowmap_matrix_handle = fx_need->GetVariableByName("shadowmap_matrix")->AsMatrix();   //shadowmap矩阵变换句柄
	ssao_matrix_handle = fx_need->GetVariableByName("ssao_matrix")->AsMatrix();             //ssao矩阵变换句柄
																							//视点及材质
	BoneTransforms = fx_need->GetVariableByName("gBoneTransforms")->AsMatrix();
	view_pos_handle = fx_need->GetVariableByName("position_view");
	material_need = fx_need->GetVariableByName("material_need");
	//光照句柄
	light_list = fx_need->GetVariableByName("light_need");                   //灯光
}
void light_pre::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
HRESULT light_pre::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting view position", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_trans_world(XMFLOAT4X4 *mat_need)
{
	XMMATRIX rec_mat = XMLoadFloat4x4(mat_need);
	XMVECTOR x_delta;
	XMMATRIX check = rec_mat;
	//法线变换
	XMMATRIX normal_need = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&x_delta, check));
	normal_need.r[0].m128_f32[3] = 0.0f;
	normal_need.r[1].m128_f32[3] = 0.0f;
	normal_need.r[2].m128_f32[3] = 0.0f;
	normal_need.r[3].m128_f32[3] = 1.0f;
	HRESULT hr;
	hr = set_matrix(world_matrix_handle, mat_need);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting world matrix", L"tip", MB_OK);
		return hr;
	}
	hr = normal_matrix_handle->SetMatrix(reinterpret_cast<float*>(&normal_need));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting normal matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(project_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_trans_shadow(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(shadowmap_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting shadowmap matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_trans_ssao(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(ssao_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting ssao matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_bone_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr)) 
	{
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_material(pancy_material material_in)
{
	HRESULT hr = material_need->SetRawValue(&material_in, 0, sizeof(material_in));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting material", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_ssaotex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_ssao_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting ssao texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_shadowtex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_shadow_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting shadowmap texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_diffusetex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_diffuse_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting diffuse texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_normaltex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_normal_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting normal texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_light(pancy_light_basic light_need, int light_num)
{
	HRESULT hr = light_list->SetRawValue(&light_need, light_num * sizeof(light_need), sizeof(light_need));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting light", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void light_pre::release() 
{
	release_basic();
}
//简单的GUI
gui_simple::gui_simple(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
HRESULT gui_simple::set_mov_xy(XMFLOAT2 mov_xy)
{
	HRESULT hr = move_handle->SetRawValue((void*)&mov_xy, 0, sizeof(mov_xy));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting gui move", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT gui_simple::set_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting gui texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void gui_simple::release()
{
	release_basic();
}
void gui_simple::init_handle()
{
	//纹理句柄
	texture_handle = fx_need->GetVariableByName("texture_need")->AsShaderResource();  //shader中的纹理资源句柄
	move_handle = fx_need->GetVariableByName("move_screen");
}
void gui_simple::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//简单的3d拾取检测
find_clip::find_clip(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
void find_clip::init_handle()
{
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();         //全套几何变换句柄
}
void find_clip::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
HRESULT find_clip::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(project_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void find_clip::release()
{
	release_basic();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~屏幕空间局部反射~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ssr_reflect::ssr_reflect(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
void ssr_reflect::init_handle()
{
	view_pos_handle = fx_need->GetVariableByName("view_position");
	view_matrix_handle = fx_need->GetVariableByName("view_matrix")->AsMatrix();       //取景变换句柄	
	ViewToTexSpace = fx_need->GetVariableByName("gViewToTexSpace")->AsMatrix();
	FrustumCorners = fx_need->GetVariableByName("gFrustumCorners")->AsVector();
	invview_matrix_handle = fx_need->GetVariableByName("invview_matrix")->AsMatrix(); //取景变换逆变换句柄
	cubeview_matrix_handle = fx_need->GetVariableByName("view_matrix_cube")->AsMatrix();
	NormalDepthMap = fx_need->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	DepthMap = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();
	texture_diffuse_handle = fx_need->GetVariableByName("gcolorMap")->AsShaderResource();
	texture_cube_handle = fx_need->GetVariableByName("texture_cube")->AsShaderResource();
	//texture_depthcube_handle = fx_need->GetVariableByName("depth_cube")->AsShaderResource();
	texture_stencilcube_handle = fx_need->GetVariableByName("stencil_cube")->AsShaderResource();
	texture_color_mask = fx_need->GetVariableByName("mask_input")->AsShaderResource();;
	texture_color_ssr = fx_need->GetVariableByName("ssrcolor_input")->AsShaderResource();;
	camera_positions = fx_need->GetVariableByName("center_position")->AsVector();
}
void ssr_reflect::release()
{
	release_basic();
}
HRESULT ssr_reflect::set_ViewToTexSpace(XMFLOAT4X4 *mat)
{
	HRESULT hr = set_matrix(ViewToTexSpace, mat);
	if (FAILED(hr))
	{
		MessageBox(0, L"set viewtotex matrix error in rlr draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_view_matrix(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(view_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting view position", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_FrustumCorners(const XMFLOAT4 v[4])
{
	HRESULT hr = FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4);
	if (FAILED(hr))
	{
		MessageBox(0, L"set FrustumCorners error in rlr draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_camera_positions(XMFLOAT3 v)
{
	HRESULT hr = camera_positions->SetRawValue((void*)&v, 0, sizeof(v));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting view position", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_NormalDepthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = NormalDepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		MessageBox(0, L"set NormalDepthtex error in rlr draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_Depthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = DepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		MessageBox(0, L"set NormalDepthtex error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_diffusetex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_diffuse_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting diffuse texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_enviroment_tex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = texture_cube_handle->SetResource(srv);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting cube texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
/*
HRESULT ssr_reflect::set_enviroment_depth(ID3D11ShaderResourceView* srv)
{
HRESULT hr = texture_depthcube_handle->SetResource(srv);
if (hr != S_OK)
{
MessageBox(0, L"an error when setting cube depthtexture", L"tip", MB_OK);
return hr;
}
return S_OK;
}
*/
HRESULT ssr_reflect::set_color_mask_tex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = texture_color_mask->SetResource(srv);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting cube depthtexture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_color_ssr_tex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = texture_color_ssr->SetResource(srv);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting cube depthtexture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_enviroment_stencil(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = texture_stencilcube_handle->SetResource(srv);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting cube depthtexture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_invview_matrix(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(invview_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT ssr_reflect::set_cubeview_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = cubeview_matrix_handle->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void ssr_reflect::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~记录cubemap方向到alpha像素~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_save_cube::shader_save_cube(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_save_cube::set_texture_input(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_input->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set cube texture error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_save_cube::set_depthtex_input(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = depth_input->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set cube depth error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_save_cube::release()
{
	release_basic();
}
void shader_save_cube::init_handle()
{
	//纹理信息句柄
	texture_input = fx_need->GetVariableByName("tex_input")->AsShaderResource();
	depth_input = fx_need->GetVariableByName("depth_input")->AsShaderResource();
	cube_count_handle = fx_need->GetVariableByName("cube_count");
}
HRESULT shader_save_cube::set_cube_count(XMFLOAT3 cube_count)
{
	HRESULT hr = cube_count_handle->SetRawValue((void*)&cube_count, 0, sizeof(cube_count));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting cube_count_handle", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_save_cube::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~反射效果模糊~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_SSRblur::shader_SSRblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_SSRblur::set_tex_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		MessageBox(0, L"set SSR blur texture error in HDR mapping", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_SSRblur::set_image_size(XMFLOAT4 range)
{
	HRESULT hr;
	hr = Texelrange->SetRawValue((void*)&range, 0, sizeof(range));
	if (FAILED(hr))
	{
		MessageBox(0, L"set image_size error in SSR blur part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_SSRblur::release()
{
	release_basic();
}
void shader_SSRblur::init_handle()
{
	Texelrange = fx_need->GetVariableByName("tex_range_color_normal");
	tex_input = fx_need->GetVariableByName("gInputImage")->AsShaderResource();
	tex_normal_input = fx_need->GetVariableByName("normal_tex")->AsShaderResource();
	tex_depth_input = fx_need->GetVariableByName("depth_tex")->AsShaderResource();
	tex_mask_input = fx_need->GetVariableByName("gInputMask")->AsShaderResource();
}
void shader_SSRblur::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
HRESULT shader_SSRblur::set_tex_normal_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_normal_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		MessageBox(0, L"set SSR blur texture error in HDR mapping", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_SSRblur::set_tex_depth_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_depth_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		MessageBox(0, L"set SSR blur texture error in HDR mapping", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_SSRblur::set_tex_mask_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_mask_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		MessageBox(0, L"set SSR blur texture error in HDR mapping", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~最终反射叠加~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_reflectfinal::shader_reflectfinal(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
void shader_reflectfinal::release()
{
	release_basic();
}
void shader_reflectfinal::init_handle()
{
	Texelrange = fx_need->GetVariableByName("tex_range_color_normal");
	tex_color_input = fx_need->GetVariableByName("gInputImage")->AsShaderResource();
	tex_reflect_input = fx_need->GetVariableByName("gInputReflect")->AsShaderResource();
}
void shader_reflectfinal::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
HRESULT shader_reflectfinal::set_tex_color_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_color_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		MessageBox(0, L"set SSR color texture error in SSR final mapping", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_reflectfinal::set_tex_reflect_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_reflect_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		MessageBox(0, L"set SSR reflect texture error in SSR final mapping", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_reflectfinal::set_image_size(XMFLOAT4 range)
{
	HRESULT hr;
	hr = Texelrange->SetRawValue((void*)&range, 0, sizeof(range));
	if (FAILED(hr))
	{
		MessageBox(0, L"set image_size error in SSR final part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~全局shader管理器~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_control::shader_control()
{
	shader_light_pre = NULL;
	shader_GUI = NULL;
	shader_find_clip = NULL;
	shader_ssreflect = NULL;
	shader_reset_alpha = NULL;
	shader_reflect_blur = NULL;
	shader_reflect_final = NULL;
}
HRESULT shader_control::shader_init(ID3D11Device *device_pancy, ID3D11DeviceContext *contex_pancy)
{
	HRESULT hr;
	shader_light_pre = new light_pre(L"light_pre.cso", device_pancy, contex_pancy);
	hr = shader_light_pre->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when pre lighting shader created", L"tip", MB_OK);
		return hr;
	}
	shader_GUI = new gui_simple(L"simplegui.cso", device_pancy, contex_pancy);
	hr = shader_GUI->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when gui shader created", L"tip", MB_OK);
		return hr;
	}
	shader_find_clip = new find_clip(L"find_clip.cso", device_pancy, contex_pancy);
	hr = shader_find_clip->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when gui shader created", L"tip", MB_OK);
		return hr;
	}
	shader_ssreflect = new ssr_reflect(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\SSR.cso", device_pancy, contex_pancy);
	hr = shader_ssreflect->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when ssreflect shader created", L"tip", MB_OK);
		return hr;
	}
	shader_reset_alpha = new shader_save_cube(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\reset_cube_alpha.cso", device_pancy, contex_pancy);
	hr = shader_reset_alpha->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when save_cube shader created", L"tip", MB_OK);
		return hr;
	}
	shader_reflect_blur = new shader_SSRblur(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\reflect_blur.cso", device_pancy, contex_pancy);
	hr = shader_reflect_blur->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when shader_reflect_blur created", L"tip", MB_OK);
		return hr;
	}
	shader_reflect_final = new shader_reflectfinal(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\reflect_final_pass.cso", device_pancy, contex_pancy);
	hr = shader_reflect_final->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when shader_reflect_final created", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_control::release()
{
	shader_GUI->release();
	shader_light_pre->release();
	shader_find_clip->release();
	shader_ssreflect->release();
	shader_reset_alpha->release();
	shader_reflect_blur->release();
	shader_reflect_final->release();
}
