
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <winuser.h>
#include <stdio.h>
#include <wingdi.h>
#include "settings.h"
#include "resource.h"
#include "win32_emu.h"
#include "cheat.h"
#include "nnndialog.h"
#include "savecfg.h"
#include "ulunzip.h"
#include "mreadstream.h"

extern emulator* emu;
extern HINSTANCE g_main_instance;
unsigned char editpalet[64*3];
extern uint8 NES_preset_palette[64][3];
extern HWND phList;
extern struct Preview_state g_Preview_State;
struct MyWindowShortCutKey g_Window_ShotrCutKey[1];
extern HWND main_window_handle;
extern struct extra_window_struct g_extra_window;


void defaultPaletteFileRead(){
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "pal\\ndefault.pal");

	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		GetModuleFileName(NULL, str, MAX_PATH);
		PathRemoveFileSpec(str);
		PathCombine(str , str, "ndefault.pal");
		hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE){
			memcpy(editpalet, NES_preset_palette, 64*3);
			return;
		}
    }
	ReadFile(hFile, editpalet, 64*3, &dwAccBytes, NULL);
	CloseHandle(hFile);
	return;
}


void defaultPaletteFileSave(){
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "pal\\ndefault.pal");

	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		GetModuleFileName(NULL, str, MAX_PATH);
		PathRemoveFileSpec(str);
		PathCombine(str , str, "ndefault.pal");
		hFile = CreateFile(str, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			return;
    }
	WriteFile(hFile, editpalet, 64*3, &dwAccBytes, NULL);
	CloseHandle(hFile);
	return;
}


void PaletteFileSave(HWND hWnd){
	HANDLE hFile;
	DWORD dwAccBytes;
    OPENFILENAME ofn;
	char szStr[MAX_PATH]="", str2[64];
	char FileName[MAX_PATH]="";
	char initdir[MAX_PATH];

	GetModuleFileName(NULL, initdir, MAX_PATH);
	PathRemoveFileSpec(initdir);
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Palette Files(*.pal)\0*.pal\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = FileName;
    ofn.lpstrFileTitle = szStr;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "pal";
	LoadString(g_main_instance, IDS_STRING_CDLG_11 , str2, 64);
	ofn.lpstrTitle	=str2;
	ofn.lpstrInitialDir = initdir;
	if(GetSaveFileName(&ofn) == 0)
        return;
   	hFile = CreateFile(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return;
	WriteFile(hFile, editpalet, 64*3, &dwAccBytes, NULL);
	CloseHandle(hFile);
	return;
}


void PaletteFileRead(HWND hWnd){
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes;
    OPENFILENAME ofn;
	char szFile[MAX_PATH]="", str2[64];
	char FileName[MAX_PATH]="";

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
//	PathCombine(str , str, "Cheat");

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Palette Files(*.pal)\0*.pal\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = FileName;
    ofn.lpstrFileTitle = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = "pal";
	LoadString(g_main_instance, IDS_STRING_CDLG_12 , str2, 64);
	ofn.lpstrTitle	=str2;
	ofn.lpstrInitialDir = str;

    if(GetOpenFileName(&ofn) == 0)
        return;
	hFile = CreateFile(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		return;
    }
	ReadFile(hFile, editpalet, 64*3, &dwAccBytes, NULL);
	CloseHandle(hFile);
	return;
}


void nnnallpaletdraw(HWND hWnd, HDC hdc, unsigned char nplt){
	int i,j;
	RGBQUAD rgbq[2];
	LPBITMAPINFO lpbi = NULL;
	unsigned char data[24*24];


	lpbi = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
	lpbi->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	lpbi->bmiHeader.biWidth=24;
	lpbi->bmiHeader.biHeight=24;
	lpbi->bmiHeader.biPlanes=1;
	lpbi->bmiHeader.biBitCount=8;
	lpbi->bmiHeader.biCompression=BI_RGB;
	lpbi->bmiHeader.biSizeImage=0;
	lpbi->bmiHeader.biXPelsPerMeter=0;
	lpbi->bmiHeader.biYPelsPerMeter=0;
	lpbi->bmiHeader.biClrUsed=0;
	lpbi->bmiHeader.biClrImportant=0;
//	bminfo.bmiColors[0]=rgbq[0];
	for(i=0;i<24;++i){
		for(j=0;j<24;++j)
			if(j==23||j==0||i==0||i==23)
				data[i*24+j]=0;
			else
				data[i*24+j]=1;
	}
	rgbq[0].rgbBlue=0xff;
	rgbq[0].rgbGreen=0xff;
	rgbq[0].rgbRed=0xff;
	rgbq[1].rgbBlue=0;
	rgbq[1].rgbGreen=0;
	rgbq[1].rgbRed=0;
	for(i=0;i<4;++i){
		for(j=0;j<16;++j){
			if(i*16+j==nplt){
				lpbi->bmiColors[0]=rgbq[1];
			}
			else{
				lpbi->bmiColors[0]=rgbq[0];
			}
			lpbi->bmiColors[1].rgbBlue=editpalet[(i*16+j)*3+2];
			lpbi->bmiColors[1].rgbGreen=editpalet[(i*16+j)*3+1];
			lpbi->bmiColors[1].rgbRed=editpalet[(i*16+j)*3];
			SetDIBitsToDevice(hdc,j*24+8,i*24+8, 24, 24, 0,0, 0, 24, &data, lpbi, DIB_RGB_COLORS);
		}
	}
	if(lpbi)
		free(lpbi);
	return;
}


void nnnEditPaletInit(HWND hDlg, unsigned char pn , unsigned char *nplt){
	nplt[2]=editpalet[pn*3+2];
	nplt[1]=editpalet[pn*3+1];
	nplt[0]=editpalet[pn*3];
	SendMessage(GetDlgItem(hDlg, IDC_SLIDERR),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,255));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDERG),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,255));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDERB),TBM_SETRANGE,FALSE,(LPARAM)MAKELONG(0,255));
	SendMessage(GetDlgItem(hDlg, IDC_SLIDERR), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)nplt[0]);
	SendMessage(GetDlgItem(hDlg, IDC_SLIDERG), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)nplt[1]);
	SendMessage(GetDlgItem(hDlg, IDC_SLIDERB), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)nplt[2]);
	SetDlgItemInt(hDlg, IDC_STATICCOLORNUMR, nplt[0], FALSE);
	SetDlgItemInt(hDlg, IDC_STATICCOLORNUMG, nplt[1], FALSE);
	SetDlgItemInt(hDlg, IDC_STATICCOLORNUMB, nplt[2], FALSE);
	SetDlgItemInt(hDlg, IDC_STATICCOLORPLTNUM, pn, FALSE);
	return;
}


//IDD_NNNEDITPALETDIALOG
LRESULT CALLBACK nnnEditPaletDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp){
	static unsigned char paletnum=0, rclick=0, temppalette[64*3]={0}, oneundo[4];
	unsigned char nplt[3];
	PAINTSTRUCT ps;
	HDC hdc;

	switch(msg){
		case WM_INITDIALOG:
			oneundo[0]=paletnum;
			memcpy(&oneundo[1], &editpalet[paletnum*3],3);
			memcpy(temppalette, editpalet, 64*3);
			nnnEditPaletInit(hDlg, paletnum, nplt);
			if(rclick==0)
				SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			else if(rclick==1)
				SendMessage(GetDlgItem(hDlg, IDC_RADIO2), BM_SETCHECK, (WPARAM)TRUE, 0L);
			else
				SendMessage(GetDlgItem(hDlg, IDC_RADIO3), BM_SETCHECK, (WPARAM)TRUE, 0L);
			break;
		case WM_PAINT:
			{
			hdc = BeginPaint(hDlg, &ps);
			nnnallpaletdraw(hDlg, hdc, paletnum);
			EndPaint(hDlg, &ps);
			}
			break;
		case WM_RBUTTONDOWN:
			{
			int x,y,i;
			x=LOWORD(lp);
			y=HIWORD(lp);
			if(x>=8 && x<24*16+8 && y>=8 && y<24*4+8){
				i = (y-8)/24*16+((x-8)/24);
				if(rclick==0){
					oneundo[0]=i;
					memcpy(&oneundo[1], &editpalet[i*3],3);
					editpalet[i*3] = editpalet[paletnum*3];
					editpalet[i*3+1] = editpalet[paletnum*3+1];
					editpalet[i*3+2] = editpalet[paletnum*3+2];
				}
				else if(rclick==1){
					oneundo[0]=paletnum;
					memcpy(&oneundo[1], &editpalet[paletnum*3],3);
//					editpalet[i*3] = (editpalet[paletnum*3]+editpalet[i*3])/2;
//					editpalet[i*3+1] = (editpalet[i*3+1]+editpalet[paletnum*3+1])/2;
//					editpalet[i*3+2] = (editpalet[i*3+2]+editpalet[paletnum*3+2])/2;
					editpalet[paletnum*3] = (editpalet[paletnum*3]+editpalet[i*3])/2;
					editpalet[paletnum*3+1] = (editpalet[i*3+1]+editpalet[paletnum*3+1])/2;
					editpalet[paletnum*3+2] = (editpalet[i*3+2]+editpalet[paletnum*3+2])/2;
				}
				else{
					unsigned char t[3];
					t[0]=editpalet[paletnum*3],t[1]=editpalet[paletnum*3+1],t[2]=editpalet[paletnum*3+2];
					editpalet[paletnum*3]=editpalet[i*3],editpalet[paletnum*3+1]=editpalet[i*3+1],editpalet[paletnum*3+2]=editpalet[i*3+2];
					editpalet[i*3]=t[0], editpalet[i*3+1]=t[1],editpalet[i*3+2]=t[2];
				}
				SetDlgItemInt(hDlg, IDC_STATICCOLORPLTNUM, paletnum, FALSE);
				nnnEditPaletInit(hDlg, paletnum, nplt);
				hdc=GetDC(hDlg);
				nnnallpaletdraw(hDlg, hdc, paletnum);
				ReleaseDC(hDlg, hdc);
				SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
				SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
			}
			}
			return TRUE;
			break;
		case WM_LBUTTONDOWN:
			{
			int x,y;
			x=LOWORD(lp);
			y=HIWORD(lp);
			if(x>=8 && x<24*16+8 && y>=8 && y<24*4+8){
				paletnum = (y-8)/24*16+((x-8)/24);
				SetDlgItemInt(hDlg, IDC_STATICCOLORPLTNUM, paletnum, FALSE);
				nnnEditPaletInit(hDlg, paletnum, nplt);
				hdc=GetDC(hDlg);
				nnnallpaletdraw(hDlg, hdc, paletnum);
				ReleaseDC(hDlg, hdc);
				oneundo[0]=paletnum;
				memcpy(&oneundo[1], &editpalet[paletnum*3],3);
			}
			}
			return TRUE;
			break;
		case WM_HSCROLL:
			if(GetDlgItem(hDlg, IDC_SLIDERR) == (HWND)lp){
				nplt[0] = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_SLIDERR), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				SetDlgItemInt(hDlg, IDC_STATICCOLORNUMR, nplt[0], FALSE);
				editpalet[paletnum*3]=nplt[0];
			}
			else if(GetDlgItem(hDlg, IDC_SLIDERG) == (HWND)lp){
				nplt[1] = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_SLIDERG), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				SetDlgItemInt(hDlg, IDC_STATICCOLORNUMG, nplt[1], FALSE);
				editpalet[paletnum*3+1]=nplt[1];
			}
			else if(GetDlgItem(hDlg, IDC_SLIDERB) == (HWND)lp){
				nplt[2] = (unsigned char)SendMessage(GetDlgItem(hDlg, IDC_SLIDERB), TBM_GETPOS, (WPARAM)0, (LPARAM)0);
				SetDlgItemInt(hDlg, IDC_STATICCOLORNUMB, nplt[2], FALSE);
				editpalet[paletnum*3+2]=nplt[2];
			}
			hdc=GetDC(hDlg);
			nnnallpaletdraw(hDlg, hdc, paletnum);
			ReleaseDC(hDlg, hdc);
			SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
			SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wp)){
				case IDC_RADIO1:
					rclick=0;
					break;
				case IDC_RADIO2:
					rclick=1;
					break;
				case IDC_RADIO3:
					rclick=2;
					break;
				case IDC_FREAD:
					PaletteFileRead(hDlg);
					hdc=GetDC(hDlg);
					nnnallpaletdraw(hDlg, hdc, paletnum);
					ReleaseDC(hDlg, hdc);
					nnnEditPaletInit(hDlg, paletnum, nplt);
					SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
					SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
					return TRUE;
				case IDC_FSAVE:
					PaletteFileSave(hDlg);
					return TRUE;
				case IDC_DEFAULT:
					memcpy(editpalet, NES_preset_palette, 64*3);
					nnnEditPaletInit(hDlg, paletnum, nplt);
					hdc=GetDC(hDlg);
					nnnallpaletdraw(hDlg, hdc, paletnum);
					ReleaseDC(hDlg, hdc);
					SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
					SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
					return TRUE;
				case IDOK:
					defaultPaletteFileSave();
					EndDialog(hDlg, IDOK);
					return TRUE;
				case IDCANCEL:
					memcpy(editpalet, temppalette, 64*3);
					SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
					SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
					EndDialog(hDlg, FALSE);
					return TRUE;
				case IDC_BUTTON1:
					memcpy(editpalet, temppalette, 64*3);
					hdc=GetDC(hDlg);
					nnnallpaletdraw(hDlg, hdc, paletnum);
					ReleaseDC(hDlg, hdc);
					nnnEditPaletInit(hDlg, paletnum, nplt);
					SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
					SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
					return TRUE;
				case IDC_BUTTON2:
					memcpy(&editpalet[oneundo[0]*3], &oneundo[1],3);
					hdc=GetDC(hDlg);
					nnnallpaletdraw(hDlg, hdc, paletnum);
					ReleaseDC(hDlg, hdc);
					nnnEditPaletInit(hDlg, paletnum, nplt);
					SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
					SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
					return TRUE;
			}
	}
	return FALSE;
}


