#pragma once
#include <conio.h>
#include <Windows.h>
#include <vector>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <fmod.hpp>
#include <iostream>
#include <fstream>
#include<time.h>
#include<directxmath.h>
using namespace DirectX;
struct sound_list
{
	char file_name[128];
	FMOD::Sound *sounds;
	int music_ID;
};
struct sound_list_withbuffer
{
	char buffer_name[128];
	FMOD::Sound *sound_now;
	char *data_need;
	bool if_empty;
	int music_ID;
	int bufflength;
	bool if_playing;//正在运行标记
};
class FMOD_basic
{
	FMOD_RESULT result;                                   //函数返回值检验
	FMOD::System* system;                                 //fmod设备指针
	std::vector<sound_list> sounds;                       //从文件载入的渲染音效
	FMOD::Channel  *channels[700];                         //通用渲染音效通道
	FMOD_VECTOR  istenerpos;                              //设定聆听者的位置
public:
	//构造函数
	FMOD_basic();
	//更新函数
	HRESULT main_update();
	//从文件中加载音乐
	HRESULT load_music(char music_name[], FMOD_CREATESOUNDEXINFO *exinfo, FMOD_MODE mode, int &sound_ID);
	//从内存中加载音乐
	HRESULT load_music_from_memory(char* data_mem, int datalength);
	//设置聆听者位置
	HRESULT set_lisener(XMFLOAT3  location, XMFLOAT3 forward, XMFLOAT3 up, XMFLOAT3 speed);
	//设置某个音乐的位置
	HRESULT set_music_place(int channel_ID, XMFLOAT3 location, XMFLOAT3 speed);
	//开始播放一个普通音乐
	HRESULT play_sound(int sound_ID, int channel_ID);
	HRESULT play_sound_single(int sound_ID, int channel_ID);
	//暂停一个通道
	HRESULT pause_sound(int channel_ID);
	//开启一个通道
	HRESULT start_sound(int channel_ID);
	//设置通道的重要度
	void set_sound_priority(int channel_ID, int priority);
	//检验当前声音是否可用
	bool check_if_virtual(int channel_ID);
	//设置音量
	HRESULT set_value(int channel_ID,float value);
	//注册系统
	HRESULT init_system();
};