/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#include "win32_directinput_input_mgr.h"
//#include "iDirectX.h"
#include "debug.h"
#include "win32_globals.h"
#include "settings.h"
#include "OSD_ButtonSettings.h"


static int JoyDevNum;
static LPDIRECTINPUTDEVICE7	lpDIKeyDev;
static LPDIRECTINPUTDEVICE7	lpDIJoyDev[MAX_JOYSTICKS];
//static LPDIDEVICEINSTANCE	lpDIKeydi;
//static LPDIDEVICEINSTANCE	lpDIJoydi[4];
//static GUID DIKeyGUID;
static GUID DIJoyGUID[MAX_JOYSTICKS];

static HWND thWnd;

static BOOL CALLBACK InitJoystickInput(LPCDIDEVICEINSTANCE pdInst,LPVOID pvRef){
	LPDIRECTINPUT7 pdi=(LPDIRECTINPUT7)pvRef;
	DIPROPRANGE di;

	if(JoyDevNum>=MAX_JOYSTICKS)
		return DIENUM_STOP;
	if(pdi->CreateDeviceEx(pdInst->guidInstance, IID_IDirectInputDevice7, (void **)&lpDIJoyDev[JoyDevNum],NULL)!=DI_OK){
		lpDIJoyDev[JoyDevNum]=NULL;
		return DIENUM_CONTINUE;
	}
	if(lpDIJoyDev[JoyDevNum]->SetDataFormat(&c_dfDIJoystick)!=DI_OK){
		lpDIJoyDev[JoyDevNum]->Release();
		lpDIJoyDev[JoyDevNum]=NULL;
		return DIENUM_CONTINUE;
	}
	if(lpDIJoyDev[JoyDevNum]->SetCooperativeLevel(thWnd, DISCL_BACKGROUND/*DISCL_FOREGROUND*/ | DISCL_NONEXCLUSIVE)!=DI_OK){
		lpDIJoyDev[JoyDevNum]->Release();
		lpDIJoyDev[JoyDevNum]=NULL;
		return DIENUM_CONTINUE;
	}

	di.diph.dwSize=sizeof(DIPROPRANGE);
	di.diph.dwHeaderSize=sizeof(di.diph);
	di.diph.dwObj=DIJOFS_X;
	di.diph.dwHow=DIPH_BYOFFSET;
	di.lMin=-AXIS_MAX;
	di.lMax=AXIS_MAX;
	lpDIJoyDev[JoyDevNum]->SetProperty(DIPROP_RANGE,&di.diph);

	di.diph.dwObj=DIJOFS_Y;
	lpDIJoyDev[JoyDevNum]->SetProperty(DIPROP_RANGE,&di.diph);

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	dipdw.diph.dwHow        = DIPH_BYOFFSET;
	dipdw.dwData            = 0x2000;
	dipdw.diph.dwObj         = DIJOFS_X;
	lpDIJoyDev[JoyDevNum]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
	dipdw.diph.dwObj         = DIJOFS_Y;
	lpDIJoyDev[JoyDevNum]->SetProperty(DIPROP_DEADZONE, &dipdw.diph);

	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = DIPROPAXISMODE_ABS;
	dipdw.diph.dwObj        = 0;
	lpDIJoyDev[JoyDevNum]->SetProperty(DIPROP_AXISMODE, &dipdw.diph);
/*
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = DIPROPAUTOCENTER_ON;
	dipdw.diph.dwObj        = 0;
	lpDIJoyDev[JoyDevNum]->SetProperty(DIPROP_AUTOCENTER, &dipdw.diph);
*/
	memcpy(&DIJoyGUID[JoyDevNum], &pdInst->guidInstance, sizeof(GUID));
	++JoyDevNum;
	return DIENUM_CONTINUE;
}


