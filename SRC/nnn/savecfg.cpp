
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>

#include <stdio.h>
#include "debug.h"
#include "settings.h"

//#include "savecfg.h"

#include <dinput.h>
#include "OSD_ButtonSettings.h"
//#include "win32_default_controls.h"
#include "nnnextkeycfg.h"
#include "resource.h"
#include "cheat.h"
#include "savecfg.h"

extern HINSTANCE g_main_instance;
extern struct cheat_info_struct g_Cheat_Info;
extern struct MyWindowShortCutKey g_Window_ShotrCutKey[1];

unsigned char Hexstrtouc(char *str){
	unsigned char ret=0, *p = (unsigned char *)str;
	for(int i=0;i<2;i++, p++){
		if(i)
			ret<<=4;
		if(*p >= 0x30 && *p <= 0x39){
			ret|=(DWORD)*p-'0';
		}
		else if(*p >= 0x41 && *p <= 0x5a){
			ret|=(DWORD)*p-0x37;
		}
		else if(*p >= 0x61 && *p <= 0x7a){
			ret|=(DWORD)*p-0x57;
		}
	}
	return ret;
}

//
extern struct Preview_state g_Preview_State;

void Ini_LoadBinary(char *inifn, char *secName, char *keyName, int size, unsigned char *pDest){
	unsigned char *p;
	int i;

	p = (unsigned char *)malloc(size*2+1);
	if(p==NULL)
		return;
	if(GetPrivateProfileString(secName, keyName, "" , (char *)p, size*2+1, inifn)){
		for(i=0; i<size; i++, pDest++){
			*pDest = Hexstrtouc((char *)&p[i*2]);
		}
	}
	free(p);
}


void Ini_SaveBinary(char *inifn, char *secName, char *keyName, int size, unsigned char *pSrc){
	unsigned char *p;
	int i;

	p = (unsigned char *)malloc(size*2+1);
	if(p==NULL)
		return;
	for(i=0; i<size; i++, pSrc++){
		wsprintf((char *)&p[i*2], "%02X", *pSrc);
	}
	WritePrivateProfileString(secName, keyName, (const char *)p, inifn);
	free(p);
}



void LoadPreviewSettings(char *inifn, char *secName){
	char kstr[32];
	int i;
	const char *databasefn[] = { "famicom.dat", "nesdbase.dat", "ingredients.dat" };

//	memset(&g_Preview_State, 0, sizeof(Preview_state));
/*
	{
		char str[16];
		LoadString(g_main_instance, IDS_STRING_LANGCODE , str, 16);
		lang_code = !strcmp(str, "JPN");
	}
*/
	for(i=0; i<COLUMN_NUM; i++){
		wsprintf(kstr, "PreviewColumn_w%u", i);
		g_Preview_State.Column_w[i] = GetPrivateProfileInt(secName, kstr, 80, inifn);
		wsprintf(kstr, "PreviewColumn_Pos%u", i);
		g_Preview_State.Column_Pos[i] = GetPrivateProfileInt(secName, kstr, i, inifn);
	}
	{
		char str[MAX_PATH];
		GetModuleFileName(NULL, str, MAX_PATH);
		for(i=0; i<3; i++){ // database filename
			PathRemoveFileSpec(str);
			PathCombine(str , str, databasefn[i]);
			wsprintf(kstr, "Database_%u", i);
			GetPrivateProfileString(secName, kstr, str, g_Preview_State.DATABASE_FileName[i], MAX_PATH, inifn);
		}
	}
	g_Preview_State.window_w = GetPrivateProfileInt(secName, "Window_w", 640, inifn);
	g_Preview_State.window_h = GetPrivateProfileInt(secName, "Window_h", 400, inifn);

	g_Preview_State.NotLoadNESJ_DB= GetPrivateProfileInt(secName, "NotLoadNESJ_DB", 0, inifn);

	g_Preview_State.NotLoadNESE_DB= GetPrivateProfileInt(secName, "NotLoadNESE_DB", 1, inifn);
	g_Preview_State.NotLoadFDS_DB = GetPrivateProfileInt(secName, "NotLoadFDS_DB", 0, inifn);
	g_Preview_State.NotApplyNES_FixDB = GetPrivateProfileInt(secName, "NotApplyNES_FixDB", 0, inifn);
	g_Preview_State.NotApplyFDS_FixDB = GetPrivateProfileInt(secName, "NotApplyFDS_FixDB", 0, inifn);
	g_Preview_State.NotLoadNES_AddDB = GetPrivateProfileInt(secName, "NotLoadNES_AddDB", 0, inifn);
	g_Preview_State.NotLoadFDS_AddDB = GetPrivateProfileInt(secName, "NotLoadFDS_AddDB", 0, inifn);
	g_Preview_State.DisableSound = GetPrivateProfileInt(secName, "DisableSound", 0, inifn);
	g_Preview_State.EmuSpeed = GetPrivateProfileInt(secName, "EmuSpeed", 0, inifn);
	g_Preview_State.NESROM_CRCFrom_Filesize = GetPrivateProfileInt(secName, "NESROM_CRCFrom_Filesize", 0, inifn);

	g_Preview_State.nTree_Width = GetPrivateProfileInt(secName, "nTree_w", 64, inifn);
	g_Preview_State.TreeDispFlag = GetPrivateProfileInt(secName, "TreeDispFlag", 1, inifn);
	g_Preview_State.ScreenDispFlag= GetPrivateProfileInt(secName, "ScreenDispFlag", 1, inifn);
	g_Preview_State.GameTitleDetailDisplay= GetPrivateProfileInt(secName, "GameTitleDetailDisplay", 0, inifn);

	g_Preview_State.BGColor		= GetPrivateProfileInt(secName, "BGColor", 0xffffff, inifn);
	g_Preview_State.FontColor	= GetPrivateProfileInt(secName, "FontColor", 0, inifn);
	{
		unsigned char p[sizeof(LOGFONT)*2+1];
		if(GetPrivateProfileString(secName, "LogFont", "" , (char *)p, sizeof(p), inifn)){
			unsigned char *gp= (unsigned char *)&g_Preview_State.LogFont;
			for(int i=0; i<sizeof(LOGFONT); i++, gp++){
				*gp = Hexstrtouc((char *)&p[i*2]);
			}
		}
	}

	g_Preview_State.nROMDirectory= GetPrivateProfileInt(secName, "nROMDirectory", 0, inifn);
	for(i=0;i<g_Preview_State.nROMDirectory;i++){
		wsprintf(kstr, "ROM_Dir_%u", i);
		GetPrivateProfileString(secName, kstr, "" , g_Preview_State.ROM_Directory[i], MAX_PATH, inifn);
		wsprintf(kstr, "ROM_DirSF_%u", i);
		g_Preview_State.ROM_SubDirectory[i] = GetPrivateProfileInt(secName, kstr, 0, inifn);
	}
}