extern unsigned char *SResulttext;
/*
//IDD_TESTDIALOG1
LRESULT CALLBACK MyLagTestDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

    switch(msg) {
		case WM_COMMAND:
            switch(LOWORD(wp)){
                case IDOK:
					GetDlgItemText(hDlgWnd, IDC_EDIT1, (LPTSTR)SResulttext, 17);
//					nnnKailleraLagNum = (unsigned char)GetDlgItemInt(hDlgWnd, IDC_EDIT1, NULL, FALSE);
					EndDialog(hDlgWnd, IDOK);
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
        case WM_INITDIALOG:
//			SetDlgItemInt(hDlgWnd, IDC_EDIT1, nnnKailleraLagNum, 0);
            return TRUE;
    }
    return FALSE;
}
*/

//IDD_MOVIEINSERTMSG
LRESULT CALLBACK MovieMessageDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

    switch(msg) {
		case WM_COMMAND:
            switch(LOWORD(wp)){
                case IDOK:
					{
						char str[256];
						str[0]=0;
						GetDlgItemText(hDlgWnd, IDC_EDIT1, (LPTSTR)str, 256);
						if(str[0])
							emu->InsertMovieMsg(str);
					}
					EndDialog(hDlgWnd, IDOK);
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



void LoadIPSPatch(unsigned char *rp, char *rfn){
	char *p, str1[MAX_PATH];
	unsigned char rbuff[5];
	DWORD dwAccBytes=0, adr, psize;
	HANDLE hFile;

	strcpy(str1, rfn);
	PathRenameExtension(str1, ".ips");
	hFile = CreateFile(str1, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		GetModuleFileName(NULL, str1, MAX_PATH);
		PathRemoveFileSpec(str1);
		p=PathFindFileName(rfn);
		if(NESTER_settings.path.UseIPSPath){
			PathCombine(str1, str1, NESTER_settings.path.szIPSPatchPath);
		}
		else{
			PathCombine(str1, str1, "ips");
		}
		PathCombine(str1, str1, p);
		PathRenameExtension(str1, ".ips");
		hFile = CreateFile(str1, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
	}
	ReadFile(hFile, rbuff, 5, &dwAccBytes, NULL);
	if(dwAccBytes!=5){
		CloseHandle(hFile);
		return;
	}
	if(strncmp((char *)rbuff, "PATCH", 5)){
		CloseHandle(hFile);
		return;
	}
	while(1){
		ReadFile(hFile, rbuff, 5, &dwAccBytes, NULL);
		if(dwAccBytes!=5)
			break;
		adr = (rbuff[0]<<16) | (rbuff[1]<<8) | rbuff[2];
		psize = (rbuff[3]<<8) | rbuff[4];
		if(psize==0){
			ReadFile(hFile, rbuff, 3, &dwAccBytes, NULL);
			psize = (rbuff[0]<<8) | rbuff[1];
			memset(&rp[adr-0x10], rbuff[2], psize);
		}
		else{
			ReadFile(hFile, &rp[adr-0x10], psize, &dwAccBytes, NULL);
			if(dwAccBytes!=psize)
				break;
		}
	}
	CloseHandle(hFile);
	return;
}


#define CHRROMEDITXN 256/8  //43		// x / 8    256,
#define CHRROMEDITYN 26		// y / 8      240,
#define CHRROMEDITSTY 80
#define CHRROMEDITSTX 16


void nnn8x8charadraw(unsigned char *outd, unsigned char *temp, int x){
	for(int l=0; l<8; ++l){
#if 0
		outd[l*x] = (((temp[l]>>7)&1) | (((temp[l+8]>>7)&1)<<1));
		outd[l*x+1] = (((temp[l]>>6)&1) | (((temp[l+8]>>6)&1)<<1));
		outd[l*x+2] = (((temp[l]>>5)&1) | (((temp[l+8]>>5)&1)<<1));
		outd[l*x+3] = (((temp[l]>>4)&1) | (((temp[l+8]>>4)&1)<<1));
		outd[l*x+4] = (((temp[l]>>3)&1) | (((temp[l+8]>>3)&1)<<1));
		outd[l*x+5] = (((temp[l]>>2)&1) | (((temp[l+8]>>2)&1)<<1));
		outd[l*x+6] = (((temp[l]>>1)&1) | (((temp[l+8]>>1)&1)<<1));
		outd[l*x+7] = ((temp[l]&1) | ((temp[l+8]&1)<<1));
#else
		outd[(7-l)*x] = (((temp[l]>>7)&1) | (((temp[l+8]>>7)&1)<<1));
		outd[(7-l)*x+1] = (((temp[l]>>6)&1) | (((temp[l+8]>>6)&1)<<1));
		outd[(7-l)*x+2] = (((temp[l]>>5)&1) | (((temp[l+8]>>5)&1)<<1));
		outd[(7-l)*x+3] = (((temp[l]>>4)&1) | (((temp[l+8]>>4)&1)<<1));
		outd[(7-l)*x+4] = (((temp[l]>>3)&1) | (((temp[l+8]>>3)&1)<<1));
		outd[(7-l)*x+5] = (((temp[l]>>2)&1) | (((temp[l+8]>>2)&1)<<1));
		outd[(7-l)*x+6] = (((temp[l]>>1)&1) | (((temp[l+8]>>1)&1)<<1));
		outd[(7-l)*x+7] = ((temp[l]&1) | ((temp[l+8]&1)<<1));
#endif
	}
	return;
}


void nnn8x8charadrawmono(unsigned char *outd, unsigned char *temp, int x){
	for(int l=0; l<8; ++l){
#if 0
		outd[l*x] = (((temp[l]>>7)&1) | (((temp[l+8]>>7)&1)<<1));
		outd[l*x+1] = (((temp[l]>>6)&1) | (((temp[l+8]>>6)&1)<<1));
		outd[l*x+2] = (((temp[l]>>5)&1) | (((temp[l+8]>>5)&1)<<1));
		outd[l*x+3] = (((temp[l]>>4)&1) | (((temp[l+8]>>4)&1)<<1));
		outd[l*x+4] = (((temp[l]>>3)&1) | (((temp[l+8]>>3)&1)<<1));
		outd[l*x+5] = (((temp[l]>>2)&1) | (((temp[l+8]>>2)&1)<<1));
		outd[l*x+6] = (((temp[l]>>1)&1) | (((temp[l+8]>>1)&1)<<1));
		outd[l*x+7] = ((temp[l]&1) | ((temp[l+8]&1)<<1));

#else
		outd[(7-l)*x] = ((temp[l]>>7)&1);
		outd[(7-l)*x+1] = ((temp[l]>>6)&1);
		outd[(7-l)*x+2] = ((temp[l]>>5)&1);
		outd[(7-l)*x+3] = ((temp[l]>>4)&1);
		outd[(7-l)*x+4] = ((temp[l]>>3)&1);
		outd[(7-l)*x+5] = ((temp[l]>>2)&1);
		outd[(7-l)*x+6] = ((temp[l]>>1)&1);
		outd[(7-l)*x+7] = (temp[l]&1);
#endif
	}
	return;
}


void nnn8x8characonv(unsigned char *outd, unsigned char *temp, int x){
	for(int l=0; l<8; ++l){
#if 0
#else
		temp[l]	= 0;
		temp[l+8] = 0;
		if(outd[(7-l)*x] & 1)
			temp[l]|=0x80;
		if(outd[(7-l)*x] & 2)
			temp[l+8]|=0x80;
		if(outd[(7-l)*x+1] & 1)
			temp[l]|=0x40;
		if(outd[(7-l)*x+1] & 2)
			temp[l+8]|=0x40;
		if(outd[(7-l)*x+2] & 1)
			temp[l]|=0x20;
		if(outd[(7-l)*x+2] & 2)
			temp[l+8]|=0x20;
		if(outd[(7-l)*x+3] & 1)
			temp[l]|=0x10;
		if(outd[(7-l)*x+3] & 2)
			temp[l+8]|=0x10;
		if(outd[(7-l)*x+4] & 1)
			temp[l]|=8;
		if(outd[(7-l)*x+4] & 2)
			temp[l+8]|=8;
		if(outd[(7-l)*x+5] & 1)
			temp[l]|=4;
		if(outd[(7-l)*x+5] & 2)
			temp[l+8]|=4;
		if(outd[(7-l)*x+6] & 1)
			temp[l]|=2;
		if(outd[(7-l)*x+6] & 2)
			temp[l+8]|=2;
		if(outd[(7-l)*x+7] & 1)
			temp[l]|=1;
		if(outd[(7-l)*x+7] & 2)
			temp[l+8]|=1;
#endif
	}
	return;
}


void nnn8x8characonvmono(unsigned char *outd, unsigned char *temp, int x){
	for(int l=0; l<8; ++l){
#if 0
#else
		temp[l]	= 0;
		if(outd[(7-l)*x] & 1)
			temp[l]|=0x80;
		if(outd[(7-l)*x+1] & 1)
			temp[l]|=0x40;
		if(outd[(7-l)*x+2] & 1)
			temp[l]|=0x20;
		if(outd[(7-l)*x+3] & 1)
			temp[l]|=0x10;
		if(outd[(7-l)*x+4] & 1)
			temp[l]|=8;
		if(outd[(7-l)*x+5] & 1)
			temp[l]|=4;
		if(outd[(7-l)*x+6] & 1)
			temp[l]|=2;
		if(outd[(7-l)*x+7] & 1)
			temp[l]|=1;
#endif
	}
	return;
}


int expchrtobmp(HWND hWnd, unsigned char *epp, DWORD x, DWORD y){
	HANDLE hFile;
	DWORD dwAccBytes;
	char str[MAX_PATH]="", str2[64];
    OPENFILENAME ofn;
	BITMAPINFOHEADER bmi;
	BITMAPFILEHEADER bmfi;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Bitmap Files(*.BMP)\0*.BMP\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = str;
    ofn.lpstrFileTitle = (char *)"";
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "BMP";
	LoadString(g_main_instance, IDS_STRING_CDLG_13 , str2, 64);
	ofn.lpstrTitle	=str2;
	ofn.lpstrInitialDir = NULL;
	if(GetSaveFileName(&ofn) == 0){
        return 0;
	}
	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
    	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return 0;
		}
    }
	bmi.biSize            = sizeof(BITMAPINFOHEADER);
	bmi.biWidth           = x*8; //8*80;
	bmi.biHeight          = y*8; //8*80*rhead[5];
	bmi.biPlanes          = 0x0001;
	bmi.biBitCount        = 0x0008;
	bmi.biCompression     = BI_RGB;
	bmi.biSizeImage       = x*8*y*8; //8*80 * 8*80 * rhead[5];
	bmi.biXPelsPerMeter   = 0x00000060;
	bmi.biYPelsPerMeter   = 0x00000060;
	bmi.biClrUsed         = 0x00000100;
	bmi.biClrImportant    = 0x00000100;

	bmfi.bfType = 0x4d42;
	bmfi.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 0x400+bmi.biSizeImage;
	bmfi.bfReserved1 = 0x0000;
	bmfi.bfReserved2 = 0x0000;
	bmfi.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 0x400;
	WriteFile(hFile, &bmfi, sizeof(BITMAPFILEHEADER), &dwAccBytes, NULL);
	WriteFile(hFile, &bmi, sizeof(BITMAPINFOHEADER), &dwAccBytes, NULL);
	{
    RGBQUAD Pal[256];
	ZeroMemory(Pal, sizeof(Pal));
	Pal[1].rgbRed=0x99;
	Pal[1].rgbGreen=0x99;
	Pal[1].rgbBlue=0x99;
	Pal[2].rgbRed=0xcc;
	Pal[2].rgbGreen=0xcc;
	Pal[2].rgbBlue=0xcc;
	Pal[3].rgbRed=0xff;
	Pal[3].rgbGreen=0xff;
	Pal[3].rgbBlue=0xff;
	WriteFile(hFile, Pal, sizeof(Pal), &dwAccBytes, NULL);
	}
	WriteFile(hFile, epp, (x*8)*(y*8), &dwAccBytes, NULL);
	CloseHandle(hFile);
	return 1;
}


int nnnEditChrRomFileWrite(char *fn, unsigned char *rp, DWORD adr){
	int i=-1;
	HANDLE hFile;
	DWORD dwAccBytes, dwAttr;

	dwAttr= GetFileAttributes(fn);
	if(dwAttr & FILE_ATTRIBUTE_READONLY){
		SetFileAttributes(fn, dwAttr^ FILE_ATTRIBUTE_READONLY);
	}
	hFile = CreateFile(fn, GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return 0;
    }
	if(-1 ==SetFilePointer(hFile, adr, 0, FILE_BEGIN)){
		CloseHandle(hFile);
		return 0;
	}
	WriteFile(hFile, rp, (CHRROMEDITYN*8)*(CHRROMEDITXN*8)/4, &dwAccBytes, NULL);
	CloseHandle(hFile);
	return 1;
}


void editpaterntoconv(unsigned char *rd, unsigned char *epp, unsigned char Mode, DWORD x, DWORD y, DWORD sx, DWORD sy){
	unsigned char *rp=rd;
	DWORD xn,yn,i;

	xn=CHRROMEDITXN/x, yn=CHRROMEDITYN/y;
	rp+= (xn*(x*y*16)*sy)+(sx*(x*y*16));
	if(Mode==0){
		for(i=0;i<(y*x);++i){
			nnn8x8characonv(&epp[(i%x*8)+(((y-1)-(i/x))*(x*8*8))], rp, x*8);
			rp+=16;
		}
	}
	else if(Mode==1){
		for(i=0;i<(y*x);++i){
			nnn8x8characonv(&epp[((((y-1)-i%y)*8*8*x)+(i/y)*8)], rp, x*8);
			rp+=16;
		}
	}
	else if(Mode==2){
		for(i=0;i<(y*x);++i){
			nnn8x8characonvmono(&epp[(i%x*8)+(((y-1)-(i/x))*(x*8*8))], rp, x*8);
			rp+=8;
		}
	}
	else if(Mode==3){
		for(i=0;i<(y*x);++i){
			nnn8x8characonvmono(&epp[((((y-1)-i%y)*8*8*x)+(i/y)*8)], rp, x*8);
			rp+=8;
		}
	}
	return;
}


int inpchrtobmp(HWND hWnd, unsigned char *epp, DWORD x, DWORD y){
	HANDLE hFile;
	DWORD dwAccBytes;
	char str[MAX_PATH]="", str2[64];
    OPENFILENAME ofn;
	BITMAPINFOHEADER bmi;
	BITMAPFILEHEADER bmfi;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Bitmap Files(*.BMP)\0*.BMP\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = str;
    ofn.lpstrFileTitle = (char *)"";
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "BMP";
	LoadString(g_main_instance, IDS_STRING_CDLG_13 , str2, 64);
	ofn.lpstrTitle	=str2;
	ofn.lpstrInitialDir = NULL;
	if(GetOpenFileName(&ofn) == 0){
        return 0;
	}
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return 0;
    }
	ReadFile(hFile, &bmfi, sizeof(BITMAPFILEHEADER), &dwAccBytes, NULL);
	if(dwAccBytes != sizeof(BITMAPFILEHEADER) || bmfi.bfType != 0x4d42){
		CloseHandle(hFile);
		return 0;
	}
	ReadFile(hFile, &bmi, sizeof(BITMAPINFOHEADER), &dwAccBytes, NULL);
	if(bmi.biBitCount != 8){
		CloseHandle(hFile);
		return 0;
	}
	SetFilePointer(hFile, bmfi.bfOffBits, 0, FILE_BEGIN);
	ReadFile(hFile, epp, x*8*y*8, &dwAccBytes, NULL);
	CloseHandle(hFile);
	return 1;
}


