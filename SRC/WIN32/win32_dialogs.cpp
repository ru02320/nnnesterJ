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

#include "win32_dialogs.h"

#include "resource.h"

#include "settings.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <ddraw.h>
#include <dinput.h>
#include <mapiguid.h>
#include <stdio.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <winuser.h>

//#include "win32_directinput_key_filter.h"
//#include "win32_directinput_keytable.h"
#include "win32_shellext.h"
#include "win32_GUID.h"
#include "win32_globals.h"
#include "iDirectX.h"
//#include "win32_default_controls.h"
#include "win32_directinput_input_mgr.h"
#include "OSD_ButtonSettings.h"
#include "webcheck.h"
#include "cheat.h"
#include "savecfg.h"

extern HWND main_window_handle;
extern HINSTANCE g_main_instance;
extern win32_directinput_input_mgr* ginp_mgr;
extern LOGFONT Default_Font;


#define ID_CTRDLGTIMER		918


/*********************************************************************************************/

static void PRF_OnFrameSkipChanged(HWND hDlg, NES_preferences_settings* p_settings)
{
	HWND hCheckBox_AutoFrameskip = GetDlgItem(hDlg, IDC_AUTOFRAMESKIP);

	// grey out or enable auto-frameskip
	int fcflag = p_settings->speed_throttling = IsDlgButtonChecked(hDlg, IDC_SPEEDTHROTTLE);
	if(hCheckBox_AutoFrameskip){
		EnableWindow(hCheckBox_AutoFrameskip, p_settings->speed_throttling);
	}
	EnableWindow(GetDlgItem(hDlg,IDC_RADIO1),fcflag);
	EnableWindow(GetDlgItem(hDlg,IDC_RADIO2),fcflag);
	EnableWindow(GetDlgItem(hDlg,IDC_RADIO3),fcflag);
	EnableWindow(GetDlgItem(hDlg,IDC_SLIDER1),fcflag);
	EnableWindow(GetDlgItem(hDlg,IDC_SLIDER2),fcflag);
}

static void PRF_InitDialog(HWND hDlg, NES_preferences_settings* p_settings)
{
	static HWND hCombo_Priority;          // handle to priority selection combo
	static HWND hFastFPS;

	CheckDlgButton(hDlg, IDC_SPEEDTHROTTLE,  p_settings->speed_throttling);
	CheckDlgButton(hDlg, IDC_AUTOFRAMESKIP,  p_settings->auto_frameskip);
	//  CheckDlgButton(hDlg, IDC_TOGGLEFAST,     p_settings->ToggleFast);
	CheckDlgButton(hDlg, IDC_CHECK1, p_settings->NotUseSleep);

	SendMessage(GetDlgItem(hDlg, IDC_SLIDER1),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,1000));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER2),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,25));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER8),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,1000));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER9),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,25));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER1), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)p_settings->StdFPS);
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER2), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)p_settings->StdSkipFrame);
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER8), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)p_settings->FastFPS);
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER9), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)p_settings->FastSkipFrame);
	SetDlgItemInt(hDlg, IDC_STATIC1, p_settings->StdFPS, FALSE);
	SetDlgItemInt(hDlg, IDC_STATIC2, p_settings->StdSkipFrame, FALSE);
	SetDlgItemInt(hDlg, IDC_STATIC3, p_settings->FastFPS, FALSE);
	SetDlgItemInt(hDlg, IDC_STATIC4, p_settings->FastSkipFrame, FALSE);

	CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO3, IDC_RADIO1+p_settings->StdSpeedState);
	CheckRadioButton( hDlg, IDC_RADIO4, IDC_RADIO6, IDC_RADIO4+p_settings->FastSpeedState);

	PRF_OnFrameSkipChanged(hDlg, p_settings);
	hCombo_Priority = GetDlgItem(hDlg, IDC_PRIORITY);
	if(hCombo_Priority)
	{
		char str[32];
		SendMessage(hCombo_Priority, CB_RESETCONTENT, 0, 0);
		for(int i=0; i<3; i++){
			LoadString(g_main_instance, IDS_STRING_DLGP_01+i , str, 32);
			SendMessage(hCombo_Priority, CB_ADDSTRING, 0, (LPARAM)str);
		}
		SendMessage(hCombo_Priority, CB_SETCURSEL, p_settings->priority, 0);
	}
	SetDlgItemInt(hDlg, IDC_EDIT1, p_settings->RewindKeyFrame, FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT2, p_settings->RewindBFrame, FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT3, p_settings->RewindKeyFrame*p_settings->RewindBFrame/60.0*1000, FALSE);
}

static void PRF_InitDialog2(HWND hDlg, NES_preferences_settings* p_settings)
{
	if(p_settings->UseRomDataBase){
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK2), FALSE);
		SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK, (WPARAM)TRUE, 0L);
	}
	else{
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK2), TRUE);
		SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK, (WPARAM)FALSE, 0L);
	}
	SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_SETCHECK, (WPARAM)p_settings->AutoRomCorrect, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_SETCHECK, (WPARAM)p_settings->AutoStateSLNotload, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK4), BM_SETCHECK, (WPARAM)p_settings->AllowMultiInstance, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK5), BM_SETCHECK, (WPARAM)p_settings->ZipDLLUseFlag, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK6), BM_SETCHECK, (WPARAM)p_settings->NotUseFDSDiskASkip, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK7), BM_SETCHECK, (WPARAM)p_settings->DisableMenuIcon, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK8), BM_SETCHECK, (WPARAM)p_settings->SaveFileToZip, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK9), BM_SETCHECK, (WPARAM)p_settings->StateFileToZip, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK10), BM_SETCHECK, (WPARAM)p_settings->JoystickUseAPI, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK11), BM_SETCHECK, (WPARAM)p_settings->DisableIPSPatch, 0L);
	CheckDlgButton(hDlg, IDC_RUNINBG,        p_settings->run_in_background);
	CheckDlgButton(hDlg, IDC_SKIPSOMEERRORS, p_settings->SkipSomeErrors);
}

BOOL CALLBACK PreferencesOptions_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	static NES_preferences_settings settings;
	NMHDR *nmhdr;
	switch(message)
	{
	case WM_INITDIALOG:
		settings = NESTER_settings.nes.preferences;
		PRF_InitDialog(hDlg, &settings);
		return TRUE;
	case WM_HSCROLL:
		{
			if((HWND)lParam == GetDlgItem( hDlg, IDC_SLIDER1)){
				settings.StdFPS = SendMessage(GetDlgItem(hDlg, IDC_SLIDER1), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				SetDlgItemInt(hDlg, IDC_STATIC1, settings.StdFPS, FALSE);
			}
			if((HWND)lParam == GetDlgItem( hDlg, IDC_SLIDER2)){
				settings.StdSkipFrame = SendMessage(GetDlgItem(hDlg, IDC_SLIDER2), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				SetDlgItemInt(hDlg, IDC_STATIC2, settings.StdSkipFrame, FALSE);
			}
			if((HWND)lParam == GetDlgItem( hDlg, IDC_SLIDER8)){
				settings.FastFPS = SendMessage(GetDlgItem(hDlg, IDC_SLIDER8), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				SetDlgItemInt(hDlg, IDC_STATIC3, settings.FastFPS, FALSE);
			}
			if((HWND)lParam == GetDlgItem( hDlg, IDC_SLIDER9)){
				settings.FastSkipFrame = SendMessage(GetDlgItem(hDlg, IDC_SLIDER9), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				SetDlgItemInt(hDlg, IDC_STATIC4, settings.FastSkipFrame, FALSE);
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_DEFAULTS:
			settings.SetDefaults();
			PRF_InitDialog(hDlg, &settings);
			return TRUE;

		case IDC_SPEEDTHROTTLE:
			// grey out or enable auto-frameskip
			PRF_OnFrameSkipChanged(hDlg, &settings);
			return TRUE;
		case IDC_AUTOFRAMESKIP:
			{
				HWND hFastFPS = GetDlgItem( hDlg, IDC_FASTFPS );
				EnableWindow( hFastFPS, IsDlgButtonChecked( hDlg, IDC_AUTOFRAMESKIP ) ? TRUE : FALSE );
				return TRUE;
			}
		case IDC_EDIT1:
		case IDC_EDIT2:
			{
				int k,b;
				k = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
				b = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, FALSE);
				SetDlgItemInt(hDlg, IDC_EDIT3, k * b / 60.0 * 1000, FALSE);
			}
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			NESTER_settings.nes.preferences.speed_throttling  = IsDlgButtonChecked(hDlg, IDC_SPEEDTHROTTLE);
			NESTER_settings.nes.preferences.auto_frameskip    = IsDlgButtonChecked(hDlg, IDC_AUTOFRAMESKIP);
			NESTER_settings.nes.preferences.NotUseSleep       = IsDlgButtonChecked(hDlg, IDC_CHECK1);
			NESTER_settings.nes.preferences.FastFPS			 = SendDlgItemMessage( hDlg, IDC_SLIDER8, TBM_GETPOS, 0,0 );
			NESTER_settings.nes.preferences.priority          = (NES_preferences_settings::NES_PRIORITY)
			        SendMessage(GetDlgItem(hDlg, IDC_PRIORITY), CB_GETCURSEL, 0, 0);
			NESTER_settings.nes.preferences.StdFPS			 = SendDlgItemMessage( hDlg, IDC_SLIDER1, TBM_GETPOS, 0,0 );
			NESTER_settings.nes.preferences.StdSkipFrame	 = SendDlgItemMessage( hDlg, IDC_SLIDER2, TBM_GETPOS, 0,0 );
			NESTER_settings.nes.preferences.FastSkipFrame	 = SendDlgItemMessage( hDlg, IDC_SLIDER9, TBM_GETPOS, 0,0 );
			NESTER_settings.nes.preferences.RewindKeyFrame = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			if(NESTER_settings.nes.preferences.RewindKeyFrame < 2){
				NESTER_settings.nes.preferences.RewindKeyFrame = 6;
			}
			NESTER_settings.nes.preferences.RewindBFrame = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, FALSE);
			if(NESTER_settings.nes.preferences.RewindBFrame < 2){
				NESTER_settings.nes.preferences.RewindBFrame = 30;
			}
			if(IsDlgButtonChecked(hDlg, IDC_RADIO1))
				NESTER_settings.nes.preferences.StdSpeedState=0;
			else if(IsDlgButtonChecked(hDlg, IDC_RADIO2))
				NESTER_settings.nes.preferences.StdSpeedState=1;
			else
				NESTER_settings.nes.preferences.StdSpeedState=2;
			if(IsDlgButtonChecked(hDlg, IDC_RADIO4))
				NESTER_settings.nes.preferences.FastSpeedState=0;
			else if(IsDlgButtonChecked(hDlg, IDC_RADIO5))
				NESTER_settings.nes.preferences.FastSpeedState=1;
			else
				NESTER_settings.nes.preferences.FastSpeedState=2;
			return TRUE;
		case PSN_RESET:
			return TRUE;
			return TRUE;
		}
		break;
	}
	return FALSE;
}


BOOL CALLBACK PreferencesOptions_DlgProc2(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static NES_preferences_settings settings;
	NMHDR *nmhdr;

	switch(message)
	{
	case WM_INITDIALOG:
		settings = NESTER_settings.nes.preferences;
		if(CheckAssociateNESExtension())
			CheckDlgButton(hDlg, IDC_CHECK12, TRUE);
		if(CheckAssociateFDSExtension())
			CheckDlgButton(hDlg, IDC_CHECK13, TRUE);
//		CheckDlgButton(hDlg, IDC_CHECK7, NESTER_settings.nes.graphics.osd.DisableMenuIcon);
//		CheckDlgButton(hDlg, IDC_CHECK10, NESTER_settings.nes.preferences.JoystickUseAPI);
		PRF_InitDialog2(hDlg, &settings);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_DEFAULTS:
			settings.SetDefaults();
			PRF_InitDialog2(hDlg, &settings);
			CheckDlgButton(hDlg, IDC_CHECK7, 0);
			return TRUE;
/*
		case IDC_ASSOCIATE:
			// user wants .NES files to be associated with nester
			AssociateNESExtension();
			return TRUE;
		case IDC_UNDO:
			// user wants to undo .NES file association
			UndoAssociateNESExtension();
			return TRUE;
		case IDC_BUTTON3:
			AssociateFDSExtension();
			return TRUE;
		case IDC_BUTTON4:
			UndoAssociateFDSExtension();
			return TRUE;
*/
		case IDC_CHECK1:
			if(SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L))
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK2), FALSE);
			else
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK2), TRUE);
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			NESTER_settings.nes.preferences.run_in_background = IsDlgButtonChecked(hDlg, IDC_RUNINBG);
			NESTER_settings.nes.preferences.SkipSomeErrors	 = IsDlgButtonChecked(hDlg, IDC_SKIPSOMEERRORS);
			NESTER_settings.nes.preferences.UseRomDataBase = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.preferences.AutoRomCorrect = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.preferences.AutoStateSLNotload = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.preferences.AllowMultiInstance= (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK4), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.preferences.ZipDLLUseFlag = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK5), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.preferences.NotUseFDSDiskASkip= (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK6), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.preferences.DisableMenuIcon = IsDlgButtonChecked(hDlg, IDC_CHECK7);
			NESTER_settings.nes.preferences.SaveFileToZip = IsDlgButtonChecked(hDlg, IDC_CHECK8);
			NESTER_settings.nes.preferences.StateFileToZip = IsDlgButtonChecked(hDlg, IDC_CHECK9);
			NESTER_settings.nes.preferences.JoystickUseAPI = IsDlgButtonChecked(hDlg, IDC_CHECK10);
			NESTER_settings.nes.preferences.DisableIPSPatch= IsDlgButtonChecked(hDlg, IDC_CHECK11);

			if(IsDlgButtonChecked(hDlg, IDC_CHECK12) != CheckAssociateNESExtension()){
				if(IsDlgButtonChecked(hDlg, IDC_CHECK12))
					AssociateNESExtension();
				else
					UndoAssociateNESExtension();
			}
			if(IsDlgButtonChecked(hDlg, IDC_CHECK13) != CheckAssociateFDSExtension()){
				if(IsDlgButtonChecked(hDlg, IDC_CHECK13))
					AssociateFDSExtension();
				else
					UndoAssociateFDSExtension();
			}

			return TRUE;
		case PSN_RESET:
			return TRUE;
		}
		return TRUE;
	}
	return FALSE;
}



static void PRF_InitDialog3(HWND hDlg, NES_preferences_settings* p_settings)
{
	CheckDlgButton(hDlg, IDC_CHECK1, p_settings->CloseButtonExit);
	CheckDlgButton(hDlg, IDC_CHECK2, p_settings->TimeUsePCounter);
	CheckDlgButton(hDlg, IDC_CHECK3, p_settings->JoystickUseGUID);
	CheckDlgButton(hDlg, IDC_CHECK4, p_settings->DisableGameInfoDisp);
	CheckDlgButton(hDlg, IDC_CHECK5, p_settings->DoubleClickFullScreen);

	CheckDlgButton(hDlg, IDC_CHECK6, p_settings->GM_WindowStyle_Title);
	CheckDlgButton(hDlg, IDC_CHECK7, p_settings->GM_WindowStyle_Menu);
	CheckDlgButton(hDlg, IDC_CHECK8, p_settings->GM_WindowStyle_SBar);

	SetDlgItemInt(hDlg, IDC_EDIT1, p_settings->nViewerReInterval, FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT2, p_settings->nPtnViewerReInterval, FALSE);

}


int FDS_BIOS_Check(HWND hDlg){
	char str[MAX_PATH], *p, *erm = "Error";
	HANDLE hFile;
	DWORD dwAccBytes, filesize;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "DISKSYS.ROM");
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return 0;
	}
	filesize = GetFileSize(hFile, NULL);
	if(filesize<0x2000){
		MessageBox(hDlg, "Incorrect File size.", erm, MB_OK | MB_ICONWARNING);
		CloseHandle(hFile);
		return 0;
	}
	if(filesize != 0x2000){
		if(-1 == SetFilePointer(hFile, 0x6010, 0, FILE_BEGIN)){
			CloseHandle(hFile);
			return 0;
		}
	}
	p = (char *)malloc(0x2000);
	if(p==NULL){
		CloseHandle(hFile);
		return 0;
	}
	ReadFile(hFile, p, 0x2000, &dwAccBytes, NULL);
	CloseHandle(hFile);
	if(dwAccBytes != 0x2000){
		MessageBox(hDlg, "File Read Error.", erm, MB_OK | MB_ICONWARNING);
		return 0;
	}
	dwAccBytes = CrcCalc((unsigned char *)p, 0x2000);
	if(dwAccBytes == 0x5e607dcf){
		MessageBox(hDlg, "FDS BIOS is correct.", "Pass", MB_OK | MB_ICONINFORMATION);
		return 1;
	}
	MessageBox(hDlg, "FDS BIOS is Incorrect.", erm, MB_OK | MB_ICONWARNING);
	return 0;
}


