
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <wingdi.h>

#include "settings.h"
#include "savecfg.h"
#include "cheat.h"
#include "arc.h"
#include "ulunzip.h"
#include "unrarp.h"

#include "resource.h"

#include "win32_emu.h"
#include "nnnkaillera.h"
#include "nnndialog.h"
#include "launcher.h"
#include "win32_screen_mgr.h"
#include "crc32.h"
#include "savecfg.h"
#include "EditView.h"
#include "extra_window.h"
#include "nes6502.h"

#define commentsize  32

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL	0x020A
#endif

extern win32_screen_mgr* gscr_mgr;
extern emulator* emu;
extern int PrevFastSw;
struct extra_window_struct g_extra_window;
extern HINSTANCE g_main_instance;
extern LOGFONT Default_Font;



//////////////  Cheat   /////////////

struct cheat_info_struct g_Cheat_Info;


// Dialog
DWORD MyCheatAdr, MyCheatNum;
unsigned char *SResulttext, *SResulttext2, *SResulttext3;
DWORD ctcmpdata2;			//pre
DWORD ctcmpdata;				//
int ctcmpflag=0;

/*
unsigned char ctcmpdtf1=0;						//Compare Checkbox
unsigned char ctcmpdtf2=0;						//Size
unsigned char ctcmpdtf3=0;						//In Type 0=Dec
unsigned char ctcmpdtf4=0;						//Out Type 0=Dec
unsigned char ctcmpdtf5=0;						//left
unsigned char ctcmpdtf6=0;						//Use Compare 0=use
/*
unsigned char cc_clistedit1 = 0;
unsigned char cc_clistedit2 = 0;

unsigned char cc_csrchrb1 = 0;
unsigned char cc_csrchrb2 = 0;
unsigned char cc_csrchrb3 = 0;

unsigned char ctrestoreaf=1;			//Cheat Apply Flag
int ctrestnum=0;					//

DWORD ctcmp2datarf;				//Compare radio botton
int ctcmpflag2=0;
*/
/*
unsigned char ctcmpflagd[0x2800];	//Search

unsigned char ctcmp2flag[0x800];	//flag
unsigned char ctcmp2data[0x800];	//Pre Value Buckup

unsigned char ctcmpdt[3][0x800];		//backup space 1-3
unsigned char ctcmpdtS[3][0x2000];		//backup space 1-3

unsigned char ctSramcmp2f[0x2000];	//flag
unsigned char ctSramcmp2d[0x2000];	//Pre Value Backup
*/

//////


///////////


char cheatkeyf;
DWORD cheatkeyadr;
DWORD cheatkeynum;
//char gdboxcancel;

LOGFONT CheatDlgFont;

DWORD CheatDlgFontColor=0;


int CtCmpListOS(HWND , unsigned char *, char *, char *, char *, char *, int );
int MyCheatSeachStr(unsigned char *, char *, DWORD *);
LRESULT CALLBACK MyDlgProc20(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);



int CreateExtraWindow_01(HWND hWnd){
	static HWND hChdWnd;
	static int windf=0;

/*	if(windf){
		DestroyWindow(hChdWnd);
	}
*/
	if(g_extra_window.CheatView[0]){
		SetForegroundWindow(g_extra_window.CheatView[0]);
		return NULL;
	}
	hChdWnd = CreateMyChild(hWnd, MyChild1Proc, "NNNMainMemViewer", "MainMemory Viewer", windf);
	ShowWindow(hChdWnd, SW_SHOW);
	UpdateWindow(hChdWnd);
	windf=1;
	return 0;
}


int CreateExtraWindow_02(HWND hWnd){
	static HWND hChdWnd;
	static int windf=0;

	if(g_extra_window.CheatView[1]){
		SetForegroundWindow(g_extra_window.CheatView[1]);
		return NULL;
	}
	hChdWnd = CreateMyChild(hWnd, MyChild2Proc, "NNNSRAMViewer", "SRAM Viewer", windf);
	ShowWindow(hChdWnd, SW_SHOW);
	UpdateWindow(hChdWnd);
	windf=1;
	return 0;
}


int CreateExtraWindow_03(HWND hWnd){
	static HWND hChdWnd;
	static int windf=0;

	if(g_extra_window.CheatView[2]){
		SetForegroundWindow(g_extra_window.CheatView[2]);
		return NULL;
	}
	hChdWnd = CreateMyChild(hWnd, MyChild3Proc, "NNNMapperMemViewer", "Mapper Extra Memory Viewer", windf);
	ShowWindow(hChdWnd, SW_SHOW);
	UpdateWindow(hChdWnd);
	windf=1;
	return 0;
}



int CreateExtraWindow_04(HWND hWnd){
	static HWND hChdWnd;
	static int windf=0;

	if(g_extra_window.CheatView[3]){
		SetForegroundWindow(g_extra_window.CheatView[3]);
		return NULL;
	}
	hChdWnd = CreateMyChild(hWnd, MyChild4Proc, "NNNAllMemViewer", "All Memory Viewer", windf);
	ShowWindow(hChdWnd, SW_SHOW);
	UpdateWindow(hChdWnd);
	windf=1;
	return 0;
}


HWND CreateMyChild(HWND hWnd, WNDPROC ChildProc,LPCTSTR szChildName, LPCTSTR title, int rflag){
    HWND hChild;
    WNDCLASSEX wc;
//	static int flag=0;
	RECT rc;
	HINSTANCE hInst;

	hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	GetWindowRect(hWnd, &rc);

	if(rflag==0)
	{
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_VREDRAW | CS_HREDRAW;
		wc.lpfnWndProc = ChildProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.lpszMenuName = NULL; //MAKEINTRESOURCE(IDR_MENUCT);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszClassName = (LPCSTR)szChildName;
		wc.hIconSm = (HICON)LoadImage(g_main_instance,MAKEINTRESOURCE(IDI_ICON29),IMAGE_ICON  , 0, 0,LR_DEFAULTCOLOR);
		if(RegisterClassEx(&wc) == 0) {
			return NULL;
		}
	}
//	flag=1;
#if 0
	hChild = CreateWindowEx(NULL, szChildName, title, WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU| WS_VSCROLL,
		rc.left+100, rc.top+100, 500, 240, hWnd, LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENUCT)), hInst, NULL);
#else
	hChild = CreateWindowEx(WS_EX_TOOLWINDOW/* NULL*/, szChildName, title, WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU| WS_VSCROLL,
		rc.left+100, rc.top+100, 500, 280, NULL/*hWnd*/, /*LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENUCT))*/NULL, hInst, NULL);
#endif
	return hChild;
}

/*
void SetStatusBarWindow_Address(HWND hStatus, int address){
	char str[32];
	wsprintf(str, "0x%06x", address);
	SendMessage(hStatus, SB_SETTEXT, 255 | 0, (WPARAM)(LPSTR)str);
}
*/

void SetWindowTitle_Address(HWND hWnd, char *Title, int address){
	char str[64];
	wsprintf(str, "%s - [0x%06X]", Title, address);
	SetWindowText(hWnd, str);
}


LRESULT CALLBACK MyChild1Proc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp){

	static HINSTANCE hInst;
	static unsigned char *mhexram, *ctlcmpflag=NULL;
//	char *pstr="    : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F : 0123456789ABCDEF";
	static My_EditView *EditView = NULL;
	static char *Title = "MainMemory Viewer";
	static int Frame_Count=0;

	switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT paint;
				HDC hdc = BeginPaint(hChild, &paint);
				if(emu && EditView)
					EditView->DrawBinEdit(hdc);
				EndPaint(hChild, &paint);	
			}
			break;
		case WM_SIZE:
			if(emu && EditView){
				EditView->SetWindowSize(LOWORD(lp), HIWORD(lp));
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_SETFOCUS:
			if(EditView)
				EditView->SetFocus();
			break;
		case WM_KILLFOCUS:
			if(EditView)
				EditView->KillFocus();
			break;
		case WM_VSCROLL:
			if(emu && EditView){
				EditView->VScroll((DWORD)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_MOUSEWHEEL:
			{
				if((int)wp<0)
					SendMessage(hChild, WM_VSCROLL, SB_LINEDOWN, 0);
				else
					SendMessage(hChild, WM_VSCROLL,  SB_LINEUP, 0);
				if(EditView)
					SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_LBUTTONDOWN:
			if(emu && EditView){
				EditView->LButtonDown(LOWORD(lp), HIWORD(lp));
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_KEYDOWN:
			if(emu && EditView){
				EditView->KeyDown((char)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_CHAR:
			if(emu && EditView){
				EditView->CharKeyDown((char)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_EXTRAWIN_01:
			{
				mhexram = NULL;
				int memsize=0;
				if(emu){
					memsize = emu->GetMainMemp(&mhexram);
					EditView->SetMemPointer(mhexram, memsize);
					SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
				}
			}
			InvalidateRect(hChild, NULL, TRUE);
			break;
		case WM_EXTRAWIN_02:
			break;
		case WM_EXTRAWIN_03:
			if(!Frame_Count)
			{
//				if(++Frame_IntervalCounter == Frame_Interval)
				if(EditView)
				{
					HDC hdc = GetDC(hChild);
					EditView->DrawBinEdit(hdc);
					ReleaseDC(hChild, hdc);	
//					Frame_IntervalCounter=0;
				}
				Frame_Count = NESTER_settings.nes.preferences.nViewerReInterval;
			}
			else{
				Frame_Count--;
			}
			break;
//		case WM_RBUTTONDOWN:
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

        case WM_COMMAND:
			switch (LOWORD(wp)){
					case IDS_STRING_EVIEWMS_1:			//
						if(EditView)
						{
							POINT Color;
							if(IDOK==DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VIEW_COLORNDIALOG), hChild, (DLGPROC)MyMemViewColorDlg, (long)&Color)){
								EditView->SetColorDisplay(Color.x, Color.y);
							}
						}
						break;
			}
			break;
#if 0
			if(emu){
				int i,a;
			switch (LOWORD(wp)){
					case IDM_CT1:			//Search Dec
						if(IDCANCEL ==DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG1), hChild, (DLGPROC)MyCheatDlg1))
							break;
						for(i=0,a=0;i<0x800 ; ++i){
							if(MyCheatAdr){			//2Byte
								if(i!=0x7ff){
									if( LOBYTE(MyCheatNum)== mhexram[i] && HIBYTE(MyCheatNum)== mhexram[i+1]){
										ctlcmpflag[i]=1;
									}
									else
										ctlcmpflag[i]=0;
								}
							}
							else{					//1byte
								if( MyCheatNum== mhexram[i]){
										ctlcmpflag[i]=1;
									}
									else
										ctlcmpflag[i]=0;
							}
						}
/*
						for(;i<0x2800 ; ++i){
							if(MyCheatAdr){			//2Byte
								if(i!=0x27ff){
									if( LOBYTE(MyCheatNum)== mhexram[i] && HIBYTE(MyCheatNum)== mhexram[i+1]){
										ctlcmpflag[i]=1;
									}
									else
										ctlcmpflag[i]=0;
								}
							}
							else{					//1byte
								if( MyCheatNum== mhexram[i]){
										ctlcmpflag[i]=1;
									}
									else
										ctlcmpflag[i]=0;
							}
						}
*/
						SResulttext=ctlcmpflag;
						ctcmpdata2=MyCheatNum;
						if(DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG2), hChild, (DLGPROC)MyCheatDlg2)){
							ctcmpflag=1;
							memcpy(ctcmpflagd, ctlcmpflag, 0x0800);
							ctcmpdata=MyCheatNum;
						}
						return 0;
						break;
					case IDM_CT2:			//Change
						SResulttext=mhexram;
						DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG3), hChild, (DLGPROC)MyCheatDlg3);
//						pophexst(hEdit, mhexram);
						return 0;
						break;
					case IDM_CT3:			//Search
						if(IDCANCEL ==DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG1), hChild, (DLGPROC)MyCheatDlg4))
							break;
						for(i=0,a=0;i<0x800 ; ++i){
							if(MyCheatAdr){			//2Byte
								if(i!=0x7ff){
									if( LOBYTE(MyCheatNum)== mhexram[i] && HIBYTE(MyCheatNum)== mhexram[i+1]){
										ctlcmpflag[i]=1;
									}
									else
										ctlcmpflag[i]=0;
								}
							}
							else{					//1Byte
								if( MyCheatNum== mhexram[i]){
										ctlcmpflag[i]=1;
									}
									else
										ctlcmpflag[i]=0;
							}
						}
/*
						for(i;i<0x2800 ; ++i){
							if(MyCheatAdr){			//2Byte
								if(i!=0x27ff){
									if( LOBYTE(MyCheatNum)== mhexram[i] && HIBYTE(MyCheatNum)== mhexram[i+1]){
										ctlcmpflag[i]=1;
									}
									else
										ctlcmpflag[i]=0;
								}
							}
							else{					//1Byte
								if( MyCheatNum== mhexram[i]){
										ctlcmpflag[i]=1;
									}
									else
										ctlcmpflag[i]=0;
							}
						}
*/
						SResulttext=ctlcmpflag;
						ctcmpdata2=MyCheatNum;
						if(DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG2), hChild, (DLGPROC)MyCheatDlg2)){
							ctcmpflag=1;
							ctcmpdata=MyCheatNum;
							memcpy(ctcmpflagd, ctlcmpflag, 0x0800);
						}
						return 0;
						break;
					case IDM_CT4:
						SResulttext=mhexram;
						DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG3), hChild, (DLGPROC)MyCheatDlg5);
						return 0;
						break;
					case IDM_CT5:		//Reflesh
						emu->readBaram(mhexram);
//						emu->readSraram(mhexram+0x800);
//						pophexst(hEdit, mhexram);
						return 0;
						break;
					case IDM_CT6:		//Search Strings
						{
//						char instr[17], rstr[20];
//						DWORD address=0;
						SResulttext=mhexram;
						DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG20), hChild, (DLGPROC)MyDlgProc20);
						}
						return 0;
						break;
				}
			}
			break;
#endif
			break;
		case WM_CREATE:
			hInst = g_main_instance; //(HINSTANCE)GetWindowLong(GetParent(hChild), GWL_HINSTANCE);
			g_extra_window.CheatView[0] = hChild;
			SetWindowTitle_Address(hChild, Title, 0);
			Frame_Count=0;
			{
				RECT rc;
				mhexram = NULL;
				int memsize=0;
				if(emu){
					memsize = emu->GetMainMemp(&mhexram);
				}
				GetClientRect(hChild, &rc);
				EditView = new My_EditView(hChild, hInst, &rc, &Default_Font, mhexram, memsize);
			}
//			ctlcmpflag = (unsigned char *)malloc(0x801);
/*
			GetClientRect(hChild, &rc);
			hFont = CreateFontIndirect(&CheatDlgFont);
//			ListView_SetTextColor(phList, CheatDlgFontColor);
//			SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
//			SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
*/
//			mhexram=(unsigned char *)malloc(0x801+0x2000);
//			emu->readBaram(mhexram);
//			emu->readSraram(mhexram+0x800);
//			pophexst(hEdit, mhexram);

			return 0;
			break;
		case WM_DESTROY:
			g_extra_window.CheatView[0] = NULL;
			if(EditView){
				delete EditView;
				EditView = NULL;
			}
//			if(ctlcmpflag)
//				free(ctlcmpflag);
//			free(mhexram);
			return 0;
			break;
		default:
			return (DefWindowProc(hChild, msg, wp, lp));
	}
	return 0L;
}



