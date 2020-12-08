

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <stddef.h>

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
#include "CheckFile.h"

#define LAUNCHER_NESROMCHECK_NRHEAD

extern HWND main_window_handle;
extern HINSTANCE g_main_instance;

extern char previewfn[MAX_PATH];
extern DWORD plistn;
extern unsigned char previewflag;
extern HWND previewh;
extern char LauncherListName[10][64];
extern int SortItemp[LUNCHCOLMAX];
extern HWND phList;
extern unsigned char ListNo;

extern win32_screen_mgr* gscr_mgr;
extern emulator* emu;
extern int PrevFastSw;

extern DWORD MyCheatAdr, MyCheatNum;
extern unsigned char *SResulttext, *SResulttext2, *SResulttext3;

/////////////  Launcher  ////////

char previewfn[MAX_PATH];		//ROM filename send to main window.
DWORD plistn;					//Launcher List Item num
unsigned char previewflag=0;	//Launcher Display Flag
HWND previewh;					//Launcher Window Handle

int SortItemp[LUNCHCOLMAX];		//Use Sort
HWND phList;					//Launcher Listview Handle
unsigned char ListNo;			//Current List No
char LauncherListName[10][64];	//List Name Strings

//Launchar List Font
LOGFONT launcherFont;
DWORD launcherFontColor=0;				//Launchar List Font Color
DWORD launcherBackColor=0x0ffffff;		//Launchar List BG Color

kailleraInfos kInfos={ "", "", kGameCallback, kchatReceived, kclientDropped, kmoreInfos};

int prevfoldan;						//Launcher Folder All num
char prevfoldsf[MAX_PATH];			//Launcher Add Folder Dialog PreSelect Folder
char prevfoldn[50][MAX_PATH];		//Launcher Folder
int prevListWidth[LUNCHCOLMAX]={80, 45, 35, 45, 45, 40, 60, 60, 60, 45, 40, 60, 45, 45, 60, 50, 50, 50,50};
int prevListArray[LUNCHCOLMAX];
int prevWindpos[4];
char prevconfig[16];
char PreOpenFolder[MAX_PATH]="";

int nnnKailleraFlag=0;
int nnnKailleraplayer;
int nnnKailleraDlgFlag=0;

struct LauncherText *g_pLauncherText;
int g_manLauncherText = 0;
int g_nLauncherText = 0;

#define LNCHWTITLE "[%u] NNN Launcher"



void SetLauncherListName(){
  HMENU hMainMenu, hFileMenu;
  int i;
  char mstr[70];
  hMainMenu = GetMenu(previewh);
  if(hMainMenu == NULL)
	  return;
  hFileMenu = GetSubMenu(hMainMenu, 0);
  while(GetMenuItemCount(hFileMenu)){
	DeleteMenu(hFileMenu , 0, MF_BYPOSITION);
  }
  for(i = 0; i < 9; i++){
	  sprintf(mstr, "[List %i] ", i+1);
	  if(LauncherListName[i][0]){
		  strcat(mstr, ". ");
		  strcat(mstr, LauncherListName[i]);
	  }
	  AppendMenu(hFileMenu , MF_STRING, IDM_PREVLISTCHGD1+i, mstr);
  }
  DrawMenuBar(previewh);
}



void LauncherROMInfoCopyC(HWND hWnd){
	int i=-1;
	char str[MAX_PATH];
	HGLOBAL hMem;
	LPTSTR lpMem;

	if((i=ListView_GetNextItem(phList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
		return;
	hMem=GlobalAlloc(GHND, MAX_PATH*20);
	if(hMem==NULL)
		return;
	lpMem=(char *)GlobalLock(hMem);
	lpMem[0]=0;

	for(int j=0; j < LUNCHCOLMAX ; ++j){
		str[0]=0;
		ListView_GetItemText(phList, i, j, str, MAX_PATH);
		if(str[0]){
			strcat(lpMem, str);
			strcat(lpMem, ", ");
		}
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


void LauncherROMFileCopy(HWND hWnd){
	int i=-1;
	char str[MAX_PATH], str2[MAX_PATH], Folder[MAX_PATH];

	if((i=ListView_GetNextItem(phList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
		return;
	if(!MyFoldOpenDlg(hWnd, Folder))
		return;
	ListView_GetItemText(phList, i, LUNCHFOLDN, str, MAX_PATH);
	ListView_GetItemText(phList, i, LUNCHFILEN, str2, MAX_PATH);
	PathCombine(str, str, str2);
	PathCombine(Folder, Folder, str2);
	CopyFile(str, Folder, TRUE);
	return;
}

extern DWORD CheatDlgFontColor;
extern LOGFONT CheatDlgFont;
extern int MyStrAtoh(char *, DWORD *);



int MCPreview(HWND hWnd){
	static HWND hChdWnd;
	static int windf=0;

	if(windf){
		DestroyWindow(hChdWnd);
	}
	hChdWnd = CreateMyPwind(hWnd, MyPrevProc, "NNNJPreview", "NNN Launcher");
	ShowWindow(hChdWnd, SW_SHOW);
	UpdateWindow(hChdWnd);
	windf=1;
	return 0;
}


HWND CreateMyPwind(HWND hWnd, WNDPROC ChildProc,LPCTSTR szChildName, LPCTSTR title){
    WNDCLASSEX wc;
	static int flag=0;
	HWND hChild;
	RECT rc;
	HINSTANCE hInst;
	int i;

	hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	GetWindowRect(hWnd, &rc);

	if(flag==0){
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_VREDRAW | CS_HREDRAW;
		wc.lpfnWndProc = ChildProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.lpszMenuName = NULL; // MAKEINTRESOURCE(IDR_MENUPREV);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = LoadIcon(g_main_instance, MAKEINTRESOURCE(IDI_PWICON));
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszClassName = (LPCSTR)szChildName;
		wc.hIconSm      = NULL;
		if(RegisterClassEx(&wc) == 0) {
			return NULL;
		}
	}
	flag=1;
	i=PreviewFLoad();
	if(i==0 || (prevWindpos[2]==0 && prevWindpos[3]==0)){
		prevWindpos[0]= rc.right;
		prevWindpos[1]= rc.top;
		prevWindpos[2]= 300;
		prevWindpos[3]= 280;
	}
	if(gscr_mgr->is_fullscreen()){
		hChild = CreateWindowEx(NULL, szChildName, title, WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | 
		WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX| WS_MAXIMIZEBOX,
		rc.left+50, rc.top+100, 620, 280, hWnd, LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENUPREV)), hInst, NULL);
	}
	else{
		hChild = CreateWindowEx(NULL, szChildName, title, WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | 
			WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX| WS_MAXIMIZEBOX, // 
			prevWindpos[0], prevWindpos[1], prevWindpos[2], prevWindpos[3], NULL, LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENUPREV)), hInst, NULL);
	}
	return hChild;
}


void LauncherOnGetDispInfo(NMLVDISPINFO* pInfo){
	if(pInfo->item.iItem == -1)
		return;

	if(pInfo->item.mask & LVIF_TEXT){
		switch(pInfo->item.iSubItem){
			case 0:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].FileName;
				break;
			case 1:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].FolderName;
				break;
			case 2:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].Mapper;
				break;
			case 3:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].PromSize;
				break;
			case 4:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].CromSize;
				break;
			case 5:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].Info;
				break;
			case 6:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].AllCRC;
				break;
			case 7:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].PROMCRC;
				break;
			case 8:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].CROMCRC;
				break;
			case 9:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].Resault;
				break;
			case 10:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].Header;
				break;
			case 11:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].DBInfo;
				break;
			case 12:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].RomDbCmp;
				break;
			case 13:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].DBTitle;
				break;
			case 14:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].DBCountry;
				break;
			case 15:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].DBMaker;
				break;
			case 16:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].DBRelease;
				break;
			case 17:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].DBPrice;
				break;
			case 18:
				pInfo->item.pszText = g_pLauncherText[pInfo->item.iItem].DBGenre;
				break;
		}
	}
}


LRESULT CALLBACK MyPrevProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp){
	RECT rc;
	int i, j;
	static int sttimerf=0, Statusbvs=0;
	static HFONT hFont= NULL;
	static HINSTANCE hInst;
	static HWND StatusbWnd;
    DWORD dwStyle;
    LV_COLUMN lvcol;
	char str[MAX_PATH];
	LV_DISPINFO *lvinfo;
	NM_LISTVIEW *pNMLV;
	POINT pt;
	LV_HITTESTINFO hitpos;
	HMENU hMenu, hSubMenu;

	switch (msg) {
		case WM_SIZE:
			SendMessage(StatusbWnd, WM_SIZE, wp, lp);
			MoveWindow(phList, 0, 0, LOWORD(lp) , HIWORD(lp) - Statusbvs, TRUE);
			return 0;
			break;
#if 0
		case WM_PARENTNOTIFY:
			if(LOWORD(wp) != WM_LBUTTONDOWN)
				return 0;
			pt.x = hitpos.pt.x = LOWORD(lp);
			pt.y = hitpos.pt.y = HIWORD(lp);
			if(-1 !=(i=ListView_HitTest(phList, &hitpos))){
				if(ListView_GetItemState(phList, i, LVIS_SELECTED)){
					ListView_GetItemText(phList, i, LUNCHFOLDN, previewfn, MAX_PATH);
					ListView_GetItemText(phList, i, LUNCHFILEN, str, MAX_PATH);
					PathCombine(previewfn, previewfn, str);
					PostMessage(main_window_handle, WM_PREVIEW, 0, 0);
/*
					if(sttimerf){
						if(!KillTimer(hChild, ID_SBTIMER))
							return 0;
					}
					if(SetTimer(hChild, ID_SBTIMER, 150, NULL)){
						sttimerf=1;
					}
*/
				}
/*				str[0]='\0';
				ListView_GetItemText(phList, i, LUNCHDBNAME, str, MAX_PATH);
				if(str[0]== '\0'){
					SendMessage(StatusbWnd, SB_SETTEXT, 255, (LPARAM)(LPSTR)"");
					break;
				}
				strcat(str, ", ");
				for(j=0, k= LUNCHDBCOUN; k <=  LUNCHDBRSIZ; ++k){
					for(;j < MAX_PATH;++j){
						if(str[j] == '\0')
							break;
					}
					if(j==MAX_PATH)
						break;
					ListView_GetItemText(phList, i, k, &str[j], MAX_PATH-j);
					if(k!=  LUNCHDBRSIZ)
						strcat(str, ", ");
				}
				SendMessage(StatusbWnd, SB_SETTEXT, 255, (LPARAM)(LPSTR)str);*/
			}
			return 0;
			break;
#endif
		case WM_CONTEXTMENU:
//			if(emu)
//				break;
			pt.x = hitpos.pt.x = LOWORD(lp);
            pt.y = hitpos.pt.y = HIWORD(lp);
            if (wp == (WPARAM)phList) {
				ScreenToClient(phList, &hitpos.pt);
				if(-1 !=(i=ListView_HitTest(phList, &hitpos))){
					hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_POPMENUL1));
					hSubMenu = GetSubMenu(hMenu, 0);
					TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_TOPALIGN,
					    pt.x, pt.y, 0, hChild, NULL);
					DestroyMenu(hSubMenu);
					DestroyMenu(hMenu);
				}
            }
			return 0;
			break;
        case WM_COMMAND:
			switch (LOWORD(wp)){
				case IDM_PREV1:		//FOLDER
					if(IDCANCEL==DialogBox(hInst, MAKEINTRESOURCE(IDD_PREVFSDIALOG), hChild, (DLGPROC)MyCheatDlg13))
						break;
					PrevScanFold(hChild, phList);
					if(prevconfig[0]){
						RomInfoScan(hChild, phList);
						if(prevconfig[3]){
							RomCrcCheck(hChild);
							LauncherWindowTset(hChild);
						}
					}
					SetLauncherListName();
					break;
				case IDM_PREV2:		//STOP
					SendMessage(main_window_handle, WM_PREVIEWS, 0, 0);
					SendMessage(StatusbWnd, SB_SETTEXT, 255, (LPARAM)(LPSTR)"");
					break;
				case IDM_PREV3:		//List Updating
					PrevScanFold(hChild, phList);
					if(prevconfig[0]){
						RomInfoScan(hChild, phList);
						if(prevconfig[3]){
							RomCrcCheck(hChild);
						}
					}
					LauncherWindowTset(hChild);
					break;
				case IDM_PREV4:
					MyListVclr(LUNCHMAPP, LUNCHCOLMAX-1 - LUNCHMAPP);
					RomInfoScan(hChild, phList);
					break;
				case IDM_PREV5:
					MyListVclr(LUNCHDBMAPP, LUNCHCOLMAX-1 - LUNCHDBMAPP);
					RomInfoScan(hChild, phList);
					if(!RomCrcCheck(hChild)){
						char str[32], str2[32];
						LoadString(g_main_instance, IDS_STRING_LMSG_01 , str, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_02 , str2, 32);
						MessageBox(hChild,(LPCSTR)str2,(LPCSTR)str,MB_OK);
					}
					FdsCrcCheck(hChild);
					LauncherWindowTset(hChild);
					break;
				case IDM_PREV6:
					RomCrcCount(phList, &i, &j);
					{
						char str2[32], str3[32];
						LoadString(g_main_instance, IDS_STRING_LMSG_03 , str2, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_04 , str3, 32);
						wsprintf(str, str3, i, j);
						MessageBox(hChild,(LPCSTR)str,(LPCSTR)str2,MB_OK);
					}
					break;
				case IDM_PREV7:
					RomCrcTout(hChild, phList);
					break;
				case IDM_PREVD1:
					FdsCrcTout(hChild, phList);
					break;
				case IDM_PREV8:
					{
						char str2[32], str3[64];
						LoadString(g_main_instance, IDS_STRING_LMSG_05 , str2, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_06 , str3, 64);
						if(IDCANCEL== MessageBox(hChild,(LPCSTR)str3,(LPCSTR)str2,MB_OKCANCEL))
							break;
						i=ChengeRomHall(phList);
//					if(i){
						LoadString(g_main_instance, IDS_STRING_LMSG_03 , str2, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_07 , str3, 64);
						wsprintf(str, str3, i);
						MessageBox(hChild, (LPCSTR)str, (LPCSTR)str2, MB_OK);
//					}
					}
					break;
				case IDM_PREV9:
					{
						char str2[32], str3[64];
						LoadString(g_main_instance, IDS_STRING_LMSG_05 , str2, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_06 , str3, 64);
						if(IDCANCEL== MessageBox(hChild,(LPCSTR)str3,(LPCSTR)str2,MB_OKCANCEL))
							break;
						i=DbToRomIChange(phList);
						
						LoadString(g_main_instance, IDS_STRING_LMSG_03 , str2, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_07 , str3, 64);
						wsprintf(str, str3, i);
						MessageBox(hChild, (LPCSTR)str, (LPCSTR)str2, MB_OK);
					}
					RomInfoScan(hChild, phList);
					RomCrcCheck(hChild);
					LauncherWindowTset(hChild);
					break;
				case IDM_PREV10:
					{
						char str2[32], str3[64];
						LoadString(g_main_instance, IDS_STRING_LMSG_05 , str2, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_06 , str3, 64);
						if(IDCANCEL== MessageBox(hChild,(LPCSTR)str3,(LPCSTR)str2,MB_OKCANCEL))
							break;
						RomFileNameC(phList, &i, &j);
						LoadString(g_main_instance, IDS_STRING_LMSG_03 , str2, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_08 , str3, 64);
						wsprintf(str, str3, i);
						MessageBox(hChild, (LPCSTR)str, (LPCSTR)str2, MB_OK);
					}
					break;
				case IDM_PREV11:
					LunchDHideMode();
					break;
				case IDM_PREV12:
					LunchDDispMode();
					break;
				case IDM_PREV13:
					LunchDInitMode();
					break;
				case IDM_PREV14:
					MyListVclr(LUNCHDBMAPP, LUNCHCOLMAX-1- LUNCHDBMAPP);
					RomCrcCheck(hChild);
					LauncherWindowTset(hChild);
					break;
				case IDM_PREV15:
					SendMessage(main_window_handle, WM_PREVIEWFS, 0, 0);
					PrevFastSw^=1;
					if(PrevFastSw)
						CheckMenuItem(GetMenu(hChild), IDM_PREV15, MFS_CHECKED);
					else
						CheckMenuItem(GetMenu(hChild), IDM_PREV15, MFS_UNCHECKED);
					break;
				case IDM_PREV16:
					LunchWidthBuckup();
					break;
				case IDM_PREV17:
					LunchWidthRestore();
					break;
				case IDM_PREVNOREMOVE:
					LauncherListNoRemove(hChild);
					LauncherWindowTset(hChild);
					break;
				case IDM_PREVGOODROMMOVE:
					LauncherListGoodRomMove(hChild);
					LauncherWindowTset(hChild);
					break;
				case IDM_PREVNOROMMOVE:
					LauncherListNoRomMove(hChild);
					LauncherWindowTset(hChild);
					break;
				case IDM_PREVFILECHECK:
					LauncherListFileCheck(hChild);
					LauncherWindowTset(hChild);
					break;
				case IDM_PREVP1:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG18), hChild, (DLGPROC)MyCheatDlg23);
					break;
				case IDM_PREVCONFIG:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_PCONFIGDIALOG), hChild, (DLGPROC)MyCheatDlg15);
					break;
				case IDM_PREVCONFIGF:
					{
						CHOOSEFONT cf;
						memset(&cf, 0, sizeof(CHOOSEFONT));
						cf.lStructSize = sizeof(CHOOSEFONT);
						cf.hwndOwner = hChild;
						cf.lpLogFont = &launcherFont;
						cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
						cf.nFontType = SCREEN_FONTTYPE;
						cf.rgbColors = launcherFontColor;
						if(ChooseFont(&cf)) {
							if(hFont)
								DeleteObject(hFont);
							hFont = CreateFontIndirect(&launcherFont);
							launcherFontColor=cf.rgbColors;
							if(hFont!=NULL){
								ListView_SetTextColor(phList, launcherFontColor);
								SendMessage(phList, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
								InvalidateRect(phList, NULL, TRUE);
							}
						}
					}
					break;
				case IDM_PREVCONFIGBC:
					{
						DWORD CustColors[16];
						CHOOSECOLOR cc;
						memset(&cc, 0, sizeof(CHOOSECOLOR));
						cc.lStructSize = sizeof(CHOOSECOLOR);
						cc.hwndOwner = hChild;
						cc.lpCustColors = CustColors;
						cc.rgbResult = launcherBackColor;
						cc.Flags = CC_RGBINIT;
						if(ChooseColor(&cc)){
							launcherBackColor=cc.rgbResult;
							ListView_SetBkColor(phList,launcherBackColor);
							ListView_SetTextBkColor(phList,launcherBackColor);
							InvalidateRect(phList, NULL, TRUE);
						}
					}
					break;
				case IDM_CTPOPL1:
					if(ChengeRomH(phList)){
						char str2[32], str3[32];
						LoadString(g_main_instance, IDS_STRING_LMSG_09 , str2, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_10 , str3, 32);
						MessageBox(hChild,(LPCSTR)str3,(LPCSTR)str2,MB_OK);
					}
					break;
				case IDM_CTPOPL2:
					if(1==(i=ChengeRomoInfo(hInst, hChild))){
						char str2[32], str3[32];
						LoadString(g_main_instance, IDS_STRING_LMSG_09 , str2, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_10 , str3, 32);
						MessageBox(hChild,(LPCSTR)str3,(LPCSTR)str2,MB_OK);
					}
					else if(i==0){
						char str2[32], str3[32];
						LoadString(g_main_instance, IDS_STRING_LMSG_11 , str2, 32);
						LoadString(g_main_instance, IDS_STRING_LMSG_12 , str3, 32);
						MessageBox(hChild,(LPCSTR)str3,(LPCSTR)str2,MB_OK);
					}
					break;
				case IDM_CTPOPL3:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_NNNEDITCHRROM), hChild, (DLGPROC)nnnEditCromDlg);
					break;
				case IDM_CTPOPL4:	//
					LauncherROMInfoCopyC(hChild);
					break;
				case IDM_CTPOPL5:	//
					LauncherROMFileCopy(hChild);
					break;
				case IDM_CTPOPL6:	//
					DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hChild, (DLGPROC)LauncherFDSSaveClearDlg);
					break;
				case IDM_KAILLERA:
