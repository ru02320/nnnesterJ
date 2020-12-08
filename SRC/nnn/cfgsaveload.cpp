



#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <winreg.h>

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
#include "myconfig.h"

#define SOFTWARE_NAME "NNNesterJ"



extern HINSTANCE g_main_instance;
extern struct cheat_info_struct g_Cheat_Info;
extern struct MyWindowShortCutKey g_Window_ShotrCutKey[1];

extern struct Preview_state g_Preview_State;


static int UseReg;
static int ReadReg = 0;


void SetUseRegistry(int use){
	HKEY sw_key;
	HKEY sn_key;
	DWORD dw, val = use;

	if(RegCreateKeyEx(HKEY_CURRENT_USER, "Software", 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &sw_key, &dw) != ERROR_SUCCESS){
		return;
	}
	if(RegCreateKeyEx(sw_key, SOFTWARE_NAME, 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &sn_key, &dw) != ERROR_SUCCESS){
		RegCloseKey(sw_key);
		return;
	}
	RegSetValueEx(sn_key, "UseRegistry", 0, REG_DWORD, (CONST BYTE*)&val, 4);
	RegCloseKey(sn_key);
	RegCloseKey(sw_key);
	UseReg = use;
//	if(use)
	{
//		SavePathSettings(SaveConfigBase *pSave, char *secName, CPathSettings *nspath);
		SaveNesMainSettings(&NESTER_settings.nes.preferences);
		SaveSoundSettings(&NESTER_settings.nes.sound);
		SaveGraphics(&NESTER_settings.nes.graphics);
		SaveAllControllerSettings(&NESTER_settings.nes.input);
		SaveHoyKeyCfg();
	}
}


int CheckUseRegistry(){
	DWORD dw;
	DWORD dwType = REG_DWORD;
	DWORD dwByte = 4;
	HKEY sw_key;
	HKEY sn_key;
	int use = 0;

	if(ReadReg)
		return UseReg;

	if(RegCreateKeyEx(HKEY_CURRENT_USER, "Software", 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &sw_key, &dw) != ERROR_SUCCESS){
		return 0;
	}
	if(RegCreateKeyEx(sw_key, SOFTWARE_NAME, 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &sn_key, &dw) != ERROR_SUCCESS){
		RegCloseKey(sw_key);
		return 0;
	}
	if(dw == REG_CREATED_NEW_KEY){
		RegCloseKey(sn_key);
		RegDeleteKey(sw_key, SOFTWARE_NAME);
		RegCloseKey(sw_key);
		return 0;
	}
	RegQueryValueEx(sn_key, "UseRegistry", NULL, &dwType, (BYTE *)&use, &dwByte);
	RegCloseKey(sw_key);
	RegCloseKey(sn_key);
	ReadReg = 1;
	UseReg = use;
	return use;
}


LoadConfigBase *CreateMyConfigLoad(){
	LoadConfigBase *tmp = NULL;

	try{
		if(CheckUseRegistry()){
			tmp = new LoadConfigWinReg(SOFTWARE_NAME);
		}
		else{
			char inifn[MAX_PATH];
			GetModuleFileName(NULL, inifn, MAX_PATH);
			PathRenameExtension(inifn, ".ini");
			tmp = new LoadConfigWinIni(inifn);
		}
	}catch(...){
		throw;
	}
	return tmp;
}


SaveConfigBase *CreateMyConfigSave(){
	SaveConfigBase *tmp = NULL;

	try{
		if(CheckUseRegistry()){
			tmp = new SaveConfigWinReg(SOFTWARE_NAME);
		}
		else{
			char inifn[MAX_PATH];
			GetModuleFileName(NULL, inifn, MAX_PATH);
			PathRenameExtension(inifn, ".ini");
			tmp = new SaveConfigWinIni(inifn);
		}
	}catch(...){
		throw;
	}
	return tmp;
}


void LoadPreviewSettings(LoadConfigBase *pLoad, char *secName){
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
		g_Preview_State.Column_w[i] = pLoad->ReadInt(secName, kstr, 80);
		wsprintf(kstr, "PreviewColumn_Pos%u", i);
		g_Preview_State.Column_Pos[i] = pLoad->ReadInt(secName, kstr, 80);
	}
	{
		char str[MAX_PATH];
		GetModuleFileName(NULL, str, MAX_PATH);
		for(i=0; i<3; i++){ // database filename
			PathRemoveFileSpec(str);
			PathCombine(str , str, databasefn[i]);
			wsprintf(kstr, "Database_%u", i);
			pLoad->ReadString(secName, kstr, g_Preview_State.DATABASE_FileName[i], MAX_PATH, str);
		}
	}

	g_Preview_State.window_w = pLoad->ReadInt(secName, "Window_w", 640);
	g_Preview_State.window_h = pLoad->ReadInt(secName, "Window_h", 400);

	g_Preview_State.NotLoadNESJ_DB = pLoad->ReadInt(secName, "NotLoadNESJ_DB", 0);
	g_Preview_State.NotLoadNESE_DB = pLoad->ReadInt(secName, "NotLoadNESE_DB", 1);
	g_Preview_State.NotLoadFDS_DB = pLoad->ReadInt(secName, "NotLoadFDS_DB", 0);
	g_Preview_State.NotApplyNES_FixDB = pLoad->ReadInt(secName, "NotApplyNES_FixDB", 0);
	g_Preview_State.NotApplyFDS_FixDB = pLoad->ReadInt(secName, "NotApplyFDS_FixDB", 0);
	g_Preview_State.NotLoadNES_AddDB = pLoad->ReadInt(secName, "NotLoadNES_AddDB", 0);
	g_Preview_State.NotLoadFDS_AddDB = pLoad->ReadInt(secName, "NotLoadFDS_AddDB", 0);
	g_Preview_State.DisableSound = pLoad->ReadInt(secName, "DisableSound", 0);
	g_Preview_State.EmuSpeed = pLoad->ReadInt(secName, "EmuSpeed", 0);
	g_Preview_State.NESROM_CRCFrom_Filesize = pLoad->ReadInt(secName, "NESROM_CRCFrom_Filesize", 0);

	g_Preview_State.nTree_Width = pLoad->ReadInt(secName, "nTree_w", 64);
	g_Preview_State.TreeDispFlag = pLoad->ReadInt(secName, "TreeDispFlag", 1);
	g_Preview_State.ScreenDispFlag = pLoad->ReadInt(secName, "ScreenDispFlag", 1);
	g_Preview_State.GameTitleDetailDisplay = pLoad->ReadInt(secName, "GameTitleDetailDisplay", 0);

	g_Preview_State.BGColor	= pLoad->ReadInt(secName, "BGColor", 0xffffff);
	g_Preview_State.FontColor = pLoad->ReadInt(secName, "FontColor", 0);
	g_Preview_State.ToolTipTXTColor = pLoad->ReadInt(secName, "ToolTipTXTColor", 0);
	g_Preview_State.ToolTipBGColor = pLoad->ReadInt(secName, "ToolTipBGColor", 0xffffff);


	pLoad->ReadHexString(secName, "LogFont", (unsigned char *)&g_Preview_State.LogFont, sizeof(LOGFONT));

	g_Preview_State.nROMDirectory = pLoad->ReadInt(secName, "nROMDirectory", 0);

	for(i=0;i<g_Preview_State.nROMDirectory;i++){
		wsprintf(kstr, "ROM_Dir_%u", i);
		pLoad->ReadString(secName, kstr, g_Preview_State.ROM_Directory[i], MAX_PATH, "");
		wsprintf(kstr, "ROM_DirSF_%u", i);
		g_Preview_State.ROM_SubDirectory[i] = pLoad->ReadInt(secName, kstr, 0);
	}
}