LRESULT CALLBACK MyChild2Proc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp){

	static HINSTANCE hInst;
	static unsigned char *mhexram, *ctlcmpflag=NULL;
//	char *pstr="    : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F : 0123456789ABCDEF";
	static My_EditView *EditView = NULL;
	static char *Title = "SRAM Viewer";
//	static int Frame_Interval=5, Frame_IntervalCounter=0;
	static int Frame_Count=0;

	switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT paint;
				HDC hdc = BeginPaint(hChild, &paint);
				if(emu)
					EditView->DrawBinEdit(hdc);
				EndPaint(hChild, &paint);	
			}
			break;
		case WM_SIZE:
			if(emu){
				EditView->SetWindowSize(LOWORD(lp), HIWORD(lp));
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_SETFOCUS:
			EditView->SetFocus();
			break;
		case WM_KILLFOCUS:
			EditView->KillFocus();
			break;
		case WM_VSCROLL:
			if(emu){
				EditView->VScroll((DWORD)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_LBUTTONDOWN:
			if(emu){
				EditView->LButtonDown(LOWORD(lp), HIWORD(lp));
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_KEYDOWN:
			if(emu){
				EditView->KeyDown((char)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_CHAR:
			if(emu){
				EditView->CharKeyDown((char)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_EXTRAWIN_01:
			{
				mhexram = NULL;
				int memsize=0;
				if(emu){
					memsize = emu->GetSramMemp(&mhexram);
					EditView->SetMemPointer(mhexram, memsize);
					SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
				}
			}
			InvalidateRect(hChild, NULL, TRUE);
			break;
		case WM_EXTRAWIN_02:
			break;
		case WM_EXTRAWIN_03:
			if(!Frame_Count)
			{
//				if(++Frame_IntervalCounter == Frame_Interval)
				if(EditView)
				{
					HDC hdc = GetDC(hChild);
					EditView->DrawBinEdit(hdc);
					ReleaseDC(hChild, hdc);	
//					Frame_IntervalCounter=0;
				}
				Frame_Count = NESTER_settings.nes.preferences.nViewerReInterval;
			}
			else{
				Frame_Count--;
			}
			break;
        case WM_COMMAND:
			switch (LOWORD(wp)){
					case IDS_STRING_EVIEWMS_1:			//
						{
							POINT Color;
							if(IDOK==DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VIEW_COLORNDIALOG), hChild, (DLGPROC)MyMemViewColorDlg, (long)&Color)){
								EditView->SetColorDisplay(Color.x, Color.y);
							}
						}
						break;
			}
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
		case WM_CREATE:
			hInst = g_main_instance; //(HINSTANCE)GetWindowLong(GetParent(hChild), GWL_HINSTANCE);
			g_extra_window.CheatView[1] = hChild;
			Frame_Count=0;
			{
				RECT rc;
				mhexram = NULL;
				int memsize=0;
				if(emu){
					memsize = emu->GetSramMemp(&mhexram);
				}
				GetClientRect(hChild, &rc);
				EditView = new My_EditView(hChild, hInst, &rc, &Default_Font, mhexram, memsize);
			}
			SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
//			ctlcmpflag = (unsigned char *)malloc(0x801);
/*
			GetClientRect(hChild, &rc);
			hFont = CreateFontIndirect(&CheatDlgFont);
//			ListView_SetTextColor(phList, CheatDlgFontColor);
//			SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
//			SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
*/
//			mhexram=(unsigned char *)malloc(0x801+0x2000);
//			emu->readBaram(mhexram);
//			emu->readSraram(mhexram+0x800);
//			pophexst(hEdit, mhexram);

			return 0;
			break;
		case WM_DESTROY:
			g_extra_window.CheatView[1] = NULL;
			delete EditView;
			EditView = NULL;
//			if(ctlcmpflag)
//				free(ctlcmpflag);
//			free(mhexram);
			return 0;
			break;
		default:
			return (DefWindowProc(hChild, msg, wp, lp));
	}
	return 0L;
}


LRESULT CALLBACK MyChild3Proc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp){

	static HINSTANCE hInst;
	static unsigned char *mhexram, *ctlcmpflag=NULL;
//	char *pstr="    : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F : 0123456789ABCDEF";
	static My_EditView *EditView = NULL;
	static char *Title = "Mapper extra memory Viewer";
//	static int Frame_Interval=5, Frame_IntervalCounter=0;
	static int Frame_Count=0;

	switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT paint;
				HDC hdc = BeginPaint(hChild, &paint);
				if(emu)
					EditView->DrawBinEdit(hdc);
				EndPaint(hChild, &paint);	
			}
			break;
		case WM_SIZE:
			if(emu){
				EditView->SetWindowSize(LOWORD(lp), HIWORD(lp));
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_SETFOCUS:
			EditView->SetFocus();
			break;
		case WM_KILLFOCUS:
			EditView->KillFocus();
			break;
		case WM_VSCROLL:
			if(emu){
				EditView->VScroll((DWORD)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_LBUTTONDOWN:
			if(emu){
				EditView->LButtonDown(LOWORD(lp), HIWORD(lp));
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_KEYDOWN:
			if(emu){
				EditView->KeyDown((char)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_CHAR:
			if(emu){
				EditView->CharKeyDown((char)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_EXTRAWIN_01:
			{
				mhexram = NULL;
				int memsize=0;
				if(emu){
					memsize = emu->GetExtMemp(&mhexram);
					EditView->SetMemPointer(mhexram, memsize);
				}
			}
			InvalidateRect(hChild, NULL, TRUE);
			break;
		case WM_EXTRAWIN_02:
			break;
		case WM_EXTRAWIN_03:
			if(!Frame_Count)
			{
//				if(++Frame_IntervalCounter == Frame_Interval)
				if(EditView)
				{
					HDC hdc = GetDC(hChild);
					EditView->DrawBinEdit(hdc);
					ReleaseDC(hChild, hdc);	
//					Frame_IntervalCounter=0;
				}
				Frame_Count = NESTER_settings.nes.preferences.nViewerReInterval;
			}
			else{
				Frame_Count--;
			}
			break;
        case WM_COMMAND:
			switch (LOWORD(wp)){
					case IDS_STRING_EVIEWMS_1:			//
						{
							POINT Color;
							if(IDOK==DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VIEW_COLORNDIALOG), hChild, (DLGPROC)MyMemViewColorDlg, (long)&Color)){
								EditView->SetColorDisplay(Color.x, Color.y);
							}
						}
						break;
			}
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
		case WM_CREATE:
			hInst = g_main_instance; //(HINSTANCE)GetWindowLong(GetParent(hChild), GWL_HINSTANCE);
			g_extra_window.CheatView[2] = hChild;
			Frame_Count=0;
			{
				RECT rc;
				mhexram = NULL;
				int memsize=0;
				if(emu){
					memsize = emu->GetExtMemp(&mhexram);
				}
				GetClientRect(hChild, &rc);
				EditView = new My_EditView(hChild, hInst, &rc, &Default_Font, mhexram, memsize);
			}
			SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
//			ctlcmpflag = (unsigned char *)malloc(0x801);
/*
			GetClientRect(hChild, &rc);
			hFont = CreateFontIndirect(&CheatDlgFont);
//			ListView_SetTextColor(phList, CheatDlgFontColor);
//			SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
//			SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
*/
//			mhexram=(unsigned char *)malloc(0x801+0x2000);
//			emu->readBaram(mhexram);
//			emu->readSraram(mhexram+0x800);
//			pophexst(hEdit, mhexram);

			return 0;
			break;
		case WM_DESTROY:
			g_extra_window.CheatView[2] = NULL;
			delete EditView;
			EditView = NULL;
//			if(ctlcmpflag)
//				free(ctlcmpflag);
//			free(mhexram);
			return 0;
			break;
		default:
			return (DefWindowProc(hChild, msg, wp, lp));
	}
	return 0L;
}




LRESULT CALLBACK MyChild4Proc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp){

	static HINSTANCE hInst;
	static unsigned char *mhexram, *ctlcmpflag=NULL;
	static My_NES_EditView *EditView = NULL;
	static char *Title = "All memory Viewer";
//	static int Frame_Interval=5, Frame_IntervalCounter=0;

	switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT paint;
				HDC hdc = BeginPaint(hChild, &paint);
				if(emu)
					EditView->DrawBinEdit(hdc);
				EndPaint(hChild, &paint);	
			}
			break;
		case WM_SIZE:
			if(emu){
				EditView->SetWindowSize(LOWORD(lp), HIWORD(lp));
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_SETFOCUS:
			EditView->SetFocus();
			break;
		case WM_KILLFOCUS:
			EditView->KillFocus();
			break;
		case WM_VSCROLL:
			if(emu){
				EditView->VScroll((DWORD)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_LBUTTONDOWN:
			if(emu){
				EditView->LButtonDown(LOWORD(lp), HIWORD(lp));
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_KEYDOWN:
			if(emu){
				EditView->KeyDown((char)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_CHAR:
			if(emu){
				EditView->CharKeyDown((char)wp);
				SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
			}
			break;
		case WM_EXTRAWIN_01:
			{
				nes6502_context Context;
				if(emu){
					emu->GetCPUContext((unsigned char *)&Context);
					EditView->SetMemPointer((unsigned char **)&Context.mem_page);
				}
			}
			InvalidateRect(hChild, NULL, TRUE);
			break;
		case WM_EXTRAWIN_02:
			break;
		case WM_EXTRAWIN_03:
			{
//				if(++Frame_IntervalCounter == Frame_Interval)
				{
					HDC hdc = GetDC(hChild);
					EditView->DrawBinEdit(hdc);
					ReleaseDC(hChild, hdc);	
//					Frame_IntervalCounter=0;
				}
			}
			break;
        case WM_COMMAND:
			switch (LOWORD(wp)){
					case IDS_STRING_EVIEWMS_1:			//
						{
							POINT Color;
							if(IDOK==DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VIEW_COLORNDIALOG), hChild, (DLGPROC)MyMemViewColorDlg, (long)&Color)){
								EditView->SetColorDisplay(Color.x, Color.y);
							}
						}
						break;
			}
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
		case WM_CREATE:
			hInst = g_main_instance; //(HINSTANCE)GetWindowLong(GetParent(hChild), GWL_HINSTANCE);
			g_extra_window.CheatView[3] = hChild;
	//		MessageBox(hChild, "test", "0", MB_OK);
			{
				RECT rc;
				nes6502_context Context;
				memset(&Context, 0, sizeof(Context));
				if(emu){ //nes6502_context
					emu->GetCPUContext((unsigned char *)&Context);
				}
				GetClientRect(hChild, &rc);
				EditView = new My_NES_EditView(hChild, hInst, &rc, &Default_Font, (unsigned char **)&Context.mem_page);
			}
//			MessageBox(hChild, "test", "1", MB_OK);
			SetWindowTitle_Address(hChild, Title, EditView->GetCursorAddress());
//			ctlcmpflag = (unsigned char *)malloc(0x801);
/*
			GetClientRect(hChild, &rc);
			hFont = CreateFontIndirect(&CheatDlgFont);
//			ListView_SetTextColor(phList, CheatDlgFontColor);
//			SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
//			SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
*/
//			mhexram=(unsigned char *)malloc(0x801+0x2000);
//			emu->readBaram(mhexram);
//			emu->readSraram(mhexram+0x800);
//			pophexst(hEdit, mhexram);

			return 0;
			break;
		case WM_DESTROY:
			g_extra_window.CheatView[3] = NULL;
			delete EditView;
			EditView = NULL;
//			if(ctlcmpflag)
//				free(ctlcmpflag);
//			free(mhexram);
			return 0;
			break;
		default:
			return (DefWindowProc(hChild, msg, wp, lp));
	}
	return 0L;
}


#if 0
int pophexst(HWND hEdit, unsigned char *hexram)
{
	int f=0, y=0, b=0, x, a=0;
	unsigned int n=0;
	char temphex[4];
	char charatmp[17];
	unsigned char headch[75*(0x85+512)+1];
//	char adressstr[]="    : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F : 0123456789ABCDEF";
//	for(a=0; adressstr[a]; ++a){
//		headch[a]=adressstr[a];
//	}
//	headch[a++]=0x0d;
//	headch[a++]=0x0a;
//	headch[a++]= ' ';
//	for(x=0;x<71;++x){
//		headch[a++]='-';
//	}
	strcpy((char *)headch, "MAIN");
	a=4;
	do{
		headch[a++]=0x0d;
		headch[a++]=0x0a;
		wsprintf(temphex, "%02X", y);
		headch[a++]=' ';
		headch[a++]=temphex[0];
		headch[a++]=temphex[1];
		headch[a++]='0';
		headch[a++]=':';
		for(x=0;x<16;++x){
			charatmp[x]=hexram[b+x];		
		}
		for(x=0;x<16;++x){
			wsprintf(temphex, " %02X", hexram[b++]);
			headch[a++]=temphex[0];
			headch[a++]=temphex[1];
			headch[a++]=temphex[2];
		}
		headch[a++]=' ';
		headch[a++]=':';
		headch[a++]=' ';
		for(x=0;x<16;++x){
			if((unsigned char)charatmp[x]<0x20 || (unsigned char)charatmp[x]>0x80)
					charatmp[x]=' ';
		}
		for(x=0;x<16;++x){
			headch[a++]=charatmp[x];
		}
	}
	while(0x7F > y++);
	y=0;
	headch[a++]=0x0d, headch[a++]=0x0a;
	headch[a++]=0x0d, headch[a++]=0x0a;
	headch[a++]='S',headch[a++]='R',headch[a++]='A',headch[a++]='M'; 
	do{
		headch[a++]=0x0d;
		headch[a++]=0x0a;
		wsprintf(temphex, "%03X", y);
		headch[a++]=' ';
		headch[a++]=temphex[0];
		headch[a++]=temphex[1];
		headch[a++]=temphex[2];
		headch[a++]='0';
		headch[a++]=':';
		for(x=0;x<16;++x){
			charatmp[x]=hexram[b+x];		
		}
		for(x=0;x<16;++x){
			wsprintf(temphex, " %02X", hexram[b++]);
			headch[a++]=temphex[0];
			headch[a++]=temphex[1];
			headch[a++]=temphex[2];
		}
		headch[a++]=' ';
		headch[a++]=':';
		headch[a++]=' ';
		for(x=0;x<16;++x){
			if((unsigned char)charatmp[x]<0x20 || (unsigned char)charatmp[x]>0x80)
					charatmp[x]=' ';
		}
		for(x=0;x<16;++x){
			headch[a++]=charatmp[x];
		}
	}
	while(0x1ff > y++);
	headch[a]='\0';
	SetWindowText(hEdit, (const char *)headch);
	return 0;
}
#endif

//Search  IDD_CTDIALOG1
LRESULT CALLBACK MyCheatDlg1(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hEdit;

    switch(msg) {
        case WM_INITDIALOG:
			hEdit = GetDlgItem(hDlgWnd, IDC_CT1EDIT1);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CT1RADIO1), BM_SETCHECK, (WPARAM)TRUE, 0L);
            return TRUE;
		   break;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					MyCheatNum = GetDlgItemInt(hDlgWnd, IDC_CT1EDIT1, NULL, FALSE);
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT1RADIO1), BM_GETCHECK, (WPARAM)TRUE, 0L))
						MyCheatAdr=0;
					else
						MyCheatAdr=1;
					EndDialog(hDlgWnd, IDOK);
                   return TRUE;
				   break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
    }
    return FALSE;
}


//Seach Result IDD_CTDIALOG2
LRESULT CALLBACK MyCheatDlg2(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hEdit;
//	int i,a,j;
//	unsigned char *outtext;
//	char temphex[5];
#if 0
    switch(msg) {
        case WM_INITDIALOG:
			outtext=(unsigned char *)malloc(0x3010*5);
			hEdit = GetDlgItem(hDlgWnd, IDC_CT2EDIT1);
			for(i=0,a=0; i<0x800; ++i){
				if(SResulttext[i]){
					wsprintf(temphex, "%04X", i);
					outtext[a++]=temphex[0];
					outtext[a++]=temphex[1];
					outtext[a++]=temphex[2];
					outtext[a++]=temphex[3];
					outtext[a++]=0x0d;
					outtext[a++]=0x0a;
				}
			}
			outtext[a++]=0x0d, outtext[a++]=0x0a;
			outtext[a++]='S', outtext[a++]='R', outtext[a++]='A', outtext[a++]='M';
			outtext[a++]=0x0d, outtext[a++]=0x0a;
			for(j=0; i<0x2800; ++i, ++j){
				if(SResulttext[i]){
					wsprintf(temphex, "%04X", j);
					outtext[a++]=temphex[0];
					outtext[a++]=temphex[1];
					outtext[a++]=temphex[2];
					outtext[a++]=temphex[3];
					outtext[a++]=0x0d;
					outtext[a++]=0x0a;
				}
			}
			outtext[a]='\0';
			SetWindowText(hEdit,(const char *) outtext);
			free(outtext);
            return TRUE;
		   break;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDC_CT2BUTTON1:			//Pre Compare
            	{
            		char str[64];
	          		LoadString(g_main_instance, IDS_STRING_CHTM_10 , str, 64);

					a=0;
					outtext=(unsigned char *)malloc(0x3021);
					wsprintf((char *)outtext, str, ctcmpdata ,ctcmpdata2);
					a+=20;
					wsprintf((char *)outtext, "address\r\n");
					a+=10;
					for(i=0; i<0x2800; ++i){
						if(SResulttext[i]==1 && ctcmpflagd[i]==1){
							wsprintf(temphex, "%04X", i);
							outtext[a++]=temphex[0];
							outtext[a++]=temphex[1];
							outtext[a++]=temphex[2];
							outtext[a++]=temphex[3];
							outtext[a++]=0x0d;
							outtext[a++]=0x0a;
						}
					}
					outtext[a]='\0';
					SetWindowText(hEdit,(const char *) outtext);
					free(outtext);
				}
                  return TRUE;
				   break;
                case IDC_CT2BUTTON2:			//Save End
					EndDialog(hDlgWnd, 1);
                   return TRUE;
				   break;
                case IDC_CT2BUTTON3:			//End
					EndDialog(hDlgWnd, 0);
                   return TRUE;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDOK);
			break;
    }
#endif
    return FALSE;
}

//Value Change  IDD_CTDIALOG3
LRESULT CALLBACK MyCheatDlg3(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hEdit, hEdit2;
	static char adtext[10]="", numtext[20]="";
//	DWORD adhex;
#if 0
    switch(msg) {
        case WM_INITDIALOG:
			hEdit = GetDlgItem(hDlgWnd, IDC_CT3EDIT1);
			hEdit2 = GetDlgItem(hDlgWnd, IDC_CT3EDIT2);
			SetWindowText(hEdit, adtext);
			SetWindowText(hEdit2, numtext);
            return TRUE;
			break;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText(hDlgWnd, IDC_CT3EDIT1, (LPTSTR)adtext, 10);
					if(!MyStrAtoh(adtext, &adhex)){
						char str[64];
						LoadString(g_main_instance, IDS_STRING_CHTM_01 , str, 64);
						MessageBox(hDlgWnd,(LPCSTR)str,(LPCSTR)"Error",MB_OK);
						break;
					}
					GetDlgItemText(hDlgWnd, IDC_CT3EDIT2, (LPTSTR)numtext, 20);
					MyCheatNum = GetDlgItemInt(hDlgWnd, IDC_CT3EDIT2, NULL, FALSE);
//					SResulttext[adhex & 0x7ff] = (unsigned char)MyCheatNum& 0xff;
					if(!emu)
						EndDialog(hDlgWnd, IDCANCEL);
					if(adhex>=0x10000)
						emu->WriteSroneb(adhex,(unsigned char)MyCheatNum);
					else
						emu->WriteBoneb(adhex,(unsigned char)MyCheatNum);
//					EndDialog(hDlgWnd, IDOK);
                   return TRUE;
				   break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			break;
    }
#endif
    return FALSE;
}


//Seach Hex  IDD_CTDIALOG1
LRESULT CALLBACK MyCheatDlg4(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hEdit;
	char hextext[20];
	DWORD adhex;

    switch(msg) {
        case WM_INITDIALOG:
			hEdit = GetDlgItem(hDlgWnd, IDC_CT1EDIT1);
    	{
    		char str[64];
    		LoadString(g_main_instance, IDS_STRING_CHTD_10 , str, 64);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_CT1STATIC2), str);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CT1RADIO1), BM_SETCHECK, (WPARAM)TRUE, 0L);
    	}
            return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText(hDlgWnd, IDC_CT1EDIT1, (LPTSTR)hextext, 19);
					if(!MyStrAtoh(hextext, &adhex))
						EndDialog(hDlgWnd, IDCANCEL);
					MyCheatNum=adhex;
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT1RADIO1), BM_GETCHECK, (WPARAM)TRUE, 0L))
						MyCheatAdr=0;
					else
						MyCheatAdr=1;
					EndDialog(hDlgWnd, IDOK);
                   return TRUE;
				   break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
    }
    return FALSE;
}


//Change Value(Hex)  IDD_CTDIALOG3
LRESULT CALLBACK MyCheatDlg5(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hEdit, hEdit2;
	static char adtext[10]="", numtext[20]="";
	DWORD adhex;

    switch(msg) {
        case WM_INITDIALOG:
			hEdit = GetDlgItem(hDlgWnd, IDC_CT3EDIT1);
			hEdit2 = GetDlgItem(hDlgWnd, IDC_CT3EDIT2);
			SetWindowText(hEdit, adtext);
			SetWindowText(hEdit2, numtext);
			{
	    		char str[64];
	    		LoadString(g_main_instance, IDS_STRING_CHTD_10 , str, 64);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_CT3STATIC2), str);
			}
            return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText(hDlgWnd, IDC_CT3EDIT1, (LPTSTR)adtext, 10);
					if(!MyStrAtoh(adtext, &adhex))
						EndDialog(hDlgWnd, IDCANCEL);
					MyCheatAdr=adhex;
					GetDlgItemText(hDlgWnd, IDC_CT3EDIT2, (LPTSTR)numtext, 20);
					if(!MyStrAtoh(numtext, &adhex))
						EndDialog(hDlgWnd, IDCANCEL);
//					SResulttext[MyCheatAdr & 0x7ff] = (unsigned char)adhex& 0xff;
					if(!emu)
						EndDialog(hDlgWnd, IDCANCEL);
					if(adhex>=0x10000)
						emu->WriteSroneb(MyCheatAdr,(unsigned char)adhex);
					else
						emu->WriteBoneb(MyCheatAdr,(unsigned char)adhex);
//					EndDialog(hDlgWnd, IDOK);
                   return TRUE;
				   break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			break;
    }
    return FALSE;
}




void InitCheatListEditDlg(HWND hList){
	char adtext[40], address[8];
    LV_ITEM item;
	ListView_DeleteAllItems(hList);
	for(int i=0; i<g_Cheat_Info.ctrestnum; ++i){
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = g_Cheat_Info.ctrestds[i].ctrescomm;
		item.iItem = i;
		item.iSubItem = 0;
		item.lParam = i;
		ListView_InsertItem(hList, &item);			//Comment
		item.mask = LVIF_TEXT;
		wsprintf(adtext, "%04X", g_Cheat_Info.ctrestds[i].ctresadd);	//Hex
		item.pszText = adtext;
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem(hList, &item);				//Address
		if(g_Cheat_Info.ctrestds[i].ctressflag){
			switch(g_Cheat_Info.ctrestds[i].ctresssize){
			case 0:
				wsprintf(adtext, "%02X", g_Cheat_Info.ctrestds[i].ctresdata);
				break;
			case 1:
				wsprintf(adtext, "%04X", g_Cheat_Info.ctrestds[i].ctresdata);
				address[0]='2';
				break;
			case 2:
				wsprintf(adtext, "%08X", g_Cheat_Info.ctrestds[i].ctresdata);
				address[0]='4';
				break;
			case 3:
				wsprintf(adtext, "%06X", g_Cheat_Info.ctrestds[i].ctresdata);
				address[0]='3';
				break;
			}
		}
		else{
			wsprintf(adtext, "%u", g_Cheat_Info.ctrestds[i].ctresdata);
		}
		item.pszText = adtext;
		item.iItem = i;
		item.iSubItem = 2;
		ListView_SetItem(hList, &item);				//Value
		
		if(g_Cheat_Info.ctrestds[i].ctressflag)
			adtext[1]='6';
		else
			adtext[1]='0';
		
		adtext[0]='1';
		adtext[2]='\0';
		item.pszText = adtext;
		item.iItem = i;
		item.iSubItem = 3;
		ListView_SetItem(hList, &item);				//Type
		if(g_Cheat_Info.ctrestds[i].ctrescflag){
			ListView_SetItemState(hList, i, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
		}
		else{
			ListView_SetItemState(hList, i, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
		}
		switch(g_Cheat_Info.ctrestds[i].ctresssize){
		case 0:
			address[0]='1';
			break;
		case 1:
			address[0]='2';
			break;
		case 2:
			address[0]='4';
			break;
		case 3:
			address[0]='3';
			break;
		}
		address[1]='\0';
		item.pszText = address;
		item.iItem = i;
		item.iSubItem = 4;
		ListView_SetItem(hList, &item);				//Size
	}
	
}



void GetCheatListEditDlg(HWND hDlgWnd, HWND hList){
	char adtext[40];
	int i=-1,j=0, k;
	DWORD adhex;
	if(g_Cheat_Info.ctrestnum != ListView_GetItemCount(hList))
		return;
	while((i=ListView_GetNextItem(hList, i, LVNI_ALL)) != -1){
		g_Cheat_Info.ctrestds[j].ctrescomm[0]=0;
		ListView_GetItemText(hList, i, 0, g_Cheat_Info.ctrestds[j].ctrescomm, 32);	//Comment
		ListView_GetItemText(hList, i, 1, adtext, 6);	//Address
		MyStrAtoh(adtext, &g_Cheat_Info.ctrestds[j].ctresadd);
		ListView_GetItemText(hList, i, 4, adtext, 3);	//size
		if(adtext[0]=='4')
			k=2;
		else if(adtext[0]=='3')
			k=3;
		else
			k=(int)adtext[0]-'1';
		ListView_GetItemText(hList, i, 3, adtext, 3);	//Type
		if(adtext[1]=='0'){
			g_Cheat_Info.ctrestds[j].ctressflag=0;
			ListView_GetItemText(hList, i, 2, adtext, 18);	//Dec
			adhex=atoi(adtext);
		}
		else{
			g_Cheat_Info.ctrestds[j].ctressflag=1;
			ListView_GetItemText(hList, i, 2, adtext, 18);	//Hex
			MyStrAtoh(adtext, &adhex);
		}
		if(k==0)
			adhex&=0xff;
		else if(k==1)
			adhex&=0xffff;
		g_Cheat_Info.ctrestds[j].ctresdata =adhex;
		g_Cheat_Info.ctrestds[j].ctrescflag=(unsigned char)ListView_GetCheckState(hList, i);
		g_Cheat_Info.ctrestds[j].ctresssize=(char)k;
		j++;
	}
	g_Cheat_Info.cc_clistedit1 = (unsigned char)SendMessage(GetDlgItem(hDlgWnd, IDC_CT5RADIO2), BM_GETCHECK, (WPARAM)TRUE, 0L);
	if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT5RADIO3), BM_GETCHECK, (WPARAM)TRUE, 0L))
		g_Cheat_Info.cc_clistedit2=0;
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT5RADIO4), BM_GETCHECK, (WPARAM)TRUE, 0L))
		g_Cheat_Info.cc_clistedit2=1;
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT5RADIO5), BM_GETCHECK, (WPARAM)TRUE, 0L))
		g_Cheat_Info.cc_clistedit2=2;
	else
		g_Cheat_Info.cc_clistedit2=3;
	g_Cheat_Info.ctrestnum=j;
}


void CheatListOnGetDispInfo(NMLVDISPINFO *pInfo){
	struct ctrestoren *data;
	char str[512];
/*
	if(g_Cheat_Info.cc_clistedit4){
		data = &g_Cheat_Info.ctrestds[pInfo->item.iItem].pSubcode[g_Cheat_Info.cc_clistedit4-1];
	}
	else{
		data = &g_Cheat_Info.ctrestds[pInfo->item.iItem];
	}
*/
	data = &g_Cheat_Info.ctrestds[pInfo->item.iItem];

	if(pInfo->item.mask & LVIF_TEXT){
		switch (pInfo->item.iSubItem){
		  case 0:	// comment
			pInfo->item.pszText = data->ctrescomm;
			break;
		  case 1:	//	address
			wsprintf(str, "%04X", data->ctresadd);
			strcpy(pInfo->item.pszText, str);
//			pInfo->item.pszText = str;
			break;
		  case 2:	//	data
			{
				char tstr[8];
				if(data->ctressflag){
					wsprintf(tstr, "%%0%uX", data->ctresssize*2);
				}
				else{
					strcpy(tstr, "%u");
				}
				wsprintf(str, tstr, data->ctresdata);
				strcpy(pInfo->item.pszText, str);
//				pInfo->item.pszText = str;
			}
			break;
		  case 3:	//	type
			if(data->ctressflag)
				strcpy(str, "10");
			else
				strcpy(str, "16");
			strcpy(pInfo->item.pszText, str);
//			pInfo->item.pszText = str;
			break;
		  case 4:	//	size
			wsprintf(str, "%u", data->ctresssize);
			strcpy(pInfo->item.pszText, str);
//			pInfo->item.pszText = str;
			break;
		  case 5:	//	opcode
			wsprintf(str, "%02X", data->opcode);
			strcpy(pInfo->item.pszText, str);
//			pInfo->item.pszText = str;
			break;
		  case 6:	//	subcode num
			wsprintf(str, "%u", data->nSubcode);
			strcpy(pInfo->item.pszText, str);
//			pInfo->item.pszText = str;
			break;
		}
	}
}


void CheatListOwnerDraw(LPDRAWITEMSTRUCT lpds){
	HDC hDC;
	HBRUSH hBrBkgnd;
	HBRUSH hBrLine;

	RECT rc;
//	int i;

	hDC = lpds->hDC;
#if 0
	hBrBkgnd = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	SetRect(&rc, lpds->rcItem.left/*+GetSystemMetrics(SM_CXMENUCHECK)*/,lpds->rcItem.top, lpds->rcItem.right, lpds->rcItem.bottom);
	FillRect(hDC, &rc, hBrBkgnd);
    DeleteObject(hBrBkgnd);
#endif
	{
		HWND hList=lpds->hwndItem;
		HFONT MenuStrFont = CreateFontIndirect(&CheatDlgFont);//(HFONT)GetStockObject(DEFAULT_GUI_FONT);
		hBrBkgnd = CreateSolidBrush(GetSysColor((lpds->itemState & ODS_SELECTED)?COLOR_HIGHLIGHT : COLOR_WINDOW));

		struct ctrestoren *data = &g_Cheat_Info.ctrestds[lpds->itemID];
		HFONT OldFont = (HFONT)SelectObject(hDC, MenuStrFont);

		if(!data->ctrescflag){
			hBrLine = CreateSolidBrush(GetSysColor((lpds->itemState & ODS_SELECTED)?COLOR_WINDOW :COLOR_HIGHLIGHT ));;
		}
		if(lpds->itemState & ODS_SELECTED){
			if(data->ctrescflag)
				SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			else
				SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
		}
		else{
			if(data->ctrescflag)
				SetTextColor(hDC, GetSysColor(COLOR_MENUTEXT));
			else
				SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
		}
		SetBkMode(lpds->hDC, TRANSPARENT);
		for(int i=0; i<7; i++){
			char str[128], *p;
			int flag = DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;
			p=str;

			ListView_GetSubItemRect(hList, lpds->itemID, i, LVIR_LABEL/*LVIR_BOUNDS*/, &rc);
			FillRect(hDC, &rc, hBrBkgnd);
			if(!data->ctrescflag){
				RECT trect;
				int ty = (rc.bottom - rc.top) / 2 + rc.top;
				SetRect(&trect, rc.left+4, ty, rc.right-4, ty+2);
				FillRect(hDC, &trect, hBrLine);
			}

			rc.left+=2;
//			ListView_GetColumn(hList, i, &LvColumn);
//			ListView_GetSubItemRect(hList,lpds->itemID,i,LVIR_LABEL,&rc);
//			DrawListItemText(hdc,Text,&rc,LvColumn.fmt);

			switch(i){
			case 0:	// comment
				p = data->ctrescomm;
				flag |= DT_LEFT;
				break;
			case 1:	//	address
				wsprintf(str, "%04X", data->ctresadd);
				flag |= DT_RIGHT;
				break;
			case 2:	//	data
				{
					char tstr[8];
					if(data->ctressflag){
						wsprintf(tstr, "%%0%uX", data->ctresssize*2);
					}
					else{
						strcpy(tstr, "%u");
					}
					wsprintf(str, tstr, data->ctresdata);
					flag |= DT_RIGHT;
				}
				break;
			case 3:	//	type
				if(data->ctressflag)
					strcpy(str, "Hex");
				else
					strcpy(str, "Dec");
				flag |= DT_CENTER;
				break;
			case 4:	//	size
				wsprintf(str, "%u", data->ctresssize);
				flag |= DT_CENTER;
				break;
			case 5:	//	opcode
				wsprintf(str, "%02X", data->opcode);
				flag |= DT_CENTER;
				break;
			case 6:	//	subcode num
				wsprintf(str, "%u", data->nSubcode);
				flag |= DT_CENTER;
				break;
			}
			DrawText(hDC, p, strlen(p), &rc, flag);
		}
		if(!data->ctrescflag){
			DeleteObject(hBrLine);
		}
		DeleteObject(hBrBkgnd);
		SelectObject(hDC, OldFont);
        DeleteObject(MenuStrFont);
	}
}


void GetCheatListEditDlgState(HWND hDlgWnd){
	g_Cheat_Info.cc_clistedit1 = (unsigned char)SendMessage(GetDlgItem(hDlgWnd, IDC_CT5RADIO2), BM_GETCHECK, (WPARAM)TRUE, 0L);
	if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT5RADIO3), BM_GETCHECK, (WPARAM)TRUE, 0L))
		g_Cheat_Info.cc_clistedit2=0;
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT5RADIO4), BM_GETCHECK, (WPARAM)TRUE, 0L))
		g_Cheat_Info.cc_clistedit2=1;
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT5RADIO5), BM_GETCHECK, (WPARAM)TRUE, 0L))
		g_Cheat_Info.cc_clistedit2=2;
	else
		g_Cheat_Info.cc_clistedit2=3;
	int i = IDC_RADIO1;
	g_Cheat_Info.cc_clistedit3 = (unsigned char)SendDlgItemMessage(hDlgWnd, IDC_COMBO1, CB_GETCURSEL, 0L, 0L);
/*	for(; i<=IDC_RADIO20; i++){
		if(SendMessage(GetDlgItem(hDlgWnd, i), BM_GETCHECK, (WPARAM)TRUE, 0L)){
			g_Cheat_Info.cc_clistedit3 = i-IDC_RADIO1;
			break;
		}
	}
*/
}


const int nCheatCodeTable[] = {
	0, 2, 3, 4, 5, 6, 7, 8, 9, 0x0a, 0x0b, 0x0c,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
	0x20, 0x21, 0x22, 0x23,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x0ff
};


void SetCheatDataToDialogItemCombo(HWND hDlgWnd, struct ctrestoren *data){

	int i, cn = data->opcode;
	int ret = -1;

	for(i=0; nCheatCodeTable[i] != 0xff; i++){
		if(nCheatCodeTable[i] == cn){
			ret = i;
			break;
		}
	}
	if(ret == -1){
		ret = SendDlgItemMessage(hDlgWnd, IDC_COMBO1, CB_GETCOUNT, (WPARAM)0, 0L);
		ret--;
//		ret = i-1;
	}
	SendDlgItemMessage(hDlgWnd, IDC_COMBO1, CB_SETCURSEL, (WPARAM)ret, 0L);
}


