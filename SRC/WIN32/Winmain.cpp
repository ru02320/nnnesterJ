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
#include <winsock.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <stddef.h>
#include <shlobj.h>

#include <commdlg.h> // for open dialog
#include <cderr.h>

#include <stdlib.h>
#include <mmsystem.h>

#include <richedit.h>

#include "mkutils.h"
#include "iDirectX.h"

#include "resource.h"

#include "win32_dialogs.h"
#include "win32_datach_barcode_dialog.h"

#include "win32_emu.h"
#include "win32_GB_emu.h"
#include "win32_PCE_emu.h"

#include "netplay.h"

#include "settings.h"

#include "debug.h"

#include "cheat.h"
#include "preview.h"
#include "launcher.h"
#include "nnndialog.h"
#include "win32_screen_mgr.h"
#include "win32_directinput_input_mgr.h"
#include "sound_mgr.h"
#include "arc.h"
#include "savecfg.h"
#include "extra_window.h"
#include "nnnkailleraproc.h"
#include "CheckFile.h"




// this will ignore every incoming 0x0118 message
// windows will continue to send it regularly when ignored
#define TOOLTIP_HACK

// WM_* in "winuser.h"
//#define DUMP_WM_MESSAGES

#ifdef DUMP_WM_MESSAGES
#define DUMP_WM_MESSAGE(NUM,MSG) \
  LOG("WM" << (NUM) << ":" << HEX((MSG),4) << ",")
#else
#define DUMP_WM_MESSAGE(NUM,MSG)
#endif


#define WINCLASS_NAME "WinClass_nnnester"

//nesterJ Ver0.502 + alpha


int savestate_slot = 0;

#define SCREEN_WIDTH_WINDOWED   (NESTER_settings.nes.graphics.osd.double_size ? \
								  2*NES_PPU::NES_SCREEN_WIDTH_VIEWABLE : \
								  NES_PPU::NES_SCREEN_WIDTH_VIEWABLE)
#define SCREEN_HEIGHT_WINDOWED  (NESTER_settings.nes.graphics.osd.double_size ? \
								  2*NES_PPU::getViewableHeight() : \
								  NES_PPU::getViewableHeight())

// used for centering
#define APPROX_WINDOW_WIDTH  (SCREEN_WIDTH_WINDOWED + 2*GetSystemMetrics(SM_CXFIXEDFRAME))
#define APPROX_WINDOW_HEIGHT (SCREEN_HEIGHT_WINDOWED + GetSystemMetrics(SM_CYMENU) \
						 + GetSystemMetrics(SM_CYSIZE) \
						 + 2*GetSystemMetrics(SM_CYFIXEDFRAME) \
						 + 1)

#define STYLE_WINDOWED (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | \
	  WS_MINIMIZEBOX /*| *//*WS_MAXIMIZEBOX | *//*WS_POPUP*//* | WS_VISIBLE*/)
#define STYLE_FULLSCREEN (WS_VISIBLE | WS_EX_TOPMOST)

#define INACTIVE_PRIORITY   NORMAL_PRIORITY_CLASS

#define TIMER_ID_MOUSE_HIDE	   1
#define MOUSE_HIDE_DELAY_SECONDS  1
static int hide_mouse;
static UINT mouse_timer = 0;

#define TIMER_ID_UNSTICK_MESSAGE_PUMP 2
#define TIMER_UNSTICK_MILLISECONDS 1
static UINT unstick_timer = 0;

#define TIMER_ID_AUTO_MENU_SHOW 5


// use this after every thaw()
// sends a one-shot timer message
// needed due to the structure of the main message loop
// lets nester achieve <1% CPU usage when idle
#define UNSTICK_MESSAGE_PUMP \
  unstick_timer = SetTimer(main_window_handle, TIMER_ID_UNSTICK_MESSAGE_PUMP, \
						   TIMER_UNSTICK_MILLISECONDS, NULL)

/*
#define EX_NONE				   0
#define EX_ARKANOID_PADDLE		2
#define EX_CRAZY_CLIMBER		  3
#define EX_DATACH_BARCODE_BATTLER 4
#define EX_DOREMIKKO_KEYBOARD	 5
#define EX_EXCITING_BOXING		6
#define EX_FAMILY_KEYBOARD		7
#define EX_FAMILY_TRAINER_A	   8
#define EX_FAMILY_TRAINER_B	   9
#define EX_HYPER_SHOT			 10
#define EX_MAHJONG				11
#define EX_OEKAKIDS_TABLET		12
#define EX_OPTICAL_GUN			13
#define EX_POKKUN_MOGURAA		 14
#define EX_POWER_PAD_A			15
#define EX_POWER_PAD_B			16
#define EX_SPACE_SHADOW_GUN	   17
#define EX_TOP_RIDER			  18
#define EX_TURBO_FILE			 19
#define EX_VS_ZAPPER			  20
#define EX_CONTROLLER_LAST		20
*/
#define USE_MOUSE_CURSOR (emu->GetExControllerType() == EX_OPTICAL_GUN || \
						  emu->GetExControllerType() == EX_SPACE_SHADOW_GUN || \
						  emu->GetExControllerType() == EX_VS_ZAPPER)

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle = NULL; // save the window handle
HINSTANCE g_main_instance = NULL; // save the instance
HINSTANCE g_prg_instance = NULL;

emulator* emu=NULL;
win32_screen_mgr* gscr_mgr=NULL;
win32_directinput_input_mgr* ginp_mgr=NULL;
win32_directsound_sound_mgr* g_snd_mgr;

extern int g_psActivePage;

RECT g_cscreen_rect;

// kaillerachat
HWND g_hKailleraLog=NULL;
HWND g_hKailleraSend=NULL;
int  g_nKailleraChatWinH=0;
int  g_nKailleraChatLogWinCharH=0;
CHARFORMAT g_Kaillera_charfmt;
HINSTANCE	hRichd32Lib=NULL;


static int g_foreground;
static int g_minimized;

int ncbuttondown_flag = 0;
int disable_flag = 0;

extern void setNTSCMode();
extern void setPALMode();


// preview
extern struct Preview_state g_Preview_State;
int Mainwindowsbhs=0;
HWND Mainwindowsbh=NULL;
HWND Mainwindow_Preview=NULL;
int g_PreviewMode=0;
int g_PreviewLClickItemn=0;

int hToolWindow=0;		//ToolWindow handle

int g_previewmode_swfullscreen;
int g_previewmode_loadROM;


extern struct extra_window_struct g_extra_window;

///////////////////////////////////////////////////////////


extern SOCKET sock; // client socket
extern SOCKET sv_sock; // server socket
extern uint8 netplay_status;
extern uint8 netplay_latency;
extern uint8 netplay_disconnect;

//////////


extern unsigned char ctrestoreaf;				//cheat
extern unsigned char ctcmpdt[3][0x800];
extern unsigned char ctcmpdtS[3][0x2000];
extern char previewfn[MAX_PATH];
extern unsigned char previewflag;
extern HWND previewh;
int PrevFastSw;
extern char prevconfig[16];
extern char cheatkeyf;
extern DWORD cheatkeyadr;
extern DWORD cheatkeynum;
//extern char usedispfpsf;
char nnnastflag=0;
extern LOGFONT CheatDlgFont;
extern DWORD CheatDlgFontColor;
extern int nnnKailleraFlag;
extern int nnnKailleraDlgFlag;
char g_ScreenMsgStr[MAX_PATH+20]="";
extern struct cheat_info_struct g_Cheat_Info;

extern LOGFONT launcherFont;
LOGFONT Default_Font;


//extern HWND phList;					//Launcher Listview Handle

unsigned char nnnKailleraLagDisp;
extern struct MenuOD Menustruct[];

HFONT MenuStrFont;


void MyLoadAllResource(){
	LOGFONT *pDefault_Font;
	HGLOBAL hRes;
	HRSRC hRsrc = FindResource(g_main_instance, MAKEINTRESOURCE(IDR_BIN_DEFAULTFONT), "BIN");
	hRes = LoadResource(g_main_instance, hRsrc);
	if(!hRes)
		return;
	pDefault_Font = (LOGFONT *)LockResource(hRes);
	memcpy(&Default_Font, pDefault_Font, sizeof(LOGFONT));
	memcpy(&g_Preview_State.LogFont, pDefault_Font, sizeof(LOGFONT));
	memcpy(&CheatDlgFont, pDefault_Font, sizeof(LOGFONT));
	memcpy(&launcherFont, pDefault_Font, sizeof(LOGFONT));

}



int MyAviFileSaveDialog(HWND hWnd, char *initpath, char *fn){
	OPENFILENAME ofn;
	char szStr[MAX_PATH]="", str[32];

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Avi Files(*.avi)\0*.avi\0All Files(*.*)\0*.*\0\0";
	ofn.lpstrFile = fn;
	ofn.lpstrFileTitle = szStr;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "avi";
	LoadString(g_main_instance, IDS_STRING_CDLG_03 , str, 32);
	ofn.lpstrTitle	=str;
	ofn.lpstrInitialDir = initpath;
	if(GetSaveFileName(&ofn) == 0)
		return 0;
	return 1;
}


int MyWavFileSaveDialog(HWND hWnd, char *initpath, char *fn){
	OPENFILENAME ofn;
	char szStr[MAX_PATH]="", str[32];

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "wav Files(*.wav)\0*.avi\0All Files(*.*)\0*.*\0\0";
	ofn.lpstrFile = fn;
	ofn.lpstrFileTitle = szStr;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "wav";
	LoadString(g_main_instance, IDS_STRING_CDLG_04 , str, 32);
	ofn.lpstrTitle	=str;
	ofn.lpstrInitialDir = initpath;
	if(GetSaveFileName(&ofn) == 0)
		return 0;
	return 1;
}




///////////////////

void InfoDisplay(char *str){
	if(!gscr_mgr)
		return;
	if(gscr_mgr->is_fullscreen()){
		strcpy(g_ScreenMsgStr, str);
		SetTimer(main_window_handle, ID_SCRMSGTIMER, 1000, NULL);
	}
	else if(Mainwindowsbh){
		SendMessage(Mainwindowsbh, SB_SETTEXT, 255, (LPARAM)(LPSTR)str);
		SetTimer(main_window_handle, ID_SCRMSGTIMER, 1000, NULL);
	}
}



void InfoDisplayt(char *str, unsigned int ms){
	if(!gscr_mgr)
		return;
	if(gscr_mgr->is_fullscreen()){
		strcpy(g_ScreenMsgStr, str);
		SetTimer(main_window_handle, ID_SCRMSGTIMER, ms, NULL);
	}
	else if(Mainwindowsbh){
		SendMessage(Mainwindowsbh, SB_SETTEXT, 255, (LPARAM)(LPSTR)str);
		SetTimer(main_window_handle, ID_SCRMSGTIMER, ms, NULL);
	}
}


void SetMainWindowTitle(){
	char wstr[312];
	strcpy(wstr, PROG_NAME);
	if(!NESTER_settings.nes.preferences.DisableGameInfoDisp && emu){
		char gt[256];
		emu->GetGameTitleName(gt);
		if(gt[0]){
			strcat(wstr, " - ");
			strcat(wstr, gt);
		}
	}
	SetWindowText(main_window_handle, wstr);
}

#if 0
void RomHeaderInfoDisplay(unsigned char *th){
	char *p=g_ScreenMsgStr, fsf=0, headerflag[5],headerflag2[5];

	if(gscr_mgr && !gscr_mgr->is_fullscreen())
		p++;
	else
		fsf=1;
	//	memcpy(p, th, 3);
	//	p+=3;
	for(int i=0,j=1; i<4; ++i, j<<=1){
		if(th[6] & j)
			headerflag[i]='1';
		else
			headerflag[i]='0';
		if(th[0x11] & j)
			headerflag2[i]='1';
		else
			headerflag2[i]='0';
	}
	headerflag[4]=0, headerflag2[4]=0;
	if(th[0x10]){
		wsprintf(p, " Mapper [ %u -> %u ], PROM %uKB, CROM %uKB, FLAG[ %s -> %s ]", (th[0x11] >> 4)|(th[0x12] & 0xF0),(th[6] >> 4)|(th[7] & 0xF0), th[4]*16, th[5]*8, headerflag2, headerflag);
	}
	else{
		wsprintf(p, " Mapper [ %u ], PROM %uKB, CROM %uKB, FLAG %s", (th[6] >> 4)|(th[7] & 0xF0), th[4]*16, th[5]*8, headerflag);
	}
	if(!fsf && Mainwindowsbh){
		SendMessage(Mainwindowsbh, SB_SETTEXT, 255, (LPARAM)(LPSTR)&g_ScreenMsgStr[1]);
		SetTimer(main_window_handle, ID_SCRMSGTIMER, 5000, NULL);
	}
	else if(fsf)
		SetTimer(main_window_handle, ID_SCRMSGTIMER, 3000, NULL);
}
#endif


void CreateMainWindowSb(){
	RECT rc;
	Mainwindowsbh = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | SBARS_SIZEGRIP | WS_VISIBLE,
								   0, 0, 0, 0, main_window_handle, (HMENU)ID_STATUSMW, g_main_instance, NULL);
	if(Mainwindowsbh){
		SendMessage(Mainwindowsbh , SB_SIMPLE, TRUE, 0L);
		GetWindowRect(Mainwindowsbh, &rc);
		Mainwindowsbhs = rc.bottom - rc.top;
	}
}


void SetToolWindowItemState(){
	SendMessage(g_Preview_State.hTool, TB_CHECKBUTTON, (WPARAM)IDS_STRING1, (LPARAM)g_Preview_State.TreeDispFlag);
	SendMessage(g_Preview_State.hTool, TB_CHECKBUTTON, (WPARAM)IDS_STRING2, (LPARAM)g_Preview_State.ScreenDispFlag);
	SendMessage(g_Preview_State.hTool, TB_CHECKBUTTON, (WPARAM)IDS_STRING5, (LPARAM)0);
	SendMessage(g_Preview_State.hTool, TB_CHECKBUTTON, (WPARAM)IDS_STRING6, (LPARAM)0);
	SendMessage(g_Preview_State.hTool, TB_CHECKBUTTON, (WPARAM)IDS_STRING7, (LPARAM)0);
	SendMessage(g_Preview_State.hTool, TB_CHECKBUTTON, (WPARAM)IDS_STRING5+g_Preview_State.CheckResaultDisplay, (LPARAM)1);
	SendMessage(g_Preview_State.hTool, TB_CHECKBUTTON, (WPARAM)IDS_STRING10, (LPARAM)g_Preview_State.GameTitleDetailDisplay);
}



void setWindowedWindowStyle()
{
#if 0
	SetWindowLong(main_window_handle, GWL_STYLE, STYLE_WINDOWED | WS_MAXIMIZEBOX |//| (emu ? WS_MAXIMIZEBOX : 0)
				  ((NESTER_settings.nes.graphics.UseStretchBlt || NESTER_settings.nes.graphics.WindowModeUseDdraw || g_PreviewMode) ?WS_THICKFRAME : 0));
	SetWindowPos(main_window_handle, ((NESTER_settings.nes.graphics.osd.WindowTopMost)? HWND_TOPMOST:HWND_NOTOPMOST), 0, 0, 0, 0,
				 SWP_NOMOVE | SWP_NOSIZE | /*SWP_NOZORDER |*/ SWP_FRAMECHANGED | SWP_SHOWWINDOW);
#else
	if(emu && !g_PreviewMode){
		if(NESTER_settings.nes.preferences.GM_WindowStyle_Menu)
			SetMenu(main_window_handle, NULL);
		if(NESTER_settings.nes.preferences.GM_WindowStyle_SBar && Mainwindowsbh)
			ShowWindow(Mainwindowsbh, SW_HIDE);
		if(NESTER_settings.nes.preferences.GM_WindowStyle_Title)
			SetWindowLong(main_window_handle, GWL_STYLE, ((NESTER_settings.nes.graphics.UseStretchBlt || NESTER_settings.nes.graphics.WindowModeUseDdraw || g_PreviewMode) ?WS_THICKFRAME : 0));
		else
			SetWindowLong(main_window_handle, GWL_STYLE, STYLE_WINDOWED | WS_MAXIMIZEBOX |//| (emu ? WS_MAXIMIZEBOX : 0)
					  ((NESTER_settings.nes.graphics.UseStretchBlt || NESTER_settings.nes.graphics.WindowModeUseDdraw || g_PreviewMode) ?WS_THICKFRAME : 0));
		SetWindowPos(main_window_handle, ((NESTER_settings.nes.graphics.osd.WindowTopMost)? HWND_TOPMOST:HWND_NOTOPMOST), 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | /*SWP_NOZORDER |*/ SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		if(Mainwindowsbh)
			ShowWindow(Mainwindowsbh, SW_SHOW);
	}
	else{
		if(GetMenu(main_window_handle)==NULL){
			SetMenu(main_window_handle, LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENU1)));
			if(!NESTER_settings.nes.preferences.DisableMenuIcon)
				SetMenuOwnerDraw(main_window_handle);
		}

		if(Mainwindowsbh)
			ShowWindow(Mainwindowsbh, SW_SHOW);
		SetWindowLong(main_window_handle, GWL_STYLE, STYLE_WINDOWED | WS_MAXIMIZEBOX |//| (emu ? WS_MAXIMIZEBOX : 0)
					  ((NESTER_settings.nes.graphics.UseStretchBlt || NESTER_settings.nes.graphics.WindowModeUseDdraw || g_PreviewMode) ?WS_THICKFRAME : 0));
		SetWindowPos(main_window_handle, ((NESTER_settings.nes.graphics.osd.WindowTopMost)? HWND_TOPMOST:HWND_NOTOPMOST), 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | /*SWP_NOZORDER |*/ SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	}
#endif
}