void nnnallcharadraw(HWND hWnd, HDC hdc, unsigned char *rd, unsigned char Mode, unsigned int x, unsigned int y){
	DWORD i,j,k,xn,yn;
	LPBITMAPINFO lpbi = NULL;
	unsigned char data[64*64], *rp;

	lpbi = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
	rp=rd;
	lpbi->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	lpbi->bmiHeader.biPlanes=1;
	lpbi->bmiHeader.biBitCount=8;
	lpbi->bmiHeader.biCompression=BI_RGB;
	lpbi->bmiHeader.biSizeImage=0;
	lpbi->bmiHeader.biXPelsPerMeter=0;
	lpbi->bmiHeader.biYPelsPerMeter=0;
	lpbi->bmiHeader.biClrUsed=0;
	lpbi->bmiHeader.biClrImportant=0;
//	bminfo.bmiColors=rgbq;
/*	for(i=0;i<24;++i){
		for(j=0;j<24;++j)
			if(j==23||j==0||i==0||i==23)
				data[i*24+j]=0;
			else
				data[i*24+j]=1;
	}
*/
//	ZeroMemory(rgbq, sizeof(rgbq));

	lpbi->bmiColors[0].rgbBlue=0;
	lpbi->bmiColors[0].rgbGreen=0;
	lpbi->bmiColors[0].rgbRed=0;
	lpbi->bmiColors[0].rgbReserved=0;
	lpbi->bmiColors[1].rgbBlue=0;
	lpbi->bmiColors[1].rgbGreen=0x77;
	lpbi->bmiColors[1].rgbRed=0x77;
	lpbi->bmiColors[1].rgbReserved=0x77;

//	ZeroMemory(bminfo.bmiColors, sizeof(rgbq));
	lpbi->bmiColors[2].rgbBlue=0xcc;
	lpbi->bmiColors[2].rgbGreen=0xcc;
	lpbi->bmiColors[2].rgbRed=0xcc;
	lpbi->bmiColors[2].rgbReserved=0;
	lpbi->bmiColors[3].rgbBlue=0xff;
	lpbi->bmiColors[3].rgbGreen=0xff;
	lpbi->bmiColors[3].rgbRed=0xff;
	lpbi->bmiColors[3].rgbReserved=0;

//	memset(&bminfo.bmiColors[0x2], 0xff, sizeof(RGBQUAD)*0xe0);

	lpbi->bmiHeader.biWidth = x*8;
	lpbi->bmiHeader.biHeight = y*8;
	xn=CHRROMEDITXN/x, yn=CHRROMEDITYN/y;
	if(Mode==0){
		for(i=0;i<yn;++i){
			for(j=0;j<xn;++j){
				for(k=0;k<(y*x);++k){
					nnn8x8charadraw(&data[(k%x*8)+(((y-1)-(k/x))*(x*8*8))], rp, x*8);
					rp+=16;
				}
				SetDIBitsToDevice(hdc,x*8*j+CHRROMEDITSTX,y*8*i+CHRROMEDITSTY, x*8, y*8, 0,0, 0, y*8, &data, lpbi, DIB_RGB_COLORS);
			}
		}
	}
	else if(Mode==1){
		for(i=0;i<yn;++i){
			for(j=0;j<xn;++j){
				for(k=0;k<(y*x);++k){
					nnn8x8charadraw(&data[((((y-1)-k%y)*8*8*x)+(k/y)*8)], rp, x*8);
					rp+=16;
				}
				SetDIBitsToDevice(hdc,x*8*j+CHRROMEDITSTX,y*8*i+CHRROMEDITSTY, x*8, y*8, 0,0, 0, y*8, &data, lpbi, DIB_RGB_COLORS);
			}
		}
	}
	else if(Mode==2){
		lpbi->bmiColors[1].rgbBlue=0xff;
		lpbi->bmiColors[1].rgbGreen=0xff;
		lpbi->bmiColors[1].rgbRed=0xff;
		for(i=0;i<yn;++i){
			for(j=0;j<xn;++j){
				for(k=0;k<(y*x);++k){
					nnn8x8charadrawmono(&data[(k%x*8)+(((y-1)-(k/x))*(x*8*8))], rp, x*8);
					rp+=8;
				}
				SetDIBitsToDevice(hdc,x*8*j+CHRROMEDITSTX,y*8*i+CHRROMEDITSTY, x*8, y*8, 0,0, 0, y*8, &data, lpbi, DIB_RGB_COLORS);
			}
		}
	}
	else if(Mode==3){
		lpbi->bmiColors[1].rgbBlue=0xff;
		lpbi->bmiColors[1].rgbGreen=0xff;
		lpbi->bmiColors[1].rgbRed=0xff;
		for(i=0;i<yn;++i){
			for(j=0;j<xn;++j){
				for(k=0;k<(y*x);++k){
					nnn8x8charadrawmono(&data[((((y-1)-k%y)*8*8*x)+(k/y)*8)], rp, x*8);
					rp+=8;
				}
				SetDIBitsToDevice(hdc,x*8*j+CHRROMEDITSTX,y*8*i+CHRROMEDITSTY, x*8, y*8, 0,0, 0, y*8, &data, lpbi, DIB_RGB_COLORS);
			}
		}
	}
	if(lpbi)
		free(lpbi);
	return;
}