//					kailleraInit();
					{
						char *tmpGames=NULL, *p, tmptxt[MAX_PATH], country[10];
						unsigned int gamesSize=8192;
						int i=-1,s=0,l;
						tmpGames=(char *)malloc(gamesSize);
						tmpGames[0]=0; tmpGames[1]=0;

						SortItemp[LUNCHDBNAME]=1;
//						ListView_SortItems(phList, MyListSort, LUNCHDBNAME);
						LauncherListSort(LUNCHDBNAME);
						while((i=ListView_GetNextItem(phList, i, LVNI_ALL)) != -1){
							tmptxt[0]=0;
							ListView_GetItemText(phList, i, LUNCHDBNAME, tmptxt, MAX_PATH);
							if(tmptxt[0]==0)
								continue;
							country[0]=0;
							ListView_GetItemText(phList, i, LUNCHDBCOUN, country, 10);
							if(country[0]!=0){
								strcat(tmptxt, "[");
								strcat(tmptxt, country);
								strcat(tmptxt, "]");
							}
							while((s+strlen(tmptxt)+4)>gamesSize)
							{
								gamesSize*=2;
								tmpGames=(char *)realloc(tmpGames, gamesSize);
							}
							p=tmpGames+s;
							strcpy(p,tmptxt);
							l=strlen(p)+1;
							p[l]=0;
							s+=l;
						}
						kInfos.appName= "NNNesterJ[K01]"; //PROG_NAME;
						kInfos.gameList=tmpGames;
						nnnKailleraDlgFlag=1;
						nnnkailleraSetInfos(&kInfos);
						nnnkailleraSelectServerDialog(hChild);
						nnnKailleraDlgFlag=0;
						free(tmpGames);
					}
