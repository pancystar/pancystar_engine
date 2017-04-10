#include"pancy_scene_design.h"
shader_snakecompute::shader_snakecompute(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_snakecompute::set_input_buffer(ID3D11ShaderResourceView *buffer_input)
{
	HRESULT hr = snakecontrol_input->SetResource(buffer_input);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when setting snake buffer input", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_snakecompute::set_output_buffer(ID3D11UnorderedAccessView *buffer_snake_draw, ID3D11UnorderedAccessView *buffer_snake_next)
{
	HRESULT hr;
	hr = snakepoint_output->SetUnorderedAccessView(buffer_snake_draw);
	if (FAILED(hr))
	{
		MessageBox(0, L"set UAV buffer error", L"tip", MB_OK);
		return hr;
	}
	hr = snakecontrol_output->SetUnorderedAccessView(buffer_snake_next);
	if (FAILED(hr))
	{
		MessageBox(0, L"set UAV buffer error", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_snakecompute::set_piccturerange(int snake_body_num, int devide_num, int radius, int others)
{
	XMUINT4 rec_float = XMUINT4(static_cast<unsigned int>(snake_body_num), static_cast<unsigned int>(devide_num), static_cast<unsigned int>(radius), static_cast<unsigned int>(others));
	HRESULT hr = snake_range->SetRawValue((void*)&rec_float, 0, sizeof(rec_float));
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when setting snake range", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT shader_snakecompute::set_snake_head(XMFLOAT3 head_input)
{
	HRESULT hr = snake_head->SetRawValue((void*)&head_input, 0, sizeof(head_input));
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when setting snake head", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_snakecompute::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
}
void shader_snakecompute::release()
{
	release_basic();
}
void shader_snakecompute::init_handle()
{
	snakecontrol_input = fx_need->GetVariableByName("input_buffer")->AsShaderResource();
	snakepoint_output = fx_need->GetVariableByName("output_buffer")->AsUnorderedAccessView();
	snakecontrol_output = fx_need->GetVariableByName("next_buffer")->AsUnorderedAccessView();
	Bspline_matrix = fx_need->GetVariableByName("Bspline_mat")->AsMatrix();
	snake_range = fx_need->GetVariableByName("input_range");
	snake_head = fx_need->GetVariableByName("snake_head_position");
}
void shader_snakecompute::dispatch(int snake_num, int snake_devide)
{
	XMFLOAT4X4 mat_Bspline;
	mat_Bspline._11 = -1.0f / 6.0f;
	mat_Bspline._12 = 3.0f / 6.0f;
	mat_Bspline._13 = -3.0f / 6.0f;
	mat_Bspline._14 = 1.0f / 6.0f;

	mat_Bspline._21 = 3.0f / 6.0f;
	mat_Bspline._22 = -6.0f / 6.0f;
	mat_Bspline._23 = 3.0f / 6.0f;
	mat_Bspline._24 = 0.0f / 6.0f;

	mat_Bspline._31 = -3.0f / 6.0f;
	mat_Bspline._32 = 0.0f / 6.0f;
	mat_Bspline._33 = 3.0f / 6.0f;
	mat_Bspline._34 = 0.0f / 6.0f;

	mat_Bspline._41 = 1.0f / 6.0f;
	mat_Bspline._42 = 4.0f / 6.0f;
	mat_Bspline._43 = 1.0f / 6.0f;
	mat_Bspline._44 = 0.0f / 6.0f;
	HRESULT hr = set_matrix(Bspline_matrix, &mat_Bspline);
	ID3DX11EffectTechnique* tech_need;
	tech_need = fx_need->GetTechniqueByName("snake_square_pass");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech_need->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		int width = snake_num / 16, height = snake_devide / 16;
		if (snake_num % 16 != 0)
		{
			width = snake_num / 16 + 1;
		}
		if (snake_devide % 16 != 0)
		{
			height = snake_devide / 16 + 1;
		}
		tech_need->GetPassByIndex(i)->Apply(0, contex_pancy);
		contex_pancy->Dispatch(width, height, 1);
	}
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	contex_pancy->CSSetShaderResources(0, 1, nullSRV);
	ID3D11UnorderedAccessView* nullUAV[2] = { 0,0 };
	contex_pancy->CSSetUnorderedAccessViews(0, 2, nullUAV, 0);
	contex_pancy->CSSetShader(0, 0, 0);
}

shader_snaketesselate::shader_snaketesselate(LPCWSTR filename, ID3D11Device *device_need, ID3D11DeviceContext *contex_need) :shader_basic(filename, device_need, contex_need)
{
}
HRESULT shader_snaketesselate::set_trans_all(XMFLOAT4X4 *mat_need)
{
	HRESULT hr = set_matrix(final_mat, mat_need);;
	if (hr != S_OK)
	{
		MessageBox(0, L"an error when setting project matrix", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
void shader_snaketesselate::release()
{
	release_basic();
}
void shader_snaketesselate::init_handle()
{
	final_mat = fx_need->GetVariableByName("final_matrix")->AsMatrix();
}
void shader_snaketesselate::set_inputpoint_desc(D3D11_INPUT_ELEMENT_DESC *member_point, UINT *num_member)
{
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION",0  ,DXGI_FORMAT_R32G32B32A32_FLOAT   ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "NORMAL"  ,0  ,DXGI_FORMAT_R32G32B32A32_FLOAT   ,0    ,16 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
	};
	*num_member = sizeof(rec) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	for (UINT i = 0; i < *num_member; ++i)
	{
		member_point[i] = rec[i];
	}
}


HRESULT snake_draw::snake_rotate()
{
	XMMATRIX view;
	//user_input->get_input();
	if (user_input->check_keyboard(DIK_UPARROW))
	{
		snake_head_normal->rotation_right(-0.02f);
	}
	if (user_input->check_keyboard(DIK_DOWNARROW))
	{
		snake_head_normal->rotation_right(0.02f);
	}
	if (user_input->check_keyboard(DIK_LEFTARROW))
	{
		snake_head_normal->rotation_up(-0.02f);
	}
	if (user_input->check_keyboard(DIK_RIGHTARROW))
	{
		snake_head_normal->rotation_up(0.02f);
	}
	return S_OK;
}
snake_draw::snake_draw(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_input *input_need, int max_length_need, int devide_num_need)
{
	snake_head_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	snake_head_normal = new pancy_camera(device_need, 0, 0);
	snake_length = 0;
	snake_radius = 1;
	max_snake_length = max_length_need * 60;
	devide_num = devide_num_need;
	device_pancy = device_need;
	contex_pancy = contex_need;
	user_input = input_need;
	UAV_controlpoint_first = NULL;
	SRV_controlpoint_first = NULL;
	UAV_controlpoint_second = NULL;
	SRV_controlpoint_second = NULL;
	UAV_draw_point_bufeer = NULL;
	first_CSshader = NULL;
	second_TLshader = NULL;
	time_all = 0.0f;
}
HRESULT snake_draw::build_controlbuffer()
{
	CreateCPUaccessBuf();
	HRESULT hr;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~创建缓冲区~~~~~~~~~~~~~~~~~~~~~~~~~~~
	ID3D11Buffer *buffer_controlpoint_first;
	ID3D11Buffer *buffer_controlpoint_second;
	//控制点缓冲区
	D3D11_BUFFER_DESC controlpoint_buffer_desc;
	controlpoint_buffer_desc.Usage = D3D11_USAGE_DEFAULT;            //通用类型
	controlpoint_buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//缓存类型为uav+srv
	controlpoint_buffer_desc.ByteWidth = max_snake_length * sizeof(point_snake_control);        //顶点缓存的大小
	controlpoint_buffer_desc.CPUAccessFlags = 0;
	controlpoint_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	controlpoint_buffer_desc.StructureByteStride = sizeof(point_snake_control);
	//第一个buffer
	point_snake_control *data_start;
	data_start = new point_snake_control[max_snake_length];
	data_start[0].position1 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	data_start[0].position2 = XMFLOAT3(1.0f, 0.0f, 1.0f);
	data_start[0].position3 = XMFLOAT3(2.0f, 3.0f, 2.0f);
	data_start[0].position4 = XMFLOAT3(8.0f, 1.0f, 4.0f);

	data_start[1].position1 = XMFLOAT3(1.0f, 0.0f, 1.0f);
	data_start[1].position2 = XMFLOAT3(2.0f, 3.0f, 2.0f);
	data_start[1].position3 = XMFLOAT3(8.0f, 1.0f, 4.0f);
	data_start[1].position4 = XMFLOAT3(12.0f, 2.0f, 7.0f);
	snake_length = 12 * 30;
	D3D11_SUBRESOURCE_DATA data_need;
	data_need.pSysMem = data_start;
	hr = device_pancy->CreateBuffer(&controlpoint_buffer_desc, &data_need, &buffer_controlpoint_first);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create snake controlpoint buffer", L"tip", MB_OK);
		return hr;
	}
	delete[] data_start;
	//第二个buffer
	hr = device_pancy->CreateBuffer(&controlpoint_buffer_desc, NULL, &buffer_controlpoint_second);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create HDR buffer", L"tip", MB_OK);
		return hr;
	}
	//控制点UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = max_snake_length;
	//创建第一个UAV，代表第一个buffer
	hr = device_pancy->CreateUnorderedAccessView(buffer_controlpoint_first, &DescUAV, &UAV_controlpoint_first);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create controlpoint UAV", L"tip", MB_OK);
		return hr;
	}
	//创建第二个UAV，代表第二个buffer
	hr = device_pancy->CreateUnorderedAccessView(buffer_controlpoint_second, &DescUAV, &UAV_controlpoint_second);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create controlpoint UAV", L"tip", MB_OK);
		return hr;
	}
	//控制点SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC DescSRV;
	ZeroMemory(&DescSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	DescSRV.Format = DXGI_FORMAT_UNKNOWN;
	DescSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	DescSRV.Buffer.FirstElement = 0;
	DescSRV.Buffer.NumElements = max_snake_length;
	//创建第一个SRV，代表第一个buffer
	hr = device_pancy->CreateShaderResourceView(buffer_controlpoint_first, &DescSRV, &SRV_controlpoint_first);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create controlpoint SRV", L"tip", MB_OK);
		return hr;
	}
	//创建第e二个SRV，代表第二个buffer
	hr = device_pancy->CreateShaderResourceView(buffer_controlpoint_second, &DescSRV, &SRV_controlpoint_second);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create controlpoint SRV", L"tip", MB_OK);
		return hr;
	}
	buffer_controlpoint_first->Release();
	buffer_controlpoint_second->Release();
	return S_OK;
}
HRESULT snake_draw::build_render_buffer()
{
	int buffer_length = max_snake_length * devide_num * 16 + 100;
	HRESULT hr;

	//绘制点缓冲区
	D3D11_BUFFER_DESC renderpoint_buffer_desc = {};
	renderpoint_buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_VERTEX_BUFFER;
	renderpoint_buffer_desc.ByteWidth = sizeof(point_snake) * buffer_length;
	renderpoint_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	//第一个buffer
	hr = device_pancy->CreateBuffer(&renderpoint_buffer_desc, NULL, &point_buffer_UAV);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create snake controlpoint buffer", L"tip", MB_OK);
		return hr;
	}
	//绘制点缓冲区
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
	DescUAV.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
	DescUAV.Buffer.NumElements = renderpoint_buffer_desc.ByteWidth / 4;
	//创建第一个UAV，绘制缓冲区
	hr = device_pancy->CreateUnorderedAccessView(point_buffer_UAV, &DescUAV, &UAV_draw_point_bufeer);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create snake draw UAV", L"tip", MB_OK);
		return hr;
	}
	/*
	D3D11_BUFFER_DESC renderpoint_buffer_desc;
	renderpoint_buffer_desc.Usage = D3D11_USAGE_DEFAULT;            //通用类型
	renderpoint_buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;   //缓存类型为uav+buffer
	renderpoint_buffer_desc.ByteWidth = buffer_length * sizeof(point_snake); //顶点缓存的大小
	renderpoint_buffer_desc.CPUAccessFlags = 0;
	renderpoint_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	renderpoint_buffer_desc.StructureByteStride = sizeof(point_snake);
	//第一个buffer
	hr = device_pancy->CreateBuffer(&renderpoint_buffer_desc, NULL, &point_buffer_UAV);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create snake controlpoint buffer", L"tip", MB_OK);
		return hr;
	}
	//绘制点缓冲区
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements = buffer_length;
	//创建第一个UAV，绘制缓冲区
	hr = device_pancy->CreateUnorderedAccessView(point_buffer_UAV, &DescUAV, &UAV_draw_point_bufeer);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create snake draw UAV", L"tip", MB_OK);
		return hr;
	}
	//细分绘制用的顶点缓冲区
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(point_snake) * buffer_length;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	//D3D11_SUBRESOURCE_DATA vinitData;
	//vinitData.pSysMem = v;

	hr = device_pancy->CreateBuffer(&vbd, 0, &point_buffer_render);
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when create snake draw buffer", L"tip", MB_OK);
		return hr;
	}
	*/
	USHORT *index_need = new USHORT[(buffer_length / 4) * 6];
	for (int i = 0; i < (buffer_length / 4) * 6; i += 6)
	{
		index_need[i] = (i / 6) * 4 + 0;
		index_need[i + 1] = (i / 6) * 4 + 1;
		index_need[i + 2] = (i / 6) * 4 + 2;
		index_need[i + 3] = (i / 6) * 4 + 0;
		index_need[i + 4] = (i / 6) * 4 + 2;
		index_need[i + 5] = (i / 6) * 4 + 3;
	}
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * (buffer_length / 4) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = index_need;

	hr = device_pancy->CreateBuffer(&ibd, &iinitData, &index_buffer_render);
	if (FAILED(hr))
	{
		MessageBox(0, L"error when create snake index buffer", L"tip", MB_OK);
		return hr;
	}
	delete[] index_need;

	//contex_pancy->CopyResource(point_buffer_render, point_buffer_UAV);

	return S_OK;
}
void snake_draw::release()
{
	safe_release(UAV_controlpoint_first);
	safe_release(SRV_controlpoint_first);
	safe_release(UAV_controlpoint_second);
	safe_release(SRV_controlpoint_second);
	safe_release(UAV_draw_point_bufeer);
	safe_release(point_buffer_UAV);
	safe_release(CPU_read_buffer);
	safe_release(index_buffer_render);
	first_CSshader->release();
	second_TLshader->release();
}
HRESULT snake_draw::create()
{
	HRESULT hr;
	first_CSshader = new shader_snakecompute(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\snake_compute.cso", device_pancy, contex_pancy);
	hr = first_CSshader->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when snake CS shader created", L"tip", MB_OK);
		return hr;
	}
	second_TLshader = new shader_snaketesselate(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\snake_tesselation.cso", device_pancy, contex_pancy);
	hr = second_TLshader->shder_create();
	if (FAILED(hr))
	{
		MessageBox(0, L"an error when snake tesselation shader created", L"tip", MB_OK);
		return hr;
	}
	if (FAILED(build_controlbuffer()))
	{
		return E_FAIL;
	}
	if (FAILED(build_render_buffer()))
	{
		return E_FAIL;
	}
	return S_OK;
}
HRESULT snake_draw::CreateCPUaccessBuf()
{
	int buffer_length = max_snake_length;
	//控制点缓冲区
	D3D11_BUFFER_DESC controlpoint_buffer_desc;
	controlpoint_buffer_desc.Usage = D3D11_USAGE_STAGING;
	controlpoint_buffer_desc.BindFlags = 0;
	controlpoint_buffer_desc.ByteWidth = buffer_length * sizeof(point_snake_control);
	controlpoint_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	controlpoint_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	controlpoint_buffer_desc.StructureByteStride = sizeof(point_snake_control);

	HRESULT hr = device_pancy->CreateBuffer(&controlpoint_buffer_desc, NULL, &CPU_read_buffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"create CPU access read buffer error", L"tip", MB_OK);
		return hr;
	}
	/*
	int buffer_length = max_snake_length * devide_num * 16 + 100;
	//控制点缓冲区
	D3D11_BUFFER_DESC controlpoint_buffer_desc;
	controlpoint_buffer_desc.Usage = D3D11_USAGE_STAGING;
	controlpoint_buffer_desc.BindFlags = 0;
	controlpoint_buffer_desc.ByteWidth = buffer_length * sizeof(point_snake);
	controlpoint_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	controlpoint_buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	controlpoint_buffer_desc.StructureByteStride = sizeof(point_snake);

	HRESULT hr = device_pancy->CreateBuffer(&controlpoint_buffer_desc, NULL, &CPU_read_buffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"create CPU access read buffer error", L"tip", MB_OK);
		return hr;
	}
	*/
	return S_OK;
}
void snake_draw::draw(XMFLOAT4X4 view_projmat)
{
	//XMVECTOR check = XMLoadFloat3(&XMFLOAT3(-0.727,0.485,0.000));
	//XMMATRIX rec =  XMMatrixRotationAxis(check,1.0643);
	first_CSshader->set_input_buffer(SRV_controlpoint_first);
	first_CSshader->set_output_buffer(UAV_draw_point_bufeer, UAV_controlpoint_second);
	first_CSshader->set_piccturerange(snake_length, devide_num, snake_radius, 0);
	first_CSshader->set_snake_head(snake_head_position);
	first_CSshader->dispatch(snake_length, devide_num);

	if (time_all > 1.0f / 30.0f)
	{
		std::swap(UAV_controlpoint_first, UAV_controlpoint_second);
		std::swap(SRV_controlpoint_first, SRV_controlpoint_second);
		time_all -= 1.0f / 30.0f;
	}

	/*
	ID3D11Resource *test;
	UAV_controlpoint_second->GetResource(&test);
	contex_pancy->CopyResource(CPU_read_buffer, test);
	test->Release();
	D3D11_MAPPED_SUBRESOURCE mappedTex2D;
	HRESULT hr;
	D3D11_BUFFER_DESC rec_now;
	CPU_read_buffer->GetDesc(&rec_now);
	hr = contex_pancy->Map(CPU_read_buffer, 0, D3D11_MAP_READ, 0, &mappedTex2D);
	point_snake_control rec_need[60];
	memcpy(rec_need, mappedTex2D.pData, 60 * sizeof(point_snake_control));
	//point_snake* rec = static_cast<point_snake*>(mappedTex2D.pData);
	contex_pancy->Unmap(CPU_read_buffer, 0);
	*/
	draw_pass(view_projmat);
}
void snake_draw::draw_pass(XMFLOAT4X4 view_projmat)
{
	second_TLshader->set_trans_all(&view_projmat);
	UINT stride = sizeof(point_snake);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	contex_pancy->IASetVertexBuffers(0, 1, &point_buffer_UAV, &stride, &offset);
	ID3DX11EffectTechnique* tech;
	second_TLshader->get_technique(&tech, "draw_snake");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		int buffer_length = snake_length * devide_num * 16;
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->Draw(buffer_length, 0);
	}
	contex_pancy->HSSetShader(NULL, 0, 0);
	contex_pancy->DSSetShader(NULL, 0, 0);
}
void snake_draw::update(float time_delta)
{

	time_all += time_delta;
	//float devide = 1 / 60.0;
	if (time_all > 1.0f / 30.0f)
	{
		snake_rotate();
		XMFLOAT3 snake_normal;
		snake_head_normal->get_view_direct(&snake_normal);
		snake_head_position.x = snake_head_position.x + snake_normal.x * (2.0f / 60.0f);
		snake_head_position.y = snake_head_position.y + snake_normal.y * (2.0f / 60.0f);
		snake_head_position.z = snake_head_position.z + snake_normal.z * (2.0f / 60.0f);
	}

}



scene_root::scene_root(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height, float near_plane, float far_plane, float angle_view)
{
	if_close_window = false;
	if_stop = false;
	user_input = input_need;
	scene_camera = camera_need;
	shader_lib = lib_need;
	device_pancy = device_need;
	contex_pancy = contex_need;
	scene_window_width = width;
	scene_window_height = height;
	//engine_state = engine_root;
	renderstate_lib = render_state;
	geometry_lib = geometry_need;
	light_list = light_need;
	time_game = 0.0f;
	//初始化投影以及取景变换矩阵
	XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(angle_view, scene_window_width*1.0f / scene_window_height*1.0f, near_plane, far_plane);
	perspective_near_plane = near_plane;
	perspective_far_plane = far_plane;
	perspective_angle = angle_view;
	//ssao_part = new ssao_pancy(render_state,device_need, contex_need, shader_lib,geometry_lib,scene_window_width, scene_window_height, XM_PI*0.25f, 300.0f);
	XMStoreFloat4x4(&proj_matrix, proj);
	XMMATRIX iden = XMMatrixIdentity();
	XMStoreFloat4x4(&view_matrix, iden);
}
HRESULT scene_root::camera_move()
{
	float move_speed = 0.15f;
	XMMATRIX view;
	user_input->get_input();
	if (user_input->check_keyboard(DIK_A))
	{
		scene_camera->walk_right(-move_speed);
	}
	if (user_input->check_keyboard(DIK_W))
	{
		scene_camera->walk_front(move_speed);
	}
	if (user_input->check_keyboard(DIK_R))
	{
		scene_camera->walk_up(move_speed);
	}
	if (user_input->check_keyboard(DIK_D))
	{
		scene_camera->walk_right(move_speed);
	}
	if (user_input->check_keyboard(DIK_S))
	{
		scene_camera->walk_front(-move_speed);
	}
	if (user_input->check_keyboard(DIK_F))
	{
		scene_camera->walk_up(-move_speed);
	}
	if (user_input->check_keyboard(DIK_Q))
	{
		scene_camera->rotation_look(0.001f);
	}
	if (user_input->check_keyboard(DIK_E))
	{
		scene_camera->rotation_look(-0.001f);
	}
	if (user_input->check_mouseDown(1))
	{
		scene_camera->rotation_up(user_input->MouseMove_X() * 0.001f);
		scene_camera->rotation_right(user_input->MouseMove_Y() * 0.001f);
	}
	scene_camera->count_view_matrix(&view_matrix);
	//XMStoreFloat4x4(&view_matrix, view);
	return S_OK;
}
void scene_root::set_proj_matrix(XMFLOAT4X4 proj_mat_need)
{
	proj_matrix = proj_mat_need;
}
void scene_root::reset_proj_matrix()
{
	XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(perspective_angle, scene_window_width*1.0f / scene_window_height*1.0f, perspective_near_plane, perspective_far_plane);
	XMStoreFloat4x4(&proj_matrix, proj);
}
void scene_root::get_gbuffer(ID3D11ShaderResourceView *normalspec_need, ID3D11ShaderResourceView *depth_need)
{
	gbuffer_normalspec = normalspec_need;
	gbuffer_depth = depth_need;
}
void scene_root::get_lbuffer(ID3D11ShaderResourceView *diffuse_need, ID3D11ShaderResourceView *specular_need)
{
	lbuffer_diffuse = diffuse_need;
	lbuffer_specular = specular_need;
}

