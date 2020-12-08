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

#include "win32_NES_pad.h"
#include "debug.h"
#include "win32_directinput_input_mgr.h"
#include "OSD_ButtonSettings.h"


win32_NES_pad::win32_NES_pad(NES_controller_input_settings* settings, NES_pad* pad, win32_directinput_input_mgr* inp_mgr)
{
	m_pad = pad;
	inpm = inp_mgr;
	/*
	  m_ButtonUp = m_ButtonDown = m_ButtonLeft = m_ButtonRight =
	    m_ButtonSelect = m_ButtonStart = m_ButtonB = m_ButtonA = NULL;
	  m_ButtonTB = m_ButtonTA = NULL;
	*/
	csettings=settings;
	AutoFireToggleSW = settings->AutoFireToggle;
	AFBState[0] = AFBState[1] = 0;
	AFBPreState[0] = AFBPreState[1] = 0;
	tat=tbt=tac=tbc=0;
	CreateButtons(settings, inp_mgr);
}

win32_NES_pad::~win32_NES_pad()
{
	DeleteButtons();
}

void win32_NES_pad::Poll(float gt)
{
	m_pad->set_button_state(NES_UP,     inpm->Pressed(&m_ButtonUp));
	m_pad->set_button_state(NES_DOWN,   inpm->Pressed(&m_ButtonDown));
	m_pad->set_button_state(NES_LEFT,   inpm->Pressed(&m_ButtonLeft));
	m_pad->set_button_state(NES_RIGHT,  inpm->Pressed(&m_ButtonRight));
	m_pad->set_button_state(NES_SELECT, inpm->Pressed(&m_ButtonSelect));
	m_pad->set_button_state(NES_START,  inpm->Pressed(&m_ButtonStart));
	m_pad->set_button_state(NES_B,      inpm->Pressed(&m_ButtonB));
	m_pad->set_button_state(NES_A,      inpm->Pressed(&m_ButtonA));
	if(AutoFireToggleSW == 1){
		if(!AFBPreState[0] && inpm->Pressed(&m_ButtonTB)){
			AFBState[0] ^= 1;
		}
		AFBPreState[0] = inpm->Pressed(&m_ButtonTB);
	}
	else if(AutoFireToggleSW == 2){
		if(!AFBPreState[0] && inpm->Pressed(&m_ButtonTB)){
			AFBState[0] ^= 1;
		}
		AFBPreState[0] = inpm->Pressed(&m_ButtonTB);
		if(!AFBPreState[1] && inpm->Pressed(&m_ButtonTA)){
			AFBState[1] ^= 1;
		}
		AFBPreState[1] = inpm->Pressed(&m_ButtonTA);
	}
	TPoll(gt);
}

void win32_NES_pad::TPoll(float gt)
{

#if 1
	int flag=0, n=csettings->btnTBsec;
	int sw = inpm->Pressed(&m_ButtonB);
	if(n && ((AutoFireToggleSW == 1 && AFBState[0] && sw) ||
		(AutoFireToggleSW == 2 && AFBState[0] && sw )||
		(AutoFireToggleSW == 0 && inpm->Pressed(&m_ButtonTB)))){
		switch(n){
		case 30:
			if(tbc & 1) n=0;
			tbc++, flag++;
			break;
		case 20:
			if(tbc >= 3) n=0;
			tbc++, flag++;
			break;
		case 15:
			if(tbc >= 4) n=0;
			tbc++, flag++;
			break;
		case 10:
			if(tbc >= 6) n=0;
			tbc++, flag++;
			break;
		}
		if(flag){
			if(!n){
				m_pad->set_button_state(NES_B, NES_B);
				tbc=0;
			}
			else {
				m_pad->set_button_state(NES_B, 0);
			}
		}
		else if((gt-tbt) >= 1000 / n){
			m_pad->set_button_state(NES_B, NES_B);
			tbt=gt;
		}
	}
	else{
		tbt=tbc=0;
	}
	n=csettings->btnTAsec;
	flag=0;
	sw = inpm->Pressed(&m_ButtonA);
	if(n && (AutoFireToggleSW == 1 && AFBState[0] && sw) ||
		(AutoFireToggleSW == 2 && AFBState[1] && sw) ||
		(AutoFireToggleSW == 0 && inpm->Pressed(&m_ButtonTA))){
		switch(n){
		case 30:
			if(tac & 1) n=0;
			tac++, flag++;
			break;
		case 20:
			if(tac >= 3) n=0;
			tac++, flag++;
			break;
		case 15:
			if(tac >= 4) n=0;
			tac++, flag++;
			break;
		case 10:
			if(tac >= 6) n=0;
			tac++, flag++;
			break;
		}
		if(flag){
			if(!n){
				m_pad->set_button_state(NES_A, NES_A);
				tac=0;
			}
			else {
				m_pad->set_button_state(NES_A, 0);
			}
		}
		else if((gt-tat) >= 1000 / n){
			m_pad->set_button_state(NES_A, NES_A);
			tat=gt;
		}
	}
	else{
		tat=tac=0;
	}
#else
	if(m_ButtonTB->Pressed() && csettings->btnTBsec){
		if((gt-tbt) > 1000 / (csettings->btnTBsec*2)){
			bflag^=NES_B;
			m_pad->set_button_state(NES_B, (bflag & NES_B));
			tbt=gt;
		}
	}
	if(m_ButtonTA->Pressed() && csettings->btnTAsec){
		if((gt-tat) > 1000 / (csettings->btnTAsec*2)){
			bflag^=NES_A;
			m_pad->set_button_state(NES_A, (bflag & NES_A));
			tat=gt;
		}
	}
#endif
}


void win32_NES_pad::CreateButtons(NES_controller_input_settings* settings,
                                  win32_directinput_input_mgr* inp_mgr)
{
	DeleteButtons();
	inpm->Setos(&settings->btnUp, &m_ButtonUp);
	inpm->Setos(&settings->btnDown, &m_ButtonDown);
	inpm->Setos(&settings->btnLeft, &m_ButtonLeft);
	inpm->Setos(&settings->btnRight, &m_ButtonRight);
	inpm->Setos(&settings->btnSelect, &m_ButtonSelect);
	inpm->Setos(&settings->btnStart, &m_ButtonStart);
	inpm->Setos(&settings->btnB, &m_ButtonB);
	inpm->Setos(&settings->btnA, &m_ButtonA);
	inpm->Setos(&settings->btnTB, &m_ButtonTB);
	inpm->Setos(&settings->btnTA, &m_ButtonTA);
}


void win32_NES_pad::DeleteButtons()
{
	m_ButtonUp.type		= D_NONE;
	m_ButtonDown.type	= D_NONE;
	m_ButtonLeft.type	= D_NONE;
	m_ButtonRight.type	= D_NONE;
	m_ButtonSelect.type	= D_NONE;
	m_ButtonStart.type	= D_NONE;
	m_ButtonB.type		= D_NONE;
	m_ButtonA.type		= D_NONE;
	m_ButtonTB.type		= D_NONE;
	m_ButtonTA.type		= D_NONE;
}