void SavePreviewSettings(char *inifn, char *secName){
	char vstr[32], kstr[32];
	int i;

	for(i=0; i<COLUMN_NUM; i++){
		wsprintf(kstr, "PreviewColumn_w%u", i);
		wsprintf(vstr, "%u", g_Preview_State.Column_w[i]);
		WritePrivateProfileString(secName, kstr, vstr, inifn);
		wsprintf(kstr, "PreviewColumn_Pos%u", i);
		wsprintf(vstr, "%u", g_Preview_State.Column_Pos[i]);
		WritePrivateProfileString(secName, kstr, vstr, inifn);
	}
	wsprintf(vstr, "%u", g_Preview_State.window_w);
	WritePrivateProfileString(secName, "Window_w", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.window_h);
	WritePrivateProfileString(secName, "Window_h", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.nROMDirectory);
	WritePrivateProfileString(secName, "nROMDirectory", vstr, inifn);
	for(i=0;i<g_Preview_State.nROMDirectory;i++){
		wsprintf(kstr, "ROM_Dir_%u", i);
		WritePrivateProfileString(secName, kstr, g_Preview_State.ROM_Directory[i], inifn);
		wsprintf(kstr, "ROM_DirSF_%u", i);
		wsprintf(vstr, "%u", g_Preview_State.ROM_SubDirectory[i]);
		WritePrivateProfileString(secName, kstr, vstr, inifn);
	}
	for(i=0;i<3;i++){ // Database Filename
		wsprintf(kstr, "Database_%u", i);
		WritePrivateProfileString(secName, kstr, g_Preview_State.DATABASE_FileName[i], inifn);
	}
	wsprintf(vstr, "%u", g_Preview_State.nTree_Width);
	WritePrivateProfileString(secName, "nTree_w", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.TreeDispFlag);
	WritePrivateProfileString(secName, "TreeDispFlag", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.ScreenDispFlag);
	WritePrivateProfileString(secName, "ScreenDispFlag", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.GameTitleDetailDisplay);
	WritePrivateProfileString(secName, "GameTitleDetailDisplay", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.NotLoadNESE_DB);
	WritePrivateProfileString(secName, "NotLoadNESE_DB", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.NotLoadNESJ_DB);
	WritePrivateProfileString(secName, "NotLoadNESJ_DB", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.NotLoadFDS_DB);
	WritePrivateProfileString(secName, "NotLoadFDS_DB", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.NotApplyNES_FixDB);
	WritePrivateProfileString(secName, "NotApplyNES_FixDB", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.NotApplyFDS_FixDB);
	WritePrivateProfileString(secName, "NotApplyFDS_FixDB", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.NotLoadNES_AddDB);
	WritePrivateProfileString(secName, "NotLoadNES_AddDB", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.NotLoadFDS_AddDB);
	WritePrivateProfileString(secName, "NotLoadFDS_AddDB", vstr, inifn);

	wsprintf(vstr, "%u", g_Preview_State.DisableSound);
	WritePrivateProfileString(secName, "DisableSound", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.EmuSpeed);
	WritePrivateProfileString(secName, "EmuSpeed", vstr, inifn);

	wsprintf(vstr, "%u", g_Preview_State.NESROM_CRCFrom_Filesize);
	WritePrivateProfileString(secName, "NESROM_CRCFrom_Filesize", vstr, inifn);

//	wsprintf(vstr, "%u", g_Preview_State.BGColor);
//	WritePrivateProfileString(secName, "BGColor", vstr, inifn);
	wsprintf(vstr, "%u", g_Preview_State.FontColor);
	WritePrivateProfileString(secName, "FontColor", vstr, inifn);
	{
		unsigned char p[sizeof(LOGFONT)*2+1];
		unsigned char *gp= (unsigned char *)&g_Preview_State.LogFont;
		for(int i=0; i<sizeof(LOGFONT); i++, gp++){
			wsprintf((char *)&p[i*2], "%02X", *gp);
		}
		WritePrivateProfileString(secName, "LogFont", (const char *)p, inifn);
	}
}






////////
/* 
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
*/
extern LOGFONT CheatDlgFont;
extern DWORD CheatDlgFontColor;


void LoadCheatDlgSettings(char *inifn, char *secName){
	g_Cheat_Info.DisableCheat = GetPrivateProfileInt(secName, "DisableCheat", 0, inifn);
	g_Cheat_Info.ctcmpdtf1 = GetPrivateProfileInt(secName, "ctcmpdtf1", 0, inifn);
	g_Cheat_Info.ctcmpdtf2 = GetPrivateProfileInt(secName, "ctcmpdtf2", 0, inifn);
	g_Cheat_Info.ctcmpdtf3 = GetPrivateProfileInt(secName, "ctcmpdtf3", 0, inifn);
	g_Cheat_Info.ctcmpdtf4 = GetPrivateProfileInt(secName, "ctcmpdtf4", 0, inifn);
	g_Cheat_Info.ctcmpdtf5 = GetPrivateProfileInt(secName, "ctcmpdtf5", 0, inifn);
	g_Cheat_Info.ctcmpdtf6 = GetPrivateProfileInt(secName, "ctcmpdtf6", 0, inifn);
	g_Cheat_Info.cc_clistedit1 = GetPrivateProfileInt(secName, "cc_clistedit1", 0, inifn);
	g_Cheat_Info.cc_clistedit2 = GetPrivateProfileInt(secName, "cc_clistedit2", 0, inifn);
	g_Cheat_Info.cc_csrchrb1 = GetPrivateProfileInt(secName, "cc_csrchrb1", 0, inifn);
	g_Cheat_Info.cc_csrchrb2 = GetPrivateProfileInt(secName, "cc_csrchrb2", 0, inifn);
	g_Cheat_Info.cc_csrchrb3 = GetPrivateProfileInt(secName, "cc_csrchrb3", 0, inifn);
	CheatDlgFontColor	= GetPrivateProfileInt(secName, "FontColor", 0, inifn);
	{
		unsigned char p[sizeof(LOGFONT)*2+1];
		if(GetPrivateProfileString(secName, "LogFont", "" , (char *)p, sizeof(p), inifn)){
			unsigned char *gp= (unsigned char *)&CheatDlgFont;
			for(int i=0; i<sizeof(LOGFONT); i++, gp++){
				*gp = Hexstrtouc((char *)&p[i*2]);
			}
		}
	}
	{
		unsigned char p[7*4+1];
		int i;
		if(GetPrivateProfileString(secName, "CC_List_Order", "" , (char *)p, sizeof(p), inifn)){
			for(i=0; i<7; i++){
				g_Cheat_Info.cc_listorder[i] = Hexstrtouc((char *)&p[i*2]);
			}
		}
		if(GetPrivateProfileString(secName, "CC_List_Width", "" , (char *)p, sizeof(p), inifn)){
			for(i=0; i<7; i++){
				g_Cheat_Info.cc_listwidth[i] = Hexstrtouc((char *)&p[i*4])<<8;
				g_Cheat_Info.cc_listwidth[i] |= Hexstrtouc((char *)&p[i*4+2]);
			}
		}
	}
}


void SaveCheatDlgSettings(char *inifn, char *secName){
	char vstr[32];

	wsprintf(vstr, "%u", g_Cheat_Info.DisableCheat);
	WritePrivateProfileString(secName, "DisableCheat", vstr, inifn);

	wsprintf(vstr, "%u", g_Cheat_Info.ctcmpdtf1);
	WritePrivateProfileString(secName, "ctcmpdtf1", vstr, inifn);
	wsprintf(vstr, "%u", g_Cheat_Info.ctcmpdtf2);
	WritePrivateProfileString(secName, "ctcmpdtf2", vstr, inifn);
	wsprintf(vstr, "%u", g_Cheat_Info.ctcmpdtf3);
	WritePrivateProfileString(secName, "ctcmpdtf3", vstr, inifn);
	wsprintf(vstr, "%u", g_Cheat_Info.ctcmpdtf4);
	WritePrivateProfileString(secName, "ctcmpdtf4", vstr, inifn);
	wsprintf(vstr, "%u", g_Cheat_Info.ctcmpdtf5);
	WritePrivateProfileString(secName, "ctcmpdtf5", vstr, inifn);
	wsprintf(vstr, "%u", g_Cheat_Info.ctcmpdtf6);
	WritePrivateProfileString(secName, "ctcmpdtf6", vstr, inifn);
	wsprintf(vstr, "%u", g_Cheat_Info.cc_clistedit1);
	WritePrivateProfileString(secName, "cc_clistedit1", vstr, inifn);
	wsprintf(vstr, "%u", g_Cheat_Info.cc_clistedit2);
	WritePrivateProfileString(secName, "cc_clistedit2", vstr, inifn);
	wsprintf(vstr, "%u", g_Cheat_Info.cc_csrchrb1);
	WritePrivateProfileString(secName, "cc_csrchrb1", vstr, inifn);
	wsprintf(vstr, "%u", g_Cheat_Info.cc_csrchrb2);
	WritePrivateProfileString(secName, "cc_csrchrb2", vstr, inifn);
	wsprintf(vstr, "%u", g_Cheat_Info.cc_csrchrb3);
	WritePrivateProfileString(secName, "cc_csrchrb3", vstr, inifn);

	wsprintf(vstr, "%u", CheatDlgFontColor);
	WritePrivateProfileString(secName, "FontColor", vstr, inifn);
	{
		unsigned char p[sizeof(LOGFONT)*2+1];
		unsigned char *gp= (unsigned char *)&CheatDlgFont;
		for(int i=0; i<sizeof(LOGFONT); i++, gp++){
			wsprintf((char *)&p[i*2], "%02X", *gp);
		}
		WritePrivateProfileString(secName, "LogFont", (const char *)p, inifn);
	}
	{
		unsigned char p[7*4+1];
		int i;
		for(i=0; i<7; i++){
			wsprintf((char *)&p[i*2], "%02X", g_Cheat_Info.cc_listorder[i]);
		}
		WritePrivateProfileString(secName, "CC_List_Order", (const char *)p, inifn);
		for(i=0; i<7; i++){
			wsprintf((char *)&p[i*4], "%04X", g_Cheat_Info.cc_listwidth[i]&0xffff);
		}
		WritePrivateProfileString(secName, "CC_List_Width", (const char *)p, inifn);
	}
}



