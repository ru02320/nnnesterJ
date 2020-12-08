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

#ifndef NES_SETTINGS_H_
#define NES_SETTINGS_H_

#include "types.h"

#include "OSD_NES_graphics_settings.h"
#include "OSD_ButtonSettings.h"
#include "nnnextkeycfg.h"

class NES_preferences_settings
{
public:
	boolean run_in_background;
	boolean SkipSomeErrors;
	boolean speed_throttling;
	boolean auto_frameskip;
	boolean ToggleFast;
	enum NES_PRIORITY { PRI_NORMAL=0, PRI_HIGH=1, PRI_REALTIME=2 };
	NES_PRIORITY priority;
	unsigned char	TV_Mode;  // 0 = Auto, 1 = NTSC, 2 = PAL
	unsigned char	TimeUsePCounter;

	unsigned char	NotUseSleep;
	unsigned char	NotUseFDSDiskASkip;
	unsigned char	JoystickUseAPI;
	unsigned char	JoystickUseGUID;

	boolean			UseRewind;
	unsigned int	RewindKeyFrame;
	unsigned int	RewindBFrame;
//	unsigned int	KailleraFrameInterval;

	unsigned char UseRomDataBase;
	unsigned char AutoRomCorrect;
	unsigned char AutoStateSL;
	unsigned char AutoStateSLNotload;
	unsigned char AllowMultiInstance;
	unsigned char ZipDLLUseFlag;
	unsigned char SaveFileToZip;
	unsigned char StateFileToZip;
	unsigned char DisableIPSPatch;
	unsigned char DisableMenuIcon;
	unsigned char CloseButtonExit;
	unsigned char ShowFPS;
	unsigned char KailleraChatWindow;

	unsigned char DisableGameInfoDisp;
	unsigned char DoubleClickFullScreen;

	unsigned char GM_WindowStyle_Title;
	unsigned char GM_WindowStyle_Menu;
	unsigned char GM_WindowStyle_SBar;

	uint32 nViewerReInterval;
	uint32 nPtnViewerReInterval;


	unsigned char	StdSpeedState;
	unsigned char	FastSpeedState;
	uint32		StdFPS;
	uint32		StdSkipFrame;
	uint32		FastFPS;
	uint32		FastSkipFrame;
	int	PreviewMode;


	void SetDefaults()
	{
		run_in_background = FALSE;
		SkipSomeErrors = BST_CHECKED;
		speed_throttling = TRUE;
		auto_frameskip = TRUE;
		priority = PRI_NORMAL;
		FastFPS = 120;
		ToggleFast = TRUE;
		UseRomDataBase=0;
		AutoRomCorrect=0;
		AutoStateSL=0;
		AutoStateSLNotload=0;
		AllowMultiInstance=0;
		ZipDLLUseFlag=0;
		StdSpeedState=0;
		FastSpeedState=0;
		StdFPS=0;
		StdSkipFrame=0;
		FastFPS=0;
		FastSkipFrame=3;
		NotUseSleep=0;
		NotUseFDSDiskASkip=0;
		JoystickUseAPI=0;
		SaveFileToZip=0;
		StateFileToZip=0;
		DisableIPSPatch=0;
		RewindKeyFrame=6;
		RewindBFrame=30;
//		KailleraFrameInterval=1;
		TV_Mode = 0;
		DisableMenuIcon = 1;
		CloseButtonExit = 0;
		PreviewMode = 0;
		TimeUsePCounter = 0;
		JoystickUseGUID =0;
		ShowFPS = 0;
		KailleraChatWindow = 1;

		GM_WindowStyle_Title =0;
		GM_WindowStyle_Menu = 0;
		GM_WindowStyle_SBar = 0;
		nPtnViewerReInterval = 10;
		nViewerReInterval = 2;
	}

	NES_preferences_settings()
	{
		SetDefaults();
	}
};

class NES_graphics_settings
{
public:
	boolean black_and_white;
	boolean show_more_than_8_sprites;
	boolean show_all_scanlines;
	boolean draw_overscan;
	boolean fullscreen_on_load;
	//  boolean fullscreen_scaling;
	uint8 fullscreen_scaling;
	boolean calculate_palette;
	boolean UseStretchBlt;
	boolean EmulateTVScanline;
	char szPaletteFile[260];
	uint8 tint;
	uint8 hue;
	OSD_NES_graphics_settings osd;

	uint8 WindowModeUseDdraw;
	uint8 WindowModeUseDdrawOverlay;
	uint8 FullscreenModeUseNewMode;
	uint8 FullscreenModeUseDefRefreshRate;
	uint8 FullscreenModeBackBuffM;
	uint8 WindowModeBackBuffM;
	uint8 FullscreenModeNotUseFlip;
	uint8 WindowDDMode_NotuseGDI;
	uint8 WindowMode_2xsai;
	uint8 FullscreenMode_2xsai;
	uint8 WindowMode_2xsaiType;
	uint8 FullscreenMode_2xsaiType;

	uint8 ScreenShotUseBmp;

	uint8 DisableSpriteClipping;
	uint8 DisableBackGClipping;


	void reset_palette()
	{
		tint = 0x86;
		hue  = 0x9d;
	}