static BOOL CALLBACK InitKeybordInput(LPCDIDEVICEINSTANCE pdInst,LPVOID pvRef){
	LPDIRECTINPUT7 pdi=(LPDIRECTINPUT7)pvRef;

	if(pdi->CreateDeviceEx(pdInst->guidInstance, IID_IDirectInputDevice7, (void **)&lpDIKeyDev,NULL)!=DI_OK){
		lpDIKeyDev=NULL;
		return DIENUM_CONTINUE;
	}
	if(lpDIKeyDev->SetDataFormat(&c_dfDIKeyboard)!=DI_OK){
		lpDIKeyDev->Release();
		lpDIKeyDev=NULL;
		return DIENUM_CONTINUE;
	}
	if(lpDIKeyDev->SetCooperativeLevel(thWnd, DISCL_BACKGROUND /*DISCL_FOREGROUND*/|DISCL_NONEXCLUSIVE)!=DI_OK){
		lpDIKeyDev->Release();
		lpDIKeyDev=NULL;
		return DIENUM_CONTINUE;
	}
//	memcpy(&DIKeyGUID, &pdInst->guidInstance, sizeof(GUID));
	return DIENUM_STOP;
}

/////////////////////////////
//// WIN32 INPUT MANAGER ////
/////////////////////////////

win32_directinput_input_mgr::win32_directinput_input_mgr(HWND hWnd, HINSTANCE hInstance)
{
	lpDI = NULL;
	JoyDevNum=0;
	lpDIKeyDev=NULL;
	for(int i=0;i<MAX_JOYSTICKS;++i){
		lpDIJoyDev[i]=NULL;
	}
	AcquireFlag=FALSE;
	m_nJoystickDevice=0;
	m_hWnd = thWnd = hWnd;
	m_hInst = hInstance;

	m_UseAPI	= NESTER_settings.nes.preferences.JoystickUseAPI;
	m_UseJoystickGUID = NESTER_settings.nes.preferences.JoystickUseGUID;

	if(FAILED(DirectInputCreateEx(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput7, (void **)&lpDI, NULL))){
		throw "Error creating DirectInput interface";
	}
	InitKeyboard();
	InitJoystick();
	m_nJoystickDevice=JoyDevNum;
	AcquireDevice();
}

win32_directinput_input_mgr::~win32_directinput_input_mgr()
{
	if(lpDIKeyDev!=NULL){
		lpDIKeyDev->Unacquire();
		lpDIKeyDev->Release();
		lpDIKeyDev=NULL;
	}
	for(int i=0; i<4; ++i){
		if(lpDIJoyDev[i]!=NULL){
			lpDIJoyDev[i]->Unacquire();
			lpDIJoyDev[i]->Release();
			lpDIJoyDev[i]=NULL;
		}
	}
	if(lpDI){
		lpDI->Release();
		lpDI=NULL;
	}
}

void win32_directinput_input_mgr::Poll()
{
	HRESULT ret;
	if(lpDIKeyDev){
		lpDIKeyDev->Poll();
		ret=lpDIKeyDev->GetDeviceState(sizeof(m_diks),&m_diks);
		if(ret==DIERR_INPUTLOST){
			ret=lpDIKeyDev->Acquire();
			if(ret==DI_OK)
				ret=lpDIKeyDev->GetDeviceState(sizeof(m_diks),&m_diks);
		}
	}
	if(m_UseAPI){
		JOYINFOEX JoyInfo;
		JOYCAPS jc;
		int jn=0;
		for(int i=0;i<2;++i){
			memset(&m_js[jn], NULL, sizeof(DIJOYSTATE));
			memset(&JoyInfo, NULL, sizeof(JOYINFOEX));
			JoyInfo.dwSize = sizeof(JOYINFOEX);
			JoyInfo.dwFlags= JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
			if(JOYERR_NOERROR != joyGetPosEx(i, &JoyInfo)){
				continue;
			}

			for(int j=0, bit=1;j<16;++j,bit<<=1){
				if(bit&JoyInfo.dwButtons)
					m_js[jn].rgbButtons[j] = 0x80;
			}
			memset(&jc, 0, sizeof(JOYCAPS));
			if(JOYERR_NOERROR != joyGetDevCaps(i, &jc, sizeof(JOYCAPS)))
				continue;

			{
				unsigned int xdef=(jc.wXmin+jc.wXmax)/2;
				unsigned int xt=xdef>>1;
				unsigned int ydef=(jc.wYmin+jc.wYmax)/2;
				unsigned int yt=xdef>>1;
				if(JoyInfo.dwXpos > xdef+xt){
					m_js[i].lX = AXIS_MAX;
				}
				else if(JoyInfo.dwXpos < xdef-xt){
					m_js[i].lX = -AXIS_MAX;
				}
				if(JoyInfo.dwYpos > ydef+yt){
					m_js[i].lY = AXIS_MAX;
				}
				else if(JoyInfo.dwYpos < ydef-yt){
					m_js[i].lY = -AXIS_MAX;
				}
			}
			++jn;
		}
		m_nJoystickDevice=jn;
	}
	else{
		for(int i=0;i<4&&lpDIJoyDev[i];++i){
			lpDIJoyDev[i]->Poll();
			ret=lpDIJoyDev[i]->GetDeviceState(sizeof(DIJOYSTATE),&m_js[i]);
			if(ret==DIERR_INPUTLOST){
				ret=lpDIJoyDev[i]->Acquire();
				if(ret==DI_OK)
					ret=lpDIJoyDev[i]->GetDeviceState(sizeof(DIJOYSTATE),&m_js[i]);
			}
		}
	}
}