void LoadPathSettings(char *inifn, char *secName, CPathSettings *nspath){
	char str[MAX_PATH], *p;
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);

	for(p=str; *p; p++);
	strcpy(p, "\\save\\");
	GetPrivateProfileString(secName, "SRAM", str , nspath->szSramPath, MAX_PATH, inifn);
	strcpy(p, "\\state\\");
	GetPrivateProfileString(secName, "State", str , nspath->szStatePath, MAX_PATH, inifn);
	strcpy(p, "\\shot\\");
	GetPrivateProfileString(secName, "Shot", str , nspath->szShotPath, MAX_PATH, inifn);
	strcpy(p, "\\wave\\");
	GetPrivateProfileString(secName, "Wave", str , nspath->szWavePath, MAX_PATH, inifn);
	strcpy(p, "\\movie\\");
	GetPrivateProfileString(secName, "Movie", str , nspath->szMoviePath, MAX_PATH, inifn);
	strcpy(p, "\\genie\\");
	GetPrivateProfileString(secName, "Genie", str , nspath->szGameGeniePath, MAX_PATH, inifn);
	strcpy(p, "\\cheat\\");
	GetPrivateProfileString(secName, "NNNcht", str , nspath->szNNNcheatPath, MAX_PATH, inifn);
	strcpy(p, "\\ips\\");
	GetPrivateProfileString(secName, "IPSPatch", str , nspath->szIPSPatchPath, MAX_PATH, inifn);
//	strcpy(p, "\\\\");
	GetPrivateProfileString(secName, "LastState", "" , nspath->szLastStatePath, MAX_PATH, inifn);

	nspath->UseSramPath = GetPrivateProfileInt(secName, "UseSramPath", 1, inifn);
	nspath->UseStatePath= GetPrivateProfileInt(secName, "UseStatePath", 1, inifn);
	nspath->UseShotPath = GetPrivateProfileInt(secName, "UseShotPath", 1, inifn);
	nspath->UseWavePath = GetPrivateProfileInt(secName, "UseWavePath", 1, inifn);
	nspath->UseMoviePath= GetPrivateProfileInt(secName, "UseMoviePath", 1, inifn);
	nspath->UseGeniePath= GetPrivateProfileInt(secName, "UseGeniePath", 1, inifn);
	nspath->UseNNNchtPath=GetPrivateProfileInt(secName, "UseNNNchtPath", 1, inifn);
	nspath->UseIPSPath  = GetPrivateProfileInt(secName, "UseIPSPath", 1, inifn);
}


void SavePathSettings(char *inifn, char *secName, CPathSettings *nspath){
	char vstr[32];

	WritePrivateProfileString(secName, "SRAM", nspath->szSramPath, inifn);
	WritePrivateProfileString(secName, "State", nspath->szStatePath, inifn);
	WritePrivateProfileString(secName, "Shot", nspath->szShotPath, inifn);
	WritePrivateProfileString(secName, "Wave", nspath->szWavePath , inifn);
	WritePrivateProfileString(secName, "Movie", nspath->szMoviePath, inifn);
	WritePrivateProfileString(secName, "Genie", nspath->szGameGeniePath, inifn);
	WritePrivateProfileString(secName, "NNNcht", nspath->szNNNcheatPath, inifn);
	WritePrivateProfileString(secName, "IPSPatch", nspath->szIPSPatchPath, inifn);
	WritePrivateProfileString(secName, "LastState", nspath->szLastStatePath, inifn);

	wsprintf(vstr, "%u", nspath->UseSramPath);
	WritePrivateProfileString(secName, "UseSramPath", vstr, inifn);
	wsprintf(vstr, "%u", nspath->UseStatePath);
	WritePrivateProfileString(secName, "UseStatePath", vstr, inifn);
	wsprintf(vstr, "%u", nspath->UseShotPath);
	WritePrivateProfileString(secName, "UseShotPath", vstr, inifn);
	wsprintf(vstr, "%u", nspath->UseWavePath);
	WritePrivateProfileString(secName, "UseWavePath", vstr, inifn);
	wsprintf(vstr, "%u", nspath->UseMoviePath);
	WritePrivateProfileString(secName, "UseMoviePath", vstr, inifn);
	wsprintf(vstr, "%u", nspath->UseGeniePath);
	WritePrivateProfileString(secName, "UseGeniePath", vstr, inifn);
	wsprintf(vstr, "%u", nspath->UseNNNchtPath);
	WritePrivateProfileString(secName, "UseNNNchtPath", vstr, inifn);
	wsprintf(vstr, "%u", nspath->UseIPSPath);
	WritePrivateProfileString(secName, "UseIPSPath", vstr, inifn);
}


