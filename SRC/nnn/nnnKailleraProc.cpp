

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <shlobj.h>
#include <commdlg.h>
#include <stddef.h>
#include <richedit.h>

#include "resource.h"
#include "win32_emu.h"
#include "settings.h"

#include "debug.h"

#include "cheat.h"
#include "savecfg.h"
#include "nnnkaillera.h"
#include "preview.h"
#include "extra_window.h"

//#include "kaillera.h"

#define WM_NNNKAILLERA (WM_USER+20)

extern HWND main_window_handle;
extern HINSTANCE g_main_instance;
extern emulator* emu;
extern struct Preview_state g_Preview_State;
extern struct extra_window_struct g_extra_window;

extern HWND Mainwindow_Preview;
extern int g_PreviewMode;
extern int nnnKailleraFlag;
extern int nnnKailleraplayer;
extern int nnnKailleraDlgFlag;
extern char previewfn[MAX_PATH];

extern HWND g_hKailleraLog;
extern HWND g_hKailleraSend;
extern int  g_nKailleraChatWinH;
extern CHARFORMAT g_Kaillera_charfmt;

static int WINAPI pkGameCallback(char *game, int player, int maxplayers);
static void WINAPI pkchatReceived(char *nick, char *text);
static void WINAPI pkclientDropped(char *nick, int playernb);
static void WINAPI pkmoreInfos(char *gamename);


static kailleraInfos pkInfos={ "", "", pkGameCallback, pkchatReceived, pkclientDropped, pkmoreInfos};

static struct SubCategory List;

//////////////////  Kaillera ///////////

extern HACCEL hAccel;
extern void EmuError(const char* );
extern void FreeROM();
extern void LoadROM(const char* rom_name);

#if 1
static int WINAPI pkGameCallback(char *game, int player, int maxplayers){
	char romfn[MAX_PATH];
	{
		int i, j, n;
		char flag=0, tstr[MAX_PATH+10];

//		if(nnnKailleraFlag)
//			return 0;

		n =	List.nListValidItem;
		for(i=0; i<n; i++){
			int k=0;
			j=0;
			do{
				tstr[j++] = g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Title[k];
			}while(g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Title[k++]);
			k=0;
			if(g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Country[k]){
				--j;
				tstr[j++] = '[';
				for(;g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Country[k];k++){
					tstr[j++] = g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Country[k];
				}
				tstr[j++] = ']';
				tstr[j++] = 0;
			}
			tstr[j++] = 0;
			if(!strcmp(tstr, game)){
				strcpy(romfn, g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].FileName);
				flag = 1;
				break;
			}
		}
		if(flag==0)
			return 0;
	}
//	kPlay=1;
	if(emu)
		FreeROM();
	nnnKailleraplayer=(unsigned char)player;
	nnnKailleraFlag=1;
	Goto_WindowGameMode();
	LoadROM(romfn);
