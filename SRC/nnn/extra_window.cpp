
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <wingdi.h>

#include "settings.h"
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
#include "savecfg.h"
#include "extra_window.h"

#include "types.h"

#include "nes6502.h"


extern struct extra_window_struct g_extra_window;
extern win32_screen_mgr* gscr_mgr;
extern emulator* emu;
extern int PrevFastSw;
extern HINSTANCE g_main_instance;
extern LOGFONT Default_Font;
extern HWND main_window_handle;

void CheatWriteStringsMem(int size, DWORD adr, unsigned char *pdata);
void CheatWriteMem(int size, DWORD adr, DWORD data);
int MyStrAtoh(char *str, DWORD *hex);


static struct MyCheatStructWriteInfo CheatStWtWindParam;



int CheckModelessDialogMessage(MSG *lpmsg){
	if(g_extra_window.CheatListEdit){
		if(IsDialogMessage(g_extra_window.CheatListEdit, lpmsg))
			return 1;
	}
	if(g_extra_window.CheatCompare){
		if(IsDialogMessage(g_extra_window.CheatCompare, lpmsg))
			return 1;
	}
	if(g_extra_window.CheatSearch){
		if(IsDialogMessage(g_extra_window.CheatSearch, lpmsg))
			return 1;
	}
	return 0;
}



void Extrawin_ExitApp(){
	int i;
	for(i=0; i<4; i++){
		if(g_extra_window.CheatView[i])
			DestroyWindow(g_extra_window.CheatView[i]);
	}
	if(g_extra_window.CheatListEdit){
		DestroyWindow(g_extra_window.CheatListEdit);
	}
	if(g_extra_window.CheatSearch){
		DestroyWindow(g_extra_window.CheatSearch);
	}
	if(g_extra_window.CheatCompare){
		DestroyWindow(g_extra_window.CheatCompare);
	}
	if(g_extra_window.PatternWindow){
		DestroyWindow(g_extra_window.PatternWindow);
	}
	if(g_extra_window.CheatStWriteWindow){
		DestroyWindow(g_extra_window.CheatStWriteWindow);
	}
	if(g_extra_window.CheatStWriteEditWindow){
		DestroyWindow(g_extra_window.CheatStWriteEditWindow);
	}
//	HWND DebugWindow;
}



void Extrawin_HideWin(){
	int i;
	for(i=0; i<4; i++){
		if(g_extra_window.CheatView[i])
			ShowWindow(g_extra_window.CheatView[i], SW_HIDE);
	}
	if(g_extra_window.CheatListEdit){
		ShowWindow(g_extra_window.CheatListEdit, SW_HIDE);
	}
	if(g_extra_window.CheatSearch){
		ShowWindow(g_extra_window.CheatSearch, SW_HIDE);
	}
	if(g_extra_window.CheatCompare){
		ShowWindow(g_extra_window.CheatCompare, SW_HIDE);
	}
	if(g_extra_window.PatternWindow){
		ShowWindow(g_extra_window.PatternWindow, SW_HIDE);
	}
	if(g_extra_window.CheatStWriteWindow){
		ShowWindow(g_extra_window.CheatStWriteWindow, SW_HIDE);
	}
	if(g_extra_window.CheatStWriteEditWindow){
		ShowWindow(g_extra_window.CheatStWriteEditWindow, SW_HIDE);
	}
//	HWND DebugWindow;
}



void Extrawin_ShowWin(){
	int i;
	for(i=0; i<4; i++){
		if(g_extra_window.CheatView[i])
			ShowWindow(g_extra_window.CheatView[i], SW_SHOW);
	}

	if(g_extra_window.CheatListEdit){
		ShowWindow(g_extra_window.CheatListEdit, SW_SHOW);
	}
	if(g_extra_window.CheatSearch){
		ShowWindow(g_extra_window.CheatSearch, SW_SHOW);
	}
	if(g_extra_window.CheatCompare){
		ShowWindow(g_extra_window.CheatCompare, SW_SHOW);
	}
	if(g_extra_window.PatternWindow){
		ShowWindow(g_extra_window.PatternWindow, SW_SHOW);
	}
	if(g_extra_window.CheatStWriteWindow){
		ShowWindow(g_extra_window.CheatStWriteWindow, SW_SHOW);
	}
	if(g_extra_window.CheatStWriteEditWindow){
		ShowWindow(g_extra_window.CheatStWriteEditWindow, SW_SHOW);
	}
//	HWND DebugWindow;
}