void LoadNesMainSettings(char *inifn, char *secName, NES_preferences_settings *nesp){

	nesp->run_in_background = GetPrivateProfileInt(secName, "run_in_background", 0, inifn);
	nesp->SkipSomeErrors = GetPrivateProfileInt(secName, "SkipSomeErrors", 1, inifn);
	nesp->speed_throttling = GetPrivateProfileInt(secName, "speed_throttling", 1, inifn);
	nesp->auto_frameskip = GetPrivateProfileInt(secName, "auto_frameskip", 1, inifn);
	nesp->FastFPS = GetPrivateProfileInt(secName, "FastFPS", 120, inifn);
	nesp->ToggleFast = GetPrivateProfileInt(secName, "ToggleFast", 0, inifn);
	nesp->priority = (NES_preferences_settings::NES_PRIORITY)GetPrivateProfileInt(secName, "priority", 0, inifn);
	nesp->NotUseSleep = GetPrivateProfileInt(secName, "NotUseSleep", 0, inifn);
	nesp->NotUseFDSDiskASkip = GetPrivateProfileInt(secName, "NotUseFDSDiskASkip", 0, inifn);
	nesp->UseRomDataBase = GetPrivateProfileInt(secName, "UseRomDataBase", 0, inifn);
	nesp->AutoRomCorrect = GetPrivateProfileInt(secName, "AutoRomCorrect", 0, inifn);
	nesp->AutoStateSL = GetPrivateProfileInt(secName, "AutoStateSL", 0, inifn);
	nesp->AutoStateSLNotload = GetPrivateProfileInt(secName, "AutoStateSLNotload", 0, inifn);
	nesp->AllowMultiInstance = GetPrivateProfileInt(secName, "AllowMultiInstance", 0, inifn);
	nesp->ZipDLLUseFlag = GetPrivateProfileInt(secName, "ZipDLLUseFlag", 0, inifn);
	nesp->SaveFileToZip = GetPrivateProfileInt(secName, "SaveFileToZip", 0, inifn);
	nesp->StateFileToZip = GetPrivateProfileInt(secName, "StateFileToZip", 0, inifn);
	nesp->DisableIPSPatch= GetPrivateProfileInt(secName, "DisableIPSPatch", 0, inifn);
	nesp->DisableMenuIcon = GetPrivateProfileInt(secName, "DisableMenuIcon", 1, inifn);

	nesp->JoystickUseAPI = GetPrivateProfileInt(secName, "JoystickUseAPI", 0, inifn);
	nesp->StdSpeedState = GetPrivateProfileInt(secName, "StdSpeedState", 0, inifn);
	nesp->FastSpeedState = GetPrivateProfileInt(secName, "FastSpeedState", 0, inifn);
	nesp->StdFPS = GetPrivateProfileInt(secName, "StdFPS", 0, inifn);
	nesp->StdSkipFrame = GetPrivateProfileInt(secName, "StdSkipFrame", 0, inifn);
	nesp->FastSkipFrame = GetPrivateProfileInt(secName, "FastSkipFrame", 3, inifn);
	nesp->KailleraFrameInterval = GetPrivateProfileInt(secName, "KailleraFrameInterval", 1, inifn);
	nesp->TV_Mode = GetPrivateProfileInt(secName, "TV_Mode", 0, inifn);
	nesp->CloseButtonExit = GetPrivateProfileInt(secName, "CloseButtonExit", 0, inifn);
	nesp->TimeUsePCounter = GetPrivateProfileInt(secName, "TimeUsePCounter", 0, inifn);
	nesp->PreviewMode = GetPrivateProfileInt(secName, "PreviewMode", 0, inifn);
	nesp->JoystickUseGUID = GetPrivateProfileInt(secName, "JoystickUseGUID", 0, inifn);
	nesp->ShowFPS = GetPrivateProfileInt(secName, "ShowFPS", 0, inifn);
	nesp->DisableGameInfoDisp= GetPrivateProfileInt(secName, "DisableGameInfoDisp", 0, inifn);
	nesp->DoubleClickFullScreen= GetPrivateProfileInt(secName, "DoubleClickFullScreen", 0, inifn);

	nesp->GM_WindowStyle_Title= GetPrivateProfileInt(secName, "GM_WindowStyle_Title", 0, inifn);
	nesp->GM_WindowStyle_Menu= GetPrivateProfileInt(secName, "GM_WindowStyle_Menu", 0, inifn);
	nesp->GM_WindowStyle_SBar= GetPrivateProfileInt(secName, "GM_WindowStyle_SBar", 0, inifn);


	nesp->nViewerReInterval= GetPrivateProfileInt(secName, "nViewerReInterval", 2, inifn);
	nesp->nPtnViewerReInterval= GetPrivateProfileInt(secName, "nPtnViewerReInterval", 10, inifn);
}


void SaveNesMainSettings(char *inifn, char *secName, NES_preferences_settings *nesp){

	char vstr[32];

	wsprintf(vstr, "%u", nesp->run_in_background);
	WritePrivateProfileString(secName, "run_in_background", vstr, inifn);
	wsprintf(vstr, "%u", nesp->SkipSomeErrors);
	WritePrivateProfileString(secName, "SkipSomeErrors", vstr, inifn);
	wsprintf(vstr, "%u", nesp->speed_throttling);
	WritePrivateProfileString(secName, "speed_throttling", vstr, inifn);
	wsprintf(vstr, "%u", nesp->auto_frameskip);
	WritePrivateProfileString(secName, "auto_frameskip", vstr, inifn);
	wsprintf(vstr, "%u", nesp->FastFPS);
	WritePrivateProfileString(secName, "FastFPS", vstr, inifn);
	wsprintf(vstr, "%u", nesp->ToggleFast);
	WritePrivateProfileString(secName, "ToggleFast", vstr, inifn);
	wsprintf(vstr, "%u", nesp->priority);
	WritePrivateProfileString(secName, "priority", vstr, inifn);
	wsprintf(vstr, "%u", nesp->NotUseSleep);
	WritePrivateProfileString(secName, "NotUseSleep", vstr, inifn);
	wsprintf(vstr, "%u", nesp->NotUseFDSDiskASkip);
	WritePrivateProfileString(secName, "NotUseFDSDiskASkip", vstr, inifn);
	wsprintf(vstr, "%u", nesp->UseRomDataBase);
	WritePrivateProfileString(secName, "UseRomDataBase", vstr, inifn);
	wsprintf(vstr, "%u", nesp->AutoRomCorrect);
	WritePrivateProfileString(secName, "AutoRomCorrect", vstr, inifn);
//	wsprintf(vstr, "%u", nesp->AutoStateSL);
//	WritePrivateProfileString(secName, "AutoStateSL", vstr, inifn);
	wsprintf(vstr, "%u", nesp->AutoStateSLNotload);
	WritePrivateProfileString(secName, "AutoStateSLNotload", vstr, inifn);
	wsprintf(vstr, "%u", nesp->AllowMultiInstance);
	WritePrivateProfileString(secName, "AllowMultiInstance", vstr, inifn);
	wsprintf(vstr, "%u", nesp->ZipDLLUseFlag);
	WritePrivateProfileString(secName, "ZipDLLUseFlag", vstr, inifn);
	wsprintf(vstr, "%u", nesp->SaveFileToZip);
	WritePrivateProfileString(secName, "SaveFileToZip", vstr, inifn);
	wsprintf(vstr, "%u", nesp->StateFileToZip);
	WritePrivateProfileString(secName, "StateFileToZip", vstr, inifn);
	wsprintf(vstr, "%u", nesp->DisableIPSPatch);
	WritePrivateProfileString(secName, "DisableIPSPatch", vstr, inifn);
	wsprintf(vstr, "%u", nesp->DisableMenuIcon);
	WritePrivateProfileString(secName, "DisableMenuIcon", vstr, inifn);

	wsprintf(vstr, "%u", nesp->JoystickUseAPI);
	WritePrivateProfileString(secName, "JoystickUseAPI", vstr, inifn);
	wsprintf(vstr, "%u", nesp->StdSpeedState);
	WritePrivateProfileString(secName, "StdSpeedState", vstr, inifn);
	wsprintf(vstr, "%u", nesp->FastSpeedState);
	WritePrivateProfileString(secName, "FastSpeedState", vstr, inifn);
	wsprintf(vstr, "%u", nesp->StdFPS);
	WritePrivateProfileString(secName, "StdFPS", vstr, inifn);
	wsprintf(vstr, "%u", nesp->StdSkipFrame);
	WritePrivateProfileString(secName, "StdSkipFrame", vstr, inifn);
	wsprintf(vstr, "%u", nesp->FastSkipFrame);
	WritePrivateProfileString(secName, "FastSkipFrame", vstr, inifn);
	wsprintf(vstr, "%u", nesp->KailleraFrameInterval);
	WritePrivateProfileString(secName, "KailleraFrameInterval", vstr, inifn);
	wsprintf(vstr, "%u", nesp->CloseButtonExit);
	WritePrivateProfileString(secName, "CloseButtonExit", vstr, inifn);
	wsprintf(vstr, "%u", nesp->TimeUsePCounter);
	WritePrivateProfileString(secName, "TimeUsePCounter", vstr, inifn);
	wsprintf(vstr, "%u", nesp->JoystickUseGUID);
	WritePrivateProfileString(secName, "JoystickUseGUID", vstr, inifn);
	wsprintf(vstr, "%u", nesp->DisableGameInfoDisp);
	WritePrivateProfileString(secName, "DisableGameInfoDisp", vstr, inifn);
	wsprintf(vstr, "%u", nesp->DoubleClickFullScreen);
	WritePrivateProfileString(secName, "DoubleClickFullScreen", vstr, inifn);

	wsprintf(vstr, "%u", nesp->GM_WindowStyle_Title);
	WritePrivateProfileString(secName, "GM_WindowStyle_Title", vstr, inifn);
	wsprintf(vstr, "%u", nesp->GM_WindowStyle_Menu);
	WritePrivateProfileString(secName, "GM_WindowStyle_Menu", vstr, inifn);
	wsprintf(vstr, "%u", nesp->GM_WindowStyle_SBar);
	WritePrivateProfileString(secName, "GM_WindowStyle_SBar", vstr, inifn);

	wsprintf(vstr, "%u", nesp->nViewerReInterval);
	WritePrivateProfileString(secName, "nViewerReInterval", vstr, inifn);
	wsprintf(vstr, "%u", nesp->nPtnViewerReInterval);
	WritePrivateProfileString(secName, "nPtnViewerReInterval", vstr, inifn);
}