static void setFullscreenWindowStyle()
{
	if(GetMenu(main_window_handle)==NULL){
		SetMenu(main_window_handle, LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENU1)));
		if(!NESTER_settings.nes.preferences.DisableMenuIcon)
			SetMenuOwnerDraw(main_window_handle);
	}

	SetWindowLong(main_window_handle, GWL_STYLE, STYLE_FULLSCREEN);
	SetWindowPos(main_window_handle, HWND_NOTOPMOST, 0, 0, 0, 0,
				 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

static void assertWindowStyle()
{
	//  if(emu)
	{
		if(gscr_mgr->is_fullscreen())	//gscr_mgr->is_fullscreen()
		{
			setFullscreenWindowStyle();
			return;
		}
		else{
			setWindowedWindowStyle();
		}
	}

}

static void set_priority(DWORD priority)
{
	// set process priority
	HANDLE pid;

	pid = GetCurrentProcess();

	SetPriorityClass(pid, priority);
}

static void assert_priority()
{
	switch(NESTER_settings.nes.preferences.priority)
	{
	case NES_preferences_settings::PRI_NORMAL:
		set_priority(NORMAL_PRIORITY_CLASS);
		break;

	case NES_preferences_settings::PRI_HIGH:
		set_priority(HIGH_PRIORITY_CLASS);
		break;

	case NES_preferences_settings::PRI_REALTIME:
		set_priority(REALTIME_PRIORITY_CLASS);
		break;
	}
}




int MyNSLFileSaveDialog(HWND hWnd, char *initpath, char *fn){
	OPENFILENAME ofn;
	char szStr[MAX_PATH]="", str[32];

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Avi Files(*.nsl)\0*.nsl\0All Files(*.*)\0*.*\0\0";
	ofn.lpstrFile = fn;
	ofn.lpstrFileTitle = szStr;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "nsl";
	strcpy(str, "Select Save File");
//	LoadString(g_main_instance, "Select Save File" , str, 32);
	ofn.lpstrTitle	=str;
	ofn.lpstrInitialDir = initpath;
	if(GetSaveFileName(&ofn) == 0)
		return 0;
	return 1;
}


static boolean GetROMFileName(char* filenamebuf, const char* path)
{
	OPENFILENAME OpenFileName;
	//	char szFilter[2048]	   = "All Supported Types|*.nes;*.fds;*.fam;*.nsf";
	//	char szAddingFilter[1024] = "NES Standard Types (*.nes;*.fds;*.fam;*.nsf)|*.nes;*.fds;*.fam;*.nsf|";
	char szFilter[2048]	   = "All Supported Types|*.nes;*.fds;*.fam;*.nsf;*.unf;*.arj;*.lzh;*.lha;*.zip;*.jar;*.rar;*.gca;*.tar;*.tgz;*.tbz;*.gz;*.bz2;*.cab;*.gza;*.bza";
	char szAddingFilter[1024] = "NES Standard Types (*.nes;*.fds;*.fam;*.nsf;*.unf)|*.nes;*.fds;*.fam;*.nsf;*.unf|LHA Archive (*.lzh;*.lha)|*.lzh;*.lha|ZIP Archive (*.zip;*.jar)|*.zip;*.jar|RAR Archive (*.rar)|*.rar|GCA Archive (*.gca)|*.gca|TAPE Archive (*.tar;*.tgz;*.tbz;*.gz;*.bz2)|*.tar;*.tgz;*.tbz;*.gz;*.bz2|MS Cabinet (*.cab)|*.cab|BGA Archive (*.gza;*.bza)|*.gza;*.bza|";

#if 0
	if( GetModuleHandle( "Unarj32j" ) )
	{
		strcat( szFilter, ";*.arj" );
		strcat( szAddingFilter, "ARJ Archive (*.arj)|*.arj|" );
	}
	if( GetModuleHandle( "Unlha32" ) )
	{
		strcat( szFilter, ";*.lzh;*.lzs;*.lha" );
		strcat( szAddingFilter, "LHA Archive (*.lzh;*.lzs;*.lha)|*.lzh;*.lzs;*.lha|" );
	}
	if( GetModuleHandle( "UnZip32" ) )
	{
		strcat( szFilter, ";*.zip;*.jar" );
		strcat( szAddingFilter, "ZIP Archive (*.zip;*.jar)|*.zip;*.jar|" );
	}
	if( GetModuleHandle( "Unrar32" ) )
	{
		strcat( szFilter, ";*.rar" );
		strcat( szAddingFilter, "RAR Archive (*.rar)|*.rar|" );
	}
	if( GetModuleHandle( "Tar32" ) )
	{
		strcat( szFilter, ";*.tar;*.tgz;*.tbz;*.gz;*.bz2" );
		strcat( szAddingFilter,
				"TAPE Archive (*.tar;*.tgz;*.tbz;*.gz;*.bz2)|*.tar;*.tgz;*.tbz;*.gz;*.bz2|"	);
	}
	if( GetModuleHandle( "Cab32" ) )
	{
		strcat( szFilter, ";*.cab" );
		strcat( szAddingFilter, "MS Cabinet (*.cab)|*.cab|" );
	}
	if( GetModuleHandle( "Bga32" ) )
	{
		strcat( szFilter, ";*.gza;*.bza" );
		strcat( szAddingFilter, "BGA Archive (*.gza;*.bza)|*.gza;*.bza|" );
	}
#endif

	strcat( szFilter, "|" );
	strcat( szFilter, szAddingFilter );
	strcat( szFilter, "All Types (*.*)|*.*|" );
	char *p = szFilter;
	while( p = StrChr( p, '|' ) )
	{
		*p = '\0';
		p++;
	}

	/*
		"GameBoy ROM (*.gb;*.gbc)\0*.gb;*.gbc\0" \
		"SNES ROM (*.smc)\0*.smc\0" \
		"N64 ROM (*.n64)\0*.n64\0" \
	*/

	char szFile[1024] = "";
	char szFileTitle[1024];

	int ret;

	memset(&OpenFileName, 0x00, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof (OPENFILENAME);
	OpenFileName.hwndOwner = main_window_handle;
	OpenFileName.hInstance = g_main_instance;
	OpenFileName.lpstrFilter = szFilter;
	OpenFileName.lpstrCustomFilter = NULL;
	OpenFileName.nMaxCustFilter = 0;
	OpenFileName.nFilterIndex = 1;
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = sizeof(szFile);
	OpenFileName.lpstrFileTitle = szFileTitle;
	OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
	if(path)
		OpenFileName.lpstrInitialDir = path;
	else
		OpenFileName.lpstrInitialDir = ".";
	OpenFileName.lpstrTitle = "Open ROM";
	OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
	OpenFileName.nFileOffset = 0;
	OpenFileName.nFileExtension = 0;
	OpenFileName.lpstrDefExt = "nes";
	OpenFileName.lCustData = 0;

	if((ret = GetOpenFileName(&OpenFileName)) == 0)
	{
		return FALSE;
	}

	if(OpenFileName.lpstrFileTitle)
	{
		strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
		return TRUE;
	}

	return FALSE;
}

static boolean GetLoadSavestateFileName(char* filenamebuf, const char* path, const char* RomName)
{
	OPENFILENAME OpenFileName;
	char szFilter[1024];
	char szFile[1024];
	char szFileTitle[1024];

	int ret;

	// create the default filename
	strcpy(szFile, filenamebuf);

	// create the filter
	{
		char* p = szFilter;
		sprintf(p, "savestate (%s.ss?)", RomName);
		p += strlen(p)+1;
		sprintf(p, "%s.ss?", RomName);
		p += strlen(p)+1;
		strcpy(p, "Save State Files(*.ss?)");
		p += strlen(p)+1;
		strcpy(p, "*.ss?");
		p += strlen(p)+1;
		strcpy(p, "Zip Save State Files(*.zs?)");
		p += strlen(p)+1;
		strcpy(p, "*.zs?");
		p += strlen(p)+1;
		strcpy(p, "All Files (*.*)");
		p += strlen(p)+1;
		strcpy(p, "*.*");
		p += strlen(p)+1;
		*p = '\0';
	}

	memset(&OpenFileName, 0x00, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof (OPENFILENAME);
	OpenFileName.hwndOwner = main_window_handle;
	OpenFileName.hInstance = g_main_instance;
	OpenFileName.lpstrFilter = szFilter;
	OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
	OpenFileName.nMaxCustFilter = 0L;
	OpenFileName.nFilterIndex = 1L;
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = sizeof(szFile);
	OpenFileName.lpstrFileTitle = szFileTitle;
	OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
	OpenFileName.lpstrInitialDir = NESTER_settings.path.szLastStatePath;
	OpenFileName.lpstrTitle = "Load State";
	OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
	OpenFileName.nFileOffset = 0;
	OpenFileName.nFileExtension = 0;
	OpenFileName.lpstrDefExt = NULL;
	OpenFileName.lCustData = 0;

	if((ret = GetOpenFileName(&OpenFileName)) == 0)
	{
		return FALSE;
	}

	if(OpenFileName.lpstrFileTitle)
	{
		strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
		strcpy( NESTER_settings.path.szLastStatePath, szFile );
		PathRemoveFileSpec( NESTER_settings.path.szLastStatePath );
		return TRUE;
	}

	return FALSE;
}

static boolean GetSaveSavestateFileName(char* filenamebuf, const char* path, const char* RomName)
{
	OPENFILENAME OpenFileName;
	char szFilter[1024];
	char szFile[1024];
	char szFileTitle[1024];

	int ret;

	// create the default filename
	strcpy(szFile, filenamebuf);

	// create the filter
	{
		char* p = szFilter;
		sprintf(p, "save state (%s.ss?)", RomName);
		p += strlen(p)+1;
		sprintf(p, "%s.ss?", RomName);
		p += strlen(p)+1;
		strcpy(p, "All Files (*.*)");
		p += strlen(p)+1;
		strcpy(p, "*.*");
		p += strlen(p)+1;
		*p = '\0';
	}

	memset(&OpenFileName, 0x00, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof (OPENFILENAME);
	OpenFileName.hwndOwner = main_window_handle;
	OpenFileName.hInstance = g_main_instance;
	OpenFileName.lpstrFilter = szFilter;
	OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
	OpenFileName.nMaxCustFilter = 0L;
	OpenFileName.nFilterIndex = 1L;
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = sizeof(szFile);
	OpenFileName.lpstrFileTitle = szFileTitle;
	OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
	OpenFileName.lpstrInitialDir = NESTER_settings.path.szLastStatePath;
	OpenFileName.lpstrTitle = "Save State";
	OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
	OpenFileName.nFileOffset = 0;
	OpenFileName.nFileExtension = 0;
	OpenFileName.lpstrDefExt = NULL; //"ss0";
	OpenFileName.lCustData = 0;

	if((ret = GetSaveFileName(&OpenFileName)) == 0)
	{
		return FALSE;
	}

	if(OpenFileName.lpstrFileTitle)
	{
		strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
		strcpy( NESTER_settings.path.szLastStatePath, szFile );
		PathRemoveFileSpec( NESTER_settings.path.szLastStatePath );
		return TRUE;
	}

	return FALSE;
}

BOOL GetMovieFileName(char* filenamebuf, char *extn, char *filter)
{
	OPENFILENAME OpenFileName;
	char szFile[1024];
	char szFileTitle[1024];

	char buf[256];
	GetModuleFileName(NULL, buf, 256);
	PathRemoveFileSpec(buf);
	if(NESTER_settings.path.UseMoviePath){
		PathCombine(buf , buf, NESTER_settings.path.szMoviePath);
	}
	else{
		PathCombine(buf , buf, "movie");
		CreateDirectory(buf, NULL);
	}

	int ret;

	// create the default filename
	sprintf(szFile, "%s.%s", emu->getROMname(), extn);

	memset(&OpenFileName, 0x00, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof (OPENFILENAME);
	OpenFileName.hwndOwner = main_window_handle;
	OpenFileName.hInstance = g_main_instance;
	OpenFileName.lpstrFilter = filter;
	OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
	OpenFileName.nMaxCustFilter = 0L;
	OpenFileName.nFilterIndex = 1L;
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = sizeof(szFile);
	OpenFileName.lpstrFileTitle = szFileTitle;
	OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
	OpenFileName.lpstrInitialDir = buf;
	OpenFileName.lpstrTitle = "movie file";
	OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
	OpenFileName.nFileOffset = 0;
	OpenFileName.nFileExtension = 0;
	OpenFileName.lpstrDefExt = extn;
	OpenFileName.lCustData = 0;

	if((ret = GetOpenFileName(&OpenFileName)) == 0)
	{
		return FALSE;
	}

	if(OpenFileName.lpstrFile)
	{
		strcpy(filenamebuf, OpenFileName.lpstrFile);
		return TRUE;
	}

	return FALSE;
}

static boolean GetTapeFileName(char* filenamebuf)
{
	OPENFILENAME OpenFileName;
	char szFilter[1024] = {
							  "data recorder file (*.tpr)\0*.tpr\0" \
							  ""
						  };
	char szFile[1024];
	char szFileTitle[1024];

	char buf[256];
	GetModuleFileName(NULL, buf, 256);
	int pt = strlen(buf);
	while(buf[pt] != '\\') pt--;
	buf[pt+1] = '\0';
	strcat(buf, "tape\\");
	CreateDirectory(buf, NULL);

	int ret;

	// create the default filename
	sprintf(szFile, "%s.tpr", emu->getROMname());

	memset(&OpenFileName, 0x00, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof (OPENFILENAME);
	OpenFileName.hwndOwner = main_window_handle;
	OpenFileName.hInstance = g_main_instance;
	OpenFileName.lpstrFilter = szFilter;
	OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
	OpenFileName.nMaxCustFilter = 0L;
	OpenFileName.nFilterIndex = 1L;
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = sizeof(szFile);
	OpenFileName.lpstrFileTitle = szFileTitle;
	OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
	OpenFileName.lpstrInitialDir = buf;
	OpenFileName.lpstrTitle = "data recorder file";
	OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
	OpenFileName.nFileOffset = 0;
	OpenFileName.nFileExtension = 0;
	OpenFileName.lpstrDefExt = "tpr";
	OpenFileName.lCustData = 0;

	if((ret = GetOpenFileName(&OpenFileName)) == 0)
	{
		return FALSE;
	}

	if(OpenFileName.lpstrFileTitle)
	{
		strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
		return TRUE;
	}

	return FALSE;
}

void UpdateSaveStateSlotMenu(HWND hMainWindow)
{
	HMENU hMainMenu;
	HMENU hFileMenu;
	HMENU hSlotMenu;
	if( !( hMainMenu = GetMenu( hMainWindow ) ) ||
			!( hFileMenu = GetSubMenu( hMainMenu, 0 ) ) ||
			!( hSlotMenu = GetSubMenu( hFileMenu, 17 ) ) )
	{
		return;
	}
	CheckMenuRadioItem( hSlotMenu, 0, 9, savestate_slot, MF_BYPOSITION );
}


void UpdateSaveStateFileDateMenu(HWND hMainWindow)
{
	HMENU hMainMenu;
	HMENU hFileMenu;
	HMENU hLSlotMenu;
	HMENU hSSlotMenu;
	int i;
	char *p;
	hMainMenu = GetMenu( hMainWindow );
	hFileMenu = GetSubMenu( hMainMenu, 0 );
	hLSlotMenu = GetSubMenu( hFileMenu, 16 );
	hSSlotMenu = GetSubMenu( hFileMenu, 15 );
	if(!(hLSlotMenu && hSSlotMenu))
		return;
	while(GetMenuItemCount(hLSlotMenu)>1){
		DeleteMenu(hLSlotMenu, 1, MF_BYPOSITION);
		DeleteMenu(hSSlotMenu, 1, MF_BYPOSITION);
	}
	if(!emu){
		char nt[4];
		for(i = 0; i < 10; i++){
			sprintf(nt, "&%u ", i);
			AppendMenu(hLSlotMenu, MF_STRING | MF_GRAYED, ID_FILE_QUICK_LOADN0+i, nt);
			AppendMenu(hSSlotMenu, MF_STRING | MF_GRAYED, ID_FILE_QUICK_SAVEN0+i, nt);
		}
		return;
	}
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	SYSTEMTIME st;
	FILETIME ft;
	int zs_flag=0;
	char fn[MAX_PATH], extn[5], datetxt[32];
	if(NESTER_settings.path.UseStatePath){
		strcpy(fn, NESTER_settings.path.szStatePath );
	}
	else
		strcpy(fn, emu->getROMpath());
	PathCombine(fn, fn, emu->getROMname());
	p=fn;
	while(*p)p++;
	for(i = 0; i < 10; i++){
		zs_flag=0;
		if(NESTER_settings.nes.preferences.StateFileToZip){
			sprintf(extn, ".zs%i", i);
		}
		else{
		defaultstatefn:
			zs_flag=1;
			sprintf(extn, ".ss%i", i);
		}
		//	  PathRenameExtension(fn, extn);
		strcpy(p, extn);
		hFind = FindFirstFile(fn, &fd);
		if(hFind == INVALID_HANDLE_VALUE || (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			if(NESTER_settings.nes.preferences.StateFileToZip&&zs_flag==0){
				goto defaultstatefn;
			}
			wsprintf(datetxt, "&%u -----", i);
			AppendMenu(hLSlotMenu, MF_STRING | MF_GRAYED, ID_FILE_QUICK_LOADN0+i, datetxt);
			AppendMenu(hSSlotMenu, MF_STRING, ID_FILE_QUICK_SAVEN0+i, datetxt);
		}
		else{
			FileTimeToLocalFileTime(&fd.ftLastWriteTime, &ft);
			FileTimeToSystemTime(&ft, &st);
			wsprintf(datetxt, "&%u  [%u/%u/%u/%u:%02u]", i, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
			if(zs_flag==0)
				strcat(datetxt, " Z");
			AppendMenu(hLSlotMenu, MF_STRING, ID_FILE_QUICK_LOADN0+i, datetxt);
			AppendMenu(hSSlotMenu, MF_STRING, ID_FILE_QUICK_SAVEN0+i, datetxt);
		}
		FindClose(hFind);
	}
	DrawMenuBar(hMainWindow);
}




void UpdateRecentROMMenu(HWND hMainWindow)
{
	HMENU hMainMenu;
	HMENU hFileMenu;
	HMENU hRecentMenu;

	// get main menu handle
	if( !( hMainMenu = GetMenu( hMainWindow ) ) ||
			!( hFileMenu = GetSubMenu( hMainMenu, 0 ) ) ||
			!( hRecentMenu = GetSubMenu( hFileMenu, 19 ) ) )
	{
		return;
	}

	// clear out the menu
	while(GetMenuItemCount(hRecentMenu))
	{
		DeleteMenu(hRecentMenu, 0, MF_BYPOSITION);
	}

	// if no recent files, add a dummy
	if(!NESTER_settings.recent_ROMs.get_num_entries())
	{
		AppendMenu(hRecentMenu, MF_GRAYED | MF_STRING, ID_FILE_RECENT_0, "None");
	}
	else
	{
		int i = 0;
		char text[256];

		for(i = 0; i < NESTER_settings.recent_ROMs.get_num_entries(); i++)
		{
			sprintf(text, "&%u ", i);
			// handle filenames with '&' in them
			for(uint32 k = 0; k < strlen(NESTER_settings.recent_ROMs.get_entry(i)); k++)
			{
				char temp[2] = " ";
				temp[0] = NESTER_settings.recent_ROMs.get_entry(i)[k];
				strcat(text, temp);
				if(temp[0] == '&') strcat(text, temp);
			}
			AppendMenu(hRecentMenu, MF_STRING, ID_FILE_RECENT_0+i, text);
		}
	}

	DrawMenuBar(hMainWindow);
}
///////////////////////////////////////////////////////
static int num_freezes;

void freeze(boolean really_freeze = TRUE)
{
	num_freezes++;

	if(emu && really_freeze)
	{
		if(!emu->frozen())
		{
			emu->freeze();
			set_priority(INACTIVE_PRIORITY);
		}
	}
}

void thaw()
{
	num_freezes--;
	if(num_freezes < 0)
	{
		LOG("Too many calls to thaw() (winmain.cpp)" << endl);
		num_freezes = 0;
	}

	if(emu && !num_freezes)
	{
		if(emu->frozen() && !emu->IsUserPause() )
		{
			emu->thaw();
			assert_priority();
			UNSTICK_MESSAGE_PUMP;
		}
	}
}
///////////////////////////////////////////////////////

void init()
{
	emu = NULL;
	num_freezes = 0;

	g_foreground = 1;
	g_minimized = 0;
}

void shutdown()
{
	if(emu)
	{
		delete emu;
		emu = NULL;
	}
}

int fullscreenlauncherflag;
void assertWindowSize();

void toggle_fullscreen()
{
	static RECT win_rect, launch_rect;
	static char launchf;
	static HMENU win_menu;
	static int bupg_PreviewMode=0;

	//  if(!emu) return;
	if(!gscr_mgr)
		return;

	if(emu)
		freeze();

	if(gscr_mgr->is_fullscreen())
	{
		setWindowedWindowStyle();

		//	SetMenu(main_window_handle, LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENU1)));
#if 0
		SetMenu(main_window_handle, win_menu);
		UpdateSaveStateFileDateMenu(main_window_handle);
#endif
		//	SetMenuOwnerDraw(hwnd);

		gscr_mgr->toggle_fullscreen();
//		if(Mainwindowsbh)
//			ShowWindow(Mainwindowsbh, SW_SHOW);
		if((!emu && NESTER_settings.nes.preferences.PreviewMode) || bupg_PreviewMode)
			Return_PreviewMode();
		else {
			assertWindowSize();
//			assertWindowStyle();
		}
		SetWindowPos(main_window_handle, HWND_NOTOPMOST, win_rect.left, win_rect.top,
					 (win_rect.right - win_rect.left), (win_rect.bottom - win_rect.top), SWP_SHOWWINDOW);
		if(launchf && previewflag){
			SetWindowPos(previewh, HWND_NOTOPMOST, launch_rect.left, launch_rect.top,
						 (launch_rect.right - launch_rect.left), (launch_rect.bottom - launch_rect.top), SWP_SHOWWINDOW);
		}
		Extrawin_FullscreentoWindow();

		//	if(previewflag){
		//		  SetWindowPos(previewh, HWND_NOTOPMOST, 0, 0, 0, 0,SWP_NOMOVE | SWP_NOSIZE | /*SWP_NOZORDER |*/ SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		//	}
	}
	else
	{
		fullscreenlauncherflag=previewflag;
		GetWindowRect(main_window_handle, &win_rect);
		if(previewflag){
			GetWindowRect(previewh, &launch_rect);
			launchf=1;
		}
		else
			launchf=0;

		if(gscr_mgr->toggle_fullscreen())
		{
			if(Mainwindowsbh)
				ShowWindow(Mainwindowsbh, SW_HIDE);
			bupg_PreviewMode = g_PreviewMode;
			if(g_PreviewMode){
				Goto_WindowGameMode();
				assertWindowSize();
			}

			setFullscreenWindowStyle();
#if 0
			{
				win_menu = GetMenu(main_window_handle);
				HMENU fullscreenmenu= win_menu;
				DeleteMenu(fullscreenmenu, 3, MF_BYPOSITION);
				InsertMenu(fullscreenmenu, 3, 0, IDM_FULLSCREENLUNCH, "Launcher");
				InsertMenu(fullscreenmenu, 4, 0, IDM_FULLSCREENRGAME, "Game");
				SetMenu(main_window_handle, fullscreenmenu);
				UpdateSaveStateFileDateMenu(main_window_handle);
			}
#endif
			//	  if(!NESTER_settings.nes.graphics.FullscreenModeUseNewMode)
			//		  SetMenu(main_window_handle, NULL);
			Extrawin_WindowtoFullscreen();
		}
		else
		{
			// recover gracefully
			SetWindowPos(main_window_handle, HWND_NOTOPMOST, win_rect.left, win_rect.top,
						 (win_rect.right - win_rect.left), (win_rect.bottom - win_rect.top), SWP_SHOWWINDOW);
		}
	}
	if(emu){
		gscr_mgr->assert_palette();
		thaw();
	}
	// update the palette
	SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);

	// make sure cursor is hidden in fullscreen mode
	SendMessage(main_window_handle, WM_SETCURSOR, 0, 0);
}

void assertWindowSize()
{
	RECT rct;
	int width, height;

	if(gscr_mgr->is_fullscreen() || g_PreviewMode)
		return;

	if(!(NESTER_settings.nes.graphics.UseStretchBlt || NESTER_settings.nes.graphics.WindowModeUseDdraw)){
		int zs = (int)NESTER_settings.nes.graphics.osd.zoom_size;
		if(zs>2 || zs==1.5)
			zs=2;
		height= zs * gscr_mgr->get_height();
		width=  zs * gscr_mgr->get_width();
	}
	else{
		height=(int)(NESTER_settings.nes.graphics.osd.zoom_size* gscr_mgr->get_height());
		width=(int)(NESTER_settings.nes.graphics.osd.zoom_size* gscr_mgr->get_width());
	}
	if(NESTER_settings.nes.graphics.osd.DtvSize && (NESTER_settings.nes.graphics.UseStretchBlt || NESTER_settings.nes.graphics.WindowModeUseDdraw)){
		width=(int)(height*1.4);
	}
	if(Mainwindowsbh && IsWindowVisible(Mainwindowsbh)){
		height += Mainwindowsbhs;
	}
	if(nnnKailleraFlag && NESTER_settings.nes.preferences.KailleraChatWindow && g_hKailleraLog/* && g_hKailleraSend*/){
		height += g_nKailleraChatWinH;
	}

	// set rc to client size
	//  SetRect(&rct, 0, 0, SCREEN_WIDTH_WINDOWED, SCREEN_HEIGHT_WINDOWED);
	SetRect(&rct, 0, 0, width, height);

	// adjust rc to full window size
	AdjustWindowRectEx(&rct, GetWindowStyle(main_window_handle),
					   GetMenu(main_window_handle) != NULL,
					   GetWindowExStyle(main_window_handle));

	SetWindowPos(main_window_handle, HWND_TOP, 0, 0,
				 rct.right-rct.left, rct.bottom-rct.top, SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
}

void CenterWindow()
{
	RECT rct;
	int width, height;

	if(gscr_mgr->is_fullscreen())
		return;

	if(!(NESTER_settings.nes.graphics.UseStretchBlt || NESTER_settings.nes.graphics.WindowModeUseDdraw)){
		int zs = (int)NESTER_settings.nes.graphics.osd.zoom_size;
		if(zs>2 || zs==1.5)
			zs=2;
		height= zs * gscr_mgr->get_height();
		width=  zs * gscr_mgr->get_width();
	}
	else{
		height=(int)(NESTER_settings.nes.graphics.osd.zoom_size* gscr_mgr->get_height());
		width=(int)(NESTER_settings.nes.graphics.osd.zoom_size* gscr_mgr->get_width());
	}
	if(NESTER_settings.nes.graphics.osd.DtvSize && (NESTER_settings.nes.graphics.UseStretchBlt || NESTER_settings.nes.graphics.WindowModeUseDdraw)){
		width=(int)(height*1.4);
	}
	if(Mainwindowsbh){
		height += Mainwindowsbhs;
	}

	// set rc to client size
	//  SetRect(&rct, 0, 0, SCREEN_WIDTH_WINDOWED, SCREEN_HEIGHT_WINDOWED);
	SetRect(&rct, 0, 0, width, height);

	// adjust rc to full window size
	AdjustWindowRectEx(&rct,
					   GetWindowStyle(main_window_handle),
					   GetMenu(main_window_handle) != NULL,
					   GetWindowExStyle(main_window_handle));

	SetWindowPos(main_window_handle, HWND_TOP,
				 GetSystemMetrics(SM_CXFULLSCREEN)/2 - (rct.right-rct.left)/2,
				 GetSystemMetrics(SM_CYFULLSCREEN)/2 - (rct.bottom-rct.top)/2,
				 0, 0, SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOZORDER);
}

void setNTSCMode()
{
	if(NESTER_settings.nes.graphics.show_all_scanlines)
	{
		freeze();
		NESTER_settings.nes.graphics.show_all_scanlines = FALSE;
		assertWindowSize();
		thaw();
	}
}



static FARPROC pKailleraChatSendWind_OProc = NULL;

LRESULT CALLBACK KailleraChatSendWind_p(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	switch(msg){
		case WM_KEYDOWN:
			{
				switch(wp){
				case VK_RETURN:		// Enter
					if(nnnKailleraFlag){
						char *p=NULL;
						int len;

						len = GetWindowTextLength(hWnd);
						if(len==0)
							return TRUE;
						len++;
						p = (char *)malloc(len);
						if(p==NULL)
							return TRUE;
						GetWindowText(hWnd, p, len);
						nnnkailleraChatSend(p);
						SetWindowText(hWnd, "");
					}
					return TRUE;
					break;
				}
			}
			return FALSE;
		default:
			break;
    }
    return (CallWindowProc((long (__stdcall *)(HWND ,unsigned int,unsigned int,long))pKailleraChatSendWind_OProc, hWnd, msg, wp, lp));
}


void CreateKailleraChatWindow(HWND hWnd){
//extern int  g_nKailleraChatWinH;

	hRichd32Lib = LoadLibrary("RICHED32.DLL");
	if(hRichd32Lib=NULL)
		return;

	HWND thwnd;
//	DWORD dwEvent;
	thwnd = CreateWindowEx(WS_EX_STATICEDGE, "RICHEDIT"/*"EDIT"*/, "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_HSCROLL| WS_VSCROLL | ES_MULTILINE | ES_READONLY/* | WS_CLIPSIBLINGS*/, 0, 0, 0, 0,
		hWnd, (HMENU)ID_KEDIT1, g_prg_instance, NULL);
//	dwEvent = SendMessage(thwnd, EM_GETEVENTMASK, 0, 0);
//	dwEvent |= ENM_MOUSEEVENTS;
//	SendMessage(thwnd, EM_SETEVENTMASK, 0, (LPARAM)dwEvent);
	if(thwnd == NULL)
		return;
	g_hKailleraLog = thwnd;
	{
		memset(&g_Kaillera_charfmt, 0, sizeof(CHARFORMAT));
		g_Kaillera_charfmt.cbSize = sizeof(CHARFORMAT);
		SendMessage(g_hKailleraLog, EM_GETCHARFORMAT, 0, (LPARAM)&g_Kaillera_charfmt);

		HDC hdc = GetDC(/*g_hKailleraLog*/hWnd);
		int h = GetDeviceCaps(hdc, LOGPIXELSY);
		int point = -MulDiv(/*m_logfont.lfHeight*/ Default_Font.lfHeight, 72, h);
		g_Kaillera_charfmt.yHeight = abs(point) * 20;
		ReleaseDC(/*g_hKailleraLog*/hWnd, hdc);
		g_nKailleraChatLogWinCharH = abs(point) + 2;
		g_nKailleraChatWinH = g_nKailleraChatLogWinCharH * 10 + 2*2 + 32;
//		g_Kaillera_charfmt.yHeight = Default_Font.lfHeight * 20;//g_nKailleraChatLogWinCharH * 20;
		g_Kaillera_charfmt.yOffset = 0;
//		g_Kaillera_charfmt.bPitchAndFamily = VARIABLE_PITCH;
		strcpy(g_Kaillera_charfmt.szFaceName, Default_Font.lfFaceName);
		g_Kaillera_charfmt.dwMask |= CFM_FACE | CFM_SIZE | CFM_OFFSET | CFM_COLOR;
		SendMessage(g_hKailleraLog, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&g_Kaillera_charfmt);
	}
	SendMessage(g_hKailleraLog, EM_SETBKGNDCOLOR, 0, 0xebebeb);


	thwnd = CreateWindowEx(WS_EX_STATICEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE| WS_BORDER | ES_AUTOHSCROLL, 0, 0,
		0, 0, hWnd, (HMENU)ID_KEDIT2, g_prg_instance, NULL);
	if(thwnd == NULL){
		DestroyWindow(g_hKailleraLog);
		g_hKailleraLog = NULL;
		return;
	}
	g_hKailleraSend = thwnd;
	pKailleraChatSendWind_OProc = (FARPROC)GetWindowLong(thwnd, GWL_WNDPROC);
	SetWindowLong(thwnd, GWL_WNDPROC, (LONG)KailleraChatSendWind_p);

	// LOG ウィンドウのフォントセット
/*
	{
		int r;
		CHARFORMAT cf;
		memset(&cf, 0, sizeof(CHARFORMAT));
		cf.cbSize = sizeof(CHARFORMAT);
		cf.bCharSet = SHIFTJIS_CHARSET;
		//	Settings.send_charformat.bPitchAndFamily = 0;
		cf.yHeight = 12*20;
		strcpy (cf.szFaceName, "ＭＳ ゴシック");
		cf.dwMask = CFM_BOLD | CFM_CHARSET | CFM_COLOR |
			CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE;
		r = SendMessage(g_hKailleraLog, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
		if(r==0){
			int i=0;
		}
		g_nKailleraChatLogWinCharH = 12 * 2;
		g_nKailleraChatWinH = g_nKailleraChatLogWinCharH * (1 + 5 )+ 2*2;
	}
*/
#if 0
	{
		HFONT hFont, hOldFont;
		TEXTMETRIC  tm;

		memset(&g_Kaillera_charfmt, 0, sizeof(CHARFORMAT));
		g_Kaillera_charfmt.cbSize = sizeof(CHARFORMAT);
		SendMessage(g_hKailleraLog, EM_GETCHARFORMAT, 0, (LPARAM)&g_Kaillera_charfmt);


		/*
		hFont = CreateFontIndirect(&Default_Font);
		HDC hdc = GetDC(g_hKailleraLog);
		hOldFont = (HFONT)SelectObject(hdc, hFont);
		GetTextMetrics(hdc, &tm);
		SelectObject(hdc, hOldFont);
		ReleaseDC(g_hKailleraLog, hdc);
//		m_nCharWidth = tm.tmAveCharWidth;
		g_nKailleraChatLogWinCharH = tm.tmHeight;
		DeleteObject(hFont);
*/
//		g_nKailleraChatLogWinCharH = Default_Font.lfHeight * 2;

		g_nKailleraChatLogWinCharH = Default_Font.lfHeight * 2;

//		g_Kaillera_charfmt.cbSize = sizeof(CHARFORMAT);
//		g_Kaillera_charfmt.bCharSet = SHIFTJIS_CHARSET;
//		g_Kaillera_charfmt.bPitchAndFamily = DEFAULT_PITCH;
		//	charformat.bPitchAndFamily = 0;
		g_Kaillera_charfmt.yHeight = Default_Font.lfHeight * 20;//g_nKailleraChatLogWinCharH * 20;
		strcpy(g_Kaillera_charfmt.szFaceName, Default_Font.lfFaceName);
		g_Kaillera_charfmt.dwMask |= CFM_FACE | CFM_SIZE | CFM_COLOR;
/*
		g_Kaillera_charfmt.dwMask = CFM_BOLD | CFM_CHARSET | CFM_COLOR | CFM_OFFSET | 
			CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE;
*/
		SendMessage(g_hKailleraLog, EM_SETCHARFORMAT, SCF_DEFAULT/*SCF_ALL*/, (LPARAM)&g_Kaillera_charfmt);
		g_nKailleraChatWinH = g_nKailleraChatLogWinCharH * (1 + 5 )+ 2*2;

//		SendMessage(g_hKailleraLog, EM_SETSEL, 0, -1);
//		SetFocus(g_hKailleraSend);
	}
#endif
	{
		RECT r;
		GetClientRect(hWnd, &r);
		SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, (r.bottom-r.top)<<16 | (r.right-r.left));
	}
	assertWindowSize();
	return;
}



void HideKailleraChatWindow(HWND hWnd){
	if(g_hKailleraLog)
		ShowWindow(g_hKailleraLog, SW_HIDE);
	if(g_hKailleraSend)
		ShowWindow(g_hKailleraSend, SW_HIDE);
	{
		RECT r;
		GetClientRect(hWnd, &r);
		SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, (r.bottom-r.top)<<16 | (r.right-r.left));
	}
	SetFocus(hWnd);
	assertWindowSize();
}


void ShowKailleraChatWindow(HWND hWnd){
	if(hRichd32Lib==NULL){
		CreateKailleraChatWindow(hWnd);
	}
	if(g_hKailleraLog)
		ShowWindow(g_hKailleraLog, SW_SHOW);
	if(g_hKailleraSend)
		ShowWindow(g_hKailleraSend, SW_SHOW);
	{
		RECT r;
		GetClientRect(hWnd, &r);
		SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, (r.bottom-r.top)<<16 | (r.right-r.left));
	}
	assertWindowSize();
	SetFocus(g_hKailleraLog);
//	SetFocus(g_hKailleraSend);
}


void assertKailleraChatWindowState(HWND hWnd){
	if(!NESTER_settings.nes.preferences.KailleraChatWindow || !nnnKailleraFlag || !emu){
		HideKailleraChatWindow(hWnd);
	}
	else{
		ShowKailleraChatWindow(hWnd);
	}
}

void setPALMode()
{
	if(!NESTER_settings.nes.graphics.show_all_scanlines)
	{
		freeze();
		NESTER_settings.nes.graphics.show_all_scanlines = TRUE;
		assertWindowSize();
		thaw();
	}
}


int MovieAddedDialog(){
	int rv;
	char str[32], str2[32];
	freeze();
	emu->nnnManuEnable(1);
	LoadString(g_main_instance, IDS_STRING_MSGB_01, str, 32);
	LoadString(g_main_instance, IDS_STRING_MSGB_02, str2, 32);
	rv = MessageBox(main_window_handle,(LPCSTR)str2,(LPCSTR)str,MB_YESNO | MB_ICONQUESTION);
	emu->nnnManuEnable(0);
	thaw();
	return rv;
}


int MovieDeleteMsgDialog(){
	int rv;
	char str[32], str2[32];
	freeze();
	emu->nnnManuEnable(1);
	rv = MessageBox(main_window_handle,(LPCSTR)str2,(LPCSTR)str,MB_YESNO | MB_ICONQUESTION);
	emu->nnnManuEnable(0);
	thaw();
	return rv;
}


void MakeSaveStateFilename(char* buf, int sn)
{
	char extension[5];

	if(emu)
	{
		if( NESTER_settings.path.UseStatePath)
		{
			strcpy( buf, NESTER_settings.path.szStatePath );
			PathAddBackslash( buf );
		}
		else
			strcpy( buf, emu->getROMpath() );

		if( GetFileAttributes( buf ) == 0xFFFFFFFF )
			MKCreateDirectories( buf );

		strcat(buf, emu->getROMname());
		if(NESTER_settings.nes.preferences.StateFileToZip)
			sprintf(extension, ".zs%i", sn);
		else
			sprintf(extension, ".ss%i", sn);
		strcat(buf, extension);
	}
	else
	{
		strcpy(buf, "");
	}
}

void MakeShortSaveStateFilename(char* buf)
{
	char extension[5];

	if(emu)
	{
		strcpy(buf, emu->getROMname());
		sprintf(extension, ".ss%i", savestate_slot);
		strcat(buf, extension);
	}
	else
	{
		strcpy(buf, "");
	}
}


void FreeROM_Proc();

int LoadROMProc(const char *rom_name, int n){
	FreeROM_Proc();
/*
	if(emu)
	{
		SocketClose();
		CheatfSave();			//Cheat File Save
		FreeROM_ExtraWindow();
		FreeCheatStruct();
		HideKailleraChatWindow(main_window_handle);
		if(NESTER_settings.nes.preferences.AutoStateSL){
			LunchAutoSSave();
		}
		delete emu;
		emu = NULL;
	}
*/
	try {

		switch(n){
			case EMUTYPE_NES:
				emu = new win32_emu(main_window_handle, g_main_instance, rom_name);
				break;
#ifndef _NES_ONLY
			case EMUTYPE_GB:
				emu = new win32_GB_emu(main_window_handle, g_main_instance, rom_name);
				break;
			case EMUTYPE_PCE: //  EMUTYPE_SNES
				emu = new win32_PCE_emu(main_window_handle, g_main_instance, rom_name);
				break;
#endif
			default:
//				MessageBox(main_window_handle, "error", "ERROR", MB_OK);
//				throw;
				SetMainWindowTitle();
				return 3;
		}

		// add the ROM to the recent ROM list
		NESTER_settings.recent_ROMs.add_entry(rom_name);

		// set the Open directory
		strcpy(NESTER_settings.OpenPath, emu->getROMpath());

		// assert the priority
		assert_priority();

		// kaillerachat
		ShowKailleraChatWindow(main_window_handle);

		//	gscr_mgr->set_screenmode(emu->GetScreenMode());
		if(g_PreviewMode==0){
			assertWindowStyle();
			assertWindowSize();
		}

		// update the palette
		SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);

		if(NESTER_settings.nes.graphics.fullscreen_on_load && !g_PreviewMode)
		{
			if(!gscr_mgr->is_fullscreen())
			{
				toggle_fullscreen();
			}
		}
		else{
			SetFocus(main_window_handle);
		}

		emu->reset(0); // ExSound
		if(emu){
			InitCheatStruct();
			CheatfLoad();
			if(NESTER_settings.nes.preferences.AutoStateSL && !NESTER_settings.nes.preferences.AutoStateSLNotload){
//				LunchAutoSLoad();
				if(nnnastflag){
					if(KillTimer(main_window_handle, ID_LUNCHASTIMER))
						nnnastflag=0;
				}
				if(SetTimer(main_window_handle, ID_LUNCHASTIMER, 120, NULL))
					nnnastflag=1;
			}
			UpdateSaveStateFileDateMenu(main_window_handle);
		}
		LoadROM_ExtraWindow();
		if(!NESTER_settings.nes.preferences.DisableGameInfoDisp)
		{
			char gs[260];
			gs[0] = 0;
			emu->GetROMInfoStr(gs);
			InfoDisplayt(gs, 1000);
//			RomHeaderInfoDisplay(th);
		}
		SetForegroundWindow(main_window_handle);
	} catch(const char* m) {
		if(!gscr_mgr->is_fullscreen() && NESTER_settings.nes.preferences.PreviewMode){
			Return_PreviewMode();
		}
//		if(g_PreviewMode==0)
			MessageBox(main_window_handle, m, "ERROR", MB_OK);
#ifdef NESTER_DEBUG
		LOG(m << endl);
#endif
		return 2;
	} catch(...) {
/*
		char *err = "unknown error while loading ROM";
		if(!gscr_mgr->is_fullscreen() && NESTER_settings.nes.preferences.PreviewMode){
			Return_PreviewMode();
		}
		MessageBox(main_window_handle, err, "ERROR", MB_OK);

#ifdef NESTER_DEBUG
		LOG(err << endl);
#endif
*/
		return 1;
	}
	if(NESTER_settings.nes.graphics.osd.next_avirec && emu){
		char path[MAX_PATH], fn[MAX_PATH];
		freeze();
		strcpy(fn, emu->getROMname());
		strcat(fn, ".avi");
		GetModuleFileName(NULL, path, MAX_PATH);
		PathRemoveFileSpec(path);
		if(MyAviFileSaveDialog(main_window_handle, path, fn)){
			if(emu->start_avirec(fn)){
				strcpy(fn, emu->getROMname());
				strcat(fn, ".wav");
				if(NESTER_settings.nes.graphics.osd.wav_avirec){
					if(MyAviFileSaveDialog(main_window_handle, path, fn)){
						emu->start_sndrec(fn);
					}
				}
				NESTER_settings.nes.graphics.osd.next_avirec = 0;
			}
		}
		thaw();
	}
	SetMainWindowTitle();
	return 0;
}


void LoadROM(const char* rom_name)
{
	char full_name_with_path[_MAX_PATH];
	if(!MKGetLongFileName( full_name_with_path, rom_name ) )
	{
		char str[64];
		LoadString(g_main_instance, IDS_STRING_ERRM_01 , str, 64);
		MessageBox(main_window_handle, str, "ERROR", MB_OK);
		return;
	}
	int i;
	if((i = CheckROMFile(full_name_with_path, main_window_handle, NULL)) != -1){
		LoadROMProc(full_name_with_path, i);
	}
	if(!emu && g_PreviewMode)
		InvalidateRect(main_window_handle, NULL, TRUE);

/*
	if(!emu && ret == 1){
		char str[128];
		if(!gscr_mgr->is_fullscreen() && NESTER_settings.nes.preferences.PreviewMode){
			Return_PreviewMode();
		}
		LoadString(g_main_instance, IDS_STRING_ERRM_02 , str, 128);
		MessageBox(main_window_handle, str, "ERROR", MB_OK);
		SetMainWindowTitle();
	}
*/
}


void FreeROM_Proc(){
	if(emu)
	{
/*
		if(nnnKailleraFlag){
			nnnkailleraEndGame();
			nnnKailleraFlag = 0; 
		}
*/
		SocketClose();
		CheatfSave();			//Cheat File Save
		FreeROM_ExtraWindow();
		FreeCheatStruct();
		HideKailleraChatWindow(main_window_handle);
		if(NESTER_settings.nes.preferences.AutoStateSL){
			LunchAutoSSave();
		}
		delete emu;
		emu = NULL;
		disable_flag=0;
		SetMainWindowTitle();
		assertWindowStyle();
	}
}



void FreeROM()
{
	FreeROM_Proc();
	UpdateSaveStateFileDateMenu(main_window_handle);
//	if(!g_PreviewMode)
//		assertWindowStyle();
	if(g_PreviewMode==0){
		assertWindowSize();
	}
	SetMainWindowTitle();
	if(!gscr_mgr->is_fullscreen() && NESTER_settings.nes.preferences.PreviewMode){
		Return_PreviewMode();
	}
	if(g_PreviewMode)
		InvalidateRect(main_window_handle, NULL, TRUE);
//		SendMessage(main_window_handle, WM_PAINT, 0, 0);
}

// nesterJ changing
void LoadCmdLineROM(char *rom_name)
{
	if( !*rom_name )
		return;
	PathRemoveArgs( rom_name );
	PathUnquoteSpaces( rom_name );
	if( !*rom_name )
		return;
	Goto_WindowGameMode();
	LoadROM( rom_name );
}

void ShowAboutDialog(HWND hWnd)
{
	freeze();
	DialogBox(g_main_instance, MAKEINTRESOURCE(IDD_HELP_ABOUT),
			  hWnd, AboutNester_DlgProc);
	thaw();
}

LRESULT CALLBACK WindowProc(HWND hwnd,
							UINT msg,
							WPARAM wparam,
							LPARAM lparam)
{
	// this is the main message handler of the system
	PAINTSTRUCT  ps;	  // used in WM_PAINT
	HDC		  hdc;	 // handle to a device context

//	DUMP_WM_MESSAGE(3,msg);

	static HINSTANCE hInst;
	static char nnnckeytflag=0;
	static char nnnprevieweditlabel=0;
	static unsigned char mflag=0, rpopupmflag=0;
	static unsigned int dclick_timer=0;
	static int MouseCapture = 0;
	static RECT TreeborderRect;
	static int EnterMenu = 0;
	static int AutoMenuShow_Timer_Id = 0, BossFlag = 0;

	// what is the message
	switch(msg)
	{
	case WM_COMMAND:
		// Handle all menu and accelerator commands
		if(emu)
		{
			if((emu->GetExControllerType() == EX_DOREMIKKO_KEYBOARD ||
					emu->GetExControllerType() == EX_FAMILY_KEYBOARD) && disable_flag)
			{
				return 0L;
				break;
			}
			if(nnnKailleraFlag && g_hKailleraSend == GetFocus()){
				return 0L;
			}
		}
//		if( !disable_flag )
		{
			switch (LOWORD(wparam))
			{
			case ID_FILE_EXIT:
				PostMessage(main_window_handle, WM_CLOSE, 0, 0L);
				return 0L;
				break;

			case ID_FILE_OPEN_ROM:
				freeze();
				{
					char filename[_MAX_PATH];
					if(GetROMFileName(filename, NESTER_settings.OpenPath))
					{
						SocketClose();
						Goto_WindowGameMode();
						LoadROM(filename);
					}
				}
				thaw();
				return 0;
				break;

			case ID_FILE_CLOSE_ROM:
				SocketClose();
				FreeROM();
				return 0;
				break;

			case ID_FILE_SOFTRESET:
				freeze();
				SocketClose();
				emu->reset(1);
				thaw();
				return 0;
				break;

			case ID_FILE_RESET:
				SocketClose();
				emu->reset(0);
				return 0;
				break;

			case ID_FILE_DISK_EJECT:
				emu->SetDiskSide( 0x00 );
				InfoDisplay("Disk Eject");
				return 0;
			case ID_FILE_DISK_1A:
				emu->SetDiskSide( 0x01 );
				InfoDisplay("Set Disk 1A");
				return 0;
			case ID_FILE_DISK_1B:
				emu->SetDiskSide( 0x02 );
				InfoDisplay("Set Disk 1B");
				return 0;
			case ID_FILE_DISK_2A:
				emu->SetDiskSide( 0x03 );
				InfoDisplay("Set Disk 2A");
				return 0;
			case ID_FILE_DISK_2B:
				emu->SetDiskSide( 0x04 );
				InfoDisplay("Set Disk 2B");
				return 0;

			case ID_FILE_SNDREC:
				if( emu->IsRecording() ){
					emu->end_sndrec();
					InfoDisplay("End Record");
				}
				else{
					emu->start_sndrec(NULL);
					InfoDisplay("Start Record");
				}
				return 0;

			case ID_FILE_SCREENSHOT:
				if( emu ){
					char ts[MAX_PATH+20];
					strcpy(ts, "Screen Shot [ ");
					emu->shot_screen(&ts[14]);
					strcat(ts, " ]");
					InfoDisplay(ts);
					/*				SetWindowText(hwnd, "Screen Shot");
									if(nnnckeytflag){
										if(KillTimer(hwnd, ID_CKEYTIMER))
											nnnckeytflag=0;
									}
									if(SetTimer(hwnd, ID_CKEYTIMER, 400, NULL))
										nnnckeytflag=1;
					*/			}
				return 0;
			case IDM_AUTOSTATESL:
				NESTER_settings.nes.preferences.AutoStateSL^=1;
				if(NESTER_settings.nes.preferences.AutoStateSL)
					CheckMenuItem(GetMenu(hwnd), IDM_AUTOSTATESL, MFS_CHECKED);
				else
					CheckMenuItem(GetMenu(hwnd), IDM_AUTOSTATESL, MFS_UNCHECKED);
				break;
				return 0;
				break;
			case ID_CHEAP_CHEATVW1:		//Mein memory viewer
				if(!emu)
					return 0;
				freeze();
				CreateExtraWindow_01(hwnd);
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEATVW2:		//SRAM viewer
				if(!emu)
					return 0;
				freeze();
				CreateExtraWindow_02(hwnd);
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEATVW3:		//Mapper Extra memory viewer
				if(!emu)
					return 0;
				freeze();
				CreateExtraWindow_03(hwnd);
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEATVW4:		//All memory viewer
				if(!emu)
					return 0;
				freeze();
				CreateExtraWindow_04(hwnd);
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEATVW5:		//Pettern viewer
				if(!emu)
					return 0;
				freeze();
				CreateMyPaternWindow(hwnd);
				thaw();
				return 0;
				break;
			case ID_MAINPREV_KAILLERA:		//Kaillera
				freeze();
				{
					int flag = 0;
					if(Mainwindow_Preview==NULL){
						Return_PreviewMode();
						assertWindowSize();
						assertWindowStyle();
						NESTER_settings.nes.preferences.PreviewMode=1;
						CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_PREVIEW, MFS_CHECKED);
						flag = 1;
					}
					CreateMyKailleraWindow(hwnd);
					if(flag){
						Goto_WindowGameMode();
						assertWindowSize();
						assertWindowStyle();
						NESTER_settings.nes.preferences.PreviewMode=0;
						CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_PREVIEW, MFS_UNCHECKED);
					}
				}
				thaw();
				return 0;
				break;
			case ID_MAINPREV_KAILLERACHAT:		//Kaillera Chat
				freeze();
//				CreateKailleraChatWindow(hwnd);

				NESTER_settings.nes.preferences.KailleraChatWindow ^= 1;
				CheckMenuItem(GetMenu(hwnd), ID_MAINPREV_KAILLERACHAT, (NESTER_settings.nes.preferences.KailleraChatWindow)?MFS_CHECKED:MFS_UNCHECKED);
				assertKailleraChatWindowState(hwnd);
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEAT2:		//Apply
				g_Cheat_Info.DisableCheat ^= 1;

				CheckMenuItem(GetMenu(hwnd), ID_CHEAP_CHEAT2, (g_Cheat_Info.DisableCheat)?MFS_UNCHECKED:MFS_CHECKED);
/*
				if(!g_Cheat_Info.DisableCheat){
					CheckMenuItem(GetMenu(hwnd), ID_CHEAP_CHEAT2, MFS_CHECKED);
				}
				else{
					CheckMenuItem(GetMenu(hwnd), ID_CHEAP_CHEAT2, MFS_UNCHECKED);
				}
*/
				return 0;
				break;
			case ID_CHEAP_CHEATH1:		//
				if(!emu)
					return 0;
				freeze();
				{
					unsigned char *p;
					if(g_Cheat_Info.mainmem_size){
						emu->GetMainMemp(&p);
						memcpy(g_Cheat_Info.pMainCmp, p, g_Cheat_Info.mainmem_size);
					}
					if(g_Cheat_Info.sram_size){
						emu->GetSramMemp(&p);
						memcpy(g_Cheat_Info.pSRAMCmp, p, g_Cheat_Info.sram_size);
					}
					if(g_Cheat_Info.extramem_size){
						emu->GetExtMemp(&p);
						memcpy(g_Cheat_Info.pExtraCmp, p, g_Cheat_Info.extramem_size);
					}
				}
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEATH2:
				if(!emu)
					return 0;
				freeze();
				{
					unsigned char *p;
					if(g_Cheat_Info.mainmem_size){
						emu->GetMainMemp(&p);
						memcpy(&g_Cheat_Info.pMainCmp[g_Cheat_Info.mainmem_size], p, g_Cheat_Info.mainmem_size);
					}
					if(g_Cheat_Info.sram_size){
						emu->GetSramMemp(&p);
						memcpy(&g_Cheat_Info.pSRAMCmp[g_Cheat_Info.sram_size], p, g_Cheat_Info.sram_size);
					}
					if(g_Cheat_Info.extramem_size){
						emu->GetExtMemp(&p);
						memcpy(&g_Cheat_Info.pExtraCmp[g_Cheat_Info.extramem_size], p, g_Cheat_Info.extramem_size);
					}
				}
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEATH3:
				if(!emu)
					return 0;
				freeze();
				{
					unsigned char *p;
					if(g_Cheat_Info.mainmem_size){
						emu->GetMainMemp(&p);
						memcpy(&g_Cheat_Info.pMainCmp[g_Cheat_Info.mainmem_size*2], p, g_Cheat_Info.mainmem_size);
					}
					if(g_Cheat_Info.sram_size){
						emu->GetSramMemp(&p);
						memcpy(&g_Cheat_Info.pSRAMCmp[g_Cheat_Info.sram_size*2], p, g_Cheat_Info.sram_size);
					}
					if(g_Cheat_Info.extramem_size){
						emu->GetExtMemp(&p);
						memcpy(&g_Cheat_Info.pExtraCmp[g_Cheat_Info.extramem_size*2], p, g_Cheat_Info.extramem_size);
					}
				}
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEAT4:			//search
				if(!emu)
					return 0;
				freeze();
				if(!g_extra_window.CheatSearch){
					g_extra_window.CheatSearch = CreateDialog(g_main_instance, MAKEINTRESOURCE(IDD_CTDIALOG9), NULL/*main_window_handle*/, (DLGPROC)MyCheatDlg11);
					RECT rct;
					GetWindowRect(hwnd, &rct);
					SetWindowPos(g_extra_window.CheatSearch, HWND_TOP, rct.left+64, rct.top+64, 0, 0, SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOZORDER);
					ShowWindow(g_extra_window.CheatSearch, SW_SHOW);
					UpdateWindow(g_extra_window.CheatSearch);
					if(gscr_mgr->is_fullscreen())
						SetWindowPos(g_extra_window.CheatSearch, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
				}
				else{
					if(gscr_mgr->is_fullscreen())
						SetWindowPos(g_extra_window.CheatSearch, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
					else
						SetForegroundWindow(g_extra_window.CheatSearch);
				}
				emu->nnnManuEnable(1);
//				DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG9), hwnd, (DLGPROC)MyCheatDlg11);
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEAT5:			//Compare
				if(!emu)
					return 0;
				freeze();
				if(!g_extra_window.CheatCompare){
					g_extra_window.CheatCompare = CreateDialog(g_main_instance, MAKEINTRESOURCE(IDD_CTDIALOG6), NULL/*main_window_handle*/, (DLGPROC)MyCheatDlg8);
					RECT rct;
					GetWindowRect(hwnd, &rct);
					SetWindowPos(g_extra_window.CheatCompare, HWND_TOP, rct.left+64, rct.top+64, 0, 0, SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOZORDER);
					ShowWindow(g_extra_window.CheatCompare, SW_SHOW);
					UpdateWindow(g_extra_window.CheatCompare);
					if(gscr_mgr->is_fullscreen())
						SetWindowPos(g_extra_window.CheatCompare, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
				}
				else{
					if(gscr_mgr->is_fullscreen())
						SetWindowPos(g_extra_window.CheatCompare, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
					else
						SetForegroundWindow(g_extra_window.CheatCompare);
				}
//				DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG6), hwnd, (DLGPROC)MyCheatDlg8);
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEAT6:			//Cheat List Edit
				if(!emu)
					return 0;
				freeze();
				if(!g_extra_window.CheatListEdit){
					RECT rct;
					g_extra_window.CheatListEdit = CreateDialog(g_main_instance, MAKEINTRESOURCE(IDD_CTDIALOG5), NULL/*main_window_handle*/, (DLGPROC)MyCheatDlg7);
					GetWindowRect(hwnd, &rct);
					SetWindowPos(g_extra_window.CheatListEdit, HWND_TOP, rct.left+64, rct.top+64, 0, 0, SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOZORDER);
					ShowWindow(g_extra_window.CheatListEdit, SW_SHOW);
					UpdateWindow(g_extra_window.CheatListEdit);
					if(gscr_mgr->is_fullscreen())
						SetWindowPos(g_extra_window.CheatListEdit, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
				}
				else{
					if(gscr_mgr->is_fullscreen())
						SetWindowPos(g_extra_window.CheatListEdit, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
					else
						SetForegroundWindow(g_extra_window.CheatListEdit);
				}
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEAT7:			//Launcher
				/*			MCPreview(hwnd);
							return 0;
							break;*/
				if(!previewflag){
					CheckMenuItem(GetMenu(hwnd), ID_CHEAP_CHEAT7, MFS_CHECKED);
					MCPreview(hwnd);
				}
				else{
					DestroyWindow(previewh);
					CheckMenuItem(GetMenu(hwnd), ID_CHEAP_CHEAT7, MFS_UNCHECKED);
				}
				return 0;
				break;
			case ID_CHEAP_CHEAT8:			//Code
				if(!emu)
					return 0;
				freeze();
				DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG17), hwnd, (DLGPROC)MyCheatDlg21);
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEAT9:		//
				if(!emu)
					return 0;
				freeze();
				DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG16), hwnd, (DLGPROC)MyCheatDlg20);
				thaw();
				return 0;
				break;
			case ID_GGENIEEDITDLG:			//GameGenie Edit
				if(!emu)
					return 0;
				freeze();
				if(IDOK==DialogBox(hInst, MAKEINTRESOURCE(IDD_GGENIEDIALOG), hwnd, (DLGPROC)GameGenieEditDlg))
					emu->Load_Genie();
				thaw();
				return 0;
				break;
			case IDM_FULLSCREENLUNCH:	//Fullscreen Only Launcher
				if(fullscreenlauncherflag){
					MCPreview(hwnd);
					fullscreenlauncherflag=0;
				}
				else if(!previewflag){
					CheckMenuItem(GetMenu(hwnd), ID_CHEAP_CHEAT7, MFS_CHECKED);
					MCPreview(hwnd);
				}
				else{
					SetFocus(previewh);
					InvalidateRect(previewh, NULL, TRUE);
				}
				return 0;
				break;
			case IDM_FULLSCREENRGAME:	//Fullscreen ReturnGame
				if(gscr_mgr && gscr_mgr->is_fullscreen()){
					SetForegroundWindow(previewh);
					SetFocus(hwnd);
					InvalidateRect(hwnd, NULL, TRUE);
					mouse_timer = SetTimer(main_window_handle, TIMER_ID_MOUSE_HIDE, 1000, NULL);
				}
				return 0;
				break;
			case ID_CHEAP_CHEATFONT:
				{
					CHOOSEFONT cf;
					memset(&cf, 0, sizeof(CHOOSEFONT));
					cf.lStructSize = sizeof(CHOOSEFONT);
					cf.hwndOwner = hwnd;
					cf.lpLogFont = &CheatDlgFont;
					cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
					cf.nFontType = SCREEN_FONTTYPE;
					cf.rgbColors = CheatDlgFontColor;
					if(ChooseFont(&cf)){
						CheatDlgFontColor=cf.rgbColors;
					}
				}
				break;
#if 0 // 削除
			case ID_CHEAP_CHEATKO:
				if(!emu)
					return 0;
				freeze();
				DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG17), hwnd, (DLGPROC)MyCheatDlg22);
				thaw();
				return 0;
				break;
#endif
			case ID_CHEATFPS:		//FPS
				NESTER_settings.nes.preferences.ShowFPS^=1;
				if(NESTER_settings.nes.preferences.ShowFPS)
					CheckMenuItem(GetMenu(hwnd), ID_CHEATFPS, MFS_CHECKED);
				else
					CheckMenuItem(GetMenu(hwnd), ID_CHEATFPS, MFS_UNCHECKED);
				return 0;
				break;
			case ID_LAGDISP:		//LAG
				nnnKailleraLagDisp^=1;
				if(nnnKailleraLagDisp)
					CheckMenuItem(GetMenu(hwnd), ID_LAGDISP, MFS_CHECKED);
				else
					CheckMenuItem(GetMenu(hwnd), ID_LAGDISP, MFS_UNCHECKED);
				return 0;
				break;
				/*		case IDM_LAGTEST:
							DialogBox(hInst, MAKEINTRESOURCE(IDD_TESTDIALOG1), hwnd, (DLGPROC)MyLagTestDlg);
							return 0;
							break;*/
#if 0		// 削除予定
			case ID_CHEAP_CHEATF1:
				if(!emu)
					return 0;
				freeze();
				if(IDOK==DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG10), hwnd, (DLGPROC)MyCheatDlg12)){
					CheatdfSave();
				}
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEATF2:
				if(!emu)
					return 0;
				freeze();
				CheatdfLoad();
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEATF3:
				if(!emu)
					return 0;
				freeze();
				CheatfSave2(hwnd);
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEATF4:
				if(!emu)
					return 0;
				freeze();
				CheatfLoad2(hwnd);
				thaw();
				return 0;
				break;
			case ID_CHEAP_CHEATF5:
				if(!emu)
					return 0;
				freeze();
				CheatfLoad3(hwnd);
				thaw();
				return 0;
				break;
#endif
			case ID_CHEAP_CHEATF6:
				if(!emu)
					return 0;
				freeze();
				CheatCfSave(hwnd);
				thaw();
				return 0;
				break;
				/*		case ID_CHEAP_CHEATF7:
							if(!emu)
								return 0;
							emu->SoftReset();
							return 0;
							break;
				/*		case ID_CHEAP_CHEATF8:
							if(!emu)
								return 0;
							freeze();
							CheatCfLoad2(hwnd);
							thaw();
							return 0;
							break;*/
			case ID_OPTIONS_INFOSB:
				if(gscr_mgr->is_fullscreen())
					return 0;
				if(NESTER_settings.nes.graphics.osd.InfoStatusBar){
					DestroyWindow(Mainwindowsbh);
					Mainwindowsbh=NULL;
					Mainwindowsbhs=0;
					NESTER_settings.nes.graphics.osd.InfoStatusBar=0;
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_INFOSB, MFS_UNCHECKED);
				}
				else{
					CreateMainWindowSb();
					NESTER_settings.nes.graphics.osd.InfoStatusBar=1;
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_INFOSB, MFS_CHECKED);
				}
				{
					RECT r;
					GetClientRect(hwnd, &r);
					SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, (r.bottom-r.top)<<16 | (r.right-r.left));
				}
				assertWindowSize();
				return 0;
				break;
#if 1
			case ID_OPTIONS_PREVIEW:
				if(NESTER_settings.nes.preferences.PreviewMode){
					Goto_WindowGameMode();
					assertWindowSize();
					assertWindowStyle();
					NESTER_settings.nes.preferences.PreviewMode=0;
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_PREVIEW, MFS_UNCHECKED);
				}
				else{
					Return_PreviewMode();
					assertWindowSize();
					assertWindowStyle();
					NESTER_settings.nes.preferences.PreviewMode=1;
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_PREVIEW, MFS_CHECKED);
				}
				return 0;
				break;