void nnnselectcharadraw(HWND hWnd, HDC hdc, unsigned char *rd, unsigned char *epp, unsigned char Mode, unsigned int x, unsigned int y, DWORD sx, DWORD sy){
	DWORD i,xn,yn;
	LPBITMAPINFO lpbi = NULL;
	unsigned char *rp;

	rp=rd;
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
//	bminfo.bmiColors=rgbq;
/*	for(i=0;i<24;++i){
		for(j=0;j<24;++j)
			if(j==23||j==0||i==0||i==23)
				data[i*24+j]=0;
			else
				data[i*24+j]=1;
	}
*/
//	ZeroMemory(rgbq, sizeof(rgbq));

	lpbi->bmiColors[0].rgbBlue=0;
	lpbi->bmiColors[0].rgbGreen=0;
	lpbi->bmiColors[0].rgbRed=0;
	lpbi->bmiColors[0].rgbReserved=0;
	lpbi->bmiColors[1].rgbBlue=0;
	lpbi->bmiColors[1].rgbGreen=0x77;
	lpbi->bmiColors[1].rgbRed=0x77;
	lpbi->bmiColors[1].rgbReserved=0x77;

//	ZeroMemory(bminfo.bmiColors, sizeof(rgbq));
	lpbi->bmiColors[2].rgbBlue=0xcc;
	lpbi->bmiColors[2].rgbGreen=0xcc;
	lpbi->bmiColors[2].rgbRed=0xcc;
	lpbi->bmiColors[2].rgbReserved=0;
	lpbi->bmiColors[3].rgbBlue=0xff;
	lpbi->bmiColors[3].rgbGreen=0xff;
	lpbi->bmiColors[3].rgbRed=0xff;
	lpbi->bmiColors[3].rgbReserved=0;

	lpbi->bmiHeader.biWidth = x*8;
	lpbi->bmiHeader.biHeight = y*8;
	xn=CHRROMEDITXN/x, yn=CHRROMEDITYN/y;
	if(Mode==0){
		rp+= (xn*(x*y*16)*sy)+(sx*(x*y*16));
		for(i=0;i<(y*x);++i){
			nnn8x8charadraw(&epp[(i%x*8)+(((y-1)-(i/x))*(x*8*8))], rp, x*8);
			rp+=16;
		}
	}
	else if(Mode==1){
		rp+= (xn*(x*y*16)*sy)+(sx*(x*y*16));
		for(i=0;i<(y*x);++i){
			nnn8x8charadraw(&epp[((((y-1)-i%y)*8*8*x)+(i/y)*8)], rp, x*8);
			rp+=16;
		}
	}
	else if(Mode==2){
		rp+= (xn*(x*y*8)*sy)+(sx*(x*y*8));
		lpbi->bmiColors[1].rgbBlue=0xff;
		lpbi->bmiColors[1].rgbGreen=0xff;
		lpbi->bmiColors[1].rgbRed=0xff;
		for(i=0;i<(y*x);++i){
			nnn8x8charadrawmono(&epp[(i%x*8)+(((y-1)-(i/x))*(x*8*8))], rp, x*8);
			rp+=8;
		}
	}
	else if(Mode==3){
		rp+= (xn*(x*y*8)*sy)+(sx*(x*y*8));
		lpbi->bmiColors[1].rgbBlue=0xff;
		lpbi->bmiColors[1].rgbGreen=0xff;
		lpbi->bmiColors[1].rgbRed=0xff;
		for(i=0;i<(y*x);++i){
			nnn8x8charadrawmono(&epp[((((y-1)-i%y)*8*8*x)+(i/y)*8)], rp, x*8);
			rp+=8;
		}
	}
	SetDIBitsToDevice(hdc, 300, 15, x*8, y*8, 0,0, 0, y*8, epp, lpbi, DIB_RGB_COLORS);
	if(lpbi)
		free(lpbi);
	return;
}


void nnnEditcharadraw(HWND hWnd, HDC hdc, unsigned char *epp, unsigned int x, unsigned int y){
	LPBITMAPINFO lpbi = NULL;

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
	lpbi->bmiColors[1].rgbBlue=0;
	lpbi->bmiColors[1].rgbGreen=0x77;
	lpbi->bmiColors[1].rgbRed=0x77;
	lpbi->bmiColors[1].rgbReserved=0x77;

	lpbi->bmiColors[2].rgbBlue=0xcc;
	lpbi->bmiColors[2].rgbGreen=0xcc;
	lpbi->bmiColors[2].rgbRed=0xcc;
	lpbi->bmiColors[2].rgbReserved=0;
	lpbi->bmiColors[3].rgbBlue=0xff;
	lpbi->bmiColors[3].rgbGreen=0xff;
	lpbi->bmiColors[3].rgbRed=0xff;
	lpbi->bmiColors[3].rgbReserved=0;

	lpbi->bmiHeader.biWidth = x*8;
	lpbi->bmiHeader.biHeight = y*8;
	SetDIBitsToDevice(hdc, 300, 15, x*8, y*8, 0,0, 0, y*8, epp, lpbi, DIB_RGB_COLORS);
	if(lpbi)
		free(lpbi);
	return;
}


int nnnEditChrRomInit(char *fn, DWORD *adr, unsigned char *header, DWORD *fs){
	int i=-1, flag=0;
	HANDLE hFile;
	DWORD dwAccBytes;
	char str1[MAX_PATH], str2[MAX_PATH], *extp;

	if((i=ListView_GetNextItem(phList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
		return 0;
	ListView_GetItemText(phList, i, LUNCHFILEN, str2, MAX_PATH);
	extp=PathFindExtension(str2);
	if(!MystrCmp(extp, ".nes") && !MystrCmp(extp, ".fds"))
		return 0;
	str1[0]=0;
	ListView_GetItemText(phList, i, LUNCHFOLDN, str1, MAX_PATH);
	PathCombine(str1 , str1, str2);
	strcpy(fn, str1);
	hFile = CreateFile(str1, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return 0;
    }
	ReadFile(hFile, header, 16, &dwAccBytes, NULL);
	if(16 != dwAccBytes){
		CloseHandle(hFile);
		return 0;
	}
	if(MystrCmp(extp, ".nes")){
		if(header[5]!=0){
			*adr=header[4]*16384+16;
		}
		else
		*adr=0x10;
	}
	else{
		*adr=0x10;
	}
	*fs = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return 1;
}


int nnnEditChrRomFileRead(char *fn, unsigned char *rp, DWORD adr){
	int i=-1, flag=0;
	HANDLE hFile;
	DWORD dwAccBytes;

/*	dwAttr= GetFileAttributes(str1);
	if(dwAttr & FILE_ATTRIBUTE_READONLY){
		SetFileAttributes(str1, dwAttr^ FILE_ATTRIBUTE_READONLY);
		flag=1;
	}
*/
	hFile = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return 0;
    }
	if(-1 ==SetFilePointer(hFile, adr, 0, FILE_BEGIN)){
		CloseHandle(hFile);
		return 0;
	}
	ZeroMemory(rp, (CHRROMEDITYN*8)*(CHRROMEDITXN*8)/4);
	ReadFile(hFile, rp, (CHRROMEDITYN*8)*(CHRROMEDITXN*8)/4, &dwAccBytes, NULL);
	CloseHandle(hFile);
	return 1;
}


void nnnEditCromSetAdrText(HWND hd, unsigned char *head, DWORD adr){
	char str[MAX_PATH];

	wsprintf(str, "%x", adr);
	SetWindowText(GetDlgItem(hd, IDC_EDIT1), str);
	if(adr < (DWORD)(head[4]*0x4000+0x10)){
		strcpy(str, "PROM");
		adr-=0x10;
	}
	else{
		strcpy(str, "CROM");
		adr-=head[4]*0x4000+0x10;
	}
	wsprintf(&str[4], "  %x", adr);
	SetWindowText(GetDlgItem(hd, IDC_STATIC1), str);
	return;
}



//IDD_NNNEDITCHRROM
LRESULT CALLBACK nnnEditCromDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp){
	PAINTSTRUCT ps;
	HDC hdc;
	static char rfn[MAX_PATH];
	static DWORD address, fsize, vsbpos, selectx=0, selecty=00;
	static unsigned char *rdata=NULL, *editpatern=NULL,header[16], charamode=0, xsize=2, ysize=2;
	char *combstr[]={"8","16","24", "32", "40", "48", "56", "64"};

	switch(msg){
		case WM_INITDIALOG:
			rdata = (unsigned char*)malloc((CHRROMEDITYN*8)*(CHRROMEDITXN*8)/4+1);
			editpatern=(unsigned char*)malloc(64*64);
			if(rdata==NULL || !nnnEditChrRomInit(rfn, &address, header, &fsize)){
				if(rdata)
					free(rdata);
				EndDialog(hDlg, FALSE);
				break;
			}
			if(!nnnEditChrRomFileRead(rfn, rdata, address)){
				free(rdata);
				EndDialog(hDlg, FALSE);
			}
			SetScrollRange(hDlg, SB_CTL, 0, (fsize-0x10)/(CHRROMEDITXN*ysize*4), 0);
			vsbpos=(address-0x10)/(CHRROMEDITXN*ysize*4);
			SetScrollPos(hDlg, SB_CTL, address, 0);
			nnnEditCromSetAdrText(hDlg, header, address);

			{
			for(int i=0; i<8; ++i){
				SendDlgItemMessage(hDlg,IDC_COMBO1,CB_INSERTSTRING,(WPARAM)i,(LPARAM)combstr[i]);
				SendDlgItemMessage(hDlg,IDC_COMBO2,CB_INSERTSTRING,(WPARAM)i,(LPARAM)combstr[i]);
			}
			}
			SendDlgItemMessage(hDlg,IDC_COMBO3,CB_INSERTSTRING,(WPARAM)0,(LPARAM)"01");
			SendDlgItemMessage(hDlg,IDC_COMBO3,CB_INSERTSTRING,(WPARAM)1,(LPARAM)"02");
			SendDlgItemMessage(hDlg,IDC_COMBO3,CB_INSERTSTRING,(WPARAM)2,(LPARAM)"MONO");
			SendDlgItemMessage(hDlg,IDC_COMBO3,CB_INSERTSTRING,(WPARAM)3,(LPARAM)"MONO2");
			SendDlgItemMessage(hDlg,IDC_COMBO1,CB_SETCURSEL, (WPARAM)xsize-1,0L);
			SendDlgItemMessage(hDlg,IDC_COMBO2,CB_SETCURSEL, (WPARAM)ysize-1,0L);
			SendDlgItemMessage(hDlg,IDC_COMBO3,CB_SETCURSEL, (WPARAM)charamode,0L);
			break;
		case WM_PAINT:
//			hdc = GetDC(hDlg);
			hdc = BeginPaint(hDlg, &ps);
			nnnallcharadraw(hDlg, hdc, rdata, charamode, xsize, ysize);
			nnnEditcharadraw(hDlg, hdc, editpatern, xsize, ysize);
//			nnnselectcharadraw(hDlg, hdc, rdata, editpatern, charamode, xsize, ysize, selectx, selecty);
//			ReleaseDC(hDlg, hdc);
			EndPaint(hDlg, &ps);
			break;
		case WM_VSCROLL:
			switch (LOWORD(wp)) {
                case SB_LINEUP:
					if(address<=(DWORD)(CHRROMEDITXN/xsize*(xsize*ysize*16)+16)){
						address = fsize-(CHRROMEDITXN*8*ysize*8/4);
					}
					else
						address -= (CHRROMEDITXN/xsize*(xsize*ysize*16));//((CHRROMEDITXN/xsize)*8*ysize*8);
                    break;
                case SB_LINEDOWN:
					address += (CHRROMEDITXN/xsize*(xsize*ysize*16));
					if(address >= fsize){
						address = 0x10;
					}
                    break;
/*                case SB_PAGEDOWN:
                    break;
                case SB_PAGEUP:
                    break;
*/			}
			hdc = GetDC(hDlg);
			if(nnnEditChrRomFileRead(rfn, rdata, address)){
				nnnallcharadraw(hDlg, hdc, rdata, charamode, xsize, ysize);
//				nnnselectcharadraw(hDlg, hdc, rdata, editpatern, charamode, xsize, ysize, selectx, selecty);
			}
			ReleaseDC(hDlg, hdc);
			nnnEditCromSetAdrText(hDlg, header, address);
			vsbpos=(address-0x10)/(CHRROMEDITXN*8*ysize*8);
			SetScrollPos(hDlg, SB_CTL, vsbpos, TRUE);
			break;
		case WM_RBUTTONDOWN:
			{
			int x,y,tsx,tsy;
			x=LOWORD(lp);
			y=HIWORD(lp);
			if(x>=CHRROMEDITSTX && x<CHRROMEDITXN/xsize*8*xsize+CHRROMEDITSTX && y>=CHRROMEDITSTY && y<CHRROMEDITYN/ysize*8*ysize+CHRROMEDITSTY){
				tsy = (y-CHRROMEDITSTY)/(ysize*8);
				tsx = (x-CHRROMEDITSTX)/(xsize*8);
				editpaterntoconv(rdata, editpatern, charamode, xsize, ysize, tsx, tsy);
				hdc = GetDC(hDlg);
				nnnallcharadraw(hDlg, hdc, rdata, charamode, xsize, ysize);
				ReleaseDC(hDlg, hdc);
			}
			}
			return TRUE;
			break;
		case WM_LBUTTONDOWN:
			{
			int x,y;
			x=LOWORD(lp);
			y=HIWORD(lp);
			if(x>=CHRROMEDITSTX && x<CHRROMEDITXN/xsize*8*xsize+CHRROMEDITSTX && y>=CHRROMEDITSTY && y<CHRROMEDITYN/ysize*8*ysize+CHRROMEDITSTY){
				selecty = (y-CHRROMEDITSTY)/(ysize*8);
				selectx = (x-CHRROMEDITSTX)/(xsize*8);
				hdc = GetDC(hDlg);
				nnnselectcharadraw(hDlg, hdc, rdata, editpatern, charamode, xsize, ysize, selectx, selecty);
				ReleaseDC(hDlg, hdc);
			}
			}
			return TRUE;
			break;
		case WM_COMMAND:
			if(HIWORD(wp)==CBN_SELCHANGE){
				switch(LOWORD(wp)){
					case IDC_COMBO1:
						xsize= SendDlgItemMessage(hDlg,IDC_COMBO1,CB_GETCURSEL,0L,0L)+1;
						selectx=0,selecty=0;
						InvalidateRect(hDlg, NULL, TRUE);
						break;
					case IDC_COMBO2:
						ysize= SendDlgItemMessage(hDlg,IDC_COMBO2,CB_GETCURSEL,0L,0L)+1;
						selectx=0,selecty=0;
						InvalidateRect(hDlg, NULL, TRUE);
						break;
					case IDC_COMBO3:
						charamode= (unsigned char)SendDlgItemMessage(hDlg,IDC_COMBO3,CB_GETCURSEL,0L,0L);
						selectx=0,selecty=0;
						InvalidateRect(hDlg, NULL, TRUE);
						break;
				}
				return TRUE;
			}
			switch (LOWORD(wp)){
				case IDCANCEL:
					free(rdata);
					free(editpatern);
					EndDialog(hDlg, FALSE);
					return TRUE;
				case IDC_BUTTON1:
					expchrtobmp(hDlg, editpatern, xsize, ysize);
					return TRUE;
				case IDC_BUTTON2:
					if(inpchrtobmp(hDlg, editpatern, xsize, ysize)){
//						editpaterntoconv(rdata, editpatern, charamode, xsize, ysize, selectx, selecty);
						hdc = GetDC(hDlg);
//						nnnallcharadraw(hDlg, hdc, rdata, charamode, xsize, ysize);
//						nnnselectcharadraw(hDlg, hdc, rdata, editpatern, charamode, xsize, ysize, selectx, selecty);
						nnnEditcharadraw(hDlg, hdc, editpatern, xsize, ysize);
						ReleaseDC(hDlg, hdc);
/*
#ifdef _NNNESTERJ_ENGLISH
						if(IDOK == MessageBox(hDlg,(LPCSTR)"May I rewrite and update ROM file?",(LPCSTR)"?",MB_OKCANCEL))
#else
						if(IDOK == MessageBox(hDlg,(LPCSTR)"ROMファイルを書き換えていいですか？",(LPCSTR)"確認",MB_OKCANCEL))
#endif
							nnnEditChrRomFileWrite(rfn, rdata, address);
*/					}
					return TRUE;
				case IDC_BUTTON3:	// <
					if(address>=fsize){
						address = fsize-(CHRROMEDITXN*8*ysize*8/4);
					}
					else
						--address;
					hdc = GetDC(hDlg);
					if(nnnEditChrRomFileRead(rfn, rdata, address)){
						nnnallcharadraw(hDlg, hdc, rdata, charamode, xsize, ysize);
//						nnnselectcharadraw(hDlg, hdc, rdata, editpatern, charamode, xsize, ysize, selectx, selecty);
					}
					ReleaseDC(hDlg, hdc);
					nnnEditCromSetAdrText(hDlg, header, address);
					return TRUE;
				case IDC_BUTTON4:	// >
					++address;
					if(address>=fsize){
						address = 0x10;
					}
					hdc = GetDC(hDlg);
					if(nnnEditChrRomFileRead(rfn, rdata, address)){
						nnnallcharadraw(hDlg, hdc, rdata, charamode, xsize, ysize);
//						nnnselectcharadraw(hDlg, hdc, rdata, editpatern, charamode, xsize, ysize, selectx, selecty);
					}
					ReleaseDC(hDlg, hdc);
					nnnEditCromSetAdrText(hDlg, header, address);
					return TRUE;
				case IDC_BUTTON5:	// <<
					if(address<=(DWORD)(CHRROMEDITXN*8*ysize*8/4+16)){
						address = fsize-(8*8/4);
					}
					else
						address -= (8*8/4);
					hdc = GetDC(hDlg);
					if(nnnEditChrRomFileRead(rfn, rdata, address)){
						nnnallcharadraw(hDlg, hdc, rdata, charamode, xsize, ysize);
//						nnnselectcharadraw(hDlg, hdc, rdata, editpatern, charamode, xsize, ysize, selectx, selecty);
					}
					ReleaseDC(hDlg, hdc);
					nnnEditCromSetAdrText(hDlg, header, address);
					return TRUE;
				case IDC_BUTTON6:	// >>
					address += (8*8/4);
					if(address >= fsize){
						address = 0x10;
					}
					hdc = GetDC(hDlg);
					if(nnnEditChrRomFileRead(rfn, rdata, address)){
						nnnallcharadraw(hDlg, hdc, rdata, charamode, xsize, ysize);
//						nnnselectcharadraw(hDlg, hdc, rdata, editpatern, charamode, xsize, ysize, selectx, selecty);
					}
					ReleaseDC(hDlg, hdc);
					nnnEditCromSetAdrText(hDlg, header, address);
					return TRUE;
				case IDC_BUTTON7:	// Write
					if(nnnEditChrRomFileWrite(rfn, rdata, address)){
						char str2[64], str3[64];
						LoadString(g_main_instance, IDS_STRING_LMSG_09 , str2, 64);
						LoadString(g_main_instance, IDS_STRING_LMSG_10 , str3, 64);
						MessageBox(hDlg,(LPCSTR)str3,(LPCSTR)str2,MB_OK);
					}
					break;
			}
	}
	return FALSE;
}


//IDD_ROMINFODIALOG
LRESULT CALLBACK nnnROMHeaderInfoDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

    switch(msg) {
		case WM_COMMAND:
            switch(LOWORD(wp)){
                case IDOK:
					EndDialog(hDlgWnd, IDOK);
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
        case WM_INITDIALOG:
			{
				char s[260];
				s[0] = 0;
				emu->GetROMInfoStr(s);
#if 0
				if(h[0x10]){
					wsprintf(s, "%u -> %u", (h[0x11] >> 4)|(h[0x12] & 0xF0),(h[6] >> 4)|(h[7] & 0xF0));
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC1), s);
					wsprintf(&s[10], "  ->  ");
					s[1]=0;
					if((h[6]&1) != (h[0x11]&1)){
						s[10] = (h[0x11]&1)?'1':'0';
						s[15] = (h[6]&1)?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC4), &s[10]);
					}
					else{
						s[0]= h[6]&1?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC4), s);
					}
					if((h[6]&2) != (h[0x11]&2)){
						s[10] = h[0x11]&2?'1':'0';
						s[15] = h[6]&2?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC5), &s[10]);
					}
					else{
						s[0]= h[6]&2?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC5), s);
					}
					if((h[6]&4) != (h[0x11]&4)){
						s[10] = h[0x11]&4?'1':'0';
						s[15] = h[6]&4?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC6), &s[10]);
					}
					else{
						s[0]= h[6]&4?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC6), s);
					}
					if((h[6]&8) != (h[0x11]&8)){
						s[10] = h[0x11]&8?'1':'0';
						s[15] = h[6]&8?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC7), &s[10]);
					}
					else{
						s[0]= h[6]&8?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC7), s);
					}
				}
				else{
					wsprintf(s, "%u", (h[6] >> 4)|(h[7] & 0xF0));
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC1), s);
					s[1]=0;
					s[0]= h[6]&1?'1':'0';
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC4), s);
					s[0]= h[6]&2?'1':'0';
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC5), s);
					s[0]= h[6]&4?'1':'0';
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC6), s);
					s[0]= h[6]&8?'1':'0';
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC7), s);
				}
				wsprintf(s, "%u", h[4]*16);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC2), s);
				wsprintf(s, "%u", h[5]*8);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC3), s);
				DWORD crc = emu->crc32();
				wsprintf(s, "%x", crc);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC9), s);
