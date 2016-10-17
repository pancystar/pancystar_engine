#include"engine_shadow.h"
shadow_basic::shadow_basic(ID3D11Device *device_need, ID3D11DeviceContext* contex_need, shader_control *shader_list_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	shader_list = shader_list_need;
	depthmap_tex = NULL;
	depthmap_target = NULL;
}
HRESULT shadow_basic::set_renderstate(XMFLOAT3 light_position, XMFLOAT3 light_dir, BoundingSphere shadow_range, light_type check)
{
	if (check == direction_light)
	{
		//光源视角取景变换
		XMVECTOR lightDir = XMLoadFloat3(&light_dir);

		XMVECTOR lightPos = -2.0f*shadow_range.Radius*lightDir;
		XMVECTOR targetPos = XMLoadFloat3(&shadow_range.Center);
		XMVECTOR up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMMATRIX viewmat = XMMatrixLookAtLH(lightPos, targetPos, up);


		XMFLOAT3 sphereCenterLS;
		XMVECTOR a = XMVector3TransformCoord(targetPos, viewmat);
		XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, viewmat));
		//平行投影
		float l = sphereCenterLS.x - shadow_range.Radius;
		float b = sphereCenterLS.y - shadow_range.Radius;
		float n = sphereCenterLS.z - shadow_range.Radius;
		float r = sphereCenterLS.x + shadow_range.Radius;
		float t = sphereCenterLS.y + shadow_range.Radius;
		float f = sphereCenterLS.z + shadow_range.Radius;
		XMMATRIX proj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

		//正投影矩阵
		XMMATRIX final_matrix = viewmat * proj;
		XMStoreFloat4x4(&shadow_build, final_matrix);

		//3D重建后的对比投影矩阵
		XMMATRIX T_need(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f
			);
		XMStoreFloat4x4(&shadow_rebuild, final_matrix*T_need);
	}
	else if (check == spot_light)
	{
		//光源视角取景变换
		XMVECTOR lightPos = XMLoadFloat3(&light_position);

		//XMVECTOR lightPos = -2.0f*shadow_range.Radius*lightDir;
		XMVECTOR targetPos = XMLoadFloat3(&shadow_range.Center);
		XMVECTOR up = XMVectorSet(0.0f, -1.0f, 1.0f, 0.0f);
		XMMATRIX viewmat = XMMatrixLookAtLH(lightPos, targetPos, up);
		//透视投影
		XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, shadowmap_width*1.0f / shadowmap_height*1.0f, 0.1f, 300.0f);

		//正投影矩阵
		XMMATRIX final_matrix = viewmat * proj;
		XMStoreFloat4x4(&shadow_build, final_matrix);

		//3D重建后的对比投影矩阵
		XMMATRIX T_need(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f
			);
		XMStoreFloat4x4(&shadow_rebuild, final_matrix*T_need);
	}
	contex_pancy->RSSetViewports(1, &shadow_map_VP);
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	contex_pancy->OMSetRenderTargets(1, renderTargets, depthmap_target);
	contex_pancy->ClearDepthStencilView(depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return S_OK;
}