#endif
			case IDM_NNNEDITPLT:
				freeze();
				NESTER_settings.nes.graphics.calculate_palette=3;
				DialogBox(hInst, MAKEINTRESOURCE(IDD_NNNEDITPALETDIALOG), hwnd, (DLGPROC)nnnEditPaletDlg);
				thaw();
				return 0;
				break;
			case ID_USEEXTKEY1:
				emu->ToggleFastFPS();
				return 0;
				break;
			case ID_USEEXTKEY2:
				{
					char sz[20];
					if(savestate_slot==9)
						savestate_slot=0;
					else
						++savestate_slot;
					wsprintf(sz, "Save Slot %u", savestate_slot);
					InfoDisplay(sz);
					/*				SetWindowText(hwnd, sz);
									if(nnnckeytflag){
										if(KillTimer(hwnd, ID_CKEYTIMER))
											nnnckeytflag=0;
									}
									if(SetTimer(hwnd, ID_CKEYTIMER, 450, NULL))
										nnnckeytflag=1;
					*/			}
				return 0;
				break;
			case ID_USEEXTKEY3:
				{
					char sz[20];
					if(savestate_slot==0)
						savestate_slot=9;
					else
						--savestate_slot;
					wsprintf(sz, "Save Slot %u", savestate_slot);
					InfoDisplay(sz);
				}
				return 0;
				break;
			case ID_USEEXTKEY4:
				if(!emu)
					break;
				{
					unsigned char ds;
					char sz[]="1A";
					ds=emu->GetDiskSide();
					if(!ds)
						break;
					if(ds&1){
						++ds;
						++sz[1];
					}
					else
						--ds;
					emu->SetDiskSide(ds);
					if(ds>2)
						++sz[0];
					InfoDisplay(sz);
				}
				return 0;
				break;
			case ID_USEEXTKEY5:
				if(emu){
					if(emu->frozen()){
						emu->SetUserPause(FALSE);
						InfoDisplay("Resume");
						thaw();
					}
					else{
						emu->SetUserPause(TRUE);
						InfoDisplay("Pause");
						freeze();
					}
				}
				return 0;
				break;
			case ID_FILE_LOAD_STATE:
				freeze();
				{
					char savestate_filename[_MAX_PATH];
					MakeShortSaveStateFilename(savestate_filename);
					if(GetLoadSavestateFileName(savestate_filename, emu->getROMpath(), emu->getROMname()))
					{
						SocketClose();
						emu->loadState(savestate_filename);
					}
				}
				thaw();
				return 0;
				break;

			case ID_FILE_SAVE_STATE:
				freeze();
				{
					char savestate_filename[_MAX_PATH];
					MakeShortSaveStateFilename(savestate_filename);
					if(GetSaveSavestateFileName(savestate_filename, emu->getROMpath(), emu->getROMname()))
					{
						emu->saveState(savestate_filename);
					}
				}
				thaw();
				return 0;
				break;

			case ID_FILE_QUICK_LOAD:
				{
					char savestate_filename[_MAX_PATH];
					MakeSaveStateFilename(savestate_filename, savestate_slot);
					SocketClose();
					boolean result = emu->loadState(savestate_filename);
					{
						char s[40];
						wsprintf(s, "Quick Load From  %u",savestate_slot);
						InfoDisplay(s);
					}
				}
				return 0;
				break;

			case ID_FILE_QUICK_SAVE:
				{
					char savestate_filename[_MAX_PATH];
					MakeSaveStateFilename(savestate_filename, savestate_slot);
					boolean result = emu->saveState(savestate_filename);
					if(result){

						{
							char s[40];
							wsprintf(s, "Quick Save  %u",savestate_slot);
							InfoDisplay(s);
						}
						/*				SetWindowText(hwnd, "State Save");
										if(nnnckeytflag){
											if(KillTimer(hwnd, ID_CKEYTIMER))
												nnnckeytflag=0;
										}
										if(SetTimer(hwnd, ID_CKEYTIMER, 400, NULL))
											nnnckeytflag=1;
						*/			}
				}
				UpdateSaveStateFileDateMenu(hwnd);
				return 0;
				break;
			case ID_FILE_QUICK_SAVEN0:
			case ID_FILE_QUICK_SAVEN1:
			case ID_FILE_QUICK_SAVEN2:
			case ID_FILE_QUICK_SAVEN3:
			case ID_FILE_QUICK_SAVEN4:
			case ID_FILE_QUICK_SAVEN5:
			case ID_FILE_QUICK_SAVEN6:
			case ID_FILE_QUICK_SAVEN7:
			case ID_FILE_QUICK_SAVEN8:
			case ID_FILE_QUICK_SAVEN9:
				{
					char fn[MAX_PATH];
					int index = LOWORD(wparam) - ID_FILE_QUICK_SAVEN0;
					MakeSaveStateFilename(fn, index);
					emu->saveState(fn);
					{
						char s[40];
						wsprintf(s, "Quick Save  %u", index);
						InfoDisplay(s);
					}
				}
				UpdateSaveStateFileDateMenu(hwnd);
				return 0;
				break;

			case ID_FILE_QUICK_LOADN0:
			case ID_FILE_QUICK_LOADN1:
			case ID_FILE_QUICK_LOADN2:
			case ID_FILE_QUICK_LOADN3:
			case ID_FILE_QUICK_LOADN4:
			case ID_FILE_QUICK_LOADN5:
			case ID_FILE_QUICK_LOADN6:
			case ID_FILE_QUICK_LOADN7:
			case ID_FILE_QUICK_LOADN8:
			case ID_FILE_QUICK_LOADN9:
				{
					char fn[MAX_PATH];
					int index = LOWORD(wparam) - ID_FILE_QUICK_LOADN0;
					MakeSaveStateFilename(fn, index);
					emu->loadState(fn);
					{
						char s[40];
						wsprintf(s, "Quick Load From  %u", index);
						InfoDisplay(s);
					}
				}
				return 0;
				break;

			case ID_FILE_SLOT_0:
			case ID_FILE_SLOT_1:
			case ID_FILE_SLOT_2:
			case ID_FILE_SLOT_3:
			case ID_FILE_SLOT_4:
			case ID_FILE_SLOT_5:
			case ID_FILE_SLOT_6:
			case ID_FILE_SLOT_7:
			case ID_FILE_SLOT_8:
			case ID_FILE_SLOT_9:
				{
					int index = LOWORD(wparam) - ID_FILE_SLOT_0;
					savestate_slot = index;
					{
						char s[40];
						wsprintf(s, "Slot Select  %u",savestate_slot);
						InfoDisplay(s);
					}
				}
				return 0;
				break;

			case ID_FILE_RECENT_0:
			case ID_FILE_RECENT_1:
			case ID_FILE_RECENT_2:
			case ID_FILE_RECENT_3:
			case ID_FILE_RECENT_4:
			case ID_FILE_RECENT_5:
			case ID_FILE_RECENT_6:
			case ID_FILE_RECENT_7:
			case ID_FILE_RECENT_8:
			case ID_FILE_RECENT_9:
				{
					int index = LOWORD(wparam) - ID_FILE_RECENT_0;
					if(NESTER_settings.recent_ROMs.get_entry(index))
					{
						SocketClose();
						Goto_WindowGameMode();
						LoadROM(NESTER_settings.recent_ROMs.get_entry(index));
					}
				}
				return 0;
				break;

			case ID_FILE_NETPLAY_ACCEPT:
				freeze();
				{
					EnableMenuItem(GetSystemMenu(main_window_handle, FALSE), SC_CLOSE, MF_GRAYED);
					if(SocketServer())
					{
						emu->reset(0);
						// send 1st pad info (dummy)
						SocketSendByte(0x00);
					}
					else
					{
						EnableMenuItem(GetSystemMenu(main_window_handle, FALSE), SC_CLOSE, MF_ENABLED);
					}
				}
				thaw();
				return 0;
				break;

			case ID_FILE_NETPLAY_CONNECT:
				freeze();
				{
					EnableMenuItem(GetSystemMenu(main_window_handle, FALSE), SC_CLOSE, MF_GRAYED);
					if(SocketClient())
					{
						emu->reset(0);
						// send 1st pad info (dummy)
						SocketSendByte(0x00);
					}
					else
					{
						EnableMenuItem(GetSystemMenu(main_window_handle, FALSE), SC_CLOSE, MF_ENABLED);
					}
				}
				thaw();
				return 0;
				break;

			case ID_FILE_NETPLAY_CLOSE:
				{
					netplay_disconnect = 1;
				}
				break;


			case ID_FILE_MOVIE_STOP:
				emu->StopMovie();
				return 0;
				break;

			case ID_FILE_MOVIE_INSMSG:
				freeze();
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_MOVIEINSERTMSG), hwnd, (DLGPROC)MovieMessageDlg);
				}
				thaw();
				return 0;
				break;

			case ID_FILE_MOVIE_DELMSG:
				freeze();
				{
					emu->DeleteMovieMsg();
				}
				thaw();
				return 0;
				break;

			case ID_FILE_MOVIE_PRESST:
				freeze();
				{
					emu->MoviePreStateRec();
				}
				thaw();
				return 0;
				break;

			case ID_FILE_MOVIE_RPAUSE:
				freeze();
				{
					emu->MovieRecPause();
				}
				thaw();
				return 0;
				break;

			case ID_FILE_MOVIE_RSTART:
				freeze();
				{
					emu->StartRecMovie("", 0);
				}
				thaw();
				return 0;
				break;

			case ID_FILE_MOVIE_PLAY:
				freeze();
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_MOVIEPLAYDIALOG), hwnd, (DLGPROC)nnnMoviePlayDlg);
				}
				thaw();
				return 0;
				break;

			case ID_FILE_MOVIE_REC:
				freeze();
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_MOVIERECDIALOG), hwnd, (DLGPROC)nnnMovieRecDlg);

				}
				thaw();
				return 0;
				break;
			case ID_FILE_NPMOVIE_REC:
				freeze();
				emu->StartPlayToRecMovie();
				thaw();
				return 0;
				break;

			case ID_OPTIONS_PREFERENCES:
				freeze();
				//		DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_PREFERENCES),hwnd, PreferencesOptions_DlgProc, (LPARAM)&NESTER_settings);
				{
					PROPSHEETPAGE psp;
					PROPSHEETHEADER psh;
					HPROPSHEETPAGE hpsp[3];
					char str[32];

					psp.dwSize = sizeof(PROPSHEETPAGE);
					psp.dwFlags = PSP_DEFAULT;
					psp.hInstance = hInst;

					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_PREFERENCES);
					psp.pfnDlgProc = (DLGPROC)PreferencesOptions_DlgProc;
					hpsp[0] = CreatePropertySheetPage(&psp);
					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_PREFERENCES2);
					psp.pfnDlgProc = (DLGPROC)PreferencesOptions_DlgProc2;
					hpsp[1] = CreatePropertySheetPage(&psp);
					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_PREFERENCES3);
					psp.pfnDlgProc = (DLGPROC)PreferencesOptions_DlgProc3;
					hpsp[2] = CreatePropertySheetPage(&psp);

					memset(&psh, 0, sizeof(PROPSHEETHEADER));
					psh.dwSize = sizeof(PROPSHEETHEADER);
					psh.dwFlags = PSH_NOAPPLYNOW;
					psh.hInstance = hInst;
					psh.hwndParent = hwnd;
					psh.nPages = 3;
					psh.phpage = hpsp;
					LoadString(g_main_instance, IDS_STRING_CDLG_05 , str, 32);
					psh.pszCaption = str;
					
					(HWND)PropertySheet(&psh);
					if(emu)
						emu->SetFrameSpeed();

					SaveNesMainSettings(&NESTER_settings.nes.preferences);
				}
				thaw();
				return 0;
				break;

			case ID_OPTIONS_GRAPHICS:
				freeze();
				//		  DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_GRAPHICS),hwnd, GraphicsOptions_DlgProc, (LPARAM)&NESTER_settings);
				{
					PROPSHEETPAGE psp;
					PROPSHEETHEADER psh;
					HPROPSHEETPAGE hpsp[3];
					char str[32];

					psp.dwSize = sizeof(PROPSHEETPAGE);
					psp.dwFlags = PSP_DEFAULT;
					psp.hInstance = hInst;

					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_GRAPHICS);
					psp.pfnDlgProc = (DLGPROC)GraphicsOptions_DlgProc;
					hpsp[0] = CreatePropertySheetPage(&psp);

					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_GRAPHICS2);
					psp.pfnDlgProc = (DLGPROC)GraphicsOptions_DlgProc2;
					hpsp[1] = CreatePropertySheetPage(&psp);

					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_GRAPHICS3);
					psp.pfnDlgProc = (DLGPROC)GraphicsOptions_DlgProc3;
					hpsp[2] = CreatePropertySheetPage(&psp);

					memset(&psh, 0, sizeof(PROPSHEETHEADER));
					psh.dwSize = sizeof(PROPSHEETHEADER);
					psh.dwFlags = PSH_NOAPPLYNOW;
					psh.hInstance = hInst;
					psh.hwndParent = hwnd;
					psh.nPages = 3;
					psh.phpage = hpsp;
					LoadString(g_main_instance, IDS_STRING_CDLG_06, str, 32);
					psh.pszCaption = str;
					(HWND)PropertySheet(&psh);
					if(gscr_mgr){
						if(!gscr_mgr->is_fullscreen()){
							assertWindowSize();
							setWindowedWindowStyle();
						}
						gscr_mgr->SetScreenmgr(-1);
						if(emu)
							gscr_mgr->assert_palette();
					}

					SaveGraphics(&NESTER_settings.nes.graphics);
					SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);
					SendMessage(main_window_handle, WM_PAINT, 0, 0);
				}
				thaw();
				return 0;
				break;

			case ID_OPTIONS_SOUND:
				freeze();