// cheatdata struct -> dialogitem
void SetCheatDataToDialogItem(HWND hDlgWnd, struct ctrestoren *data){
//	int i;
	char str[16];

/*
	wsprintf(str, "(%u)", data->nSubcode);
	SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC1), str);
*/
	SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK1), BM_SETCHECK, (WPARAM)data->ctrescflag, 0L);

	CheckRadioButton( hDlgWnd, IDC_CT5RADIO1, IDC_CT5RADIO2, IDC_CT5RADIO1+data->ctressflag);
	CheckRadioButton( hDlgWnd, IDC_CT5RADIO3, IDC_CT5RADIO6, IDC_CT5RADIO3+data->ctresssize-1);


	wsprintf(str, "%02X", data->opcode);
	SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT1), str);

//	SetWindowText(GetDlgItem(hDlgWnd, IDC_CT5EDIT2), str);
	SetCheatDataToDialogItemCombo(hDlgWnd, data);
//	g_Cheat_Info.cc_clistedit4 = 0;
	wsprintf(str, "%u / %u", g_Cheat_Info.cc_clistedit4, data->nSubcode);
	SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT2), str);
	SendMessage(GetDlgItem(hDlgWnd, IDC_SPIN1),UDM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,data->nSubcode+1));
	SendMessage(GetDlgItem(hDlgWnd, IDC_SPIN1),UDM_SETPOS,FALSE,(LPARAM)g_Cheat_Info.cc_clistedit4);
	wsprintf(str, "%04X", data->ctresadd);
	SetWindowText(GetDlgItem(hDlgWnd, IDC_CT5EDIT1), str);
	if(data->ctressflag){
		char tstr[8];
		wsprintf(tstr, "%%0%uX", data->ctresssize*2);
		wsprintf(str, tstr, data->ctresdata);
	}
	else
		wsprintf(str, "%u", data->ctresdata);
	SetWindowText(GetDlgItem(hDlgWnd, IDC_CT5EDIT2), str);
	SetWindowText(GetDlgItem(hDlgWnd, IDC_CT5EDIT3), data->ctrescomm);
}


// dialogitem -> cheatdata struct
int GetCheatDataFromDialogItem(HWND hDlgWnd, struct ctrestoren *data){
//	int i;
	char str[64];

	HWND hList = GetDlgItem(hDlgWnd, IDC_CT5LIST1);
	GetCheatListEditDlgState(hDlgWnd);

	data->ctrescflag = IsDlgButtonChecked(hDlgWnd, IDC_CHECK1);
	data->ctressflag = g_Cheat_Info.cc_clistedit1;
	data->ctresssize = g_Cheat_Info.cc_clistedit2+1;

/*
	i = g_Cheat_Info.cc_clistedit3;
	if(i){
		switch(i&0x0f0){
			case 0x00:
				if(i>0x0c)
					i = 0xFF;
				else
					i++;
				break;
			case 0x10:
				if(i>0x18)
					i = 0xFF;
				else
					i += 0x04;
				break;
			case 0x20:
				if(i>0x18)
					i = 0xFF;
				else
					i -= 1;		//
				break;
			case 0x30:
				if(i>0x32)
					i = 0xFF;
				else
					i -= 1;		//
				break;
			default:
				i = 0xFF;
				break;
		}
	}
	data->opcode = i;
*/
	GetDlgItemText(hDlgWnd, IDC_EDIT1, (LPTSTR)str , 16);
	if(!MyStrAtoh(str, &data->opcode))
		data->opcode = 0;
	if(!((data->opcode >= 0x10 && data->opcode <0x28) || (data->opcode >= 0x30 && data->opcode <0x38) ||
		(data->opcode >= 0x40 && data->opcode <0x48) || (data->opcode >= 0x50 && data->opcode <0x58))){
		if(data->pSubcode){
			free(data->pSubcode);
			data->pSubcode = NULL;
			data->nSubcode = 0;
		}
	}

	GetDlgItemText(hDlgWnd, IDC_CT5EDIT1, (LPTSTR)str , 16);
	if(!MyStrAtoh(str, &data->ctresadd))
		data->ctresadd = 0;

	GetDlgItemText(hDlgWnd, IDC_CT5EDIT2, (LPTSTR)str , 16);

	if(g_Cheat_Info.cc_clistedit1){	//Hex
		if(!MyStrAtoh(str, &data->ctresdata))
			data->ctresdata = 0;
	}
	else{		// Dec
		data->ctresdata = atoi(str);
	}
	GetDlgItemText(hDlgWnd, IDC_CT5EDIT3, (LPTSTR)data->ctrescomm, 32);
	return 1;
}



//Cheat List Edit    IDD_CTDIALOG5
LRESULT CALLBACK MyCheatDlg7(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hList, hEdit, hEdit2, hEdit3;
//	char adtext[40], value[20], address[5];
	char *ermsgstr="Error";
	static int nSelect = -1, DisableFlag = 0, DisableEditCode = 0, DisableCombo = 0;
	int i;
//	DWORD adhex;
	DWORD dwStyle;
//    LV_ITEM item;
	static HFONT hlFont= NULL;
	static HINSTANCE hInst;

    switch(msg) {
        case WM_INITDIALOG:
			nSelect = -1;
			DisableFlag = 0;
			DisableEditCode = DisableCombo = 0;
			hInst = g_main_instance;
			hList = GetDlgItem(hDlgWnd, IDC_CT5LIST1);
			hEdit = GetDlgItem(hDlgWnd, IDC_CT5EDIT1);
			hEdit2 = GetDlgItem(hDlgWnd, IDC_CT5EDIT2);
			hEdit3 = GetDlgItem(hDlgWnd, IDC_CT5EDIT3);
			{
				char *ps[] = {"WT",	
					"ADD", "SUB", "AND",  "OR", "XOR", "SHL", "SHR", "MUL", "DIV", "MOD", "CPY",
					"IF &", "IF !(&)", "IF (==)", "IF (!=)", "IF  (<)", "IF  (>)", "IF (<=)", "IF (>=)", "IF  (1)",
					"REP SETPARAM", "REP OP", "REP COPY", "REP STORE",
					"IF M&TM", "IF !(M&TM)", "IF M==TM", "IF M!=TM", "IF M<TM", "IF M>TM", "IF M<=TM", "IF M>=TM",
					"TM[n]=m[x]", "m[x]=TM[n]", "TM[x]=n", "TM[x]+=n", "TM[x]-=n", 
					"IF TM&n", "IF !(TM&n)", "IF TM==n", "IF TM!=n", "IF TM<n", "IF TM>n", "IF TM<=n", "IF TM>=n",
					"IF M&M", "IF !(M&M)", "IF M==M", "IF M!=M", "IF M<M", "IF M>M", "IF M<=M", "IF M>=M",
					"----", NULL };
				for(int i=0; ps[i]; i++){
					SendDlgItemMessage(hDlgWnd,IDC_COMBO1,CB_INSERTSTRING,(WPARAM)i,(LPARAM)ps[i]);
				}
				SendDlgItemMessage(hDlgWnd,IDC_COMBO1,CB_SETCURSEL,(WPARAM)0,0L);
			}
			if(g_Cheat_Info.cc_clistedit1)
				SendMessage(GetDlgItem(hDlgWnd, IDC_CT5RADIO2), BM_SETCHECK, (WPARAM)TRUE, 0L);
			else
				SendMessage(GetDlgItem(hDlgWnd, IDC_CT5RADIO1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			CheckRadioButton( hDlgWnd, IDC_CT5RADIO3, IDC_CT5RADIO6, IDC_CT5RADIO3+g_Cheat_Info.cc_clistedit2);
//			CheckRadioButton( hDlgWnd, IDC_RADIO1, IDC_RADIO20, IDC_CT5RADIO3+g_Cheat_Info.cc_clistedit3);
			SendMessage(GetDlgItem(hDlgWnd, IDC_SPIN1),UDM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,0));
			SendMessage(GetDlgItem(hDlgWnd, IDC_SPIN1),UDM_SETPOS,FALSE,(LPARAM)0);
			g_Cheat_Info.cc_clistedit4 = 0;
			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT2), "0");
			hlFont = CreateFontIndirect(&CheatDlgFont);
			ListView_SetTextColor(hList, CheatDlgFontColor);
			if(hlFont!=NULL){
				SendMessage(hList, WM_SETFONT, (WPARAM)hlFont, MAKELPARAM(TRUE, 0));
			}

			dwStyle = ListView_GetExtendedListViewStyle(hList);
			dwStyle |= /*LVS_EX_CHECKBOXES |*/LVS_OWNERDRAWFIXED | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;
			ListView_SetExtendedListViewStyle(hList, dwStyle);
	    	{
	    		char str[32];
				LV_COLUMN lvcol;
				int i, *wp, cwidth[] = {120, 70, 85, 38, 38, 38, 38};

				for(i=0; i<7; i++){
					if(g_Cheat_Info.cc_listwidth[i])
						break;
				}
				if(i==7)
					wp = cwidth;
				else
					wp = g_Cheat_Info.cc_listwidth;
				lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
				lvcol.fmt = LVCFMT_LEFT;
				lvcol.cx = wp[0];
				LoadString(g_main_instance, IDS_STRING_CHTD_01 , str, 32);
				lvcol.pszText = str;
				lvcol.iSubItem = 0;
				ListView_InsertColumn(hList, 0, &lvcol);
				lvcol.fmt = LVCFMT_RIGHT;
				lvcol.cx = wp[1];
				LoadString(g_main_instance, IDS_STRING_CHTD_02 , str, 32);
				lvcol.pszText = str;
				lvcol.iSubItem = 1;
				ListView_InsertColumn(hList, 1, &lvcol);
				lvcol.cx = wp[2];
				LoadString(g_main_instance, IDS_STRING_CHTD_03 , str, 32);
				lvcol.pszText = str;
				lvcol.iSubItem = 2;
				ListView_InsertColumn(hList, 2, &lvcol);
				lvcol.fmt = LVCFMT_CENTER;
				lvcol.cx = wp[3];
				LoadString(g_main_instance, IDS_STRING_CHTD_04 , str, 32);
				lvcol.pszText = str;
				lvcol.iSubItem = 3;
				ListView_InsertColumn(hList, 3, &lvcol);
				lvcol.cx = wp[4];
				LoadString(g_main_instance, IDS_STRING_CHTD_05 , str, 32);
				lvcol.pszText = str;
				lvcol.iSubItem = 4;
				ListView_InsertColumn(hList, 4, &lvcol);
				lvcol.cx = wp[5];
				LoadString(g_main_instance, IDS_STRING_CHTD_13 , str, 32);
				lvcol.pszText = str;
				lvcol.iSubItem = 5;
				ListView_InsertColumn(hList, 5, &lvcol);
				lvcol.cx = wp[6];
				LoadString(g_main_instance, IDS_STRING_CHTD_14 , str, 32);
				lvcol.pszText = str;
				lvcol.iSubItem = 6;
				ListView_InsertColumn(hList, 6, &lvcol);

				if(g_Cheat_Info.cc_listorder[0] != g_Cheat_Info.cc_listorder[1])
					ListView_SetColumnOrderArray(hList, 7, g_Cheat_Info.cc_listorder);
	    	}
//			InitCheatListEditDlg(hList);
			ListView_SetItemCount(hList, g_Cheat_Info.ctrestnum);
            return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
				case IDCANCEL:
//					EndDialog(hDlgWnd, IDCANCEL);
					DestroyWindow(hDlgWnd);
					return TRUE;
					break;
				case IDOK:		//Add
					if(g_Cheat_Info.ctrestnum>= CTRESTMAX){
						return TRUE;
					}
					memset(&g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum], 0, sizeof(struct ctrestoren));
					g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctresssize = 1;
					g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctrescflag = 1;
					g_Cheat_Info.ctrestnum++;
					ListView_SetItemCount(hList, g_Cheat_Info.ctrestnum);
					ListView_SetItemState(hList, g_Cheat_Info.ctrestnum-1, LVIS_SELECTED, LVIS_SELECTED);
					return TRUE;
				   break;
				case IDC_CT5BUTTON2:		//Del
					{
						char str[64];
						if(!g_Cheat_Info.cc_clistedit4)
							LoadString(g_main_instance, IDS_STRING_CHTM_03 , str, 64);
						else
							LoadString(g_main_instance, IDS_STRING_CHTM_11 , str, 64);

						if(IDCANCEL== MessageBox(hDlgWnd,(LPCSTR)str,(LPCSTR)"Delete",MB_OKCANCEL))
							break;
						if(!g_Cheat_Info.cc_clistedit4){
							i=-1;
							if((i=ListView_GetNextItem(hList,-1,LVNI_ALL | LVNI_SELECTED)) != -1){
								int j = g_Cheat_Info.ctrestnum-1;
								if(g_Cheat_Info.ctrestds[i].nSubcode){
									g_Cheat_Info.ctrestds[i].nSubcode = 0;
									freemem_macro(g_Cheat_Info.ctrestds[i].pSubcode);
								}
								for(; i<j; i++){
									g_Cheat_Info.ctrestds[i] = g_Cheat_Info.ctrestds[i+1];
								}
								--g_Cheat_Info.ctrestnum;
								nSelect = -1;
								ListView_SetItemCount(hList, g_Cheat_Info.ctrestnum);
							}
							i = -1;
							if((i=ListView_GetNextItem(hList,-1,LVNI_ALL | LVNI_SELECTED)) != -1){
								ListView_SetItemState(hList, i, 0, LVIS_SELECTED);
								ListView_SetItemState(hList, i, LVIS_SELECTED, LVIS_SELECTED);
							}
						}
						else{
							struct ctrestoren *tdata;
							int max = g_Cheat_Info.ctrestds[nSelect].nSubcode-1;
							tdata = g_Cheat_Info.ctrestds[nSelect].pSubcode;
							for(i=g_Cheat_Info.cc_clistedit4; i<max; i++){
								tdata[i] = tdata[i+1];
							}
							g_Cheat_Info.ctrestds[nSelect].pSubcode = (struct ctrestoren *)realloc(tdata, sizeof(struct ctrestoren)*max);
							g_Cheat_Info.ctrestds[nSelect].nSubcode--;
							if(g_Cheat_Info.cc_clistedit4 > max){
								g_Cheat_Info.cc_clistedit4--;
							}
							if(!g_Cheat_Info.cc_clistedit4){
								SetCheatDataToDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect]);
							}
							else{
								SetCheatDataToDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect].pSubcode[g_Cheat_Info.cc_clistedit4-1]);
							}
						}
					}
//					GetCheatListEditDlg(hDlgWnd, hList);
					return TRUE;
					break;
				case IDC_CT5BUTTON3:
					SetWindowText(hEdit, "");
					SetWindowText(hEdit2, "");
					SetWindowText(hEdit3, "");
					return TRUE;
					break;
				case IDC_CT5BUTTON4:		//All Del
					{
						char str[64];
						LoadString(g_main_instance, IDS_STRING_CHTM_04 , str, 64);
						if(IDCANCEL== MessageBox(hDlgWnd,(LPCSTR)str,(LPCSTR)"caution",MB_OKCANCEL))
							break;
						for(int i=0; i<CTRESTMAX; i++){
							if(g_Cheat_Info.ctrestds[i].pSubcode)
								free(g_Cheat_Info.ctrestds[i].pSubcode);
						}
						g_Cheat_Info.ctrestnum=0;
						nSelect = -1;
						ListView_SetItemCount(hList, g_Cheat_Info.ctrestnum);
					}
					return TRUE;
					break;
				case IDC_BUTTON1:	// UP
					if(nSelect!=-1 && nSelect!=0){
						struct ctrestoren Tmp;
						Tmp = g_Cheat_Info.ctrestds[nSelect-1];
						g_Cheat_Info.ctrestds[nSelect-1] = g_Cheat_Info.ctrestds[nSelect];
						g_Cheat_Info.ctrestds[nSelect] = Tmp;
						int i = nSelect;
						nSelect = -1;
						ListView_SetItemState(hList, i, 0, LVIS_SELECTED);
						ListView_SetItemState(hList, i-1, LVIS_SELECTED, LVIS_SELECTED);

						RECT rc;
						ListView_GetItemRect(hList, nSelect, &rc, LVIR_BOUNDS);
						InvalidateRect(hList, &rc, TRUE);
						ListView_GetItemRect(hList, nSelect-1, &rc, LVIR_BOUNDS);
						InvalidateRect(hList, &rc, TRUE);
					}
					break;
				case IDC_BUTTON2:	// DOWN
					if(nSelect!=-1 && nSelect != g_Cheat_Info.ctrestnum-1){
						struct ctrestoren Tmp;
						Tmp = g_Cheat_Info.ctrestds[nSelect+1];
						g_Cheat_Info.ctrestds[nSelect+1] = g_Cheat_Info.ctrestds[nSelect];
						g_Cheat_Info.ctrestds[nSelect] = Tmp;
						int i = nSelect;
						nSelect = -1;
						ListView_SetItemState(hList, i, 0, LVIS_SELECTED);
						ListView_SetItemState(hList, i+1, LVIS_SELECTED, LVIS_SELECTED);

						RECT rc;
						ListView_GetItemRect(hList, nSelect, &rc, LVIR_BOUNDS);
						InvalidateRect(hList, &rc, TRUE);
						ListView_GetItemRect(hList, nSelect+1, &rc, LVIR_BOUNDS);
						InvalidateRect(hList, &rc, TRUE);
					}
					break;
#if 0
				case IDM_CTPOP21:
/*
					i=-1;
					while((i=ListView_GetNextItem(hList,i, LVNI_ALL | LVNI_SELECTED)) != -1){
						adtext[0]=0;
						ListView_GetItemText(hList, i, 0, adtext, 32);
						ListView_GetItemText(hList, i, 1, address, 6);
						ListView_GetItemText(hList, i, 2, value, 9);
						SResulttext= (unsigned char *)address;
						SResulttext2= (unsigned char *)value;
						SResulttext3= (unsigned char *)adtext;
						if(IDOK==DialogBox(hInst , MAKEINTRESOURCE(IDD_CTDIALOG11), hDlgWnd, (DLGPROC)MyCheatDlg14)){
							ListView_SetItemText(hList, i, 1, (char *)SResulttext);
							ListView_SetItemText(hList, i, 2, (char *)SResulttext2);
							ListView_SetItemText(hList, i, 0, (char *)SResulttext3);
						}
					}
					GetCheatListEditDlg(hDlgWnd, hList);
*/
					return TRUE;
					break;
#endif
				case IDM_CTPOP22:
					DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CTDIALOG13), hDlgWnd, (DLGPROC)MyCheatDlg17, (LPARAM)&g_Cheat_Info.ctrestds[nSelect]);

//					CheatCodeDisp(hDlgWnd, hList);
					return TRUE;
					break;
				case IDM_CTPOP23:
					CheatCodeCCopy(hDlgWnd, hList);
					return TRUE;
					break;
				case IDM_CTPOP24:
					CheatCodeACCopy(hDlgWnd, hList);
					return TRUE;
					break;
				case IDC_COMBO1:
					if(!DisableCombo)
					{
						int i = (unsigned char)SendDlgItemMessage(hDlgWnd, IDC_COMBO1, CB_GETCURSEL, 0L, 0L);
						i = nCheatCodeTable[i];
						char str[8];
						wsprintf(str, "%02X", i);
						DisableEditCode = 1;
						SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT1), str);
						DisableEditCode = 0;
					}
					break;
				case IDC_EDIT1:
				case IDC_CHECK1:
				case IDC_CT5EDIT1: case IDC_CT5EDIT2: case IDC_CT5EDIT3:
				case IDC_CT5RADIO1: case IDC_CT5RADIO2: case IDC_CT5RADIO3: case IDC_CT5RADIO4: case IDC_CT5RADIO5:
					if(!DisableFlag && nSelect != -1){
						if(!g_Cheat_Info.cc_clistedit4)
							GetCheatDataFromDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect]);
						else
							GetCheatDataFromDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect].pSubcode[g_Cheat_Info.cc_clistedit4-1]);
						RECT rc;
						ListView_GetItemRect(hList, nSelect, &rc, LVIR_BOUNDS);
						InvalidateRect(hList, &rc, TRUE);
						if(!DisableEditCode && LOWORD(wp)==IDC_EDIT1){
							DisableCombo = 1;
							if(!g_Cheat_Info.cc_clistedit4)
								SetCheatDataToDialogItemCombo(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect]);
							else
								SetCheatDataToDialogItemCombo(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect].pSubcode[g_Cheat_Info.cc_clistedit4-1]);
							DisableCombo = 0;
						}
					}
					break;
			}
            break;
		case WM_CONTEXTMENU:
			{
				POINT pt;
				HMENU hMenu, hSubMenu;
				pt.x = LOWORD(lp);
				pt.y = HIWORD(lp);
				
				if (wp == (WPARAM)hList) {
					//				ClientToScreen(hWnd, &pt);
					hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_POPMENU2));
					hSubMenu = GetSubMenu(hMenu, 0);
					TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_TOPALIGN,
						pt.x, pt.y, 0, hDlgWnd, NULL);
					DestroyMenu(hSubMenu);
					DestroyMenu(hMenu);
				}
			}
			return TRUE;
			break;
		case WM_DRAWITEM:
			{
				LPDRAWITEMSTRUCT lpds = (LPDRAWITEMSTRUCT)lp;
				if(lpds->CtlID == IDC_CT5LIST1){
					CheatListOwnerDraw((LPDRAWITEMSTRUCT)lpds);
				}
			}
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *lvinfo = (LV_DISPINFO *)lp;
				if((int)wp == IDC_CT5LIST1){
					switch(lvinfo->hdr.code){
/*
						case LVN_ODSTATECHANGED:
							{
								int i= 0;
							}
							break;
*/
						case LVN_ITEMCHANGED:
							{
								NM_LISTVIEW *nmlv = (NM_LISTVIEW *)lp;
								if((nmlv->uNewState&2) && !(nmlv->uOldState&2)){	// select
									int i=nmlv->iItem;
									DisableFlag = 1;
									if(nSelect!=-1){
										if(!g_Cheat_Info.cc_clistedit4)
											GetCheatDataFromDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect]);
										else
											GetCheatDataFromDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect].pSubcode[g_Cheat_Info.cc_clistedit4-1]);
									}
									nSelect = i;
									g_Cheat_Info.cc_clistedit4 = 0;
									SetCheatDataToDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[i]);
//									GetCheatListEditDlg(hDlgWnd, hList);
									DisableFlag = 0;
								}
								else if(!(nmlv->uNewState&2) && (nmlv->uOldState&2)){	// 
									int i=nmlv->iItem;
									if(nSelect!=-1){
										if(!g_Cheat_Info.cc_clistedit4)
											GetCheatDataFromDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect]);
										else
											GetCheatDataFromDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect].pSubcode[g_Cheat_Info.cc_clistedit4-1]);
									}
//									SetCheatDataToDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[i]);
//									GetCheatListEditDlg(hDlgWnd, hList);
									nSelect = -1;
									g_Cheat_Info.cc_clistedit4 = 0;
								}
							}
							break;
						case LVN_GETDISPINFO:
							CheatListOnGetDispInfo((NMLVDISPINFO *)lp);
							break;
/*
						case LVN_SETDISPINFO:
							{
							}
							break;
*/
						case LVN_ODFINDITEM:
//							return OnOdfinditem((LPNMLVFINDITEM)lparam);
							break;
/*
						case LVN_COLUMNCLICK:
							{
								NM_LISTVIEW *pNMLV = (NM_LISTVIEW *)lparam;
								g_Preview_State.PrevSortItemp[pNMLV->iSubItem]^=1;
								PreviewListSort(pNMLV->iSubItem);
							}
							break;
*/
						case NM_DBLCLK:
							{
								LVHITTESTINFO hitpos;
								int i;
								
								memset(&hitpos, 0, sizeof(LVHITTESTINFO));
								GetCursorPos(&hitpos.pt);
								POINT pt = hitpos.pt;
								ScreenToClient(hDlgWnd, &pt);
								ScreenToClient(hList, &hitpos.pt);
								if(-1 !=(i=ListView_HitTest(hList, &hitpos))){
									RECT rc;
									g_Cheat_Info.ctrestds[i].ctrescflag^=1;
									SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK1), BM_SETCHECK, (WPARAM)g_Cheat_Info.ctrestds[i].ctrescflag, 0L);
									ListView_GetItemRect(hList, i, &rc, LVIR_BOUNDS);
									InvalidateRect(hList, &rc, TRUE);
								}
							}
							break;