void Extrawin_FullscreentoWindow(){
/*
	int i;
	for(i=0; i<4; i++){
		if(g_extra_window.CheatView[i])
			SendMessage(g_extra_window.CheatView[i], WM_EXTRAWIN_03, 0, 0);
	}
*/
	if(g_extra_window.CheatListEdit){
		SetWindowPos(g_extra_window.CheatListEdit, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	}
	if(g_extra_window.CheatSearch){
		SetWindowPos(g_extra_window.CheatSearch, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	}
	if(g_extra_window.CheatCompare){
//		ShowWindow(g_hWndDlg, SW_SHOW);
		SetWindowPos(g_extra_window.CheatCompare, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	}
}




void Extrawin_WindowtoFullscreen(){
/*
	int i;
	for(i=0; i<4; i++){
		if(g_extra_window.CheatView[i])
			SendMessage(g_extra_window.CheatView[i], WM_EXTRAWIN_03, 0, 0);
	}
*/
	if(g_extra_window.CheatListEdit){
		DestroyWindow(g_extra_window.CheatListEdit);
	}
	if(g_extra_window.CheatSearch){
		DestroyWindow(g_extra_window.CheatSearch);
	}
	if(g_extra_window.CheatCompare){
		DestroyWindow(g_extra_window.CheatCompare);
	}
}




void RunFrame_ExtraWindow(){
	int i;
	for(i=0; i<4; i++){
		if(g_extra_window.CheatView[i])
			SendMessage(g_extra_window.CheatView[i], WM_EXTRAWIN_03, 0, 0);
	}
	if(g_extra_window.PatternWindow)
		SendMessage(g_extra_window.PatternWindow, WM_EXTRAWIN_03, 0, 0);
}


void LoadROM_ExtraWindow(){
	int i;
	for(i=0; i<4; i++){
		if(g_extra_window.CheatView[i])
			SendMessage(g_extra_window.CheatView[i], WM_EXTRAWIN_01, 0, 0);
	}
	if(g_extra_window.CheatListEdit)
		SendMessage(g_extra_window.CheatListEdit, WM_EXTRAWIN_01, 0, 0);
	if(g_extra_window.CheatCompare)
		SendMessage(g_extra_window.CheatCompare, WM_EXTRAWIN_01, 0, 0);
	if(g_extra_window.CheatSearch)
		SendMessage(g_extra_window.CheatSearch, WM_EXTRAWIN_01, 0, 0);
	if(g_extra_window.PatternWindow)
		SendMessage(g_extra_window.PatternWindow, WM_EXTRAWIN_01, 0, 0);
}


void FreeROM_ExtraWindow(){
	int i;
	for(i=0; i<4; i++){
		if(g_extra_window.CheatView[i])
			SendMessage(g_extra_window.CheatView[i], WM_EXTRAWIN_02, 0, 0);
	}
	if(g_extra_window.CheatListEdit)
		SendMessage(g_extra_window.CheatListEdit, WM_EXTRAWIN_02, 0, 0);
	if(g_extra_window.CheatSearch)
		SendMessage(g_extra_window.CheatSearch, WM_EXTRAWIN_02, 0, 0);
	if(g_extra_window.CheatCompare)
		SendMessage(g_extra_window.CheatCompare, WM_EXTRAWIN_02, 0, 0);
	if(g_extra_window.PatternWindow)
		SendMessage(g_extra_window.PatternWindow, WM_EXTRAWIN_02, 0, 0);
}



LRESULT CALLBACK MyPatternWindProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp);

HWND CreateMyPaternWindow(HWND hWnd){
    WNDCLASSEX wc;
	static int rflag=0;
	RECT rc, rct;
	HINSTANCE hInst;

	if(g_extra_window.PatternWindow){
		SetForegroundWindow(g_extra_window.PatternWindow);
		return NULL;
	}
	hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	GetWindowRect(hWnd, &rc);

	if(rflag==0)
	{
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_VREDRAW | CS_HREDRAW;
		wc.lpfnWndProc = MyPatternWindProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.lpszMenuName = NULL; //MAKEINTRESOURCE(IDR_MENUCT);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszClassName = (LPCSTR)"Pattern";
		wc.hIconSm = (HICON)LoadImage(g_main_instance,MAKEINTRESOURCE(IDI_ICON29),IMAGE_ICON  , 0, 0,LR_DEFAULTCOLOR);
		if(RegisterClassEx(&wc) == 0) {
			return NULL;
		}
		rflag=1;
	}
	g_extra_window.PatternWindow = CreateWindowEx(WS_EX_TOOLWINDOW/* NULL*/, "Pattern", "Pattern", WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
		rc.left+100, rc.top+100, 256/*256*/, 256, NULL/*hWnd*/, /*LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENUCT))*/NULL, hInst, NULL);
	ShowWindow(g_extra_window.PatternWindow, SW_SHOW);
	UpdateWindow(g_extra_window.PatternWindow);

	SetRect(&rct, 0, 0, 256, 128);
	AdjustWindowRectEx(&rct, GetWindowStyle(g_extra_window.PatternWindow),
					   GetMenu(g_extra_window.PatternWindow) != NULL,
					   GetWindowExStyle(g_extra_window.PatternWindow));
	SetWindowPos(g_extra_window.PatternWindow, HWND_TOP, 0, 0,
				 rct.right-rct.left, rct.bottom-rct.top, SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
	return g_extra_window.PatternWindow;
}


extern void nnn8x8charadraw(unsigned char *outd, unsigned char *temp, int x);


void MyDrawNESPattern(HWND hWnd, HDC hdc, unsigned char *pPPU[]){
	int tn, x, y, offset=0, Vbank=0;
	LPBITMAPINFO lpbi = NULL;
	unsigned char data[8*8], *ddata;
	RECT rect;

	ddata = (unsigned char *)malloc(128*128*2);
	if(ddata==NULL)
		return;
	lpbi = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
	lpbi->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	lpbi->bmiHeader.biPlanes=1;
	lpbi->bmiHeader.biBitCount=8;
	lpbi->bmiHeader.biCompression=BI_RGB;
	lpbi->bmiHeader.biSizeImage=0;
	lpbi->bmiHeader.biXPelsPerMeter=0;
	lpbi->bmiHeader.biYPelsPerMeter=0;
	lpbi->bmiHeader.biClrUsed=0;
	lpbi->bmiHeader.biClrImportant=0;

	lpbi->bmiColors[0].rgbBlue=0;
	lpbi->bmiColors[0].rgbGreen=0;
	lpbi->bmiColors[0].rgbRed=0;
	lpbi->bmiColors[0].rgbReserved=0;
	lpbi->bmiColors[1].rgbBlue=00;
	lpbi->bmiColors[1].rgbGreen=0x80;
	lpbi->bmiColors[1].rgbRed=00;
	lpbi->bmiColors[1].rgbReserved=0;

	lpbi->bmiColors[2].rgbBlue=0xE0;
	lpbi->bmiColors[2].rgbGreen=0xE0;
	lpbi->bmiColors[2].rgbRed=0;
	lpbi->bmiColors[2].rgbReserved=0;
	lpbi->bmiColors[3].rgbBlue=0xff;
	lpbi->bmiColors[3].rgbGreen=0xff;
	lpbi->bmiColors[3].rgbRed=0;
	lpbi->bmiColors[3].rgbReserved=0;

//	memset(data, 0, sizeof(data));
#if 0
	lpbi->bmiHeader.biWidth = 8;
	lpbi->bmiHeader.biHeight = 8;
	for(tn=0; tn<2; tn++){
		for(y=0; y<(128/8); y++){
			for(x=0; x<(128/8); x++, offset+=0x10){
				if(offset==0x400){
					offset = 0;
					Vbank++;
				}
				nnn8x8charadraw(data, &pPPU[Vbank][offset], 8);
				SetDIBitsToDevice(hdc, x*8+tn*128, y*8, 8, 8, 0, 0, 0, 8, data, lpbi, DIB_RGB_COLORS);
			}
		}
	}
#else
	// 一度に描画
	lpbi->bmiHeader.biWidth = 256;
	lpbi->bmiHeader.biHeight = 128;
	for(tn=0; tn<2; tn++){
		for(y=0; y<(128/8); y++){
			for(x=0; x<(128/8); x++, offset+=0x10){
				if(offset==0x400){
					offset = 0;
					Vbank++;
				}
				nnn8x8charadraw(data, &pPPU[Vbank][offset], 8);

				unsigned char *p = &ddata[tn*128 + y*8*256+ x*8];
				for(int j=0; j<8; j++){
					for(int k=0; k<8; k++){
						p[j*256+k] = data[j*8+k];
					}
				}
			}
		}
	}
	GetClientRect(hWnd, &rect);
	if((rect.right - rect.left)==256 && (rect.bottom-rect.top)==128){
		SetStretchBltMode(hdc, COLORONCOLOR);	// 
		SetDIBitsToDevice(hdc, 0, 0, 128*2, 128, 0, 0, 0, 128, ddata, lpbi, DIB_RGB_COLORS);
	}
	else{
		StretchDIBits(hdc, 0, 0, rect.right - rect.left, rect.bottom-rect.top, 0, 0, 128*2, 128, ddata, lpbi, DIB_RGB_COLORS, SRCCOPY);
	}

#endif
	if(ddata)
		free(ddata);
	if(lpbi)
		free(lpbi);
}


void SetPatternWinSize(HWND hWnd, RECT *rect){
	AdjustWindowRectEx(rect, GetWindowStyle(hWnd),
		GetMenu(hWnd) != NULL, GetWindowExStyle(hWnd));
	SetWindowPos(hWnd, HWND_TOP, 0, 0, rect->right - rect->left, rect->bottom - rect->top, SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
}


LRESULT CALLBACK MyPatternWindProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp){

	static HINSTANCE hInst;
	static unsigned char *pPPU[15];
	static int Frame_Count=0;

	switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT paint;
				HDC hdc = BeginPaint(hChild, &paint);
				if(emu && pPPU[0]){
					MyDrawNESPattern(hChild, hdc, pPPU);
				}
				EndPaint(hChild, &paint);
			}
			break;
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
			if(!Frame_Count)
			{
				if(emu && pPPU[0])
				{
					emu->Get_PPUMemp(pPPU);
					HDC hdc = GetDC(hChild);
					MyDrawNESPattern(hChild, hdc, pPPU);
					ReleaseDC(hChild, hdc);	
//					Frame_IntervalCounter=0;
				}
				Frame_Count = NESTER_settings.nes.preferences.nPtnViewerReInterval;
			}
			else{
				Frame_Count--;
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
					AppendMenu(hPMenu, MF_STRING, ID_OPTIONS_1xSIZE, "1x");
					AppendMenu(hPMenu, MF_STRING, ID_OPTIONS_DOUBLESIZE, "2x");
					TrackPopupMenu(hPMenu, TPM_LEFTALIGN /*| TPM_TOPALIGN*/, pt.x, pt.y, 0, hChild, NULL);
					DestroyMenu(hPMenu);
				}
			}
			break;
        case WM_COMMAND:
			switch (LOWORD(wp)){
				case ID_OPTIONS_1xSIZE:
					{
						RECT rct;
						SetRect(&rct, 0, 0, 256, 128);
						SetPatternWinSize(hChild, &rct);
					}
					break;
				case ID_OPTIONS_DOUBLESIZE:
					{
						RECT rct;
						SetRect(&rct, 0, 0, 256*2, 128*2);
						SetPatternWinSize(hChild, &rct);
					}
					break;
			}
			break;
		case WM_CREATE:
			memset(&pPPU, 0, sizeof(pPPU));
			hInst = g_main_instance; //(HINSTANCE)GetWindowLong(GetParent(hChild), GWL_HINSTANCE);
			Frame_Count=0;
			if(emu){
				emu->Get_PPUMemp(pPPU);
			}
			return 0;
			break;
		case WM_DESTROY:
			g_extra_window.PatternWindow= NULL;
			memset(&pPPU, 0, sizeof(pPPU));
			return 0;
			break;
		default:
			return (DefWindowProc(hChild, msg, wp, lp));
	}
	return 0L;
}




LRESULT CALLBACK MyNESDebugDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){

	switch(msg){
	case WM_INITDIALOG:
//		SetWindowText(hWnd, "Test");
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wp)){
			case IDOK:
				{
//					GetDlgItemText(hWnd, IDC_EDIT1, str, sizeof(str));
//					wsprintf(str, "%u", cd);
//					MessageBox(hWnd, str, "Result", MB_OK);
				}
				break;
			case IDCANCEL:
				EndDialog(hWnd, TRUE);
				break;
		}
		return TRUE;
	}
    return FALSE;
}