#if 0
				if(DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_SOUND),
								  hwnd, SoundOptions_DlgProc, (LPARAM)&NESTER_settings))
				{
					int stereo = g_snd_mgr->IsStereo();
					delete g_snd_mgr;
					g_snd_mgr = new win32_directsound_sound_mgr(hwnd,
						NESTER_settings.nes.sound.sample_bits,
						NESTER_settings.nes.sound.sample_rate, NESTER_settings.nes.sound.buffer_len, stereo, NESTER_settings.DirectSoundVolume);
					if(emu)
					{
						emu->sound_settings_changed();
//						emu->enable_sound(NESTER_settings.nes.sound.enabled);
					}

					SaveSoundSettings(&NESTER_settings.nes.sound);
				}
#endif
				{
					PROPSHEETPAGE psp;
					PROPSHEETHEADER psh;
					HPROPSHEETPAGE hpsp[2];
					char str[32];

					psp.dwSize = sizeof(PROPSHEETPAGE);
					psp.dwFlags = PSP_DEFAULT;
					psp.hInstance = hInst;

					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_SOUND);
					psp.pfnDlgProc = (DLGPROC)SoundOptions_DlgProc;
					hpsp[0] = CreatePropertySheetPage(&psp);
					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_SOUND2);
					psp.pfnDlgProc = (DLGPROC)SoundOptions_DlgProc2;
					hpsp[1] = CreatePropertySheetPage(&psp);

					memset(&psh, 0, sizeof(PROPSHEETHEADER));
					psh.dwSize = sizeof(PROPSHEETHEADER);
					psh.dwFlags = PSH_NOAPPLYNOW;
					psh.hInstance = hInst;
					psh.hwndParent = hwnd;
					psh.nPages = 2;
					psh.phpage = hpsp;
					LoadString(g_main_instance, IDS_STRING_CDLG_16 , str, 32);
					psh.pszCaption = str;
					
					(HWND)PropertySheet(&psh);

					int stereo = g_snd_mgr->IsStereo();
					delete g_snd_mgr;
					g_snd_mgr = new win32_directsound_sound_mgr(hwnd,
						NESTER_settings.nes.sound.sample_bits,
						NESTER_settings.nes.sound.sample_rate, NESTER_settings.nes.sound.buffer_len, stereo, NESTER_settings.DirectSoundVolume);
					if(emu)
					{
						emu->sound_settings_changed();
//						emu->enable_sound(NESTER_settings.nes.sound.enabled);
					}
					SaveSoundSettings(&NESTER_settings.nes.sound);
				}
				thaw();
				return 0;
				break;

			case ID_OPTIONS_CONTROLLERS:
				freeze();
				{
					PROPSHEETPAGE psp;
					PROPSHEETHEADER psh;
					HPROPSHEETPAGE hpsp[7];
					char str[32];

					psp.dwSize = sizeof(PROPSHEETPAGE);
					psp.dwFlags = PSP_DEFAULT;
					psp.hInstance = hInst;

					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_CONTROLLERS);
					psp.pfnDlgProc = (DLGPROC)ControllersOptions_DlgProc1;
					hpsp[0] = CreatePropertySheetPage(&psp);
					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_CONTROLLERS2);
					psp.pfnDlgProc = (DLGPROC)ControllersOptions_DlgProc2;
					hpsp[1] = CreatePropertySheetPage(&psp);
					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_CONTROLLERS3);
					psp.pfnDlgProc = (DLGPROC)ControllersOptions_DlgProc3;
					hpsp[2] = CreatePropertySheetPage(&psp);
					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_CONTROLLERS4);
					psp.pfnDlgProc = (DLGPROC)ControllersOptions_DlgProc4;
					hpsp[3] = CreatePropertySheetPage(&psp);
					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_EXTCONTROLLERS1);
					psp.pfnDlgProc = (DLGPROC)ExtButtonControllersOptions_DlgProc;
					hpsp[4] = CreatePropertySheetPage(&psp);
					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_EXTCONTROLLERS2);
					psp.pfnDlgProc = (DLGPROC)ExtButtonControllersOptions_DlgProc2;
					hpsp[5] = CreatePropertySheetPage(&psp);
					psp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS_CONTROLLERS5);
					psp.pfnDlgProc = (DLGPROC)ControllersJoyTest_DlgProc;
					hpsp[6] = CreatePropertySheetPage(&psp);

					memset(&psh, 0, sizeof(PROPSHEETHEADER));
					psh.dwSize = sizeof(PROPSHEETHEADER);
					psh.dwFlags = PSH_NOAPPLYNOW;
					psh.hInstance = hInst;
					psh.hwndParent = hwnd;
					psh.nPages = 7;	//6
					psh.phpage = hpsp;
					LoadString(g_main_instance, IDS_STRING_CDLG_07, str, 32);
					psh.pszCaption = str;
					g_psActivePage=0;
					(HWND)PropertySheet(&psh);
					if(emu) emu->input_settings_changed();
					g_psActivePage=0;