/*
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
*/
					}
				}
			}
			break;
		case WM_VSCROLL:
			if(GetDlgItem(hDlgWnd, IDC_SPIN1) == (HWND)lp){
				char str[64];
				unsigned int i;
				int nScrollCode = (int)LOWORD(wp);
				struct ctrestoren *tdata;
//				int Code[] = { SB_PAGEDOWN, SB_PAGEUP, SB_BOTTOM, SB_ENDSCROLL, SB_LINEDOWN, SB_LINEUP,SB_THUMBPOSITION, SB_TOP };

				if(nScrollCode != SB_THUMBPOSITION || g_Cheat_Info.ctrestds[nSelect].opcode < 0x10)
					break;

				i = LOWORD(SendMessage(GetDlgItem(hDlgWnd, IDC_SPIN1),UDM_GETPOS,0L,0L));
				if((unsigned int)g_Cheat_Info.cc_clistedit4 == i)
					break;

				DisableFlag = 1;
				if(g_Cheat_Info.ctrestds[nSelect].nSubcode < i){
					char str2[32];
					LoadString(g_main_instance, IDS_STRING_LMSG_17 , str, 64);
					LoadString(g_main_instance, IDS_STRING_LMSG_05, str2, 32);
					if(IDCANCEL== MessageBox(hDlgWnd,(LPCSTR)str,(LPCSTR)str2,MB_OKCANCEL)){
						--i;
						wsprintf(str, "%u / %u", i, g_Cheat_Info.ctrestds[nSelect].nSubcode);
						SetDlgItemText(hDlgWnd, IDC_EDIT2, str);
						g_Cheat_Info.cc_clistedit4 = i;
						SendMessage(GetDlgItem(hDlgWnd, IDC_SPIN1),UDM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,i+1));
						SendMessage(GetDlgItem(hDlgWnd, IDC_SPIN1),UDM_SETPOS,FALSE,(LPARAM)i);
						DisableFlag = 0;
						return TRUE;
						break;
					}
				}
				if(!g_Cheat_Info.cc_clistedit4){
					GetCheatDataFromDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect]);
				}
				else{
					GetCheatDataFromDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect].pSubcode[g_Cheat_Info.cc_clistedit4-1]);
				}
				tdata = g_Cheat_Info.ctrestds[nSelect].pSubcode;
				if(g_Cheat_Info.ctrestds[nSelect].nSubcode < i){
					g_Cheat_Info.ctrestds[nSelect].pSubcode = (struct ctrestoren *)realloc(tdata, sizeof(struct ctrestoren)*i);
					memset(&g_Cheat_Info.ctrestds[nSelect].pSubcode[i-1], 0, sizeof(struct ctrestoren));
					g_Cheat_Info.ctrestds[nSelect].pSubcode[i-1].ctresssize = 1;
					g_Cheat_Info.ctrestds[nSelect].pSubcode[i-1].ctrescflag = 1;
					g_Cheat_Info.ctrestds[nSelect].nSubcode = i;
				}
				if(!i){
					SetCheatDataToDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect]);
				}
				else{
					SetCheatDataToDialogItem(hDlgWnd, &g_Cheat_Info.ctrestds[nSelect].pSubcode[i-1]);
				}
				wsprintf(str, "%u / %u", i, g_Cheat_Info.ctrestds[nSelect].nSubcode);
				SetDlgItemText(hDlgWnd, IDC_EDIT2, str);
				g_Cheat_Info.cc_clistedit4 = i;
				SendMessage(GetDlgItem(hDlgWnd, IDC_SPIN1),UDM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,i+1));
				SendMessage(GetDlgItem(hDlgWnd, IDC_SPIN1),UDM_SETPOS,FALSE,(LPARAM)i);
				DisableFlag = 0;
				return TRUE;
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hDlgWnd);
			break;
		case WM_DESTROY:
			{
				int i;
				for(i=0;i<7;i++){
					g_Cheat_Info.cc_listwidth[i] = ListView_GetColumnWidth(hList, i);
				}
			}
			ListView_GetColumnOrderArray(hList, 7, g_Cheat_Info.cc_listorder);
			DeleteObject(hlFont);
			g_extra_window.CheatListEdit = NULL;
			return TRUE;
		case WM_EXTRAWIN_01:
			InitCheatListEditDlg(hList);
			return TRUE;
		case WM_EXTRAWIN_02:
			ListView_DeleteAllItems(hList);
			return TRUE;
    }
    return FALSE;
}


DWORD CheatCompareReadMem(int size, DWORD adr, int n){
	unsigned char *p, *pDest, *pSrc;
	DWORD msize, Data, nadr;

	Data = 0;
	pDest = (unsigned char *)&Data;
	if(adr < 0x10000){
		msize=	g_Cheat_Info.mainmem_size;
		p	=	&g_Cheat_Info.pMainCmp[msize*n];
		nadr = adr;
		pSrc = &p[nadr];
	}
	else if(adr < 0x20000){
		msize=	g_Cheat_Info.sram_size;
		p	=	&g_Cheat_Info.pSRAMCmp[msize*n];
		nadr = adr&0xffff;
		pSrc = &p[nadr];
	}
	else{
		msize=	g_Cheat_Info.extramem_size;
		p	=	&g_Cheat_Info.pExtraCmp[msize*n];
		nadr = adr&0xffff;
		pSrc = &p[nadr];
	}
	for(int i=0; i<size && ((nadr+i)<msize); i++, pDest++, pSrc++){
		*pDest = *pSrc;
	}
	return Data;
}


DWORD CheatReadMem(int size, DWORD adr);


void CheatCompareOnGetDispInfo(NMLVDISPINFO *pInfo){
	char str[512];
	int adr;
	DWORD dw;

	adr = g_Cheat_Info.pCmpOData[pInfo->item.iItem];

	if(pInfo->item.mask & LVIF_TEXT){
		switch (pInfo->item.iSubItem){
		  case 0:	// address
			  wsprintf(str, "%04X", adr);
			  strcpy(pInfo->item.pszText, str);
//			  pInfo->item.pszText = str;
			  break;
		  case 1:	// 1 value
		  case 2:
		  case 3:
			  dw = CheatCompareReadMem(g_Cheat_Info.ctcmpdtf2, adr, pInfo->item.iSubItem-1);
			  if(g_Cheat_Info.ctcmpdtf4){	//hex
				  char tstr[8];
				  wsprintf(tstr, "%%0%uX", (g_Cheat_Info.ctcmpdtf2)*2);
				  wsprintf(str, tstr, dw);
			  }
			  else{				//dec
				  wsprintf(str, "%u", dw);
			  }
			  strcpy(pInfo->item.pszText, str);
//			  pInfo->item.pszText = str;
			  break;
		  case 4:	// now
			  dw = CheatReadMem(g_Cheat_Info.ctcmpdtf2, adr);
			  if(g_Cheat_Info.ctcmpdtf4){	//hex
				  char tstr[8];
				  wsprintf(tstr, "%%0%uX", (g_Cheat_Info.ctcmpdtf2)*2);
				  wsprintf(str, tstr, dw);
			  }
			  else{				//dec
				  wsprintf(str, "%u", dw);
			  }
			  strcpy(pInfo->item.pszText, str);
//			  pInfo->item.pszText = str;
			  break;
		}
	}
}


void GetCheatCompareDlgItem(HWND hDlgWnd){
	g_Cheat_Info.ctcmpdtf6= (unsigned char)SendMessage(GetDlgItem(hDlgWnd, IDC_FC6CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L);
	if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOA1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf3=0;
	}
	else{
		g_Cheat_Info.ctcmpdtf3=1;
	}
	if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOD1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf4=0;
	}
	else{
		g_Cheat_Info.ctcmpdtf4=1;
	}
	if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf2=1;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB2), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf2=2;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB3), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf2=3;
	}
	else{
		g_Cheat_Info.ctcmpdtf2=4;
	}
	if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf1=0;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC2), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf1=1;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC3), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf1=2;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC4), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf1=3;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC5), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf1=4;
	}
	else{
		g_Cheat_Info.ctcmpdtf1=5;
	}
	if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOE1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf5=0;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOE2), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.ctcmpdtf5=1;
	}
	else{
		g_Cheat_Info.ctcmpdtf5=2;
	}

}


//Compare    IDD_CTDIALOG6
LRESULT CALLBACK MyCheatDlg8(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hList, hEdit, hEdit2, hEdit3, hEdit4;
	char adtext[30], adtext2[30], adtext3[30];
	char dectext[30];
	int i;

	static HFONT hlFont= NULL;
	POINT pt;
	static HINSTANCE hInst;
	DWORD dwStyle;

    switch(msg) {
        case WM_INITDIALOG:
			hInst = g_main_instance;
			hList = GetDlgItem(hDlgWnd, IDC_CT6LIST1);
			hEdit = GetDlgItem(hDlgWnd, IDC_CT6EDIT1);
			hEdit2 = GetDlgItem(hDlgWnd, IDC_CT6EDIT2);
			hEdit3 = GetDlgItem(hDlgWnd, IDC_CT6EDIT3);
			hEdit4 = GetDlgItem(hDlgWnd, IDC_CT6EDIT4);
			if(g_Cheat_Info.ctcmpdtf3)
				SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOA2), BM_SETCHECK, (WPARAM)TRUE, 0L);
			else
				SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOA1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			switch(g_Cheat_Info.ctcmpdtf2){
				case 0:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB1), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 1:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB2), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 2:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB3), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 3:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB4), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
			}
			switch(g_Cheat_Info.ctcmpdtf1){
				case 0:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC1), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 1:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC2), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 2:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC3), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 3:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC4), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 4:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC5), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 5:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOC6), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
			}
			if(g_Cheat_Info.ctcmpdtf4)
				SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOD2), BM_SETCHECK, (WPARAM)TRUE, 0L);
			else
				SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOD1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			switch(g_Cheat_Info.ctcmpdtf5){
				case 0:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOE1), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 1:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOE2), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 2:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOE3), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
			}
			SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK16), BM_SETCHECK, (WPARAM)TRUE, 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK21), BM_SETCHECK, (WPARAM)TRUE, 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK22), BM_SETCHECK, (WPARAM)TRUE, 0L);
			if(g_Cheat_Info.ctcmpdtf6){
				SendMessage(GetDlgItem(hDlgWnd, IDC_FC6CHECK1), BM_SETCHECK, (WPARAM)TRUE, 0L);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC1), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC2), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC3), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC4), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC5), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC6), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOE1), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOE2), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOE3), FALSE);
			}
			EnableWindow(GetDlgItem(hDlgWnd, IDC_CHECK16), (g_Cheat_Info.mainmem_size)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hDlgWnd, IDC_CHECK21), (g_Cheat_Info.sram_size)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hDlgWnd, IDC_CHECK22), (g_Cheat_Info.extramem_size)?TRUE:FALSE);

			hlFont = CreateFontIndirect(&CheatDlgFont);
			ListView_SetTextColor(hList, CheatDlgFontColor);

			if(hlFont!=NULL){
				SendMessage(hList, WM_SETFONT, (WPARAM)hlFont, MAKELPARAM(TRUE, 0));
			}

			SetWindowText(hEdit, "?");
			SetWindowText(hEdit2, "?");
			SetWindowText(hEdit3, "?");
			SetWindowText(hEdit4, "?");
			dwStyle = ListView_GetExtendedListViewStyle(hList);
			dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES ;
			ListView_SetExtendedListViewStyle(hList, dwStyle);

    	{
			LV_COLUMN lvcol;
			char str[32];
			lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvcol.fmt = LVCFMT_LEFT;
			lvcol.cx = 60;
			LoadString(g_main_instance, IDS_STRING_CHTD_02 , str, 32);
			lvcol.pszText = str;
			lvcol.iSubItem = 0;
			ListView_InsertColumn(hList, 0, &lvcol);
			lvcol.fmt = LVCFMT_RIGHT;
			lvcol.cx = 65;
			lvcol.pszText = "1";
			lvcol.iSubItem = 1;
			ListView_InsertColumn(hList, 1, &lvcol);
			lvcol.pszText = "2";
			lvcol.iSubItem = 2;
			ListView_InsertColumn(hList, 2, &lvcol);
			lvcol.pszText = "3";
			lvcol.iSubItem = 3;
			ListView_InsertColumn(hList, 3, &lvcol);
//			lvcol.cx = 55;
			lvcol.cx = 65;
			LoadString(g_main_instance, IDS_STRING_CHTD_06 , str, 32);
			lvcol.pszText = str;
			lvcol.iSubItem = 4;
			ListView_InsertColumn(hList, 4, &lvcol);
    	}

//			emu->readBaram(tempdram);
//			emu->readSraram(tempdramS);
           return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
				case IDCANCEL:
					DestroyWindow(hDlgWnd);
//					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
					break;
				case IDC_FC6CHECK1:
					if(1== (g_Cheat_Info.ctcmpdtf6= (unsigned char)SendMessage(GetDlgItem(hDlgWnd, IDC_FC6CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L))){
						i=0;
					}
					else{
						i=1;
					}
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC1), i);
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC2), i);
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC3), i);
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC4), i);
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC5), i);
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOC6), i);
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOE1), i);
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOE2), i);
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CT6RADIOE3), i);
					return 0;
					break;
				case IDM_CTPOP1:		//Change Value
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						g_Cheat_Info.ctcmpdtf2=1;
					}
					else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB2), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						g_Cheat_Info.ctcmpdtf2=2;
					}
					else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB3), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						g_Cheat_Info.ctcmpdtf2=3;
					}
					else
						g_Cheat_Info.ctcmpdtf2=4;
					i=-1;
					if((i=ListView_GetNextItem(hList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
						return TRUE;
					ListView_GetItemText(hList, i, 0, adtext, 15);			//Address
					MyStrAtoh(adtext, &MyCheatAdr);
					if(IDOK==DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG7), hDlgWnd, (DLGPROC)MyCheatDlg9)){
					}
					return TRUE;
					break;
 				case IDM_CTPOP2:		//Add Cheat
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						g_Cheat_Info.ctcmpdtf2=0;
					}
					else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB2), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						g_Cheat_Info.ctcmpdtf2=1;
					}
					else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT6RADIOB3), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						g_Cheat_Info.ctcmpdtf2=1;
					}
					else
						g_Cheat_Info.ctcmpdtf2=2;
					i=-1;
					if((i=ListView_GetNextItem(hList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
						return TRUE;
					ListView_GetItemText(hList, i, 0, adtext, 15);			//Address
					MyStrAtoh(adtext, &MyCheatAdr);
					if(IDOK==DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG8), hDlgWnd, (DLGPROC)MyCheatDlg10)){
					}
					return TRUE;
				   break;
               case IDOK:
				    if(!emu)
						break;
					{
						
						GetDlgItemText(hDlgWnd, IDC_CT6EDIT1, (LPTSTR)adtext, 28);
						GetDlgItemText(hDlgWnd, IDC_CT6EDIT2, (LPTSTR)dectext, 28);
						GetDlgItemText(hDlgWnd, IDC_CT6EDIT3, (LPTSTR)adtext2, 28);
						GetDlgItemText(hDlgWnd, IDC_CT6EDIT4, (LPTSTR)adtext3, 28);
						GetCheatCompareDlgItem(hDlgWnd);
						ListView_SetItemCount(hList, 0);
						unsigned char *pD;
						g_Cheat_Info.nCmpOData= 0;
						if(IsDlgButtonChecked(hDlgWnd, IDC_CHECK16) && emu->GetMainMemp(&pD)){
							CtCmpListO(hList, g_Cheat_Info.mainmem_size, 0, g_Cheat_Info.pMainCmp, pD, adtext, dectext, adtext2, adtext3);
						}
						if(IsDlgButtonChecked(hDlgWnd, IDC_CHECK21) && emu->GetSramMemp(&pD)){
							CtCmpListO(hList, g_Cheat_Info.sram_size,    0x10000, g_Cheat_Info.pSRAMCmp, pD, adtext, dectext, adtext2, adtext3);
						}
						if(IsDlgButtonChecked(hDlgWnd, IDC_CHECK22) && emu->GetExtMemp(&pD)){
							CtCmpListO(hList, g_Cheat_Info.extramem_size, 0x20000, g_Cheat_Info.pExtraCmp, pD, adtext, dectext, adtext2, adtext3);
						}
						ListView_SetItemCount(hList, g_Cheat_Info.nCmpOData);
					}
                   return TRUE;
			   case IDC_CT6RADIOD1: case IDC_CT6RADIOD2:
				   GetCheatCompareDlgItem(hDlgWnd);
				   InvalidateRect(hList, NULL, TRUE);
				   break;
			}
            break;
		case WM_CONTEXTMENU:
			{
				pt.x = LOWORD(lp);
				pt.y = HIWORD(lp);
				HMENU hMenu, hSubMenu;
				if (wp == (WPARAM)hList) {
					//				ClientToScreen(hWnd, &pt);
					hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_POPMENU1));
					hSubMenu = GetSubMenu(hMenu, 0);
					TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_TOPALIGN,
						pt.x, pt.y, 0, hDlgWnd, NULL);
					DestroyMenu(hSubMenu);
					DestroyMenu(hMenu);
				}
			}
			return TRUE;
			break;
		case WM_NOTIFY:
			if(emu){
				LV_DISPINFO *lvinfo = (LV_DISPINFO *)lp;
				if((int)wp == IDC_CT6LIST1){
					switch(lvinfo->hdr.code){
						case LVN_GETDISPINFO:
							CheatCompareOnGetDispInfo((NMLVDISPINFO *)lp);
							break;
					}
				}
			}
			break;
		case WM_EXTRAWIN_01:
			ListView_SetItemCount(hList, 0);
			EnableWindow(GetDlgItem(hDlgWnd, IDC_CHECK16), (g_Cheat_Info.mainmem_size)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hDlgWnd, IDC_CHECK21), (g_Cheat_Info.sram_size)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hDlgWnd, IDC_CHECK22), (g_Cheat_Info.extramem_size)?TRUE:FALSE);
			break;
		case WM_EXTRAWIN_02:
			ListView_SetItemCount(hList, 0);
			break;
		case WM_CLOSE:
			DestroyWindow(hDlgWnd);
			break;
		case WM_DESTROY:
			g_extra_window.CheatCompare = NULL;
			DeleteObject(hlFont);
			break;
    }
    return FALSE;
}


//Value Change  IDD_CTDIALOG7
LRESULT CALLBACK MyCheatDlg9(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hEdit, hEdit2;
	char adtext[30];
	DWORD adhex;

    switch(msg) {
        case WM_INITDIALOG:
			hEdit = GetDlgItem(hDlgWnd, IDC_CT7EDIT1);
			hEdit2 = GetDlgItem(hDlgWnd, IDC_CT7EDIT2);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CT7RADIO1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			wsprintf(adtext, "%04X", MyCheatAdr);	//16
			SetWindowText(hEdit , adtext);
    	{
    		int n=0;
     		char str[64], ostr[64];
    		LoadString(g_main_instance, IDS_STRING_CHTD_12 , str, 64);
    		wsprintf(ostr, str, n);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_CT7STATIC2) , ostr);
		}
			return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT7RADIO1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						adhex = GetDlgItemInt(hDlgWnd, IDC_CT7EDIT2, NULL, FALSE);
					}
					else{			//16
						GetDlgItemText(hDlgWnd, IDC_CT7EDIT2, (LPTSTR)adtext, 25);
						if(!MyStrAtoh(adtext, &adhex))
							EndDialog(hDlgWnd, IDCANCEL);
					}
					if(MyCheatAdr >= 0x10000){
						emu->WriteSroneb(MyCheatAdr, (unsigned char)adhex);
						if(g_Cheat_Info.ctcmpdtf2){
							emu->WriteSroneb(MyCheatAdr+1, (unsigned char)(adhex>>8));
							if(g_Cheat_Info.ctcmpdtf2>=2)
								emu->WriteSroneb(MyCheatAdr+2, (unsigned char)(adhex>>16));
							if(g_Cheat_Info.ctcmpdtf2==2)
								emu->WriteSroneb(MyCheatAdr+3, (unsigned char)(adhex>>24));
						}
					}
					else{
						emu->WriteBoneb(MyCheatAdr, (unsigned char)adhex);
						if(g_Cheat_Info.ctcmpdtf2){
							emu->WriteBoneb(MyCheatAdr+1, (unsigned char)(adhex>>8));
							if(g_Cheat_Info.ctcmpdtf2>=2)
							emu->WriteBoneb(MyCheatAdr+2, (unsigned char)(adhex>>16));
							if(g_Cheat_Info.ctcmpdtf2==2)
							emu->WriteBoneb(MyCheatAdr+3, (unsigned char)(adhex>>24));
						}
					}
  					EndDialog(hDlgWnd, IDOK);
					return TRUE;
				   break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			return TRUE;
			break;
    }
    return FALSE;
}


//Add Cheat   IDD_CTDIALOG8
LRESULT CALLBACK MyCheatDlg10(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hEdit, hEdit2, hEdit3;
	char adtext[34];
	DWORD adhex;

    switch(msg) {
        case WM_INITDIALOG:
			hEdit = GetDlgItem(hDlgWnd, IDC_CT8EDIT1);
			hEdit2 = GetDlgItem(hDlgWnd, IDC_CT8EDIT2);
			hEdit3 = GetDlgItem(hDlgWnd, IDC_CT8EDIT3);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CT8RADIO1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CT8RADIO3), BM_SETCHECK, (WPARAM)TRUE, 0L);
			wsprintf(adtext, "%04X", MyCheatAdr);
			SetWindowText(hEdit , adtext);
			return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT8RADIO1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						adhex = GetDlgItemInt(hDlgWnd, IDC_CT8EDIT2, NULL, FALSE);
						g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctressflag=0;
					}
					else{
						GetDlgItemText(hDlgWnd, IDC_CT8EDIT2, (LPTSTR)adtext, 25);
						if(!MyStrAtoh(adtext, &adhex))
							EndDialog(hDlgWnd, IDCANCEL);
						g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctressflag=1;
					}
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT8RADIO3), BM_GETCHECK, (WPARAM)TRUE, 0L)){
							adhex&=0xff;
							g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctresssize=1;
					}
					else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT8RADIO4), BM_GETCHECK, (WPARAM)TRUE, 0L)){
							adhex&=0xffff;
							g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctresssize=2;
					}
					else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT8RADIO5), BM_GETCHECK, (WPARAM)TRUE, 0L)){
							adhex&=0xffffff;
							g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctresssize=3;
					}
					else{
							g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctresssize=4;
					}
					g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctresadd = MyCheatAdr;
					g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctresdata= adhex;
					GetDlgItemText(hDlgWnd, IDC_CT8EDIT3, (LPTSTR)g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctrescomm, 33);
					g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].ctrescflag=1;
					++g_Cheat_Info.ctrestnum;
  					EndDialog(hDlgWnd, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			return TRUE;
			break;
    }
    return FALSE;
}