int MyCheatStWriteFileOpenDlg(HWND hWnd, char *fn){
	OPENFILENAME ofn;
	char szFile[MAX_PATH]="";
	char FileName[MAX_PATH]="";
//	char FoldName[MAX_PATH];
	char str[64];

//	GetModuleFileName(NULL, FoldName, MAX_PATH);
//	PathRemoveFileSpec(FoldName);

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "text files\0*.cdf;*.txt\0All files(*.*)\0*.*\0\0";
	ofn.lpstrFile = FileName;
	ofn.lpstrFileTitle = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = NULL;
 	LoadString(g_main_instance, IDS_STRING_CDLG_15 , str, 64);
	ofn.lpstrTitle = str;
	ofn.lpstrInitialDir = NULL; //FoldName; //OpenFolder;
	if(GetOpenFileName(&ofn) == 0)
		return 0;
	strcpy(fn, FileName);
	return 1;
}



int MyCheatStWriteFileSaveDlg(HWND hWnd, char *fn){
    OPENFILENAME ofn;
	char szStr[MAX_PATH]="", str2[64];
	char FileName[MAX_PATH]="";
//	char initdir[MAX_PATH];

	fn[0] = 0;
//	GetModuleFileName(NULL, initdir, MAX_PATH);
//	PathRemoveFileSpec(initdir);
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = fn;
    ofn.lpstrFileTitle = szStr;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "txt";
	LoadString(g_main_instance, IDS_STRING_CDLG_11 , str2, 64);
	ofn.lpstrTitle	= str2;
	ofn.lpstrInitialDir = NULL; //initdir;
	if(GetSaveFileName(&ofn) == 0)
        return 0;
	return 1;
}




LRESULT CALLBACK MyCheatDtWriteWindProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp);

HWND CreateMyCheatStWriteWindow(HWND hWnd){
    WNDCLASSEX wc;
	static int rflag=0;
	RECT rc;
	HINSTANCE hInst;

	if(g_extra_window.CheatStWriteWindow){
		SetForegroundWindow(g_extra_window.CheatStWriteWindow);
		return NULL;
	}
	hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	GetWindowRect(hWnd, &rc);

	if(rflag==0)
	{
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_VREDRAW | CS_HREDRAW;
		wc.lpfnWndProc = MyCheatDtWriteWindProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.lpszMenuName = NULL; //MAKEINTRESOURCE(IDR_MENUCT);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
		wc.lpszClassName = (LPCSTR)"CheatStWrite";
		wc.hIconSm = (HICON)LoadImage(g_main_instance,MAKEINTRESOURCE(IDI_ICON43),IMAGE_ICON  , 0, 0,LR_DEFAULTCOLOR);
		if(RegisterClassEx(&wc) == 0) {
			return NULL;
		}
		rflag=1;
	}
	g_extra_window.CheatStWriteWindow = CreateWindowEx(/*WS_EX_TOOLWINDOW*/ NULL, "CheatStWrite", "---", /*WS_OVERLAPPED | */WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
		rc.left+100, rc.top+100, 256/*256*/, 256, NULL/*hWnd*/, /*LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENUCT))*/NULL, hInst, NULL);
	ShowWindow(g_extra_window.CheatStWriteWindow, SW_SHOW);
	UpdateWindow(g_extra_window.CheatStWriteWindow);
	return g_extra_window.CheatStWriteWindow;
}

void CreateCheatDtWriteWind(struct MyCheatStructWriteInfo *Param, HWND hWnd);
void WriteCheatDtWriteWind(struct MyCheatStructWriteInfo *Param);

//#define PROP_EDITITEM "Edit_Param"

LRESULT CALLBACK New_EditWind_Proc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	int nPage, CurPage, m, n;

//	struct MyCheatStructWriteItemInfo *pItem = (struct MyCheatStructWriteItemInfo *)GetProp(hWnd, PROP_EDITITEM);
	CurPage = CheatStWtWindParam.nCurPage;
/*
	for(i=0; i<CheatStWtWindParam.Page[CurPage].nItem; i++){
		if(CheatStWtWindParam.Page[CurPage].Item[i].hEdit == hWnd)
			break;
	}
	if(i==CheatStWtWindParam.Page[CurPage].nItem)
		i = 0;
*/
	nPage = CheatStWtWindParam.nPage;
	if(nPage == 0)
		nPage =1;
	for(m=0; m<nPage; m++){
		for(n=0; n<CheatStWtWindParam.Page[m].nItem; n++){
			if(CheatStWtWindParam.Page[m].Item[n].hEdit == hWnd){
				goto loopend;
			}
		}
	}
loopend:

	switch(msg){
		case WM_KEYDOWN:
			{
				switch(wp){
				case VK_TAB:		// TAB
#if 0
					{
						HWND fWind = GetFocus();
						int i;
					
						for(i=0; i<CheatStWtWindParam.nItem; i++){
							if(fWind ==  CheatStWtWindParam.Item[i].hEdit){
								SetFocus(CheatStWtWindParam.Item[(i+1)%CheatStWtWindParam.nItem].hEdit);
								break;
							}
						}
					}
#else
					{
						int j;

						if(m!=CurPage){
							SetFocus(CheatStWtWindParam.Page[CurPage].Item[0].hEdit);
							return TRUE;
						}

						for(j=(n+1)%CheatStWtWindParam.Page[m].nItem; j!=n; j=(j+1)%CheatStWtWindParam.Page[m].nItem){
							if(!CheatStWtWindParam.Page[m].Item[j].DisableInput){
								SetFocus(CheatStWtWindParam.Page[m].Item[j].hEdit);
								break;
							}
						}
					}
#endif
					return TRUE;
					break;
/*
				case VK_RETURN:
					if(emu){
						WriteCheatDtWriteWind(&CheatStWtWindParam);
					}
					return TRUE;
					break;
*/
				}
			}
			break;
//			return FALSE;
		default:
			break;
	}
	return (CallWindowProc((long (__stdcall *)(HWND ,unsigned int,unsigned int,long))CheatStWtWindParam.Page[m].Item[n].OldWind_Proc, hWnd, msg, wp, lp));
//	return (CallWindowProc((long (__stdcall *)(HWND ,unsigned int,unsigned int,long))pItem->OldWind_Proc, hWnd, msg, wp, lp));
}


void CheatStWt_ChangeTabWindowState(struct MyCheatStructWriteInfo *Param);


// read
inline DWORD myreadnumtomem(int size, unsigned char *pmem){
	unsigned char *p;
	DWORD dw = 0;
	int i;

	p = (unsigned char *)&dw;
	for(i=0; i<size; i++){
		p[i] = pmem[i];
	}
	return dw;
}


// mask
inline void mynummask(int size, DWORD *dw){
	unsigned char *p;
	int i;

	p = (unsigned char *)dw;
	for(i=0; i<4; i++){
		if(size <= i)
			p[i] = 0;
	}
}


