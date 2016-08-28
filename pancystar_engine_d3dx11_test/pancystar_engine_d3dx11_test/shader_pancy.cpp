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
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
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
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
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
shader_ssaodepthnormal_map::shader_ssaodepthnormal_map(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
void shader_ssaodepthnormal_map::init_handle()
{
	world_matrix_handle = fx_need->GetVariableByName("world_matrix")->AsMatrix();
	normal_matrix_handle = fx_need->GetVariableByName("normal_matrix")->AsMatrix();
	project_matrix_handle = fx_need->GetVariableByName("final_matrix")->AsMatrix();
	texture_need = fx_need->GetVariableByName("texture_diffuse")->AsShaderResource();
}
HRESULT shader_ssaodepthnormal_map::set_trans_world(XMFLOAT4X4 *mat_world, XMFLOAT4X4 *mat_view)
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
HRESULT shader_ssaodepthnormal_map::set_trans_all(XMFLOAT4X4 *mat_final) 
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
HRESULT shader_ssaodepthnormal_map::set_texture(ID3D11ShaderResourceView *tex_in) 
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
void shader_ssaodepthnormal_map::release()
{
	release_basic();
}
void shader_ssaodepthnormal_map::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT" ,0  ,DXGI_FORMAT_R32G32B32_FLOAT   ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
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
		{ "TEXCOORD",0  ,DXGI_FORMAT_R32G32_FLOAT      ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~全局shader管理器~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
shader_control::shader_control()
{
	shader_light_pre = NULL;
	shader_light_deferred = NULL;
	shader_shadowmap = NULL;
	shader_cubemap = NULL;
	shader_ssao_depthnormal = NULL;
	shader_ssao_draw = NULL;
	shader_ssao_blur = NULL;
	particle_build = NULL;
	particle_show = NULL;
	shader_HDR_average = NULL;
	shader_HDR_preblur = NULL;
	shader_HDR_blur = NULL;
	shader_HDR_final = NULL;
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

	shader_shadowmap = new light_shadow(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\shadowmap.cso", device_pancy, contex_pancy);
	hr = shader_shadowmap->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when shadowmap shader created", L"tip", MB_OK);
		return hr;
	}
	
	shader_ssao_depthnormal = new shader_ssaodepthnormal_map(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\ssao_normaldepth_map.cso", device_pancy, contex_pancy);
	hr = shader_ssao_depthnormal->shder_create();
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
	return S_OK;
}
void shader_control::release()
{
	shader_light_pre->release();
	shader_shadowmap->release();
	shader_ssao_depthnormal->release();
	shader_ssao_draw->release();
	shader_ssao_blur->release();
	shader_cubemap->release();
	shader_HDR_average->release();
	shader_HDR_preblur->release();
	shader_HDR_blur->release();
	shader_HDR_final->release();
}