//					kailleraShutdown();
					break;
				case IDM_PREVLISTCHGD1:
				case IDM_PREVLISTCHGD2:
				case IDM_PREVLISTCHGD3:
				case IDM_PREVLISTCHGD4:
				case IDM_PREVLISTCHGD5:
				case IDM_PREVLISTCHGD6:
				case IDM_PREVLISTCHGD7:
				case IDM_PREVLISTCHGD8:
				case IDM_PREVLISTCHGD9:
					i = LOWORD(wp) - IDM_PREVLISTCHGD1;
					{
					for(j=0;j<9;++j){
						if(i==j){
							CheckMenuItem(GetMenu(hChild), (j+IDM_PREVLISTCHGD1), MFS_CHECKED);
						}
						else{
							CheckMenuItem(GetMenu(hChild), (j+IDM_PREVLISTCHGD1), MFS_UNCHECKED);
						}
					}
					if(i!=ListNo){
						LFileListSave(phList);
						ListNo=i;
						plistn=LFileListLoad(phList);
						LauncherWindowTset(hChild);
					}
					}
			}
			return 0;
			break;
		case WM_NOTIFY:
			if ((int)wp == ID_LISTVIEW){
				lvinfo = (LV_DISPINFO *)lp;
				switch (lvinfo->hdr.code) {
					case LVN_GETDISPINFO:
						LauncherOnGetDispInfo((NMLVDISPINFO *)lp);
						break;
					case LVN_COLUMNCLICK:
						pNMLV = (NM_LISTVIEW *)lp;
						if (SortItemp[pNMLV->iSubItem] == 1)
								SortItemp[pNMLV->iSubItem] = 2;	
						else
							SortItemp[pNMLV->iSubItem] = 1;	
//						ListView_SortItems(phList, MyListSort, pNMLV->iSubItem);
						LauncherListSort((int)pNMLV->iSubItem);
						break;
					case LVN_ITEMCHANGED:
						pNMLV = (NM_LISTVIEW *)lp;
						if(pNMLV->uOldState == 0 && pNMLV->uNewState == 3){
							str[0]='\0';
							ListView_GetItemText(phList, pNMLV->iItem, LUNCHCRCR, str, MAX_PATH);
							if(str[0]== '\0'){
								SendMessage(StatusbWnd, SB_SETTEXT, 255, (LPARAM)(LPSTR)"");
								break;
							}
							strcat(str, ", ");
							for(j=0, i= LUNCHDBNAME; i <= (LUNCHCOLMAX-1); ++i){
								for(;j < MAX_PATH;++j){
									if(str[j] == '\0')
										break;
								}
								if(j==MAX_PATH)
									break;
								ListView_GetItemText(phList, pNMLV->iItem, i, &str[j], MAX_PATH-j);
								if(i!=  (LUNCHCOLMAX-1))
									strcat(str, ", ");
							}
							SendMessage(StatusbWnd, SB_SETTEXT, 255, (LPARAM)(LPSTR)str);
						}
						break;
					case NM_DBLCLK:
						{
							LVHITTESTINFO hitpos;
							int i;

							memset(&hitpos, 0, sizeof(LVHITTESTINFO));
							GetCursorPos(&hitpos.pt);
							POINT pt = hitpos.pt;
							ScreenToClient(hChild, &pt);
							ScreenToClient(phList, &hitpos.pt);
							if(-1 !=(i=ListView_HitTest(phList, &hitpos))){
								ListView_GetItemText(phList, i, LUNCHFOLDN, previewfn, MAX_PATH);
								ListView_GetItemText(phList, i, LUNCHFILEN, str, MAX_PATH);
								PathCombine(previewfn, previewfn, str);
								PostMessage(main_window_handle, WM_PREVIEW, 0, 0);
							}
						}
						break;
					case LVN_KEYDOWN:
						{
						LV_KEYDOWN *kst;
						kst = (LV_KEYDOWN *)lp;
						switch(kst->wVKey){
							case VK_RETURN:
								i=-1;
								if((i=ListView_GetNextItem(phList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
									return 2;
								ListView_GetItemText(phList, i, LUNCHFOLDN, previewfn, MAX_PATH);
								ListView_GetItemText(phList, i, LUNCHFILEN, str, MAX_PATH);
								PathCombine(previewfn, previewfn, str);
								PostMessage(main_window_handle, WM_PREVIEW, 0, 0);
/*
								if(sttimerf){
									if(!KillTimer(hChild, ID_SBTIMER))
										return 0;
								}
								if(SetTimer(hChild, ID_SBTIMER, 150, NULL)){
									sttimerf=1;
									break;
								}
*/
						}
						}
						break;
				}
			}
			break;
		case WM_SETFOCUS:
			if(phList != GetFocus())
				SetFocus(phList);
			break;
		case WM_TIMER:
			KillTimer(hChild, ID_SBTIMER);
			sttimerf=0;
//			SendMessage(main_window_handle, WM_PREVIEW, 0, 0);
			PostMessage(main_window_handle, WM_PREVIEW, 0, 0);
			SetFocus(main_window_handle);
			break;
		case WM_CREATE:
			previewflag=1;
			previewh=hChild;
			g_pLauncherText=NULL;
			g_manLauncherText = 0;
			hInst = g_main_instance; //(HINSTANCE)GetWindowLong(GetParent(hChild), GWL_HINSTANCE);
			GetClientRect(hChild, &rc);
			NNNlunchcfgload();
			SetLauncherListName();
			if(nnnkailleraInit())
	            EnableMenuItem(GetMenu(hChild), IDM_KAILLERA,MF_ENABLED);
			CheckMenuItem(GetMenu(hChild), (ListNo+IDM_PREVLISTCHGD1), MFS_CHECKED);
            phList = CreateWindowEx(0, WC_LISTVIEW, "", WS_BORDER | WS_CHILD | WS_VISIBLE | LVS_REPORT| LVS_SINGLESEL | LVS_OWNERDATA |
				LVS_SHOWSELALWAYS| LVS_SINGLESEL, 0, 0, 0, 0, hChild, (HMENU)ID_LISTVIEW, hInst, NULL);
			dwStyle = ListView_GetExtendedListViewStyle(phList);
			dwStyle |= LVS_EX_GRIDLINES| LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP;
			ListView_SetBkColor(phList,launcherBackColor);
			ListView_SetTextBkColor(phList,launcherBackColor);
			ListView_SetTextColor(phList, launcherFontColor);
			ListView_SetExtendedListViewStyle(phList, dwStyle);
			InvalidateRect(phList,NULL,FALSE);

			lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM ;
			for(i=0;i<LUNCHCOLMAX;++i){
				for(j=0;j<LUNCHCOLMAX;++j){
					char str[64];
					if(i== j){
						LoadString(g_main_instance, IDS_STRING_LNCHCT01+j , str, 64);
						if(j==0||j==1||j==11||(j>=13&&j!=LUNCHDBPRIC))
							lvcol.fmt = LVCFMT_LEFT;
						else if(j==9||j==10)
							lvcol.fmt = LVCFMT_CENTER;
						else
							lvcol.fmt = LVCFMT_RIGHT;
						lvcol.cx = prevListWidth[j];
						lvcol.pszText = str;
						lvcol.iSubItem = i;
						ListView_InsertColumn(phList, i, &lvcol);
						break;
					}
				}
			}

			hFont = CreateFontIndirect(&launcherFont);
			if(hFont!=NULL){
				SendMessage(phList, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
			}
			InvalidateRect(phList, NULL, TRUE);
			StatusbWnd = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | SBARS_SIZEGRIP | WS_VISIBLE,
					0, 0, 0, 0, hChild, (HMENU)ID_STATUS, hInst, NULL);
			SendMessage(StatusbWnd , SB_SIMPLE, TRUE, 0L);
			GetWindowRect(StatusbWnd, &rc);
            Statusbvs = rc.bottom - rc.top;
			if(prevconfig[8]){
				PrevScanFold(hChild, phList);
				if(prevconfig[0]){
					RomInfoScan(hChild, phList);
					if(prevconfig[3]){
						RomCrcCheck(hChild);
						LauncherWindowTset(hChild);
					}
				}
			}
			else if(!(plistn=LFileListLoad(phList))){
/*				PrevScanFold(hChild, phList);
				if(prevconfig[0]){
					RomInfoScan(hChild, phList);
					if(prevconfig[3]){
						RomCrcCheck(hChild);
						LauncherWindowTset(hChild);
					}
				}*/
			}
			else{
				LauncherWindowTset(hChild);
			}
			if(prevListArray[0] != prevListArray[1])
				ListView_SetColumnOrderArray(phList, LUNCHCOLMAX, prevListArray);
			PrevFastSw=0;
			return 0;
			break;
		case WM_CLOSE:
			SendMessage(main_window_handle, WM_PREVIEWSC, 0, 0);
			DestroyWindow(hChild);
			return 0;
			break;
		case WM_DESTROY:
			NNNlunchcfgsave();
			LFileListSave(phList);
			PreviewFSave(hChild);
			DeleteObject(hFont);
			PrevFastSw=0;
//			nnnkailleraShutdown();
			if(sttimerf)
				KillTimer(hChild, ID_SBTIMER);
			previewflag=0;
			ListView_SetItemCount(phList, 0);
			if(g_pLauncherText){
				free(g_pLauncherText);
				g_pLauncherText=NULL;
			}
			g_manLauncherText = 0;
			previewh = NULL;
			return 0;
			break;
		default:
			return (DefWindowProc(hChild, msg, wp, lp));
	}
	return 0L;
}



int AddLauncherItem(char *fn, char *foldn){
	if((unsigned int)g_manLauncherText == plistn){
		g_pLauncherText = (LauncherText *)realloc(g_pLauncherText, sizeof(LauncherText)*(g_manLauncherText+500));
		if(g_pLauncherText==NULL)
			return 0;
		memset((unsigned char *)&g_pLauncherText[g_manLauncherText], 0, 500*sizeof(LauncherText));
		g_manLauncherText+=500;
	}
	strcpy(g_pLauncherText[plistn].FileName, fn);
	strcpy(g_pLauncherText[plistn].FolderName, foldn);
	++plistn;
	return 1;
}



void PrevScanFold(HWND hWnd, HWND hList){
	char	dirstr[MAX_PATH];
	int		lp, j=0;
	HANDLE	hFind;
	WIN32_FIND_DATA		fd;

	ListView_SetItemCount(hList, 0);
	if(g_pLauncherText){
		free(g_pLauncherText);
		g_pLauncherText = NULL;
	}
	g_manLauncherText = 0;
	plistn=0;
	if(!prevfoldan)
		return;
	for(lp=0; lp<prevfoldan; ++lp){
		strcpy(dirstr, prevfoldn[lp]);
		if(prevfoldsf[lp]){
			PrevScanSubFold(hWnd, dirstr);
			continue;
		}
		PathCombine(dirstr , dirstr, "*.*");
		hFind = FindFirstFile(dirstr, &fd);
		if(hFind == INVALID_HANDLE_VALUE) {
			continue;
		}
		do {
			if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
				char tfn[MAX_PATH];
				PathCombine(tfn, prevfoldn[lp], fd.cFileName);
				if(-1 != CheckROMFile(tfn, hWnd, NULL)){
					char wstr[30];
					AddLauncherItem(fd.cFileName, prevfoldn[lp]);
					wsprintf(wstr, LNCHWTITLE, plistn);
					SetWindowText(hWnd, wstr);
				}
			}
		}while(FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	ListView_SetItemCount(phList, plistn);
	return;
}


int PrevScanSubFold(HWND hWnds, char *subd)
{
	char	filestr[MAX_PATH], dirstr[MAX_PATH];
	int		i=0,j=0;
	HANDLE	hFind;
	WIN32_FIND_DATA		fd;

	strcpy(dirstr, subd);
	PathCombine(dirstr , dirstr, "*.*");
	hFind = FindFirstFile(dirstr, &fd);
	if(hFind == INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return 0;
	}

	do {
		if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			char tfn[MAX_PATH];
			PathCombine(tfn, subd, fd.cFileName);
			if(-1 != CheckROMFile(tfn, hWnds, NULL)){
				char wstr[30];
				AddLauncherItem(fd.cFileName, subd);
				wsprintf(wstr, LNCHWTITLE, plistn);
				SetWindowText(hWnds, wstr);
			}
		}
		else{
			if(fd.cFileName[0] != '.'){
				strcpy(filestr, subd);
				PathCombine(filestr , filestr, fd.cFileName);
				PrevScanSubFold(hWnds, filestr);
			}
		}
	}while(FindNextFile(hFind, &fd));
	FindClose(hFind);
	return 1;
}

void PreviewFSave(HWND hWnd){
	char str[MAX_PATH];
	DWORD i,j,k;
//	RECT rect;
	HANDLE hFile;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "Preview.cfg");
	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
    	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return;
		}
    }
	ZeroMemory(str, MAX_PATH);
	if(!WriteFile(hFile, str, sizeof(int), &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	WINDOWPLACEMENT wp;
	wp.length = sizeof( WINDOWPLACEMENT );
	GetWindowPlacement(hWnd, &wp);
	prevWindpos[0] = wp.rcNormalPosition.left;
	prevWindpos[1] = wp.rcNormalPosition.top;
	prevWindpos[2] = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	prevWindpos[3] = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	if(!WriteFile(hFile, prevWindpos, sizeof(int)*4, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	if(!WriteFile(hFile, prevconfig, 16, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}

	{
		int tmparray[LUNCHCOLMAX];
		memset(tmparray, 0, sizeof(tmparray));
		ListView_GetColumnOrderArray(phList, LUNCHCOLMAX, tmparray);

		for(i=0; i < LUNCHCOLMAX ; ++i){
			j= ListView_GetColumnWidth(phList, i);
			if(!WriteFile(hFile, &j, 2, &dwAccBytes, NULL)){
				CloseHandle(hFile);
				return;
			}
			if(!WriteFile(hFile, &tmparray[i], 2, &dwAccBytes, NULL)){
				CloseHandle(hFile);
				return;
			}
		}
	}
	j=dwAccBytes;
	if(!WriteFile(hFile, str, 256-(sizeof(int)*(LUNCHCOLMAX+5)+16), &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	k=strlen(PreOpenFolder);
	++k;
	if(!WriteFile(hFile, &k, 2, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	if(!WriteFile(hFile, &PreOpenFolder, k, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	if(!WriteFile(hFile, &prevfoldan, sizeof(int), &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return;
	}
	for(i=prevfoldan,j=0; i; --i, ++j){
		if(!WriteFile(hFile, &prevfoldsf[j], 1, &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		k=strlen(prevfoldn[j]);
		++k;
		if(!WriteFile(hFile, &k, 2, &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		if(!WriteFile(hFile, prevfoldn[j], k, &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}		
	}
	CloseHandle(hFile);
	return;
}


int PreviewFLoad(){
	char str[MAX_PATH];
	DWORD wd, i, j;
	HANDLE hFile;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "Preview.cfg");
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		return 0;
    }
	if(!ReadFile(hFile, &wd, 4, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return 0;
	}
	if(4 != dwAccBytes || wd!=0){
		CloseHandle(hFile);
		return 0;
	}
	SetFilePointer(hFile, 0x15, 0, FILE_BEGIN);
	ReadFile(hFile, &wd, 1, &dwAccBytes, NULL);
	if(wd){
		prevWindpos[0]=0;
		prevWindpos[1]=0;
		prevWindpos[2]=0;
		prevWindpos[3]=0;
	}
	else{
		SetFilePointer(hFile, 4, 0, FILE_BEGIN);
		ReadFile(hFile, prevWindpos, sizeof(int)*4, &dwAccBytes, NULL);
		if(sizeof(int)*4 != dwAccBytes){
			CloseHandle(hFile);
			prevfoldan=0;
			return 0;
		}
	}
	SetFilePointer(hFile, 0x16, 0, FILE_BEGIN);
	ReadFile(hFile, &wd, 1, &dwAccBytes, NULL);
	if(wd){
	}
	else{
		SetFilePointer(hFile, 0x24, 0, FILE_BEGIN);
		memset(prevListArray, 0, sizeof(int)*LUNCHCOLMAX);
		for(i=0;i<LUNCHCOLMAX;++i){
			ReadFile(hFile, &prevListWidth[i], 2, &dwAccBytes, NULL);
			ReadFile(hFile, &prevListArray[i], 2, &dwAccBytes, NULL);
/*			if(sizeof(int)*LUNCHCOLMAX != dwAccBytes){
				CloseHandle(hFile);
				prevfoldan=0;
				return 0;
			}*/
		}
	}
/*	for(i=0; i<LUNCHCOLMAX;++i){
		if(prevListWidth[i]== 0)
			prevListWidth[i]= 5;
	}*/
	SetFilePointer(hFile, 0x14, 0, FILE_BEGIN);
	ReadFile(hFile, prevconfig, 16, &dwAccBytes, NULL);
	if(16 != dwAccBytes){
		CloseHandle(hFile);
		prevfoldan=0;
		return 0;
	}
	SetFilePointer(hFile, 0x100, 0, FILE_BEGIN);
	j=0;
	ReadFile(hFile, &j, 2, &dwAccBytes, NULL);
	ReadFile(hFile, &PreOpenFolder, j, &dwAccBytes, NULL);
	ReadFile(hFile, &wd, sizeof(int), &dwAccBytes, NULL);
	if(sizeof(int) != dwAccBytes){
		CloseHandle(hFile);
		prevfoldan=0;
		return 0;
	}
	for(i=0, j=0; wd; --wd, ++i){
		ReadFile(hFile, &prevfoldsf[i], 1, &dwAccBytes, NULL);
		ReadFile(hFile, &j, 2, &dwAccBytes, NULL);
		ReadFile(hFile, prevfoldn[i], j, &dwAccBytes, NULL);
	}
	CloseHandle(hFile);
	prevfoldan=i;
	return 1;
}




//Preview Folder Select    IDD_PREVFSDIALOG
LRESULT CALLBACK MyCheatDlg13(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	char Folder[MAX_PATH];
	int i,j;
	LV_COLUMN lvcol;
	DWORD dwStyle;
    LV_ITEM item;
	static HWND hList;

    switch(msg) {
        case WM_INITDIALOG:
			hList = GetDlgItem(hDlgWnd, IDC_PREVFSLIST);

			dwStyle = ListView_GetExtendedListViewStyle(hList);
			dwStyle |= LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
			ListView_SetExtendedListViewStyle(hList, dwStyle);
			lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvcol.fmt = LVCFMT_LEFT;
			lvcol.cx = 300;
			lvcol.iSubItem = 0;
			lvcol.pszText = "Folder Name";
			ListView_InsertColumn(hList, 0, &lvcol);
			if(prevfoldan){
				for(i=0; i<prevfoldan; ++i){
					item.mask = LVIF_TEXT | LVIF_PARAM;
					item.pszText = prevfoldn[i];
					item.iItem = i;
					item.iSubItem = 0;
					item.lParam = i;
					ListView_InsertItem(hList, &item);
					if(prevfoldsf[i]){
						ListView_SetItemState(hList, i, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
					}
					else{
						ListView_SetItemState(hList, i, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
					}
				}
			}
			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT1), LauncherListName[ListNo]);
            return TRUE;
			break;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDC_PVFSBUTTON1:		//Add
					if(MyFoldOpenDlg(hDlgWnd, Folder)){
						if(!AddPFoldchk(hList, Folder)){
							item.mask = LVIF_TEXT | LVIF_PARAM;
							item.pszText = Folder;
							item.iItem = prevfoldan;
							item.iSubItem = 0;
							item.lParam = prevfoldan;
							ListView_InsertItem(hList, &item);
						}
					}
					return TRUE;
				   break;
				case IDC_PVFSBUTTON2:		//Del
					i=-1;
					while((i=ListView_GetNextItem(hList,-1,LVNI_ALL | LVNI_SELECTED)) != -1){
						ListView_DeleteItem(hList,i);
					}
					return TRUE;
					break;
                case IDOK:
					prevfoldan=0;
					for(i=-1, j=0, prevfoldan=0;(i=ListView_GetNextItem(hList, i,LVNI_ALL)) != -1; ++j){
						ListView_GetItemText(hList, i, 0, prevfoldn[j], MAX_PATH);
						prevfoldsf[j]=(char)ListView_GetCheckState(hList, i);
						++prevfoldan;
					}
					GetDlgItemText(hDlgWnd, IDC_EDIT1, (LPTSTR)LauncherListName[ListNo], 63);
					EndDialog(hDlgWnd, IDOK);
					return TRUE;
				   break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
					break;
			}
            break;
    }
    return FALSE;
}


int AddPFoldchk(HWND hWnd, char *fdname){
	int i=-1;
	char str[MAX_PATH];
	while((i=ListView_GetNextItem(hWnd, i,LVNI_ALL)) != -1){
		ListView_GetItemText(hWnd, i, 0, str, MAX_PATH);
		if(MystrCmp(str, fdname)){
			return 1;
		}
	}
	return 0;
}


static int _stdcall FolderOpens( HWND hwnd, UINT uiMsg, LPARAM lParam, LPARAM lpData)
{
	if(uiMsg == BFFM_INITIALIZED){
		if( lpData != NULL){
			SendMessage( hwnd, BFFM_SETSELECTION, TRUE, lpData );
		}
	}
	return(0);
}


int MyFoldOpenDlg(HWND hWnds, char *folds){
    BROWSEINFO  bi;
    ITEMIDLIST  *idl;
    LPMALLOC    g_pMalloc;
	char str[64];
    SHGetMalloc(&g_pMalloc);
    bi.hwndOwner        =hWnds;
    bi.pidlRoot         =NULL;
    bi.pszDisplayName   =folds;

	LoadString(g_main_instance, IDS_STRING_CDLG_09 , str, 64);
    bi.lpszTitle        =str;
    bi.ulFlags          =BIF_RETURNONLYFSDIRS;
	if(prevconfig[4]){
	    bi.lpfn             =(BFFCALLBACK)0;
	    bi.lParam           =(LPARAM)0;
	}
	else{
	    bi.lpfn             =(BFFCALLBACK)FolderOpens;
	    bi.lParam           =(LPARAM)PreOpenFolder;
	}
    bi.iImage           =0;
    idl=SHBrowseForFolder(&bi);	
    if(idl == NULL){
		return 0;							//error
	}
    if(SHGetPathFromIDList(idl,folds) == FALSE){
		return 0;							//error
	}
	g_pMalloc -> Free( idl);
	strcpy(PreOpenFolder, folds);
	return 1;
}

void LunchDInitMode(){
	int i;
	for(i=0; i< LUNCHCOLMAX; ++i){
		ListView_SetColumnWidth(phList, i, 50);
	}
	return;
}


int MystrFnIcut(char *sa){
	int i=0,j=0;

	while(sa[j]){
		if(sa[j]=='<' || sa[j]=='>' || sa[j]=='"'){
			++j;
			if(sa[j]=='\0')
				break;
		}
		sa[i]=sa[j];
		++i,++j;
	}
	sa[i]='\0';
	if(i==j)
		return 0;
	return 1;
}


int LFileListSave(HWND hList){
	int i, j, n;
	char str[sizeof(LauncherText)];
	const char *sc = ";";
	HANDLE hFile;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "Launcher");
	if(ListNo){
		char tstr[2]={0x30,0};
		tstr[0]+=ListNo;
		strcat(str,tstr);
	}
	strcat(str, ".lst");
	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
    	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return 0;
		}
    }
	for(i=0; (unsigned int)i< plistn; i++){
		n=0;
		for(j=0;g_pLauncherText[i].FileName[j] ; j++){ str[n++] = g_pLauncherText[i].FileName[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].FolderName[j]; j++){ str[n++] = g_pLauncherText[i].FolderName[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].Mapper[j]; j++){ str[n++] = g_pLauncherText[i].Mapper[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].PromSize[j]; j++){ str[n++] = g_pLauncherText[i].PromSize[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].CromSize[j]; j++){ str[n++] = g_pLauncherText[i].CromSize[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].Info[j]; j++){ str[n++] = g_pLauncherText[i].Info[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].AllCRC[j]; j++){ str[n++] = g_pLauncherText[i].AllCRC[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].PROMCRC[j]; j++){ str[n++] = g_pLauncherText[i].PROMCRC[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].CROMCRC[j]; j++){ str[n++] = g_pLauncherText[i].CROMCRC[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].Resault[j]; j++){ str[n++] = g_pLauncherText[i].Resault[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].Header[j]; j++){ str[n++] = g_pLauncherText[i].Header[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].DBInfo[j]; j++){ str[n++] = g_pLauncherText[i].DBInfo[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].RomDbCmp[j]; j++){ str[n++] = g_pLauncherText[i].RomDbCmp[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].DBTitle[j]; j++){ str[n++] = g_pLauncherText[i].DBTitle[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].DBCountry[j]; j++){ str[n++] = g_pLauncherText[i].DBCountry[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].DBMaker[j]; j++){ str[n++] = g_pLauncherText[i].DBMaker[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].DBRelease[j]; j++){ str[n++] = g_pLauncherText[i].DBRelease[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].DBPrice[j]; j++){ str[n++] = g_pLauncherText[i].DBPrice[j];}
		str[n++]=';';
		for(j=0; g_pLauncherText[i].DBGenre[j]; j++){ str[n++] = g_pLauncherText[i].DBGenre[j];}
		str[n++]=';';
		str[n++]=0x0d;
		str[n++]=0x0a;
		WriteFile(hFile, str, n, &dwAccBytes, NULL);
	}
	CloseHandle(hFile);
	return 1;
}


int LFileListLoad(HWND hList){
	int i=0, j, k, listn=0, flag;
	char str[MAX_PATH], *temp;
	char tstr[LUNCHCOLMAX][MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes=1;


	ListView_SetItemCount(hList, 0);
	if(g_pLauncherText){
		free(g_pLauncherText);
		g_pLauncherText = NULL;
	}
	g_manLauncherText = 0;
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "Launcher");
	if(ListNo){
		char tstr[2]={0x30,0};
		tstr[0]+=ListNo;
		strcat(str,tstr);
	}
	strcat(str, ".lst");
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		return 0;
    }
	dwAccBytes = GetFileSize(hFile, NULL);
	if(dwAccBytes==-1){
		return 0;
	}
	temp=(char *)malloc(dwAccBytes);
	if(NULL==temp){
		CloseHandle(hFile);
		return 0;
	}
	ReadFile(hFile, temp, dwAccBytes, &dwAccBytes, NULL);
	CloseHandle(hFile);

	while(dwAccBytes){
		for(j=0, flag=0; !flag; ++j){
			if(j>=LUNCHCOLMAX){
				for(k=0; ; ++k,++i,--dwAccBytes){
					if(!dwAccBytes){
						break;
					}
					else if(temp[i]==0x0d){
						++i, --dwAccBytes;
						if(!dwAccBytes)
							break;
						++i, --dwAccBytes;
						break;
					}
				}
				break;
			}
			for(k=0; ; ++k){
				str[k] = temp[i++];
				--dwAccBytes;
				if(str[k]==';' || str[k] == 0x0d || !dwAccBytes)
					break;
			}
			if(!dwAccBytes)
				break;
			if(str[k]==0x0d){
				str[k] = temp[i++];
				--dwAccBytes;
				flag=1;
			}
			str[k]='\0';
			strcpy(tstr[j], str);
		}
		if(g_manLauncherText == listn){
			g_pLauncherText = (LauncherText *)realloc(g_pLauncherText, sizeof(LauncherText)*(g_manLauncherText+500));
			if(g_pLauncherText==NULL)
				return 0;
			memset((unsigned char *)&g_pLauncherText[g_manLauncherText], 0, 500*sizeof(LauncherText));
			g_manLauncherText+=500;
		}
		for(k=0;k<j;++k){
			for(int l=0; l<LUNCHCOLMAX; ++l){
				if(k==l){
					char *p;
					switch(k){
						case 0:
							p = g_pLauncherText[listn].FileName;
							break;
						case 1:
							p = g_pLauncherText[listn].FolderName;
							break;
						case 2:
							p = g_pLauncherText[listn].Mapper;
							break;
						case 3:
							p = g_pLauncherText[listn].PromSize;
							break;
						case 4:
							p = g_pLauncherText[listn].CromSize;
							break;
						case 5:
							p = g_pLauncherText[listn].Info;
							break;
						case 6:
							p = g_pLauncherText[listn].AllCRC;
							break;
						case 7:
							p = g_pLauncherText[listn].PROMCRC;
							break;
						case 8:
							p = g_pLauncherText[listn].CROMCRC;
							break;
						case 9:
							p = g_pLauncherText[listn].Resault;
							break;
						case 10:
							p = g_pLauncherText[listn].Header;
							break;
						case 11:
							p = g_pLauncherText[listn].DBInfo;
							break;
						case 12:
							p = g_pLauncherText[listn].RomDbCmp;
							break;
						case 13:
							p = g_pLauncherText[listn].DBTitle;
							break;
						case 14:
							p = g_pLauncherText[listn].DBCountry;
							break;
						case 15:
							p = g_pLauncherText[listn].DBMaker;
							break;
						case 16:
							p = g_pLauncherText[listn].DBRelease;
							break;
						case 17:
							p = g_pLauncherText[listn].DBPrice;
							break;
						case 18:
							p = g_pLauncherText[listn].DBGenre;
							break;
					}
					strcpy(p, tstr[l]);
				}
			}
		}
		++listn;
	}
	free(temp);
	ListView_SetItemCount(hList, listn);
	return listn;
}


void LunchDHideMode(){
	int i;
	for(i= LUNCHMAPP; i< LUNCHDBNAME; ++i){
		prevListWidth[i] = ListView_GetColumnWidth(phList, i);
		ListView_SetColumnWidth(phList, i, 0);
	}
	return;
}


void LunchDDispMode(){
	int i;
	for(i= LUNCHMAPP; i< LUNCHDBNAME; ++i){
		if(prevListWidth[i])
			ListView_SetColumnWidth(phList, i, prevListWidth[i]);
		else
			ListView_SetColumnWidth(phList, i, 50);
	}
	return;
}

void LunchWidthBuckup(){
	int i;
	for(i= 0; i< LUNCHCOLMAX; ++i){
		prevListWidth[i] = ListView_GetColumnWidth(phList, i);
	}
	return;
}


void LunchWidthRestore(){
	int i;
	for(i= 0; i< LUNCHCOLMAX; ++i){
		ListView_SetColumnWidth(phList, i, prevListWidth[i]);
	}
	return;
}


void LauncherListGoodRomMove(HWND hwnd){
	int i=-1, lvflag, movenum=0;
	char flag1=0, str1[MAX_PATH], str2[MAX_PATH], str3[MAX_PATH], Folder[MAX_PATH];
	char wstr[30];

	if(!MyFoldOpenDlg(hwnd, Folder))
		return;
	if(IDCANCEL==DialogBox((HINSTANCE)GetWindowLong(GetParent(hwnd), GWL_HINSTANCE), MAKEINTRESOURCE(IDD_CTDIALOG19), hwnd, (DLGPROC)MyCheatDlg24))
		return;

	if(MyCheatAdr)
		lvflag=LVNI_ALL | LVNI_SELECTED;
	else
		lvflag=LVNI_ALL;
	flag1=(char)MyCheatNum;
	while((i=ListView_GetNextItem(phList, i, lvflag)) != -1){
		str1[0]=0;
		ListView_GetItemText(phList, i, LUNCHCRCR, str1, 15);
		if(str1[0]!='G' && str1[0]!='H')
			continue;
		ListView_GetItemText(phList, i, LUNCHFOLDN, str1, MAX_PATH);
		ListView_GetItemText(phList, i, LUNCHFILEN, str3, MAX_PATH);
		PathCombine(str1, str1, str3);
		if(!MoveFile(Folder, str1)){
			strcpy(str2, Folder);
			PathCombine(str2, str2, str3);
			if(CopyFile(str1, str2, TRUE)){
				DeleteFile(str1);
			}
			else
				continue;
		}
		if(!flag1){
			ListView_DeleteItem(phList, i);
			--plistn;
			i=-1;
		}
		else{
			strcpy(g_pLauncherText[i].FolderName, Folder );
//			ListView_SetItemText(phList, i, prevListArray[LUNCHFOLDN], Folder);
		}
		++movenum;
		wsprintf(wstr, "[%u / %u] GOOD ROM File Move...", movenum, plistn);
		SetWindowText(hwnd, wstr);
	}
	return;
}



void LauncherListNoRomMove(HWND hwnd){
	int i=-1, lvflag, movenum=0;
	char flag1=0, str1[MAX_PATH], str2[MAX_PATH], str3[MAX_PATH], Folder[MAX_PATH];
	char wstr[30];

	if(!MyFoldOpenDlg(hwnd, Folder))
		return;
	if(IDCANCEL==DialogBox((HINSTANCE)GetWindowLong(GetParent(hwnd), GWL_HINSTANCE), MAKEINTRESOURCE(IDD_CTDIALOG19), hwnd, (DLGPROC)MyCheatDlg24))
		return;

	if(MyCheatAdr)
		lvflag=LVNI_ALL | LVNI_SELECTED;
	else
		lvflag=LVNI_ALL;
	flag1=(char)MyCheatNum;
	while((i=ListView_GetNextItem(phList, i, lvflag)) != -1){
		str1[0]=0;
		ListView_GetItemText(phList, i, LUNCHCRCR, str1, 15);
		if(str1[0]=='G' || str1[0]=='H')
			continue;
		ListView_GetItemText(phList, i, LUNCHFOLDN, str1, MAX_PATH);
		ListView_GetItemText(phList, i, LUNCHFILEN, str3, MAX_PATH);
		PathCombine(str1, str1, str3);
		if(!MoveFile(Folder, str1)){
			strcpy(str2, Folder);
			PathCombine(str2, str2, str3);
			if(CopyFile(str1, str2, TRUE)){
				DeleteFile(str1);
			}
			else
				continue;
		}
		if(!flag1){
			ListView_DeleteItem(phList, i);
			--plistn;
			i=-1;
		}
		else{
			strcpy(g_pLauncherText[i].FolderName, Folder );
//			ListView_SetItemText(phList, i, prevListArray[LUNCHFOLDN], Folder);
		}
		++movenum;
		wsprintf(wstr, "[%u / %u] BAD? ROM File Move...", movenum, plistn);
		SetWindowText(hwnd, wstr);
	}
	return;
}



void LauncherListFileCheck(HWND hwnd){
	int i=-1, movenum=0;
	char str1[MAX_PATH], str2[MAX_PATH];
	char wstr[30];
	HANDLE hFind;
	WIN32_FIND_DATA fd;

	while((i=ListView_GetNextItem(phList, i, LVNI_ALL)) != -1){
		ListView_GetItemText(phList, i, LUNCHFOLDN, str1, MAX_PATH);
		ListView_GetItemText(phList, i, LUNCHFILEN, str2, MAX_PATH);
		PathCombine(str1, str1, str2);
		hFind = FindFirstFile(str1, &fd);
		if(hFind == INVALID_HANDLE_VALUE){
			ListView_DeleteItem(phList, i);
			i=-1;
			--plistn;
		}
		FindClose(hFind);
		++movenum;
		wsprintf(wstr, "[%u / %u] File Check...", movenum, plistn);
		SetWindowText(hwnd, wstr);
	}
	return;
}



int g_structoffset;
int g_subItemn;

int LauncherSorts(const void *p1 , const void *p2){

	char *n1, *n2;

	n1 = (char *)p1;
	n2 = (char *)p2;
	n1+=g_structoffset;
	n2+=g_structoffset;

	{
		if(SortItemp[(int)g_subItemn] == 1){
			return MystrJscmp(n1, n2, 0);
		}
		else{
		    return MystrJscmp(n1, n2, 1) * -1;
		}
	}
}

int LauncherSortn(const void *p1 , const void *p2){

	char *n1, *n2;

	n1 = (char *)p1;
	n2 = (char *)p2;
	n1+=g_structoffset;
	n2+=g_structoffset;

	{
		if(SortItemp[(int)g_subItemn] == 1){
			return MystrJncmp(n1, n2, 0);
		}
		else{
		    return MystrJncmp(n1, n2, 1) * -1;
		}
	}
}

void LauncherListSort(int nItem){
	int structoffset;
	char cflag= 0;
	
	g_subItemn = nItem;

	switch(nItem){
		case 0:
			structoffset = offsetof(struct LauncherText,  FileName);
			break;
		case 1:
			structoffset = offsetof(struct LauncherText,  FolderName);
			break;
		case 2:
			structoffset = offsetof(struct LauncherText,  Mapper);
			cflag = 1;
			break;
		case 3:
			structoffset = offsetof(struct LauncherText,  PromSize);
			cflag = 1;
			break;
		case 4:
			structoffset = offsetof(struct LauncherText,  CromSize);
			cflag = 1;
			break;
		case 5:
			structoffset = offsetof(struct LauncherText,  Info);
			break;
		case 6:
			structoffset = offsetof(struct LauncherText,  AllCRC);
			cflag = 1;
			break;
		case 7:
			structoffset = offsetof(struct LauncherText,  PROMCRC);
			cflag = 1;
			break;
		case 8:
			structoffset = offsetof(struct LauncherText,  CROMCRC);
			cflag = 1;
			break;
		case 9:
			structoffset = offsetof(struct LauncherText,  Resault);
			break;
		case 10:
			structoffset = offsetof(struct LauncherText,  Header);
			break;
		case 11:
			structoffset = offsetof(struct LauncherText,  DBInfo);
			break;
		case 12:
			structoffset = offsetof(struct LauncherText,  RomDbCmp);
			break;
		case 13:
			structoffset = offsetof(struct LauncherText,  DBTitle);
			break;
		case 14:
			structoffset = offsetof(struct LauncherText,  DBCountry);
			break;
		case 15:
			structoffset = offsetof(struct LauncherText,  DBMaker);
			break;
		case 16:
			structoffset = offsetof(struct LauncherText,  DBRelease);
			cflag = 1;
			break;
		case 17:
			structoffset = offsetof(struct LauncherText,  DBPrice);
			cflag = 1;
			break;
		case 18:
			structoffset = offsetof(struct LauncherText,  DBGenre);
			break;
	}
	g_structoffset = structoffset;
	if(cflag == 0){
		qsort((void *)g_pLauncherText, plistn, sizeof(LauncherText), LauncherSorts);
	}
	else{
		qsort((void *)g_pLauncherText, plistn, sizeof(LauncherText), LauncherSortn);
	}
	ListView_SetItemCount(phList, plistn);
}


int RomInfoScan(HWND hWnd, HWND hList){
	int i=-1, listn=0;
	char str1[MAX_PATH], str2[MAX_PATH], *extp;
	HANDLE	hFile;
    DWORD ReadByte;
	unsigned long romcrc;
	unsigned char *temp=NULL;
	const char *nesExtensions[] = { "*.nes", "*.fds", "*.fam", "*.nsf", NULL };

	while((i=ListView_GetNextItem(hList, i, LVNI_ALL)) != -1){
		ListView_GetItemText(hList, i, LUNCHFILEN, str1, MAX_PATH);
		{
		char wstr[30];
		wsprintf(wstr, "[%u / %u] File Scan ...", listn,plistn);
		SetWindowText(hWnd, wstr);
		++listn;
		}
		extp=PathFindExtension(str1);
		++extp;
		temp = NULL;
		if(MystrCmp(extp , "nes")||MystrCmp(extp , "fds")){
			ListView_GetItemText(hList, i, LUNCHFOLDN, str2, MAX_PATH);
			PathCombine(str2 , str2, str1);
			hFile = CreateFile(str2, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile==INVALID_HANDLE_VALUE)
				continue;
			ReadByte = GetFileSize(hFile, NULL);
			temp=(unsigned char *)malloc(ReadByte);
			if(temp==NULL){
				CloseHandle(hFile);
				continue;
			}

			ReadFile(hFile, temp, ReadByte, &ReadByte, NULL);
			CloseHandle(hFile);
			hFile=NULL;
		}
		else{
			ListView_GetItemText(hList, i, LUNCHFOLDN, str2, MAX_PATH);
			PathCombine(str2 , str2, str1);
			if(!Uncompress(hWnd, &temp, str2, &ReadByte, nesExtensions))
				continue;
			if(temp==NULL)
				continue;
		}
		if(!memcmp(temp, "NES", 3)){
			if(!RomInfoSet(hList, temp, i)){
				free(temp);
				temp=NULL;
				continue;
			}
			if(ReadByte<((unsigned int)temp[4]*0x4000+(unsigned int)temp[5]*0x2000+0x10 +((temp[6]&4)?512:0))){
				free(temp);
				temp=NULL;
				continue;
			}
			unsigned int PROM_Size = temp[4]*0x4000;
			unsigned char *ch = temp+0x10+temp[4]*0x4000;

			if(temp[6]&4){
				PROM_Size+=512;
				ch+=512;
			}
			romcrc=CrcCalc(temp+0x10, PROM_Size+(unsigned int)temp[5]*0x2000);
			wsprintf(str1, "%08x", romcrc);
			strcpy(g_pLauncherText[i].AllCRC, str1);
//			ListView_SetItemText(hList, i, prevListArray[LUNCHALLCRC], str1);
			romcrc=CrcCalc(temp+0x10, (unsigned int)PROM_Size);
			wsprintf(str1, "%08x", romcrc);
			strcpy(g_pLauncherText[i].PROMCRC, str1);
//			ListView_SetItemText(hList, i, prevListArray[LUNCHPCRC], str1);
			if(temp[6]&4)
				romcrc=CrcCalc(ch, (unsigned int)temp[5]*0x2000+512);
			else
				romcrc=CrcCalc(ch, (unsigned int)temp[5]*0x2000);
			wsprintf(str1, "%08x", romcrc);
			strcpy(g_pLauncherText[i].CROMCRC, str1);
//			ListView_SetItemText(hList, i, prevListArray[LUNCHCCRC], str1);
			free(temp);
			temp=NULL;
		}
		else if(!memcmp(temp, "FDS", 3)){
			if(temp){
				FdsInfoSet(hList, temp, i);
				free(temp);
				temp=NULL;
			}
		}
		else{ // !FDS !NES
		}
		if(temp){
			free(temp);
			temp=NULL;
		}
	}
	LauncherWindowTset(hWnd);
	return 1;
}


int RomInfoSet(HWND hList, unsigned char *hp, int item){
	char str[16];
	int i,j;

	if(hp[0]!='N' || hp[1]!='E' || hp[2]!='S' || hp[3]!=0x1a){
		return 0;
	}
	wsprintf(str, "%uKB", (unsigned char)hp[4]*16);
	strcpy(g_pLauncherText[item].PromSize, str);
//	ListView_SetItemText(hList, item, prevListArray[LUNCHPSIZE], str);
	wsprintf(str, "%uKB", (unsigned char)hp[5]*8);
	strcpy(g_pLauncherText[item].CromSize, str);
//	ListView_SetItemText(hList, item, prevListArray[LUNCHCSIZE], str);
	wsprintf(str, "%u", (((unsigned char)hp[6]>>4 &0x0f)|(unsigned char)hp[7]&0xf0));
	strcpy(g_pLauncherText[item].Mapper, str);
//	ListView_SetItemText(hList, item, prevListArray[LUNCHMAPP], str);
	if(hp[6]&1)
		str[0]='V';
	else
		str[0]='H';
	if(hp[6]&2)
		str[1]='1';
	else
		str[1]='0';
	if(hp[6]&4)
		str[2]='1';
	else
		str[2]='0';
	if(hp[6]&8)
		str[3]='1';
	else
		str[3]='0';
	for(i=8,j=0; i < 0x10; ++i){
		j|=(int)hp[i];
	}
//	if(hp[7]&0x0f)
//		j=1;
	if(j){
		strcpy(g_pLauncherText[item].Header, "1");
//		ListView_SetItemText(hList, item, prevListArray[LUNCHHEADS], "1");
	}
	else{
		strcpy(g_pLauncherText[item].Header, "0");
//		ListView_SetItemText(hList, item, prevListArray[LUNCHHEADS], "0");
	}
	str[4]=0;
	strcpy(g_pLauncherText[item].Info, str);
//	ListView_SetItemText(hList, item, prevListArray[LUNCHRINFO], str);
	return 1;
}


int RomCrcCheck(HWND hWnd){
	int i=-1, j, listn=0;
	char c, str[MAX_PATH], str1[10], str2[10];
	unsigned char *temp=NULL, *temp2=NULL, flag;
	HANDLE hFile;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	if(prevconfig[6])
		PathCombine(str , str, "nesdbase.dat");
	else
		PathCombine(str , str, "famicom.dat");
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE){
		dwAccBytes = GetFileSize(hFile, NULL);
		if(dwAccBytes!=-1){
			temp=(unsigned char *)malloc( dwAccBytes+1);
			if(temp!=NULL){
				temp[dwAccBytes]=0;
				ReadFile(hFile, temp, dwAccBytes, &dwAccBytes, NULL);
			}
		}
		CloseHandle(hFile);
    }
	if((prevconfig[9]==0) || (prevconfig[9]==1 && temp==NULL)){
		PathRemoveFileSpec(str);
		if(prevconfig[6])
			PathCombine(str , str, "famicom.dat");
		else
			PathCombine(str , str, "nesdbase.dat");
		hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE){
			dwAccBytes = GetFileSize(hFile, NULL);
			if(dwAccBytes!=-1){
				temp2=(unsigned char *)malloc( dwAccBytes+1);
				if(temp2!=NULL){
					temp2[dwAccBytes]=0;
					ReadFile(hFile, temp2, dwAccBytes, &dwAccBytes, NULL);
				}
			}
			CloseHandle(hFile);
		}
	}
	while((i=ListView_GetNextItem(phList, i, LVNI_ALL)) != -1){
		str1[0]=0, flag=0;
		ListView_GetItemText(phList, i, LUNCHALLCRC, str1, 9);
		if(str1[0]==0)
			continue;
		ListView_GetItemText(phList, i, LUNCHPCRC, str2, 9);
		if(temp!=NULL){
			if(-1 != (j= NesDbSrch( (char *)temp, str1, str2, &c))){
				if(c){
					strcpy(g_pLauncherText[plistn].Resault, "HACK");
				}
				else{
					strcpy(g_pLauncherText[plistn].Resault, "GOOD");
				}
				NesDbGetI(phList, i, (char *)&temp[j]);
				flag=1;
			}
		}
		if(flag == 0 && temp2 != NULL){
			if(-1 != (j= NesDbSrch( (char *)temp2, str1, str2, &c))){
				if(c){
					strcpy(g_pLauncherText[plistn].Resault, "HACK");
				}
				else{
					strcpy(g_pLauncherText[plistn].Resault, "GOOD");
				}
				NesDbGetI(phList, i, (char *)&temp2[j]);
				flag=1;
			}
		}
		if(flag==0){
			strcpy(g_pLauncherText[i].Resault, "NO?");
//			ListView_SetItemText(phList, i, prevListArray[LUNCHCRCR], "NO?");
		}
		else{
			++listn;
			char wstr[20];
			wsprintf(wstr, "[%u / %u] NES ROM CRC check", listn, plistn);
			SetWindowText(hWnd, wstr);
		}
	}
	if(temp)
		free(temp);
	if(temp2)
		free(temp2);
	RomAndDbICmp(hWnd);
	FdsCrcCheck(hWnd);
	return 1;
}


int NesDbSrch(char *db, char *str1, char *str2, char *para){
	int i=0,j,k;
	
	while(1){
		while(db[i]==';'){
			for(;db[i]!=0;++i){
				if(db[i]==0x0d){
					if(db[i+1]==0x0a){
						i+=2;
						break;
					}
				}
			}
		}
		if(db[i]==str1[0]){
			for(j=i, k=0; db[i]!=0; ++k, ++i){
				if(db[i]!=str1[k])
					break;
			}
			if(db[i]==0)
				return -1;
			if(db[i]==';' && str1[k]==0){
				for(k=0, ++i;db[i]!=0; ++k, ++i){
					if(db[i]!=str2[k])
						break;
				}
				if(db[i]==0)
					return -1;
				if(db[i]==';' && str2[k]==0){
					*para=0;
					return i+1;
				}
				else{
					while(db[i++]!=';');
					*para=1;
					return i;
				}
			}
		}
		while(db[i]!=0){
			if(db[i]==0x0d){
				if(db[i+1]==0x0a){
					i+=2;
					break;
				}
			}
			++i;
		}
		if(db[i]==0)
			return -1;
	}
	return -1;
}


void NesDbGetI(HWND hList, int lvi, char *dbp){
	int i=0,j,k,l=0;
	char str[MAX_PATH], str2[128], tmp[16];

	j=MystrOr(dbp, str, &i);
	l+=i;
	if(prevconfig[7]){
		MystrFnIcut(str);
	}
	else{
		while(MystrCcutk(str));
	}
	strcpy(g_pLauncherText[lvi].DBTitle, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBNAME], str);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	k=atoi(str);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	i=atoi(str);
	wsprintf(str2, "%u:", (k>>4&0x0f)|(i&0xf0));
	if(k&1)
		tmp[0]='V';
	else
		tmp[0]='H';
	if(k&2)
		tmp[1]='1';
	else
		tmp[1]='0';
	if(k&4)
		tmp[2]='1';
	else
		tmp[2]='0';
	if(k&8)
		tmp[3]='1';
	else
		tmp[3]='0';
	tmp[4]=0;
	strcat(str2, tmp);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	i=atoi(str);
	wsprintf(tmp, ":%uKB:", (i*16));
	strcat(str2, tmp);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	i=atoi(str);
	wsprintf(tmp, "%uKB", (i*8));
	strcat(str2, tmp);
	strcpy(g_pLauncherText[lvi].DBInfo, str2);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBMAPP], str2);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	strcpy(g_pLauncherText[lvi].DBCountry, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBCOUN], str);
	if(j==1)
		strcpy(str, "----");
	else{
		j=MystrOr(&dbp[l], str, &i);
		l+=i;
	}
	strcpy(g_pLauncherText[lvi].DBMaker, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBMAKE], str);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	strcpy(g_pLauncherText[lvi].DBRelease, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBDATE], str);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	strcpy(g_pLauncherText[lvi].DBPrice, str);
	ListView_SetItemText(hList, lvi, LUNCHDBPRIC, str);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	strcpy(g_pLauncherText[lvi].DBGenre, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBJUN], str);
/*
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBRIDC], str);
	if(j)
		return;
/*
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBRSIZ], str);
*/
//	if(j)
//		return;
	return;
}


int MystrOr(char *orstr, char *sadr, int *len){
	int i=0;

	while(orstr[i] != '\0' && orstr[i] != ';'&& orstr[i] != 0x0d){
		sadr[i] =orstr[i];
		if(IsDBCSLeadByte(orstr[i])){
			++i;
			sadr[i] =orstr[i];
			++i;
		}
		else{
			++i;
		}
	}
	sadr[i]=0;
	*len=i+1;
	if(orstr[i] == ';')
		return 0;
	return 1;
}


void RomCrcCount(HWND hList, int *ia, int *ib){
	int i=-1;
	char str[16];
	*ia=0, *ib=0;

	while((i=ListView_GetNextItem(hList, i, LVNI_ALL)) != -1){
		str[0]=0;
		ListView_GetItemText(hList, i, LUNCHCRCR, str, 15);
		if(str[0]==0)
			continue;
		if(str[0]=='G' || str[0]=='H'){
			++*ia;
		}
		else if(str[0]=='N'){
			++*ib;
		}
	}
	return;
}


void RomCrcTout(HWND hWnd, HWND hList){
	int i=-1, j,k;
	HANDLE hFile;
	DWORD dwAccBytes;
    OPENFILENAME ofn;
	char szStr[MAX_PATH]="";
	char FileName[MAX_PATH]="", str[64];

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "DataBase Files(*.dat)\0*.dat\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = FileName;
    ofn.lpstrFileTitle = szStr;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "dat";
	LoadString(g_main_instance, IDS_STRING_CDLG_10 , str, 64);
	ofn.lpstrTitle	=str;
	ofn.lpstrInitialDir = NULL;
	if(GetSaveFileName(&ofn) == 0)
        return;
	hFile = CreateFile(FileName, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
    	hFile = CreateFile(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return;
		}
    }
	while((i=ListView_GetNextItem(hList, i, LVNI_ALL)) != -1){
/*		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHCRCR, szStr, 15);
		if(szStr[0]=='N')
			continue;*/
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHMAPP, szStr, 15);
		if(strcmp(szStr, "20")==0)
			continue;
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHALLCRC, szStr, 15);
		if(szStr[0]==0)
			continue;
		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHPCRC, szStr, 15);
		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHDBNAME, szStr, MAX_PATH);
		if(szStr[0]==0)
			ListView_GetItemText(hList, i, LUNCHFILEN, szStr, MAX_PATH);
		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHMAPP, szStr, MAX_PATH);
		k=atoi(szStr);
		j=0;
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHRINFO, szStr, MAX_PATH);
		if(szStr[0]=='V')
			j=1;
		if(szStr[1]=='1')
			j|=2;
		if(szStr[2]=='1')
			j|=4;
		if(szStr[3]=='1')
			j|=8;
		j|=(k<<4 & 0xf0);
		wsprintf(szStr, "%u", j);
		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		wsprintf(szStr, "%u", k&0xf0);
		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}		
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHPSIZE, szStr, MAX_PATH);
		for(j=0; szStr[j]!=0; ++j){
			if(szStr[j]=='K'){
				szStr[j]=0;
				break;
			}
		}
		j=atoi(szStr);
		wsprintf(szStr, "%u", j/16);
		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHCSIZE, szStr, MAX_PATH);
		for(j=0; szStr[j]!=0; ++j){
			if(szStr[j]=='K'){
				szStr[j]=0;
				break;
			}
		}
		j=atoi(szStr);
		wsprintf(szStr, "%u", j/8);
		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHDBCOUN, szStr, 15);
		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHDBMAKE, szStr, MAX_PATH);
		strcat(szStr, "\r\n");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
	}
	CloseHandle(hFile);
	return;
}