// Create Window
void CreateCheatDtWriteWind(struct MyCheatStructWriteInfo *Param, HWND hWnd){
	int i, j, nItem=0, Estyle,nPage;
	HINSTANCE hInst;

	hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	nPage = Param->nPage;
	if(nPage == 0)
		nPage = 1;
	// Create Tab window
	if(Param->nPage){
		TCITEM TCItem;
		Param->hTabWin = CreateWindowEx(0 , WC_TABCONTROL , NULL , WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE /*| TCS_OWNERDRAWFIXED */,
				0 , 0 , 1 , 1 , hWnd , (HMENU)213 , hInst, NULL );
/*		TCS_BUTTONS TCS_FLATBUTTONS  TCS_FLATBUTTONS */
/*		TabCtrl_SetExtendedStyle(Settings.hTabCtrl, TCS_EX_FLATSEPARATORS);*/
//		Settings.hImageTab = ImageList_Create(16, 16, ILC_COLOR | ILC_MASK, 2, 0);
//		ImageList_AddIcon(Settings.hImageTab, LoadIcon(hInst, MAKEINTRESOURCE(IDI_TABICON1)));
//		ImageList_AddIcon(Settings.hImageTab, LoadIcon(hInst, MAKEINTRESOURCE(IDI_TABICON2)));
//		TabCtrl_SetImageList(Settings.hTabCtrl, Settings.hImageTab);
		for(i = 0; i<Param->nPage; i++){
			TCItem.mask = TCIF_TEXT/* | TCIF_IMAGE*/;
//			TCItem.iImage = 0;
			TCItem.pszText = Param->Page[i].Title;
			TabCtrl_InsertItem(Param->hTabWin , i , &TCItem);
		}
	}
	for(i=0; i<nPage; i++){
		for(j=0; j<Param->Page[i].nItem; j++, nItem++){
			if(Param->Page[i].Item[j].EditType == 0)
				Estyle = ES_LEFT;
			else
				Estyle = ES_RIGHT;
			if(Param->Page[i].Item[j].DisableInput)
				Estyle |= ES_READONLY;
			
			Estyle |= ES_AUTOHSCROLL;
			// Create Edit window
			Param->Page[i].Item[j].hEdit = CreateWindowEx(WS_EX_STATICEDGE, "EDIT", "", Estyle | WS_CHILD | WS_VISIBLE| WS_BORDER | ES_AUTOHSCROLL, 0, 0,
				0, 0, hWnd, (HMENU)IDC_EDIT1 + nItem, hInst, NULL);
//			SetProp(Param->Page[i].Item[j].hEdit, PROP_EDITITEM, &Param->Page[i].Item[j]);
			
			Param->Page[i].Item[j].OldWind_Proc = (FARPROC)GetWindowLong(Param->Page[i].Item[j].hEdit, GWL_WNDPROC);
			SetWindowLong(Param->Page[i].Item[j].hEdit, GWL_WNDPROC, (LONG)New_EditWind_Proc);
			
			if(Param->Page[i].Item[j].LoadValue && emu){	// 読み込む
				DWORD dw;
				char str[260];
				if(Param->Page[i].Item[j].EditType == 1){		// Hex
					dw = CheatReadMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address);
					wsprintf(str, "%X", dw);
					SetWindowText(Param->Page[i].Item[j].hEdit, str);
				}
				else if(Param->Page[i].Item[j].EditType == 2){	// Dec
					dw = CheatReadMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address);
					wsprintf(str, "%u", dw);
					SetWindowText(Param->Page[i].Item[j].hEdit, str);
				}
/*
				else if(buf3[j] == 'V') Param->Page[nCurPage].Item[nCurPageItem].EditType = 3;
				else if(buf3[j] == 'N') Param->Page[nCurPage].Item[nCurPageItem].EditType = 4;
*/
				else if(Param->Page[i].Item[j].EditType >= 0x80 && Param->Page[i].Item[j].EditType <= 0x8F){	// Table
					int l, n, k;
					unsigned char tData[260];
					char *p;

					p = str;
					// n = テーブル番号
					n = Param->Page[i].Item[j].EditType & 0x0f;
					CheatReadStringsMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address, tData);

					for(l=0; l < Param->Page[i].Item[j].Size; l++){
						int m;

						for(m=0; m<Param->Table[n].nIn; m++){
							int data;
							DWORD dw;

							if(Param->Table[n].Info[m].Out_Size > (Param->Page[i].Item[j].Size - l))	// サイズが足りないばやい
								continue;
							dw = myreadnumtomem(Param->Table[n].Info[m].Out_Size, &tData[l]);

							if(Param->Table[n].Info[m].In_Type){
								// 入力が連続してるばやい
								int g = Param->Table[n].Info[m].In_s - Param->Table[n].Info[m].In_e;
								if((Param->Table[n].Info[m].Out <= dw) && (Param->Table[n].Info[m].Out + g >= dw)){
									data = Param->Table[n].Info[m].In_s + (Param->Table[n].Info[m].Out + g) - dw;
									if(data & 0xff00){
										*(short *)p = (short)data;
										p+=2;
									}
									else{
										*p = (char)data;
										p++;
									}
									l+=(Param->Table[n].Info[m].Out_Size-1);
									break;
								}
							}
							else{
								if(dw == Param->Table[n].Info[m].Out){
									data = Param->Table[n].Info[m].In_s;
									if(data & 0xff00){
										*(short *)p = (short)data;
										p+=2;
									}
									else{
										*p = (char)data;
										p++;
									}
									l+=(Param->Table[n].Info[m].Out_Size-1);
									break;
								}
							}
						}
						// 該当無し
						//				if(l == Param->Table[n].nIn){
						//				}
					}
					*p = '\0';
					SetWindowText(Param->Page[i].Item[j].hEdit, str);
				}
			}
			// 初期値セット
			else if(Param->Page[i].Item[j].InitValue){
				char str[260];
				if(Param->Page[i].Item[j].EditType == 1){
					wsprintf(str, "%X", Param->Page[i].Item[j].Data);
					SetWindowText(Param->Page[i].Item[j].hEdit, str);
				}
				else if(Param->Page[i].Item[j].EditType == 2){
					wsprintf(str, "%u", Param->Page[i].Item[j].Data);
					SetWindowText(Param->Page[i].Item[j].hEdit, str);
				}
				else 
					//				if(Param->Item[i].EditType == 0)
				{
					SetWindowText(Param->Page[i].Item[j].hEdit, Param->Page[i].Item[j].Str);
					//				strcpy(Param->Item[Param->nItem].Text, buf3);
				}
			}
		}
	}
	// Create Button window
	if(Param->ButtonName[0] == 0){
		strcpy(Param->ButtonName, "OK");
	}
	Param->hWriteButton = CreateWindow("BUTTON", Param->ButtonName, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			1, 1, 1, 1, hWnd, (HMENU)IDC_BUTTON1, hInst ,NULL);
}


// DestroyWindow
void DestroyCheatDtWriteWind(struct MyCheatStructWriteInfo *Param){
	int i,j,nPage;

	nPage = Param->nPage;
	if(nPage == 0)
		nPage = 1;
	for(i=0; i<nPage; i++){
		for(j=0; j<Param->Page[i].nItem; j++){
//			RemoveProp(Param->Page[i].Item[j].hEdit, PROP_EDITITEM);
			DestroyWindow(Param->Page[i].Item[j].hEdit);
		}
		Param->Page[i].nItem = 0;
	}
	if(Param->hWriteButton){
		DestroyWindow(Param->hWriteButton);
		Param->hWriteButton = NULL;
	}
	if(Param->hTabWin){
		DestroyWindow(Param->hTabWin);
		Param->hTabWin = NULL;
	}
	Param->nItem = 0;
}


// テキスト描画
void PaintCheatDtWriteWind(struct MyCheatStructWriteInfo *Param, HDC hDC, HWND hwnd){
	int i, nPage, OldMode;
	HFONT hFont, hOldFont;
	HBRUSH hBrBkgnd;
	RECT rc;

	hBrBkgnd = CreateSolidBrush(GetSysColor(COLOR_MENU));
	GetClientRect(hwnd, &rc);
	FillRect(hDC, &rc, hBrBkgnd);
    DeleteObject(hBrBkgnd);

	hFont = CreateFontIndirect(&Default_Font);
	hOldFont = (HFONT)SelectObject(hDC, hFont);
	OldMode = SetBkMode(hDC, TRANSPARENT);

	SetTextColor(hDC, GetSysColor(COLOR_MENUTEXT));

	nPage = Param->nCurPage;

//	TextOutA(hDC, 0, 120, "Test", strlen("Test"));

	if(Param->nPage){
		for(i=0; i<Param->Page[nPage].nItem; i++){
			TextOutA(hDC, Param->Page[nPage].Item[i].TEXT_X, Param->Page[nPage].Item[i].TEXT_Y + 30, Param->Page[nPage].Item[i].Text, strlen(Param->Page[nPage].Item[i].Text));
		}
	}
	else{
		for(i=0; i<Param->Page[nPage].nItem; i++){
			TextOutA(hDC, Param->Page[nPage].Item[i].TEXT_X, Param->Page[nPage].Item[i].TEXT_Y, Param->Page[nPage].Item[i].Text, strlen(Param->Page[nPage].Item[i].Text));
		}
	}
	SelectObject(hDC, hOldFont);
	SetBkMode(hDC, OldMode);

}


static char *mycopystrings(char *d, char *s){
	for(;*s != 0x09 && *s != 0x20 && *s != '\0'; d++, s++)
		*d = *s;
	*d = '\0';
	return s;
}


static char *mycopyostrings(char *d, char *s, char c){
	for(;*s != 0x09 && *s != 0x20 && *s != c && *s != '\0'; d++, s++)
		*d = *s;
	*d = '\0';
	return s+1;
}


static char *myskiptabstrings(char *s){
	while((*s == 0x09 || *s == 0x20) && *s != '\0')
		s++;
	return s;
}


static char *getstroneline(char *d, char *s, int byte){
	int i;

	byte--;
	if(*s == '\0')
		return NULL;
	for(i=0;*s != 0x0d && *s != '\0' && i<byte; s++, d++, i++)
		*d = *s;

	*d = '\0';
	s++;
	if(*s == 0x0a)
		s++;
	return s;
}


