#include"pancy_posttreatment.h"
render_posttreatment_HDR::render_posttreatment_HDR(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, ID3D11RenderTargetView *rendertarget_need, shader_control *shaderlist_need, int width_need, int height_need, d3d_pancy_basic *rec_rootstate)
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