ID3D11ShaderResourceView* shadow_basic::get_mapresource()
{
	return depthmap_tex;
}
void shadow_basic::release()
{
	depthmap_tex->Release();
	depthmap_tex = NULL;
	depthmap_target->Release();
	depthmap_target = NULL;
}
HRESULT shadow_basic::set_viewport(int width_need, int height_need)
{
	shadowmap_width = width_need;
	shadowmap_height = height_need;
	//释放之前的shader resource view以及render target view
	if (depthmap_tex != NULL)
	{
		depthmap_tex->Release();
		depthmap_tex = NULL;
	}
	if (depthmap_target != NULL)
	{
		depthmap_target->Release();
		depthmap_target = NULL;
	}
	//指定渲染视口的大小
	shadow_map_VP.TopLeftX = 0.0f;
	shadow_map_VP.TopLeftY = 0.0f;
	shadow_map_VP.Width = static_cast<float>(shadowmap_width);
	shadow_map_VP.Height = static_cast<float>(shadowmap_height);
	shadow_map_VP.MinDepth = 0.0f;
	shadow_map_VP.MaxDepth = 1.0f;
	/*
	//指定用于存储深度记录的shader图片资源的格式
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = shadowmap_width;
	texDesc.Height = shadowmap_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//建立CPU上的纹理资源
	ID3D11Texture2D* depthMap = 0;
	HRESULT hr = device_pancy->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
	{
		MessageBox(0, L"create texture2D error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	//建立GPU上的两种资源：纹理资源以及渲染目标资源
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device_pancy->CreateDepthStencilView(depthMap, &dsvDesc, &depthmap_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create shader resource view error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = device_pancy->CreateShaderResourceView(depthMap, &srvDesc, &depthmap_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create render target view error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	//释放CPU上的纹理资源
	if (depthMap != NULL)
	{
		depthMap->Release();
	}*/
	return S_OK;
}
HRESULT shadow_basic::init_texture(ID3D11Texture2D* depthMap_array, int index_need)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~深度模板渲染目标~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC texDesc;
	depthMap_array->GetDesc(&texDesc);
	//建立GPU上的两种资源：纹理资源以及渲染目标资源
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = 
	{
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_DSV_DIMENSION_TEXTURE2DARRAY
	};
	dsvDesc.Texture2DArray.ArraySize = 1;
	dsvDesc.Texture2DArray.FirstArraySlice = index_need;
	HRESULT hr = device_pancy->CreateDepthStencilView(depthMap_array, &dsvDesc, &depthmap_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create shader resource view error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~深度信息访问资源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = 
	{
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
		D3D11_SRV_DIMENSION_TEXTURE2DARRAY
	};
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2DArray.ArraySize = 1;
	srvDesc.Texture2DArray.FirstArraySlice = index_need;
	hr = device_pancy->CreateShaderResourceView(depthMap_array, &srvDesc, &depthmap_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create render target view error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shadow_basic::create(int width_need, int height_need)
{
	return set_viewport(width_need, height_need);
}
HRESULT shadow_basic::set_transparent_tex(ID3D11ShaderResourceView *tex_in)
{
	auto *shader_shadow = shader_list->get_shader_shadowmap();
	HRESULT hr = shader_shadow->set_texture(tex_in);
	if (FAILED(hr))
	{
		MessageBox(0, L"set normal depth texture error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
ID3DX11EffectTechnique* shadow_basic::get_technique()
{
	ID3DX11EffectTechnique   *teque_need;       //渲染路径
	auto* shader_test = shader_list->get_shader_shadowmap();
	HRESULT hr = shader_test->get_technique(&teque_need, "ShadowTech");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create shadowmap resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_need;
}
ID3DX11EffectTechnique* shadow_basic::get_technique_transparent()
{
	ID3DX11EffectTechnique   *teque_need;       //渲染路径
	auto* shader_test = shader_list->get_shader_shadowmap();
	HRESULT hr = shader_test->get_technique(&teque_need, "ShadowTech_transparent");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create shadowmap resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_need;
}
ID3DX11EffectTechnique* shadow_basic::get_technique_skin()
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec_point[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION"    ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"      ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT"     ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "BONEINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "WEIGHTS"     ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES"  ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD"    ,0  ,DXGI_FORMAT_R32G32_FLOAT       ,0    ,84 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	int num_member = sizeof(rec_point) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	ID3DX11EffectTechnique   *teque_need;       //渲染路径
	auto* shader_test = shader_list->get_shader_shadowmap();
	HRESULT hr = shader_test->get_technique(rec_point, num_member,&teque_need, "Shadow_skinTech");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create shadowmap resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_need;
}
ID3DX11EffectTechnique* shadow_basic::get_technique_skin_transparent()
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec_point[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION"    ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"      ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TANGENT"     ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,24 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "BONEINDICES" ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "WEIGHTS"     ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT ,0    ,52 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES"  ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,68 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD"    ,0  ,DXGI_FORMAT_R32G32_FLOAT       ,0    ,84 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	int num_member = sizeof(rec_point) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	ID3DX11EffectTechnique   *teque_need;       //渲染路径
	auto* shader_test = shader_list->get_shader_shadowmap();
	HRESULT hr = shader_test->get_technique(rec_point, num_member,&teque_need, "Shadow_skinTech_transparent");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create shadowmap resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_need;
}
HRESULT shadow_basic::set_shaderresource(XMFLOAT4X4 word_matrix)
{
	auto* shader_test = shader_list->get_shader_shadowmap();
	XMMATRIX rec_final, rec_world;
	XMFLOAT4X4 rec_mat;
	rec_final = XMLoadFloat4x4(&shadow_build);
	rec_world = XMLoadFloat4x4(&word_matrix);
	rec_final = rec_world*rec_final;
	XMStoreFloat4x4(&rec_mat, rec_final);
	HRESULT hr = shader_test->set_trans_all(&rec_mat);
	if (FAILED(hr))
	{
		MessageBox(0, L"set shader matrix error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shadow_basic::set_bone_matrix(XMFLOAT4X4 *bone_matrix,int cnt_need) 
{
	auto* shader_test = shader_list->get_shader_shadowmap();
	HRESULT hr = shader_test->set_bone_matrix(bone_matrix,cnt_need);
	if (FAILED(hr)) 
	{
		return hr;
	}
	return S_OK;
}


pancy_shadow_volume::pancy_shadow_volume(ID3D11Device *device_need, ID3D11DeviceContext* contex_need, shader_control *shader_list_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	shader_list = shader_list_need;
	vertex_need = NULL;
}
HRESULT pancy_shadow_volume::set_renderstate(ID3D11DepthStencilView* depth_input,XMFLOAT3 light_position, XMFLOAT3 light_dir, light_type check)
{
	//绑定深度模板缓冲区，保留之前的深度
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	contex_pancy->OMSetRenderTargets(1, renderTargets, depth_input);
	//只清空模板缓冲区
	contex_pancy->ClearDepthStencilView(depth_input, D3D11_CLEAR_STENCIL, 1.0f, 0);

	auto shader_volume = shader_list->get_shader_shadowvolume();
	shader_volume->set_light_pos(light_position);
	shader_volume->set_light_dir(light_dir);
	/*
	UINT offset = 0;
	contex_pancy->SOSetTargets(1, &vertex_need, &offset);
	*/
	return S_OK;
}
HRESULT pancy_shadow_volume::init_buffer(int buffer_num)
{
	HRESULT hr;
	//初始顶点缓存
	D3D11_BUFFER_DESC VB_desc;
	VB_desc.Usage = D3D11_USAGE_DEFAULT;
	VB_desc.CPUAccessFlags = 0;
	VB_desc.MiscFlags = 0;
	VB_desc.StructureByteStride = 0;
	VB_desc.ByteWidth = sizeof(XMFLOAT3) * buffer_num;
	VB_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	hr = device_pancy->CreateBuffer(&VB_desc, 0, &vertex_need);
	if (FAILED(hr))
	{
		MessageBox(0, L"create shadow volume vertex error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT pancy_shadow_volume::set_shaderresource(XMFLOAT4X4 word_matrix)
{
	auto shader_volume = shader_list->get_shader_shadowvolume();
	//选定绘制路径
	HRESULT hr = shader_volume->get_technique(&teque_need, "StreamOutTech");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create shadowvolume resource", L"tip", MB_OK);
		return hr;
	}
	hr = shader_volume->get_technique(&teque_transparent, "StreamOutTech");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create shadowvolume resource", L"tip", MB_OK);
		return hr;
	}
	return shader_volume->set_trans_world(&word_matrix);
}
HRESULT pancy_shadow_volume::create(int buffer_num)
{
	return init_buffer(buffer_num);
}
void pancy_shadow_volume::release()
{
	vertex_need->Release();
}
void pancy_shadow_volume::draw_SOvertex()
{
	UINT stride = sizeof(XMFLOAT3);     //顶点结构的位宽
	UINT offset = 0;
	D3DX11_TECHNIQUE_DESC techDesc;
	/*
	//解绑顶点输出缓冲
	ID3D11Buffer* bufferArray[1] = { 0 };
	contex_pancy->SOSetTargets(1, bufferArray, &offset);
	*/
	auto shader_shadowvolume_draw = shader_list->get_shader_shadowvolume_draw();
	shader_shadowvolume_draw->get_technique(&teque_need, "ShadowTech");
	contex_pancy->IASetVertexBuffers(0, 1, &vertex_need, &stride, &offset);
	//contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_);
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawAuto();
	}
}
HRESULT pancy_shadow_volume::set_view_projmat(XMFLOAT4X4 mat_need)
{
	auto shader_need = shader_list->get_shader_shadowvolume_draw();
	auto shader_need2 = shader_list->get_shader_shadowvolume();
	HRESULT hr = shader_need->set_trans_all(&mat_need);
	if (FAILED(hr)) 
	{
		return hr;
	}
	hr = shader_need2->set_trans_all(&mat_need);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
