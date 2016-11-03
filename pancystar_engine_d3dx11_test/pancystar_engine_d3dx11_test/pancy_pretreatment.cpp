#include"pancy_pretreatment.h"
Pretreatment_gbuffer::Pretreatment_gbuffer(int width_need, int height_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *renderstate_need, shader_control *shader_need, geometry_control *geometry_need, pancy_camera *camera_need, light_control *light_need)
{
	map_width = width_need;
	map_height = height_need;
	device_pancy = device_need;
	contex_pancy = contex_need;
	renderstate_lib = renderstate_need;
	shader_list = shader_need;
	geometry_lib = geometry_need;
	camera_use = camera_need;
	light_list = light_need;
	depthmap_tex = NULL;
	depthmap_target = NULL;
	normalspec_target = NULL;
	normalspec_tex = NULL;
	gbuffer_diffuse_target = NULL;
	gbuffer_diffuse_tex = NULL;
	gbuffer_specular_target = NULL;
	gbuffer_specular_tex = NULL;
	lightbuffer_VB = NULL;
	lightbuffer_IB = NULL;
}
HRESULT Pretreatment_gbuffer::init_texture()
{
	HRESULT hr;
	//指定用于存储深度记录的shader图片资源的格式
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 4;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//建立4xMSAA抗锯齿渲染目标
	ID3D11Texture2D* depthMap = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
	{
		MessageBox(0, L"create texture2D error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device_pancy->CreateDepthStencilView(depthMap, &dsvDesc, &depthmap_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create shader resource view error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = device_pancy->CreateShaderResourceView(depthMap, &srvDesc, &depthmap_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create render target view error when create shadowmap resource", L"tip", MB_OK);
		return hr;
	}
	if (depthMap != NULL)
	{
		depthMap->Release();
	}
	//~~~~~~~~~~~~~~~法线&镜面反射纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//指定4xMSAA抗锯齿纹理格式并创建纹理资源
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 4;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* normalspec_buf = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &normalspec_buf);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = device_pancy->CreateRenderTargetView(normalspec_buf, 0, &normalspec_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	//释放纹理资源
	normalspec_buf->Release();
	//创建非抗锯齿纹理
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	ID3D11Texture2D* normalspec_singlebuf = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &normalspec_singlebuf);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	hr = device_pancy->CreateShaderResourceView(normalspec_singlebuf, 0, &normalspec_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	normalspec_singlebuf->Release();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~光照信息存储纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ID3D11Texture2D *diffuse_buf = 0, *specular_buf = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &diffuse_buf);
	if (FAILED(hr))
	{
		MessageBox(0, L"create diffuse_buf texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &specular_buf);
	if (FAILED(hr))
	{
		MessageBox(0, L"create specular_buf texture error", L"tip", MB_OK);
		return hr;
	}
	//根据纹理资源创建访问资源以及渲染目标
	hr = device_pancy->CreateShaderResourceView(diffuse_buf, 0, &gbuffer_diffuse_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(diffuse_buf, 0, &gbuffer_diffuse_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}

	hr = device_pancy->CreateShaderResourceView(specular_buf, 0, &gbuffer_specular_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(specular_buf, 0, &gbuffer_specular_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	diffuse_buf->Release();
	specular_buf->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~msaa重采样纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* depth_singlebuffer = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &depth_singlebuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"create depth_singlebuffer texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(depth_singlebuffer, 0, &depthmap_single_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(depth_singlebuffer, 0, &depthmap_single_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalspec_buf texture error", L"tip", MB_OK);
		return hr;
	}
	depth_singlebuffer->Release();
	return S_OK;
}
HRESULT Pretreatment_gbuffer::init_buffer()
{
	HRESULT hr;
	simpletex_point v_need[4];
	v_need[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v_need[1].position = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v_need[2].position = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v_need[3].position = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	v_need[0].tex = XMFLOAT2(0.0f, static_cast<float>(map_height));
	v_need[1].tex = XMFLOAT2(0.0f, 0.0f);
	v_need[2].tex = XMFLOAT2(static_cast<float>(map_width), 0.0f);
	v_need[3].tex = XMFLOAT2(static_cast<float>(map_width), static_cast<float>(map_height));

	D3D11_BUFFER_DESC data_desc;
	data_desc.Usage = D3D11_USAGE_IMMUTABLE;
	data_desc.ByteWidth = sizeof(simpletex_point) * 4;
	data_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	data_desc.CPUAccessFlags = 0;
	data_desc.MiscFlags = 0;
	data_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA data_buf;
	data_buf.pSysMem = v_need;
	hr = device_pancy->CreateBuffer(&data_desc, &data_buf, &depthbuffer_VB);
	if (FAILED(hr))
	{
		MessageBox(0, L"create buffer error", L"tip", MB_OK);
		return hr;
	}
	pancy_point v[4];

	v[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].position = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v[2].position = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v[3].position = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	// 远截面的四个角点
	v[0].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v[1].normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	v[2].normal = XMFLOAT3(2.0f, 0.0f, 0.0f);
	v[3].normal = XMFLOAT3(3.0f, 0.0f, 0.0f);

	v[0].tex = XMFLOAT2(0.0f, 1.0f);
	v[1].tex = XMFLOAT2(0.0f, 0.0f);
	v[2].tex = XMFLOAT2(1.0f, 0.0f);
	v[3].tex = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(pancy_point) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	hr = device_pancy->CreateBuffer(&vbd, &vinitData, &lightbuffer_VB);
	if (FAILED(hr))
	{
		MessageBox(0, L"create buffer error", L"tip", MB_OK);
		return hr;
	}
	USHORT indices[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	hr = device_pancy->CreateBuffer(&ibd, &iinitData, &lightbuffer_IB);
	if (FAILED(hr))
	{
		MessageBox(0, L"create buffer error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void Pretreatment_gbuffer::set_size()
{
	//半屏幕渲染
	render_viewport.TopLeftX = 0.0f;
	render_viewport.TopLeftY = 0.0f;
	render_viewport.Width = static_cast<float>(map_width);
	render_viewport.Height = static_cast<float>(map_height);
	render_viewport.MinDepth = 0.0f;
	render_viewport.MaxDepth = 1.0f;
	float fovy = XM_PI*0.25f;
	float farZ = 300.0f;
	XMStoreFloat4x4(&proj_matrix_gbuffer, DirectX::XMMatrixPerspectiveFovLH(fovy, map_width*1.0f / map_height*1.0f, 0.1f, farZ));
	BuildFrustumFarCorners(fovy, farZ);
}
void Pretreatment_gbuffer::BuildFrustumFarCorners(float fovy, float farZ)
{
	float aspect = (float)map_width / (float)map_height;

	float halfHeight = farZ * tanf(0.5f*fovy);
	float halfWidth = aspect * halfHeight;

	FrustumFarCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	FrustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	FrustumFarCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	FrustumFarCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
}
void Pretreatment_gbuffer::set_normalspecdepth_target()
{
	ID3D11RenderTargetView* renderTargets[1] = { normalspec_target };
	contex_pancy->OMSetRenderTargets(1, renderTargets, depthmap_target);
	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	contex_pancy->ClearRenderTargetView(normalspec_target, clearColor);
	contex_pancy->ClearDepthStencilView(depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void Pretreatment_gbuffer::set_multirender_target()
{
	ID3D11RenderTargetView* renderTargets[2] = { gbuffer_diffuse_target,gbuffer_specular_target };
	contex_pancy->OMSetRenderTargets(2, renderTargets, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	contex_pancy->ClearRenderTargetView(gbuffer_diffuse_target, clearColor);
	contex_pancy->ClearRenderTargetView(gbuffer_specular_target, clearColor);
	//contex_pancy->ClearDepthStencilView(depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void Pretreatment_gbuffer::set_resolvdepth_target()
{
	ID3D11RenderTargetView* renderTargets[1] = { depthmap_single_target };
	contex_pancy->OMSetRenderTargets(1, renderTargets, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	contex_pancy->ClearRenderTargetView(depthmap_single_target, clearColor);
}
HRESULT Pretreatment_gbuffer::create()
{
	HRESULT hr;
	hr = init_buffer();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = init_texture();
	if (FAILED(hr))
	{
		return hr;
	}
	set_size();
	return S_OK;
}
ID3DX11EffectTechnique* Pretreatment_gbuffer::get_technique()
{
	HRESULT hr;
	//选定绘制路径
	ID3DX11EffectTechnique   *teque_need;          //通用渲染路径
	hr = shader_list->get_shader_gbufferdepthnormal()->get_technique(&teque_need, "NormalDepth");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_need;
}
ID3DX11EffectTechnique* Pretreatment_gbuffer::get_technique_transparent()
{
	HRESULT hr;
	ID3DX11EffectTechnique   *teque_transparent;          //通用渲染路径
														  //选定绘制路径
	hr = shader_list->get_shader_gbufferdepthnormal()->get_technique(&teque_transparent, "NormalDepth_withalpha");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_transparent;
}
ID3DX11EffectTechnique* Pretreatment_gbuffer::get_technique_skin()
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
	HRESULT hr;
	//选定绘制路径
	ID3DX11EffectTechnique   *teque_need;          //通用渲染路径
	hr = shader_list->get_shader_gbufferdepthnormal()->get_technique(rec_point, num_member, &teque_need, "NormalDepth_skin");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_need;
}
ID3DX11EffectTechnique* Pretreatment_gbuffer::get_technique_skin_transparent()
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
	HRESULT hr;
	ID3DX11EffectTechnique   *teque_transparent;          //通用渲染路径
	hr = shader_list->get_shader_gbufferdepthnormal()->get_technique(rec_point, num_member, &teque_transparent, "NormalDepth_skin_withalpha");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_transparent;
}
ID3DX11EffectTechnique* Pretreatment_gbuffer::get_technique_normal()
{
	HRESULT hr;
	//选定绘制路径
	ID3DX11EffectTechnique   *teque_need;          //通用渲染路径
	hr = shader_list->get_shader_gbufferdepthnormal()->get_technique(&teque_need, "NormalDepth_withnormal");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_need;
}
ID3DX11EffectTechnique* Pretreatment_gbuffer::get_technique_skin_normal()
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
	HRESULT hr;
	//选定绘制路径
	ID3DX11EffectTechnique   *teque_need;          //通用渲染路径
	hr = shader_list->get_shader_gbufferdepthnormal()->get_technique(rec_point, num_member, &teque_need, "NormalDepth_skin_withnormal");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return NULL;
	}
	return teque_need;
}
void Pretreatment_gbuffer::render_gbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix)
{
	//关闭alpha混合
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	contex_pancy->RSSetViewports(1, &render_viewport);
	set_normalspecdepth_target();
	//绘制gbuffer
	scene_geometry_list *list = geometry_lib->get_model_list();
	geometry_member *now_rec = list->get_geometry_head();
	auto *g_shader = shader_list->get_shader_gbufferdepthnormal();
	for (int count = 0; count < list->get_geometry_num(); ++count)
	{
		/*
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~全部几何体渲染~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//设置世界变换矩阵
		XMFLOAT4X4 final_matrix;
		XMStoreFloat4x4(&final_matrix, XMLoadFloat4x4(&now_rec->get_world_matrix()) * XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
		g_shader->set_trans_world(&now_rec->get_world_matrix(), &view_matrix);
		g_shader->set_trans_all(&final_matrix);
		//set_normaldepth_mat(now_rec->get_world_matrix(), view_matrix, final_matrix);
		if (now_rec->check_if_skin() == true)
		{
		//设置骨骼矩阵
		g_shader->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
		//set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
		now_rec->draw_full_geometry(get_technique_skin());
		}
		else
		{
		now_rec->draw_full_geometry(get_technique());
		}
		*/
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~半透明部分渲染~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		for (int i = 0; i < now_rec->get_geometry_data()->get_meshnum(); ++i)
		{
			if (now_rec->get_geometry_data()->check_alpha(i))
			{
				//设置世界变换矩阵
				//shadowmap_deal->set_shaderresource(now_rec._Ptr->get_world_matrix());
				//设置半透明纹理
				//shadowmap_deal->set_transparent_tex(now_rec._Ptr->get_transparent_tex());
				material_list rec_mat;
				now_rec->get_geometry_data()->get_texture(&rec_mat, i);
				g_shader->set_texture(rec_mat.tex_diffuse_resource);
				//set_transparent_tex(rec_mat.tex_diffuse_resource);
				XMFLOAT4X4 final_matrix;
				XMStoreFloat4x4(&final_matrix, XMLoadFloat4x4(&now_rec->get_world_matrix()) * XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
				//set_normaldepth_mat(now_rec->get_world_matrix(), view_matrix, final_matrix);
				g_shader->set_trans_world(&now_rec->get_world_matrix(), &view_matrix);
				g_shader->set_trans_all(&final_matrix);
				if (now_rec->check_if_skin() == true)
				{
					//set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
					g_shader->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
					now_rec->draw_transparent_part(get_technique_skin_transparent(), i);
				}
				else
				{
					now_rec->draw_transparent_part(get_technique_transparent(), i);
				}
			}

		}
		for (int i = 0; i < now_rec->get_geometry_data()->get_meshnormalnum(); ++i)
		{
			//设置世界变换矩阵
			XMFLOAT4X4 final_matrix;
			XMStoreFloat4x4(&final_matrix, XMLoadFloat4x4(&now_rec->get_world_matrix()) * XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
			g_shader->set_trans_world(&now_rec->get_world_matrix(), &view_matrix);
			g_shader->set_trans_all(&final_matrix);
			//设置法线贴图
			material_list rec_mat;
			now_rec->get_geometry_data()->get_normaltexture(&rec_mat, i);
			if (rec_mat.texture_normal_resource == NULL)
			{
				if (now_rec->check_if_skin() == true)
				{
					//设置骨骼矩阵
					g_shader->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
					now_rec->draw_normal_part(get_technique_skin(), i);
				}
				else
				{
					now_rec->draw_normal_part(get_technique(), i);
				}
			}
			else
			{
				g_shader->set_texture_normal(rec_mat.texture_normal_resource);
				if (now_rec->check_if_skin() == true)
				{
					g_shader->set_bone_matrix(now_rec->get_bone_matrix(), now_rec->get_bone_num());
					now_rec->draw_normal_part(get_technique_skin_normal(), i);
				}
				else
				{
					now_rec->draw_normal_part(get_technique_normal(), i);
				}
			}
		}
		now_rec = now_rec->get_next_member();
	}
	//还原渲染状态
	contex_pancy->RSSetState(0);
	ID3D11Resource * normalDepthTex = 0;
	ID3D11Resource * normalDepthTex_singlesample = 0;
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(1, NULL_target, NULL);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~存储法线镜面反射光纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	normalspec_target->GetResource(&normalDepthTex);
	normalspec_tex->GetResource(&normalDepthTex_singlesample);
	//将多重采样纹理转换至非多重纹理
	contex_pancy->ResolveSubresource(normalDepthTex_singlesample, D3D11CalcSubresource(0, 0, 1), normalDepthTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	normalspec_tex->Release();
	normalspec_tex = NULL;
	device_pancy->CreateShaderResourceView(normalDepthTex_singlesample, 0, &normalspec_tex);
	normalDepthTex->Release();
	normalDepthTex_singlesample->Release();
	//msaa-shader重采样
	set_resolvdepth_target();
	auto shader_resolve = shader_list->get_shader_resolve_depthstencil();
	shader_resolve->set_texture_MSAA(depthmap_tex);
	XMFLOAT3 rec_proj_vec;
	rec_proj_vec.x = 1.0f / proj_matrix_gbuffer._43;
	rec_proj_vec.y = -1.0f / proj_matrix_gbuffer._43;
	rec_proj_vec.z = 0.0f;
	shader_resolve->set_projmessage(rec_proj_vec);
	ID3DX11EffectTechnique *tech_need;
	shader_resolve->get_technique(&tech_need, "resolove_msaa");
	resolve_depth_render(tech_need);
	shader_resolve->set_texture_MSAA(NULL);
	contex_pancy->OMSetRenderTargets(1, NULL_target, NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void Pretreatment_gbuffer::display(bool if_shadow)
{
	XMFLOAT4X4 view_matrix_gbuffer, invview_matrix_lbuffer;         //取景变换&逆变换
	camera_use->count_view_matrix(&view_matrix_gbuffer);
	camera_use->count_invview_matrix(&invview_matrix_lbuffer);
	render_gbuffer(view_matrix_gbuffer, proj_matrix_gbuffer);
	render_lbuffer(view_matrix_gbuffer, invview_matrix_lbuffer, if_shadow);
}
void Pretreatment_gbuffer::resolve_depth_render(ID3DX11EffectTechnique* tech)
{
	//渲染屏幕空间像素图
	UINT stride = sizeof(simpletex_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &depthbuffer_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(lightbuffer_IB, DXGI_FORMAT_R16_UINT, 0);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}

}
void Pretreatment_gbuffer::light_buffer_render(ID3DX11EffectTechnique* tech)
{
	//渲染屏幕空间像素图
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &lightbuffer_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(lightbuffer_IB, DXGI_FORMAT_R16_UINT, 0);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
}
void Pretreatment_gbuffer::release()
{
	safe_release(depthmap_tex);
	safe_release(depthmap_target);
	safe_release(normalspec_target);
	safe_release(normalspec_tex);
	safe_release(gbuffer_diffuse_target);
	safe_release(gbuffer_diffuse_tex);
	safe_release(gbuffer_specular_target);
	safe_release(gbuffer_specular_tex);
	safe_release(lightbuffer_VB);
	safe_release(lightbuffer_IB);
	safe_release(depthmap_single_target);
	safe_release(depthmap_single_tex);
	safe_release(depthbuffer_VB);
}
void Pretreatment_gbuffer::render_lbuffer(XMFLOAT4X4 view_matrix, XMFLOAT4X4 invview_matrix, bool if_shadow)
{
	contex_pancy->RSSetViewports(1, &render_viewport);
	set_multirender_target();
	auto lbuffer_shader = shader_list->get_shader_defferedlight_lightbuffer();
	lbuffer_shader->set_DepthMap_tex(depthmap_single_tex);
	lbuffer_shader->set_Normalspec_tex(normalspec_tex);
	lbuffer_shader->set_FrustumCorners(FrustumFarCorner);
	lbuffer_shader->set_view_matrix(&view_matrix);
	lbuffer_shader->set_invview_matrix(&invview_matrix);
	lbuffer_shader->set_shadow_tex(light_list->get_shadow_map_resource());
	ID3DX11EffectTechnique *tech_need;
	if (if_shadow == true) 
	{
		lbuffer_shader->get_technique(&tech_need, "draw_common");
	}
	else 
	{
		lbuffer_shader->get_technique(&tech_need, "draw_withoutshadow");
	}
	light_buffer_render(tech_need);
	ID3D11RenderTargetView* NULL_target[2] = { NULL,NULL };
	contex_pancy->OMSetRenderTargets(2, NULL_target, 0);
	lbuffer_shader->set_DepthMap_tex(NULL);
	lbuffer_shader->set_Normalspec_tex(NULL);
	lbuffer_shader->set_shadow_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}

}