//List View Resault Display（Compare）
int CtCmpListO(HWND hWnd, int nItem, DWORD prefa, unsigned char *data, unsigned char *ina, char *str1, char *str2, char *str3, char *str4){
	int listn, i, str1f=0, str2f=0, str3f=0, str4f=0;
	DWORD cmphex1, strhex1, strhex2, strhex3, strhex4, temphex1, temphex2, temphex3, temphex4;
	unsigned char *pbData1, *pbData2, *pbData3;
//    LV_ITEM item;
//	char temptext[20], temptext2[20], temptext3[20], temptext4[20];

	listn = g_Cheat_Info.nCmpOData;

	pbData1 = data;
	pbData2 = &data[nItem];
	pbData3 = &data[nItem*2];

	if(g_Cheat_Info.ctcmpdtf3){							//Hex
		if(!MyStrAtoh(str1, &strhex1))
			str1f=1;
		if(!MyStrAtoh(str2, &strhex2))
			str2f=1;
		if(!MyStrAtoh(str3, &strhex3))
			str3f=1;
		if(!MyStrAtoh(str4, &strhex4))
			str4f=1;
	}
	else {									//Dec
		for(i=0,str1f=0; str1[i]; ++i){
			if(str1[i]=='?')
				str1f=1;
		}
		for(i=0,str2f=0; str2[i]; ++i){
			if(str2[i]=='?')
				str2f=1;
		}
		for(i=0,str3f=0; str3[i]; ++i){
			if(str3[i]=='?')
				str3f=1;
		}
		for(i=0,str4f=0; str4[i]; ++i){
			if(str4[i]=='?')
				str4f=1;
		}
		if(!str1f){
			strhex1=atoi(str1);
		}
		if(!str2f){
			strhex2=atoi(str2);
		}
		if(!str3f){
			strhex3=atoi(str3);
		}
		if(!str4f){
			strhex4=atoi(str4);
		}
	}
	for(i=0; i<nItem; ++i){
		if(g_Cheat_Info.ctcmpdtf2==1){			// 1 byte
			temphex1=(unsigned int)pbData1[i];
			temphex2=(unsigned int)ina[i];
			temphex3=(unsigned int)pbData2[i];
			temphex4=(unsigned int)pbData3[i];
		}
		else if(g_Cheat_Info.ctcmpdtf2==2){		//2
			if(i>=nItem-1)
				continue;
			temphex1=(unsigned int)(*(short *)&pbData1[i])&0xffff;
			temphex2=(unsigned int)(*(short *)&ina[i])&0xffff;
			temphex3=(unsigned int)(*(short *)&pbData2[i])&0xffff;
			temphex4=(unsigned int)(*(short *)&pbData3[i])&0xffff;
		}
		else if(g_Cheat_Info.ctcmpdtf2==3){
			if(i>=nItem-2)
				continue;
			temphex1=(unsigned int)(*(short *)&pbData1[i])&0xffff;
			temphex2=(unsigned int)(*(short *)&ina[i])&0xffff;
			temphex3=(unsigned int)(*(short *)&pbData2[i])&0xffff;
			temphex4=(unsigned int)(*(short *)&pbData3[i])&0xffff;

			temphex1|=(unsigned int)pbData1[i+2] << 16;
			temphex2|=(unsigned int)ina[i+2] << 16;
			temphex3|=(unsigned int)pbData2[i+2] << 16;
			temphex4|=(unsigned int)pbData3[i+2] << 16;
		}
		else{						//4
			if(i>=nItem-3)
				continue;
			temphex1=(unsigned int)(*(DWORD *)&pbData1[i]);
			temphex2=(unsigned int)(*(DWORD *)&ina[i]);
			temphex3=(unsigned int)(*(DWORD *)&pbData2[i]);
			temphex4=(unsigned int)(*(DWORD *)&pbData3[i]);
		}
		if(g_Cheat_Info.ctcmpdtf6==0){
			if(g_Cheat_Info.ctcmpdtf5==0)
				cmphex1= temphex1;
			else if(g_Cheat_Info.ctcmpdtf5==1)
				cmphex1= temphex3;
			else
				cmphex1= temphex4;

			switch(g_Cheat_Info.ctcmpdtf1){
				case 0:
					if(!(cmphex1 < temphex2))		//<
						continue;
					break;
				case 1:
					if(!(cmphex1 <= temphex2))		//<=
						continue;
					break;
				case 2:
					if(!(cmphex1 == temphex2))		//==
						continue;
					break;
				case 3:
					if(!(cmphex1 > temphex2))		//>
						continue;
					break;
				case 4:
					if(!(cmphex1 >= temphex2))		//>=
						continue;
					break;
				case 5:
					if(!(cmphex1 != temphex2))		//!=
						continue;
					break;
			}
		}
		if(!str1f){
			if(strhex1 != temphex1)
				continue;
		}
		if(!str2f){
			if(strhex2 != temphex2)
				continue;
		}
		if(!str3f){
			if(strhex3 != temphex3)
				continue;
		}
		if(!str4f){
			if(strhex4 != temphex4)
				continue;
		}
		g_Cheat_Info.pCmpOData[listn] = i+prefa;

#if 0
		wsprintf(temptext, "%04X", i + prefa);
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = temptext;
		item.iItem = listn;
		item.iSubItem = 0;
		item.lParam = listn;
		ListView_InsertItem(hWnd, &item);			//address
		if(!g_Cheat_Info.ctcmpdtf4){
			wsprintf(temptext, "%u", temphex1);
			wsprintf(temptext2, "%u", temphex2);
			wsprintf(temptext3, "%u", temphex3);
			wsprintf(temptext4, "%u", temphex4);
		}
		else {
			switch(g_Cheat_Info.ctcmpdtf2){
				case 0:
					wsprintf(temptext, "%02X", temphex1);
					wsprintf(temptext2, "%02X", temphex2);
					wsprintf(temptext3, "%02X", temphex3);
					wsprintf(temptext4, "%02X", temphex4);
					break;
				case 1:
					wsprintf(temptext, "%04X", temphex1);
					wsprintf(temptext2, "%04X", temphex2);
					wsprintf(temptext3, "%04X", temphex3);
					wsprintf(temptext4, "%04X", temphex4);
					break;
				case 2:
					wsprintf(temptext, "%06X", temphex1);
					wsprintf(temptext2, "%06X", temphex2);
					wsprintf(temptext3, "%06X", temphex3);
					wsprintf(temptext4, "%06X", temphex4);
					break;
				case 3:
					wsprintf(temptext, "%08X", temphex1);
					wsprintf(temptext2, "%08X", temphex2);
					wsprintf(temptext3, "%08X", temphex3);
					wsprintf(temptext4, "%08X", temphex4);
					break;
			}
		}
		item.mask = LVIF_TEXT;
		item.pszText = temptext;
		item.iItem = listn;
		item.iSubItem = 1;
		ListView_SetItem(hWnd, &item);				//1
		item.mask = LVIF_TEXT;
		item.pszText = temptext3;
		item.iItem = listn;
		item.iSubItem = 2;
		ListView_SetItem(hWnd, &item);				//2
		item.mask = LVIF_TEXT;
		item.pszText = temptext4;
		item.iItem = listn;
		item.iSubItem = 3;
		ListView_SetItem(hWnd, &item);				//3
		item.pszText = temptext2;
		item.iItem = listn;
		item.iSubItem = 4;
		ListView_SetItem(hWnd, &item);			//Now
#endif
		++listn;
	}
	g_Cheat_Info.nCmpOData = listn;
	return listn;
}



int MyStrAtoh(char *str, DWORD *hex){
	int i=0,j=0,k=0;
	DWORD lhex=0;
	*hex=0;

	while(str[i]==' ')
		++i;
	while(str[i]!='\0'){
		++i;
		++j;
	}
	if(j==0)
		return 0;
	else if(j>8){
		j=8;
	}
	--i;
	while(1){
		if(!MyHexCtoh(&str[i--] , &lhex))
			return 0;
		*hex|=lhex<<k;
		if(!--j)
			break;
		k+=4;
	}
	return 1;
}


int MyHexCtoh(char *hexchara, DWORD *hex){
	if(*hexchara >= 0x30 && *hexchara <= 0x39){
		*hex=(DWORD)*hexchara-'0';
		return 1;
	}
	else if(*hexchara >= 0x41 && *hexchara <= 0x5a){
		*hex=(DWORD)*hexchara-0x37;
		return 1;
	}
	else if(*hexchara >= 0x61 && *hexchara <= 0x7a){
		*hex=(DWORD)*hexchara-0x57;
		return 1;
	}
	else{
		return 0;
	}
	return 0;
}


int ReadMemContextPages(unsigned char **mempages, int adr, int size, unsigned char *pdata){
	int i, Page, cur;

	for(i = 0, cur = adr; i<size; i++, cur++){
		Page = cur>>13;
		if(mempages[Page] == NULL)
			break;
		*pdata = mempages[Page][cur & 0x1fff];
		pdata++;
	}
	return i;
}



void CheatWriteStringsMem(int size, DWORD adr, unsigned char *pdata){
	unsigned char *p ,*pDest, *pSrc = pdata;
	DWORD msize, nadr;

	if(adr >= 0x20000){
		msize = emu->GetExtMemp(&p);
		nadr = adr&0xffff;
		pDest = &p[nadr];
	}
	else if(adr >= 0x10000){
		msize = emu->GetSramMemp(&p);
		nadr = adr&0xffff;
		pDest = &p[nadr];
	}
	else if(adr >= 0x800){	// ROM
/*
		nes6502_context Context;
		memset(&Context, 0, sizeof(Context));
		emu->GetCPUContext((unsigned char *)&Context);
		ReadMemContextPages((unsigned char **)&Context.mem_page, adr, size, pdata);
*/
		return;
	}
	else{
		msize = emu->GetMainMemp(&p);
		nadr = adr&0xffff;
		pDest = &p[nadr];
	}
	for(int i=0; i<size && ((nadr+i)<msize); i++, pDest++, pSrc++){
		*pDest = *pSrc;
	}
}



void CheatWriteMem(int size, DWORD adr, DWORD data){
	unsigned char *p ,*pDest, *pSrc = (unsigned char *)&data;
	DWORD msize, nadr;

	if(adr >= 0x20000){
		msize = emu->GetExtMemp(&p);
		nadr = adr&0xffff;
		pDest = &p[nadr];
	}
	else if(adr >= 0x10000){
		msize = emu->GetSramMemp(&p);
		nadr = adr&0xffff;
		pDest = &p[nadr];
	}
	else{
		msize = emu->GetMainMemp(&p);
		nadr = adr&0xffff;
		pDest = &p[nadr];
	}
	for(int i=0; i<size && ((nadr+i)<msize); i++, pDest++, pSrc++){
		*pDest = *pSrc;
	}
}


DWORD CheatReadMem(int size, DWORD adr){
	unsigned char *p, *pDest, *pSrc;
	DWORD msize, Data, nadr;

	Data = 0;
	pDest = (unsigned char *)&Data;
	if(adr < 0x800){		// Main memory
		msize = emu->GetMainMemp(&p);
		nadr = adr;
		pSrc = &p[nadr];
	}
	else if(adr < 0x10000){	// ROM
		nes6502_context Context;
		memset(&Context, 0, sizeof(Context));
		emu->GetCPUContext((unsigned char *)&Context);

		ReadMemContextPages((unsigned char **)&Context.mem_page, adr, size, (unsigned char *)&Data);
		return Data;
	}
	else if(adr < 0x20000){	// SRAM
		msize = emu->GetSramMemp(&p);
		nadr = adr & 0xffff;
		pSrc = &p[nadr];
	}
	else{					// Mapper memory
		msize = emu->GetExtMemp(&p);
		nadr = adr & 0xffff;
		pSrc = &p[nadr];
	}
	for(int i=0; i<size && ((nadr+i)<msize); i++, pDest++, pSrc++){
		*pDest = *pSrc;
	}
	return Data;
}



void CheatReadStringsMem(int size, DWORD adr, unsigned char *pdata){
	unsigned char *p ,*pDest, *pSrc = pdata;
	DWORD msize, nadr;

	if(adr >= 0x20000){
		msize = emu->GetExtMemp(&p);
		nadr = adr&0xffff;
		pDest = &p[nadr];
	}
	else if(adr >= 0x10000){
		msize = emu->GetSramMemp(&p);
		nadr = adr&0xffff;
		pDest = &p[nadr];
	}
	else if(adr >= 0x800){	// ROM
		nes6502_context Context;
		memset(&Context, 0, sizeof(Context));
		emu->GetCPUContext((unsigned char *)&Context);
		ReadMemContextPages((unsigned char **)&Context.mem_page, adr, size, pdata);
		return;
	}
	else{
		msize = emu->GetMainMemp(&p);
		nadr = adr&0xffff;
		pDest = &p[nadr];
	}
	for(int i=0; i<size && ((nadr+i)<msize); i++, pDest++, pSrc++){
		 *pSrc = *pDest;
	}
}



int OperateCheatCode(struct ctrestoren *Code){
	DWORD dw, dw2;
	static DWORD T_Adress, T_Data, T_Size;


	switch(Code->opcode){
		case 0x00:	// Write
		case 0x01:
			CheatWriteMem(Code->ctresssize, Code->ctresadd, Code->ctresdata);
			break;
		case 0x02:	// ADD
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw+= Code->ctresdata;
			CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			break;
		case 0x03:	// SUB
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw-= Code->ctresdata;
			CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			break;
		case 0x04:	// AND
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw&= Code->ctresdata;
			CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			break;
		case 0x05:	// OR
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw |= Code->ctresdata;
			CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			break;
		case 0x06:	// XOR
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw ^= Code->ctresdata;
			CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			break;
		case 0x07:	// SHL
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw <<= Code->ctresdata;
			CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			break;
		case 0x08:	// SHR
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw >>= Code->ctresdata;
			CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			break;
		case 0x09:	// MUL
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw *= Code->ctresdata;
			CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			break;
		case 0x0A:	// DIV
			if(Code->ctresdata){
				dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
				dw /= Code->ctresdata;
				CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			}
			break;
		case 0x0B:	// %
			if(Code->ctresdata){
				dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
				dw %= Code->ctresdata;
				CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			}
			break;
		case 0x0C:	// NOT
			if(Code->ctresdata){
				dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
				dw = ~dw;
				CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			}
			break;
		case 0x10:	// IF &
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			return (dw & Code->ctresdata);
			break;
		case 0x11:	// IF !&
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			return !(dw & Code->ctresdata);
			break;
		case 0x12:	// IF ==
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			return (dw == Code->ctresdata);
			break;
		case 0x13:	// IF !=
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			return (dw != Code->ctresdata);
			break;
		case 0x14:	// IF <
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			return (dw < Code->ctresdata);
			break;
		case 0x15:	// IF >
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			return (dw > Code->ctresdata);
			break;
		case 0x16:	// IF <=
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			return (dw <= Code->ctresdata);
			break;
		case 0x17:	// IF >=
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			return (dw >= Code->ctresdata);
			break;
		case 0x18:	// IF (1)
			return 1;
			break;
		case 0x20:	// REPERT Param set
			T_Adress = Code->ctresadd;
			T_Data	 = Code->ctresdata;
			T_Size	 = Code->ctresssize;
			break;
		case 0x21:	// REPERT Execute Opecode
			{
				struct ctrestoren Tmp;
				memset(&Tmp, 0, sizeof(Tmp));

				Tmp.ctresadd = T_Adress;
				Tmp.ctresdata= T_Data;
				Tmp.ctresssize= T_Size;
				Tmp.opcode = Code->ctresdata;
				for(unsigned int i=0; i<Code->ctresadd; i++, Tmp.ctresadd+=T_Size){
					OperateCheatCode(&Tmp);
				}
				T_Adress = Tmp.ctresadd;
			}
			break;
		case 0x22:	// REPERT Copy
			{
				DWORD SrcAdd = Code->ctresadd;
				for(unsigned int i=0; i<Code->ctresadd; i++, T_Adress+=T_Size, SrcAdd+=T_Size){
					dw = CheatReadMem(Code->ctresssize, SrcAdd);
					CheatWriteMem(T_Size, T_Adress, dw);
				}
			}
			break;
		case 0x23:	// REPERT Store
			{
				for(unsigned int i=0; i<Code->ctresadd; i++, T_Adress+=T_Size){
					CheatWriteMem(T_Size, T_Adress, T_Data);
				}
			}
			break;

		// 一時メモリ
		case 0x30:	// IF &
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = g_Cheat_Info.CheatTempMemory[Code->ctresdata & 31];
			return (dw & dw2);
			break;
		case 0x31:	// IF !&
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = g_Cheat_Info.CheatTempMemory[Code->ctresdata & 31];
			return !(dw & dw2);
			break;
		case 0x32:	// IF ==
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = g_Cheat_Info.CheatTempMemory[Code->ctresdata & 31];
			return (dw == dw2);
			break;
		case 0x33:	// IF !=
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = g_Cheat_Info.CheatTempMemory[Code->ctresdata & 31];
			return (dw != dw2);
			break;
		case 0x34:	// IF <
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = g_Cheat_Info.CheatTempMemory[Code->ctresdata & 31];
			return (dw < dw2);
			break;
		case 0x35:	// IF >
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = g_Cheat_Info.CheatTempMemory[Code->ctresdata & 31];
			return (dw > dw2);
			break;
		case 0x36:	// IF <=
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = g_Cheat_Info.CheatTempMemory[Code->ctresdata & 31];
			return (dw <= dw2);
			break;
		case 0x37:	// IF >=
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = g_Cheat_Info.CheatTempMemory[Code->ctresdata & 31];
			return (dw >= dw2);
			break;

		// 一時メモリ操作
		case 0x38:	// TMA[nn]   = m[xxxx]  // メモリを保存	;xxxx = メモリのアドレス, nn = 保存メモリのアドレス
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			g_Cheat_Info.CheatTempMemory[Code->ctresdata & 31] = dw;
			break;
		case 0x39:	// 一時メモリからメモリに書き込み	;xxxx = メモリのアドレス, nn = 保存メモリのアドレス
			dw = g_Cheat_Info.CheatTempMemory[Code->ctresdata & 31];
			CheatWriteMem(Code->ctresssize, Code->ctresadd, dw);
			break;
		case 0x3a:	//  TMA[xxxx] = nn    一時メモリに数値書き込み		;xxxx = 保存メモリのアドレス, nn = 書き込むデータ
			g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31] = Code->ctresdata;
			break;
		case 0x3b:	// 一時メモリに加算	;xxxx = 保存メモリのアドレス, nn = 数値
			g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31] += Code->ctresdata;
			break;
		case 0x3c:	// 一時メモリに減算	;xxxx = 保存メモリのアドレス, nn = 数値
			g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31] -= Code->ctresdata;
			break;

		// 一時メモリと数値
		case 0x40:	// IF &
			dw = g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31];
			return (dw & Code->ctresdata);
			break;
		case 0x41:	// IF !&
			dw = g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31];
			return !(dw & Code->ctresdata);
			break;
		case 0x42:	// IF ==
			dw = g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31];
			return (dw == Code->ctresdata);
			break;
		case 0x43:	// IF !=
			dw = g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31];
			return (dw != Code->ctresdata);
			break;
		case 0x44:	// IF <
			dw = g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31];
			return (dw < Code->ctresdata);
			break;
		case 0x45:	// IF >
			dw = g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31];
			return (dw > Code->ctresdata);
			break;
		case 0x46:	// IF <=
			dw = g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31];
			return (dw <= Code->ctresdata);
			break;
		case 0x47:	// IF >=
			dw = g_Cheat_Info.CheatTempMemory[Code->ctresadd & 31];
			return (dw >= Code->ctresdata);
			break;

			// メモリ同士
		case 0x50:	// IF &
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = CheatReadMem(Code->ctresssize, Code->ctresdata);
			return (dw & dw2);
			break;
		case 0x51:	// IF !&
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = CheatReadMem(Code->ctresssize, Code->ctresdata);
			return !(dw & dw2);
			break;
		case 0x52:	// IF ==
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = CheatReadMem(Code->ctresssize, Code->ctresdata);
			return (dw == dw2);
			break;
		case 0x53:	// IF !=
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = CheatReadMem(Code->ctresssize, Code->ctresdata);
			return (dw != dw2);
			break;
		case 0x54:	// IF <
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = CheatReadMem(Code->ctresssize, Code->ctresdata);
			return (dw < dw2);
			break;
		case 0x55:	// IF >
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = CheatReadMem(Code->ctresssize, Code->ctresdata);
			return (dw > dw2);
			break;
		case 0x56:	// IF <=
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = CheatReadMem(Code->ctresssize, Code->ctresdata);
			return (dw <= dw2);
			break;
		case 0x57:	// IF >=
			dw = CheatReadMem(Code->ctresssize, Code->ctresadd);
			dw2 = CheatReadMem(Code->ctresssize, Code->ctresdata);
			return (dw >= dw2);
			break;
	}
	return 0;
}


void MyCtRestore(){
	int i;

	if(g_Cheat_Info.DisableCheat){
		return;
	}
	for(i=0; i<g_Cheat_Info.ctrestnum; ++i){
		if(g_Cheat_Info.ctrestds[i].ctrescflag){
			int ret = OperateCheatCode(&g_Cheat_Info.ctrestds[i]);
			if(ret && g_Cheat_Info.ctrestds[i].nSubcode && g_Cheat_Info.ctrestds[i].pSubcode){
				for(unsigned int j=0; j<g_Cheat_Info.ctrestds[i].nSubcode; j++){
					if(g_Cheat_Info.ctrestds[i].pSubcode[j].ctrescflag)
						OperateCheatCode(&g_Cheat_Info.ctrestds[i].pSubcode[j]);
				}
			}
		}
	}
	return;
}



void GetCheatSearchDialogState(HWND hDlgWnd){
	g_Cheat_Info.cc_csrchrb2=(char)SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO2), BM_GETCHECK, (WPARAM)TRUE, 0L);
	if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.cc_csrchrb1=0;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC2), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.cc_csrchrb1=1;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC3), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.cc_csrchrb1=2;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC4), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.cc_csrchrb1=3;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC5), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.cc_csrchrb1=4;
	}
	else
		g_Cheat_Info.cc_csrchrb1=5;
	if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO3), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.cc_csrchrb3=0;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO4), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.cc_csrchrb3=1;
	}
	else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO5), BM_GETCHECK, (WPARAM)TRUE, 0L)){
		g_Cheat_Info.cc_csrchrb3=2;
	}
	else
		g_Cheat_Info.cc_csrchrb3=3;
}


// 
void CheatSearchProc1(int size, unsigned char *preData, unsigned char *nowData, unsigned char *pFlag, int inwild, unsigned int tempinhex){
	int i,tcmpflag,cmps,ctype;
	DWORD temphex, temphex2;

	ctype = g_Cheat_Info.cc_csrchrb1;
	cmps = g_Cheat_Info.cc_csrchrb3+1;

	for(i=size-cmps; i<size; i++){
		pFlag[i] = 1;
	}
	for(i=0;i<size;++i){
		switch(cmps){		//1byte
		case 1:
			temphex = (unsigned int)preData[i];
			temphex2 = (unsigned int)nowData[i];
			break;
		case 2:
			if(i>=size-1)
				continue;
			temphex = (unsigned int)(*(short *)&preData[i]);
			temphex2 = (unsigned int)(*(short *)&nowData[i]);
			break;
		case 3:
			if(i>=size-2)
				continue;
			temphex = (unsigned int)(*(short *)&preData[i]);
			temphex |= (unsigned int)preData[i+2] << 16;
			temphex2 = (unsigned int)(*(short *)&nowData[i]);
			temphex2 |= (unsigned int)nowData[i+2] << 16;
			break;
		case 4:
			if(i>=size-3)
				continue;
			temphex = (unsigned int)(*(DWORD *)&preData[i]);
			temphex2 = (unsigned int)(*(DWORD *)&nowData[i]);
			break;
		}
		tcmpflag=0;
		switch(ctype){
		case 0:
			if(temphex < temphex2){		//<
				tcmpflag=1;
			}
			break;
		case 1:
			if(temphex > temphex2){		//>
				tcmpflag=1;
			}
			break;
		case 2:
			if(temphex == temphex2){	//==
				tcmpflag=1;
			}
			break;
		case 3:
			if(temphex != temphex2){	//!=
				tcmpflag=1;
			}
			break;
		case 4:
			if(inwild){
				tcmpflag=1;
			}
			else{
				if(tempinhex == temphex2){
					tcmpflag=1;
				}
			}
			break;
		case 5:
			if(inwild){
				switch(cmps){
				case 1:
					if(((temphex - tempinhex)& 0x0ff) == temphex2){
						tcmpflag=1;
					}
					break;
				case 2:
					if(((temphex - tempinhex)& 0x0ffff) == temphex2){
						tcmpflag=1;
					}
				case 3:
					if(((temphex - tempinhex)& 0x0ffffff) == temphex2){
						tcmpflag=1;
					}
					break;
				case 4:
					if((temphex - tempinhex) == temphex2){
						tcmpflag=1;
					}
					break;
				}
			}
			else{
				if((tempinhex+temphex) == temphex2){
					tcmpflag=1;
				}
			}
			break;
		}
		if(!tcmpflag){
			pFlag[i] = 1;
		}
	}
}


int MyCheat11s(HWND hWnd, int size, unsigned char *flagbuff, unsigned char *databuff, unsigned char *ndatabuff, DWORD pad){
	int i, listn, cmpsize, type, nAdd;
//	DWORD temphex, temphex2;
//	char temptext[10], temptext2[10];
//    LV_ITEM item;

	cmpsize = g_Cheat_Info.cc_csrchrb3;
	type = g_Cheat_Info.cc_csrchrb2;

	nAdd = 	g_Cheat_Info.nSearchOData;

	listn = ListView_GetItemCount(hWnd);
	for(i=0; i< size; i++){
		if(!flagbuff[i]){
			g_Cheat_Info.pSearchOData[nAdd] = i+pad;
			nAdd++;
#if 0
			wsprintf(temptext, "%04X", i + pad);
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = temptext;
			item.iItem = listn;
			item.iSubItem = 0;
			item.lParam = listn;
			ListView_InsertItem(hWnd, &item);
			switch(cmpsize){
				case 0:
					temphex=(unsigned int)databuff[i];
					temphex2=(unsigned int)ndatabuff[i];
					break;
				case 1:
					temphex=(unsigned int)(*(WORD *)&databuff[i])&0xffff;
					temphex2=(unsigned int)(*(WORD *)&ndatabuff[i])&0xffff;
					break;
				case 2:
					temphex=(unsigned int)(*(WORD *)&databuff[i])&0xffff;
					temphex2=(unsigned int)(*(WORD *)&ndatabuff[i])&0xffff;
					temphex|=(unsigned int)databuff[i+2] <<16;
					temphex2|=(unsigned int)ndatabuff[i+2] <<16;
					break;
				case 3:
					temphex=(unsigned int)(*(DWORD *)&databuff[i]);
					temphex2=(unsigned int)(*(DWORD *)&ndatabuff[i]);
					break;
			}
			if(type){
				switch(cmpsize){
					case 0:
						wsprintf(temptext, "%02X", temphex);
						wsprintf(temptext2, "%02X", temphex2);
						break;
					case 1:
						wsprintf(temptext, "%04X", temphex);
						wsprintf(temptext2, "%04X", temphex2);
						break;
					case 2:
						wsprintf(temptext, "%06X", temphex);
						wsprintf(temptext2, "%06X", temphex2);
						break;
					case 3:
						wsprintf(temptext, "%08X", temphex);
						wsprintf(temptext2, "%08X", temphex2);
						break;
				}
			}
			else{
					wsprintf(temptext, "%u", temphex);
					wsprintf(temptext2, "%u", temphex2);
			}
			item.mask = LVIF_TEXT;
			item.pszText = temptext;
			item.iItem = listn;
			item.iSubItem = 1;
			ListView_SetItem(hWnd, &item);
			item.mask = LVIF_TEXT;
			item.pszText = temptext2;
			item.iItem = listn;
			item.iSubItem = 2;
			ListView_SetItem(hWnd, &item);
			++listn;
#endif
		}
	}
	g_Cheat_Info.nSearchOData = nAdd;
	return 0;
}



void InitCheatSearchMem(HWND hDlg){
	unsigned char *pD;
	if(!emu)
		return;
	if(g_Cheat_Info.mainmem_size){
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK1), TRUE);
		if(emu->GetMainMemp(&pD))
			memcpy(g_Cheat_Info.pMainMemData, pD, g_Cheat_Info.mainmem_size);
		ZeroMemory(g_Cheat_Info.pMainMemFlag, g_Cheat_Info.mainmem_size);
	}
	else
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK1), FALSE);
	if(g_Cheat_Info.sram_size){
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK2), TRUE);
		if(emu->GetSramMemp(&pD))
			memcpy(g_Cheat_Info.pSRAMData, pD, g_Cheat_Info.sram_size);
		ZeroMemory(g_Cheat_Info.pSRAMFlag, g_Cheat_Info.sram_size);
	}
	else
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK2), FALSE);
	if(g_Cheat_Info.extramem_size){
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK3), TRUE);
		if(emu->GetExtMemp(&pD))
			memcpy(g_Cheat_Info.pExtraMemData, pD, g_Cheat_Info.extramem_size);
		ZeroMemory(g_Cheat_Info.pExtraMemFlag, g_Cheat_Info.extramem_size);
	}
	else
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK3), FALSE);
}




