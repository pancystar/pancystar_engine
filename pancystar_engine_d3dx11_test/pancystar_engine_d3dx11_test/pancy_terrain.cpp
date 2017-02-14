#include"pancy_terrain.h"
pancy_terrain_build::pancy_terrain_build(pancy_physx* physic_need,ID3D11Device *device_need, ID3D11DeviceContext *contex_need, shader_control *shader_need, int map_number, int map_devide, float map_range, std::vector<LPCWSTR> height_map, std::vector<LPCWSTR> diffuse_map)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	shader_list = shader_need;
	physic_pancy = physic_need;
	map_first_level_width = map_number;
	map_second_level_width = map_devide;
	map_width_physics = map_range;
	for (int i = 0; i < height_map.size(); ++i)
	{
		height_map_file_name.push_back(height_map[i]);
	}
	for (int i = 0; i < diffuse_map.size(); ++i)
	{
		diffuse_map_file_name.push_back(diffuse_map[i]);
	}
	height_map_atrray = NULL;
	square_map_point = NULL;
}
HRESULT pancy_terrain_build::create()
{
	HRESULT hr = build_buffer();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = build_texture();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = build_physics();
	if (FAILED(hr))
	{
		return hr;
	}
	return S_OK;
}
void pancy_terrain_build::show_terrain(XMFLOAT4X4 viewproj_mat)
{
	auto shader_deffered = shader_list->get_shader_light_deffered_draw();
	shader_deffered->set_terainbumptex(height_map_atrray);
	shader_deffered->set_teraintex(diffuse_map_atrray);
	XMFLOAT4X4 world_mat;
	XMStoreFloat4x4(&world_mat, XMMatrixIdentity());
	shader_deffered->set_trans_world(&world_mat);
	shader_deffered->set_trans_all(&viewproj_mat);
	pancy_material test_Mt;
	XMFLOAT4 rec_ambient2(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 rec_diffuse2(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 rec_specular2(0.2f, 0.2f, 0.2f, 1.0f);
	test_Mt.ambient = rec_ambient2;
	test_Mt.diffuse = rec_diffuse2;
	test_Mt.specular = rec_specular2;
	shader_deffered->set_material(test_Mt);
	UINT stride = sizeof(point_terrain);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	contex_pancy->IASetVertexBuffers(0, 1, &square_map_point, &stride, &offset);
	ID3DX11EffectTechnique* tech;
	//设置顶点声明
	D3D11_INPUT_ELEMENT_DESC rec_point[] =
	{
		//语义名    语义索引      数据格式          输入槽 起始地址     输入槽的格式 
		{ "POSITION"    ,0  ,DXGI_FORMAT_R32G32B32_FLOAT    ,0    ,0  ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXINDICES"  ,0  ,DXGI_FORMAT_R32G32B32A32_UINT  ,0    ,12 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD"    ,0  ,DXGI_FORMAT_R32G32_FLOAT       ,0    ,28 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 },
		{ "TEXCOORD"    ,1  ,DXGI_FORMAT_R32G32_FLOAT       ,0    ,36 ,D3D11_INPUT_PER_VERTEX_DATA  ,0 }
	};
	int num_member = sizeof(rec_point) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	shader_deffered->get_technique(rec_point, num_member, &tech, "Lightterrain");
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		int buffer_length = map_first_level_width * map_first_level_width * map_second_level_width * map_second_level_width * 4;
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->Draw(buffer_length, 0);
	}
	contex_pancy->HSSetShader(NULL, 0, 0);
	contex_pancy->DSSetShader(NULL, 0, 0);
}
void pancy_terrain_build::show_terrainshape(ID3DX11EffectTechnique* tech)
{
	UINT stride = sizeof(point_terrain);
	UINT offset = 0;
	contex_pancy->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	contex_pancy->IASetVertexBuffers(0, 1, &square_map_point, &stride, &offset);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		int buffer_length = map_first_level_width * map_first_level_width * map_second_level_width * map_second_level_width * 4;
		tech->GetPassByIndex(p)->Apply(0, contex_pancy);
		contex_pancy->Draw(buffer_length, 0);
	}
	contex_pancy->HSSetShader(NULL, 0, 0);
	contex_pancy->DSSetShader(NULL, 0, 0);
}
HRESULT pancy_terrain_build::build_buffer()
{
	HRESULT hr;
	int number_point = map_first_level_width * map_first_level_width * map_second_level_width * map_second_level_width * 4;
	point_terrain *data = new point_terrain[number_point];
	int real_range1 = map_first_level_width * map_first_level_width;
	int real_range2 = map_second_level_width * map_second_level_width;

	XMFLOAT3 square[4] =
	{
		XMFLOAT3(-1.0f,0.0f,1.0f),
		XMFLOAT3(1.0f,0.0f,1.0f),
		XMFLOAT3(-1.0f,0.0f,-1.0f),
		XMFLOAT3(1.0f,0.0f,-1.0f)
	};
	float uv_need[4][2] = { 0.0f,1.0f, 1.0f,1.0f,0.0f,0.0f,1.0f,0.0f };
	for (int i = 0; i < real_range1; ++i)
	{
		for (int j = 0; j < real_range2; ++j)
		{
			//当前一级地图的中心点位置
			XMFLOAT3 firstlevel_center_point, secondlevel_center_point, final_center_point;
			int first_x = i / map_first_level_width;
			int first_z = i % map_first_level_width;
			float first_step_need = map_width_physics / static_cast<float>(map_first_level_width);
			float first_rightdown_st = 0.5f * first_step_need - (map_width_physics / 2.0f);
			firstlevel_center_point.x = first_rightdown_st + first_x * first_step_need;
			firstlevel_center_point.y = 0.0f;
			firstlevel_center_point.z = first_rightdown_st + first_z * first_step_need;
			//当前二级地图的中心点位置
			int second_x = j / map_second_level_width;
			int second_z = j % map_second_level_width;
			float second_step_need = first_step_need / static_cast<float>(map_second_level_width);
			float second_rightdown_st = 0.5f * second_step_need - (first_step_need / 2.0f);
			secondlevel_center_point.x = second_rightdown_st + second_x * second_step_need;
			secondlevel_center_point.y = 0.0f;
			secondlevel_center_point.z = second_rightdown_st + second_z * second_step_need;
			//当前中心点的物理位置
			final_center_point.x = firstlevel_center_point.x + secondlevel_center_point.x;
			final_center_point.y = 0.0f;
			final_center_point.z = firstlevel_center_point.z + secondlevel_center_point.z;
			float map_self_offset = second_step_need / 2.0f;
			float texuv_offset = 1.0f / static_cast<float>(map_second_level_width);
			for (int k = 0; k < 4; ++k)
			{
				data[i * real_range2 + j * 4 + k].tex_id = XMUINT4(i, i, 0, 1);

				data[i * real_range2 + j * 4 + k].position.x = final_center_point.x + square[k].x * map_self_offset;
				data[i * real_range2 + j * 4 + k].position.y = final_center_point.y + square[k].y;
				data[i * real_range2 + j * 4 + k].position.z = final_center_point.z + square[k].z * map_self_offset;

				float now_uvst_x = static_cast<float>(second_x) / static_cast<float>(map_second_level_width);
				float now_uvst_z = static_cast<float>(second_z) / static_cast<float>(map_second_level_width);
				data[i * real_range2 + j * 4 + k].tex_height.x = now_uvst_x + uv_need[k][0] * texuv_offset;
				data[i * real_range2 + j * 4 + k].tex_height.y = now_uvst_z + uv_need[k][1] * texuv_offset;
				data[i * real_range2 + j * 4 + k].tex_diffuse.x = (now_uvst_x + uv_need[k][0] * texuv_offset) * 10.0f;
				data[i * real_range2 + j * 4 + k].tex_diffuse.y = (now_uvst_z + uv_need[k][1] * texuv_offset) * 10.0f;
			}
		}
	}
	D3D11_BUFFER_DESC data_desc;
	data_desc.Usage = D3D11_USAGE_IMMUTABLE;
	data_desc.ByteWidth = sizeof(point_terrain) * number_point;
	data_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	data_desc.CPUAccessFlags = 0;
	data_desc.MiscFlags = 0;
	data_desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA data_buf;
	data_buf.pSysMem = data;
	hr = device_pancy->CreateBuffer(&data_desc, &data_buf, &square_map_point);
	if (FAILED(hr))
	{
		MessageBox(0, L"create terrain buffer error", L"tip", MB_OK);
		return hr;
	}
	delete[] data;
	return S_OK;
}
HRESULT pancy_terrain_build::build_texture()
{
	HRESULT hr;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~地形的深度图~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::vector<ID3D11Texture2D*> depthmap_Tex(height_map_file_name.size());
	//从文件中导入纹理资源
	for (int i = 0; i < height_map_file_name.size(); ++i)
	{
		hr = CreateDDSTextureFromFileEx(device_pancy, height_map_file_name[i], 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ, 0, false, (ID3D11Resource**)&depthmap_Tex[i], 0, 0);
		if (FAILED(hr))
		{
			MessageBox(0, L"create terrain depth texture error", L"tip", MB_OK);
			return hr;
		}
	}
	//创建纹理数组
	D3D11_TEXTURE2D_DESC texElementDesc;
	depthmap_Tex[0]->GetDesc(&texElementDesc);
	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = texElementDesc.Width;
	texArrayDesc.Height = texElementDesc.Height;
	texArrayDesc.MipLevels = texElementDesc.MipLevels;
	texArrayDesc.ArraySize = height_map_file_name.size();
	texArrayDesc.Format = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = 0;
	ID3D11Texture2D* texArray = 0;
	device_pancy->CreateTexture2D(&texArrayDesc, 0, &texArray);
	//填充纹理数组
	for (UINT texElement = 0; texElement < height_map_file_name.size(); ++texElement)
	{
		//每层mipmap都需要填充
		for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			HRESULT hr;
			hr = contex_pancy->Map(depthmap_Tex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
			contex_pancy->UpdateSubresource(texArray, D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels), 0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);
			contex_pancy->Unmap(depthmap_Tex[texElement], mipLevel);
		}
	}
	//创建SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = height_map_file_name.size();
	device_pancy->CreateShaderResourceView(texArray, &viewDesc, &height_map_atrray);
	//释放资源
	texArray->Release();
	for (UINT i = 0; i < height_map_file_name.size(); ++i)
	{
		depthmap_Tex[i]->Release();
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~地形的纹理图~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::vector<ID3D11Texture2D*> diffusemap_Tex(diffuse_map_file_name.size());
	for (int i = 0; i < diffuse_map_file_name.size(); ++i)
	{
		hr = CreateDDSTextureFromFileEx(device_pancy, diffuse_map_file_name[i], 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ, 0, false, (ID3D11Resource**)&diffusemap_Tex[i], 0, 0);
		if (FAILED(hr))
		{
			MessageBox(0, L"create terrain diffuse texture error", L"tip", MB_OK);
			return hr;
		}
	}
	diffusemap_Tex[0]->GetDesc(&texElementDesc);
	texArrayDesc.Width = texElementDesc.Width;
	texArrayDesc.Height = texElementDesc.Height;
	texArrayDesc.MipLevels = texElementDesc.MipLevels;
	texArrayDesc.ArraySize = diffuse_map_file_name.size();
	texArrayDesc.Format = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = 0;
	texArray = NULL;
	device_pancy->CreateTexture2D(&texArrayDesc, 0, &texArray);
	//填充纹理数组
	for (UINT texElement = 0; texElement < diffuse_map_file_name.size(); ++texElement)
	{
		//每层mipmap都需要填充
		for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			HRESULT hr;
			hr = contex_pancy->Map(diffusemap_Tex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
			contex_pancy->UpdateSubresource(texArray, D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels), 0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);
			contex_pancy->Unmap(diffusemap_Tex[texElement], mipLevel);
		}
	}
	//创建SRV
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = diffuse_map_file_name.size();
	device_pancy->CreateShaderResourceView(texArray, &viewDesc, &diffuse_map_atrray);
	//释放资源
	texArray->Release();
	for (UINT i = 0; i < diffuse_map_file_name.size(); ++i)
	{
		diffusemap_Tex[i]->Release();
	}
	return S_OK;
}
HRESULT pancy_terrain_build::build_physics() 
{
	float width_map_real, height_map_real, height_terrain_scal;
	width_map_real = 3000.0f;
	height_map_real = 3000.0f;
	height_terrain_scal = 300.0f;
	int depth_map_scal = 1;   //高度图的平平滑等级
	int sample_distance = 4; //高度图的采样距离
	int high_add = 0;       //适量抬高高度图防止物体陷入不精确区域
	int offset_x = 0, offset_z = 0;
	ID3D11Texture2D *tex_height;
	//创建高度图
	HRESULT hr = CreateDDSTextureFromFileEx(device_pancy, height_map_file_name[0], 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ, 0, false, (ID3D11Resource**)&tex_height, 0, 0);
	D3D11_TEXTURE2D_DESC texElementDesc;
	tex_height->GetDesc(&texElementDesc);
	unsigned int rec_answer;
	for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
	{
		D3D11_MAPPED_SUBRESOURCE mappedTex2D;
		HRESULT hr;
		hr = contex_pancy->Map(tex_height, mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
		D3D11_TEXTURE2D_DESC rec_desc;
		tex_height->GetDesc(&rec_desc);
		numRows = (rec_desc.Height / sample_distance) * depth_map_scal;
		numCols = (rec_desc.Width / sample_distance) * depth_map_scal;
		samples = (physx::PxHeightFieldSample*)malloc(sizeof(physx::PxHeightFieldSample)*(numRows*numCols));
		for (UINT i = 0; i < numRows; ++i)
		{
			float sample_00, sample_01, sample_10, sample_11;
			int now_row_0 = (i / depth_map_scal) * sample_distance + offset_x;
			int now_row_1 = (i / depth_map_scal + 1) * sample_distance + offset_x;
			float row_sample_pos = static_cast<float>(i) / depth_map_scal - static_cast<float>(i / depth_map_scal);
			//unsigned char* rec = static_cast<unsigned char*>(mappedTex2D.pData) + (mappedTex2D.RowPitch) * i;
			unsigned char* rec_row_0 = static_cast<unsigned char*>(mappedTex2D.pData) + (mappedTex2D.RowPitch) * now_row_0;
			unsigned char* rec_row_1 = static_cast<unsigned char*>(mappedTex2D.pData) + (mappedTex2D.RowPitch) * now_row_1;
			for (UINT j = 0; j < numCols; ++j)
			{
				int now_col_0 = (j / depth_map_scal) * sample_distance + offset_z;
				int now_col_1 = (j / depth_map_scal + 1) * sample_distance + offset_z;
				float col_sample_pos = static_cast<float>(j) / depth_map_scal - static_cast<float>(j / depth_map_scal);
				sample_00 = 2 * rec_row_0[now_col_0 * 4 + 3] - 255;
				sample_01 = 2 * rec_row_0[now_col_1 * 4 + 3] - 255;

				sample_10 = 2 * rec_row_1[now_col_0 * 4 + 3] - 255;
				sample_11 = 2 * rec_row_1[now_col_1 * 4 + 3] - 255;
				float first_height_col = sample_00 + (sample_01 - sample_00) * col_sample_pos;
				float second_height_col = sample_10 + (sample_11 - sample_10) * col_sample_pos;
				samples[j * numRows + i].height = first_height_col + (second_height_col - first_height_col) * row_sample_pos + high_add;
				//samples[j * numRows + i].height = 2 * rec[j * 4 + 3] - 255;
			}
		}
		contex_pancy->Unmap(tex_height, mipLevel);
	}
	tex_height->Release();
	physx::PxHeightFieldDesc hfDesc;
	hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
	hfDesc.nbColumns = numCols;
	hfDesc.nbRows = numRows;
	hfDesc.samples.data = samples;
	hfDesc.samples.stride = sizeof(physx::PxHeightFieldSample);
	physx::PxVec3 rec_scal(height_terrain_scal / 255.0, (1.0 / numRows) * width_map_real, (1.0 / numCols) * height_map_real);
	terrain_mat_force = physic_pancy->create_material(0.5,0.5,0.5);
	return physic_pancy->create_terrain(hfDesc, rec_scal, physx::PxTransform(physx::PxVec3(-1500.0f, 0.0f, -1500.0f)), terrain_mat_force);
}
void pancy_terrain_build::release()
{
	diffuse_map_atrray->Release();
	height_map_atrray->Release();
	square_map_point->Release();
}