int ChengeRomH(HWND hList){
	int i=-1, flag=0;
	HANDLE hFile;
	DWORD dwAccBytes, dwAttr;
	char str1[MAX_PATH], str2[MAX_PATH], *extp;

	if((i=ListView_GetNextItem(hList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
		return 0;
	ListView_GetItemText(hList, i, LUNCHFILEN, str2, MAX_PATH);
	extp=PathFindExtension(str2);
	if(!MystrCmp(extp, ".nes"))
		return 0;
	str1[0]=0;
	ListView_GetItemText(hList, i, LUNCHHEADS, str1, MAX_PATH);
	if(str1[0]!='1')
		return 0;
	ListView_GetItemText(hList, i, LUNCHFOLDN, str1, MAX_PATH);
	PathCombine(str1 , str1, str2);
	dwAttr= GetFileAttributes(str1);
	if(dwAttr & FILE_ATTRIBUTE_READONLY){
		SetFileAttributes(str1, dwAttr^ FILE_ATTRIBUTE_READONLY);
		flag=1;
	}
	hFile = CreateFile(str1, GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		return 0;
    }
	if(-1 ==SetFilePointer(hFile, 8, 0, FILE_BEGIN)){
		CloseHandle(hFile);
		return 0;
	}
	ZeroMemory(str2, 9);
/*	ReadFile(hFile, str2, 1, &dwAccBytes, NULL);
	if(!dwAccBytes){
		CloseHandle(hFile);
		return 0;
	}
	if(-1 ==SetFilePointer(hFile, 7, 0, FILE_BEGIN)){
		CloseHandle(hFile);
		return 0;
	}
	str2[0]&=0xf0;*/
	if(!WriteFile(hFile, str2, 8, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return 0;
	}
	if(flag){
		SetFileAttributes(str1, dwAttr);
	}
	strcpy(g_pLauncherText[i].Header, "0");
//	ListView_SetItemText(hList, i, prevListArray[LUNCHHEADS], "0");
	CloseHandle(hFile);
	return 1;
}


int ChengeRomHall(HWND hList){
	int i=-1, flag=0, change=0;
	HANDLE hFile;
	DWORD dwAccBytes, dwAttr;
	char str1[MAX_PATH], str2[MAX_PATH], *extp;

	while((i=ListView_GetNextItem(hList, i, LVNI_ALL)) != -1){
		str1[0]=0;
		ListView_GetItemText(hList, i, LUNCHHEADS, str1, MAX_PATH);
		if(str1[0]!='1')
			continue;
		ListView_GetItemText(hList, i, LUNCHFILEN, str2, MAX_PATH);
		extp=PathFindExtension(str2);
		if(!MystrCmp(extp, ".nes"))
			continue;
		ListView_GetItemText(hList, i, LUNCHFOLDN, str1, MAX_PATH);
		PathCombine(str1 , str1, str2);
		dwAttr= GetFileAttributes(str1);
		if(dwAttr & FILE_ATTRIBUTE_READONLY){
			SetFileAttributes(str1, dwAttr^ FILE_ATTRIBUTE_READONLY);
			flag=1;
		}
		hFile = CreateFile(str1, GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			continue;
		}
		if(-1 ==SetFilePointer(hFile, 8, 0, FILE_BEGIN)){
			CloseHandle(hFile);
			continue;
		}
		ZeroMemory(str2, 9);
/*		ReadFile(hFile, str2, 1, &dwAccBytes, NULL);
		if(!dwAccBytes){
			CloseHandle(hFile);
			continue;
		}
		if(-1 ==SetFilePointer(hFile, 7, 0, FILE_BEGIN)){
			CloseHandle(hFile);
			continue;
		}
		str2[0]&=0xf0;*/
		if(!WriteFile(hFile, str2, 8, &dwAccBytes, NULL)){
			CloseHandle(hFile);
			continue;
		}
		if(flag){
			SetFileAttributes(str1, dwAttr);
			flag=0;
		}
		strcpy(g_pLauncherText[i].Header, "0");
//		ListView_SetItemText(hList, i, prevListArray[LUNCHHEADS], "0");
		CloseHandle(hFile);
		++change;
	}
	return change;
}


//IDD_PCONFIGDIALOG
LRESULT CALLBACK MyCheatDlg15(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch(msg) {
        case WM_INITDIALOG:
			if(prevconfig[0]){
				SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			}
			else{
				EnableWindow(GetDlgItem(hDlgWnd, IDC_PCFGCHECK4), FALSE);
			}
			SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK2), BM_SETCHECK, (WPARAM)prevconfig[1], 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK3), BM_SETCHECK, (WPARAM)prevconfig[2], 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK4), BM_SETCHECK, (WPARAM)prevconfig[3], 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK5), BM_SETCHECK, (WPARAM)prevconfig[4], 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK6), BM_SETCHECK, (WPARAM)prevconfig[6], 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK7), BM_SETCHECK, (WPARAM)prevconfig[7], 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK8), BM_SETCHECK, (WPARAM)prevconfig[8], 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK9), BM_SETCHECK, (WPARAM)prevconfig[9], 0L);
			return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
				case IDC_PCFGCHECK1:
					if(SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L)){
						EnableWindow(GetDlgItem(hDlgWnd, IDC_PCFGCHECK4), TRUE);
					}
					else{
						EnableWindow(GetDlgItem(hDlgWnd, IDC_PCFGCHECK4), FALSE);
					}
					return TRUE;
					break;
               case IDOK:
					prevconfig[0]= (char)SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK1), BM_GETCHECK, (WPARAM)TRUE, 0L);
					prevconfig[1]= (char)SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK2), BM_GETCHECK, (WPARAM)TRUE, 0L);
					prevconfig[2]= (char)SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK3), BM_GETCHECK, (WPARAM)TRUE, 0L);
					prevconfig[3]= (char)SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK4), BM_GETCHECK, (WPARAM)TRUE, 0L);
					prevconfig[4]= (char)SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK5), BM_GETCHECK, (WPARAM)TRUE, 0L);
					prevconfig[6]= (char)SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK6), BM_GETCHECK, (WPARAM)TRUE, 0L);
					prevconfig[7]= (char)SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK7), BM_GETCHECK, (WPARAM)TRUE, 0L);
					prevconfig[8]= (char)SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK8), BM_GETCHECK, (WPARAM)TRUE, 0L);
					prevconfig[9]= (char)SendMessage(GetDlgItem(hDlgWnd, IDC_PCFGCHECK9), BM_GETCHECK, (WPARAM)TRUE, 0L);
					EndDialog(hDlgWnd, IDOK);
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
					break;
			}
            break;
		case WM_CLOSE:
			EndDialog(hDlgWnd, IDCANCEL);
			return TRUE;
			break;
    }
    return FALSE;
}