void SavePreviewSettings(SaveConfigBase *pSave, char *secName){
	char kstr[32];
	int i;

	for(i=0; i<COLUMN_NUM; i++){
		wsprintf(kstr, "PreviewColumn_w%u", i);
		pSave->WriteInt(secName, kstr, g_Preview_State.Column_w[i]);
		wsprintf(kstr, "PreviewColumn_Pos%u", i);
		pSave->WriteInt(secName, kstr, g_Preview_State.Column_Pos[i]);
	}
	pSave->WriteInt(secName, "Window_w", g_Preview_State.window_w);
	pSave->WriteInt(secName, "Window_h", g_Preview_State.window_h);
	pSave->WriteInt(secName, "nROMDirectory", g_Preview_State.nROMDirectory);

	for(i=0;i<g_Preview_State.nROMDirectory;i++){
		wsprintf(kstr, "ROM_Dir_%u", i);
		pSave->WriteString(secName, kstr, g_Preview_State.ROM_Directory[i]);
		wsprintf(kstr, "ROM_DirSF_%u", i);
		pSave->WriteInt(secName, kstr, g_Preview_State.ROM_SubDirectory[i]);
	}
	for(i=0;i<3;i++){ // Database Filename
		wsprintf(kstr, "Database_%u", i);
		pSave->WriteString(secName, kstr, g_Preview_State.DATABASE_FileName[i]);
	}
	pSave->WriteInt(secName, "nTree_w", g_Preview_State.nTree_Width);
	pSave->WriteInt(secName, "TreeDispFlag", g_Preview_State.TreeDispFlag);
	pSave->WriteInt(secName, "ScreenDispFlag", g_Preview_State.ScreenDispFlag);
	pSave->WriteInt(secName, "GameTitleDetailDisplay", g_Preview_State.GameTitleDetailDisplay);
	pSave->WriteInt(secName, "NotLoadNESE_DB", g_Preview_State.NotLoadNESE_DB);
	pSave->WriteInt(secName, "NotLoadNESJ_DB", g_Preview_State.NotLoadNESJ_DB);
	pSave->WriteInt(secName, "NotLoadFDS_DB", g_Preview_State.NotLoadFDS_DB);
	pSave->WriteInt(secName, "NotApplyNES_FixDB", g_Preview_State.NotApplyNES_FixDB);
	pSave->WriteInt(secName, "NotApplyFDS_FixDB", g_Preview_State.NotApplyFDS_FixDB);
	pSave->WriteInt(secName, "NotLoadNES_AddDB", g_Preview_State.NotLoadNES_AddDB);
	pSave->WriteInt(secName, "NotLoadFDS_AddDB", g_Preview_State.NotLoadFDS_AddDB);

	pSave->WriteInt(secName, "DisableSound", g_Preview_State.DisableSound);
	pSave->WriteInt(secName, "EmuSpeed", g_Preview_State.EmuSpeed);
	pSave->WriteInt(secName, "NESROM_CRCFrom_Filesize", g_Preview_State.NESROM_CRCFrom_Filesize);

//	wsprintf(vstr, "%u", g_Preview_State.BGColor);
//	WritePrivateProfileString(secName, "BGColor", vstr, inifn);

	pSave->WriteInt(secName, "FontColor", g_Preview_State.FontColor);
	pSave->WriteHexString(secName, "LogFont", (unsigned char *)&g_Preview_State.LogFont, sizeof(LOGFONT));

	pSave->WriteInt(secName, "ToolTipTXTColor", g_Preview_State.ToolTipTXTColor);
	pSave->WriteInt(secName, "ToolTipBGColor", g_Preview_State.ToolTipBGColor);
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


void LoadCheatDlgSettings(LoadConfigBase *pLoad, char *secName){

	g_Cheat_Info.DisableCheat = pLoad->ReadInt(secName, "DisableCheat", 0);
	g_Cheat_Info.ctcmpdtf1 = pLoad->ReadInt(secName, "ctcmpdtf1", 0);
	g_Cheat_Info.ctcmpdtf2 = pLoad->ReadInt(secName, "ctcmpdtf2", 0);
	g_Cheat_Info.ctcmpdtf3 = pLoad->ReadInt(secName, "ctcmpdtf3", 0);
	g_Cheat_Info.ctcmpdtf4 = pLoad->ReadInt(secName, "ctcmpdtf4", 0);
	g_Cheat_Info.ctcmpdtf5 = pLoad->ReadInt(secName, "ctcmpdtf5", 0);
	g_Cheat_Info.ctcmpdtf6 = pLoad->ReadInt(secName, "ctcmpdtf6", 0);
	g_Cheat_Info.cc_clistedit1 = pLoad->ReadInt(secName, "cc_clistedit1", 0);
	g_Cheat_Info.cc_clistedit2 = pLoad->ReadInt(secName, "cc_clistedit2", 0);
	g_Cheat_Info.cc_csrchrb1 = pLoad->ReadInt(secName, "cc_csrchrb1", 0);
	g_Cheat_Info.cc_csrchrb2 = pLoad->ReadInt(secName, "cc_csrchrb2", 0);
	g_Cheat_Info.cc_csrchrb3 = pLoad->ReadInt(secName, "cc_csrchrb3", 0);

	CheatDlgFontColor = pLoad->ReadInt(secName, "FontColor", 0);
	
	pLoad->ReadHexString(secName, "LogFont", (unsigned char *)&CheatDlgFont, sizeof(LOGFONT));
	{
		unsigned char p[7*4+1];
		int i;

		memset(&g_Cheat_Info.cc_listorder, 0, sizeof(int)*7);
		memset(&g_Cheat_Info.cc_listwidth, 0, sizeof(int)*7);
		if(pLoad->ReadString(secName, "CC_List_Order", (char *)p, 7*2+1, "")){
			if(p[0]!=0){
				for(i=0; i<7; i++){
					g_Cheat_Info.cc_listorder[i] = Hexstrtouc((char *)&p[i*2]);
				}
			}
		}
		memset(p, 0, sizeof(p));
		if(pLoad->ReadHexString(secName, "CC_List_Width", p, 7*2)){
			if(p[0]!=0){
				for(i=0; i<7; i++){
					g_Cheat_Info.cc_listwidth[i] = p[i*2];
					g_Cheat_Info.cc_listwidth[i] |= p[i*2+1]<<8;
				}
			}
		}
	}
}


void SaveCheatDlgSettings(SaveConfigBase *pSave, char *secName){

	pSave->WriteInt(secName, "DisableCheat", g_Cheat_Info.DisableCheat);

	pSave->WriteInt(secName, "ctcmpdtf1", g_Cheat_Info.ctcmpdtf1);
	pSave->WriteInt(secName, "ctcmpdtf2", g_Cheat_Info.ctcmpdtf2);
	pSave->WriteInt(secName, "ctcmpdtf3", g_Cheat_Info.ctcmpdtf3);
	pSave->WriteInt(secName, "ctcmpdtf4", g_Cheat_Info.ctcmpdtf4);
	pSave->WriteInt(secName, "ctcmpdtf5", g_Cheat_Info.ctcmpdtf5);
	pSave->WriteInt(secName, "ctcmpdtf6", g_Cheat_Info.ctcmpdtf6);
	pSave->WriteInt(secName, "cc_clistedit1", g_Cheat_Info.cc_clistedit1);
	pSave->WriteInt(secName, "cc_clistedit2", g_Cheat_Info.cc_clistedit2);
	pSave->WriteInt(secName, "cc_csrchrb1", g_Cheat_Info.cc_csrchrb1);
	pSave->WriteInt(secName, "cc_csrchrb2", g_Cheat_Info.cc_csrchrb2);
	pSave->WriteInt(secName, "cc_csrchrb3", g_Cheat_Info.cc_csrchrb3);
	pSave->WriteInt(secName, "FontColor", CheatDlgFontColor);

	pSave->WriteHexString(secName, "LogFont", (unsigned char *)&CheatDlgFont, sizeof(LOGFONT));

	{
		int i;
		unsigned char p[7*4+1];
		short *ps = (short *)p;

		for(i=0; i<7; i++){
			p[i] = (unsigned char )g_Cheat_Info.cc_listorder[i];
		}
		pSave->WriteHexString(secName, "CC_List_Order", (unsigned char *)p, 7);
		for(i=0; i<7; i++){
			ps[i] = (short)g_Cheat_Info.cc_listwidth[i];
		}
		pSave->WriteHexString(secName, "CC_List_Width", (unsigned char *)p, 7*2);
	}
}



void LoadPathSettings(LoadConfigBase *pLoad, char *secName, CPathSettings *nspath){
	char str[MAX_PATH], *p;
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);

	for(p=str; *p; p++);
	strcpy(p, "\\save\\");

	pLoad->ReadString(secName, "SRAM", nspath->szSramPath, MAX_PATH, str);
	strcpy(p, "\\state\\");
	pLoad->ReadString(secName, "State", nspath->szStatePath, MAX_PATH, str);
	strcpy(p, "\\shot\\");
	pLoad->ReadString(secName, "Shot", nspath->szShotPath, MAX_PATH, str);
	strcpy(p, "\\wave\\");
	pLoad->ReadString(secName, "Wave", nspath->szWavePath, MAX_PATH, str);
	strcpy(p, "\\movie\\");
	pLoad->ReadString(secName, "Movie", nspath->szMoviePath, MAX_PATH, str);
	strcpy(p, "\\genie\\");
	pLoad->ReadString(secName, "Genie", nspath->szGameGeniePath, MAX_PATH, str);
	strcpy(p, "\\cheat\\");
	pLoad->ReadString(secName, "NNNcht", nspath->szNNNcheatPath, MAX_PATH, str);
	strcpy(p, "\\ips\\");
	pLoad->ReadString(secName, "IPSPatch", nspath->szIPSPatchPath, MAX_PATH, str);
//	strcpy(p, "\\\\");
	pLoad->ReadString(secName, "LastState", nspath->szLastStatePath, MAX_PATH, str);

	nspath->UseSramPath = pLoad->ReadInt(secName, "UseSramPath", 1);
	nspath->UseStatePath = pLoad->ReadInt(secName, "UseStatePath", 1);
	nspath->UseShotPath = pLoad->ReadInt(secName, "UseShotPath", 1);
	nspath->UseWavePath = pLoad->ReadInt(secName, "UseWavePath", 1);
	nspath->UseMoviePath = pLoad->ReadInt(secName, "UseMoviePath", 1);
	nspath->UseGeniePath = pLoad->ReadInt(secName, "UseGeniePath", 1);
	nspath->UseNNNchtPath = pLoad->ReadInt(secName, "UseNNNchtPath", 1);
	nspath->UseIPSPath = pLoad->ReadInt(secName, "UseIPSPath", 1);
}