inline void mywritenumtomem(int size, DWORD dw, unsigned char *pmem){
	unsigned char *p;
	int i;

	p = (unsigned char *)&dw;

	for(i=0; i<size; i++){
		pmem[i] = p[i];
	}
}



// 書き換え実行
void WriteCheatDtWriteWind(struct MyCheatStructWriteInfo *Param){
	int i, j, nPage;
	DWORD dw;
	char str[260];

	nPage = Param->nPage;
	if(nPage == 0)
		nPage = 1;
	for(i=0; i<nPage; i++){
		for(j=0; j<Param->Page[i].nItem; j++){
			GetWindowText(Param->Page[i].Item[j].hEdit, str, 260);
			if(str[0] == '\0')
				continue;
			if(Param->Page[i].Item[j].EditType == 0){	//
				if(Param->Page[i].Item[j].DisableInput)
					CheatWriteStringsMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address, (unsigned char *)Param->Page[i].Item[j].Str);
				int n = strlen(str);
				memset(&str[i], Param->Page[i].Item[j].Padding, Param->Page[i].Item[j].Size - n);
				CheatWriteStringsMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address, (unsigned char *)str);
			}
			else if(Param->Page[i].Item[j].EditType == 1){
				if(Param->Page[i].Item[j].DisableInput)
					CheatWriteMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address, Param->Page[i].Item[j].Data);
				MyStrAtoh(str, &dw);
				CheatWriteMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address, dw);
			}
			else if(Param->Page[i].Item[j].EditType == 2){
				if(Param->Page[i].Item[j].DisableInput)
					CheatWriteMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address, Param->Page[i].Item[j].Data);
				dw = atoi(str);
				CheatWriteMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address, dw);
			}
			else if(Param->Page[i].Item[j].EditType == 3){	// hex
				char *p, tmp[64];
				int k;
				unsigned char tData[260];
				if(Param->Page[i].Item[j].DisableInput)
					p = Param->Page[i].Item[j].Str;
				else
					p = str;
				
				memset(tData, Param->Page[i].Item[j].Padding, Param->Page[i].Item[j].Size);
				for(k=0; *p != '\0' && *p != 0x09; k++){
					p = mycopyostrings(tmp, p, ',');
					MyStrAtoh(tmp, &dw);
					tData[k] = (unsigned char)dw;
					//				dw = atoi(str);
				}
				CheatWriteStringsMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address, tData);
			}
			else if(Param->Page[i].Item[j].EditType == 4){	// dec
				char *p, tmp[64];
				int k;
				unsigned char tData[260];
				if(Param->Page[i].Item[j].DisableInput)
					p = Param->Page[i].Item[j].Str;
				else
					p = str;
				
				memset(tData, Param->Page[i].Item[j].Padding, Param->Page[i].Item[j].Size);
				for(k=0; *p != '\0' && *p != 0x09; k++){
					p = mycopyostrings(tmp, p, ',');
					MyStrAtoh(tmp, &dw);
					//				tData[j] = dw;
					tData[k] = (unsigned char)atoi(str);
				}
				CheatWriteStringsMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address, tData);
			}
			else if(Param->Page[i].Item[j].EditType >= 0x80 && Param->Page[i].Item[j].EditType <= 0x8F){	// Table
				int l, n, k;
				char *p;
				unsigned char tData[512];
				
				
				n = Param->Page[i].Item[j].EditType & 0x0f;
				if(Param->Page[i].Item[j].DisableInput)
					p = Param->Page[i].Item[j].Str;
				else
					p = str;
				
				memset(tData, Param->Page[i].Item[j].Padding, Param->Page[i].Item[j].Size);
				for(l=0; *p != '\0'; ){
					int m;
					if(IsDBCSLeadByte((unsigned char)*p)){
						k = *(unsigned short *)p;
						p+=2;
					}
					else{
						k = *p;
						p++;
					}
					for(m=0; m<Param->Table[n].nIn; m++){
						if(Param->Table[n].Info[m].In_Type){
							if(k >= Param->Table[n].Info[m].In_s && k <= Param->Table[n].Info[m].In_e){
//								tData[l] = Param->Table[n].Info[m].Out + (k - Param->Table[n].Info[m].In_s);
								mywritenumtomem(Param->Table[n].Info[m].Out_Size, Param->Table[n].Info[m].Out + (k - Param->Table[n].Info[m].In_s), &tData[l]);
								l+=Param->Table[n].Info[m].Out_Size;
								break;
							}
						}
						else{
							if(Param->Table[n].Info[m].In_s == k){
//								tData[l] = Param->Table[n].Info[m].Out;
								mywritenumtomem(Param->Table[n].Info[m].Out_Size, Param->Table[n].Info[m].Out, &tData[l]);
								l+=Param->Table[n].Info[m].Out_Size;
								break;
							}
						}
					}
					// 該当無し
					//				if(l == Param->Table[n].nIn){
					//				}
				}
				CheatWriteStringsMem(Param->Page[i].Item[j].Size, Param->Page[i].Item[j].Address, tData);
			}
		}
	}
}



// 配置
void MoveCheatDtWriteWind(struct MyCheatStructWriteInfo *Param){
	int i, j, nPage;

	nPage = Param->nPage;
	if(nPage == 0)
		nPage = 1;
	if(Param->nPage){	// ページあり
		MoveWindow(Param->hTabWin, 0, 0, Param->WindowW, 28/*Param->WindowH*/, NULL);
		for(i=0; i<nPage; i++){
			for(j=0; j<Param->Page[i].nItem; j++){
				MoveWindow(Param->Page[i].Item[j].hEdit, Param->Page[i].Item[j].EDIT_X,
					Param->Page[i].Item[j].EDIT_Y + 30, Param->Page[i].Item[j].Width, 24, NULL);
			}
		}
	}
	else{	// ページなし
		for(i=0; i<nPage; i++){
			for(j=0; j<Param->Page[i].nItem; j++){
				MoveWindow(Param->Page[i].Item[j].hEdit, Param->Page[i].Item[j].EDIT_X,
					Param->Page[i].Item[j].EDIT_Y, Param->Page[i].Item[j].Width, 24, NULL);
			}
		}
	}
	MoveWindow(Param->hWriteButton, Param->WriteButtonRect.left,
		Param->WriteButtonRect.top +((Param->nPage)?30:0), Param->WriteButtonRect.right, Param->WriteButtonRect.bottom, NULL);

}


// ページ切り替わった時
void CheatStWt_ChangeTabWindowState(struct MyCheatStructWriteInfo *Param){
	int i,j;

//	return;
	for(i=0; i<Param->nPage; i++){
		if(i == Param->nCurPage){
			for(j=0; j<Param->Page[i].nItem; j++){
				ShowWindow(Param->Page[i].Item[j].hEdit, SW_SHOW);
			}
		}
		else{
			for(j=0; j<Param->Page[i].nItem; j++){
				ShowWindow(Param->Page[i].Item[j].hEdit, SW_HIDE);
			}
		}
	}
}



LRESULT CALLBACK MyCheatDtWriteWindProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp){

	static HINSTANCE hInst;

	switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT paint;
				HDC hdc = BeginPaint(hChild, &paint);
				PaintCheatDtWriteWind(&CheatStWtWindParam, hdc, hChild);
				EndPaint(hChild, &paint);
			}
			break;
		case WM_SIZE:
//			MoveCheatDtWriteWind(&CheatStWtWindParam);
			break;
		case WM_ERASEBKGND:
			break;
/*
		case WM_CTLCOLOREDIT:
			{
				HDC hdc;
				hdc = (HDC)wp;
				HBRUSH hbr_BG;
//				if((HWND)lParam == hwndEdit1)
				{
					SetTextColor(hdc, cr_FG1);
					SetBkColor(hdc, cr_BG1);
				}
				return (LRESULT)hbr_BG;
//				return (LRESULT)hbr_BG;
			}
*/
			break;

#if 0
		case WM_KEYDOWN:
			if((char)wp == VK_TAB){
				HWND fWind = GetFocus();
				int i;

				for(i=0; i<CheatStWtWindParam.nItem; i++){
					if(fWind ==  CheatStWtWindParam.Item[i].hEdit){
						SetFocus(CheatStWtWindParam.Item[(i+1)%CheatStWtWindParam.nItem].hEdit);
						break;
					}
				}
			}
			break;