//					SaveAllControllerSettings();
					SaveAllControllerSettings(&NESTER_settings.nes.input);
				}
				thaw();
				return 0;
				break;

			case ID_OPTIONS_EXCONTROLLER_00:
				emu->SetExControllerType(0);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_02:
				emu->SetExControllerType(2);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_03:
				emu->SetExControllerType(3);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_04:
				emu->SetExControllerType(4);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_05:
				emu->SetExControllerType(5);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_06:
				emu->SetExControllerType(6);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_07:
				emu->SetExControllerType(7);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_08:
				emu->SetExControllerType(8);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_09:
				emu->SetExControllerType(9);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_10:
				emu->SetExControllerType(10);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_11:
				emu->SetExControllerType(11);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_12:
				emu->SetExControllerType(12);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_13:
				emu->SetExControllerType(13);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_14:
				emu->SetExControllerType(14);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_15:
				emu->SetExControllerType(15);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_16:
				emu->SetExControllerType(16);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_17:
				emu->SetExControllerType(17);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_18:
				emu->SetExControllerType(18);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_19:
				emu->SetExControllerType(19);
				return 0;
				break;
			case ID_OPTIONS_EXCONTROLLER_20:
				emu->SetExControllerType(20);
				return 0;
				break;

			case ID_OPTIONS_BARCODE_INPUT:
				{
					freeze();
					BARCODEVALUE bv;
					int result = DialogBoxParam(
									 g_main_instance, MAKEINTRESOURCE(IDD_DATACHBARCODE),hwnd,
									 DatachBarcodeDialogProc, (LPARAM)&bv);
					if(result == IDOK)
						emu->SetBarcodeValue(bv.value_low, bv.value_high);
					thaw();
					return 0;
				}
				break;
			case ID_OPTIONS_FBASIC_INPUT_CB:
				{
					my_memread_stream *stream;
					unsigned char *mem;
					HGLOBAL hGlobal;
					LPSTR lpStr;
					int size, i;

					freeze();
					if(!IsClipboardFormatAvailable(CF_TEXT)) {
						thaw();
						break;
					}
					OpenClipboard(hwnd);
					hGlobal = (HGLOBAL)GetClipboardData(CF_TEXT);
					if (hGlobal == NULL) {
						CloseClipboard();
						thaw();
						break;
					}
					lpStr = (LPSTR)GlobalLock(hGlobal);
					size = lstrlen(lpStr);

					mem = (unsigned char *)malloc(size+1);
					if(mem == NULL){
						GlobalUnlock(hGlobal);
						CloseClipboard();
						thaw();
						break;
					}
					memcpy(mem, lpStr, size);
					GlobalUnlock(hGlobal);
					CloseClipboard();
					mem[size] = 0;
					stream = new my_memread_stream(mem, size);
					if(emu->StartFamilyBasicAutoInput(stream) == 0)
						delete stream;
					thaw();
				}
				break;
			case ID_OPTIONS_FBASIC_INPUT:
				freeze();
				{
					char fn[MAX_PATH];
					if(MyCheatStWriteFileOpenDlg(hwnd, fn)){
						HANDLE hFile;
						DWORD dwAccBytes, Filesize;
						my_memread_stream *stream;
						unsigned char *mem;
						hFile = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if(hFile != INVALID_HANDLE_VALUE){
							Filesize = GetFileSize(hFile, NULL);
							mem = (unsigned char *)malloc(Filesize+1);
							if(mem == NULL){
								CloseHandle(hFile);
								thaw();
								break;
							}
							mem[Filesize] = 0;
							if(!ReadFile(hFile, mem, Filesize, &dwAccBytes, NULL)){
								thaw();
								break;
							}
							CloseHandle(hFile);
							stream = new my_memread_stream(mem, Filesize);
							if(emu->StartFamilyBasicAutoInput(stream) == 0)
								delete stream;
						}
					}
				}
				thaw();
				break;

			case ID_OPTIONS_TAPE_STOP:
				emu->StopTape();
				return 0;
				break;

			case ID_OPTIONS_TAPE_PLAY:
				freeze();
				{
					char filename[_MAX_PATH];
					if(GetTapeFileName(filename))
					{
						emu->StartPlayTape(filename);
					}
				}
				thaw();
				return 0;
				break;

			case ID_OPTIONS_TAPE_REC:
				freeze();
				{
					char filename[_MAX_PATH];
					if(GetTapeFileName(filename))
					{
						emu->StartRecTape(filename);
					}
				}
				thaw();
				return 0;
				break;

			case ID_OPTIONS_PATHS:
				freeze();
				DialogBoxParam(
					g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_PATHS),
					hwnd, PathsDlgProc, (LPARAM)&NESTER_settings );
				thaw();
				return 0;
				break;

			case ID_OPTIONS_1xSIZE:
				NESTER_settings.nes.graphics.osd.zoom_size = 1;
				if(!NESTER_settings.nes.graphics.WindowModeUseDdraw && !NESTER_settings.nes.graphics.UseStretchBlt){
					if(!gscr_mgr->is_fullscreen()){
						assertWindowSize();
						setWindowedWindowStyle();
					}
					gscr_mgr->SetScreenmgr(0);
					if(emu)
						gscr_mgr->assert_palette();
					return 0;
				}
				if(!gscr_mgr->is_fullscreen())
					assertWindowSize();
				return 0;
				break;
			case ID_OPTIONS_DOUBLESIZE:
				NESTER_settings.nes.graphics.osd.zoom_size = 2;
				if(!NESTER_settings.nes.graphics.WindowModeUseDdraw && !NESTER_settings.nes.graphics.UseStretchBlt){
					if(!gscr_mgr->is_fullscreen()){
						assertWindowSize();
						setWindowedWindowStyle();
					}
					gscr_mgr->SetScreenmgr(0);
					if(emu)
						gscr_mgr->assert_palette();
					return 0;
				}
				if(!gscr_mgr->is_fullscreen())
					assertWindowSize();
				return 0;
				break;
			case ID_OPTIONS_15xSIZE:
				NESTER_settings.nes.graphics.osd.zoom_size = 1.5;
				if(!gscr_mgr->is_fullscreen())
					assertWindowSize();
				return 0;
				break;
			case ID_OPTIONS_25xSIZE:
				NESTER_settings.nes.graphics.osd.zoom_size = 2.5;
				if(!gscr_mgr->is_fullscreen())
					assertWindowSize();
				return 0;
				break;
			case ID_OPTIONS_30xSIZE:
				NESTER_settings.nes.graphics.osd.zoom_size = 3;
				if(!gscr_mgr->is_fullscreen())
					assertWindowSize();
				return 0;
				break;
			case ID_OPTIONS_35xSIZE:
				NESTER_settings.nes.graphics.osd.zoom_size = 3.5;
				if(!gscr_mgr->is_fullscreen())
					assertWindowSize();
				return 0;
				break;
			case ID_OPTIONS_40xSIZE:
				NESTER_settings.nes.graphics.osd.zoom_size = 4;
				if(!gscr_mgr->is_fullscreen())
					assertWindowSize();
				return 0;
				break;
			case ID_OPTIONS_DTVSIZE:
				if(NESTER_settings.nes.graphics.osd.DtvSize)
					NESTER_settings.nes.graphics.osd.DtvSize=0;
				else
					NESTER_settings.nes.graphics.osd.DtvSize=1;
				if(!gscr_mgr->is_fullscreen())
					assertWindowSize();
				return 0;
				break;

			case ID_OPTIONS_FULLSCREEN:
				//		  if(emu)
				{
					toggle_fullscreen();
				}
				return 0;
				break;
			case IDM_WINDOWTOPMOST:
				if(NESTER_settings.nes.graphics.osd.WindowTopMost){
					NESTER_settings.nes.graphics.osd.WindowTopMost = 0;
				}
				else{
					NESTER_settings.nes.graphics.osd.WindowTopMost = 1;
				}
				assertWindowStyle();
				CheckMenuItem(GetMenu(hwnd), IDM_WINDOWTOPMOST, NESTER_settings.nes.graphics.osd.WindowTopMost? MFS_CHECKED : MFS_UNCHECKED);
				return 0;
				break;
			case ID_ROMINFODISPLAY:
				if(emu){
					freeze();
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ROMINFODIALOG), hwnd, (DLGPROC)nnnROMHeaderInfoDlg);
					thaw();
				}
				return 0;
				break;
			case ID_HELP_ABOUT:
				if(emu)
				{
					if(!gscr_mgr->is_fullscreen())
					{
						ShowAboutDialog(hwnd);
					}
				}
				else
				{
					ShowAboutDialog(hwnd);
				}
				return 0;
				break;
			case ID_OPTIONS_NTSC:
				NESTER_settings.nes.preferences.TV_Mode = 1;
				if(emu){
					emu->SetScreenMode(1);
				}
				return 0;
			case ID_OPTIONS_PAL:
				NESTER_settings.nes.preferences.TV_Mode = 2;
				if(emu){
					emu->SetScreenMode(2);
				}
				return 0;
			case ID_OPTIONS_AUTO:
				NESTER_settings.nes.preferences.TV_Mode = 0;
				return 0;
/*
			case ID_KEY_DISABLE:
				if(nnnprevieweditlabel){
					SendMessage(hwnd, WM_KEYDOWN, (int)VK_BACK, 0);
					break;
				}
				{
					if(emu)
					{
						if(emu->GetExControllerType() == EX_DOREMIKKO_KEYBOARD ||
								emu->GetExControllerType() == EX_FAMILY_KEYBOARD)
						{
							disable_flag = 1;
						}
					}
				}
				return 0;
				break;
*/
			case ID_PREVIEW_MENU_CHK:
				{
					int i;
					if(-1 == (i=GetListItemROMInfoStruct(g_PreviewLClickItemn)))
						break;
					DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_PRVROMCHKDIALOG),hwnd, (DLGPROC)PreviewCheckROMStateDlg, (LPARAM)i);
				}
				return 0;
				break;
			case ID_PREVIEW_MENU_ADEL:
/*
				ListView_SetItemCount(Mainwindow_Preview, 0);
				if(g_Preview_State.pPreviewItemText){
					free(g_Preview_State.pPreviewItemText);
					g_Preview_State.pPreviewItemText = NULL;
				}
				g_Preview_State.PreviewItemTextmn=0;
				g_Preview_State.PreviewListAll=0;
*/
				return 0;
				break;
			case ID_PREVIEW_MENU_ADD:
				{
					char FileName[MAX_PATH];
					if(MyFileOpenDlg(hwnd, FileName)){
						Add_PreviewListToFile(FileName);
						SetAllListViewValidItem();
						//					SetListViewValidItem(0, 0);
						PreviewToolChangeDisplay();
					}
				}
				return 0;
				break;
			case ID_PREVIEW_MENU_FADD:
				ExeCheck_NESROM_Folder(0);
				PreviewToolChangeDisplay();
/*
				Preview_Folderadd(hwnd);
				SetAllListViewValidItem();
//				SetListViewValidItem(0, 0);
				PreviewToolChangeDisplay();
*/
				return 0;
				break;
			case ID_PREVIEW_MENU_FSADD:
				ExeCheck_NESROM_Folder(1);
				PreviewToolChangeDisplay();
/*
				{
					char dirname[MAX_PATH];
					if(!MyFolderOpenDlg(hwnd, dirname))
							return 0;
					Preview_sFolderadd(dirname);
					SetAllListViewValidItem();
//					SetListViewValidItem(0, 0);
					PreviewToolChangeDisplay();
				}
*/
				return 0;
				break;
			case ID_PREVIEW_MENU_FIX:
				{
					int i,j;
					if((i=ListView_GetNextItem(Mainwindow_Preview,-1,LVNI_ALL | LVNI_SELECTED)) != -1){
						j=GetListItemROMInfoStruct(i);
						if(j!=-1){
							if(User_FixROMDatabase(hwnd, j))
								InvalidateRect(Mainwindow_Preview, NULL, TRUE);
						}
					}
				}
				break;
			case ID_PREVIEW_MENU_CFG:
				break;
			case ID_PREVIEW_MENU_STOP:
				FreeROM();
				return 0;
				break;
			case ID_PREVIEW_MENU_EST:
				{
					int i;
					char lvfn[MAX_PATH]="";

//					i = g_PreviewLClickItemn;
//					ListView_GetItemText(Mainwindow_Preview, i, 5, lvfn, MAX_PATH);
					if(-1 == (i=GetListItemROMInfoStruct(g_PreviewLClickItemn)))
						break;
					strcpy(lvfn, g_Preview_State.DB.pListViewItem[i].FileName);


					if(lvfn[0]){
						int tflag = 0;
						if(!g_Preview_State.ScreenDispFlag){
							g_Preview_State.DB.pListViewItem[i].nPlay++;
							Goto_WindowGameMode();
						}
						else {
							if(g_Preview_State.DisableSound){
								tflag = NESTER_settings.nes.sound.enabled;
								NESTER_settings.nes.sound.enabled = 0;
							}
						}
						LoadROM(lvfn);
						g_Preview_State.EmuSpeedBackup = 0;
						g_Preview_State.DisableSoundBackup= 0;
						if(emu && g_Preview_State.EmuSpeed && g_Preview_State.ScreenDispFlag){
							emu->ToggleFastFPS();
							g_Preview_State.EmuSpeedBackup = 1;
						}
						if(tflag){
							NESTER_settings.nes.sound.enabled = tflag;
							g_Preview_State.DisableSoundBackup= 1;
						}
					}
				}
				return 0;
				break;
			case IDM_AVIREC:
				{
					if(emu){
						char path[MAX_PATH], fn[MAX_PATH];
						freeze();
						strcpy(fn, emu->getROMname());
						strcat(fn, ".avi");
						GetModuleFileName(NULL, path, MAX_PATH);
						PathRemoveFileSpec(path);
						if(MyAviFileSaveDialog(hwnd, path, fn)){
							if(emu->start_avirec(fn)){
								strcpy(fn, emu->getROMname());
								strcat(fn, ".wav");
								if(NESTER_settings.nes.graphics.osd.wav_avirec){
									if(MyAviFileSaveDialog(hwnd, path, fn)){
										emu->start_sndrec(fn);
									}
								}
								NESTER_settings.nes.graphics.osd.next_avirec = 0;
							}
						}
						thaw();
					}
				}
				return 0;
			case IDM_AVISTOP:
				if(emu){
					freeze();
					emu->end_avirec();
					thaw();
				}
				return 0;
			case IDM_NEXTAVIREC:
				NESTER_settings.nes.graphics.osd.next_avirec^=1;
				return 0;
			case IDM_WAVAAVIREC:
				NESTER_settings.nes.graphics.osd.wav_avirec^=1;
				return 0;
			case IDM_LANGUAGESELECT:
				LaungageResourceSelect(hwnd);
				return 0;
			case IDM_CHEATSTWRITE:
				CreateMyCheatStWriteWindow(hwnd);
				return 0;
			case IDM_CHEATSTWRITEEDIT:
				CreateMyCheatStWriteEditWindow(hwnd);
				return 0;
			case IDM_SHORTCUTKEY:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hwnd, (DLGPROC)MyShortCutKeyDlg);
				return 0;

			case IDM_ONYONIPUU:
				if(g_extra_window.hOnyoniDlg){
					SetForegroundWindow(g_extra_window.hOnyoniDlg);
					return 0;
				}
				g_extra_window.hOnyoniDlg = CreateDialog(g_main_instance, MAKEINTRESOURCE(IDD_DIALOG4), NULL, (DLGPROC)MyOnyoNiPuuDlg);
				ShowWindow(g_extra_window.hOnyoniDlg, SW_SHOW);
				UpdateWindow(g_extra_window.hOnyoniDlg);
				return 0;
			case IDS_STRING1:
				g_Preview_State.TreeDispFlag^=1;
				{
					RECT r;
					GetClientRect(hwnd, &r);
					SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, (r.bottom-r.top)<<16 | (r.right-r.left));
					PreviewDisplayModeChange();
					UpdateWindow(hwnd);
				}
				break;
			case IDS_STRING2:
				g_Preview_State.ScreenDispFlag^=1;
				{
					if(!g_Preview_State.ScreenDispFlag && emu){
						FreeROM();
					}
					RECT r;
					GetClientRect(hwnd, &r);
					SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, (r.bottom-r.top)<<16 | (r.right-r.left));
				}
				break;
			case IDS_STRING3:
				if(IDOK == DialogBox(hInst, MAKEINTRESOURCE(IDD_PREVIEW_CFGDIALOG), hwnd, (DLGPROC)Preview_ConfigDlg)){
					char str2[64], str3[64];
					MyChangeAllToolTipColor();
					LoadString(g_main_instance, IDS_STRING_LMSG_05 , str2, 64);
					LoadString(g_main_instance, IDS_STRING_MSGB_14 , str3, 64);
					if(IDYES == MessageBox(hwnd, str3, str2, MB_YESNO | MB_ICONQUESTION)){
						LockWindowUpdate(hwnd);
						MyTreeView_DestroyCategory();
						SetListViewItem_NES();
						MyTreeView_CreateCategory();
						LockWindowUpdate(NULL);
					}
				}
				break;
			case IDS_STRING4:
				if(IDOK==DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINPREVFSDIALOG), hwnd, (DLGPROC)PreviewROMDirectoryDlg)){
				}
				break;
			case IDS_STRING5:		// All
				if(g_Preview_State.CheckResaultDisplay!=0){
					g_Preview_State.CheckResaultDisplay = 0;
					SetToolWindowItemState();
					PreviewToolChangeDisplay();
				}
				else{
					SetToolWindowItemState();
				}
				break;
			case IDS_STRING6:		// Have
				if(g_Preview_State.CheckResaultDisplay!=1){
					g_Preview_State.CheckResaultDisplay = 1;
					SetToolWindowItemState();
					PreviewToolChangeDisplay();
				}
				else{
					SetToolWindowItemState();
				}
				break;
			case IDS_STRING7:		// Miss
				if(g_Preview_State.CheckResaultDisplay!=2){
					g_Preview_State.CheckResaultDisplay = 2;
					SetToolWindowItemState();
					PreviewToolChangeDisplay();
				}
				else{
					SetToolWindowItemState();
				}
				break;
			case IDS_STRING8:		// Rescan
				if(emu)
					freeze();
				ListCheckResultClear();
				ExeCheck_NESROM();
				PreviewToolChangeDisplay();
				if(emu)
					thaw();
				break;
			case IDS_STRING9:		// Clear
				{
					char str[64];
					LoadString(g_main_instance, IDS_STRING_MSGB_13 , str, 64);
					if(IDYES == MessageBox(main_window_handle,(LPCSTR)str,(LPCSTR)"Clear",MB_YESNO | MB_ICONQUESTION)){
						ListCheckResultClear();
						PreviewToolChangeDisplay();
					}
				}
				break;
			case IDS_STRING10:
				g_Preview_State.GameTitleDetailDisplay^=1;
				SetToolWindowItemState();
				InvalidateRect(Mainwindow_Preview, NULL, TRUE);
				break;
			case IDS_STRING11:
				SendMessage(hwnd, WM_COMMAND, ID_FILE_CLOSE_ROM, 0);
				break;
			case IDS_STRING12:
				SendMessage(hwnd, WM_COMMAND, ID_OPTIONS_PREVIEW, 0);
				break;
			case IDS_STRING13:
				{
					char fn[MAX_PATH];
					if(MyFileOpenDlg(hwnd, fn)){
						User_AddFileToROMDatabase(hwnd, fn);
					}
				}
				break;
			case IDS_STRING14:
				{
					char dn[MAX_PATH]="";
					if(MyFolderOpenDlg(hwnd, dn)){
						User_AddFileinFolderToROMDatabase(hwnd, dn, 0);
					}
				}
				break;
			case IDS_STRING15:
/*
				{
					struct NES_ROM_Data ROM_data;
					DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_USERADD_DB_DIALOG),hwnd, (DLGPROC)UserAdd_DatabaseDlg, (LPARAM)&ROM_data);
				}
				break;
*/
				{
					char dn[MAX_PATH]="";
					if(MyFolderOpenDlg(hwnd, dn)){
						User_AddFileinFolderToROMDatabase(hwnd, dn, 1);
					}
				}
				break;
			case IDS_STRING16:	// Preview search string
				{
					freeze();
					char str[260];
					strcpy(str, g_Preview_State.SearchString);
					if(IDOK == DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_DIALOG5), hwnd, (DLGPROC)PreviewSearchStringDlg, (LPARAM)str)){
						PreviewSearchItem(str, 0);
					}
					thaw();
				}
				break;
			case IDS_STRING17:	// Next Search
				freeze();
				PreviewSearchItem(NULL, 1);
				thaw();
				break;
			case IDS_STRING18:	// pre Search
				freeze();
				PreviewSearchItem(NULL, -1);
				thaw();
				break;
			case IDS_STRING19:	// Preview randam select
				PreviewRandamSelectItem();
				break;
			case IDM_APULOG:
				if(emu){
					if(emu->IsAPULog()){
						emu->StopAPULog();
					}
					else {
						char fn[MAX_PATH];
						fn[0] = 0;
						if(MyNSLFileSaveDialog(hwnd, NULL, fn)){
							char *pptext[5];
							char GameT[512];

							GameT[0] = 0;
							emu->GetGameTitleName(GameT);
							pptext[0] = (char *)emu->crc32(); // CRC32
							pptext[1] = GameT; // 
							pptext[2] = GameT; // title
							pptext[3] = NULL; // 
							emu->StartAPULog(fn, pptext);
						}
					}
				}
				break;
			case IDM_USEREWIND:
				NESTER_settings.nes.preferences.UseRewind = !NESTER_settings.nes.preferences.UseRewind;
				if(emu){
					emu->ChangeRewindStatus(NESTER_settings.nes.preferences.UseRewind);
				}
				break;
			}

		}
/*
		else
		{
			if (LOWORD(wparam) == ID_KEY_DISABLE)
			{
				disable_flag = 0;
			}
		}
*/
		break;
	case WM_PREVIEW:
		//		freeze();
		//		SetFocus(hwnd);
		SetForegroundWindow(hwnd);
		/*
				if(emu){
					FreeROM();
				}
		*/
		SocketClose();
		SetFocus(hwnd);
		Goto_WindowGameMode();
		LoadROM(previewfn);
		SetFocus(hwnd);
		//		thaw();
		return 0;
		break;
	case WM_PREVIEWS:
		if(emu){
			FreeROM();
		}
		return 0;
		break;
	case WM_PREVIEWSC:
		CheckMenuItem(GetMenu(hwnd), ID_CHEAP_CHEAT7, MFS_UNCHECKED);
		previewflag=0;
		return 0;
		break;
	case WM_PREVIEWFS:
		if(emu)
			emu->ToggleFastFPS();
		SetFocus(hwnd);
		return 0;
		break;

	case WM_INITMENUPOPUP:
		switch(LOWORD(lparam))
		{
		case 0: // file menu
			{
				UINT flag;

				flag = emu ? MF_ENABLED : MF_GRAYED;
				EnableMenuItem((HMENU)wparam, ID_FILE_CLOSE_ROM,  flag);
				EnableMenuItem((HMENU)wparam, ID_FILE_RESET,	  flag);
				EnableMenuItem((HMENU)wparam, ID_FILE_SOFTRESET,  flag);
				EnableMenuItem((HMENU)wparam, ID_FILE_SCREENSHOT, flag);
				EnableMenuItem((HMENU)wparam, ID_FILE_LOAD_STATE, flag);
				EnableMenuItem((HMENU)wparam, ID_FILE_SAVE_STATE, flag);
				EnableMenuItem((HMENU)wparam, ID_FILE_QUICK_LOAD, flag);
				EnableMenuItem((HMENU)wparam, ID_FILE_QUICK_SAVE, flag);

				EnableMenuItem((HMENU)wparam, ID_FILE_DISK_EJECT, MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_DISK_1A,	  MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_DISK_1B,	MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_DISK_2A,	MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_DISK_2B,	MF_GRAYED);

				EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_PLAY, MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_REC,  MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_STOP, MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_NPMOVIE_REC, MF_GRAYED);

				EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_RSTART, MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_NPMOVIE_REC, MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_INSMSG, MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_DELMSG, MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_RPAUSE, MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_PRESST, MF_GRAYED);

				EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_ACCEPT,  MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CONNECT, MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CANCEL,  MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CLOSE,   MF_GRAYED);


				if( emu )
				{
					switch( emu->GetDiskSideNum() )
					{
					case 4:
						EnableMenuItem((HMENU)wparam, ID_FILE_DISK_2B, MF_ENABLED);
					case 3:
						EnableMenuItem((HMENU)wparam, ID_FILE_DISK_2A, MF_ENABLED);
					case 2:
						EnableMenuItem((HMENU)wparam, ID_FILE_DISK_1B, MF_ENABLED);
					case 1:
						EnableMenuItem((HMENU)wparam, ID_FILE_DISK_1A, MF_ENABLED);
						EnableMenuItem((HMENU)wparam, ID_FILE_DISK_EJECT, MF_ENABLED);
						HMENU hmDiskSideChange = GetSubMenu( (HMENU)wparam, 5 );
						CheckMenuRadioItem( hmDiskSideChange, 0, 5, emu->GetDiskSide(), MF_BYPOSITION );
					}

					switch( emu->GetMovieStatus() )
					{
					case 0:
						EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_PLAY, MF_ENABLED);
						EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_REC,  MF_ENABLED);
						break;
					case 1:
					case 2:
						EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_STOP,	  MF_ENABLED);