DWORD CheatSearchReadPreMem(int size, DWORD adr){
	unsigned char *p, *pDest, *pSrc;
	DWORD msize, Data, nadr;

	Data = 0;
	pDest = (unsigned char *)&Data;
	if(adr < 0x10000){
		msize=	g_Cheat_Info.mainmem_size;
		p	=	&g_Cheat_Info.pBUP_MainMem[g_Cheat_Info.mainmem_size];
		nadr = adr;
		pSrc = &p[nadr];
	}
	else if(adr < 0x20000){
		msize=	g_Cheat_Info.sram_size;
		p	=	&g_Cheat_Info.pBUP_SRAM[g_Cheat_Info.sram_size];
		nadr = adr & 0xffff;
		pSrc = &p[nadr];
	}
	else{
		msize=	g_Cheat_Info.extramem_size;
		p	=	&g_Cheat_Info.pBUP_ExtraMem[g_Cheat_Info.extramem_size];
		nadr = adr & 0xffff;
		pSrc = &p[nadr];
	}
	for(int i=0; i<size && ((nadr+i)<msize); i++, pDest++, pSrc++){
		*pDest = *pSrc;
	}
	return Data;
}



void CheatSearchOnGetDispInfo(NMLVDISPINFO *pInfo){
	char str[512];
	int adr;
	DWORD dw;


	adr = g_Cheat_Info.pSearchOData[pInfo->item.iItem];

	if(pInfo->item.mask & LVIF_TEXT){
		switch (pInfo->item.iSubItem){
		  case 0:	// address
			  wsprintf(str, "%05X", adr);
			  strcpy(pInfo->item.pszText, str);
//			  pInfo->item.pszText = str;
			  break;
		  case 1:	// pre value
			  dw = CheatSearchReadPreMem(g_Cheat_Info.cc_csrchrb3+1, adr);
			  if(g_Cheat_Info.cc_csrchrb2){	//hex
				  char tstr[8];
				  wsprintf(tstr, "%%0%uX", (g_Cheat_Info.cc_csrchrb3+1)*2);
				  wsprintf(str, tstr, dw);
			  }
			  else{				//dec
				  wsprintf(str, "%u", dw);
			  }
			  strcpy(pInfo->item.pszText, str);
//			  pInfo->item.pszText = str;
			  break;
		  case 2:	// now
			  dw = CheatReadMem(g_Cheat_Info.cc_csrchrb3+1, adr);
			  if(g_Cheat_Info.cc_csrchrb2){	//hex
				  char tstr[8];
				  wsprintf(tstr, "%%0%uX", (g_Cheat_Info.cc_csrchrb3+1)*2);
				  wsprintf(str, tstr, dw);
			  }
			  else{				//dec
				  wsprintf(str, "%u", dw);
			  }
			  strcpy(pInfo->item.pszText, str);
//			  pInfo->item.pszText = str;
			  break;
		}
	}
}



//Value Serch  IDD_CTDIALOG9
LRESULT CALLBACK MyCheatDlg11(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hEdit, hList;
	static int buckup;

//	static char radio1=0, radio2=0 ,radio3=0;
	static HFONT hlFont= NULL;
	static HINSTANCE hInst;
	DWORD dwStyle;

    switch(msg) {
        case WM_INITDIALOG:
/*			tempmem = (unsigned char *)malloc(0x800);
			tempmem2 = (unsigned char *)malloc(0x2000);
			tempmemS = (unsigned char *)malloc(0x800);
			tempmemS2 = (unsigned char *)malloc(0x2000);
*/			hInst = g_main_instance;
			hEdit = GetDlgItem(hDlgWnd, IDC_CT9EDIT1);
			hList = GetDlgItem(hDlgWnd, IDC_CT9LIST);
			hlFont = CreateFontIndirect(&CheatDlgFont);
			ListView_SetTextColor(hList, CheatDlgFontColor);
			if(hlFont!=NULL){
				SendMessage(hList, WM_SETFONT, (WPARAM)hlFont, MAKELPARAM(TRUE, 0));
			}
			if(g_Cheat_Info.cc_csrchrb2)
	 			SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO2), BM_SETCHECK, (WPARAM)TRUE, 0L);
			else
	 			SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			EnableWindow(hEdit , FALSE);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK2), BM_SETCHECK, (WPARAM)TRUE, 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK3), BM_SETCHECK, (WPARAM)TRUE, 0L);
			switch(g_Cheat_Info.cc_csrchrb1){
				case 0:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC1), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 1:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC2), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 2:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC3), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 3:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC4), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 4:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC5), BM_SETCHECK, (WPARAM)TRUE, 0L);
					EnableWindow(hEdit , TRUE);
					break;
				case 5:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIOC6), BM_SETCHECK, (WPARAM)TRUE, 0L);
					EnableWindow(hEdit , TRUE);
					break;
			}
			switch(g_Cheat_Info.cc_csrchrb3){
				case 0:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO3), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 1:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO4), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 2:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO5), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 3:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO6), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
			}
//			InitCheatSearchMem(hDlgWnd);

			dwStyle = ListView_GetExtendedListViewStyle(hList);
			dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES ;
			ListView_SetExtendedListViewStyle(hList, dwStyle);

			{
				LV_COLUMN lvcol;
				char str[32];
				lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
				lvcol.fmt = LVCFMT_LEFT;
				lvcol.cx = 70;
//				MessageBox(hDlgWnd,(LPCSTR)str,(LPCSTR)"Error",MB_OK);
				LoadString(g_main_instance, IDS_STRING_CHTD_02 , str, 32);
//				MessageBox(hDlgWnd,(LPCSTR)str,(LPCSTR)"Error",MB_OK);
				lvcol.pszText = str;
				lvcol.iSubItem = 0;
				ListView_InsertColumn(hList, 0, &lvcol);
				lvcol.fmt = LVCFMT_RIGHT;
				lvcol.cx = 100;
				LoadString(g_main_instance, IDS_STRING_CHTD_07 , str, 32);
				lvcol.pszText = str;
				lvcol.iSubItem = 1;
				ListView_InsertColumn(hList, 1, &lvcol);
				lvcol.cx = 110;
				LoadString(g_main_instance, IDS_STRING_CHTD_08 , str, 32);
				lvcol.pszText = str;
				lvcol.iSubItem = 2;
				ListView_InsertColumn(hList, 2, &lvcol);
			}
           return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
				case IDCANCEL:
					GetCheatSearchDialogState(hDlgWnd);
					DestroyWindow(hDlgWnd);
					return TRUE;
					break;
				case IDC_CT9BUTTON1:	//Search
					if(!emu)
						break;
					{
						DWORD i, tempinhex, temphex/*, temphex2, temprb1*/;
						int inwild=0;
						char adtext[31], *p;
						unsigned char *pD;
						GetCheatSearchDialogState(hDlgWnd);
						GetDlgItemText(hDlgWnd, IDC_CT9EDIT1, (LPTSTR)adtext, 25);
						if(g_Cheat_Info.cc_csrchrb1==4){
							for(i=0 , inwild=0; adtext[i]; ++i){
								if(adtext[i] == '?')
									inwild=1;
							}
							if(!inwild){
								if(!g_Cheat_Info.cc_csrchrb2){
									tempinhex=GetDlgItemInt(hDlgWnd, IDC_CT9EDIT1, NULL, FALSE);
								}
								else{
									if(!MyStrAtoh(adtext, &tempinhex)){
										tempinhex=0;
									}
								}
							}
						}
						else if(g_Cheat_Info.cc_csrchrb1==5){
							for(i=0 , inwild=0, p=adtext; ; ++p){
								if(*p == '-'){
									inwild=1;
								}
								else if(*p == ' ' || *p == 9){
								}
								else 
									break;
							}
							if(!inwild){
								if(!g_Cheat_Info.cc_csrchrb2){
									tempinhex=atoi(p);
								}
								else{
									if(!MyStrAtoh( p, &temphex))
										tempinhex=0;
								}
							}
							else{
								if(!g_Cheat_Info.cc_csrchrb2){
									tempinhex=atoi(p);
								}
								else{
									if(!MyStrAtoh( p, &temphex))
										tempinhex=0;
								}
							}
						}
						ListView_DeleteAllItems(hList);		//Clear
						g_Cheat_Info.nSearchOData = 0;
						if(IsDlgButtonChecked(hDlgWnd, IDC_CHECK1) && emu->GetMainMemp(&pD)){
							memcpy(g_Cheat_Info.pBUP_MainMem, g_Cheat_Info.pMainMemFlag, g_Cheat_Info.mainmem_size);
							memcpy(&g_Cheat_Info.pBUP_MainMem[g_Cheat_Info.mainmem_size], g_Cheat_Info.pMainMemData, g_Cheat_Info.mainmem_size);
							CheatSearchProc1(g_Cheat_Info.mainmem_size, g_Cheat_Info.pMainMemData, pD, g_Cheat_Info.pMainMemFlag, inwild, tempinhex);
							MyCheat11s(hList, g_Cheat_Info.mainmem_size, g_Cheat_Info.pMainMemFlag, g_Cheat_Info.pMainMemData, pD, 0);
							memcpy(g_Cheat_Info.pMainMemData, pD, g_Cheat_Info.mainmem_size);
						}
						if(IsDlgButtonChecked(hDlgWnd, IDC_CHECK2) && emu->GetSramMemp(&pD)){
							memcpy(g_Cheat_Info.pBUP_SRAM, g_Cheat_Info.pSRAMFlag, g_Cheat_Info.sram_size);
							memcpy(&g_Cheat_Info.pBUP_SRAM[g_Cheat_Info.sram_size], g_Cheat_Info.pSRAMData, g_Cheat_Info.sram_size);
							CheatSearchProc1(g_Cheat_Info.sram_size, g_Cheat_Info.pSRAMData, pD, g_Cheat_Info.pSRAMFlag, inwild, tempinhex);
							MyCheat11s(hList, g_Cheat_Info.sram_size, g_Cheat_Info.pSRAMFlag, g_Cheat_Info.pSRAMData, pD, 0x10000);
							memcpy(g_Cheat_Info.pSRAMData, pD, g_Cheat_Info.sram_size);
						}
						if(IsDlgButtonChecked(hDlgWnd, IDC_CHECK3) && emu->GetExtMemp(&pD)){
							memcpy(g_Cheat_Info.pBUP_ExtraMem, g_Cheat_Info.pExtraMemFlag, g_Cheat_Info.extramem_size);
							memcpy(&g_Cheat_Info.pBUP_ExtraMem[g_Cheat_Info.extramem_size], g_Cheat_Info.pExtraMemData, g_Cheat_Info.extramem_size);
							CheatSearchProc1(g_Cheat_Info.extramem_size, g_Cheat_Info.pExtraMemData, pD, g_Cheat_Info.pExtraMemFlag, inwild, tempinhex);
							MyCheat11s(hList, g_Cheat_Info.extramem_size, g_Cheat_Info.pExtraMemFlag, g_Cheat_Info.pExtraMemData, pD, 0x20000);
							memcpy(g_Cheat_Info.pExtraMemData, pD, g_Cheat_Info.extramem_size);
						}
						ListView_SetItemCount(hList, g_Cheat_Info.nSearchOData);
					}
					return TRUE;
					break;
				case IDC_CT9BUTTON2:	//Clear
					ListView_DeleteAllItems(hList);		//Clear
					if(!emu)
						break;
					InitCheatSearchMem(hDlgWnd);
					return TRUE;
					break;
				case IDC_CT9BUTTON3:	//Undo
					if(g_Cheat_Info.mainmem_size){
						memcpy(g_Cheat_Info.pMainMemFlag, g_Cheat_Info.pBUP_MainMem, g_Cheat_Info.mainmem_size);
						memcpy(g_Cheat_Info.pMainMemData, &g_Cheat_Info.pBUP_MainMem[g_Cheat_Info.mainmem_size], g_Cheat_Info.mainmem_size);
					}
					if(g_Cheat_Info.sram_size){
						memcpy(g_Cheat_Info.pSRAMFlag, g_Cheat_Info.pBUP_SRAM, g_Cheat_Info.sram_size);
						memcpy(g_Cheat_Info.pSRAMData, &g_Cheat_Info.pBUP_SRAM[g_Cheat_Info.sram_size], g_Cheat_Info.sram_size);
					}
					if(g_Cheat_Info.extramem_size){
						memcpy(g_Cheat_Info.pExtraMemFlag, g_Cheat_Info.pBUP_ExtraMem, g_Cheat_Info.extramem_size);
						memcpy(g_Cheat_Info.pExtraMemData, &g_Cheat_Info.pBUP_ExtraMem[g_Cheat_Info.extramem_size], g_Cheat_Info.extramem_size);
					}
					return TRUE;
					break;
				case IDC_CT9RADIOC1:
				case IDC_CT9RADIOC2:
				case IDC_CT9RADIOC3:
				case IDC_CT9RADIOC4:
					GetCheatSearchDialogState(hDlgWnd);
					EnableWindow(hEdit , FALSE);
					return TRUE;
					break;
				case IDC_CT9RADIOC5:
				case IDC_CT9RADIOC6:
					GetCheatSearchDialogState(hDlgWnd);
					EnableWindow(hEdit , TRUE);
					SetFocus(hEdit);
					return TRUE;
					break;
				case IDC_CT9RADIO1:case IDC_CT9RADIO2:case IDC_CT9RADIO3:
				case IDC_CT9RADIO4:case IDC_CT9RADIO5:case IDC_CT9RADIO6:
					GetCheatSearchDialogState(hDlgWnd);
					InvalidateRect(hList, NULL, TRUE);
					break;
				case IDM_CTPOP1:
					GetCheatSearchDialogState(hDlgWnd);
/*
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO3), BM_GETCHECK, (WPARAM)TRUE, 0L))
						ctcmpdtf2=0;
					else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO4), BM_GETCHECK, (WPARAM)TRUE, 0L))
						ctcmpdtf2=1;
					else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO5), BM_GETCHECK, (WPARAM)TRUE, 0L))
						ctcmpdtf2=3;
					else
						ctcmpdtf2=2;
*/
					{
					int i=-1;
					char adtext[64];
					if((i=ListView_GetNextItem(hList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
						return TRUE;
					ListView_GetItemText(hList, i, 0, adtext, 15);			//Address
					MyStrAtoh(adtext, &MyCheatAdr);
					if(IDOK==DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG7), hDlgWnd, (DLGPROC)MyCheatDlg9)){
					}
					}
					return TRUE;
					break;
 				case IDM_CTPOP2:
//					ctcmpdtf2=(unsigned char)SendMessage(GetDlgItem(hDlgWnd, IDC_CT9RADIO3), BM_GETCHECK, (WPARAM)TRUE, 0L);
					GetCheatSearchDialogState(hDlgWnd);
					{
					int i=-1;
					char adtext[64];
					if((i=ListView_GetNextItem(hList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
						return TRUE;
					ListView_GetItemText(hList, i, 0, adtext, 15);			//Address
					MyStrAtoh(adtext, &MyCheatAdr);
					if(IDOK==DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG8), hDlgWnd, (DLGPROC)MyCheatDlg10)){
					}
					}
					return TRUE;
			}
            break;
		case WM_CONTEXTMENU:
			{
				POINT pt;
				HMENU hMenu, hSubMenu;
				pt.x = LOWORD(lp);
				pt.y = HIWORD(lp);
				
				if (wp == (WPARAM)hList) {
					//				ClientToScreen(hWnd, &pt);
					hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_POPMENU1));
					hSubMenu = GetSubMenu(hMenu, 0);
					TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_TOPALIGN,
						pt.x, pt.y, 0, hDlgWnd, NULL);
					DestroyMenu(hSubMenu);
					DestroyMenu(hMenu);
				}
			}
			return TRUE;
			break;
		case WM_NOTIFY:
			{
				LV_DISPINFO *lvinfo = (LV_DISPINFO *)lp;
				if((int)wp == IDC_CT9LIST){
					switch(lvinfo->hdr.code){
						case LVN_GETDISPINFO:
							CheatSearchOnGetDispInfo((NMLVDISPINFO *)lp);
							break;
					}
				}
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hDlgWnd);
			break;
		case WM_DESTROY:
			DeleteObject(hlFont);
			g_extra_window.CheatSearch = NULL;
			return TRUE;
//		case WM_EXTRAWIN_01:
		case WM_EXTRAWIN_02:
			ListView_SetItemCount(hList, 0);
			InitCheatSearchMem(hDlgWnd);
			break;

/*		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			DeleteObject(hlFont);
			return TRUE;
			break;
*/
    }
    return FALSE;
}



void FreeCheatStruct(){
	freemem_macro(g_Cheat_Info.pExtraCmp);
	freemem_macro(g_Cheat_Info.pExtraMemData);
	freemem_macro(g_Cheat_Info.pExtraMemFlag);
	freemem_macro(g_Cheat_Info.pMainCmp);
	freemem_macro(g_Cheat_Info.pMainMemData);
	freemem_macro(g_Cheat_Info.pMainMemFlag);
	freemem_macro(g_Cheat_Info.pSRAMCmp);
	freemem_macro(g_Cheat_Info.pSRAMData);
	freemem_macro(g_Cheat_Info.pSRAMFlag);
	freemem_macro(g_Cheat_Info.pBUP_MainMem);
	freemem_macro(g_Cheat_Info.pBUP_SRAM);
	freemem_macro(g_Cheat_Info.pBUP_ExtraMem);

	freemem_macro(g_Cheat_Info.pSearchOData);
	freemem_macro(g_Cheat_Info.pCmpOData);

	for(int i=0; i<CTRESTMAX; i++){
		if(g_Cheat_Info.ctrestds[i].pSubcode)
			free(g_Cheat_Info.ctrestds[i].pSubcode);
	}
//	memset(&g_Cheat_Info, 0, sizeof(g_Cheat_Info));
}


void InitCheatStruct(){
	if(!emu)
		return;
	unsigned char *p;
	int i;

	g_Cheat_Info.mainmem_size = emu->GetMainMemp(&p);
	g_Cheat_Info.sram_size=		emu->GetSramMemp(&p);
	g_Cheat_Info.extramem_size= emu->GetExtMemp(&p);

	g_Cheat_Info.pMainMemData = (unsigned char *)malloc(g_Cheat_Info.mainmem_size);
	g_Cheat_Info.pMainMemFlag = (unsigned char *)malloc(g_Cheat_Info.mainmem_size);
	g_Cheat_Info.pSRAMData =	(unsigned char *)malloc(g_Cheat_Info.sram_size);
	g_Cheat_Info.pSRAMFlag =	(unsigned char *)malloc(g_Cheat_Info.sram_size);
	g_Cheat_Info.pExtraMemData= (unsigned char *)malloc(g_Cheat_Info.extramem_size);
	g_Cheat_Info.pExtraMemFlag= (unsigned char *)malloc(g_Cheat_Info.extramem_size);
	memset(g_Cheat_Info.pMainMemData, 0, g_Cheat_Info.mainmem_size);
	memset(g_Cheat_Info.pSRAMData, 0, g_Cheat_Info.sram_size);
	memset(g_Cheat_Info.pExtraMemData, 0, g_Cheat_Info.extramem_size);

	memset(g_Cheat_Info.pMainMemFlag, 0, g_Cheat_Info.mainmem_size);
	memset(g_Cheat_Info.pSRAMFlag, 0, g_Cheat_Info.sram_size);
	memset(g_Cheat_Info.pExtraMemFlag, 0, g_Cheat_Info.extramem_size);

	g_Cheat_Info.pMainCmp = (unsigned char *)malloc(g_Cheat_Info.mainmem_size*3);
	g_Cheat_Info.pSRAMCmp = (unsigned char *)malloc(g_Cheat_Info.sram_size*3);
	g_Cheat_Info.pExtraCmp= (unsigned char *)malloc(g_Cheat_Info.extramem_size*3);
	memset(g_Cheat_Info.pMainCmp, 0, g_Cheat_Info.mainmem_size*3);
	memset(g_Cheat_Info.pSRAMCmp, 0, g_Cheat_Info.sram_size*3);
	memset(g_Cheat_Info.pExtraCmp, 0, g_Cheat_Info.extramem_size*3);

	g_Cheat_Info.pBUP_MainMem = (unsigned char *)malloc(g_Cheat_Info.mainmem_size*2);
	g_Cheat_Info.pBUP_SRAM =	(unsigned char *)malloc(g_Cheat_Info.sram_size*2);
	g_Cheat_Info.pBUP_ExtraMem= (unsigned char *)malloc(g_Cheat_Info.extramem_size*2);
	memset(g_Cheat_Info.pBUP_MainMem, 0, g_Cheat_Info.mainmem_size*2);
	memset(g_Cheat_Info.pBUP_SRAM, 0, g_Cheat_Info.sram_size*2);
	memset(g_Cheat_Info.pBUP_ExtraMem, 0, g_Cheat_Info.extramem_size*2);

	i = sizeof(int)*(g_Cheat_Info.mainmem_size+g_Cheat_Info.sram_size+g_Cheat_Info.extramem_size);
	g_Cheat_Info.pSearchOData = (int *)malloc(i);
	g_Cheat_Info.pCmpOData = (int *)malloc(i);
	memset(g_Cheat_Info.pSearchOData, 0, i);
	memset(g_Cheat_Info.pCmpOData, 0, i);

	ZeroMemory(&g_Cheat_Info.ctrestds, sizeof(ctrestoren)*CTRESTMAX);
	g_Cheat_Info.ctrestnum=0;
	if(PrevFastSw)
		emu->ToggleFastFPS();
	return;
}


//
void CheatDataToCheatCode(struct ctrestoren *data, char *Code){
	char *p = Code;
	unsigned int i;

	if(data->opcode != 0){
		p += wsprintf(p, "%02X", data->opcode);
	}
	if(data->nSubcode && data->pSubcode){
		p += wsprintf(p, "[");
		for(i=0; i<data->nSubcode; i++){
			if(i){
				p += wsprintf(p, ",");
			}
			p += wsprintf(p, "%02X", data->pSubcode[i].opcode);
			p += wsprintf(p, "-%04X", data->pSubcode[i].ctresadd);
			p += wsprintf(p, "-%X", data->pSubcode[i].ctresssize);
			p += wsprintf(p, "-%X", data->pSubcode[i].ctresdata);
#if 1
			if(data->pSubcode[i].ctrescomm[0]){
				p += wsprintf(p, "-%s", data->pSubcode[i].ctrescomm);
			}
#endif
		}
		p += wsprintf(p, "]");
	}
	if(data->opcode != 0){
		p += wsprintf(p, "-");
	}
	p += wsprintf(p, "%04X", data->ctresadd);
	p += wsprintf(p, "-%X", data->ctresssize);
	p += wsprintf(p, "-%X", data->ctresdata);
	if(data->ctrescomm[0]){
		p += wsprintf(p, "-%s", data->ctrescomm);
	}
}


