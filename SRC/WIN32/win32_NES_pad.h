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

#ifndef WIN32_NES_PAD_H_
#define WIN32_NES_PAD_H_

#include "NES_pad.h"
#include "NES_settings.h"
#include "win32_directinput_input_mgr.h"
//#include "INPButton.h"
#include <windows.h>
#include <dinput.h>



// ack. ptooie.

class win32_NES_pad
{
public:
	win32_NES_pad(NES_controller_input_settings* settings, NES_pad* pad, win32_directinput_input_mgr* inp_mgr);
	~win32_NES_pad();

	void Poll(float);

	void TPoll(float);
	NES_controller_input_settings* csettings;
protected:
	NES_pad* m_pad;
	win32_directinput_input_mgr *inpm;

	OSD_ButtonSettings m_ButtonUp;
	OSD_ButtonSettings m_ButtonDown;
	OSD_ButtonSettings m_ButtonLeft;
	OSD_ButtonSettings m_ButtonRight;
	OSD_ButtonSettings m_ButtonSelect;
	OSD_ButtonSettings m_ButtonStart;
	OSD_ButtonSettings m_ButtonB;
	OSD_ButtonSettings m_ButtonA;
	OSD_ButtonSettings m_ButtonTB;		//nnn auto fire button
	OSD_ButtonSettings m_ButtonTA;

	//  INPButton* CreateButton(OSD_ButtonSettings* settings, win32_directinput_input_mgr* inp_mgr);

	unsigned char tac, tbc;
	float tat,tbt;
	int AutoFireToggleSW;
	int AFBPreState[2];
	int AFBState[2];

	void CreateButtons(NES_controller_input_settings* settings, win32_directinput_input_mgr* inp_mgr);
	void DeleteButtons();

private:
};

#endif