int RomAndDbICmp(HWND hWnd){
	int i=-1, j, l, flag;
	char str[MAX_PATH], str1[10], str2[10];

	while((i=ListView_GetNextItem(phList, i, LVNI_ALL)) != -1){
		str[0]=0;
		ListView_GetItemText(phList, i, LUNCHDBMAPP, str, MAX_PATH);
		if(str[0]==0)
			continue;
		for(l=0; str[l]; ++l){
			if(str[l]==':')
				str[l]=';';
		}
		l=0, flag=0;
		MystrOr(&str[l], str1, &j);
		l+=j;
		ListView_GetItemText(phList, i, LUNCHMAPP, str2, MAX_PATH);
		if(strcmp(str1, str2))
			flag=1;
		MystrOr(&str[l], str1, &j);
		l+=j;
		ListView_GetItemText(phList, i, LUNCHRINFO, str2, MAX_PATH);
		if(strcmp(str1, str2))
			flag=1;
		MystrOr(&str[l], str1, &j);
		l+=j;
		ListView_GetItemText(phList, i, LUNCHPSIZE, str2, MAX_PATH);
		if(strcmp(str1, str2))
			flag=1;
		MystrOr(&str[l], str1, &j);
		l+=j;
		ListView_GetItemText(phList, i, LUNCHCSIZE, str2, MAX_PATH);
		if(strcmp(str1, str2))
			flag=1;
		if(flag){
			strcpy(g_pLauncherText[i].RomDbCmp, "!");
//			ListView_SetItemText(phList, i, prevListArray[LUNCHDBAROM], "!");
		}
		else{
			strcpy(g_pLauncherText[i].RomDbCmp, "=");
//			ListView_SetItemText(phList, i, prevListArray[LUNCHDBAROM], "=");
		}
	}
	return 1;
}