/*
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_ACCEPT,  MF_GRAYED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CONNECT, MF_GRAYED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CANCEL,  MF_GRAYED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CLOSE,   MF_GRAYED);
*/
						if(emu->GetMovieStatus()==2){
							EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_RPAUSE, MF_ENABLED);
							EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_PRESST, MF_ENABLED);
						}
						else{
							EnableMenuItem((HMENU)wparam, ID_FILE_NPMOVIE_REC, MF_ENABLED);
							EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_RPAUSE, MF_ENABLED);
						}
						EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_INSMSG, MF_ENABLED);
						EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_DELMSG, MF_ENABLED);
						break;
					case 3:
						EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_RSTART, MF_ENABLED);
						break;
					}

					switch(netplay_status)
					{
					case 0:
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_ACCEPT,  MF_ENABLED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CONNECT, MF_ENABLED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CANCEL,  MF_ENABLED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CLOSE,   MF_GRAYED);
						break;
					case 1:
					case 2:
						EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_PLAY,	  MF_GRAYED);
						EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_REC,	   MF_GRAYED);
						EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_STOP,	  MF_GRAYED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NPMOVIE_REC,	  MF_GRAYED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_ACCEPT,  MF_GRAYED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CONNECT, MF_GRAYED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CANCEL,  MF_GRAYED);
						EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CLOSE,   MF_ENABLED);
						break;
					}

				}

				UpdateSaveStateSlotMenu(main_window_handle);
				UpdateRecentROMMenu(main_window_handle);

				{
					MENUITEMINFO mii;
					ZeroMemory( &mii, sizeof(MENUITEMINFO) );
					mii.cbSize = sizeof(MENUITEMINFO);
					mii.fMask = MIIM_STATE;
					mii.fState = MFS_UNCHECKED | MFS_GRAYED;
					if( emu )
					{
						if( emu->sound_enabled() )
						{
							if( emu->IsRecording() )
								mii.fState = MFS_CHECKED;
							else
								mii.fState = MFS_UNCHECKED;
						}
					}
					SetMenuItemInfo( (HMENU)wparam, ID_FILE_SNDREC, FALSE, &mii );
				}

				UpdateSaveStateSlotMenu(main_window_handle);
				UpdateRecentROMMenu(main_window_handle);
			}
			break;

		case 1: // options menu
			{
				UINT flag;

				flag = emu ? MF_ENABLED : MF_GRAYED;

				EnableMenuItem((HMENU)wparam, ID_OPTIONS_PREFERENCES, MF_ENABLED);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_GRAPHICS, MF_ENABLED);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_CONTROLLERS, MF_ENABLED);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_PATHS, MF_ENABLED);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_CONTROLLERSTPS, MF_ENABLED);

				EnableMenuItem((HMENU)wparam, IDM_NNNEDITPLT, MF_ENABLED);  //palette edit

				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_00, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_02, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_03, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_04, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_05, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_06, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_07, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_08, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_09, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_10, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_11, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_12, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_13, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_14, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_15, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_16, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_17, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_18, MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_19, flag);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_20, flag);

				EnableMenuItem((HMENU)wparam, ID_OPTIONS_BARCODE_INPUT,   MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_PLAY,	   MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_REC,		MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_STOP,	   MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_FBASIC_INPUT,		MF_GRAYED);
				EnableMenuItem((HMENU)wparam, ID_OPTIONS_FBASIC_INPUT_CB,   MF_GRAYED);


				CheckMenuItem((HMENU)wparam, ID_OPTIONS_PAL, MFS_UNCHECKED);
				CheckMenuItem((HMENU)wparam, ID_OPTIONS_NTSC, MFS_UNCHECKED);
				CheckMenuItem((HMENU)wparam, ID_OPTIONS_AUTO, MFS_UNCHECKED);

				{
					int n = NESTER_settings.nes.preferences.TV_Mode;
					CheckMenuItem((HMENU)wparam, ID_OPTIONS_AUTO+n, MFS_CHECKED);
				}

				if( emu )
				{
					HMENU hmSelectExController = GetSubMenu( (HMENU)wparam, 7);
					CheckMenuRadioItem( hmSelectExController, 0, 20, emu->GetExControllerType(), MF_BYPOSITION );

					if(emu->GetExControllerType() == EX_DATACH_BARCODE_BATTLER)
					{
						EnableMenuItem((HMENU)wparam, ID_OPTIONS_BARCODE_INPUT, MF_ENABLED);
					}
					if(emu->GetExControllerType() == EX_FAMILY_KEYBOARD)
					{
						switch( emu->GetTapeStatus() )
						{
						case 0:
							EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_PLAY, MF_ENABLED);
							EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_REC,  MF_ENABLED);
							break;
						case 1:
						case 2:
							EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_STOP, MF_ENABLED);
							break;
						}
						EnableMenuItem((HMENU)wparam, ID_OPTIONS_FBASIC_INPUT, MF_ENABLED);
						EnableMenuItem((HMENU)wparam, ID_OPTIONS_FBASIC_INPUT_CB, MF_ENABLED);

					}
/*
					if(emu->GetScreenMode() == 1){
						EnableMenuItem((HMENU)wparam, ID_OPTIONS_PAL, MF_ENABLED);
						CheckMenuItem((HMENU)wparam, ID_OPTIONS_NTSC, MFS_CHECKED);
					}
					else{
						EnableMenuItem((HMENU)wparam, ID_OPTIONS_NTSC, MF_ENABLED);
						CheckMenuItem((HMENU)wparam, ID_OPTIONS_PAL, MFS_CHECKED);
					}
*/
				}

				EnableMenuItem((HMENU)wparam, ID_OPTIONS_SOUND,( emu && emu->IsRecording() ) ? MF_GRAYED : MF_ENABLED );

				/*
							// handle double-size check mark
							{
							  MENUITEMINFO menuItemInfo;

							  memset((void*)&menuItemInfo, 0x00, sizeof(menuItemInfo));
							  menuItemInfo.cbSize = sizeof(menuItemInfo);
							  menuItemInfo.fMask = MIIM_STATE;
							  menuItemInfo.fState = NESTER_settings.nes.graphics.osd.double_size ? MFS_CHECKED : MFS_UNCHECKED;
							  menuItemInfo.hbmpChecked = NULL;
							  SetMenuItemInfo((HMENU)wparam, ID_OPTIONS_DOUBLESIZE, FALSE, &menuItemInfo);
							}
				*/

				//			CheckMenuItem(GetMenu(hwnd), IDM_WINDOWTOPMOST, NESTER_settings.nes.graphics.osd.WindowTopMost? MFS_CHECKED : MFS_UNCHECKED);
				{
					int ws_flag = (NESTER_settings.nes.graphics.UseStretchBlt || NESTER_settings.nes.graphics.WindowModeUseDdraw)?MF_ENABLED:MF_GRAYED;
					EnableMenuItem((HMENU)wparam, ID_OPTIONS_15xSIZE, ws_flag);
					//				EnableMenuItem((HMENU)wparam, ID_OPTIONS_DOUBLESIZE, MF_GRAYED);
					EnableMenuItem((HMENU)wparam, ID_OPTIONS_25xSIZE, ws_flag);
					EnableMenuItem((HMENU)wparam, ID_OPTIONS_30xSIZE, ws_flag);
					EnableMenuItem((HMENU)wparam, ID_OPTIONS_35xSIZE, ws_flag);
					EnableMenuItem((HMENU)wparam, ID_OPTIONS_40xSIZE, ws_flag);
					EnableMenuItem((HMENU)wparam, ID_OPTIONS_DTVSIZE, ws_flag);
				}
				CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_1xSIZE, MFS_UNCHECKED);
				CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_DOUBLESIZE, MFS_UNCHECKED);
				CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_15xSIZE, MFS_UNCHECKED);
				CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_25xSIZE, MFS_UNCHECKED);
				CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_30xSIZE, MFS_UNCHECKED);
				CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_35xSIZE, MFS_UNCHECKED);
				CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_40xSIZE, MFS_UNCHECKED);

				if(NESTER_settings.nes.graphics.osd.zoom_size == 1)
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_1xSIZE, MFS_CHECKED);
				else if(NESTER_settings.nes.graphics.osd.zoom_size == 1.5)
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_15xSIZE, MFS_CHECKED);
				else if(NESTER_settings.nes.graphics.osd.zoom_size == 2)
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_DOUBLESIZE, MFS_CHECKED);
				else if(NESTER_settings.nes.graphics.osd.zoom_size == 2.5)
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_25xSIZE, MFS_CHECKED);
				else if(NESTER_settings.nes.graphics.osd.zoom_size == 3)
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_30xSIZE, MFS_CHECKED);
				else if(NESTER_settings.nes.graphics.osd.zoom_size == 3.5)
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_35xSIZE, MFS_CHECKED);
				else if(NESTER_settings.nes.graphics.osd.zoom_size == 4)
					CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_40xSIZE, MFS_CHECKED);
				CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_DTVSIZE, NESTER_settings.nes.graphics.osd.DtvSize? MFS_CHECKED : MFS_UNCHECKED);
				CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_FULLSCREEN, (gscr_mgr && gscr_mgr->is_fullscreen())? MFS_CHECKED : MFS_UNCHECKED);
				//			EnableMenuItem((HMENU)wparam, ID_OPTIONS_FULLSCREEN, flag);
			}
			break;

		case 2:
			{
				int flag;
				if(emu)
					flag=MF_ENABLED;
				else
					flag=MF_GRAYED;
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATVW1,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATVW2,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATVW3,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATVW4,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATVW5,flag);
//				EnableMenuItem((HMENU)wparam, ID_MAINPREV_KAILLERA, (Mainwindow_Preview==NULL)?MF_GRAYED:MF_ENABLED);

				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEAT4,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEAT5,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEAT6,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEAT8,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATKO,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEAT9,flag);
				EnableMenuItem((HMENU)wparam, ID_GGENIEEDITDLG,flag);
#if 0
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATH1,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATH2,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATH3,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATF1,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATF2,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATF3,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATF4,flag);
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATF5,flag);
#endif
				EnableMenuItem((HMENU)wparam, ID_CHEAP_CHEATF6,flag);

				EnableMenuItem((HMENU)wparam, IDM_AVIREC,flag);
				EnableMenuItem((HMENU)wparam, IDM_AVISTOP,flag);
				CheckMenuItem(GetMenu(hwnd), IDM_NEXTAVIREC, NESTER_settings.nes.graphics.osd.next_avirec? MFS_CHECKED : MFS_UNCHECKED);
				CheckMenuItem(GetMenu(hwnd), IDM_WAVAAVIREC, NESTER_settings.nes.graphics.osd.wav_avirec? MFS_CHECKED : MFS_UNCHECKED);
				CheckMenuItem(GetMenu(hwnd), IDM_USEREWIND, NESTER_settings.nes.preferences.UseRewind? MFS_CHECKED : MFS_UNCHECKED);
			}
			break;
		case 3:
			EnableMenuItem((HMENU)wparam, ID_ROMINFODISPLAY, (emu)?MF_ENABLED:MF_GRAYED);
		}
		//	  return 0;
		break;
//	case WM_CHAR:
		return 0;
	case WM_KEYDOWN:
//		if(nnnprevieweditlabel)
//			break;
		if( emu )
		{
			if(emu->GetExControllerType() == EX_DOREMIKKO_KEYBOARD ||
					emu->GetExControllerType() == EX_FAMILY_KEYBOARD)
			{
				if(wparam==VK_BACK){
					disable_flag^= 1;
					break;
				}
			}
		}
		if( !disable_flag )
		{
			switch(wparam)
			{
			case VK_ESCAPE:
				if(emu)
				{
					char str[32], str2[32];
					// if ESC is pressed in fullscreen mode, go to windowed mode
					if(gscr_mgr->is_fullscreen())
					{
						PostMessage(main_window_handle, WM_COMMAND,ID_OPTIONS_FULLSCREEN,0);
						break;
					}
					else{
						freeze();
						LoadString(g_main_instance, IDS_STRING_MSGB_05 , str, 32);
						LoadString(g_main_instance, IDS_STRING_MSGB_06 , str2, 32);
						if(IDYES == MessageBox(main_window_handle,(LPCSTR)str2,(LPCSTR)str,MB_YESNO | MB_ICONQUESTION)){
							FreeROM();
						}
						thaw();
						break;
					}
				}
				{
					char str[32], str2[32];
					LoadString(g_main_instance, IDS_STRING_MSGB_07 , str, 32);
					LoadString(g_main_instance, IDS_STRING_MSGB_08 , str2, 32);
					if(IDYES == MessageBox(main_window_handle,(LPCSTR)str2,(LPCSTR)str,MB_YESNO | MB_ICONQUESTION)){
						DestroyWindow(main_window_handle);
					}
				}
				break;
				/*		case VK_TAB:
						  if( emu &&
							  !NESTER_settings.nes.preferences.ToggleFast &&
							  !( lparam & 0x40000000 ) &&
							  !( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
							  emu->ToggleFastFPS();
						  break;
				*/
			case VK_PAUSE:
				if( emu && !( lparam & 0x40000000 ) )
				{
					if( emu->frozen() )
					{
						emu->SetUserPause(FALSE);
						thaw();
					}
					else
					{
						emu->SetUserPause(TRUE);
						freeze();
					}
				}
				break;
			case VK_F6:
				PostMessage( hwnd, WM_QUERYNEWPALETTE, 0, 0 );
				break;
			case VK_RETURN:
				if(!cheatkeyf || !emu)
					break;
				{
					char tstr[30];
					switch(cheatkeyf){
					case 1:
						if(cheatkeyadr >= 0x10000)
							emu->WriteSroneb(cheatkeyadr, (unsigned char)cheatkeynum);
						else
							emu->WriteBoneb(cheatkeyadr, (unsigned char)cheatkeynum);
						wsprintf(tstr, "(Write)  %05x : %02x", cheatkeyadr, cheatkeynum);
						break;
					case 2:
						if(cheatkeyadr >= 0x10000){
							emu->WriteSroneb(cheatkeyadr, (unsigned char)cheatkeynum);
							if(cheatkeyadr<=0x11ffe)
								emu->WriteSroneb(cheatkeyadr+1, (unsigned char)(cheatkeynum>>8));
						}
						else{
							emu->WriteBoneb(cheatkeyadr, (unsigned char)cheatkeynum);
							if(cheatkeyadr<=0x7fe)
								emu->WriteBoneb(cheatkeyadr+1, (unsigned char)(cheatkeynum>>8));
						}
						wsprintf(tstr, "(Write)  %05x : %04x", cheatkeyadr, cheatkeynum);
						break;
					case 3:
						if(cheatkeyadr >= 0x10000){
							emu->WriteSroneb(cheatkeyadr, (unsigned char)cheatkeynum);
							if(cheatkeyadr<=0x11ffe)
								emu->WriteSroneb(cheatkeyadr+1, (unsigned char)(cheatkeynum>>8));
							if(cheatkeyadr<=0x11ffd)
								emu->WriteSroneb(cheatkeyadr+2, (unsigned char)(cheatkeynum>>16));
						}
						else{
							emu->WriteBoneb(cheatkeyadr, (unsigned char)cheatkeynum);
							if(cheatkeyadr<=0x7fe)
								emu->WriteBoneb(cheatkeyadr+1, (unsigned char)(cheatkeynum>>8));
							if(cheatkeyadr<=0x7fd)
								emu->WriteBoneb(cheatkeyadr+2, (unsigned char)(cheatkeynum>>16));
						}
						wsprintf(tstr, "(Write)  %05x : %06x", cheatkeyadr, cheatkeynum);
						break;
					case 4:
						if(cheatkeyadr >= 0x10000){
							emu->WriteSroneb(cheatkeyadr, (unsigned char)cheatkeynum);
							if(cheatkeyadr<=0x11ffe)
								emu->WriteSroneb(cheatkeyadr+1, (unsigned char)(cheatkeynum>>8));
							if(cheatkeyadr<=0x11ffd)
								emu->WriteSroneb(cheatkeyadr+2, (unsigned char)(cheatkeynum>>16));
							if(cheatkeyadr<=0x11ffc)
								emu->WriteSroneb(cheatkeyadr+3, (unsigned char)(cheatkeynum>>24));
						}
						else{
							emu->WriteBoneb(cheatkeyadr, (unsigned char)cheatkeynum);
							if(cheatkeyadr<=0x7fe)
								emu->WriteBoneb(cheatkeyadr+1, (unsigned char)(cheatkeynum>>8));
							if(cheatkeyadr<=0x7fd)
								emu->WriteBoneb(cheatkeyadr+2, (unsigned char)(cheatkeynum>>16));
							if(cheatkeyadr<=0x7fc)
								emu->WriteBoneb(cheatkeyadr+3, (unsigned char)(cheatkeynum>>24));
						}
						wsprintf(tstr, "(Write)  %05x : %08x", cheatkeyadr, cheatkeynum);
						break;
					}
					SetWindowText(hwnd, tstr);
					if(nnnckeytflag){
						if(KillTimer(hwnd, ID_CKEYTIMER))
							nnnckeytflag=0;
					}
					if(SetTimer(hwnd, ID_CKEYTIMER, CHEATKEYWTT	, NULL))
						nnnckeytflag=1;
				}
				break;
			case VK_DOWN:
				if(!cheatkeyf || !emu)
					break;
				{
					char tstr[30];
					if(GetAsyncKeyState(VK_SHIFT)){
						cheatkeynum-=0x10;
						strcpy(tstr, "(V-10h) ");
					}
					else{
						--cheatkeynum;
						strcpy(tstr, "(V -1 ) ");
					}
					ckreadmm(tstr);
					SetWindowText(hwnd, tstr);
					if(nnnckeytflag){
						if(KillTimer(hwnd, ID_CKEYTIMER))
							nnnckeytflag=0;
					}
					if(SetTimer(hwnd, ID_CKEYTIMER, CHEATKEYWTT	, NULL))
						nnnckeytflag=1;
				}
				break;
			case VK_UP:
				if(!cheatkeyf || !emu)
					break;
				{
					char tstr[30];
					if(GetAsyncKeyState(VK_SHIFT)){
						cheatkeynum+=0x10;
						strcpy(tstr, "(V+10h) ");
					}
					else{
						++cheatkeynum;
						strcpy(tstr, "(V +1 ) ");
					}
					ckreadmm(tstr);
					SetWindowText(hwnd, tstr);
					if(nnnckeytflag){
						if(KillTimer(hwnd, ID_CKEYTIMER))
							nnnckeytflag=0;
					}
					if(SetTimer(hwnd, ID_CKEYTIMER, CHEATKEYWTT	, NULL))
						nnnckeytflag=1;
				}
				break;
			case VK_RIGHT:
				if(!cheatkeyf || !emu)
					break;
				{
					char tstr[30];
					unsigned char nn;
					if(emu)
						emu->ReadBoneb(cheatkeyadr, &nn);
					if(GetAsyncKeyState(VK_SHIFT)){
						cheatkeyadr+=0x10;
						strcpy(tstr, "(A+10h) ");
					}
					else{
						++cheatkeyadr;
						strcpy(tstr, "(A +1 ) ");
					}
					if(cheatkeyadr > 0x11fff)
						cheatkeyadr &=0x7ff;
					if(cheatkeyadr < 0x10000 && cheatkeyadr > 0x7ff)
						cheatkeyadr =0x10000;
					ckreadmm(tstr);
					SetWindowText(hwnd, tstr);
					if(nnnckeytflag){
						if(KillTimer(hwnd, ID_CKEYTIMER))
							nnnckeytflag=0;
					}
					if(SetTimer(hwnd, ID_CKEYTIMER, CHEATKEYWTT	, NULL))
						nnnckeytflag=1;
				}
				break;
			case VK_LEFT:
				if(!cheatkeyf || !emu)
					break;
				{
					char tstr[30];
					unsigned char nn;
					if(emu)
						emu->ReadBoneb(cheatkeyadr, &nn);
					if(GetAsyncKeyState(VK_SHIFT)){
						cheatkeyadr-=0x10;
						strcpy(tstr, "(A-10h) ");
					}
					else{
						--cheatkeyadr;
						strcpy(tstr, "(A -1 ) ");
					}
					if(cheatkeyadr > 0x11fff)
						cheatkeyadr =0x10000;
					if(cheatkeyadr < 0x10000 && cheatkeyadr > 0x7ff)
						cheatkeyadr &=0x7ff;
					ckreadmm(tstr);
					SetWindowText(hwnd, tstr);
					if(nnnckeytflag){
						if(KillTimer(hwnd, ID_CKEYTIMER))
							nnnckeytflag=0;
					}
					if(SetTimer(hwnd, ID_CKEYTIMER, CHEATKEYWTT	, NULL))
						nnnckeytflag=1;
				}
				break;
			case 'K':
				if(!cheatkeyf || !emu)
					break;
				{
					char tstr[30];
					strcpy(tstr, "(DISP)  ");
					ckreadmm(tstr);
					SetWindowText(hwnd, tstr);
					if(nnnckeytflag){
						if(KillTimer(hwnd, ID_CKEYTIMER))
							nnnckeytflag=0;
					}
					if(SetTimer(hwnd, ID_CKEYTIMER, CHEATKEYWTT	, NULL))
						nnnckeytflag=1;
				}
				break;
			case 'L':
				if(!cheatkeyf || !emu)
					break;
				{
					char tstr[30];
					unsigned char nn;
					cheatkeynum=0;

					emu->ReadBoneb(cheatkeyadr, &nn);
					cheatkeynum=(DWORD)nn;
					if(cheatkeyf>1){
						emu->ReadBoneb(cheatkeyadr+1, &nn);
						cheatkeynum|=(DWORD)nn<<8;
						if(cheatkeyf>2){
							emu->ReadBoneb(cheatkeyadr+2, &nn);
							cheatkeynum|=(DWORD)nn<<16;
						}
						if(cheatkeyf==4){
							emu->ReadBoneb(cheatkeyadr+3, &nn);
							cheatkeynum|=(DWORD)nn<<24;
						}
					}
					strcpy(tstr, "(Read)  ");
					ckreadmm(tstr);
					SetWindowText(hwnd, tstr);
					if(nnnckeytflag){
						if(KillTimer(hwnd, ID_CKEYTIMER))
							nnnckeytflag=0;
					}
					if(SetTimer(hwnd, ID_CKEYTIMER, CHEATKEYWTT, NULL))
						nnnckeytflag=1;
				}
				break;
			}
		}
		return 0;
		break;
