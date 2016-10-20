#include"pancy_model_import.h"

assimp_basic::assimp_basic(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* pFile, char *texture_path)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	filename = pFile;
	strcpy(rec_texpath, texture_path);
	model_need = NULL;
	matlist_need = NULL;
	material_optimization = 0;
}
int assimp_basic::get_meshnum()
{
	if (model_need == NULL)
	{
		return 0;
	}
	return mesh_optimization;
}
int assimp_basic::get_meshnormalnum()
{
	if (model_need == NULL)
	{
		return 0;
	}
	return mesh_normal_optimization;
}
void assimp_basic::remove_texture_path(char rec[])
{
	int rec_num = strlen(rec);
	int start = 0;
	for (int i = 0; i < rec_num; ++i)
	{
		if (rec[i] == '\\')
		{
			start = i + 1;
		}
	}
	strcpy(rec, &rec[start]);
}
void assimp_basic::change_texturedesc_2dds(char rec[])
{
	int rec_num = strlen(rec);
	int start = 0;
	for (int i = 0; i < rec_num; ++i)
	{
		if (rec[i] == '.')
		{
			rec[i + 1] = 'd';
			rec[i + 2] = 'd';
			rec[i + 3] = 's';
			rec[i + 4] = '\0';
		}
	}
	strcpy(rec, &rec[start]);
}
HRESULT assimp_basic::model_create(bool if_adj, bool if_optimize, int alpha_partnum, int* alpha_part)
{
	for (int i = 0; i < 10000; ++i)
	{
		if_alpha_array[i] = false;
		if_normal_array[i] = false;
	}
	if (alpha_partnum != 0 && alpha_part != NULL)
	{
		for (int i = 0; i < alpha_partnum; ++i)
		{
			if_alpha_array[alpha_part[i]] = true;
		}
	}

	aiProcess_ConvertToLeftHanded;
	model_need = importer.ReadFile(filename,
		aiProcess_MakeLeftHanded |
		aiProcess_FlipWindingOrder |
		aiProcess_CalcTangentSpace |             //计算切线和副法线
												 //aiProcess_Triangulate |                 //将四边形面转换为三角面
		aiProcess_JoinIdenticalVertices		//合并相同的顶点
											//aiProcess_SortByPType
		);//将不同图元放置到不同的模型中去，图片类型可能是点、直线、三角形等
	if (!model_need)
	{
		return E_FAIL;
	}
	matlist_need = new material_list[model_need->mNumMaterials];
	for (unsigned int i = 0; i < model_need->mNumMaterials; ++i)
	{
		const aiMaterial* pMaterial = model_need->mMaterials[i];
		aiString Path;
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0 && pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			strcpy(matlist_need[i].texture_diffuse, Path.data);
			remove_texture_path(matlist_need[i].texture_diffuse);
			change_texturedesc_2dds(matlist_need[i].texture_diffuse);
			char rec_name[128];
			strcpy(rec_name, rec_texpath);
			strcat(rec_name, matlist_need[i].texture_diffuse);
			strcpy(matlist_need[i].texture_diffuse, rec_name);
		}
		if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0 && pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			strcpy(matlist_need[i].texture_normal, Path.data);
			remove_texture_path(matlist_need[i].texture_normal);
			change_texturedesc_2dds(matlist_need[i].texture_normal);
			char rec_name[128];
			strcpy(rec_name, rec_texpath);
			strcat(rec_name, matlist_need[i].texture_normal);
			strcpy(matlist_need[i].texture_normal, rec_name);
		}
	}
	HRESULT hr;
	hr = init_mesh(if_adj);
	if (hr != S_OK)
	{
		MessageBox(0, L"create model error when init mesh", L"tip", MB_OK);
		return hr;
	}
	hr = init_texture();
	if (hr != S_OK)
	{
		MessageBox(0, L"create model error when init texture", L"tip", MB_OK);
		return hr;
	}
	if (if_optimize == true)
	{
		optimization_mesh(if_adj);
	}
	hr = combine_vertex_array(alpha_partnum, alpha_part, if_adj);
	optimization_normalmesh(if_adj);
	if (hr != S_OK)
	{
		MessageBox(0, L"create model error when combine scene mesh", L"tip", MB_OK);
		return hr;
	}
	return S_OK;
}
HRESULT assimp_basic::init_texture()
{
	HRESULT hr_need;
	for (int i = 0; i < model_need->mNumMaterials; ++i)
	{
		//创建漫反射贴图
		if (matlist_need[i].texture_diffuse[0] != '\0')
		{
			//转换文件名为unicode
			size_t len = strlen(matlist_need[i].texture_diffuse) + 1;
			size_t converted = 0;
			wchar_t *texture_name;
			texture_name = (wchar_t*)malloc(len*sizeof(wchar_t));
			mbstowcs_s(&converted, texture_name, len, matlist_need[i].texture_diffuse, _TRUNCATE);
			//根据文件名创建纹理资源
			//hr_need = CreateDDSTextureFromFile(device_pancy, texture_name, 0, &matlist_need[i].tex_diffuse_resource, 0, 0);
			hr_need = CreateDDSTextureFromFileEx(device_pancy, contex_pancy, texture_name, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0., false, NULL, &matlist_need[i].tex_diffuse_resource);
			if (FAILED(hr_need))
			{
				MessageBox(0, L"create texture error", L"tip", MB_OK);
				return E_FAIL;
			}
			//释放临时文件名
			free(texture_name);
			texture_name = NULL;
		}
		//创建法线贴图
		if (matlist_need[i].texture_normal[0] != '\0')
		{
			//转换文件名为unicode
			size_t len = strlen(matlist_need[i].texture_normal) + 1;
			size_t converted = 0;
			wchar_t *texture_name;
			texture_name = (wchar_t*)malloc(len*sizeof(wchar_t));
			mbstowcs_s(&converted, texture_name, len, matlist_need[i].texture_normal, _TRUNCATE);
			//根据文件名创建纹理资源

			ID3D11Resource* Texture = NULL;
			//hr_need = CreateDDSTextureFromFile(device_pancy, texture_name, &Texture, &matlist_need[i].texture_normal_resource, 0, 0);
			hr_need = CreateDDSTextureFromFileEx(device_pancy, contex_pancy, texture_name, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0., false, NULL, &matlist_need[i].texture_normal_resource);

			//contex_pancy->GenerateMips();
			if (FAILED(hr_need))
			{
				MessageBox(0, L"create texture error", L"tip", MB_OK);
				return E_FAIL;
			}
			//释放临时文件名
			free(texture_name);
			texture_name = NULL;
		}
	}
	material_optimization = model_need->mNumMaterials;
	return S_OK;
}
HRESULT assimp_basic::get_technique(ID3DX11EffectTechnique *teque_need)
{
	if (teque_need == NULL)
	{
		MessageBox(0, L"get render technique error", L"tip", MB_OK);
		return E_FAIL;
	}
	teque_pancy = teque_need;
	return S_OK;
}
void assimp_basic::release_basic()
{
	//释放纹理资源
	/*
	if (model_need != NULL)
	{
		for (int i = 0; i < material_optimization; ++i)
		{
			if (matlist_need[i].tex_diffuse_resource != NULL)
			{
				matlist_need[i].tex_diffuse_resource->Release();
				matlist_need[i].tex_diffuse_resource = NULL;
			}
			if (matlist_need[i].texture_normal_resource != NULL)
			{
				matlist_need[i].texture_normal_resource->Release();
				matlist_need[i].texture_normal_resource = NULL;
			}
		}
		//释放表资源
		delete[] matlist_need;
		model_need->~aiScene();
	}*/
}


