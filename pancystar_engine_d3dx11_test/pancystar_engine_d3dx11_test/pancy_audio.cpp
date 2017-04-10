#include"pancy_audio.h"
FMOD_basic::FMOD_basic()
{
}
HRESULT FMOD_basic::init_system()
{
	result = FMOD::System_Create(&system);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	unsigned int version;
	result = system->getVersion(&version);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	if (version < FMOD_VERSION)
	{
		printf("当前计算机的库文件对应版本%08x 与本程序所使用版本 %08x 不符合，请检验安装的库文件", version, FMOD_VERSION);
		return E_FAIL;
	}
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	result = system->init(700, FMOD_INIT_NORMAL, 0);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	result = system->set3DSettings(1.0f, 1.0f, 0.3f);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	return S_OK;
}
HRESULT FMOD_basic::load_music_from_memory(char* data_mem, int datalength)
{
	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.length = datalength;
	exinfo.format = FMOD_SOUND_FORMAT_PCM32;
	exinfo.fileoffset = 0;
	exinfo.numchannels = 2;
	exinfo.defaultfrequency = 48000;

	sound_list now_sounds;
	strcpy(now_sounds.file_name, "memory_music");
	now_sounds.music_ID = sounds.size();
	result = system->createSound(data_mem, FMOD_OPENMEMORY | FMOD_OPENRAW | FMOD_3D, &exinfo, &now_sounds.sounds);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	result = now_sounds.sounds->set3DMinMaxDistance(0.5f, 5000.0f);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	result = now_sounds.sounds->setMode(FMOD_LOOP_OFF);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	sounds.push_back(now_sounds);
	return S_OK;
}
HRESULT FMOD_basic::load_music(char music_name[], FMOD_CREATESOUNDEXINFO *exinfo, FMOD_MODE mode, int &sound_ID)
{
	sound_list now_sounds;
	strcpy(now_sounds.file_name, music_name);
	now_sounds.music_ID = sounds.size();
	sound_ID = now_sounds.music_ID;
	result = system->createSound(music_name, mode, exinfo, &now_sounds.sounds);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	result = now_sounds.sounds->set3DMinMaxDistance(0.5f, 5000.0f);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	result = now_sounds.sounds->setMode(FMOD_LOOP_NORMAL);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	sounds.push_back(now_sounds);
	return S_OK;
}
HRESULT FMOD_basic::main_update()
{
	result = system->update();
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	return S_OK;
}
HRESULT FMOD_basic::pause_sound(int channel_ID)
{
	result = channels[channel_ID]->setPaused(true);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	return S_OK;
}
HRESULT FMOD_basic::start_sound(int channel_ID)
{
	result = channels[channel_ID]->setPaused(false);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	return S_OK;
}
bool FMOD_basic::check_if_virtual(int channel_ID)
{
	bool check_data;
	channels[channel_ID]->isVirtual(&check_data);
	float volume;
	channels[channel_ID]->getVolume(&volume);
	return check_data;
}
HRESULT  FMOD_basic::set_value(int channel_ID, float value)
{
	result = channels[channel_ID]->setVolume(value);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	return S_OK;
}
HRESULT FMOD_basic::play_sound(int sound_ID, int channel_ID)
{
	result = system->playSound(sounds[sound_ID].sounds, 0, true, &channels[channel_ID]);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	result = channels[channel_ID]->setPaused(false);

	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	return S_OK;
}
HRESULT FMOD_basic::play_sound_single(int sound_ID, int channel_ID)
{
	
	result = system->playSound(sounds[sound_ID].sounds, 0, true, &channels[channel_ID]);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	result = channels[channel_ID]->setPaused(false);
	channels[channel_ID]->setLoopCount(0);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	return S_OK;
}
HRESULT FMOD_basic::set_lisener(XMFLOAT3 location, XMFLOAT3 forward, XMFLOAT3 up, XMFLOAT3 speed)
{
	FMOD_VECTOR listenerpos, vel, forward_need, up_need;
	listenerpos.x = location.x;
	vel.x = speed.x;
	forward_need.x = forward.x;
	up_need.x = up.x;

	listenerpos.y = location.y;
	vel.y = speed.y;
	forward_need.y = forward.y;
	up_need.y = up.y;

	listenerpos.z = location.z;
	vel.z = speed.z;
	forward_need.z = forward.z;
	up_need.z = up.z;

	result = system->set3DListenerAttributes(0, &listenerpos, &vel, &forward_need, &up_need);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	return S_OK;
}
HRESULT FMOD_basic::set_music_place(int channel_ID, XMFLOAT3 location, XMFLOAT3 speed)
{
	FMOD_VECTOR pos, vel;
	pos.x = location.x;
	vel.x = speed.x;

	pos.y = location.y;
	vel.y = speed.x;

	pos.z = location.z;
	vel.z = speed.x;

	result = channels[channel_ID]->set3DAttributes(&pos, &vel);
	if (result != FMOD_OK)
	{
		return E_FAIL;
	}
	return S_OK;
}
void FMOD_basic::set_sound_priority(int channel_ID, int priority)
{
	channels[channel_ID]->setPriority(priority);
}