int DbToRomIChange(HWND hList){
	int change=0, i=-1, j, l, flag;
	HANDLE hFile;
	DWORD dwAccBytes, dwAttr;
	char str1[MAX_PATH], str2[MAX_PATH], str3[2], str4[10] ,*extp;

	while((i=ListView_GetNextItem(hList, i, LVNI_ALL)) != -1){
		str1[0]=0;
		ListView_GetItemText(hList, i, LUNCHDBAROM, str1, MAX_PATH);
		if(str1[0]==0 || str1[0]=='=')
			continue;
		ListView_GetItemText(hList, i, LUNCHFILEN, str1, MAX_PATH);
		extp=PathFindExtension(str1);
		if(!MystrCmp(extp, ".nes"))
			continue;
		flag=0;
		ListView_GetItemText(hList, i, LUNCHFOLDN, str2, MAX_PATH);
		PathCombine(str2 , str2, str1);
		dwAttr= GetFileAttributes(str2);
		if(dwAttr & FILE_ATTRIBUTE_READONLY){
			SetFileAttributes(str2, dwAttr^ FILE_ATTRIBUTE_READONLY);
			flag=1;
		}
		hFile = CreateFile(str2, GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			continue;
		}
		if(-1== SetFilePointer(hFile, 6, 0, FILE_BEGIN)){
			CloseHandle(hFile);
			continue;
		}
		ListView_GetItemText(hList, i, LUNCHDBMAPP, str1, MAX_PATH);
		for(l=0; str1[l]; ++l){
			if(str1[l]==':')
				str1[l]=';';
		}
		l=0;
		MystrOr(&str1[l], str4, &j);
		l+=j;
		j= atoi(str4);
		str3[1]= (char)j&0xf0;
		str3[0]= (char)j<<4&0xf0;
		MystrOr(&str1[l], str4, &j);
		l+=j;
		if(str4[0]=='V')
			str3[0]|=1;
		if(str4[1]=='1')
			str3[0]|=2;
		if(str4[2]=='1')
			str3[0]|=4;
		if(str4[3]=='1')
			str3[0]|=8;
		if(!WriteFile(hFile, str3, 2, &dwAccBytes, NULL)){
			CloseHandle(hFile);
			continue;
		}
		if(flag){
			SetFileAttributes(str2, dwAttr);
			flag=0;
		}
		CloseHandle(hFile);
		++change;
	}
	return change;
}


int FdsInfoSet(HWND hList, unsigned char *hp, int item){
	char str[10], Dcstr[MAX_PATH], crcstr[MAX_PATH]="";
	int i, j, filen=0;
	unsigned long romcrc;

	
	if(hp[0]!='F' || hp[1]!='D' || hp[2]!='S' || hp[3]!=0x1a){
		return 0;
	}
	if(hp[0x20]==0){
		str[0]=' ', str[1]=' ', str[2]=' ', str[3]=' ', str[4]=0;
	}
	else{
		str[0]=hp[0x20], str[1]=hp[0x21], str[2]=hp[0x22], str[3]=hp[0x23], str[4]=0;
	}
	strcpy(g_pLauncherText[item].Info, str);
//	ListView_SetItemText(hList, item, prevListArray[LUNCHRINFO], str);
	if(hp[0x3c] || hp[0x3d] || hp[0x3e]){
		strcpy(Dcstr, "1925/00/00");
		if(hp[0x3c]>>4<=9)
			Dcstr[2]+=(hp[0x3c]>>4);
		if(hp[0x3c]&0x0f<=9)
			Dcstr[3]+=(hp[0x3c]&0xf);
		if(hp[0x3d]>>4<=9)
			Dcstr[5]+=(hp[0x3d]>>4);
		if(hp[0x3d]&0x0f<=9)
			Dcstr[6]+=(hp[0x3d]&0xf);
		if(hp[0x3e]>>4<=9)
			Dcstr[8]+=(hp[0x3e]>>4);
		if(hp[0x3e]&0x0f<=9)
			Dcstr[9]+=(hp[0x3e]&0xf);
		strcpy(g_pLauncherText[item].CromSize, Dcstr);
//		ListView_SetItemText(hList, item, prevListArray[LUNCHCSIZE], Dcstr);
	}
	wsprintf(str, "%02u", hp[0x49]);
	strcpy(g_pLauncherText[item].PromSize, str);
//	ListView_SetItemText(hList, item, prevListArray[LUNCHPSIZE], str);
	i= 0x4A;
	while(1){
		if(hp[i]!=3)
			break;
		Dcstr[filen]=hp[15+i]+'0';
		j= hp[13+i]&0xff |(int)hp[14+i]<<8;
		romcrc= CrcCalc((unsigned char *)&hp[i+17], j);
		wsprintf(str, "%08x:", romcrc);
		strcat(crcstr, str);
		i+=j+17;
		++filen;
	}
	for(j=0;crcstr[j]!=0;++j);
	crcstr[j-1]=0;
	Dcstr[filen]=0;
	strcpy(g_pLauncherText[item].Mapper, "20");
//	ListView_SetItemText(hList, item, prevListArray[LUNCHMAPP], "20");
	strcpy(g_pLauncherText[item].PROMCRC, Dcstr);
//	ListView_SetItemText(hList, item, prevListArray[LUNCHPCRC], Dcstr);
	wsprintf(Dcstr, "%02u", filen);
	strcpy(g_pLauncherText[item].CROMCRC, Dcstr);
	strcpy(g_pLauncherText[item].AllCRC, crcstr);
//	ListView_SetItemText(hList, item, prevListArray[LUNCHCCRC], Dcstr);
//	ListView_SetItemText(hList, item, prevListArray[LUNCHALLCRC], crcstr);
	return 1;
}