BOOL CALLBACK PreferencesOptions_DlgProc3(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static NES_preferences_settings settings;
	NMHDR *nmhdr;

	switch(message)
	{
	case WM_INITDIALOG:
		settings = NESTER_settings.nes.preferences;
		PRF_InitDialog3(hDlg, &settings);
		CheckDlgButton(hDlg, IDC_CHECK9, CheckUseRegistry());
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_DEFAULTS:
			settings.SetDefaults();
			PRF_InitDialog3(hDlg, &settings);
			CheckDlgButton(hDlg, IDC_CHECK9, FALSE);
			return TRUE;
		case IDC_BUTTON3:
			FDS_BIOS_Check(hDlg);
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			NESTER_settings.nes.preferences.CloseButtonExit = IsDlgButtonChecked(hDlg, IDC_CHECK1);
			NESTER_settings.nes.preferences.TimeUsePCounter = IsDlgButtonChecked(hDlg, IDC_CHECK2);
			NESTER_settings.nes.preferences.JoystickUseGUID = IsDlgButtonChecked(hDlg, IDC_CHECK3);
			NESTER_settings.nes.preferences.DisableGameInfoDisp = IsDlgButtonChecked(hDlg, IDC_CHECK4);
			NESTER_settings.nes.preferences.DoubleClickFullScreen = IsDlgButtonChecked(hDlg, IDC_CHECK5);

			NESTER_settings.nes.preferences.nViewerReInterval = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			NESTER_settings.nes.preferences.nPtnViewerReInterval = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, FALSE);

			NESTER_settings.nes.preferences.GM_WindowStyle_Title = IsDlgButtonChecked(hDlg, IDC_CHECK6);
			NESTER_settings.nes.preferences.GM_WindowStyle_Menu = IsDlgButtonChecked(hDlg, IDC_CHECK7);
			NESTER_settings.nes.preferences.GM_WindowStyle_SBar = IsDlgButtonChecked(hDlg, IDC_CHECK8);

			if(CheckUseRegistry() != IsDlgButtonChecked(hDlg, IDC_CHECK9)){
				if(IsDlgButtonChecked(hDlg, IDC_CHECK9)){
					SetUseRegistry(1);
				}
				else{
					SetUseRegistry(0);
				}
			}
			return TRUE;
		case PSN_RESET:
			return TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

/*********************************************************************************************/

// ptr to current display device GUID
static GUID* GRAPHICS_CurDeviceGUID;
static uint32 GRAPHICS_CurGUIDFound; // was the current GUID found and set?
static int GRAPHICS_FirstGUIDIndex;

static BOOL WINAPI GRAPHICS_DDEnumCallback_Devices(GUID FAR *lpGUID, LPSTR  lpDriverDescription,
        LPSTR  lpDriverName, LPVOID lpContext)
{
	HWND hComboDevices = (HWND)lpContext;
	LONG index;
	LPVOID lpDevice;

	index = SendMessage(hComboDevices, CB_ADDSTRING, 0, (LPARAM)lpDriverDescription);

	if(index != CB_ERR)
	{
		if(NULL == lpGUID)
		{
			lpDevice = NULL;
		}
		else
		{
			lpDevice = malloc(sizeof(GUID));
			if(lpDevice)
			{
				memcpy((void*)lpDevice, (const void*)lpGUID, sizeof(GUID));
			}
		}

		SendMessage(hComboDevices, CB_SETITEMDATA, index, (LPARAM)lpDevice);

		// if this is the current GUID, select it
		if(lpGUID)
		{
			if(!memcmp(lpGUID, GRAPHICS_CurDeviceGUID, sizeof(GUID)))
			{
				SendMessage(hComboDevices, CB_SETCURSEL, index, 0);
				GRAPHICS_CurGUIDFound = 1;
			}
		}
		else
		{
			// if current device is default display driver...
			if(NULL == GetGUIDPtr(GRAPHICS_CurDeviceGUID))
			{
				SendMessage(hComboDevices, CB_SETCURSEL, index, 0);
				GRAPHICS_CurGUIDFound = 1;
			}
		}

		// if this is the first item, set the index
		if(GRAPHICS_FirstGUIDIndex < 0)
		{
			GRAPHICS_FirstGUIDIndex = index;
		}

	}

	return DDENUMRET_OK;
}

// CALL THIS ON GRAPHICS DIALOG TERMINATION
static void GRAPHICS_ClearDevicesComboBox(HWND hDlg)
{
	HWND hComboDevices = GetDlgItem(hDlg, IDC_DEVICE);
	int i;
	int num_items;
	void* data;

	num_items = SendMessage(hComboDevices, CB_GETCOUNT, 0, 0);
	for(i = 0; i < num_items; i++)
	{
		data = (void*)SendMessage(hComboDevices, CB_GETITEMDATA, i, 0);
		if(data) free(data);
	}

	SendMessage(hComboDevices, CB_RESETCONTENT, 0, 0);
}

static void GRAPHICS_UpdateFullscreenDevices(HWND hDlg, GUID* CurDeviceGUID)
{
	HWND hComboDevices = GetDlgItem(hDlg, IDC_DEVICE);

	// copy the GUID location
	GRAPHICS_CurDeviceGUID = CurDeviceGUID;

	GRAPHICS_CurGUIDFound = 0;
	GRAPHICS_FirstGUIDIndex = -1;

	// clear the combo box
	GRAPHICS_ClearDevicesComboBox(hDlg);

	DirectDrawEnumerate(GRAPHICS_DDEnumCallback_Devices, (LPVOID)hComboDevices);

	if(!GRAPHICS_CurGUIDFound)
	{
		if(GRAPHICS_FirstGUIDIndex >= 0)
		{
			// select the first GUID
			SendMessage(hComboDevices, CB_SETCURSEL, (WPARAM)GRAPHICS_FirstGUIDIndex, 0);
			// notify the dialog
			SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_DEVICE, CBN_SELCHANGE), (LPARAM)hComboDevices);
		}
	}
}


static uint32 GRAPHICS_CurFullscreenWidth;
static uint32 GRAPHICS_CurFullscreenHeight;
static uint8  GRAPHICS_CurFullscreenBit;

static uint32 GRAPHICS_CurWidthFound; // was the current width setting found and set?
static int GRAPHICS_FirstWidthIndex;

static HRESULT WINAPI GRAPHICS_DDEnumCallback_Modes(LPDDSURFACEDESC lpDDSurfaceDesc,
        LPVOID lpContext)
{
	HWND hComboModes = (HWND)lpContext;
	char buf[256];
	int index;

	// check the vid mode

	// 8 bit?
//	if(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount != 8) return DDENUMRET_OK;

	// square ratio?
//	if(((lpDDSurfaceDesc->dwWidth * 3) / 4) != lpDDSurfaceDesc->dwHeight) return DDENUMRET_OK;

	// vid mode is OK

	sprintf(buf, "%dx%dx%d", lpDDSurfaceDesc->dwWidth,
	        lpDDSurfaceDesc->dwHeight, lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);

	{
		int num_items;
		DWORD tdw;

		num_items = SendMessage(hComboModes, CB_GETCOUNT, 0, 0);


		for(index = 0; index < num_items; index++)
		{
			// if our width is less than the width at this index, insert before
			tdw = SendMessage(hComboModes, CB_GETITEMDATA, index, 0);
			if(tdw > (lpDDSurfaceDesc->dwWidth<<20 | lpDDSurfaceDesc->dwHeight<<8 | lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount))
				break;
		}

		// insert the mode at the appropriate spot
		index = SendMessage(hComboModes, CB_INSERTSTRING, index, (LPARAM)buf);
	}

	if((index != CB_ERR) && (index != CB_ERRSPACE))
	{
		// set the data item to the mode width
		SendMessage(hComboModes, CB_SETITEMDATA, index, (LPARAM)(lpDDSurfaceDesc->dwWidth<<20)| (lpDDSurfaceDesc->dwHeight<<8) | lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);

		// if this is the first item, set the index
		if(GRAPHICS_FirstWidthIndex < 0)
		{
			GRAPHICS_FirstWidthIndex = index;
		}

		// if this is the current mode, select it
		if(lpDDSurfaceDesc->dwWidth == GRAPHICS_CurFullscreenWidth && lpDDSurfaceDesc->dwHeight == GRAPHICS_CurFullscreenHeight && 
			lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == GRAPHICS_CurFullscreenBit)
		{
			SendMessage(hComboModes, CB_SETCURSEL, index, 0);
			GRAPHICS_CurWidthFound = 1;
		}
	}

	return DDENUMRET_OK;
}

// CALL THIS ON GRAPHICS DIALOG TERMINATION
static void GRAPHICS_ClearModesComboBox(HWND hDlg)
{
	HWND hComboModes = GetDlgItem(hDlg, IDC_MODE);

	// nothing to free <@:)

	SendMessage(hComboModes, CB_RESETCONTENT, 0, 0);
}

static void GRAPHICS_UpdateFullscreenModes(HWND hDlg, GUID* CurDeviceGUID, uint32 CurFullscreenWidth)
{
	HWND hComboModes = GetDlgItem(hDlg, IDC_MODE);
	LPDIRECTDRAW lpDD;

	// set the current fullscreen width
	GRAPHICS_CurFullscreenBit = CurFullscreenWidth&0x0ff;
	GRAPHICS_CurFullscreenWidth = CurFullscreenWidth>>20;
	GRAPHICS_CurFullscreenHeight= (CurFullscreenWidth>>8)&0x0fff;

	// clear the combo box
	GRAPHICS_ClearModesComboBox(hDlg);

	lpDD = iDirectX::getDirectDraw1(GetGUIDPtr(CurDeviceGUID));
	if(lpDD)
	{
		GRAPHICS_CurWidthFound = 0;
		GRAPHICS_FirstWidthIndex = -1;

		lpDD->EnumDisplayModes(0, NULL, (LPVOID)hComboModes, GRAPHICS_DDEnumCallback_Modes);

		// if current mode was not in the list...
		if(!GRAPHICS_CurWidthFound)
		{
			if(GRAPHICS_FirstWidthIndex >= 0)
			{
				// select the first mode
				SendMessage(hComboModes, CB_SETCURSEL, (WPARAM)GRAPHICS_FirstWidthIndex, 0);
				// notify the dialog
				SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_MODE, CBN_SELCHANGE), (LPARAM)hComboModes);
			}
		}
	}
}


static void GRAPHICS_OnBlackAndWhite_Changed(HWND hDlg, NES_graphics_settings* settings, NES_graphics_settings* active_settings)
{
	settings->black_and_white = IsDlgButtonChecked(hDlg, IDC_BLACKANDWHITE);
	// show the results immediately
	active_settings->black_and_white = settings->black_and_white;
	SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);
	SendMessage(main_window_handle, WM_PAINT, 0, 0);
}

static void GRAPHICS_OnCalculatePalette_Changed(HWND hDlg, NES_graphics_settings* settings, NES_graphics_settings* active_settings)
{
	settings->calculate_palette = IsDlgButtonChecked(hDlg, IDC_CALCPALETTE)
	                              ? 1
	                              : IsDlgButtonChecked(hDlg, IDC_READPALETTEFILE)
	                              ? 2
	                              : IsDlgButtonChecked(hDlg, IDC_EDITPALETTE)
	                              ? 3
	                              : 0;
	GetDlgItemText( hDlg, IDC_PALETTEFILE, settings->szPaletteFile, 260 );
	// show the results immediately
	active_settings->calculate_palette = settings->calculate_palette;
	strcpy( active_settings->szPaletteFile, settings->szPaletteFile );
	SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);
	SendMessage(main_window_handle, WM_PAINT, 0, 0);
}

static void GRAPHICS_ChangePaletteRadioButton( HWND hDlg )
{
	HWND hPaletteFile	 = GetDlgItem( hDlg, IDC_PALETTEFILE ),
	                     hRefPaletteFile = GetDlgItem( hDlg, IDC_REF_PALETTEFILE ),
	                                       hLabelTint		 = GetDlgItem( hDlg, IDC_LABEL_TINT ),
	                                                      hLabelHue		 = GetDlgItem( hDlg, IDC_LABEL_HUE ),
	                                                                    hTint			 = GetDlgItem( hDlg, IDC_TINT ),
	                                                                               hHue			 = GetDlgItem( hDlg, IDC_HUE ),
	                                                                                         hReset			 = GetDlgItem( hDlg, ID_RESET );

	EnableWindow( hPaletteFile,	   FALSE );
	EnableWindow( hRefPaletteFile, FALSE );
	EnableWindow( hLabelTint,	   FALSE );
	EnableWindow( hLabelHue,	   FALSE );
	EnableWindow( hTint,		   FALSE );
	EnableWindow( hHue,			   FALSE );
	EnableWindow( hReset,		   FALSE );
	if( IsDlgButtonChecked( hDlg, IDC_READPALETTEFILE ) )
	{
		EnableWindow( hPaletteFile,	   TRUE );
		EnableWindow( hRefPaletteFile, TRUE );
	}
	else if( IsDlgButtonChecked( hDlg, IDC_CALCPALETTE ) )
	{
		EnableWindow( hLabelTint, TRUE );
		EnableWindow( hLabelHue,  TRUE );
		EnableWindow( hTint,	  TRUE );
		EnableWindow( hHue,		  TRUE );
		EnableWindow( hReset,	  TRUE );
	}
}


//General Setting
static void GRAPHICS_InitDialog(HWND hDlg, NES_graphics_settings* settings, NES_graphics_settings* active_settings)
{
	HWND hSlider_Tint;
	HWND hSlider_Hue;

	CheckDlgButton(hDlg, IDC_BLACKANDWHITE, settings->black_and_white);
	CheckDlgButton(hDlg, IDC_SHOWSPRITES, settings->show_more_than_8_sprites);
	CheckDlgButton(hDlg, IDC_SHOWALLSCANLINES, settings->show_all_scanlines);
	CheckDlgButton(hDlg, IDC_CHECK2, settings->DisableSpriteClipping);
	CheckDlgButton(hDlg, IDC_CHECK3, settings->DisableBackGClipping);

	CheckDlgButton(hDlg, IDC_EMULATETV, settings->EmulateTVScanline);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK, (WPARAM)settings->ScreenShotUseBmp, 0L);

	switch(settings->calculate_palette){
	case  1:
		CheckRadioButton( hDlg, IDC_DEFAULTPALETTE, IDC_EDITPALETTE, IDC_CALCPALETTE );
		break;
	case  2:
		CheckRadioButton( hDlg, IDC_DEFAULTPALETTE, IDC_EDITPALETTE, IDC_READPALETTEFILE );
		break;
	case  3:
		CheckRadioButton( hDlg, IDC_DEFAULTPALETTE, IDC_EDITPALETTE, IDC_EDITPALETTE );
		break;
	default:
		CheckRadioButton( hDlg, IDC_DEFAULTPALETTE, IDC_EDITPALETTE, IDC_DEFAULTPALETTE );
		break;
	}

	SetDlgItemText( hDlg, IDC_PALETTEFILE, settings->szPaletteFile );

	hSlider_Tint = GetDlgItem(hDlg, IDC_TINT);
	if(hSlider_Tint)
	{
		SendMessage(hSlider_Tint, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM) MAKELONG(0,255));
		SendMessage(hSlider_Tint, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)1);
		SendMessage(hSlider_Tint, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)settings->tint);
	}
	hSlider_Hue  = GetDlgItem(hDlg, IDC_HUE);
	if(hSlider_Hue)
	{
		SendMessage(hSlider_Hue, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM) MAKELONG(0,255));
		SendMessage(hSlider_Hue, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)1);
		SendMessage(hSlider_Hue, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)settings->hue);
	}
	GRAPHICS_ChangePaletteRadioButton( hDlg );
	GRAPHICS_OnBlackAndWhite_Changed(hDlg, settings, active_settings);
}


//WindowMode Setting
static void GRAPHICS_InitDialog2(HWND hDlg, NES_graphics_settings* settings, NES_graphics_settings* active_settings)
{
	//  CheckDlgButton(hDlg, IDC_DOUBLESIZE, settings->osd.double_size);

	SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK, (WPARAM)settings->WindowModeUseDdraw, 0L);
	if(settings->WindowModeUseDdraw){
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK2),1);
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO1),1);
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO2),1);
	}
	else{
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK2),0);
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO1),0);
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO2),0);
	}
	if(settings->WindowModeUseDdrawOverlay){
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO1),FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO2),FALSE);
	}
	SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_SETCHECK, (WPARAM)settings->WindowModeUseDdrawOverlay, 0L);
	CheckDlgButton(hDlg, IDC_USESTRETCHBLT, settings->UseStretchBlt);
	CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1+settings->WindowModeBackBuffM);
	if(SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		EnableWindow(GetDlgItem(hDlg, IDC_DOUBLESIZE),0);
		EnableWindow(GetDlgItem(hDlg, IDC_USESTRETCHBLT),0);
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK2),1);
	}
	else{
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK2),0);
		EnableWindow(GetDlgItem(hDlg, IDC_DOUBLESIZE),1);
		EnableWindow(GetDlgItem(hDlg, IDC_USESTRETCHBLT),1);
	}
	SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_SETCHECK, (WPARAM)settings->WindowMode_2xsai, 0L);
	if(!settings->WindowMode_2xsai)
		EnableWindow(GetDlgItem(hDlg, IDC_COMBO1),0);
}


