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
	for (UINT i = 0; i<techDesc.Passes; ++i)
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

snake_draw::snake_draw(ID3D11Device *device_need, ID3D11DeviceContext *contex_need,int max_length_need, int devide_num_need)
{
	snake_length = 0;
	snake_radius = 1;
	max_snake_length = max_length_need;
	devide_num = devide_num_need;
	device_pancy = device_need;
	contex_pancy = contex_need;
	UAV_controlpoint_first = NULL;
	SRV_controlpoint_first = NULL;
	UAV_controlpoint_second = NULL;
	SRV_controlpoint_second = NULL;
	UAV_draw_point_bufeer = NULL;
	first_CSshader = NULL;
	second_TLshader = NULL;
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
	snake_length = 2;
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
		index_need[i+1] = (i / 6) * 4 + 1;
		index_need[i+2] = (i / 6) * 4 + 2;
		index_need[i+3] = (i / 6) * 4 + 0;
		index_need[i+4] = (i / 6) * 4 + 2;
		index_need[i+5] = (i / 6) * 4 + 3;
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
	first_CSshader = new shader_snakecompute(L"F:\\Microsoft Visual Studio\\pancystar_engine\\pancystar_engine_d3dx11_test\\Debug\\snake_compute.cso", device_pancy, contex_pancy);
	HRESULT hr = first_CSshader->shder_create();
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
	return S_OK;
}
void snake_draw::draw(XMFLOAT4X4 view_projmat) 
{
	//XMVECTOR check = XMLoadFloat3(&XMFLOAT3(-0.727,0.485,0.000));
	//XMMATRIX rec =  XMMatrixRotationAxis(check,1.0643);
	first_CSshader->set_input_buffer(SRV_controlpoint_first);
	first_CSshader->set_output_buffer(UAV_draw_point_bufeer,UAV_controlpoint_second);
	first_CSshader->set_piccturerange(snake_length, devide_num, snake_radius,0);
	first_CSshader->dispatch(snake_length, devide_num);
	//contex_pancy->CopyResource(point_buffer_render, point_buffer_UAV);
	/*
	contex_pancy->CopyResource(CPU_read_buffer, point_buffer_UAV);
	D3D11_MAPPED_SUBRESOURCE mappedTex2D;
	HRESULT hr;
	D3D11_BUFFER_DESC rec_now;
	CPU_read_buffer->GetDesc(&rec_now);
	hr = contex_pancy->Map(CPU_read_buffer, 0, D3D11_MAP_READ, 0, &mappedTex2D);
	point_snake rec_need[100];
	memcpy(rec_need, mappedTex2D.pData, 100 * sizeof(point_snake));
	//point_snake* rec = static_cast<point_snake*>(mappedTex2D.pData);
	contex_pancy->Unmap(CPU_read_buffer, 0);
	*/
	
	second_TLshader->set_trans_all(&view_projmat);
	UINT stride = sizeof(point_snake);
	UINT offset = 0;
	/*
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contex_pancy->IASetVertexBuffers(0, 1, &point_buffer_UAV, &stride, &offset);
	contex_pancy->IASetIndexBuffer(index_buffer_render, DXGI_FORMAT_R16_UINT, 0);
	ID3DX11EffectTechnique* tech;
	second_TLshader->get_technique(&tech, "draw_snake");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		int buffer_length = snake_length * devide_num * 24;
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->DrawIndexed(buffer_length, 0, 0);
	}
	*/
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	contex_pancy->IASetVertexBuffers(0, 1, &point_buffer_UAV, &stride, &offset);
	ID3DX11EffectTechnique* tech;
	second_TLshader->get_technique(&tech, "draw_snake");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		int buffer_length = snake_length * devide_num * 16;
		//int buffer_length = 4;
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->Draw(buffer_length, 0);
	}
	contex_pancy->HSSetShader(NULL, 0, 0);
	contex_pancy->DSSetShader(NULL, 0, 0);
}
void snake_draw::draw_pass(shader_control * shader_lib)
{
}
void snake_draw::update() 
{
}