/*
	case WM_KEYUP:
		if(emu)
		{
			if(emu->GetExControllerType() != EX_DOREMIKKO_KEYBOARD &&
					emu->GetExControllerType() != EX_FAMILY_KEYBOARD)
			{
				disable_flag = 0;
			}
		}
		/*
			  if( !disable_flag )
		  	  {
				switch(wparam)
				{
				case VK_TAB:
					if( emu )
						emu->ToggleFastFPS();
					break;
				}
		  	  }
		break;
*/
	case WM_DROPFILES:
		{
			char filename[_MAX_PATH] = "";

			freeze();

			// file has been dropped onto window
			int uFileNo = DragQueryFile((HDROP)wparam, 0xFFFFFFFF, NULL, 0);

			// nesterJ changed for long file name
			SetForegroundWindow(main_window_handle);
			if(g_PreviewMode){
				for(int i=0;i<uFileNo;++i){
					DragQueryFile((HDROP)wparam,i,filename, sizeof(filename));
					if(!User_AddFileToROMDatabase(hwnd, filename)){
						Add_PreviewListToFile(filename);
						SetAllListViewValidItem();
						PreviewToolChangeDisplay();
					}
				}
			}
			else{
				DragQueryFile((HDROP)wparam,0,filename, sizeof(filename));
				// if emulator active, try and load a savestate
				if(emu)
				{
					if(!emu->loadState(filename))
					{
						// not a savestate, try loading as a ROM
						SocketClose();
						Goto_WindowGameMode();
						LoadROM(filename);
					}
				}
				else
				{
					// try loading as a ROM
					SocketClose();
					Goto_WindowGameMode();
					LoadROM(filename);
				}
			}
			DragFinish((HDROP)wparam);
			thaw();
		}
		return 0;
		break;

	case WM_CREATE:
		// do initialization stuff here
		init();
		hInst = g_main_instance;
		gscr_mgr = new win32_screen_mgr(hwnd);
		ginp_mgr = new win32_directinput_input_mgr(hwnd, hInst);
		g_snd_mgr = new win32_directsound_sound_mgr(hwnd, NESTER_settings.nes.sound.sample_bits,
			NESTER_settings.nes.sound.sample_rate, NESTER_settings.nes.sound.buffer_len, 0, NESTER_settings.DirectSoundVolume);
		DragAcceptFiles(hwnd, TRUE);
		{
			INITCOMMONCONTROLSEX ic;
			ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
			ic.dwICC = ICC_LISTVIEW_CLASSES;
			InitCommonControlsEx(&ic);
		}
		if(!NESTER_settings.nes.preferences.DisableMenuIcon)
			SetMenuOwnerDraw(hwnd);
		CheckMenuItem(GetMenu(hwnd), ID_CHEAP_CHEAT2, (g_Cheat_Info.DisableCheat)?MFS_UNCHECKED:MFS_CHECKED);
		CheckMenuItem(GetMenu(hwnd), IDM_WINDOWTOPMOST, NESTER_settings.nes.graphics.osd.WindowTopMost? MFS_CHECKED : MFS_UNCHECKED);

		if(NESTER_settings.nes.graphics.osd.InfoStatusBar){
			RECT rc;
			Mainwindowsbh = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | SBARS_SIZEGRIP | WS_VISIBLE,
										   0, 0, 0, 0, hwnd, (HMENU)ID_STATUSMW, hInst, NULL);
			if(Mainwindowsbh){
				SendMessage(Mainwindowsbh , SB_SIMPLE, TRUE, 0L);
				GetWindowRect(Mainwindowsbh, &rc);
				Mainwindowsbhs = rc.bottom - rc.top;
			}
			CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_INFOSB, MFS_CHECKED);
		}
		NNNcfgload();
		defaultPaletteFileRead();

		if(previewflag){
			CheckMenuItem(GetMenu(hwnd), ID_CHEAP_CHEAT7, MFS_CHECKED);
			MCPreview(hwnd);
		}
		if(NESTER_settings.nes.preferences.AutoStateSL)
			CheckMenuItem(GetMenu(hwnd), IDM_AUTOSTATESL, MFS_CHECKED);
		if(NESTER_settings.nes.preferences.ShowFPS)
			CheckMenuItem(GetMenu(hwnd), ID_CHEATFPS, MFS_CHECKED);
		if(NESTER_settings.nes.preferences.KailleraChatWindow)
			CheckMenuItem(GetMenu(hwnd), ID_MAINPREV_KAILLERACHAT, MFS_CHECKED);
		MyRegistShortCutKey(hwnd);
		return(0);
		break;

	case WM_DESTROY:
		MyUnRegistShortCutKey(hwnd);
		Extrawin_ExitApp();
		{
			WINDOWPLACEMENT wp;
			wp.length = sizeof( WINDOWPLACEMENT );
			GetWindowPlacement(hwnd, &wp);
			NESTER_settings.nes.graphics.osd.WindowPos_x = wp.rcNormalPosition.left;
			NESTER_settings.nes.graphics.osd.WindowPos_y = wp.rcNormalPosition.top;
		}
		if(Mainwindow_Preview)
			Destroy_MainPreviewWind();
		NNNcfgsave();
		if(previewflag){
			DestroyWindow(previewh);
		}
		if(emu){
			CheatfSave();			//Cheat File Save
			if(NESTER_settings.nes.preferences.AutoStateSL){
				LunchAutoSSave();
			}
		}
		shutdown();
		if(gscr_mgr)
			delete gscr_mgr;
		if(ginp_mgr)
			delete ginp_mgr;
		if(g_snd_mgr)
			delete g_snd_mgr;

		DragAcceptFiles(hwnd, FALSE);
		// kill the application
		PostQuitMessage(0);
		return(0);
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return(0);
		break;
	case WM_ACTIVATE:
		if(LOWORD(wparam)==WA_INACTIVE){
//			InfoDisplay("KillFocus");
			if(g_PreviewMode)
				SetFocus(Mainwindow_Preview);
			if(g_foreground)
			{
				g_foreground = 0;
				if(NESTER_settings.nes.preferences.run_in_background || nnnKailleraFlag){
					freeze(FALSE);
				}
				else{
					freeze();
				}
				if(emu){
					if(!gscr_mgr->is_fullscreen()){
						if(NESTER_settings.nes.graphics.WindowModeUseDdraw && NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay){
							gscr_mgr->nnnManuEnable(1);
						}
					}
				}
			}
		}
		else {
			if(!g_foreground)
			{
				g_foreground = 1;
				if(NESTER_settings.nes.preferences.run_in_background || nnnKailleraFlag){
					thaw();
				}
				else{
					thaw();
				}
				if(emu){
					if(!gscr_mgr->is_fullscreen()){
						if(NESTER_settings.nes.graphics.WindowModeUseDdraw && NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay){
							gscr_mgr->nnnManuEnable(0);
						}
					}
				}
			}
		}
		return 0;
		break;

		// user is using menu
	case WM_ENTERMENULOOP:
		EnterMenu = 1;
		freeze();
		if(emu){
			if(!gscr_mgr->is_fullscreen()){
				if(NESTER_settings.nes.graphics.WindowModeUseDdraw && NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay){
					emu->nnnManuEnable(1);
				}
			}
		}
		DefWindowProc(hwnd, msg, wparam, lparam);
		return 0;
		break;
	case WM_EXITMENULOOP:
		EnterMenu = 0;
		thaw();
		if(emu){
			if(!gscr_mgr->is_fullscreen()){
				if(NESTER_settings.nes.graphics.WindowModeUseDdraw && NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay){
					emu->nnnManuEnable(0);
				}
			}
		}
		DefWindowProc(hwnd, msg, wparam, lparam);
		return 0;
		break;

		// user is moving window
	case WM_ENTERSIZEMOVE:
		freeze();
		DefWindowProc(hwnd, msg, wparam, lparam);
		return 0;
		break;
	case WM_EXITSIZEMOVE:
		thaw();
		if(ncbuttondown_flag)
		{
			ncbuttondown_flag = 0;
			thaw();
		}
		DefWindowProc(hwnd, msg, wparam, lparam);
		return 0;
		break;

		// user has clicked the title bar
	case WM_NCLBUTTONDOWN:
		{
			if(wparam == HTCAPTION)
			{
				ncbuttondown_flag = 1;
				freeze();
				DefWindowProc(hwnd, msg, wparam, lparam);
				return 0;
			}
		}
		break;
		// this only comes if WM_NCLBUTTONDOWN and SC_MOVE both return 0
		// not even... I'll leave it in anyway, with a guard around thaw()
	case WM_NCLBUTTONUP:
		{
			if(wparam == HTCAPTION)
			{
				if(ncbuttondown_flag)
				{
					ncbuttondown_flag = 0;
					thaw();
				}
				DefWindowProc(hwnd, msg, wparam, lparam);
				return 0;
			}
		}
		break;
		// this is sent after WM_NCLBUTTONDOWN, when the button is released, or a move cycle starts
	case WM_CAPTURECHANGED:
		if(ncbuttondown_flag)
		{
			ncbuttondown_flag = 0;
			thaw();
		}
		DefWindowProc(hwnd, msg, wparam, lparam);
		return 0;
		break;

	case WM_MOVE:
		if(emu)
		{
			if(!gscr_mgr->is_fullscreen())
			{
				emu->blt();
			}
		}
		break;
/*
	case WM_MOVING:
		{
			LPRECT r;
			RECT dRect;
			r = (LPRECT)lparam;
			GetWindowRect(GetDesktopWindow(), &dRect);
			if(r->left <= 10){
				r->right -=(r->left);
				r->left = 0;
				return TRUE;
			}
		}
		break;
*/
	case WM_ERASEBKGND:
		if(/*emu ||*/ g_PreviewMode)
			return TRUE;
		if(emu && gscr_mgr->is_fullscreen())
			return TRUE;
		return(DefWindowProc(hwnd, msg, wparam, lparam));
		break;
	case WM_PAINT:
		// start painting
		hdc = BeginPaint(hwnd, &ps);

/*
		else if(!emu && gscr_mgr->is_fullscreen()){
			HBRUSH hBrush, hOldBrush;
			hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
			hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
			Rectangle(hdc, 0, 0, NESTER_settings.nes.graphics.osd.fullscreen_width-1, NESTER_settings.nes.graphics.osd.fullscreen_height-1);//fullscreen_width*3/4);
			SelectObject(hdc, hOldBrush);
		}
*/
		if(g_PreviewMode){
			if(/*!emu &&*/ g_Preview_State.TreeDispFlag){//&& g_Preview_State.ScreenDispFlag/*g_Preview_State.PreviewScreenRect.right >= 264*/){
				HBRUSH hBrush, hOldBrush;
				hBrush = CreateSolidBrush(GetSysColor(COLOR_MENU));  //
				hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
				Rectangle(hdc, g_Preview_State.nTree_Width-1, 0,
					g_Preview_State.nTree_Width+6, g_Preview_State.PreviewScreenRect.bottom);
				SelectObject(hdc, hOldBrush);
				DeleteObject(hBrush);
			}
			if(g_Preview_State.ScreenDispFlag){
				HBRUSH hBrush, hOldBrush;
				hBrush = GetStockBrush(BLACK_BRUSH);  //
				hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
				Rectangle(hdc, g_Preview_State.PreviewScreenRect.left, g_Preview_State.PreviewScreenRect.top,
					g_Preview_State.PreviewScreenRect.right, g_Preview_State.PreviewScreenRect.bottom);
				SelectObject(hdc, hOldBrush);
//				DeleteObject(hBrush);

				// screen image
				if(!emu && g_Preview_State.hBitmap != NULL){
					HDC memDC;
					HBITMAP oldBitmap;

					memDC = CreateCompatibleDC(NULL);
					oldBitmap = (HBITMAP)SelectObject(memDC, g_Preview_State.hBitmap); 
					BitBlt(hdc, g_Preview_State.PreviewScreenDrawRect.left, g_Preview_State.PreviewScreenDrawRect.top, 256, 224, memDC, 0, 0, SRCCOPY);
					SelectObject(memDC, oldBitmap);
					DeleteDC(memDC);
				}
			}
		}
/*
		else if(!g_PreviewMode && !NESTER_settings.nes.preferences.PreviewMode){
			RECT rect;
			GetClientRect(hwnd, &rect);
			HBRUSH hBrush, hOldBrush;
			hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
			hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
			Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
			SelectObject(hdc, hOldBrush);
		}
*/
		// end painting
		EndPaint(hwnd, &ps);

		if(emu)
		{
			if(!gscr_mgr->is_fullscreen())
				emu->blt();
		}
		return(0);
		break;

	case WM_QUERYNEWPALETTE:
		if(emu)
		{
			if(!gscr_mgr->is_fullscreen())
			{
				try {
					emu->assert_palette();
				} catch(...) {
					return FALSE;
				}
				return TRUE;
			}
		}
		else
		{
			return FALSE;
		}
		break;

	case WM_MOUSEMOVE:
		{
			static LPARAM last_pos;

			if(lparam != last_pos)
			{
				last_pos = lparam;
				hide_mouse = 0;
				mouse_timer = SetTimer(main_window_handle, TIMER_ID_MOUSE_HIDE, 1000*MOUSE_HIDE_DELAY_SECONDS, NULL);
			}
			if(MouseCapture){
				RECT r;
				GetClientRect(hwnd, &r);
				HDC hdc = GetDC(hwnd);
				DrawFocusRect(hdc, &TreeborderRect);
//				ReleaseDC(hwnd, &hdc);
				g_Preview_State.nTree_Width = LOWORD(lparam);
				if(g_Preview_State.nTree_Width < 16 || g_Preview_State.nTree_Width > 0x8000){
					g_Preview_State.nTree_Width = 16;
				}
//				HDC hdc = GetDC(hwnd);
				SetRect(&TreeborderRect, g_Preview_State.nTree_Width-1, g_Preview_State.nToolTipHeight, g_Preview_State.nTree_Width+2, r.bottom);
				DrawFocusRect(hdc, &TreeborderRect);
				ReleaseDC(hwnd, hdc);
//				SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, (r.bottom-r.top)<<16 | (r.right-r.left));
				break;
			}
			if(g_PreviewMode){
				if(/*&& g_Preview_State.TreeDispFlag*/ g_Preview_State.nTree_Width-1 <LOWORD(lparam) && g_Preview_State.nTree_Width+4 >LOWORD(lparam)){
					SetCursor(LoadCursor(NULL,IDC_SIZEWE));
				}
				else{
					SetCursor(LoadCursor(NULL,IDC_ARROW));
				}
			}
#if 1
			if(emu && HIWORD(lparam) < 32){
				if(!g_PreviewMode && !gscr_mgr->is_fullscreen() && GetMenu(hwnd)==NULL){
					SetMenu(main_window_handle, LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENU1)));
					if(!NESTER_settings.nes.preferences.DisableMenuIcon)
						SetMenuOwnerDraw(main_window_handle);
					AutoMenuShow_Timer_Id = SetTimer(main_window_handle, TIMER_ID_AUTO_MENU_SHOW, 1000, NULL);
				}
			}
#endif
		}
		break;

	case WM_NCMOUSEMOVE:
		hide_mouse = 0;
		if(mouse_timer)
		{
			KillTimer(main_window_handle, mouse_timer);
			mouse_timer = 0;
		}
		break;

	case WM_TIMER:
		switch(wparam)
		{
		case TIMER_ID_MOUSE_HIDE:
			if(mouse_timer)
			{
				KillTimer(main_window_handle, mouse_timer);
				mouse_timer = 0;
			}
			hide_mouse = 1;
			if(emu && !rpopupmflag && !g_PreviewMode)
			{
				if(!USE_MOUSE_CURSOR)
				{
					SetCursor(NULL); // hide the mouse cursor
				}
			}
			return 0;
			break;
		case TIMER_ID_UNSTICK_MESSAGE_PUMP:
			if(unstick_timer)
			{
				KillTimer(main_window_handle, unstick_timer);
				unstick_timer = 0;
			}
			return 0;
			break;
		case TIMER_ID_AUTO_MENU_SHOW:
//			if(AutoMenuShow_Timer_Id)
			if(!EnterMenu)
			{
				if(emu)
					SetMenu(main_window_handle, NULL);
				KillTimer(main_window_handle, AutoMenuShow_Timer_Id);
				AutoMenuShow_Timer_Id = 0;
			}
			return 0;
			break;
#if 0
		case TIMER_ID_AUTOSAVE:
			{
			}
			return 0;
			break;
#endif
		case ID_LUNCHASTIMER:
			KillTimer(hwnd, ID_LUNCHASTIMER);
			nnnastflag=0;
			SetFocus(hwnd);
			LunchAutoSLoad();
			return 0;
			break;
		case ID_CKEYTIMER:
			if(KillTimer(hwnd, ID_CKEYTIMER))
				nnnckeytflag=0;
			SetWindowText(hwnd, PROG_NAME);
			return 0;
			break;
		case ID_SCRMSGTIMER:
			if(KillTimer(hwnd, ID_SCRMSGTIMER)){
				if(gscr_mgr->is_fullscreen()){
					g_ScreenMsgStr[0]=0;
				}
				else{
					if(Mainwindowsbh)
						SendMessage(Mainwindowsbh, SB_SETTEXT, 255, (LPARAM)(LPSTR)"");
				}
			}
			return 0;
			break;
		case ID_DCLKTIMER:
			KillTimer(hwnd, ID_DCLKTIMER);
			dclick_timer=0;
			SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE|0x02, 0L);
			return 0;
		}
		break;
	case WM_SETCURSOR:
		if(g_PreviewMode)
			break;
		if(emu)
		{
			if(gscr_mgr->is_fullscreen())
			{
				if(HTMENU == LOWORD(lparam) || HTCAPTION == LOWORD(lparam)) {
					if(!mflag){
						SetCursor(LoadCursor(NULL, IDC_ARROW));
						DrawMenuBar(hwnd);
						emu->nnnManuEnable(1);
						mflag=1;
					}
					return TRUE;
				}
				else
				{
					if(mflag){
						emu->nnnManuEnable(0);
						mflag=0;
					}
					if(!USE_MOUSE_CURSOR)
						SetCursor(NULL); // hide the mouse cursor
					else{
						SetCursor(LoadCursor(hInst, MAKEINTRESOURCE(IDC_CURSOR1)));
					}
					return TRUE;
				}
			}
			else
			{
				if(hide_mouse && !USE_MOUSE_CURSOR)
				{
					SetCursor(NULL); // hide the mouse cursor
					return TRUE;
				}
				if(USE_MOUSE_CURSOR){
					SetCursor(LoadCursor(hInst, MAKEINTRESOURCE(IDC_CURSOR1)));
					return TRUE;
				}
			}
		}
		break;
	case WM_SYSCOMMAND:
		switch(LOWORD(wparam & 0xfff0)) // & to catch double-click on title bar maximize
		{
		case SC_CLOSE:
			if(!NESTER_settings.nes.preferences.CloseButtonExit && emu)
			{
				if(gscr_mgr->is_fullscreen()){
					return 0;
				}
				else{
					if(!nnnKailleraFlag){
						FreeROM();
						return 0;
					}
				}
			}
			if(nnnKailleraFlag){
				nnnKailleraFlag=0;
//				FreeROM();
				return 0;
			}
			if(nnnKailleraDlgFlag){
				return 0;
			}
			break;
		case SC_MAXIMIZE:
			//		  if(emu)
			{
				// if window is minimized, restore it first
				SendMessage(main_window_handle, WM_SYSCOMMAND, SC_RESTORE, 0);

				if(!gscr_mgr->is_fullscreen())
					toggle_fullscreen();
			}
			return 0;
			break;
		case SC_MINIMIZE:
			freeze();
			g_minimized = 1;
			// make the minimize happen
			DefWindowProc(hwnd, msg, wparam, lparam);
			return 0;
			break;

		case SC_RESTORE:
			if(g_minimized) thaw();
			g_minimized = 0;
			// make the restore happen
			DefWindowProc(hwnd, msg, wparam, lparam);
			return 0;
			break;

		case SC_MOVE:
			// this is called when the user clicks on the title bar and does not move
			// if we don't do this, we don't actually move
			DefWindowProc(hwnd, msg, wparam, lparam);
			// if we don't do this (or we call DefWindowProc), we don't get a WM_NCLBUTTONUP
			return 0;
			break;
		}
		break;

	case WM_SOCKET:
		{
			if(!WSAGETSELECTERROR(lparam))
			{
				if(WSAGETSELECTEVENT(lparam) == FD_CLOSE)
				{
					netplay_disconnect = 1;
				}
			}
		}
		break;
	case WM_COPYDATA:
		{
			HWND			 hwndFrom = (HWND)wparam;
			COPYDATASTRUCT*  pcds	 = (COPYDATASTRUCT*)lparam;
			char fn[MAX_PATH];
			strcpy(fn, (LPCTSTR)pcds->lpData);
			Goto_WindowGameMode();
			LoadROM(fn);
			return 0;
		}
		break;
	case WM_SIZE:
//		LockWindowUpdate(hwnd);
		SetRect(&g_cscreen_rect, 0, 0, LOWORD(lparam), HIWORD(lparam));
		if(Mainwindowsbh && IsWindowVisible(Mainwindowsbh)){
			SendMessage(Mainwindowsbh, WM_SIZE, wparam, lparam);
			g_cscreen_rect.bottom -= Mainwindowsbhs;
		}
		if(!g_PreviewMode && nnnKailleraFlag && NESTER_settings.nes.preferences.KailleraChatWindow && g_hKailleraLog){
			g_cscreen_rect.bottom -= g_nKailleraChatWinH;
			int y = g_cscreen_rect.bottom + 2;
			MoveWindow(g_hKailleraLog, 0, y, LOWORD(lparam), g_nKailleraChatLogWinCharH*10, TRUE);
			y+= (g_nKailleraChatLogWinCharH*10)+2;
			MoveWindow(g_hKailleraSend, 0, y, LOWORD(lparam), g_nKailleraChatWinH - (g_nKailleraChatLogWinCharH*10+4), TRUE);
		}
		//		MoveWindow(hwnd, 0, 0, LOWORD(lparam) , HIWORD(lparam) - Mainwindowsbhs, TRUE);
		if(g_PreviewMode){
			int x = 0;
			int w = LOWORD(lparam);
			int h = HIWORD(lparam) - (Mainwindowsbhs+g_Preview_State.nToolTipHeight);
			if(g_Preview_State.ScreenDispFlag){
				w-=264;
			}
			if(g_Preview_State.TreeDispFlag){
				MoveWindow(g_Preview_State.hTreeView, x, g_Preview_State.nToolTipHeight, g_Preview_State.nTree_Width, h, TRUE);
				x+=g_Preview_State.nTree_Width+4;
				w-=g_Preview_State.nTree_Width+4;
			}
			MoveWindow(Mainwindow_Preview, x, g_Preview_State.nToolTipHeight, w , h, TRUE);
			g_Preview_State.PreviewScreenRect.top	=	g_Preview_State.nToolTipHeight;
			g_Preview_State.PreviewScreenRect.bottom=	g_Preview_State.nToolTipHeight+h;
			g_Preview_State.PreviewScreenRect.left	=	x+w;
			g_Preview_State.PreviewScreenRect.right	=	x+w+264;
			{
				g_Preview_State.PreviewScreenDrawRect.left=x+w+4;
				if(240 > h){
					g_Preview_State.PreviewScreenDrawRect.top =g_Preview_State.nToolTipHeight;
				}
				else{
					g_Preview_State.PreviewScreenDrawRect.top = h/2-240/2;
				}
			}
			SendMessage(g_Preview_State.hTool, WM_SIZE, wparam, lparam);
		}
//		LockWindowUpdate(NULL);
		return 0;
	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lparam;
			if(lpmis->CtlType== ODT_MENU){
				HDC hDC = GetDC(hwnd);
				MEASUREITEMSTRUCT* pmis = (MEASUREITEMSTRUCT*)lparam;
				HFONT hfontOld = (HFONT)SelectObject(hDC, MenuStrFont);
				SIZE size;
				int i;
				for(i=0;i<33;++i){
					if(Menustruct[i].MenuID == pmis->itemID)
						break;
				}
				if(i>=33)
					return TRUE;
				GetTextExtentPoint32(hDC, Menustruct[i].MenuStr, lstrlen(Menustruct[i].MenuStr), &size);
				if(size.cy<18)
					size.cy=18;
				pmis->itemWidth = size.cx+20;
				pmis->itemHeight = size.cy;
				SelectObject(hDC, hfontOld);
				ReleaseDC(hwnd, hDC);
			}
		}
		return TRUE;
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lparam;
			if(lpdis->CtlType== ODT_MENU){
				DrawMenuOwnerDraw((DRAWITEMSTRUCT*)lparam);
			}
		}
		return TRUE;
	case WM_LBUTTONDOWN:
		if(emu && emu->GetExControllerType())
			break;
		if(gscr_mgr->is_fullscreen())
			break;
		if(NESTER_settings.nes.preferences.DoubleClickFullScreen){
			if(dclick_timer){
				KillTimer(hwnd, ID_DCLKTIMER);
				dclick_timer=0;
			}
			dclick_timer = SetTimer(main_window_handle, ID_DCLKTIMER, GetDoubleClickTime(), NULL);
		}
/*
		if(g_PreviewMode && LOWORD(lparam)>264){
			break;
		}
*/
		if(g_PreviewMode && g_Preview_State.nTree_Width-1 <LOWORD(lparam) && g_Preview_State.nTree_Width+4 >LOWORD(lparam)){
			RECT r;
			GetClientRect(hwnd, &r);
			MouseCapture=1;
			SetCapture(hwnd);
			SetCursor(LoadCursor(NULL,IDC_SIZEWE));
			HDC hdc = GetDC(hwnd);
			SetRect(&TreeborderRect, g_Preview_State.nTree_Width-1, g_Preview_State.nToolTipHeight, g_Preview_State.nTree_Width+2, r.bottom);
			DrawFocusRect(hdc, &TreeborderRect);
			ReleaseDC(hwnd, hdc);
			break;
		}
		SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE|0x02, 0L);
		return 0;
	case WM_LBUTTONUP:
		if(MouseCapture){
			RECT r;
			ReleaseCapture();
			MouseCapture=0;
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			GetClientRect(hwnd, &r);
			{
				HDC hdc = GetDC(hwnd);
				DrawFocusRect(hdc, &TreeborderRect);
				ReleaseDC(hwnd, hdc);
			}
			if(r.right>LOWORD(lparam)/*&&r.left<LOWORD(lp)*/){
				if(g_Preview_State.nTree_Width < 16 || g_Preview_State.nTree_Width > 0x8000){
					g_Preview_State.nTree_Width = 16;
				}
				g_Preview_State.nTree_Width = LOWORD(lparam);
				SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, (r.bottom-r.top)<<16 | (r.right-r.left));
				SendMessage(hwnd, WM_PAINT, 0, 0);
			}
		}
		break;
	case WM_CONTEXTMENU:
		if(emu){
			int t;
			t = emu->GetExControllerType();
			if(t && t != EX_FAMILY_KEYBOARD)
				break;
		}
