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

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>

#include <stdio.h>
#include "debug.h"
#include "settings.h"

// this function sets the OS-dependent input setting defaults
#include <dinput.h>
#include "OSD_ButtonSettings.h"
//#include "win32_default_controls.h"
#include "nnnextkeycfg.h"

extern LOGFONT CheatDlgFont;
extern DWORD CheatDlgFontColor;




// KEY NAMES
#define NESTER_KEY_NAME           "NNNesterJ"
#define NES_KEY_NAME              "NES"
#define NES_PREFERENCES_KEY_NAME  "Preferences"
#define NES_GRAPHICS_KEY_NAME     "Graphics"
#define NES_SOUND_KEY_NAME        "Sound"
#define NES_INPUT_KEY_NAME        "Input"
#define NES_INPUT_CNT1_KEY_NAME   "Controller1"
#define NES_INPUT_CNT2_KEY_NAME   "Controller2"
#define NES_INPUT_CNT3_KEY_NAME   "Controller3"
#define NES_INPUT_CNT4_KEY_NAME   "Controller4"
#define NES_INPUT_UP_KEY_NAME     "Up"
#define NES_INPUT_DOWN_KEY_NAME   "Down"
#define NES_INPUT_LEFT_KEY_NAME   "Left"
#define NES_INPUT_RIGHT_KEY_NAME  "Right"
#define NES_INPUT_SELECT_KEY_NAME "Select"
#define NES_INPUT_START_KEY_NAME  "Start"
#define NES_INPUT_B_KEY_NAME      "B"
#define NES_INPUT_A_KEY_NAME      "A"
#define RECENT_KEY_NAME           "Recent"

#define NES_INPUT_TB_KEY_NAME      "TB"			//NNN
#define NES_INPUT_TA_KEY_NAME      "TA"
#define NES_INPUT_TBPS_KEY_NAME    "TurboPSecB"
#define NES_INPUT_TAPS_KEY_NAME    "TurboPSecA"
#define NES_EXT_INPUT_KEY_NAME     "ExtInput"
#define NES_INPUT_MIC_KEY_NAME     "MIC"
//#define NES_EXT_INPUT_BTN_KEY_NAME     "EXTBTN"


// VALUE NAMES
#define NES_PREFERENCES_RUNINBACKGROUND_VALUE_NAME      "RunInBackground"
#define NES_PREFERENCES_SKIPSOMEERRORS_VALUE_NAME		"SkipSomeErrors"
#define NES_PREFERENCES_SPEEDTHROTTLE_VALUE_NAME        "SpeedThrottle"
#define NES_PREFERENCES_AUTOFRAMESKIP_VALUE_NAME        "AutoFrameskip"
#define NES_PREFERENCES_FASTFPS_VALUE_NAME				"FastFPS"
#define NES_PREFERENCES_TOGGLEFAST_VALUE_NAME			"ToggleFast"
#define NES_PREFERENCES_PRIORITY_VALUE_NAME             "Priority"

#define NES_GRAPHICS_DOUBLESIZE_VALUE_NAME              "DoubleSize"
#define NES_GRAPHICS_SCANLINEGAPS_VALUE_NAME            "ScanlineGaps"
#define NES_GRAPHICS_BLACKANDWHITE_VALUE_NAME           "BlackAndWhite"
#define NES_GRAPHICS_MORETHAN8SPRITESPERLINE_VALUE_NAME "MoreThan8Sprites"
#define NES_GRAPHICS_SHOWALLSCANLINES_VALUE_NAME        "ShowAllScanlines"
#define NES_GRAPHICS_DRAWOVERSCAN_VALUE_NAME            "DrawOverscan"
#define NES_GRAPHICS_FULLSCREENONLOAD_VALUE_NAME        "FullscreenOnLoad"
#define NES_GRAPHICS_FULLSCREENSCALING_VALUE_NAME       "FullscreenScaling"
#define NES_GRAPHICS_CALCPALETTE_VALUE_NAME             "CalculatePalette"
#define NES_GRAPHICS_PALETTEFILE_VALUE_NAME				"PaletteFile"
#define NES_GRAPHICS_TINT_VALUE_NAME                    "Tint"
#define NES_GRAPHICS_HUE_VALUE_NAME                     "Hue"
#define NES_GRAPHICS_FULLSCREENWIDTH_VALUE_NAME         "FullscreenWidth"
#define NES_GRAPHICS_DEVICEGUID_VALUE_NAME              "DeviceGUID"
#define NES_GRAPHICS_USESTRETCHBLT_VALUE_NAME			"UseStretchBlt"
#define NES_GRAPHICS_EMULATETVSCANLINE_VALUE_NAME		"EmulateTVScanline"

#define NES_SOUND_SOUNDENABLED_VALUE_NAME               "SoundEnabled"
#define NES_SOUND_SAMPLEBITS_VALUE_NAME                 "SampleBits"
#define NES_SOUND_SAMPLERATE_VALUE_NAME                 "SampleRate"
#define NES_SOUND_RECTANGLE1_VALUE_NAME                 "Rectangle1"
#define NES_SOUND_RECTANGLE2_VALUE_NAME                 "Rectangle2"
#define NES_SOUND_TRIANGLE_VALUE_NAME                   "Triangle"
#define NES_SOUND_NOISE_VALUE_NAME                      "Noise"
#define NES_SOUND_DPCM_VALUE_NAME                       "DPCM"
#define NES_SOUND_EXT_VALUE_NAME                        "EXT"
#define NES_SOUND_IDEALTRIANGLE_VALUE_NAME              "IdealTriangle"
#define NES_SOUND_SMOOTHENVELOPE_VALUE_NAME             "SmoothEnvelope"
#define NES_SOUND_SMOOTHSWEEP_VALUE_NAME                "SmoothSweep"
#define NES_SOUND_BUFFERLEN_VALUE_NAME                  "BufferLength"
#define NES_SOUND_FILTERTYPE_VALUE_NAME                 "FilterType"

//#define NES_INPUT_BUTTON_DEVICEGUID_VALUE_NAME          "DeviceGUID"
#define NES_INPUT_BUTTON_DEVICETYPE_VALUE_NAME          "DeviceType"
#define NES_INPUT_BUTTON_KEY_VALUE_NAME                 "Key"
#define NES_INPUT_BUTTON_JOFFSET_VALUE_NAME             "Offset"
#define NES_INPUT_BUTTON_JAXISPOSITIVE_VALUE_NAME       "JAxisPositive"
#define NES_INPUT_BUTTON_DEVNO_VALUE_NAME				"DeviceNo"