//FullScreenMode Setting
static void GRAPHICS_InitDialog3(HWND hDlg, NES_graphics_settings* settings, NES_graphics_settings* active_settings)
{
	/* SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_SETCHECK, (WPARAM)settings->FullscreenModeUseNewMode, 0L);
	  if(settings->FullscreenModeUseNewMode)
		  EnableWindow(GetDlgItem(hDlg, IDC_CHECK4),1);
	  else
		  EnableWindow(GetDlgItem(hDlg, IDC_CHECK4),0);
	*/

	if(settings->fullscreen_scaling==0)			//nnn
		CheckRadioButton(hDlg, IDC_FULLSCREENSCALING1, IDC_FULLSCREENSCALING3,  IDC_FULLSCREENSCALING1);
	else if(settings->fullscreen_scaling==1)
		CheckRadioButton(hDlg, IDC_FULLSCREENSCALING1, IDC_FULLSCREENSCALING3,  IDC_FULLSCREENSCALING2);
	else
		CheckRadioButton(hDlg, IDC_FULLSCREENSCALING1, IDC_FULLSCREENSCALING3,  IDC_FULLSCREENSCALING3);
	CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1+settings->FullscreenModeBackBuffM);
	GRAPHICS_UpdateFullscreenDevices(hDlg, &settings->osd.device_GUID);
	GRAPHICS_UpdateFullscreenModes(hDlg, &settings->osd.device_GUID, settings->osd.fullscreen_width<<20|settings->osd.fullscreen_height<<8|settings->osd.fullscreen_BitDepth);
	CheckDlgButton(hDlg, IDC_DRAWOVERSCAN, settings->draw_overscan);
	CheckDlgButton(hDlg, IDC_FULLSCREENONLOAD, settings->fullscreen_on_load);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK, (WPARAM)settings->FullscreenModeUseDefRefreshRate, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_SETCHECK, (WPARAM)settings->FullscreenModeNotUseFlip, 0L);
	SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_SETCHECK, (WPARAM)settings->FullscreenMode_2xsai, 0L);
	if(!settings->FullscreenMode_2xsai)
		EnableWindow(GetDlgItem(hDlg, IDC_COMBO1),0);
}


BOOL CALLBACK GraphicsOptions_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static NES_graphics_settings saved_settings;
	static NES_graphics_settings settings;
	static HWND hSlider_Tint;
	static HWND hSlider_Hue;
	static OPENFILENAME ofn;
	static char pfname[260] = "";
	static char initdir[260] = "";
	NMHDR *nmhdr;

	switch(message){
	case WM_INITDIALOG:
		saved_settings = NESTER_settings.nes.graphics;
		settings = NESTER_settings.nes.graphics;

		hSlider_Tint = GetDlgItem(hDlg, IDC_TINT);
		hSlider_Hue  = GetDlgItem(hDlg, IDC_HUE);

		memset( &ofn, 0x00, sizeof(OPENFILENAME) );
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hDlg;
		ofn.lpstrFilter = "Palette File (*.pal)\0*.pal\0" \
		                  "All Types (*.*)\0*.*";
		ofn.lpstrFile = pfname;
		ofn.nMaxFile = 260;
		ofn.lpstrInitialDir = initdir;
		ofn.Flags = OFN_HIDEREADONLY;

		GRAPHICS_InitDialog(hDlg, &settings, &saved_settings);
		return TRUE;
		break;
	case WM_HSCROLL:
		if(hSlider_Tint)
		{
			settings.tint = (uint8)SendMessage(hSlider_Tint, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
		}
		if(hSlider_Hue)
		{
			settings.hue = (uint8)SendMessage(hSlider_Hue, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
		}
		// show the results immediately
		NESTER_settings.nes.graphics.tint = settings.tint;
		NESTER_settings.nes.graphics.hue  = settings.hue;
		SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);
		SendMessage(main_window_handle, WM_PAINT, 0, 0);
		return TRUE;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_DEFAULTS:
			settings.SetDefaults();

			GRAPHICS_ClearDevicesComboBox(hDlg);
			GRAPHICS_ClearModesComboBox(hDlg);

			GRAPHICS_InitDialog(hDlg, &settings, &saved_settings);
			return TRUE;
			break;
		case IDC_BLACKANDWHITE:
			GRAPHICS_OnBlackAndWhite_Changed(hDlg, &settings, &saved_settings);
			return TRUE;
			break;
		case IDC_EMULATETV:
			EnableWindow( GetDlgItem(hDlg, IDC_USESTRETCHBLT), !IsDlgButtonChecked( hDlg, IDC_EMULATETV ) );
			return TRUE;
			break;
		case IDC_DEFAULTPALETTE:
		case IDC_READPALETTEFILE:
		case IDC_CALCPALETTE:
		case IDC_EDITPALETTE:		//nnn
			GRAPHICS_ChangePaletteRadioButton( hDlg );
		case IDC_PALETTEFILE:
			GRAPHICS_OnCalculatePalette_Changed(hDlg, &settings, &saved_settings);
			return TRUE;
			break;
		case IDC_REF_PALETTEFILE:
			{
				GetDlgItemText( hDlg, IDC_PALETTEFILE, initdir, 260 );
				DWORD attrib;
				attrib = GetFileAttributes( initdir );
				if( attrib == 0xFFFFFFFF || !( attrib & FILE_ATTRIBUTE_DIRECTORY ) )
				{
					PathRemoveFileSpec( initdir );
					attrib = GetFileAttributes( initdir );
					if( attrib == 0xFFFFFFFF || !( attrib & FILE_ATTRIBUTE_DIRECTORY ) )
						strcpy( initdir, NESTER_settings.path.szAppPath );
				}

				if( GetOpenFileName( &ofn ) )
					SetDlgItemText( hDlg, IDC_PALETTEFILE, ofn.lpstrFile );

				return TRUE;
			}
			break;
		case ID_RESET:
			// reset the tint and hue
			NESTER_settings.nes.graphics.reset_palette();
			if(hSlider_Tint)
			{
				SendMessage(hSlider_Tint, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)NESTER_settings.nes.graphics.tint);
			}
			if(hSlider_Hue)
			{
				SendMessage(hSlider_Hue, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)NESTER_settings.nes.graphics.hue);
			}
			// show the results immediately
			//            NESTER_settings.nes.graphics.tint = settings.tint;
			//            NESTER_settings.nes.graphics.hue  = settings.hue;
			SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);
			SendMessage(main_window_handle, WM_PAINT, 0, 0);
			return TRUE;
			/*
			*/
		}
		break;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			NESTER_settings.nes.graphics.black_and_white = IsDlgButtonChecked(hDlg, IDC_BLACKANDWHITE);
			NESTER_settings.nes.graphics.show_more_than_8_sprites = IsDlgButtonChecked(hDlg, IDC_SHOWSPRITES);
			NESTER_settings.nes.graphics.show_all_scanlines = IsDlgButtonChecked(hDlg, IDC_SHOWALLSCANLINES);
			NESTER_settings.nes.graphics.EmulateTVScanline = IsDlgButtonChecked(hDlg, IDC_EMULATETV);

			NESTER_settings.nes.graphics.calculate_palette = IsDlgButtonChecked(hDlg, IDC_CALCPALETTE)
			        ? 1
			        : IsDlgButtonChecked(hDlg, IDC_READPALETTEFILE)
			        ? 2
			        : IsDlgButtonChecked(hDlg, IDC_EDITPALETTE) //nnn
			        ? 3
			        : 0;
			GetDlgItemText( hDlg, IDC_PALETTEFILE, NESTER_settings.nes.graphics.szPaletteFile, 260 );
			if(hSlider_Tint){
				NESTER_settings.nes.graphics.tint = (uint8)SendMessage(hSlider_Tint, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			}
			if(hSlider_Hue){
				NESTER_settings.nes.graphics.hue = (uint8)SendMessage(hSlider_Hue, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			}
			NESTER_settings.nes.graphics.ScreenShotUseBmp = IsDlgButtonChecked(hDlg, IDC_CHECK1);
			NESTER_settings.nes.graphics.DisableSpriteClipping = IsDlgButtonChecked(hDlg, IDC_CHECK2);
			NESTER_settings.nes.graphics.DisableBackGClipping = IsDlgButtonChecked(hDlg, IDC_CHECK3);
			return TRUE;
			break;
		case PSN_RESET:
			NESTER_settings.nes.graphics = saved_settings;
			return TRUE;
		}
		break;
	}
	return FALSE;
}



//windowmode
BOOL CALLBACK GraphicsOptions_DlgProc2(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static NES_graphics_settings saved_settings;
	static NES_graphics_settings settings;
	NMHDR *nmhdr;

	switch(message)
	{
	case WM_INITDIALOG:
		saved_settings = NESTER_settings.nes.graphics;
		settings = NESTER_settings.nes.graphics;
		{
			char *ps[] = {"2xSaI","Super2xSaI","SuperEagle", "New2xSaI"};
			for(int i=0; i<4;i++){
				SendDlgItemMessage(hDlg,IDC_COMBO1,CB_INSERTSTRING,(WPARAM)i,(LPARAM)ps[i]);
			}
			SendDlgItemMessage(hDlg,IDC_COMBO1,CB_SETCURSEL,(WPARAM)settings.WindowMode_2xsaiType,0L);
		}

		GRAPHICS_InitDialog2(hDlg, &settings, &saved_settings);
		return TRUE;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_CHECK1:
			if(SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
				EnableWindow(GetDlgItem(hDlg, IDC_DOUBLESIZE),0);
				EnableWindow(GetDlgItem(hDlg, IDC_USESTRETCHBLT),0);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK2),1);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO1),1);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO2),1);
			}
			else{
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK2),0);
				EnableWindow(GetDlgItem(hDlg, IDC_DOUBLESIZE),1);
				EnableWindow(GetDlgItem(hDlg, IDC_USESTRETCHBLT),1);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO1),0);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO2),0);
			}
			return TRUE;
			break;
		case IDC_CHECK2:
			{
				bool b = (bool)!SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_GETCHECK, (WPARAM)TRUE, 0L);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), b);
				EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), b);
			}
			return TRUE;
		case IDC_CHECK3:
			{
				BOOL b= SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_GETCHECK, (WPARAM)TRUE, 0L);
				EnableWindow(GetDlgItem(hDlg, IDC_COMBO1),b);
			}
			return TRUE;
		case IDC_DEFAULTS:
			settings.SetDefaults();
			GRAPHICS_InitDialog2(hDlg, &settings, &saved_settings);
			return TRUE;
		}
		return TRUE;
		break;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			//			  NESTER_settings.nes.graphics.osd.double_size = IsDlgButtonChecked(hDlg, IDC_DOUBLESIZE);
			NESTER_settings.nes.graphics.UseStretchBlt = IsDlgButtonChecked(hDlg, IDC_USESTRETCHBLT);
			NESTER_settings.nes.graphics.WindowModeUseDdraw = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.WindowModeBackBuffM = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_RADIO2), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.WindowMode_2xsai= (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.WindowMode_2xsaiType= (unsigned char)SendDlgItemMessage(hDlg,IDC_COMBO1,CB_GETCURSEL,0L,0L);
			return TRUE;
			//			  EndDialog(hDlg, TRUE);
			break;
		case PSN_RESET:
			return FALSE;
		}
		break;
	}
	return FALSE;
}



//FullScreen
BOOL CALLBACK GraphicsOptions_DlgProc3(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static NES_graphics_settings saved_settings;
	static NES_graphics_settings settings;
	NMHDR *nmhdr;

	switch(message)
	{
	case WM_INITDIALOG:
		saved_settings = NESTER_settings.nes.graphics;
		settings = NESTER_settings.nes.graphics;
		{
			char *ps[] = {"2xSaI","Super2xSaI","SuperEagle", "New2xSaI"};
			for(int i=0; i<4;i++){
				SendDlgItemMessage(hDlg,IDC_COMBO1,CB_INSERTSTRING,(WPARAM)i,(LPARAM)ps[i]);
			}
			SendDlgItemMessage(hDlg,IDC_COMBO1,CB_SETCURSEL,(WPARAM)settings.FullscreenMode_2xsaiType,0L);
		}

		GRAPHICS_InitDialog3(hDlg, &settings, &saved_settings);
		return TRUE;
		break;
	case WM_COMMAND:
		if(HIWORD(wParam) == CBN_SELCHANGE)
		{
			switch(LOWORD(wParam))
			{
			case IDC_DEVICE:
				{
					int index;
					GUID* ptr;

					// get the index
					index = SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_GETCURSEL, 0, 0);
					if(index == CB_ERR) break;

					// get the GUID ptr
					ptr = (GUID*)SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_GETITEMDATA, (WPARAM)index, 0);
					if((LRESULT)ptr == CB_ERR) break;

					// copy the GUID
					if(ptr == NULL){
						memset(&settings.osd.device_GUID, 0x00, sizeof(GUID));
					}
					else{
						memcpy(&settings.osd.device_GUID, ptr, sizeof(GUID));
					}
				}

				GRAPHICS_UpdateFullscreenModes(hDlg, &settings.osd.device_GUID, settings.osd.fullscreen_width<<20|settings.osd.fullscreen_height<<8|settings.osd.fullscreen_BitDepth);
				return TRUE;
				break;
			case IDC_MODE:
				{
					int index;
					uint32 width;

					// get the index
					index = SendMessage(GetDlgItem(hDlg, IDC_MODE), CB_GETCURSEL, 0, 0);
					if(index == CB_ERR) break;

					// get the width
					width = SendMessage(GetDlgItem(hDlg, IDC_MODE), CB_GETITEMDATA, (WPARAM)index, 0);
					if(index == CB_ERR) break;

					// copy the width in
					settings.osd.fullscreen_BitDepth= width&0xff;
					settings.osd.fullscreen_width = width>>20;
					settings.osd.fullscreen_height= (width>>8)&0x0fff;
				}
				return TRUE;
			}
		}
		else
		{
			switch(LOWORD(wParam))
			{
			case IDC_CHECK3:
				if(SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_GETCHECK, (WPARAM)TRUE, 0L))
					EnableWindow(GetDlgItem(hDlg, IDC_COMBO1),1);
				else
					EnableWindow(GetDlgItem(hDlg, IDC_COMBO1),0);
				return TRUE;
				break;
			case IDC_DEFAULTS:
				settings.SetDefaults();
				GRAPHICS_ClearDevicesComboBox(hDlg);
				GRAPHICS_ClearModesComboBox(hDlg);
				GRAPHICS_InitDialog3(hDlg, &settings, &saved_settings);
				return TRUE;
			}
		}
		return TRUE;
		break;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			NESTER_settings.nes.graphics.osd.fullscreen_BitDepth = settings.osd.fullscreen_BitDepth;
			NESTER_settings.nes.graphics.osd.fullscreen_width = settings.osd.fullscreen_width;
			NESTER_settings.nes.graphics.osd.fullscreen_height = settings.osd.fullscreen_height;
			NESTER_settings.nes.graphics.osd.device_GUID = settings.osd.device_GUID;

			if(IsDlgButtonChecked(hDlg, IDC_FULLSCREENSCALING3))			//nnn
				NESTER_settings.nes.graphics.fullscreen_scaling = 2;
			else if(IsDlgButtonChecked(hDlg, IDC_FULLSCREENSCALING2))
				NESTER_settings.nes.graphics.fullscreen_scaling = 1;
			else
				NESTER_settings.nes.graphics.fullscreen_scaling = 0;
			//				  NESTER_settings.nes.graphics.FullscreenModeUseNewMode = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.FullscreenModeBackBuffM = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_RADIO2), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.draw_overscan = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_DRAWOVERSCAN), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.fullscreen_on_load = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_FULLSCREENONLOAD), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.FullscreenModeUseDefRefreshRate = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.FullscreenModeNotUseFlip = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.FullscreenMode_2xsai = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_GETCHECK, (WPARAM)TRUE, 0L);
			NESTER_settings.nes.graphics.FullscreenMode_2xsaiType= (unsigned char)SendDlgItemMessage(hDlg,IDC_COMBO1,CB_GETCURSEL,0L,0L);
			GRAPHICS_ClearDevicesComboBox(hDlg);
			GRAPHICS_ClearModesComboBox(hDlg);
			return TRUE;
			break;
		case PSN_RESET:
			GRAPHICS_ClearDevicesComboBox(hDlg);
			GRAPHICS_ClearModesComboBox(hDlg);
			return FALSE;
		}
		break;
	}
	return FALSE;
}




/*********************************************************************************************/