void LoadSoundSettings(char *inifn, char *secName, NES_sound_settings *nss){
	nss->enabled = GetPrivateProfileInt(secName, "enabled", 1, inifn);
	nss->sample_bits = GetPrivateProfileInt(secName, "sample_bits", 8, inifn);
	nss->sample_rate = GetPrivateProfileInt(secName, "sample_rate", 44100, inifn);
	nss->rectangle1_enabled = GetPrivateProfileInt(secName, "rectangle1_enabled", 1, inifn);
	nss->rectangle2_enabled = GetPrivateProfileInt(secName, "rectangle2_enabled", 1, inifn);
	nss->triangle_enabled = GetPrivateProfileInt(secName, "triangle_enabled", 1, inifn);
	nss->noise_enabled = GetPrivateProfileInt(secName, "noise_enabled", 1, inifn);
	nss->dpcm_enabled = GetPrivateProfileInt(secName, "dpcm_enabled", 1, inifn);
	nss->ext_enabled = GetPrivateProfileInt(secName, "ext_enabled", 1, inifn);

	nss->ideal_triangle_enabled = GetPrivateProfileInt(secName, "ideal_triangle_enabled", 0, inifn);
	nss->smooth_envelope_enabled = GetPrivateProfileInt(secName, "smooth_envelope_enabled", 0, inifn);
	nss->smooth_sweep_enabled = GetPrivateProfileInt(secName, "smooth_sweep_enabled", 0, inifn);
	nss->buffer_len = GetPrivateProfileInt(secName, "buffer_len", 3, inifn);
	nss->filter_type = (NES_sound_settings::filter_type_t)GetPrivateProfileInt(secName, "filter_type", 1, inifn);
	nss->lowpass_filter_f = GetPrivateProfileInt(secName, "lowpass_filter_f", 15000, inifn);

	*((DWORD *)&nss->rectangle1_volumed) = GetPrivateProfileInt(secName, "rectangle1_volumed", 0, inifn);
	*((DWORD *)&nss->rectangle2_volumed) = GetPrivateProfileInt(secName, "rectangle2_volumed", 0, inifn);
	*((DWORD *)&nss->triangle_volumed) = GetPrivateProfileInt(secName, "triangle_volumed", 0, inifn);
	*((DWORD *)&nss->noise_volumed) = GetPrivateProfileInt(secName, "noise_volumed", 0, inifn);
	*((DWORD *)&nss->dpcm_volumed) = GetPrivateProfileInt(secName, "dpcm_volumed", 0, inifn);
	*((DWORD *)&nss->ext_volumed) = GetPrivateProfileInt(secName, "ext_volumed", 0, inifn);
}


void SaveSoundSettings(char *inifn, char *secName, NES_sound_settings *nss){
	char vstr[32];

	wsprintf(vstr, "%u", nss->enabled);
	WritePrivateProfileString(secName, "enabled", vstr, inifn);
	wsprintf(vstr, "%u", nss->sample_bits);
	WritePrivateProfileString(secName, "sample_bits", vstr, inifn);
	wsprintf(vstr, "%u", nss->sample_rate);
	WritePrivateProfileString(secName, "sample_rate", vstr, inifn);
	wsprintf(vstr, "%u", nss->rectangle1_enabled);
	WritePrivateProfileString(secName, "rectangle1_enabled", vstr, inifn);
	wsprintf(vstr, "%u", nss->rectangle2_enabled);
	WritePrivateProfileString(secName, "rectangle2_enabled", vstr, inifn);

	wsprintf(vstr, "%u", nss->triangle_enabled);
	WritePrivateProfileString(secName, "triangle_enabled", vstr, inifn);
	wsprintf(vstr, "%u", nss->noise_enabled);
	WritePrivateProfileString(secName, "noise_enabled", vstr, inifn);
	wsprintf(vstr, "%u", nss->dpcm_enabled);
	WritePrivateProfileString(secName, "dpcm_enabled", vstr, inifn);
	wsprintf(vstr, "%u", nss->ext_enabled);
	WritePrivateProfileString(secName, "ext_enabled", vstr, inifn);
	wsprintf(vstr, "%u", nss->ideal_triangle_enabled);
	WritePrivateProfileString(secName, "ideal_triangle_enabled", vstr, inifn);
	wsprintf(vstr, "%u", nss->smooth_envelope_enabled);
	WritePrivateProfileString(secName, "smooth_envelope_enabled", vstr, inifn);
	wsprintf(vstr, "%u", nss->smooth_sweep_enabled);
	WritePrivateProfileString(secName, "smooth_sweep_enabled", vstr, inifn);
	wsprintf(vstr, "%u", nss->buffer_len);
	WritePrivateProfileString(secName, "buffer_len", vstr, inifn);
	wsprintf(vstr, "%u", nss->filter_type);
	WritePrivateProfileString(secName, "filter_type", vstr, inifn);
	wsprintf(vstr, "%u", nss->lowpass_filter_f);
	WritePrivateProfileString(secName, "lowpass_filter_f", vstr, inifn);

	wsprintf(vstr, "%u", *((DWORD *)&nss->rectangle1_volumed));
	WritePrivateProfileString(secName, "rectangle1_volumed", vstr, inifn);
	wsprintf(vstr, "%u", *((DWORD *)&nss->rectangle2_volumed));
	WritePrivateProfileString(secName, "rectangle2_volumed", vstr, inifn);
	wsprintf(vstr, "%u", *((DWORD *)&nss->triangle_volumed));
	WritePrivateProfileString(secName, "triangle_volumed", vstr, inifn);
	wsprintf(vstr, "%u", *((DWORD *)&nss->noise_volumed));
	WritePrivateProfileString(secName, "noise_volumed", vstr, inifn);
	wsprintf(vstr, "%u", *((DWORD *)&nss->dpcm_volumed));
	WritePrivateProfileString(secName, "dpcm_volumed", vstr, inifn);
	wsprintf(vstr, "%u", *((DWORD *)&nss->ext_volumed));
	WritePrivateProfileString(secName, "ext_volumed", vstr, inifn);
}