void SavePathSettings(SaveConfigBase *pSave, char *secName, CPathSettings *nspath){

	pSave->WriteString(secName, "SRAM",	nspath->szSramPath);
	pSave->WriteString(secName, "State",nspath->szStatePath);
	pSave->WriteString(secName, "Shot", nspath->szShotPath);
	pSave->WriteString(secName, "Wave", nspath->szWavePath);
	pSave->WriteString(secName, "Movie",nspath->szMoviePath);
	pSave->WriteString(secName, "Genie",nspath->szGameGeniePath);
	pSave->WriteString(secName, "NNNcht", nspath->szNNNcheatPath);
	pSave->WriteString(secName, "IPSPatch", nspath->szIPSPatchPath);
	pSave->WriteString(secName, "LastState", nspath->szLastStatePath);

	pSave->WriteInt(secName, "UseSramPath", nspath->UseSramPath);
	pSave->WriteInt(secName, "UseStatePath", nspath->UseStatePath);
	pSave->WriteInt(secName, "UseShotPath", nspath->UseShotPath);
	pSave->WriteInt(secName, "UseWavePath", nspath->UseWavePath);
	pSave->WriteInt(secName, "UseMoviePath", nspath->UseMoviePath);
	pSave->WriteInt(secName, "UseGeniePath", nspath->UseGeniePath);
	pSave->WriteInt(secName, "UseNNNchtPath", nspath->UseNNNchtPath);
	pSave->WriteInt(secName, "UseIPSPath", nspath->UseIPSPath);

}