void win32_directinput_input_mgr::AcquireDevice()
{
	HRESULT ret;
	if(AcquireFlag)
		return;
	if(lpDIKeyDev){
		ret = lpDIKeyDev->Acquire();
		if(ret==DIERR_INPUTLOST){
			lpDIKeyDev->Acquire();
		}
	}
	for(int i=0;i<4;++i){
		if(lpDIJoyDev[i]){
			ret = lpDIJoyDev[i]->Acquire();
			if(ret==DIERR_INPUTLOST){
				lpDIJoyDev[i]->Acquire();
			}
		}
	}
	AcquireFlag = TRUE;
}


void win32_directinput_input_mgr::UnacquireDevice(){
	AcquireFlag=FALSE;
	return;
	if(lpDIKeyDev)
		lpDIKeyDev->Unacquire();
	if(lpDIJoyDev[0])
		lpDIJoyDev[0]->Unacquire();
	if(lpDIJoyDev[1])
		lpDIJoyDev[1]->Unacquire();
	if(lpDIJoyDev[2])
		lpDIJoyDev[2]->Unacquire();
	if(lpDIJoyDev[3])
		lpDIJoyDev[3]->Unacquire();
}


BOOL win32_directinput_input_mgr::InitJoystick(){
	HRESULT ret;
	ret=lpDI->EnumDevices(DIDEVTYPE_JOYSTICK,InitJoystickInput,lpDI,DIEDFL_ATTACHEDONLY);
	if(ret!=DI_OK)
		return FALSE;
	return TRUE;
}


BOOL win32_directinput_input_mgr::InitKeyboard(){
	HRESULT ret;
	ret=lpDI->EnumDevices(DIDEVTYPE_KEYBOARD,InitKeybordInput,lpDI,DIEDFL_ATTACHEDONLY);
	if(ret!=DI_OK)
		return FALSE;
	return TRUE;
}


void win32_directinput_input_mgr::Setos(OSD_ButtonSettings *obs, OSD_ButtonSettings *obs2){
	switch(obs->type){
	case D_KEYBOARD_KEY:
		if(lpDIKeyDev){
			memcpy(obs2, obs, sizeof(OSD_ButtonSettings));
			return;
		}
		break;
	case D_JOYSTICK_BUTTON:
	case D_JOYSTICK_AXIS:
		if(m_UseAPI || (!m_UseJoystickGUID && !m_UseAPI)){
			if(obs->dev_n < (unsigned int)m_nJoystickDevice){
				memcpy(obs2, obs, sizeof(OSD_ButtonSettings));
				return;
			}
		}
		else{
			int i;
			for(i=0; i<m_nJoystickDevice; i++){
				if(!memcmp(&obs->DIJoyGUID, &DIJoyGUID[i], sizeof(GUID))){
					memcpy(obs2, obs, sizeof(OSD_ButtonSettings));
					obs2->dev_n = i;
					return;
				}
			}
		}
		break;
	}
	obs2->type=D_NONE;
}