static void SOUND_UpdateFilterSettings(HWND hDlg, NES_sound_settings& settings)
{
	CheckDlgButton(hDlg, IDC_SOUND_FILTER_NONE, BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_SOUND_FILTER_LOWPASS, BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_SOUND_FILTER_LOWPASS_WEIGHTED, BST_UNCHECKED);
	if(settings.filter_type == NES_sound_settings::FILTER_NONE)
	{
		CheckDlgButton(hDlg, IDC_SOUND_FILTER_NONE, BST_CHECKED);
	}
	else if(settings.filter_type == NES_sound_settings::FILTER_LOWPASS)
	{
		CheckDlgButton(hDlg, IDC_SOUND_FILTER_LOWPASS, BST_CHECKED);
	}
	else if(settings.filter_type == NES_sound_settings::FILTER_LOWPASS_WEIGHTED)
	{
		CheckDlgButton(hDlg, IDC_SOUND_FILTER_LOWPASS_WEIGHTED, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hDlg, IDC_RADIO1, BST_CHECKED);
	}

	EnableWindow(GetDlgItem(hDlg,IDC_SOUND_FILTER_NONE), settings.enabled);
	EnableWindow(GetDlgItem(hDlg,IDC_SOUND_FILTER_LOWPASS), settings.enabled);
	EnableWindow(GetDlgItem(hDlg,IDC_SOUND_FILTER_LOWPASS_WEIGHTED), settings.enabled);
	EnableWindow(GetDlgItem(hDlg,IDC_RADIO1), settings.enabled);
}

static void SOUND_OnEnableChanged(HWND hDlg, NES_sound_settings* settings)
{
	settings->enabled = IsDlgButtonChecked(hDlg, IDC_SOUNDENABLE);

	EnableWindow(GetDlgItem(hDlg, IDC_SAMPLEBITS), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SAMPLERATE), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_BUFFERLEN), settings->enabled);

	EnableWindow(GetDlgItem(hDlg, IDC_IDEALTRIANGLE), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_RECTANGLE1), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_RECTANGLE2), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_TRIANGLE), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_NOISE), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_DPCM), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_EXT), settings->enabled);

	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER2), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER3), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER4), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER5), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER6), settings->enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER7), settings->enabled);
	/*
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC1), settings->enabled);
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC2), settings->enabled);
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC3), settings->enabled);
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC4), settings->enabled);
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC5), settings->enabled);
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC6), settings->enabled);
	*/
	SOUND_UpdateFilterSettings(hDlg, *settings);
}

int soundvolss(float v){
	float tv;
	if(v==0)
		tv=10;
	else if(v<1)
		tv=20-(v*10);
	else
		tv=10-v;
	return (int)tv;
}


float soundvolsg(int sv){
	float rv;
	if(sv<=10)
		rv=(float)(10-sv);
	else{
		if(sv==20)
			rv=(float)0.1;
		else
			rv=(float)(10-(sv-10))/10;
	}
	return rv;
}


static void SOUND_SetSliderDialog(HWND hDlg, NES_sound_settings* settings){
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER2), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)soundvolss(settings->rectangle1_volumed));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER3), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)soundvolss(settings->rectangle2_volumed));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER4), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)soundvolss(settings->triangle_volumed));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER5), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)soundvolss(settings->noise_volumed));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER6), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)soundvolss(settings->dpcm_volumed));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER7), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)soundvolss(settings->ext_volumed));
}


static void SOUND_InitDialog(HWND hDlg, NES_sound_settings* settings)
{
	HWND hCombo_samplebits;
	HWND hCombo_samplerate;
	HWND hCombo_bufferlen;

	CheckDlgButton(hDlg, IDC_SOUNDENABLE, settings->enabled);

	// fill in channel enables
	CheckDlgButton(hDlg, IDC_RECTANGLE1, settings->rectangle1_enabled);
	CheckDlgButton(hDlg, IDC_RECTANGLE2, settings->rectangle2_enabled);
	CheckDlgButton(hDlg, IDC_TRIANGLE, settings->triangle_enabled);
	CheckDlgButton(hDlg, IDC_NOISE, settings->noise_enabled);
	CheckDlgButton(hDlg, IDC_DPCM, settings->dpcm_enabled);
	CheckDlgButton(hDlg, IDC_EXT, settings->ext_enabled);

	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER2), settings->rectangle1_enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER3), settings->rectangle2_enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER4), settings->triangle_enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER5), settings->noise_enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER6), settings->dpcm_enabled);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER7), settings->ext_enabled);

//	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER8), settings->ext_enabled);
	/*
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC1), settings->rectangle1_enabled);
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC2), settings->rectangle2_enabled);
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC3), settings->triangle_enabled);
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC4), settings->noise_enabled);
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC5), settings->dpcm_enabled);
	  EnableWindow(GetDlgItem(hDlg, IDC_STATIC6), settings->ext_enabled);
	*/
	CheckDlgButton(hDlg, IDC_RECTANGLE2, settings->rectangle2_enabled);
	CheckDlgButton(hDlg, IDC_TRIANGLE, settings->triangle_enabled);
	CheckDlgButton(hDlg, IDC_NOISE, settings->noise_enabled);
	CheckDlgButton(hDlg, IDC_DPCM, settings->dpcm_enabled);
	CheckDlgButton(hDlg, IDC_EXT, settings->ext_enabled);

	CheckDlgButton(hDlg, IDC_IDEALTRIANGLE, settings->ideal_triangle_enabled);

	hCombo_samplebits = GetDlgItem(hDlg, IDC_SAMPLEBITS);
	if(hCombo_samplebits)
	{
		SendMessage(hCombo_samplebits, CB_RESETCONTENT, 0, 0);
		SendMessage(hCombo_samplebits, CB_ADDSTRING, 0, (LPARAM)"8");
		SendMessage(hCombo_samplebits, CB_ADDSTRING, 0, (LPARAM)"16");
		SendMessage(hCombo_samplebits, CB_SETCURSEL,
		            (settings->sample_bits == 16) ? 1 : 0, 0);
	}


	// fill in the sample rate combo box
	hCombo_samplerate = GetDlgItem(hDlg, IDC_SAMPLERATE);

	if(hCombo_samplerate)
	{
		SendMessage(hCombo_samplerate, CB_RESETCONTENT, 0, 0);

		SendMessage(hCombo_samplerate, CB_ADDSTRING, 0, (LPARAM)"11025");
		SendMessage(hCombo_samplerate, CB_ADDSTRING, 0, (LPARAM)"22050");
		SendMessage(hCombo_samplerate, CB_ADDSTRING, 0, (LPARAM)"44100");
		SendMessage(hCombo_samplerate, CB_ADDSTRING, 0, (LPARAM)"48000");

		// select 44100 by default
		SendMessage(hCombo_samplerate, CB_SETCURSEL, 2, 0);

		if(settings->sample_rate == 11025)
			SendMessage(hCombo_samplerate, CB_SETCURSEL, 0, 0);
		else if(settings->sample_rate == 22050)
			SendMessage(hCombo_samplerate, CB_SETCURSEL, 1, 0);
		else if(settings->sample_rate == 44100)
			SendMessage(hCombo_samplerate, CB_SETCURSEL, 2, 0);
		else if(settings->sample_rate == 48000)
			SendMessage(hCombo_samplerate, CB_SETCURSEL, 3, 0);
	}

	// fill in the buffer length combo box
	hCombo_bufferlen = GetDlgItem(hDlg, IDC_BUFFERLEN);

	if(hCombo_bufferlen)
	{
		int i;
		int index;
		char buf[3];

		SendMessage(hCombo_bufferlen, CB_RESETCONTENT, 0, 0);

		if(settings->buffer_len < 1)  settings->buffer_len = 1;
		if(settings->buffer_len > 10) settings->buffer_len = 10;

		for(i = 1; i <= 10; i++)
		{
			sprintf(buf, "%d", i);
			index = SendMessage(hCombo_bufferlen, CB_ADDSTRING, 0, (LPARAM)buf);
			if(index == CB_ERR) continue;
			SendMessage(hCombo_bufferlen, CB_SETITEMDATA, index, (LPARAM)i);
			if(i == (int)settings->buffer_len) SendMessage(hCombo_bufferlen, CB_SETCURSEL, index, 0);
		}
	}
	// disable buttons if no sound
	SOUND_OnEnableChanged(hDlg, settings);

	SendMessage(GetDlgItem(hDlg, IDC_SLIDER2),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,20));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER3),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,20));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER4),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,20));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER5),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,20));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER6),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,20));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER7),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,20));

	SendMessage(GetDlgItem(hDlg, IDC_SLIDER8),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,100));

	SOUND_SetSliderDialog(hDlg, settings);

	SendMessage(GetDlgItem(hDlg, IDC_SLIDER8), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)NESTER_settings.DirectSoundVolume);
	{
		char s[32];
		itoa(NESTER_settings.DirectSoundVolume, s, 10);
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC1), s);
	}

	if(settings->filter_type == NES_sound_settings::FILTER_OLOWPASS){
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT1), TRUE);
	}
	SetDlgItemInt(hDlg, IDC_EDIT1, settings->lowpass_filter_f ,FALSE);
}

BOOL CALLBACK SoundOptions_DlgProc(HWND hDlg, UINT message,
                                   WPARAM wParam, LPARAM lParam)
{
	static NES_sound_settings settings;
	static NES_sound_settings* p_settings;

	switch(message)
	{
	case WM_INITDIALOG:
		p_settings = &NESTER_settings.nes.sound;
		settings = *p_settings;

		SOUND_InitDialog(hDlg, &settings);

		return TRUE;

	case WM_HSCROLL:
		if(GetDlgItem(hDlg, IDC_SLIDER8) == (HWND)lParam){
			char s[32];
			int n;
			n = SendMessage(GetDlgItem(hDlg, IDC_SLIDER8), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			itoa(n, s, 10);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC1), s);
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_DEFAULTS:
			settings.SetDefaults();
			SOUND_InitDialog(hDlg, &settings);
			return TRUE;
		case IDC_SOUNDENABLE:
			// grey out or enable stuff
			SOUND_OnEnableChanged(hDlg, &settings);
			return TRUE;
		case IDC_BUTTON1:
			settings.rectangle1_volumed = 0;
			settings.rectangle2_volumed = 0;
			settings.triangle_volumed = 0;
			settings.noise_volumed = 0;
			settings.dpcm_volumed = 0;
			settings.ext_volumed = 0;
			SOUND_SetSliderDialog(hDlg, &settings);
			return TRUE;

		case IDC_SOUND_FILTER_NONE:
		case IDC_SOUND_FILTER_LOWPASS:
		case IDC_SOUND_FILTER_LOWPASS_WEIGHTED:
		case IDC_RADIO1:
			EnableWindow(GetDlgItem(hDlg,IDC_EDIT1), FALSE);
			if(IsDlgButtonChecked(hDlg, IDC_SOUND_FILTER_NONE))
			{
				settings.filter_type = NES_sound_settings::FILTER_NONE;
			}
			else if(IsDlgButtonChecked(hDlg, IDC_SOUND_FILTER_LOWPASS))
			{
				settings.filter_type = NES_sound_settings::FILTER_LOWPASS;
			}
			else if(IsDlgButtonChecked(hDlg, IDC_SOUND_FILTER_LOWPASS_WEIGHTED))
			{
				settings.filter_type = NES_sound_settings::FILTER_LOWPASS_WEIGHTED;
			}
			else{
				settings.filter_type = NES_sound_settings::FILTER_OLOWPASS;
				EnableWindow(GetDlgItem(hDlg,IDC_EDIT1), TRUE);
			}
			SOUND_UpdateFilterSettings(hDlg, settings);
			return TRUE;

//		case IDOK:
//			EndDialog(hDlg, TRUE);
			return TRUE;

		}
		break;
	case WM_NOTIFY:
		{
			NMHDR *nmhdr = (NMHDR *)lParam;
			switch(nmhdr->code) {
			case PSN_APPLY:
				NESTER_settings.nes.sound.enabled = IsDlgButtonChecked(hDlg, IDC_SOUNDENABLE);
				
				NESTER_settings.nes.sound.rectangle1_enabled = IsDlgButtonChecked(hDlg, IDC_RECTANGLE1);
				NESTER_settings.nes.sound.rectangle2_enabled = IsDlgButtonChecked(hDlg, IDC_RECTANGLE2);
				NESTER_settings.nes.sound.triangle_enabled = IsDlgButtonChecked(hDlg, IDC_TRIANGLE);
				NESTER_settings.nes.sound.noise_enabled = IsDlgButtonChecked(hDlg, IDC_NOISE);
				NESTER_settings.nes.sound.dpcm_enabled = IsDlgButtonChecked(hDlg, IDC_DPCM);
				NESTER_settings.nes.sound.ext_enabled = IsDlgButtonChecked(hDlg, IDC_EXT);
				
				NESTER_settings.nes.sound.rectangle1_volumed= soundvolsg(SendMessage(GetDlgItem(hDlg, IDC_SLIDER2), TBM_GETPOS, (WPARAM)0, (LPARAM)0));
				NESTER_settings.nes.sound.rectangle2_volumed= soundvolsg(SendMessage(GetDlgItem(hDlg, IDC_SLIDER3), TBM_GETPOS, (WPARAM)0, (LPARAM)0));
				NESTER_settings.nes.sound.triangle_volumed= soundvolsg(SendMessage(GetDlgItem(hDlg, IDC_SLIDER4), TBM_GETPOS, (WPARAM)0, (LPARAM)0));
				NESTER_settings.nes.sound.noise_volumed= soundvolsg(SendMessage(GetDlgItem(hDlg, IDC_SLIDER5), TBM_GETPOS, (WPARAM)0, (LPARAM)0));
				NESTER_settings.nes.sound.dpcm_volumed= soundvolsg(SendMessage(GetDlgItem(hDlg, IDC_SLIDER6), TBM_GETPOS, (WPARAM)0, (LPARAM)0));
				NESTER_settings.nes.sound.ext_volumed= soundvolsg(SendMessage(GetDlgItem(hDlg, IDC_SLIDER7), TBM_GETPOS, (WPARAM)0, (LPARAM)0));
				
				NESTER_settings.DirectSoundVolume = SendMessage(GetDlgItem(hDlg, IDC_SLIDER8), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				
				NESTER_settings.nes.sound.ideal_triangle_enabled = IsDlgButtonChecked(hDlg, IDC_IDEALTRIANGLE);
				
				{
					HWND hCombo_samplebits;
					char sample_bits_str[3];
					hCombo_samplebits = GetDlgItem(hDlg, IDC_SAMPLEBITS);
					if(hCombo_samplebits)
					{
						GetWindowText( hCombo_samplebits, sample_bits_str, 3 );
						NESTER_settings.nes.sound.sample_bits = atoi(sample_bits_str);
					}
				}
				
				{
					HWND hCombo_samplerate;
					char sample_rate_str[51] = "";
					
					hCombo_samplerate = GetDlgItem(hDlg, IDC_SAMPLERATE);
					if(hCombo_samplerate)
					{
						GetWindowText( hCombo_samplerate, sample_rate_str, 50 );
						NESTER_settings.nes.sound.sample_rate = atoi(sample_rate_str);
					}
				}
				
				{
					HWND hCombo_bufferlen;
					int index;
					
					hCombo_bufferlen = GetDlgItem(hDlg, IDC_BUFFERLEN);
					if(hCombo_bufferlen)
					{
						index = SendMessage(hCombo_bufferlen, CB_GETCURSEL, 0, 0);
						NESTER_settings.nes.sound.buffer_len = SendMessage(hCombo_bufferlen, CB_GETITEMDATA, index, 0);
					}
				}
				NESTER_settings.nes.sound.lowpass_filter_f = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
				
	//			*p_settings = settings;
				return TRUE;
				break;
			case PSN_RESET:
				return FALSE;
		}
		break;
		}
	}
	return FALSE;
}





BOOL CALLBACK SoundOptions_DlgProc2(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static NES_sound_settings settings;
	static NES_sound_settings* p_settings;

	switch(message)
	{
	case WM_INITDIALOG:
		p_settings = &NESTER_settings.nes.sound;
		settings = *p_settings;
//		SOUND_InitDialog(hDlg, &settings);
		CheckDlgButton(hDlg, IDC_RADIO1 + settings.vrc6_type, TRUE);
		CheckDlgButton(hDlg, IDC_RADIO3 + settings.vrc7_type, TRUE);
		return TRUE;

/*	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
//		case IDC_DEFAULTS:
//			settings.SetDefaults();
//			PRF_InitDialog3(hDlg, &settings);
//			return TRUE;
		}
		break;
**/
	case WM_NOTIFY:
		{
			NMHDR * nmhdr = (NMHDR *)lParam;
			switch(nmhdr->code) {
			case PSN_APPLY:
				NESTER_settings.nes.sound.vrc6_type = IsDlgButtonChecked(hDlg, IDC_RADIO2);
				NESTER_settings.nes.sound.vrc7_type = IsDlgButtonChecked(hDlg, IDC_RADIO4);
				return TRUE;
			case PSN_RESET:
				return TRUE;
			}
		}
		return TRUE;
	}
	return FALSE;
}


/*********************************************************************************************/
//IDD_DIALOG2