	void SetDefaults()
	{
		black_and_white = FALSE;
		show_more_than_8_sprites = FALSE;
		show_all_scanlines = FALSE;
		draw_overscan = FALSE;
		fullscreen_on_load = FALSE;
		fullscreen_scaling = FALSE;
		calculate_palette = FALSE;
		UseStretchBlt = FALSE;
		EmulateTVScanline = FALSE;
		WindowModeUseDdraw=0;
		WindowModeUseDdrawOverlay=0;
		FullscreenModeUseNewMode=0;
		FullscreenModeUseDefRefreshRate=0;
		FullscreenModeBackBuffM=0;
		WindowModeBackBuffM=0;
		ScreenShotUseBmp=0;
		FullscreenModeNotUseFlip=0;
		*szPaletteFile = '\0';
		reset_palette();
		osd.Init();
		DisableSpriteClipping=0;
		DisableBackGClipping=0;
		WindowDDMode_NotuseGDI=0;
		WindowMode_2xsai = 0;
		WindowMode_2xsaiType= 0;
		FullscreenMode_2xsai= 0;
		FullscreenMode_2xsaiType= 0;
		
	}

	NES_graphics_settings()
	{
		SetDefaults();
	}
};

class NES_sound_settings
{
public:
	boolean enabled;
	uint32 sample_bits;
	uint32 sample_rate;

	boolean rectangle1_enabled;
	boolean rectangle2_enabled;
	boolean triangle_enabled;
	boolean noise_enabled;
	boolean dpcm_enabled;
	boolean ext_enabled;

	boolean ideal_triangle_enabled;
	boolean smooth_envelope_enabled; // reserved
	boolean smooth_sweep_enabled; // reserved


	float rectangle1_volumed;
	float rectangle2_volumed;
	float triangle_volumed;
	float noise_volumed;
	float dpcm_volumed;
	float ext_volumed;

	enum { LENGTH_MIN = 1, LENGTH_MAX = 10 };
	uint32 buffer_len;

	enum filter_type_t { FILTER_NONE, FILTER_LOWPASS, FILTER_LOWPASS_WEIGHTED, FILTER_OLOWPASS};
	filter_type_t filter_type;
	uint32 lowpass_filter_f;

	uint32 vrc6_type;
	uint32 vrc7_type;

	void SetDefaults()
	{
		enabled = TRUE;
		sample_bits = 8;
		sample_rate = 44100;
		buffer_len = 3;

		filter_type = FILTER_LOWPASS_WEIGHTED;
		lowpass_filter_f = 15000;

		rectangle1_enabled = TRUE;
		rectangle2_enabled = TRUE;
		triangle_enabled = TRUE;
		noise_enabled = TRUE;
		dpcm_enabled = TRUE;
		ext_enabled = TRUE;

		ideal_triangle_enabled = FALSE;
		smooth_envelope_enabled = FALSE;
		smooth_sweep_enabled = FALSE;

		rectangle1_volumed=0;
		rectangle2_volumed=0;
		triangle_volumed=0;
		noise_volumed=0;
		dpcm_volumed=0;
		ext_volumed=0;
		vrc6_type = 0;
		vrc7_type = 0;
	}

	NES_sound_settings()
	{
		SetDefaults();
	}
};

class NES_controller_input_settings
{
public:
	OSD_ButtonSettings btnUp;
	OSD_ButtonSettings btnDown;
	OSD_ButtonSettings btnLeft;
	OSD_ButtonSettings btnRight;
	OSD_ButtonSettings btnSelect;
	OSD_ButtonSettings btnStart;
	OSD_ButtonSettings btnB;
	OSD_ButtonSettings btnA;

	OSD_ButtonSettings btnTB;	//nnn Auto fire button
	OSD_ButtonSettings btnTA;
	int btnTBsec;
	int btnTAsec;

	int AutoFireToggle;


	// OS-specific
	void OSD_SetDefaults(int num); // 0 == first player

	void Clear()
	{
		btnUp.type= D_NONE;
		btnDown.type= D_NONE;
		btnLeft.type= D_NONE;
		btnRight.type= D_NONE;
		btnSelect.type= D_NONE;
		btnStart.type= D_NONE;
		btnB.type= D_NONE;
		btnA.type= D_NONE;
		btnTB.type= D_NONE;
		btnTA.type= D_NONE;
		AutoFireToggle = 0;
	}

	NES_controller_input_settings(int num)
	{
		Clear();
		//    OSD_SetDefaults(num);
	}
};

class NES_input_settings
{
public:
	NES_controller_input_settings player1;
	NES_controller_input_settings player2;
	NES_controller_input_settings player3;
	NES_controller_input_settings player4;

	OSD_ButtonSettings MicButton;
	OSD_ButtonSettings extkeycfg[EXTKEYALLNUM];		//nnn
	void OSD_EKey_SetDefaults();
	//  void OSD_EKey_SetDefaults2();
	void ExtBtnClear()
	{
		for(int i=0; i<EXTKEYALLNUM; ++i){
			extkeycfg[i].type= D_NONE;
		}
	}
	unsigned char extkeycfgFastFPSToggle;

	void SetDefaults()
	{
		player1.Clear();
		player2.Clear();
		player3.Clear();
		player4.Clear();
		MicButton.type = D_KEYBOARD_KEY;
		MicButton.j_offset = 0x32;
		player1.OSD_SetDefaults(0);
		player2.OSD_SetDefaults(1);
		player3.OSD_SetDefaults(2);
		player4.OSD_SetDefaults(3);
		ExtBtnClear();
		OSD_EKey_SetDefaults();
		//	OSD_EKey_SetDefaults2();
	}

	NES_input_settings() : player1(0), player2(1), player3(2), player4(3)
	{
		ExtBtnClear();
		OSD_EKey_SetDefaults();
		//	OSD_EKey_SetDefaults2();
	}
};

class NES_settings
{
public:
	NES_preferences_settings  preferences;
	NES_graphics_settings     graphics;
	NES_sound_settings        sound;
	NES_input_settings        input;


	NES_settings() : preferences(), graphics(), sound(), input()
	{
	}
};


#endif