int FdsCrcCheck(HWND hWnd){
	int i=-1, j, k, listn=0;
	char str[MAX_PATH], str1[10], str2[10];
	unsigned char *temp=NULL, *temp2=NULL, flag;
	HANDLE hFile;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	if(prevconfig[6])
		PathCombine(str , str, "ingredients.dat");
	else
		PathCombine(str , str, "disksys.dat");
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE){
		dwAccBytes = GetFileSize(hFile, NULL);
		if(dwAccBytes!=-1){
			temp=(unsigned char *)malloc( dwAccBytes+1);
			if(temp!=NULL){
				temp[dwAccBytes]=0;
				ReadFile(hFile, temp, dwAccBytes, &dwAccBytes, NULL);
			}
		}
		CloseHandle(hFile);
    }
	if((prevconfig[9]==0) || (prevconfig[9]==1 && temp==NULL)){
		PathRemoveFileSpec(str);
		if(prevconfig[6])
			PathCombine(str , str, "disksys.dat");
		else
			PathCombine(str , str, "ingredients.dat");
		hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE){
			dwAccBytes = GetFileSize(hFile, NULL);
			if(dwAccBytes!=-1){
				temp2=(unsigned char *)malloc( dwAccBytes+1);
				if(temp2!=NULL){
					temp2[dwAccBytes]=0;
					ReadFile(hFile, temp2, dwAccBytes, &dwAccBytes, NULL);
				}
			}
			CloseHandle(hFile);
		}
	}
	while((i=ListView_GetNextItem(phList, i, LVNI_ALL)) != -1){
		ListView_GetItemText(phList, i, LUNCHMAPP, str1, 9);
		if(0!=strcmp(str1, "20"))
			continue;
		str1[0]=0, flag=0;
		ListView_GetItemText(phList, i, LUNCHRINFO, str1, 9);
		if(str1[0]==0)
			continue;
		ListView_GetItemText(phList, i, LUNCHALLCRC, str, MAX_PATH);
		ListView_GetItemText(phList, i, LUNCHCCRC, str2, 9);
		for(k=0; str[k]; ++k){
			if(str[k]==':')
				str[k]=';';
		}
		if(temp!=NULL){
			if(-1 != (j= FdsDbSrch( (char *)temp, str1, str2, str))){
				FdsDbGetI(phList, i, (char *)&temp[j]);
				strcpy(g_pLauncherText[i].Resault, "GOOD");
//				ListView_SetItemText(phList, i, prevListArray[LUNCHCRCR], "GOOD");
				flag=1;
			}
		}
		if(temp2!=NULL && flag==0){
			if(-1 != (j= FdsDbSrch( (char *)temp2, str1, str2, str))){
				FdsDbGetI(phList, i, (char *)&temp2[j]);
				strcpy(g_pLauncherText[i].Resault, "GOOD");
//				ListView_SetItemText(phList, i, prevListArray[LUNCHCRCR], "GOOD");
				flag=1;
			}
		}
		if(flag==0){
			strcpy(g_pLauncherText[i].Resault, "NO?");
//			ListView_SetItemText(phList, i, prevListArray[LUNCHCRCR], "NO?");
		}
		else{
			++listn;
			char wstr[20];
			wsprintf(wstr, "[%u / %u] FDS image CRC check", listn,plistn);
			SetWindowText(hWnd, wstr);
		}
	}
	if(temp)
		free(temp);
	if(temp2)
		free(temp2);
	RomAndDbICmp(hWnd);
	return 1;
}


int FdsDbSrch(char *db, char *str1, char *str2, char *str3){
	int i=0,j,k;

	while(1){
		while(db[i]==';'){
			for(;db[i]!=0;++i){
				if(db[i]==0x0d){
					if(db[i+1]==0x0a){
						i+=2;
						break;
					}
				}
			}
		}
		if(db[i]==str1[0]){
			for(j=i, k=0; db[i]!=0; ++k, ++i){
				if(db[i]!=str1[k])
					break;
			}
			if(db[i]==0)
				return -1;
			else if(db[i]==';'){
				for(++i, k=0; k<4; ++i){
					if(db[i]==';'){
						++k;
					}
				}
				for(k=0;db[i]!=0; ++k, ++i){
					if(db[i]!=str2[k])
						break;
				}
				if(db[i]==0)
					return -1;
				if(db[i]==';'){
					for(k=0, ++i;db[i]!=0; ++k, ++i){
						if((db[i] >= 'A' && db[i] <= 'F' || db[i] >= 'a' && db[i] <= 'f')&&(str3[k] >= 'A' && str3[k] <= 'F' || str3[k] >= 'a' && str3[k] <= 'f')){
							if((db[i]| 0x20) != (str3[k] | 0x20)){
								break;
							}
						}
						else{
							if(db[i]!=str3[k])
								break;
						}
					}
				}
				if(db[i]==0)
					return -1;
				if(db[i]==';' && str3[k] ==0)
					return i+1;
			}
		}
		while(db[i]!=0){
			if(db[i]==0x0d){
				if(db[i+1]==0x0a){
					i+=2;
					break;
				}
			}
			++i;
		}
		if(db[i]==0)
			return -1;
	}
	return -1;
}


void FdsDbGetI(HWND hList, int lvi, char *dbp){
	int i=0, j, l=0;
	char str[MAX_PATH];

	j=MystrOr(dbp, str, &i);
	l+=i;
	if(prevconfig[7]){
		MystrFnIcut(str);
	}
	else{
		while(MystrCcutk(str));
	}
	strcpy(g_pLauncherText[lvi].DBTitle, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBNAME], str);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	strcpy(g_pLauncherText[lvi].DBCountry, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBCOUN], str);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	strcpy(g_pLauncherText[lvi].DBMaker, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBMAKE], str);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	strcpy(g_pLauncherText[lvi].DBRelease, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBDATE], str);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	strcpy(g_pLauncherText[lvi].DBPrice, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBPRIC], str);
	if(j)
		return;
	j=MystrOr(&dbp[l], str, &i);
	l+=i;
	strcpy(g_pLauncherText[lvi].DBGenre, str);
//	ListView_SetItemText(hList, lvi, prevListArray[LUNCHDBJUN], str);
	return;
}


int MystrCcutk(char *sa){
	int i=0,j, flag, n=0;

	do{
		flag = 0;
		for(;sa[i];++i){
			if(sa[i]=='"' || sa[i]=='<')
				break;
		}
		if(sa[i]==0)
			return n;
		j=i+1;
		for(;sa[j];++j){
			if(sa[j]=='"' || sa[j]=='>')
				break;
		}	
		if(sa[j]==0)
			return n;
		for(++j;sa[j];++i,++j){
			sa[i]=sa[j];
		}
		sa[i]='\0';
		flag = 1;
		n++;
		i=0;
	}while(flag == 1);
	return n;
}


int RomFileNameC(HWND hList, int *s, int *f){
	int i=-1, change=0, nchange=0;
	char str[MAX_PATH], str1[MAX_PATH], str2[MAX_PATH], str3[10], *extp, *extp2;

	while((i=ListView_GetNextItem(hList, i, LVNI_ALL)) != -1){
		str1[0]=0;
		ListView_GetItemText(hList, i, LUNCHDBNAME, str1, MAX_PATH);
		if(str1[0]==0)
			continue;
		ListView_GetItemText(hList, i, LUNCHFOLDN, str, MAX_PATH);
		ListView_GetItemText(hList, i, LUNCHFILEN, str2, MAX_PATH);
		PathCombine(str, str, str2);
		ListView_GetItemText(hList, i, LUNCHFOLDN, str2, MAX_PATH);
		if(NULL== (extp=PathFindExtension(str)))
			continue;
		if(NULL!= (extp2=PathFindExtension(str1))){
			if(MystrCmp(extp2 , ".fds"))
				PathRemoveExtension(str1);
		}
		str3[0]=0;
		ListView_GetItemText(hList, i, LUNCHDBCOUN, str3, 9);
		if(str3[0]=='J'){
			strcat(str1, "(");
			strcat(str1, str3);
			strcat(str1, ")");
		}
		if(MystrCmp(extp , ".nes" )){
			strcat(str1, ".nes");
		}
		else if(MystrCmp(extp , ".fds" )){
			strcat(str1, ".fds");
		}
		else if(MystrCmp(extp , ".zip" )){
			strcat(str1, ".zip");
		}
		else
			continue;
		PathCombine(str2, str2, str1);
		if(0==strcmp(str, str2))
			continue;
		if(MoveFile(str, str2 )){
			strcpy(g_pLauncherText[i].FileName, str1);
			++change;
		}
		else{
			++nchange;
		}
	}
	*s=change;
	*f=nchange;
	return 1;
}


void FdsCrcTout(HWND hWnd, HWND hList){
	int i=-1, k;
	HANDLE hFile;
	DWORD dwAccBytes;
    OPENFILENAME ofn;
	char szStr[MAX_PATH]="";
	char FileName[MAX_PATH]="", str[64];

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "DataBase Files(*.dat)\0*.dat\0All Files(*.*)\0*.*\0\0";
    ofn.lpstrFile = FileName;
    ofn.lpstrFileTitle = szStr;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "dat";
	LoadString(g_main_instance, IDS_STRING_CDLG_10 , str, 64);
	ofn.lpstrTitle	=str;
	ofn.lpstrInitialDir = NULL;
	if(GetSaveFileName(&ofn) == 0)
        return;
	hFile = CreateFile(FileName, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
    	hFile = CreateFile(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return;
		}
    }
	while((i=ListView_GetNextItem(hList, i, LVNI_ALL)) != -1){
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHMAPP, szStr, 5);
		if(strcmp(szStr, "20")!=0)
			continue;
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHRINFO, szStr, 7);
		strcat(szStr,";;;;;");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHCCRC, szStr, MAX_PATH);
		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHALLCRC, szStr, MAX_PATH);
		for(k=0; szStr[k]; ++k){
			if(szStr[k]==':')
				szStr[k]=';';
		}
		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
		szStr[0]=0;
		ListView_GetItemText(hList, i, LUNCHDBNAME, szStr, MAX_PATH);
		if(szStr[0]==0)
			ListView_GetItemText(hList, i, LUNCHFILEN, szStr, MAX_PATH);
		strcat(szStr, "\r\n");
//		strcat(szStr,";");
		if(!WriteFile(hFile, szStr, strlen(szStr), &dwAccBytes, NULL)){
			CloseHandle(hFile);
			return;
		}
	}
	CloseHandle(hFile);
	return;
}

void LunchAutoSSave(){
	char str[MAX_PATH];
	WIN32_FIND_DATA fd;
	HANDLE hFind;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "nnnssave");
	hFind = FindFirstFile(str, &fd);
	if(hFind == INVALID_HANDLE_VALUE || (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))){
		CreateDirectory(str , NULL);
	}
	FindClose(hFind);
	PathCombine(str , str, emu->getROMname());
	strcat(str, ".ss0");
	emu->saveState(str);
	return;
}


void LunchAutoSLoad(){
	char str[MAX_PATH];

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "nnnssave");
	PathCombine(str , str, emu->getROMname());
	strcat(str, ".ss0");
	emu->loadState(str);
	return;
}

void MyListVclr(int j, int k){
	int i=-1;
	while((i=ListView_GetNextItem(phList, i, LVNI_ALL)) != -1){
		strcpy(g_pLauncherText[i].DBInfo, "");
		strcpy(g_pLauncherText[i].RomDbCmp, "");
		strcpy(g_pLauncherText[i].DBTitle, "");
		strcpy(g_pLauncherText[i].DBCountry, "");
		strcpy(g_pLauncherText[i].DBMaker, "");
		strcpy(g_pLauncherText[i].DBRelease, "");
		strcpy(g_pLauncherText[i].DBPrice, "");
		strcpy(g_pLauncherText[i].DBGenre, "");
	}
	return;
}

void LauncherWindowTset(HWND hWnd){
	char wstr[30];
	wsprintf(wstr, LNCHWTITLE, plistn);
	SetWindowText(hWnd, wstr);
	return;
}