BOOL CALLBACK AboutMsgNester_DlgProc(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	switch(message)
	{
	case WM_INITDIALOG:
		{
			int res=0;
			char str[8192], tpname[32], vups[8192];
			if(res=webcheck(PROG_NAME, tpname, vups)){
				if(res==1){
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
					SetWindowText(hDlg, "Version UP!");
					wsprintf(str, "Current Version , %s\r\nNew Version :     %s\r\n\r\n%s", PROG_NAME, tpname, vups);
					SetDlgItemText(hDlg, IDC_EDIT1, (LPTSTR)str);
				}
				else{
					SetWindowText(hDlg, "Not Version up");
					wsprintf(str, "Current Version : %s \r\n %s", tpname, vups);
					SetDlgItemText(hDlg, IDC_EDIT1, (LPTSTR)str);
				}
			}
			else{
				EndDialog(hDlg, FALSE);
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON1:
			ShellExecute(hDlg, "open", "http://www.emulation9.com/r1/", NULL, NULL, SW_SHOWNORMAL);
			return TRUE;
		case IDOK:
			EndDialog(hDlg, TRUE);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


BOOL CALLBACK AboutNester_DlgProc(HWND hDlg, UINT message,
                                  WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		/*{
		  HWND hVersion;
		  char temp[256];
		  strcpy(temp, "nester ");
		  strcat(temp, NESTER_settings.version);
		  hVersion = GetDlgItem(hDlg, IDC_TITLE);
		  SetWindowText(hVersion, temp);
	}*/
		return TRUE;
	case WM_SETCURSOR:
		if(/*(HWND)wParam == GetDlgItem(hDlg, IDC_STATIC1) ||*/ (HWND)wParam == GetDlgItem(hDlg, IDC_STATIC2)){
			HCURSOR hCursor = LoadCursor(g_main_instance, MAKEINTRESOURCE(IDC_CURSOR2));
			SetCursor(hCursor);
			SetWindowLong(hDlg, DWL_MSGRESULT, MAKELONG(TRUE, 0));
		}
		else{
			HCURSOR hCursor = LoadCursor(NULL,IDC_ARROW);
			SetCursor(hCursor);
			SetWindowLong(hDlg, DWL_MSGRESULT, MAKELONG(TRUE, 0));
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
//		case IDC_STATIC1: //IDC_BUTTON1:
//			DialogBox(g_main_instance, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, AboutMsgNester_DlgProc);

//			return TRUE;
		case IDC_STATIC2:
			{
				char url[MAX_PATH];
				GetWindowText(GetDlgItem(hDlg, IDC_STATIC2), url, MAX_PATH);
				ShellExecute(hDlg, "open", url, NULL, NULL, SW_SHOWNORMAL);
			}
			return TRUE;
		case IDOK:
			EndDialog(hDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

BOOL CALLBACK PathsDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	CPathSettings *pPath = &(NESTER_settings.path);
	static LPMALLOC lpMalloc = NULL;
	switch(message)
	{
	case WM_INITDIALOG:
		if( SHGetMalloc( &lpMalloc ) != ERROR_SUCCESS )
			EndDialog( hDlg, -1 );
		SetDlgItemText( hDlg, IDC_SRAMPATH, pPath->szSramPath );
		SetDlgItemText( hDlg, IDC_STATEPATH, pPath->szStatePath );
		SetDlgItemText( hDlg, IDC_SHOTPATH, pPath->szShotPath );
		SetDlgItemText( hDlg, IDC_WAVEPATH, pPath->szWavePath );
		SetDlgItemText( hDlg, IDC_EDIT, pPath->szMoviePath);
		SetDlgItemText( hDlg, IDC_EDIT5, pPath->szGameGeniePath);
		SetDlgItemText( hDlg, IDC_EDIT6, pPath->szNNNcheatPath);
		SetDlgItemText( hDlg, IDC_EDIT7, pPath->szIPSPatchPath);
		CheckDlgButton( hDlg, IDC_USE_SRAMPATH,( pPath->UseSramPath ) ? BST_CHECKED : BST_UNCHECKED );
		CheckDlgButton( hDlg, IDC_USE_STATEPATH,( pPath->UseStatePath ) ? BST_CHECKED : BST_UNCHECKED );
		CheckDlgButton( hDlg, IDC_USE_SHOTPATH,( pPath->UseShotPath ) ? BST_CHECKED : BST_UNCHECKED );
		CheckDlgButton( hDlg, IDC_USE_WAVEPATH,( pPath->UseWavePath ) ? BST_CHECKED : BST_UNCHECKED );
		CheckDlgButton( hDlg, IDC_CHECK1,( pPath->UseMoviePath) ? BST_CHECKED : BST_UNCHECKED );
		CheckDlgButton( hDlg, IDC_CHECK2,( pPath->UseGeniePath) ? BST_CHECKED : BST_UNCHECKED );
		CheckDlgButton( hDlg, IDC_CHECK3,( pPath->UseNNNchtPath) ? BST_CHECKED : BST_UNCHECKED );
		CheckDlgButton( hDlg, IDC_CHECK4,( pPath->UseIPSPath) ? BST_CHECKED : BST_UNCHECKED );
		return TRUE;
	case WM_DROPFILES:
		{
			char dir[MAX_PATH];
			DragQueryFile( (HDROP)wParam, 0, dir, MAX_PATH );
			if( !( GetFileAttributes( dir ) & FILE_ATTRIBUTE_DIRECTORY ) )
				PathRemoveFileSpec( dir );
			PathAddBackslash( dir );

			POINT point;
			DragQueryPoint( (HDROP)wParam, &point );

			switch( GetDlgCtrlID( ChildWindowFromPoint( hDlg, point ) ) )
			{
case IDC_LABEL_SRAM:	case IDC_USE_SRAMPATH:
case IDC_SRAMPATH:		case IDC_REF_SRAMPATH:
				SetDlgItemText( hDlg, IDC_SRAMPATH, dir );
				break;

case IDC_LABEL_STATE:	case IDC_USE_STATEPATH:
case IDC_STATEPATH:		case IDC_REF_STATEPATH:
				SetDlgItemText( hDlg, IDC_STATEPATH, dir );
				break;

case IDC_LABEL_SHOT:	case IDC_USE_SHOTPATH:
case IDC_SHOTPATH:		case IDC_REF_SHOTPATH:
				SetDlgItemText( hDlg, IDC_SHOTPATH, dir );
				break;

case IDC_LABEL_WAVE:	case IDC_USE_WAVEPATH:
case IDC_WAVEPATH:		case IDC_REF_WAVEPATH:
				SetDlgItemText( hDlg, IDC_WAVEPATH, dir );
				break;

			case IDC_EDIT:		//Movie
				SetDlgItemText( hDlg, IDC_EDIT, dir );
				break;
			case IDC_EDIT5:		//Genie
				SetDlgItemText( hDlg, IDC_EDIT5, dir );
				break;
			case IDC_EDIT6:		//NNN cht
				SetDlgItemText( hDlg, IDC_EDIT6, dir );
				break;
			case IDC_EDIT7:		//IPS
				SetDlgItemText( hDlg, IDC_EDIT7, dir );
				break;
			}
			DragFinish( (HDROP)wParam );
			return TRUE;
		}

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_REF_SRAMPATH:
		case IDC_REF_STATEPATH:
		case IDC_REF_SHOTPATH:
		case IDC_REF_WAVEPATH:
		case IDC_BUTTON3:
		case IDC_BUTTON4:
		case IDC_BUTTON5:
		case IDC_BUTTON6:
			{
				char dir[MAX_PATH];
				char str[64];
				int idctrl;
				BROWSEINFO bi;
				memset( &bi, 0x00, sizeof(BROWSEINFO) );

				switch(LOWORD(wParam))
				{
				case IDC_REF_SRAMPATH:
					LoadString(g_main_instance, IDS_STRING_DLGF_01 , str, 64);
					idctrl = IDC_SRAMPATH;
					break;

				case IDC_REF_STATEPATH:
					LoadString(g_main_instance, IDS_STRING_DLGF_02 , str, 64);
					idctrl = IDC_STATEPATH;
					break;

				case IDC_REF_SHOTPATH:
					LoadString(g_main_instance, IDS_STRING_DLGF_03 , str, 64);
					idctrl = IDC_SHOTPATH;
					break;

				case IDC_REF_WAVEPATH:
					LoadString(g_main_instance, IDS_STRING_DLGF_04 , str, 64);
					idctrl = IDC_WAVEPATH;
					break;

				case IDC_BUTTON3:	//movie
					LoadString(g_main_instance, IDS_STRING_DLGF_05 , str, 64);
					idctrl = IDC_EDIT;
					break;
				case IDC_BUTTON4:	//Genie
					LoadString(g_main_instance, IDS_STRING_DLGF_06 , str, 64);
					idctrl = IDC_EDIT5;
					break;
				case IDC_BUTTON5:	//NNN cht
					LoadString(g_main_instance, IDS_STRING_DLGF_07 , str, 64);
					idctrl = IDC_EDIT6;
					break;
				case IDC_BUTTON6:	//IPS
					LoadString(g_main_instance, IDS_STRING_DLGF_08 , str, 64);
					idctrl = IDC_EDIT7;
					break;
				}

				bi.hwndOwner = hDlg;
				bi.ulFlags = BIF_RETURNONLYFSDIRS;
				LPITEMIDLIST lpiil;

				if( lpiil = SHBrowseForFolder( &bi ) )
				{
					if( SHGetPathFromIDList( lpiil, dir ) )
					{
						PathAddBackslash( dir );
						SetDlgItemText( hDlg, idctrl, dir );
					}
					lpMalloc->Free( lpiil );
				}
				return TRUE;
			}
		case IDOK:
			GetDlgItemText( hDlg, IDC_SRAMPATH, pPath->szSramPath, MAX_PATH );
			PathAddBackslash( pPath->szSramPath );

			GetDlgItemText( hDlg, IDC_STATEPATH, pPath->szStatePath, MAX_PATH );
			PathAddBackslash( pPath->szStatePath );

			GetDlgItemText( hDlg, IDC_SHOTPATH, pPath->szShotPath, MAX_PATH );
			PathAddBackslash( pPath->szWavePath );

			GetDlgItemText( hDlg, IDC_WAVEPATH, pPath->szWavePath, MAX_PATH );
			PathAddBackslash( pPath->szWavePath );

			GetDlgItemText( hDlg, IDC_EDIT, pPath->szMoviePath, MAX_PATH );
			PathAddBackslash( pPath->szWavePath );

			GetDlgItemText( hDlg, IDC_EDIT5, pPath->szGameGeniePath, MAX_PATH );
			PathAddBackslash( pPath->szWavePath );

			GetDlgItemText( hDlg, IDC_EDIT6, pPath->szNNNcheatPath, MAX_PATH );
			PathAddBackslash( pPath->szWavePath );

			GetDlgItemText( hDlg, IDC_EDIT7, pPath->szIPSPatchPath, MAX_PATH );
			PathAddBackslash( pPath->szWavePath );


			pPath->UseSramPath =
			    ( IsDlgButtonChecked( hDlg, IDC_USE_SRAMPATH ) == BST_CHECKED );

			pPath->UseStatePath =
			    ( IsDlgButtonChecked( hDlg, IDC_USE_STATEPATH ) == BST_CHECKED );

			pPath->UseShotPath =
			    ( IsDlgButtonChecked( hDlg, IDC_USE_SHOTPATH ) == BST_CHECKED );

			pPath->UseWavePath =
			    ( IsDlgButtonChecked( hDlg, IDC_USE_WAVEPATH ) == BST_CHECKED );
			pPath->UseMoviePath =( IsDlgButtonChecked( hDlg, IDC_CHECK1) == BST_CHECKED );
			pPath->UseGeniePath =( IsDlgButtonChecked( hDlg, IDC_CHECK2) == BST_CHECKED );
			pPath->UseNNNchtPath =( IsDlgButtonChecked( hDlg, IDC_CHECK3) == BST_CHECKED );
			pPath->UseIPSPath =( IsDlgButtonChecked( hDlg, IDC_CHECK4) == BST_CHECKED );
			lpMalloc->Release();
			EndDialog( hDlg, TRUE );
			return TRUE;

		case IDCANCEL:
			lpMalloc->Release();
			EndDialog( hDlg, FALSE );
			return TRUE;

		case IDC_DEFAULTS:
			{
				char tmp[MAX_PATH], *p;
				GetModuleFileName(NULL, tmp, MAX_PATH);
				PathRemoveFileSpec(tmp);

				for(p=tmp; *p; p++);
				strcpy(p, "\\save\\");
				SetDlgItemText( hDlg, IDC_SRAMPATH, tmp );

				strcpy(p, "\\state\\" );
				SetDlgItemText( hDlg, IDC_STATEPATH, tmp );

				strcpy( p, "\\shot\\" );
				SetDlgItemText( hDlg, IDC_SHOTPATH, tmp );

				strcpy( p, "\\wave\\" );
				SetDlgItemText( hDlg, IDC_WAVEPATH, tmp );

				strcpy( p, "\\movie\\" );
				SetDlgItemText( hDlg, IDC_EDIT, tmp );
				strcpy( p, "\\genie\\" );
				SetDlgItemText( hDlg, IDC_EDIT5, tmp );
				strcpy( p, "\\cheat\\" );
				SetDlgItemText( hDlg, IDC_EDIT6, tmp );
				strcpy( p, "\\ips\\" );
				SetDlgItemText( hDlg, IDC_EDIT7, tmp );

				CheckDlgButton( hDlg, IDC_USE_SRAMPATH, BST_CHECKED );
				CheckDlgButton( hDlg, IDC_USE_STATEPATH, BST_UNCHECKED );
				CheckDlgButton( hDlg, IDC_USE_WAVEPATH, BST_CHECKED );
				CheckDlgButton( hDlg, IDC_CHECK1, BST_UNCHECKED );
				CheckDlgButton( hDlg, IDC_CHECK2, BST_UNCHECKED );
				CheckDlgButton( hDlg, IDC_CHECK3, BST_UNCHECKED );
				CheckDlgButton( hDlg, IDC_CHECK4, BST_UNCHECKED );
				return TRUE;
			}
		}
	}
	return FALSE;
}







/*********************************************************************************************/
/*********************************************************************************************/
// input config
extern char *dikeystr[256];

#define KEY_UP_X		50
#define KEY_UP_Y		45
#define KEY_DOWN_X		50
#define KEY_DOWN_Y		140
#define KEY_LEFT_X		5
#define KEY_LEFT_Y		95
#define KEY_RIGHT_X		100
#define KEY_RIGHT_Y		95
#define KEY_SELECT_X	130
#define KEY_SELECT_Y	140
#define KEY_START_X		200
#define KEY_START_Y		140
#define KEY_B_X			270
#define KEY_B_Y			140
#define KEY_A_X			340
#define KEY_A_Y			140
#define KEY_BF_X		270
#define KEY_BF_Y		55
#define KEY_AF_X		340
#define KEY_AF_Y		55
#define KEY_MIC_X		200
#define KEY_MIC_Y		55

#define KEY_STR_W		60
#define KEY_STR_H		16



void SetControllerDlgAutoFireItem(HWND hDlg, int afs){
	CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1+afs-1);
	char str[32];
	switch(afs){
		case 0:
			SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK, (WPARAM)FALSE, 0L);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), FALSE);
			LoadString(g_main_instance, IDS_STRING_DLGC_01 , str, 32);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC1), str);
			LoadString(g_main_instance, IDS_STRING_DLGC_02 , str, 32);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC2), str);
			break;
		case 1:
			LoadString(g_main_instance, IDS_STRING_DLGC_03 , str, 32);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC1), str);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC2), "");
			SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), TRUE);
			break;
		case 2:
			LoadString(g_main_instance, IDS_STRING_DLGC_04 , str, 32);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC1), str);
			LoadString(g_main_instance, IDS_STRING_DLGC_05 , str, 32);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC2), str);
			SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), TRUE);
			break;
	}
}


void InitControllerDlg(HWND hDlg, NES_controller_input_settings *ncs){
	SetDlgItemInt(hDlg, IDC_EDIT1, ncs->btnTBsec, FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT2, ncs->btnTAsec, FALSE);
	SetControllerDlgAutoFireItem(hDlg, ncs->AutoFireToggle);

	SendMessage(GetDlgItem(hDlg, IDC_SLIDER1),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,30));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER2),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,30));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER1), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)ncs->btnTBsec);
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER2), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)ncs->btnTAsec);
}

void ButtonToStr(OSD_ButtonSettings *bs, char *s){
	switch(bs->type){
	case D_NONE:
		strcpy(s, "NONE");
		return;
	case D_KEYBOARD_KEY:
		wsprintf(s, "%s", dikeystr[bs->j_offset]);
		return;
	case D_JOYSTICK_AXIS:
		wsprintf(s, "J%u ", bs->dev_n);
		if(bs->j_offset){
			if(bs->j_axispositive)
				strcat(s, "X+");
			else
				strcat(s, "X-");
		}
		else{
			if(bs->j_axispositive)
				strcat(s, "Y+");
			else
				strcat(s, "Y-");
		}
		return;
	case D_JOYSTICK_BUTTON:
		wsprintf(s, "J%u B%u", bs->dev_n, bs->j_offset);
		return;
	}
}


