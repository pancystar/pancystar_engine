
/*
struct pancy_light_dir//方向光结构
{
	//光照强度
	float4 ambient;
	float4 diffuse;
	float4 specular;
	//光照方向
	float3 dir;
	//光照范围
	float  range;
};
struct pancy_light_point//点光源结构
{
	//光照强度
	float4 ambient;
	float4 diffuse;
	float4 specular;
	//光照位置及衰减
	float3 position;
	//光照范围
	float  range;
	
	float3 decay;
};
struct pancy_light_spot//聚光灯结构
{
	//光照强度
	float4    ambient;
	float4    diffuse;
	float4    specular;
	//光照位置，方向及衰减
	float3    dir;
	float     spot;

	//聚光灯属性
	float3    position;
	float     theta;

	float3    decay;
	float     range;
};
*/
struct pancy_light_basic//聚光灯结构
{
	//光照强度
	float4    ambient;
	float4    diffuse;
	float4    specular;
	//光照位置，方向及衰减
	float3    dir;
	float     spot;

	//聚光灯属性
	float3    position;
	float     theta;

	float3    decay;
	float     range;
	//光照类型
	uint4   type;
};
struct pancy_material
{
	float4   ambient;   //材质的环境光反射系数
	float4   diffuse;   //材质的漫反射系数
	float4   specular;  //材质的镜面反射系数
	float4   reflect;  //材质的镜面反射系数
};
void compute_dirlight(
	pancy_material mat,
	pancy_light_basic light_dir,
	float3 normal,
	float3 direction_view,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	ambient = mat.ambient * light_dir.ambient;         //环境光
	float diffuse_angle = dot(-light_dir.dir, normal); //漫反射夹角
	[flatten]
	if (diffuse_angle > 0.0f)
	{
		float3 v = reflect(light_dir.dir, normal);
		float spec_angle = pow(max(dot(v, direction_view), 0.0f), mat.specular.w);

		diffuse = diffuse_angle * mat.diffuse * light_dir.diffuse;//漫反射光

		spec = spec_angle * mat.specular * light_dir.specular;    //镜面反射光
	}
}

void compute_pointlight(
	pancy_material mat,
	pancy_light_basic light_point,
	float3 pos,
	float3 normal,
	float3 position_view,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 lightVec = light_point.position - pos;
	float d = length(lightVec);
	ambient = mat.ambient * light_point.ambient;         //环境光

	float3 eye_direct = normalize(position_view  - pos);
	if (d > light_point.range)
	{
		return;
	}
	//光照方向          
	lightVec = lightVec /= d;
	//漫反射夹角
	float diffuse_angle = dot(lightVec, normal);
	//直线衰减效果
	float4 distance_need;
	distance_need = float4(1.0f, d, d*d, 0.0f);
	float decay_final = 1.0 / dot(distance_need, float4(light_point.decay, 0.0f));
	//镜面反射
	[flatten]
	if (diffuse_angle > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float spec_angle = pow(max(dot(v, eye_direct), 0.0f), mat.specular.w);
		diffuse = decay_final * diffuse_angle * mat.diffuse * light_point.diffuse;//漫反射光
		spec = decay_final * spec_angle * mat.specular * light_point.specular;    //镜面反射光
	}
}

void compute_spotlight(
	pancy_material mat,
	pancy_light_basic light_spot,
	float3 pos,
	float3 normal,
	float3 direction_view,
	out float4 ambient,
	out float4 diffuse,
	out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float3 lightVec = light_spot.position - pos;
	float d = length(lightVec);
	//光照方向
	lightVec /= d;
	light_spot.dir = normalize(light_spot.dir);
	ambient = mat.ambient * light_spot.ambient;//环境光
	float tmp = -dot(lightVec, light_spot.dir);//照射向量与光线向量的夹角
	if (tmp < cos(light_spot.theta))//聚光灯方向之外
	{
		return;
	}
	if (d > light_spot.range)//聚光灯范围之外
	{
		return;
	}
	//漫反射夹角
	float diffuse_angle = dot(lightVec, normal);
	//直线衰减效果
	float4 distance_need;
	distance_need = float4(1.0f, d, d*d, 0.0f);
	float decay_final = 1.0 / dot(distance_need, float4(light_spot.decay, 0.0f));
	//环形衰减效果
	float decay_spot = pow(tmp, light_spot.spot);
	//镜面反射
	[flatten]
	if (diffuse_angle > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		
		float spec_angle = pow(max(dot(v, direction_view), 0.0f), mat.specular.w);
		diffuse = decay_spot*decay_final * diffuse_angle * mat.diffuse * light_spot.diffuse;//漫反射光
		spec = decay_spot*decay_final * spec_angle * mat.specular * light_spot.specular;    //镜面反射光
	}
	
}