void LoadNesMainSettings(LoadConfigBase *pLoad, char *secName, NES_preferences_settings *nesp){

	nesp->run_in_background = pLoad->ReadInt(secName, "run_in_background", 0);
	nesp->SkipSomeErrors = pLoad->ReadInt(secName, "SkipSomeErrors", 1);
	nesp->speed_throttling = pLoad->ReadInt(secName, "speed_throttling", 1);
	nesp->auto_frameskip = pLoad->ReadInt(secName, "auto_frameskip", 1);
	nesp->FastFPS = pLoad->ReadInt(secName, "FastFPS", 120);
	nesp->ToggleFast = pLoad->ReadInt(secName, "ToggleFast", 0);
	nesp->priority = (NES_preferences_settings::NES_PRIORITY)pLoad->ReadInt(secName, "priority", 0);
	nesp->NotUseSleep = pLoad->ReadInt(secName, "NotUseSleep", 0);
	nesp->NotUseFDSDiskASkip = pLoad->ReadInt(secName, "NotUseFDSDiskASkip", 0);
	nesp->UseRomDataBase = pLoad->ReadInt(secName, "UseRomDataBase", 0);
	nesp->AutoRomCorrect = pLoad->ReadInt(secName, "AutoRomCorrect", 0);
	nesp->AutoStateSL = pLoad->ReadInt(secName, "AutoStateSL", 0);
	nesp->AutoStateSLNotload = pLoad->ReadInt(secName, "AutoStateSLNotload", 0);
	nesp->AllowMultiInstance = pLoad->ReadInt(secName, "AllowMultiInstance", 0);
	nesp->ZipDLLUseFlag = pLoad->ReadInt(secName, "ZipDLLUseFlag", 0);
	nesp->SaveFileToZip = pLoad->ReadInt(secName, "SaveFileToZip", 0);
	nesp->StateFileToZip = pLoad->ReadInt(secName, "StateFileToZip", 0);
	nesp->DisableIPSPatch = pLoad->ReadInt(secName, "DisableIPSPatch", 0);
	nesp->DisableMenuIcon = pLoad->ReadInt(secName, "DisableMenuIcon", 1);
	nesp->UseRewind = pLoad->ReadInt(secName, "UseRewind", 0);
	nesp->RewindBFrame = pLoad->ReadInt(secName, "RewindBFrame", 30);
	nesp->RewindKeyFrame = pLoad->ReadInt(secName, "RewindKeyFrame", 6);

	nesp->JoystickUseAPI = pLoad->ReadInt(secName, "JoystickUseAPI", 0);
	nesp->StdSpeedState = pLoad->ReadInt(secName, "StdSpeedState", 0);
	nesp->FastSpeedState = pLoad->ReadInt(secName, "FastSpeedState", 0);
	nesp->StdFPS = pLoad->ReadInt(secName, "StdFPS", 0);
	nesp->StdSkipFrame = pLoad->ReadInt(secName, "StdSkipFrame", 0);
	nesp->FastSkipFrame = pLoad->ReadInt(secName, "FastSkipFrame", 3);
//	nesp->KailleraFrameInterval = pLoad->ReadInt(secName, "KailleraFrameInterval", 1);
	nesp->TV_Mode = pLoad->ReadInt(secName, "TV_Mode", 0);
	nesp->CloseButtonExit = pLoad->ReadInt(secName, "CloseButtonExit", 0);
	nesp->TimeUsePCounter = pLoad->ReadInt(secName, "TimeUsePCounter", 0);
	nesp->PreviewMode = pLoad->ReadInt(secName, "PreviewMode", 0);
	nesp->JoystickUseGUID = pLoad->ReadInt(secName, "JoystickUseGUID", 0);
	nesp->ShowFPS = pLoad->ReadInt(secName, "ShowFPS", 0);
	nesp->DisableGameInfoDisp = pLoad->ReadInt(secName, "DisableGameInfoDisp", 0);
	nesp->DoubleClickFullScreen = pLoad->ReadInt(secName, "DoubleClickFullScreen", 0);

	nesp->GM_WindowStyle_Title = pLoad->ReadInt(secName, "GM_WindowStyle_Title", 0);
	nesp->GM_WindowStyle_Menu = pLoad->ReadInt(secName, "GM_WindowStyle_Menu", 0);
	nesp->GM_WindowStyle_SBar = pLoad->ReadInt(secName, "GM_WindowStyle_SBar", 0);

	nesp->nViewerReInterval = pLoad->ReadInt(secName, "nViewerReInterval", 2);
	nesp->nPtnViewerReInterval = pLoad->ReadInt(secName, "nPtnViewerReInterval", 10);
}


void SaveNesMainSettings(NES_preferences_settings *nesp){
	char *secName = "NESMAIN";
	SaveConfigBase *pSave=NULL;

	try{
		pSave = CreateMyConfigSave();

		pSave->WriteInt(secName, "run_in_background", nesp->run_in_background);
		pSave->WriteInt(secName, "SkipSomeErrors", nesp->SkipSomeErrors);
		pSave->WriteInt(secName, "speed_throttling", nesp->speed_throttling);
		pSave->WriteInt(secName, "auto_frameskip", nesp->auto_frameskip);
		pSave->WriteInt(secName, "FastFPS", nesp->FastFPS);
		pSave->WriteInt(secName, "ToggleFast", nesp->ToggleFast);
		pSave->WriteInt(secName, "priority", nesp->priority);
		pSave->WriteInt(secName, "NotUseSleep", nesp->NotUseSleep);
		pSave->WriteInt(secName, "NotUseFDSDiskASkip", nesp->NotUseFDSDiskASkip);
		pSave->WriteInt(secName, "UseRomDataBase", nesp->UseRomDataBase);
		pSave->WriteInt(secName, "AutoRomCorrect", nesp->AutoRomCorrect);
//		pSave->WriteInt(secName, "AutoStateSL", nesp->AutoStateSL);
		pSave->WriteInt(secName, "AutoStateSLNotload", nesp->AutoStateSLNotload);
		pSave->WriteInt(secName, "AllowMultiInstance", nesp->AllowMultiInstance);
		pSave->WriteInt(secName, "ZipDLLUseFlag", nesp->ZipDLLUseFlag);
		pSave->WriteInt(secName, "SaveFileToZip", nesp->SaveFileToZip);
		pSave->WriteInt(secName, "StateFileToZip", nesp->StateFileToZip);
		pSave->WriteInt(secName, "DisableIPSPatch", nesp->DisableIPSPatch);
		pSave->WriteInt(secName, "DisableMenuIcon", nesp->DisableMenuIcon);

		pSave->WriteInt(secName, "RewindKeyFrame", nesp->RewindKeyFrame);
		pSave->WriteInt(secName, "RewindBFrame", nesp->RewindBFrame);

		pSave->WriteInt(secName, "JoystickUseAPI", nesp->JoystickUseAPI);
		pSave->WriteInt(secName, "StdSpeedState", nesp->StdSpeedState);
		pSave->WriteInt(secName, "FastSpeedState", nesp->FastSpeedState);
		pSave->WriteInt(secName, "StdFPS", nesp->StdFPS);
		pSave->WriteInt(secName, "StdSkipFrame", nesp->StdSkipFrame);
		pSave->WriteInt(secName, "FastSkipFrame", nesp->FastSkipFrame);
//		pSave->WriteInt(secName, "KailleraFrameInterval", nesp->KailleraFrameInterval);
		pSave->WriteInt(secName, "CloseButtonExit", nesp->CloseButtonExit);
		pSave->WriteInt(secName, "TimeUsePCounter", nesp->TimeUsePCounter);
		pSave->WriteInt(secName, "JoystickUseGUID", nesp->JoystickUseGUID);
		pSave->WriteInt(secName, "DisableGameInfoDisp", nesp->DisableGameInfoDisp);
		pSave->WriteInt(secName, "DoubleClickFullScreen", nesp->DoubleClickFullScreen);

		pSave->WriteInt(secName, "GM_WindowStyle_Title", nesp->GM_WindowStyle_Title);
		pSave->WriteInt(secName, "GM_WindowStyle_Menu", nesp->GM_WindowStyle_Menu);
		pSave->WriteInt(secName, "GM_WindowStyle_SBar", nesp->GM_WindowStyle_SBar);

		pSave->WriteInt(secName, "nViewerReInterval", nesp->nViewerReInterval);
		pSave->WriteInt(secName, "nPtnViewerReInterval", nesp->nPtnViewerReInterval);
		delete pSave;
	}catch(...){
		if(pSave)
			delete pSave;
	}

}