void LoadGraphics(char *inifn, char *secName, NES_graphics_settings *ngs){

	ngs->osd.InfoStatusBar = GetPrivateProfileInt(secName, "InfoStatusBar", 0, inifn);
	ngs->osd.WindowTopMost = GetPrivateProfileInt(secName, "WindowTopMost", 0, inifn);
	*((DWORD *)&ngs->osd.zoom_size) = GetPrivateProfileInt(secName, "zoom_size", 0x3f800000, inifn);
	ngs->black_and_white = GetPrivateProfileInt(secName, "black_and_white", 0, inifn);
	ngs->show_more_than_8_sprites = GetPrivateProfileInt(secName, "show_more_than_8_sprites", 0, inifn);
	ngs->show_all_scanlines = GetPrivateProfileInt(secName, "show_all_scanlines", 0, inifn);
	ngs->draw_overscan = GetPrivateProfileInt(secName, "draw_overscan", 0, inifn);
	ngs->fullscreen_on_load = GetPrivateProfileInt(secName, "fullscreen_on_load", 0, inifn);
	ngs->fullscreen_scaling = GetPrivateProfileInt(secName, "fullscreen_scaling", 0, inifn);
	ngs->calculate_palette = GetPrivateProfileInt(secName, "calculate_palette", 0, inifn);
	GetPrivateProfileString(secName, "szPaletteFile", "" , ngs->szPaletteFile , MAX_PATH, inifn);

	ngs->tint = GetPrivateProfileInt(secName, "tint", 0x86, inifn);
	ngs->hue = GetPrivateProfileInt(secName, "hue", 0x9d, inifn);
	ngs->osd.fullscreen_width = GetPrivateProfileInt(secName, "fullscreen_width", 320, inifn);
	ngs->osd.fullscreen_height = GetPrivateProfileInt(secName, "fullscreen_height", ngs->osd.fullscreen_width*3/4, inifn);
	ngs->osd.fullscreen_BitDepth= GetPrivateProfileInt(secName, "fullscreen_BitDepth", 8, inifn);
	{
		unsigned char p[sizeof(GUID)*2+1];
		unsigned char *gp= (unsigned char *)&ngs->osd.device_GUID;
		if(GetPrivateProfileString(secName, "device_GUID", "" , (char *)p, sizeof(p), inifn)){
			for(int i=0; i<sizeof(GUID); i++, gp++){
				*gp = Hexstrtouc((char *)&p[i*2]);
			}
		}
		else
			memset(gp, 0, sizeof(GUID));
	}
//	ngs->osd.device_GUID = GetPrivateProfileInt(secName, "device_GUID", 0, inifn);
	ngs->UseStretchBlt = GetPrivateProfileInt(secName, "UseStretchBlt", 0, inifn);
	ngs->EmulateTVScanline = GetPrivateProfileInt(secName, "EmulateTVScanline", 0, inifn);

	ngs->WindowModeUseDdraw = GetPrivateProfileInt(secName, "WindowModeUseDdraw", 0, inifn);
	ngs->WindowModeUseDdrawOverlay = GetPrivateProfileInt(secName, "WindowModeUseDdrawOverlay", 0, inifn);
	ngs->FullscreenModeUseNewMode = GetPrivateProfileInt(secName, "FullscreenModeUseNewMode", 0, inifn);
	ngs->FullscreenModeUseDefRefreshRate = GetPrivateProfileInt(secName, "FullscreenModeUseDefRefreshRate", 0, inifn);
	ngs->FullscreenModeBackBuffM = GetPrivateProfileInt(secName, "FullscreenModeBackBuffM", 0, inifn);
	ngs->WindowModeBackBuffM = GetPrivateProfileInt(secName, "WindowModeBackBuffM", 0, inifn);
	ngs->osd.DtvSize = GetPrivateProfileInt(secName, "DtvSize", 0, inifn);
	ngs->ScreenShotUseBmp = GetPrivateProfileInt(secName, "ScreenShotUseBmp", 0, inifn);
	ngs->FullscreenModeNotUseFlip = GetPrivateProfileInt(secName, "FullscreenModeNotUseFlip", 0, inifn);
	ngs->DisableSpriteClipping = GetPrivateProfileInt(secName, "DisableSpriteClipping", 0, inifn);
	ngs->DisableBackGClipping = GetPrivateProfileInt(secName, "DisableBackGClipping", 0, inifn);
	ngs->WindowDDMode_NotuseGDI= GetPrivateProfileInt(secName, "WindowDDMode_NotuseGDI", 0, inifn);
	ngs->WindowMode_2xsai	 = GetPrivateProfileInt(secName, "WindowMode_2xsai", 0, inifn);
	ngs->FullscreenMode_2xsai= GetPrivateProfileInt(secName, "FullscreenMode_2xsai", 0, inifn);
	ngs->WindowMode_2xsaiType	 = GetPrivateProfileInt(secName, "WindowMode_2xsaiType", 0, inifn);
	ngs->FullscreenMode_2xsaiType= GetPrivateProfileInt(secName, "FullscreenMode_2xsaiType", 0, inifn);

	ngs->osd.WindowPos_x = GetPrivateProfileInt(secName, "WindowPos_x", CW_USEDEFAULT, inifn);
	ngs->osd.WindowPos_y = GetPrivateProfileInt(secName, "WindowPos_y", CW_USEDEFAULT, inifn);
}


void SaveSettingsOnMenu(char *inifn, NES_settings *ness){
	const char *secName = "Graphics";
	char vstr[32];

	wsprintf(vstr, "%u", ness->graphics.osd.InfoStatusBar);
	WritePrivateProfileString(secName, "InfoStatusBar", vstr, inifn);
	wsprintf(vstr, "%u", ness->graphics.osd.WindowTopMost);
	WritePrivateProfileString(secName, "WindowTopMost", vstr, inifn);
	wsprintf(vstr, "%u", *((DWORD *)&ness->graphics.osd.zoom_size));
	WritePrivateProfileString(secName, "zoom_size", vstr, inifn);
	wsprintf(vstr, "%u", ness->graphics.osd.DtvSize);
	WritePrivateProfileString(secName, "DtvSize", vstr, inifn);
	wsprintf(vstr, "%u", ness->preferences.AutoStateSL);
	WritePrivateProfileString("NESMAIN", "AutoStateSL", vstr, inifn);
	wsprintf(vstr, "%u", ness->preferences.PreviewMode);
	WritePrivateProfileString("NESMAIN", "PreviewMode", vstr, inifn);
	wsprintf(vstr, "%u", ness->preferences.TV_Mode);
	WritePrivateProfileString("NESMAIN", "TV_Mode", vstr, inifn);
	wsprintf(vstr, "%u", ness->preferences.ShowFPS);
	WritePrivateProfileString("NESMAIN", "ShowFPS", vstr, inifn);

	wsprintf(vstr, "%u", ness->graphics.osd.WindowPos_x);
	WritePrivateProfileString(secName, "WindowPos_x", vstr, inifn);
	wsprintf(vstr, "%u", ness->graphics.osd.WindowPos_y);
	WritePrivateProfileString(secName, "WindowPos_y", vstr, inifn);
}