#endif
				SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT1), s);
			}
            return TRUE;
    }
    return FALSE;
}


#if 0
//IDD_ROMINFODIALOG
LRESULT CALLBACK EditLauncherListNameDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

    switch(msg) {
		case WM_COMMAND:
            switch(LOWORD(wp)){
                case IDOK:
					EndDialog(hDlgWnd, IDOK);
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
        case WM_INITDIALOG:
			{
				unsigned char h[0x13];
				char s[20];
				emu->GetROMHeaderInfo(h);
				if(h[0x10]){
					wsprintf(s, "%u -> %u", (h[0x11] >> 4)|(h[0x12] & 0xF0),(h[6] >> 4)|(h[7] & 0xF0));
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC1), s);
					wsprintf(&s[10], "  ->  ");
					s[1]=0;
					if((h[6]&1) != (h[0x11]&1)){
						s[10] = (h[0x11]&1)?'1':'0';
						s[15] = (h[6]&1)?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC4), &s[10]);
					}
					else{
						s[0]= h[6]&1?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC4), s);
					}
					if((h[6]&2) != (h[0x11]&2)){
						s[10] = h[0x11]&2?'1':'0';
						s[15] = h[6]&2?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC5), &s[10]);
					}
					else{
						s[0]= h[6]&2?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC5), s);
					}
					if((h[6]&4) != (h[0x11]&4)){
						s[10] = h[0x11]&4?'1':'0';
						s[15] = h[6]&4?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC6), &s[10]);
					}
					else{
						s[0]= h[6]&4?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC6), s);
					}
					if((h[6]&8) != (h[0x11]&8)){
						s[10] = h[0x11]&8?'1':'0';
						s[15] = h[6]&8?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC7), &s[10]);
					}
					else{
						s[0]= h[6]&8?'1':'0';
						SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC7), s);
					}
				}
				else{
					wsprintf(s, "%u", (h[6] >> 4)|(h[7] & 0xF0));
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC1), s);
					s[1]=0;
					s[0]= h[6]&1?'1':'0';
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC4), s);
					s[0]= h[6]&2?'1':'0';
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC5), s);
					s[0]= h[6]&4?'1':'0';
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC6), s);
					s[0]= h[6]&8?'1':'0';
					SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC7), s);
				}
				wsprintf(s, "%u", h[4]*16);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC2), s);
				wsprintf(s, "%u", h[5]*8);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC3), s);
			}
            return TRUE;
    }
    return FALSE;
}
#endif

extern BOOL GetMovieFileName(char *, char *, char *);

//IDD_MOVIERECDIALOG
LRESULT CALLBACK nnnMovieRecDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp){
	static char movie_filename[_MAX_PATH];

    switch(msg) {
		case WM_COMMAND:
            switch(LOWORD(wp)){
				case IDC_RADIO1:
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CHECK5), TRUE);
					break;
				case IDC_RADIO2:
				case IDC_RADIO3:
					EnableWindow(GetDlgItem(hDlgWnd, IDC_CHECK5), FALSE);
					break;
				case IDC_BUTTON3:
					if(GetMovieFileName(movie_filename, "mv5", "movie file (*.mv5)\0*.mv5\0All Files (*.*)\0*.*\0\0"))
						SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT1), movie_filename);
					break;
				case IDOK:
					{
						char tstr[256];
						int i,s_flag;
						FILE *fmovie;
						if(SendMessage(GetDlgItem(hDlgWnd, IDC_RADIO1), BM_GETCHECK, (WPARAM)TRUE, 0L))
							s_flag = 0;
						else if(SendMessage(GetDlgItem(hDlgWnd, IDC_RADIO2), BM_GETCHECK, (WPARAM)TRUE, 0L))
							s_flag = 1;
						else
							s_flag = 2;
						i = GetDlgItemText(hDlgWnd, IDC_EDIT1, (LPTSTR)movie_filename, 256);
						if(i==0)
							break;
						fmovie = fopen(movie_filename, "wb");
						if(NULL == fmovie)
							break;
						uint32 fsize = 0;
						fputc('M', fmovie);
						fputc('V', fmovie);
						fputc('5', fmovie);
						fputc(0x1A, fmovie);
//						fwrite(&fsize, 4, 1, fmovie);
						for(i = 0; i < 0x20-4; i++){
							fputc(0x00, fmovie);
						}
						strcpy(tstr, PROG_NAME);
						i=strlen(tstr);
						fputc(i, fmovie);
						if(i)
							fwrite(tstr, 1, i, fmovie);
						i = GetDlgItemText(hDlgWnd, IDC_EDIT2, (LPTSTR)tstr, 256);
						fputc(i, fmovie);
						if(i)
							fwrite(tstr, 1, i, fmovie);
						i = GetDlgItemText(hDlgWnd, IDC_EDIT3, (LPTSTR)tstr, 256);
						fputc(i, fmovie);
						if(i)
							fwrite(tstr, 1, i, fmovie);
						fseek(fmovie, 8, SEEK_SET);
						fputc(s_flag, fmovie);
						i=0;
						fputc(i, fmovie);
						if(SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L))
							i|=0x80;
						if(SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK2), BM_GETCHECK, (WPARAM)TRUE, 0L))
							i|=0x40;
						if(SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK3), BM_GETCHECK, (WPARAM)TRUE, 0L))
							i|=0x20;
						if(SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK4), BM_GETCHECK, (WPARAM)TRUE, 0L))
							i|=0x10;
						fputc(i, fmovie);
						fputc(2, fmovie);
						DWORD crc32 = emu->crc32();
						fwrite(&crc32, 1, 4, fmovie);
						fclose(fmovie);
						if(SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK5), BM_GETCHECK, (WPARAM)TRUE, 0L))
							i|=1;