#endif
		case WM_NOTIFY :
			switch(((LPNMHDR)lp)->code)
			{
				case TCN_SELCHANGE:	/* タブコントロール */
					CheatStWtWindParam.nCurPage = TabCtrl_GetCurSel(CheatStWtWindParam.hTabWin);
					CheatStWt_ChangeTabWindowState(&CheatStWtWindParam);
					InvalidateRect(hChild, NULL, TRUE);
					break;
			}
			break;

		case WM_EXTRAWIN_01:
			{
				RECT rct;
				CreateCheatDtWriteWind(&CheatStWtWindParam, hChild);
				MoveCheatDtWriteWind(&CheatStWtWindParam);
				CheatStWt_ChangeTabWindowState(&CheatStWtWindParam);


				SetRect(&rct, 0, 0, CheatStWtWindParam.WindowW, CheatStWtWindParam.WindowH);
				AdjustWindowRectEx(&rct, GetWindowStyle(hChild),
					GetMenu(hChild) != NULL,
					GetWindowExStyle(hChild));
				SetWindowPos(hChild, HWND_TOP, 0, 0,
					rct.right-rct.left, rct.bottom-rct.top, SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
//				if(CheatStWtWindParam.Title[0])
					SetWindowText(hChild, CheatStWtWindParam.Title);

				SetForegroundWindow(hChild);
				InvalidateRect(hChild, NULL, TRUE);
			}
			break;
		case WM_LBUTTONDOWN:
			SendMessage(hChild, WM_SYSCOMMAND, SC_MOVE|0x02, 0L);
			break;

#if 0
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
#endif

        case WM_COMMAND:
			switch (LOWORD(wp)){
				case IDC_BUTTON1: // write
					if(emu){
						WriteCheatDtWriteWind(&CheatStWtWindParam);
					}
					break;
			}
			break;

		case WM_CREATE:
			{
				char fn[MAX_PATH];
				RECT rct;
				if(CheatStWtWindParam.nItem == 0){
//					memset(&CheatStWtWindParam, 0, sizeof(struct MyCheatStructWriteInfo));
					if(MyCheatStWriteFileOpenDlg(hChild, fn) == 0)
						DestroyWindow(hChild);
					if(LoadCheatStructWriteFile(fn, &CheatStWtWindParam))
						DestroyWindow(hChild);
				}

				CreateCheatDtWriteWind(&CheatStWtWindParam, hChild);
				MoveCheatDtWriteWind(&CheatStWtWindParam);
				CheatStWt_ChangeTabWindowState(&CheatStWtWindParam);
				if(CheatStWtWindParam.Title[0])
					SetWindowText(hChild, CheatStWtWindParam.Title);

				SetRect(&rct, 0, 0, CheatStWtWindParam.WindowW, CheatStWtWindParam.WindowH);
				AdjustWindowRectEx(&rct, GetWindowStyle(hChild),
					GetMenu(hChild) != NULL,
					GetWindowExStyle(hChild));
				SetWindowPos(hChild, HWND_TOP, 0, 0,
					rct.right-rct.left, rct.bottom-rct.top, SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
			}
			return 0;
			break;
		case WM_DESTROY:
			g_extra_window.CheatStWriteWindow= NULL;
			DestroyCheatDtWriteWind(&CheatStWtWindParam);
			return 0;
			break;
		default:
			return (DefWindowProc(hChild, msg, wp, lp));
	}
	return 0L;
}




int LoadCheatStructWriteMem(char *mem, struct MyCheatStructWriteInfo *Param);


unsigned char *ReadFileToMem(char *fn){
	HANDLE hFile;
	DWORD dwAccBytes, Filesize;
	unsigned char *mem;
	hFile = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return 0;
	}
	Filesize = GetFileSize(hFile, NULL);
	mem = (unsigned char *)malloc(Filesize+1);
	if(mem == NULL){
		CloseHandle(hFile);
		return 0;
	}
	mem[Filesize] = 0;
	if(!ReadFile(hFile, mem, Filesize, &dwAccBytes, NULL)){
		return 0;
	}
	CloseHandle(hFile);
//	free(mem);
	return mem;
}


int LoadCheatStructWriteFile(char *fn, struct MyCheatStructWriteInfo *Param){
	unsigned char *mem;

	mem = ReadFileToMem(fn);
	if(mem==NULL)
		return 1;
	LoadCheatStructWriteMem((char *)mem, Param);
	free(mem);
	return 0;
}