int ChengeRomoInfo(HINSTANCE hInst, HWND hWnd){
	int i=-1, flag=0, l, j;
	HANDLE hFile;
	DWORD dwAccBytes, dwAttr;
	char str[MAX_PATH], str1[MAX_PATH], str2[MAX_PATH], *extp;

	if((i=ListView_GetNextItem(phList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
		return 2;
	str2[0]=0;
	ListView_GetItemText(phList, i, LUNCHFILEN, str2, MAX_PATH);
	extp=PathFindExtension(str2);
	if(!MystrCmp(extp, ".nes"))
		return 2;
	str[0]=0;
	ListView_GetItemText(phList, i, LUNCHRINFO, str, MAX_PATH);
	if(str[0] == 0)
		return 2;
	MyCheatNum = 0;
	if(str[0]=='V')
		MyCheatNum |= 1;
	if(str[1]=='1')
		MyCheatNum |= 2;
	if(str[2]=='1')
		MyCheatNum |= 4;
	if(str[3]=='1')
		MyCheatNum |= 8;
	ListView_GetItemText(phList, i, LUNCHMAPP, str, MAX_PATH);
	MyCheatAdr= atoi(str);
	if(IDCANCEL ==DialogBox(hInst, MAKEINTRESOURCE(IDD_CTDIALOG15), hWnd, (DLGPROC)MyCheatDlg19))
		return 2;
	ListView_GetItemText(phList, i, LUNCHFOLDN, str1, MAX_PATH);
	PathCombine(str1 , str1, str2);
	dwAttr= GetFileAttributes(str1);
	if(dwAttr & FILE_ATTRIBUTE_READONLY){
		SetFileAttributes(str1, dwAttr^ FILE_ATTRIBUTE_READONLY);
		flag=1;
	}
	hFile = CreateFile(str1, GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		return 0;
    }
	if(-1 ==SetFilePointer(hFile, 6, 0, FILE_BEGIN)){
		CloseHandle(hFile);
		return 0;
	}
	str2[0] = (unsigned char)(((MyCheatAdr <<4) & 0xf0)|(MyCheatNum&0x0f));
	str2[1] = (unsigned char)(MyCheatAdr& 0xf0);
	if(!WriteFile(hFile, str2, 2, &dwAccBytes, NULL)){
		CloseHandle(hFile);
		return 0;
	}
	if(flag){
		SetFileAttributes(str1, dwAttr);
	}
	CloseHandle(hFile);
	if(MyCheatNum&1)
		str[0]='V';
	else
		str[0]='H';
	if(MyCheatNum&2)
		str[1]='1';
	else
		str[1]='0';
	if(MyCheatNum&4)
		str[2]='1';
	else
		str[2]='0';
	if(MyCheatNum&8)
		str[3]='1';
	else
		str[3]='0';
	str[4]=0;
	strcpy(g_pLauncherText[i].Info, str);
	wsprintf(str, "%u", MyCheatAdr);
	strcpy(g_pLauncherText[i].Mapper, str);
	str[0]=0;
	ListView_GetItemText(phList, i, LUNCHDBMAPP, str, MAX_PATH);
	if(str[0]==0)
		return 1;
	for(l=0; str[l]; ++l){
		if(str[l]==':')
			str[l]=';';
	}
	l=0, flag=0;
	MystrOr(&str[l], str1, &j);
	l+=j;
	ListView_GetItemText(phList, i, LUNCHMAPP, str2, MAX_PATH);
	if(strcmp(str1, str2))
		flag=1;
	MystrOr(&str[l], str1, &j);
	l+=j;
	ListView_GetItemText(phList, i, LUNCHRINFO, str2, MAX_PATH);
	if(strcmp(str1, str2))
		flag=1;
	MystrOr(&str[l], str1, &j);
	l+=j;
	ListView_GetItemText(phList, i, LUNCHPSIZE, str2, MAX_PATH);
	if(strcmp(str1, str2))
		flag=1;
	MystrOr(&str[l], str1, &j);
	l+=j;
	ListView_GetItemText(phList, i, LUNCHCSIZE, str2, MAX_PATH);
	if(strcmp(str1, str2))
		flag=1;
	if(flag){
		strcpy(g_pLauncherText[i].RomDbCmp, "!");
	}
	else{
		strcpy(g_pLauncherText[i].RomDbCmp, "=");
	}
	return 1;
}


void LauncherListNoRemove(HWND hwnd){
	int i=-1;
	char str1[16];

	char str2[64], str3[64];
	LoadString(g_main_instance, IDS_STRING_LMSG_05 , str2, 64);
	LoadString(g_main_instance, IDS_STRING_LMSG_13 , str3, 64);
	if(IDCANCEL== MessageBox(hwnd,(LPCSTR)str3,(LPCSTR)str2,MB_OKCANCEL))
		return;
	while((i=ListView_GetNextItem(phList, i, LVNI_ALL)) != -1){
		str1[0]=0;
		ListView_GetItemText(phList, i, LUNCHCRCR, str1, 15);
		if(str1[0]=='G')
			continue;
		ListView_DeleteItem(phList, i);
		i=-1;
		--plistn;
	}
	return;
}


//////////////////  Kaillera ///////////

extern HACCEL hAccel;
extern void EmuError(const char* );
extern void FreeROM();


int WINAPI kGameCallback(char *game, int player, int maxplayers){
	{
		int i=-1;
		char flag=0, tstr[MAX_PATH], cstr[10];

//		if(nnnKailleraFlag)
//			return 0;

		while((i=ListView_GetNextItem(phList, i, LVNI_ALL)) != -1){
			tstr[0]=0;
			ListView_GetItemText(phList, i, LUNCHDBNAME, tstr, MAX_PATH);
			if(tstr[0]==0)
				continue;
			cstr[0]=0;
			ListView_GetItemText(phList, i, LUNCHDBCOUN, cstr, 10);
			if(cstr[0]!=0){
				strcat(tstr, "[");
				strcat(tstr, cstr);
				strcat(tstr, "]");
			}
			if(0==strcmp(game, tstr)){
				ListView_GetItemText(phList, i, LUNCHFOLDN, previewfn, MAX_PATH);
				ListView_GetItemText(phList, i, LUNCHFILEN, tstr, MAX_PATH);
				PathCombine(previewfn, previewfn, tstr);
//				SetTimer(previewh, ID_SBTIMER, 150, NULL);
				flag=1;
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

	MSG msg;
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
#ifdef TOOLTIP_HACK
				if(msg.message != 0x0118)
#endif
					if(!TranslateAccelerator(main_window_handle, hAccel, &msg)){
						TranslateMessage(&msg);
						DispatchMessage(&msg);
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
#ifdef TOOLTIP_HACK
				if(msg.message != 0x0118)
#endif
					if(!TranslateAccelerator(main_window_handle, hAccel, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
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
//	MamePlayGame();
//	kPlay=0;
	nnnKailleraFlag=0;
	nnnkailleraEndGame();
	if(emu)
		FreeROM();
	return 0;
}


void WINAPI kchatReceived(char *nick, char *text){
	return;
}

void WINAPI kclientDropped(char *nick, int playernb){
	return;
}

void WINAPI kmoreInfos(char *gamename){
	return;
}





//////  Dialog ////////

//  IDD_CTDIALOG18
LRESULT CALLBACK MyCheatDlg23(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	DWORD i,j;

    switch(msg) {
		case WM_COMMAND:
			switch (LOWORD(wp)){
				case IDOK:
				   for(i=0, j=IDC_CHECK1; i<LUNCHCOLMAX ;++i,++j){
					   if(SendMessage(GetDlgItem(hDlgWnd, j), BM_GETCHECK, (WPARAM)FALSE, 0L)){
						   if(5>ListView_GetColumnWidth(phList, i)){
								ListView_SetColumnWidth(phList, i, 70);
						   }
					   }
					   else{
						   ListView_SetColumnWidth(phList, i, 0);
					   }
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
		case WM_INITDIALOG:
			for(i=0, j=IDC_CHECK1; i<LUNCHCOLMAX ;++i,++j){
				if(5>ListView_GetColumnWidth(phList, i))
					SendMessage(GetDlgItem(hDlgWnd, j), BM_SETCHECK, (WPARAM)FALSE, 0L);
				else
					SendMessage(GetDlgItem(hDlgWnd, j), BM_SETCHECK, (WPARAM)TRUE, 0L);
			}
			return TRUE;
			break;
	}
	return FALSE;
}


//IDD_CTDIALOG19
LRESULT CALLBACK MyCheatDlg24(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{

    switch(msg) {
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					MyCheatAdr=SendMessage(GetDlgItem(hDlgWnd, IDC_RADIO2), BM_GETCHECK, (WPARAM)TRUE, 0L);
					MyCheatNum=SendMessage(GetDlgItem(hDlgWnd, IDC_RADIO4), BM_GETCHECK, (WPARAM)TRUE, 0L);
					EndDialog(hDlgWnd, IDOK);
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
        case WM_INITDIALOG:
			SendMessage(GetDlgItem(hDlgWnd, IDC_RADIO1), BM_SETCHECK, (WPARAM)TRUE, 0L);
			SendMessage(GetDlgItem(hDlgWnd, IDC_RADIO3), BM_SETCHECK, (WPARAM)TRUE, 0L);
            return TRUE;
    }
    return FALSE;
}


//NES Header Info Change   IDD_CTDIALOG15
LRESULT CALLBACK MyCheatDlg19(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	int i;
	char *str[]={"0","1","H","V"};
	char gstr[10];

    switch(msg) {
		case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					GetDlgItemText(hDlgWnd, IDC_CT15EDIT1, (LPTSTR)gstr, 10);
					if(gstr[0]==0)
						break;
					if((MyCheatAdr = atoi(gstr))>=0x100){
						char str2[64], str3[64];
						LoadString(g_main_instance, IDS_STRING_CHTM_02 , str2, 64);
						LoadString(g_main_instance, IDS_STRING_CHTM_09 , str3, 64);
						MessageBox(hDlgWnd,(LPCSTR)str2,(LPCSTR)str3,MB_OK);
						break;
					}
	            	{
						char str2[64], str3[64];
						LoadString(g_main_instance, IDS_STRING_LMSG_14 , str2, 64);
						LoadString(g_main_instance, IDS_STRING_LMSG_05 , str3, 64);
						MessageBox(hDlgWnd,(LPCSTR)str2,(LPCSTR)str3,MB_OK);
	            		if(IDCANCEL== MessageBox(hDlgWnd,(LPCSTR)str2,(LPCSTR)str3,MB_OKCANCEL))
		            		break;
	            	}
					MyCheatNum=0;
					if(SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO1, CB_GETCURSEL,0L,0L))
						MyCheatNum |= 1;
					if(SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO2, CB_GETCURSEL,0L,0L))
						MyCheatNum |= 2;
					if(SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO3, CB_GETCURSEL,0L,0L))
						MyCheatNum |= 4;
					if(SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO4, CB_GETCURSEL,0L,0L))
						MyCheatNum |= 8;
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
        case WM_INITDIALOG:
			wsprintf(gstr, "%u", MyCheatAdr);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_CT15EDIT1), (char *)gstr);
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO1, CB_INSERTSTRING,(WPARAM)0,(LPARAM)str[2]);
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO2, CB_INSERTSTRING,(WPARAM)0,(LPARAM)str[0]);
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO3, CB_INSERTSTRING,(WPARAM)0,(LPARAM)str[0]);
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO4, CB_INSERTSTRING,(WPARAM)0,(LPARAM)str[0]);
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO1, CB_INSERTSTRING,(WPARAM)1,(LPARAM)str[3]);
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO2, CB_INSERTSTRING,(WPARAM)1,(LPARAM)str[1]);
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO3, CB_INSERTSTRING,(WPARAM)1,(LPARAM)str[1]);
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO4, CB_INSERTSTRING,(WPARAM)1,(LPARAM)str[1]);
			if(MyCheatNum&1)
				i=1;
			else
				i=0;
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO1, CB_SETCURSEL,(WPARAM)i,0);
			if(MyCheatNum&2)
				i=1;
			else
				i=0;
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO2, CB_SETCURSEL,(WPARAM)i,0);
			if(MyCheatNum&4)
				i=1;
			else
				i=0;
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO3, CB_SETCURSEL,(WPARAM)i,0);
			if(MyCheatNum&8)
				i=1;
			else
				i=0;
			SendDlgItemMessage(hDlgWnd, IDC_CT15COMBO4, CB_SETCURSEL,(WPARAM)i,0);
            return TRUE;
    }
    return FALSE;
}


//FDS File List       IDD_DIALOG1
LRESULT CALLBACK LauncherFDSSaveClearDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp){
	DWORD dwStyle;
	LV_COLUMN lvcol;
	static HFONT hlFont= NULL;
	static unsigned char *fbp=NULL;
	static HANDLE hFile;
	static DWORD fsize;
	static HWND hList;
    LV_ITEM item;
	char str[MAX_PATH], str2[MAX_PATH], *extp=NULL;
	int i,j,k;
	DWORD dwAccBytes;

    switch(msg) {
        case WM_INITDIALOG:
			hList = GetDlgItem(hDlg, IDC_LIST1);
			i=-1;
			if((i=ListView_GetNextItem(phList, i, LVNI_ALL | LVNI_SELECTED)) == -1){
				EndDialog(hDlg, IDCANCEL);
				return TRUE;
			}
			ListView_GetItemText(phList, i, LUNCHFOLDN, str, MAX_PATH);
			ListView_GetItemText(phList, i, LUNCHFILEN, str2, MAX_PATH);
			extp=PathFindExtension(str2);
			if(extp==NULL){
				EndDialog(hDlg, IDCANCEL);
				return TRUE;
			}
			++extp;
			if(!MystrCmp(extp, "fds")){
				EndDialog(hDlg, IDCANCEL);
				return TRUE;
			}
			PathCombine(str, str, str2);
			i= GetFileAttributes(str);
			if(i & FILE_ATTRIBUTE_READONLY){
				SetFileAttributes(str, i^ FILE_ATTRIBUTE_READONLY);
			}
			hFile = CreateFile(str, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile == INVALID_HANDLE_VALUE)
				EndDialog(hDlg, IDCANCEL);
			fsize = GetFileSize(hFile, NULL);
			fbp = (unsigned char *)malloc(fsize+1);
			if(fbp==NULL){
				CloseHandle(hFile);
				EndDialog(hDlg, IDCANCEL);
				return TRUE;
			}
			ReadFile(hFile, fbp, fsize, &dwAccBytes, NULL);

			dwStyle = ListView_GetExtendedListViewStyle(hList);
			dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES ;
			ListView_SetExtendedListViewStyle(hList, dwStyle);

			lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvcol.fmt = LVCFMT_RIGHT; //LVCFMT_LEFT
			lvcol.cx = 55;
			lvcol.pszText = "Address";
			lvcol.iSubItem = 0;
			ListView_InsertColumn(hList, 0, &lvcol);
			lvcol.fmt = LVCFMT_RIGHT;
			lvcol.cx = 50;
			lvcol.pszText = "side";
			lvcol.iSubItem = 1;
			ListView_InsertColumn(hList, 1, &lvcol);
			lvcol.pszText = "FileName";
			lvcol.iSubItem = 2;
			ListView_InsertColumn(hList, 2, &lvcol);
			lvcol.pszText = "Size";
			lvcol.iSubItem = 3;
			ListView_InsertColumn(hList, 3, &lvcol);
			lvcol.pszText = "Type";
			lvcol.iSubItem = 4;
			ListView_InsertColumn(hList, 4, &lvcol);
			hlFont = CreateFontIndirect(&CheatDlgFont);
			ListView_SetTextColor(hList, CheatDlgFontColor);

			if(hlFont!=NULL){
				SendMessage(hList, WM_SETFONT, (WPARAM)hlFont, MAKELPARAM(TRUE, 0));
			}
			{
			int dn=(int)fbp[4];
			int in=0;
			for(j=0;j<dn;++j){
				for(i=0x4a;1;){
					int t=fbp[(j*65500)+i];
					if(t!=3){
						break;
					}
					k= fbp[13+i+(j*65500)]&0xff |(int)fbp[14+i+(j*65500)]<<8;
					{
						char tstr[256];
						wsprintf(tstr, "%08x", j*65500+i);
						item.mask = LVIF_TEXT | LVIF_PARAM;
						item.pszText = tstr;
						item.iItem = in;
						item.iSubItem = 0;
						item.lParam = in;
						ListView_InsertItem(hList, &item);
						item.mask = LVIF_TEXT;
						wsprintf(tstr, "%u", j);
						item.iSubItem = 1;
						ListView_SetItem(hList, &item);
						item.mask = LVIF_TEXT;
						unsigned char *tp=&fbp[(j*65500)+i+3];
						memcpy(tstr, tp, 8);
						tstr[8]=0;
						item.iSubItem = 2;
						item.mask = LVIF_TEXT;
						ListView_SetItem(hList, &item);
						wsprintf(tstr, "%u", k);
						item.iSubItem = 3;
						ListView_SetItem(hList, &item);
						wsprintf(tstr, "%u", fbp[(j*65500)+i+15]);
						item.iSubItem = 4;
						ListView_SetItem(hList, &item);
						++in;
					}
					i+=(k+17);
				}
			}
			}
			return TRUE;
		case WM_COMMAND:
            switch (LOWORD(wp)) {
				case IDC_BUTTON1:
					i=-1;
					if((i=ListView_GetNextItem(hList, i, LVNI_ALL | LVNI_SELECTED)) == -1)
						return TRUE;
					{
						DWORD size, offset;
						ListView_GetItemText(hList, i, 0, str, 30);
						MyStrAtoh(str, &offset);
						ListView_GetItemText(hList, i, 3, str, 30);
						size = atoi(str);
						memset(&fbp[offset+0x11], 0, size);
					}
	            	{
						char str2[64], str3[64];
						LoadString(g_main_instance, IDS_STRING_LMSG_16 , str2, 64);
						LoadString(g_main_instance, IDS_STRING_LMSG_15 , str3, 64);
						MessageBox(hDlg,(LPCSTR)str2,(LPCSTR)str3,MB_OK);
	            	}
					return TRUE;
					break;
				case IDCANCEL:
					CloseHandle(hFile);
					free(fbp);
			 		DeleteObject(hlFont);
					EndDialog(hDlg, IDCANCEL);
					return TRUE;
					break;
				case IDOK:
						{
							char str2[64], str3[64];
							LoadString(g_main_instance, IDS_STRING_LMSG_06 , str2, 64);
							LoadString(g_main_instance, IDS_STRING_LMSG_05 , str3, 64);
							MessageBox(hDlg,(LPCSTR)str2,(LPCSTR)str3,MB_OK);
							if(IDOK== MessageBox(hDlg,(LPCSTR)str2, (LPCSTR)str3, MB_OKCANCEL)){
								SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
								if(WriteFile(hFile, fbp, fsize, &dwAccBytes, NULL))
									LoadString(g_main_instance, IDS_STRING_LMSG_10 , str2, 64);
									LoadString(g_main_instance, IDS_STRING_LMSG_09 , str3, 64);
									MessageBox(hDlg,(LPCSTR)str2,(LPCSTR)str3,MB_OK);
							}
						}
						return TRUE;
						break;
			}
			return TRUE;
		}
		return FALSE;
}