void LoadSoundSettings(LoadConfigBase *pLoad, char *secName, NES_sound_settings *nss){

	nss->enabled = pLoad->ReadInt(secName, "enabled", 1);
	nss->sample_bits = pLoad->ReadInt(secName, "sample_bits", 8);
	nss->sample_rate = pLoad->ReadInt(secName, "sample_rate", 44100);
	nss->rectangle1_enabled = pLoad->ReadInt(secName, "rectangle1_enabled", 1);
	nss->rectangle2_enabled = pLoad->ReadInt(secName, "rectangle2_enabled", 1);
	nss->triangle_enabled = pLoad->ReadInt(secName, "triangle_enabled", 1);
	nss->noise_enabled = pLoad->ReadInt(secName, "noise_enabled", 1);
	nss->dpcm_enabled = pLoad->ReadInt(secName, "dpcm_enabled", 1);
	nss->ext_enabled = pLoad->ReadInt(secName, "ext_enabled", 1);

	nss->ideal_triangle_enabled = pLoad->ReadInt(secName, "ideal_triangle_enabled", 0);
	nss->smooth_envelope_enabled = pLoad->ReadInt(secName, "smooth_envelope_enabled", 0);
	nss->smooth_sweep_enabled = pLoad->ReadInt(secName, "smooth_sweep_enabled", 0);
	nss->buffer_len = pLoad->ReadInt(secName, "buffer_len", 3);
	nss->filter_type = (NES_sound_settings::filter_type_t)pLoad->ReadInt(secName, "filter_type", 1);
	nss->lowpass_filter_f = pLoad->ReadInt(secName, "lowpass_filter_f", 15000);

	*((DWORD *)&nss->rectangle1_volumed) = pLoad->ReadInt(secName, "rectangle1_volumed", 0);
	*((DWORD *)&nss->rectangle2_volumed) = pLoad->ReadInt(secName, "rectangle2_volumed", 0);
	*((DWORD *)&nss->triangle_volumed) = pLoad->ReadInt(secName, "triangle_volumed", 0);
	*((DWORD *)&nss->noise_volumed) = pLoad->ReadInt(secName, "noise_volumed", 0);
	*((DWORD *)&nss->dpcm_volumed) = pLoad->ReadInt(secName, "dpcm_volumed", 0);
	*((DWORD *)&nss->ext_volumed) = pLoad->ReadInt(secName, "ext_volumed", 0);

	nss->vrc6_type = pLoad->ReadInt(secName, "vrc6_type", 0);
	nss->vrc7_type = pLoad->ReadInt(secName, "vrc7_type", 0);
}


void SaveSoundSettings(NES_sound_settings *nss){
	char *secName = "SOUND";

	SaveConfigBase *pSave=NULL;

	try{
		pSave = CreateMyConfigSave();

		pSave->WriteInt(secName, "enabled", nss->enabled);
		pSave->WriteInt(secName, "sample_bits", nss->sample_bits);
		pSave->WriteInt(secName, "sample_rate", nss->sample_rate);
		pSave->WriteInt(secName, "rectangle1_enabled", nss->rectangle1_enabled);
		pSave->WriteInt(secName, "rectangle2_enabled", nss->rectangle2_enabled);

		pSave->WriteInt(secName, "triangle_enabled", nss->triangle_enabled);
		pSave->WriteInt(secName, "noise_enabled", nss->noise_enabled);
		pSave->WriteInt(secName, "dpcm_enabled", nss->dpcm_enabled);
		pSave->WriteInt(secName, "ext_enabled", nss->ext_enabled);
		pSave->WriteInt(secName, "ideal_triangle_enabled", nss->ideal_triangle_enabled);
		pSave->WriteInt(secName, "smooth_envelope_enabled", nss->smooth_envelope_enabled);
		pSave->WriteInt(secName, "smooth_sweep_enabled", nss->smooth_sweep_enabled);
		pSave->WriteInt(secName, "buffer_len", nss->buffer_len);
		pSave->WriteInt(secName, "filter_type", nss->filter_type);
		pSave->WriteInt(secName, "lowpass_filter_f", nss->lowpass_filter_f);

		pSave->WriteInt(secName, "rectangle1_volumed", *((DWORD *)&nss->rectangle1_volumed));
		pSave->WriteInt(secName, "rectangle2_volumed", *((DWORD *)&nss->rectangle2_volumed));
		pSave->WriteInt(secName, "triangle_volumed", *((DWORD *)&nss->triangle_volumed));
		pSave->WriteInt(secName, "noise_volumed", *((DWORD *)&nss->noise_volumed));
		pSave->WriteInt(secName, "dpcm_volumed", *((DWORD *)&nss->dpcm_volumed));
		pSave->WriteInt(secName, "ext_volumed", *((DWORD *)&nss->ext_volumed));

		pSave->WriteInt(secName, "vrc6_type", nss->vrc6_type);
		pSave->WriteInt(secName, "vrc7_type", nss->vrc7_type);

		delete pSave;
	}catch(...){
		if(pSave)
			delete pSave;
	}
}