int LoadCheatStructWriteMem(char *mem, struct MyCheatStructWriteInfo *Param){
	char buf[512], buf2[260], *p;
	int i = 0, ItemSection = 0, SepItemPos = 0, nSepItemPos, nCurPage = 0, nCurPageItem = 0;
	int FirstPage = 1;

	DestroyCheatDtWriteWind(Param);
	memset(Param, 0, sizeof(struct MyCheatStructWriteInfo));

	while(mem = getstroneline(buf, mem, 512)){
		if(buf[0] == ';' || buf[0] == '\0')	// comment
			continue;
		i = strlen(buf) - 1;
//		if(buf[i] == 0x0a)
//			buf[i] = 0;
		p = mycopystrings(buf2, buf);
		p = myskiptabstrings(p);
//		if(ItemSection != 6){
			if(!strcmp(buf2, "[INDEX]")){
				ItemSection = 1;
			}
			else if(!strcmp(buf2, "[TABLE]")){
				ItemSection = 2;
			}
			else if(!strcmp(buf2, "[WINDOW]")){
				Param->PosUserArrange = 1;
				ItemSection = 3;
			}
			else if(!strcmp(buf2, "[PAGE]")){
				ItemSection = 4;
				Param->nPage++;
				if(!FirstPage)
					nCurPage++;
				else
					FirstPage = 0;
				nCurPageItem = 0;
				nSepItemPos=0;
			}
			else if(!strcmp(buf2, "[ITEM_POS]")){
				ItemSection = 5;
				SepItemPos = 1;
				nSepItemPos = 0;
			}
			else if(!strcmp(buf2, "[ITEM]")){
				ItemSection = 6;
			}
			else if(ItemSection == 1 && !strcmp(buf2, "TITLE")){
				strcpy(Param->Title, p);
			}
			else if(ItemSection == 2 && !strncmp(buf2, "TBL", 3)){	// Table
				static int TableCount = 0;
				int n;
				char buf3[64];
				char *p2, md;


				p2 = mycopyostrings(buf3, buf2, '_');

				md = *p2;

				p = mycopystrings(buf2, p);
				p = myskiptabstrings(p);

				n = atoi(&buf3[3]);
				if(n > 9)
					break;
				if(md == 'O'){
					p2 = buf2;
					int l;
					DWORD dw;

					l = 0;
				lbl1:
					for(; *p2 != '\0'; l++){
						char *p3 = NULL;
						int cur_size;
						p2 = mycopyostrings(buf3, p2, ',');

						if(buf3[0]=='W'){		// WORD
							Param->Table[n].Info[l].Out_Size = 2;
							cur_size = 2;
						}
						else if(buf3[0]=='D'){	// DWORD
							Param->Table[n].Info[l].Out_Size = 4;
							cur_size = 4;
						}
						else if(buf3[0]=='T'){	// TBYTE
							Param->Table[n].Info[l].Out_Size = 3;
							cur_size = 3;
						}
						else {
							Param->Table[n].Info[l].Out_Size = 1;
							cur_size = 1;
						}
						for(p3=buf3; *p3 != '\0'; p3++){
							if(*p3 == '-'){
								*p3 = '\0';
								p3++;
								int ti, tn = atoi(p3); //MyStrAtoh(p3, &dw);
								if(cur_size != 1)
									MyStrAtoh(&buf3[1], &dw);
								else
									MyStrAtoh(buf3, &dw);
								for(ti = 0; ti<tn; ti++, dw++, l++){
									Param->Table[n].Info[l].Out_Size = cur_size;
									Param->Table[n].Info[l].Out = dw;
									mynummask(cur_size, &Param->Table[n].Info[l].Out);
								}
								break;
							}
						}
						if(*p3 == '\0'){
							if(cur_size != 1)
								MyStrAtoh(&buf3[1], &dw);
							else
								MyStrAtoh(buf3, &dw);
							mynummask(cur_size, &dw);
							Param->Table[n].Info[l].Out = dw;
						}
					}
					if(l && *(p2-1)==','){
						if(mem = getstroneline(buf, mem, 512)){
							p = myskiptabstrings(buf);
							p2 = mycopystrings(buf2, p);
							goto lbl1;
						}
					}
				}
				else if(md == 'I'){
					p2 = buf2;
					int l;

					l=0;
				lbl2:
					for(; *p2 != '\0'; l++){
						p2 = mycopyostrings(buf3, p2, ',');
						
						char *p3 = buf3;
						if(IsDBCSLeadByte((unsigned char)*p3)){
							Param->Table[n].Info[l].In_s = *(unsigned short *)p3;
							p3+=2;
						}
						else{
							Param->Table[n].Info[l].In_s = *p3;
							p3++;
						}
						
						if(*p3 == '-'){
							p3++;
							if(IsDBCSLeadByte((unsigned char)*p3)){
								Param->Table[n].Info[l].In_e = *(unsigned short *)p3;
								p3+=2;
							}
							else{
								Param->Table[n].Info[l].In_e = *p3;
								p3++;
							}
							// 連続した数値
							Param->Table[n].Info[l].In_Type = 1;
						}
						else{
							Param->Table[n].Info[l].In_Type = 0;
						}
						Param->Table[n].nIn++;
					}
					if(l && *(p2-1)==','){
						if(mem = getstroneline(buf, mem, 512)){
							p = myskiptabstrings(buf);
							p2 = mycopystrings(buf2, p);
							p2 = buf2;
							goto lbl2;
						}
					}

				}
//				strcpy(Param->Title, p);

				TableCount++;
				if(TableCount == 2){
					TableCount = 0;
					Param->nTable++;
				}
			}
			else if(ItemSection == 3 && !strcmp(buf2, "WINDOW_W")){	// WINDOW
				Param->WindowW = atoi(p);
			}
			else if(ItemSection == 3 && !strcmp(buf2, "WINDOW_H")){
				Param->WindowH = atoi(p);
			}
			else if(ItemSection == 3 && !strcmp(buf2, "BTN_POS")){
				char buf3[64];
				char *p2;

				p2 = mycopyostrings(buf3, p, ',');
				Param->WriteButtonRect.left = atoi(buf3);	// X
				p2 = mycopyostrings(buf3, p2, ',');
				Param->WriteButtonRect.top = atoi(buf3);	// Y
				p2 = mycopyostrings(buf3, p2, ',');
				Param->WriteButtonRect.right = atoi(buf3);	// W
				p2 = mycopyostrings(buf3, p2, ',');
				Param->WriteButtonRect.bottom = atoi(buf3);	// H
			}
			else if(ItemSection == 3 && !strcmp(buf2, "BTN_TXT")){
				strcpy(Param->ButtonName, p);
			}
/*
			else if(ItemSection == 3 && !strcmp(buf2, "OK_X")){
				Param->WriteButtonRect.left = atoi(p);
			}
			else if(ItemSection == 3 && !strcmp(buf2, "OK_Y")){
				Param->WriteButtonRect.top = atoi(p);
			}
			else if(ItemSection == 3 && !strcmp(buf2, "OK_W")){
				Param->WriteButtonRect.right = atoi(p);
			}
			else if(ItemSection == 3 && !strcmp(buf2, "OK_H")){
				Param->WriteButtonRect.bottom = atoi(p);
			}
*/
/*
			else if(ItemSection == 4 && !strcmp(buf2, "NO")){		// [PAGE]
				int nt = atoi(p);
				if(Param->nPage < nt){
					Param->nPage = nt;
				}
				Param->Page[nCurPage].nItem = nCurPageItem;
				nCurPage = nt;
				nCurPageItem = 0;
			}
*/
			else if(ItemSection == 4 && !strcmp(buf2, "TITLE")){
				strcpy(Param->Page[nCurPage].Title, p);
			}
			else if(ItemSection == 5){	// item pos
				char buf3[64];
				char *p2;

				p2 = mycopyostrings(buf3, buf2, ',');
				Param->Page[nCurPage].Item[nSepItemPos].TEXT_X= atoi(buf3);	// X
				Param->Page[nCurPage].Item[nSepItemPos].EDIT_X = Param->Page[nCurPage].Item[nSepItemPos].TEXT_X;	// X
				p2 = mycopyostrings(buf3, p2, ',');
				Param->Page[nCurPage].Item[nSepItemPos].TEXT_Y = atoi(buf3);	// Y
				Param->Page[nCurPage].Item[nSepItemPos].EDIT_Y = Param->Page[nCurPage].Item[nSepItemPos].TEXT_Y + 16;	// Y
				p2 = mycopyostrings(buf3, p2, ',');
				Param->Page[nCurPage].Item[nSepItemPos].Width = atoi(buf3);	// W

				nSepItemPos++;
			}
			else if(ItemSection == 6){	// Item section
				char buf3[64];
				char *p2;
				int j;
				
				if(buf2[0] == '\0')
					continue;
				
				if(Param->PosUserArrange && !SepItemPos){
					
					p2 = mycopyostrings(buf3, buf2, ',');
					Param->Page[nCurPage].Item[nCurPageItem].TEXT_X= atoi(buf3);	// X
					Param->Page[nCurPage].Item[nCurPageItem].EDIT_X = Param->Page[nCurPage].Item[nCurPageItem].TEXT_X;	// X
					p2 = mycopyostrings(buf3, p2, ',');
					Param->Page[nCurPage].Item[nCurPageItem].TEXT_Y = atoi(buf3);	// Y
					Param->Page[nCurPage].Item[nCurPageItem].EDIT_Y = Param->Page[nCurPage].Item[nCurPageItem].TEXT_Y + 16;	// Y
					p2 = mycopyostrings(buf3, p2, ',');
					Param->Page[nCurPage].Item[nCurPageItem].Width = atoi(buf3);	// W
					//				p = mycopyostrings(buf3, p, ',');
					//				Param->Item[Param->nItem].TEXT_Y = atoi(buf3);	// H
					
					p = mycopystrings(buf2, p);
					p = myskiptabstrings(p);
				}
				else{
					//				p = mycopystrings(buf2, buf);
					//				p = myskiptabstrings(p);
				}
				strcpy(Param->Page[nCurPage].Item[nCurPageItem].Text, buf2);	// Text
				
				
				p = mycopystrings(buf2, p);
				p = myskiptabstrings(p);
				p2 = mycopyostrings(buf3, buf2, '-');
				MyStrAtoh(buf3, &Param->Page[nCurPage].Item[nCurPageItem].Address);	// address
				
				p2 = mycopyostrings(buf3, p2, '-');
				Param->Page[nCurPage].Item[nCurPageItem].Size = atoi(buf3);	// size
				
				p2 = mycopyostrings(buf3, p2, '-');
				j = 0;
				if(buf3[j] == '#'){
					Param->Page[nCurPage].Item[nCurPageItem].LoadValue = 1;
					j++;
				}
				if(buf3[j] == '_'){
					Param->Page[nCurPage].Item[nCurPageItem].DisableInput = 1;
					j++;
				}
				if(buf3[j] == 'S')
					Param->Page[nCurPage].Item[nCurPageItem].EditType = 0;
				else if(buf3[j] == 'H')
					Param->Page[nCurPage].Item[nCurPageItem].EditType = 1;
				else if(buf3[j] == 'D')
					Param->Page[nCurPage].Item[nCurPageItem].EditType = 2;
				else if(buf3[j] == 'V')
					Param->Page[nCurPage].Item[nCurPageItem].EditType = 3;
				else if(buf3[j] == 'N')
					Param->Page[nCurPage].Item[nCurPageItem].EditType = 4;
				else if(buf3[j] == 'T')
					Param->Page[nCurPage].Item[nCurPageItem].EditType = 0x80;
				j++;
				if(buf3[j] != '\0'){
					DWORD dw;
					if(Param->Page[nCurPage].Item[nCurPageItem].EditType == 0x80){
						char tablen[5];
						
						tablen[1]=0;
						tablen[0]=buf3[j++];
						MyStrAtoh(tablen, &dw);
						Param->Page[nCurPage].Item[nCurPageItem].EditType += dw;
						Param->Page[nCurPage].Item[nCurPageItem].EditType &= 0x8F;
						Param->Page[nCurPage].Item[nCurPageItem].Padding = (unsigned char)MyStrAtoh(&buf3[j], &dw);
					}
					else{
						Param->Page[nCurPage].Item[nCurPageItem].Padding = (unsigned char)MyStrAtoh(&buf3[j], &dw);
					}
				}
				//			strcpy(Param->Item[Param->nItem].Text, buf3);	// type
				if(*p2 != '\0'){	// Default
					p2 = mycopyostrings(buf3, p2, '-');
					if(Param->Page[nCurPage].Item[nCurPageItem].EditType == 0){
						strcpy(Param->Page[nCurPage].Item[nCurPageItem].Str, buf3);
					}
					else if(Param->Page[nCurPage].Item[nCurPageItem].EditType == 1){
						MyStrAtoh(buf3, &Param->Page[nCurPage].Item[nCurPageItem].Data);
					}
					else if(Param->Page[nCurPage].Item[nCurPageItem].EditType == 2){
						Param->Page[nCurPage].Item[nCurPageItem].Data = atoi(buf3);
					}
					Param->Page[nCurPage].Item[nCurPageItem].InitValue = 1;
				}
				Param->Page[nCurPage].nItem++;
				Param->nItem++;
				nCurPageItem++;
			}
	}

	if(!Param->PosUserArrange){  // ウィンドウの配置をセット
		int nh, xn, nPage, j, PageH = 0, PageW=0;

		Param->WindowH = 0;
		Param->WindowW = 0;

		nPage = Param->nPage;
		if(nPage == 0)
			nPage = 1;

		for(i=0; i<nPage; i++){

			if(Param->Page[i].nItem <= 1){
				nh = 1;		 // 横1列
				xn = 1;
			}
			else if(Param->Page[i].nItem <= 20){
				nh = Param->Page[i].nItem / 2 + (Param->Page[i].nItem & 1); // 横2列
				xn = 2;
			}
			else{
				nh = Param->Page[i].nItem / 3 + ((Param->Page[i].nItem % 3)?1:0); // 横3列
				xn = 3;
			}
			for(j=0; j<Param->Page[i].nItem; j++){
				
				Param->Page[i].Item[j].EDIT_X = 5 + j/nh * 120;
				Param->Page[i].Item[j].TEXT_X = 5 + j/nh * 120;
				
				Param->Page[i].Item[j].TEXT_Y = (j % nh) * 48;
				Param->Page[i].Item[j].EDIT_Y = (j % nh) * 48 + 16;
				
				Param->Page[i].Item[j].Width = 110;
				
			}
			PageH = nh * 48 + 48;
			if(Param->WindowH < PageH)
				Param->WindowH = PageH;
			PageW = 120*xn+10;
			if(Param->WindowW < PageW)
				Param->WindowW = PageW;

		}
		Param->WriteButtonRect.left = Param->WindowW / 2 - 30;
		Param->WriteButtonRect.right= 60;
		Param->WriteButtonRect.top  = Param->WindowH - 30;
		Param->WriteButtonRect.bottom=24;
		if(Param->nPage)
			Param->WindowH += 30;
	}
//	Param->nCurPage = nCurPage;

	return 0;
}