void SaveGraphics(char *inifn, char *secName, NES_graphics_settings *ngs){
	char vstr[32];
/*
	wsprintf(vstr, "%u", ngs->osd.InfoStatusBar);
	WritePrivateProfileString(secName, "InfoStatusBar", vstr, inifn);
	wsprintf(vstr, "%u", ngs->osd.WindowTopMost);
	WritePrivateProfileString(secName, "WindowTopMost", vstr, inifn);
	wsprintf(vstr, "%u", ngs->osd.double_size);
	WritePrivateProfileString(secName, "double_size", vstr, inifn);
	wsprintf(vstr, "%u", ngs->osd.DtvSize);
	WritePrivateProfileString(secName, "DtvSize", vstr, inifn);

*/

	wsprintf(vstr, "%u", ngs->black_and_white);
	WritePrivateProfileString(secName, "black_and_white", vstr, inifn);
	wsprintf(vstr, "%u", ngs->show_more_than_8_sprites);
	WritePrivateProfileString(secName, "show_more_than_8_sprites", vstr, inifn);
	wsprintf(vstr, "%u", ngs->show_all_scanlines);
	WritePrivateProfileString(secName, "show_all_scanlines", vstr, inifn);
	wsprintf(vstr, "%u", ngs->draw_overscan);
	WritePrivateProfileString(secName, "draw_overscan", vstr, inifn);
	wsprintf(vstr, "%u", ngs->fullscreen_on_load);
	WritePrivateProfileString(secName, "fullscreen_on_load", vstr, inifn);
	wsprintf(vstr, "%u", ngs->fullscreen_scaling);
	WritePrivateProfileString(secName, "fullscreen_scaling", vstr, inifn);
	wsprintf(vstr, "%u", ngs->calculate_palette);
	WritePrivateProfileString(secName, "calculate_palette", vstr, inifn);
	WritePrivateProfileString(secName, "szPaletteFile", ngs->szPaletteFile, inifn);
	wsprintf(vstr, "%u", ngs->tint);
	WritePrivateProfileString(secName, "tint", vstr, inifn);
	wsprintf(vstr, "%u", ngs->hue);
	WritePrivateProfileString(secName, "hue", vstr, inifn);
	wsprintf(vstr, "%u", ngs->osd.fullscreen_width);
	WritePrivateProfileString(secName, "fullscreen_width", vstr, inifn);
	wsprintf(vstr, "%u", ngs->osd.fullscreen_height);
	WritePrivateProfileString(secName, "fullscreen_height", vstr, inifn);
	wsprintf(vstr, "%u", ngs->osd.fullscreen_BitDepth);
	WritePrivateProfileString(secName, "fullscreen_BitDepth", vstr, inifn);
	{
		unsigned char p[sizeof(GUID)*2+1];
		unsigned char *gp= (unsigned char *)&ngs->osd.device_GUID;
		for(int i=0; i<sizeof(GUID); i++, gp++){
			wsprintf((char *)&p[i*2], "%02X", *gp);
		}
		WritePrivateProfileString(secName, "device_GUID", (const char *)p, inifn);
	}
//	wsprintf(vstr, "%u", ngs->osd.device_GUID);

	wsprintf(vstr, "%u", ngs->UseStretchBlt);
	WritePrivateProfileString(secName, "UseStretchBlt", vstr, inifn);
	wsprintf(vstr, "%u", ngs->EmulateTVScanline);
	WritePrivateProfileString(secName, "EmulateTVScanline", vstr, inifn);

	wsprintf(vstr, "%u", ngs->WindowModeUseDdraw);
	WritePrivateProfileString(secName, "WindowModeUseDdraw", vstr, inifn);
	wsprintf(vstr, "%u", ngs->WindowModeUseDdrawOverlay);
	WritePrivateProfileString(secName, "WindowModeUseDdrawOverlay", vstr, inifn);
	wsprintf(vstr, "%u", ngs->FullscreenModeUseNewMode);
	WritePrivateProfileString(secName, "FullscreenModeUseNewMode", vstr, inifn);
	wsprintf(vstr, "%u", ngs->FullscreenModeUseDefRefreshRate);
	WritePrivateProfileString(secName, "FullscreenModeUseDefRefreshRate", vstr, inifn);
	wsprintf(vstr, "%u", ngs->FullscreenModeBackBuffM);
	WritePrivateProfileString(secName, "FullscreenModeBackBuffM", vstr, inifn);
	wsprintf(vstr, "%u", ngs->WindowModeBackBuffM);
	WritePrivateProfileString(secName, "WindowModeBackBuffM", vstr, inifn);

	wsprintf(vstr, "%u", ngs->ScreenShotUseBmp);
	WritePrivateProfileString(secName, "ScreenShotUseBmp", vstr, inifn);
	wsprintf(vstr, "%u", ngs->FullscreenModeNotUseFlip);
	WritePrivateProfileString(secName, "FullscreenModeNotUseFlip", vstr, inifn);
	wsprintf(vstr, "%u", ngs->DisableSpriteClipping);
	WritePrivateProfileString(secName, "DisableSpriteClipping", vstr, inifn);
	wsprintf(vstr, "%u", ngs->DisableBackGClipping);
	WritePrivateProfileString(secName, "DisableBackGClipping", vstr, inifn);
	wsprintf(vstr, "%u", ngs->WindowDDMode_NotuseGDI);
	WritePrivateProfileString(secName, "WindowDDMode_NotuseGDI", vstr, inifn);
	wsprintf(vstr, "%u", ngs->FullscreenMode_2xsai);
	WritePrivateProfileString(secName, "FullscreenMode_2xsai", vstr, inifn);
	wsprintf(vstr, "%u", ngs->WindowMode_2xsai);
	WritePrivateProfileString(secName, "WindowMode_2xsai", vstr, inifn);
	wsprintf(vstr, "%u", ngs->FullscreenMode_2xsaiType);
	WritePrivateProfileString(secName, "FullscreenMode_2xsaiType", vstr, inifn);
	wsprintf(vstr, "%u", ngs->WindowMode_2xsaiType);
	WritePrivateProfileString(secName, "WindowMode_2xsaiType", vstr, inifn);
}




void LoadRecentFiles(char *inifn, char *secName, recent_list& rl){
	char buff[MAX_PATH], keyName[32];
	rl.clear();
	for(int i = 9; i >= 0; i--){
		wsprintf(keyName, "RECENT%i", i);
		buff[0] =0;
		if(GetPrivateProfileString(secName, keyName, "" ,buff , MAX_PATH, inifn))
			rl.add_entry(buff);
	}
}

void SaveRecentFiles(char *inifn, char *secName, const recent_list& rl){
	int max = rl.get_max_entries();
	char buff[MAX_PATH], keyName[32];
	for(int i = 0; i < max; i++){
		wsprintf(keyName, "RECENT%i", i);
		buff[0] =0;
		if(rl.get_entry(i)){
			strcpy(buff, rl.get_entry(i));
		}
		WritePrivateProfileString(secName, keyName, buff, inifn);
	}
}


void LoadButtonSettings_ini(char *inifn, char *secName, char *keyName, OSD_ButtonSettings* bsettings){
	unsigned char value[4];

	*((DWORD *)&value[0]) = GetPrivateProfileInt(secName, keyName, -1, inifn);
	if(*((long *)&value[0]) != -1){
		bsettings->type = (unsigned int)value[0];
		bsettings->dev_n= (unsigned int)value[1];
		bsettings->j_offset= (unsigned int)value[2];
		bsettings->j_axispositive= (unsigned int)value[3];
		if(!NESTER_settings.nes.preferences.JoystickUseAPI && NESTER_settings.nes.preferences.JoystickUseGUID){
			char vstr[32];
			strcpy(vstr, keyName);
			strcat(vstr, "_GUID");
			Ini_LoadBinary(inifn, secName, vstr, sizeof(GUID), (unsigned char *)&bsettings->DIJoyGUID);
		}
	}
}



void LoadOneControllerSettings(char *inifn, char *secName, NES_controller_input_settings *nescs){

	LoadButtonSettings_ini(inifn, secName, "UP", &nescs->btnUp);
	LoadButtonSettings_ini(inifn, secName, "DOUN", &nescs->btnDown);
	LoadButtonSettings_ini(inifn, secName, "LEFT", &nescs->btnLeft);
	LoadButtonSettings_ini(inifn, secName, "RIGHT", &nescs->btnRight);
	LoadButtonSettings_ini(inifn, secName, "SELECT", &nescs->btnSelect);
	LoadButtonSettings_ini(inifn, secName, "START", &nescs->btnStart);
	LoadButtonSettings_ini(inifn, secName, "B", &nescs->btnB);
	LoadButtonSettings_ini(inifn, secName, "A", &nescs->btnA);
	LoadButtonSettings_ini(inifn, secName, "TB", &nescs->btnTB);
	LoadButtonSettings_ini(inifn, secName, "TA", &nescs->btnTA);
	nescs->btnTBsec = GetPrivateProfileInt(secName, "TBSPD", 30, inifn);
	nescs->btnTAsec = GetPrivateProfileInt(secName, "TASPD", 30, inifn);
	nescs->AutoFireToggle= GetPrivateProfileInt(secName, "AutoFireToggle", 0, inifn);
}


void LoadAllControllerSettings(char *inifn,  NES_input_settings *nesis){
	char tstr[32];
	NES_controller_input_settings *ncs;
	for(int i=0; i<4; i++){
		wsprintf(tstr,"Controller %u", i+1);
		switch(i){
			case 0:
				ncs = &nesis->player1;
				break;
			case 1:
				ncs = &nesis->player2;
				break;
			case 2:
				ncs = &nesis->player3;
				break;
			case 3:
				ncs = &nesis->player4;
				break;
		}
		LoadOneControllerSettings(inifn, tstr, ncs);
	}
	LoadButtonSettings_ini(inifn, "ETCBTN", "MIC", &nesis->MicButton);
	for(i=0; i<EXTKEYALLNUM;++i){
		wsprintf(tstr, "EXTBTN%u", i);
		LoadButtonSettings_ini(inifn, "EXTBTN", tstr, &nesis->extkeycfg[i]);
	}
}

