cbuffer perframe
{
	float3 position_view;    //视点位置
	float3 position_start;   //粒子产生源位置
	float3 direction_start;  //粒子产生方向
	float game_time;         //游戏时间(产生随机数种子)
	float delta_time;        //帧动画时间
	float4x4 final_matrix;   //总变换矩阵
};
struct Particle
{
	float3 position : POSITION;
	float3 speed    : VELOCITY;
	float2 SizeW    : SIZE;
	float Age : AGE;
	uint Type       : TYPE;
};
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
Texture2D texture_first; //火焰纹理数据
Texture1D texture_random;//随机向量纹理
cbuffer cbfixed
{
	float3 gAccelW = { 0.0f, 1.3f, 0.0f };//火焰粒子加速度
										  //广告牌纹理坐标
	float2 gTexC[4] =
	{
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f)
	};
};
#define PT_EMITTER 0
#define PT_FLARE 1
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~粒子产生shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};
DepthStencilState NoDepthWrites
{
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
};
BlendState AdditiveBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = ONE;
	BlendOp = ADD;
	SrcBlendAlpha = ZERO;
	DestBlendAlpha = ZERO;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};
Particle StreamOutVS(Particle vin)
{
	return vin;
}
float3 RandUnitVec3(float offset)
{
	//根据时间获得随机数种子
	float u = (game_time + offset);
	//向量纹理采样
	float3 v = 2.0f * texture_random.SampleLevel(samLinear, u, 0).xyz - 1.0f;
	//归一化
	return normalize(v);
}
[maxvertexcount(25)]
void StreamOutGS(point Particle gin[1],
	inout PointStream<Particle> ptStream)
{
	gin[0].Age += delta_time;
	//是否是源粒子
	if (gin[0].Type == PT_EMITTER)
	{
		// 是否应当产生新的粒子
		if (gin[0].Age > 0.005f)
		{
			float3 vRandom = RandUnitVec3(0.0f);
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;
			if (vRandom.y < 0)
				vRandom.y = -vRandom.y;
			//vRandom.y *= 0.6f;
			Particle p;
			p.position = position_start.xyz;
			//p.position.z -= 0.73*j;
			//p.position.z -= 4.76*i;
			p.speed = 4.0f*vRandom;
			p.SizeW = float2(0.5f, 0.5f);
			p.Age = 0.0f;
			p.Type = PT_FLARE;
			ptStream.Append(p);
			//还原源粒子的时间
			gin[0].Age = 0.0f;
			/*
			for (int i = 0; i < 6; ++i)
			{
				for (int j = 0; j < 2; ++j)
				{
					float3 vRandom = RandUnitVec3(0.0f);
					vRandom.x *= 0.5f;
					vRandom.z *= 0.5f;
					if (vRandom.y < 0)
						vRandom.y = -vRandom.y;
					//vRandom.y *= 0.6f;
					Particle p;
					p.position = position_start.xyz;
					p.position.z -= 0.73*j;
					p.position.z -= 4.76*i;
					p.speed = 4.0f*vRandom;
					p.SizeW = float2(0.5f, 0.5f);
					p.Age = 0.0f;
					p.Type = PT_FLARE;

					ptStream.Append(p);

					//还原源粒子的时间
					gin[0].Age = 0.0f;
				}
			}
			for (int i = 0; i < 6; ++i)
			{
				for (int j = 0; j < 2; ++j)
				{
					float3 vRandom = RandUnitVec3(0.0f);
					vRandom.x *= 0.5f;
					vRandom.z *= 0.5f;
					if (vRandom.y < 0)
						vRandom.y = -vRandom.y;
					//vRandom.y *= 0.6f;
					Particle p;
					p.position = position_start.xyz;
					p.position.z -= 0.73*j;
					p.position.z -= 4.76*i;
					p.position.x -= 14.6;
					p.speed = 4.0f*vRandom;
					p.SizeW = float2(0.5f, 0.5f);
					p.Age = 0.0f;
					p.Type = PT_FLARE;

					ptStream.Append(p);

					//还原源粒子的时间
					gin[0].Age = 0.0f;
				}
			}
			*/
		}

		// 将源粒子送入顶点缓冲区
		ptStream.Append(gin[0]);
	}
	else
	{
		//非源粒子，若还处于活跃时期则加入顶点缓存
		if (gin[0].Age <= 0.6f)
			ptStream.Append(gin[0]);
	}
}
GeometryShader gsStreamOut = ConstructGSWithSO(
	CompileShader(gs_5_0, StreamOutGS()),
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x");

technique11 StreamOutTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, StreamOutVS()));
		SetGeometryShader(gsStreamOut);
		// 禁用像素着色器
		SetPixelShader(NULL);
		// 禁用深度缓冲
		SetDepthStencilState(DisableDepth, 0);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~粒子渲染shader~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct Vout
{
	float3 position  : POSITION; //位置
	float2 size      : SIZE;     //大小
	float4 alpha     : COLOR;    //颜色淡化度
	uint   Type      : TYPE;     //类型
};
struct GeoOut
{
	float4 PosH  : SV_Position;
	float3 PosW  : POSITION;
	float4 alpha : COLOR;
	float2 Tex   : TEXCOORD;
};
Vout DrawVS(Particle vin)
{
	Vout vout;
	float t = vin.Age;
	float3 rec_acce = -vin.speed * 0.8f + gAccelW;
	rec_acce.y = gAccelW.y;
	//计算粒子当前的位置s[i+1] = a*t^2/2 + v*t + s[i];
	vout.position = 0.5f*t*t*rec_acce + 0.3*t*vin.speed + vin.position;
	//根据粒子的寿命计算颜色衰减
	float opacity = 1.0f - smoothstep(0.0f, 1.0f, t / 1.0f);
	vout.alpha = float4(1.0f, 1.0f, 1.0f, opacity);
	vout.size = vin.SizeW;
	vout.Type = vin.Type;
	return vout;
}
[maxvertexcount(4)]
void DrawGS(point Vout gin[1],
	inout TriangleStream<GeoOut> triStream)
{
	//判断粒子是否是源粒子
	if (gin[0].Type != PT_EMITTER)
	{

		float3 up_camera = float3(0.0f, 1.0f, 0.0f);
		//计算公告板的面朝方向
		float3 look_camera = position_view - gin[0].position;
		look_camera.y = 0.0f;
		look_camera = normalize(look_camera);
		float3 right_camera = cross(up_camera, look_camera);
		//根据面朝方向信息获取公告板的四个顶点
		float half_width = 0.5f * gin[0].size.x;
		float half_height = 0.5f* gin[0].size.y;
		float4 g_vout[4];
		g_vout[0] = float4(gin[0].position + half_width*right_camera - half_height*up_camera, 1.0f);
		g_vout[1] = float4(gin[0].position + half_width*right_camera + half_height*up_camera, 1.0f);
		g_vout[2] = float4(gin[0].position - half_width*right_camera - half_height*up_camera, 1.0f);
		g_vout[3] = float4(gin[0].position - half_width*right_camera + half_height*up_camera, 1.0f);
		GeoOut gout;
		[unroll]
		for (int i = 0; i < 4; ++i)//两个三角形组成的三角带
		{
			gout.PosH = mul(g_vout[i], final_matrix);
			gout.PosW = g_vout[i].xyz;
			gout.Tex = gTexC[i];
			gout.alpha = gin[0].alpha;
			triStream.Append(gout);
		}
	}
}
float4 DrawPS(GeoOut pin) : SV_TARGET
{
	return texture_first.Sample(samLinear,pin.Tex)*(pin.alpha*0.8f);
}
technique11 DrawTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, DrawVS()));
		SetGeometryShader(CompileShader(gs_5_0, DrawGS()));
		SetPixelShader(CompileShader(ps_5_0, DrawPS()));
		SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
		SetDepthStencilState(NoDepthWrites, 0);
	}
}