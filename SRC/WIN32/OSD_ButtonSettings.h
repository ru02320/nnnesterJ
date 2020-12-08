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

#ifndef _OSD_BUTTONSETTINGS_H_
#define _OSD_BUTTONSETTINGS_H_

// win32 button settings

#include "types.h"
#include "debug.h"

#include <windows.h>
#include <dinput.h>


#define	D_NONE			0
#define	D_KEYBOARD_KEY	1
#define	D_JOYSTICK_BUTTON	2
#define	D_JOYSTICK_AXIS		3



struct OSD_ButtonSettings
{
	uint32 type;
	uint32 dev_n;
	uint32 j_offset;
	uint32 j_axispositive;
	GUID	DIJoyGUID;
	//  uint8 key;
};

#endif