unsigned char Load_ini_Dll_Settings(class settings_t& settings){
	char inifn[MAX_PATH];
	GetModuleFileName(NULL, inifn, MAX_PATH);
	PathRenameExtension(inifn, ".ini");
	GetPrivateProfileString("RESOURCE", "FILENAME", "" , settings.ResourceDll, sizeof(settings.ResourceDll), inifn);
	return TRUE;
}


unsigned char Load_ini_Settings(class settings_t& settings){
	char inifn[MAX_PATH];
	GetModuleFileName(NULL, inifn, MAX_PATH);
	PathRenameExtension(inifn, ".ini");

	GetPrivateProfileString("etc", "OpenPath", "" , settings.OpenPath, sizeof(settings.OpenPath), inifn);

	settings.DirectSoundVolume = GetPrivateProfileInt("GENERAL", "DirectSoundVolume", 100, inifn);

	LoadPathSettings(inifn, "PATH", &settings.path);
	LoadRecentFiles(inifn, "RECENT", settings.recent_ROMs);
	LoadNesMainSettings(inifn, "NESMAIN", &settings.nes.preferences);
	LoadGraphics(inifn, "Graphics", &settings.nes.graphics);
	LoadSoundSettings(inifn, "SOUND", &settings.nes.sound);
	LoadCheatDlgSettings(inifn, "CheatDlgState");
	LoadAllControllerSettings(inifn, &settings.nes.input);
	LoadPreviewSettings(inifn, "Preview");
	LoadHoyKeyCfg();

	return TRUE;
}


void SaveButtonSettings_ini(char *inifn, char *secName, char *keyName, OSD_ButtonSettings* bsettings){
	char vstr[32];
	unsigned char value[4];
	value[0] = (unsigned char)bsettings->type;
	value[1] = (unsigned char)bsettings->dev_n;
	value[2] = (unsigned char)bsettings->j_offset;
	value[3] = (unsigned char)bsettings->j_axispositive;
	wsprintf(vstr, "%u", *((DWORD *)&value[0]));
	WritePrivateProfileString(secName, keyName, vstr, inifn);
	if(!NESTER_settings.nes.preferences.JoystickUseAPI && NESTER_settings.nes.preferences.JoystickUseGUID){
		strcpy(vstr, keyName);
		strcat(vstr, "_GUID");
		Ini_SaveBinary(inifn, secName, vstr, sizeof(GUID), (unsigned char *)&bsettings->DIJoyGUID);
	}
}


void SaveOneControllerSettings(char *inifn, char *secName, NES_controller_input_settings *nescs){

	SaveButtonSettings_ini(inifn, secName, "UP", &nescs->btnUp);
	SaveButtonSettings_ini(inifn, secName, "DOUN", &nescs->btnDown);
	SaveButtonSettings_ini(inifn, secName, "LEFT", &nescs->btnLeft);
	SaveButtonSettings_ini(inifn, secName, "RIGHT", &nescs->btnRight);
	SaveButtonSettings_ini(inifn, secName, "SELECT", &nescs->btnSelect);
	SaveButtonSettings_ini(inifn, secName, "START", &nescs->btnStart);
	SaveButtonSettings_ini(inifn, secName, "B", &nescs->btnB);
	SaveButtonSettings_ini(inifn, secName, "A", &nescs->btnA);
	SaveButtonSettings_ini(inifn, secName, "TB", &nescs->btnTB);
	SaveButtonSettings_ini(inifn, secName, "TA", &nescs->btnTA);
	{
		char vstr[32];
		wsprintf(vstr, "%u", nescs->btnTBsec);
		WritePrivateProfileString(secName, "TBSPD", vstr, inifn);
		wsprintf(vstr, "%u", nescs->btnTAsec);
		WritePrivateProfileString(secName, "TASPD", vstr, inifn);
		wsprintf(vstr, "%u", nescs->AutoFireToggle);
		WritePrivateProfileString(secName, "AutoFireToggle", vstr, inifn);
	}
}


void SaveAllControllerSettings(char *inifn, NES_input_settings *nesis){
	NES_controller_input_settings *ncs;
	char tstr[64];
	for(int i=0; i<4; i++){
		wsprintf(tstr,"Controller %u", i+1);
		switch(i){
			case 0:
				ncs = &nesis->player1;
				break;
			case 1:
				ncs = &nesis->player2;
				break;
			case 2:
				ncs = &nesis->player3;
				break;
			case 3:
				ncs = &nesis->player4;
				break;
		}
		SaveOneControllerSettings(inifn, tstr, ncs);
	}
	SaveButtonSettings_ini(inifn, "ETCBTN", "MIC", &nesis->MicButton);
	for(i=0; i<EXTKEYALLNUM;++i){
		wsprintf(tstr, "EXTBTN%u", i);
		SaveButtonSettings_ini(inifn, "EXTBTN", tstr, &nesis->extkeycfg[i]);
	}
}



unsigned char Save_ini_Settings(class settings_t& settings){
	char inifn[MAX_PATH], vstr[64];
	GetModuleFileName(NULL, inifn, MAX_PATH);
	PathRenameExtension(inifn, ".ini");

//	wsprintf(tstr,"%d", );
	WritePrivateProfileString("VERSION", "Ver", "0.22", inifn);
	WritePrivateProfileString("etc", "OpenPath", settings.OpenPath, inifn);
	WritePrivateProfileString("RESOURCE", "FILENAME", settings.ResourceDll, inifn);

	wsprintf(vstr, "%u", settings.DirectSoundVolume);
	WritePrivateProfileString("GENERAL", "DirectSoundVolume", vstr, inifn);

	SavePathSettings(inifn, "PATH", &settings.path);
	SaveRecentFiles(inifn, "RECENT", settings.recent_ROMs);
//	SaveNesMainSettings(inifn, "NESMAIN", &settings.nes.preferences);
//	SaveGraphics(inifn, "Graphics", &settings.nes.graphics);
//	SaveSoundSettings(inifn, "SOUND", &settings.nes.sound);
//	SaveAllControllerSettings(inifn, &settings.nes.input);
	SaveCheatDlgSettings(inifn, "CheatDlgState");
	SaveSettingsOnMenu(inifn, &settings.nes);

	SavePreviewSettings(inifn, "Preview");
	return TRUE;
}



void SaveHoyKeyCfg(){
	char inifn[MAX_PATH], vstr[64], keyname[64], *sname = "HOTKEY";
	int i;
	GetModuleFileName(NULL, inifn, MAX_PATH);
	PathRenameExtension(inifn, ".ini");


	for(i=0; i<1; i++){
		wsprintf(keyname, "HotKey_%02u", i);
		wsprintf(vstr, "%u", g_Window_ShotrCutKey[i].KeyConfig);
		WritePrivateProfileString(sname, keyname, vstr, inifn);

		wsprintf(keyname, "HKActive_%02u", i);
		wsprintf(vstr, "%u", g_Window_ShotrCutKey[i].Active);
		WritePrivateProfileString(sname, keyname, vstr, inifn);
	}

}


void LoadHoyKeyCfg(){
	char inifn[MAX_PATH], vstr[64], keyname[64], *sname = "HOTKEY";
	int i;
	GetModuleFileName(NULL, inifn, MAX_PATH);
	PathRenameExtension(inifn, ".ini");

	for(i=0; i<1; i++){
		wsprintf(keyname, "HotKey_%02u", i);
		g_Window_ShotrCutKey[i].KeyConfig = GetPrivateProfileInt(sname, keyname, 0, inifn);
		wsprintf(keyname, "HKActive_%02u", i);
		g_Window_ShotrCutKey[i].Active = GetPrivateProfileInt(sname, keyname, 0, inifn);
	}
}