scene_engine_test::scene_engine_test(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height, float near_plane, float far_plane, float angle_view) : scene_root(device_need, contex_need, render_state, input_need, camera_need, lib_need, geometry_need, light_need, width, height, near_plane, far_plane, angle_view)
{
	//nonshadow_light_list.clear();
	//shadowmap_light_list.clear();
	particle_fire = new particle_system<fire_point>(device_need, contex_need, 3000, lib_need, PARTICLE_TYPE_FIRE);
}
HRESULT scene_engine_test::scene_create()
{
	HRESULT hr_need;
	//光源注册
	basic_lighting rec_need(point_light, shadow_none, shader_lib, device_pancy, contex_pancy, renderstate_lib, geometry_lib);
	light_list->add_light_without_shadow(rec_need);
	rec_need.set_light_range(5.0f);
	rec_need.set_light_ambient(0.0f, 0.0f, 0.0f, 0.0f);
	rec_need.set_light_diffuse(0.3f, 0.3f, 0.0f, 0.0f);
	rec_need.set_light_specular(0.3f, 0.3f, 0.0f, 0.0f);
	rec_need.set_light_decay(0.2f, 1.2f, 0.0f);
	light_list->add_light_without_shadow(rec_need);
	for (int i = 0; i < 6; ++i)
	{
		rec_need.set_light_position(7.3f, 4.8f, 13.0f - 4.8f*i);
		light_list->add_light_without_shadow(rec_need);
		//nonshadow_light_list.push_back(rec_need);
	}
	for (int i = 0; i < 6; ++i)
	{
		rec_need.set_light_position(-7.3f, 4.8f, 12.85f - 4.8*i);
		light_list->add_light_without_shadow(rec_need);
		//nonshadow_light_list.push_back(rec_need);
	}
	light_with_shadowmap rec_shadow(direction_light, shadow_map, shader_lib, device_pancy, contex_pancy, renderstate_lib, geometry_lib);
	hr_need = rec_shadow.create(1024, 1024);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	light_list->add_light_witn_shadow_map(rec_shadow);
	/*
	light_with_shadowmap rec_shadow1(spot_light, shadow_map, shader_lib, device_pancy, contex_pancy, renderstate_lib, geometry_lib);
	rec_shadow1.set_light_position(1,2.5,2.5);
	rec_shadow1.set_light_dir(-1.0f, -1.0f, -1.0f);
	hr_need = rec_shadow1.create(1024, 1024);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	light_list->add_light_witn_shadow_map(rec_shadow1);

	light_with_shadowmap rec_shadow2(spot_light, shadow_map, shader_lib, device_pancy, contex_pancy, renderstate_lib, geometry_lib);
	rec_shadow2.set_light_position(-1, 2.5, 2.5);
	rec_shadow2.set_light_dir(1.0f, -1.0f, -1.0f);
	hr_need = rec_shadow2.create(1024, 1024);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	light_list->add_light_witn_shadow_map(rec_shadow2);
	*/
	//外部模型导入及创建访问器
	hr_need = particle_fire->create(L"flare0.dds");
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load fire particle error", L"tip", MB_OK);
		return hr_need;
	}
	int index_model_rec;
	hr_need = geometry_lib->load_modelresource_from_file("castelmodel\\castel.obj", "castelmodel\\", false, true, false, 0, NULL, "castel_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "model_castel");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	int alpha_yuri[] = { 0,1,2,3 };
	hr_need = geometry_lib->load_modelresource_from_file("yurimodel_skin\\yuri.FBX", "yurimodel_skin\\", true, false, false, 4, alpha_yuri, "yuri_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "model_yuri");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	//内置模型访问器
	hr_need = geometry_lib->add_buildin_modelview_by_name("geometry_cube", "geometry_floor", pancy_geometry_cube);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"add buildin model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_buildin_modelview_by_name("geometry_ball", "geometry_sky", pancy_geometry_ball);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"add buildin model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_buildin_modelview_by_name("geometry_cube", "geometry_aotest", pancy_geometry_cube);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"add buildin model error", L"tip", MB_OK);
		return hr_need;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~纹理注册~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int texture_index;
	hr_need = geometry_lib->load_texture_from_file(L"floor.dds", true, "floor_diffuse", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"floor_n.dds", true, "floor_normal", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"Texture_cube.dds", true, "sky_cube", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"RoughBlades.dds", true, "grass_diffuse", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"RoughBlades_Normal.dds", true, "grass_normal", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"RoughBlades_Spec.dds", true, "grass_specular", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	return S_OK;
}
HRESULT scene_engine_test::display()
{
	XMFLOAT4X4 view_proj;
	XMStoreFloat4x4(&view_proj, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	/*
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(rsDesc));
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.DepthClipEnable = true;
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	rsDesc.FrontCounterClockwise = false;
	ID3D11RasterizerState *rsState(NULL);
	device_pancy->CreateRasterizerState(&rsDesc, &rsState);
	contex_pancy->RSSetState(rsState);
	*/
	//contex_pancy->RSSetState(renderstate_lib->get_CULL_none_rs());
	contex_pancy->RSSetState(NULL);
	show_ball();
	show_lightsource();
	show_floor();
	show_castel_deffered("LightTech");
	//show_castel("draw_withshadowssao", "draw_withshadowssaonormal");
	show_aotestproj();
	//show_yuri();
	show_yuri_animation_deffered();
	show_yuri_animation();
	//show_billboard();
	//清空深度模板缓冲，在AO绘制阶段记录下深度信息
	//show_fire_particle();
	return S_OK;
}
HRESULT scene_engine_test::display_enviroment()
{
	show_ball();
	show_lightsource();
	show_floor();
	//show_castel("draw_withtexture", "draw_withtexturenormal");
	show_castel_deffered("LightTech");
	return S_OK;
}
HRESULT scene_engine_test::display_nopost()
{
	//renderstate_lib->restore_rendertarget(ssao_part->get_depthstencilmap());
	renderstate_lib->restore_rendertarget();
	show_fire_particle();
	return S_OK;
}
void scene_engine_test::show_yuri_animation()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//几何体属性
	auto* model_yuri_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_yuri");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need, *teque_normal, *teque_hair;
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

	shader_test->get_technique(rec_point, num_member, &teque_need, "drawskin_withshadowssao");
	shader_test->get_technique(rec_point, num_member, &teque_normal, "drawskin_withshadowssaonormal");
	shader_test->get_technique(rec_point, num_member, &teque_hair, "drawskin_hair");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	world_matrix = model_yuri_pack->get_world_matrix();
	rec_world = XMLoadFloat4x4(&world_matrix);
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//获取渲染路径并渲染
	int yuri_render_order[11] = { 4,5,6,7,8,9,10,3,0,2,1 };
	XMFLOAT4X4 *rec_bonematrix = model_yuri_pack->get_bone_matrix();
	shader_test->set_bone_matrix(rec_bonematrix, model_yuri_pack->get_bone_num());
	//alpha混合设定
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//绘制头发
	material_list rec_need;
	model_yuri_pack->get_texture(&rec_need, yuri_render_order[7]);
	shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
	shader_test->set_normaltex(rec_need.texture_normal_resource);
	model_yuri_pack->draw_mesh_part(teque_hair, yuri_render_order[7]);
	contex_pancy->OMSetDepthStencilState(NULL, 0);
	contex_pancy->OMSetBlendState(0, blendFactor, 0xffffffff);
	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
void scene_engine_test::show_yuri_animation_deffered()
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的属性
	auto* model_yuri_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_yuri");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
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
	shader_test->get_technique(rec_point, num_member, &teque_need, "LightWithBone");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(0.4f, 0.4f, 0.4f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	world_matrix = model_yuri_pack->get_world_matrix();
	rec_world = XMLoadFloat4x4(&world_matrix);
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//设定ssao变换及贴图
	/*
	XMMATRIX T_need(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
		);
	XMFLOAT4X4 ssao_matrix;
	XMStoreFloat4x4(&ssao_matrix, worldViewProj*T_need);
	shader_test->set_trans_ssao(&ssao_matrix);
	shader_test->set_ssaotex(ssao_part->get_aomap());
	*/
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	//获取渲染路径并渲染
	//model_yuri->get_technique(teque_need);
	//model_yuri->draw_mesh();
	int yuri_render_order[11] = { 4,5,6,7,8,9,10,3,0,2,1 };
	XMFLOAT4X4 *rec_bonematrix = model_yuri_pack->get_bone_matrix();
	shader_test->set_bone_matrix(rec_bonematrix, model_yuri_pack->get_bone_num());
	for (int i = 0; i < 7; ++i)
	{
		material_list rec_need;
		model_yuri_pack->get_texture(&rec_need, yuri_render_order[i]);
		//model_yuri->get_texture(&rec_need, yuri_render_order[i]);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_yuri_pack->draw_mesh_part(teque_need, yuri_render_order[i]);
		//model_yuri->get_technique(teque_need);
		//model_yuri->draw_part(yuri_render_order[i]);
	}
	//alpha混合设定
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState *rec = renderstate_lib->get_blend_common();
	contex_pancy->OMSetBlendState(rec, blendFactor, 0xffffffff);
	for (int i = 8; i < model_yuri_pack->get_geometry_num(); ++i)
	{
		material_list rec_need;
		//model_yuri->get_texture(&rec_need, yuri_render_order[i]);
		model_yuri_pack->get_texture(&rec_need, yuri_render_order[i]);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_yuri_pack->draw_mesh_part(teque_need, yuri_render_order[i]);
		//model_yuri->get_technique(teque_need);
		//model_yuri->draw_part(yuri_render_order[i]);
	}
	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	//shader_test->set_ssaotex(NULL);
	shader_test->set_diffuse_light_tex(NULL);
	shader_test->set_specular_light_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void scene_engine_test::show_castel(LPCSTR techname, LPCSTR technamenormal)
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_castel");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need, *teque_normal;
	shader_test->get_technique(&teque_need, techname);
	shader_test->get_technique(&teque_normal, technamenormal);
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//设定世界变换
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//alpha混合设定
	for (int i = 0; i < model_castel_pack->get_geometry_num(); ++i)
	{
		//纹理设定
		material_list rec_need;
		model_castel_pack->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		if (rec_need.texture_normal_resource != NULL)
		{
			shader_test->set_normaltex(rec_need.texture_normal_resource);
			model_castel_pack->draw_mesh_part(teque_normal, i);
		}
		else
		{
			model_castel_pack->draw_mesh_part(teque_need, i);
		}
	}
}
void scene_engine_test::show_castel_deffered(LPCSTR techname)
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_castel");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, techname);
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	shader_test->set_material(test_Mt);


	//设定世界变换
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	shader_test->set_trans_world(&world_matrix);
	for (int i = 0; i < model_castel_pack->get_geometry_num(); ++i)
	{
		//纹理设定
		material_list rec_need;
		model_castel_pack->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_castel_pack->draw_mesh_part(teque_need, i);
	}

	shader_test->set_diffuse_light_tex(NULL);
	shader_test->set_specular_light_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}

	contex_pancy->RSSetState(NULL);
}
void scene_engine_test::show_ball()
{
	contex_pancy->RSSetState(renderstate_lib->get_CULL_front_rs());
	auto* shader_test = shader_lib->get_shader_reflect();
	auto* ball_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_sky");
	//auto* tex_skycube = geometry_lib->get_sky_cube_tex();
	auto* tex_skycube = geometry_lib->get_texture_byname("sky_cube")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_reflect");

	/*
	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(50.0f, 50.0f, 50.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	*/
	shader_test->set_trans_world(&ball_need->get_world_matrix());
	//设定立方贴图
	shader_test->set_tex_resource(tex_skycube);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&ball_need->get_world_matrix());
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	ball_need->draw_full_geometry(teque_need);
	contex_pancy->RSSetState(NULL);
}
void scene_engine_test::show_lightsource()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	auto* floor_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_floor");
	//auto* tex_floor = geometry_lib->get_basic_floor_tex();
	//auto* tex_normal = geometry_lib->get_floor_normal_tex();
	auto* tex_floor = geometry_lib->get_texture_byname("floor_diffuse")->data->get_data();
	auto* tex_normal = geometry_lib->get_texture_byname("floor_normal")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 2.5, 2.5);
	scal_world = XMMatrixScaling(0.1f, 0.1f, 0.1f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	floor_need->draw_full_geometry(teque_need);
}
void scene_engine_test::show_floor()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	auto* floor_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_floor");
	//auto* tex_floor = geometry_lib->get_basic_floor_tex();
	//auto* tex_normal = geometry_lib->get_floor_normal_tex();
	auto* tex_floor = geometry_lib->get_texture_byname("floor_diffuse")->data->get_data();
	auto* tex_normal = geometry_lib->get_texture_byname("floor_normal")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_withshadownormal");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);
	/*
	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0f, -1.2f, 0.0f);
	scal_world = XMMatrixScaling(35.0f, 0.55f, 35.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	*/
	shader_test->set_trans_world(&floor_need->get_world_matrix());
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&floor_need->get_world_matrix());

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	//floor_need->get_teque(teque_need);
	//floor_need->show_mesh();
	floor_need->draw_full_geometry(teque_need);
}
void scene_engine_test::show_aotestproj()
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	auto* floor_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_aotest");
	//auto* tex_floor = geometry_lib->get_basic_floor_tex();
	//auto* tex_normal = geometry_lib->get_floor_normal_tex();
	auto* tex_floor = geometry_lib->get_texture_byname("floor_diffuse")->data->get_data();
	auto* tex_normal = geometry_lib->get_texture_byname("floor_normal")->data->get_data();
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(0.6f, 0.6f, 0.6f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	//shader_test->set_normaltex(tex_normal);
	shader_test->set_trans_world(&floor_need->get_world_matrix());
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&floor_need->get_world_matrix());

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	floor_need->draw_full_geometry(teque_need);
}
void scene_engine_test::show_fire_particle()
{
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMFLOAT3 st_pos = XMFLOAT3(7.25f, 4.6f, 13.2f);
	XMFLOAT3 st_dir = XMFLOAT3(0.0f, 1.0f, 0.0f);
	particle_fire->set_particle_direct(&st_pos, &st_dir);
	particle_fire->draw_particle();
	contex_pancy->RSSetState(0);
	contex_pancy->OMSetDepthStencilState(0, 0);
	contex_pancy->OMSetBlendState(0, blendFactor, 0xffffffff);
}
void scene_engine_test::show_billboard()
{
	contex_pancy->RSSetState(renderstate_lib->get_CULL_none_rs());
	auto* shader_test = shader_lib->get_shader_grass_billboard();
	auto* floor_need = geometry_lib->get_grass_common();
	shader_test->set_texture_diffuse(geometry_lib->get_texture_byname("grass_diffuse")->data->get_data());
	shader_test->set_texture_normal(geometry_lib->get_texture_byname("grass_normal")->data->get_data());
	shader_test->set_texture_specular(geometry_lib->get_texture_byname("grass_specular")->data->get_data());
	XMFLOAT4X4 rec_mat;
	XMStoreFloat4x4(&rec_mat, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	shader_test->set_trans_all(&rec_mat);
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_with_tex");
	floor_need->get_teque(teque_need);
	floor_need->show_mesh();
	contex_pancy->RSSetState(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
HRESULT scene_engine_test::update(float delta_time)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新场景摄像机~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HRESULT hr = camera_move();
	if (hr != S_OK)
	{
		MessageBox(0, L"camera system has an error", L"tip", MB_OK);
		return hr;
	}
	XMFLOAT3 eyePos_rec;
	scene_camera->get_view_position(&eyePos_rec);
	auto* shader_ref = shader_lib->get_shader_reflect();
	shader_ref->set_view_pos(eyePos_rec);
	auto* shader_test = shader_lib->get_shader_prelight();
	shader_test->set_view_pos(eyePos_rec);
	auto* shader_grass = shader_lib->get_shader_grass_billboard();
	shader_grass->set_view_pos(eyePos_rec);
	auto* shader_deff = shader_lib->get_shader_light_deffered_draw();
	shader_deff->set_view_pos(eyePos_rec);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新几何体世界变换~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//auto* model_list = geometry_lib->get_model_list();
	//更新yuri世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.5);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rotation_world = XMMatrixRotationY(3.141592653f);
	rec_world = scal_world * rotation_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_assimp_MRV_byname("model_yuri", world_matrix, delta_time);
	geometry_lib->update_assimp_MRV_byname("model_yuri_trans", world_matrix, delta_time);
	//更新castel世界变换
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_assimp_MRV_byname("model_castel", world_matrix, delta_time);
	//更新光源世界变换
	trans_world = XMMatrixTranslation(0.0, 2.5, 2.5);
	scal_world = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	//更新天空球世界变换
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(50.0f, 50.0f, 50.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_buildin_GRV_byname("geometry_sky", world_matrix, delta_time);


	//设定世界变换
	trans_world = XMMatrixTranslation(0.0f, -1.2f, 0.0f);
	scal_world = XMMatrixScaling(35.0f, 0.55f, 35.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_buildin_GRV_byname("geometry_floor", world_matrix, delta_time);
	//设定ao遮蔽板的世界变换
	trans_world = XMMatrixTranslation(0.0f, 0.0f, -1.1f);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	geometry_lib->update_buildin_GRV_byname("geometry_aotest", world_matrix, delta_time);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~设置shadowmap光源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	light_list->update_and_setlight();
	/*
	int count = 0;
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
		rec_shadow_light._Ptr->set_frontlight(count);
		rec_shadow_light._Ptr->set_defferedlight(count);
		count += 1;
	}
	//设置无影光源
	for (auto rec_non_light = nonshadow_light_list.begin(); rec_non_light != nonshadow_light_list.end(); ++rec_non_light)
	{
		rec_non_light._Ptr->set_frontlight(count);
		rec_non_light._Ptr->set_defferedlight(count);
		count += 1;
	}
	//设置shadowvolume光源
	for (auto rec_shadow_volume = shadowvalume_light_list.begin(); rec_shadow_volume != shadowvalume_light_list.end(); ++rec_shadow_volume)
	{
		//rec_shadow_volume._Ptr->set_frontlight(count);
		//rec_shadow_volume._Ptr->set_defferedlight(count);
		//count++;
		//rec_shadow_volume._Ptr->update_view_proj_matrix(view_proj);
	}*/
	XMFLOAT4X4 view_proj;
	XMStoreFloat4x4(&view_proj, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	time_game += delta_time*0.3;
	particle_fire->update(delta_time*0.3, time_game, &view_proj, &eyePos_rec);
	return S_OK;
}
HRESULT scene_engine_test::release()
{
	particle_fire->release();
	return S_OK;
}


scene_engine_snake::scene_engine_snake(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height, float near_plane, float far_plane, float angle_view) : scene_root(device_need, contex_need, render_state, input_need, camera_need, lib_need, geometry_need, light_need, width, height, near_plane, far_plane, angle_view)
{
	particle_fire = new particle_system<fire_point>(device_need, contex_need, 3000, lib_need, PARTICLE_TYPE_FIRE);
	test_snake = new snake_draw(device_need, contex_need, user_input, 1000, 3);
}
HRESULT scene_engine_snake::scene_create()
{
	HRESULT hr_need;
	hr_need = test_snake->create();
	if (FAILED(hr_need))
	{
		return E_FAIL;
	}
	hr_need = particle_fire->create(L"flare0.dds");
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load fire particle error", L"tip", MB_OK);
		return hr_need;
	}
	int index_model_rec;
	hr_need = geometry_lib->load_modelresource_from_file("snakemodel\\snake.FBX", "snakemodel\\", true, false, false, 0, NULL, "snake_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "model_snake");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	int texture_index;
	hr_need = geometry_lib->load_texture_from_file(L"floor.dds", true, "floor_diffuse", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"floor_n.dds", true, "floor_normal", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"Texture_cube.dds", true, "sky_cube", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	//内置模型访问器
	hr_need = geometry_lib->add_buildin_modelview_by_name("geometry_cube", "geometry_floor", pancy_geometry_cube);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"add buildin model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_buildin_modelview_by_name("geometry_ball", "geometry_sky", pancy_geometry_ball);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"add buildin model error", L"tip", MB_OK);
		return hr_need;
	}
	return S_OK;
}
void scene_engine_snake::show_ball()
{
	contex_pancy->RSSetState(renderstate_lib->get_CULL_front_rs());
	auto* shader_test = shader_lib->get_shader_reflect();
	auto* ball_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_sky");
	//auto* tex_skycube = geometry_lib->get_sky_cube_tex();
	auto* tex_skycube = geometry_lib->get_texture_byname("sky_cube")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_reflect");

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(50.0f, 50.0f, 50.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定立方贴图
	shader_test->set_tex_resource(tex_skycube);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	ball_need->draw_full_geometry(teque_need);
	contex_pancy->RSSetState(NULL);
}
void scene_engine_snake::show_floor()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	auto* floor_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_floor");
	//auto* tex_floor = geometry_lib->get_basic_floor_tex();
	//auto* tex_normal = geometry_lib->get_floor_normal_tex();
	auto* tex_floor = geometry_lib->get_texture_byname("floor_diffuse")->data->get_data();
	auto* tex_normal = geometry_lib->get_texture_byname("floor_normal")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_withshadownormal");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);

	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0f, -1.2f, 0.0f);
	scal_world = XMMatrixScaling(55.0f, 0.55f, 55.0f);

	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	//floor_need->get_teque(teque_need);
	//floor_need->show_mesh();
	floor_need->draw_full_geometry(teque_need);
}
void scene_engine_snake::show_snake_animation_deffered()
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的属性
	auto* model_snake_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_snake");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
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
	shader_test->get_technique(rec_point, num_member, &teque_need, "LightWithBone");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(0.4f, 0.4f, 0.4f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	world_matrix = model_snake_pack->get_world_matrix();
	rec_world = XMLoadFloat4x4(&world_matrix);
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	//获取渲染路径并渲染
	XMFLOAT4X4 *rec_bonematrix = model_snake_pack->get_bone_matrix();
	shader_test->set_bone_matrix(rec_bonematrix, model_snake_pack->get_bone_num());
	for (int i = 0; i < model_snake_pack->get_geometry_num(); ++i)
	{
		material_list rec_need;
		model_snake_pack->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_snake_pack->draw_mesh_part(teque_need, i);
	}
	shader_test->set_diffuse_light_tex(NULL);
	shader_test->set_specular_light_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
HRESULT scene_engine_snake::display()
{
	XMFLOAT4X4 view_proj;
	XMStoreFloat4x4(&view_proj, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	test_snake->draw(view_proj);
	show_ball();
	show_floor();
	show_snake_animation_deffered();
	return S_OK;
}
HRESULT scene_engine_snake::display_enviroment()
{
	return S_OK;
}
HRESULT scene_engine_snake::display_nopost()
{
	renderstate_lib->restore_rendertarget();
	return S_OK;
}
HRESULT scene_engine_snake::release()
{
	test_snake->release();
	particle_fire->release();
	return S_OK;
}
HRESULT scene_engine_snake::update(float delta_time)
{
	test_snake->update(delta_time);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新场景摄像机~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HRESULT hr = camera_move();
	if (hr != S_OK)
	{
		MessageBox(0, L"camera system has an error", L"tip", MB_OK);
		return hr;
	}
	XMFLOAT3 eyePos_rec;
	scene_camera->get_view_position(&eyePos_rec);
	auto* shader_ref = shader_lib->get_shader_reflect();
	shader_ref->set_view_pos(eyePos_rec);
	auto* shader_test = shader_lib->get_shader_prelight();
	shader_test->set_view_pos(eyePos_rec);
	auto* shader_grass = shader_lib->get_shader_grass_billboard();
	shader_grass->set_view_pos(eyePos_rec);
	auto* shader_deff = shader_lib->get_shader_light_deffered_draw();
	shader_deff->set_view_pos(eyePos_rec);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新模型~~~~~~~~~~~~~~~~~~~~~~~~~
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(test_snake->get_head_location().x - 0.05f, test_snake->get_head_location().y - 1.9f, test_snake->get_head_location().z + 5.0f);
	scal_world = XMMatrixScaling(1.2f, 1.2f, 1.2f);
	XMMATRIX trans_world2 = XMMatrixTranslation(0.0f, -0.3f, 0.6f);
	XMFLOAT4X4 rec_rot;
	test_snake->get_head_rotation(&rec_rot);
	rotation_world = XMLoadFloat4x4(&rec_rot);
	rec_world = scal_world * trans_world2 * rotation_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_assimp_MRV_byname("model_snake", world_matrix, delta_time);


	//设定世界变换
	trans_world = XMMatrixTranslation(0.0f, -1.2f, 0.0f);
	scal_world = XMMatrixScaling(55.0f, 0.55f, 55.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_buildin_GRV_byname("geometry_floor", world_matrix, delta_time);

	//设定世界变换
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(50.0f, 50.0f, 50.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	geometry_lib->update_buildin_GRV_byname("geometry_sky", world_matrix, delta_time);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~设置shadowmap光源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	light_list->update_and_setlight();
	XMFLOAT4X4 view_proj;
	XMStoreFloat4x4(&view_proj, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	time_game += delta_time*0.3;
	particle_fire->update(delta_time*0.3, time_game, &view_proj, &eyePos_rec);
	return S_OK;
}




player_basic::player_basic(string model_resource_name, string player_name, geometry_control *geometry_need, pancy_physx  *physic_need, shader_control *shader_need, model_data_type model_type_need, XMFLOAT3 st_position, bool bound_box_need)
{
	model_resource = model_resource_name;
	model_view_data_name = player_name;
	geometry_pancy = geometry_need;
	physic_pancy = physic_need;
	shader_lib = shader_need;
	model_type = model_type_need;
	rotation_angle = 0.0f;
	player = NULL;
	mat_force = NULL;
	x_speed = 0.0f;
	z_speed = 0.0f;
	model_scaling = 1.0f;
	rec_offset_x = 0;
	rec_offset_z = 0;
	if_show_bound_box = bound_box_need;
	init_position = st_position;
	init_position.y = st_position.y + 50.0f;
	time_animation = 0.0f;
}
HRESULT player_basic::create(float raidus, float height, float foward, float slide, player_shape_physic player_shape_choose)
{
	HRESULT hr;
	model_head_radius = raidus;
	model_height = height;
	model_length_forward = foward;
	model_length_slide = slide;
	if (if_show_bound_box)
	{
		hr = init_geometry_bounding_box();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	if (model_type == model_data_type::pancy_model_buildin)
	{
		hr = init_geometry_buildin();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	else if (model_type == model_data_type::pancy_model_assimp)
	{
		hr = init_geometry_assimp();
		if (FAILED(hr))
		{
			return hr;
		}
	}
	else
	{
		return E_FAIL;
	}

	hr = init_physics(player_shape_choose);
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}
HRESULT player_basic::init_geometry_buildin()
{
	HRESULT hr = geometry_pancy->add_buildin_modelview_by_name(model_resource, model_view_data_name, Geometry_type::pancy_geometry_cube);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT player_basic::init_geometry_assimp()
{
	HRESULT hr = geometry_pancy->add_assimp_modelview_by_name(model_resource, model_view_data_name);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT player_basic::init_geometry_bounding_box()
{
	HRESULT hr = geometry_pancy->add_buildin_modelview_by_name("geometry_cube", model_view_data_name + "bounding_box_player", Geometry_type::pancy_geometry_cube);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void player_basic::display(XMFLOAT4X4 view_proj_matrix)
{
	if (if_show_bound_box)
	{
		show_bounding_box(view_proj_matrix);
	}
	if (model_type == model_data_type::pancy_model_buildin)
	{
		show_build_in_model(view_proj_matrix);

	}
	else
	{
		show_assimp_skinmesh_model(view_proj_matrix);
	}

}

HRESULT player_basic::init_physics(player_shape_physic player_shape_choose)
{
	player_shape_use = player_shape_choose;
	mat_force = physic_pancy->create_material(0.1, 0.1, 0.0);
	if (player_shape_choose == player_shape_physic::player_shape_box)
	{
		physx::PxBoxControllerDesc test_box_desc;
		test_box_desc.position = physx::PxExtendedVec3(init_position.x, init_position.y, init_position.z);
		test_box_desc.contactOffset = 0.05f;
		test_box_desc.stepOffset = 1.51;
		//test_capsule_desc.radius = 3.5;
		//test_capsule_desc.height = 7;
		//test_box_desc.radius = model_head_radius;
		//test_box_desc.height = model_height;
		test_box_desc.halfHeight = model_height / 2.0f;
		test_box_desc.halfForwardExtent = model_length_forward;
		test_box_desc.halfSideExtent = model_length_slide / 2.0f;

		test_box_desc.upDirection = physx::PxVec3(0, 1, 0);
		test_box_desc.material = mat_force;
		test_box_desc.maxJumpHeight = 5.0f;
		test_box_desc.slopeLimit = 0.2f;
		bool rec_check = test_box_desc.isValid();
		HRESULT hr = physic_pancy->create_charactor(test_box_desc, &player);
		if (FAILED(hr))
		{
			return hr;
		}
		return S_OK;
	}
	else
	{
		physx::PxCapsuleControllerDesc test_capsule_desc;
		test_capsule_desc.position = physx::PxExtendedVec3(init_position.x, init_position.y, init_position.z);
		test_capsule_desc.contactOffset = 0.05f;
		test_capsule_desc.stepOffset = 0.51;
		test_capsule_desc.slopeLimit = 0.25f;
		//test_capsule_desc.radius = 3.5;
		//test_capsule_desc.height = 7;
		test_capsule_desc.radius = model_head_radius;
		test_capsule_desc.height = model_height;
		test_capsule_desc.upDirection = physx::PxVec3(0, 1, 0);
		test_capsule_desc.material = mat_force;
		test_capsule_desc.maxJumpHeight = 5.0f;
		bool rec_check = test_capsule_desc.isValid();
		HRESULT hr = physic_pancy->create_charactor(test_capsule_desc, &player);
		if (FAILED(hr))
		{
			return hr;
		}
		return S_OK;
	}
}
void player_basic::update(float delta_time)
{
	physx::PxVec3 data_need = player->getUpDirection();
	if (player != NULL)
	{
		time_animation += delta_time;
		physx::PxVec3 disp = physx::PxVec3(x_speed, -9.8f, z_speed);
		physx::PxF32 minDist = 0.01;

		//physx::PxF32 elapsedTime = static_cast<physx::PxF32>(0.005f);
		physx::PxF32 elapsedTime = static_cast<physx::PxF32>(0.033);
		physx::PxControllerFilters filters;
		while (time_animation > 0.033f)
		{
			player->move(disp, minDist, elapsedTime, filters);
			time_animation -= 0.033f;
		}
		physx::PxExtendedVec3 rec = player->getPosition();
		rec.x -= rec_offset_x;
		rec.z -= rec_offset_z;


		XMMATRIX rec_world_trans = XMMatrixTranslation(rec.x, rec.y, rec.z);
		XMMATRIX rec_world_scal;
		if (player_shape_use == player_shape_physic::player_shape_box)
		{
			rec_world_scal = XMMatrixScaling(model_length_slide, model_height, model_length_forward);
		}
		else
		{
			rec_world_scal = XMMatrixScaling(2.0f * model_head_radius, model_height + 2.0f * model_head_radius, 2.0f * model_head_radius);
		}
		XMFLOAT4X4 world_mat_final;
		//更新包围盒信息
		if (if_show_bound_box)
		{
			auto* floor_need = geometry_pancy->get_buildin_GeometryResourceView_by_name(model_view_data_name + "bounding_box_player");
			XMStoreFloat4x4(&world_mat_final, rec_world_scal * rec_world_trans);
			floor_need->update(world_mat_final, delta_time);
		}
		//更新几何体信息
		if (model_type == model_data_type::pancy_model_buildin)
		{
			XMMATRIX model_world_trans = XMMatrixTranslation(rec.x, rec.y, rec.z);
			XMMATRIX model_world_scal = XMMatrixScaling(1.0f, 1.0f, 1.0f);
			XMStoreFloat4x4(&world_mat_final, model_world_scal * model_world_trans);
			auto* floor_need = geometry_pancy->get_buildin_GeometryResourceView_by_name(model_view_data_name);
			floor_need->update(world_mat_final, delta_time);
		}
		else
		{
			XMMATRIX model_world_trans = XMMatrixTranslation(rec.x, rec.y - 7.7, rec.z);
			XMMATRIX model_world_scal = XMMatrixScaling(model_scaling, model_scaling, model_scaling);
			XMMATRIX model_world_rotation = XMMatrixRotationY(rotation_angle);
			XMStoreFloat4x4(&world_mat_final, model_world_scal * model_world_rotation * model_world_trans);
			auto* floor_need = geometry_pancy->get_assimp_ModelResourceView_by_name(model_view_data_name);
			floor_need->update(world_mat_final, delta_time);
		}
	}
}
void player_basic::get_now_position(float &x, float &y, float &z)
{
	physx::PxExtendedVec3 rec = player->getPosition();
	rec.x -= rec_offset_x;
	rec.z -= rec_offset_z;

	x = rec.x;
	y = rec.y;
	z = rec.z;
}
void player_basic::set_speed(float x, float z)
{
	x_speed = x;
	z_speed = z;
}
void player_basic::release()
{
	player->release();
}
void player_basic::show_bounding_box(XMFLOAT4X4 view_proj_matrix)
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	auto* floor_need = geometry_pancy->get_buildin_GeometryResourceView_by_name(model_view_data_name + "bounding_box_player");
	auto* tex_floor = geometry_pancy->get_texture_byname("floor_diffuse")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	//设定世界变换
	shader_test->set_trans_world(&floor_need->get_world_matrix());
	//设定总变换
	XMMATRIX view_proj = XMLoadFloat4x4(&view_proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&floor_need->get_world_matrix());
	XMMATRIX worldViewProj = world_matrix_rec*view_proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	floor_need->draw_full_geometry(teque_need);
}
void player_basic::show_build_in_model(XMFLOAT4X4 view_proj_matrix)
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	auto* floor_need = geometry_pancy->get_buildin_GeometryResourceView_by_name(model_view_data_name);
	auto* tex_floor = geometry_pancy->get_texture_byname("floor_diffuse")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	//设定世界变换
	shader_test->set_trans_world(&floor_need->get_world_matrix());
	//设定总变换
	XMMATRIX view_proj = XMLoadFloat4x4(&view_proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&floor_need->get_world_matrix());
	XMMATRIX worldViewProj = world_matrix_rec*view_proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	floor_need->draw_full_geometry(teque_need);
}
void player_basic::show_assimp_skinmesh_model(XMFLOAT4X4 view_proj_matrix)
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的属性
	auto* model_yuri_pack = geometry_pancy->get_assimp_ModelResourceView_by_name(model_view_data_name);
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
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
	shader_test->get_technique(rec_point, num_member, &teque_need, "LightWithBone");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	world_matrix = model_yuri_pack->get_world_matrix();
	rec_world = XMLoadFloat4x4(&world_matrix);
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX viewproj = XMLoadFloat4x4(&view_proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*viewproj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//获取渲染路径并渲染
	int yuri_render_order[11] = { 4,5,6,7,8,9,10,3,0,2,1 };
	XMFLOAT4X4 *rec_bonematrix = model_yuri_pack->get_bone_matrix();
	shader_test->set_bone_matrix(rec_bonematrix, model_yuri_pack->get_bone_num());


	for (int i = 0; i < model_yuri_pack->get_geometry_num(); ++i)
	{
		material_list rec_need;
		model_yuri_pack->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_yuri_pack->draw_mesh_part(teque_need, i);
	}

	/*
	for (int i = 0; i < 7; ++i)
	{
		material_list rec_need;
		model_yuri_pack->get_texture(&rec_need, yuri_render_order[i]);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_yuri_pack->draw_mesh_part(teque_need, yuri_render_order[i]);
	}
	//alpha混合设定

	//float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//ID3D11BlendState *rec = renderstate_lib->get_blend_common();
	//contex_pancy->OMSetBlendState(rec, blendFactor, 0xffffffff);
	for (int i = 8; i < model_yuri_pack->get_geometry_num(); ++i)
	{
		material_list rec_need;
		model_yuri_pack->get_texture(&rec_need, yuri_render_order[i]);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_yuri_pack->draw_mesh_part(teque_need, yuri_render_order[i]);
	}
	show_transparent_part(view_proj_matrix, 3);
*/
//contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
//shader_test->set_diffuse_light_tex(NULL);
//shader_test->set_specular_light_tex(NULL);
//D3DX11_TECHNIQUE_DESC techDesc;
//teque_need->GetDesc(&techDesc);
//for (UINT p = 0; p < techDesc.Passes; ++p)
//{
//	teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
//}
}
void player_basic::show_transparent_part(XMFLOAT4X4 view_proj_matrix, int part)
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//几何体属性
	auto* model_yuri_pack = geometry_pancy->get_assimp_ModelResourceView_by_name(model_view_data_name);
	//选定绘制路径
	ID3DX11EffectTechnique *teque_hair;
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

	shader_test->get_technique(rec_point, num_member, &teque_hair, "drawskin_hair");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	world_matrix = model_yuri_pack->get_world_matrix();
	rec_world = XMLoadFloat4x4(&world_matrix);
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX viewproj = XMLoadFloat4x4(&view_proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*viewproj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//获取渲染路径并渲染
	XMFLOAT4X4 *rec_bonematrix = model_yuri_pack->get_bone_matrix();
	shader_test->set_bone_matrix(rec_bonematrix, model_yuri_pack->get_bone_num());
	//alpha混合设定
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//绘制头发
	material_list rec_need;
	model_yuri_pack->get_texture(&rec_need, part);
	shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
	shader_test->set_normaltex(rec_need.texture_normal_resource);
	model_yuri_pack->draw_mesh_part(teque_hair, part);

}
void player_basic::get_look_position(float &x, float &y, float &z)
{
	XMVECTOR look_pos = XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, -1.0f));
	XMMATRIX rotation_mat = XMMatrixRotationY(rotation_angle);
	XMVECTOR look_now = XMVector3TransformCoord(look_pos, rotation_mat);
	XMFLOAT3 rec_ans;
	XMStoreFloat3(&rec_ans, look_now);
	x = rec_ans.x;
	y = rec_ans.y;
	z = rec_ans.z;
}
void player_basic::reset_rotation_angle(float delta_angle)
{
	rotation_angle = delta_angle;
}
void player_basic::change_rotation_angle(float delta_angle)
{
	rotation_angle += delta_angle;
}


AI_animal::AI_animal(string model_resource_name, string player_name, geometry_control *geometry_need, pancy_physx  *physic_need, shader_control *shader_need, model_data_type model_type_need, animal_attribute animal_data, XMFLOAT3 st_position, bool bound_box_need) :player_basic(model_resource_name, player_name, geometry_need, physic_need, shader_need, model_type_need, st_position, bound_box_need)
{
	animal_HP = 1000;
	animal_attack = 10;
	if_dead = false;
	rebirth_time = 300;
	time_dead = 0.0f;
	resource_name = model_resource_name;
	attack_time = 0.0f;
	//	real_speed_direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
		//real_speed_data.push(XMFLOAT3(0.0f, 1.0f, 0.0f));
	music_ID = -1;
	channel_ID = -1;
	speed_animal_dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
	animal_message = animal_data;
	animal_animation_time = 0.0f;
	animal_now_behavior = animal_behavior::animal_walk_around;
	if_stop = true;
	all_time = 0.0f;
	speed_data_size = 60;
}
XMFLOAT3 AI_animal::normalize_float(XMFLOAT3 input)
{
	XMFLOAT3 output;
	float normalize_need = sqrt(input.x * input.x + input.y * input.y + input.z * input.z);
	if (normalize_need < 0.01f)
	{
		return input;
	}
	output.x = input.x / normalize_need;
	output.y = input.y / normalize_need;
	output.z = input.z / normalize_need;
	return output;
}
void AI_animal::set_music_message(int music_ID_need, int channel_ID_need)
{
	music_ID = music_ID_need;
	channel_ID = channel_ID_need;
}
void AI_animal::set_music_voice_message(int music_ID_need, int channel_ID_need)
{
	music_voice_ID = music_ID_need;
	channel_voice_ID = channel_ID_need;
}
void AI_animal::update(float delta_time)
{
	if (if_dead)
	{
		time_dead += delta_time;
		if (time_dead >= rebirth_time)
		{
			player->setPosition(physx::PxExtendedVec3(animal_message.position_center.x, animal_message.position_center.y, animal_message.position_center.z));
			if_dead = false;
			time_dead = 0.0f;
			animal_HP = 100;
		}
	}
	else
	{
		if (animal_now_behavior == animal_behavior::animal_run_target && attack_time < 12.0f)
		{
			attack_time += delta_time * 5;
		}
		else
		{
			attack_time = 0.0f;
		}
		all_time += delta_time;
		update_target();
		check_if_stop();
		physx::PxVec3 data_need = player->getUpDirection();
		if (player != NULL)
		{
			time_animation += delta_time;
			physx::PxVec3 disp = physx::PxVec3(x_speed, -9.8f, z_speed);
			physx::PxF32 minDist = 0.01;
			//physx::PxF32 elapsedTime = static_cast<physx::PxF32>(0.005f);
			physx::PxF32 elapsedTime = static_cast<physx::PxF32>(0.033);
			physx::PxControllerFilters filters;
			while (time_animation > 0.033f)
			{
				//get_now_position(last_position.x, last_position.y, last_position.z);
				player->move(disp, minDist, elapsedTime, filters);
				/*
				XMFLOAT3 walk_dir_need;
				get_walk_dir(walk_dir_need);
				XMFLOAT3 delta_distance;
				real_speed_data.push(walk_dir_need);
				real_speed_direction.x += walk_dir_need.x / 30.0f;
				real_speed_direction.y += walk_dir_need.y / 30.0f;
				real_speed_direction.z += walk_dir_need.z / 30.0f;
				if (real_speed_data.size() > 30)
				{
					real_speed_direction.x -= real_speed_data.front().x / 30.0f;
					real_speed_direction.y -= real_speed_data.front().y / 30.0f;
					real_speed_direction.z -= real_speed_data.front().z / 30.0f;
					real_speed_data.pop();
				}
				*/
				time_animation -= 0.033f;
			}
			physx::PxExtendedVec3 rec = player->getPosition();
			rec.x -= rec_offset_x;
			rec.z -= rec_offset_z;


			XMMATRIX rec_world_trans = XMMatrixTranslation(rec.x, rec.y, rec.z);
			XMMATRIX rec_world_scal;
			if (player_shape_use == player_shape_physic::player_shape_box)
			{
				rec_world_scal = XMMatrixScaling(model_length_slide, model_height, model_length_forward);
			}
			else
			{
				rec_world_scal = XMMatrixScaling(2.0f * model_head_radius, model_height + 2.0f * model_head_radius, 2.0f * model_head_radius);
			}
			XMFLOAT4X4 world_mat_final;
			//更新包围盒信息
			if (if_show_bound_box)
			{
				auto* floor_need = geometry_pancy->get_buildin_GeometryResourceView_by_name(model_view_data_name + "bounding_box_player");
				XMStoreFloat4x4(&world_mat_final, rec_world_scal * rec_world_trans);
				floor_need->update(world_mat_final, delta_time);
			}
			//更新几何体信息
			if (model_type == model_data_type::pancy_model_buildin)
			{
				XMMATRIX model_world_trans = XMMatrixTranslation(rec.x, rec.y, rec.z);
				XMMATRIX model_world_scal = XMMatrixScaling(1.0f, 1.0f, 1.0f);
				XMStoreFloat4x4(&world_mat_final, model_world_scal * model_world_trans);
				auto* floor_need = geometry_pancy->get_buildin_GeometryResourceView_by_name(model_view_data_name);
				floor_need->update(world_mat_final, delta_time);
			}
			else
			{
				XMMATRIX model_world_trans = XMMatrixTranslation(rec.x, rec.y - 7.7, rec.z);
				XMMATRIX model_world_scal = XMMatrixScaling(model_scaling, model_scaling, model_scaling);
				XMMATRIX model_world_rotation = XMMatrixRotationY(rotation_angle);

				//获取物体的移动速度
				physx::PxRigidDynamic *test_speed = player->getActor();
				physx::PxVec3 speed_vector = test_speed->getLinearVelocity();
				XMFLOAT3 vector_speed_rec;
				vector_speed_rec.x = speed_vector.x;
				vector_speed_rec.y = speed_vector.y;
				vector_speed_rec.z = speed_vector.z;
				//检验移动速度的方向
				XMFLOAT3 look_dir;
				get_look_position(look_dir.x, look_dir.y, look_dir.z);
				HRESULT hr = check_walk_dir(vector_speed_rec, look_dir);
				if (!FAILED(hr))
				{
					//变更速度
					speed_animal_dir.x += vector_speed_rec.x / static_cast<float>(speed_data_size);
					speed_animal_dir.y += vector_speed_rec.y / static_cast<float>(speed_data_size);
					speed_animal_dir.z += vector_speed_rec.z / static_cast<float>(speed_data_size);
					real_speed_data.push(vector_speed_rec);
					if (real_speed_data.size() > speed_data_size)
					{
						speed_animal_dir.x -= real_speed_data.front().x / static_cast<float>(speed_data_size);
						speed_animal_dir.y -= real_speed_data.front().y / static_cast<float>(speed_data_size);
						speed_animal_dir.z -= real_speed_data.front().z / static_cast<float>(speed_data_size);
						real_speed_data.pop();
					}
				}
				//计算当前的动物头部朝向
				XMMATRIX mat_rotation_angle;
				XMFLOAT3 speed_direction_need = normalize_float(speed_animal_dir);
				hr = check_walk_dir(speed_direction_need, look_dir);
				/*
				if (FAILED(hr))
				{
					mat_rotation_angle = XMMatrixIdentity();

					XMVECTOR look_vec = XMLoadFloat3(&look_dir);
					XMVECTOR target_vec = XMLoadFloat3(&speed_animal_dir);
					//XMVECTOR up_vec = XMLoadFloat3(&XMFLOAT3(0,1,0));
					XMVECTOR right_need = XMVector3Cross(look_vec, target_vec);
					XMFLOAT3 angle_need;
					XMStoreFloat3(&angle_need, XMVector2AngleBetweenVectors(look_vec, target_vec));
					mat_rotation_angle = XMMatrixRotationAxis(right_need, angle_need.x);

				}
				else
				{
					XMVECTOR look_vec = XMLoadFloat3(&look_dir);
					XMVECTOR target_vec = XMLoadFloat3(&speed_animal_dir);
					//XMVECTOR up_vec = XMLoadFloat3(&XMFLOAT3(0,1,0));
					XMVECTOR right_need = XMVector3Cross(look_vec, target_vec);
					XMFLOAT3 angle_need;
					XMStoreFloat3(&angle_need, XMVector2AngleBetweenVectors(look_vec, target_vec));
					mat_rotation_angle = XMMatrixRotationAxis(right_need, angle_need.x);
				}
				*/
				mat_rotation_angle = XMMatrixIdentity();
				XMStoreFloat4x4(&world_mat_final, model_world_scal * model_world_rotation * mat_rotation_angle * model_world_trans);
				auto* floor_need = geometry_pancy->get_assimp_ModelResourceView_by_name(model_view_data_name);
				floor_need->update(world_mat_final, delta_time);
			}
		}
	}
}
bool AI_animal::check_if_hit_player()
{
	if (attack_time > 11.9f)
	{
		attack_time = 0.0f;
		return true;
	}
	return false;
}
void AI_animal::hit_animal(int hp_minus)
{
	animal_now_behavior = animal_behavior::animal_run_target;
	animal_HP -= hp_minus;
	if (animal_HP <= 0)
	{
		player->setPosition(physx::PxExtendedVec3(2000.0f, 2000.0f, 2000.0f));
		animal_now_behavior = animal_behavior::animal_walk_around;
		if_dead = true;
	}
}
animal_type AI_animal::get_animal_type()
{
	if (resource_name == "bull_model_resource")
	{
		return animal_type::animal_bull;
	}
	else if (resource_name == "panther_model_resource")
	{
		return animal_type::animal_panther;
	}
	else if (resource_name == "rhino_model_resource")
	{
		return animal_type::animal_rhino;
	}
	else if (resource_name == "wolf_model_resource")
	{
		return animal_type::animal_wolf;
	}
	return animal_type::animal_bull;
}
void AI_animal::update_target()
{
	if (animal_now_behavior == animal_behavior::animal_walk_around || animal_now_behavior == animal_behavior::animal_run_around)
	{
		if (if_stop)
		{
			animal_message.position_center;
			//随机向量
			float rec_x = static_cast<float>(rand() % 100000) / static_cast<float>(100000);
			float rec_z = static_cast<float>(rand() % 100000) / static_cast<float>(100000);
			float square_plus = sqrt(rec_x * rec_x + rec_z * rec_z);
			rec_x /= square_plus;
			rec_z /= square_plus;
			//随机长度
			float rec_length = animal_message.view_range / 2.0f + static_cast<float>(rand() % 100000) / static_cast<float>(100000) * animal_message.view_range / 2.0f;

			//随机位置
			target.x = animal_message.position_center.x + rec_x * rec_length;
			target.y = 0;
			target.z = animal_message.position_center.z + rec_z * rec_length;
			if_stop = false;
			//重设速度
			XMFLOAT3 now_position_need;
			get_now_position(now_position_need.x, now_position_need.y, now_position_need.z);
			//change_rotation_angle();
			float delta_x = target.x - now_position_need.x;
			float delta_z = target.z - now_position_need.z;
			if (abs(delta_x) < 3.0001f && abs(delta_z) < 3.0001f)
			{
				if_stop = true;
				set_speed(0.0f, 0.0f);
				return;
			}
			float square_plus_speed = sqrt(delta_x * delta_x + delta_z * delta_z);
			delta_x /= square_plus_speed;
			delta_z /= square_plus_speed;
			//计算旋转角度
			float dot_angle = delta_x  * 0.0f + delta_z*-1.0f;
			float final_angle;
			final_angle = acos(dot_angle);
			//判断补角
			float cross = delta_x  * -1.0f - 0.0f * delta_z;
			if (cross < 0.0f)
			{
				final_angle = XM_2PI - final_angle;
			}
			reset_rotation_angle(final_angle);
			if (animal_now_behavior == animal_behavior::animal_walk_around)
			{
				set_speed(delta_x * animal_message.velocity_walk, delta_z * animal_message.velocity_walk);
			}
			else if (animal_now_behavior == animal_behavior::animal_run_around || animal_now_behavior == animal_behavior::animal_run_target)
			{
				set_speed(delta_x * animal_message.velocity_run, delta_z * animal_message.velocity_run);
			}

		}
	}
}
void AI_animal::reset_target(XMFLOAT3 target_pos)
{
	if (animal_now_behavior == animal_behavior::animal_run_target)
	{
		target = target_pos;
		if_stop = false;
		//重设速度
		XMFLOAT3 now_position_need;
		get_now_position(now_position_need.x, now_position_need.y, now_position_need.z);
		//change_rotation_angle();
		float delta_x = target.x - now_position_need.x;
		float delta_z = target.z - now_position_need.z;
		if (abs(delta_x) < 3.0001f && abs(delta_z) < 3.0001f)
		{
			if_stop = true;
			set_speed(0.0f, 0.0f);
			return;
		}
		float square_plus_speed = sqrt(delta_x * delta_x + delta_z * delta_z);
		delta_x /= square_plus_speed;
		delta_z /= square_plus_speed;
		//计算旋转角度
		float dot_angle = delta_x  * 0.0f + delta_z*-1.0f;
		float final_angle;
		final_angle = acos(dot_angle);
		//判断补角
		float cross = delta_x  * -1.0f - 0.0f * delta_z;
		if (cross < 0.0f)
		{
			final_angle = XM_2PI - final_angle;
		}
		reset_rotation_angle(final_angle);

		set_speed(delta_x * animal_message.velocity_run, delta_z * animal_message.velocity_run);
	}
}

void AI_animal::show_assimp_skinmesh_model(XMFLOAT4X4 view_proj_matrix)
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的属性
	auto* model_yuri_pack = geometry_pancy->get_assimp_ModelResourceView_by_name(model_view_data_name);
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
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
	shader_test->get_technique(rec_point, num_member, &teque_need, "LightWithBone");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	world_matrix = model_yuri_pack->get_world_matrix();
	rec_world = XMLoadFloat4x4(&world_matrix);
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX viewproj = XMLoadFloat4x4(&view_proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*viewproj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//获取渲染路径并渲染
	int yuri_render_order[11] = { 4,5,6,7,8,9,10,3,0,2,1 };
	XMFLOAT4X4 *rec_bonematrix = model_yuri_pack->get_bone_matrix();
	shader_test->set_bone_matrix(rec_bonematrix, model_yuri_pack->get_bone_num());


	for (int i = 0; i < model_yuri_pack->get_geometry_num(); ++i)
	{
		material_list rec_need;
		model_yuri_pack->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_yuri_pack->draw_mesh_part(teque_need, i);
	}
}
void AI_animal::get_animation_data(float &animation_st, float &animation_ed, float &animation_speed)
{
	if (animal_now_behavior == animal_behavior::animal_walk_around)
	{
		animation_st = animal_message.animation_walk_start;
		animation_ed = animal_message.animation_walk_end;
		animation_speed = animal_message.animation_walk_speed;
	}
	else if (animal_now_behavior == animal_behavior::animal_run_around || animal_now_behavior == animal_behavior::animal_run_target)
	{
		animation_st = animal_message.animation_run_start;
		animation_ed = animal_message.animation_run_end;
		animation_speed = animal_message.animation_run_speed;
	}
}
void AI_animal::set_animation_time(float time)
{
	animal_animation_time = time;
}
void AI_animal::check_if_stop()
{
	if (all_time > 10.0f)
	{
		all_time -= 10.0f;
		if_stop = true;
		set_speed(0.0f, 0.0f);
		return;
	}
	/*
	XMFLOAT3 now_position_need;
	get_now_position(now_position_need.x, now_position_need.y, now_position_need.z);
	//change_rotation_angle();
	float delta_x = target.x - now_position_need.x;
	float delta_z = target.z - now_position_need.z;
	if (abs(delta_x) < 3.0001f && abs(delta_z) < 3.0001f)
	{
		if_stop = true;
		set_speed(0.0f, 0.0f);
		return;
		//return XMFLOAT3(0.0f, 0.0f, 0.0f);
	}*/
	/*
	float square_plus = sqrt(delta_x * delta_x + delta_z * delta_z);
	delta_x /= square_plus;
	delta_z /= square_plus;
	//计算旋转角度
	float delta_angle = delta_x * 0.0f + delta_z * 1.0f;
	change_rotation_angle(acos(delta_angle));
	if (animal_now_behavior == animal_behavior::animal_walk_around)
	{
		set_speed(delta_x * animal_message.velocity_walk,delta_z * animal_message.velocity_walk);
		//return XMFLOAT3(delta_x * animal_message.velocity_walk,0.0f, delta_y * animal_message.velocity_walk);
	}
	else if (animal_now_behavior == animal_behavior::animal_run_around || animal_now_behavior == animal_behavior::animal_run_target)
	{
		set_speed(delta_x * animal_message.velocity_run,delta_z * animal_message.velocity_run);
		//return XMFLOAT3(delta_x * animal_message.velocity_run, 0.0f, delta_y * animal_message.velocity_run);
	}
	*/
}
/*
HRESULT AI_animal::get_walk_dir(XMFLOAT3 &walk_dir)
{
	XMFLOAT3 now_position;
	get_now_position(now_position.x, now_position.y, now_position.z);
	XMFLOAT3 delta_position;
	delta_position.x = now_position.x - last_position.x;
	delta_position.y = now_position.y - last_position.y;
	delta_position.z = now_position.z - last_position.z;
	float normalize_need = sqrt(delta_position.x * delta_position.x + delta_position.y * delta_position.y + delta_position.z * delta_position.z);
	if (normalize_need < 0.01f)
	{
		walk_dir.x = 0.0f;
		walk_dir.y = 0.0f;
		walk_dir.z = 0.0f;
		return E_FAIL;
	}
	walk_dir.x = delta_position.x;
	walk_dir.y = delta_position.y;
	walk_dir.z = delta_position.z;
	return S_OK;
}
*/
HRESULT AI_animal::check_walk_dir(XMFLOAT3 walk_dir, XMFLOAT3 now_dir)
{
	float dot_data = walk_dir.x * now_dir.x + walk_dir.y * now_dir.y + walk_dir.z * now_dir.z;
	if (dot_data < 0.01f)
	{
		return E_FAIL;
	}
	return S_OK;
}
//地图资源实例类
static_resource_basic::static_resource_basic(static_resource_type resource_type_need, int map_ID, std::string model_resource_view_name, pancy_physx  *physic_need, geometry_control *geometry_need, XMFLOAT3 position_need, float scaling_need)
{
	resource_hp = 700;
	rebirth_time = 300;
	if_dead = false;
	time_dead = 0.0f;
	resource_type = resource_type_need;
	resource_map_ID = map_ID;
	resource_view_name = model_resource_view_name;
	physic_pancy = physic_need;
	geometry_pancy = geometry_need;
	scaling = scaling_need;
	position = position_need;
	mat_force = NULL;
}
HRESULT static_resource_basic::create()
{
	HRESULT hr;
	//检验该种资源访问表是否存在
	auto rec_resource_type = geometry_pancy->get_plant_ResourceView_by_name(resource_view_name);
	if (rec_resource_type == NULL)
	{
		MessageBox(0, L"can not find the plant resource", L"tip", MB_OK);
		return E_FAIL;
	}

	//hr = init_geometry_bounding_box();
	//if (FAILED(hr))
	//{
	//return hr;
	//}
	hr = init_physics();
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void static_resource_basic::release()
{
}
HRESULT static_resource_basic::init_physics()
{
	return S_OK;
}
HRESULT static_resource_basic::init_geometry_bounding_box()
{
	return S_OK;
}
void static_resource_basic::show_bounding_box(XMFLOAT4X4 view_proj_matrix)
{
}
HRESULT static_resource_basic::init_data2pipeline()
{
	auto geometry_resource_view_data = geometry_pancy->get_plant_ResourceView_by_name(resource_view_name);
	if (geometry_resource_view_data == NULL)
	{
		MessageBox(0, L"can not find the plant resource", L"tip", MB_OK);
		return E_FAIL;
	}
	XMFLOAT4X4 mat_translation;
	XMStoreFloat4x4(&mat_translation, XMMatrixScaling(scaling, scaling, scaling)*XMMatrixRotationX(0.5f*XM_PI)*XMMatrixTranslation(position.x, position.y, position.z));
	int check_num = geometry_resource_view_data->get_instance_num();
	if (check_num >= MAX_PLANT - 10)
	{
		return E_FAIL;
	}
	HRESULT hr = geometry_resource_view_data->add_a_instance(resource_map_ID, mat_translation);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
int static_resource_basic::get_num_geometry_view()
{
	auto geometry_resource_view_data = geometry_pancy->get_plant_ResourceView_by_name(resource_view_name);
	if (geometry_resource_view_data == NULL)
	{
		MessageBox(0, L"can not find the plant resource", L"tip", MB_OK);
		return 0;
	}
	return geometry_resource_view_data->get_instance_num();
}
void static_resource_basic::update_time(float now_time)
{
	if (if_dead)
	{
		//time_dead += now_time;
		if (now_time - time_dead >= rebirth_time)
		{
			if_dead = false;
			time_dead = 0.0f;
			resource_hp = 100;
		}
	}
}
void static_resource_basic::hit_resource(int hp_minus)
{
	resource_hp -= hp_minus;
	if (resource_hp <= 0)
	{
		if_dead = true;
	}
}
//地图装饰品实例类
static_decorate_basic::static_decorate_basic(int map_ID, std::string model_resource_view_name, pancy_physx  *physic_need, geometry_control *geometry_need, XMFLOAT3 position_need, float scaling_need)
{
	resource_map_ID = map_ID;
	resource_view_name = model_resource_view_name;
	physic_pancy = physic_need;
	geometry_pancy = geometry_need;
	scaling = scaling_need;
	position = position_need;
	mat_force = NULL;
}
HRESULT static_decorate_basic::create()
{
	HRESULT hr;
	//检验该种资源访问表是否存在
	auto rec_resource_type = geometry_pancy->get_plant_ResourceView_by_name(resource_view_name);
	if (rec_resource_type == NULL)
	{
		MessageBox(0, L"can not find the plant resource", L"tip", MB_OK);
		return E_FAIL;
	}

	//hr = init_geometry_bounding_box();
	//if (FAILED(hr))
	//{
	//return hr;
	//}
	hr = init_physics();
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void static_decorate_basic::release()
{
}
HRESULT static_decorate_basic::init_physics()
{
	return S_OK;
}
HRESULT static_decorate_basic::init_geometry_bounding_box()
{
	return S_OK;
}
void static_decorate_basic::show_bounding_box(XMFLOAT4X4 view_proj_matrix)
{
}
HRESULT static_decorate_basic::init_data2pipeline()
{
	auto geometry_resource_view_data = geometry_pancy->get_plant_ResourceView_by_name(resource_view_name);
	if (geometry_resource_view_data == NULL)
	{
		MessageBox(0, L"can not find the plant resource", L"tip", MB_OK);
		return E_FAIL;
	}
	XMFLOAT4X4 mat_translation;
	XMStoreFloat4x4(&mat_translation, XMMatrixScaling(scaling, scaling, scaling)*XMMatrixRotationX(0.5f*XM_PI)*XMMatrixTranslation(position.x, position.y, position.z));
	int check_num = geometry_resource_view_data->get_instance_num();
	if (check_num >= MAX_PLANT - 10)
	{
		return E_FAIL;
	}
	HRESULT hr = geometry_resource_view_data->add_a_instance(resource_map_ID, mat_translation);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
int static_decorate_basic::get_num_geometry_view()
{
	auto geometry_resource_view_data = geometry_pancy->get_plant_ResourceView_by_name(resource_view_name);
	if (geometry_resource_view_data == NULL)
	{
		MessageBox(0, L"can not find the plant resource", L"tip", MB_OK);
		return 0;
	}
	return geometry_resource_view_data->get_instance_num();
}
//用户地图管理
pancy_world_map::pancy_world_map(FMOD_basic * audio_engine_need, ID3D11DeviceContext *contex_need, pancy_renderstate *renderstate_need, geometry_control *geometry_need, pancy_physx  *physic_need, shader_control *shader_need, pancy_terrain_build *terrain_input, float resource_range_need, float decorate_range_need)
{
	audio_engine = audio_engine_need;
	time_game = 0.0f;
	contex_pancy = contex_need;
	renderstate_lib = renderstate_need;
	geometry_pancy = geometry_need;
	physic_pancy = physic_need;
	shader_pancy = shader_need;
	terrain_data = terrain_input;
	resource_range = resource_range_need;
	decorate_range = decorate_range_need;
}
HRESULT pancy_world_map::create()
{
	return S_OK;
}
void pancy_world_map::display(XMFLOAT4X4 view_matrix, XMFLOAT4X4 proj_matrix)
{
	for (auto data = resourcedataview_namelist.begin(); data != resourcedataview_namelist.end(); ++data)
	{
		auto *floor_need = geometry_pancy->get_plant_ResourceView_by_name(*data._Ptr);
		if (floor_need->get_instance_num() == 0)
		{
			continue;
		}
		//设置渲染模式
		float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		ID3D11BlendState *rec = renderstate_lib->get_blend_tocoverage();
		contex_pancy->OMSetBlendState(renderstate_lib->get_blend_tocoverage(), blendFactor, 0xffffffff);
		//获取渲染路径
		auto* shader_test = shader_pancy->get_shader_light_deffered_draw();
		ID3DX11EffectTechnique *teque_need;
		shader_test->get_technique(&teque_need, "LightTech_instance");
		//设定世界变换
		XMFLOAT4X4 rec_mat[MAX_PLANT];
		int mat_num;
		floor_need->get_world_matrix_array(mat_num, rec_mat);
		shader_test->set_world_matrix_array(rec_mat, mat_num);
		//设定总变换
		XMFLOAT4X4 world_viewrec;
		XMMATRIX view = XMLoadFloat4x4(&view_matrix);
		XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
		XMMATRIX ViewProj = view*proj;
		XMStoreFloat4x4(&world_viewrec, ViewProj);
		shader_test->set_trans_viewproj(&world_viewrec);
		//材质
		pancy_material test_Mt;
		XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
		XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
		XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
		test_Mt.ambient = rec_ambient2;
		test_Mt.diffuse = rec_diffuse2;
		test_Mt.specular = rec_specular2;
		test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		shader_test->set_material(test_Mt);
		material_list rec_texture;
		for (int i = 0; i < floor_need->get_geometry_num(); ++i)
		{
			floor_need->get_texture(&rec_texture, i);
			shader_test->set_diffusetex(rec_texture.tex_diffuse_resource);
			floor_need->draw_mesh_part(teque_need, i);
		}
		contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	}
	for (auto data = decoratedataview_namelist.begin(); data != decoratedataview_namelist.end(); ++data)
	{
		auto *floor_need = geometry_pancy->get_plant_ResourceView_by_name(*data._Ptr);
		if (floor_need->get_instance_num() == 0)
		{
			continue;
		}
		//设置渲染模式
		float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		ID3D11BlendState *rec = renderstate_lib->get_blend_tocoverage();
		contex_pancy->OMSetBlendState(renderstate_lib->get_blend_tocoverage(), blendFactor, 0xffffffff);
		//获取渲染路径
		auto* shader_test = shader_pancy->get_shader_light_deffered_draw();
		ID3DX11EffectTechnique *teque_need;
		shader_test->get_technique(&teque_need, "LightTech_instance");
		//设定世界变换
		XMFLOAT4X4 rec_mat[MAX_PLANT];
		int mat_num;
		floor_need->get_world_matrix_array(mat_num, rec_mat);
		shader_test->set_world_matrix_array(rec_mat, mat_num);
		//设定总变换
		XMFLOAT4X4 world_viewrec;
		XMMATRIX view = XMLoadFloat4x4(&view_matrix);
		XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
		XMMATRIX ViewProj = view*proj;
		XMStoreFloat4x4(&world_viewrec, ViewProj);
		shader_test->set_trans_viewproj(&world_viewrec);
		//材质
		pancy_material test_Mt;
		XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
		XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
		XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
		test_Mt.ambient = rec_ambient2;
		test_Mt.diffuse = rec_diffuse2;
		test_Mt.specular = rec_specular2;
		test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		shader_test->set_material(test_Mt);
		material_list rec_texture;
		for (int i = 0; i < floor_need->get_geometry_num(); ++i)
		{
			floor_need->get_texture(&rec_texture, i);
			shader_test->set_diffusetex(rec_texture.tex_diffuse_resource);
			floor_need->draw_mesh_part(teque_need, i);
		}
		contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	}
	for (auto data = animaldataview_namelist.begin(); data != animaldataview_namelist.end(); ++data)
	{
		XMFLOAT4X4 world_viewrec;
		XMMATRIX view = XMLoadFloat4x4(&view_matrix);
		XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
		XMMATRIX ViewProj = view*proj;
		XMStoreFloat4x4(&world_viewrec, ViewProj);
		if ((*data._Ptr)->check_if_dead() == false)
		{
			(*data._Ptr)->display(world_viewrec);
		}
		//auto *floor_need = geometry_pancy->get_assimp_ModelResourceView_by_name(*data._Ptr);
		//floor_need->
	}
}
void pancy_world_map::get_view_cormer(float angle_view, XMFLOAT3 direction_view, XMFLOAT3 &direction_corner1, XMFLOAT3 &direction_corner2)
{
	float a = direction_view.x;
	float b = direction_view.z;
	float c = cos(angle_view / 2.0f);
	direction_corner1.x = (c - (b*(a*sqrt((a*a + b*b - c*c)) + b*c)) / (a*a + b*b)) / a;
	direction_corner1.y = 0.0f;
	direction_corner1.z = (a*sqrt((a*a + b*b - c*c)) + b*c) / (a*a + b*b);
	direction_corner2.x = (c + (b*(a*sqrt((a*a + b*b - c*c)) - b*c)) / (a*a + b*b)) / a;
	direction_corner2.y = 0.0f;
	direction_corner2.z = -(a*sqrt((a*a + b*b - c*c)) - b*c) / (a*a + b*b);
}
bool pancy_world_map::check_if_in_view(float angle_view, XMFLOAT3 position_view, XMFLOAT3 direction_view, XMFLOAT3 position_test)
{
	float check_direction_x = position_test.x - position_view.x;
	float check_direction_z = position_test.z - position_view.z;
	float rec_final = sqrt(check_direction_x * check_direction_x + check_direction_z * check_direction_z);
	check_direction_x /= rec_final;
	check_direction_z /= rec_final;
	float view_final = sqrt(direction_view.x * direction_view.x + direction_view.z * direction_view.z);
	float check_x_view = direction_view.x / view_final;
	float check_z_view = direction_view.z / view_final;
	float angle_dot = check_x_view * check_direction_x + check_z_view * check_direction_z;
	float width_height = 1.7786;
	if (angle_dot > cos((angle_view / 2.0f) * width_height))
	{
		return true;
	}
	return false;
}
bool pancy_world_map::check_if_in_range(XMFLOAT3 position_camera, XMFLOAT3 position_target, float distance_max)
{
	float distance = sqrt((position_camera.x - position_target.x) * (position_camera.x - position_target.x) + (position_camera.y - position_target.y)*(position_camera.y - position_target.y) + (position_camera.z - position_target.z) * (position_camera.z - position_target.z));
	if (distance > distance_max)
	{
		return false;
	}
	return true;
}
HRESULT pancy_world_map::init_decorate_by_view(XMFLOAT3 now_pos_camera, XMFLOAT3 now_lool_camera, float angle_projection, int dataID)
{
	XMFLOAT3 test_pos;
	HRESULT hr = find_decoratedata_position_byID(dataID, test_pos);
	if (FAILED(hr))
	{
		return hr;
	}
	bool check_ans = check_if_in_view(angle_projection, now_pos_camera, now_lool_camera, test_pos);
	if (!check_ans)
	{
		return E_FAIL;
	}
	hr = init_decoratedata_to_pipeline(dataID);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT pancy_world_map::init_resource_by_view(XMFLOAT3 now_pos_camera, XMFLOAT3 now_lool_camera, float angle_projection, int dataID)
{
	XMFLOAT3 test_pos;
	HRESULT hr = find_resource_position_byID(dataID, test_pos);
	if (FAILED(hr))
	{
		return hr;
	}
	bool check_ans = check_if_in_view(angle_projection, now_pos_camera, now_lool_camera, test_pos);
	if (!check_ans)
	{
		return E_FAIL;
	}
	hr = init_resourcedata_to_pipeline(dataID);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
static_resource_type pancy_world_map::get_resource_type_byname(string name_in)
{
	if (name_in == "RoughGrass_resource_view")
	{
		return static_resource_grass;
	}
	if (name_in == "BradfordPear_resource_view")
	{
		return static_resource_tree;
	}
	if (name_in == "Pumpkin_resource_view")
	{
		return static_resource_pumpkin;
	}
	if (name_in == "EuropeanAspen_resource_view")
	{
		return static_resource_bamboo;
	}
	if (name_in == "stone_model_resource_view")
	{
		return static_resource_stone;
	}
	return static_resource_grass;
}
void pancy_world_map::update(float delta_time, XMFLOAT3 now_pos_camera, XMFLOAT3 now_lool_camera, float angle_projection, XMFLOAT3 player_pos, float&attack_value)
{
	time_game += delta_time;
	for (auto data = decoratedataview_namelist.begin(); data != decoratedataview_namelist.end(); ++data)
	{
		auto *floor_need = geometry_pancy->get_plant_ResourceView_by_name(*data._Ptr);
		if (floor_need->get_instance_num() == 0)
		{
			continue;
		}
		floor_need->clear_instance();
	}
	for (auto data = resourcedataview_namelist.begin(); data != resourcedataview_namelist.end(); ++data)
	{
		auto *floor_need = geometry_pancy->get_plant_ResourceView_by_name(*data._Ptr);
		if (floor_need->get_instance_num() == 0)
		{
			continue;
		}
		floor_need->clear_instance();
	}
	//find_resource_position_byID();
	//获取摄像机位置所在的地图区域
	int center_ID;
	find_decoratedata_ID_byposition(now_pos_camera, center_ID);
	int count_render = 0;
	//渲染摄像机附近的装饰品
	int row_data = (center_ID / 10000) % 10000;
	int col_data = center_ID % 10000;
	//todo:快速可见性判断，暂时先用向量法进行裁剪，后期再加入其他算法
	int start_x = 0;
	int start_y = 0;
	int final_id = 1 * 100000000 + (row_data + start_x) * 10000 + col_data + start_y;
	init_decorate_by_view(now_pos_camera, now_lool_camera, angle_projection, final_id);
	//螺旋循环

	for (int i = 0; i < 80; ++i)
	{
		for (int j = 0; j < i; ++j)
		{
			int positive = 1;
			if (i % 2 == 0)
			{
				positive = -1;
			}
			start_x += positive;
			final_id = 1 * 100000000 + (row_data + start_x) * 10000 + col_data + start_y;
			init_decorate_by_view(now_pos_camera, now_lool_camera, angle_projection, final_id);
		}
		for (int j = 0; j < i; ++j)
		{
			int positive = 1;
			if (i % 2 == 0)
			{
				positive = -1;
			}
			start_y += positive;
			final_id = 1 * 100000000 + (row_data + start_x) * 10000 + col_data + start_y;
			init_decorate_by_view(now_pos_camera, now_lool_camera, angle_projection, final_id);
		}
	}
	find_resource_ID_byposition(now_pos_camera, center_ID);
	//渲染摄像机附近的资源
	row_data = (center_ID / 10000) % 10000;
	col_data = center_ID % 10000;
	//todo:快速可见性判断，暂时先用向量法进行裁剪，后期再加入其他算法
	start_x = 0;
	start_y = 0;
	final_id = 1 * 100000000 + (row_data + start_x) * 10000 + col_data + start_y;
	init_resource_by_view(now_pos_camera, now_lool_camera, angle_projection, final_id);
	//螺旋循环
	for (int i = 0; i < 80; ++i)
	{
		for (int j = 0; j < i; ++j)
		{
			int positive = 1;
			if (i % 2 == 0)
			{
				positive = -1;
			}
			start_x += positive;
			final_id = 1 * 100000000 + (row_data + start_x) * 10000 + col_data + start_y;
			init_resource_by_view(now_pos_camera, now_lool_camera, angle_projection, final_id);
		}
		for (int j = 0; j < i; ++j)
		{
			int positive = 1;
			if (i % 2 == 0)
			{
				positive = -1;
			}
			start_y += positive;
			final_id = 1 * 100000000 + (row_data + start_x) * 10000 + col_data + start_y;
			init_resource_by_view(now_pos_camera, now_lool_camera, angle_projection, final_id);
		}
	}
	//清空动物显示表单
	clear_animal_list();
	int count_animal_data = 0;
	attack_value = 0.0f;
	for (auto animal_data = animaldata_list.begin(); animal_data != animaldata_list.end(); ++animal_data)
	{
		XMFLOAT3 position_rec;
		animal_data._Ptr->get_now_position(position_rec.x, position_rec.y, position_rec.z);
		bool check_ans = check_if_in_view(angle_projection, now_pos_camera, now_lool_camera, position_rec);
		bool check_dis = check_if_in_range(now_pos_camera, position_rec, 500.0f);
		int music_data, channel_data;
		animal_data._Ptr->get_music_message(music_data, channel_data);
		bool check_dis_audio = check_if_in_range(now_pos_camera, position_rec, 300.0f);

		if (animal_data._Ptr->get_animal_behavior() == animal_behavior::animal_run_target)
		{
			bool check_if_attack = check_if_in_range(player_pos, position_rec, 20.0f);
			if (check_if_attack && animal_data._Ptr->check_if_hit_player())
			{
				attack_value += animal_data._Ptr->get_attack();
			}
			check_ans = true;
			check_dis = true;
			animal_data._Ptr->reset_target(player_pos);
		}
		int voice_data, channel_voice;
		animal_data._Ptr->get_music_voice_message(voice_data, channel_voice);
		if (check_dis && animal_data._Ptr->check_if_dead() == false)
		{
			if (channel_data >= 0)
			{
				audio_engine->start_sound(channel_data);
				audio_engine->set_sound_priority(channel_data, 129);
				XMFLOAT3 now_pos;
				animal_data._Ptr->get_now_position(now_pos.x, now_pos.y, now_pos.z);
				audio_engine->set_music_place(channel_data, now_pos, XMFLOAT3(0.0f, 0.0f, 0.0f));
				audio_engine->set_value(channel_data, 8.0f);


				audio_engine->start_sound(channel_voice);
				audio_engine->set_sound_priority(channel_voice, 129);
				animal_data._Ptr->get_now_position(now_pos.x, now_pos.y, now_pos.z);
				audio_engine->set_music_place(channel_voice, now_pos, XMFLOAT3(0.0f, 0.0f, 0.0f));
				audio_engine->set_value(channel_voice, 3.0f);

			}
		}
		else
		{
			audio_engine->set_sound_priority(channel_data, 255);
			audio_engine->pause_sound(channel_data);
			audio_engine->set_value(channel_data, 0.0f);

			audio_engine->set_sound_priority(channel_voice, 255);
			audio_engine->pause_sound(channel_voice);
			audio_engine->set_value(channel_voice, 0.0f);
		}
		if (check_ans && check_dis && animal_data._Ptr->check_if_dead() == false)
		{

			count_animal_data++;
			add_animal_data_view_byAIdata(animal_data._Ptr);
			//XMFLOAT3 now_position_check1,now_position_check2;
			//animal_data._Ptr->get_now_position(now_position_check1.x, now_position_check1.y, now_position_check1.z);
			XMFLOAT3 animal_data_position, animal_data_normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
			animal_data._Ptr->get_now_position(animal_data_position.x, animal_data_position.y, animal_data_position.z);
			//terrain_data->get_position_normal(animal_data_position, animal_data_normal);
			//animal_data._Ptr->set_now_up_normal(animal_data_normal);
			animal_data._Ptr->update(delta_time);

			//animal_data._Ptr->get_now_position(now_position_check2.x, now_position_check2.y, now_position_check2.z);
			/*
			now_position_check2.x -= now_position_check1.x;
			now_position_check2.y -= now_position_check1.y;
			now_position_check2.z -= now_position_check1.z;
			volatile XMFLOAT3 now_position_check3, now_position_check4;
			now_position_check3.x = now_position_check2.x / delta_time;
			now_position_check3.y = now_position_check2.y;
			now_position_check3.z = now_position_check2.z / delta_time;
			*/
			//volatile int a = 0;
		}

	}
	int a = 0;
	/*
	for (int i = 0; i < 20; ++i)
	{
		for (int j = 0; j < 20; ++j)
		{
			int final_id = 0 * 100000000 + (row_data + i) * 10000 + col_data + j;
			XMFLOAT3 test_pos;
			HRESULT hr = find_resource_position_byID(final_id, test_pos);
			if (hr == S_OK)
			{
				bool check_ans = check_if_in_view(angle_projection, now_pos_camera, now_lool_camera, test_pos);
				if (check_ans)
				{
					hr = init_resourcedata_to_pipeline(final_id);
					if (hr == S_OK)
					{
						count_render += 1;
					}
				}
			}
			if (i != 0 || j != 0)
			{
				final_id = 0 * 100000000 + (row_data - i) * 10000 + col_data - j;
				hr = find_resource_position_byID(final_id, test_pos);
				if (hr == S_OK)
				{
					bool check_ans = check_if_in_view(angle_projection, now_pos_camera, now_lool_camera, test_pos);
					if (check_ans)
					{
						hr = init_resourcedata_to_pipeline(final_id);
						if (hr == S_OK)
						{
							count_render += 1;
						}
					}
				}
			}

		}
	}
	*/
}
HRESULT pancy_world_map::add_animal_data(AI_animal *animal_data_need)
{
	if (animal_data_need == NULL)
	{
		return E_FAIL;
	}
	animaldata_list.push_back(*animal_data_need);
	return S_OK;
}
void pancy_world_map::release()
{
	for (auto data_need = animaldata_list.begin(); data_need != animaldata_list.end(); ++data_need)
	{
		data_need._Ptr->release();
	}
	animaldata_list.clear();
	resourcedataview_namelist.clear();
	decoratedataview_namelist.clear();
	animaldataview_namelist.clear();
	data_decorate_map.clear();
	data_resource_map.clear();
}
HRESULT pancy_world_map::add_decorate_instance_byname(std::string geometryresourceview_name, int resource_map_ID, float scal_range)
{
	XMFLOAT3 position;
	HRESULT hr = find_decoratedata_position_byID(resource_map_ID, position);
	if (FAILED(hr))
	{
		return hr;
	}
	static_decorate_basic *decorate_data_rec = new static_decorate_basic(resource_map_ID, geometryresourceview_name, physic_pancy, geometry_pancy, position, scal_range);
	hr = decorate_data_rec->create();
	//std::pair<int, static_decorate_basic> data_need;
	std::pair<int, static_decorate_basic> data_need(resource_map_ID, *decorate_data_rec);
	//data_need.second = *decorate_data_rec;
	//data_need.first = resource_map_ID;
	auto check_iferror = data_decorate_map.insert(data_need);
	//decorate_data_rec->init_data2pipeline();
	decorate_data_rec->release();
	delete decorate_data_rec;
	if (!check_iferror.second)
	{
		MessageBox(0, L"add instance failed,a repeat one already in", L"tip", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}
HRESULT pancy_world_map::add_resource_instance_byname(std::string geometryresourceview_name, int resource_map_ID, float scal_range)
{
	XMFLOAT3 position;
	HRESULT hr = find_resource_position_byID(resource_map_ID, position);
	if (FAILED(hr))
	{
		return hr;
	}
	static_resource_basic *resource_data_rec = new static_resource_basic(get_resource_type_byname(geometryresourceview_name), resource_map_ID, geometryresourceview_name, physic_pancy, geometry_pancy, position, scal_range);
	hr = resource_data_rec->create();
	std::pair<int, static_resource_basic> data_need(resource_map_ID, *resource_data_rec);
	//data_need.second = new static_resource_basic(resource_map_ID, geometryresourceview_name, physic_pancy, geometry_pancy, position, scal_range);
	//data_need.second = *resource_data_rec;
	//data_need.first = resource_map_ID;
	auto check_iferror = data_resource_map.insert(data_need);
	//resource_data_rec->init_data2pipeline();
	resource_data_rec->release();
	delete resource_data_rec;
	if (!check_iferror.second)
	{
		//MessageBox(0, L"add instance failed,a repeat one already in", L"tip", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}
HRESULT pancy_world_map::delete_resource_instance_byID(int resource_map_ID)
{
	auto data_rec_now = data_resource_map.find(resource_map_ID);
	if (data_rec_now != data_resource_map.end())
	{
		data_rec_now->second.release();
		//成功
		data_resource_map.erase(data_rec_now);
	}
	else
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT pancy_world_map::add_animal_data_byname(XMFLOAT3 bound_box_range, std::string geometryresource_name, std::string geometryresourceview_name, int resource_map_ID, float scal_range, animal_attribute animal_data_need)
{
	XMFLOAT3 position;
	HRESULT hr = find_resource_position_byID(resource_map_ID, position);
	if (FAILED(hr))
	{
		return hr;
	}
	AI_animal *animal_data_rec = new AI_animal(geometryresource_name, geometryresourceview_name, geometry_pancy, physic_pancy, shader_pancy, model_data_type::pancy_model_assimp, animal_data_need, position, false);
	hr = animal_data_rec->create(bound_box_range.x, 7.0, bound_box_range.y, bound_box_range.z, player_shape_box);
	animal_data_rec->set_position_center(position);
	if (FAILED(hr))
	{
		return hr;
	}
	animal_data_rec->set_model_scaling(scal_range);

	//创建音效
	int audio_turn;
	audio_engine->load_music("music\\walk.mp3", NULL, FMOD_3D, audio_turn);
	animal_data_rec->set_music_message(audio_turn, audio_turn);
	HRESULT hr_rec = audio_engine->play_sound(audio_turn, audio_turn);
	audio_engine->pause_sound(audio_turn);


	if (geometryresource_name == "wolf_model_resource")
	{
		audio_engine->load_music("music\\walf.mp3", NULL, FMOD_3D, audio_turn);
	}
	else if (geometryresource_name == "bull_model_resource")
	{
		audio_engine->load_music("music\\bullvoice.mp3", NULL, FMOD_3D, audio_turn);
	}
	else if (geometryresource_name == "panther_model_resource")
	{
		audio_engine->load_music("music\\panther.mp3", NULL, FMOD_3D, audio_turn);
	}
	else if (geometryresource_name == "rhino_model_resource")
	{
		audio_engine->load_music("music\\rhinovoice.mp3", NULL, FMOD_3D, audio_turn);
	}

	animal_data_rec->set_music_voice_message(audio_turn, audio_turn);
	hr_rec = audio_engine->play_sound(audio_turn, audio_turn);
	audio_engine->pause_sound(audio_turn);
	//加入队列
	//animal_data_rec->set_speed(0.00f, -0.06f);
	animaldata_list.push_back(*animal_data_rec);
	//animal_data_rec->release();
	delete_animal_data_view_byname(geometryresourceview_name);
	if (animal_data_rec->check_if_show_bound())
	{
		geometry_pancy->delete_BuiltIn_modelview_by_name(animal_data_rec->get_bound_view_name());
	}
	delete animal_data_rec;

	return S_OK;
}

HRESULT pancy_world_map::add_resource_data_view_byname(std::string geometryresource_name, std::string resource_data_view_name)
{
	HRESULT hr = geometry_pancy->add_plant_modelview_by_name(geometryresource_name, resource_data_view_name);
	if (FAILED(hr))
	{
		return hr;
	}
	resourcedataview_namelist.push_back(resource_data_view_name);
	return S_OK;
}
HRESULT pancy_world_map::add_decorate_data_view_byname(std::string geometryresource_name, std::string decorate_data_view_name)
{
	HRESULT hr = geometry_pancy->add_plant_modelview_by_name(geometryresource_name, decorate_data_view_name);
	if (FAILED(hr))
	{
		return hr;
	}
	decoratedataview_namelist.push_back(decorate_data_view_name);
	return S_OK;
}
HRESULT pancy_world_map::add_animal_data_view_byAIdata(AI_animal* AI_animal_data)
{
	//续接动画时间
	HRESULT hr = geometry_pancy->add_assimp_modelview_by_name(AI_animal_data->get_model_resource_name(), AI_animal_data->get_model_view_name(), AI_animal_data->get_animation_time());
	if (FAILED(hr))
	{
		return hr;
	}
	//更新动物状态
	assimpmodel_resource_view *data_assimp = geometry_pancy->get_assimp_ModelResourceView_by_name(AI_animal_data->get_model_view_name());
	float st_anim = 0.0f, ed_anim = 0.0f, speed_anim = 0.0f;
	AI_animal_data->get_animation_data(st_anim, ed_anim, speed_anim);
	data_assimp->reset_animation_data(st_anim, ed_anim, speed_anim);
	//data_assimp->reset_animation_data(0, 20,5);
	//data_assimp->reset_animation_data(45, 60, 5);
	if (AI_animal_data->check_if_show_bound())
	{
		HRESULT hr = geometry_pancy->add_buildin_modelview_by_name("geometry_cube", AI_animal_data->get_bound_view_name(), Geometry_type::pancy_geometry_cube);
		if (FAILED(hr))
		{
			return hr;
		}
	}
	//AI_animal_data->set_speed(0.00f, -0.06f);
	animaldataview_namelist.push_back(AI_animal_data);
	return S_OK;
}
void pancy_world_map::clear_animal_list()
{
	for (auto data = animaldataview_namelist.begin(); data != animaldataview_namelist.end(); ++data)
	{
		auto data_animation = geometry_pancy->get_assimp_ModelResourceView_by_name((*data._Ptr)->get_model_view_name());
		//存储动画时间
		(*data._Ptr)->set_animation_time(data_animation->get_animation_time());
		//暂停音乐播放
		//(*data._Ptr)->get_music_message();
		//(*data._Ptr)->set_speed(0.0f, 0.0f);
		delete_animal_data_view_byname((*data._Ptr)->get_model_view_name());
		if ((*data._Ptr)->check_if_show_bound())
		{
			geometry_pancy->delete_BuiltIn_modelview_by_name((*data._Ptr)->get_bound_view_name());
		}
	}
	animaldataview_namelist.clear();
}
void pancy_world_map::delete_animal_data_view_byname(std::string geometryresourceview_name)
{
	geometry_pancy->delete_assimp_modelview_by_name(geometryresourceview_name);
}
HRESULT pancy_world_map::init_decoratedata_to_pipeline(int dataID)
{
	auto data_rec_now = data_decorate_map.find(dataID);
	if (data_rec_now != data_decorate_map.end())
	{
		//成功
		HRESULT hr = data_rec_now->second.init_data2pipeline();
		if (FAILED(hr))
		{
			return E_FAIL;
		}
	}
	else
	{
		//MessageBox(0, L"find world map resource data error", L"tip", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}
HRESULT pancy_world_map::init_resourcedata_to_pipeline(int dataID)
{
	auto data_rec_now = data_resource_map.find(dataID);
	if (data_rec_now != data_resource_map.end())
	{
		if (data_rec_now->second.check_if_dead())
		{
			return E_FAIL;
		}
		//成功
		HRESULT hr = data_rec_now->second.init_data2pipeline();
		if (FAILED(hr))
		{
			return E_FAIL;
		}
	}
	else
	{
		//	MessageBox(0, L"find world map resource data error", L"tip", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}
HRESULT pancy_world_map::find_decoratedata_position_byID(int ID_find, XMFLOAT3 &position_out)
{
	HRESULT hr = terrain_data->get_position_ID(ID_find, decorate_range, position_out);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT pancy_world_map::find_resource_position_byID(int ID_find, XMFLOAT3 &position_out)
{
	HRESULT hr = terrain_data->get_position_ID(ID_find, resource_range, position_out);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT pancy_world_map::find_decoratedata_ID_byposition(XMFLOAT3 position_in, int &ID_out)
{
	HRESULT hr = terrain_data->get_ID_position(position_in, decorate_range, ID_out);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT pancy_world_map::find_resource_ID_byposition(XMFLOAT3 position_in, int &ID_out)
{
	HRESULT hr = terrain_data->get_ID_position(position_in, resource_range, ID_out);
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT pancy_world_map::hit_the_resource_byposition(player_weapon weapon_need, XMFLOAT3 position_in, int attack_value, bool &if_killed, static_resource_type &type_need)
{
	type_need = static_resource_type::static_resource_grass;
	if_killed = false;
	int ID_find;
	HRESULT hr = find_resource_ID_byposition(position_in, ID_find);
	if (FAILED(hr))
	{
		return E_FAIL;
	}
	int ID_first = (ID_find / 10000) % 10000;
	int ID_second = ID_find % 10000;
	int check_turn[] = { 0,-1,1,-2,2 };
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			int final_ID = 100000000 + (ID_first + check_turn[i]) * 10000 + ID_second + check_turn[j];
			auto data_rec_now = data_resource_map.find(final_ID);
			if (data_rec_now != data_resource_map.end())
			{
				if (!data_rec_now->second.check_if_dead())
				{

					if (weapon_need == player_weapon::weapon_axe)
					{
						if (data_rec_now->second.get_resource_type() != static_resource_type::static_resource_tree)
						{
							continue;
						}
					}
					if (weapon_need == player_weapon::weapon_hand)
					{
						if (data_rec_now->second.get_resource_type() != static_resource_type::static_resource_grass && data_rec_now->second.get_resource_type() != static_resource_type::static_resource_bamboo && data_rec_now->second.get_resource_type() != static_resource_type::static_resource_pumpkin)
						{
							continue;
						}
					}
					if (weapon_need == player_weapon::weapon_pick)
					{
						if (data_rec_now->second.get_resource_type() != static_resource_type::static_resource_stone)
						{
							continue;
						}
					}
					data_rec_now->second.hit_resource(attack_value);
					type_need = data_rec_now->second.get_resource_type();
					if (data_rec_now->second.check_if_dead())
					{
						if_killed = true;
					}
					return S_OK;
				}
			}
		}
	}


	//data_rec_now->second.hit_resource(attack_value);
	return E_FAIL;
}
HRESULT pancy_world_map::hit_the_animal_byposition(player_weapon weapon_need, XMFLOAT3 position_in, XMFLOAT3 now_lool_camera, float angle_projection, int attack_value, bool &if_killed, animal_type &type_need)
{
	if_killed = false;
	for (auto animal_data = animaldata_list.begin(); animal_data != animaldata_list.end(); ++animal_data)
	{
		XMFLOAT3 animal_position;
		animal_data._Ptr->get_now_position(animal_position.x, animal_position.y, animal_position.z);
		bool check_ans = check_if_in_view(angle_projection, position_in, now_lool_camera, animal_position);
		bool check_dis = check_if_in_range(position_in, animal_position, 50.0f);
		if (check_dis && animal_data._Ptr->check_if_dead() == false)
		{
			animal_data._Ptr->hit_animal(attack_value);
			type_need = animal_data._Ptr->get_animal_type();
			if (animal_data._Ptr->check_if_dead())
			{
				if_killed = true;
			}
			return S_OK;
		}
	}
	return E_FAIL;
}



pancy_map_design::pancy_map_design(pancy_world_map *map_control_need)
{
	map_control = map_control_need;
}
HRESULT pancy_map_design::create()
{
	return build_random_map();
}
void pancy_map_design::release()
{
}
HRESULT pancy_map_design::load_map_fromfile()
{
	return S_OK;
}
HRESULT pancy_map_design::build_random_map()
{
	map_control->add_resource_data_view_byname("RoughGrass_resource", "RoughGrass_resource_view");
	map_control->add_resource_data_view_byname("BradfordPear_resource", "BradfordPear_resource_view");
	map_control->add_resource_data_view_byname("EuropeanAspen_resource", "EuropeanAspen_resource_view");
	map_control->add_resource_data_view_byname("Pumpkin_resource", "Pumpkin_resource_view");
	map_control->add_resource_data_view_byname("stone_model_resource", "stone_model_resource_view");
	map_control->add_resource_data_view_byname("square_model_resource", "square_model_resource_view");
	/*
	map_control->add_decorate_data_view_byname("RoughGrass_resource", "RoughGrass_resource_view");
	map_control->add_decorate_data_view_byname("BradfordPear_resource", "BradfordPear_resource_view");
	map_control->add_decorate_data_view_byname("EuropeanAspen_resource", "EuropeanAspen_resource_view");
	map_control->add_decorate_data_view_byname("Pumpkin_resource", "Pumpkin_resource_view");
	*/
	resource_name_list.push_back("RoughGrass_resource_view");
	resource_name_list.push_back("BradfordPear_resource_view");
	resource_name_list.push_back("EuropeanAspen_resource_view");
	resource_name_list.push_back("Pumpkin_resource_view");
	resource_name_list.push_back("stone_model_resource_view");
	resource_name_list.push_back("square_model_resource_view");
	srand((unsigned)time(NULL));
	bool rec_use[900];
	float scal_plant[] = { 3.0f,4.0f,1.0f,3.0f ,0.1f };
	//{bull,panthor,rhino,wolf}
	float scal_animal[] = { 16.0f,16.0f,16.0f,10.0f };
	//{height,foward,slide}
	float anim_walk_st_animal[] = { 0,0,0,0 };
	float anim_walk_ed_animal[] = { 100,20,100,20 };
	float anim_run_st_animal[] = { 0,40,0,40 };
	float anim_run_ed_animal[] = { 100,65,100,65 };
	XMFLOAT3 bound_box_nimal[] = { XMFLOAT3{ 3.5f,14.0f,14.0f },XMFLOAT3{ 3.5f,14.0f,14.0f } ,XMFLOAT3{ 3.5f,14.0f,14.0f } ,XMFLOAT3{ 3.5f,14.0f,14.0f } };
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			for (int k = 0; k < 900; ++k)
			{
				rec_use[k] = true;
			}
			int count = 0;
			while (true)
			{
				int now_use = rand() % 900;
				if (rec_use[now_use] == true)
				{
					int type_res = 100000000;
					int now_pos_x = 30 * i + now_use / 30;
					int now_pos_y = 30 * j + now_use % 30;
					int final_data = type_res + now_pos_x * 10000 + now_pos_y;
					int now_plant = rand() % 5;
					int now_animal = rand() % 4;
					map_control->add_resource_instance_byname(resource_name_list[now_plant], final_data, scal_plant[now_plant]);
					if (count % 20 == 0)
					{
						std::stringstream sstr2;
						sstr2 << final_data;
						string str2;
						sstr2 >> str2;
						string name_animal = "test_animal0" + str2;
						animal_attribute animal_data_need;
						animal_data_need.burn_position_id = final_data;
						animal_data_need.animation_walk_speed = 10.0f;
						animal_data_need.animation_run_speed = 12.0f;
						animal_data_need.velocity_walk = 0.33f;
						animal_data_need.velocity_run = 0.55f;
						animal_data_need.animation_walk_start = anim_walk_st_animal[now_animal];
						animal_data_need.animation_walk_end = anim_walk_ed_animal[now_animal];
						animal_data_need.animation_run_start = anim_run_st_animal[now_animal];
						animal_data_need.animation_run_end = anim_run_ed_animal[now_animal];
						animal_data_need.view_range = 300.0f;
						map_control->add_animal_data_byname(bound_box_nimal[now_animal], animal_geometry_name_list[now_animal], name_animal, final_data, scal_animal[now_animal], animal_data_need);
						int rec_a = 0;
					}
					count += 1;
					rec_use[now_use] = false;
				}
				if (count >= 30)
				{
					break;
				}
			}

		}
	}
	/*
	for (int i = -50; i < 50; ++i)
	{
		for (int j = -50; j < 50; ++j)
		{
			int type_res = 100000000;
			int now_pos_x = 160 + i * 2;
			int now_pos_y = 160 + j * 2;
			int final = type_res + now_pos_x * 10000 + now_pos_y;
			map_control->add_decorate_instance_byname(decorate_name_list[3], final, 3.0f);
		}
	}
	*/
	return S_OK;
}
HRESULT pancy_map_design::save_map_tofile()
{
	return S_OK;
}

goods_member::goods_member(GUI_control  *GUI_engine_need, ID3D11Device *device_need, string goods_name_need, int tex_ID_need)
{
	GUI_engine = GUI_engine_need;
	goods_name = goods_name_need;
	device_pancy = device_need;
	goods_position = XMFLOAT3(0, 0, 0);
	goods_number = 0;
	tex_ID = tex_ID_need;
	pos_ID = 0;
}
HRESULT goods_member::create(wchar_t *introduce_texture)
{
	HRESULT hr = CreateDDSTextureFromFile(device_pancy, introduce_texture, 0, &goods_introduce_texture, 0, 0);
	if (FAILED(hr))
	{
		MessageBox(0, L"init goods texture error", L"tip", MB_OK);
	}
	return S_OK;
}
void goods_member::add_a_goods()
{
	goods_number += 1;
}
void goods_member::delete_a_goods()
{
	goods_number -= 1;
}
void goods_member::release()
{
	goods_introduce_texture->Release();
}
void goods_member::change_position(XMFLOAT3 goods_position_need)
{
	goods_position = goods_position_need;
}
void goods_member::change_pos_ID(int goods_position_ID)
{
	pos_ID = goods_position_ID;
}

build_goods_member::build_goods_member(GUI_control  *GUI_engine_need, ID3D11Device *device_need, string goods_name_need, string build_out_name_need, std::vector<string> resourceuse_type_need, int* resourceuse_num_need, int tex_ID_need) :goods_member(GUI_engine_need, device_need, goods_name_need, tex_ID_need)
{
	int num = 0;
	for (auto data_need = resourceuse_type_need.begin(); data_need != resourceuse_type_need.end(); data_need++)
	{
		resource_use_type.push_back(*data_need._Ptr);
		resource_use_num[num] = resourceuse_num_need[num];
		num += 1;
	}
	resource_out_build = build_out_name_need;
}
HRESULT build_goods_member::get_resource_use_by_index(int index, int& out_resourceuse_num, string &out_resourceuse_type)
{
	if (index >= resource_use_type.size())
	{
		return E_FAIL;
	}
	out_resourceuse_num = resource_use_num[index];
	out_resourceuse_type = resource_use_type[index];
	return S_OK;
}

package_design::package_design(GUI_control  *GUI_engine_need, ID3D11Device *device_need, wchar_t *package_texture_name, int UI_num_per_row, int UI_num_per_col, wchar_t *goodstex_name_need, wchar_t *number_tex_name_need)
{
	device_pancy = device_need;
	GUI_engine = GUI_engine_need;
	goods_render_num = 0;
	bag_width = 5;
	bag_height = 6;
	package_tex_name = package_texture_name;
	goods_tex_name = goodstex_name_need;
	number_tex_name = number_tex_name_need;
	row_length_UI = UI_num_per_row;
	col_height_UI = UI_num_per_col;
	packet_position_start = XMFLOAT3(-0.815f, 0.315f, 0.0f);
	packet_position_offset = XMFLOAT3(0.185f, -0.23f, 0.0f);

	if_click = false;
	if_show_introduce = false;
	if_package_open = false;
}
HRESULT package_design::create()
{
	HRESULT hr = GUI_engine->add_a_common(package_tex_name, "package_main");
	if (FAILED(hr))
	{
		return hr;
	}
	hr = GUI_engine->add_a_common(goods_tex_name, "goods_instancing");
	if (FAILED(hr))
	{
		return hr;
	}
	hr = GUI_engine->add_a_common(number_tex_name, "goods_number");
	if (FAILED(hr))
	{
		return hr;
	}
	hr = GUI_engine->add_a_common(goods_tex_name, "goods_introduce");
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
HRESULT package_design::add_a_goods_type(int tex_id, string goods_name, wchar_t *tex_introduce)
{
	goods_member *goods_need = new goods_member(GUI_engine, device_pancy, goods_name, tex_id);
	HRESULT hr = goods_need->create(tex_introduce);
	if (FAILED(hr))
	{
		return hr;
	}
	std::pair<string, goods_member> data_need(goods_name, *goods_need);
	auto check_iferror = goods_list.insert(data_need);
	delete goods_need;
	if (!check_iferror.second)
	{
		MessageBox(0, L"add goods_view failed,a repeat one already in", L"tip", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}
void package_design::add_goodsnum_byname(string goods_name)
{
	auto data_rec_now = goods_list.find(goods_name);
	if (data_rec_now != goods_list.end())
	{
		data_rec_now->second.add_a_goods();
	}
}
void package_design::delete_goodsnum_byname(string goods_name)
{
	auto data_rec_now = goods_list.find(goods_name);
	if (data_rec_now != goods_list.end())
	{
		data_rec_now->second.delete_a_goods();
	}
}
void package_design::change_goods_position_byname(string goods_name, XMFLOAT3 position_input)
{
	auto data_rec_now = goods_list.find(goods_name);
	if (data_rec_now != goods_list.end())
	{
		data_rec_now->second.change_position(position_input);
	}
}
void package_design::change_goods_position_byname(string goods_name, int position_bag)
{
	float x_offset = static_cast<float>(position_bag % bag_width);
	float y_offset = static_cast<float>(position_bag / bag_width);
	XMFLOAT3 position_in;
	position_in = packet_position_start;
	position_in.x += packet_position_offset.x * x_offset;
	position_in.y += packet_position_offset.y * y_offset;
	auto data_rec_now = goods_list.find(goods_name);
	if (data_rec_now != goods_list.end())
	{
		data_rec_now->second.change_position(position_in);
		data_rec_now->second.change_pos_ID(position_bag);
	}
}
void package_design::open_package()
{
	if_package_open = true;
	for (auto data_save = goods_list.begin(); data_save != goods_list.end(); ++data_save)
	{
		if (data_save->second.get_resource_num() != 0)
		{
			change_goods_position_byname(data_save->first, data_save->second.get_pos_id());
		}
	}
};
void package_design::close_package()
{
	if_package_open = false;
};
void package_design::display()
{
	if (!if_package_open)
	{
		return;
	}
	GUI_engine->display_common("package_main");
	if (goods_render_num != 0)
	{
		GUI_engine->display_common_instancing("goods_instancing", goods_render_num, position_list, static_cast<float>(row_length_UI), static_cast<float>(col_height_UI));
		GUI_engine->display_common_instancing("goods_number", goods_render_num * 3, numpos_list, 11.0f, 1.0f);
	}
	if (if_show_introduce == true)
	{
		auto data_rec_now = goods_list.find(goods_introduce);
		if (data_rec_now != goods_list.end())
		{
			GUI_engine->display_common("goods_introduce", data_rec_now->second.get_introduce_tex());
		}
	}
}
void package_design::update(float delta_time)
{
	if (!if_package_open)
	{
		return;
	}
	//设置背包的大小
	GUI_engine->set_size_common("package_main", 1.0f, 1.0f, 0.0f);
	GUI_engine->set_position_common("package_main", 0.0f, 0.0f);
	GUI_engine->update_common(delta_time, "package_main");
	//设置物品的大小
	GUI_engine->set_size_common("goods_instancing", 0.07f, 0.09f, 0.0f);
	GUI_engine->set_position_common("goods_instancing", 0.0f, 0.0f);
	GUI_engine->update_common(delta_time, "goods_instancing");
	//设置解释栏的大小
	GUI_engine->set_size_common("goods_introduce", 0.2f, 0.2f, 0.0f);
	//GUI_engine->set_position_common("goods_introduce", 0.0f, 0.0f);
	GUI_engine->update_common(delta_time, "goods_introduce");
	//设置背包数字的大小
	GUI_engine->set_size_common("goods_number", 0.02f, 0.035f, 0.0f);
	GUI_engine->set_position_common("goods_number", 0.0f, 0.0f);
	GUI_engine->update_common(delta_time, "goods_number");

	if (GUI_engine->get_mouse_state() == mouse_state_down)
	{
		//先将鼠标状态标记为点击态
		if (if_click == false)
		{
			goods_click = get_now_mouse_goods();
			if_click = true;
		}
		if (goods_click != "")
		{
			auto data_rec_now = goods_list.find(goods_click);
			if (data_rec_now != goods_list.end())
			{
				XMFLOAT3 position_now;
				position_now.x = GUI_engine->get_mouse_position().x;
				position_now.y = GUI_engine->get_mouse_position().y;
				position_now.z = GUI_engine->get_mouse_position().z;
				data_rec_now->second.change_position(position_now);
			}
		}
	}
	else if (GUI_engine->get_mouse_state() == mouse_state_up)
	{
		//将鼠标还原
		if_click = false;
		//重新计算物品所在栏
		int check_now_pos = count_num_byposition(GUI_engine->get_mouse_position());
		//检测鼠标之前是否点选到了物品
		if (goods_click != "")
		{
			auto data_rec_now = goods_list.find(goods_click);
			//检测鼠标下面是不是有别的物品
			string check_if_use = get_now_pos_goods(check_now_pos);
			if (check_now_pos >= 0 && check_now_pos < bag_width * bag_height)
			{
				//成功更换物品所在栏
				string check_if_use = get_now_pos_goods(check_now_pos);
				if (check_if_use != "")
				{
					//成功交换物品栏
					change_goods_position_byname(check_if_use, data_rec_now->second.get_pos_id());
				}
				change_goods_position_byname(goods_click, check_now_pos);
			}
			else
			{
				//鼠标点选在了非法位置，还原物品原来的位置
				auto data_rec_now = goods_list.find(goods_click);
				if (data_rec_now != goods_list.end())
				{
					change_goods_position_byname(goods_click, data_rec_now->second.get_pos_id());
				}
			}
		}

	}
	else if (GUI_engine->get_mouse_state() == mouse_state_move)
	{
		int check_now_pos = count_num_byposition(GUI_engine->get_mouse_position());
		string check_if_use = get_now_pos_goods(check_now_pos);
		if (check_if_use != "")
		{
			if_show_introduce = true;
			goods_introduce = check_if_use;
			GUI_engine->set_position_common("goods_introduce", GUI_engine->get_mouse_position().x + 0.2f, GUI_engine->get_mouse_position().y + 0.2f);
		}
		else
		{
			if_show_introduce = false;
		}
	}
	get_pos();
}
void  package_design::get_pos()
{
	goods_render_num = 0;
	for (auto data_save = goods_list.begin(); data_save != goods_list.end(); ++data_save)
	{
		if (data_save->second.get_resource_num() != 0)
		{
			//XMFLOAT3 rec_pos_data = data_save._Ptr->get_goods_position();
			//float x_offset = static_cast<float>(turn % row_length_UI);
			//float y_offset = static_cast<float>(turn / row_length_UI);
			position_list[goods_render_num].x = data_save->second.get_goods_position().x;
			position_list[goods_render_num].y = data_save->second.get_goods_position().y;
			int turn = data_save->second.get_tex_id();
			position_list[goods_render_num].z = static_cast<float>(turn % row_length_UI) / static_cast<float>(row_length_UI);
			position_list[goods_render_num].w = static_cast<float>(turn / row_length_UI) / static_cast<float>(col_height_UI);
			//背包数字乘号
			numpos_list[goods_render_num * 3].x = data_save->second.get_goods_position().x + 0.04 - 0.03f;
			numpos_list[goods_render_num * 3].y = data_save->second.get_goods_position().y + 0.1;
			int goods_resource_num = data_save->second.get_resource_num();
			numpos_list[goods_render_num * 3].z = 10.0f / 11.0f;
			numpos_list[goods_render_num * 3].w = 0;
			//背包数字第一位
			numpos_list[goods_render_num * 3 + 1].x = data_save->second.get_goods_position().x + 0.04;
			numpos_list[goods_render_num * 3 + 1].y = data_save->second.get_goods_position().y + 0.06;
			numpos_list[goods_render_num * 3 + 1].z = static_cast<float>(goods_resource_num / 10) / 11.0f;
			numpos_list[goods_render_num * 3 + 1].w = 0;
			//背包数字第二位
			numpos_list[goods_render_num * 3 + 2].x = data_save->second.get_goods_position().x + 0.04 + 0.03f;
			numpos_list[goods_render_num * 3 + 2].y = data_save->second.get_goods_position().y + 0.06;
			numpos_list[goods_render_num * 3 + 2].z = static_cast<float>(goods_resource_num % 10) / 11.0f;
			numpos_list[goods_render_num * 3 + 2].w = 0;

			goods_render_num += 1;
		}
	}
}
bool package_design::check_mouse_on_instancing(XMFLOAT3 position_bag)
{
	XMFLOAT4 position_in;
	position_in.x = position_bag.x;
	position_in.y = position_bag.y;
	return GUI_engine->get_common_byname("goods_instancing")->check_if_in_range_instancing(position_in);
}
string package_design::get_now_mouse_goods()
{
	for (auto data_save = goods_list.begin(); data_save != goods_list.end(); ++data_save)
	{
		if (data_save->second.get_resource_num() != 0)
		{
			if (check_mouse_on_instancing(data_save->second.get_goods_position()))
			{
				return data_save->first;
			}
		}
	}
	return "";
}
string package_design::get_now_pos_goods(int pos_in)
{
	for (auto data_save = goods_list.begin(); data_save != goods_list.end(); ++data_save)
	{
		if (data_save->second.get_resource_num() != 0)
		{
			if (data_save->second.get_pos_id() == pos_in)
			{
				return data_save->first;
			}
		}
	}
	return "";
}
int package_design::count_num_byposition(XMFLOAT4 position_in)
{
	int row_value = static_cast<int>((position_in.x - packet_position_start.x) / packet_position_offset.x + 0.5f);
	int col_value = static_cast<int>((position_in.y - packet_position_start.y) / packet_position_offset.y + 0.5f);
	if (row_value < 0 || row_value >= bag_width || col_value < 0 || col_value >= bag_height)
	{
		return -1;
	}
	return row_value + col_value * bag_width;
}
void package_design::reset_goods_packetID(string goodsname)
{
	for (int i = 0; i < bag_width * bag_height; ++i)
	{
		if (get_now_pos_goods(i) == "")
		{
			change_goods_position_byname(goodsname, i);
			break;
		}
	}
}
void package_design::add_goods_by_resource(static_resource_type resource_in)
{
	if (resource_in == static_resource_type::static_resource_grass)
	{
		auto data_rec_now = goods_list.find("grass");
		if (data_rec_now != goods_list.end())
		{
			if (data_rec_now->second.get_resource_num() == 0)
			{
				reset_goods_packetID(data_rec_now->first);
			}
			data_rec_now->second.add_a_goods();
		}
	}
	if (resource_in == static_resource_type::static_resource_pumpkin)
	{
		auto data_rec_now = goods_list.find("pumpkin");
		if (data_rec_now != goods_list.end())
		{
			if (data_rec_now->second.get_resource_num() == 0)
			{
				reset_goods_packetID(data_rec_now->first);
			}
			data_rec_now->second.add_a_goods();
		}
	}
	if (resource_in == static_resource_type::static_resource_tree)
	{
		auto data_rec_now = goods_list.find("wood");
		if (data_rec_now != goods_list.end())
		{
			if (data_rec_now->second.get_resource_num() == 0)
			{
				reset_goods_packetID(data_rec_now->first);
			}
			data_rec_now->second.add_a_goods();
			data_rec_now->second.add_a_goods();
		}
		data_rec_now = goods_list.find("branch");
		if (data_rec_now != goods_list.end())
		{
			if (data_rec_now->second.get_resource_num() == 0)
			{
				reset_goods_packetID(data_rec_now->first);
			}
			data_rec_now->second.add_a_goods();
			data_rec_now->second.add_a_goods();
			data_rec_now->second.add_a_goods();
			data_rec_now->second.add_a_goods();
		}
	}
	if (resource_in == static_resource_type::static_resource_grass)
	{
		auto data_rec_now = goods_list.find("wood");
		if (data_rec_now != goods_list.end())
		{
			if (data_rec_now->second.get_resource_num() == 0)
			{
				reset_goods_packetID(data_rec_now->first);
			}
			data_rec_now->second.add_a_goods();
		}
		data_rec_now = goods_list.find("branch");
		if (data_rec_now != goods_list.end())
		{
			if (data_rec_now->second.get_resource_num() == 0)
			{
				reset_goods_packetID(data_rec_now->first);
			}
			data_rec_now->second.add_a_goods();
			data_rec_now->second.add_a_goods();
		}
	}
	if (resource_in == static_resource_type::static_resource_stone)
	{
		auto data_rec_now = goods_list.find("coal");
		if (data_rec_now != goods_list.end())
		{
			if (data_rec_now->second.get_resource_num() == 0)
			{
				reset_goods_packetID(data_rec_now->first);
			}
			data_rec_now->second.add_a_goods();
		}
		data_rec_now = goods_list.find("stone");
		if (data_rec_now != goods_list.end())
		{
			if (data_rec_now->second.get_resource_num() == 0)
			{
				reset_goods_packetID(data_rec_now->first);
			}
			data_rec_now->second.add_a_goods();
			data_rec_now->second.add_a_goods();
		}
	}
}
void package_design::add_goods_animal()
{
	auto data_rec_now = goods_list.find("meet");
	if (data_rec_now != goods_list.end())
	{
		if (data_rec_now->second.get_resource_num() == 0)
		{
			reset_goods_packetID(data_rec_now->first);
		}
		data_rec_now->second.add_a_goods();
	}

}
void package_design::release()
{
	for (auto data_save = goods_list.begin(); data_save != goods_list.end(); ++data_save)
	{
		data_save->second.release();
	}
	goods_list.clear();

}


package_with_build::package_with_build(GUI_control  *GUI_engine_need, ID3D11Device *device_need, wchar_t *package_texture_name, int UI_num_per_row, int UI_num_per_col, wchar_t *goodstex_name_need, wchar_t *number_tex_name_need) : package_design(GUI_engine_need, device_need, package_texture_name, UI_num_per_row, UI_num_per_col, goodstex_name_need, number_tex_name_need)
{
	build_width = 5;
	build_height = 3;
	if_click_build = false;
	build_position_start = XMFLOAT3(0.15f, 0.67f, 0.0f);
	//快速物品栏
	fastpacket_position_start = XMFLOAT3(-0.34f, -0.95f, 0.0f);
	fastpacket_position_offset = XMFLOAT3(0.1f, -0.23f, 0.0f);
}
HRESULT package_with_build::add_a_build_type(int tex_id, string build_name, string goods_out_name, std::vector<string> use_goods_type, int *use_goods_num, wchar_t *tex_introduce)
{
	build_goods_member *build_need = new build_goods_member(GUI_engine, device_pancy, build_name, goods_out_name, use_goods_type, use_goods_num, tex_id);
	HRESULT hr = build_need->create(tex_introduce);
	//确定建造物体的位置
	float x_offset = static_cast<float>(builds_list.size() % bag_width);
	float y_offset = static_cast<float>(builds_list.size() / bag_width);
	XMFLOAT3 build_position_need;
	XMFLOAT3 position_in;
	position_in = build_position_start;
	position_in.x += packet_position_offset.x * x_offset;
	position_in.y += packet_position_offset.y * y_offset;
	build_need->change_position(position_in);
	build_need->change_pos_ID(builds_list.size());
	if (FAILED(hr))
	{
		return hr;
	}
	std::pair<string, build_goods_member> data_need(build_name, *build_need);
	auto check_iferror = builds_list.insert(data_need);
	delete build_need;
	if (!check_iferror.second)
	{
		MessageBox(0, L"add build_view failed,a repeat one already in", L"tip", MB_OK);
		return E_FAIL;
	}
	return S_OK;
}
void package_with_build::display()
{
	if (!if_package_open)
	{
		if (goods_render_num != 0)
		{
			GUI_engine->display_common_instancing("goods_instancing", goods_render_num, position_list, static_cast<float>(row_length_UI), static_cast<float>(col_height_UI));
			GUI_engine->display_common_instancing("goods_number", goods_render_num * 3, numpos_list, 11.0f, 1.0f);
		}
		return;
	}
	GUI_engine->display_common("package_main");
	if (goods_render_num != 0)
	{
		GUI_engine->display_common_instancing("goods_instancing", goods_render_num, position_list, static_cast<float>(row_length_UI), static_cast<float>(col_height_UI));
		GUI_engine->display_common_instancing("goods_number", (goods_render_num - build_render_num) * 3, numpos_list, 11.0f, 1.0f);
	}
	if (if_show_introduce == true)
	{
		auto data_rec_now = goods_list.find(goods_introduce);
		if (data_rec_now != goods_list.end())
		{
			GUI_engine->display_common("goods_introduce", data_rec_now->second.get_introduce_tex());
		}
		else
		{
			auto data_rec_now = builds_list.find(goods_introduce);
			if (data_rec_now != builds_list.end())
			{
				GUI_engine->display_common("goods_introduce", data_rec_now->second.get_introduce_tex());
			}
		}
	}
}
void package_with_build::update(float delta_time)
{
	if (!if_package_open)
	{
		float scal_need = 0.6f;
		//设置物品的大小
		GUI_engine->set_size_common("goods_instancing", 0.07f * scal_need, 0.09f * scal_need, 0.0f);
		GUI_engine->set_position_common("goods_instancing", 0.0f, 0.0f);
		GUI_engine->update_common(delta_time, "goods_instancing");
		//设置背包数字的大小
		GUI_engine->set_size_common("goods_number", 0.02f * scal_need, 0.035f * scal_need, 0.0f);
		GUI_engine->set_position_common("goods_number", 0.0f, 0.0f);
		GUI_engine->update_common(delta_time, "goods_number");
		get_pos_first8();
		return;
	}
	//设置背包的大小
	GUI_engine->set_size_common("package_main", 1.0f, 1.0f, 0.0f);
	GUI_engine->set_position_common("package_main", 0.0f, 0.0f);
	GUI_engine->update_common(delta_time, "package_main");
	//设置物品的大小
	GUI_engine->set_size_common("goods_instancing", 0.07f, 0.09f, 0.0f);
	GUI_engine->set_position_common("goods_instancing", 0.0f, 0.0f);
	GUI_engine->update_common(delta_time, "goods_instancing");
	//设置解释栏的大小
	GUI_engine->set_size_common("goods_introduce", 0.5f * 0.7, 0.75f* 0.7, 0.0f);
	//GUI_engine->set_position_common("goods_introduce", 0.0f, 0.0f);
	GUI_engine->update_common(delta_time, "goods_introduce");
	//设置背包数字的大小
	GUI_engine->set_size_common("goods_number", 0.02f, 0.035f, 0.0f);
	GUI_engine->set_position_common("goods_number", 0.0f, 0.0f);
	GUI_engine->update_common(delta_time, "goods_number");
	if (GUI_engine->get_mouse_state() == mouse_state_down)
	{
		if (if_click_build == false)
		{
			int check_now_pos = count_build_num_bypos(GUI_engine->get_mouse_position());
			string build_goods_name = get_now_pos_build(check_now_pos);
			make_a_build_byname(build_goods_name);
			if_click_build = true;
		}
		//先将鼠标状态标记为点击态
		if (if_click == false)
		{
			goods_click = get_now_mouse_goods();
			if_click = true;
		}
		if (goods_click != "")
		{
			auto data_rec_now = goods_list.find(goods_click);
			if (data_rec_now != goods_list.end())
			{
				XMFLOAT3 position_now;
				position_now.x = GUI_engine->get_mouse_position().x;
				position_now.y = GUI_engine->get_mouse_position().y;
				position_now.z = GUI_engine->get_mouse_position().z;
				data_rec_now->second.change_position(position_now);
			}
		}
	}
	else if (GUI_engine->get_mouse_state() == mouse_state_up)
	{
		//将鼠标还原
		if_click = false;
		if_click_build = false;
		//重新计算物品所在栏
		int check_now_pos = count_num_byposition(GUI_engine->get_mouse_position());
		//检测鼠标之前是否点选到了物品
		if (goods_click != "")
		{
			auto data_rec_now = goods_list.find(goods_click);
			//检测鼠标下面是不是有别的物品
			string check_if_use = get_now_pos_goods(check_now_pos);
			if (check_now_pos >= 0 && check_now_pos < bag_width * bag_height)
			{
				//成功更换物品所在栏
				string check_if_use = get_now_pos_goods(check_now_pos);
				if (check_if_use != "")
				{
					//成功交换物品栏
					change_goods_position_byname(check_if_use, data_rec_now->second.get_pos_id());
				}
				change_goods_position_byname(goods_click, check_now_pos);
			}
			else
			{
				//鼠标点选在了非法位置，还原物品原来的位置
				auto data_rec_now = goods_list.find(goods_click);
				if (data_rec_now != goods_list.end())
				{
					change_goods_position_byname(goods_click, data_rec_now->second.get_pos_id());
				}
			}
		}

	}
	else if (GUI_engine->get_mouse_state() == mouse_state_move)
	{
		int check_now_pos = count_num_byposition(GUI_engine->get_mouse_position());
		string check_if_use = get_now_pos_goods(check_now_pos);
		if (check_if_use == "")
		{
			check_now_pos = count_build_num_bypos(GUI_engine->get_mouse_position());
			check_if_use = get_now_pos_build(check_now_pos);
			if (check_if_use != "")
			{
				if_show_introduce = true;
				goods_introduce = check_if_use;
				GUI_engine->set_position_common("goods_introduce", 0.45f, -0.6f);
			}
			else
			{
				if_show_introduce = false;
			}
		}
		else
		{
			if_show_introduce = true;
			goods_introduce = check_if_use;
			GUI_engine->set_position_common("goods_introduce", GUI_engine->get_mouse_position().x + 0.2f, GUI_engine->get_mouse_position().y + 0.2f);
		}

	}
	get_pos();
}
void package_with_build::release()
{
	for (auto data_save = goods_list.begin(); data_save != goods_list.end(); ++data_save)
	{
		data_save->second.release();
	}
	goods_list.clear();
	for (auto data_save = builds_list.begin(); data_save != builds_list.end(); ++data_save)
	{
		data_save->second.release();
	}
	builds_list.clear();
}
void package_with_build::get_pos_first8()
{
	for (int i = 0; i < 8; ++i)
	{
		package_fast[i] = -1;
	}
	goods_render_num = 0;
	for (auto data_save = goods_list.begin(); data_save != goods_list.end(); ++data_save)
	{
		if (data_save->second.get_resource_num() != 0 && data_save->second.get_pos_id() < 8)
		{
			package_fast[data_save->second.get_pos_id()] = data_save->second.get_tex_id();
			XMFLOAT3 position_now = fastpacket_position_start;
			position_now.x += static_cast<float>(data_save->second.get_pos_id()) * fastpacket_position_offset.x;
			data_save->second.change_position(position_now);

			position_list[goods_render_num].x = data_save->second.get_goods_position().x;
			position_list[goods_render_num].y = data_save->second.get_goods_position().y;
			int turn = data_save->second.get_tex_id();
			position_list[goods_render_num].z = static_cast<float>(turn % row_length_UI) / static_cast<float>(row_length_UI);
			position_list[goods_render_num].w = static_cast<float>(turn / row_length_UI) / static_cast<float>(col_height_UI);
			//背包数字乘号
			numpos_list[goods_render_num * 3].x = data_save->second.get_goods_position().x + 0.02 - 0.02f;
			numpos_list[goods_render_num * 3].y = data_save->second.get_goods_position().y + 0.1;
			int goods_resource_num = data_save->second.get_resource_num();
			numpos_list[goods_render_num * 3].z = 10.0f / 11.0f;
			numpos_list[goods_render_num * 3].w = 0;
			//背包数字第一位
			numpos_list[goods_render_num * 3 + 1].x = data_save->second.get_goods_position().x + 0.02;
			numpos_list[goods_render_num * 3 + 1].y = data_save->second.get_goods_position().y + 0.06;
			numpos_list[goods_render_num * 3 + 1].z = static_cast<float>(goods_resource_num / 10) / 11.0f;
			numpos_list[goods_render_num * 3 + 1].w = 0;
			//背包数字第二位
			numpos_list[goods_render_num * 3 + 2].x = data_save->second.get_goods_position().x + 0.02 + 0.02f;
			numpos_list[goods_render_num * 3 + 2].y = data_save->second.get_goods_position().y + 0.06;
			numpos_list[goods_render_num * 3 + 2].z = static_cast<float>(goods_resource_num % 10) / 11.0f;
			numpos_list[goods_render_num * 3 + 2].w = 0;

			goods_render_num += 1;
		}
	}
}
void  package_with_build::get_pos()
{
	goods_render_num = 0;
	for (auto data_save = goods_list.begin(); data_save != goods_list.end(); ++data_save)
	{
		if (data_save->second.get_resource_num() != 0)
		{
			//XMFLOAT3 rec_pos_data = data_save._Ptr->get_goods_position();
			//float x_offset = static_cast<float>(turn % row_length_UI);
			//float y_offset = static_cast<float>(turn / row_length_UI);
			position_list[goods_render_num].x = data_save->second.get_goods_position().x;
			position_list[goods_render_num].y = data_save->second.get_goods_position().y;
			int turn = data_save->second.get_tex_id();
			position_list[goods_render_num].z = static_cast<float>(turn % row_length_UI) / static_cast<float>(row_length_UI);
			position_list[goods_render_num].w = static_cast<float>(turn / row_length_UI) / static_cast<float>(col_height_UI);
			//背包数字乘号
			numpos_list[goods_render_num * 3].x = data_save->second.get_goods_position().x + 0.04 - 0.03f;
			numpos_list[goods_render_num * 3].y = data_save->second.get_goods_position().y + 0.1;
			int goods_resource_num = data_save->second.get_resource_num();
			numpos_list[goods_render_num * 3].z = 10.0f / 11.0f;
			numpos_list[goods_render_num * 3].w = 0;
			//背包数字第一位
			numpos_list[goods_render_num * 3 + 1].x = data_save->second.get_goods_position().x + 0.04;
			numpos_list[goods_render_num * 3 + 1].y = data_save->second.get_goods_position().y + 0.06;
			numpos_list[goods_render_num * 3 + 1].z = static_cast<float>(goods_resource_num / 10) / 11.0f;
			numpos_list[goods_render_num * 3 + 1].w = 0;
			//背包数字第二位
			numpos_list[goods_render_num * 3 + 2].x = data_save->second.get_goods_position().x + 0.04 + 0.03f;
			numpos_list[goods_render_num * 3 + 2].y = data_save->second.get_goods_position().y + 0.06;
			numpos_list[goods_render_num * 3 + 2].z = static_cast<float>(goods_resource_num % 10) / 11.0f;
			numpos_list[goods_render_num * 3 + 2].w = 0;

			goods_render_num += 1;
		}
	}
	build_render_num = 0;
	for (auto data_save = builds_list.begin(); data_save != builds_list.end(); ++data_save)
	{
		position_list[goods_render_num].x = data_save->second.get_goods_position().x;
		position_list[goods_render_num].y = data_save->second.get_goods_position().y;
		int turn = data_save->second.get_tex_id();
		position_list[goods_render_num].z = static_cast<float>(turn % row_length_UI) / static_cast<float>(row_length_UI);
		position_list[goods_render_num].w = static_cast<float>(turn / row_length_UI) / static_cast<float>(col_height_UI);
		goods_render_num += 1;
		build_render_num += 1;
	}
}
int  package_with_build::count_build_num_bypos(XMFLOAT4 position_in)
{
	int row_value = static_cast<int>((position_in.x - build_position_start.x) / packet_position_offset.x + 0.5f);
	int col_value = static_cast<int>((position_in.y - build_position_start.y) / packet_position_offset.y + 0.5f);
	if (row_value < 0 || row_value >= build_width || col_value < 0 || col_value >= build_height)
	{
		return -1;
	}
	return row_value + col_value * bag_width;
}
string package_with_build::get_now_pos_build(int pos_in)
{
	for (auto data_save = builds_list.begin(); data_save != builds_list.end(); ++data_save)
	{
		if (data_save->second.get_pos_id() == pos_in)
		{
			return data_save->first;
		}
	}
	return "";
}
HRESULT package_with_build::make_a_build_byname(string name)
{
	auto data_rec_now = builds_list.find(name);
	if (data_rec_now != builds_list.end())
	{
		//检验是否能够合成物品
		for (int i = 0; i < data_rec_now->second.get_resource_use_typenum(); ++i)
		{
			string goods_name;
			int goods_num;
			data_rec_now->second.get_resource_use_by_index(i, goods_num, goods_name);
			auto data_goods_now = goods_list.find(goods_name);
			if (data_goods_now == goods_list.end())
			{
				return E_FAIL;
			}
			if (data_goods_now->second.get_resource_num() < goods_num)
			{
				return E_FAIL;
			}
		}
		auto data_goods_out = goods_list.find(data_rec_now->second.get_resource_out_name());
		if (data_goods_out == goods_list.end())
		{
			return E_FAIL;
		}
		//开始合成物品
		for (int i = 0; i < data_rec_now->second.get_resource_use_typenum(); ++i)
		{
			string goods_name;
			int goods_num;
			data_rec_now->second.get_resource_use_by_index(i, goods_num, goods_name);
			auto data_goods_now = goods_list.find(goods_name);
			for (int j = 0; j < goods_num; ++j)
			{
				data_goods_now->second.delete_a_goods();
			}
		}
		if (data_goods_out->second.get_resource_num() == 0)
		{
			reset_goods_packetID(data_rec_now->second.get_resource_out_name());
		}
		data_goods_out->second.add_a_goods();
		return S_OK;
	}
}
int package_with_build::get_fast_pocket(int turn)
{
	if (turn >= 8 || turn < 0)
	{
		return -1;
	}
	return package_fast[turn];
}
/*
class static_resource_basic
{
	int resource_map_ID;
	XMFLOAT3 position;
	//几何属性
	geometry_control *geometry_pancy;
	std::string model_resource;
	std::string model_view_data_name;
	int model_view_data_index;
	//物理属性
	pancy_physx  *physic_pancy;
	physx::PxRigidStatic *bound_box;
	physx::PxMaterial *mat_force;
public:
	static_resource_basic(string model_resource_name, string model_resource_view_name, geometry_control *geometry_need, pancy_physx  *physic_need, shader_control *shader_need);
	HRESULT create();
	virtual void update(float delta_time);
	virtual void display(XMFLOAT4X4 view_proj_matrix);
	void release();
private:
	HRESULT init_physics();
	HRESULT init_geometry_assimp();
	HRESULT init_geometry_bounding_box();
	void show_bounding_box(XMFLOAT4X4 view_proj_matrix);
};
static_resource_basic::static_resource_basic(string model_resource_name, string model_resource_view_name, geometry_control *geometry_need, pancy_physx  *physic_need, shader_control *shader_need)
{
	model_resource = model_resource_name;
	model_view_data_name = model_resource_view_name;
	geometry_pancy = geometry_need;
	physic_pancy = physic_need;
	shader_lib = shader_need;
	mat_force = NULL;
}
HRESULT static_resource_basic::create()
{
	HRESULT hr;

	//hr = init_geometry_bounding_box();
	//if (FAILED(hr))
	//{
	//return hr;
	//}

	hr = init_geometry_assimp();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = init_physics();
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
*/
scene_engine_physicx::scene_engine_physicx(FMOD_basic *audio_engine_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_physx *physx_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height, float near_plane, float far_plane, float angle_view) : scene_root(device_need, contex_need, render_state, input_need, camera_need, lib_need, geometry_need, light_need, width, height, near_plane, far_plane, angle_view)
{
	last_ID = 0;
	if_build = false;
	if_use_fast = false;
	now_fast_choose = 0;
	now_weapon = player_weapon::weapon_hand;
	if_click_bag = false;
	physics_pancy = physx_need;
	camera_height = 0.0f;
	audio_engine = audio_engine_need;
	if_call_died = false;
	if_dead = false;
	//角色动画控制
	attack_time_save = 0.0f;
	if_attack = false;
	player_food = 100.0f;
	player_HP = 100.0f;
	player_power = 100.0f;
	if_use_eat = false;
	//test_start = NULL;
	//mouse_basic = NULL;
	//prograss_test = NULL;
}
HRESULT scene_engine_physicx::scene_create()
{
	HRESULT hr_need;
	//光源注册
	/*
	light_with_shadowmap rec_shadow(direction_light, shadow_map, shader_lib, device_pancy, contex_pancy, renderstate_lib, geometry_lib);
	hr_need = rec_shadow.create(1024, 1024);
	rec_shadow.set_light_dir(0.0f, -1.0f, 0.0f);
	rec_shadow.set_light_diffuse(0.6f, 0.6f,0.6f,1.0f);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	light_list->add_light_witn_shadow_map(rec_shadow);
	*/
	light_list->set_sunlight(XMFLOAT3(3.0f, -1.0f, 0.0f), 0.75, 4, 1024, 1024);
	//声音引擎
	/*
	if (audio_engine->init_system() != S_OK)
	{
		return E_FAIL;
	}
	*/
	//FMOD_CREATESOUNDEXINFO info_need;
	int music_ID_bgm, music_ID_enviroment;
	HRESULT hr = audio_engine->load_music("music\\gamebgm.mp3", NULL, FMOD_LOOP_NORMAL, music_ID_bgm);
	hr = audio_engine->load_music("music\\walk.mp3", NULL, FMOD_3D, music_ID_walk);
	hr = audio_engine->play_sound(music_ID_walk, music_ID_walk);
	audio_engine->pause_sound(music_ID_walk);
	hr = audio_engine->load_music("music\\wave.mp3", NULL, FMOD_LOOP_OFF, music_ID_wave);
	hr = audio_engine->load_music("music\\environment.mp3", NULL, FMOD_LOOP_NORMAL, music_ID_enviroment);
	hr = audio_engine->play_sound(music_ID_enviroment, music_ID_enviroment);
	hr = audio_engine->load_music("music\\woodhit.mp3", NULL, FMOD_LOOP_NORMAL, music_ID_woodhit);
	hr = audio_engine->load_music("music\\treedied.mp3", NULL, FMOD_LOOP_NORMAL, music_ID_treedied);
	hr = audio_engine->load_music("music\\hearted.mp3", NULL, FMOD_LOOP_NORMAL, music_ID_hearted);
	hr = audio_engine->load_music("music\\rhinohit.mp3", NULL, FMOD_LOOP_NORMAL, music_ID_hited);
	hr = audio_engine->load_music("music\\stone.wav", NULL, FMOD_LOOP_NORMAL, music_ID_stone_hit);
	hr = audio_engine->load_music("music\\stonedied.mp3", NULL, FMOD_LOOP_NORMAL, music_ID_stone_died);
	hr = audio_engine->play_sound(music_ID_bgm, music_ID_bgm);
	hr = audio_engine->pause_sound(music_ID_bgm);
	hr = audio_engine->pause_sound(music_ID_enviroment);
	audio_engine->set_value(music_ID_bgm, 0.1f);
	//物理引擎
	std::vector<LPCWSTR> height_map;
	std::vector<LPCWSTR> diffuse_map;
	height_map.push_back(L"terrain_height.dds");
	diffuse_map.push_back(L"terrain_color.dds");
	/*
	hr_need = physics_test->create();
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	*/
	terrain_test = new pancy_terrain_build(physics_pancy, device_pancy, contex_pancy, shader_lib, 1, 5, 3000, height_map, diffuse_map);
	hr_need = terrain_test->create();
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	geometry_lib->build_terrain_from_memory(terrain_test);
	world_map_main = new pancy_world_map(audio_engine, contex_pancy, renderstate_lib, geometry_lib, physics_pancy, shader_lib, terrain_test, 10.0f, 10.0f);
	hr_need = world_map_main->create();
	if (FAILED(hr_need))
	{
		return hr_need;
	}


	//读取场景需要的纹理
	int texture_index;
	hr_need = geometry_lib->load_texture_from_file(L"floor.dds", true, "floor_diffuse", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"floor_n.dds", true, "floor_normal", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"Texture_cube.dds", true, "sky_cube", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	//内置模型访问器
	//hr_need = geometry_lib->add_buildin_modelview_by_name("geometry_cube", "geometry_floor");
	//if (FAILED(hr_need))
	//{
	//	return hr_need;
	//}
	hr_need = geometry_lib->add_buildin_modelview_by_name("geometry_cube", "geometry_boxtest", pancy_geometry_cube);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->add_buildin_modelview_by_name("geometry_ball", "geometry_sky", pancy_geometry_ball);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	//外部模型导入
	int index_model_rec;
	int alpha_yuri[] = { 4,6,9,11 };
	//hr_need = geometry_lib->load_modelresource_from_file("panther\\panther.FBX", "panther\\", true, false, false, 4, alpha_yuri, "yuri_model_resource", index_model_rec);
	hr_need = geometry_lib->load_modelresource_from_file("stoneaxe\\stoneaxe.obj", "stoneaxe\\", false, false, false, 0, NULL, "stoneaxe_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "stoneaxe_model_resource_view");
	hr_need = geometry_lib->load_modelresource_from_file("pick\\pick.obj", "pick\\", false, false, false, 0, NULL, "pick_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "pick_model_resource_view");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("bull\\bull.FBX", "bull\\", true, false, false, 0, NULL, "bull_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("panther\\panther.FBX", "panther\\", true, false, false, 0, NULL, "panther_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("rhino\\rhino.FBX", "rhino\\", true, false, false, 0, NULL, "rhino_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("wolf\\wolf.FBX", "wolf\\", true, false, false, 0, NULL, "wolf_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("player_main\\player.FBX", "player_main\\", true, false, false, 4, alpha_yuri, "yuri_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("BroomSnakeweed_Cluster_Low\\BroomSnakeweed_Cluster_Low.obj", "BroomSnakeweed_Cluster_Low\\", false, false, false, 0, NULL, "grass_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("Cattail_Med\\Cattail_Med.obj", "Cattail_Med\\", false, false, false, 0, NULL, "RoughGrass_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("BradfordPear_Low\\BradfordPear_Low.obj", "BradfordPear_Low\\", false, false, false, 0, NULL, "BradfordPear_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("EuropeanAspen_Low\\EuropeanAspen_Low.obj", "EuropeanAspen_Low\\", false, false, false, 0, NULL, "EuropeanAspen_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("Pumpkin_Low\\Pumpkin_Low.obj", "Pumpkin_Low\\", false, false, false, 0, NULL, "Pumpkin_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("stone\\stone.fbx", "stone\\", false, false, false, 0, NULL, "stone_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->load_modelresource_from_file("stone\\square.obj", "stone\\", false, false, false, 0, NULL, "square_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	map_designer = new pancy_map_design(world_map_main);
	map_designer->add_an_animal_type("bull_model_resource");
	map_designer->add_an_animal_type("panther_model_resource");
	map_designer->add_an_animal_type("rhino_model_resource");
	map_designer->add_an_animal_type("wolf_model_resource");
	hr_need = map_designer->create();
	if (FAILED(hr_need))
	{
		MessageBox(0, L"init map error", L"tip", MB_OK);
		return hr_need;
	}
	/*
	world_map_main->add_decorate_data_view_byname("RoughGrass_resource", "RoughGrass_resource_view");
	for (int i = -50; i < 50; ++i)
	{
		for (int j = -50; j < 50; ++j)
		{
			int type_res = 100000000;
			int now_pos_x = 160 + i*2;
			int now_pos_y = 160 + j*2;
			int final = type_res + now_pos_x * 10000 + now_pos_y;
			world_map_main->add_decorate_instance_byname("RoughGrass_resource_view", final, 5.0f);
		}
	}
	*/
	//world_map_main->add_decorate_instance_byname("RoughGrass_resource_view", 101620172, 5.0f);
	//world_map_main->add_decorate_instance_byname("RoughGrass_resource_view", 101620173, 5.0f);
	//world_map_main->add_decorate_instance_byname("RoughGrass_resource_view", 101620174, 5.0f);
	/*
	hr_need = geometry_lib->add_plant_modelview_by_name("grass_model_resource", "grass_test_instance");
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	auto* floor_need = geometry_lib->get_plant_ResourceView_by_name("grass_test_instance");
	XMFLOAT4X4 rec_trans_mat;
	float scal_range = 2.0f;
	for (int i = 0; i < 260000; ++i)
	{
		XMStoreFloat4x4(&rec_trans_mat, XMMatrixScaling(scal_range, scal_range, scal_range)*XMMatrixRotationX(0.5f*XM_PI)*XMMatrixTranslation(0, 121, 0));
		floor_need->add_a_instance(i, rec_trans_mat);
	}
	XMStoreFloat4x4(&rec_trans_mat, XMMatrixScaling(scal_range, scal_range, scal_range)*XMMatrixRotationX(0.5f*XM_PI)*XMMatrixTranslation(0, 121, 0));
	floor_need->add_a_instance(0, rec_trans_mat);
	XMStoreFloat4x4(&rec_trans_mat, XMMatrixScaling(scal_range, scal_range, scal_range)*XMMatrixRotationX(0.5f*XM_PI)*XMMatrixTranslation(-14 - 40, 121, 0));
	floor_need->add_a_instance(1, rec_trans_mat);
	XMStoreFloat4x4(&rec_trans_mat, XMMatrixScaling(scal_range, scal_range, scal_range)*XMMatrixRotationX(0.5f*XM_PI)*XMMatrixTranslation(-28 - 40, 121, 0));
	floor_need->add_a_instance(2, rec_trans_mat);
	XMStoreFloat4x4(&rec_trans_mat, XMMatrixScaling(scal_range, scal_range, scal_range)*XMMatrixRotationX(0.5f*XM_PI)*XMMatrixTranslation(-32 - 40, 121, 0));
	floor_need->add_a_instance(3, rec_trans_mat);
	XMStoreFloat4x4(&rec_trans_mat, XMMatrixScaling(scal_range, scal_range, scal_range)*XMMatrixRotationX(0.5f*XM_PI)*XMMatrixTranslation(-46 - 40, 121, 0));
	floor_need->add_a_instance(4, rec_trans_mat);
	*/
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~GUI界面~~~~~~~~~~~~~~~~~~~~~~~~~~
	gui_list = new GUI_control(static_cast<float>(scene_window_width), static_cast<float>(scene_window_height), user_input, device_pancy, contex_pancy, shader_lib);
	hr_need = gui_list->create(L"UI\\mouse_move.dds", L"UI\\mouse_up.dds", L"UI\\mouse_down.dds");
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	gui_list->set_mouse_size(0.07f, 0.07f, 0.0f);
	player_package = new package_with_build(gui_list, device_pancy, L"UI\\package_UI.dds", 3, 4, L"UI\\icon1.dds", L"UI\\number.dds");
	hr_need = player_package->create();
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = player_package->add_a_goods_type(0, "pumpkin", L"UI\\introduce\\pumpkin.dds");
	hr_need = player_package->add_a_goods_type(1, "coal", L"UI\\introduce\\coal.dds");
	hr_need = player_package->add_a_goods_type(2, "iron", L"UI\\introduce\\iron.dds");
	hr_need = player_package->add_a_goods_type(3, "wood", L"UI\\introduce\\wood.dds");
	hr_need = player_package->add_a_goods_type(4, "stone", L"UI\\introduce\\stone.dds");
	hr_need = player_package->add_a_goods_type(5, "orange", L"UI\\introduce\\orange.dds");
	hr_need = player_package->add_a_goods_type(6, "meet", L"UI\\introduce\\meat.dds");
	hr_need = player_package->add_a_goods_type(7, "grass", L"UI\\introduce\\grass.dds");
	hr_need = player_package->add_a_goods_type(8, "bow", L"UI\\introduce\\bow.dds");
	hr_need = player_package->add_a_goods_type(9, "axe", L"UI\\introduce\\axe.dds");
	hr_need = player_package->add_a_goods_type(10, "pick", L"UI\\introduce\\pick.dds");
	hr_need = player_package->add_a_goods_type(11, "branch", L"UI\\introduce\\branch.dds");
	std::vector<string> axebuild_list;
	axebuild_list.push_back("stone");
	axebuild_list.push_back("branch");
	std::vector<string> pickbuild_list;
	pickbuild_list.push_back("stone");
	pickbuild_list.push_back("branch");
	int num_axe[2] = { 3,5 };
	int num_pick[2] = { 2,5 };
	//hr_need = player_package->add_a_build_type(8, "bow_goods_build", "bow", axebuild_list, num, L"UI\\introduce\\bow.dds");
	hr_need = player_package->add_a_build_type(9, "axe_goods_build", "axe", axebuild_list, num_axe, L"UI\\introduce\\axebuild.dds");
	hr_need = player_package->add_a_build_type(10, "pick_goods_build", "pick", pickbuild_list, num_pick, L"UI\\introduce\\pickbuild.dds");

	player_package->add_goodsnum_byname("pumpkin");
	player_package->add_goodsnum_byname("pumpkin");
	player_package->add_goodsnum_byname("pumpkin");
	for (int i = 0; i < 10; ++i)
	{
		player_package->add_goodsnum_byname("stone");
		player_package->add_goodsnum_byname("branch");
	}
	player_package->change_goods_position_byname("pumpkin", 0);
	player_package->change_goods_position_byname("stone", 1);
	player_package->change_goods_position_byname("branch", 2);
	gui_list->add_a_button(L"UI\\rebirth.dds", "ui_rebirth_game");
	gui_list->add_a_button(L"UI\\end.dds", "ui_end_game");
	gui_list->add_a_button(L"UI\\returngame.dds", "ui_return_game");

	gui_list->add_a_prograssbar(L"UI\\water.dds", "ui_food_prograssbar");
	gui_list->add_a_prograssbar(L"UI\\HP.dds", "ui_HP_prograssbar");
	gui_list->add_a_prograssbar(L"UI\\water.dds", "ui_power_prograssbar");

	gui_list->add_a_common(L"UI\\fast_inventory.dds", "ui_fast_inventory");
	//gui_list->add_a_common(L"UI\\package_UI.dds", "ui_package");
	/*
	mouse_basic = new GUI_mouse(static_cast<float>(scene_window_width), static_cast<float>(scene_window_height),user_input,device_pancy, contex_pancy, shader_lib);
	hr_need = mouse_basic->create(L"UI\\mouse_move.dds", L"UI\\mouse_up.dds", L"UI\\mouse_down.dds");
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	mouse_basic->set_mouse_size(0.07f, 0.07f, 0.0f);

	test_start = new GUI_button(mouse_basic,device_pancy,contex_pancy, shader_lib);
	hr_need = test_start->create(L"UI\\start1.dds");
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	test_start->set_UI_position(0, 0);
	test_start->set_UI_range(0.2f, 0.2f,0.0f);

	prograss_test = new GUI_progressbar(device_pancy, contex_pancy, shader_lib);
	hr_need = prograss_test->create(L"UI\\HP.dds");
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	prograss_test->set_UI_position(0.2f, 0);
	prograss_test->set_UI_range(0.2f, 0.1f, 0.0f);
	*/

	//创建角色
	player_main = new player_basic("yuri_model_resource", "test_player_one", geometry_lib, physics_pancy, shader_lib, model_data_type::pancy_model_assimp, XMFLOAT3(0.0f, 250.0f, 0.0f), false);
	//physics_test = new pancy_physx(device_need, contex_need);
	hr_need = player_main->create(3.5, 7.0, 0, 0, player_shape_capsule);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	player_main->set_model_scaling(16.0f);
	hr = audio_engine->start_sound(music_ID_bgm);
	hr = audio_engine->start_sound(music_ID_enviroment);

	//hr_need = geometry_lib->add_buildin_modelview_by_name("geometry_cube", "geometry_project_aabb", pancy_geometry_cube);
	//if (FAILED(hr_need))
	//{
	//	return hr_need;
	//}
	return S_OK;
}
void scene_engine_physicx::show_grass()
{
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState *rec = renderstate_lib->get_blend_tocoverage();
	contex_pancy->OMSetBlendState(renderstate_lib->get_blend_tocoverage(), blendFactor, 0xffffffff);
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	auto* floor_need = geometry_lib->get_plant_ResourceView_by_name("grass_test_instance");
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech_instance");
	//设定世界变换
	XMFLOAT4X4 rec_mat[MAX_PLANT];
	int mat_num;
	floor_need->get_world_matrix_array(mat_num, rec_mat);
	shader_test->set_world_matrix_array(rec_mat, mat_num);
	//设定总变换
	XMFLOAT4X4 world_viewrec;
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX ViewProj = view*proj;
	XMStoreFloat4x4(&world_viewrec, ViewProj);
	shader_test->set_trans_viewproj(&world_viewrec);



	material_list rec_texture;
	for (int i = 0; i < floor_need->get_geometry_num(); ++i)
	{
		floor_need->get_texture(&rec_texture, i);
		shader_test->set_diffusetex(rec_texture.tex_diffuse_resource);
		floor_need->draw_mesh_part(teque_need, i);
	}
	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
void scene_engine_physicx::show_ball()
{
	contex_pancy->RSSetState(renderstate_lib->get_CULL_front_rs());
	auto* shader_test = shader_lib->get_shader_reflect();
	auto* ball_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_sky");
	//auto* tex_skycube = geometry_lib->get_sky_cube_tex();
	auto* tex_skycube = geometry_lib->get_texture_byname("sky_cube")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_reflect");
	shader_test->set_trans_world(&ball_need->get_world_matrix());
	//设定立方贴图
	shader_test->set_tex_resource(tex_skycube);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&ball_need->get_world_matrix());
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);;
	ball_need->draw_full_geometry(teque_need);
	contex_pancy->RSSetState(NULL);
}
void scene_engine_physicx::show_floor()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	auto* floor_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_project_aabb");
	//auto* tex_floor = geometry_lib->get_basic_floor_tex();
	//auto* tex_normal = geometry_lib->get_floor_normal_tex();
	auto* tex_floor = geometry_lib->get_texture_byname("floor_diffuse")->data->get_data();
	auto* tex_normal = geometry_lib->get_texture_byname("floor_normal")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_withshadownormal");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);

	//设定世界变换
	shader_test->set_trans_world(&floor_need->get_world_matrix());
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&floor_need->get_world_matrix());

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	//floor_need->get_teque(teque_need);
	//floor_need->show_mesh();
	floor_need->draw_full_geometry(teque_need);
}
void scene_engine_physicx::show_box()
{
	//auto* shader_test = shader_lib->get_shader_prelight();
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	auto* floor_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_boxtest");
	//auto* tex_floor = geometry_lib->get_basic_floor_tex();
	//auto* tex_normal = geometry_lib->get_floor_normal_tex();
	auto* tex_floor = geometry_lib->get_texture_byname("floor_diffuse")->data->get_data();
	auto* tex_normal = geometry_lib->get_texture_byname("floor_normal")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	//shader_test->set_normaltex(tex_normal);

	//设定世界变换
	shader_test->set_trans_world(&floor_need->get_world_matrix());
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&floor_need->get_world_matrix());

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);

	//floor_need->get_teque(teque_need);
	//floor_need->show_mesh();
	floor_need->draw_full_geometry(teque_need);
}
void scene_engine_physicx::show_pick()
{
	if (now_weapon != player_weapon::weapon_pick)
	{
		return;
	}
	auto *floor_need = geometry_lib->get_assimp_ModelResourceView_by_name("pick_model_resource_view");
	//获取渲染路径
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");
	//设定世界变换
	XMFLOAT4X4 world_matrix;
	world_matrix = floor_need->get_world_matrix();
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMFLOAT4X4 world_viewrec;
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX ViewProj = XMLoadFloat4x4(&world_matrix) * view*proj;
	XMStoreFloat4x4(&world_viewrec, ViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	shader_test->set_material(test_Mt);
	material_list rec_texture;
	for (int i = 0; i < floor_need->get_geometry_num(); ++i)
	{
		floor_need->get_texture(&rec_texture, i);
		shader_test->set_diffusetex(rec_texture.tex_diffuse_resource);
		floor_need->draw_mesh_part(teque_need, i);
	}
}
void scene_engine_physicx::show_axe()
{
	if (now_weapon != player_weapon::weapon_axe)
	{
		return;
	}
	auto *floor_need = geometry_lib->get_assimp_ModelResourceView_by_name("stoneaxe_model_resource_view");
	//获取渲染路径
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");
	//设定世界变换
	XMFLOAT4X4 world_matrix;
	world_matrix = floor_need->get_world_matrix();
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMFLOAT4X4 world_viewrec;
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX ViewProj = XMLoadFloat4x4(&world_matrix) * view*proj;
	XMStoreFloat4x4(&world_viewrec, ViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	shader_test->set_material(test_Mt);
	material_list rec_texture;
	for (int i = 0; i < floor_need->get_geometry_num(); ++i)
	{
		floor_need->get_texture(&rec_texture, i);
		shader_test->set_diffusetex(rec_texture.tex_diffuse_resource);
		floor_need->draw_mesh_part(teque_need, i);
	}
}
HRESULT scene_engine_physicx::display()
{
	XMFLOAT4X4 view_proj;
	XMStoreFloat4x4(&view_proj, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	auto shader_deffered = shader_lib->get_shader_light_deffered_draw();
	shader_deffered->set_diffuse_light_tex(lbuffer_diffuse);
	shader_deffered->set_specular_light_tex(lbuffer_specular);

	terrain_test->show_terrain(view_proj);
	show_ball();
	//show_floor();
	show_box();
	show_axe();
	show_pick();
	//show_grass();
	world_map_main->display(view_matrix, proj_matrix);
	if (if_dead == false)
	{
		player_main->display(view_proj);
	}


	contex_pancy->OMSetDepthStencilState(NULL, 0);

	return S_OK;
}
HRESULT scene_engine_physicx::display_enviroment()
{
	return S_OK;
}
HRESULT scene_engine_physicx::display_nopost()
{
	renderstate_lib->restore_rendertarget();
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState *rec = renderstate_lib->get_blend_common();
	contex_pancy->OMSetBlendState(rec, blendFactor, 0xffffffff);
	if (if_call_died)
	{
		gui_list->display_button("ui_rebirth_game");
		gui_list->display_button("ui_end_game");
		if (if_dead == false)
		{
			gui_list->display_button("ui_return_game");
		}
	}
	gui_list->display_prograssbar("ui_HP_prograssbar");
	gui_list->display_prograssbar("ui_food_prograssbar");
	gui_list->display_prograssbar("ui_power_prograssbar");
	gui_list->display_common("ui_fast_inventory");
	player_package->display();
	//gui_list->display_common("ui_package");
	gui_list->display_mouse();

	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	//test_start->display();
	//prograss_test->display();
	//mouse_basic->display();

	return S_OK;
}
HRESULT scene_engine_physicx::release()
{
	//physics_test->release();
	terrain_test->release();
	player_main->release();
	gui_list->release();
	player_package->release();
	return S_OK;
}
HRESULT scene_engine_physicx::update(float delta_time)
{
	audio_engine->main_update();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新场景摄像机~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	set_camera_player(delta_time);
	/*
	if (user_input->check_keyboard(DIK_U))
	{
		player_main->set_speed(0.03f, 0.00);
		//player_main->add_offset(0.03f,0.00);
	}
	else if (user_input->check_keyboard(DIK_J))
	{
		player_main->set_speed(-0.03f, 0.00);
		//player_main->add_offset(-0.03f, 0.00);
	}
	else if (user_input->check_keyboard(DIK_H))
	{
		player_main->set_speed(0.00f, 0.03);
		//player_main->add_offset(0.00f, 0.03);
	}
	else if (user_input->check_keyboard(DIK_K))
	{
		player_main->set_speed(0.00f, -0.03);
		//player_main->add_offset(0.00f, -0.03);
	}
	else
	{
		player_main->set_speed(0.0, 0.0);
	}
	HRESULT hr = camera_move();
	if (hr != S_OK)
	{
		MessageBox(0, L"camera system has an error", L"tip", MB_OK);
		return hr;
	}
	*/
	XMFLOAT3 eyePos_rec;
	scene_camera->get_view_position(&eyePos_rec);
	auto* shader_ref = shader_lib->get_shader_reflect();
	shader_ref->set_view_pos(eyePos_rec);
	auto* shader_test = shader_lib->get_shader_prelight();
	shader_test->set_view_pos(eyePos_rec);
	auto* shader_grass = shader_lib->get_shader_grass_billboard();
	shader_grass->set_view_pos(eyePos_rec);
	auto* shader_deff = shader_lib->get_shader_light_deffered_draw();
	shader_deff->set_view_pos(eyePos_rec);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新场景信息~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//auto* floor_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_floor");
	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	//设定世界变换
	auto* box_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_boxtest");
	box_need->update_physx_worldmatrix(delta_time);

	XMFLOAT3 eyeDir_rec;
	scene_camera->get_view_direct(&eyeDir_rec);
	if (if_attack == true)
	{
		attack_time_save += delta_time*20.0f;
	}
	player_main->update(delta_time);

	XMFLOAT3 player_pos_rec;
	player_main->get_now_position(player_pos_rec.x, player_pos_rec.y, player_pos_rec.z);
	player_heart_now = 0.0f;
	world_map_main->update(delta_time, eyePos_rec, eyeDir_rec, perspective_angle, player_pos_rec, player_heart_now);
	if (player_heart_now > 0.5f)
	{
		audio_engine->play_sound_single(music_ID_hited, music_ID_hited);
		audio_engine->play_sound_single(music_ID_hearted, music_ID_hearted);
	}
	//设定装备变换
	auto* axe_need = geometry_lib->get_assimp_ModelResourceView_by_name("stoneaxe_model_resource_view");
	auto* player_need = geometry_lib->get_assimp_ModelResourceView_by_name("test_player_one");

	XMMATRIX mat_playerworld = XMLoadFloat4x4(&player_need->get_world_matrix());
	XMMATRIX mat_playerhand = XMLoadFloat4x4(&player_need->get_hand_matrix());
	if (now_weapon != player_weapon::weapon_axe)
	{
		scal_world = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	}
	else
	{
		scal_world = XMMatrixScaling(0.6f, 0.6f, 0.6f);
	}
	rotation_world = XMMatrixRotationY(XM_PIDIV2) * XMMatrixRotationZ(-XM_PIDIV2);
	trans_world = XMMatrixTranslation(-0.9f, -0.2f, -1.55f);
	rec_world = scal_world * rotation_world * trans_world * mat_playerhand * mat_playerworld;
	XMStoreFloat4x4(&world_matrix, rec_world);
	axe_need->update(world_matrix, delta_time);
	//设定装备变换
	auto* pick_need = geometry_lib->get_assimp_ModelResourceView_by_name("pick_model_resource_view");
	if (now_weapon != player_weapon::weapon_pick)
	{
		scal_world = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	}
	else
	{
		scal_world = XMMatrixScaling(0.6f, 0.6f, 0.6f);
	}
	rotation_world = XMMatrixRotationY(XM_PIDIV2) * XMMatrixRotationZ(-XM_PIDIV2);
	trans_world = XMMatrixTranslation(-0.9f, -0.2f, -1.55f);
	rec_world = scal_world * rotation_world * trans_world * mat_playerhand * mat_playerworld;
	XMStoreFloat4x4(&world_matrix, rec_world);
	pick_need->update(world_matrix, delta_time);
	//更新天空球世界变换
	trans_world = XMMatrixTranslation(eyePos_rec.x, eyePos_rec.y, eyePos_rec.z);
	scal_world = XMMatrixScaling(1000.0f, 1000.0f, 1000.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_buildin_GRV_byname("geometry_sky", world_matrix, delta_time);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~设置shadowmap光源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	light_list->update_and_setlight();
	XMFLOAT4X4 view_proj;
	XMStoreFloat4x4(&view_proj, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	time_game += delta_time*0.3;

	gui_list->update_mouse(delta_time);
	gui_list->update_button(delta_time, "ui_rebirth_game");
	gui_list->update_button(delta_time, "ui_end_game");
	gui_list->update_button(delta_time, "ui_return_game");
	gui_list->update_prograssbar(delta_time, "ui_HP_prograssbar");
	gui_list->update_prograssbar(delta_time, "ui_food_prograssbar");
	gui_list->update_prograssbar(delta_time, "ui_power_prograssbar");
	gui_list->update_common(delta_time, "ui_fast_inventory");
	player_package->update(delta_time);
	//gui_list->update_common(delta_time, "ui_package");
	gui_list->set_size_button("ui_rebirth_game", 0.2f, 0.12f, 0.0f);
	gui_list->set_position_button("ui_rebirth_game", 0, 0);
	gui_list->set_size_button("ui_end_game", 0.2f, 0.12f, 0.0f);
	gui_list->set_position_button("ui_end_game", 0, -0.12f);
	gui_list->set_size_button("ui_return_game", 0.2f, 0.12f, 0.0f);
	gui_list->set_position_button("ui_return_game", 0, -0.24f);
	if (player_HP < 11.0f)
	{
		if_dead = true;
		if_call_died = true;
	}
	else
	{
		if (if_dead == true)
		{
			if_call_died = false;
		}
		if_dead = false;
	}
	if (user_input->check_keyboard(DIK_ESCAPE))
	{
		if_call_died = true;
	}
	auto data_end = gui_list->get_button_byname("ui_end_game");
	if (if_call_died && data_end->check_if_under_mouse() && user_input->check_mouseDown(0))
	{
		if_close_window = true;
	}
	data_end = gui_list->get_button_byname("ui_return_game");
	if (if_dead == false && if_call_died && data_end->check_if_under_mouse() && user_input->check_mouseDown(0))
	{
		if_call_died = false;
	}
	data_end = gui_list->get_button_byname("ui_rebirth_game");
	if (if_dead == true && if_call_died && data_end->check_if_under_mouse() && user_input->check_mouseDown(0))
	{
		player_HP = 100.0f;
	}

	gui_list->set_size_common("ui_fast_inventory", 0.4f, 0.1f, 0.0f);
	gui_list->set_position_common("ui_fast_inventory", 0.0f, -0.95f);

	//gui_list->set_size_common("ui_package", 1.0f, 1.0f, 0.0f);
	//gui_list->set_position_common("ui_package", 0.0f, 0.0f);

	gui_list->set_size_prograssbar("ui_power_prograssbar", 0.2f, 0.1f, 0.0f);
	gui_list->set_position_prograssbar("ui_power_prograssbar", 0.8f, -0.7);
	gui_list->set_size_prograssbar("ui_HP_prograssbar", 0.2f, 0.1f, 0.0f);
	gui_list->set_position_prograssbar("ui_HP_prograssbar", 0.8f, -0.8);
	gui_list->set_size_prograssbar("ui_food_prograssbar", 0.2f, 0.1f, 0.0f);
	gui_list->set_position_prograssbar("ui_food_prograssbar", 0.8f, -0.9);

	//mouse_basic->update(delta_time);
	//test_start->update(delta_time);
	return S_OK;
}
void scene_engine_physicx::set_camera_player(float delta_time)
{
	float update_power = 8.0f * delta_time;
	float update_HP = -player_heart_now;
	float update_food = -0.2f * delta_time;
	user_input->get_input();
	assimpmodel_resource_view *data_assimp = geometry_lib->get_assimp_ModelResourceView_by_name(player_main->get_model_view_name());
	player_main->change_rotation_angle(user_input->MouseMove_X() * 0.001f);
	camera_height += user_input->MouseMove_Y() * 0.001f;
	if (camera_height > XM_PI / 4.0f)
	{
		camera_height = XM_PI / 4.0f;
	}
	if (camera_height < -XM_PI / 4.0f)
	{
		camera_height = -XM_PI / 4.0f;
	}
	//scene_camera->rotation_up(user_input->MouseMove_X() * 0.001f);
	//scene_camera->rotation_right(user_input->MouseMove_Y() * 0.001f);
	XMFLOAT3 rec_pos, player_pos;
	player_main->get_now_position(rec_pos.x, rec_pos.y, rec_pos.z);
	player_pos = rec_pos;
	XMFLOAT3 look_dir;
	player_main->get_look_position(look_dir.x, look_dir.y, look_dir.z);
	XMFLOAT3 up_dir = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//设置主角聆听位置
	audio_engine->set_lisener(rec_pos, look_dir, up_dir, XMFLOAT3(0.0f, 0.0f, 0.0f));
	audio_engine->set_music_place(1, rec_pos, XMFLOAT3(0.0f, 0.0f, 0.0f));
	XMFLOAT3 player_pos_change;
	player_pos_change.x = rec_pos.x - 40 * look_dir.x;
	player_pos_change.z = rec_pos.z - 40 * look_dir.z;
	player_pos_change.y = rec_pos.y + 20;
	XMFLOAT3 right_direction;
	scene_camera->get_right_direct(&right_direction);
	XMMATRIX matrix_rotation = XMMatrixRotationAxis(XMLoadFloat3(&right_direction), camera_height);
	XMVECTOR mid_ans_look = XMLoadFloat3(&look_dir);
	mid_ans_look = XMVector3TransformCoord(mid_ans_look, matrix_rotation);
	XMStoreFloat3(&look_dir, mid_ans_look);

	scene_camera->set_camera(look_dir, up_dir, player_pos_change);
	scene_camera->count_view_matrix(&view_matrix);

	audio_engine->pause_sound(music_ID_walk);
	if (!if_dead)
	{
		if (user_input->check_keyboard(DIK_F))
		{
			if_build = true;
		}
		if (if_build)
		{
			if (last_ID != 0)
			{
				world_map_main->delete_resource_instance_byID(last_ID);
			}
			XMFLOAT3 position_player = rec_pos;
			XMFLOAT3 position_check;
			//scene_camera->
			position_check.x = player_pos.x + look_dir.x * (50 + look_dir.y * 100);
			position_check.y = player_pos.y;
			position_check.z = player_pos.z + look_dir.z * (50 + look_dir.y * 100);
			int ID_save;
			world_map_main->find_resource_ID_byposition(position_check, ID_save);
			HRESULT hr = world_map_main->add_resource_instance_byname("square_model_resource_view", ID_save, 1.5f);
			if (!FAILED(hr))
			{
				last_ID = ID_save;
			}
			else
			{
				last_ID = 0;
			}
			if (user_input->check_mouseDown(0))
			{
				last_ID = 0;
				if_build = false;
			}
			if (user_input->check_mouseDown(1))
			{
				last_ID = 0;
				world_map_main->delete_resource_instance_byID(last_ID);
				if_build = false;
			}
		}
		else
		{
			//攻击性动作
			if (if_attack)
			{
				update_power = -3.0f * delta_time;
			}
			if (user_input->check_mouseDown(0) && if_attack == false && player_power > 10.0f)
			{
				audio_engine->play_sound_single(music_ID_wave, music_ID_wave);
				if_attack = true;
				float st_anim = 75.0f, ed_anim = 90.0f, speed_anim = 30.0f;
				data_assimp->reset_animation_time(75.0f);
				data_assimp->reset_animation_data(st_anim, ed_anim, speed_anim);
				player_main->set_speed(0.0f * look_dir.x, 0.0f* look_dir.z);
			}
			else if (if_attack == false)
			{
				if (user_input->check_keyboard(DIK_W))
				{
					audio_engine->start_sound(music_ID_walk);
					user_input->get_input();
					if (user_input->check_keyboard(DIK_LSHIFT) && player_power > 10.0f)
					{
						update_power = -3.0f * delta_time;
						float st_anim = 40.0f, ed_anim = 66.0f, speed_anim = 30.0f;
						data_assimp->reset_animation_data(st_anim, ed_anim, speed_anim);
						player_main->set_speed(0.89f * look_dir.x, 0.89f* look_dir.z);
					}
					else
					{
						update_power = 4.0f * delta_time;
						float st_anim = 0.0f, ed_anim = 28.0f, speed_anim = 20.0f;
						data_assimp->reset_animation_data(st_anim, ed_anim, speed_anim);
						player_main->set_speed(0.39f * look_dir.x, 0.39f* look_dir.z);
					}
				}
				else
				{
					player_main->set_speed(0.0, 0.0);
				}
			}
			else if (attack_time_save > 12.0f)
			{
				if_attack = false;
				attack_time_save = 0.0f;
				player_main->set_speed(0.0f, 0.0f);
				bool if_killed;
				static_resource_type type_need = static_resource_type::static_resource_tree;
				animal_type type_need1;
				XMFLOAT3 eyeDir_rec;
				scene_camera->get_view_direct(&eyeDir_rec);
				XMFLOAT3 eyePos_rec;
				scene_camera->get_view_position(&eyePos_rec);
				HRESULT hr_need = world_map_main->hit_the_animal_byposition(now_weapon, player_pos, eyeDir_rec, perspective_angle, 50, if_killed, type_need1);
				if (!FAILED(hr_need))
				{
					audio_engine->play_sound_single(music_ID_woodhit, music_ID_woodhit);
					if (if_killed == true)
					{
						player_package->add_goods_animal();
					}
				}
				else
				{
					HRESULT hr_need = world_map_main->hit_the_resource_byposition(now_weapon, player_pos, 50, if_killed, type_need);
					if (!FAILED(hr_need))
					{
						if (type_need == static_resource_type::static_resource_tree)
						{
							if (if_killed == true)
							{
								audio_engine->play_sound_single(music_ID_treedied, music_ID_treedied);
							}
							else
							{
								audio_engine->play_sound_single(music_ID_woodhit, music_ID_woodhit);
							}
						}
						else if (type_need == static_resource_type::static_resource_stone)
						{
							if (if_killed == true)
							{
								audio_engine->play_sound_single(music_ID_stone_died, music_ID_stone_died);
							}
							else
							{
								audio_engine->play_sound_single(music_ID_stone_hit, music_ID_stone_hit);
							}
						}

						if (if_killed == true)
						{
							player_package->add_goods_by_resource(type_need);
						}
					}
				}

				float st_anim = 0.0f, ed_anim = 28.0f, speed_anim = 20.0f;
				data_assimp->reset_animation_data(st_anim, ed_anim, speed_anim);
				player_main->set_speed(0.0f, 0.0f);
			}
		}



		if (user_input->check_keyboard(DIK_I))
		{
			if_click_bag = true;
		}
		else
		{
			if (if_click_bag == true)
			{
				if (player_package->check_if_open() == true)
				{
					player_package->close_package();
				}
				else
				{
					player_package->open_package();
				}
				if_click_bag = false;
			}

		}
		check_fast_inventory();
		if (if_use_fast == true)
		{
			if_use_fast = false;
			int count = player_package->get_fast_pocket(now_fast_choose - 1);
			if (count == 9)
			{
				now_weapon = player_weapon::weapon_axe;
			}
			else if (count == 10)
			{
				now_weapon = player_weapon::weapon_pick;
			}
			else
			{
				now_weapon = player_weapon::weapon_hand;
			}
			if (count == 0 && if_use_eat == false)
			{
				player_package->delete_goodsnum_byname("pumpkin");
				update_food = 10.0f;
			}
			if (count == 6 && if_use_eat == false)
			{
				player_package->delete_goodsnum_byname("meet");
				update_food = 20.0f;
			}
			if_use_eat = true;
		}
		else
		{
			if_use_eat = false;
		}
	}

	if (player_HP + update_HP > 0.0f && player_HP + update_HP < 100.0f)
	{
		player_HP += update_HP;
	}
	if (player_power + update_power > 0.0f && player_power + update_power < 100.0f)
	{
		player_power += update_power;
	}
	if (player_food + update_food > 0.0f && player_food + update_food < 100.0f)
	{
		player_food += update_food;
	}

	auto food_need = gui_list->get_prograssbar_byname("ui_food_prograssbar");
	food_need->change_now_percent(player_food / 100.0f);
	auto HP_need = gui_list->get_prograssbar_byname("ui_HP_prograssbar");
	HP_need->change_now_percent(player_HP / 100.0f);
	auto power_need = gui_list->get_prograssbar_byname("ui_power_prograssbar");
	power_need->change_now_percent(player_power / 100.0f);
	/*
	float move_speed = 0.75f;
	XMMATRIX view;
	//user_input->get_input();
	if (user_input->check_keyboard(DIK_A))
	{
		scene_camera->walk_right(-move_speed);
	}
	if (user_input->check_keyboard(DIK_W))
	{
		scene_camera->walk_front(move_speed);
	}
	if (user_input->check_keyboard(DIK_R))
	{
		scene_camera->walk_up(move_speed);
	}
	if (user_input->check_keyboard(DIK_D))
	{
		scene_camera->walk_right(move_speed);
	}
	if (user_input->check_keyboard(DIK_S))
	{
		scene_camera->walk_front(-move_speed);
	}
	if (user_input->check_keyboard(DIK_F))
	{
		scene_camera->walk_up(-move_speed);
	}
	if (user_input->check_keyboard(DIK_Q))
	{
		scene_camera->rotation_look(0.001f);
	}
	if (user_input->check_keyboard(DIK_E))
	{
		scene_camera->rotation_look(-0.001f);
	}
	if (user_input->check_mouseDown(1))
	{
		scene_camera->rotation_up(user_input->MouseMove_X() * 0.001f);
		scene_camera->rotation_right(user_input->MouseMove_Y() * 0.001f);
	}
	scene_camera->count_view_matrix(&view_matrix);
	*/
	//XMStoreFloat4x4(&view_matrix, view);
	//return S_OK;

}
void scene_engine_physicx::check_fast_inventory()
{
	if (user_input->check_keyboard(DIK_1))
	{
		if_use_fast = true;
		now_fast_choose = 1;
	}
	else if (user_input->check_keyboard(DIK_2))
	{
		if_use_fast = true;
		now_fast_choose = 2;
	}
	else if (user_input->check_keyboard(DIK_3))
	{
		if_use_fast = true;
		now_fast_choose = 3;
	}
	else if (user_input->check_keyboard(DIK_4))
	{
		if_use_fast = true;
		now_fast_choose = 4;
	}
	else if (user_input->check_keyboard(DIK_5))
	{
		if_use_fast = true;
		now_fast_choose = 5;
	}
	else if (user_input->check_keyboard(DIK_6))
	{
		if_use_fast = true;
		now_fast_choose = 6;
	}
	else if (user_input->check_keyboard(DIK_7))
	{
		if_use_fast = true;
		now_fast_choose = 7;
	}
	else if (user_input->check_keyboard(DIK_8))
	{
		if_use_fast = true;
		now_fast_choose = 8;
	}
}





scene_engine_UI::scene_engine_UI(FMOD_basic *audio_engine_need, ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height, float near_plane, float far_plane, float angle_view) : scene_root(device_need, contex_need, render_state, input_need, camera_need, lib_need, geometry_need, light_need, width, height, near_plane, far_plane, angle_view)
{
	//nonshadow_light_list.clear();
	//shadowmap_light_list.clear();
	audio_engine = audio_engine_need;
	rotate_angle = 0.0f;
	if_first = false;
	particle_fire = new particle_system<fire_point>(device_need, contex_need, 3000, lib_need, PARTICLE_TYPE_FIRE);
}
HRESULT scene_engine_UI::scene_create()
{

	//Sleep(5000);
	HRESULT hr_need;
	//光源注册
	light_list->set_sunlight(XMFLOAT3(0.0f, -1.0f, 2.0f), 0.75, 4, 1024, 1024);

	HRESULT hr = audio_engine->load_music("music\\openbgm.mp3", NULL, FMOD_LOOP_NORMAL, music_ID_bgm);
	hr = audio_engine->play_sound(music_ID_bgm, music_ID_bgm);
	audio_engine->pause_sound(music_ID_bgm);
	gui_list = new GUI_control(static_cast<float>(scene_window_width), static_cast<float>(scene_window_height), user_input, device_pancy, contex_pancy, shader_lib);
	hr_need = gui_list->create(L"UI\\mouse_move.dds", L"UI\\mouse_up.dds", L"UI\\mouse_down.dds");
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	gui_list->set_mouse_size(0.07f, 0.07f, 0.0f);



	gui_list->add_a_button(L"UI\\start.dds", "ui_start_game");
	gui_list->add_a_button(L"UI\\save.dds", "ui_save_game");
	gui_list->add_a_button(L"UI\\read.dds", "ui_read_game");
	gui_list->add_a_button(L"UI\\end.dds", "ui_end_game");
	gui_list->add_a_common(L"UI\\gameface1.dds", "ui_face");
	gui_list->add_a_common(L"UI\\loading.dds", "ui_loading");
	//gui_list->add_a_prograssbar(L"UI\\water.dds", "ui_food_prograssbar");
	//gui_list->add_a_prograssbar(L"UI\\HP.dds", "ui_HP_prograssbar");
	//gui_list->add_a_common(L"UI\\fast_inventory.dds", "ui_fast_inventory");
	//外部模型导入及创建访问器
	hr_need = particle_fire->create(L"flare0.dds");
	if (hr_need != S_OK)
	{
		MessageBox(0, L"load fire particle error", L"tip", MB_OK);
		return hr_need;
	}
	int index_model_rec;
	//地形
	hr_need = geometry_lib->load_modelresource_from_file("UI_terrain\\terrain.obj", "UI_terrain\\", false, false, false, 0, NULL, "terrain_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "model_terrain");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	//火堆
	hr_need = geometry_lib->load_modelresource_from_file("fireplace\\fireplace.obj", "fireplace\\", false, false, false, 0, NULL, "fireplace_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "model_fireplace");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	//树木
	hr_need = geometry_lib->load_modelresource_from_file("UI_tree\\EuropeanLinden_Med.fbx", "UI_tree\\", false, false, false, 0, NULL, "tree_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "model_tree");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "model_tree2");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	//文字
	hr_need = geometry_lib->load_modelresource_from_file("text\\text.obj", "text\\", false, false, false, 0, NULL, "text_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "model_text");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	//房屋
	hr_need = geometry_lib->load_modelresource_from_file("house\\house.obj", "house\\", false, false, false, 0, NULL, "castel_model_resource", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "model_castel");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	int alpha_yuri[] = { 0,1,2,3 };
	hr_need = geometry_lib->load_modelresource_from_file("player_main\\player_sit.FBX", "player_main\\", true, false, false, 0, NULL, "yuri_model_resource1", index_model_rec);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	hr_need = geometry_lib->add_assimp_modelview_by_index(index_model_rec, "model_yuri");
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model error", L"tip", MB_OK);
		return hr_need;
	}
	//内置模型访问器
	hr_need = geometry_lib->add_buildin_modelview_by_name("geometry_ball", "geometry_sky", pancy_geometry_ball);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"add buildin model error", L"tip", MB_OK);
		return hr_need;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~纹理注册~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int texture_index;
	hr_need = geometry_lib->load_texture_from_file(L"floor.dds", true, "floor_diffuse", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"floor_n.dds", true, "floor_normal", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"Texture_cube.dds", true, "sky_cube", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"RoughBlades.dds", true, "grass_diffuse", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"RoughBlades_Normal.dds", true, "grass_normal", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	hr_need = geometry_lib->load_texture_from_file(L"RoughBlades_Spec.dds", true, "grass_specular", texture_index);
	if (FAILED(hr_need))
	{
		return hr_need;
	}
	audio_engine->start_sound(music_ID_bgm);
	return S_OK;

}
HRESULT scene_engine_UI::display()
{
	XMFLOAT4X4 view_proj;
	XMStoreFloat4x4(&view_proj, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	contex_pancy->RSSetState(NULL);
	show_ball();
	show_floor();
	show_tree("model_tree");
	show_tree("model_tree2");
	show_tree("model_text");
	show_fire_place();
	show_castel_deffered("LightTech");
	show_yuri_animation_deffered();
	//show_yuri_animation();
	//show_billboard();
	//清空深度模板缓冲，在AO绘制阶段记录下深度信息
	//show_fire_particle();
	return S_OK;
}
HRESULT scene_engine_UI::display_enviroment()
{
	show_ball();
	show_floor();
	show_castel_deffered("LightTech");
	return S_OK;
}
HRESULT scene_engine_UI::display_nopost()
{
	//renderstate_lib->restore_rendertarget(ssao_part->get_depthstencilmap());
	renderstate_lib->restore_rendertarget();
	show_fire_particle();
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11BlendState *rec = renderstate_lib->get_blend_tocoverage();
	contex_pancy->OMSetBlendState(rec, blendFactor, 0xffffffff);
	if (check_if_stop())
	{
		gui_list->display_common("ui_loading");
		gui_list->display_common("ui_face");
	}
	else
	{
		gui_list->display_mouse();
		gui_list->display_button("ui_start_game");
		gui_list->display_button("ui_save_game");
		gui_list->display_button("ui_read_game");
		gui_list->display_button("ui_end_game");
	}
	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);

	return S_OK;
}
void scene_engine_UI::show_yuri_animation()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//几何体属性
	auto* model_yuri_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_yuri");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need, *teque_normal, *teque_hair;
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

	shader_test->get_technique(rec_point, num_member, &teque_need, "drawskin_withshadowssao");
	shader_test->get_technique(rec_point, num_member, &teque_normal, "drawskin_withshadowssaonormal");
	shader_test->get_technique(rec_point, num_member, &teque_hair, "drawskin_hair");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	world_matrix = model_yuri_pack->get_world_matrix();
	rec_world = XMLoadFloat4x4(&world_matrix);
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//获取渲染路径并渲染
	int yuri_render_order[11] = { 4,5,6,7,8,9,10,3,0,2,1 };
	XMFLOAT4X4 *rec_bonematrix = model_yuri_pack->get_bone_matrix();
	shader_test->set_bone_matrix(rec_bonematrix, model_yuri_pack->get_bone_num());
	//alpha混合设定
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//绘制头发
	material_list rec_need;
	model_yuri_pack->get_texture(&rec_need, yuri_render_order[7]);
	shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
	shader_test->set_normaltex(rec_need.texture_normal_resource);
	model_yuri_pack->draw_mesh_part(teque_hair, yuri_render_order[7]);
	contex_pancy->OMSetDepthStencilState(NULL, 0);
	contex_pancy->OMSetBlendState(0, blendFactor, 0xffffffff);
	contex_pancy->OMSetBlendState(NULL, blendFactor, 0xffffffff);
}
void scene_engine_UI::show_yuri_animation_deffered()
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的属性
	auto* model_yuri_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_yuri");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
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
	shader_test->get_technique(rec_point, num_member, &teque_need, "LightWithBone");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(0.4f, 0.4f, 0.4f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	shader_test->set_material(test_Mt);
	//设定世界变换
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	world_matrix = model_yuri_pack->get_world_matrix();
	rec_world = XMLoadFloat4x4(&world_matrix);
	XMStoreFloat4x4(&world_matrix, rec_world);
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//设定ssao变换及贴图
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	//获取渲染路径并渲染
	XMFLOAT4X4 *rec_bonematrix = model_yuri_pack->get_bone_matrix();
	shader_test->set_bone_matrix(rec_bonematrix, model_yuri_pack->get_bone_num());
	for (int i = 0; i < model_yuri_pack->get_geometry_num(); ++i)
	{
		material_list rec_need;
		model_yuri_pack->get_texture(&rec_need, i);
		//model_yuri->get_texture(&rec_need, yuri_render_order[i]);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_yuri_pack->draw_mesh_part(teque_need, i);
		//model_yuri->get_technique(teque_need);
		//model_yuri->draw_part(yuri_render_order[i]);
	}
	shader_test->set_diffuse_light_tex(NULL);
	shader_test->set_specular_light_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void scene_engine_UI::show_castel(LPCSTR techname, LPCSTR technamenormal)
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_castel");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need, *teque_normal;
	shader_test->get_technique(&teque_need, techname);
	shader_test->get_technique(&teque_normal, technamenormal);
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//设定世界变换
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	shader_test->set_trans_world(&world_matrix);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	//alpha混合设定
	for (int i = 0; i < model_castel_pack->get_geometry_num(); ++i)
	{
		//纹理设定
		material_list rec_need;
		model_castel_pack->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		if (rec_need.texture_normal_resource != NULL)
		{
			shader_test->set_normaltex(rec_need.texture_normal_resource);
			model_castel_pack->draw_mesh_part(teque_normal, i);
		}
		else
		{
			model_castel_pack->draw_mesh_part(teque_need, i);
		}
	}
}
void scene_engine_UI::show_castel_deffered(LPCSTR techname)
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_castel");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, techname);
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	shader_test->set_material(test_Mt);


	//设定世界变换
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	shader_test->set_trans_world(&world_matrix);
	for (int i = 0; i < model_castel_pack->get_geometry_num(); ++i)
	{
		//纹理设定
		material_list rec_need;
		model_castel_pack->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_castel_pack->draw_mesh_part(teque_need, i);
	}

	shader_test->set_diffuse_light_tex(NULL);
	shader_test->set_specular_light_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}

	contex_pancy->RSSetState(NULL);
}


void scene_engine_UI::show_tree(string name)
{

	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_assimp_ModelResourceView_by_name(name);
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	shader_test->set_material(test_Mt);


	//设定世界变换
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	shader_test->set_trans_world(&world_matrix);
	for (int i = 0; i < model_castel_pack->get_geometry_num(); ++i)
	{
		//纹理设定
		material_list rec_need;
		model_castel_pack->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_castel_pack->draw_mesh_part(teque_need, i);
	}

	shader_test->set_diffuse_light_tex(NULL);
	shader_test->set_specular_light_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}

	contex_pancy->RSSetState(NULL);
}
void scene_engine_UI::show_ball()
{
	contex_pancy->RSSetState(renderstate_lib->get_CULL_front_rs());
	auto* shader_test = shader_lib->get_shader_reflect();
	auto* ball_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_sky");
	//auto* tex_skycube = geometry_lib->get_sky_cube_tex();
	auto* tex_skycube = geometry_lib->get_texture_byname("sky_cube")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_reflect");

	/*
	//设定世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(50.0f, 50.0f, 50.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	*/
	shader_test->set_trans_world(&ball_need->get_world_matrix());
	//设定立方贴图
	shader_test->set_tex_resource(tex_skycube);
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&ball_need->get_world_matrix());
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	ball_need->draw_full_geometry(teque_need);
	contex_pancy->RSSetState(NULL);
}
void scene_engine_UI::show_floor()
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_terrain");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	shader_test->set_material(test_Mt);


	//设定世界变换
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	shader_test->set_trans_world(&world_matrix);
	for (int i = 0; i < model_castel_pack->get_geometry_num(); ++i)
	{
		//纹理设定
		material_list rec_need;
		model_castel_pack->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_castel_pack->draw_mesh_part(teque_need, i);
	}

	shader_test->set_diffuse_light_tex(NULL);
	shader_test->set_specular_light_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}

	contex_pancy->RSSetState(NULL);
	/*
	auto* shader_test = shader_lib->get_shader_prelight();
	auto* floor_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_floor");
	//auto* tex_floor = geometry_lib->get_basic_floor_tex();
	//auto* tex_normal = geometry_lib->get_floor_normal_tex();
	auto* tex_floor = geometry_lib->get_texture_byname("floor_diffuse")->data->get_data();
	auto* tex_normal = geometry_lib->get_texture_byname("floor_normal")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_withshadownormal");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f, 1.0f, 1.0f, 12.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_test->set_material(test_Mt);
	//纹理设定
	shader_test->set_diffusetex(tex_floor);
	shader_test->set_normaltex(tex_normal);
	shader_test->set_trans_world(&floor_need->get_world_matrix());
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&floor_need->get_world_matrix());

	XMMATRIX worldViewProj = world_matrix_rec*view*proj;

	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	floor_need->draw_full_geometry(teque_need);
	*/
}
void scene_engine_UI::show_fire_particle()
{
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMFLOAT3 st_pos = XMFLOAT3(6.2, 1.5, -4.0);
	XMFLOAT3 st_dir = XMFLOAT3(0.0f, 1.0f, 0.0f);
	particle_fire->set_particle_direct(&st_pos, &st_dir);
	particle_fire->draw_particle();
	contex_pancy->RSSetState(0);
	contex_pancy->OMSetDepthStencilState(0, 0);
	contex_pancy->OMSetBlendState(0, blendFactor, 0xffffffff);
}
void scene_engine_UI::show_billboard()
{
	contex_pancy->RSSetState(renderstate_lib->get_CULL_none_rs());
	auto* shader_test = shader_lib->get_shader_grass_billboard();
	auto* floor_need = geometry_lib->get_grass_common();
	shader_test->set_texture_diffuse(geometry_lib->get_texture_byname("grass_diffuse")->data->get_data());
	shader_test->set_texture_normal(geometry_lib->get_texture_byname("grass_normal")->data->get_data());
	shader_test->set_texture_specular(geometry_lib->get_texture_byname("grass_specular")->data->get_data());
	XMFLOAT4X4 rec_mat;
	XMStoreFloat4x4(&rec_mat, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	shader_test->set_trans_all(&rec_mat);
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_with_tex");
	floor_need->get_teque(teque_need);
	floor_need->show_mesh();
	contex_pancy->RSSetState(NULL);
	ID3D11RenderTargetView* NULL_target[1] = { NULL };
	contex_pancy->OMSetRenderTargets(0, NULL_target, 0);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}
}
void scene_engine_UI::show_fire_place()
{
	auto* shader_test = shader_lib->get_shader_light_deffered_draw();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_fireplace");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "LightTech");
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 6.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	test_Mt.reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	shader_test->set_material(test_Mt);


	//设定世界变换
	std::vector<light_with_shadowmap> shadowmap_light_list;
	shadowmap_light_list = *light_list->get_lightdata_shadow();
	XMFLOAT4X4 world_matrix = model_castel_pack->get_world_matrix();
	XMMATRIX rec_world = XMLoadFloat4x4(&model_castel_pack->get_world_matrix());
	//设定总变换
	XMMATRIX view = XMLoadFloat4x4(&view_matrix);
	XMMATRIX proj = XMLoadFloat4x4(&proj_matrix);
	XMMATRIX world_matrix_rec = XMLoadFloat4x4(&world_matrix);
	XMMATRIX worldViewProj = world_matrix_rec*view*proj;
	XMFLOAT4X4 world_viewrec;
	XMStoreFloat4x4(&world_viewrec, worldViewProj);
	shader_test->set_trans_all(&world_viewrec);
	shader_test->set_diffuse_light_tex(lbuffer_diffuse);
	shader_test->set_specular_light_tex(lbuffer_specular);
	shader_test->set_trans_world(&world_matrix);
	for (int i = 0; i < model_castel_pack->get_geometry_num(); ++i)
	{
		//纹理设定
		material_list rec_need;
		model_castel_pack->get_texture(&rec_need, i);
		shader_test->set_diffusetex(rec_need.tex_diffuse_resource);
		model_castel_pack->draw_mesh_part(teque_need, i);
	}

	shader_test->set_diffuse_light_tex(NULL);
	shader_test->set_specular_light_tex(NULL);
	D3DX11_TECHNIQUE_DESC techDesc;
	teque_need->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		teque_need->GetPassByIndex(p)->Apply(0, contex_pancy);
	}

	contex_pancy->RSSetState(NULL);
}
/*
camera_right = {x=-0.981215835 y=0.0366602764 z=0.189397946 }
camera_look = {x=-0.180782139 y=0.167918935 z=-0.969082594 }
camera_up = {x=0.0673303306 y=0.985118926 z=0.158137187 }
camera_position = { x = 9.13656616 y = 2.49802685 z = 8.78969383 }
*/
HRESULT scene_engine_UI::update(float delta_time)
{
	gui_list->update_mouse(delta_time);
	//四个按钮
	gui_list->update_button(delta_time, "ui_start_game");
	gui_list->set_size_button("ui_start_game", 0.2f, 0.12f, 0.0f);
	gui_list->set_position_button("ui_start_game", 0, 0);
	auto data_start = gui_list->get_button_byname("ui_start_game");
	if (data_start->check_if_under_mouse() && user_input->check_mouseDown(0))
	{
		audio_engine->pause_sound(music_ID_bgm);
		audio_engine->set_sound_priority(music_ID_bgm, 255);
		if_stop = true;
	}

	gui_list->update_button(delta_time, "ui_save_game");
	gui_list->set_size_button("ui_save_game", 0.2f, 0.12f, 0.0f);
	gui_list->set_position_button("ui_save_game", 0, -0.12f);

	gui_list->update_button(delta_time, "ui_read_game");
	gui_list->set_size_button("ui_read_game", 0.2f, 0.12f, 0.0f);
	gui_list->set_position_button("ui_read_game", 0, -0.24f);

	gui_list->update_button(delta_time, "ui_end_game");
	gui_list->set_size_button("ui_end_game", 0.2f, 0.12f, 0.0f);
	gui_list->set_position_button("ui_end_game", 0, -0.36f);
	data_start = gui_list->get_button_byname("ui_end_game");
	if (data_start->check_if_under_mouse() && user_input->check_mouseDown(0))
	{
		if_close_window = true;
	}

	//切屏原画
	gui_list->update_common(delta_time, "ui_face");
	gui_list->set_size_common("ui_face", 1.0f, 1.0f, 0.0f);
	gui_list->set_position_common("ui_face", 0, 0);

	if (window_width == 800)
	{
		gui_list->update_common(delta_time, "ui_loading");
		gui_list->set_size_common("ui_loading", 0.2f, 0.2f, 0.0f);
		gui_list->set_position_common("ui_loading", 0.2, 0.2);
	}
	else
	{
		gui_list->update_common(delta_time, "ui_loading");
		gui_list->set_size_common("ui_loading", 0.1f, 0.1f, 0.0f);
		gui_list->set_position_common("ui_loading", 0.2, 0.2);
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新场景摄像机~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HRESULT hr = camera_move();
	if (hr != S_OK)
	{
		MessageBox(0, L"camera system has an error", L"tip", MB_OK);
		return hr;
	}
	XMFLOAT3 camera_look = XMFLOAT3(-0.180782139, 0.167918935, -0.969082594);
	XMFLOAT3 camera_up = XMFLOAT3(0.0673303306, 0.985118926, 0.158137187);
	XMFLOAT3 camera_position = XMFLOAT3(9.13656616, 2.49802685, 8.78969383);
	scene_camera->set_camera(camera_look, camera_up, camera_position);
	XMFLOAT3 eyePos_rec;
	scene_camera->get_view_position(&eyePos_rec);
	auto* shader_ref = shader_lib->get_shader_reflect();
	shader_ref->set_view_pos(eyePos_rec);
	auto* shader_test = shader_lib->get_shader_prelight();
	shader_test->set_view_pos(eyePos_rec);
	auto* shader_grass = shader_lib->get_shader_grass_billboard();
	shader_grass->set_view_pos(eyePos_rec);
	auto* shader_deff = shader_lib->get_shader_light_deffered_draw();
	shader_deff->set_view_pos(eyePos_rec);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~更新几何体世界变换~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//auto* model_list = geometry_lib->get_model_list();
	//更新yuri世界变换
	XMMATRIX trans_world;
	XMMATRIX scal_world;
	XMMATRIX rotation_world;
	XMMATRIX rec_world;
	XMFLOAT4X4 world_matrix;
	trans_world = XMMatrixTranslation(7.5, 0.8, -2.0);
	scal_world = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	rotation_world = XMMatrixRotationX(XM_PIDIV2)*XMMatrixRotationY(XM_PIDIV4);
	rec_world = scal_world * rotation_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_assimp_MRV_byname("model_yuri", world_matrix, delta_time);
	geometry_lib->update_assimp_MRV_byname("model_yuri_trans", world_matrix, delta_time);
	//更新castel世界变换
	trans_world = XMMatrixTranslation(12.0, -0.5f, 0.0);
	scal_world = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_assimp_MRV_byname("model_castel", world_matrix, delta_time);
	//更新terrain世界变换
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	scal_world = XMMatrixScaling(100.0f, 50.0f, 100.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_assimp_MRV_byname("model_terrain", world_matrix, delta_time);
	//更新火堆世界变换
	trans_world = XMMatrixTranslation(6.0, 1.2, -4.0);
	//rotation_world = XMMatrixRotationZ(-XM_PI / 12.0f);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rec_world = scal_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_assimp_MRV_byname("model_fireplace", world_matrix, delta_time);
	//更新树木世界变换
	trans_world = XMMatrixTranslation(16.0, 1.2, -14.0);
	rotation_world = XMMatrixRotationX(XM_PIDIV2);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rec_world = scal_world * rotation_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_assimp_MRV_byname("model_tree", world_matrix, delta_time);
	//更新树木世界变换
	trans_world = XMMatrixTranslation(-5.0, 0.2, -14.0);
	rotation_world = XMMatrixRotationX(XM_PIDIV2);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rec_world = scal_world * rotation_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_assimp_MRV_byname("model_tree2", world_matrix, delta_time);
	//更新文字世界变换
	rotate_angle += delta_time;
	if (rotate_angle > XM_2PI)
	{
		rotate_angle -= XM_2PI;
	}
	trans_world = XMMatrixTranslation(6.5, 5.8, 0.00);
	rotation_world = XMMatrixRotationY(XM_PI);
	scal_world = XMMatrixScaling(0.04f, 0.04f, 0.04f);
	rec_world = scal_world * rotation_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_assimp_MRV_byname("model_text", world_matrix, delta_time);
	//更新光源世界变换
	trans_world = XMMatrixTranslation(0.0, 2.5, 2.5);
	scal_world = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	//更新天空球世界变换
	trans_world = XMMatrixTranslation(0.0, 0.0, 0.0);
	rotation_world = XMMatrixRotationY(-2 * XM_PI / 10);
	scal_world = XMMatrixScaling(50.0f, 50.0f, 50.0f);
	rec_world = scal_world * rotation_world * trans_world;
	XMStoreFloat4x4(&world_matrix, rec_world);
	geometry_lib->update_buildin_GRV_byname("geometry_sky", world_matrix, delta_time);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~设置shadowmap光源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	light_list->update_and_setlight();
	/*
	int count = 0;
	for (auto rec_shadow_light = shadowmap_light_list.begin(); rec_shadow_light != shadowmap_light_list.end(); ++rec_shadow_light)
	{
	rec_shadow_light._Ptr->set_frontlight(count);
	rec_shadow_light._Ptr->set_defferedlight(count);
	count += 1;
	}
	//设置无影光源
	for (auto rec_non_light = nonshadow_light_list.begin(); rec_non_light != nonshadow_light_list.end(); ++rec_non_light)
	{
	rec_non_light._Ptr->set_frontlight(count);
	rec_non_light._Ptr->set_defferedlight(count);
	count += 1;
	}
	//设置shadowvolume光源
	for (auto rec_shadow_volume = shadowvalume_light_list.begin(); rec_shadow_volume != shadowvalume_light_list.end(); ++rec_shadow_volume)
	{
	//rec_shadow_volume._Ptr->set_frontlight(count);
	//rec_shadow_volume._Ptr->set_defferedlight(count);
	//count++;
	//rec_shadow_volume._Ptr->update_view_proj_matrix(view_proj);
	}*/
	XMFLOAT4X4 view_proj;
	XMStoreFloat4x4(&view_proj, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	time_game += delta_time*0.3;
	particle_fire->update(delta_time*0.3, time_game, &view_proj, &eyePos_rec);
	return S_OK;
}
HRESULT scene_engine_UI::release()
{
	geometry_lib->delete_BuiltIn_modelview_by_name("geometry_sky");
	geometry_lib->delete_assimp_modelview_by_name("model_yuri");
	geometry_lib->delete_assimp_modelview_by_name("model_terrain");
	geometry_lib->delete_assimp_modelview_by_name("model_fireplace");
	geometry_lib->delete_assimp_modelview_by_name("model_tree");
	geometry_lib->delete_assimp_modelview_by_name("model_tree2");
	geometry_lib->delete_assimp_modelview_by_name("model_castel");
	geometry_lib->delete_assimp_modelview_by_name("model_text");
	particle_fire->release();
	gui_list->release();
	return S_OK;
}