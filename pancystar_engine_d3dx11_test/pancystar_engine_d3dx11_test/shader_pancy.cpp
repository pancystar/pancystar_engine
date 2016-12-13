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
	shadowmap_matrix_handle = fx_need->GetVariableByName("shadowmap_matrix")->AsMatrix();   //shadowmap矩阵变换句柄
	ssao_matrix_handle = fx_need->GetVariableByName("ssao_matrix")->AsMatrix();             //ssao矩阵变换句柄
																							//视点及材质
	BoneTransforms = fx_need->GetVariableByName("gBoneTransforms")->AsMatrix();
	view_pos_handle = fx_need->GetVariableByName("position_view");
	material_need = fx_need->GetVariableByName("material_need");
	//光照句柄
	light_list = fx_need->GetVariableByName("light_need");                   //灯光
	light_num_handle = fx_need->GetVariableByName("light_num");                       //光源数量句柄
	shadow_num_handle = fx_need->GetVariableByName("shadow_num");                     //阴影数量句柄
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
HRESULT light_pre::set_shadow_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = shadowmap_matrix_handle->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
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
HRESULT light_pre::set_light_num(XMUINT3 all_light_num)
{
	HRESULT hr = light_num_handle->SetRawValue((void*)&all_light_num, 0, sizeof(all_light_num));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting light num", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_pre::set_shadow_num(XMUINT3 all_light_num)
{
	HRESULT hr = shadow_num_handle->SetRawValue((void*)&all_light_num, 0, sizeof(all_light_num));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting light num", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void light_pre::release() 
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~shadow map部分~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
light_shadow::light_shadow(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
HRESULT light_shadow::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(project_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_shadow::set_bone_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT light_shadow::set_texture(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr;
	hr = texture_need->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set tex_normaldepth error in ssao depth normal part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void light_shadow::init_handle()
{
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();         //全套几何变换句柄
	texture_need = fx_need->GetVariableByName("texture_diffuse")->AsShaderResource();
	BoneTransforms = fx_need->GetVariableByName("gBoneTransforms")->AsMatrix();
}
void light_shadow::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
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
void light_shadow::release()
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao 深度法线记录部分~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_gbufferdepthnormal_map::shader_gbufferdepthnormal_map(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
void shader_gbufferdepthnormal_map::init_handle()
{
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
	BoneTransforms = fx_need->GetVariableByName("gBoneTransforms")->AsMatrix();
	texture_need = fx_need->GetVariableByName("texture_diffuse")->AsShaderResource();
	texture_normal = fx_need->GetVariableByName("texture_normal")->AsShaderResource();
}
HRESULT shader_gbufferdepthnormal_map::set_trans_world(XMFLOAT4X4 *mat_world, XMFLOAT4X4 *mat_view)
{
	XMVECTOR x_delta;
	XMMATRIX world_need = XMLoadFloat4x4(mat_world);
	XMMATRIX view_need = XMLoadFloat4x4(mat_view);
	XMMATRIX normal_need = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&x_delta, world_need));
	normal_need.r[0].m128_f32[3] = 0;
	normal_need.r[1].m128_f32[3] = 0;
	normal_need.r[2].m128_f32[3] = 0;
	normal_need.r[3].m128_f32[3] = 1;
	HRESULT hr;
	hr = world_matrix_handle->SetMatrix(reinterpret_cast<float*>(&(world_need*view_need)));
	if (FAILED(hr))
	{
		MessageBox(0, L"set world matrix error in ssao depthnormal part", L"tip", MB_OK);
		return hr;
	}
	hr = normal_matrix_handle->SetMatrix(reinterpret_cast<float*>(&(normal_need*view_need)));
	if (FAILED(hr))
	{
		MessageBox(0, L"set view matrix error in ssao depthnormal part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_gbufferdepthnormal_map::set_trans_all(XMFLOAT4X4 *mat_final) 
{
	HRESULT hr;
	hr = set_matrix(project_matrix_handle, mat_final);
	if (FAILED(hr))
	{
		MessageBox(0, L"set final matrix error in ssao depthnormal part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_gbufferdepthnormal_map::set_texture(ID3D11ShaderResourceView *tex_in) 
{
	HRESULT hr;
	hr = texture_need->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set tex_normaldepth error in ssao depth normal part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_gbufferdepthnormal_map::set_texture_normal(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr;
	hr = texture_normal->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set tex_normaldepth error in ssao depth normal part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_gbufferdepthnormal_map::set_bone_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void shader_gbufferdepthnormal_map::release()
{
	release_basic();
}
void shader_gbufferdepthnormal_map::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao 遮蔽渲染部分~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ssaomap::shader_ssaomap(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
void shader_ssaomap::init_handle()
{
	ViewToTexSpace = fx_need->GetVariableByName("gViewToTexSpace")->AsMatrix();
	OffsetVectors = fx_need->GetVariableByName("gOffsetVectors")->AsVector();
	FrustumCorners = fx_need->GetVariableByName("gFrustumCorners")->AsVector();

	NormalDepthMap = fx_need->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	DepthMap = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();
	RandomVecMap = fx_need->GetVariableByName("gRandomVecMap")->AsShaderResource();
}
void shader_ssaomap::release()
{
	release_basic();
}
HRESULT shader_ssaomap::set_ViewToTexSpace(XMFLOAT4X4 *mat)
{
	HRESULT hr = set_matrix(ViewToTexSpace, mat);
	if (FAILED(hr))
	{
		MessageBox(0, L"set viewtotex matrix error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaomap::set_FrustumCorners(const XMFLOAT4 v[4])
{
	HRESULT hr = FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4);
	if (FAILED(hr))
	{
		MessageBox(0, L"set FrustumCorners error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaomap::set_OffsetVectors(const XMFLOAT4 v[14])
{
	HRESULT hr = OffsetVectors->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 14);
	if (FAILED(hr))
	{
		MessageBox(0, L"set OffsetVectors error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaomap::set_NormalDepthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = NormalDepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		MessageBox(0, L"set NormalDepthtex error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaomap::set_Depthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = DepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		MessageBox(0, L"set NormalDepthtex error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaomap::set_randomtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = RandomVecMap->SetResource(srv);
	if (FAILED(hr))
	{
		MessageBox(0, L"set randomtex error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_ssaomap::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ssao ao图模糊处理部分~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_ssaoblur::shader_ssaoblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
void shader_ssaoblur::release()
{
	release_basic();
}
void shader_ssaoblur::init_handle()
{
	TexelWidth = fx_need->GetVariableByName("gTexelWidth")->AsScalar();
	TexelHeight = fx_need->GetVariableByName("gTexelHeight")->AsScalar();

	NormalDepthMap = fx_need->GetVariableByName("gNormalDepthMap")->AsShaderResource();
	DepthMap = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();
	InputImage = fx_need->GetVariableByName("gInputImage")->AsShaderResource();
}
HRESULT shader_ssaoblur::set_image_size(float width, float height)
{
	HRESULT hr;
	hr = TexelWidth->SetFloat(width);
	if (FAILED(hr))
	{
		MessageBox(0, L"set image_size error in ssao blur part", L"tip", MB_OK);
		return hr;
	}
	hr = TexelHeight->SetFloat(height);
	if (FAILED(hr))
	{
		MessageBox(0, L"set image_size error in ssao blur part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaoblur::set_tex_resource(ID3D11ShaderResourceView* tex_normaldepth, ID3D11ShaderResourceView* tex_aomap)
{
	HRESULT hr;
	hr = NormalDepthMap->SetResource(tex_normaldepth);
	if (FAILED(hr))
	{
		MessageBox(0, L"set tex_normaldepth error in ssao blur part", L"tip", MB_OK);
		return hr;
	}
	hr = InputImage->SetResource(tex_aomap);
	if (FAILED(hr))
	{
		MessageBox(0, L"set tex_aomap error in ssao blur part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_ssaoblur::set_Depthtex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = DepthMap->SetResource(srv);
	if (FAILED(hr))
	{
		MessageBox(0, L"set NormalDepthtex error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_ssaoblur::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~cube mapping~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_reflect::shader_reflect(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_reflect::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting view position", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_reflect::set_trans_world(XMFLOAT4X4 *mat_need)
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
HRESULT shader_reflect::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(project_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_reflect::set_tex_resource(ID3D11ShaderResourceView* tex_cube)
{
	HRESULT hr;
	hr = cubemap_texture->SetResource(tex_cube);
	if (FAILED(hr))
	{
		MessageBox(0, L"set cube texture error in cube mapping", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_reflect::init_handle()
{
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();         //全套几何变换句柄
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();           //世界变换句柄
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();         //法线变换句柄
	view_pos_handle = fx_need->GetVariableByName("position_view");
	cubemap_texture = fx_need->GetVariableByName("texture_cube")->AsShaderResource();  //shader中的纹理资源句柄
}
void shader_reflect::release()
{
	release_basic();
}
void shader_reflect::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR亮度平均~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
compute_averagelight::compute_averagelight(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
HRESULT compute_averagelight::set_compute_tex(ID3D11ShaderResourceView *tex_input)
{
	HRESULT hr = texture_input->SetResource(tex_input);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when setting HDR texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT compute_averagelight::set_compute_buffer(ID3D11UnorderedAccessView *buffer_input_need, ID3D11UnorderedAccessView *buffer_output_need)
{
	HRESULT hr;
	hr = buffer_input->SetUnorderedAccessView(buffer_input_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"set UAV buffer error", L"tip", MB_OK);
		return hr;
	}
	hr = buffer_output->SetUnorderedAccessView(buffer_output_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"set UAV buffer error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT compute_averagelight::set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth)
{
	XMUINT4 rec_float = XMUINT4(static_cast<unsigned int>(width_need), static_cast<unsigned int>(height_need), static_cast<unsigned int>(buffer_num), static_cast<unsigned int>(bytewidth));
	HRESULT hr = texture_range->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when setting HDR range", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void compute_averagelight::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
}
void compute_averagelight::release()
{
	release_basic();
}
void compute_averagelight::init_handle()
{
	texture_input = fx_need->GetVariableByName("input_tex")->AsShaderResource();
	buffer_input = fx_need->GetVariableByName("input_buffer")->AsUnorderedAccessView();
	buffer_output = fx_need->GetVariableByName("output_buffer")->AsUnorderedAccessView();
	texture_range = fx_need->GetVariableByName("input_range");
}
void compute_averagelight::dispatch(int width_need,int height_need,int final_need,int map_need)
{
	ID3DX11EffectTechnique* tech_need;
	tech_need = fx_need->GetTechniqueByName("HDR_average_pass");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	for (UINT i = 0; i<techDesc.Passes; ++i)
	{
		tech_need->GetPassByIndex(i)->Apply(0, contex_pancy);
		if (i == 0) 
		{
			contex_pancy->Dispatch(width_need, height_need, 1);
		}
		else if(i == 1)
		{
			contex_pancy->Dispatch(final_need, 1, 1);
		}
		else 
		{
			contex_pancy->Dispatch(map_need, 1, 1);
		}
	}
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	contex_pancy->CSSetShaderResources(0, 1, nullSRV);
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	contex_pancy->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	contex_pancy->CSSetShader(0, 0, 0);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_高光提取~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_HDRpreblur::shader_HDRpreblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_HDRpreblur::set_buffer_input(ID3D11ShaderResourceView *buffer_need, ID3D11ShaderResourceView *tex_need)
{
	HRESULT hr = tex_input->SetResource(tex_need);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting HDR preblur tex", L"tip", MB_OK);
		return hr;
	}

	hr = buffer_input->SetResource(buffer_need);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting HDR preblur buffer", L"tip", MB_OK);
		return hr;
	}
	XMFLOAT4X4 yuv_rgb = XMFLOAT4X4
		(1.0f, 1.0f, 1.0f, 0.0f,
			0.0f, -0.3441f, 1.7720f, 0.0f,
			1.4020f, -0.7141f, 0.0f, 0.0f,
			-0.7010f, 0.5291f, -0.8860f, 1.0f);
	hr = set_matrix(matrix_YUV2RGB, &yuv_rgb);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting YUV2RGB matrix", L"tip", MB_OK);
		return hr;
	}
	XMFLOAT4X4 rgb_yuv = XMFLOAT4X4(0.2990f, -0.1687f, 0.5f, 0.0f,
		0.5870, -0.3313f, -0.4187f, 0.0f,
		0.1140, 0.5f, -0.0813f, 0.0f,
		0.0f, 0.5f, 0.5f, 1.0f);
	hr = set_matrix(matrix_RGB2YUV, &rgb_yuv);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting RGB2YUV matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_HDRpreblur::set_lum_message(float average_lum, float HighLight_divide, float HightLight_max, float key_tonemapping)
{
	XMFLOAT4 rec_float = XMFLOAT4(average_lum, HighLight_divide, HightLight_max, key_tonemapping);
	HRESULT hr = lum_message->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when setting HDR lum_message in preblur pass", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_HDRpreblur::set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth)
{
	XMUINT4 rec_float = XMUINT4(static_cast<unsigned int>(width_need), static_cast<unsigned int>(height_need), static_cast<unsigned int>(buffer_num), static_cast<unsigned int>(bytewidth));
	HRESULT hr = texture_range->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when setting HDR range", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_HDRpreblur::init_handle()
{
	matrix_YUV2RGB = fx_need->GetVariableByName("YUV2RGB")->AsMatrix();
	matrix_RGB2YUV = fx_need->GetVariableByName("RGB2YUV")->AsMatrix();
	tex_input = fx_need->GetVariableByName("input_tex")->AsShaderResource();
	lum_message = fx_need->GetVariableByName("light_average");
	texture_range = fx_need->GetVariableByName("input_range");
	buffer_input = fx_need->GetVariableByName("input_buffer")->AsShaderResource();
}
void shader_HDRpreblur::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
void shader_HDRpreblur::release() 
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_高光模糊~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_HDRblur::shader_HDRblur(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_HDRblur::set_tex_resource(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr;
	hr = tex_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		MessageBox(0, L"set HDR blur texture error in HDR mapping", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_HDRblur::set_image_size(float width, float height)
{
	HRESULT hr;
	hr = TexelWidth->SetFloat(width);
	if (FAILED(hr))
	{
		MessageBox(0, L"set image_size error in HDR blur part", L"tip", MB_OK);
		return hr;
	}
	hr = TexelHeight->SetFloat(height);
	if (FAILED(hr))
	{
		MessageBox(0, L"set image_size error in HDR blur part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_HDRblur::release()
{
	release_basic();
}
void shader_HDRblur::init_handle()
{
	TexelWidth = fx_need->GetVariableByName("gTexelWidth")->AsScalar();
	TexelHeight = fx_need->GetVariableByName("gTexelHeight")->AsScalar();
	tex_input = fx_need->GetVariableByName("gInputImage")->AsShaderResource();
}
void shader_HDRblur::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~HDR_最终渲染~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_HDRfinal::shader_HDRfinal(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_HDRfinal::set_tex_resource(ID3D11ShaderResourceView *tex_input_need, ID3D11ShaderResourceView *tex_bloom_need, ID3D11ShaderResourceView *buffer_need)
{
	HRESULT hr;
	hr = tex_input->SetResource(tex_input_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"set HDR blur texture error in HDR final mapping", L"tip", MB_OK);
		return hr;
	}
	hr = tex_bloom->SetResource(tex_bloom_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"set HDR blur texture error in HDR final mapping", L"tip", MB_OK);
		return hr;
	}
	hr = buffer_input->SetResource(buffer_need);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting HDR preblur buffer", L"tip", MB_OK);
		return hr;
	}
	XMFLOAT4X4 yuv_rgb = XMFLOAT4X4
		(1.0f, 1.0f, 1.0f, 0.0f,
			0.0f, -0.3441f, 1.7720f, 0.0f,
			1.4020f, -0.7141f, 0.0f, 0.0f,
			-0.7010f, 0.5291f, -0.8860f, 1.0f);
	hr = set_matrix(matrix_YUV2RGB, &yuv_rgb);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting YUV2RGB matrix", L"tip", MB_OK);
		return hr;
	}
	XMFLOAT4X4 rgb_yuv = XMFLOAT4X4(0.2990f, -0.1687f, 0.5f, 0.0f,
		0.5870, -0.3313f, -0.4187f, 0.0f,
		0.1140, 0.5f, -0.0813f, 0.0f,
		0.0f, 0.5f, 0.5f, 1.0f);
	hr = set_matrix(matrix_RGB2YUV, &rgb_yuv);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting RGB2YUV matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_HDRfinal::set_lum_message(float average_lum, float HighLight_divide, float HightLight_max, float key_tonemapping)
{
	XMFLOAT4 rec_float = XMFLOAT4(average_lum, HighLight_divide, HightLight_max, key_tonemapping);
	HRESULT hr = lum_message->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when setting HDR lum_message in preblur pass", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_HDRfinal::set_piccturerange(int width_need, int height_need, int buffer_num, int bytewidth)
{
	XMUINT4 rec_float = XMUINT4(static_cast<unsigned int>(width_need), static_cast<unsigned int>(height_need), static_cast<unsigned int>(buffer_num), static_cast<unsigned int>(bytewidth));
	HRESULT hr = texture_range->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when setting HDR range", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_HDRfinal::release()
{
	release_basic();
}
void shader_HDRfinal::init_handle()
{
	lum_message = fx_need->GetVariableByName("light_average");
	matrix_YUV2RGB = fx_need->GetVariableByName("YUV2RGB")->AsMatrix();
	matrix_RGB2YUV = fx_need->GetVariableByName("RGB2YUV")->AsMatrix();
	tex_input = fx_need->GetVariableByName("input_tex")->AsShaderResource();
	tex_bloom = fx_need->GetVariableByName("input_bloom")->AsShaderResource();
	texture_range = fx_need->GetVariableByName("input_range");
	buffer_input = fx_need->GetVariableByName("input_buffer")->AsShaderResource();
}
void shader_HDRfinal::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~粒子着色~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_particle::shader_particle(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
void shader_particle::init_handle()
{
	//纹理信息句柄
	texture_handle = fx_need->GetVariableByName("texture_first")->AsShaderResource();
	RandomTex_handle = fx_need->GetVariableByName("texture_random")->AsShaderResource();
	//几何变换信息句柄
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
	view_pos_handle = fx_need->GetVariableByName("position_view");
	//粒子产生信息
	start_position_handle = fx_need->GetVariableByName("position_start");
	start_direction_handle = fx_need->GetVariableByName("direction_start");
	//动画时间
	time_game_handle = fx_need->GetVariableByName("game_time")->AsScalar();
	time_delta_handle = fx_need->GetVariableByName("delta_time")->AsScalar();
}
HRESULT shader_particle::set_viewposition(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (FAILED(hr))
	{
		MessageBox(0, L"set particle view position error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_particle::set_startposition(XMFLOAT3 start_pos)
{
	HRESULT hr = start_position_handle->SetRawValue((void*)&start_pos, 0, sizeof(start_pos));
	if (FAILED(hr))
	{
		MessageBox(0, L"set particle position error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_particle::set_startdirection(XMFLOAT3 start_dir)
{
	HRESULT hr = start_direction_handle->SetRawValue((void*)&start_dir, 0, sizeof(start_dir));
	if (FAILED(hr))
	{
		MessageBox(0, L"set particle direction error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_particle::set_frametime(float game_time, float delta_time)
{
	HRESULT hr = time_game_handle->SetFloat(game_time);
	if (FAILED(hr))
	{
		MessageBox(0, L"set particle time error", L"tip", MB_OK);
		return hr;
	}
	hr = time_delta_handle->SetFloat(delta_time);
	if (FAILED(hr))
	{
		MessageBox(0, L"set particle time error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_particle::set_randomtex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = RandomTex_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set particle random matrix error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_particle::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(project_matrix_handle, mat_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"set particle matrix error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_particle::set_texture(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_handle->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set particle texture error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_particle::release()
{
	release_basic();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~火焰粒子~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_fire::shader_fire(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_particle(filename, device_need, contex_need)
{
}
void shader_fire::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TYPE",     0, DXGI_FORMAT_R32_UINT,        0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~阴影体~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_shadow_volume::shader_shadow_volume(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_shadow_volume::set_trans_world(XMFLOAT4X4 *mat_need)
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
HRESULT shader_shadow_volume::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(project_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_shadow_volume::set_light_pos(XMFLOAT3 light_pos)
{
	HRESULT hr = position_light_handle->SetRawValue((void*)&light_pos, 0, sizeof(light_pos));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting view position", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_shadow_volume::set_light_dir(XMFLOAT3 light_dir)
{
	HRESULT hr = direction_light_handle->SetRawValue((void*)&light_dir, 0, sizeof(light_dir));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting view position", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_shadow_volume::release()
{
	release_basic();
}
void shader_shadow_volume::init_handle()
{
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();  //世界变换句柄
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();//法线变换句柄
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();//全局变换句柄
	position_light_handle = fx_need->GetVariableByName("position_light");
	direction_light_handle = fx_need->GetVariableByName("direction_light");
}
void shader_shadow_volume::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~阴影体绘制~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_shadow_volume_draw::shader_shadow_volume_draw(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_shadow_volume_draw::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(project_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_shadow_volume_draw::release()
{
	release_basic();
}
void shader_shadow_volume_draw::init_handle()
{
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();           //世界变换句柄
}
void shader_shadow_volume_draw::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~草地公告板~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_grass::shader_grass(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_grass::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(project_matrix_handle, mat_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"set particle matrix error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_grass::set_texture_diffuse(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_need->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set billboard texture error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_grass::set_texture_normal(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_normal->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set billboard texture error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_grass::set_texture_specular(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_specular->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set billboard texture error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_grass::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting view position", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_grass::release()
{
	release_basic();
}
void shader_grass::init_handle()
{
	//纹理信息句柄
	texture_need = fx_need->GetVariableByName("texture_first")->AsShaderResource();
	texture_normal = fx_need->GetVariableByName("texture_normal")->AsShaderResource();
	texture_specular = fx_need->GetVariableByName("texture_specular")->AsShaderResource();
	//几何变换信息句柄
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
	view_pos_handle = fx_need->GetVariableByName("position_view");
}
void shader_grass::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MSAA深度模板缓冲重采样~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_resolvedepth::shader_resolvedepth(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_resolvedepth::set_texture_MSAA(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_MSAA->SetResource(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set billboard texture error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_resolvedepth::set_projmessage(XMFLOAT3 proj_message)
{
	HRESULT hr = projmessage_handle->SetRawValue((void*)&proj_message, 0, sizeof(proj_message));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting view position", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_resolvedepth::release()
{
	release_basic();
}
void shader_resolvedepth::init_handle()
{
	//纹理信息句柄
	texture_MSAA = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();
	//几何变换信息句柄
	projmessage_handle = fx_need->GetVariableByName("proj_desc");
}
void shader_resolvedepth::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~延迟光照算法光照缓冲区~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
light_defered_lightbuffer::light_defered_lightbuffer(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
HRESULT light_defered_lightbuffer::set_light(pancy_light_basic light_need, int light_num)
{
	HRESULT hr = light_list->SetRawValue(&light_need, light_num * sizeof(light_need), sizeof(light_need));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting light", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_lightbuffer::set_FrustumCorners(const XMFLOAT4 v[4])
{
	HRESULT hr = FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4);
	if (FAILED(hr))
	{
		MessageBox(0, L"set FrustumCorners error in ssao draw part", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_lightbuffer::set_shadow_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = shadow_matrix_handle->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_lightbuffer::set_view_matrix(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(view_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_lightbuffer::set_invview_matrix(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(invview_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}

HRESULT light_defered_lightbuffer::set_light_num(XMUINT3 all_light_num)
{
	HRESULT hr = light_num_handle->SetRawValue((void*)&all_light_num, 0, sizeof(all_light_num));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting light num", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_lightbuffer::set_shadow_num(XMUINT3 all_light_num)
{
	HRESULT hr = shadow_num_handle->SetRawValue((void*)&all_light_num, 0, sizeof(all_light_num));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting light num", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_lightbuffer::set_Normalspec_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = NormalspecMap->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting ssao texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_lightbuffer::set_DepthMap_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = DepthMap->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting ssao texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_lightbuffer::set_shadow_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_shadow->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting ssao texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void light_defered_lightbuffer::release()
{
	release_basic();
}
void light_defered_lightbuffer::init_handle()
{
	shadow_matrix_handle = fx_need->GetVariableByName("shadowmap_matrix")->AsMatrix();//阴影变换句柄
	view_matrix_handle = fx_need->GetVariableByName("view_matrix")->AsMatrix();       //取景变换句柄	
	invview_matrix_handle = fx_need->GetVariableByName("invview_matrix")->AsMatrix(); //取景变换逆变换句柄
	light_list = fx_need->GetVariableByName("light_need");                            //光照句柄
	light_num_handle = fx_need->GetVariableByName("light_num");                       //光源数量句柄
	shadow_num_handle = fx_need->GetVariableByName("shadow_num");                     //阴影数量句柄
	FrustumCorners = fx_need->GetVariableByName("gFrustumCorners")->AsVector();       //3D还原角点句柄

	NormalspecMap = fx_need->GetVariableByName("gNormalspecMap")->AsShaderResource();  //shader中的纹理资源句柄
	DepthMap = fx_need->GetVariableByName("gdepth_map")->AsShaderResource();  //shader中的纹理资源句柄
	texture_shadow = fx_need->GetVariableByName("texture_shadow")->AsShaderResource();  //shader中的纹理资源句柄
}
void light_defered_lightbuffer::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~延迟光照算法最终渲染~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
light_defered_draw::light_defered_draw(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) : shader_basic(filename, device_need, contex_need)
{
}
HRESULT light_defered_draw::set_view_pos(XMFLOAT3 eye_pos)
{
	HRESULT hr = view_pos_handle->SetRawValue((void*)&eye_pos, 0, sizeof(eye_pos));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting view position", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_trans_world(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(world_matrix_handle, mat_need);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting world matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_trans_ssao(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(ssao_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting ssao matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(final_matrix_handle, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_bone_matrix(const XMFLOAT4X4* M, int cnt)
{
	HRESULT hr = BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(M), 0, cnt);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_material(pancy_material material_in)
{
	HRESULT hr = material_need->SetRawValue(&material_in, 0, sizeof(material_in));
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting material", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_ssaotex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_ssao_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting ssao texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_diffusetex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = texture_diffuse_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting diffuse texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_diffuse_light_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = tex_light_diffuse_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting diffuse texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_terainbumptex(ID3D11ShaderResourceView *tex_in) 
{
	HRESULT hr = texture_terainbump_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting terainbump texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_teraintex(ID3D11ShaderResourceView *tex_in) 
{
	HRESULT hr = texture_terain_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting terain texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_specular_light_tex(ID3D11ShaderResourceView *tex_in)
{
	HRESULT hr = tex_light_specular_handle->SetResource(tex_in);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting diffuse texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT light_defered_draw::set_enviroment_tex(ID3D11ShaderResourceView* srv)
{
	HRESULT hr = texture_cube_handle->SetResource(srv);
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting cube texture", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void light_defered_draw::release()
{
	release_basic();
}
void light_defered_draw::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
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
void light_defered_draw::init_handle()
{
	texture_diffuse_handle = fx_need->GetVariableByName("texture_diffuse")->AsShaderResource();  //shader中的纹理资源句柄
	tex_light_diffuse_handle = fx_need->GetVariableByName("texture_light_diffuse")->AsShaderResource();    //法线贴图纹理
	tex_light_specular_handle = fx_need->GetVariableByName("texture_light_specular")->AsShaderResource();    //阴影贴图句柄
	texture_ssao_handle = fx_need->GetVariableByName("texture_ssao")->AsShaderResource();        //环境光贴图句柄
	texture_cube_handle = fx_need->GetVariableByName("texture_cube")->AsShaderResource();

	texture_terainbump_handle = fx_need->GetVariableByName("texture_terrain_bump")->AsShaderResource();
	texture_terain_handle = fx_need->GetVariableByName("texture_terrain_diffuse")->AsShaderResource();
	//几何变换句柄
	view_pos_handle = fx_need->GetVariableByName("position_view");
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();           //世界变换句柄
	final_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();         //全套几何变换句柄
	ssao_matrix_handle = fx_need->GetVariableByName("ssao_matrix")->AsMatrix();             //ssao矩阵变换句柄
	BoneTransforms = fx_need->GetVariableByName("gBoneTransforms")->AsMatrix();
	material_need = fx_need->GetVariableByName("material_need");
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
	//shader_light_deferred = NULL;
	shader_shadowmap = NULL;
	shader_cubemap = NULL;
	shader_gbuffer_depthnormal = NULL;
	shader_ssao_draw = NULL;
	shader_ssao_blur = NULL;
	particle_fire = NULL;
	shader_HDR_average = NULL;
	shader_HDR_preblur = NULL;
	shader_HDR_blur = NULL;
	shader_HDR_final = NULL;
	shader_shadowvolume = NULL;
	shader_shadowvolume_draw = NULL;
	shader_grass_billboard = NULL;
	shader_resolve_depthstencil = NULL;
	shader_light_deffered_lbuffer = NULL;
	shader_light_deffered_draw = NULL;
	shader_ssreflect = NULL;
	shader_reset_alpha = NULL;
	shader_reflect_blur = NULL;
	shader_reflect_final = NULL;
}
HRESULT shader_control::shader_init(ID3D11Device *device_pancy, ID3D11DeviceContext *contex_pancy)
{
	HRESULT hr;
	shader_light_pre = new light_pre(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\light_pre.cso", device_pancy, contex_pancy);
	hr = shader_light_pre->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when pre lighting shader created", L"tip", MB_OK);
		return hr;
	}
	shader_light_deffered_lbuffer = new light_defered_lightbuffer(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\light_defferd_lightbuffer.cso", device_pancy, contex_pancy);
	hr = shader_light_deffered_lbuffer->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when deffered lighting shader created", L"tip", MB_OK);
		return hr;
	}
	shader_light_deffered_draw = new light_defered_draw(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\light_defferd_darw.cso", device_pancy, contex_pancy);
	hr = shader_light_deffered_draw->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when deffered lighting shader created", L"tip", MB_OK);
		return hr;
	}
	shader_shadowmap = new light_shadow(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\shadowmap.cso", device_pancy, contex_pancy);
	hr = shader_shadowmap->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when shadowmap shader created", L"tip", MB_OK);
		return hr;
	}
	
	shader_gbuffer_depthnormal = new shader_gbufferdepthnormal_map(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\gbuffer_normaldepth_map.cso", device_pancy, contex_pancy);
	hr = shader_gbuffer_depthnormal->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when ssao_depthnormal shader created", L"tip", MB_OK);
		return hr;
	}
	
	shader_ssao_draw = new shader_ssaomap(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\ssao_draw_aomap.cso", device_pancy, contex_pancy);
	hr = shader_ssao_draw->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when shader_ssaomap shader created", L"tip", MB_OK);
		return hr;
	}

	shader_ssao_blur = new shader_ssaoblur(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\ssao_blur_map.cso", device_pancy, contex_pancy);
	hr = shader_ssao_blur->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when shader_ssaoblur shader created", L"tip", MB_OK);
		return hr;
	}
	
	shader_cubemap = new shader_reflect(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\light_reflect.cso", device_pancy, contex_pancy);
	hr = shader_cubemap->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when reflect lighting shader created", L"tip", MB_OK);
		return hr;
	}

	shader_HDR_average = new compute_averagelight(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\HDR_average_pass.cso", device_pancy, contex_pancy);
	hr = shader_HDR_average->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when HDR average shader created", L"tip", MB_OK);
		return hr;
	}
	shader_HDR_preblur = new shader_HDRpreblur(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\HDR_preblur_pass.cso", device_pancy, contex_pancy);
	hr = shader_HDR_preblur->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when HDR preblur shader created", L"tip", MB_OK);
		return hr;
	}
	shader_HDR_blur = new shader_HDRblur(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\HDR_blur_pass.cso", device_pancy, contex_pancy);
	hr = shader_HDR_blur->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when HDR blur shader created", L"tip", MB_OK);
		return hr;
	}
	shader_HDR_final = new shader_HDRfinal(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\HDR_final.cso", device_pancy, contex_pancy);
	hr = shader_HDR_final->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when HDR final shader created", L"tip", MB_OK);
		return hr;
	}

	particle_fire = new shader_fire(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\fire.cso", device_pancy, contex_pancy);
	hr = particle_fire->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when fire particle shader created", L"tip", MB_OK);
		return hr;
	}

	shader_shadowvolume = new shader_shadow_volume(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\shadow_volume.cso", device_pancy, contex_pancy);
	hr = shader_shadowvolume->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when shadow volume shader created", L"tip", MB_OK);
		return hr;
	}
	shader_shadowvolume_draw = new shader_shadow_volume_draw(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\shadow_volume_draw.cso", device_pancy, contex_pancy);
	hr = shader_shadowvolume_draw->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when shadow volume shader created", L"tip", MB_OK);
		return hr;
	}
	shader_grass_billboard = new shader_grass(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\cross_grass.cso", device_pancy, contex_pancy);
	hr = shader_grass_billboard->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when grass billboard shader created", L"tip", MB_OK);
		return hr;
	}
	shader_resolve_depthstencil = new shader_resolvedepth(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\ResolveMSAAdepthstencil.cso", device_pancy, contex_pancy);
	hr = shader_resolve_depthstencil->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when grass resolve_depthstencil shader created", L"tip", MB_OK);
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
	shader_light_pre->release();
	shader_light_deffered_lbuffer->release();
	shader_shadowmap->release();
	shader_shadowvolume->release();
	shader_shadowvolume_draw->release();
	shader_gbuffer_depthnormal->release();
	shader_ssao_draw->release();
	shader_ssao_blur->release();
	shader_cubemap->release();
	shader_HDR_average->release();
	shader_HDR_preblur->release();
	shader_HDR_blur->release();
	shader_HDR_final->release();
	particle_fire->release();
	shader_grass_billboard->release();
	shader_resolve_depthstencil->release();
	shader_light_deffered_draw->release();
	shader_ssreflect->release();
	shader_reset_alpha->release();
	shader_reflect_blur->release();
	shader_reflect_final->release();
}