scene_root::scene_root(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need,light_control *light_need,int width, int height)
{
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
	XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, scene_window_width*1.0f / scene_window_height*1.0f, 0.1f, 300.f);
	//ssao_part = new ssao_pancy(render_state,device_need, contex_need, shader_lib,geometry_lib,scene_window_width, scene_window_height, XM_PI*0.25f, 300.0f);
	XMStoreFloat4x4(&proj_matrix, proj);
	XMMATRIX iden = XMMatrixIdentity();
	XMStoreFloat4x4(&view_matrix, iden);
}
HRESULT scene_root::camera_move()
{
	XMMATRIX view;
	user_input->get_input();
	if (user_input->check_keyboard(DIK_A))
	{
		scene_camera->walk_right(-0.01f);
	}
	if (user_input->check_keyboard(DIK_W))
	{
		scene_camera->walk_front(0.01f);
	}
	if (user_input->check_keyboard(DIK_R))
	{
		scene_camera->walk_up(0.01f);
	}
	if (user_input->check_keyboard(DIK_D))
	{
		scene_camera->walk_right(0.01f);
	}
	if (user_input->check_keyboard(DIK_S))
	{
		scene_camera->walk_front(-0.01f);
	}
	if (user_input->check_keyboard(DIK_F))
	{
		scene_camera->walk_up(-0.01f);
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
	XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PI*0.25f, scene_window_width*1.0f / scene_window_height*1.0f, 0.1f, 300.f);
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


scene_engine_test::scene_engine_test(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height) : scene_root(device_need, contex_need, render_state, input_need, camera_need, lib_need,geometry_need, light_need,width, height)
{
	//nonshadow_light_list.clear();
	//shadowmap_light_list.clear();
	particle_fire = new particle_system<fire_point>(device_need, contex_need, 3000, lib_need, PARTICLE_TYPE_FIRE);
	test_snake = new snake_draw(device_need, contex_need,1000,100);
}
HRESULT scene_engine_test::scene_create()
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
	contex_pancy->RSSetState(renderstate_lib->get_CULL_none_rs());
	test_snake->draw(view_proj);
	contex_pancy->RSSetState(NULL);
	show_ball();
	show_lightsource();
	show_floor();
	show_castel_deffered("LightTech");
	//show_castel("draw_withshadowssao", "draw_withshadowssaonormal");
	//show_aotestproj();
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
	//show_ball();
	show_lightsource();
	show_floor();
	show_castel("draw_withtexture", "draw_withtexturenormal");
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

	shader_test->get_technique(rec_point, num_member ,&teque_need, "drawskin_withshadowssao");
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
	model_yuri_pack->draw_mesh_part(teque_hair,yuri_render_order[7]);
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
	for (int i = 8; i <  model_yuri_pack->get_geometry_num(); ++i)
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
void scene_engine_test::show_castel(LPCSTR techname,LPCSTR technamenormal)
{
	auto* shader_test = shader_lib->get_shader_prelight();
	//几何体的打包(动画)属性
	auto* model_castel_pack = geometry_lib->get_assimp_ModelResourceView_by_name("model_castel");
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need,*teque_normal;
	shader_test->get_technique(&teque_need, techname);
	shader_test->get_technique(&teque_normal, technamenormal);
	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(1.0f,1.0f, 1.0f, 6.0f);
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
			model_castel_pack->draw_mesh_part(teque_normal,i);
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
		model_castel_pack->draw_mesh_part(teque_need,i);
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
void scene_engine_test::show_aotestproj()
{
	auto* shader_test = shader_lib->get_shader_prelight();
	auto* floor_need = geometry_lib->get_buildin_GeometryResourceView_by_name("geometry_floor");
	//auto* tex_floor = geometry_lib->get_basic_floor_tex();
	//auto* tex_normal = geometry_lib->get_floor_normal_tex();
	auto* tex_floor = geometry_lib->get_texture_byname("floor_diffuse")->data->get_data();
	auto* tex_normal = geometry_lib->get_texture_byname("floor_normal")->data->get_data();
	//选定绘制路径
	ID3DX11EffectTechnique *teque_need;
	shader_test->get_technique(&teque_need, "draw_withshadowssao");

	//地面的材质
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_diffuse2(0.6f, 0.6f, 0.6f, 1.0f);
	XMFLOAT4 rec_specular2(0.0f, 0.0f, 0.0f, 12.0f);
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
	trans_world = XMMatrixTranslation(0.0f, 0.0f, -1.1f);
	scal_world = XMMatrixScaling(1.0f, 1.0f, 1.0f);

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
	XMStoreFloat4x4(&rec_mat,XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
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
	particle_fire->update(delta_time*0.3, time_game,&view_proj,&eyePos_rec);
	return S_OK;
}
HRESULT scene_engine_test::release()
{
	test_snake->release();
	particle_fire->release();
	return S_OK;
}

/*
scene_engine_snake::scene_engine_snake(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, pancy_renderstate *render_state, pancy_input *input_need, pancy_camera *camera_need, shader_control *lib_need, geometry_control *geometry_need, light_control *light_need, int width, int height) : scene_root(device_need, contex_need, render_state, input_need, camera_need, lib_need, geometry_need, light_need, width, height)
{
}
HRESULT scene_engine_snake::scene_create()
{
	return S_OK;
}
HRESULT scene_engine_snake::display()
{
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
	return S_OK;
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
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~设置shadowmap光源~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	light_list->update_and_setlight();
	XMFLOAT4X4 view_proj;
	XMStoreFloat4x4(&view_proj, XMLoadFloat4x4(&view_matrix) * XMLoadFloat4x4(&proj_matrix));
	time_game += delta_time*0.3;
	particle_fire->update(delta_time*0.3, time_game, &view_proj, &eyePos_rec);
	return S_OK;
}
*/