geometry_member::geometry_member(assimp_basic *model_input, bool if_skin, XMFLOAT4X4 matrix_need,XMFLOAT4X4 *matrix_bone, int bone_num_need, std::string name_need, int indexnum_need)
{
	model_data = model_input;
	if_skinmesh = if_skin;
	bone_matrix = matrix_bone;
	bone_num = bone_num_need;
	geometry_name = name_need;
	indexnum_geometry = indexnum_need;
	next = NULL;
	pre = NULL;
	world_matrix = matrix_need;
}
void geometry_member::draw_full_geometry(ID3DX11EffectTechnique *tech_common)
{
	model_data->get_technique(tech_common);
	model_data->draw_mesh();
}
void geometry_member::draw_full_geometry_adj(ID3DX11EffectTechnique *tech_common)
{
	model_data->get_technique(tech_common);
	model_data->draw_mesh_adj();
}
void geometry_member::draw_transparent_part(ID3DX11EffectTechnique *tech_transparent,int transparent_part)
{
	model_data->get_technique(tech_transparent);
	model_data->draw_part(transparent_part);
}
void geometry_member::draw_normal_part(ID3DX11EffectTechnique *tech_transparent, int normal_part)
{
	model_data->get_technique(tech_transparent);
	model_data->draw_normal_part(normal_part);
}
void geometry_member::release()
{ 
	if (if_skinmesh == true)
	{
		skin_mesh *model_animation = static_cast<skin_mesh *>(model_data);
		model_animation->release_all();
	}
	else 
	{
		model_data->release();
	}
};
geometry_control::geometry_control(ID3D11Device *device_need, ID3D11DeviceContext *contex_need)
{
	device_pancy = device_need;
	contex_pancy = contex_need;
	list_model_assimp = new scene_geometry_list();
}
void geometry_member::update(XMFLOAT4X4 world_matrix_need, float delta_time)
{
	reset_world_matrix(world_matrix_need);
	if (if_skinmesh == true)
	{
		skin_mesh *model_animation = static_cast<skin_mesh *>(model_data);
		model_animation->update_animation(delta_time * 20);
		model_animation->update_mesh_offset();
		XMFLOAT4X4 *rec_bonematrix = model_animation->get_bone_matrix();
		int rec_bone_num = model_animation->get_bone_num();
		reset_bone_matrix(rec_bonematrix, rec_bone_num);
	}
}
HRESULT geometry_control::create()
{
	HRESULT hr_need;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~填充几何体列表~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	model_reader_assimp<point_with_tangent> *castel_model;
	skin_mesh           *yuri_animation_model;
	castel_model = new model_reader_assimp<point_with_tangent>(device_pancy, contex_pancy, "castelmodel\\castel.obj", "castelmodel\\");
	yuri_animation_model = new skin_mesh(device_pancy, contex_pancy, "yurimodel_skin\\yuri.FBX", "yurimodel_skin\\");
	//城堡模型
	hr_need = castel_model->model_create(false, true, 0, NULL);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model file error", L"tip", MB_OK);
		return hr_need;
	}
	//不来方夕莉
	int alpha_yuri[] = { 0,1,2,3 };
	hr_need = yuri_animation_model->model_create(false, false, 4, alpha_yuri);
	if (FAILED(hr_need))
	{
		MessageBox(0, L"load model file error", L"tip", MB_OK);
		return hr_need;
	}
	//添加场景几何体部分
	XMFLOAT4X4 world_matrix;
	XMStoreFloat4x4(&world_matrix,XMMatrixIdentity());
	geometry_member *rec_mesh_need = new geometry_member(yuri_animation_model, true, world_matrix, NULL, 0,"model_yuri",0);
	material_list rec_tex;
	yuri_animation_model->get_texture(&rec_tex, 3);
	//geometry_member *rec_mesh_need_trans = new geometry_member(yuri_animation_model, true, true,3, world_matrix, NULL, 0, rec_tex.tex_diffuse_resource,"model_yuri", 0);
	geometry_member *rec_mesh_castel = new geometry_member(castel_model, false, world_matrix, NULL, 0, "model_castel", 0);
	list_model_assimp->add_new_geometry(rec_mesh_need);
	//list_model_assimp->add_new_geometry(rec_mesh_need_trans);
	list_model_assimp->add_new_geometry(rec_mesh_castel);
	return S_OK;
}
void geometry_control::release()
{
	list_model_assimp->release();
}