//						if(s_flag)
//							emu->reset(0);
						emu->StartRecMovie(movie_filename, i);
					}
					EndDialog(hDlgWnd, IDOK);
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
			return TRUE;
        case WM_INITDIALOG:
//			strcpy(movie_filename, emu->getROMname());
//			strcat(movie_filename, ".mov");
			SendMessage(GetDlgItem(hDlgWnd, IDC_RADIO1), BM_SETCHECK, (WPARAM)1, 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK1), BM_SETCHECK, (WPARAM)1, 0L);
//			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT1), movie_filename);
			{
				char gt[256]="";
				emu->GetGameTitleName(gt);
				if(gt[0]){
					SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT2), gt);
				}
			}
//			if(emu->GetDiskSideNum())
//				SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK6), BM_SETCHECK, (WPARAM)1, 0L);
			return TRUE;
	}
	return FALSE;
}



void MoviePlayDlgInit(HWND hDlgWnd, char *movie_filename){
	SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT1), movie_filename);
	{
		FILE* fmovie = fopen(movie_filename, "rb");
		if(fmovie == NULL)
			return;
		unsigned char pflag, ctype=0;
		unsigned int hsize;
		DWORD hdw;
		my_stream_base *pstream = NULL;
		fread(&hdw, 4, 1, fmovie);

		if(hdw == 0x1A35564D){
			pstream = new my_file_stream(fmovie);
		}
		else {
			fclose(fmovie);
			unsigned char *p;
			const char *mvext[] = { "*.mv5", NULL};

			p = (unsigned char *)malloc(256);
			if(p==NULL)
				return;
			if(!UseUnzipLibExeLoad_P(movie_filename, p, 256, NULL, mvext)){
				free(p);
				return;
			}
			if(*(DWORD *)&p[0] != 0x1A35564D){
				free(p);
				return;
			}
			pstream = new my_mem_stream(p, 256);
		}
		int dat;
		{
			char s1[256];
			unsigned int i,n;
			pstream->set_pos(8);
			dat = pstream->get_ch();
			ctype= pstream->get_ch();
			pflag = pstream->get_ch();
			hsize = (unsigned int)pstream->get_ch();
			hsize <<=4;
			pstream->read((unsigned char *)&i, 4);
			pstream->read((unsigned char *)&n, 4);
			wsprintf(s1, "%X", i);
			if(i !=	emu->crc32()){
				char str2[64], str3[64];
				LoadString(g_main_instance, IDS_STRING_MSGB_11 , str2, 64);
				LoadString(g_main_instance, IDS_STRING_MSGB_12 , str3, 64);
				MessageBox(hDlgWnd,(LPCSTR)str3,(LPCSTR)str2 ,MB_OK);
			}
			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT5), s1);
			wsprintf(s1, "%X", n);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT6), s1);
			{
				pstream->set_pos(hsize);
				i=pstream->get_ch();
				s1[i]=0;
				if(i)
					pstream->read((unsigned char *)s1, i);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT4), s1);
				i=pstream->get_ch();
				s1[i]=0;
				if(i)
					pstream->read((unsigned char *)s1, i);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT2), s1);
				i=pstream->get_ch();
				s1[i]=0;
				if(i)
					pstream->read((unsigned char *)s1, i);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT3), s1);
			}
		}
		EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC1), FALSE);
		EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC2), FALSE);
		EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC7), FALSE);
		EnableWindow(GetDlgItem(hDlgWnd, IDC_CHECK1), FALSE);
		if(dat==0)
			EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC1), TRUE);
		else if(dat==1)
			EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC2), TRUE);
		else{
			EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC7), TRUE);
			EnableWindow(GetDlgItem(hDlgWnd, IDC_CHECK1), TRUE);
		}
		//			if(ctype)
		EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC3), (pflag&0x80));
		EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC4), (pflag&0x40));
		EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC5), (pflag&0x20));
		EnableWindow(GetDlgItem(hDlgWnd, IDC_STATIC6), (pflag&0x10));
		//			fclose(fmovie);
		if(pstream)
			delete pstream;
	}
}



//IDD_MOVIEPLAYDIALOG
LRESULT CALLBACK nnnMoviePlayDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp){
	static char movie_filename[MAX_PATH];
	char *dextn = "mv5", *fltn = "movie file (*.mv5,*.zip,*.fmv)\0*.mv5;*.zip;*.fmv\0All Files (*.*)\0*.*\0\0";

    switch(msg) {
		case WM_COMMAND:
            switch(LOWORD(wp)){
				case IDC_BUTTON3:
					if(GetMovieFileName(movie_filename, dextn, fltn)){
						SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT1), movie_filename);
						MoviePlayDlgInit(hDlgWnd, movie_filename);
					}
					break;
				case IDOK:
					{
						FILE* fmovie = fopen(movie_filename, "rb");
						if(fmovie == NULL)
							break;
						DWORD hdw;
						fread(&hdw, 4, 1, fmovie);
						fclose(fmovie);
						if(hdw == 0x1A35564D || hdw == 0x1A564D46 || (hdw&0x0000ffff) == 0x00004b50)
						{
/*
							int dat, hsize, i, ssize;
							fread(&ssize, 4, 1, fmovie);
							dat = fgetc(fmovie);
							if(dat&0x80 && ssize){
								fseek(fmovie, 0x0B, SEEK_SET);
								hsize = fgetc(fmovie);
								hsize <<= 4;
								fseek(fmovie, hsize, SEEK_SET);
								i=hsize;
								i+=fgetc(fmovie)+1;
								fseek(fmovie, i, SEEK_SET);
								i+=fgetc(fmovie)+1;
								fseek(fmovie, i, SEEK_SET);
								i+=fgetc(fmovie)+1;
								fseek(fmovie, i, SEEK_SET);
								FILE* fstate = fopen(state_filename, "wb");
								unsigned char *p = (unsigned char *)malloc(ssize+4);
								if(p!=NULL){
									fread(p, ssize, 1, fmovie);
									fwrite(p, ssize, 1, fstate);
									free(p);
									fclose(fstate);
									emu->loadState(state_filename);
									remove(state_filename);
								}
							}
							else{
								emu->reset(0);
							}
							fclose(fmovie);
*/
							unsigned char l_flag = (unsigned char)SendMessage(GetDlgItem(hDlgWnd, IDC_CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L);
							emu->StartPlayMovie(movie_filename, l_flag);
						}
					}
					EndDialog(hDlgWnd, IDOK);
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
			return TRUE;
        case WM_INITDIALOG:
			if(!GetMovieFileName(movie_filename, dextn, fltn)){
				EndDialog(hDlgWnd, IDCANCEL);
				return TRUE;
			}
			MoviePlayDlgInit(hDlgWnd, movie_filename);
			return TRUE;
	}
	return FALSE;
}



struct MenuOD Menustruct[]={
	{ ID_FILE_OPEN_ROM, IDI_ICON1, NULL, NULL},
	{ ID_FILE_CLOSE_ROM, IDI_ICON2, NULL, NULL},
	{ ID_FILE_SOFTRESET, IDI_ICON3, NULL, NULL},
	{ ID_FILE_RESET, IDI_ICON4, NULL, NULL},
	{ ID_FILE_SCREENSHOT, IDI_ICON5, NULL, NULL},
	{ ID_FILE_SNDREC, IDI_ICON6, IDI_ICON7, NULL},
	{ ID_FILE_LOAD_STATE, IDI_ICON8, NULL, NULL},
	{ ID_FILE_SAVE_STATE, IDI_ICON9, NULL, NULL},
	{ ID_FILE_EXIT, IDI_ICON10, NULL, NULL},

	{ ID_OPTIONS_PREFERENCES, IDI_ICON11, NULL, NULL},
	{ ID_OPTIONS_GRAPHICS, IDI_ICON12, NULL, NULL},
	{ ID_OPTIONS_SOUND, IDI_ICON13, NULL, NULL},
	{ ID_OPTIONS_CONTROLLERS, IDI_ICON14, NULL, NULL},
	{ ID_OPTIONS_PATHS, IDI_ICON15, NULL, NULL},
	{ IDM_NNNEDITPLT, IDI_ICON16, NULL, NULL},
	{ ID_OPTIONS_FULLSCREEN, IDI_ICON17, IDI_ICON18, NULL},
	{ IDM_WINDOWTOPMOST, IDI_ICON19, IDI_ICON20, NULL},
	{ ID_OPTIONS_INFOSB, IDI_ICON21, IDI_ICON22, NULL},
	{ ID_OPTIONS_PREVIEW, IDI_ICON41, IDI_ICON42, NULL},
	
	{ ID_CHEAP_CHEAT2, IDI_ICON23, IDI_ICON24, NULL},
	{ ID_CHEAP_CHEAT4, IDI_ICON25, NULL, NULL},
	{ ID_CHEAP_CHEAT5, IDI_ICON26, NULL, NULL},
	{ ID_CHEAP_CHEAT6, IDI_ICON27, NULL, NULL},
	{ ID_CHEAP_CHEAT9, IDI_ICON28, NULL, NULL},
//	{ ID_CHEAP_CHEATVW1, IDI_ICON29, NULL, NULL},		//
	{ ID_CHEAP_CHEAT8, IDI_ICON30, NULL, NULL},
	{ ID_CHEAP_CHEATKO, IDI_ICON31, NULL, NULL},
	{ ID_CHEAP_CHEAT7, IDI_ICON32, IDI_ICON33, NULL},
	{ ID_CHEATFPS, IDI_ICON34, IDI_ICON35, NULL},
	{ ID_CHEAP_CHEATFONT, IDI_ICON36, NULL, NULL},
	{ IDM_AUTOSTATESL, IDI_ICON37, IDI_ICON38, NULL},

	{ ID_ROMINFODISPLAY, IDI_ICON39, NULL, NULL},
	{ ID_HELP_ABOUT, IDI_ICON40, NULL, NULL},
	{ NULL, NULL, NULL, NULL}
	/*
*/
};
extern HFONT MenuStrFont;

void SetMenuOwnerDraw(HWND hWnd){
#if 0
	HMENU hMenu;
	int i;
	MenuStrFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hMenu = GetMenu(hWnd);

	for(i=0;Menustruct[i].MenuID;++i){
		GetMenuString(hMenu, Menustruct[i].MenuID, Menustruct[i].MenuStr, 64, MF_BYCOMMAND);
		ModifyMenu(hMenu, Menustruct[i].MenuID, MF_BYCOMMAND | MF_OWNERDRAW, Menustruct[i].MenuID, Menustruct[i].MenuStr);
	}
#else
	HMENU hMenu, hMenu2/*, hMenu3*/;
	int i;
	MenuStrFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hMenu = GetMenu(hWnd);
	hMenu2 = GetSubMenu(hMenu,0);
	for(i=0;i<9;++i){
		GetMenuString(hMenu2, Menustruct[i].MenuID, Menustruct[i].MenuStr, 64, MF_BYCOMMAND);
		ModifyMenu(hMenu2, Menustruct[i].MenuID, MF_BYCOMMAND | MF_OWNERDRAW, Menustruct[i].MenuID, Menustruct[i].MenuStr);
	}
	hMenu2 = GetSubMenu(hMenu,1);
	for(;i<19;++i){
		GetMenuString(hMenu2, Menustruct[i].MenuID, Menustruct[i].MenuStr, 64, MF_BYCOMMAND);
		ModifyMenu(hMenu2, Menustruct[i].MenuID, MF_BYCOMMAND | MF_OWNERDRAW, Menustruct[i].MenuID, Menustruct[i].MenuStr);
	}
	hMenu2 = GetSubMenu(hMenu,2);
	for(;i<31;++i){
		GetMenuString(hMenu2, Menustruct[i].MenuID, Menustruct[i].MenuStr, 64, MF_BYCOMMAND);
		ModifyMenu(hMenu2, Menustruct[i].MenuID, MF_BYCOMMAND | MF_OWNERDRAW, Menustruct[i].MenuID, Menustruct[i].MenuStr);
	}
	hMenu2 = GetSubMenu(hMenu,3);
	for(;i<33;++i){
		GetMenuString(hMenu2, Menustruct[i].MenuID, Menustruct[i].MenuStr, 64, MF_BYCOMMAND);
		ModifyMenu(hMenu2, Menustruct[i].MenuID, MF_BYCOMMAND | MF_OWNERDRAW, Menustruct[i].MenuID, Menustruct[i].MenuStr);
	}
#endif
}