#define RECENT_VALUE_NAME                               "Recent"
#define OPENPATH_VALUE_NAME                             "OpenPath"


/////////////////////////////////////////////////
// Windows emulator settings saving/loading code
// uses the *registry*!  tada!
/////////////////////////////////////////////////

#define LOAD_SETTING(KEY, VAR, VAL_NAME) \
  data_size = sizeof(VAR); \
  RegQueryValueEx(KEY, VAL_NAME, NULL, &data_type, (LPBYTE)&VAR, &data_size)

#define SAVE_SETTING(KEY, VAR, TYPE, VAL_NAME) \
  RegSetValueEx(KEY, VAL_NAME, 0, TYPE, (CONST BYTE*)&VAR, sizeof(VAR))

void LoadNESSettings(HKEY nester_key, NES_settings& settings);
void SaveNESSettings(HKEY nester_key, NES_settings& settings);
void LoadRecentFiles(HKEY nester_key, recent_list& rl);
void SaveRecentFiles(HKEY nester_key, const recent_list& rl);
void LoadControllerSettings(HKEY NES_input_key, const char* keyName, NES_controller_input_settings* settings);
void SaveControllerSettings(HKEY NES_input_key, const char* keyName, NES_controller_input_settings* settings);

void LoadCheatDlgSettings(HKEY );
void SaveCheatDlgSettings(HKEY );

