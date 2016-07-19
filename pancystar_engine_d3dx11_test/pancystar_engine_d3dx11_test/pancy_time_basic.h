#pragma once
#include<windows.h>
#include<iostream>
class time_count
{
	double   count_freq;      //系统时钟频率
	double   delta_time;      //两帧之间时间差
	double   all_timeneed;    //总运行时间
	double   all_pause_time;  //暂停的总时间
	__int64  start_time;      //起始时间
	__int64  stop_time;       //停止时间
	__int64  now_time;        //当前时间
	__int64  last_time;       //上一帧的时间
	bool     if_stop;         //是否暂停
public:
	time_count();          //构造函数
	void reset();          //时间重置
	void start();          //开始计时
	void stop();           //暂停计时
	void refresh();        //刷新计时器
	float get_pause();     //获取总的暂停时间
	float get_delta();     //获取帧间时间差
	float get_alltime();   //获取总时间
};