BOOL win32_directinput_input_mgr::Pressed(OSD_ButtonSettings *dii){
	//	if(!AcquireFlag)
	//		return FALSE;
	switch(dii->type){
		/*		case D_NONE:
					SetNone();
					break;
		*/
	case D_KEYBOARD_KEY:
		return (BOOL)m_diks[dii->j_offset];
	case D_JOYSTICK_BUTTON:
		{
			return (BOOL)m_js[dii->dev_n].rgbButtons[dii->j_offset];
		}
	case D_JOYSTICK_AXIS:
		{
			if(dii->j_axispositive){
				if(dii->j_offset){
					if((m_js[dii->dev_n].lX) > 0x8000)
						return TRUE;
				}
				else{
					if((m_js[dii->dev_n].lY) > 0x8000)
						return TRUE;
				}
			}
			else{
				if(dii->j_offset){
					if((m_js[dii->dev_n].lX) < -0x8000)
						return TRUE;
				}
				else{
					if((m_js[dii->dev_n].lY) < -0x8000)
						return TRUE;
				}
			}
		}
	}
	return FALSE;
}


void win32_directinput_input_mgr::ScanInputDevice(OSD_ButtonSettings *Dii){
	int i,j;

	Dii->type = D_NONE;
	//	if(!AcquireFlag){
	//		return;
	//	}
	memset(m_diks, 0, sizeof(m_diks));
	memset(m_js, 0, sizeof(m_js));
	Poll();
	if(lpDIKeyDev){
		for(i=0;i<256;++i){
			if(m_diks[i]){
				if(i==0x94 || i==0x3a)
					continue;
				Dii->type = D_KEYBOARD_KEY;
				Dii->dev_n = 0;
				Dii->j_offset = i;
				return;
			}
		}
	}
	for(i=0;i<m_nJoystickDevice;++i){
		for(j=0;j<32;++j){
			if(m_js[i].rgbButtons[j]){
				Dii->type = D_JOYSTICK_BUTTON;
				Dii->dev_n = i;
				Dii->j_offset  = j;
				memcpy(&Dii->DIJoyGUID, &DIJoyGUID[i], sizeof(GUID));
				return;
			}
		}
		if(m_js[i].lX > 0x8000){
			Dii->type = D_JOYSTICK_AXIS;
			Dii->dev_n = i;
			Dii->j_offset  = 4;
			Dii->j_axispositive = 1;
			memcpy(&Dii->DIJoyGUID, &DIJoyGUID[i], sizeof(GUID));
			return;
		}
		if(m_js[i].lX < -0x8000){
			Dii->type = D_JOYSTICK_AXIS;
			Dii->dev_n = i;
			Dii->j_offset  = 4;
			Dii->j_axispositive = 0;
			memcpy(&Dii->DIJoyGUID, &DIJoyGUID[i], sizeof(GUID));
			return;
		}
		if(m_js[i].lY > 0x8000){
			Dii->type = D_JOYSTICK_AXIS;
			Dii->dev_n = i;
			Dii->j_offset  = 0;
			Dii->j_axispositive = 1;
			memcpy(&Dii->DIJoyGUID, &DIJoyGUID[i], sizeof(GUID));
			return;
		}
		if(m_js[i].lY < -0x8000){
			Dii->type = D_JOYSTICK_AXIS;
			Dii->dev_n = i;
			Dii->j_offset  = 0;
			Dii->j_axispositive = 0;
			memcpy(&Dii->DIJoyGUID, &DIJoyGUID[i], sizeof(GUID));
			return;
		}
	}
}


int win32_directinput_input_mgr::GetJoystickState(int nJoy, DIJOYSTATE **JoyState){
	if(nJoy >= m_nJoystickDevice)
		return 0;
	Poll();
	*JoyState = &m_js[nJoy];
	return 1;
}