LRESULT CALLBACK MyCheatDtWriteEditWindProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp);


HWND CreateMyCheatStWriteEditWindow(HWND hWnd){
    WNDCLASSEX wc;
	static int rflag=0;
	RECT rc;
	HINSTANCE hInst;

	if(g_extra_window.CheatStWriteEditWindow){
		SetForegroundWindow(g_extra_window.CheatStWriteEditWindow);
		return NULL;
	}
	hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	GetWindowRect(hWnd, &rc);

	if(rflag==0)
	{
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_VREDRAW | CS_HREDRAW;
		wc.lpfnWndProc = MyCheatDtWriteEditWindProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.lpszMenuName = NULL; //MAKEINTRESOURCE(IDR_MENUCT1);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
		wc.lpszClassName = (LPCSTR)"CheatStEditWrite";
		wc.hIconSm = (HICON)LoadImage(g_main_instance,MAKEINTRESOURCE(IDI_ICON44),IMAGE_ICON  , 0, 0,LR_DEFAULTCOLOR);
		if(RegisterClassEx(&wc) == 0) {
			return NULL;
		}
		rflag=1;
	}
	g_extra_window.CheatStWriteEditWindow = CreateWindowEx(/*WS_EX_TOOLWINDOW*/ NULL, "CheatStEditWrite", "Edit Window", WS_OVERLAPPED  | WS_THICKFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
		rc.left+100, rc.top+100, 256/*256*/, 256, NULL/*hWnd*/, LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENUCT1))/*NULL*/, hInst, NULL);
	ShowWindow(g_extra_window.CheatStWriteEditWindow, SW_SHOW);
	UpdateWindow(g_extra_window.CheatStWriteEditWindow);
	return g_extra_window.CheatStWriteEditWindow;
}




LRESULT CALLBACK MyCheatDtWriteEditWindProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp){

	static HINSTANCE hInst;
	static HWND hEdit, hButton;

	switch (msg) {
		case WM_PAINT:
			{
				PAINTSTRUCT paint;
				HDC hdc = BeginPaint(hChild, &paint);
				EndPaint(hChild, &paint);
			}
			break;
		case WM_SIZE:
//			MoveCheatDtWriteWind(&CheatStWtWindParam);
//nWidth = LOWORD(lParam);
//nHeight = HIWORD(lParam);
			MoveWindow(hEdit, 0, 0, LOWORD(lp), HIWORD(lp)-30, NULL);
			MoveWindow(hButton, LOWORD(lp)/2-40, HIWORD(lp)-28, 80, 25, NULL);

			break;
#if 0
		case WM_KEYDOWN:
			if((char)wp == VK_TAB){
				HWND fWind = GetFocus();
				int i;

				for(i=0; i<CheatStWtWindParam.nItem; i++){
					if(fWind ==  CheatStWtWindParam.Item[i].hEdit){
						SetFocus(CheatStWtWindParam.Item[(i+1)%CheatStWtWindParam.nItem].hEdit);
						break;
					}
				}
			}
			break;
#endif
		case WM_LBUTTONDOWN:
			SendMessage(hChild, WM_SYSCOMMAND, SC_MOVE|0x02, 0L);
			break;

#if 0
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
#endif

        case WM_COMMAND:
			switch (LOWORD(wp)){
				case IDC_BUTTON1: // OK
//					if(g_extra_window.CheatStWriteWindow)
					{
						unsigned char *mem;
						int size;
						size=GetWindowTextLength(hEdit);
						mem = (unsigned char *)malloc(size+1);
						if(mem == NULL)
							break;
						mem[size] = 0;
						GetWindowText(hEdit, (char *)mem, size+1);
						LoadCheatStructWriteMem((char *)mem, &CheatStWtWindParam);
						free(mem);
						if(g_extra_window.CheatStWriteWindow){
							SendMessage(g_extra_window.CheatStWriteWindow, WM_EXTRAWIN_01, 0, 0);
						}
						else{
							if(CheatStWtWindParam.nItem)
								CreateMyCheatStWriteWindow(main_window_handle);
						}
//						WriteCheatDtWriteWind(&CheatStWtWindParam);
					}
					break;
				case IDM_CT1:	// read
					{
						char fn[MAX_PATH];
						unsigned char *mem;
						if(MyCheatStWriteFileOpenDlg(hChild, fn) == 0)
							break;
						mem = ReadFileToMem(fn);
						if(mem == NULL)
							break;
						SetWindowText(hEdit, (char *)mem);
						free(mem);
						return 0;
					}
					break;
				case IDM_CT3:	// write
					{
						char fn[MAX_PATH];
						unsigned char *mem;
						int size;
						FILE *fp = NULL;
						if(MyCheatStWriteFileSaveDlg(hChild, fn) == 0)
							break;
						size=GetWindowTextLength(hEdit);
						mem = (unsigned char *)malloc(size+1);
						if(mem == NULL)
							break;
						GetWindowText(hEdit, (char *)mem, size+1);
						fp = fopen(fn, "wb");
						if(!fp){
							free(mem);
							break;
						}
						fwrite(mem, size, 1, fp);
						fclose(fp);
						free(mem);
						return 0;
					}
					break;
			}
			break;

		case WM_CREATE:
			{
				DWORD param[1] = { 16 };
				hButton = CreateWindow("BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
					1, 1, 1, 1, hChild, (HMENU)IDC_BUTTON1, g_main_instance ,NULL);
				hEdit = CreateWindowEx(WS_EX_STATICEDGE, "EDIT", "", WS_CHILD|  WS_VISIBLE| WS_BORDER
					| ES_MULTILINE | ES_WANTRETURN | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0, 0,
					0, 0, hChild, (HMENU)IDC_EDIT1, g_main_instance, NULL);
				SendMessage(hEdit, EM_SETTABSTOPS, 1, (LPARAM)param);
//				SendMessage(hEdit, EM_SETMARGINS, );
			}
			return 0;
			break;
		case WM_DESTROY:
			g_extra_window.CheatStWriteEditWindow= NULL;
			return 0;
			break;
		default:
			return (DefWindowProc(hChild, msg, wp, lp));
	}
	return 0L;
}


