#include"light_define.fx"
cbuffer perframe
{
	//pancy_light_dir    dir_light_need[10];    //方向光源
	//pancy_light_point  point_light_need[5];   //点光源
	//pancy_light_spot   spot_light_need[15];   //聚光灯光源
	float3             position_view;         //视点位置
	int num_dir;
	int num_point;
	int num_spot;
};
Texture2D  texture_normal;  //深度法线记录
Texture2D  texture_diffuse; //漫反射材质
Texture2D  texture_specular;//镜面反射材质