BOOL CALLBACK DrawIconProc(HDC hDC, LPARAM lParam, WPARAM wParam, int cx, int cy)
{
	DrawIconEx(hDC,	0, 0, (HICON)lParam, cx, cy, 0, NULL, DI_NORMAL);
	return	TRUE;
}


void DrawMenuOwnerDraw(DRAWITEMSTRUCT *dms){
	HDC hDC;
	HBRUSH hBrBkgnd;
	HICON hIcon;
	RECT rc;
	int i;

	hDC = dms->hDC;

	for(i=0;Menustruct[i].MenuID;++i){
		if(Menustruct[i].MenuID == dms->itemID)
			break;
	}
	if(!Menustruct[i].MenuID)
		return;

	hBrBkgnd = CreateSolidBrush(GetSysColor(COLOR_MENU));
	SetRect(&rc, dms->rcItem.left/*+GetSystemMetrics(SM_CXMENUCHECK)*/,dms->rcItem.top, dms->rcItem.right, dms->rcItem.bottom);
	FillRect(hDC, &rc, hBrBkgnd);
    DeleteObject(hBrBkgnd);

	if(Menustruct[i].IconID){
		if(dms->itemState & ODS_SELECTED){
			HPEN hPen = CreatePen(PS_SOLID, 1, 0xffffff);
			HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);
			MoveToEx(hDC, dms->rcItem.left, dms->rcItem.top+17, NULL);
			LineTo(hDC,  dms->rcItem.left, dms->rcItem.top);
			LineTo(hDC,  dms->rcItem.left+17, dms->rcItem.top);
			SelectObject(hDC, hOldPen);
			DeleteObject(hPen);
			hPen = CreatePen(PS_SOLID, 1, 0);
			hOldPen = (HPEN)SelectObject(hDC, hPen);
			LineTo(hDC,  dms->rcItem.left+17, dms->rcItem.top+17);
			LineTo(hDC,  dms->rcItem.left, dms->rcItem.top+17);
			DeleteObject(hPen);
		}
		if((dms->itemState & ODS_CHECKED)&&Menustruct[i].cIconID){
			hIcon= LoadIcon(g_main_instance, MAKEINTRESOURCE(Menustruct[i].cIconID));
		}
		else
			hIcon= LoadIcon(g_main_instance, MAKEINTRESOURCE(Menustruct[i].IconID));
		if(dms->itemState & ODS_GRAYED){
			DrawState(hDC, NULL, &DrawIconProc, (LPARAM)hIcon, 0, dms->rcItem.left+1, dms->rcItem.top+1, 16, 16, DST_COMPLEX | DSS_DISABLED);
		}
		else
			DrawIconEx(hDC, dms->rcItem.left+1,dms->rcItem.top+1, hIcon, 16,16, 0, NULL, DI_NORMAL);
		DestroyIcon(hIcon);
	}
	{ //dms->itemState & ODS_GRAYED
		hBrBkgnd = CreateSolidBrush(GetSysColor((dms->itemState & ODS_SELECTED)?COLOR_HIGHLIGHT : COLOR_MENU));
		SetRect(&rc, dms->rcItem.left+19/*+GetSystemMetrics(SM_CXMENUCHECK)*/,dms->rcItem.top, dms->rcItem.right, dms->rcItem.bottom);
		FillRect(hDC, &rc, hBrBkgnd);
        DeleteObject(hBrBkgnd);
		SetTextColor(dms->hDC, GetSysColor((dms->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));
		SetBkMode(dms->hDC, TRANSPARENT);
		HFONT OldFont = (HFONT)SelectObject(hDC, MenuStrFont);
		if(dms->itemState & ODS_GRAYED){
			DrawState(hDC, NULL, NULL,(LPARAM)Menustruct[i].MenuStr, strlen(Menustruct[i].MenuStr), rc.left, rc.top, 0, 0, DST_PREFIXTEXT/*DST_TEXT*/| DSS_DISABLED);
		}
		else{
			DrawText(hDC, Menustruct[i].MenuStr, strlen(Menustruct[i].MenuStr), &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		SelectObject(hDC, OldFont);
	}
}




// IDD_GGENIEDIALOG
LRESULT CALLBACK GameGenieEditDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp){


    switch(msg) {
        case WM_INITDIALOG:
			{
				unsigned char *fp=NULL;
				char fn[MAX_PATH];
				HANDLE hFile;
				DWORD dwAccBytes;
				if(NESTER_settings.path.UseGeniePath){
					strcpy(fn, NESTER_settings.path.szGameGeniePath);
				}
				else{
					GetModuleFileName(NULL, fn, MAX_PATH);
					PathCombine(fn, fn, "genie");
				}
				PathCombine(fn,fn, emu->getROMname());
				strcat(fn, ".gen");
				hFile = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if(hFile == INVALID_HANDLE_VALUE)
					break;
				int fsize= GetFileSize(hFile, NULL);
				fp = (unsigned char *)malloc(fsize+1);
				if(fp==NULL){
					CloseHandle(hFile);
					return TRUE;
				}
				ReadFile(hFile, fp, fsize, &dwAccBytes, NULL);
				CloseHandle(hFile);
				fp[fsize]=0;
				SetDlgItemText(hDlg, IDC_EDIT1, (LPTSTR)fp);
			}
			return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
					return TRUE;
					break;
				case IDC_BUTTON1:
					{
						unsigned char *fp=NULL;
						char fn[MAX_PATH];
						HANDLE hFile;
						DWORD dwAccBytes;
						if(NESTER_settings.path.UseGeniePath){
							strcpy(fn, NESTER_settings.path.szGameGeniePath);
						}
						else{
							GetModuleFileName(NULL, fn, MAX_PATH);
							PathCombine(fn, fn, "genie");
						}
						PathCombine(fn,fn, emu->getROMname());
						strcat(fn, ".gen");
						hFile = CreateFile(fn, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if(hFile == INVALID_HANDLE_VALUE){
							hFile = CreateFile(fn, GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
						}
						if(hFile != INVALID_HANDLE_VALUE){
							int size=GetWindowTextLength(GetDlgItem(hDlg, IDC_EDIT1));
							fp = (unsigned char *)malloc(size+1);
							if(fp){
								GetDlgItemText(hDlg, IDC_EDIT1, (LPTSTR)fp, size+1);
								WriteFile(hFile, fp, size+1, &dwAccBytes, NULL);
								free(fp);
							}
							CloseHandle(hFile);
						}
					}
					EndDialog(hDlg, IDOK);
					return TRUE;
					break;
			}
			return TRUE;
		}
		return FALSE;
}




int MyDatabaseFileOpenDlg(HWND hWnd, char *fn){
	OPENFILENAME ofn;
	char szFile[MAX_PATH]="";
	char FileName[MAX_PATH]="";
	char FoldName[MAX_PATH];
	char str[64];


	GetModuleFileName(NULL, FoldName, MAX_PATH);
	PathRemoveFileSpec(FoldName);

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Database files\0*.dat\0All files(*.*)\0*.*\0\0";
	ofn.lpstrFile = FileName;
	ofn.lpstrFileTitle = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = NULL;
 	LoadString(g_main_instance, IDS_STRING_CDLG_15 , str, 64);
	ofn.lpstrTitle = str;
	ofn.lpstrInitialDir = FoldName; //OpenFolder;
	if(GetOpenFileName(&ofn) == 0)
		return 0;
	strcpy(fn, FileName);
	return 1;
}


int MyGetColorSelectDialog(HWND hWnd, DWORD *Color){
	DWORD CustColors[16];
	CHOOSECOLOR cc;

	memset(&cc, 0, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hWnd;
	cc.lpCustColors = CustColors;
	cc.rgbResult = *Color;
	cc.Flags = CC_RGBINIT;
	if(ChooseColor(&cc)){
		*Color = cc.rgbResult;
		return 1;
	}
	return 0;
}



void PreviewCfgItemODrawItem(LPDRAWITEMSTRUCT lpDraw, DWORD *color){
	HBRUSH hBrushOld, hBrush;
	HDC hdc = lpDraw->hDC;
	int i=0;

	switch(lpDraw->CtlID){
		case IDC_BUTTON7:
			i = 0;
			break;
		case IDC_BUTTON8:
			i = 1;
			break;
		default:
			return;
	}

	hBrush = CreateSolidBrush(color[i]);
	hBrushOld = (HBRUSH)SelectObject(hdc, hBrush);
	Rectangle(hdc, lpDraw->rcItem.left, lpDraw->rcItem.top, lpDraw->rcItem.right, lpDraw->rcItem.bottom);
	SelectObject(hdc, hBrushOld);
	DeleteObject(hBrush);
}




//IDD_PREVIEW_CFGDIALOG
LRESULT CALLBACK Preview_ConfigDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	static DWORD dwColor[2];

    switch(msg) {
		case WM_DRAWITEM:
			PreviewCfgItemODrawItem((LPDRAWITEMSTRUCT)lp, dwColor);
			break;
		case WM_COMMAND:
            switch(LOWORD(wp)){
				case IDC_BUTTON3:
					{
						CHOOSEFONT cf;
						memset(&cf, 0, sizeof(CHOOSEFONT));
						cf.lStructSize = sizeof(CHOOSEFONT);
						cf.hwndOwner = hDlg;
						cf.lpLogFont = &g_Preview_State.LogFont;
						cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
						cf.nFontType = SCREEN_FONTTYPE;
						cf.rgbColors = g_Preview_State.FontColor;
						if(ChooseFont(&cf)) {
							if(g_Preview_State.hFont)
								DeleteObject(g_Preview_State.hFont);
							g_Preview_State.hFont = CreateFontIndirect(&g_Preview_State.LogFont);
							g_Preview_State.FontColor=cf.rgbColors;
							if(g_Preview_State.hFont!=NULL){
								ListView_SetTextColor(g_Preview_State.hListView, g_Preview_State.FontColor);
								SendMessage(g_Preview_State.hListView, WM_SETFONT, (WPARAM)g_Preview_State.hFont, MAKELPARAM(TRUE, 0));
								InvalidateRect(g_Preview_State.hListView, NULL, TRUE);
							}
						}
					}
					break;
				case IDC_BUTTON4:
					{
						char str[MAX_PATH];
						if(MyDatabaseFileOpenDlg(hDlg, str)){
							SetDlgItemText(hDlg, IDC_EDIT1, str);
						}
					}
					return TRUE;
					break;
				case IDC_BUTTON5:
					{
						char str[MAX_PATH];
						if(MyDatabaseFileOpenDlg(hDlg, str)){
							SetDlgItemText(hDlg, IDC_EDIT2, str);
						}
					}
					return TRUE;
					break;
				case IDC_BUTTON6:
					{
						char str[MAX_PATH];
						if(MyDatabaseFileOpenDlg(hDlg, str)){
							SetDlgItemText(hDlg, IDC_EDIT3, str);
						}
					}
					return TRUE;
					break;
				case IDC_BUTTON7:
					if(MyGetColorSelectDialog(hDlg, &dwColor[0]))
						InvalidateRect(hDlg, NULL, FALSE);
					break;
				case IDC_BUTTON8:
					if(MyGetColorSelectDialog(hDlg, &dwColor[1]))
						InvalidateRect(hDlg, NULL, FALSE);
					break;
                case IDOK:
					g_Preview_State.NotLoadNESJ_DB = !SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L);
					g_Preview_State.NotLoadNESE_DB = !SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_GETCHECK, (WPARAM)TRUE, 0L);
					g_Preview_State.NotLoadFDS_DB = !SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_GETCHECK, (WPARAM)TRUE, 0L);
					g_Preview_State.NotApplyNES_FixDB= SendMessage(GetDlgItem(hDlg, IDC_CHECK4), BM_GETCHECK, (WPARAM)TRUE, 0L);
					g_Preview_State.NotApplyFDS_FixDB= SendMessage(GetDlgItem(hDlg, IDC_CHECK5), BM_GETCHECK, (WPARAM)TRUE, 0L);
					g_Preview_State.NotLoadNES_AddDB= SendMessage(GetDlgItem(hDlg, IDC_CHECK6), BM_GETCHECK, (WPARAM)TRUE, 0L);
					g_Preview_State.NotLoadFDS_AddDB= SendMessage(GetDlgItem(hDlg, IDC_CHECK7), BM_GETCHECK, (WPARAM)TRUE, 0L);
					g_Preview_State.DisableSound= SendMessage(GetDlgItem(hDlg, IDC_CHECK8), BM_GETCHECK, (WPARAM)TRUE, 0L);
					g_Preview_State.EmuSpeed	= SendMessage(GetDlgItem(hDlg, IDC_CHECK9), BM_GETCHECK, (WPARAM)TRUE, 0L);
					g_Preview_State.NESROM_CRCFrom_Filesize = SendMessage(GetDlgItem(hDlg, IDC_CHECK10), BM_GETCHECK, (WPARAM)TRUE, 0L);
					g_Preview_State.ToolTipBGColor = dwColor[0];
					g_Preview_State.ToolTipTXTColor = dwColor[1];

					{
//						char str[MAX_PATH];
						GetDlgItemText(hDlg, IDC_EDIT1, (LPTSTR)g_Preview_State.DATABASE_FileName[0], MAX_PATH);
						GetDlgItemText(hDlg, IDC_EDIT2, (LPTSTR)g_Preview_State.DATABASE_FileName[1], MAX_PATH);
						GetDlgItemText(hDlg, IDC_EDIT3, (LPTSTR)g_Preview_State.DATABASE_FileName[2], MAX_PATH);
					}
					EndDialog(hDlg, IDOK);
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
					return TRUE;
			}
            break;
        case WM_INITDIALOG:
			SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK, (WPARAM)!g_Preview_State.NotLoadNESJ_DB, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_SETCHECK, (WPARAM)!g_Preview_State.NotLoadNESE_DB, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_SETCHECK, (WPARAM)!g_Preview_State.NotLoadFDS_DB, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_CHECK4), BM_SETCHECK, (WPARAM)g_Preview_State.NotApplyNES_FixDB, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_CHECK5), BM_SETCHECK, (WPARAM)g_Preview_State.NotApplyFDS_FixDB, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_CHECK6), BM_SETCHECK, (WPARAM)g_Preview_State.NotLoadNES_AddDB, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_CHECK7), BM_SETCHECK, (WPARAM)g_Preview_State.NotLoadFDS_AddDB, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_CHECK8), BM_SETCHECK, (WPARAM)g_Preview_State.DisableSound, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_CHECK9), BM_SETCHECK, (WPARAM)g_Preview_State.EmuSpeed, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_CHECK10), BM_SETCHECK, (WPARAM)g_Preview_State.NESROM_CRCFrom_Filesize, 0L);
			SetDlgItemText(hDlg, IDC_EDIT1, g_Preview_State.DATABASE_FileName[0]);
			SetDlgItemText(hDlg, IDC_EDIT2, g_Preview_State.DATABASE_FileName[1]);
			SetDlgItemText(hDlg, IDC_EDIT3, g_Preview_State.DATABASE_FileName[2]);
			dwColor[0] = g_Preview_State.ToolTipBGColor;
			dwColor[1] = g_Preview_State.ToolTipTXTColor;
            return TRUE;
    }
    return FALSE;
}