boolean OSD_LoadSettings(class settings_t& settings)
{
	HKEY software_key;
	HKEY nester_key;

	// open the "software" key
	if(RegCreateKey(HKEY_CURRENT_USER, "Software", &software_key) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	// open the "nester" key
	if(RegCreateKey(software_key, NESTER_KEY_NAME, &nester_key) != ERROR_SUCCESS)
	{
		RegCloseKey(software_key);
		return FALSE;
	}
	LoadCheatDlgSettings(nester_key);

	// load the NES settings
	LoadNESSettings(nester_key, settings.nes);

	// load the recent files
	LoadRecentFiles(nester_key, settings.recent_ROMs);

	// load the open path
	{
		DWORD data_type;
		DWORD data_size;

		LOAD_SETTING(nester_key, settings.OpenPath, OPENPATH_VALUE_NAME);
	}

	// close the keys
	RegCloseKey(nester_key);
	RegCloseKey(software_key);
	return TRUE;
}


boolean OSD_SaveSettings(class settings_t& settings)
{
	HKEY software_key;
	HKEY nester_key;

	// open the "software" key
	if(RegCreateKey(HKEY_CURRENT_USER, "Software", &software_key) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	// open the "nester" key
	if(RegCreateKey(software_key, NESTER_KEY_NAME, &nester_key) != ERROR_SUCCESS)
	{
		RegCloseKey(software_key);
		return FALSE;
	}

	SaveCheatDlgSettings(nester_key);

	// save the NES settings
	SaveNESSettings(nester_key, settings.nes);

	// save the recent files
	SaveRecentFiles(nester_key, settings.recent_ROMs);

	// save the open path
	SAVE_SETTING(nester_key, settings.OpenPath, REG_SZ, OPENPATH_VALUE_NAME);

	// close the keys
	RegCloseKey(nester_key);
	RegCloseKey(software_key);
	return TRUE;
}

void LoadButtonSettings(HKEY NES_cntr_key, const char* keyName, OSD_ButtonSettings* settings)
{
	HKEY NES_button_key;
	DWORD data_type;
	DWORD data_size;

	try {
		NES_button_key = 0;

		// open key
		if(RegCreateKey(NES_cntr_key, keyName, &NES_button_key) != ERROR_SUCCESS) throw -1;

		// save settings
		//    LOAD_SETTING(NES_button_key, settings->deviceGUID, NES_INPUT_BUTTON_DEVICEGUID_VALUE_NAME);
		LOAD_SETTING(NES_button_key, settings->dev_n, NES_INPUT_BUTTON_DEVNO_VALUE_NAME);
		LOAD_SETTING(NES_button_key, settings->type, NES_INPUT_BUTTON_DEVICETYPE_VALUE_NAME);
		//    LOAD_SETTING(NES_button_key, settings->key, NES_INPUT_BUTTON_KEY_VALUE_NAME);
		LOAD_SETTING(NES_button_key, settings->j_offset, NES_INPUT_BUTTON_JOFFSET_VALUE_NAME);
		LOAD_SETTING(NES_button_key, settings->j_axispositive, NES_INPUT_BUTTON_JAXISPOSITIVE_VALUE_NAME);

		RegCloseKey(NES_button_key);
	} catch(...)
	{
		settings->type = D_NONE;
		if(NES_button_key) RegCloseKey(NES_button_key);
	}
}

void SaveButtonSettings(HKEY NES_cntr_key, const char* keyName, OSD_ButtonSettings* settings)
{
	HKEY NES_button_key;

	try {
		NES_button_key = 0;

		// open key
		if(RegCreateKey(NES_cntr_key, keyName, &NES_button_key) != ERROR_SUCCESS) throw -1;

		// save settings
		//    SAVE_SETTING(NES_button_key, settings->deviceGUID, REG_BINARY, NES_INPUT_BUTTON_DEVICEGUID_VALUE_NAME);
		SAVE_SETTING(NES_button_key, settings->dev_n, REG_DWORD, NES_INPUT_BUTTON_DEVNO_VALUE_NAME);
		SAVE_SETTING(NES_button_key, settings->type, REG_DWORD, NES_INPUT_BUTTON_DEVICETYPE_VALUE_NAME);
		//    SAVE_SETTING(NES_button_key, settings->key, REG_BINARY, NES_INPUT_BUTTON_KEY_VALUE_NAME);
		SAVE_SETTING(NES_button_key, settings->j_offset, REG_DWORD, NES_INPUT_BUTTON_JOFFSET_VALUE_NAME);
		SAVE_SETTING(NES_button_key, settings->j_axispositive, REG_DWORD, NES_INPUT_BUTTON_JAXISPOSITIVE_VALUE_NAME);

		RegCloseKey(NES_button_key);
	} catch(...)
	{
		if(NES_button_key) RegCloseKey(NES_button_key);
	}
}

void LoadNESSettings(HKEY nester_key, NES_settings& settings)
{
	HKEY NES_key;
	HKEY NES_preferences_key;
	HKEY NES_graphics_key;
	HKEY NES_sound_key;
	HKEY NES_input_key;
	HKEY NES_ext_input_key;
	DWORD data_type;
	DWORD data_size;

	// open the "NES" key
	if(RegCreateKey(nester_key, NES_KEY_NAME, &NES_key) != ERROR_SUCCESS)
	{
		return;
	}

	// load preferences settings
	try {
		NES_preferences_key = 0;

		// open key
		if(RegCreateKey(NES_key, NES_PREFERENCES_KEY_NAME, &NES_preferences_key) != ERROR_SUCCESS) throw -1;

		// load settings
		LOAD_SETTING(NES_preferences_key, settings.preferences.run_in_background, NES_PREFERENCES_RUNINBACKGROUND_VALUE_NAME);
		LOAD_SETTING(NES_preferences_key, settings.preferences.SkipSomeErrors, NES_PREFERENCES_SKIPSOMEERRORS_VALUE_NAME);
		LOAD_SETTING(NES_preferences_key, settings.preferences.speed_throttling, NES_PREFERENCES_SPEEDTHROTTLE_VALUE_NAME);
		LOAD_SETTING(NES_preferences_key, settings.preferences.auto_frameskip, NES_PREFERENCES_AUTOFRAMESKIP_VALUE_NAME);
		LOAD_SETTING(NES_preferences_key, settings.preferences.FastFPS, NES_PREFERENCES_FASTFPS_VALUE_NAME);
		LOAD_SETTING(NES_preferences_key, settings.preferences.ToggleFast, NES_PREFERENCES_TOGGLEFAST_VALUE_NAME);
		LOAD_SETTING(NES_preferences_key, settings.preferences.priority, NES_PREFERENCES_PRIORITY_VALUE_NAME);
		LOAD_SETTING(NES_preferences_key, settings.preferences.NotUseSleep, "NotUseSleep");
		LOAD_SETTING(NES_preferences_key, settings.preferences.NotUseFDSDiskASkip, "NotUseFDSDiskASkip");

		data_size=1;
		RegQueryValueEx(NES_preferences_key, "UseRomDataBase", 0, &data_type, (LPBYTE)&settings.preferences.UseRomDataBase, &data_size);
		data_size=1;
		RegQueryValueEx(NES_preferences_key, "AutoRomCorrect", 0, &data_type, (LPBYTE)&settings.preferences.AutoRomCorrect, &data_size);
		data_size=1;
		RegQueryValueEx(NES_preferences_key, "AutoStateSL", 0, &data_type, (LPBYTE)&settings.preferences.AutoStateSL, &data_size);
		data_size=1;
		RegQueryValueEx(NES_preferences_key, "AutoStateSLNotload", 0, &data_type, (LPBYTE)&settings.preferences.AutoStateSLNotload, &data_size);
		data_size=1;
		RegQueryValueEx(NES_preferences_key, "AllowMultiInstance", 0, &data_type, (LPBYTE)&settings.preferences.AllowMultiInstance, &data_size);
		data_size=1;
		RegQueryValueEx(NES_preferences_key, "ZipDLLUseFlag", 0, &data_size, (LPBYTE)&settings.preferences.ZipDLLUseFlag, &data_size);
		data_size=1;
		RegQueryValueEx(NES_preferences_key, "SaveFileToZip", 0, &data_size, (LPBYTE)&settings.preferences.SaveFileToZip, &data_size);
		data_size=1;
		RegQueryValueEx(NES_preferences_key, "SaveFileToZip", 0, &data_size, (LPBYTE)&settings.preferences.StateFileToZip, &data_size);
		data_size=1;
		RegQueryValueEx(NES_preferences_key, "JoystickUseAPI", 0, &data_size, (LPBYTE)&settings.preferences.JoystickUseAPI, &data_size);

		data_size=1;
		RegQueryValueEx(NES_preferences_key, "StdSpeedState", 0, &data_size, (LPBYTE)&settings.preferences.StdSpeedState, &data_size);
		data_size=1;
		RegQueryValueEx(NES_preferences_key, "FastSpeedState", 0, &data_size, (LPBYTE)&settings.preferences.FastSpeedState, &data_size);
		data_size=4;
		RegQueryValueEx(NES_preferences_key, "StdFPS", 0, &data_size, (LPBYTE)&settings.preferences.StdFPS, &data_size);
		data_size=4;
		RegQueryValueEx(NES_preferences_key, "StdSkipFrame", 0, &data_size, (LPBYTE)&settings.preferences.StdSkipFrame, &data_size);
		data_size=4;
		RegQueryValueEx(NES_preferences_key, "FastSkipFrame", 0, &data_size, (LPBYTE)&settings.preferences.FastSkipFrame, &data_size);
		data_size=4;
		RegQueryValueEx(NES_preferences_key, "KailleraFrameInterval", 0, &data_size, (LPBYTE)&settings.preferences.KailleraFrameInterval, &data_size);

		RegCloseKey(NES_preferences_key);
	} catch(...)
	{
		if(NES_preferences_key) RegCloseKey(NES_preferences_key);
	}

	// load graphics settings
	try {
		NES_graphics_key = 0;

		// open key
		if(RegCreateKey(NES_key, NES_GRAPHICS_KEY_NAME, &NES_graphics_key) != ERROR_SUCCESS) throw -1;

		// load settings
		LOAD_SETTING(NES_graphics_key, settings.graphics.osd.InfoStatusBar, "InfoStatusBar");
		LOAD_SETTING(NES_graphics_key, settings.graphics.osd.WindowTopMost, "WindowTopMost");

		LOAD_SETTING(NES_graphics_key, settings.graphics.osd.double_size, NES_GRAPHICS_DOUBLESIZE_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.black_and_white, NES_GRAPHICS_BLACKANDWHITE_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.show_more_than_8_sprites, NES_GRAPHICS_MORETHAN8SPRITESPERLINE_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.show_all_scanlines, NES_GRAPHICS_SHOWALLSCANLINES_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.draw_overscan, NES_GRAPHICS_DRAWOVERSCAN_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.fullscreen_on_load, NES_GRAPHICS_FULLSCREENONLOAD_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.fullscreen_scaling, NES_GRAPHICS_FULLSCREENSCALING_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.calculate_palette, NES_GRAPHICS_CALCPALETTE_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.szPaletteFile, NES_GRAPHICS_PALETTEFILE_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.tint, NES_GRAPHICS_TINT_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.hue, NES_GRAPHICS_HUE_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.osd.fullscreen_width, NES_GRAPHICS_FULLSCREENWIDTH_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.osd.device_GUID, NES_GRAPHICS_DEVICEGUID_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.UseStretchBlt, NES_GRAPHICS_USESTRETCHBLT_VALUE_NAME);
		LOAD_SETTING(NES_graphics_key, settings.graphics.EmulateTVScanline, NES_GRAPHICS_EMULATETVSCANLINE_VALUE_NAME);

		LOAD_SETTING(NES_graphics_key, settings.graphics.DisableSpriteClipping, "DisableSpriteClipping");
		LOAD_SETTING(NES_graphics_key, settings.graphics.DisableBackGClipping, "DisableBackGClipping");
		LOAD_SETTING(NES_graphics_key, settings.graphics.osd.DisableMenuIcon, "DisableMenuIcon");

		data_size=1;
		RegQueryValueEx(NES_graphics_key, "WindowModeUseDdraw", 0, &data_type, (LPBYTE)&settings.graphics.WindowModeUseDdraw, &data_size);
		data_size=1;
		RegQueryValueEx(NES_graphics_key, "WindowModeUseDdrawOverlay", 0, &data_type, (LPBYTE)&settings.graphics.WindowModeUseDdrawOverlay, &data_size);
		data_size=1;
		RegQueryValueEx(NES_graphics_key, "FullscreenModeUseNewMode", 0, &data_type, (LPBYTE)&settings.graphics.FullscreenModeUseNewMode, &data_size);
		data_size=1;
		RegQueryValueEx(NES_graphics_key, "FullscreenModeUseDefRefreshRate", 0, &data_type, (LPBYTE)&settings.graphics.FullscreenModeUseDefRefreshRate, &data_size);
		data_size=1;
		RegQueryValueEx(NES_graphics_key, "FullscreenModeBackBuffM", 0, &data_type, (LPBYTE)&settings.graphics.FullscreenModeBackBuffM, &data_size);
		data_size=1;
		RegQueryValueEx(NES_graphics_key, "WindowModeBackBuffM", 0, &data_type, (LPBYTE)&settings.graphics.WindowModeBackBuffM, &data_size);
		data_size=1;
		RegQueryValueEx(NES_graphics_key, "WindowModeDtvSize", 0, &data_type, (LPBYTE)&settings.graphics.osd.DtvSize, &data_size);
		data_size=1;
		RegQueryValueEx(NES_graphics_key, "ScreenShotUseBmp", 0, &data_type, (LPBYTE)&settings.graphics.ScreenShotUseBmp, &data_size);
		data_size=1;
		RegQueryValueEx(NES_graphics_key, "FullscreenModeNotUseFlip", 0, &data_type, (LPBYTE)&settings.graphics.FullscreenModeNotUseFlip, &data_size);

		RegCloseKey(NES_graphics_key);
	} catch(...)
	{
		if(NES_graphics_key) RegCloseKey(NES_graphics_key);
	}

	// load sound settings
	try {
		NES_sound_key = 0;

		// open key
		if(RegCreateKey(NES_key, NES_SOUND_KEY_NAME, &NES_sound_key) != ERROR_SUCCESS) throw -1;

		// load settings
		LOAD_SETTING(NES_sound_key, settings.sound.enabled, NES_SOUND_SOUNDENABLED_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.sample_bits, NES_SOUND_SAMPLEBITS_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.sample_rate, NES_SOUND_SAMPLERATE_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.rectangle1_enabled, NES_SOUND_RECTANGLE1_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.rectangle2_enabled, NES_SOUND_RECTANGLE2_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.triangle_enabled, NES_SOUND_TRIANGLE_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.noise_enabled, NES_SOUND_NOISE_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.dpcm_enabled, NES_SOUND_DPCM_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.ext_enabled, NES_SOUND_EXT_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.ideal_triangle_enabled, NES_SOUND_IDEALTRIANGLE_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.smooth_envelope_enabled, NES_SOUND_SMOOTHENVELOPE_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.smooth_sweep_enabled, NES_SOUND_SMOOTHSWEEP_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.buffer_len, NES_SOUND_BUFFERLEN_VALUE_NAME);
		LOAD_SETTING(NES_sound_key, settings.sound.filter_type, NES_SOUND_FILTERTYPE_VALUE_NAME);

		data_size=4;
		RegQueryValueEx(NES_sound_key, "rectangle1_volumed", 0, &data_type, (LPBYTE)&settings.sound.rectangle1_volumed, &data_size);
		data_size=4;
		RegQueryValueEx(NES_sound_key, "rectangle2_volumed", 0, &data_type, (LPBYTE)&settings.sound.rectangle2_volumed, &data_size);
		data_size=4;
		RegQueryValueEx(NES_sound_key, "triangle_volumed", 0, &data_type, (LPBYTE)&settings.sound.triangle_volumed, &data_size);
		data_size=4;
		RegQueryValueEx(NES_sound_key, "noise_volumed", 0, &data_type, (LPBYTE)&settings.sound.noise_volumed, &data_size);
		data_size=4;
		RegQueryValueEx(NES_sound_key, "dpcm_volumed", 0, &data_type, (LPBYTE)&settings.sound.dpcm_volumed, &data_size);
		data_size=4;
		RegQueryValueEx(NES_sound_key, "ext_volumed", 0, &data_type, (LPBYTE)&settings.sound.ext_volumed, &data_size);

		RegCloseKey(NES_sound_key);
	} catch(...)
	{
		if(NES_sound_key) RegCloseKey(NES_sound_key);
	}

	// load input settings
	try {
		NES_input_key = 0;

		// open key
		if(RegCreateKey(NES_key, NES_INPUT_KEY_NAME, &NES_input_key) != ERROR_SUCCESS) throw -1;

		// load settings

		// load controller 1
		LoadControllerSettings(NES_input_key, NES_INPUT_CNT1_KEY_NAME, &settings.input.player1);

		// load controller 2
		LoadControllerSettings(NES_input_key, NES_INPUT_CNT2_KEY_NAME, &settings.input.player2);
		LoadControllerSettings(NES_input_key, NES_INPUT_CNT3_KEY_NAME, &settings.input.player3);
		LoadControllerSettings(NES_input_key, NES_INPUT_CNT4_KEY_NAME, &settings.input.player4);
		LoadButtonSettings(NES_input_key, NES_INPUT_MIC_KEY_NAME, &settings.input.MicButton);

		RegCloseKey(NES_input_key);
	} catch(...)
	{
		if(NES_input_key) RegCloseKey(NES_input_key);
	}

	try {
		NES_ext_input_key = 0;

		// open key
		if(RegCreateKey(NES_key, NES_EXT_INPUT_KEY_NAME, &NES_ext_input_key) != ERROR_SUCCESS) throw -1;
		// save settings
		{
			char tstr[10];
			for(int i=0; i< EXTKEYALLNUM;++i){
				wsprintf(tstr, "EXTBTN%u", i);
				LoadButtonSettings(NES_ext_input_key, tstr, &settings.input.extkeycfg[i]);
			}
			data_size=1;
			RegQueryValueEx(NES_ext_input_key, "extkeycfgFastFPSToggle", 0, &data_type, (LPBYTE)&settings.input.extkeycfgFastFPSToggle, &data_size);
		}
		RegCloseKey(NES_ext_input_key);
	} catch(...)
	{
		if(NES_ext_input_key) RegCloseKey(NES_ext_input_key);
	}

	// close the "NES" key
	RegCloseKey(NES_key);
}

void SaveNESSettings(HKEY nester_key, NES_settings& settings)
{
	HKEY NES_key;
	HKEY NES_preferences_key;
	HKEY NES_graphics_key;
	HKEY NES_sound_key;

	// open the "NES" key
	if(RegCreateKey(nester_key, NES_KEY_NAME, &NES_key) != ERROR_SUCCESS)
	{
		return;
	}

	// save preferences settings
	try {
		NES_preferences_key = 0;

		// open key
		if(RegCreateKey(NES_key, NES_PREFERENCES_KEY_NAME, &NES_preferences_key) != ERROR_SUCCESS) throw -1;

		// save settings
		SAVE_SETTING(NES_preferences_key, settings.preferences.run_in_background, REG_BINARY, NES_PREFERENCES_RUNINBACKGROUND_VALUE_NAME);
		SAVE_SETTING(NES_preferences_key, settings.preferences.SkipSomeErrors, REG_BINARY, NES_PREFERENCES_SKIPSOMEERRORS_VALUE_NAME);
		SAVE_SETTING(NES_preferences_key, settings.preferences.speed_throttling, REG_BINARY, NES_PREFERENCES_SPEEDTHROTTLE_VALUE_NAME);
		SAVE_SETTING(NES_preferences_key, settings.preferences.auto_frameskip, REG_BINARY, NES_PREFERENCES_AUTOFRAMESKIP_VALUE_NAME);
		SAVE_SETTING(NES_preferences_key, settings.preferences.FastFPS, REG_DWORD, NES_PREFERENCES_FASTFPS_VALUE_NAME);
		SAVE_SETTING(NES_preferences_key, settings.preferences.ToggleFast, REG_BINARY, NES_PREFERENCES_TOGGLEFAST_VALUE_NAME);
		SAVE_SETTING(NES_preferences_key, settings.preferences.priority, REG_DWORD, NES_PREFERENCES_PRIORITY_VALUE_NAME);
		SAVE_SETTING(NES_preferences_key, settings.preferences.NotUseSleep, REG_BINARY, "NotUseSleep");
		SAVE_SETTING(NES_preferences_key, settings.preferences.NotUseFDSDiskASkip, REG_BINARY, "NotUseFDSDiskASkip");

		RegSetValueEx(NES_preferences_key, "UseRomDataBase", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.UseRomDataBase, 1);
		RegSetValueEx(NES_preferences_key, "AutoRomCorrect", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.AutoRomCorrect, 1);
		RegSetValueEx(NES_preferences_key, "AutoStateSL", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.AutoStateSL, 1);
		RegSetValueEx(NES_preferences_key, "AutoStateSLNotload", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.AutoStateSLNotload, 1);
		RegSetValueEx(NES_preferences_key, "AllowMultiInstance", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.AllowMultiInstance, 1);
		RegSetValueEx(NES_preferences_key, "ZipDLLUseFlag", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.ZipDLLUseFlag, 1);
		RegSetValueEx(NES_preferences_key, "SaveFileToZip", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.SaveFileToZip, 1);
		RegSetValueEx(NES_preferences_key, "StateFileToZip", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.StateFileToZip, 1);
		RegSetValueEx(NES_preferences_key, "JoystickUseAPI", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.JoystickUseAPI, 1);

		RegSetValueEx(NES_preferences_key, "StdSpeedState", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.StdSpeedState, 1);
		RegSetValueEx(NES_preferences_key, "FastSpeedState", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.FastSpeedState, 1);
		RegSetValueEx(NES_preferences_key, "StdFPS", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.StdFPS, 4);
		RegSetValueEx(NES_preferences_key, "StdSkipFrame", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.StdSkipFrame, 4);
		RegSetValueEx(NES_preferences_key, "FastSkipFrame", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.FastSkipFrame, 4);
		RegSetValueEx(NES_preferences_key, "KailleraFrameInterval", 0, REG_BINARY, (CONST BYTE*)&settings.preferences.KailleraFrameInterval, 4);

		RegCloseKey(NES_preferences_key);
	} catch(...)
	{
		if(NES_preferences_key) RegCloseKey(NES_preferences_key);
	}

	// save graphics settings
	try {
		NES_graphics_key = 0;

		// open key
		if(RegCreateKey(NES_key, NES_GRAPHICS_KEY_NAME, &NES_graphics_key) != ERROR_SUCCESS) throw -1;

		// save settings
		SAVE_SETTING(NES_graphics_key, settings.graphics.osd.InfoStatusBar, REG_BINARY, "InfoStatusBar");
		SAVE_SETTING(NES_graphics_key, settings.graphics.osd.WindowTopMost, REG_BINARY, "WindowTopMost");

		SAVE_SETTING(NES_graphics_key, settings.graphics.osd.double_size, REG_BINARY, NES_GRAPHICS_DOUBLESIZE_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.black_and_white, REG_BINARY, NES_GRAPHICS_BLACKANDWHITE_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.show_more_than_8_sprites, REG_BINARY, NES_GRAPHICS_MORETHAN8SPRITESPERLINE_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.show_all_scanlines, REG_BINARY, NES_GRAPHICS_SHOWALLSCANLINES_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.draw_overscan, REG_BINARY, NES_GRAPHICS_DRAWOVERSCAN_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.fullscreen_on_load, REG_BINARY, NES_GRAPHICS_FULLSCREENONLOAD_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.fullscreen_scaling, REG_BINARY, NES_GRAPHICS_FULLSCREENSCALING_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.calculate_palette, REG_BINARY, NES_GRAPHICS_CALCPALETTE_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.szPaletteFile, REG_SZ, NES_GRAPHICS_PALETTEFILE_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.tint, REG_BINARY, NES_GRAPHICS_TINT_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.hue, REG_BINARY, NES_GRAPHICS_HUE_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.osd.fullscreen_width, REG_DWORD, NES_GRAPHICS_FULLSCREENWIDTH_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.osd.device_GUID, REG_BINARY, NES_GRAPHICS_DEVICEGUID_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.UseStretchBlt, REG_BINARY, NES_GRAPHICS_USESTRETCHBLT_VALUE_NAME);
		SAVE_SETTING(NES_graphics_key, settings.graphics.EmulateTVScanline, REG_BINARY, NES_GRAPHICS_EMULATETVSCANLINE_VALUE_NAME);

		RegSetValueEx(NES_graphics_key, "WindowModeUseDdraw", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.WindowModeUseDdraw, 1);
		RegSetValueEx(NES_graphics_key, "WindowModeUseDdrawOverlay", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.WindowModeUseDdrawOverlay, 1);
		RegSetValueEx(NES_graphics_key, "FullscreenModeUseNewMode", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.FullscreenModeUseNewMode, 1);
		RegSetValueEx(NES_graphics_key, "FullscreenModeUseDefRefreshRate", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.FullscreenModeUseDefRefreshRate, 1);
		RegSetValueEx(NES_graphics_key, "FullscreenModeBackBuffM", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.FullscreenModeBackBuffM, 1);
		RegSetValueEx(NES_graphics_key, "WindowModeBackBuffM", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.WindowModeBackBuffM, 1);
		RegSetValueEx(NES_graphics_key, "WindowModeDtvSize", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.osd.DtvSize, 1);
		RegSetValueEx(NES_graphics_key, "ScreenShotUseBmp", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.ScreenShotUseBmp, 1);
		RegSetValueEx(NES_graphics_key, "FullscreenModeNotUseFlip", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.FullscreenModeNotUseFlip, 1);
		RegSetValueEx(NES_graphics_key, "DisableSpriteClipping", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.DisableSpriteClipping, 1);
		RegSetValueEx(NES_graphics_key, "DisableBackGClipping", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.DisableBackGClipping, 1);
		RegSetValueEx(NES_graphics_key, "DisableMenuIcon", 0, REG_BINARY, (CONST BYTE*)&settings.graphics.osd.DisableMenuIcon, 1);

		RegCloseKey(NES_graphics_key);
	} catch(...)
	{
		if(NES_graphics_key) RegCloseKey(NES_graphics_key);
	}

	// save sound settings
	try {
		NES_sound_key = 0;

		// open key
		if(RegCreateKey(NES_key, NES_SOUND_KEY_NAME, &NES_sound_key) != ERROR_SUCCESS) throw -1;

		// save settings
		SAVE_SETTING(NES_sound_key, settings.sound.enabled, REG_BINARY, NES_SOUND_SOUNDENABLED_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.sample_bits, REG_DWORD, NES_SOUND_SAMPLEBITS_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.sample_rate, REG_DWORD, NES_SOUND_SAMPLERATE_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.rectangle1_enabled, REG_BINARY, NES_SOUND_RECTANGLE1_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.rectangle2_enabled, REG_BINARY, NES_SOUND_RECTANGLE2_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.triangle_enabled, REG_BINARY, NES_SOUND_TRIANGLE_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.noise_enabled, REG_BINARY, NES_SOUND_NOISE_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.dpcm_enabled, REG_BINARY, NES_SOUND_DPCM_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.ext_enabled, REG_BINARY, NES_SOUND_EXT_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.ideal_triangle_enabled, REG_BINARY, NES_SOUND_IDEALTRIANGLE_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.smooth_envelope_enabled, REG_BINARY, NES_SOUND_SMOOTHENVELOPE_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.smooth_sweep_enabled, REG_BINARY, NES_SOUND_SMOOTHSWEEP_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.buffer_len, REG_DWORD, NES_SOUND_BUFFERLEN_VALUE_NAME);
		SAVE_SETTING(NES_sound_key, settings.sound.filter_type, REG_DWORD, NES_SOUND_FILTERTYPE_VALUE_NAME);
		RegSetValueEx(NES_sound_key, "rectangle1_volumed", 0, REG_BINARY, (CONST BYTE*)&settings.sound.rectangle1_volumed, 4);
		RegSetValueEx(NES_sound_key, "rectangle2_volumed", 0, REG_BINARY, (CONST BYTE*)&settings.sound.rectangle2_volumed, 4);
		RegSetValueEx(NES_sound_key, "triangle_volumed", 0, REG_BINARY, (CONST BYTE*)&settings.sound.triangle_volumed, 4);
		RegSetValueEx(NES_sound_key, "noise_volumed", 0, REG_BINARY, (CONST BYTE*)&settings.sound.noise_volumed, 4);
		RegSetValueEx(NES_sound_key, "dpcm_volumed", 0, REG_BINARY, (CONST BYTE*)&settings.sound.dpcm_volumed, 4);
		RegSetValueEx(NES_sound_key, "ext_volumed", 0, REG_BINARY, (CONST BYTE*)&settings.sound.ext_volumed, 4);

		RegCloseKey(NES_sound_key);
	} catch(...)
	{
		if(NES_sound_key) RegCloseKey(NES_sound_key);
	}

	// close the "NES" key
	RegCloseKey(NES_key);
}




void LoadControllerSettings(HKEY NES_input_key, const char* keyName, NES_controller_input_settings* settings)
{
	HKEY NES_cntr_key;
	DWORD data_size;
	DWORD data_type;

	try {
		NES_cntr_key = 0;

		// open key
		if(RegCreateKey(NES_input_key, keyName, &NES_cntr_key) != ERROR_SUCCESS) throw -1;

		// save settings
		LoadButtonSettings(NES_cntr_key, NES_INPUT_UP_KEY_NAME, &settings->btnUp);
		LoadButtonSettings(NES_cntr_key, NES_INPUT_DOWN_KEY_NAME, &settings->btnDown);
		LoadButtonSettings(NES_cntr_key, NES_INPUT_LEFT_KEY_NAME, &settings->btnLeft);
		LoadButtonSettings(NES_cntr_key, NES_INPUT_RIGHT_KEY_NAME, &settings->btnRight);
		LoadButtonSettings(NES_cntr_key, NES_INPUT_SELECT_KEY_NAME, &settings->btnSelect);
		LoadButtonSettings(NES_cntr_key, NES_INPUT_START_KEY_NAME, &settings->btnStart);
		LoadButtonSettings(NES_cntr_key, NES_INPUT_B_KEY_NAME, &settings->btnB);
		LoadButtonSettings(NES_cntr_key, NES_INPUT_A_KEY_NAME, &settings->btnA);
		LoadButtonSettings(NES_cntr_key, NES_INPUT_TB_KEY_NAME, &settings->btnTB);		//nnn turbo button
		LoadButtonSettings(NES_cntr_key, NES_INPUT_TA_KEY_NAME, &settings->btnTA);
		LOAD_SETTING(NES_cntr_key, settings->btnTBsec, NES_INPUT_TBPS_KEY_NAME);
		LOAD_SETTING(NES_cntr_key, settings->btnTAsec, NES_INPUT_TAPS_KEY_NAME);

		RegCloseKey(NES_cntr_key);
	} catch(...)
	{
		if(NES_cntr_key) RegCloseKey(NES_cntr_key);
	}
}

void SaveControllerSettings(HKEY NES_input_key, const char* keyName, NES_controller_input_settings* settings)
{
	HKEY NES_cntr_key;

	try {
		NES_cntr_key = 0;

		// open key
		if(RegCreateKey(NES_input_key, keyName, &NES_cntr_key) != ERROR_SUCCESS) throw -1;

		// save settings
		SaveButtonSettings(NES_cntr_key, NES_INPUT_UP_KEY_NAME, &settings->btnUp);
		SaveButtonSettings(NES_cntr_key, NES_INPUT_DOWN_KEY_NAME, &settings->btnDown);
		SaveButtonSettings(NES_cntr_key, NES_INPUT_LEFT_KEY_NAME, &settings->btnLeft);
		SaveButtonSettings(NES_cntr_key, NES_INPUT_RIGHT_KEY_NAME, &settings->btnRight);
		SaveButtonSettings(NES_cntr_key, NES_INPUT_SELECT_KEY_NAME, &settings->btnSelect);
		SaveButtonSettings(NES_cntr_key, NES_INPUT_START_KEY_NAME, &settings->btnStart);
		SaveButtonSettings(NES_cntr_key, NES_INPUT_B_KEY_NAME, &settings->btnB);
		SaveButtonSettings(NES_cntr_key, NES_INPUT_A_KEY_NAME, &settings->btnA);
		SaveButtonSettings(NES_cntr_key, NES_INPUT_TB_KEY_NAME, &settings->btnTB);	//nnn turbo button
		SaveButtonSettings(NES_cntr_key, NES_INPUT_TA_KEY_NAME, &settings->btnTA);
		SAVE_SETTING(NES_cntr_key, settings->btnTBsec, REG_DWORD, NES_INPUT_TBPS_KEY_NAME);
		SAVE_SETTING(NES_cntr_key, settings->btnTAsec, REG_DWORD, NES_INPUT_TAPS_KEY_NAME);

		RegCloseKey(NES_cntr_key);
	} catch(...)
	{
		if(NES_cntr_key) RegCloseKey(NES_cntr_key);
	}
}


void LoadRecentFiles(HKEY nester_key, recent_list& rl)
{
	HKEY recent_key;
	DWORD data_type;
	DWORD data_size;

	char buf[256];
	char value_name[recent_list::ENTRY_LEN];

	// open the "Recent" key
	if(RegCreateKey(nester_key, RECENT_KEY_NAME, &recent_key) != ERROR_SUCCESS)
		return;

	rl.clear();

	for(int i = rl.get_max_entries()-1; i >= 0; i--)
	{
		sprintf(value_name, "%s%i", RECENT_VALUE_NAME, i);
		try {
			data_size = sizeof(buf);
			RegQueryValueEx(recent_key, value_name, NULL, &data_type,
			                (LPBYTE)buf, &data_size);

			if(data_type != REG_SZ) throw -1;

			if(strlen(buf))
				rl.add_entry(buf);

		} catch(...) {
		}
	}

	// close the "Recent" Key
	RegCloseKey(recent_key);
}

void SaveRecentFiles(HKEY nester_key, const recent_list& rl)
{
	HKEY recent_key;

	char buf[256];
	char value_name[recent_list::ENTRY_LEN];

	// open the "Recent" key
	if(RegCreateKey(nester_key, RECENT_KEY_NAME, &recent_key) != ERROR_SUCCESS)
		return;

	for(int i = 0; i < rl.get_max_entries(); i++)
	{
		sprintf(value_name, "%s%i", RECENT_VALUE_NAME, i);

		strcpy(buf, "");
		if(rl.get_entry(i))
		{
			strcpy(buf, rl.get_entry(i));
		}

		try {
			RegSetValueEx(recent_key, value_name, 0, REG_SZ,
			              (CONST BYTE*)buf, sizeof(buf));

		} catch(...) {
		}
	}

	// close the "Recent" Key
	RegCloseKey(recent_key);
}


void SaveAllControllerSettings(){
	HKEY NES_key;
	HKEY NES_iskey;
	HKEY NES_input_key;
	HKEY software_key;
	HKEY nester_key;

	// open the "software" key
	if(RegCreateKey(HKEY_CURRENT_USER, "Software", &software_key) != ERROR_SUCCESS){
		return;
	}
	// open the "nester" key
	if(RegCreateKey(software_key, NESTER_KEY_NAME, &nester_key) != ERROR_SUCCESS){
		RegCloseKey(software_key);
		return;
	}
	if(RegCreateKey(nester_key, NES_KEY_NAME, &NES_key) != ERROR_SUCCESS){
		return;
	}
	// open key
	if(RegCreateKey(NES_key, NES_INPUT_KEY_NAME, &NES_input_key) != ERROR_SUCCESS)
		return;
	// save controller 1 - 4
	SaveControllerSettings(NES_input_key, NES_INPUT_CNT1_KEY_NAME, &NESTER_settings.nes.input.player1);
	SaveControllerSettings(NES_input_key, NES_INPUT_CNT2_KEY_NAME, &NESTER_settings.nes.input.player2);
	SaveControllerSettings(NES_input_key, NES_INPUT_CNT3_KEY_NAME, &NESTER_settings.nes.input.player3);
	SaveControllerSettings(NES_input_key, NES_INPUT_CNT4_KEY_NAME, &NESTER_settings.nes.input.player4);
	SaveButtonSettings(NES_input_key, NES_INPUT_MIC_KEY_NAME, &NESTER_settings.nes.input.MicButton);
	// save Ext input settings
	NES_iskey = 0;
	if(RegCreateKey(NES_key, NES_EXT_INPUT_KEY_NAME, &NES_iskey) == ERROR_SUCCESS)
	{
		char tstr[10];
		for(int i=0; i<EXTKEYALLNUM;++i){
			wsprintf(tstr, "EXTBTN%u", i);
			SaveButtonSettings(NES_iskey, tstr, &NESTER_settings.nes.input.extkeycfg[i]);
		}
		RegSetValueEx(NES_iskey, "extkeycfgFastFPSToggle", 0, REG_BINARY, (CONST BYTE*)&NESTER_settings.nes.input.extkeycfgFastFPSToggle, 1);
	}
	RegCloseKey(NES_iskey);
	RegCloseKey(NES_key);
	RegCloseKey(nester_key);
	RegCloseKey(software_key);
}


//////// cheat dialog setting
extern unsigned char ctcmpdtf1;
extern unsigned char ctcmpdtf2;
extern unsigned char ctcmpdtf3;
extern unsigned char ctcmpdtf4;
extern unsigned char ctcmpdtf5;
extern unsigned char ctcmpdtf6;
extern unsigned char cc_clistedit1;
extern unsigned char cc_clistedit2;
extern unsigned char cc_csrchrb1;
extern unsigned char cc_csrchrb2;
extern unsigned char cc_csrchrb3;


void LoadCheatDlgSettings(HKEY nestkey){
	HKEY hKey;
	DWORD rb,dt;
	rb=sizeof(LOGFONT);
	RegQueryValueEx(nestkey, "CheatDlgFont", 0, &dt, (LPBYTE)&CheatDlgFont, &rb);
	rb=sizeof(DWORD);
	RegQueryValueEx(nestkey, "CheatDlgFontColor", 0, &dt, (LPBYTE)&CheatDlgFontColor, &rb);
	if(RegCreateKey(nestkey, "CheatDlgCfg", &hKey) != ERROR_SUCCESS)
		return;
	rb=1, RegQueryValueEx(hKey, "cmp1", 0, &dt, (LPBYTE)&ctcmpdtf1, &rb);
	rb=1, RegQueryValueEx(hKey, "cmp2", 0, &dt, (LPBYTE)&ctcmpdtf2, &rb);
	rb=1, RegQueryValueEx(hKey, "cmp3", 0, &dt, (LPBYTE)&ctcmpdtf3, &rb);
	rb=1, RegQueryValueEx(hKey, "cmp4", 0, &dt, (LPBYTE)&ctcmpdtf4, &rb);
	rb=1, RegQueryValueEx(hKey, "cmp5", 0, &dt, (LPBYTE)&ctcmpdtf5, &rb);
	rb=1, RegQueryValueEx(hKey, "cmp6", 0, &dt, (LPBYTE)&ctcmpdtf6, &rb);
	rb=1, RegQueryValueEx(hKey, "cle1", 0, &dt, (LPBYTE)&cc_clistedit1, &rb);
	rb=1, RegQueryValueEx(hKey, "cle2", 0, &dt, (LPBYTE)&cc_clistedit2, &rb);
	rb=1, RegQueryValueEx(hKey, "csr1", 0, &dt, (LPBYTE)&cc_csrchrb1, &rb);
	rb=1, RegQueryValueEx(hKey, "csr2", 0, &dt, (LPBYTE)&cc_csrchrb2, &rb);
	rb=1, RegQueryValueEx(hKey, "csr3", 0, &dt, (LPBYTE)&cc_csrchrb3, &rb);
	RegCloseKey(hKey);
}


void SaveCheatDlgSettings(HKEY nestkey){
	HKEY hKey;
	RegSetValueEx(nestkey, "CheatDlgFont", 0, REG_BINARY, (CONST BYTE*)&CheatDlgFont, sizeof(LOGFONT));
	RegSetValueEx(nestkey, "CheatDlgFontColor", 0, REG_BINARY, (CONST BYTE*)&CheatDlgFontColor, sizeof(DWORD));
	if(RegCreateKey(nestkey, "CheatDlgCfg", &hKey) != ERROR_SUCCESS)
		return;
	RegSetValueEx(hKey, "cmp1", 0, REG_BINARY, (CONST BYTE*)&ctcmpdtf1, 1);
	RegSetValueEx(hKey, "cmp2", 0, REG_BINARY, (CONST BYTE*)&ctcmpdtf2, 1);
	RegSetValueEx(hKey, "cmp3", 0, REG_BINARY, (CONST BYTE*)&ctcmpdtf3, 1);
	RegSetValueEx(hKey, "cmp4", 0, REG_BINARY, (CONST BYTE*)&ctcmpdtf4, 1);
	RegSetValueEx(hKey, "cmp5", 0, REG_BINARY, (CONST BYTE*)&ctcmpdtf5, 1);
	RegSetValueEx(hKey, "cmp6", 0, REG_BINARY, (CONST BYTE*)&ctcmpdtf6, 1);
	RegSetValueEx(hKey, "cle1", 0, REG_BINARY, (CONST BYTE*)&cc_clistedit1, 1);
	RegSetValueEx(hKey, "cle2", 0, REG_BINARY, (CONST BYTE*)&cc_clistedit2, 1);
	RegSetValueEx(hKey, "csr1", 0, REG_BINARY, (CONST BYTE*)&cc_csrchrb1, 1);
	RegSetValueEx(hKey, "csr2", 0, REG_BINARY, (CONST BYTE*)&cc_csrchrb2, 1);
	RegSetValueEx(hKey, "csr3", 0, REG_BINARY, (CONST BYTE*)&cc_csrchrb3, 1);
	RegCloseKey(hKey);
};




