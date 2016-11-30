struct point_snake
{
	float4 position;
	float4 center_position;
};
struct point_snake_control
{
	float3 position1;
	float3 position2;
	float3 position3;
	float3 position4;
};
float3   snake_head_position;
uint4    input_range;//(蛇体数量,每一节的细分程度,蛇体宽度(半径),保留参数)
float4x4 Bspline_mat;//b样条矩阵
StructuredBuffer<point_snake_control>   input_buffer; //输入控制点集合

//RWStructuredBuffer<point_snake>         output_buffer;//输出顶点数组
RWByteAddressBuffer output_buffer;//输出顶点数组
RWStructuredBuffer<point_snake_control> next_buffer;  //下一组控制点集合
void rotation_axis(float theta,float3 normal,out float3 vec1, out float3 vec2,out float3 vec3)
{
	normal = normalize(normal);
	float nx = normal.x;
	float ny = normal.y;
	float nz = normal.z;
	//float3 vec1 = float3(0.0f, 0.0f, 0.0f);
	//float3 vec2 = float3(0.0f, 0.0f, 0.0f);
	//float3 vec3 = float3(0.0f, 0.0f, 0.0f);
	vec1.x = (cos(theta) + nx * nx * (1 - cos(theta)));
	vec2.x = (nx * ny * (1 - cos(theta)) - nz * sin(theta));
	vec3.x = (nx * nz * (1 - cos(theta)) + ny * sin(theta));
	
	vec1.y = (nx * ny * (1 - cos(theta)) + nz * sin(theta));
	vec2.y = (ny * ny * (1 - cos(theta)) + cos(theta));
	vec3.y = (ny * nz * (1 - cos(theta)) - nx * sin(theta));
	
	vec1.z = (nx * nz * (1 - cos(theta)) - ny * sin(theta));
	vec2.z = (ny * nz * (1 - cos(theta)) + nx * sin(theta));
	vec3.z = (nz * nz * (1 - cos(theta)) + cos(theta));
	int a = 0;
}
void get_point_message(float4x3 final_mat, 
	float t_now, 
	out float3 position, 
	out float3 tangent, 
	out float3 up, 
	out float3 right) 
{
	float4 input_vector = float4(t_now*t_now*t_now, t_now*t_now, t_now, 1.0f);
	position = mul(input_vector, final_mat);
	//切方向
	float4 input_tangent = float4(normalize(float3(3 * t_now*t_now, 2 * t_now, 1.0f)),0.0f);
	tangent = normalize(mul(input_tangent, final_mat));
	//绕法线旋转的角度
	float angle = acos(dot(tangent, float3(0.0f, 0.0f, 1.0f)));
	float3 normal = cross(float3(0.0f, 0.0f, 1.0f), tangent);
	//恰好与初始点的切线方向一致或相反
	if (abs(normal.x) < 0.001f && abs(normal.y) < 0.001f && abs(normal.z) < 0.001f)
	{
		normal.y = 1.0f;
		angle = acos(dot(tangent, float3(0.0f, 0.0f, 1.0f)));
	}
	//将原始坐标系绕法线旋转
	float3 vec1, vec2, vec3;
	rotation_axis(angle, normal,vec1, vec2, vec3);
	float3x3 mat_axis = float3x3(vec1, vec2, vec3);
	right = mul(float3(1.0f, 0.0f, 0.0f), mat_axis);
	up = mul(float3(0.0f, 1.0f, 0.0f), mat_axis);
}
void store_data(int index,float3 position_square,float3 position_center)
{
	output_buffer.Store(index * 32 + 0, asuint(position_square.x));
	output_buffer.Store(index * 32 + 4, asuint(position_square.y));
	output_buffer.Store(index * 32 + 8, asuint(position_square.z));
	output_buffer.Store(index * 32 + 12, asuint(1.0f));
	output_buffer.Store(index * 32 + 16, asuint(position_center.x));
	output_buffer.Store(index * 32 + 20, asuint(position_center.y));
	output_buffer.Store(index * 32 + 24, asuint(position_center.z));
	output_buffer.Store(index * 32 + 28, asuint(1.0f));
}
//第一遍处理，读入未处理的图像，获取其亮度数据，并下采样至1/16的buffer
[numthreads(16, 16, 1)]
void main_first(uint3 DTid : SV_DispatchThreadID)
{
	uint2 basic_pos;
	point_snake_control now_control_point = input_buffer[DTid.x];
	if (DTid.y >= input_range.y) 
	{
		return;
	}
	if (DTid.y == 0 && DTid.x < input_range.x)
	{
		if (DTid.x > 0) 
		{
			/*todo: in intel card HD4600 we can't use DTid-1 to find the corect array index*/
			point_snake_control rec = input_buffer[DTid.x-1];
			next_buffer[DTid.x].position1 = rec.position1;
			next_buffer[DTid.x].position2 = rec.position2;
			next_buffer[DTid.x].position3 = rec.position3;
			next_buffer[DTid.x].position4 = rec.position4;
		}
		else 
		{
			next_buffer[DTid.x].position1 = snake_head_position;
			next_buffer[DTid.x].position2 = now_control_point.position1;
			next_buffer[DTid.x].position3 = now_control_point.position2;
			next_buffer[DTid.x].position4 = now_control_point.position3;
		}
	}
	//控制点矩阵
	float4x3 control_mat = float4x3(now_control_point.position1, now_control_point.position2, now_control_point.position3, now_control_point.position4);
	//样条线矩阵
	float4x3 final_mat = mul(Bspline_mat, control_mat);
	//根据线程Id获取输入参数向量
	float3 position1, position2;
	float3 tangent1, tangent2;
	float3 up1, up2;
	float3 right1, right2;
	//第一个点的信息
	float t_now = float(DTid.y) / input_range.y;
	get_point_message(final_mat,t_now, position1, tangent1, up1, right1);
	float3 position_square_first[4];
	position_square_first[0] = position1 + (input_range.z * right1) - (input_range.z * up1);
	position_square_first[1] = position1 + (input_range.z * right1) + (input_range.z * up1);
	position_square_first[2] = position1 - (input_range.z * right1) + (input_range.z * up1);
	position_square_first[3] = position1 - (input_range.z * right1) - (input_range.z * up1);
	//第二个点的信息
	t_now = float(DTid.y + 1) / input_range.y;
	get_point_message(final_mat, t_now, position2, tangent2, up2, right2);
	float3 position_square_second[4];
	position_square_second[0] = position2 + (input_range.z * right2) - (input_range.z * up2);
	position_square_second[1] = position2 + (input_range.z * right2) + (input_range.z * up2);
	position_square_second[2] = position2 - (input_range.z * right2) + (input_range.z * up2);
	position_square_second[3] = position2 - (input_range.z * right2) - (input_range.z * up2);
	
	//包围四面体第一个面
	int index = DTid.x * input_range.y * 16 + DTid.y * 16 + 0;
	store_data(index, position_square_first[0], position1);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 1;
	store_data(index, position_square_first[1], position1);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 3;
	store_data(index, position_square_second[1], position2);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 2;
	store_data(index, position_square_second[0], position2);
	//包围四面体第二个面
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 4;
	store_data(index, position_square_first[1], position1);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 5;
	store_data(index, position_square_first[2], position1);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 7;
	store_data(index, position_square_second[2], position2);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 6;
	store_data(index, position_square_second[1], position2);
	//包围四面体第三个面
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 8;
	store_data(index, position_square_first[2], position1);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 9;
	store_data(index, position_square_first[3], position1);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 11;
	store_data(index, position_square_second[3], position2);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 10;
	store_data(index, position_square_second[2], position2);
	//包围四面体第四个面
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 12;
	store_data(index, position_square_first[3], position1);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 13;
	store_data(index, position_square_first[0], position1);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 15;
	store_data(index, position_square_second[0], position2);
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 14;
	store_data(index, position_square_second[3], position2);
	
	/*
	//包围四面体第一个面
	int index = DTid.x * input_range.y * 16 + DTid.y * 16 + 0;
	output_buffer[index].position = position_square_first[0];
	output_buffer[index].center_position = position1;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 1;
	output_buffer[index].position = position_square_first[1];
	output_buffer[index].center_position = position1;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 2;
	output_buffer[index].position = position_square_second[1];
	output_buffer[index].center_position = position2;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 3;
	output_buffer[index].position = position_square_second[0];
	output_buffer[index].center_position = position2;
	//包围四面体第二个面
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 4;
	output_buffer[index].position = position_square_first[1];
	output_buffer[index].center_position = position1;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 5;
	output_buffer[index].position = position_square_first[2];
	output_buffer[index].center_position = position1;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 6;
	output_buffer[index].position = position_square_second[2];
	output_buffer[index].center_position = position2;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 7;
	output_buffer[index].position = position_square_second[1];
	output_buffer[index].center_position = position2;
	//包围四面体第三个面
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 8;
	output_buffer[index].position = position_square_first[2];
	output_buffer[index].center_position = position1;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 9;
	output_buffer[index].position = position_square_first[3];
	output_buffer[index].center_position = position1;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 10;
	output_buffer[index].position = position_square_second[3];
	output_buffer[index].center_position = position2;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 11;
	output_buffer[index].position = position_square_second[2];
	output_buffer[index].center_position = position2;
	//包围四面体第四个面
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 12;
	output_buffer[index].position = position_square_first[3];
	output_buffer[index].center_position = position1;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 13;
	output_buffer[index].position = position_square_first[0];
	output_buffer[index].center_position = position1;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 14;
	output_buffer[index].position = position_square_second[0];
	output_buffer[index].center_position = position2;
	index = DTid.x * input_range.y * 16 + DTid.y * 16 + 15;
	output_buffer[index].position = position_square_second[3];
	output_buffer[index].center_position = position2;
	*/
}
technique11 snake_square_pass
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetGeometryShader(NULL);
		SetComputeShader(CompileShader(cs_5_0, main_first()));
	}
}