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

#ifndef _WIN32_DIRECTINPUT_INPUT_MGR_H_
#define _WIN32_DIRECTINPUT_INPUT_MGR_H_
#define DIRECTINPUT_VERSION	0x0700

#include <windows.h>
#include <windowsx.h>
#include <dinput.h>

#include "types.h"
//#include "input_mgr.h"
//#include "iDIDevice.h"
#include "OSD_ButtonSettings.h"


/////////////////////////////
//// WIN32 INPUT MANAGER ////
/////////////////////////////

#define MAX_JOYSTICKS 8
#define AXIS_MAX 0x10000


class win32_directinput_input_mgr
{
public:

	win32_directinput_input_mgr(HWND hWnd, HINSTANCE hInstance);
	~win32_directinput_input_mgr();

	void Poll();

	void AcquireDevice();
	void UnacquireDevice();

	void ScanInputDevice(OSD_ButtonSettings *);

	void Setos(OSD_ButtonSettings *, OSD_ButtonSettings *);
	BOOL Pressed(OSD_ButtonSettings *);
	int GetJoystickn(){ return m_nJoystickDevice; };
	int GetJoystickState(int nJoy, DIJOYSTATE **JoyState);

protected:
	HWND m_hWnd;
	HINSTANCE m_hInst;

	DIJOYSTATE m_js[MAX_JOYSTICKS];
	BYTE m_diks[256];
	LPDIRECTINPUT7 lpDI;
	int m_nJoystickDevice;
	BOOL InitKeyboard();
	BOOL InitJoystick();
	BOOL AcquireFlag;

	int m_UseAPI;
	int m_UseJoystickGUID;
};

#endif