//IDD_VIEW_COLORNDIALOG
LRESULT CALLBACK MyMemViewColorDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	static POINT *Param=NULL;

    switch(msg) {
		case WM_COMMAND:
            switch(LOWORD(wp)){
                case IDOK:
					if(SendMessage(GetDlgItem(hDlg, IDC_RADIO3), BM_GETCHECK, (WPARAM)TRUE, 0L))
						Param->x = 1;
					else if(SendMessage(GetDlgItem(hDlg, IDC_RADIO4), BM_GETCHECK, (WPARAM)TRUE, 0L))
						Param->x = 2;
					else if(SendMessage(GetDlgItem(hDlg, IDC_RADIO5), BM_GETCHECK, (WPARAM)TRUE, 0L))
						Param->x = 3;
					else if(SendMessage(GetDlgItem(hDlg, IDC_RADIO6), BM_GETCHECK, (WPARAM)TRUE, 0L))
						Param->x = 4;
					else
						Param->x = 0;
					if(SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						DWORD Dec=0;
						char str[64];
						GetDlgItemText(hDlg, IDC_EDIT1, (LPTSTR)str, 64);
						Dec = atoi(str);
						Param->y = Dec;
					}
					else{
						DWORD Hex=0;
						char str[64];
						GetDlgItemText(hDlg, IDC_EDIT1, (LPTSTR)str, 64);
						MyStrAtoh(str, &Hex);
						Param->y = Hex;
					}
					EndDialog(hDlg, IDOK);
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
					return TRUE;
			}
            break;
        case WM_INITDIALOG:
			Param = (POINT *)lp;
			SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_SETCHECK, (WPARAM)1, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_RADIO3), BM_SETCHECK, (WPARAM)1, 0L);
            return TRUE;
    }
    return FALSE;
}



/*********/


void MyUnRegistShortCutKey(HWND hWnd){
	int i;

	for(i=0; i<1; i++){
		if(g_Window_ShotrCutKey[i].Regist){
			UnregisterHotKey(hWnd, i);
			g_Window_ShotrCutKey[i].Regist = 0;
		}
	}

}


void MyRegistShortCutKey(HWND hWnd){
	int i;
	unsigned int Mod, tMod, vk;

	for(i=0; i<1; i++){
		if(g_Window_ShotrCutKey[i].Active && g_Window_ShotrCutKey[i].KeyConfig){
			Mod = 0;
			tMod = g_Window_ShotrCutKey[i].KeyConfig >> 8;
			vk = g_Window_ShotrCutKey[i].KeyConfig & 0xff;
			if(tMod & HOTKEYF_ALT)
				Mod = MOD_ALT;
			if(tMod & HOTKEYF_CONTROL)
				Mod |= MOD_CONTROL;
			if(tMod & HOTKEYF_SHIFT)
				Mod |= MOD_SHIFT;
			g_Window_ShotrCutKey[i].Regist = (int)RegisterHotKey(hWnd, i, Mod, vk);
		}
	}
}



// 
LRESULT CALLBACK MyShortCutKeyDlg(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){

	switch(msg){
	case WM_INITDIALOG:
		{
//			int i, id;
//			SetFocus(GetDlgItem(hWnd, IDC_BUTTON1));
			SendMessage(GetDlgItem(hWnd, IDC_CHECK1), BM_SETCHECK, (WPARAM)g_Window_ShotrCutKey[0].Active, 0L);

			SendMessage(GetDlgItem(hWnd, IDC_HOTKEY1), HKM_SETHOTKEY, (WPARAM)g_Window_ShotrCutKey[0].KeyConfig, 0);
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wp)){
			case IDOK:
				MyUnRegistShortCutKey(main_window_handle);
				g_Window_ShotrCutKey[0].KeyConfig = SendMessage(GetDlgItem(hWnd, IDC_HOTKEY1), HKM_GETHOTKEY, (WPARAM)0, 0);
				g_Window_ShotrCutKey[0].Active= (unsigned char)SendMessage(GetDlgItem(hWnd, IDC_CHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L);
				MyRegistShortCutKey(main_window_handle);
				SaveHoyKeyCfg();
				EndDialog(hWnd, IDOK);
				return TRUE;
			case IDCANCEL:
				EndDialog(hWnd, IDCANCEL);
				return TRUE;

			case IDC_BUTTON2:
				{
/*
					int i;
					int id = IDC_HOTKEY1;
					for(i=0; i<3; i++, id++){
						SendMessage(GetDlgItem(hWnd, id), HKM_SETHOTKEY, (WPARAM)Default_KeyConfig[i+18], 0);
					}
*/
				}
				break;
		}
//		SetFocus(GetDlgItem(hWnd, IDC_BUTTON1));
		return TRUE;
//	case WM_KEYDOWN:
//		SetFocus(GetDlgItem(hWnd, IDC_BUTTON1));
		break;
	}
    return FALSE;
}





/*******/


DWORD dwAdd_OnyoniPuu;
unsigned char ucDat_OnyoniPuu;


void OnyoniPuuDlgGetState(HWND hWnd){
	int i, id, tdw;
	
	id = IDC_CHECK1;
	tdw = 0;
	for(i=0; i<14; id++, i++, tdw<<=1){
		if(SendMessage(GetDlgItem(hWnd, id), BM_GETCHECK, (WPARAM)TRUE, 0L))
			tdw|=1;
	}
	dwAdd_OnyoniPuu = tdw;
	tdw = 0;
	for(i=0; i<8; id++, i++, tdw<<=1){
		if(SendMessage(GetDlgItem(hWnd, id), BM_GETCHECK, (WPARAM)TRUE, 0L))
			tdw|=1;
	}
	ucDat_OnyoniPuu = (unsigned char)tdw;
}


// 
LRESULT CALLBACK MyOnyoNiPuuDlg(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){

	switch(msg){
	case WM_INITDIALOG:
		{
			int i, id, tdw;
//			SetFocus(GetDlgItem(hWnd, IDC_BUTTON1));
			id = IDC_CHECK1;

			tdw = dwAdd_OnyoniPuu;
			for(i=0; i<14; id++, i++, tdw<<=1){
				SendMessage(GetDlgItem(hWnd, id), BM_SETCHECK, (WPARAM)(tdw&0x2000), 0L);
			}
			tdw = ucDat_OnyoniPuu;
			for(i=0; i<8; id++, i++, tdw<<=1){
				SendMessage(GetDlgItem(hWnd, id), BM_SETCHECK, (WPARAM)(tdw&0x80), 0L);
			}

		}
		return TRUE;
	case WM_COMMAND:
		if(LOWORD(wp) >= IDC_CHECK1 && LOWORD(wp) <= IDC_CHECK22){
			OnyoniPuuDlgGetState(hWnd);
			break;
		}
		switch(LOWORD(wp)){
			case IDCANCEL:
				EndDialog(hWnd, IDCANCEL);
				return TRUE;

			case IDC_DEFAULTS:
				{
					int i;
					int id = IDC_CHECK1;
					for(i=0; i<22; i++, id++){
						SendMessage(GetDlgItem(hWnd, id), BM_SETCHECK, (WPARAM)0, 0L);
					}
					OnyoniPuuDlgGetState(hWnd);
				}
				break;
		}
//		SetFocus(GetDlgItem(hWnd, IDC_BUTTON1));
		return TRUE;
//	case WM_KEYDOWN:
//		SetFocus(GetDlgItem(hWnd, IDC_BUTTON1));
		break;
	case WM_DESTROY:
		g_extra_window.hOnyoniDlg = NULL;
		break;
	}
    return FALSE;
}


// IDD_DIALOG5
LRESULT CALLBACK PreviewSearchStringDlg(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	static char *pret;

	switch(msg){
	case WM_INITDIALOG:
		pret = (char *)lp;
		SetDlgItemText(hWnd, IDC_EDIT1, (LPTSTR)pret);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wp)){
			case IDOK:
				GetDlgItemText(hWnd, IDC_EDIT1, (LPTSTR)pret, 260);
				EndDialog(hWnd, IDOK);
				return TRUE;
			case IDCANCEL:
				EndDialog(hWnd, IDCANCEL);
				return TRUE;
		}
		return TRUE;
	}
    return FALSE;
}