int CheatCodeToCheatData_Proc(struct ctrestoren *data, char *Code){
	char str[260], *p=Code;
	int i, CodeBlock=0;
	DWORD dw, nSub;
//ZZ[ZZ-xxxx-y-nn-comment, ]-xxxx-y-nn- = 
//   ↑(条件分岐時のみ)

	nSub = 0;
	for(i=0; *p!='[' && *p!='-'&& *p!=0; p++, i++){
		str[i] = *p;
	}
	if(*p==0)
		return 0;
	str[i] = 0;
	data->ctressflag = 1;	//Hex
	if(*p=='-' && i>=4){	//old version code
//		int j;
		if(!MyStrAtoh(str, &dw))
			return 0;		//error
		data->ctresadd = dw;
		p++;
		for(i=0; *p!=0 && *p!='-'; p++, i++){
			str[i] = *p;
		}
		str[i] = 0;
		if(*p==0 || i==0)
			return 0;
		if(!MyStrAtoh(str, &dw))
			return 0;		//error
		p++;
		data->ctresssize = dw;
		for(i=0; *p!=0 && *p!='-' && *p!=' '; p++, i++){
			str[i] = *p;
		}
		str[i] = 0;
		if(i == 0){
			return 0;
		}
		if(!MyStrAtoh(str, &dw))
			return 0;		//error
		data->ctresdata = dw;
		if(*p == 0)
			return 1;
		p++;
		for(i=0; *p!=0 && *p!='-'; p++, i++){
			str[i] = *p;
		}
		str[i] = 0;
		if(i == 0){
			return 0;
		}
		p++;
		if(strlen(str) <=32){
			strcpy(data->ctrescomm, str);
		}
		return 1;
	}
	if(*p=='[' && i == 0){
		dw = 0x18;
		CodeBlock = 1;		// Code 18
		data->ctresssize = 1;
	}
	else if(!MyStrAtoh(str, &dw))
		return 0;		//error
	data->opcode = dw;
	if(*p=='['){		//subcode
		if(dw<0x10){	// invalid code
			while(*p!=0 && *p!='-')p++;
			if(*p==0)
				return 0;
		}
		else{
//			data->pSubcode = (struct ctrestoren *)malloc(sizeof(struct ctrestoren));
//			memset(data->pSubcode, 0, sizeof(struct ctrestoren));
			p++;
			while(1){
				data->pSubcode = (struct ctrestoren *)realloc(data->pSubcode, sizeof(struct ctrestoren)*(nSub+1));
				memset(&data->pSubcode[nSub], 0, sizeof(struct ctrestoren));
				data->pSubcode[nSub].ctressflag = 1;	//Hex
				data->pSubcode[nSub].ctrescflag = 1;	//Enable
				for(i=0; *p!=']' && *p!='-'&& *p!=0; p++, i++){
					str[i] = *p;
				}
				if(*p==0)
					return 0;
				str[i] = 0;
#if 0
				if(*p=='-' && i>=4){	//old version code
					int j;
					if(!MyStrAtoh(str, &dw))
						return 0;		//error
					data->ctresadd = dw;
					p++;
					for(i=0; *p!=0 && *p!='-'; p++, i++){
						str[i] = *p;
					}
					str[i] = 0;
					if(*p==0 || i==0)
						return 0;
					if(!MyStrAtoh(str, &dw))
						return 0;		//error
					p++;
					data->ctresssize = dw;
					for(i=0; *p!=0 && *p!='-' && *p!=' '; p++, i++){
						str[i] = *p;
					}
					str[i] = 0;
					if(i == 0){
						return 0;
					}
					if(!MyStrAtoh(str, &dw))
						return 0;		//error
					data->ctresdata = dw;
					if(*p == 0)
						return 1;
					p++;
					for(i=0; *p!=0 && *p!='-'; p++, i++){
						str[i] = *p;
					}
					str[i] = 0;
					if(i == 0){
						return 0;
					}
					p++;
					if(strlen(str) <=32){
						strcpy(data->ctrescomm, str);
					}
					return 1;
				}
#endif
				if(!MyStrAtoh(str, &dw))
					return 0;		//error
				data->pSubcode[nSub].opcode = dw;
				if(*p=='[')			//subcode(pass)
					while(*p!='-'&& *p!=0)p++;
				if(*p==0)
					return 0;
				p++;
				for(i=0; *p!='-'&& *p!=0; p++, i++){		//address
					str[i] = *p;
				}
				if(*p==0)
					return 0;
				str[i] = 0;
				if(!MyStrAtoh(str, &dw))
					return 0;		//error
				data->pSubcode[nSub].ctresadd = dw;
				p++;
				for(i=0; *p!='-'&& *p!=0; p++, i++){		//size
					str[i] = *p;
				}
				if(*p==0)
					return 0;
				str[i] = 0;
				if(!MyStrAtoh(str, &dw))
					return 0;		//error
				data->pSubcode[nSub].ctresssize = dw;
				p++;
				for(i=0; *p!='-'&& *p!=','&& *p!=']'&& *p!=0; p++, i++){		//data
					str[i] = *p;
				}
				if(*p==0)
					return 0;
				str[i] = 0;
				if(!MyStrAtoh(str, &dw))
					return 0;		//error
				data->pSubcode[nSub].ctresdata = dw;
				if(*p=='-'){
					p++;
					for(i=0; *p!=','&& *p!=']'&& *p!=0; p++, i++){		//comment
						str[i] = *p;
					}
					if(*p==0)
						return 0;
					if(strlen(str) >= 32)
						return 0;
					strcpy(data->pSubcode[nSub].ctrescomm, str);
				}
				nSub++;
				if(*p==']'){
					p++;
					break;
				}
				p++;
				while(*p==' ')p++;
			}
			data->nSubcode = nSub;
			if(CodeBlock)
				return 1;
		}
	}
	p++;
	for(i=0; *p!='-'&& *p!=0; p++, i++){		//address
		str[i] = *p;
	}
	if(*p==0)
		return 0;
	str[i] = 0;
	if(!MyStrAtoh(str, &dw))
		return 0;		//error
	data->ctresadd = dw;
	p++;
	for(i=0; *p!='-'&& *p!=0; p++, i++){		//size
		str[i] = *p;
	}
	if(*p==0)
		return 0;
	str[i] = 0;
	if(!MyStrAtoh(str, &dw))
		return 0;		//error
	data->ctresssize = (unsigned char)dw;
	p++;
	for(i=0; *p!='-'&& *p!=0; p++, i++){		//data
		str[i] = *p;
	}
//	if(*p==0)
//		return 0;
	str[i] = 0;
	if(!MyStrAtoh(str, &dw))
		return 0;		//error
	data->ctresdata = dw;
	if(*p=='-'){
		p++;
		strcpy(data->ctrescomm, p);
	}
	return 1;
}


int CheatCodeToCheatData(struct ctrestoren *data, char *Code){
	memset(data, 0, sizeof(struct ctrestoren));
	if(!CheatCodeToCheatData_Proc(data, Code)){
		freemem_macro(data->pSubcode);
		return 0;
	}
	data->ctrescflag = 1;		// Code Enable
	return 1;
}


void WriteCheatFileProc_v2(struct ctrestoren *data, HANDLE hFile){
	DWORD dwAccBytes;
	WriteFile(hFile, &data->ctrescomm, 36, &dwAccBytes, NULL);
	WriteFile(hFile, &data->ctresadd, 4, &dwAccBytes, NULL);
	WriteFile(hFile, &data->ctresdata, 4, &dwAccBytes, NULL);
	WriteFile(hFile, &data->opcode, 4, &dwAccBytes, NULL);
	WriteFile(hFile, &data->nSubcode, 4, &dwAccBytes, NULL);
	WriteFile(hFile, &data->ctrescflag, 1, &dwAccBytes, NULL);
	WriteFile(hFile, &data->ctressflag, 1, &dwAccBytes, NULL);
	WriteFile(hFile, &data->ctresssize, 1, &dwAccBytes, NULL);
}


int ReadCheatFileProc_v2(struct ctrestoren *data, HANDLE hFile){
	DWORD dwAccBytes;
	ReadFile(hFile, &data->ctrescomm, 36, &dwAccBytes, NULL);
	if(dwAccBytes!=36)
		return 0;
	ReadFile(hFile, &data->ctresadd, 4, &dwAccBytes, NULL);
	ReadFile(hFile, &data->ctresdata, 4, &dwAccBytes, NULL);
	ReadFile(hFile, &data->opcode, 4, &dwAccBytes, NULL);
	ReadFile(hFile, &data->nSubcode, 4, &dwAccBytes, NULL);
	ReadFile(hFile, &data->ctrescflag, 1, &dwAccBytes, NULL);
	ReadFile(hFile, &data->ctressflag, 1, &dwAccBytes, NULL);
	ReadFile(hFile, &data->ctresssize, 1, &dwAccBytes, NULL);
	return 1;
}


void CheatfSave(){
	char str[MAX_PATH], temp[16];
	DWORD wd;
	HANDLE hFile;
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	DWORD dwAccBytes;

//	if(!ctrestnum){
//		return;
//	}
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	if(NESTER_settings.path.UseNNNchtPath){
		PathCombine(str , str, NESTER_settings.path.szNNNcheatPath);
	}
	else{
		PathCombine(str , str, "Cheat");
	}
	hFind = FindFirstFile(str, &fd);
	if(hFind == INVALID_HANDLE_VALUE || (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))){
		CreateDirectory(str , NULL);
	}
	FindClose(hFind);
	PathCombine(str , str, emu->getROMname());
	PathAddExtension(str, ".cht");
	if(!g_Cheat_Info.ctrestnum){
		hFind = FindFirstFile(str, &fd);
		if(hFind == INVALID_HANDLE_VALUE){
			FindClose(hFind);
			return;
		}
		FindClose(hFind);
	}
	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
    	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return;
		}
    }
	wd=2;
	ZeroMemory(temp, sizeof(temp));
	if(!WriteFile(hFile, &wd, 4, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	WriteFile(hFile, &g_Cheat_Info.ctrestnum, 4, &dwAccBytes, NULL);
	*((DWORD *)&temp[0]) = emu->crc32();
	WriteFile(hFile, temp, 8, &dwAccBytes, NULL);
	unsigned int i,j;
	for(i=0; i<(unsigned int)g_Cheat_Info.ctrestnum; i++){
		WriteCheatFileProc_v2(&g_Cheat_Info.ctrestds[i], hFile);
		if(g_Cheat_Info.ctrestds[i].nSubcode){
			for(j=0; j<g_Cheat_Info.ctrestds[i].nSubcode; j++){
				WriteCheatFileProc_v2(&g_Cheat_Info.ctrestds[i].pSubcode[j], hFile);
			}
		}
	}
	CloseHandle(hFile);
	return;
}




void CheatfLoad(){
	char str[MAX_PATH];
	DWORD i, wd;
	HANDLE hFile;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	if(NESTER_settings.path.UseNNNchtPath){
		PathCombine(str , str, NESTER_settings.path.szNNNcheatPath);
	}
	else{
		PathCombine(str , str, "Cheat");
	}
	PathCombine(str , str, emu->getROMname());
	PathAddExtension(str, ".cht");
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		return;
    }
	wd=0;
	if(!ReadFile(hFile, &wd, 4, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	if(4 != dwAccBytes){
		CloseHandle(hFile);
		return;
	}
	if(!wd){
		ReadFile(hFile, &wd, 4, &dwAccBytes, NULL);
		if(4 != dwAccBytes){
			CloseHandle(hFile);
			g_Cheat_Info.ctrestnum=0;
			return;
		}
		if(-1== SetFilePointer(hFile, 0x10, 0, FILE_BEGIN)){
			CloseHandle(hFile);
			return;
		}
		for(i=0; i<wd; ++i){
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctrescomm, 17, &dwAccBytes, NULL);
			if(17 != dwAccBytes){
				CloseHandle(hFile);
				return;
			}
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctresadd, 4, &dwAccBytes, NULL);
			if(4 != dwAccBytes){
				CloseHandle(hFile);
				return;
			}
			g_Cheat_Info.ctrestds[i].ctresdata=0;
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctresdata, 1, &dwAccBytes, NULL);
			if(1 != dwAccBytes){
				CloseHandle(hFile);
				return;
			}
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctrescflag, 2, &dwAccBytes, NULL);
			if(2 != dwAccBytes){
				CloseHandle(hFile);
				return;
			}
			g_Cheat_Info.ctrestds[i].ctresssize=1;
		}
		g_Cheat_Info.ctrestnum=wd;
		CloseHandle(hFile);
		return;
	}
	else if(wd==1){
		ReadFile(hFile, &wd, 4, &dwAccBytes, NULL);
		if(4 != dwAccBytes){
			CloseHandle(hFile);
			g_Cheat_Info.ctrestnum=0;
			return;
		}
		if(-1== SetFilePointer(hFile, 0x10, 0, FILE_BEGIN)){
			CloseHandle(hFile);
			return;
		}
		for(i=0; i<wd; ++i){
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctrescomm, 36, &dwAccBytes, NULL);
			if(36 != dwAccBytes){
				CloseHandle(hFile);
				return;
			}
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctresadd, 4, &dwAccBytes, NULL);
			if(4 != dwAccBytes){
				CloseHandle(hFile);
				return;
			}
			g_Cheat_Info.ctrestds[i].ctresdata=0;
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctresdata, 4, &dwAccBytes, NULL);
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctrescflag, 1, &dwAccBytes, NULL);
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctressflag, 1, &dwAccBytes, NULL);
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctresssize, 1, &dwAccBytes, NULL);
			ReadFile(hFile, &g_Cheat_Info.ctrestds[i].ctresssize, 1, &dwAccBytes, NULL);
//			g_Cheat_Info.ctrestds[i].ctresssize++;
			if(g_Cheat_Info.ctrestds[i].ctresssize == 2)
				g_Cheat_Info.ctrestds[i].ctresssize = 4;
			else if(g_Cheat_Info.ctrestds[i].ctresssize == 3)
				g_Cheat_Info.ctrestds[i].ctresssize = 3;
			else g_Cheat_Info.ctrestds[i].ctresssize++;
		}
		g_Cheat_Info.ctrestnum=wd;
		CloseHandle(hFile);
		return;
	}
	else if(wd==2){
		ReadFile(hFile, &wd, 4, &dwAccBytes, NULL);
		if(4 != dwAccBytes){
			CloseHandle(hFile);
			g_Cheat_Info.ctrestnum=0;
			return;
		}
		if(-1== SetFilePointer(hFile, 0x10, 0, FILE_BEGIN)){
			CloseHandle(hFile);
			return;
		}
		for(i=0; i<wd; ++i){
			if(!ReadCheatFileProc_v2(&g_Cheat_Info.ctrestds[i], hFile)){
				CloseHandle(hFile);
				return;
			}
			if(g_Cheat_Info.ctrestds[i].nSubcode){
				g_Cheat_Info.ctrestds[i].pSubcode = (struct ctrestoren *)malloc(sizeof(struct ctrestoren)*g_Cheat_Info.ctrestds[i].nSubcode);
				memset(g_Cheat_Info.ctrestds[i].pSubcode, 0, sizeof(struct ctrestoren)*g_Cheat_Info.ctrestds[i].nSubcode);
				for(unsigned int j=0; j<g_Cheat_Info.ctrestds[i].nSubcode; j++){
					ReadCheatFileProc_v2(&g_Cheat_Info.ctrestds[i].pSubcode[j], hFile);
				}
			}
		}
		g_Cheat_Info.ctrestnum=wd;
		CloseHandle(hFile);
	}
	return;
}


//File Out Cheat   IDD_CTDIALOG10
LRESULT CALLBACK MyCheatDlg12(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
        case WM_INITDIALOG:
			SendMessage(GetDlgItem(hDlgWnd, IDC_CT10CHECK1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CT10CHECK2), BM_SETCHECK, (WPARAM)TRUE, 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CT10CHECK3), BM_SETCHECK, (WPARAM)TRUE, 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CT10CHECK4), BM_SETCHECK, (WPARAM)TRUE, 0L);
//			SetWindowText(hEdit , adtext);
			return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					MyCheatAdr=0;
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT10CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						MyCheatAdr=1;
					}
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT10CHECK2), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						MyCheatAdr|=2;
					}
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT10CHECK3), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						MyCheatAdr|=4;
					}
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT10CHECK4), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						MyCheatAdr|=8;
					}
  					EndDialog(hDlgWnd, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			return TRUE;
			break;
    }
    return FALSE;
}