void SetActiveButtonItem(HWND hDlg, HDC hdc, int n, NES_controller_input_settings *cs, int pn, OSD_ButtonSettings *mbs){
	char str[32];
	int j, i = IDC_EDIT1 + n;
	RECT rect;
	LPBITMAPINFO lpbi = NULL;
	unsigned char data[(KEY_STR_W+4)*(KEY_STR_H+4)];
	RGBQUAD rgbq[2];
	HFONT hFont;

	lpbi = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
	lpbi->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	lpbi->bmiHeader.biWidth=KEY_STR_W+4;
	lpbi->bmiHeader.biHeight=KEY_STR_H+4;
	lpbi->bmiHeader.biPlanes=1;
	lpbi->bmiHeader.biBitCount=8;
	lpbi->bmiHeader.biCompression=BI_RGB;
	lpbi->bmiHeader.biSizeImage=0;
	lpbi->bmiHeader.biXPelsPerMeter=0;
	lpbi->bmiHeader.biYPelsPerMeter=0;
	lpbi->bmiHeader.biClrUsed=0;
	lpbi->bmiHeader.biClrImportant=0;
	for(i=0;i<KEY_STR_H+4;++i){
		for(j=0;j<KEY_STR_W+4;++j)
			if(j>=KEY_STR_W+2||j<=1||i<=1||i>=KEY_STR_H+2)
				data[i*(KEY_STR_W+4)+j]=0;
			else
				data[i*(KEY_STR_W+4)+j]=1;
	}
#if 0
	memset(data, 0, sizeof(data));
#endif

	rgbq[0].rgbBlue=0xff;
	rgbq[0].rgbGreen=0;
	rgbq[0].rgbRed=0;
	rgbq[1].rgbBlue=0;
	rgbq[1].rgbGreen=0;
	rgbq[1].rgbRed=0;

	lpbi->bmiColors[2].rgbBlue	=0;
	lpbi->bmiColors[2].rgbGreen	=0;
	lpbi->bmiColors[2].rgbRed	=0;
	lpbi->bmiColors[3].rgbBlue	=0xe1;
	lpbi->bmiColors[3].rgbGreen	=0x69;
	lpbi->bmiColors[3].rgbRed	=0x41;
#if 1
if(n==0){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
	else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
	SetDIBitsToDevice(hdc, KEY_UP_X, KEY_UP_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	if(n==1){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
	else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
	SetDIBitsToDevice(hdc, KEY_DOWN_X, KEY_DOWN_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	if(n==2){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
	else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
	SetDIBitsToDevice(hdc, KEY_LEFT_X, KEY_LEFT_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	if(n==3){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
	else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
	SetDIBitsToDevice(hdc, KEY_RIGHT_X, KEY_RIGHT_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	if(n==4){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
	else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
	SetDIBitsToDevice(hdc, KEY_SELECT_X, KEY_SELECT_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	if(n==5){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
	else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
	SetDIBitsToDevice(hdc, KEY_START_X, KEY_START_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	if(n==6){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
	else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
	SetDIBitsToDevice(hdc, KEY_B_X, KEY_B_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	if(n==7){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
	else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
	SetDIBitsToDevice(hdc, KEY_A_X, KEY_A_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	if(n==8){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
	else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
	SetDIBitsToDevice(hdc, KEY_BF_X, KEY_BF_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	if(n==9){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
	else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
	SetDIBitsToDevice(hdc, KEY_AF_X, KEY_AF_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	if(pn==2){
		if(n==10){ lpbi->bmiColors[0]=rgbq[0]; lpbi->bmiColors[1]=lpbi->bmiColors[2];}
		else{lpbi->bmiColors[0]=rgbq[1]; lpbi->bmiColors[1]=lpbi->bmiColors[3];}
		SetDIBitsToDevice(hdc, KEY_MIC_X, KEY_MIC_Y, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
	}

#endif
	SetBkMode(hdc, 0);
	SetTextColor(hdc, RGB(255, 255, 255));
	hFont = CreateFontIndirect(&Default_Font);
	SelectObject(hdc, hFont);

	ButtonToStr(&cs->btnUp, str);
	rect.left = KEY_UP_X+2, rect.right = KEY_UP_X+2+KEY_STR_W, rect.top = KEY_UP_Y+2, rect.bottom = KEY_UP_Y+2+KEY_STR_H;
	DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	ButtonToStr(&cs->btnDown, str);
	rect.left = KEY_DOWN_X+2, rect.right = KEY_DOWN_X+2+KEY_STR_W, rect.top = KEY_DOWN_Y+2, rect.bottom = KEY_DOWN_Y+2+KEY_STR_H;
	DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	ButtonToStr(&cs->btnLeft, str);
	rect.left = KEY_LEFT_X+2, rect.right = KEY_LEFT_X+2+KEY_STR_W, rect.top = KEY_LEFT_Y+2, rect.bottom = KEY_LEFT_Y+2+KEY_STR_H;
	DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	ButtonToStr(&cs->btnRight, str);
	rect.left = KEY_RIGHT_X+2, rect.right = KEY_RIGHT_X+2+KEY_STR_W, rect.top = KEY_RIGHT_Y+2, rect.bottom = KEY_RIGHT_Y+2+KEY_STR_H;
	DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	ButtonToStr(&cs->btnSelect, str);
	rect.left = KEY_SELECT_X+2, rect.right = KEY_SELECT_X+2+KEY_STR_W, rect.top = KEY_SELECT_Y+2, rect.bottom = KEY_SELECT_Y+2+KEY_STR_H;
	DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	ButtonToStr(&cs->btnStart, str);
	rect.left = KEY_START_X+2, rect.right = KEY_START_X+2+KEY_STR_W, rect.top = KEY_START_Y+2, rect.bottom = KEY_START_Y+2+KEY_STR_H;
	DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	ButtonToStr(&cs->btnB, str);
	rect.left = KEY_B_X+2, rect.right = KEY_B_X+2+KEY_STR_W, rect.top = KEY_B_Y+2, rect.bottom = KEY_B_Y+2+KEY_STR_H;
	DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	ButtonToStr(&cs->btnA, str);
	rect.left = KEY_A_X+2, rect.right = KEY_A_X+2+KEY_STR_W, rect.top = KEY_A_Y+2, rect.bottom = KEY_A_Y+2+KEY_STR_H;
	DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	ButtonToStr(&cs->btnTB, str);
	rect.left = KEY_BF_X+2, rect.right = KEY_BF_X+2+KEY_STR_W, rect.top = KEY_BF_Y+2, rect.bottom = KEY_BF_Y+2+KEY_STR_H;
	DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	ButtonToStr(&cs->btnTA, str);
	rect.left = KEY_AF_X+2, rect.right = KEY_AF_X+2+KEY_STR_W, rect.top = KEY_AF_Y+2, rect.bottom = KEY_AF_Y+2+KEY_STR_H;
	DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	if(pn==2){
		ButtonToStr(mbs, str);
		rect.left = KEY_MIC_X+2, rect.right = KEY_MIC_X+2+KEY_STR_W, rect.top = KEY_MIC_Y+2, rect.bottom = KEY_MIC_Y+2+KEY_STR_H;
		DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	DeleteObject(hFont);
	if(lpbi)
		free(lpbi);
}


int ClickBtnPosition(int x, int y, int pn){
	int kn;
	if(x >= KEY_UP_X && x <(KEY_UP_X+KEY_STR_W) && y >= KEY_UP_Y && y <(KEY_UP_Y+KEY_STR_H))
		kn = 0;
	else if(x >= KEY_DOWN_X && x <(KEY_DOWN_X+KEY_STR_W) && y >= KEY_DOWN_Y && y <(KEY_DOWN_Y+KEY_STR_H))
		kn = 1;
	else if(x >= KEY_LEFT_X && x <(KEY_LEFT_X+KEY_STR_W) && y >= KEY_LEFT_Y && y <(KEY_LEFT_Y+KEY_STR_H))
		kn = 2;
	else if(x >= KEY_RIGHT_X && x <(KEY_RIGHT_X+KEY_STR_W) && y >= KEY_RIGHT_Y && y <(KEY_RIGHT_Y+KEY_STR_H))
		kn = 3;
	else if(x >= KEY_SELECT_X && x <(KEY_SELECT_X+KEY_STR_W) && y >= KEY_SELECT_Y && y <(KEY_SELECT_Y+KEY_STR_H))
		kn = 4;
	else if(x >= KEY_START_X && x <(KEY_START_X+KEY_STR_W) && y >= KEY_START_Y && y <(KEY_START_Y+KEY_STR_H))
		kn = 5;
	else if(x >= KEY_B_X && x <(KEY_B_X+KEY_STR_W) && y >= KEY_B_Y && y <(KEY_B_Y+KEY_STR_H))
		kn = 6;
	else if(x >= KEY_A_X && x <(KEY_A_X+KEY_STR_W) && y >= KEY_A_Y && y <(KEY_A_Y+KEY_STR_H))
		kn = 7;
	else if(x >= KEY_BF_X && x <(KEY_BF_X+KEY_STR_W) && y >= KEY_BF_Y && y <(KEY_BF_Y+KEY_STR_H))
		kn = 8;
	else if(x >= KEY_AF_X && x <(KEY_AF_X+KEY_STR_W) && y >= KEY_AF_Y && y <(KEY_AF_Y+KEY_STR_H))
		kn = 9;
	else if(pn==2&& x >= KEY_MIC_X && x <(KEY_MIC_X+KEY_STR_W) && y >= KEY_MIC_Y && y <(KEY_MIC_Y+KEY_STR_H))
		kn = 10;
	else
		return -1;
	return kn;
}

int g_psActivePage=0;



void CheckJoystickDevice_Player(NES_controller_input_settings *psettings){
	OSD_ButtonSettings tmpbs;
	
	if(psettings->btnA.type >= D_JOYSTICK_BUTTON){
		tmpbs = psettings->btnA;
		ginp_mgr->Setos(&tmpbs, &psettings->btnA);
	}
	if(psettings->btnB.type >= D_JOYSTICK_BUTTON){
		tmpbs = psettings->btnB;
		ginp_mgr->Setos(&tmpbs, &psettings->btnB);
	}
	if(psettings->btnDown.type >= D_JOYSTICK_BUTTON){
		tmpbs = psettings->btnDown;
		ginp_mgr->Setos(&tmpbs, &psettings->btnDown);
	}
	if(psettings->btnLeft.type >= D_JOYSTICK_BUTTON){
		tmpbs = psettings->btnLeft;
		ginp_mgr->Setos(&tmpbs, &psettings->btnLeft);
	}
	if(psettings->btnRight.type >= D_JOYSTICK_BUTTON){
		tmpbs = psettings->btnRight;
		ginp_mgr->Setos(&tmpbs, &psettings->btnRight);
	}
	if(psettings->btnSelect.type >= D_JOYSTICK_BUTTON){
		tmpbs = psettings->btnSelect;
		ginp_mgr->Setos(&tmpbs, &psettings->btnSelect);
	}
	if(psettings->btnStart.type >= D_JOYSTICK_BUTTON){
		tmpbs = psettings->btnStart;
		ginp_mgr->Setos(&tmpbs, &psettings->btnStart);
	}
	if(psettings->btnUp.type >= D_JOYSTICK_BUTTON){
		tmpbs = psettings->btnUp;
		ginp_mgr->Setos(&tmpbs, &psettings->btnUp);
	}
	if(psettings->btnTA.type >= D_JOYSTICK_BUTTON){
		tmpbs = psettings->btnTA;
		ginp_mgr->Setos(&tmpbs, &psettings->btnTA);
	}
	if(psettings->btnTB.type >= D_JOYSTICK_BUTTON){
		tmpbs = psettings->btnTB;
		ginp_mgr->Setos(&tmpbs, &psettings->btnTB);
	}
}



//Player 1
BOOL CALLBACK ControllersOptions_DlgProc1(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static NES_controller_input_settings settings(0);
	static OSD_ButtonSettings preobs;
	static int keyactiven;
	NMHDR *nmhdr;
	HDC hDC;
	PAINTSTRUCT ps;
	int i;
	static BOOL activeflag=0;

	switch(message){
	case WM_INITDIALOG:
		settings = NESTER_settings.nes.input.player1;
		if(ginp_mgr && NESTER_settings.nes.preferences.JoystickUseGUID){
			CheckJoystickDevice_Player(&settings);
		}
		InitControllerDlg(hDlg, &settings);
		keyactiven=0,activeflag=0;
		memset(&preobs, 0, sizeof(OSD_ButtonSettings));
		return TRUE;
	case WM_LBUTTONDOWN:
		{
			i = ClickBtnPosition(LOWORD(lParam), HIWORD(lParam), 1);
			if(i>=0){
				keyactiven = i;
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 1, NULL);
				ReleaseDC(hDlg, hDC);
			}
		}
		return TRUE;
	case WM_RBUTTONDOWN:
		{
			i = ClickBtnPosition(LOWORD(lParam), HIWORD(lParam), 1);
			if(i>=0){
				keyactiven = i;
				OSD_ButtonSettings *tobs;
				switch(keyactiven){
				case 0: tobs = &settings.btnUp; break;
				case 1: tobs = &settings.btnDown; break;
				case 2: tobs = &settings.btnLeft; break;
				case 3: tobs = &settings.btnRight; break;
				case 4: tobs = &settings.btnSelect; break;
				case 5: tobs = &settings.btnStart; break;
				case 6: tobs = &settings.btnB; break;
				case 7: tobs = &settings.btnA; break;
				case 8: tobs = &settings.btnTB; break;
				case 9: tobs = &settings.btnTA; break;
				}
				if(wParam&MK_LBUTTON){
					tobs->type = D_KEYBOARD_KEY;
					tobs->j_offset = DIK_RETURN;
				}
				else
					tobs->type = D_NONE;
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 1, NULL);
				ReleaseDC(hDlg, hDC);
			}
		}
		return TRUE;
	case WM_PAINT:
		hDC = BeginPaint(hDlg, &ps);
		SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 1, NULL);
		EndPaint(hDlg, &ps);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_BUTTON1:
			settings.Clear();
			hDC = GetDC(hDlg);
			SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 1, NULL);
			ReleaseDC(hDlg, hDC);
			break;
		case IDC_CHECK1:
			{
				int flag;
				if(!IsDlgButtonChecked(hDlg, IDC_CHECK1))
					flag = 0;
				else 
					flag = 1;
				settings.AutoFireToggle = flag;
				SetControllerDlgAutoFireItem(hDlg, flag);
			}
			break;
		case IDC_RADIO1:
		case IDC_RADIO2:
			{
				int flag;
				if(IsDlgButtonChecked(hDlg, IDC_RADIO1))
					flag = 1;
				else 
					flag = 2;
				settings.AutoFireToggle = flag;
				SetControllerDlgAutoFireItem(hDlg, flag);
			}
			break;
		}
		return TRUE;
	case WM_TIMER:
		if(GetForegroundWindow() != GetParent(hDlg))
			return TRUE;
		KillTimer(hDlg, ID_CTRDLGTIMER);
		if(g_psActivePage!=1){
			activeflag=0;
			return TRUE;
		}
		if(ginp_mgr){
			OSD_ButtonSettings tDii;
			memset(&tDii, 0, sizeof(OSD_ButtonSettings));
			ginp_mgr->ScanInputDevice(&tDii);
			if(tDii.type && memcmp((void *)&preobs, (void *)&tDii, sizeof(OSD_ButtonSettings))){
				OSD_ButtonSettings *tobs;
				switch(keyactiven){
				case 0: tobs = &settings.btnUp; break;
				case 1: tobs = &settings.btnDown; break;
				case 2: tobs = &settings.btnLeft; break;
				case 3: tobs = &settings.btnRight; break;
				case 4: tobs = &settings.btnSelect; break;
				case 5: tobs = &settings.btnStart; break;
				case 6: tobs = &settings.btnB; break;
				case 7: tobs = &settings.btnA; break;
				case 8: tobs = &settings.btnTB; break;
				case 9: tobs = &settings.btnTA; break;
				}
				keyactiven= ++keyactiven%10;
				memcpy(tobs, &tDii, sizeof(OSD_ButtonSettings));
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 1, NULL);
				ReleaseDC(hDlg, hDC);
				SetFocus(GetDlgItem(hDlg, IDC_BUTTON3));
				//					do{
				//						ginp_mgr->ScanInputDevice(&tDii);
				//					}while(tDii.type);
			}
			memcpy(&preobs, &tDii, sizeof(OSD_ButtonSettings));
		}
		//			if(activeflag)
		SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		return TRUE;
	case WM_HSCROLL:
		if(GetDlgItem(hDlg, IDC_SLIDER1) == (HWND)lParam){
			settings.btnTBsec = SendMessage(GetDlgItem(hDlg, IDC_SLIDER1), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			SetDlgItemInt(hDlg, IDC_EDIT1, settings.btnTBsec, FALSE);
		}
		else if(GetDlgItem(hDlg, IDC_SLIDER2) == (HWND)lParam){
			settings.btnTAsec = SendMessage(GetDlgItem(hDlg, IDC_SLIDER2), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			SetDlgItemInt(hDlg, IDC_EDIT2, settings.btnTAsec, FALSE);
		}
		return TRUE;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			NESTER_settings.nes.input.player1 = settings;
			return TRUE;
		case PSN_RESET:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			return FALSE;
		}
	case PSN_SETACTIVE:
		if(!activeflag){
			g_psActivePage=1;
			activeflag=1;
			SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK ControllersOptions_DlgProc2(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static NES_controller_input_settings settings(0);
	static OSD_ButtonSettings preobs;
	static OSD_ButtonSettings mics;
	static int keyactiven;
	NMHDR *nmhdr;
	HDC hDC;
	PAINTSTRUCT ps;
	int i;
	static BOOL activeflag=0;

	switch(message){
	case WM_INITDIALOG:
		settings = NESTER_settings.nes.input.player2;
		mics = NESTER_settings.nes.input.MicButton;
		if(ginp_mgr && NESTER_settings.nes.preferences.JoystickUseGUID){
			CheckJoystickDevice_Player(&settings);
		}
		InitControllerDlg(hDlg, &settings);
		keyactiven=0,activeflag=0;
		memset(&preobs, 0, sizeof(OSD_ButtonSettings));
		return TRUE;
	case WM_LBUTTONDOWN:
		{
			i = ClickBtnPosition(LOWORD(lParam), HIWORD(lParam), 2);
			if(i>=0){
				keyactiven = i;
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 2, &mics);
				ReleaseDC(hDlg, hDC);
			}
		}
		return TRUE;
	case WM_RBUTTONDOWN:
		{
			i = ClickBtnPosition(LOWORD(lParam), HIWORD(lParam), 2);
			if(i>=0){
				keyactiven = i;
				OSD_ButtonSettings *tobs;
				switch(keyactiven){
				case 0: tobs = &settings.btnUp; break;
				case 1: tobs = &settings.btnDown; break;
				case 2: tobs = &settings.btnLeft; break;
				case 3: tobs = &settings.btnRight; break;
				case 4: tobs = &settings.btnSelect; break;
				case 5: tobs = &settings.btnStart; break;
				case 6: tobs = &settings.btnB; break;
				case 7: tobs = &settings.btnA; break;
				case 8: tobs = &settings.btnTB; break;
				case 9: tobs = &settings.btnTA; break;
				case 10: tobs = &mics; break;
				}
				if(wParam&MK_LBUTTON){
					tobs->type = D_KEYBOARD_KEY;
					tobs->j_offset = DIK_RETURN;
				}
				else
					tobs->type = D_NONE;
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 2, &mics);
				ReleaseDC(hDlg, hDC);
			}
		}
		return TRUE;
	case WM_PAINT:
		hDC = BeginPaint(hDlg, &ps);
		SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 2, &mics);
		EndPaint(hDlg, &ps);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_BUTTON1:
			settings.Clear();
			hDC = GetDC(hDlg);
			SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 2, &mics);
			ReleaseDC(hDlg, hDC);
			break;
		case IDC_CHECK1:
			{
				int flag;
				if(!IsDlgButtonChecked(hDlg, IDC_CHECK1))
					flag = 0;
				else 
					flag = 1;
				settings.AutoFireToggle = flag;
				SetControllerDlgAutoFireItem(hDlg, flag);
			}
			break;
		case IDC_RADIO1:
		case IDC_RADIO2:
			{
				int flag;
				if(IsDlgButtonChecked(hDlg, IDC_RADIO1))
					flag = 1;
				else 
					flag = 2;
				settings.AutoFireToggle = flag;
				SetControllerDlgAutoFireItem(hDlg, flag);
			}
			break;
		}
		return TRUE;
	case WM_TIMER:
		if(GetForegroundWindow() != GetParent(hDlg))
			return TRUE;
		KillTimer(hDlg, ID_CTRDLGTIMER);
		if(g_psActivePage!=2){
			activeflag=0;
			return TRUE;
		}
		if(ginp_mgr){
			OSD_ButtonSettings tDii;
			memset(&tDii, 0, sizeof(OSD_ButtonSettings));
			ginp_mgr->ScanInputDevice(&tDii);
			if(tDii.type && memcmp((void *)&preobs, (void *)&tDii, sizeof(OSD_ButtonSettings))){
				OSD_ButtonSettings *tobs;
				switch(keyactiven){
				case 0: tobs = &settings.btnUp; break;
				case 1: tobs = &settings.btnDown; break;
				case 2: tobs = &settings.btnLeft; break;
				case 3: tobs = &settings.btnRight; break;
				case 4: tobs = &settings.btnSelect; break;
				case 5: tobs = &settings.btnStart; break;
				case 6: tobs = &settings.btnB; break;
				case 7: tobs = &settings.btnA; break;
				case 8: tobs = &settings.btnTB; break;
				case 9: tobs = &settings.btnTA; break;
				case 10: tobs = &mics; break;
				}
				keyactiven= ++keyactiven%11;
				memcpy(tobs, &tDii, sizeof(OSD_ButtonSettings));
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 2, &mics);
				ReleaseDC(hDlg, hDC);
				SetFocus(GetDlgItem(hDlg, IDC_BUTTON3));
			}
			memcpy(&preobs, &tDii, sizeof(OSD_ButtonSettings));
		}
		//			if(activeflag)
		SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		return TRUE;
	case WM_HSCROLL:
		if(GetDlgItem(hDlg, IDC_SLIDER1) == (HWND)lParam){
			settings.btnTBsec = SendMessage(GetDlgItem(hDlg, IDC_SLIDER1), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			SetDlgItemInt(hDlg, IDC_EDIT1, settings.btnTBsec, FALSE);
		}
		else if(GetDlgItem(hDlg, IDC_SLIDER2) == (HWND)lParam){
			settings.btnTAsec = SendMessage(GetDlgItem(hDlg, IDC_SLIDER2), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			SetDlgItemInt(hDlg, IDC_EDIT2, settings.btnTAsec, FALSE);
		}
		return TRUE;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			NESTER_settings.nes.input.player2 = settings;
			NESTER_settings.nes.input.MicButton = mics;
			return TRUE;
		case PSN_RESET:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			return FALSE;
		}
	case PSN_SETACTIVE:
		if(!activeflag){
			g_psActivePage=2;
			activeflag=1;
			SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK ControllersOptions_DlgProc3(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static NES_controller_input_settings settings(0);
	static int keyactiven;
	static OSD_ButtonSettings preobs;
	NMHDR *nmhdr;
	HDC hDC;
	PAINTSTRUCT ps;
	int i;
	static BOOL activeflag=0;

	switch(message){
	case WM_INITDIALOG:
		settings = NESTER_settings.nes.input.player3;
		if(ginp_mgr && NESTER_settings.nes.preferences.JoystickUseGUID){
			CheckJoystickDevice_Player(&settings);
		}
		InitControllerDlg(hDlg, &settings);
		keyactiven=0,activeflag=0;
		memset(&preobs, 0, sizeof(OSD_ButtonSettings));
		return TRUE;
	case WM_LBUTTONDOWN:
		{
			i = ClickBtnPosition(LOWORD(lParam), HIWORD(lParam), 3);
			if(i>=0){
				keyactiven = i;
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 3, NULL);
				ReleaseDC(hDlg, hDC);
			}
		}
		return TRUE;
	case WM_RBUTTONDOWN:
		{
			i = ClickBtnPosition(LOWORD(lParam), HIWORD(lParam), 3);
			if(i>=0){
				keyactiven = i;
				OSD_ButtonSettings *tobs;
				switch(keyactiven){
				case 0: tobs = &settings.btnUp; break;
				case 1: tobs = &settings.btnDown; break;
				case 2: tobs = &settings.btnLeft; break;
				case 3: tobs = &settings.btnRight; break;
				case 4: tobs = &settings.btnSelect; break;
				case 5: tobs = &settings.btnStart; break;
				case 6: tobs = &settings.btnB; break;
				case 7: tobs = &settings.btnA; break;
				case 8: tobs = &settings.btnTB; break;
				case 9: tobs = &settings.btnTA; break;
				}
				if(wParam&MK_LBUTTON){
					tobs->type = D_KEYBOARD_KEY;
					tobs->j_offset = DIK_RETURN;
				}
				else
					tobs->type = D_NONE;
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 3, NULL);
				ReleaseDC(hDlg, hDC);
			}
		}
		return TRUE;
	case WM_PAINT:
		hDC = BeginPaint(hDlg, &ps);
		SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 3, NULL);
		EndPaint(hDlg, &ps);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_BUTTON1:
			settings.Clear();
			hDC = GetDC(hDlg);
			SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 3, NULL);
			ReleaseDC(hDlg, hDC);
			break;
		case IDC_CHECK1:
			{
				int flag;
				if(!IsDlgButtonChecked(hDlg, IDC_CHECK1))
					flag = 0;
				else 
					flag = 1;
				settings.AutoFireToggle = flag;
				SetControllerDlgAutoFireItem(hDlg, flag);
			}
			break;
		case IDC_RADIO1:
		case IDC_RADIO2:
			{
				int flag;
				if(IsDlgButtonChecked(hDlg, IDC_RADIO1))
					flag = 1;
				else 
					flag = 2;
				settings.AutoFireToggle = flag;
				SetControllerDlgAutoFireItem(hDlg, flag);
			}
			break;
		}
		return TRUE;
	case WM_TIMER:
		if(GetForegroundWindow() != GetParent(hDlg))
			return TRUE;
		KillTimer(hDlg, ID_CTRDLGTIMER);
		if(g_psActivePage!=3){
			activeflag=0;
			return TRUE;
		}
		if(ginp_mgr){
			OSD_ButtonSettings tDii;
			memset(&tDii, 0, sizeof(OSD_ButtonSettings));
			ginp_mgr->ScanInputDevice(&tDii);
			if(tDii.type && memcmp((void *)&preobs, (void *)&tDii, sizeof(OSD_ButtonSettings))){
				OSD_ButtonSettings *tobs;
				switch(keyactiven){
				case 0: tobs = &settings.btnUp; break;
				case 1: tobs = &settings.btnDown; break;
				case 2: tobs = &settings.btnLeft; break;
				case 3: tobs = &settings.btnRight; break;
				case 4: tobs = &settings.btnSelect; break;
				case 5: tobs = &settings.btnStart; break;
				case 6: tobs = &settings.btnB; break;
				case 7: tobs = &settings.btnA; break;
				case 8: tobs = &settings.btnTB; break;
				case 9: tobs = &settings.btnTA; break;
				}
				keyactiven= ++keyactiven%10;
				memcpy(tobs, &tDii, sizeof(OSD_ButtonSettings));
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 3, NULL);
				ReleaseDC(hDlg, hDC);
				SetFocus(GetDlgItem(hDlg, IDC_BUTTON3));
			}
			memcpy(&preobs, &tDii, sizeof(OSD_ButtonSettings));
		}
		//			if(activeflag)
		SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		return TRUE;
	case WM_HSCROLL:
		if(GetDlgItem(hDlg, IDC_SLIDER1) == (HWND)lParam){
			settings.btnTBsec = SendMessage(GetDlgItem(hDlg, IDC_SLIDER1), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			SetDlgItemInt(hDlg, IDC_EDIT1, settings.btnTBsec, FALSE);
		}
		else if(GetDlgItem(hDlg, IDC_SLIDER2) == (HWND)lParam){
			settings.btnTAsec = SendMessage(GetDlgItem(hDlg, IDC_SLIDER2), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			SetDlgItemInt(hDlg, IDC_EDIT2, settings.btnTAsec, FALSE);
		}
		return TRUE;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			NESTER_settings.nes.input.player3 = settings;
			return TRUE;
		case PSN_RESET:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			return FALSE;
		}
	case PSN_SETACTIVE:
		if(!activeflag){
			g_psActivePage=3;
			activeflag=1;
			SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK ControllersOptions_DlgProc4(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static NES_controller_input_settings settings(0);
	static int keyactiven;
	static OSD_ButtonSettings preobs;
	NMHDR *nmhdr;
	HDC hDC;
	PAINTSTRUCT ps;
	int i;
	static BOOL activeflag;

	switch(message){
	case WM_INITDIALOG:
		settings = NESTER_settings.nes.input.player4;
		if(ginp_mgr && NESTER_settings.nes.preferences.JoystickUseGUID){
			CheckJoystickDevice_Player(&settings);
		}
		InitControllerDlg(hDlg, &settings);
		keyactiven=0,activeflag=0;
		memset(&preobs, 0, sizeof(OSD_ButtonSettings));
		return TRUE;
	case WM_LBUTTONDOWN:
		{
			i = ClickBtnPosition(LOWORD(lParam), HIWORD(lParam), 4);
			if(i>=0){
				keyactiven = i;
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 3, NULL);
				ReleaseDC(hDlg, hDC);
			}
		}
		return TRUE;
	case WM_RBUTTONDOWN:
		{
			i = ClickBtnPosition(LOWORD(lParam), HIWORD(lParam), 4);
			if(i>=0){
				keyactiven = i;
				OSD_ButtonSettings *tobs;
				switch(keyactiven){
				case 0: tobs = &settings.btnUp; break;
				case 1: tobs = &settings.btnDown; break;
				case 2: tobs = &settings.btnLeft; break;
				case 3: tobs = &settings.btnRight; break;
				case 4: tobs = &settings.btnSelect; break;
				case 5: tobs = &settings.btnStart; break;
				case 6: tobs = &settings.btnB; break;
				case 7: tobs = &settings.btnA; break;
				case 8: tobs = &settings.btnTB; break;
				case 9: tobs = &settings.btnTA; break;
				}
				if(wParam&MK_LBUTTON){
					tobs->type = D_KEYBOARD_KEY;
					tobs->j_offset = DIK_RETURN;
				}
				else
					tobs->type = D_NONE;
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 4, NULL);
				ReleaseDC(hDlg, hDC);
			}
		}
		return TRUE;
	case WM_PAINT:
		hDC = BeginPaint(hDlg, &ps);
		SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 4, NULL);
		EndPaint(hDlg, &ps);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_BUTTON1:
			settings.Clear();
			hDC = GetDC(hDlg);
			SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 4, NULL);
			ReleaseDC(hDlg, hDC);
			break;
		case IDC_CHECK1:
			{
				int flag;
				if(!IsDlgButtonChecked(hDlg, IDC_CHECK1))
					flag = 0;
				else 
					flag = 1;
				settings.AutoFireToggle = flag;
				SetControllerDlgAutoFireItem(hDlg, flag);
			}
			break;
		case IDC_RADIO1:
		case IDC_RADIO2:
			{
				int flag;
				if(IsDlgButtonChecked(hDlg, IDC_RADIO1))
					flag = 1;
				else 
					flag = 2;
				settings.AutoFireToggle = flag;
				SetControllerDlgAutoFireItem(hDlg, flag);
			}
			break;
		}
		return TRUE;
	case WM_TIMER:
		if(GetForegroundWindow() != GetParent(hDlg))
			return TRUE;
		KillTimer(hDlg, ID_CTRDLGTIMER);
		if(g_psActivePage!=4){
			activeflag=0;
			return TRUE;
		}
		if(ginp_mgr){
			OSD_ButtonSettings tDii;
			memset(&tDii, 0, sizeof(OSD_ButtonSettings));
			ginp_mgr->ScanInputDevice(&tDii);
			if(tDii.type && memcmp((void *)&preobs, (void *)&tDii, sizeof(OSD_ButtonSettings))){
				OSD_ButtonSettings *tobs;
				switch(keyactiven){
				case 0: tobs = &settings.btnUp; break;
				case 1: tobs = &settings.btnDown; break;
				case 2: tobs = &settings.btnLeft; break;
				case 3: tobs = &settings.btnRight; break;
				case 4: tobs = &settings.btnSelect; break;
				case 5: tobs = &settings.btnStart; break;
				case 6: tobs = &settings.btnB; break;
				case 7: tobs = &settings.btnA; break;
				case 8: tobs = &settings.btnTB; break;
				case 9: tobs = &settings.btnTA; break;
				}
				keyactiven= ++keyactiven%10;
				memcpy(tobs, &tDii, sizeof(OSD_ButtonSettings));
				hDC = GetDC(hDlg);
				SetActiveButtonItem(hDlg, hDC, keyactiven, &settings, 4, NULL);
				ReleaseDC(hDlg, hDC);
				SetFocus(GetDlgItem(hDlg, IDC_BUTTON3));
			}
			memcpy(&preobs, &tDii, sizeof(OSD_ButtonSettings));
		}
		//			if(activeflag)
		SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		return TRUE;
	case WM_HSCROLL:
		if(GetDlgItem(hDlg, IDC_SLIDER1) == (HWND)lParam){
			settings.btnTBsec = SendMessage(GetDlgItem(hDlg, IDC_SLIDER1), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			SetDlgItemInt(hDlg, IDC_EDIT1, settings.btnTBsec, FALSE);
		}
		else if(GetDlgItem(hDlg, IDC_SLIDER2) == (HWND)lParam){
			settings.btnTAsec = SendMessage(GetDlgItem(hDlg, IDC_SLIDER2), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
			SetDlgItemInt(hDlg, IDC_EDIT2, settings.btnTAsec, FALSE);
		}
		return TRUE;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			NESTER_settings.nes.input.player4 = settings;
			return TRUE;
		case PSN_RESET:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			return FALSE;
		}
	case PSN_SETACTIVE:
		if(!activeflag){
			g_psActivePage=4;
			activeflag=1;
			SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		}
		return TRUE;
	}
	return FALSE;}


#define EKEY_0_X 150
#define EKEY_0_Y 25


void SetActiveExtButtonItem(HWND hDlg, HDC hdc, int n, OSD_ButtonSettings *cs){
	char str[32];
	int j, i;
	RECT rect;
	LPBITMAPINFO lpbi = NULL;
	unsigned char data[(KEY_STR_W+4)*(KEY_STR_H+4)];
	RGBQUAD rgbq[2];
	HFONT hFont;

	lpbi = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
	lpbi->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	lpbi->bmiHeader.biWidth=KEY_STR_W+4;
	lpbi->bmiHeader.biHeight=KEY_STR_H+4;
	lpbi->bmiHeader.biPlanes=1;
	lpbi->bmiHeader.biBitCount=8;
	lpbi->bmiHeader.biCompression=BI_RGB;
	lpbi->bmiHeader.biSizeImage=0;
	lpbi->bmiHeader.biXPelsPerMeter=0;
	lpbi->bmiHeader.biYPelsPerMeter=0;
	lpbi->bmiHeader.biClrUsed=0;
	lpbi->bmiHeader.biClrImportant=0;
	for(i=0;i<KEY_STR_H+4;++i){
		for(j=0;j<KEY_STR_W+4;++j)
			if(j>=KEY_STR_W+2||j<=1||i<=1||i>=KEY_STR_H+2)
				data[i*(KEY_STR_W+4)+j]=0;
			else
				data[i*(KEY_STR_W+4)+j]=1;
	}
#if 0
	memset(data, 0, sizeof(data));
#endif

	rgbq[0].rgbBlue=0xff;
	rgbq[0].rgbGreen=0;
	rgbq[0].rgbRed=0;
	rgbq[1].rgbBlue=0;
	rgbq[1].rgbGreen=0;
	rgbq[1].rgbRed=0;

	lpbi->bmiColors[2].rgbBlue	=0;
	lpbi->bmiColors[2].rgbGreen	=0;
	lpbi->bmiColors[2].rgbRed	=0;
	lpbi->bmiColors[3].rgbBlue	=0xe1;
	lpbi->bmiColors[3].rgbGreen	=0x69;
	lpbi->bmiColors[3].rgbRed	=0x41;
#if 1

	for(i=0; i<2; ++i){
		for(j=0;j<8;++j){
			if(n==(i*8+j)){
				lpbi->bmiColors[0]=rgbq[0];
				lpbi->bmiColors[1]=lpbi->bmiColors[2];
			}
			else{
				lpbi->bmiColors[0]=rgbq[1];
				lpbi->bmiColors[1]=lpbi->bmiColors[3];
			}
			SetDIBitsToDevice(hdc, EKEY_0_X+i*230, EKEY_0_Y+j*30, KEY_STR_W+4, KEY_STR_H+4, 0,0, 0, KEY_STR_H+4, &data, lpbi, DIB_RGB_COLORS);
		}
	}

#endif
	SetBkMode(hdc, 0);
	SetTextColor(hdc, RGB(255, 255, 255));
	hFont = CreateFontIndirect(&Default_Font);
	SelectObject(hdc, hFont);

	for(i=0;i<2;++i){
		for(j=0;j<8;++j){
			ButtonToStr(&cs[i*8+j], str);
			rect.left = i*230+EKEY_0_X+2, rect.right = i*230+EKEY_0_X+2+KEY_STR_W, rect.top = j*30+EKEY_0_Y+2, rect.bottom = j*30+EKEY_0_Y+2+KEY_STR_H;
			DrawText(hdc, str, strlen(str), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
	}
	DeleteObject(hFont);
	if(lpbi)
		free(lpbi);
}




BOOL CALLBACK ExtButtonControllersOptions_DlgProc(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static OSD_ButtonSettings settings[16];
	static int keyactiven;
	static BOOL activeflag=0;
	static OSD_ButtonSettings preobs;
	NMHDR *nmhdr;
	HDC hDC;
	PAINTSTRUCT ps;
	int i,j;

	switch(message){
	case WM_INITDIALOG:
		for(i=0;i<16;++i){
			settings[i] = NESTER_settings.nes.input.extkeycfg[i];
			if(ginp_mgr && NESTER_settings.nes.preferences.JoystickUseGUID){
				if(settings[i].type >= D_JOYSTICK_BUTTON){
					ginp_mgr->Setos(&NESTER_settings.nes.input.extkeycfg[i], &settings[i]);
				}
			}
		}
		SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK, (WPARAM)NESTER_settings.nes.input.extkeycfgFastFPSToggle, 0L);
		keyactiven=0;
		activeflag=0;
		return TRUE;
	case WM_LBUTTONDOWN:
		{
			int x,y;
			x=LOWORD(lParam);
			y=HIWORD(lParam);
			for(i=0; i<2; ++i){
				for(j=0;j<8;++j){
					if(x >= EKEY_0_X+i*230 && x < EKEY_0_X+i*230+KEY_STR_W && y >= EKEY_0_Y+j*30 && y < EKEY_0_Y+j*30+KEY_STR_H){
						keyactiven = i*8+j;
						hDC = GetDC(hDlg);
						SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
						ReleaseDC(hDlg, hDC);
					}
				}
			}
		}
		return TRUE;
	case WM_RBUTTONDOWN:
		{
			int x,y;
			x=LOWORD(lParam);
			y=HIWORD(lParam);
			for(i=0; i<2; ++i){
				for(j=0;j<8;++j){
					if(x >= EKEY_0_X+i*230 && x < EKEY_0_X+i*230+KEY_STR_W && y >= EKEY_0_Y+j*30 && y < EKEY_0_Y+j*30+KEY_STR_H){
						if(wParam&MK_LBUTTON){
							settings[i*8+j].type = D_KEYBOARD_KEY;
							settings[i*8+j].j_offset = DIK_RETURN;
						}
						else
							settings[i*8+j].type=D_NONE;
						hDC = GetDC(hDlg);
						SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
						ReleaseDC(hDlg, hDC);
					}
				}
			}
		}
		return TRUE;
	case WM_PAINT:
		hDC = BeginPaint(hDlg, &ps);
		SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
		EndPaint(hDlg, &ps);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_BUTTON1:
			for(i=0;i<16;++i){
				settings[i].type = D_NONE;
			}
			hDC = GetDC(hDlg);
			SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
			ReleaseDC(hDlg, hDC);
			break;
		case IDC_BUTTON2:
			for(i=0;i<16;++i){
				settings[i].type = D_NONE;
			}
			settings[3].type = D_KEYBOARD_KEY;		// FDS 1A
			settings[3].j_offset = DIK_A;
			settings[4].type = D_KEYBOARD_KEY;		// FDS 1B
			settings[4].j_offset = DIK_B;
			settings[5].type = D_KEYBOARD_KEY;		// FDS 2A
			settings[5].j_offset = DIK_C;
			settings[6].type = D_KEYBOARD_KEY;		// FDS 2B
			settings[6].j_offset = DIK_D;
			settings[7].type = D_KEYBOARD_KEY;		// FDS EJECT
			settings[7].j_offset = DIK_E;
			settings[12].type = D_KEYBOARD_KEY;		// Screen Shot
			settings[12].j_offset = DIK_F12;
			settings[14].type = D_KEYBOARD_KEY;		// Wave Record
			settings[14].j_offset = DIK_F11;
			hDC = GetDC(hDlg);
			SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
			ReleaseDC(hDlg, hDC);
			break;
		}
		return TRUE;
	case WM_TIMER:
		if(GetForegroundWindow() != GetParent(hDlg))
			return TRUE;
		KillTimer(hDlg, ID_CTRDLGTIMER);
		if(g_psActivePage!=5){
			activeflag=0;
			return TRUE;
		}
		if(ginp_mgr){
			OSD_ButtonSettings tDii;
			memset(&tDii, 0, sizeof(OSD_ButtonSettings));
			ginp_mgr->ScanInputDevice(&tDii);
			if(tDii.type && memcmp((void *)&preobs, (void *)&tDii, sizeof(OSD_ButtonSettings))){
				settings[keyactiven] = tDii;
				keyactiven= ++keyactiven%16;
				hDC = GetDC(hDlg);
				SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
				ReleaseDC(hDlg, hDC);
				SetFocus(GetDlgItem(hDlg, IDC_BUTTON3));
			}
			memcpy(&preobs, &tDii, sizeof(OSD_ButtonSettings));
		}
		//			if(activeflag)
		SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		return TRUE;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			for(i=0;i<16;++i){
				NESTER_settings.nes.input.extkeycfg[i] = settings[i];
			}
			NESTER_settings.nes.input.extkeycfgFastFPSToggle = IsDlgButtonChecked(hDlg, IDC_CHECK1);
			return TRUE;
		case PSN_RESET:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			return FALSE;
		}
	case PSN_SETACTIVE:
		if(!activeflag){
			SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
			activeflag=1;
			g_psActivePage=5;
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CALLBACK ExtButtonControllersOptions_DlgProc2(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static OSD_ButtonSettings settings[16];
	static int keyactiven;
	static BOOL activeflag=0;
	static OSD_ButtonSettings preobs;
	NMHDR *nmhdr;
	HDC hDC;
	PAINTSTRUCT ps;
	int i,j;

	switch(message){
	case WM_INITDIALOG:
		for(i=0;i<8;++i){
			settings[i] = NESTER_settings.nes.input.extkeycfg[16+i];
			if(ginp_mgr && NESTER_settings.nes.preferences.JoystickUseGUID){
				if(settings[i].type >= D_JOYSTICK_BUTTON){
					ginp_mgr->Setos(&NESTER_settings.nes.input.extkeycfg[i], &settings[i]);
				}
			}
		}
		keyactiven=0;
		activeflag=0;
		return TRUE;
	case WM_LBUTTONDOWN:
		{
			int x,y;
			x=LOWORD(lParam);
			y=HIWORD(lParam);
			for(i=0; i<2; ++i){
				for(j=0;j<8;++j){
					if(x >= EKEY_0_X+i*230 && x < EKEY_0_X+i*230+KEY_STR_W && y >= EKEY_0_Y+j*30 && y < EKEY_0_Y+j*30+KEY_STR_H){
						if(i*8+j < 8){
							keyactiven = i*8+j;
							hDC = GetDC(hDlg);
							SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
							ReleaseDC(hDlg, hDC);
						}
						return TRUE;
					}
				}
			}
		}
		return TRUE;
	case WM_RBUTTONDOWN:
		{
			int x,y;
			x=LOWORD(lParam);
			y=HIWORD(lParam);
			for(i=0; i<2; ++i){
				for(j=0;j<8;++j){
					if(x >= EKEY_0_X+i*230 && x < EKEY_0_X+i*230+KEY_STR_W && y >= EKEY_0_Y+j*30 && y < EKEY_0_Y+j*30+KEY_STR_H){
						if(i*8+j < 8){
							if(wParam&MK_LBUTTON){
								settings[i*8+j].type = D_KEYBOARD_KEY;
								settings[i*8+j].j_offset = DIK_RETURN;
							}
							else
								settings[i*8+j].type=D_NONE;
							hDC = GetDC(hDlg);
							SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
							ReleaseDC(hDlg, hDC);
						}
						return TRUE;
					}
				}
			}
		}
		return TRUE;
	case WM_PAINT:
		hDC = BeginPaint(hDlg, &ps);
		SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
		EndPaint(hDlg, &ps);
		break;
//	case WM_SETFOCUS:
//		SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));
//		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_BUTTON1:
			for(i=0;i<16;++i){
				settings[i].type = D_NONE;
			}
			hDC = GetDC(hDlg);
			SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
			ReleaseDC(hDlg, hDC);
			break;
		case IDC_BUTTON2:
			for(i=0;i<16;++i){
				settings[i].type = D_NONE;
			}
			settings[0].type = D_KEYBOARD_KEY;		// Quick Save
			settings[0].j_offset = DIK_F5;
			settings[1].type = D_KEYBOARD_KEY;		// Quick Load
			settings[1].j_offset = DIK_F7;
			hDC = GetDC(hDlg);
			SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
			ReleaseDC(hDlg, hDC);
			break;
		}
		return TRUE;
	case WM_TIMER:
		if(GetForegroundWindow() != GetParent(hDlg))
			return TRUE;
		KillTimer(hDlg, ID_CTRDLGTIMER);
		if(g_psActivePage!=6){
			activeflag=0;
			return TRUE;
		}
		if(ginp_mgr){
			OSD_ButtonSettings tDii;
			memset(&tDii, 0, sizeof(OSD_ButtonSettings));
			ginp_mgr->ScanInputDevice(&tDii);
			if(tDii.type && memcmp((void *)&preobs, (void *)&tDii, sizeof(OSD_ButtonSettings))){
				settings[keyactiven] = tDii;
				keyactiven= ++keyactiven%8;
				hDC = GetDC(hDlg);
				SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
				ReleaseDC(hDlg, hDC);
				SetFocus(GetDlgItem(hDlg, IDC_BUTTON3));
			}
			memcpy(&preobs, &tDii, sizeof(OSD_ButtonSettings));
		}
		//			if(activeflag)
		SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		return TRUE;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			for(i=0;i<8;++i){
				NESTER_settings.nes.input.extkeycfg[16+i] = settings[i];
			}
			//				NESTER_settings.nes.input.extkeycfgFastFPSToggle = IsDlgButtonChecked(hDlg, IDC_CHECK1);
			return TRUE;
		case PSN_RESET:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			return FALSE;
		}
	case PSN_SETACTIVE:
		if(!activeflag){
			SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
			activeflag=1;
			g_psActivePage=6;
		}
		return TRUE;
	}
	return FALSE;
}



BOOL CALLBACK ControllersJoyTest_DlgProc(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam){
	static int keyactiven, ActiveJoyn, AllJoyn;
	static BOOL activeflag=0;
	NMHDR *nmhdr;
//	HDC hDC;
//	PAINTSTRUCT ps;
	int i;

	switch(message){
	case WM_INITDIALOG:
		keyactiven=0;
		activeflag=0;
		ActiveJoyn=0;
		AllJoyn = ginp_mgr->GetJoystickn();
		{
			char str[10];
			for(i=0;i<AllJoyn;++i){
				wsprintf(str, "JOY %i", i);
				SendDlgItemMessage(hDlg,IDC_COMBO1,CB_INSERTSTRING,(WPARAM)i,(LPARAM)str);
			}
			SendDlgItemMessage(hDlg,IDC_COMBO1,CB_SETCURSEL, (WPARAM)0,0L);
		}
		return TRUE;
/*
	case WM_PAINT:
		hDC = BeginPaint(hDlg, &ps);
		SetActiveExtButtonItem(hDlg, hDC, keyactiven, settings);
		EndPaint(hDlg, &ps);
		break;
*/
	case WM_COMMAND:
		if(HIWORD(wParam)==CBN_SELCHANGE){
			switch(LOWORD(wParam)){
				case IDC_COMBO1:
					ActiveJoyn= SendDlgItemMessage(hDlg,IDC_COMBO1,CB_GETCURSEL,0L,0L);
					break;
			}
			return TRUE;
		}
		break;
	case WM_TIMER:
		if(GetForegroundWindow() != GetParent(hDlg))
			return TRUE;
		KillTimer(hDlg, ID_CTRDLGTIMER);
		if(g_psActivePage!=7){
			activeflag=0;
			return TRUE;
		}
		if(ginp_mgr){
			DIJOYSTATE *pjs=NULL;
			if(ginp_mgr->GetJoystickState(ActiveJoyn, &pjs)){
				char str[20];
				wsprintf(str, "%i", pjs->lX);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC1), str);
				wsprintf(str, "%i", pjs->lY);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC2), str);
			}
		}
		//			if(activeflag)
		SetTimer(hDlg, ID_CTRDLGTIMER, 100, NULL);
		return TRUE;
	case WM_NOTIFY:
		nmhdr = (NMHDR *)lParam;
		switch(nmhdr->code) {
		case PSN_APPLY:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			return TRUE;
		case PSN_RESET:
			if(activeflag)
				KillTimer(hDlg, ID_CTRDLGTIMER);
			return FALSE;
		case PSN_SETACTIVE:
			if(!activeflag){
				SetTimer(hDlg, ID_CTRDLGTIMER, 50, NULL);
				activeflag=1;
				g_psActivePage=7;
			}
			return TRUE;
		}
	}
	return FALSE;
}

