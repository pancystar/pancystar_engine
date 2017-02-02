#include"pancy_posttreatment.h"
render_posttreatment_HDR::render_posttreatment_HDR(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, ID3D11RenderTargetView *rendertarget_need, shader_control *shaderlist_need, int width_need, int height_need, pancy_renderstate *rec_rootstate)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	rendertarget_input = rendertarget_need;
	shader_list = shaderlist_need;
	width = width_need;
	height = height_need;
	root_state_need = rec_rootstate;
	CPU_read_buffer = NULL;
	average_light_last = 0.0f;
}
HRESULT render_posttreatment_HDR::count_average_light()
{
	HRESULT hr;
	//pass1计算平均亮度
	auto shader_test = shader_list->get_shader_HDRaverage();
	ID3D11Resource * normalDepthTex = 0;
	ID3D11Resource * normalDepthTex_singlesample = 0;
	rendertarget_input->GetResource(&normalDepthTex);
	SRV_HDR_use->GetResource(&normalDepthTex_singlesample);
	//将多重采样纹理转换至非多重纹理
	contex_pancy->ResolveSubresource(normalDepthTex_singlesample, D3D11CalcSubresource(0, 0, 1), normalDepthTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	SRV_HDR_use->Release();
	SRV_HDR_use = NULL;
	device_pancy->CreateShaderResourceView(normalDepthTex_singlesample, 0, &SRV_HDR_use);
	normalDepthTex->Release();
	normalDepthTex_singlesample->Release();

	hr = shader_test->set_compute_tex(SRV_HDR_use);
	if (FAILED(hr))
	{
		MessageBox(0, L"set HDR shader resource error", L"tip", MB_OK);
		return hr;
	}
	hr = shader_test->set_compute_buffer(UAV_HDR_mid, UAV_HDR_final);
	if (FAILED(hr))
	{
		MessageBox(0, L"set HDR shader resource error", L"tip", MB_OK);
		return hr;
	}
	width_rec = width / 4;
	height_rec = height / 4;
	//计算线程数量
	if (width % 4 != 0)
	{
		width_rec += 1;
	}
	if (height % 4 != 0)
	{
		height_rec += 1;
	}
	//计算线程组的数量
	if (width_rec % 16 != 0)
	{
		width_rec = (width_rec / 16 + 1) * 16;
	}
	if (height_rec % 16 != 0)
	{
		height_rec = (height_rec / 16 + 1) * 16;
	}

	//计算线程数量
	buffer_num = width_rec * height_rec;
	//计算线程组数量
	if (buffer_num % 256 != 0)
	{
		buffer_num = buffer_num / 256 + 1;
	}
	else
	{
		buffer_num = buffer_num / 256;
	}
	hr = shader_test->set_piccturerange(width, height, width_rec * height_rec, height_rec);
	if (FAILED(hr))
	{
		MessageBox(0, L"set HDR shader resource error", L"tip", MB_OK);
		return hr;
	}
	map_num = buffer_num / 256;
	if (buffer_num % 256 != 0)
	{
		map_num = buffer_num / 256 + 1;
	}
	shader_test->dispatch(width_rec / 16, height_rec / 16, buffer_num, map_num);
	shader_test->set_compute_buffer(NULL, NULL);
	shader_test->set_compute_tex(NULL);
	return S_OK;
}
HRESULT render_posttreatment_HDR::build_preblur_map()
{
	//拷贝到CPU上进行平均亮度计算的map方法
	/*
	ID3D11Resource *check_rec;
	UAV_HDR_mid->GetResource(&check_rec);
	ID3D11Buffer* pDebugBuffer = NULL;
	D3D11_BOX box;
	box.left = 0;
	box.right = sizeof(float) * map_num;
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;
	contex_pancy->CopySubresourceRegion(CPU_read_buffer, 0, 0, 0, 0, check_rec, 0, &box);
	D3D11_MAPPED_SUBRESOURCE vertex_resource;
	hr = contex_pancy->Map(CPU_read_buffer, 0, D3D11_MAP_READ, 0, &vertex_resource);
	if (FAILED(hr))
	{
	MessageBox(0, L"get vertex buffer map error", L"tip", MB_OK);
	return hr;
	}
	float vertex[1000];
	memcpy(static_cast<void*>(vertex), vertex_resource.pData, map_num * sizeof(float));
	average_light = 0.0f;
	for (int i = 0; i < map_num; ++i)
	{
	average_light += vertex[i];
	}
	//average_light /= width * height;
	average_light = exp(average_light);
	check_rec->Release();
	contex_pancy->Unmap(CPU_read_buffer, 0);
	*/

	//pass2高光模糊的预处理
	auto shader_test2 = shader_list->get_shader_HDRpreblur();
	shader_test2->set_buffer_input(SRV_HDR_map, SRV_HDR_use);
	shader_test2->set_lum_message(0.0f, 1.0f, 1.5f, 0.38f);
	shader_test2->set_piccturerange(width, height, map_num, width * height);
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };

	ID3D11RenderTargetView* renderTargets[1] = { RTV_HDR_blur1 };
	contex_pancy->OMSetRenderTargets(1, renderTargets, 0);
	contex_pancy->ClearRenderTargetView(RTV_HDR_blur1, black);
	contex_pancy->RSSetViewports(1, &render_viewport);

	UINT stride = sizeof(HDR_fullscreen);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &HDRMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(HDRMap_IB, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	shader_test2->get_technique(&tech, "draw_preblur");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_test2->set_buffer_input(NULL, NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	return S_OK;
}
HRESULT render_posttreatment_HDR::blur_map()
{
	//pass3高光模糊
	basic_blur(SRV_HDR_blur1, RTV_HDR_blur2, true);
	basic_blur(SRV_HDR_blur2, RTV_HDR_blur1, false);
	return S_OK;
}
HRESULT render_posttreatment_HDR::HDR_map()
{
	//pass4最终合成
	root_state_need->restore_rendertarget();
	auto shader_test3 = shader_list->get_shader_HDRfinal();
	shader_test3->set_tex_resource(SRV_HDR_use, SRV_HDR_blur1, SRV_HDR_map);
	shader_test3->set_lum_message(average_light, 1.0f, 2.0f, 0.68f);
	shader_test3->set_piccturerange(width, height, map_num, width * height);


	UINT stride = sizeof(HDR_fullscreen);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &HDRMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(HDRMap_IB, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	shader_test3->get_technique(&tech, "draw_HDRfinal");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_test3->set_tex_resource(NULL, NULL, NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
	return S_OK;
}
HRESULT render_posttreatment_HDR::CreateCPUaccessBuf(int size_need)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = size_need*sizeof(float);        //顶点缓存的大小
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(float);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.BindFlags = 0;
	//bufferDesc.MiscFlags = 0;
	HRESULT hr = device_pancy->CreateBuffer(&bufferDesc, NULL, &CPU_read_buffer);

	if (FAILED(hr))
	{
		MessageBox(0, L"create CPU access read buffer error in HDR pass", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}

HRESULT render_posttreatment_HDR::create()
{
	HRESULT hr;
	hr = build_fullscreen_picturebuff();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = init_texture();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = init_buffer();
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT render_posttreatment_HDR::init_texture()
{
	//创建输入资源
	HRESULT hr;
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* HDR_averagetex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &HDR_averagetex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_averagetex texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(HDR_averagetex, 0, &SRV_HDR_use);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_prepass texture error", L"tip", MB_OK);
		return hr;
	}
	HDR_averagetex->Release();
	//创建高光存储资源
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	ID3D11Texture2D* HDR_preblurtex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &HDR_preblurtex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_prepass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(HDR_preblurtex, 0, &SRV_HDR_save);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_prepass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(HDR_preblurtex, 0, &RTV_HDR_save);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_prepass texture error", L"tip", MB_OK);
		return hr;
	}
	HDR_preblurtex->Release();
	//创建高斯模糊处理资源
	texDesc.Width = width / 4.0f;
	texDesc.Height = height / 4.0f;
	ID3D11Texture2D* HDR_blurtex1 = 0, *HDR_blurtex2 = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &HDR_blurtex1);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &HDR_blurtex2);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(HDR_blurtex1, 0, &SRV_HDR_blur1);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(HDR_blurtex2, 0, &SRV_HDR_blur2);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(HDR_blurtex1, 0, &RTV_HDR_blur1);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(HDR_blurtex2, 0, &RTV_HDR_blur2);
	if (FAILED(hr))
	{
		MessageBox(0, L"create HDR_blurpass texture error", L"tip", MB_OK);
		return hr;
	}
	HDR_blurtex1->Release();
	HDR_blurtex2->Release();
	//注册视口信息
	render_viewport.TopLeftX = 0.0f;
	render_viewport.TopLeftY = 0.0f;
	render_viewport.Width = static_cast<float>(width / 4.0f);
	render_viewport.Height = static_cast<float>(height / 4.0f);
	render_viewport.MinDepth = 0.0f;
	render_viewport.MaxDepth = 1.0f;
	return S_OK;
}
HRESULT render_posttreatment_HDR::init_buffer()
{
	HRESULT hr;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~创建缓冲区~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Buffer *buffer_HDR_mid;
	ID3D11Buffer *buffer_HDR_final;
	int width_rec = width / 4;
	int height_rec = height / 4;
	//设定线程数量
	if (width % 4 != 0)
	{
		width_rec += 1;
	}
	if (height % 4 != 0)
	{
		height_rec += 1;
	}
	//设定线程组的数量
	if (width_rec % 16 != 0)
	{
		width_rec = (width_rec / 16 + 1) * 16;
	}
	if (height_rec % 16 != 0)
	{
		height_rec = (height_rec / 16 + 1) * 16;
	}
	//创建HDR的第一个buffer，用于1/16的向下采样
	D3D11_BUFFER_DESC HDR_buffer_desc;
	HDR_buffer_desc.Usage = D3D11_USAGE_DEFAULT;            //通用类型
	HDR_buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//缓存类型为uav+srv
	HDR_buffer_desc.ByteWidth = width_rec * height_rec*sizeof(float);        //顶点缓存的大小
	HDR_buffer_desc.CPUAccessFlags = 0;
	HDR_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	HDR_buffer_desc.StructureByteStride = sizeof(float);
	hr = device_pancy->CreateBuffer(&HDR_buffer_desc, NULL, &buffer_HDR_mid);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR buffer", L"tip", MB_OK);
		return hr;
	}
	//创建第二个buffer，用于1/256的向下采样
	int buffer_num = width_rec * height_rec;
	if (buffer_num % 256 != 0)
	{
		buffer_num = buffer_num / 256 + 1;
	}
	else
	{
		buffer_num = buffer_num / 256;
	}
	HDR_buffer_desc.ByteWidth = buffer_num * sizeof(float);
	hr = device_pancy->CreateBuffer(&HDR_buffer_desc, NULL, &buffer_HDR_final);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR buffer", L"tip", MB_OK);
		return hr;
	}
	/*
	hr = CreateCPUaccessBuf(buffer_num);
	if (FAILED(hr))
	{
	MessageBox(0, L"an error when create HDR buffer", L"tip", MB_OK);
	return hr;
	}
	*/
	//创建第一个UAV，代表第一个buffer
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = width_rec * height_rec;

	hr = device_pancy->CreateUnorderedAccessView(buffer_HDR_mid, &DescUAV, &UAV_HDR_mid);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR UAV", L"tip", MB_OK);
		return hr;
	}

	//创建第二个UAV，代表第二个buffer
	DescUAV.Buffer.NumElements = buffer_num;
	hr = device_pancy->CreateUnorderedAccessView(buffer_HDR_final, &DescUAV, &UAV_HDR_final);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR UAV", L"tip", MB_OK);
		return hr;
	}


	D3D11_SHADER_RESOURCE_VIEW_DESC DescSRV;
	ZeroMemory(&DescSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	DescSRV.Format = DXGI_FORMAT_UNKNOWN;
	DescSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	DescSRV.Buffer.FirstElement = 0;
	DescSRV.Buffer.NumElements = width_rec * height_rec;
	hr = device_pancy->CreateShaderResourceView(buffer_HDR_mid, &DescSRV, &SRV_HDR_map);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR SRV", L"tip", MB_OK);
		return hr;
	}

	buffer_HDR_mid->Release();
	buffer_HDR_final->Release();
	return S_OK;
}
void render_posttreatment_HDR::release()
{
	SRV_HDR_use->Release();
	UAV_HDR_mid->Release();
	UAV_HDR_final->Release();
	SRV_HDR_map->Release();
	if (CPU_read_buffer != NULL)
	{
		CPU_read_buffer->Release();
	}
	HDRMap_VB->Release();
	HDRMap_IB->Release();
	SRV_HDR_save->Release();
	RTV_HDR_save->Release();
	SRV_HDR_blur1->Release();
	RTV_HDR_blur1->Release();
	SRV_HDR_blur2->Release();
	RTV_HDR_blur2->Release();
}
HRESULT render_posttreatment_HDR::display()
{
	count_average_light();
	build_preblur_map();
	blur_map();
	HDR_map();
	return S_OK;
}
void render_posttreatment_HDR::basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz)
{
	//设置渲染目标
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	ID3D11RenderTargetView* renderTargets[1] = { output };
	contex_pancy->OMSetRenderTargets(1, renderTargets, 0);
	contex_pancy->ClearRenderTargetView(output, black);
	contex_pancy->RSSetViewports(1, &render_viewport);
	auto shader_blur = shader_list->get_shader_HDRblur();
	shader_blur->set_image_size(1.0f / (width / 4.0f), 1.0f / (height / 4.0f));
	shader_blur->set_tex_resource(input);

	UINT stride = sizeof(HDR_fullscreen);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &HDRMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(HDRMap_IB, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	if (if_horz == true)
	{
		shader_blur->get_technique(&tech, "HorzBlur");
	}
	else
	{
		shader_blur->get_technique(&tech, "VertBlur");
	}
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_blur->set_tex_resource(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
	//tech->GetPassByIndex(0)->Apply(0, contex_pancy);
}
HRESULT render_posttreatment_HDR::build_fullscreen_picturebuff()
{
	HRESULT hr;
	HDR_fullscreen v[4];
	v[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	v[1].position = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	v[2].position = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	v[3].position = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	v[0].tex = XMFLOAT2(0.0f, 1.0f);
	v[1].tex = XMFLOAT2(0.0f, 0.0f);
	v[2].tex = XMFLOAT2(1.0f, 0.0f);
	v[3].tex = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(HDR_fullscreen) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;

	hr = device_pancy->CreateBuffer(&vbd, &vinitData, &HDRMap_VB);
	if (FAILED(hr))
	{
		MessageBox(0, L"error when create HDR full screen buffer", L"tip", MB_OK);
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

	hr = device_pancy->CreateBuffer(&ibd, &iinitData, &HDRMap_IB);
	if (FAILED(hr))
	{
		MessageBox(0, L"error when create HDR full screen buffer", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}

render_posttreatment_SSR::render_posttreatment_SSR(pancy_camera *camera_need, pancy_renderstate *renderstate_need, ID3D11Device* device, ID3D11DeviceContext* dc, shader_control *shader_need, geometry_control *geometry_need, int width, int height, float near_plane, float far_plane, float angle_view)
{
	camera_use = camera_need;
	device_pancy = device;
	contex_pancy = dc;
	set_size(width, height, angle_view, far_plane);
	perspective_near_plane = near_plane;
	perspective_far_plane = far_plane;
	perspective_angle = angle_view;
	shader_list = shader_need;
	renderstate_lib = renderstate_need;
	geometry_lib = geometry_need;
	reflectMap_VB = NULL;
	reflectMap_IB = NULL;
	color_tex = NULL;
	reflect_target = NULL;
	reflect_tex = NULL;
	final_reflect_target = NULL;
	final_reflect_tex = NULL;
	blur_reflect_tex = NULL;
	blur_reflect_target = NULL;
	reflect_cube_SRV = NULL;
	mask_target = NULL;
	mask_tex = NULL;
	reflect_cubestencil_SRV = NULL;
	for (int i = 0; i < 6; ++i)
	{
		reflect_depthcube_SRV[i] = NULL;
		reflect_cubeinput_RTV[i] = NULL;
		reflect_cubeinput_SRV[i] = NULL;
		reflect_cube_RTV[i] = NULL;
		reflect_DSV[i] = NULL;
		reflect_cubestencil_RTV[i] = NULL;
	}
	width_static_cube = 256.0f;
}
HRESULT render_posttreatment_SSR::create()
{
	build_fullscreen_picturebuff();
	HRESULT hr;
	//创建纹理
	hr = build_texture();
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT render_posttreatment_SSR::build_texture()
{
	HRESULT hr;
	//创建输入资源
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = map_width;
	texDesc.Height = map_height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* inputcolortex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &inputcolortex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create SSR_color texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(inputcolortex, 0, &color_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create SSR_color texture error", L"tip", MB_OK);
		return hr;
	}
	inputcolortex->Release();

	ID3D11Texture2D* inputmasktex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &inputmasktex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create SSR_mask texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(inputmasktex, 0, &input_mask_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create SSR_mask texture error", L"tip", MB_OK);
		return hr;
	}
	inputmasktex->Release();
	//作为shader resource view的普通纹理(无多重采样)
	texDesc.Width = half_render_viewport.Width;
	texDesc.Height = half_render_viewport.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	//用作pass1反射记录的普通纹理
	ID3D11Texture2D* reflecttex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &reflecttex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(reflecttex, 0, &reflect_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(reflecttex, 0, &reflect_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	reflecttex->Release();
	//ssr pass1区分是否计算成功的掩码纹理
	ID3D11Texture2D* reflectmasktex = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &reflectmasktex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(reflectmasktex, 0, &mask_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(reflectmasktex, 0, &mask_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	reflectmasktex->Release();
	//最终合并渲染的反射纹理
	ID3D11Texture2D* final_reflect_data = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &final_reflect_data);
	if (FAILED(hr))
	{
		MessageBox(0, L"create final reflect texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(final_reflect_data, 0, &final_reflect_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create final reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(final_reflect_data, 0, &final_reflect_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create final reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	final_reflect_data->Release();
	//存储高斯模糊结果的中间纹理
	ID3D11Texture2D* blur_reflect_data = 0;
	hr = device_pancy->CreateTexture2D(&texDesc, 0, &blur_reflect_data);
	if (FAILED(hr))
	{
		MessageBox(0, L"create final reflect texture error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateShaderResourceView(blur_reflect_data, 0, &blur_reflect_tex);
	if (FAILED(hr))
	{
		MessageBox(0, L"create blur reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	hr = device_pancy->CreateRenderTargetView(blur_reflect_data, 0, &blur_reflect_target);
	if (FAILED(hr))
	{
		MessageBox(0, L"create blur reflect map texture1 error", L"tip", MB_OK);
		return hr;
	}
	blur_reflect_data->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~静态cubemap输入资源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	texDesc.Width = static_cast<float>(width_static_cube);
	texDesc.Height = static_cast<float>(width_static_cube);
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* tex_cubeinput[6];
	for (int i = 0; i < 6; ++i)
	{
		hr = device_pancy->CreateTexture2D(&texDesc, 0, &tex_cubeinput[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect cubemap depthtex error", L"tip", MB_OK);
			return hr;
		}
		hr = device_pancy->CreateShaderResourceView(tex_cubeinput[i], 0, &reflect_cubeinput_SRV[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
			return hr;
		}
		hr = device_pancy->CreateRenderTargetView(tex_cubeinput[i], 0, &reflect_cubeinput_RTV[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect map texture1 error", L"tip", MB_OK);
			return hr;
		}
		tex_cubeinput[i]->Release();
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~静态cubemap深度缓冲~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width = static_cast<float>(width_static_cube);
	dsDesc.Height = static_cast<float>(width_static_cube);
	dsDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	dsDesc.ArraySize = 6;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MipLevels = 1;
	dsDesc.MiscFlags = 0;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	ID3D11Texture2D* depthStencilBuffer;
	hr = device_pancy->CreateTexture2D(&dsDesc, 0, &depthStencilBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect cubemap depthtex error", L"tip", MB_OK);
		return hr;
	}
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc =
	{
		DXGI_FORMAT_D32_FLOAT,
		D3D11_DSV_DIMENSION_TEXTURE2DARRAY
	};
	dsvDesc.Texture2DArray.ArraySize = 1;
	dsvDesc.Texture2DArray.MipSlice = 0;
	for (int i = 0; i < 6; ++i)
	{
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		hr = device_pancy->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, &reflect_DSV[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect cubemap depthbuffer error", L"tip", MB_OK);
			return hr;
		}
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC depthsrvDesc =
	{
		DXGI_FORMAT_R32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2DARRAY
	};
	for (int i = 0; i < 6; ++i)
	{

		depthsrvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		depthsrvDesc.Texture2D.MostDetailedMip = 0;
		depthsrvDesc.Texture2DArray.ArraySize = 1;
		depthsrvDesc.Texture2DArray.FirstArraySlice = i;
		hr = device_pancy->CreateShaderResourceView(depthStencilBuffer, &depthsrvDesc, &reflect_depthcube_SRV[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect cubemap depthtex error", L"tip", MB_OK);
			return hr;
		}
	}

	depthStencilBuffer->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~静态cubemap纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//渲染目标
	D3D11_TEXTURE2D_DESC cubeMapDesc;
	cubeMapDesc.Width = static_cast<float>(width_static_cube);
	cubeMapDesc.Height = static_cast<float>(width_static_cube);
	cubeMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	cubeMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubeMapDesc.ArraySize = 6;
	cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapDesc.CPUAccessFlags = 0;
	cubeMapDesc.MipLevels = 1;
	cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	cubeMapDesc.SampleDesc.Count = 1;
	cubeMapDesc.SampleDesc.Quality = 0;
	//使用以上描述创建纹理
	ID3D11Texture2D *cubeMap(NULL);
	hr = device_pancy->CreateTexture2D(&cubeMapDesc, 0, &cubeMap);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect cubemap texture1 error", L"tip", MB_OK);
		return hr;
	}
	//创建六个rendertarget
	D3D11_RENDER_TARGET_VIEW_DESC rtv_stencil_Desc;
	rtv_stencil_Desc.Format = cubeMapDesc.Format;
	rtv_stencil_Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtv_stencil_Desc.Texture2DArray.ArraySize = 1;
	rtv_stencil_Desc.Texture2DArray.MipSlice = 0;
	for (UINT i = 0; i < 6; ++i)
	{
		rtv_stencil_Desc.Texture2DArray.FirstArraySlice = i;
		hr = device_pancy->CreateRenderTargetView(cubeMap, &rtv_stencil_Desc, &reflect_cube_RTV[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect cubemap stencil RTV error", L"tip", MB_OK);
			return hr;
		}
	}
	//创建一个SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc_stencil;
	srvDesc_stencil.Format = cubeMapDesc.Format;
	srvDesc_stencil.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc_stencil.TextureCube.MipLevels = 1;
	srvDesc_stencil.TextureCube.MostDetailedMip = 0;
	hr = device_pancy->CreateShaderResourceView(cubeMap, &srvDesc_stencil, &reflect_cube_SRV);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect cubemap SRV error", L"tip", MB_OK);
		return hr;
	}
	cubeMap->Release();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~创建cube面信息记录纹理~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//渲染目标
	cubeMapDesc.Width = static_cast<float>(width_static_cube);
	cubeMapDesc.Height = static_cast<float>(width_static_cube);
	cubeMapDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	cubeMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	cubeMapDesc.ArraySize = 6;
	cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
	cubeMapDesc.CPUAccessFlags = 0;
	cubeMapDesc.MipLevels = 1;
	cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	cubeMapDesc.SampleDesc.Count = 1;
	cubeMapDesc.SampleDesc.Quality = 0;
	//使用以上描述创建纹理
	ID3D11Texture2D *cubeMap_stencil(NULL);
	hr = device_pancy->CreateTexture2D(&cubeMapDesc, 0, &cubeMap_stencil);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect cubemap texture1 error", L"tip", MB_OK);
		return hr;
	}
	//创建六个rendertarget
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = cubeMapDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;
	rtvDesc.Texture2DArray.MipSlice = 0;
	for (UINT i = 0; i < 6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		hr = device_pancy->CreateRenderTargetView(cubeMap_stencil, &rtvDesc, &reflect_cubestencil_RTV[i]);
		if (FAILED(hr))
		{
			MessageBox(0, L"create reflect cubemap RTV error", L"tip", MB_OK);
			return hr;
		}
	}
	//创建一个SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = cubeMapDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;
	hr = device_pancy->CreateShaderResourceView(cubeMap_stencil, &srvDesc, &reflect_cubestencil_SRV);
	if (FAILED(hr))
	{
		MessageBox(0, L"create reflect cubemap SRV error", L"tip", MB_OK);
		return hr;
	}
	cubeMap_stencil->Release();
	return S_OK;
}
void render_posttreatment_SSR::set_static_cube_rendertarget(int count_cube, XMFLOAT4X4 &mat_project)
{
	ID3D11RenderTargetView* renderTargets[1] = { reflect_cubeinput_RTV[count_cube] };
	contex_pancy->OMSetRenderTargets(1, renderTargets, reflect_DSV[count_cube]);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
	contex_pancy->ClearRenderTargetView(reflect_cubeinput_RTV[count_cube], clearColor);
	contex_pancy->ClearDepthStencilView(reflect_DSV[count_cube], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, count_cube);
	D3D11_VIEWPORT shadow_map_VP;
	shadow_map_VP.TopLeftX = 0.0f;
	shadow_map_VP.TopLeftY = 0.0f;
	shadow_map_VP.Width = width_static_cube;
	shadow_map_VP.Height = width_static_cube;
	shadow_map_VP.MinDepth = 0.0f;
	shadow_map_VP.MaxDepth = 1.0f;
	contex_pancy->RSSetViewports(1, &shadow_map_VP);
	XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.5f*XM_PI, 1.0f, perspective_near_plane, perspective_far_plane);
	XMStoreFloat4x4(&mat_project, P);
}
void render_posttreatment_SSR::draw_static_cube(int count_cube)
{
	ID3D11RenderTargetView* renderTargets[2] = { reflect_cube_RTV[count_cube],reflect_cubestencil_RTV[count_cube] };
	contex_pancy->OMSetRenderTargets(2, renderTargets, NULL);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1e5f };
	contex_pancy->ClearRenderTargetView(reflect_cube_RTV[count_cube], clearColor);
	contex_pancy->ClearRenderTargetView(reflect_cubestencil_RTV[count_cube], clearColor);
	D3D11_VIEWPORT shadow_map_VP;
	shadow_map_VP.TopLeftX = 0.0f;
	shadow_map_VP.TopLeftY = 0.0f;
	shadow_map_VP.Width = width_static_cube;
	shadow_map_VP.Height = width_static_cube;
	shadow_map_VP.MinDepth = 0.0f;
	shadow_map_VP.MaxDepth = 1.0f;
	auto shader_draw = shader_list->get_shader_cubesave();
	XMFLOAT3 cubecount_vec = XMFLOAT3(static_cast<float>(count_cube), 0.0f, 0.0f);
	shader_draw->set_cube_count(cubecount_vec);
	shader_draw->set_texture_input(reflect_cubeinput_SRV[count_cube]);
	shader_draw->set_depthtex_input(reflect_depthcube_SRV[count_cube]);
	//渲染屏幕空间像素图
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &reflectMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(reflectMap_IB, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	//选定绘制路径
	shader_draw->get_technique(&tech, "resolove_alpha");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_draw->set_texture_input(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void render_posttreatment_SSR::BuildFrustumFarCorners(float fovy, float farZ)
{
	float aspect = (float)map_width / (float)map_height;

	float halfHeight = farZ * tanf(0.5f*fovy);
	float halfWidth = aspect * halfHeight;

	FrustumFarCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	FrustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	FrustumFarCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	FrustumFarCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
}
void render_posttreatment_SSR::set_size(int width, int height, float fovy, float farZ)
{
	map_width = width;
	map_height = height;
	//半屏幕渲染
	render_viewport.TopLeftX = 0.0f;
	render_viewport.TopLeftY = 0.0f;
	render_viewport.Width = static_cast<float>(width);
	render_viewport.Height = static_cast<float>(height);
	render_viewport.MinDepth = 0.0f;
	render_viewport.MaxDepth = 1.0f;
	BuildFrustumFarCorners(fovy, farZ);

	half_render_viewport.TopLeftX = 0.0f;
	half_render_viewport.TopLeftY = 0.0f;
	half_render_viewport.Width = static_cast<float>(width);
	half_render_viewport.Height = static_cast<float>(height);
	half_render_viewport.MinDepth = 0.0f;
	half_render_viewport.MaxDepth = 1.0f;
	BuildFrustumFarCorners(fovy, farZ);
}
void render_posttreatment_SSR::build_fullscreen_picturebuff()
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

	device_pancy->CreateBuffer(&vbd, &vinitData, &reflectMap_VB);

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

	device_pancy->CreateBuffer(&ibd, &iinitData, &reflectMap_IB);
}
void render_posttreatment_SSR::set_normaldepthcolormap(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need)
{
	normaldepth_tex = normalspec_need;
	depth_tex = depth_need;
}
void render_posttreatment_SSR::release()
{
	safe_release(mask_tex);
	safe_release(mask_target);
	safe_release(input_mask_tex);
	safe_release(reflectMap_VB);
	safe_release(reflectMap_IB);
	safe_release(reflect_target);
	safe_release(reflect_tex);
	safe_release(color_tex);
	safe_release(reflect_cube_SRV);
	safe_release(final_reflect_target);
	safe_release(final_reflect_tex);
	safe_release(blur_reflect_target);
	safe_release(blur_reflect_tex);
	safe_release(reflect_cubestencil_SRV);
	for (int i = 0; i < 6; ++i)
	{
		safe_release(reflect_depthcube_SRV[i]);
		safe_release(reflect_cubeinput_RTV[i]);
		safe_release(reflect_cubeinput_SRV[i]);
		safe_release(reflect_cube_RTV[i]);
		safe_release(reflect_DSV[i]);
		safe_release(reflect_cubestencil_RTV[i]);
	}
}
void render_posttreatment_SSR::build_reflect_map(ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input)
{
	//将多重采样纹理转换至非多重纹理
	ID3D11Resource *rendertargetTex = 0;
	ID3D11Resource *rendertargetTex_singlesample = 0;
	rendertarget_input->GetResource(&rendertargetTex);
	color_tex->GetResource(&rendertargetTex_singlesample);
	contex_pancy->ResolveSubresource(rendertargetTex_singlesample, D3D11CalcSubresource(0, 0, 1), rendertargetTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	color_tex->Release();
	color_tex = NULL;
	device_pancy->CreateShaderResourceView(rendertargetTex_singlesample, 0, &color_tex);
	rendertargetTex->Release();
	rendertargetTex_singlesample->Release();

	rendertargetTex = 0;
	rendertargetTex_singlesample = 0;
	mask_target_input->GetResource(&rendertargetTex);
	input_mask_tex->GetResource(&rendertargetTex_singlesample);
	contex_pancy->ResolveSubresource(rendertargetTex_singlesample, D3D11CalcSubresource(0, 0, 1), rendertargetTex, D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R16G16B16A16_FLOAT);
	input_mask_tex->Release();
	input_mask_tex = NULL;
	device_pancy->CreateShaderResourceView(rendertargetTex_singlesample, 0, &input_mask_tex);
	rendertargetTex->Release();
	rendertargetTex_singlesample->Release();
	//绑定渲染目标纹理，不设置深度模缓冲区因为这里不需要
	ID3D11RenderTargetView* renderTargets[2] = { reflect_target,mask_target };
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	contex_pancy->OMSetRenderTargets(2, renderTargets, 0);
	contex_pancy->ClearRenderTargetView(reflect_target, clearColor);
	contex_pancy->RSSetViewports(1, &half_render_viewport);
	//renderstate_lib->clear_posttreatmentcrendertarget();
	//设置渲染状态
	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(perspective_angle, map_width*1.0f / map_height*1.0f, perspective_near_plane, perspective_far_plane);
	XMFLOAT4X4 PT;
	XMStoreFloat4x4(&PT, P*T);
	auto *shader_reflectpass = shader_list->get_shader_ssreflect();
	XMFLOAT4X4 invview_mat, view_mat;
	XMFLOAT3 view_pos;
	camera_use->count_invview_matrix(&invview_mat);
	camera_use->count_view_matrix(&view_mat);
	XMMATRIX rec_need = XMLoadFloat4x4(&invview_mat) * XMLoadFloat4x4(&view_mat);

	camera_use->get_view_position(&view_pos);
	shader_reflectpass->set_invview_matrix(&invview_mat);
	shader_reflectpass->set_view_matrix(&view_mat);
	shader_reflectpass->set_view_pos(view_pos);
	shader_reflectpass->set_ViewToTexSpace(&PT);
	shader_reflectpass->set_FrustumCorners(FrustumFarCorner);
	shader_reflectpass->set_NormalDepthtex(normaldepth_tex);
	shader_reflectpass->set_Depthtex(depth_tex);
	shader_reflectpass->set_diffusetex(color_tex);
	shader_reflectpass->set_color_mask_tex(input_mask_tex);

	shader_reflectpass->set_camera_positions(center_position);
	shader_reflectpass->set_cubeview_matrix(static_cube_view_matrix, 6);
	//shader_reflectpass->set_enviroment_depth(reflect_depthcube_SRV);
	shader_reflectpass->set_enviroment_tex(reflect_cube_SRV);
	shader_reflectpass->set_enviroment_stencil(reflect_cubestencil_SRV);
	//渲染屏幕空间像素图
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &reflectMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(reflectMap_IB, DXGI_FORMAT_R16_UINT, 0);

	ID3DX11EffectTechnique* tech, *tech_cube;
	//选定绘制路径
	shader_reflectpass->get_technique(&tech, "draw_ssrmap");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	tech->GetPassByIndex(0)->Apply(0, contex_pancy);
	contex_pancy->DrawIndexed(6, 0, 0);
	shader_reflectpass->set_diffusetex(color_tex);
	shader_reflectpass->set_color_mask_tex(mask_tex);
	shader_reflectpass->set_color_ssr_tex(reflect_tex);
	ID3D11RenderTargetView* NULL_target[2] = { NULL,NULL };
	contex_pancy->OMSetRenderTargets(2, NULL_target, 0);

	//basic_blur(mask_tex,reflect_tex, blur_reflect_target, true);
	//basic_blur(mask_tex,blur_reflect_tex, reflect_target, false);
	//basic_blur(mask_tex, reflect_tex, blur_reflect_target, true);
	//basic_blur(mask_tex, blur_reflect_tex, reflect_target, false);
	//basic_blur(mask_tex, blur_reflect_target, true);
	//basic_blur(blur_reflect_tex, mask_target, false);

	ID3D11RenderTargetView* renderTarget_final[1] = { final_reflect_target };
	contex_pancy->OMSetRenderTargets(1, renderTarget_final, 0);
	contex_pancy->ClearRenderTargetView(final_reflect_target, clearColor);


	tech->GetPassByIndex(1)->Apply(0, contex_pancy);
	contex_pancy->DrawIndexed(6, 0, 0);

	shader_reflectpass->set_NormalDepthtex(NULL);
	shader_reflectpass->set_diffusetex(NULL);
	shader_reflectpass->set_enviroment_tex(NULL);
	shader_reflectpass->set_color_mask_tex(NULL);
	shader_reflectpass->set_color_ssr_tex(NULL);
	contex_pancy->OMSetRenderTargets(1, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void render_posttreatment_SSR::blur_map()
{
	basic_blur(mask_tex,final_reflect_tex, blur_reflect_target, true);
	basic_blur(mask_tex,blur_reflect_tex, final_reflect_target, false);
	//basic_blur(final_reflect_tex, blur_reflect_target, true);
	//basic_blur(blur_reflect_tex, final_reflect_target, false);
}
void render_posttreatment_SSR::basic_blur(ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz)
{
	//设置渲染目标
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	ID3D11RenderTargetView* renderTargets[1] = { output };
	contex_pancy->OMSetRenderTargets(1, renderTargets, 0);
	contex_pancy->ClearRenderTargetView(output, black);
	contex_pancy->RSSetViewports(1, &half_render_viewport);
	auto shader_blur = shader_list->get_shader_reflect_blur();
	XMFLOAT4 map_range = XMFLOAT4(1.0f / half_render_viewport.Width, 1.0f / half_render_viewport.Height, 1.0f / render_viewport.Width, 1.0f / render_viewport.Height);
	shader_blur->set_image_size(map_range);
	shader_blur->set_tex_resource(input);
	shader_blur->set_tex_normal_resource(normaldepth_tex);
	shader_blur->set_tex_depth_resource(depth_tex);
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &reflectMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(reflectMap_IB, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	if (if_horz == true)
	{
		shader_blur->get_technique(&tech, "HorzBlur");
	}
	else
	{
		shader_blur->get_technique(&tech, "VertBlur");
	}
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_blur->set_tex_resource(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void render_posttreatment_SSR::basic_blur(ID3D11ShaderResourceView *mask, ID3D11ShaderResourceView *input, ID3D11RenderTargetView *output, bool if_horz)
{
	//设置渲染目标
	float black[4] = { 0.0f,0.0f,0.0f,0.0f };
	ID3D11RenderTargetView* renderTargets[1] = { output };
	contex_pancy->OMSetRenderTargets(1, renderTargets, 0);
	contex_pancy->ClearRenderTargetView(output, black);
	contex_pancy->RSSetViewports(1, &half_render_viewport);
	auto shader_blur = shader_list->get_shader_reflect_blur();
	XMFLOAT4 map_range = XMFLOAT4(1.0f / half_render_viewport.Width, 1.0f / half_render_viewport.Height, 1.0f / render_viewport.Width, 1.0f / render_viewport.Height);
	shader_blur->set_image_size(map_range);
	shader_blur->set_tex_resource(input);
	shader_blur->set_tex_normal_resource(normaldepth_tex);
	shader_blur->set_tex_depth_resource(depth_tex);
	shader_blur->set_tex_mask_resource(mask);
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &reflectMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(reflectMap_IB, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	if (if_horz == true)
	{
		shader_blur->get_technique(&tech, "HorzBlur_color");
	}
	else
	{
		shader_blur->get_technique(&tech, "VertBlur_color");
	}
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_blur->set_tex_resource(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void render_posttreatment_SSR::draw_to_posttarget()
{
	renderstate_lib->set_posttreatment_rendertarget();
	renderstate_lib->clear_posttreatmentcrendertarget();
	auto shader_final_pass = shader_list->get_shader_reflect_final();
	shader_final_pass->set_tex_color_resource(color_tex);
	shader_final_pass->set_tex_reflect_resource(final_reflect_tex);
	XMFLOAT4 map_range = XMFLOAT4(1.0f / half_render_viewport.Width, 1.0f / half_render_viewport.Height, 1.0f / render_viewport.Width, 1.0f / render_viewport.Height);
	shader_final_pass->set_image_size(map_range);
	UINT stride = sizeof(pancy_point);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &reflectMap_VB, &stride, &offset);
	contex_pancy->IASetIndexBuffer(reflectMap_IB, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	shader_final_pass->get_technique(&tech, "blend_reflect");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(6, 0, 0);
	}
	shader_final_pass->set_tex_color_resource(NULL);
	shader_final_pass->set_tex_reflect_resource(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void render_posttreatment_SSR::draw_reflect(ID3D11RenderTargetView *rendertarget_input, ID3D11RenderTargetView *mask_target_input)
{
	build_reflect_map(rendertarget_input,mask_target_input);
	blur_map();
	draw_to_posttarget();
	renderstate_lib->clear_basicrendertarget();
}

void render_posttreatment_SSR::set_static_cube_view_matrix(int count_cube, XMFLOAT4X4 mat_input)
{
	static_cube_view_matrix[count_cube] = mat_input;
}
void render_posttreatment_SSR::set_static_cube_centerposition(XMFLOAT3 pos_input)
{
	center_position = pos_input;
}