scene_geometry_list::scene_geometry_list()
{
	head = NULL;
	tail = NULL;
	number_list = 0;
}
void scene_geometry_list::add_new_geometry(geometry_member *data_input)
{
	if (tail == NULL)
	{
		head = data_input;
		tail = data_input;
		tail->set_next_member(NULL);
		tail->set_pre_member(NULL);
	}
	else
	{
		data_input->set_pre_member(tail);
		tail->set_next_member(data_input);
		tail = tail->get_next_member();
	}
	number_list += 1;
}
void scene_geometry_list::delete_geometry_byname(std::string name_input)
{
	geometry_member *find_ptr = head;
	for (int i = 0; i < number_list; ++i)
	{
		std::string name = find_ptr->get_geometry_name();
		if (name == name_input)
		{
			if (find_ptr->get_last_member() == NULL)
			{
				if (find_ptr->get_next_member() == NULL)
				{
					head = NULL;
					tail = NULL;
					delete find_ptr;
					number_list -= 1;
				}
				else
				{
					head = find_ptr->get_next_member();
					head->set_pre_member(NULL);
					delete find_ptr;
					number_list -= 1;
				}
			}
			else
			{
				if (find_ptr->get_next_member() == NULL)
				{
					tail = find_ptr->get_last_member();
					tail->set_next_member(NULL);
					delete find_ptr;
					number_list -= 1;
				}
				else
				{
					find_ptr->get_last_member()->set_next_member(find_ptr->get_next_member());
					find_ptr->get_next_member()->set_pre_member(find_ptr->get_last_member());
					delete find_ptr;
					number_list -= 1;
				}
			}
			break;
		}
		find_ptr = find_ptr->get_next_member();
	}
}
geometry_member *scene_geometry_list::get_geometry_byname(std::string name_input)
{
	geometry_member *find_ptr = head;
	for (int i = 0; i < number_list; ++i)
	{
		std::string name = find_ptr->get_geometry_name();
		if (name == name_input)
		{
			return find_ptr;
		}
		find_ptr = find_ptr->get_next_member();
	}
	return NULL;
}
void scene_geometry_list::update_geometry_byname(std::string name_input, XMFLOAT4X4 world_matrix_need, float delta_time)
{
	geometry_member *find_ptr = head;
	for (int i = 0; i < number_list; ++i)
	{
		std::string name = find_ptr->get_geometry_name();
		if (name == name_input)
		{
			find_ptr->update(world_matrix_need, delta_time);
			break;
		}
		find_ptr = find_ptr->get_next_member();
	}
}
void scene_geometry_list::release()
{
	geometry_member *find_ptr = head;
	for (int i = 0; i < number_list; ++i)
	{
		geometry_member *now_rec = find_ptr;
		find_ptr = find_ptr->get_next_member();
		now_rec->release();
		delete now_rec;
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~骨骼动画~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
skin_mesh::skin_mesh(ID3D11Device *device_need, ID3D11DeviceContext *contex_need, char* filename, char* texture_path) : model_reader_assimp(device_need, contex_need, filename, texture_path)
{
	root_skin = new skin_tree;
	strcpy(root_skin->bone_ID, "root_node");
	root_skin->son = new skin_tree;
	first_animation = NULL;
	bone_num = 0;
	time_all = 0.0f;
	//float rec_need1[16] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	for (int i = 0; i < 100; ++i)
	{
		XMStoreFloat4x4(&bone_matrix_array[i], XMMatrixIdentity());
		XMStoreFloat4x4(&offset_matrix_array[i], XMMatrixIdentity());
		XMStoreFloat4x4(&final_matrix_array[i], XMMatrixIdentity());
	}

	for (int i = 0; i < 100; ++i)
	{
		for (int j = 0; j < 100; ++j)
		{
			tree_node_num[i][j] = 0;
		}
	}
}
void skin_mesh::set_matrix(XMFLOAT4X4 &out, aiMatrix4x4 *in)
{
	out._11 = in->a1;
	out._21 = in->a2;
	out._31 = in->a3;
	out._41 = in->a4;

	out._12 = in->b1;
	out._22 = in->b2;
	out._32 = in->b3;
	out._42 = in->b4;

	out._13 = in->c1;
	out._23 = in->c2;
	out._33 = in->c3;
	out._43 = in->c4;

	out._14 = in->d1;
	out._24 = in->d2;
	out._34 = in->d3;
	out._44 = in->d4;
}

bool skin_mesh::check_ifsame(char a[], char b[])
{
	int length = strlen(a);
	if (strlen(a) != strlen(b))
	{
		return false;
	}
	for (int i = 0; i < length; ++i)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}
aiNode *skin_mesh::find_skinroot(aiNode *now_node, char root_name[])
{
	if (now_node == NULL)
	{
		return NULL;
	}
	if (check_ifsame(root_name, now_node->mName.data))
	{
		return now_node;
	}
	for (int i = 0; i < now_node->mNumChildren; ++i)
	{
		aiNode *p = find_skinroot(now_node->mChildren[i], root_name);
		if (p != NULL)
		{
			return p;
		}
	}
	return NULL;
}
HRESULT skin_mesh::build_skintree(aiNode *now_node, skin_tree *now_root)
{
	if (now_node != NULL)
	{
		strcpy(now_root->bone_ID, now_node->mName.data);
		set_matrix(now_root->animation_matrix, &now_node->mTransformation);
		//set_matrix(now_root->basic_matrix, &now_node->mTransformation);
		now_root->bone_number = bone_num++;
		if (now_node->mNumChildren > 0)
		{
			//如果有至少一个儿子则建立儿子结点
			now_root->son = new skin_tree();
			build_skintree(now_node->mChildren[0], now_root->son);
		}
		skin_tree *p = now_root->son;
		for (int i = 1; i < now_node->mNumChildren; ++i)
		{
			//建立所有的兄弟链
			p->brother = new skin_tree();
			build_skintree(now_node->mChildren[i], p->brother);
			p = p->brother;
		}
	}
	return S_OK;

}
HRESULT skin_mesh::build_animation_list()
{
	for (int i = 0; i < model_need->mNumAnimations; ++i)
	{
		animation_set *p = new animation_set;
		p->animation_length = model_need->mAnimations[i]->mDuration;
		strcpy(p->animation_name, model_need->mAnimations[i]->mName.data);
		p->number_animation = model_need->mAnimations[i]->mNumChannels;
		p->head_animition = NULL;
		for (int j = 0; j < p->number_animation; ++j)
		{
			animation_data *q = new animation_data;
			strcpy(q->bone_name, model_need->mAnimations[i]->mChannels[j]->mNodeName.data);
			q->bone_point = find_tree(root_skin, q->bone_name);
			//旋转四元数
			q->number_rotation = model_need->mAnimations[i]->mChannels[j]->mNumRotationKeys;
			q->rotation_key = new quaternion_animation[q->number_rotation];

			for (int k = 0; k < q->number_rotation; ++k)
			{
				q->rotation_key[k].time = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mTime;
				q->rotation_key[k].main_key[0] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.x;
				q->rotation_key[k].main_key[1] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.y;
				q->rotation_key[k].main_key[2] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.z;
				q->rotation_key[k].main_key[3] = model_need->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.w;
			}
			//平移向量
			q->number_translation = model_need->mAnimations[i]->mChannels[j]->mNumPositionKeys;
			q->translation_key = new vector_animation[q->number_translation];
			for (int k = 0; k < q->number_translation; ++k)
			{
				q->translation_key[k].time = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mTime;
				q->translation_key[k].main_key[0] = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.x;
				q->translation_key[k].main_key[1] = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.y;
				q->translation_key[k].main_key[2] = model_need->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue.z;
			}
			//缩放向量
			q->number_scaling = model_need->mAnimations[i]->mChannels[j]->mNumScalingKeys;
			q->scaling_key = new vector_animation[q->number_scaling];
			for (int k = 0; k < q->number_scaling; ++k)
			{
				q->scaling_key[k].time = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mTime;
				q->scaling_key[k].main_key[0] = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.x;
				q->scaling_key[k].main_key[1] = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.y;
				q->scaling_key[k].main_key[2] = model_need->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue.z;
			}

			q->next = p->head_animition;
			p->head_animition = q;
		}
		p->next = first_animation;
		first_animation = p;
	}
	return S_OK;
}
skin_tree* skin_mesh::find_tree(skin_tree* p, char name[])
{
	if (check_ifsame(p->bone_ID, name))
	{
		return p;
	}
	else
	{
		skin_tree* q;
		if (p->brother != NULL)
		{
			q = find_tree(p->brother, name);
			if (q != NULL)
			{
				return q;
			}
		}
		if (p->son != NULL)
		{
			q = find_tree(p->son, name);
			if (q != NULL)
			{
				return q;
			}
		}
	}
	return NULL;
}
skin_tree* skin_mesh::find_tree(skin_tree* p, int num)
{
	if (p->bone_number == num)
	{
		return p;
	}
	else
	{
		skin_tree* q;
		if (p->brother != NULL)
		{
			q = find_tree(p->brother, num);
			if (q != NULL)
			{
				return q;
			}
		}
		if (p->son != NULL)
		{
			q = find_tree(p->son, num);
			if (q != NULL)
			{
				return q;
			}
		}
	}
	return NULL;
}
void skin_mesh::update_root(skin_tree *root, XMFLOAT4X4 matrix_parent)
{
	if (root == NULL)
	{
		return;
	}
	//float rec[16];
	XMMATRIX rec = XMLoadFloat4x4(&root->animation_matrix);
	XMStoreFloat4x4(&root->now_matrix, rec * XMLoadFloat4x4(&matrix_parent));
	//memcpy(rec, root->animation_matrix, 16 * sizeof(float));
	//MatrixMultiply(root->now_matrix, rec, matrix_parent);
	if (root->bone_number >= 0)
	{//memcpy(&bone_matrix_array[root->bone_number], root->now_matrix, 16 * sizeof(float));
		bone_matrix_array[root->bone_number] = root->now_matrix;
	}
	update_root(root->brother, matrix_parent);
	update_root(root->son, root->now_matrix);
}
void skin_mesh::update_mesh_offset(int i)
{
	for (int j = 0; j < model_need->mMeshes[i]->mNumBones; ++j)
	{
		set_matrix(offset_matrix_array[tree_node_num[i][j]], &model_need->mMeshes[i]->mBones[j]->mOffsetMatrix);
	}
}
void skin_mesh::update_mesh_offset()
{
	for (int i = 0; i < model_need->mNumMeshes; ++i)
	{
		for (int j = 0; j < model_need->mMeshes[i]->mNumBones; ++j)
		{
			set_matrix(offset_matrix_array[tree_node_num[i][j]], &model_need->mMeshes[i]->mBones[j]->mOffsetMatrix);
		}
	}
}
HRESULT skin_mesh::init_mesh(bool if_adj)
{
	point_withskin *point_need;
	unsigned int *index_need;
	build_skintree(model_need->mRootNode, root_skin->son);
	build_animation_list();
	//创建网格记录表
	mesh_need = new mesh_list<point_withskin>[model_need->mNumMeshes];
	mesh_optimization = model_need->mNumMeshes;
	for (int i = 0; i < model_need->mNumMeshes; i++)
	{

		//创建顶点缓存区
		const aiMesh* paiMesh = model_need->mMeshes[i];
		mesh_need[i].material_use = paiMesh->mMaterialIndex;
		//模型的第i个模块的顶点及索引信息
		mesh_need[i].point_buffer = new mesh_comman<point_withskin>(device_pancy, contex_pancy, paiMesh->mNumVertices, paiMesh->mNumFaces * 3);
		point_need = (point_withskin*)malloc(paiMesh->mNumVertices * sizeof(point_withskin));
		index_need = (unsigned int*)malloc(paiMesh->mNumFaces * 3 * sizeof(unsigned int));
		for (int j = 0; j < paiMesh->mNumVertices; ++j)
		{
			point_need[j].bone_id.x = 99;
			point_need[j].bone_id.y = 99;
			point_need[j].bone_id.z = 99;
			point_need[j].bone_id.w = 99;
			point_need[j].bone_weight.x = 0.0f;
			point_need[j].bone_weight.y = 0.0f;
			point_need[j].bone_weight.z = 0.0f;
			point_need[j].bone_weight.w = 0.0f;
		}
		//顶点缓存
		for (unsigned int j = 0; j < paiMesh->mNumVertices; j++)
		{
			point_need[j].position.x = paiMesh->mVertices[j].x;
			point_need[j].position.y = paiMesh->mVertices[j].y;
			point_need[j].position.z = paiMesh->mVertices[j].z;

			point_need[j].normal.x = paiMesh->mNormals[j].x;
			point_need[j].normal.y = paiMesh->mNormals[j].y;
			point_need[j].normal.z = paiMesh->mNormals[j].z;

			if (paiMesh->HasTextureCoords(0))
			{
				point_need[j].tex.x = paiMesh->mTextureCoords[0][j].x;
				point_need[j].tex.y = 1 - paiMesh->mTextureCoords[0][j].y;
			}
			else
			{
				point_need[j].tex.x = 0.0f;
				point_need[j].tex.y = 0.0f;
			}
			if (paiMesh->mTangents != NULL)
			{
				point_need[j].tangent.x = paiMesh->mTangents[j].x;
				point_need[j].tangent.y = paiMesh->mTangents[j].y;
				point_need[j].tangent.z = paiMesh->mTangents[j].z;
			}
			else
			{
				point_need[j].tangent.x = 0.0f;
				point_need[j].tangent.y = 0.0f;
				point_need[j].tangent.z = 0.0f;
			}
		}
		for (int j = 0; j < paiMesh->mNumBones; ++j)
		{
			skin_tree * now_node = find_tree(root_skin, paiMesh->mBones[j]->mName.data);
			tree_node_num[i][j] = now_node->bone_number;
			//set_matrix(now_node->basic_matrix, &paiMesh->mBones[j]->mOffsetMatrix);
			for (int k = 0; k < paiMesh->mBones[j]->mNumWeights; ++k)
			{
				if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.x == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.x = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.x = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.y == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.y = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.y = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.z == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.z = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.z = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
				else if (point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.w == 99)
				{
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_id.w = now_node->bone_number;
					point_need[paiMesh->mBones[j]->mWeights[k].mVertexId].bone_weight.w = paiMesh->mBones[j]->mWeights[k].mWeight;
				}
			}
		}
		int count_index = 0;
		for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
		{
			if (paiMesh->mFaces[i].mNumIndices == 3)
			{
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[0];
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[1];
				index_need[count_index++] = paiMesh->mFaces[j].mIndices[2];
			}
			else
			{
				return E_FAIL;
			}
		}
		//根据内存信息创建显存区
		HRESULT hr = mesh_need[i].point_buffer->create_object(point_need, index_need, if_adj);
		if (FAILED(hr))
		{
			MessageBox(0, L"create model mesh error", L"tip", MB_OK);
			return hr;
		}
		//释放内存
		free(point_need);
		point_need = NULL;
		free(index_need);
		index_need = NULL;
	}


	//float matrix_identi[] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	XMFLOAT4X4 matrix_identi;
	XMStoreFloat4x4(&matrix_identi, XMMatrixIdentity());
	update_root(root_skin, matrix_identi);
	return S_OK;
}
void skin_mesh::release_all()
{
	release();
	free_tree(root_skin);
}
void skin_mesh::free_tree(skin_tree *now)
{
	if (now->brother != NULL)
	{
		free_tree(now->brother);
	}
	if (now->son != NULL)
	{
		free_tree(now->son);
	}
	if (now != NULL)
	{
		free(now);
	}
}
XMFLOAT4X4* skin_mesh::get_bone_matrix(int i, int &num_bone)
{
	num_bone = model_need->mMeshes[i]->mNumBones;
	for (int j = 0; j < model_need->mMeshes[i]->mNumBones; ++j)
	{
		//MatrixMultiply(&final_matrix_array[tree_node_num[i][j] * 16], &offset_matrix_array[tree_node_num[i][j] * 16], &bone_matrix_array[tree_node_num[i][j] * 16]);
		XMStoreFloat4x4(&final_matrix_array[tree_node_num[i][j]], XMLoadFloat4x4(&offset_matrix_array[tree_node_num[i][j]]) * XMLoadFloat4x4(&bone_matrix_array[tree_node_num[i][j]]));
	}
	return final_matrix_array;
}
XMFLOAT4X4* skin_mesh::get_bone_matrix()
{
	for (int i = 0; i < 100; ++i)
	{
		//MatrixMultiply(&final_matrix_array[tree_node_num[i][j] * 16], &offset_matrix_array[tree_node_num[i][j] * 16], &bone_matrix_array[tree_node_num[i][j] * 16]);
		XMStoreFloat4x4(&final_matrix_array[i], XMLoadFloat4x4(&offset_matrix_array[i]) * XMLoadFloat4x4(&bone_matrix_array[i]));
	}
	return final_matrix_array;
}
void skin_mesh::update_animation(float delta_time)
{
	time_all = (time_all + delta_time);
	if (time_all >= first_animation->animation_length)
	{
		time_all -= first_animation->animation_length;
	}
	update_anim_data(first_animation->head_animition);
	//float matrix_identi[] = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	XMFLOAT4X4 matrix_identi;
	XMStoreFloat4x4(&matrix_identi, XMMatrixIdentity());
	update_root(root_skin, matrix_identi);
}
void skin_mesh::update_anim_data(animation_data *now)
{
	//float rec_trans[16], rec_rot[16], rec_scal[16], rec_mid[16];
	XMMATRIX rec_trans, rec_scal;
	XMFLOAT4X4 rec_rot;
	int start_anim, end_anim;
	if (now == NULL)
	{
		return;
	}
	find_anim_sted(start_anim, end_anim, now->rotation_key, now->number_rotation);
	//四元数插值并寻找变换矩阵
	quaternion_animation rotation_now;
	//rotation_now = now->rotation_key[0];
	if (start_anim == end_anim)
	{
		rotation_now = now->rotation_key[start_anim];
	}
	else
	{
		Interpolate(rotation_now, now->rotation_key[start_anim], now->rotation_key[end_anim], (time_all - now->rotation_key[start_anim].time) / (now->rotation_key[end_anim].time - now->rotation_key[start_anim].time));
	}
	//
	Get_quatMatrix(rec_rot, rotation_now);
	//缩放变换
	find_anim_sted(start_anim, end_anim, now->scaling_key, now->number_scaling);
	vector_animation scalling_now;
	if (start_anim == end_anim)
	{
		scalling_now = now->scaling_key[start_anim];
	}
	else
	{
		Interpolate(scalling_now, now->scaling_key[start_anim], now->scaling_key[end_anim], (time_all - now->scaling_key[start_anim].time) / (now->scaling_key[end_anim].time - now->scaling_key[start_anim].time));
	}
	//scalling_now = now->scaling_key[0];
	rec_scal = XMMatrixScaling(scalling_now.main_key[0], scalling_now.main_key[1], scalling_now.main_key[2]);
	//MatrixScaling(rec_scal, scalling_now.main_key[0], scalling_now.main_key[1], scalling_now.main_key[2]);
	//平移变换
	find_anim_sted(start_anim, end_anim, now->translation_key, now->number_translation);
	vector_animation translation_now;
	if (start_anim == end_anim)
	{
		translation_now = now->translation_key[start_anim];
	}
	else
	{
		Interpolate(translation_now, now->translation_key[start_anim], now->translation_key[end_anim], (time_all - now->translation_key[start_anim].time) / (now->translation_key[end_anim].time - now->translation_key[start_anim].time));
	}
	//translation_now = now->translation_key[0];
	rec_trans = XMMatrixTranslation(translation_now.main_key[0], translation_now.main_key[1], translation_now.main_key[2]);
	//MatrixTranslation(rec_trans, translation_now.main_key[0], translation_now.main_key[1], translation_now.main_key[2]);
	//总变换
	//MatrixMultiply(rec_mid, rec_scal, rec_rot);
	//MatrixMultiply(now->bone_point->animation_matrix, rec_mid, rec_trans);
	XMStoreFloat4x4(&now->bone_point->animation_matrix, rec_scal * XMLoadFloat4x4(&rec_rot) * rec_trans);

	update_anim_data(now->next);
}
void skin_mesh::Interpolate(quaternion_animation& pOut, quaternion_animation pStart, quaternion_animation pEnd, float pFactor)
{
	// calc cosine theta
	float cosom = pStart.main_key[0] * pEnd.main_key[0] + pStart.main_key[1] * pEnd.main_key[1] + pStart.main_key[2] * pEnd.main_key[2] + pStart.main_key[3] * pEnd.main_key[3];


	// adjust signs (if necessary)
	quaternion_animation end = pEnd;
	if (cosom < static_cast<float>(0.0))
	{
		cosom = -cosom;
		end.main_key[0] = -end.main_key[0];   // Reverse all signs
		end.main_key[1] = -end.main_key[1];
		end.main_key[2] = -end.main_key[2];
		end.main_key[3] = -end.main_key[3];
	}

	// Calculate coefficients
	float sclp, sclq;
	if ((static_cast<float>(1.0) - cosom) > static_cast<float>(0.0001)) // 0.0001 -> some epsillon
	{
		// Standard case (slerp)
		float omega, sinom;
		omega = acos(cosom); // extract theta from dot product's cos theta
		sinom = sin(omega);
		sclp = sin((static_cast<float>(1.0) - pFactor) * omega) / sinom;
		sclq = sin(pFactor * omega) / sinom;
	}
	else
	{
		// Very close, do linear interp (because it's faster)
		sclp = static_cast<float>(1.0) - pFactor;
		sclq = pFactor;
	}

	pOut.main_key[0] = sclp * pStart.main_key[0] + sclq * end.main_key[0];
	pOut.main_key[1] = sclp * pStart.main_key[1] + sclq * end.main_key[1];
	pOut.main_key[2] = sclp * pStart.main_key[2] + sclq * end.main_key[2];
	pOut.main_key[3] = sclp * pStart.main_key[3] + sclq * end.main_key[3];
}
void skin_mesh::Interpolate(vector_animation& pOut, vector_animation pStart, vector_animation pEnd, float pFactor)
{
	for (int i = 0; i < 3; ++i)
	{
		pOut.main_key[i] = pStart.main_key[i] + pFactor * (pEnd.main_key[i] - pStart.main_key[i]);
	}
}
void skin_mesh::find_anim_sted(int &st, int &ed, quaternion_animation *input, int num_animation)
{
	if (time_all < 0)
	{
		st = 0;
		ed = 1;
		return;
	}
	if (time_all > input[num_animation - 1].time)
	{
		st = num_animation - 1;
		ed = num_animation - 1;
		return;
	}
	for (int i = 0; i < num_animation; ++i)
	{
		if (time_all >= input[i].time && time_all <= input[i + 1].time)
		{
			st = i;
			ed = i + 1;
			return;
		}
	}
	st = num_animation - 1;
	ed = num_animation - 1;
}
void skin_mesh::find_anim_sted(int &st, int &ed, vector_animation *input, int num_animation)
{
	if (time_all < 0)
	{
		st = 0;
		ed = 1;
		return;
	}
	if (time_all > input[num_animation - 1].time)
	{
		st = num_animation - 1;
		ed = num_animation - 1;
		return;
	}
	for (int i = 0; i < num_animation; ++i)
	{
		if (time_all >= input[i].time && time_all <= input[i + 1].time)
		{
			st = i;
			ed = i + 1;
			return;
		}
	}
	st = num_animation - 1;
	ed = num_animation - 1;
}
void skin_mesh::Get_quatMatrix(XMFLOAT4X4 &resMatrix, quaternion_animation& pOut)
{
	resMatrix._11 = static_cast<float>(1.0) - static_cast<float>(2.0) * (pOut.main_key[1] * pOut.main_key[1] + pOut.main_key[2] * pOut.main_key[2]);
	resMatrix._21 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[1] - pOut.main_key[2] * pOut.main_key[3]);
	resMatrix._31 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[2] + pOut.main_key[1] * pOut.main_key[3]);
	resMatrix._41 = 0.0f;

	resMatrix._12 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[1] + pOut.main_key[2] * pOut.main_key[3]);
	resMatrix._22 = static_cast<float>(1.0) - static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[0] + pOut.main_key[2] * pOut.main_key[2]);
	resMatrix._32 = static_cast<float>(2.0) * (pOut.main_key[1] * pOut.main_key[2] - pOut.main_key[0] * pOut.main_key[3]);
	resMatrix._42 = 0.0f;

	resMatrix._13 = static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[2] - pOut.main_key[1] * pOut.main_key[3]);
	resMatrix._23 = static_cast<float>(2.0) * (pOut.main_key[1] * pOut.main_key[2] + pOut.main_key[0] * pOut.main_key[3]);
	resMatrix._33 = static_cast<float>(1.0) - static_cast<float>(2.0) * (pOut.main_key[0] * pOut.main_key[0] + pOut.main_key[1] * pOut.main_key[1]);
	resMatrix._43 = 0.0f;

	resMatrix._14 = 0.0f;
	resMatrix._24 = 0.0f;
	resMatrix._34 = 0.0f;
	resMatrix._44 = 1.0f;
}