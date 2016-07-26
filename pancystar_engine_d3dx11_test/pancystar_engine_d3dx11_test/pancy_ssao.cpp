#include"pancy_ssao.h"
ssao_pancy::ssao_pancy(ID3D11Device* device, ID3D11DeviceContext* dc, shader_control *shader_need, int width, int height, float fovy, float farZ)
{
	device_pancy = device;
	contex_pancy = dc;
	set_size(width, height, fovy, farZ);
	shader_list = shader_need;
	teque_need = NULL;
}
HRESULT ssao_pancy::basic_create()
{
	build_fullscreen_picturebuff();
	build_offset_vector();
	HRESULT hr;
	//选定绘制路径
	hr = shader_list->get_shader_ssaodepthnormal()->get_technique(&teque_need, "NormalDepth");
	if (FAILED(hr))
	{
		MessageBox(0, L"get technique error when create ssao resource", L"tip", MB_OK);
		return hr;
	}
	//创建纹理
	hr = build_randomtex();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = build_texture();
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}

void ssao_pancy::compute_ssaomap()
{
	//绑定渲染目标纹理，不设置深度模缓冲区因为这里不需要
	ID3D11RenderTargetView* renderTargets[1] = { ambient_target0 };
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	contex_pancy->OMSetRenderTargets(1, renderTargets, 0);
	contex_pancy->ClearRenderTargetView(ambient_target0, clearColor);
	contex_pancy->RSSetViewports(1, &render_viewport);
	//设置渲染状态
	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, map_width*1.0f / map_height*1.0f, 0.1f, 1000.0f);
	XMFLOAT4X4 PT;
	XMStoreFloat4x4(&PT, P*T);
	auto *shader_aopass = shader_list->get_shader_ssaodraw();

	shader_aopass->set_ViewToTexSpace(&PT);
	shader_aopass->set_OffsetVectors(random_Offsets);
	shader_aopass->set_FrustumCorners(FrustumFarCorner);
	shader_aopass->set_NormalDepthtex(normaldepth_tex);
	shader_aopass->set_randomtex(randomtex);

	//渲染屏幕空间像素图
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &AoMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(AoMap_IB, DXGI_FORMAT_R16_UINT, 0);

	ID3DX11EffectTechnique* tech;
	//选定绘制路径
	shader_aopass->get_technique(&tech, "draw_ssaomap");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_aopass->set_NormalDepthtex(NULL);
	shader_aopass->set_randomtex(NULL);
	tech->GetPassByIndex(0)->Apply(0, contex_pancy);
}
ID3D11ShaderResourceView* ssao_pancy::get_aomap()
{
	return ambient_tex0;
}
void ssao_pancy::set_normaldepth_target(ID3D11DepthStencilView* dsv)
{
	ID3D11RenderTargetView* renderTargets[1] = { normaldepth_target };
	contex_pancy->OMSetRenderTargets(1, renderTargets, depthmap_target);
	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	contex_pancy->ClearRenderTargetView(normaldepth_target, clearColor);
	contex_pancy->ClearDepthStencilView(depthmap_target, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
HRESULT ssao_pancy::set_normaldepth_mat(XMFLOAT4X4 world_mat, XMFLOAT4X4 view_mat,XMFLOAT4X4 final_mat)
{
	auto *shader_depthnormal = shader_list->get_shader_ssaodepthnormal();
	HRESULT hr = shader_depthnormal->set_trans_world(&world_mat, &view_mat);
	if (FAILED(hr))
	{
		MessageBox(0, L"set normal depth matrix error", L"tip", MB_OK);
		return hr;
	}
	hr = shader_depthnormal->set_trans_all(&final_mat);
	if (FAILED(hr))
	{
		MessageBox(0, L"set normal depth matrix error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}

HRESULT ssao_pancy::build_texture()
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* normalDepthTex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &normalDepthTex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalDepth texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(normalDepthTex, 0, &normaldepth_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalDepth texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(normalDepthTex, 0, &normaldepth_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create normalDepth texture error", L"tip", MB_OK);
		return hr;
	}
	normalDepthTex->Release();

	//半屏幕纹理
	texDesc.Width = map_width / 2;
	texDesc.Height = map_height / 2;
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	ID3D11Texture2D* ambientTex0 = 0;
	device_pancy->CreateTexture2D(&texDesc, 0, &ambientTex0);
	if (FAILED(hr))
	{
		MessageBox(0, L"create ambient map texture1 error", L"tip", MB_OK);
		return hr;
	}
	device_pancy->CreateShaderResourceView(ambientTex0, 0, &ambient_tex0);
	if (FAILED(hr))
	{
		MessageBox(0, L"create ambient map texture1 error", L"tip", MB_OK);
		return hr;
	}
	device_pancy->CreateRenderTargetView(ambientTex0, 0, &ambient_target0);
	if (FAILED(hr))
	{
		MessageBox(0, L"create ambient map texture1 error", L"tip", MB_OK);
		return hr;
	}

	ID3D11Texture2D* ambientTex1 = 0;
	device_pancy->CreateTexture2D(&texDesc, 0, &ambientTex1);
	if (FAILED(hr))
	{
		MessageBox(0, L"create ambient map texture2 error", L"tip", MB_OK);
		return hr;
	}
	device_pancy->CreateShaderResourceView(ambientTex1, 0, &ambient_tex1);
	if (FAILED(hr))
	{
		MessageBox(0, L"create ambient map texture2 error", L"tip", MB_OK);
		return hr;
	}
	device_pancy->CreateRenderTargetView(ambientTex1, 0, &ambient_target1);
	if (FAILED(hr))
	{
		MessageBox(0, L"create ambient map texture2 error", L"tip", MB_OK);
		return hr;
	}


	D3D11_TEXTURE2D_DESC texDesc2;
	texDesc2.Width = map_width;
	texDesc2.Height = map_height;
	texDesc2.MipLevels = 1;
	texDesc2.ArraySize = 1;
	texDesc2.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc2.SampleDesc.Count = 1;
	texDesc2.SampleDesc.Quality = 0;
	texDesc2.Usage = D3D11_USAGE_DEFAULT;
	texDesc2.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc2.CPUAccessFlags = 0;
	texDesc2.MiscFlags = 0;

	ID3D11Texture2D* depthMap = 0;
	hr = device_pancy->CreateTexture2D(&texDesc2, 0, &depthMap);
	if (FAILED(hr))
	{
		MessageBox(0, L"create depth buffer texture2 error", L"tip", MB_OK);
		return hr;
	}
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device_pancy->CreateDepthStencilView(depthMap, &dsvDesc, &depthmap_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create depth buffer texture2 error", L"tip", MB_OK);
		return hr;
	}
	ambientTex0->Release();
	ambientTex1->Release();
	depthMap->Release();
	return S_OK;
}
void ssao_pancy::BuildFrustumFarCorners(float fovy, float farZ)
{
	float aspect = (float)map_width / (float)map_height;

	float halfHeight = farZ * tanf(0.5f*fovy);
	float halfWidth = aspect * halfHeight;

	FrustumFarCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	FrustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	FrustumFarCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	FrustumFarCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
}
void ssao_pancy::set_size(int width, int height, float fovy, float farZ)
{
	map_width = width;
	map_height = height;
	//半屏幕渲染
	render_viewport.TopLeftX = 0.0f;
	render_viewport.TopLeftY = 0.0f;
	render_viewport.Width = width / 2.0f;
	render_viewport.Height = height / 2.0f;
	render_viewport.MinDepth = 0.0f;
	render_viewport.MaxDepth = 1.0f;
	BuildFrustumFarCorners(fovy, farZ);
}
void ssao_pancy::build_fullscreen_picturebuff()
{
	pancy_point v[4];

	v[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].position = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v[2].position = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v[3].position = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	// Store far plane frustum corner indices in Normal.x slot.
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

	device_pancy->CreateBuffer(&vbd, &vinitData, &AoMap_VB);

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

	device_pancy->CreateBuffer(&ibd, &iinitData, &AoMap_IB);
}
void ssao_pancy::build_offset_vector()
{
	random_Offsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	random_Offsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	random_Offsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	random_Offsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	random_Offsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	random_Offsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	random_Offsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	random_Offsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 centers of cube faces
	random_Offsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	random_Offsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	random_Offsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	random_Offsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	random_Offsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	random_Offsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i)
	{
		// Create random lengths in [0.25, 1.0].
		float s = 0.25f + 0.75f * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&random_Offsets[i]));
		XMStoreFloat4(&random_Offsets[i], v);
	}
}
HRESULT ssao_pancy::build_randomtex()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.SysMemPitch = 256 * 4 * sizeof(char);
	unsigned char* color;
	color = (unsigned char*)malloc(256 * 256 * 4 * sizeof(unsigned char));
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				color[i * 256 * 4 + j * 4 + k] = rand() % 256;
			}
			color[i * 256 * 4 + j * 4 + 3] = 0;
		}
	}
	initData.pSysMem = color;

	HRESULT hr;
	ID3D11Texture2D* tex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, &initData, &tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create random texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(tex, 0, &randomtex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create random texture error", L"tip", MB_OK);
		return hr;
	}
	tex->Release();
	free(color);
	return S_OK;
}
void ssao_pancy::blur_ssaomap()
{
	for (int i = 0; i < 2; ++i)
	{
		basic_blur(ambient_tex0, ambient_target1, true);
		basic_blur(ambient_tex1, ambient_target0, false);
	}
}
void ssao_pancy::basic_blur(ID3D11ShaderResourceView *texin, ID3D11RenderTargetView *texout, bool if_row)
{
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	ID3D11RenderTargetView* renderTargets[1] = { texout };
	
	contex_pancy->OMSetRenderTargets(1, renderTargets, 0);
	contex_pancy->ClearRenderTargetView(texout, black);
	

	contex_pancy->RSSetViewports(1, &render_viewport);
	auto *shader_blurpass = shader_list->get_shader_ssaoblur();
	shader_blurpass->set_image_size(1.0f / render_viewport.Width, 1.0f / render_viewport.Height);
	shader_blurpass->set_tex_resource(normaldepth_tex, texin);
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;

	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &AoMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(AoMap_IB, DXGI_FORMAT_R16_UINT, 0);

	ID3DX11EffectTechnique* tech;
	//选定绘制路径
	if (if_row)
	{
		shader_blurpass->get_technique(&tech, "HorzBlur");
	}
	else
	{
		shader_blurpass->get_technique(&tech, "VertBlur");
	}
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_blurpass->set_tex_resource(NULL, NULL);
	tech->GetPassByIndex(0)->Apply(0, contex_pancy);
}
void ssao_pancy::release()
{
	safe_release(AoMap_VB);
	safe_release(AoMap_IB);
	safe_release(randomtex);
	safe_release(normaldepth_target);
	safe_release(normaldepth_tex);
	safe_release(depthmap_target);
	safe_release(ambient_target0);
	safe_release(ambient_tex0);
	safe_release(ambient_target1);
	safe_release(ambient_tex1);
}