/*
	{
	DWORD pt;
	pt=GetCurrentTime();
	while((pt+700)>GetCurrentTime()){
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if(!TranslateAccelerator(main_window_handle, hAccel, &msg)){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	}
	SendMessage(main_window_handle, WM_PREVIEW, 0, 0);
//	PostMessage(main_window_handle, WM_PREVIEW, 0, 0);
	SetFocus(main_window_handle);
*/
/*	{
	int recv;
	char buff[5];
	buff[0]='S';
	if(-1 == (recv= nnnkailleraModifyPlayValues(&buff,1))){
		nnnKailleraFlag=0;
		if(emu)
			FreeROM();
		return 0;
	}
	do{
		if(-1 == (recv= nnnkailleraModifyPlayValues(&buff,0))){
			nnnKailleraFlag=0;
			if(emu)
				FreeROM();
			return 0;
		}
	}while(recv==0);
	}*/
	MSG msg;

	while(1){
		if(emu /*&& !emu->frozen()*/){
			try {
				emu->do_frame();
			} catch(const char* s){
				LOG("EXCEPTION: " << s << endl);
				EmuError(s);
			} catch(...) {
				LOG("Caught unknown exception in " << __FILE__ << endl);
				EmuError("unknown error");
			}
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
//        DUMP_WM_MESSAGE(1,msg.message);
				if(msg.message == WM_QUIT) break;
				if(!CheckModelessDialogMessage(&msg)){
#ifdef TOOLTIP_HACK
				if(msg.message != 0x0118)
#endif
					if(!TranslateAccelerator(main_window_handle, hAccel, &msg)){
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}
		}
		else if(!emu){
			break;
		}
#if 0
		else
		{
			if(GetMessage(&msg, main_window_handle/*NULL*/, 0, 0))
			{
//        DUMP_WM_MESSAGE(2,msg.message);
				if(!CheckModelessDialogMessage(&msg)){
#ifdef TOOLTIP_HACK
				if(msg.message != 0x0118)
#endif
					if(!TranslateAccelerator(main_window_handle, hAccel, &msg))
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
#endif
		if(!nnnKailleraFlag)
			break;
	}
//	kPlay=0;
/*
	if(nnnKailleraFlag){
		nnnkailleraEndGame();
		nnnKailleraFlag = 0;
	}
*/
	nnnkailleraEndGame();
	nnnKailleraFlag = 0;
	if(emu)
		FreeROM();
	return 0;
}
#else
static int WINAPI pkGameCallback(char *game, int player, int maxplayers){
	char romfn[MAX_PATH];
	{
		int i, j, n;
		char flag=0, tstr[MAX_PATH+10];

//		if(nnnKailleraFlag)
//			return 0;

		n =	List.nListValidItem;
		for(i=0; i<n; i++){
			int k=0;
			j=0;
			do{
				tstr[j++] = g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Title[k];
			}while(g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Title[k++]);
			k=0;
			if(g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Country[k]){
				--j;
				tstr[j++] = '[';
				for(;g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Country[k];k++){
					tstr[j++] = g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Country[k];
				}
				tstr[j++] = ']';
				tstr[j++] = 0;
			}
			tstr[j++] = 0;
			if(!strcmp(tstr, game)){
				strcpy(romfn, g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].FileName);
				flag = 1;
				break;
			}
		}
		if(flag==0)
			return 0;
	}
//	if(emu)
//		FreeROM();
	FreeROM();
	nnnKailleraplayer=(unsigned char)player;
	nnnKailleraFlag=1;

	LoadROM(romfn);
	return 0;
}
#endif


static void OutputKailleraChatLog(char *addtext, DWORD color){
//	SendMessage(g_hKailleraLog, EM_SETCHARFORMAT, SCF_ALL/*SCF_SELECTION*/, (LPARAM)&g_Kaillera_charfmt);
	int Pos = SendMessage(g_hKailleraLog, WM_GETTEXTLENGTH, 0, 0);
	if((Pos + strlen(addtext)) >= 10*1024){
		Pos = SendMessage(g_hKailleraLog, EM_LINEFROMCHAR, 1024, 0) + 1;
		Pos = SendMessage(g_hKailleraLog, EM_LINEINDEX, Pos, 0);
		SendMessage(g_hKailleraLog, EM_SETSEL, 0, Pos);
		SendMessage(g_hKailleraLog, EM_REPLACESEL, FALSE, (LPARAM)"");
		Pos = SendMessage(g_hKailleraLog, WM_GETTEXTLENGTH, 0, 0);
	}
	SendMessage(g_hKailleraLog, EM_SETSEL, Pos, Pos);
	SendMessage(g_hKailleraLog, EM_REPLACESEL, FALSE, (LPARAM)addtext);

	int epos = SendMessage(g_hKailleraLog, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(g_hKailleraLog, EM_SETSEL, Pos, epos);
	g_Kaillera_charfmt.crTextColor = color;
	SendMessage(g_hKailleraLog, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&g_Kaillera_charfmt);
	SendMessage(g_hKailleraLog, EM_SETSEL, epos, epos);
	{
		int lc = SendMessage(g_hKailleraLog, EM_GETLINECOUNT, 0, 0);
		int i;
//		SendMessage(hMainEdit, EM_SCROLL, (WPARAM)SB_LINEDOWN, 0);
		for (i = 0; i < lc; i++)
			SendMessage(g_hKailleraLog, EM_SCROLL, (WPARAM)SB_LINEDOWN, 0);
	}
}



static void WINAPI pkchatReceived(char *nick, char *text){
	if(g_hKailleraLog){
		char addtext[1024];
		wsprintf(addtext, "%s : %s\r\n", nick, text);
		OutputKailleraChatLog(addtext, 0);
	}
	return;
}

static void WINAPI pkclientDropped(char *nick, int playernb){
	char str[260*2];
	wsprintf(str, "%s‚³‚ñ—£’E(L„t¼j\r\n", nick);
	OutputKailleraChatLog(str, 0x00FF00);
	return;
}

static void WINAPI pkmoreInfos(char *gamename){
	return;
}

//////


LRESULT CALLBACK MyKailleraDammyWindProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp);

HWND CreateMyKailleraWindow(HWND hWnd){
    WNDCLASSEX wc;
	static int rflag=0;
	RECT rc;
	HINSTANCE hInst;

	if(g_extra_window.KailleraWindow){
		SetForegroundWindow(g_extra_window.KailleraWindow);
		return NULL;
	}
	hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	GetWindowRect(hWnd, &rc);

	if(rflag==0)
	{
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_VREDRAW | CS_HREDRAW;
		wc.lpfnWndProc = MyKailleraDammyWindProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.lpszMenuName = NULL; //MAKEINTRESOURCE(IDR_MENUCT);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszClassName = (LPCSTR)"NNNKaillera";
		wc.hIconSm = (HICON)LoadImage(g_main_instance,MAKEINTRESOURCE(IDI_KAILLERAICON),IMAGE_ICON  , 0, 0,LR_DEFAULTCOLOR);
		if(RegisterClassEx(&wc) == 0) {
			return NULL;
		}
		rflag=1;
	}
	g_extra_window.KailleraWindow = CreateWindowEx(NULL, "NNNKaillera", "NNNKaillera", WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
		rc.left+100, rc.top+100, 0, 0, NULL/*hWnd*/, /*LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENUCT))*/NULL, hInst, NULL);
	ShowWindow(g_extra_window.KailleraWindow, SW_SHOW);
	UpdateWindow(g_extra_window.KailleraWindow);
/*
	SetRect(&rct, 0, 0, 256, 128);
	AdjustWindowRectEx(&rct, GetWindowStyle(g_extra_window.PatternWindow),
					   GetMenu(g_extra_window.PatternWindow) != NULL,
					   GetWindowExStyle(g_extra_window.PatternWindow));
	SetWindowPos(g_extra_window.PatternWindow, HWND_TOP, 0, 0,
				 rct.right-rct.left, rct.bottom-rct.top, SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
*/
	PostMessage(g_extra_window.KailleraWindow, WM_NNNKAILLERA, 0, 0);
	return g_extra_window.KailleraWindow;
}




LRESULT CALLBACK MyKailleraDammyWindProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp){

	static HINSTANCE hInst;

	switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT paint;
				HDC hdc = BeginPaint(hChild, &paint);
				EndPaint(hChild, &paint);
			}
			break;
		case WM_NNNKAILLERA:
			{
				char *tmpGames=NULL;
				int i, j=0, n;

				CopySubCategory(&g_Preview_State.Category[0].Sub[0], &List);
				n = List.nListValidItem;
				tmpGames=(char *)malloc(n*(260+6));

				PreviewDataStructSort(List.pListViewValidItem, n, 0, 1);
				for(i=0; i<n; i++){
					int k=0;
					do{
						tmpGames[j++] = g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Title[k];
					}while(g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Title[k++]);
					k=0;
					if(g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Country[k]){
						--j;
						tmpGames[j++] = '[';
						for(;g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Country[k];k++){
							tmpGames[j++] = g_Preview_State.DB.pListViewItem[List.pListViewValidItem[i]].Country[k];
						}
						tmpGames[j++] = ']';
						tmpGames[j++] = 0;
					}
				}
				tmpGames[j++] = 0;
				tmpGames[j++] = 0;
				tmpGames[j++] = 0;
/*
				{
					char *pt;
					pt = (char *)realloc(tmpGames, j);
					tmpGames=pt;
				}
*/
				pkInfos.appName= "NNNesterJ[K01]";  //PROG_NAME;
				pkInfos.gameList=tmpGames;
				nnnKailleraDlgFlag=1;
				nnnkailleraSetInfos(&pkInfos);
				free(tmpGames);
				nnnkailleraSelectServerDialog(hChild);
//				free(tmpGames);
				nnnKailleraDlgFlag=0;
				FreeMemSubCategory(&List);
				nnnkailleraShutdown();
				DestroyWindow(hChild);
			}
			break;
#if 0
		case WM_SIZE:
			break;
		case WM_KEYDOWN:
			break;
		case WM_EXTRAWIN_01:
			if(emu){
				emu->Get_PPUMemp(pPPU);
			}
			break;
		case WM_EXTRAWIN_02:
			break;
		case WM_EXTRAWIN_03:
			{
				if(emu && pPPU[0])
				{
					emu->Get_PPUMemp(pPPU);
					HDC hdc = GetDC(hChild);
					MyDrawNESPattern(hdc, pPPU);
					ReleaseDC(hChild, hdc);	
//					Frame_IntervalCounter=0;
				}
			}
			break;
		case WM_LBUTTONDOWN:
			SendMessage(hChild, WM_SYSCOMMAND, SC_MOVE|0x02, 0L);
			break;
		case WM_CONTEXTMENU:
			{
				POINT pt;
				pt.x = LOWORD(lp);
				pt.y = HIWORD(lp);
				HMENU hPMenu = CreatePopupMenu();
				if(hPMenu!=NULL){
					char str[64];
					LoadString(g_main_instance, IDS_STRING_EVIEWMS_1 , str, 64);
					AppendMenu(hPMenu, MF_STRING, IDS_STRING_EVIEWMS_1, str);
					TrackPopupMenu(hPMenu, TPM_LEFTALIGN /*| TPM_TOPALIGN*/, pt.x, pt.y, 0, hChild, NULL);
					DestroyMenu(hPMenu);
				}
			}
			break;
/*
        case WM_COMMAND:
			switch (LOWORD(wp)){
			}
			break;
*/
#endif
		case WM_CREATE:
			if(Mainwindow_Preview==NULL || g_Preview_State.Category[0].Sub[0].pListViewValidItem==NULL || !nnnkailleraInit()){
				DestroyWindow(hChild);
				break;
			}
			hInst = g_main_instance; //(HINSTANCE)GetWindowLong(GetParent(hChild), GWL_HINSTANCE);
			break;
		case WM_DESTROY:
//			nnnkailleraShutdown();
			g_extra_window.KailleraWindow= NULL;
			break;
		default:
			return (DefWindowProc(hChild, msg, wp, lp));
	}
	return 0L;
}




