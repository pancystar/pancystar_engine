#include"PancyInput.h"
pancy_input::pancy_input(HWND hwnd, ID3D11Device *d3d_device,HINSTANCE hinst)
{
	pancy_d3ddevice = d3d_device;//获取D3D设备接口
	pancy_dinput = NULL;
	DirectInput8Create(hinst,DIRECTINPUT_HEADER_VERSION,IID_IDirectInput8,(void**)&pancy_dinput,NULL);//获取DirectInput设备
	dinput_clear(hwnd,(DISCL_FOREGROUND | DISCL_NONEXCLUSIVE),(DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));//创建键盘及鼠标
}
pancy_input::~pancy_input()
{
	if(dinput_keyboard != NULL)
	{
		dinput_keyboard->Unacquire();
		dinput_keyboard->Release();
	}
	if(dinput_mouse != NULL)
	{
		dinput_mouse->Unacquire();
		dinput_mouse->Release();
	}
	if(pancy_dinput != NULL)
	{
		pancy_dinput->Release();
	}
}
void pancy_input::dinput_clear(HWND hwnd,DWORD keyboardCoopFlags, DWORD mouseCoopFlags)
{
	//创建键盘设备
	pancy_dinput->CreateDevice(GUID_SysKeyboard,&dinput_keyboard,NULL);
	dinput_keyboard->SetDataFormat(&c_dfDIKeyboard);//设置设备的数据格式
	dinput_keyboard->SetCooperativeLevel(hwnd,keyboardCoopFlags);//设置设备的独占等级
	dinput_keyboard->Acquire();//获取设备的控制权
	dinput_keyboard->Poll();//设置轮询
	//创建鼠标设备
	pancy_dinput->CreateDevice(GUID_SysMouse,&dinput_mouse,NULL);
	dinput_mouse->SetDataFormat(&c_dfDIMouse);//设置设备的数据格式
	dinput_mouse->SetCooperativeLevel(hwnd,mouseCoopFlags);//设置设备的独占等级
	dinput_mouse->Acquire();//获取设备的控制权
	dinput_mouse->Poll();//设置轮询
}
void pancy_input::get_input()
{
	//获取鼠标消息
	ZeroMemory(&mouse_buffer,sizeof(mouse_buffer));
	while(true)
	{
		dinput_mouse->Poll();
		dinput_mouse->Acquire();
		HRESULT hr;
		if(SUCCEEDED(dinput_mouse->GetDeviceState(sizeof(mouse_buffer),(LPVOID)&mouse_buffer)))
		{
			break;
		}
		else
		{
			hr = dinput_mouse->GetDeviceState(sizeof(mouse_buffer),(LPVOID)&mouse_buffer);
		}
		if (hr != DIERR_INPUTLOST || hr != DIERR_NOTACQUIRED)
		{
			break;
		};
		if (FAILED(dinput_mouse->Acquire()))
		{
			break;
		};
	}
	//获取键盘消息
	ZeroMemory(&key_buffer,sizeof(key_buffer));
	while(true)
	{
		dinput_keyboard->Poll();
		dinput_keyboard->Acquire();
		HRESULT hr;
		if(SUCCEEDED(dinput_keyboard->GetDeviceState(sizeof(key_buffer),(LPVOID)&key_buffer)))
		{
			break;
		}
		else
		{
			hr = dinput_keyboard->GetDeviceState(sizeof(key_buffer),(LPVOID)&key_buffer);
		}
		if (hr != DIERR_INPUTLOST || hr != DIERR_NOTACQUIRED)
		{
			break;
		};
		if (FAILED(dinput_keyboard->Acquire()))
		{
			break;
		};
	}
	
}
bool pancy_input::check_keyboard(int key_value)
{
	if(key_buffer[key_value] & 0x80)
	{
		return true;
	}
	return false;
}
bool pancy_input::check_mouseDown(int mouse_value)
{
	if((mouse_buffer.rgbButtons[mouse_value]&0x80) != 0)
	{
		return true;
	}
	return false;
}
float pancy_input::MouseMove_X()
{
	return (float)mouse_buffer.lX;
}
float pancy_input::MouseMove_Y()
{
	return (float)mouse_buffer.lY;
}
float pancy_input::MouseMove_Z()
{
	return (float)mouse_buffer.lZ;
}