void LoadGraphics(LoadConfigBase *pLoad, char *secName, NES_graphics_settings *ngs){

	
	ngs->osd.InfoStatusBar = pLoad->ReadInt(secName, "InfoStatusBar", 0);
	ngs->osd.WindowTopMost = pLoad->ReadInt(secName, "WindowTopMost", 0);
	*((DWORD *)&ngs->osd.zoom_size) = pLoad->ReadInt(secName, "zoom_size", 0x3f800000);
	ngs->black_and_white = pLoad->ReadInt(secName, "black_and_white", 0);
	ngs->show_more_than_8_sprites = pLoad->ReadInt(secName, "show_more_than_8_sprites", 0);
	ngs->show_all_scanlines = pLoad->ReadInt(secName, "show_all_scanlines", 0);
	ngs->draw_overscan = pLoad->ReadInt(secName, "draw_overscan", 0);
	ngs->fullscreen_on_load = pLoad->ReadInt(secName, "fullscreen_on_load", 0);
	ngs->fullscreen_scaling = pLoad->ReadInt(secName, "fullscreen_scaling", 0);
	ngs->calculate_palette = pLoad->ReadInt(secName, "calculate_palette", 0);
	pLoad->ReadString(secName, "szPaletteFile", ngs->szPaletteFile, MAX_PATH, "");
	ngs->tint = pLoad->ReadInt(secName, "tint", 0x86);
	ngs->hue = pLoad->ReadInt(secName, "hue", 0x9d);
	ngs->osd.fullscreen_width = pLoad->ReadInt(secName, "fullscreen_width", 320);
	ngs->osd.fullscreen_height = pLoad->ReadInt(secName, "fullscreen_height", ngs->osd.fullscreen_width*3/4);
	ngs->osd.fullscreen_BitDepth = pLoad->ReadInt(secName, "fullscreen_BitDepth", 8);
	
	pLoad->ReadHexString(secName, "device_GUID", (unsigned char *)&ngs->osd.device_GUID, sizeof(GUID));
	
	ngs->UseStretchBlt = pLoad->ReadInt(secName, "UseStretchBlt", 0);
	ngs->EmulateTVScanline = pLoad->ReadInt(secName, "EmulateTVScanline", 0);
	ngs->WindowModeUseDdraw = pLoad->ReadInt(secName, "WindowModeUseDdraw", 0);
	ngs->WindowModeUseDdrawOverlay = pLoad->ReadInt(secName, "WindowModeUseDdrawOverlay", 0);
	ngs->FullscreenModeUseNewMode = pLoad->ReadInt(secName, "FullscreenModeUseNewMode", 0);
	ngs->FullscreenModeUseDefRefreshRate = pLoad->ReadInt(secName, "FullscreenModeUseDefRefreshRate", 0);
	ngs->FullscreenModeBackBuffM = pLoad->ReadInt(secName, "FullscreenModeBackBuffM", 0);
	ngs->WindowModeBackBuffM = pLoad->ReadInt(secName, "WindowModeBackBuffM", 0);
	ngs->osd.DtvSize = pLoad->ReadInt(secName, "DtvSize", 0);
	ngs->ScreenShotUseBmp = pLoad->ReadInt(secName, "ScreenShotUseBmp", 0);
	ngs->FullscreenModeNotUseFlip = pLoad->ReadInt(secName, "FullscreenModeNotUseFlip", 0);
	ngs->DisableSpriteClipping = pLoad->ReadInt(secName, "DisableSpriteClipping", 0);
	ngs->DisableBackGClipping = pLoad->ReadInt(secName, "DisableBackGClipping", 0);
	ngs->WindowDDMode_NotuseGDI = pLoad->ReadInt(secName, "WindowDDMode_NotuseGDI", 0);
	ngs->WindowMode_2xsai = pLoad->ReadInt(secName, "WindowMode_2xsai", 0);
	ngs->FullscreenMode_2xsai = pLoad->ReadInt(secName, "FullscreenMode_2xsai", 0);
	ngs->WindowMode_2xsaiType = pLoad->ReadInt(secName, "WindowMode_2xsaiType", 0);
	ngs->FullscreenMode_2xsaiType = pLoad->ReadInt(secName, "FullscreenMode_2xsaiType", 0);
	
	ngs->osd.WindowPos_x = pLoad->ReadInt(secName, "WindowPos_x", CW_USEDEFAULT);
	ngs->osd.WindowPos_y = pLoad->ReadInt(secName, "WindowPos_y", CW_USEDEFAULT);
}


void SaveSettingsOnMenu(SaveConfigBase *pSave, NES_settings *ness){
	char *secName = "Graphics";
	char *secName2 = "NESMAIN";

	pSave->WriteInt(secName, "InfoStatusBar", ness->graphics.osd.InfoStatusBar);
	pSave->WriteInt(secName, "WindowTopMost", ness->graphics.osd.WindowTopMost);
	pSave->WriteInt(secName, "zoom_size", *((DWORD *)&ness->graphics.osd.zoom_size));
	pSave->WriteInt(secName, "DtvSize", ness->graphics.osd.DtvSize);

	pSave->WriteInt(secName2, "AutoStateSL", ness->preferences.AutoStateSL);
	pSave->WriteInt(secName2, "PreviewMode", ness->preferences.PreviewMode);
	pSave->WriteInt(secName2, "TV_Mode", ness->preferences.TV_Mode);
	pSave->WriteInt(secName2, "ShowFPS", ness->preferences.ShowFPS);
	pSave->WriteInt(secName2, "UseRewind", ness->preferences.UseRewind);

	pSave->WriteInt(secName, "WindowPos_x", ness->graphics.osd.WindowPos_x);
	pSave->WriteInt(secName, "WindowPos_y", ness->graphics.osd.WindowPos_y);

}


void SaveGraphics(NES_graphics_settings *ngs){
	char *secName = "Graphics";
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
	SaveConfigBase *pSave=NULL;

	try{
		pSave = CreateMyConfigSave();

		pSave->WriteInt(secName, "black_and_white", ngs->black_and_white);
		pSave->WriteInt(secName, "show_more_than_8_sprites", ngs->show_more_than_8_sprites);
		pSave->WriteInt(secName, "show_all_scanlines", ngs->show_all_scanlines);
		pSave->WriteInt(secName, "draw_overscan", ngs->draw_overscan);
		pSave->WriteInt(secName, "fullscreen_on_load", ngs->fullscreen_on_load);
		pSave->WriteInt(secName, "fullscreen_scaling", ngs->fullscreen_scaling);
		pSave->WriteInt(secName, "calculate_palette", ngs->calculate_palette);
		pSave->WriteString(secName, "szPaletteFile", ngs->szPaletteFile);
		pSave->WriteInt(secName, "tint", ngs->tint);
		pSave->WriteInt(secName, "hue", ngs->hue);
		pSave->WriteInt(secName, "fullscreen_width", ngs->osd.fullscreen_width);
		pSave->WriteInt(secName, "fullscreen_height", ngs->osd.fullscreen_height);
		pSave->WriteInt(secName, "fullscreen_BitDepth", ngs->osd.fullscreen_BitDepth);

		pSave->WriteHexString(secName, "device_GUID", (unsigned char *)&ngs->osd.device_GUID, sizeof(GUID));

		pSave->WriteInt(secName, "UseStretchBlt", ngs->UseStretchBlt);
		pSave->WriteInt(secName, "EmulateTVScanline", ngs->EmulateTVScanline);

		pSave->WriteInt(secName, "WindowModeUseDdraw", ngs->WindowModeUseDdraw);
		pSave->WriteInt(secName, "WindowModeUseDdrawOverlay", ngs->WindowModeUseDdrawOverlay);
		pSave->WriteInt(secName, "FullscreenModeUseNewMode", ngs->FullscreenModeUseNewMode);
		pSave->WriteInt(secName, "FullscreenModeUseDefRefreshRate", ngs->FullscreenModeUseDefRefreshRate);
		pSave->WriteInt(secName, "FullscreenModeBackBuffM", ngs->FullscreenModeBackBuffM);
		pSave->WriteInt(secName, "WindowModeBackBuffM", ngs->WindowModeBackBuffM);

		pSave->WriteInt(secName, "ScreenShotUseBmp", ngs->ScreenShotUseBmp);
		pSave->WriteInt(secName, "FullscreenModeNotUseFlip", ngs->FullscreenModeNotUseFlip);
		pSave->WriteInt(secName, "DisableSpriteClipping", ngs->DisableSpriteClipping);
		pSave->WriteInt(secName, "DisableBackGClipping", ngs->DisableBackGClipping);
		pSave->WriteInt(secName, "WindowDDMode_NotuseGDI", ngs->WindowDDMode_NotuseGDI);
		pSave->WriteInt(secName, "FullscreenMode_2xsai", ngs->FullscreenMode_2xsai);
		pSave->WriteInt(secName, "WindowMode_2xsai", ngs->WindowMode_2xsai);
		pSave->WriteInt(secName, "FullscreenMode_2xsaiType", ngs->FullscreenMode_2xsaiType);
		pSave->WriteInt(secName, "WindowMode_2xsaiType", ngs->WindowMode_2xsaiType);

		delete pSave;
	}catch(...){
		if(pSave)
			delete pSave;
	}
}