#if 0
void CheatdfSave(){
	char str[MAX_PATH], temp[16];
	DWORD wd;
	HANDLE hFile;
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	if(NESTER_settings.path.UseNNNchtPath){
		PathCombine(str , str, NESTER_settings.path.szNNNcheatPath);
	}
	else{
		PathCombine(str , str, "Cheat");
	}
	hFind = FindFirstFile(str, &fd);
	if(hFind == INVALID_HANDLE_VALUE || (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))){
		CreateDirectory(str , NULL);
	}
	FindClose(hFind);
	PathCombine(str , str, emu->getROMname());
	PathAddExtension(str, ".cdt");
	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
    	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return;
		}
    }
	wd=0;
	ZeroMemory(temp, sizeof(temp));
	if(!WriteFile(hFile, &wd, 4, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	WriteFile(hFile, &MyCheatAdr, 4, &dwAccBytes, NULL);
	WriteFile(hFile, temp, 8, &dwAccBytes, NULL);
	if(MyCheatAdr & 1){
		WriteFile(hFile, ctcmpdt[0], 0x800, &dwAccBytes, NULL);
	}
	if(MyCheatAdr & 2){
		WriteFile(hFile, ctcmpdt[1], 0x800, &dwAccBytes, NULL);
	}
	if(MyCheatAdr & 4){
		WriteFile(hFile, ctcmpdt[2], 0x800, &dwAccBytes, NULL);
	}
	if(MyCheatAdr & 8){
		WriteFile(hFile, ctcmp2data, 0x800, &dwAccBytes, NULL);
		WriteFile(hFile, ctcmp2flag, 0x800, &dwAccBytes, NULL);
	}
	CloseHandle(hFile);
	return;
}


void CheatdfLoad(){
	char str[MAX_PATH], temp[16];
	DWORD wd;
	HANDLE hFile;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	if(NESTER_settings.path.UseNNNchtPath){
		PathCombine(str , str, NESTER_settings.path.szNNNcheatPath);
	}
	else{
		PathCombine(str , str, "Cheat");
	}
	PathCombine(str , str, emu->getROMname());
	PathAddExtension(str, ".cdt");
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		return;
    }
	wd=0;
	if(!ReadFile(hFile, &wd, 4, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	if(4 != dwAccBytes){
		CloseHandle(hFile);
		return;
	}
	ReadFile(hFile, &wd, 4, &dwAccBytes, NULL);
	if(4 != dwAccBytes){
		CloseHandle(hFile);
		ctrestnum=0;
		return;
	}
	ReadFile(hFile, temp, 8, &dwAccBytes, NULL);
	if(8 != dwAccBytes){
		CloseHandle(hFile);
		ctrestnum=0;
		return;
	}
	if(MyCheatAdr & 1){
		if(!ReadFile(hFile, ctcmpdt[0], 0x800, &dwAccBytes, NULL))
			return;
	}
	if(MyCheatAdr & 2){
		if(!ReadFile(hFile, ctcmpdt[1], 0x800, &dwAccBytes, NULL))
			return;
	}
	if(MyCheatAdr & 4){
		if(!ReadFile(hFile, ctcmpdt[2], 0x800, &dwAccBytes, NULL))
			return;
	}
	if(MyCheatAdr & 8){
		if(!ReadFile(hFile, ctcmp2data, 0x800, &dwAccBytes, NULL))
			return;
		if(!ReadFile(hFile, ctcmp2flag, 0x800, &dwAccBytes, NULL))
			return;
	}
	CloseHandle(hFile);
	return;
}
#endif


int MystrCmp(char *sa,char *sb){
	int	i=0;
	while(sa[i]){
		if(IsDBCSLeadByte((unsigned char)sa[i])){
			if(sa[i] != sb[i] || sa[i+1] != sb[i+1] )
				return 0;
			i+=2;
		}
		else{
			if((sa[i] >= 'A' && sa[i] <= 'Z' || sa[i] >= 'a' && sa[i] <= 'z')&&(sb[i] >= 'A' && sb[i] <= 'Z' || sb[i] >= 'a' && sb[i] <= 'z')){
				if((sa[i] | 0x20) != (sb[i] | 0x20))
					return 0;
			}
			else{
				if(sa[i] != sb[i])
					return 0;
			}
			++i;
		}
	}
	if(sa[i]==sb[i])
		return 1;
	return 0;
}




//Change Cheat Code Value(Hex)  IDD_CTDIALOG11
LRESULT CALLBACK MyCheatDlg14(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND hEdit, hEdit2, hEdit3;

    switch(msg) {
        case WM_INITDIALOG:
			hEdit = GetDlgItem(hDlgWnd, IDC_CT11EDIT1);
			hEdit2 = GetDlgItem(hDlgWnd, IDC_CT11EDIT2);
			hEdit3 = GetDlgItem(hDlgWnd, IDC_CT11EDIT3);
			SetWindowText(hEdit, (char *)SResulttext);
			SetWindowText(hEdit2, (char *)SResulttext2);
			SetWindowText(hEdit3, (char *)SResulttext3);
            return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText(hDlgWnd, IDC_CT11EDIT1, (LPTSTR)SResulttext, 6);
					GetDlgItemText(hDlgWnd, IDC_CT11EDIT2, (LPTSTR)SResulttext2, 9);
					GetDlgItemText(hDlgWnd, IDC_CT11EDIT3, (LPTSTR)SResulttext3, 32);
					EndDialog(hDlgWnd, IDOK);
                   return TRUE;
				   break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			break;
    }
    return FALSE;
}


int MystrJscmp(char *stra,char *strb, char sp){
	int i=0, j=0, f1=0, f2=0;
	while(stra[i])++i;
	while(strb[j])++j;
	if(i>j)
		j=i;
	i=0;

	if(sp == 0){
		if(stra[0]==0 && strb[0]!=0)
			return 1;
		if(strb[0]==0 && stra[0]!=0)
			return -1;
	}
	if(sp == 1){
		if(stra[0]==0 && strb[0]!=0)
			return -1;
		if(strb[0]==0 && stra[0]!=0)
			return 1;
	}
	while(j--){
		if(f1==0){
			if(IsDBCSLeadByte((unsigned char)stra[i]))
				f1=1;
		}
		else
			++f1;
		if(f2==0){
			if(IsDBCSLeadByte((unsigned char)strb[i]))
				f2=1;
		}
		else
			++f2;
		if(f1 || f2){
			if((unsigned char)stra[i] == (unsigned char)strb[i])
					++i;
			else if((unsigned char)stra[i] > (unsigned char)strb[i])
				return 1;
			else if((unsigned char)stra[i] < (unsigned char)strb[i])
				return -1;
		}
		else{
			if((stra[i] >= 'A' && stra[i] <= 'Z' || stra[i] >= 'a' && stra[i] <= 'z')&&(strb[i] >= 'A' && strb[i] <= 'Z' || strb[i] >= 'a' && strb[i] <= 'z')){
				if((stra[i] | 0x20) == (strb[i] | 0x20))
					++i;
				else if((stra[i] | 0x20) > (strb[i] | 0x20))
					return 1;
				else if((stra[i] | 0x20) < (strb[i] | 0x20))
					return -1;
			}
			else{
				if(stra[i] == strb[i])
					++i;
				else if((unsigned char)stra[i] > (unsigned char)strb[i])
					return 1;
				else if((unsigned char)stra[i] < (unsigned char)strb[i])
					return -1;
			}
		}
		if(f1==2)
			f1=0;
		if(f2==2)
			f2=0;
	}
	return 0;
}


#if 0

void CheatfSave2(HWND hWnd){
	char str[MAX_PATH], temp[32];
	DWORD wd;
	HANDLE hFile;
	DWORD dwAccBytes;
    OPENFILENAME ofn;
	char szFile[MAX_PATH]="";
	char FileName[MAX_PATH]="";

//	if(!ctrestnum){
//		return;
//	}
	strcpy(FileName, emu->getROMname());
	PathAddExtension(FileName, ".cht");
	GetModuleFileName(NULL, str, MAX_PATH);
	PathStripPath(str);
	if(NESTER_settings.path.UseNNNchtPath){
		PathCombine(str , str, NESTER_settings.path.szNNNcheatPath);
	}
	else{
		PathCombine(str , str, "Cheat");
	}

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Cheat Files(*.cht)\0*.cht\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = FileName;
    ofn.lpstrFileTitle = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "cht";
	LoadString(g_main_instance, IDS_STRING_CDLG_02 , temp, 32);
	ofn.lpstrTitle	=temp;
	ofn.lpstrInitialDir = str;
	if(GetSaveFileName(&ofn) == 0)
        return;
	hFile = CreateFile(FileName, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
    	hFile = CreateFile(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return;
		}
    }
	wd=1;
	ZeroMemory(temp, sizeof(temp));
	if(!WriteFile(hFile, &wd, 4, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	WriteFile(hFile, &ctrestnum, 4, &dwAccBytes, NULL);
	WriteFile(hFile, temp, 8, &dwAccBytes, NULL);
	WriteFile(hFile, &ctrestds, sizeof(ctrestoren)*ctrestnum, &dwAccBytes, NULL);
	CloseHandle(hFile);
	return;
}


void CheatfLoad2(HWND hWnd){
	char str[MAX_PATH], temp[32];
	DWORD wd, i;
	HANDLE hFile;
	DWORD dwAccBytes;
    OPENFILENAME ofn;
	char szFile[MAX_PATH]="";
	char FileName[MAX_PATH];

	strcpy(FileName, emu->getROMname());
	PathAddExtension(FileName, ".cht");
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	if(NESTER_settings.path.UseNNNchtPath){
		PathCombine(str , str, NESTER_settings.path.szNNNcheatPath);
	}
	else{
		PathCombine(str , str, "Cheat");
	}

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Cheat Files(*.cht)\0*.cht\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = FileName;
    ofn.lpstrFileTitle = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = "cht";
	LoadString(g_main_instance, IDS_STRING_CDLG_01 , temp, 32);
	ofn.lpstrTitle	=temp;
	ofn.lpstrInitialDir = str;

    if(GetOpenFileName(&ofn) == 0)
        return;
	hFile = CreateFile(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		return;
    }
	wd=0;
	if(!ReadFile(hFile, &wd, 4, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	if(4 != dwAccBytes){
		CloseHandle(hFile);
		return;
	}
	if(!wd){
		ReadFile(hFile, &wd, 4, &dwAccBytes, NULL);
		if(4 != dwAccBytes){
			CloseHandle(hFile);
			ctrestnum=0;
			return;
		}
		if(-1== SetFilePointer(hFile, 0x10, 0, FILE_BEGIN)){
			CloseHandle(hFile);
			return;
		}
		for(i=0; (unsigned int)i<wd; ++i){
			ReadFile(hFile, &ctrestds[i].ctrescomm, 17, &dwAccBytes, NULL);
			if(17 != dwAccBytes){
				CloseHandle(hFile);
				ctrestnum=0;
				return;
			}
			ReadFile(hFile, &ctrestds[i].ctresadd, 4, &dwAccBytes, NULL);
			if(4 != dwAccBytes){
				CloseHandle(hFile);
				ctrestnum=0;
				return;
			}
			ctrestds[i].ctresdata=0;
			ReadFile(hFile, &ctrestds[i].ctresdata, 1, &dwAccBytes, NULL);
			if(1 != dwAccBytes){
				CloseHandle(hFile);
				ctrestnum=0;
				return;
			}
			ReadFile(hFile, &ctrestds[i].ctrescflag, 2, &dwAccBytes, NULL);
			if(2 != dwAccBytes){
				CloseHandle(hFile);
				ctrestnum=0;
				return;
			}
			ctrestds[i].ctresssize=1;
		}
		ctrestnum=wd;
		CloseHandle(hFile);
		return;
	}
	else if(wd!=1)
		return;
	ReadFile(hFile, &ctrestnum, 4, &dwAccBytes, NULL);
	if(4 != dwAccBytes){
		CloseHandle(hFile);
		ctrestnum=0;
		return;
	}
	if(-1== SetFilePointer(hFile, 0x10, 0, FILE_BEGIN)){
		CloseHandle(hFile);
		return;
	}
	ReadFile(hFile, &ctrestds, sizeof(ctrestoren)*ctrestnum, &dwAccBytes, NULL);
	if(sizeof(ctrestoren)*ctrestnum != dwAccBytes){
		CloseHandle(hFile);
		ctrestnum=0;
		return;
	}
	CloseHandle(hFile);
	return;
}


void CheatfLoad3(HWND hWnd){
	char str[MAX_PATH], temp[32];
	DWORD wd, i,j;
	HANDLE hFile;
	DWORD dwAccBytes;
    OPENFILENAME ofn;
	char szFile[MAX_PATH]="";
	char FileName[MAX_PATH];

	strcpy(FileName, emu->getROMname());
	PathAddExtension(FileName, ".cht");
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	if(NESTER_settings.path.UseNNNchtPath){
		PathCombine(str , str, NESTER_settings.path.szNNNcheatPath);
	}
	else{
		PathCombine(str , str, "Cheat");
	}

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Cheat Files(*.cht)\0*.cht\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = FileName;
    ofn.lpstrFileTitle = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = "cht";
	LoadString(g_main_instance, IDS_STRING_CDLG_01 , temp, 32);
	ofn.lpstrTitle	=temp;
	ofn.lpstrInitialDir = str;

    if(GetOpenFileName(&ofn) == 0)
        return;
	hFile = CreateFile(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		return;
    }
	wd=0;
	if(!ReadFile(hFile, &wd, 4, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	if(4 != dwAccBytes){
		CloseHandle(hFile);
		return;
	}
	if(!wd){
		ReadFile(hFile, &wd, 4, &dwAccBytes, NULL);
		if(4 != dwAccBytes){
			CloseHandle(hFile);
			ctrestnum=0;
			return;
		}
		if(-1== SetFilePointer(hFile, 0x10, 0, FILE_BEGIN)){
			CloseHandle(hFile);
			return;
		}
		if((ctrestnum+wd) >= CTRESTMAX){
			wd=CTRESTMAX-(ctrestnum+1);
		}
		for(i=0, j=ctrestnum; i<wd; ++i, ++j){
			ReadFile(hFile, &ctrestds[j].ctrescomm, 17, &dwAccBytes, NULL);
			if(17 != dwAccBytes){
				CloseHandle(hFile);
				ctrestnum=0;
				return;
			}
			ReadFile(hFile, &ctrestds[j].ctresadd, 4, &dwAccBytes, NULL);
			if(4 != dwAccBytes){
				CloseHandle(hFile);
				ctrestnum=0;
				return;
			}
			ctrestds[j].ctresdata=0;
			ReadFile(hFile, &ctrestds[j].ctresdata, 1, &dwAccBytes, NULL);
			if(1 != dwAccBytes){
				CloseHandle(hFile);
				ctrestnum=0;
				return;
			}
			ReadFile(hFile, &ctrestds[j].ctrescflag, 2, &dwAccBytes, NULL);
			if(2 != dwAccBytes){
				CloseHandle(hFile);
				ctrestnum=0;
				return;
			}
			ctrestds[j].ctresssize=0;
		}
		ctrestnum=wd;
		CloseHandle(hFile);
		return;
	}
	else if(wd!=1)
		return;
	ReadFile(hFile, &wd, 4, &dwAccBytes, NULL);
	if(4 != dwAccBytes){
		CloseHandle(hFile);
		ctrestnum=0;
		return;
	}
	if(-1== SetFilePointer(hFile, 0x10, 0, FILE_BEGIN)){
		CloseHandle(hFile);
		return;
	}
	if((ctrestnum+wd) >= CTRESTMAX){
		wd=CTRESTMAX-(ctrestnum+1);
	}
	ReadFile(hFile, &ctrestds[ctrestnum], sizeof(ctrestoren)*wd, &dwAccBytes, NULL);
	if(sizeof(ctrestoren)*wd != dwAccBytes){
		CloseHandle(hFile);
		ctrestnum=0;
		return;
	}
	ctrestnum+=wd;
	CloseHandle(hFile);
	return;
}
#endif


#if 0
void CheatCodeDisp(HWND hWnd, HWND hList){
	int i, dhex;
	char sc, str[50], str2[35];
	HINSTANCE hInstd;

	if((i=ListView_GetNextItem(hList,-1,LVNI_ALL | LVNI_SELECTED)) == -1)
		return;
	g_Cheat_Info.ctrestds[i]
	ListView_GetItemText(hList, i, 1, str, 7);	//Address
	strcat(str, "-");
	ListView_GetItemText(hList, i, 4, str2, 3);	//size
	sc=str2[0];
	strcat(str2, "-");
	strcat(str, str2);
	ListView_GetItemText(hList, i, 3, str2, 3);	//Type
	if(str2[1]=='0'){
		ListView_GetItemText(hList, i, 2, str2, 5);	//Dec
		dhex=atoi(str2);
		switch(sc){
			case '1':
				wsprintf(str2, "%02X", dhex);
				break;
			case '2':
				wsprintf(str2, "%04X", dhex);
				break;
			case '3':
				wsprintf(str2, "%06X", dhex);
				break;
			case '4':
				wsprintf(str2, "%08X", dhex);
				break;
		}
	}
	else{
		ListView_GetItemText(hList, i, 2, str2, 10);	//Hex
	}
	strcat(str, str2);
	str2[0]='\0';
	ListView_GetItemText(hList, i, 0, str2, 32);	//Comment
	if(str2[0]!='\0'){
		strcat(str, "-");
		strcat(str, str2);
	}
	hInstd = g_main_instance;
	SResulttext= (unsigned char *)str;
	DialogBox(hInstd, MAKEINTRESOURCE(IDD_CTDIALOG13), hWnd, (DLGPROC)MyCheatDlg17);
	return;
}
#endif

//Cheat Code Disp  IDD_CTDIALOG13
LRESULT CALLBACK MyCheatDlg17(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					EndDialog(hDlgWnd, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
        case WM_INITDIALOG:
			{
				char str[260];
				str[0]=0;
				CheatDataToCheatCode((struct ctrestoren *)lp, str);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_CT13EDIT1), (const char *)str);
			}
            return TRUE;
    }
    return FALSE;
}


#if 1
void CheatCodeCCopy(HWND hWnd, HWND hList){
	int i, dhex;
	char str[MAX_PATH];
	HGLOBAL hMem;
	LPTSTR lpMem;

	if((i=ListView_GetNextItem(hList,-1,LVNI_ALL | LVNI_SELECTED)) == -1)
		return;
	
	CheatDataToCheatCode(&g_Cheat_Info.ctrestds[i], str);

	hMem=GlobalAlloc(GHND,MAX_PATH);
	lpMem=(char *)GlobalLock(hMem);
	strcpy(lpMem, str);
	GlobalUnlock(hMem);
	if(OpenClipboard(hWnd)){
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}
	GlobalFree(hMem);
	return;
}


void CheatCodeACCopy(HWND hWnd, HWND hList){
	int i=-1;
	char str[MAX_PATH];
	HGLOBAL hMem;
	LPTSTR lpMem;

	hMem=GlobalAlloc(GHND, CTRESTMAX * MAX_PATH+ MAX_PATH);
	if(hMem==NULL)
		return;
	lpMem=(char *)GlobalLock(hMem);
	strcpy(lpMem, emu->getROMname());
	if(lpMem[0]!=0){
		strcat(lpMem, ";\r\n");
	}

	while((i=ListView_GetNextItem(hList, i, LVNI_ALL)) != -1){
		CheatDataToCheatCode(&g_Cheat_Info.ctrestds[i], str);
		strcat(str, "\r\n");
		strcat(lpMem, str);
	}
	if(lpMem[0]==0)
		return;
	GlobalUnlock(hMem);
	if(OpenClipboard(hWnd)){
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}
	GlobalFree(hMem);
	return;
}
#endif


int	MystrJncmp(char *stra,char *strb, char sp){
	int i=0, j=0;
	while(stra[i])++i;
	while(strb[j])++j;

	if(sp == 0){
		if(stra[0]==0 && strb[0]!=0)
			return 1;
		if(strb[0]==0 && stra[0]!=0)
			return -1;
	}
	if(sp == 1){
		if(stra[0]==0 && strb[0]!=0)
			return -1;
		if(strb[0]==0 && stra[0]!=0)
			return 1;
	}
	if(i==j){
		j=0;
		while(j < i){
			if(stra[j] == strb[j])
				++j;
			else if(stra[j] > strb[j])
				return 1;
			else
				return -1;
		}
	}
	else if(i>j)
		return 1;
	else
		return -1;
	return 0;
}



#if 1
int CheatCfSave(HWND hWnd){
	int i;
	HANDLE hFile;
	DWORD dwAccBytes;
    OPENFILENAME ofn;
	const char *fnp;
	char szStr[MAX_PATH]="", str[260], temp[32];
	char FileName[MAX_PATH]="";

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Cheat Code File(*.txt)\0*.txt\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = FileName;
    ofn.lpstrFileTitle = szStr;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "txt";
	LoadString(g_main_instance, IDS_STRING_CDLG_02 , temp, 32);
	ofn.lpstrTitle	=temp;
	ofn.lpstrInitialDir = NULL;
	if(GetSaveFileName(&ofn) == 0)
        return 0;
	hFile = CreateFile(FileName, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
    	hFile = CreateFile(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return 0;
		}
    }

	if(!emu)
		return 0;
	fnp=emu->getROMname();
	szStr[0]=';';
	strcpy(&szStr[1], fnp);
	strcat(szStr, "\r\n");
	if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return 0;
	}

	for(i=0; i<g_Cheat_Info.ctrestnum; i++){
		CheatDataToCheatCode(&g_Cheat_Info.ctrestds[i], str);
		strcat(str, "\r\n");
		if(!WriteFile(hFile, str, strlen(str), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return 0;
		}
	}
	CloseHandle(hFile);
	return 1;
}
#endif




//  IDD_CTDIALOG16
LRESULT CALLBACK MyCheatDlg20(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	char str[10];
	DWORD j, adhex;

    switch(msg) {
		case WM_COMMAND:
            switch (LOWORD(wp)) {
				case IDC_CT16CHECK1:
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT16CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16EDIT1), TRUE);
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16EDIT2), TRUE);
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO1), TRUE);
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO2), TRUE);
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO3), TRUE);
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO4), TRUE);
					}
					else{
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16EDIT1), FALSE);
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16EDIT2), FALSE);
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO1), FALSE);
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO2), FALSE);
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO3), FALSE);
						EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO4), FALSE);
					}
					return TRUE;
					break;
                case IDOK:
					GetDlgItemText(hDlgWnd, IDC_CT16EDIT1, (LPTSTR)str, 10);
					if(!MyStrAtoh(str, &adhex)){
						char str[64];
						LoadString(g_main_instance, IDS_STRING_CHTM_01 , str, 64);
						break;
					}
					GetDlgItemText(hDlgWnd, IDC_CT16EDIT2, (LPTSTR)str, 10);
					if(!MyStrAtoh(str, &j)){
						char str[64];
						LoadString(g_main_instance, IDS_STRING_CHTM_02 , str, 64);
						break;
					}
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT16CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT16RADIO1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
							if(j>=0x100){
								char str[64], str2[64];
								LoadString(g_main_instance, IDS_STRING_CHTM_05 , str, 64);
								strcpy(str2, "1 ");
								strcat(str2, str);
								MessageBox(hDlgWnd,(LPCSTR)str2,(LPCSTR)"Error",MB_OK);
								break;
							}
							cheatkeyf=1;
						}
						else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT16RADIO2), BM_GETCHECK, (WPARAM)TRUE, 0L)){
							if(j>=0x10000){
								char str[64], str2[64];
								LoadString(g_main_instance, IDS_STRING_CHTM_05 , str, 64);
								strcpy(str2, "2 ");
								strcat(str2, str);
								MessageBox(hDlgWnd,(LPCSTR)str2,(LPCSTR)"Error",MB_OK);
								break;
							}
							cheatkeyf=2;
						}
						else if(SendMessage(GetDlgItem(hDlgWnd, IDC_CT16RADIO3), BM_GETCHECK, (WPARAM)TRUE, 0L)){
							if(j>=0x1000000){
								char str[64], str2[64];
								LoadString(g_main_instance, IDS_STRING_CHTM_05 , str, 64);
								strcpy(str2, "3 ");
								strcat(str2, str);
								MessageBox(hDlgWnd,(LPCSTR)str2,(LPCSTR)"Error",MB_OK);
								break;
							}
							cheatkeyf=3;
						}
						else{
							if(j>=0x100000000){
								char str[64], str2[64];
								LoadString(g_main_instance, IDS_STRING_CHTM_05 , str, 64);
								strcpy(str2, "4 ");
								strcat(str2, str);
								MessageBox(hDlgWnd,(LPCSTR)str2,(LPCSTR)"Error",MB_OK);
								break;
							}
							cheatkeyf=4;
						}
					}
					else
						cheatkeyf=0;
					cheatkeyadr = adhex;
					cheatkeynum = j;
  					EndDialog(hDlgWnd, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			return TRUE;
			break;
        case WM_INITDIALOG:
			SendMessage(GetDlgItem(hDlgWnd, IDC_CT16CHECK1), BM_SETCHECK, (WPARAM)cheatkeyf, 0L);
			if(cheatkeyf){
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16EDIT1), TRUE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16EDIT2), TRUE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO1), TRUE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO2), TRUE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO3), TRUE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO4), TRUE);
			}
			else{
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16EDIT1), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16EDIT2), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO1), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO2), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO3), FALSE);
				EnableWindow(GetDlgItem(hDlgWnd, IDC_CT16RADIO4), FALSE);
			}
			wsprintf(str, "%X", cheatkeyadr);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_CT16EDIT1), str);
			wsprintf(str, "%X", cheatkeynum);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_CT16EDIT2), str);
			switch(cheatkeyf){
				case 0:
				case 1:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT16RADIO1), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 2:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT16RADIO2), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 3:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT16RADIO3), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
				case 4:
					SendMessage(GetDlgItem(hDlgWnd, IDC_CT16RADIO4), BM_SETCHECK, (WPARAM)TRUE, 0L);
					break;
			}
			return TRUE;
			break;
	}
	return FALSE;
}


void ckreadmm(char *pstr){
	DWORD n=0, size;
	unsigned char c;
	char str[30];

	size=cheatkeyf;
	if(cheatkeyadr >= 0x10000)
		emu->ReadSroneb(cheatkeyadr, &c);
	else
		emu->ReadBoneb(cheatkeyadr, &c);
	if(!(--size)){
		cheatkeynum&=0xff;
		wsprintf(str, " %05x : %02x, [%02x]", cheatkeyadr, cheatkeynum, c);
		strcat(pstr, str);
		return;
	}
	n|=(DWORD)c;
	if(cheatkeyadr >= 0x10000)
		emu->ReadSroneb(cheatkeyadr+1, &c);
	else
		emu->ReadBoneb(cheatkeyadr+1, &c);
	n|=(DWORD)c<<8;
	if(!(--size)){
		cheatkeynum&=0xffff;
		wsprintf(str, "%05x : %04x, [%04x]", cheatkeyadr, cheatkeynum, n);
		strcat(pstr, str);
		return;
	}
	if(cheatkeyadr >= 0x10000)
		emu->ReadSroneb(cheatkeyadr+2, &c);
	else
		emu->ReadBoneb(cheatkeyadr+2, &c);
	n|=(DWORD)c<<16;
	if(!(--size)){
		cheatkeynum&=0xffffff;
		wsprintf(str, "%05x : %06x, [%06x]", cheatkeyadr, cheatkeynum, n);
		strcat(pstr, str);
		return;
	}
	if(cheatkeyadr >= 0x10000)
		emu->ReadSroneb(cheatkeyadr+3, &c);
	else
		emu->ReadBoneb(cheatkeyadr+3, &c);
	n|=(DWORD)c<<24;
	wsprintf(str, "%04x : %08x, [%08x]", cheatkeyadr, cheatkeynum, n);
	strcat(pstr, str);
	return;
}



//Add Cheat Code  IDD_CTDIALOG17
LRESULT CALLBACK MyCheatDlg21(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

    switch(msg) {
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDC_CT17BUTTON1:
					{
						char *estr;
						int size, i=0, add=0;
						size=GetWindowTextLength(GetDlgItem(hDlgWnd, IDC_CT17EDIT1));
						estr = (char *)malloc(size+1);
						if(estr==NULL)
							EndDialog(hDlgWnd, IDOK);
						GetDlgItemText(hDlgWnd, IDC_CT17EDIT1, (LPTSTR)estr, size+1);
						while(1){
							char str[260];
							int j;

							if(g_Cheat_Info.ctrestnum >= CTRESTMAX)
								break;
							for(j=0; estr[i] && estr[i]!=0x0d; j++, i++){
								str[j] = estr[i];
							}
							str[j] = 0;
							if(CheatCodeToCheatData(&g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum], str)){
								if(g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum].opcode == 1){
									OperateCheatCode(&g_Cheat_Info.ctrestds[g_Cheat_Info.ctrestnum]);
								}
								else{
									g_Cheat_Info.ctrestnum++;
									add++;
								}
							}
							while(estr[i] == 0x0d || estr[i] == 0x0a)i++;
							if(estr[i]==0)
								break;
						}
						free(estr);

//						g_Cheat_Info.ctrestnum+=add;
						char kstr[64];
						char str[64];
						LoadString(g_main_instance, IDS_STRING_CHTM_06 , str, 64);
						wsprintf(kstr, "%u", add);
						strcat(kstr, str);
						if(g_extra_window.CheatListEdit)
							SendMessage(g_extra_window.CheatListEdit, WM_EXTRAWIN_01, 0, 0);
						MessageBox(hDlgWnd,(LPCSTR)kstr,(LPCSTR)"Result",MB_OK);
					}
					return TRUE;
                case IDC_CT17BUTTON2:
					{
					char str[64];
					LoadString(g_main_instance, IDS_STRING_CHTM_07 , str, 64);
					if(IDCANCEL== MessageBox(hDlgWnd,(LPCSTR)str,(LPCSTR)"Clear",MB_OKCANCEL))
						break;
					SetWindowText(GetDlgItem(hDlgWnd, IDC_CT17EDIT1), "");
					}
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			return TRUE;
			break;
    }
    return FALSE;
}


#if 0
//Add Cheat Code  IDD_CTDIALOG17
LRESULT CALLBACK MyCheatDlg22(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	char *str , *bstr, flag;
	DWORD i=0, size, adhex, adr, data, add=0;

    switch(msg) {
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDC_CT17BUTTON1:
					size=GetWindowTextLength(GetDlgItem(hDlgWnd, IDC_CT17EDIT1));
					str = (char *)malloc(size+1);
					if(str==NULL)
	  					EndDialog(hDlgWnd, IDOK);
					GetDlgItemText(hDlgWnd, IDC_CT17EDIT1, (LPTSTR)str, size+1);
					while(1){
						while(str[i]==' ')++i;
						if(str[i] == 0x0d ){
							i+=2;
							continue;
						}
						bstr= &str[i];
						while(str[i] != 0x0d && i<size){
							if(str[i]=='-'||str[i]==' ')
								break;
							++i;
						}
						if(str[i] == 0x0d ){
							i+=2;
							continue;
						}
						if(i>=size)
							break;
						str[i]= '\0';
						if(!MyStrAtoh(bstr, &adr)){
							while(str[i]!=0x0d && i<size)++i;
							if(i>=size)
								break;
							i+=2;
							continue;
						}
						bstr= &str[i+1];
						i+=2;
						if(str[i] != '-'){
							if(str[i+1] == ' '){
								--i;
								adhex=1;
							}
							else{
								while(str[i]!=0x0d && i<size)++i;
								if(i>=size)
									break;
								i+=2;
								continue;
							}
						}
						else{
							str[i]= '\0';
							++i;
							if(!MyStrAtoh(bstr, &adhex)){
								while(str[i]!=0x0d && i<size)++i;
								if(i>=size)
									break;
								i+=2;
								continue;
							}
						}
						bstr= &str[i];
						for(flag=0; str[i]!=0x0d && i<size; ++i){
							if(str[i]=='-' || str[i]==' '){
								flag=1;
								break;
							}
						}
						str[i]='\0';
						if(flag)
							i+=2;
						if(!MyStrAtoh(bstr, &data)){
							while(str[i]!=0x0d && i<size)++i;
							if(i>=size)
								break;
							i+=2;
							continue;
						}
						while(str[i]!=0x0d && i<size)++i;
						i+=2;
						if(adr >= 0x10000){
							emu->WriteSroneb(adr, (unsigned char)data);
							if(adhex>1){
								emu->WriteSroneb(adr+1, (unsigned char)(data>>8));
								if(adhex>2)
									emu->WriteSroneb(adr+2, (unsigned char)(data>>16));
								if(adhex==4)
									emu->WriteSroneb(adr+3, (unsigned char)(data>>24));
							}
						}
						else{
							emu->WriteBoneb(adr, (unsigned char)data);
							if(adhex>1){
								emu->WriteBoneb(adr+1, (unsigned char)(data>>8));
								if(adhex>2)
									emu->WriteBoneb(adr+2, (unsigned char)(data>>16));
								if(adhex==4)
									emu->WriteBoneb(adr+3, (unsigned char)(data>>24));
							}
						}
						++add;
						if(i>=size)
							break;
					}
					free(str);
					{
					char kstr[64];
					char str[64];
					LoadString(g_main_instance, IDS_STRING_CHTM_06 , str, 64);
					wsprintf(kstr, "%u", add);
					strcat(kstr, str);
					MessageBox(hDlgWnd,(LPCSTR)kstr,(LPCSTR)"Result",MB_OK);
					}
					return TRUE;
                case IDC_CT17BUTTON2:
					{
					char str[64];
					LoadString(g_main_instance, IDS_STRING_CHTM_07 , str, 64);
					if(IDCANCEL== MessageBox(hDlgWnd,(LPCSTR)str,(LPCSTR)"Clear",MB_OKCANCEL))
						break;
					SetWindowText(GetDlgItem(hDlgWnd, IDC_CT17EDIT1), "");
					}
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			return TRUE;
			break;
    }
    return FALSE;
}
#endif



int MyCheatSeachStr(unsigned char *ram, char *str, DWORD *adr){
	DWORD i,sn;
	unsigned char rstr[17], uc;
	char gstr[17];

	if(str[0]=='\0' || str[1]=='\0')
		return 0;
	if((unsigned char)str[0] >= '0' && (unsigned char)str[0] <= '9'){
		uc='0';
	}
	else if((unsigned char)str[0] >= 'A' && (unsigned char)str[0] <= 'Z'){
		uc='A';
	}
	else if((unsigned char)str[0] >= 'a' && (unsigned char)str[0] <= 'z'){
		uc='a';
	}
	else if((unsigned char)str[0] >= (unsigned char)'ｱ' && (unsigned char)str[0] <= (unsigned char)'ﾝ'){
		uc=(unsigned char)'ｱ';
	}
	else 
		return 0;
	for(i=0;str[i];++i){
		rstr[i] = (unsigned char)str[i]-uc;
	}
	sn=(i-1);
	for(i=0;i<sn;++i){
		gstr[i] = str[i] - str[i+1];
	}
	for(i= *adr; i <0x2800-sn;++i){
//		if(i==0x9c6){
//			int a=0;
//		}
		if((char)(ram[i]-ram[i+1])==gstr[0]){
			if(sn==1){
				*adr=i;
				return 1;
			}
			for(unsigned int k=1;k<sn;++k){
				if((char)(ram[k+i]-ram[k+i+1])==gstr[k]){
					if(k==(sn-1)){
						*adr=i;
						return 1;
					}
				}
				else
					break;
			}
		}
	}
	return 0;
}


//IDD_CTDIALOG20
LRESULT CALLBACK MyDlgProc20(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static DWORD address;
	static char instr[17], rstr[20];

    switch(msg) {
		case WM_COMMAND:
            switch(LOWORD(wp)){
                case IDOK:
					GetDlgItemText(hDlgWnd, IDC_EDIT1, (LPTSTR)instr, 17);
					address=0;
					if(MyCheatSeachStr(SResulttext , instr, &address)){
						if(address < 0x800){
							wsprintf(rstr, "Address  %x", address);
						}
						else{
							wsprintf(rstr, "Address SRAM %x", (address-0x800));
						}
						MessageBox(hDlgWnd,(LPCSTR)rstr,(LPCSTR)"Result",MB_OK);
					}
					return TRUE;
					break;
                case IDC_BUTTON1:
//					GetDlgItemText(hDlgWnd, IDC_EDIT1, (LPTSTR)SResulttext, 17);
					++address;
					if(MyCheatSeachStr(SResulttext , instr, &address)){
						if(address < 0x800){
							wsprintf(rstr, "Address  %x", address);
						}
						else{
							wsprintf(rstr, "Address SRAM %x", (address-0x800));
						}
						MessageBox(hDlgWnd,(LPCSTR)rstr,(LPCSTR)"Result",MB_OK);
					}
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
        case WM_INITDIALOG:
            return TRUE;
    }
    return FALSE;
}