//		if(g_PreviewMode && LOWORD(lparam)>264)
//			break;
		{
			POINT pt;
			pt.x = LOWORD(lparam);
			pt.y = HIWORD(lparam);
			ScreenToClient(hwnd, &pt);
			if(pt.y<0)
				break;
		}
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		rpopupmflag=1;
		if(wparam == (WPARAM)Mainwindow_Preview){
			int i;
			POINT pt;
			LV_HITTESTINFO lvhit;
			pt.x = LOWORD(lparam);
			pt.y = HIWORD(lparam);
			memset(&lvhit, 0, sizeof(LVHITTESTINFO));
			lvhit.pt = pt;
			ScreenToClient(Mainwindow_Preview, &lvhit.pt);
			HMENU hPMenu = CreatePopupMenu();
			if(hPMenu!=NULL){
				char str[64];
				if(-1 != (i = ListView_HitTest(Mainwindow_Preview, &lvhit))){
					g_PreviewLClickItemn = i;
					LoadString(g_main_instance, IDS_STRING_PMENU_01 , str, 64);
					AppendMenu(hPMenu, MF_STRING, ID_PREVIEW_MENU_EST, str);
					LoadString(g_main_instance, IDS_STRING_PMENU_02 , str, 64);
					AppendMenu(hPMenu, MF_STRING, ID_PREVIEW_MENU_CHK, str);
					LoadString(g_main_instance, IDS_STRING_PMENU_03 , str, 64);
					AppendMenu(hPMenu, MF_STRING, ID_PREVIEW_MENU_FIX, str);
					AppendMenu(hPMenu, MF_SEPARATOR, 0, NULL);
				}
				if(emu){
					LoadString(g_main_instance, IDS_STRING_PMENU_04 , str, 64);
					AppendMenu(hPMenu, MF_STRING, ID_PREVIEW_MENU_STOP, str);
					AppendMenu(hPMenu, MF_SEPARATOR, 0, NULL);
				}
				LoadString(g_main_instance, IDS_STRING_PMENU_05 , str, 64);
				AppendMenu(hPMenu, MF_STRING, ID_PREVIEW_MENU_ADD, str);
				LoadString(g_main_instance, IDS_STRING_PMENU_06 , str, 64);
				AppendMenu(hPMenu, MF_STRING, ID_PREVIEW_MENU_FADD, str);
				LoadString(g_main_instance, IDS_STRING_PMENU_07 , str, 64);
				AppendMenu(hPMenu, MF_STRING, ID_PREVIEW_MENU_FSADD, str);
//				AppendMenu(hPMenu, MF_STRING, ID_PREVIEW_MENU_CFG,  "Config");
				TrackPopupMenu(hPMenu, TPM_LEFTALIGN /*| TPM_TOPALIGN*/, pt.x, pt.y, 0, hwnd, NULL);
				DestroyMenu(hPMenu);
			}
		}
		else if(wparam == (WPARAM)hwnd)
		{
			POINT pt;
			int i,j;
			pt.x = LOWORD(lparam);
			pt.y = HIWORD(lparam);
			HMENU hrMenu = CreatePopupMenu();
			HMENU hLSlotMenu=NULL,hSSlotMenu=NULL;
			if(hrMenu==NULL)
				return TRUE;
//			ClientToScreen(hwnd, &pt);

			if(emu){
					char str[64];
				{
					hLSlotMenu = CreateMenu();
					hSSlotMenu = CreateMenu();
					char *p;
					HANDLE hFind;
					WIN32_FIND_DATA fd;
					SYSTEMTIME st;
					FILETIME ft;
					int zs_flag=0;
					char fn[MAX_PATH], extn[5], datetxt[32];
					if(NESTER_settings.path.UseStatePath){
						strcpy(fn, NESTER_settings.path.szStatePath );
					}
					else
						strcpy(fn, emu->getROMpath());
					PathCombine(fn, fn, emu->getROMname());
					p=fn;
					while(*p)p++;
					for(i = 0; i < 10; i++){
						zs_flag=0;
						if(NESTER_settings.nes.preferences.StateFileToZip){
							sprintf(extn, ".zs%i", i);
						}
						else{
				defaultstatefn:
							zs_flag=1;
							sprintf(extn, ".ss%i", i);
						}
						strcpy(p, extn);
						hFind = FindFirstFile(fn, &fd);
						if(hFind == INVALID_HANDLE_VALUE || (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
							if(NESTER_settings.nes.preferences.StateFileToZip&&zs_flag==0)
								goto defaultstatefn;
							wsprintf(datetxt, "&%u -----", i);
							AppendMenu(hLSlotMenu, MF_STRING | MF_GRAYED, ID_FILE_QUICK_LOADN0+i, datetxt);
							AppendMenu(hSSlotMenu, MF_STRING, ID_FILE_QUICK_SAVEN0+i, datetxt);
						}
						else{
							FileTimeToLocalFileTime(&fd.ftLastWriteTime, &ft);
							FileTimeToSystemTime(&ft, &st);
							wsprintf(datetxt, "&%u  [%u/%u/%u/%u:%02u]", i, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
							if(zs_flag==0)
								strcat(datetxt, " Z");
							AppendMenu(hLSlotMenu, MF_STRING, ID_FILE_QUICK_LOADN0+i, datetxt);
							AppendMenu(hSSlotMenu, MF_STRING, ID_FILE_QUICK_SAVEN0+i, datetxt);
						}
						FindClose(hFind);
					}
					AppendMenu(hrMenu, MF_POPUP|MF_STRING, (unsigned int)hLSlotMenu, "Quick Load");
					AppendMenu(hrMenu, MF_POPUP|MF_STRING, (unsigned int)hSSlotMenu, "Quick Save");
					AppendMenu(hrMenu, MF_STRING, ID_FILE_RESET, "Reset");
					AppendMenu(hrMenu, MF_STRING, ID_FILE_CLOSE_ROM, "Close");
				}
				if(emu->GetMovieStatus()){
					AppendMenu(hrMenu, MF_SEPARATOR, 0, NULL);
				}
				if(emu->GetMovieStatus()==1){
					LoadString(g_main_instance, IDS_STRING_MENU_01 , str, 64);
					AppendMenu(hrMenu, MF_STRING, ID_FILE_MOVIE_STOP, str);
					LoadString(g_main_instance, IDS_STRING_MENU_02 , str, 64);
					AppendMenu(hrMenu, MF_STRING, ID_FILE_NPMOVIE_REC, str);
					LoadString(g_main_instance, IDS_STRING_MENU_03 , str, 64);
					AppendMenu(hrMenu, MF_STRING, ID_FILE_MOVIE_INSMSG, str);
					LoadString(g_main_instance, IDS_STRING_MENU_04 , str, 64);
					AppendMenu(hrMenu, MF_STRING, ID_FILE_MOVIE_DELMSG, str);
				}
				else if(emu->GetMovieStatus()==2){
					LoadString(g_main_instance, IDS_STRING_MENU_05 , str, 64);
					AppendMenu(hrMenu, MF_STRING, ID_FILE_MOVIE_STOP, str);
					LoadString(g_main_instance, IDS_STRING_MENU_06 , str, 64);
					AppendMenu(hrMenu, MF_STRING, ID_FILE_MOVIE_RPAUSE, str);
					LoadString(g_main_instance, IDS_STRING_MENU_07 , str, 64);
					AppendMenu(hrMenu, MF_STRING, ID_FILE_MOVIE_PRESST, str);
					LoadString(g_main_instance, IDS_STRING_MENU_03 , str, 64);
					AppendMenu(hrMenu, MF_STRING, ID_FILE_MOVIE_INSMSG, str);
					LoadString(g_main_instance, IDS_STRING_MENU_04 , str, 64);
					AppendMenu(hrMenu, MF_STRING, ID_FILE_MOVIE_DELMSG, str);
				}
				else if(emu->GetMovieStatus()==3){
					LoadString(g_main_instance, IDS_STRING_MENU_08 , str, 64);
					AppendMenu(hrMenu, MF_STRING, ID_FILE_MOVIE_RSTART, str);
				}
				AppendMenu(hrMenu, MF_SEPARATOR, 0, NULL);
				if(emu->GetExControllerType() == EX_FAMILY_KEYBOARD){
					AppendMenu(hrMenu, MF_STRING, ID_OPTIONS_FBASIC_INPUT, "Family Basic Auto Input(File)");
					AppendMenu(hrMenu, MF_STRING, ID_OPTIONS_FBASIC_INPUT_CB, "Family Basic Auto Input(CB)");
					AppendMenu(hrMenu, MF_SEPARATOR, 0, NULL);
				}
				if(emu){
					if(emu->IsAPULog()){
						AppendMenu(hrMenu, MF_STRING, IDM_APULOG, "APU LOG STOP");
					}
					else {
						AppendMenu(hrMenu, MF_STRING, IDM_APULOG, "APU LOG START");
					}
				}
			}
			AppendMenu(hrMenu, MF_STRING, ID_FILE_OPEN_ROM, "Open");
			AppendMenu(hrMenu, MF_SEPARATOR, 0, NULL);

			j=NESTER_settings.recent_ROMs.get_num_entries();
			if(!j)
				return TRUE;
			char text[256];
			for(i = 0; i < j; i++){
				sprintf(text, "&%u ", i);
				char *p=NULL;
				p = PathFindFileName(NESTER_settings.recent_ROMs.get_entry(i));
				if(p==NULL)
					continue;
				strcat(text, p);
/*
				for(uint32 k = 0; k < strlen(NESTER_settings.recent_ROMs.get_entry(i)); k++){
					char temp[2] = " ";
					temp[0] = NESTER_settings.recent_ROMs.get_entry(i)[k];
					strcat(text, temp);
					if(temp[0] == '&') strcat(text, temp);
				}
*/
				AppendMenu(hrMenu, MF_STRING, ID_FILE_RECENT_0+i, text);
			}
			TrackPopupMenu(hrMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
			if(hLSlotMenu)
				DestroyMenu(hLSlotMenu);
			if(hLSlotMenu)
				DestroyMenu(hLSlotMenu);
			DestroyMenu(hrMenu);
			rpopupmflag=0;
		}
		return TRUE;
	case WM_LBUTTONDBLCLK:
		if(dclick_timer){
			KillTimer(hwnd, ID_DCLKTIMER);
			dclick_timer=0;
		}
		if(gscr_mgr->is_fullscreen()){
			if(NESTER_settings.nes.preferences.DoubleClickFullScreen)
				toggle_fullscreen();
			break;
		}
		if(emu && emu->GetExControllerType())
			break;
		if(emu && g_PreviewMode && LOWORD(lparam)>g_Preview_State.PreviewScreenRect.left && LOWORD(lparam)<g_Preview_State.PreviewScreenRect.right){
			Goto_WindowGameMode();
			if(emu){
//				emu->ToggleFastFPS();
				assertWindowSize();
				assertWindowStyle();
			}
			return 0;
		}
		else if(g_PreviewMode==0 && NESTER_settings.nes.preferences.PreviewMode){
			if(!NESTER_settings.nes.preferences.DoubleClickFullScreen){
				Return_PreviewMode();
				return 0;
			}
			else{
				RECT rect;
				GetClientRect(hwnd, &rect);
				if(LOWORD(lparam) < rect.right/2)
					toggle_fullscreen();
				else
					Return_PreviewMode();
				return 0;
			}
		}
		if(g_PreviewMode==0 && NESTER_settings.nes.preferences.DoubleClickFullScreen){
			toggle_fullscreen();
		}
		break;
/*
	case WM_PARENTNOTIFY:
		if(LOWORD(wparam) == WM_LBUTTONDOWN){
			int in;
			RECT rect, lvrect;
			POINT pt, pt2;
			pt.x = LOWORD(lparam);
			pt.y = HIWORD(lparam);
			ClientToScreen(hwnd, &pt);
			ScreenToClient(Mainwindow_Preview, &pt);
			GetClientRect(Mainwindow_Preview, &rect);
			ListView_GetItemRect(Mainwindow_Preview, 0, &lvrect, LVIR_SELECTBOUNDS);
			pt2 = pt;
			pt2.y-=(lvrect.bottom-lvrect.top);
			if(pt2.y >= 0 && pt2.x >= 0 && pt2.y < rect.bottom && pt2.x < rect.right){
				LV_HITTESTINFO lvhit;
				lvhit.pt = pt;
				if(-1 != (in = ListView_HitTest(Mainwindow_Preview, &lvhit))){
					g_PreviewLClickItemn = in;
					SendMessage(hwnd, WM_COMMAND, ID_PREVIEW_MENU_EST, 0);
					return TRUE;
				}
			}
		}
		return TRUE;
		break;
*/
	case WM_NOTIFY:
		{
			if((int)wparam==ID_PREVIELV){
				LV_DISPINFO *lvinfo = (LV_DISPINFO *)lparam;
				static HWND hEdit;
//				char tmp[256];
				switch(lvinfo->hdr.code){
					case LVN_GETDISPINFO:
						OnGetDispInfo((NMLVDISPINFO *)lparam);
						break;
					case LVN_ODFINDITEM:
						return OnOdfinditem((LPNMLVFINDITEM)lparam);
						break;
/*
					case LVN_BEGINLABELEDIT:
						nnnprevieweditlabel = 1;
						hEdit = ListView_GetEditControl(Mainwindow_Preview);
						break;
					case LVN_ENDLABELEDIT:
						GetWindowText(hEdit, tmp, 256);
						strcpy(g_Preview_State.pPreviewItemText[lvinfo->item.iItem].Title, tmp);
						ListView_SetItemCount(Mainwindow_Preview, g_Preview_State.PreviewListAll);
						nnnprevieweditlabel = 0;
						break;
*/
					case LVN_COLUMNCLICK:
						{
							NM_LISTVIEW *pNMLV = (NM_LISTVIEW *)lparam;
							g_Preview_State.PrevSortItemp[pNMLV->iSubItem]^=1;
							PreviewListSort(pNMLV->iSubItem);
						}
						break;
					case NM_DBLCLK:
						{
							LVHITTESTINFO hitpos;
							int i;

							memset(&hitpos, 0, sizeof(LVHITTESTINFO));
							GetCursorPos(&hitpos.pt);
							POINT pt = hitpos.pt;
							ScreenToClient(hwnd, &pt);
							ScreenToClient(Mainwindow_Preview, &hitpos.pt);
							if(-1 !=(i=ListView_HitTest(Mainwindow_Preview, &hitpos))){
								g_PreviewLClickItemn = i;
								SendMessage(hwnd, WM_COMMAND, ID_PREVIEW_MENU_EST, 0);
							}
						}
						break;
					case LVN_KEYDOWN:
						{
							LPNMLVKEYDOWN lpkaydown = (LPNMLVKEYDOWN)lparam;
							int i=-1;
							switch(lpkaydown->wVKey){
								case VK_ESCAPE:
									if(emu)
										FreeROM();
									break;
								case VK_RETURN:
									i=ListView_GetNextItem(Mainwindow_Preview, i, LVNI_ALL | LVNI_SELECTED);
									if(i==-1)
										break;
									g_PreviewLClickItemn = i;
									SendMessage(hwnd, WM_COMMAND, ID_PREVIEW_MENU_EST, 0);
									break;
								case VK_TAB:
									if(g_Preview_State.TreeDispFlag)
										SetFocus(g_Preview_State.hTreeView);
									break;
							}
						}
						break;
				}
			}
			else if((int)wparam == ID_TREEVIEW){
				NM_TREEVIEW *lpnmt = (NM_TREEVIEW *)lparam;
				switch(lpnmt->hdr.code){
					case TVN_SELCHANGED:
						SetTreeViewSelectItem((HTREEITEM)lpnmt->itemNew.hItem);
						break;
					case TVN_KEYDOWN:
						{
							LPNMTVKEYDOWN ptvkd = (LPNMTVKEYDOWN)lparam;
							switch(ptvkd->wVKey){
								case VK_TAB:
									SetFocus(Mainwindow_Preview);
									break;
							}
						}
						break;
				}
			}
		}
		return 0;
		break;
//		return TRUE;
	case WM_HOTKEY:
		switch((int)wparam){
			case 0:	// 
				{
					static int f = 0;

					if(BossFlag){
						if(f){
							thaw();
							f = 0;
						}
						ShowWindow(hwnd, SW_SHOW);
						if(previewh)
							ShowWindow(previewh, SW_SHOW);
						Extrawin_ShowWin();
						SetForegroundWindow(hwnd);
					}
					else{
						if(emu){
							freeze();
							f = 1;
						}
						Extrawin_HideWin();
						ShowWindow(hwnd, SW_HIDE);
						if(previewh)
							ShowWindow(previewh, SW_HIDE);
					}
					BossFlag ^= 1;
					break;
				}
				break;
		}
		return 0;
		break;
	default:
		break;

	} // end switch

	// process any messages that we didn't take care of
	return(DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

void EmuError(const char* error)
{
	char msg[1024];
	strcpy(msg, "Emulation error:\n");
	strcat(msg, error);
	strcat(msg, "\nFreeing ROM and halting emulation.");
	MessageBox(main_window_handle, msg, "Error", MB_OK);
	LOG(msg << endl);
	SocketClose();
	FreeROM();
}



void MainWinLoop(MSG& msg, HWND hwnd, HACCEL hAccel)
{
	while(1)
	{
		if(emu && !emu->frozen())
		{
			try {
				emu->do_frame();
				RunFrame_ExtraWindow();
			} catch(const char* s) {
				LOG("EXCEPTION: " << s << endl);
				EmuError(s);
			} catch(...) {
				LOG("Caught unknown exception in " << __FILE__ << endl);
				EmuError("unknown error");
			}

			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				//		DUMP_WM_MESSAGE(1,msg.message);

				if(msg.message == WM_QUIT) return;
				if(!CheckModelessDialogMessage(&msg)){
					
#ifdef TOOLTIP_HACK
					if(msg.message != 0x0118)
#endif
						if(!TranslateAccelerator(hwnd, hAccel, &msg))
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
				}
			}
		}
		else
		{

			if(emu && hwnd==GetForegroundWindow()){
				Sleep(100);
				ginp_mgr->Poll();
				emu->ExtKeyExe();
				if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
					if(msg.message == WM_QUIT) return;
					if(!CheckModelessDialogMessage(&msg)){
#ifdef TOOLTIP_HACK
						if(msg.message != 0x0118)
#endif
							if(!TranslateAccelerator(hwnd, hAccel, &msg))
							{
								TranslateMessage(&msg);
								DispatchMessage(&msg);
							}
					}
				}
			}
			else if(GetMessage(&msg, NULL, 0, 0))
			{
				//		DUMP_WM_MESSAGE(2,msg.message);
				if(!CheckModelessDialogMessage(&msg)){

#ifdef TOOLTIP_HACK
				if(msg.message != 0x0118)
#endif
					if(!TranslateAccelerator(hwnd, hAccel, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}
			else
			{
				return;
			}
		}

		// network disconnect
		if(netplay_disconnect)
		{
			SocketClose();
			MessageBox(main_window_handle, "network disconnected", "NETOWRK ERROR", MB_OK);
			FreeROM();
		}
	}
}

void InitControlsNStuff()
{
	//  INITCOMMONCONTROLSEX icce;

	//  memset((void*)&icce, (int)0, sizeof(icce));
	//  icce.dwSize = sizeof(icce);
	//  icce.dwICC = ICC_BAR_CLASSES;
	//  InitCommonControlsEx(&icce);
	// win95 barfs on InitCommonControlsEx()
	InitCommonControls();
}


BOOL CheckMutex(HANDLE *hMutex, char *clfn){
	*hMutex = CreateMutex(NULL, TRUE, "NNNesterJ");
	//	if(hMutex)
	//		return FALSE;
	if(GetLastError() == ERROR_ALREADY_EXISTS) {
		HWND hWnd = FindWindow(WINCLASS_NAME, NULL);
		if(hWnd){
			if(IsIconic(hWnd))
				ShowWindow(hWnd, SW_RESTORE );
			SetForegroundWindow(hWnd);
			if(clfn){
				char rfn[MAX_PATH*2];
				strcpy(rfn, clfn);
				PathRemoveArgs(rfn);
				PathUnquoteSpaces(rfn);
				COPYDATASTRUCT  cds;
				cds.dwData = 0;
				cds.lpData = (void*)rfn;
				cds.cbData = lstrlen(rfn)+1;
				if(PathFileExists(rfn)){
					SendMessage(hWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds );
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}




// WINMAIN ////////////////////////////////////////////////

HACCEL   hAccel;  // handle to keyboard accelerators

int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE hprevinstance,
				   LPSTR lpcmdline,
				   int ncmdshow)
{

	WNDCLASSEX  winclass; // window class we create
	HWND	 hwnd;	  // window handle
	MSG	  msg;	   // message
	HANDLE hMutex=NULL;


	try {
		memset(&g_extra_window, 0, sizeof(g_extra_window));
		memset(&g_Cheat_Info, 0, sizeof(g_Cheat_Info));
		NESTER_settings.DllLoad();
		if(NESTER_settings.ResourceDll[0]){
			g_main_instance	= LoadLibrary(NESTER_settings.ResourceDll);
			if(g_main_instance==NULL)
				NESTER_settings.ResourceDll[0] = 0;
		}
		if(!NESTER_settings.ResourceDll[0]){
			char str[MAX_PATH];
			GetModuleFileName(NULL, str, MAX_PATH);
			PathRemoveFileSpec(str);
			PathCombine(str , str, "lang_eng.dll");
			g_main_instance	= LoadLibrary(str);
			if(g_main_instance==NULL){
				PathRemoveFileSpec(str);
				PathCombine(str , str, "lang_jpn.dll");
				g_main_instance	= LoadLibrary(str);
				if(g_main_instance==NULL){
					MessageBox(NULL, "Resource DLL load error.", "ERROR", MB_OK);
					return 0;
				}
			}
		}
		MyLoadAllResource();
		NESTER_settings.Load();
	} catch(const char* IFDEBUG(s)) {
		LOG(s);
	} catch(...) {
	}
	if(!NESTER_settings.nes.preferences.AllowMultiInstance){
		if(CheckMutex(&hMutex, lpcmdline)){
			FreeLibrary(g_main_instance);
			return(0);
		}
	}

	// fill in the window class stucture
	winclass.cbSize	 = sizeof(winclass);
	winclass.style	  = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_DBLCLKS;
	winclass.lpfnWndProc  = WindowProc;
	winclass.cbClsExtra   = 0;
	winclass.cbWndExtra   = 0;
	winclass.hInstance	= hinstance;
	winclass.hIcon		= LoadIcon(g_main_instance, MAKEINTRESOURCE(IDI_NNNICON)); //
	winclass.hCursor	  = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground  = (HBRUSH)GetStockBrush(BLACK_BRUSH);
	winclass.lpszMenuName   = NULL; //LoadMenu(g_res_instance, MAKEINTRESOURCE(IDR_MENU1));
	winclass.lpszClassName  = WINCLASS_NAME;
	winclass.hIconSm	  = NULL;

	// register the window class
	if(!RegisterClassEx(&winclass)){
		FreeLibrary(g_main_instance);
		return(0);
	}

	InitControlsNStuff();

	hAccel = LoadAccelerators(g_main_instance, MAKEINTRESOURCE(IDR_MAIN_ACCEL));

//	g_main_instance	  = hinstance;
	g_prg_instance = hinstance;

	// create the window
	if(!(hwnd = CreateWindowEx(0,
							   WINCLASS_NAME, // class
							   PROG_NAME,   // title
							   STYLE_WINDOWED /*| WS_CLIPCHILDREN*/,
							   NESTER_settings.nes.graphics.osd.WindowPos_x, // x
							   NESTER_settings.nes.graphics.osd.WindowPos_y, // y
							   0,  // width
							   0, // height
							   NULL,	 // handle to parent
							   LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENU1)), //NULL,	 // handle to menu
							   hinstance,// instance
							   NULL))){  // creation parms
		FreeLibrary(g_main_instance);
		return(0);
	}

	// save the window handle and instance in globals
	main_window_handle = hwnd;

	if(NESTER_settings.nes.preferences.PreviewMode)
	{
		Create_MainPreviewWind(hwnd, g_main_instance);
		CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_PREVIEW, MFS_CHECKED);
	}
	else{
		assertWindowSize();
		CenterWindow();
	}
	setWindowedWindowStyle();

	ShowWindow(hwnd, ncmdshow);
	UpdateWindow(hwnd);
	SetFocus(hwnd);


	// init network
	WSADATA wsaData;
	WSAStartup(0x0101, &wsaData);

	sock = INVALID_SOCKET;
	sv_sock = INVALID_SOCKET;
	netplay_status = 0;
	netplay_disconnect = 0;



	try {
		LoadCmdLineROM(lpcmdline);
		timeBeginPeriod(1);
		// sit and spin
		MainWinLoop(msg, hwnd, hAccel);
		timeEndPeriod(1);

		// shut down
		NESTER_settings.Save();

		if(emu) FreeROM();

		// make sure directx is shut down
		iDirectX::releaseAll();

		// shutdown network
		SocketClose();
		WSACleanup();


	} catch(const char* IFDEBUG(s)) {
		LOG("EXCEPTION: " << s << endl);
	} catch(...) {
		LOG("Caught unknown exception in " << __FILE__ << endl);
	}

	FreeLibrary(g_main_instance);
	if(hRichd32Lib)
		FreeLibrary(hRichd32Lib);
	if(hMutex){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}
	nnnkailleraShutdown();

	return(msg.wParam);
}