void LoadRecentFiles(LoadConfigBase *pLoad, char *secName, recent_list& rl){
	char buff[MAX_PATH], keyName[32];
	rl.clear();
	for(int i = 9; i >= 0; i--){
		wsprintf(keyName, "RECENT%i", i);
		buff[0] =0;
		if(pLoad->ReadString(secName, keyName, buff, MAX_PATH, "")){
			if(buff[0] != 0)
				rl.add_entry(buff);
		}
	}
}


void SaveRecentFiles(SaveConfigBase *pSave, char *secName, const recent_list& rl){
	int max = rl.get_max_entries();
	char buff[MAX_PATH], keyName[32];
	for(int i = 0; i < max; i++){
		wsprintf(keyName, "RECENT%i", i);
		buff[0] =0;
		if(rl.get_entry(i)){
			strcpy(buff, rl.get_entry(i));
		}
		pSave->WriteString(secName, keyName, buff);
	}
}


void LoadButtonSettings_ini(LoadConfigBase *pLoad, char *secName, char *keyName, OSD_ButtonSettings* bsettings){
	unsigned char value[4];

	*((DWORD *)&value[0]) = pLoad->ReadInt(secName, keyName, -1);
	if(*((long *)&value[0]) != -1){
		bsettings->type = (unsigned int)value[0];
		bsettings->dev_n= (unsigned int)value[1];
		bsettings->j_offset= (unsigned int)value[2];
		bsettings->j_axispositive= (unsigned int)value[3];
		if(!NESTER_settings.nes.preferences.JoystickUseAPI && NESTER_settings.nes.preferences.JoystickUseGUID){
			char vstr[32];
			strcpy(vstr, keyName);
			strcat(vstr, "_GUID");
			pLoad->ReadHexString(secName, vstr, (unsigned char *)&bsettings->DIJoyGUID, sizeof(GUID));
		}
	}
}



void LoadOneControllerSettings(LoadConfigBase *pLoad, char *secName, NES_controller_input_settings *nescs){

	LoadButtonSettings_ini(pLoad, secName, "UP", &nescs->btnUp);
	LoadButtonSettings_ini(pLoad, secName, "DOUN", &nescs->btnDown);
	LoadButtonSettings_ini(pLoad, secName, "LEFT", &nescs->btnLeft);
	LoadButtonSettings_ini(pLoad, secName, "RIGHT", &nescs->btnRight);
	LoadButtonSettings_ini(pLoad, secName, "SELECT", &nescs->btnSelect);
	LoadButtonSettings_ini(pLoad, secName, "START", &nescs->btnStart);
	LoadButtonSettings_ini(pLoad, secName, "B", &nescs->btnB);
	LoadButtonSettings_ini(pLoad, secName, "A", &nescs->btnA);
	LoadButtonSettings_ini(pLoad, secName, "TB", &nescs->btnTB);
	LoadButtonSettings_ini(pLoad, secName, "TA", &nescs->btnTA);
	nescs->btnTBsec = pLoad->ReadInt(secName, "TBSPD", 30);
	nescs->btnTAsec = pLoad->ReadInt(secName, "TASPD", 30);
	nescs->AutoFireToggle = pLoad->ReadInt(secName, "AutoFireToggle", 0);
}


void LoadAllControllerSettings(LoadConfigBase *pLoad,  NES_input_settings *nesis){
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
		LoadOneControllerSettings(pLoad, tstr, ncs);
	}
	LoadButtonSettings_ini(pLoad, "ETCBTN", "MIC", &nesis->MicButton);
	for(i=0; i<EXTKEYALLNUM;++i){
		wsprintf(tstr, "EXTBTN%u", i);
		LoadButtonSettings_ini(pLoad, "EXTBTN", tstr, &nesis->extkeycfg[i]);
	}
}


unsigned char Load_ini_Dll_Settings(class settings_t& settings){
	LoadConfigBase *pLoad=NULL;

	try{
		pLoad = CreateMyConfigLoad();
		pLoad->ReadString("RESOURCE", "FILENAME", settings.ResourceDll, sizeof(settings.ResourceDll), "");
		delete pLoad;
	}catch(...){
		if(pLoad)
			delete pLoad;
		return FALSE;
	}
	return TRUE;
}


unsigned char Load_ini_Settings(class settings_t& settings){
	LoadConfigBase *pLoad=NULL;

	try{
		pLoad = CreateMyConfigLoad();

		pLoad->ReadString("etc", "OpenPath", settings.OpenPath, sizeof(settings.OpenPath), "");
		settings.DirectSoundVolume = pLoad->ReadInt("GENERAL", "DirectSoundVolume", 100);

		LoadPathSettings(pLoad, "PATH", &settings.path);
		LoadRecentFiles(pLoad, "RECENT", settings.recent_ROMs);
		LoadNesMainSettings(pLoad, "NESMAIN", &settings.nes.preferences);
		LoadGraphics(pLoad, "Graphics", &settings.nes.graphics);
		LoadSoundSettings(pLoad, "SOUND", &settings.nes.sound);
		LoadCheatDlgSettings(pLoad, "CheatDlgState");
		LoadAllControllerSettings(pLoad, &settings.nes.input);
		LoadPreviewSettings(pLoad, "Preview");
		LoadHoyKeyCfg();
		delete pLoad;
	}catch(...){
		if(pLoad)
			delete pLoad;
		return FALSE;
	}
	return TRUE;
}


void SaveButtonSettings_ini(SaveConfigBase *pSave, char *secName, char *keyName, OSD_ButtonSettings* bsettings){
	unsigned char value[4];
	char vstr[64];
	value[0] = (unsigned char)bsettings->type;
	value[1] = (unsigned char)bsettings->dev_n;
	value[2] = (unsigned char)bsettings->j_offset;
	value[3] = (unsigned char)bsettings->j_axispositive;

	pSave->WriteInt(secName, keyName, *((DWORD *)&value[0]));
	if(!NESTER_settings.nes.preferences.JoystickUseAPI && NESTER_settings.nes.preferences.JoystickUseGUID){
		strcpy(vstr, keyName);
		strcat(vstr, "_GUID");
		pSave->WriteHexString(secName, vstr, (unsigned char *)&bsettings->DIJoyGUID, sizeof(GUID));
	}
}


