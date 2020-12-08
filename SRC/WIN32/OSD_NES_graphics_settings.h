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

#ifndef OSD_NES_GRAPHICS_SETTINGS_H_
#define OSD_NES_GRAPHICS_SETTINGS_H_

#include "types.h"
#include <windows.h>

class OSD_NES_graphics_settings
{
public:
	GUID device_GUID;
	uint32 fullscreen_width;
	uint32 fullscreen_height;
	float zoom_size;
	unsigned char DtvSize;
	unsigned char InfoStatusBar;
	unsigned char WindowTopMost;
	unsigned char fullscreen_BitDepth;
	unsigned char next_avirec;
	unsigned char wav_avirec;

	int WindowPos_x;
	int WindowPos_y;

	void Init()
	{
		memset(&device_GUID, 0x00, sizeof(GUID));
		fullscreen_width = 320;
		fullscreen_height= 240;
		zoom_size = 1;
		DtvSize=0;
		InfoStatusBar=0;
		WindowTopMost=0;
		fullscreen_BitDepth=8;
		next_avirec=0;
		wav_avirec=0;
		WindowPos_x = 0;
		WindowPos_y = 0;
	}

protected:
private:
};

#endif