void SaveOneControllerSettings(SaveConfigBase *pSave, char *secName, NES_controller_input_settings *nescs){

	SaveButtonSettings_ini(pSave, secName, "UP", &nescs->btnUp);
	SaveButtonSettings_ini(pSave, secName, "DOUN", &nescs->btnDown);
	SaveButtonSettings_ini(pSave, secName, "LEFT", &nescs->btnLeft);
	SaveButtonSettings_ini(pSave, secName, "RIGHT", &nescs->btnRight);
	SaveButtonSettings_ini(pSave, secName, "SELECT", &nescs->btnSelect);
	SaveButtonSettings_ini(pSave, secName, "START", &nescs->btnStart);
	SaveButtonSettings_ini(pSave, secName, "B", &nescs->btnB);
	SaveButtonSettings_ini(pSave, secName, "A", &nescs->btnA);
	SaveButtonSettings_ini(pSave, secName, "TB", &nescs->btnTB);
	SaveButtonSettings_ini(pSave, secName, "TA", &nescs->btnTA);
	pSave->WriteInt(secName, "TBSPD", nescs->btnTBsec);
	pSave->WriteInt(secName, "TASPD", nescs->btnTAsec);
	pSave->WriteInt(secName, "AutoFireToggle", nescs->AutoFireToggle);
}


void SaveAllControllerSettings(NES_input_settings *nesis){
	NES_controller_input_settings *ncs;
	char tstr[64];

	SaveConfigBase *pSave=NULL;

	try{
		pSave = CreateMyConfigSave();
		
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
			SaveOneControllerSettings(pSave, tstr, ncs);
		}
		SaveButtonSettings_ini(pSave, "ETCBTN", "MIC", &nesis->MicButton);
		for(i=0; i<EXTKEYALLNUM;++i){
			wsprintf(tstr, "EXTBTN%u", i);
			SaveButtonSettings_ini(pSave, "EXTBTN", tstr, &nesis->extkeycfg[i]);
		}
		delete pSave;
	}catch(...){
		if(pSave)
			delete pSave;
	}
}



unsigned char Save_ini_Settings(class settings_t& settings){
	SaveConfigBase *pSave=NULL;

	try{
		pSave = CreateMyConfigSave();

		pSave->WriteString("VERSION", "Ver", "0.22");
		pSave->WriteString("etc", "OpenPath", settings.OpenPath);
		pSave->WriteString("RESOURCE", "FILENAME", settings.ResourceDll);

		pSave->WriteInt("GENERAL", "DirectSoundVolume", settings.DirectSoundVolume);

		SavePathSettings(pSave, "PATH", &settings.path);
		SaveRecentFiles(pSave, "RECENT", settings.recent_ROMs);
		//	SaveNesMainSettings(inifn, "NESMAIN", &settings.nes.preferences);
		//	SaveGraphics(inifn, "Graphics", &settings.nes.graphics);
		//	SaveSoundSettings(inifn, "SOUND", &settings.nes.sound);
		//	SaveAllControllerSettings(inifn, &settings.nes.input);
		SaveCheatDlgSettings(pSave, "CheatDlgState");
		SaveSettingsOnMenu(pSave, &settings.nes);

		SavePreviewSettings(pSave, "Preview");

		delete pSave;
	}catch(...){
		if(pSave)
			delete pSave;
	}

	return TRUE;
}



void SaveHoyKeyCfg(){
//	char inifn[MAX_PATH], vstr[64]
	char keyname[64];
	char *sname = "HOTKEY";
	int i;
	SaveConfigBase *pSave=NULL;

	try{
		pSave = CreateMyConfigSave();

		for(i=0; i<1; i++){
			wsprintf(keyname, "HotKey_%02u", i);
			pSave->WriteInt(sname, keyname, g_Window_ShotrCutKey[i].KeyConfig);

			wsprintf(keyname, "HKActive_%02u", i);
			pSave->WriteInt(sname, keyname, g_Window_ShotrCutKey[i].Active);
		}
		delete pSave;
	}catch(...){
		if(pSave)
			delete pSave;
	}
}


void LoadHoyKeyCfg(){
//	char inifn[MAX_PATH], vstr[64]
	char keyname[64];
	char *sname = "HOTKEY";
	int i;

	LoadConfigBase *pLoad=NULL;

	try{
		pLoad = CreateMyConfigLoad();

		for(i=0; i<1; i++){
			wsprintf(keyname, "HotKey_%02u", i);
			g_Window_ShotrCutKey[i].KeyConfig = pLoad->ReadInt(sname, keyname, 0);
			wsprintf(keyname, "HKActive_%02u", i);
			g_Window_ShotrCutKey[i].Active = pLoad->ReadInt(sname, keyname, 0);
		}
		delete pLoad;
	}catch(...){
		if(pLoad)
			delete pLoad;
	}
}


/***  ***/
extern unsigned char ListNo;
extern unsigned char previewflag;
extern char LauncherListName[10][64];
extern DWORD launcherFontColor;
extern DWORD launcherBackColor;
extern LOGFONT launcherFont;
/***  ***/

void NNNcfgsave(){
	char *secName = "Launcher";
	SaveConfigBase *pSave=NULL;

	try{
		pSave = CreateMyConfigSave();
		pSave->WriteInt(secName, "Show", previewflag);
		delete pSave;
	}catch(...){
		if(pSave)
			delete pSave;
	}
}


void NNNcfgload(){
	char *secName = "Launcher";
	int i;

	LoadConfigBase *pLoad=NULL;

	try{
		pLoad = CreateMyConfigLoad();
		previewflag = pLoad->ReadInt(secName, "Show", 0);
		delete pLoad;
	}catch(...){
		if(pLoad)
			delete pLoad;
	}
}


void NNNlunchcfgsave(){
	char *secName = "Launcher";
	SaveConfigBase *pSave=NULL;

	try{
		pSave = CreateMyConfigSave();
		pSave->WriteInt(secName, "ListNo", ListNo);
		{
			char kn[16];
			for(int i=0;i<9;++i){
				wsprintf(kn, "ListName%u", i);
				pSave->WriteString(secName, kn, LauncherListName[i]);
			}
		}
		pSave->WriteHexString(secName, "LOGFONT", (unsigned char *)&launcherFont, sizeof(LOGFONT));
		pSave->WriteInt(secName, "FontColor", launcherFontColor);
		pSave->WriteInt(secName, "BGColor", launcherBackColor);
		delete pSave;
	}catch(...){
		if(pSave)
			delete pSave;
	}
}


void NNNlunchcfgload(){
	char *secName = "Launcher";

	LoadConfigBase *pLoad=NULL;

	try{
		pLoad = CreateMyConfigLoad();
		ListNo = pLoad->ReadInt(secName, "ListNo", 0);
		launcherFontColor = pLoad->ReadInt(secName, "FontColor", 0);
		launcherBackColor = pLoad->ReadInt(secName, "BGColor", 0xffffffff);
		{
			char kn[16];
			for(int i=0;i<9;++i){
				wsprintf(kn, "ListName%u", i);
				pLoad->ReadString(secName, kn, LauncherListName[i], MAX_PATH, "");
			}
		}
		pLoad->ReadHexString(secName, "LOGFONT", (unsigned char *)&launcherFont, sizeof(LOGFONT));
		delete pLoad;
	}catch(...){
		if(pLoad)
			delete pLoad;
	}
}
