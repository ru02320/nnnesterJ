

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <shlobj.h>
#include <commdlg.h>
#include <stddef.h>

#include "resource.h"
#include "win32_dialogs.h"
#include "win32_datach_barcode_dialog.h"

#include "win32_emu.h"
#include "settings.h"

#include "debug.h"

#include "cheat.h"
#include "preview.h"
#include "launcher.h"
#include "nnndialog.h"
#include "savecfg.h"
#include "arc.h"
#include "crc32.h"
#include "CheckFile.h"



#define ID_PROGRESSTIMER 200



extern HWND main_window_handle;
extern HINSTANCE g_main_instance;
extern emulator* emu;


struct Preview_state g_Preview_State;

extern int Mainwindowsbhs;
extern HWND Mainwindowsbh;
extern HWND Mainwindow_Preview;
extern int g_PreviewMode;
extern int g_PreviewLClickItemn;

extern int MyStrAtoh(char *, DWORD *);
extern int MystrFnIcut(char *);
extern int MystrCcutk(char *);



extern void SetToolWindowItemState();
extern int MystrCmp(char *,char *);

const char *GenreStr[] = { "ACT", "ADV", "CACT", "EDU", "ETC", "PZL", "QUIZ", "RPG",
		"SLG", "SPT", "STG", "TAB", "TRPG", NULL};


TBBUTTON tbstruct[] = {
	{0, IDS_STRING1, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
	{1, IDS_STRING2, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
	{2, IDS_STRING3, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{3, IDS_STRING4, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{4, IDS_STRING5, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
	{5, IDS_STRING6, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
	{6, IDS_STRING7, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
	{7, IDS_STRING8, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{8, IDS_STRING9, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{9, IDS_STRING10, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
	{10, IDS_STRING11, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{11, IDS_STRING12, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{12, IDS_STRING13, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{13, IDS_STRING14, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{14, IDS_STRING15, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{15, IDS_STRING16, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{16, IDS_STRING17, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{17, IDS_STRING18, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{18, IDS_STRING19, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0}
};


TBBUTTON tbSep = {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0, 0};


#define PREVIEW_TOOLBAR_BTNNUM 19

HWND CreateToolWnd(HWND hWnd, HINSTANCE hInst){
	HWND hTool, hToolTip;
	static TBADDBITMAP tbab;
	static TOOLINFO ti;
	static int stdid, id;
	UINT uToolType;

	hTool = CreateToolbarEx( hWnd,/*WS_BORDER |*/ WS_CHILD | WS_VISIBLE,
		ID_MYTOOLBAR, PREVIEW_TOOLBAR_BTNNUM, hInst, IDR_TOOLBAR1, (LPCTBBUTTON)tbstruct, PREVIEW_TOOLBAR_BTNNUM, 0, 0,
		16, 16, sizeof(TBBUTTON));

	SendMessage(hTool, TB_INSERTBUTTON, 2, (LPARAM)&tbSep);
	SendMessage(hTool, TB_INSERTBUTTON, 5, (LPARAM)&tbSep);
	SendMessage(hTool, TB_INSERTBUTTON, 9, (LPARAM)&tbSep);
	SendMessage(hTool, TB_INSERTBUTTON, 12, (LPARAM)&tbSep);
	SendMessage(hTool, TB_INSERTBUTTON, 16, (LPARAM)&tbSep);
	SendMessage(hTool, TB_INSERTBUTTON, 20, (LPARAM)&tbSep);
	SendMessage(hTool, TB_INSERTBUTTON, 24, (LPARAM)&tbSep);
	SendMessage(hTool, TB_AUTOSIZE, 0, 0);

	hToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, /*TTS_ALWAYSTIP*/ TTS_NOPREFIX ,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hWnd, NULL, hInst, NULL);

	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hTool;
	ti.hinst = hInst;
	ti.uFlags = TTF_SUBCLASS;

	for (id = 0; id < PREVIEW_TOOLBAR_BTNNUM; id++) {
		SendMessage(hTool, TB_GETITEMRECT, id, (LPARAM)&ti.rect);
		ti.uId = tbstruct[id].idCommand;
		ti.lpszText = (LPTSTR)tbstruct[id].idCommand;
		SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	}
	SendMessage(hTool, TB_SETTOOLTIPS, (WPARAM)hToolTip, 0);
	uToolType = (UINT)GetWindowLong(hTool, GWL_STYLE);
	uToolType |= (TBSTYLE_FLAT);
	SetWindowLong(hTool, GWL_STYLE, (LONG)uToolType);
	g_Preview_State.hToolTip = hToolTip;
	g_Preview_State.hTool = hTool;
	{
		RECT rc;
		GetWindowRect(hTool, &rc);
		g_Preview_State.nToolTipHeight =  rc.bottom - rc.top;
	}
	InvalidateRect(hTool, NULL, TRUE);
	return hTool;
}




int MyFolderOpenDlg(HWND hWnds, char *folds){
	char str[64];
	BROWSEINFO  bi;
	ITEMIDLIST  *idl;
	LPMALLOC	g_pMalloc;
	SHGetMalloc(&g_pMalloc);
	bi.hwndOwner		=hWnds;
	bi.pidlRoot		 =NULL;
	bi.pszDisplayName   =folds;
	LoadString(g_main_instance, IDS_STRING_CDLG_14 , str, 64);
	bi.lpszTitle	=str;
	bi.ulFlags		  =BIF_RETURNONLYFSDIRS;
	bi.lpfn			 =NULL; //(BFFCALLBACK)FolderOpens;
	bi.lParam		   =NULL; //(LPARAM)OpenFolder;
	bi.iImage		   =0;
	idl=SHBrowseForFolder(&bi);
	if(idl == NULL){
		return 0;
	}
	if(SHGetPathFromIDList(idl,folds) == FALSE){
		return 0;
	}
	g_pMalloc->Free(idl);
//	strcpy(OpenFolder, folds);
	return 1;
}


int MyFileOpenDlg(HWND hWnd, char *fn){
	OPENFILENAME ofn;
	char szFile[MAX_PATH]="";
	char FileName[MAX_PATH]="";
	char str[64];

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "NES,FDS files\0*.nes;*.fds\0arc files\0*.zip;*.lzh;*.rar;*.gca\0All files(*.*)\0*.*\0\0";
	ofn.lpstrFile = FileName;
	ofn.lpstrFileTitle = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = NULL;
 	LoadString(g_main_instance, IDS_STRING_CDLG_15 , str, 64);
	ofn.lpstrTitle = str;
	ofn.lpstrInitialDir = NULL; //OpenFolder;
	if(GetOpenFileName(&ofn) == 0)
		return 0;
	strcpy(fn, FileName);
	return 1;
}




void CheckMessage(){
	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


void MyWaitGetMessage(){
	MSG msg;
	if(GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


static int CancelButton=0;
static int nProgressState=0;
static char ProgressFileName[MAX_PATH];


//IDD_PROGRESSDIALOG
// 経過表示
LRESULT CALLBACK MyProgressDlg(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){
	static int nMax;

	switch(msg){
	case WM_INITDIALOG:
		{
			HWND hProg;
			nMax = (int)lp;
			CancelButton = 0;
			hProg = GetDlgItem(hWnd, IDC_PROGRESS1);
			SendMessage(hProg, PBM_SETRANGE,(WPARAM)0,MAKELPARAM(0, nMax));
			SetTimer(hWnd, ID_PROGRESSTIMER, 100, NULL);
		}
		return TRUE;
	case WM_TIMER:
		{
			char s[64];
			SendMessage(GetDlgItem(hWnd, IDC_PROGRESS1), PBM_SETPOS, (WPARAM)nProgressState, 0);
			wsprintf(s, "%u / %u", nProgressState, nMax);
			SetWindowText(GetDlgItem(hWnd, IDC_STATIC1), s);
			SetWindowText(GetDlgItem(hWnd, IDC_STATIC2), ProgressFileName);
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wp)){
			case IDC_BUTTON1:
//				KillTimer(hWnd, ID_PROGRESSTIMER);
				CancelButton = 1;
				DestroyWindow(hWnd);
				break;
		}
		return TRUE;
	case WM_DESTROY:
		KillTimer(hWnd, ID_PROGRESSTIMER);
		return TRUE;
	}
	return FALSE;
}



int MyPreviewCheckROMFile(char *fn, struct NES_ROM_Data *ROM_data){
//	const char *nesExtensions[] = { "*.nes","*.fds","*.fam","*.unf","*.nsf",NULL };
//	const char *arcExtensions[] = { "zip","lzh","rar","gca",NULL };

	unsigned char *temp=NULL;
	HANDLE hFile;
	DWORD dwAccBytes, filesize, headerid;
	int arcflag=0, result=0, romtype;

#if 1	//	file extension check
	{
//		int i;

		romtype = CheckROMFile(fn, main_window_handle, &arcflag);
		if(romtype==-1)
			return 0;
#if 0
		char *extp = PathFindExtension(fn);
		if(extp==NULL)
			return 0;
		extp++;
		for(i=0; arcExtensions[i]; i++){
			if(MystrCmp(extp, (char *)arcExtensions[i]))
				break;
		}
		if(arcExtensions[i]){
			arcflag=1;
		}
#endif
	}
#endif

	if(!arcflag){
		hFile = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			return 0;
		}
		filesize = GetFileSize(hFile, NULL);
		temp = (unsigned char *)malloc(filesize);
		if(temp==NULL){
			CloseHandle(hFile);
			return 0;
		}
		ReadFile(hFile, temp, filesize, &dwAccBytes, NULL);
		CloseHandle(hFile);
	}
	else{
//		if(!Uncompress(Mainwindow_Preview, &temp, fn, &filesize, nesExtensions)){
		if(!UncompressROMFile(fn, main_window_handle, &temp, &filesize, romtype)){
			return 0;
		}
	}

	headerid = *((DWORD *)&temp[0]);
	if(romtype == EMUTYPE_NES){
		if(headerid==0x1A53454E){	//NES
			DWORD romsize;
			romsize = temp[4] * 0x4000;
			romsize += temp[5] * 0x2000;
			if(temp[6]&4)
				romsize+=512;
			if(g_Preview_State.NESROM_CRCFrom_Filesize || (romsize > (filesize-0x10))){
				romsize = filesize-0x10;
			}
			ROM_data->Crc = CrcCalc(temp+0x10, romsize);
			romsize = temp[4] * 0x4000;
			if(temp[6]&4)
				romsize+=512;
			if((filesize-0x10) >= romsize)
				ROM_data->PROM_Crc = CrcCalc(temp+0x10, romsize);
			else
				ROM_data->PROM_Crc = 0;
			ROM_data->PROM_Size = temp[4];
			ROM_data->CROM_Size = temp[5];
			ROM_data->HeaderInfo1 = temp[6];
			ROM_data->HeaderInfo2 = temp[7];
			ROM_data->Flag = 1;
			result = 1;
		}
		else if(headerid==0x1A534446){	//FDS
			DWORD blocksize;
			int j=0, i = 0x4A;
			while((unsigned int)i<filesize){
				if(temp[i]!=3)
					break;
				blocksize= temp[13+i]&0xff |(int)temp[14+i]<<8;
				ROM_data->FDS_Crc[j++]= CrcCalc((unsigned char *)&temp[i+17], blocksize);
				i+=blocksize+17;
			}
			ROM_data->HeaderInfo1 = j;
			ROM_data->Flag = 3;
			result = 2;
		}
	}
	else if(romtype == EMUTYPE_PCE){
		DWORD romsize;
		romsize = filesize;
		ROM_data->Crc = CrcCalc(temp, romsize);
		ROM_data->PROM_Crc = 0;
		ROM_data->PROM_Size = 0;
		ROM_data->CROM_Size = 0;
		ROM_data->HeaderInfo1 = 0;
		ROM_data->HeaderInfo2 = 0;
		ROM_data->Flag = 7;
		result = 4;
	}
	if(temp){
		free(temp);
		temp = NULL;
	}
	return result;
}

/*
int Preview_Folderadd(HWND hWnd){
	char	fn[MAX_PATH], dirname[MAX_PATH];
	int		i=0,j=0;
	HANDLE	hFind;
	WIN32_FIND_DATA	fd;

	if(!MyFolderOpenDlg(hWnd, dirname))
		return 0;
	strcpy(fn, dirname);
	PathCombine(dirname, dirname, "*.*");
	hFind = FindFirstFile(dirname, &fd);
	if(hFind == INVALID_HANDLE_VALUE){
		FindClose(hFind);
		return 0;
	}
	do{
		if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			char tmp[MAX_PATH];
			strcpy(tmp, fn);
			PathCombine(tmp, tmp, fd.cFileName);
			Add_PreviewListToFile(tmp);
		}
		CheckMessage();
	}while(FindNextFile(hFind, &fd));
	FindClose(hFind);
	return 1;
}





void Preview_sFolderadd(char *dirname){
	char	fn[MAX_PATH], dirstr[MAX_PATH];
	int		i=0,j=0;
	HANDLE	hFind;
	WIN32_FIND_DATA		fd;

	strcpy(fn, dirname);
	PathCombine(dirname, dirname, "*.*");
	hFind = FindFirstFile(dirstr, &fd);
	if(hFind == INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return;
	}
	do {
		if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			Add_PreviewListToFile(fd.cFileName);
		}
		else{
			if(fd.cFileName[0] != '.'){
				char tmp[MAX_PATH];
				strcpy(tmp, fn);
				PathCombine(tmp, tmp, fd.cFileName);
				Add_PreviewListToFile(tmp);
			}
		}
		CheckMessage();
	}while(FindNextFile(hFind, &fd));
	FindClose(hFind);
	return;
}
*/


int MyFoldFileSrch(char *subd)
{
	char	dirstr[MAX_PATH];
	int		i=0,j=0;
	HANDLE	hFind;
	WIN32_FIND_DATA		fd;

	strcpy(dirstr, subd);
	PathCombine(dirstr, dirstr, "*.*");
	hFind = FindFirstFile(dirstr, &fd);
	if(hFind == INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return 0;
	}
	do {
		if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			char filestr[MAX_PATH];
			PathCombine(filestr, subd, fd.cFileName);
			Add_PreviewListToFile(filestr);
			strcpy(ProgressFileName, filestr);
		}
		CheckMessage();
	} while(FindNextFile(hFind, &fd) && !CancelButton);
	FindClose(hFind);
	return 1;
}


int MyFoldFileSrchS(char *subd)
{
	char	dirstr[MAX_PATH];
	int		i=0,j=0;
	HANDLE	hFind;
	WIN32_FIND_DATA		fd;

	strcpy(dirstr, subd);
	PathCombine(dirstr, dirstr, "*.*");
	hFind = FindFirstFile(dirstr, &fd);
	if(hFind == INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return 0;
	}

	do {
		if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			char filestr[MAX_PATH];
			PathCombine(filestr, subd, fd.cFileName);
			Add_PreviewListToFile(filestr);
			strcpy(ProgressFileName, filestr);
		}
		else{
			if(fd.cFileName[0] != '.'){
				char sfstr[MAX_PATH];
				PathCombine(sfstr, subd, fd.cFileName);
				MyFoldFileSrchS(sfstr);
			}
		}
		CheckMessage();
	} while(FindNextFile(hFind, &fd) && !CancelButton);
	FindClose(hFind);
	return 1;
}



void Add_PreviewListToFile(char *fn){
	struct NES_ROM_Data ROM_Data;
	int ret;
	ret = MyPreviewCheckROMFile(fn, &ROM_Data);

	if(ret == 1){
		for(int i=0; i<g_Preview_State.DB.nListViewNESItem; i++){
			if(g_Preview_State.DB.pListViewItem[i].Crc == ROM_Data.Crc){
				if(!(g_Preview_State.DB.pListViewItem[i].Flag & 1))
					nProgressState++;
				g_Preview_State.DB.pListViewItem[i].Flag |= 1;
				strcpy(g_Preview_State.DB.pListViewItem[i].FileName, fn);
				g_Preview_State.DB.ChangeHaveItem = 1;
				break;
			}
		}
	}
	else if(ret == 2){	//FDS
		int i,j;
		for(i=g_Preview_State.DB.nListViewNESItem; i<g_Preview_State.DB.nListViewItem; i++){
			int flag = 0;
			j = ROM_Data.HeaderInfo1;
			if(g_Preview_State.DB.pListViewItem[i].HeaderInfo1 != j)
				continue;
			for(int l=0; l<j; l++){
				if(g_Preview_State.DB.pListViewItem[i].FDS_Crc[l] != ROM_Data.FDS_Crc[l]){
					flag = 1;
					break;
				}
			}
			if(!flag){
				if(!(g_Preview_State.DB.pListViewItem[i].Flag & 1))
					nProgressState++;
				g_Preview_State.DB.pListViewItem[i].Flag |= 1;
				strcpy(g_Preview_State.DB.pListViewItem[i].FileName, fn);
				g_Preview_State.DB.ChangeHaveItem = 1;
				break;
			}
		}
	}
	else if(ret == 4){	//PCE
		int i,j;
		for(i=g_Preview_State.DB.nListViewPCE_Stpos, j=0; j<g_Preview_State.DB.nListViewPCEItem; i++, j++){
			if(g_Preview_State.DB.pListViewItem[i].Crc == ROM_Data.Crc){
				if(!(g_Preview_State.DB.pListViewItem[i].Flag & 1))
					nProgressState++;
				g_Preview_State.DB.pListViewItem[i].Flag |= 1;
				strcpy(g_Preview_State.DB.pListViewItem[i].FileName, fn);
				g_Preview_State.DB.ChangeHaveItem = 1;
				break;
			}
		}
	}
}


static int ExeCheck_NESROM_Flag = 0;

void ExeCheck_NESROM(){
	HWND hDlgWnd = NULL;

	if(ExeCheck_NESROM_Flag)
		return;
	ExeCheck_NESROM_Flag = 1;
	nProgressState=g_Preview_State.Category[0].Sub[0].nListValidItem;
	ProgressFileName[0]=0;
	hDlgWnd = CreateDialogParam(g_main_instance, MAKEINTRESOURCE(IDD_PROGRESSDIALOG), main_window_handle, (DLGPROC)MyProgressDlg, g_Preview_State.DB.nListViewItem);
	ShowWindow(hDlgWnd, SW_SHOW);
	UpdateWindow(hDlgWnd);
	for(int i=0; i<g_Preview_State.nROMDirectory && g_Preview_State.ROM_Directory[i][0]; i++){
		if(g_Preview_State.ROM_SubDirectory[i])
			MyFoldFileSrchS(g_Preview_State.ROM_Directory[i]);
		else
			MyFoldFileSrch(g_Preview_State.ROM_Directory[i]);
	}
	if(!CancelButton){
		DestroyWindow(hDlgWnd);
	}
//	SetListViewValidItem(0, 0);
	SetAllListViewValidItem();
	InvalidateRect(Mainwindow_Preview, NULL, TRUE);
	ExeCheck_NESROM_Flag = 0;
}


void ExeCheck_NESROM_Folder(int sff){
	HWND hDlgWnd = NULL;
	char dirname[MAX_PATH];

	if(ExeCheck_NESROM_Flag)
		return;
	if(!MyFolderOpenDlg(main_window_handle, dirname))
		return;
	ExeCheck_NESROM_Flag = 1;
	nProgressState=g_Preview_State.Category[0].Sub[0].nListValidItem;
	ProgressFileName[0]=0;
	hDlgWnd = CreateDialogParam(g_main_instance, MAKEINTRESOURCE(IDD_PROGRESSDIALOG), main_window_handle, (DLGPROC)MyProgressDlg, g_Preview_State.DB.nListViewItem);
	ShowWindow(hDlgWnd, SW_SHOW);
	UpdateWindow(hDlgWnd);
	if(sff)
		MyFoldFileSrchS(dirname);
	else
		MyFoldFileSrch(dirname);
	if(!CancelButton){
		DestroyWindow(hDlgWnd);
	}
//	SetListViewValidItem(0, 0);
	SetAllListViewValidItem();
	InvalidateRect(Mainwindow_Preview, NULL, TRUE);
	ExeCheck_NESROM_Flag = 0;
}


int OnOdfinditem(LPNMLVFINDITEM pInfo){
	int *pData_Index, i, max;

	int Root = g_Preview_State.nTreeSelect;
	int SubItem = g_Preview_State.nSubTreeSelect;
	if(SubItem == -1){
		ListView_SetItemCount(Mainwindow_Preview, 0);
		return -1;
	}
	switch(g_Preview_State.CheckResaultDisplay){
		case 0:
			pData_Index = g_Preview_State.Category[Root].Sub[SubItem].pListViewAllItem;
			max = g_Preview_State.Category[Root].Sub[SubItem].nListViewItem;
			break;
		case 1:
			pData_Index = g_Preview_State.Category[Root].Sub[SubItem].pListViewValidItem;
			max = g_Preview_State.Category[Root].Sub[SubItem].nListViewItem;
			break;
		case 2:
			pData_Index = g_Preview_State.Category[Root].Sub[SubItem].pListViewInValidItem;
			max = g_Preview_State.Category[Root].Sub[SubItem].nListViewItem;
			break;
		default:
			return -1;
	}

	for(i=0; i<max; i++){
		if(pInfo->lvfi.flags & (LVFI_PARTIAL | LVFI_STRING)){
			if(g_Preview_State.DB.pListViewItem[pData_Index[i]].Title[0] == pInfo->lvfi.psz[0])
				return i;
		}
/*
		else if(pInfo->lvfi.flags & LVFI_STRING){
			if(!strcmp(g_Preview_State.DB.pListViewItem[pData_Index[i]].Title, pInfo->lvfi.psz))
				return i;
		}
*/
	}
	return -1;
}


void OnGetDispInfo(NMLVDISPINFO* pInfo){
	static char str[512];
	if(pInfo->item.iItem == -1)
		return;

	int *pData_Index;

	int Root = g_Preview_State.nTreeSelect;
	int SubItem = g_Preview_State.nSubTreeSelect;
	if(SubItem == -1){
		ListView_SetItemCount(Mainwindow_Preview, 0);
		return;
	}
	switch(g_Preview_State.CheckResaultDisplay){
		case 0:
			pData_Index = g_Preview_State.Category[Root].Sub[SubItem].pListViewAllItem;
			break;
		case 1:
			pData_Index = g_Preview_State.Category[Root].Sub[SubItem].pListViewValidItem;
			break;
		case 2:
			pData_Index = g_Preview_State.Category[Root].Sub[SubItem].pListViewInValidItem;
			break;
		default:
			return;
	}

	if(pInfo->item.mask & LVIF_TEXT){
		switch (pInfo->item.iSubItem){
		  case 0:
			  if(!g_Preview_State.GameTitleDetailDisplay)
			  {
				  strcpy(str, g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Title);
				  MystrCcutk(str);
				  strcpy(pInfo->item.pszText, str);
//				  pInfo->item.pszText = str;
			  }
			  else{
				  strcpy(str, g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Title);
				  MystrFnIcut(str);
				  strcpy(pInfo->item.pszText, str);
//				  pInfo->item.pszText = str;
			  }
			break;
		  case 1:
			pInfo->item.pszText = g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Country;
			break;
		  case 2:
			pInfo->item.pszText = g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Maker;
			break;
		  case 3:
			pInfo->item.pszText = g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Release;
			break;
		  case 4:
			pInfo->item.pszText = g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Price;
			break;
		  case 5:
			pInfo->item.pszText = g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Genre;
			break;
		  case 6:
			itoa(g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].nPlay, str, 10);
			strcpy(pInfo->item.pszText, str);
//			pInfo->item.pszText = str;
			break;
		  case 7:
			pInfo->item.pszText = g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].FileName;
			break;
		}
	}
	if(pInfo->item.mask & LVIF_IMAGE){
		pInfo->item.iImage = g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Flag;
/*
		if(g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Flag&2){
			if(g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Flag&1)
				pInfo->item.iImage = 3;
			else
				pInfo->item.iImage = 2;
		}
		else {
			if(g_Preview_State.DB.pListViewItem[pData_Index[pInfo->item.iItem]].Flag&1)
				pInfo->item.iImage = 4;
			else
				pInfo->item.iImage = 5;
		}
*/
	}
}


// Search Item
int PreviewSearchItem(char *sstr, int dir){
	int *pData_Index;
	char *p;
	int nItem,i;

	int Root = g_Preview_State.nTreeSelect;
	int SubItem = g_Preview_State.nSubTreeSelect;
	if(SubItem == -1){
		ListView_SetItemCount(Mainwindow_Preview, 0);
		return 0;
	}
	switch(g_Preview_State.CheckResaultDisplay){
		case 0:
			pData_Index = g_Preview_State.Category[Root].Sub[SubItem].pListViewAllItem;
			nItem = g_Preview_State.Category[Root].Sub[SubItem].nListViewItem;
			break;
		case 1:
			pData_Index = g_Preview_State.Category[Root].Sub[SubItem].pListViewValidItem;
			nItem = g_Preview_State.Category[Root].Sub[SubItem].nListValidItem;
			break;
		case 2:
			pData_Index = g_Preview_State.Category[Root].Sub[SubItem].pListViewInValidItem;
			nItem = g_Preview_State.Category[Root].Sub[SubItem].nListInValidItem;
			break;
		default:
			return 0;
	}
	// first
	if(dir == 0){
		i = 0;
		p = sstr;
		if(sstr[0] == 0)
			return 0;
		strcpy(g_Preview_State.SearchString, sstr);
		dir = 1;
	}
	else if(dir ==1){	// next
		i = ListView_GetHotItem(g_Preview_State.hListView);
		i++;
		if(i < 0 || i > nItem)
			return 0;
//			i = 0;
		p = g_Preview_State.SearchString;
		if(p[0]==0)
			return 0;
	}
	else {	// pre
		i = ListView_GetHotItem(g_Preview_State.hListView);
		i--;
		if(i < 0)
			return 0;
		p = g_Preview_State.SearchString;
		if(p[0]==0)
			return 0;
	}
	for(; i<nItem && i >= 0; i+=dir){
		if(strstr((const char *)g_Preview_State.DB.pListViewItem[pData_Index[i]].Title, (const char *)p)!=NULL){
			ListView_EnsureVisible(g_Preview_State.hListView, i, FALSE);
			ListView_SetItemState(g_Preview_State.hListView, i, LVIS_SELECTED, LVIS_SELECTED);
			ListView_SetHotItem(g_Preview_State.hListView, i);
			return i;
		}
	}
	return 0;
}



// get item num
int GetCurrentListItemNum(){
	int nItem;

	int Root = g_Preview_State.nTreeSelect;
	int SubItem = g_Preview_State.nSubTreeSelect;
	if(SubItem == -1){
		ListView_SetItemCount(Mainwindow_Preview, 0);
		return 0;
	}
	switch(g_Preview_State.CheckResaultDisplay){
		case 0:
			nItem = g_Preview_State.Category[Root].Sub[SubItem].nListViewItem;
			break;
		case 1:
			nItem = g_Preview_State.Category[Root].Sub[SubItem].nListValidItem;
			break;
		case 2:
			nItem = g_Preview_State.Category[Root].Sub[SubItem].nListInValidItem;
			break;
		default:
			return 0;
	}
	return nItem;
}


void PreviewRandamSelectItem(){
	int i,n;

	n = GetCurrentListItemNum();
	i = (rand()%n);
	ListView_EnsureVisible(g_Preview_State.hListView, i, FALSE);
	ListView_SetItemState(g_Preview_State.hListView, i, LVIS_SELECTED, LVIS_SELECTED);
//	ListView_SetHotItem(g_Preview_State.hListView, i);
}


static int g_previewsort_structoffset;
static int g_previewsort_dir;


int PreviewSortVal(const void *p1 , const void *p2){
	char *n1, *n2;
	int v1, v2;

	n1 = (char *)&g_Preview_State.DB.pListViewItem[*((int *)p1)];
	n2 = (char *)&g_Preview_State.DB.pListViewItem[*((int *)p2)];
	n1+=g_previewsort_structoffset;
	n2+=g_previewsort_structoffset;
	v1 = *((int *)n1);
	v2 = *((int *)n2);

	{
		if(v1==v2)
			return 0;
		if(g_previewsort_dir == 1){
			return (v1 < v2)? 1 : -1;
		}
		else{
			return (v1 < v2)? -1 : 1;
		}
	}
}


int PreviewSorts(const void *p1 , const void *p2){

	char *n1, *n2;

	n1 = (char *)&g_Preview_State.DB.pListViewItem[*((int *)p1)];
	n2 = (char *)&g_Preview_State.DB.pListViewItem[*((int *)p2)];
	n1+=g_previewsort_structoffset;
	n2+=g_previewsort_structoffset;

	{
		if(g_previewsort_dir == 1){
			return MystrJscmp(n1, n2, 0);
		}
		else{
			return MystrJscmp(n1, n2, 1) * -1;
		}
	}
}

int PreviewSortn(const void *p1 , const void *p2){

	char *n1, *n2;

	n1 = (char *)&g_Preview_State.DB.pListViewItem[*((int *)p1)];
	n2 = (char *)&g_Preview_State.DB.pListViewItem[*((int *)p2)];
	n1+=g_previewsort_structoffset;
	n2+=g_previewsort_structoffset;

	{
		if(g_previewsort_dir == 1){
			return MystrJncmp(n1, n2, 0);
		}
		else{
			return MystrJncmp(n1, n2, 1) * -1;
		}
	}
}

void PreviewListSort(int nItem){
	int structoffset;
	char cflag= 0;
	
	g_previewsort_dir = g_Preview_State.PrevSortItemp[(int)nItem];

	switch(nItem){
		case 0:
			structoffset = offsetof(struct NES_ROM_Data,  Title);
			break;
		case 1:
			structoffset = offsetof(struct NES_ROM_Data,  Country);
			break;
		case 2:
			structoffset = offsetof(struct NES_ROM_Data,  Maker);
			break;
		case 3:
			structoffset = offsetof(struct NES_ROM_Data,  Release);
//			cflag = 1;
			break;
		case 4:
			structoffset = offsetof(struct NES_ROM_Data,  Price);
			cflag = 1;
			break;
		case 5:
			structoffset = offsetof(struct NES_ROM_Data,  Genre);
			break;
		case 6:
			structoffset = offsetof(struct NES_ROM_Data,  nPlay);
			cflag = 2;
			break;
		case 7:
			structoffset = offsetof(struct NES_ROM_Data,  FileName);
			break;
	}
	g_previewsort_structoffset = structoffset;
	int *pROM_Data;
	int n=0;

	int Root = g_Preview_State.nTreeSelect;
	int SubItem = g_Preview_State.nSubTreeSelect;
	if(SubItem == -1){
		ListView_SetItemCount(Mainwindow_Preview, 0);
		return;
	}
	switch(g_Preview_State.CheckResaultDisplay){
		case 0:		//全て表示
			pROM_Data = g_Preview_State.Category[Root].Sub[SubItem].pListViewAllItem;
			n = g_Preview_State.Category[Root].Sub[SubItem].nListViewItem;
			break;
		case 1:		//所持品のみ表示
			pROM_Data = g_Preview_State.Category[Root].Sub[SubItem].pListViewValidItem;
			n = g_Preview_State.Category[Root].Sub[SubItem].nListValidItem;
			break;
		case 2:		//非所持品のみ表示
			pROM_Data = g_Preview_State.Category[Root].Sub[SubItem].pListViewInValidItem;
			n = g_Preview_State.Category[Root].Sub[SubItem].nListInValidItem;
			break;
		default:
			return;
	}
	switch(cflag){
		case 0:
			qsort((void *)pROM_Data, n, sizeof(int *), PreviewSorts);
			break;
		case 1:
			qsort((void *)pROM_Data, n, sizeof(int *), PreviewSortn);
			break;
		case 2:
			qsort((void *)pROM_Data, n, sizeof(int *), PreviewSortVal);
			break;
	}
	ListView_SetItemCount(Mainwindow_Preview, n);
}


//

void PreviewDataStructSort(int *dest, int Num, int nItem, int Sortd){
	int structoffset;
	char cflag= 0;

	g_previewsort_dir = Sortd;

	switch(nItem){
		case 0:
			structoffset = offsetof(struct NES_ROM_Data,  Title);
			break;
		case 1:
			structoffset = offsetof(struct NES_ROM_Data,  Country);
			break;
		case 2:
			structoffset = offsetof(struct NES_ROM_Data,  Maker);
			break;
		case 3:
			structoffset = offsetof(struct NES_ROM_Data,  Release);
//			cflag = 1;
			break;
		case 4:
			structoffset = offsetof(struct NES_ROM_Data,  Price);
			cflag = 1;
			break;
		case 5:
			structoffset = offsetof(struct NES_ROM_Data,  Genre);
			break;
		case 6:
			structoffset = offsetof(struct NES_ROM_Data,  nPlay);
			cflag = 2;
			break;
		case 7:
			structoffset = offsetof(struct NES_ROM_Data,  FileName);
			break;
	}
	g_previewsort_structoffset = structoffset;

	switch(cflag){
		case 0:
			qsort((void *)dest, Num, sizeof(int *), PreviewSorts);
			break;
		case 1:
			qsort((void *)dest, Num, sizeof(int *), PreviewSortn);
			break;
		case 2:
			qsort((void *)dest, Num, sizeof(int *), PreviewSortVal);
			break;
	}
}


/*
int CALLBACK MyPreviewListSort(LPARAM lp1, LPARAM lp2, LPARAM lp3)
{
	static LV_FINDINFO lvf;
	static int nItem1, nItem2, numbera, numberb, ressult;
	static char buf1[MAX_PATH], buf2[MAX_PATH];

	lvf.flags = LVFI_PARAM;
	lvf.lParam = lp1;
	nItem1 = ListView_FindItem(Mainwindow_Preview, -1, &lvf);

	lvf.lParam = lp2;
	nItem2 = ListView_FindItem(Mainwindow_Preview, -1, &lvf);

	buf1[0]=0, buf2[0]=0;
	ListView_GetItemText(Mainwindow_Preview, nItem1, (int)lp3, buf1, sizeof(buf1));
	ListView_GetItemText(Mainwindow_Preview, nItem2, (int)lp3, buf2, sizeof(buf2));

	if(lp3!=2 && lp3!=3){
		if (g_Preview_State.PrevSortItemp[(int)lp3] == 1)
			return(MystrJncmp(buf1, buf2, 0));
		else
			return(MystrJncmp(buf1, buf2, 1) * -1);
	}
	else{
		if(g_Preview_State.PrevSortItemp[(int)lp3] == 1)
			return(MystrJscmp(buf1, buf2, 0));
		else
			return(MystrJscmp(buf1, buf2, 1) * -1);
	}
}
*/

void SetListViewValidItemOne(int Root, int SubItem, int nAdd){
	if(SubItem == -1){
		return;
	}
	struct NES_ROM_Data *rominfo = (NES_ROM_Data *)g_Preview_State.DB.pListViewItem;
	int *all = (int *)g_Preview_State.Category[Root].Sub[SubItem].pListViewAllItem;
	if(rominfo[all[nAdd]].Flag & 1){
		int *valid = (int *)g_Preview_State.Category[Root].Sub[SubItem].pListViewValidItem;
		int n = g_Preview_State.Category[Root].Sub[SubItem].nListValidItem;
		valid[n] = all[nAdd];
		g_Preview_State.Category[Root].Sub[SubItem].nListValidItem++;
	}
	else{
		int *invalid = (int *)g_Preview_State.Category[Root].Sub[SubItem].pListViewInValidItem;
		int n = g_Preview_State.Category[Root].Sub[SubItem].nListInValidItem;
		invalid[n] = all[nAdd];
		g_Preview_State.Category[Root].Sub[SubItem].nListInValidItem++;
	}
}


void SetListViewValidItem(int Root, int SubItem){
	if(SubItem == -1){
		return;
	}
	struct NES_ROM_Data *rominfo = (NES_ROM_Data *)g_Preview_State.DB.pListViewItem;
	int *all = (int *)g_Preview_State.Category[Root].Sub[SubItem].pListViewAllItem;
	int *valid = (int *)g_Preview_State.Category[Root].Sub[SubItem].pListViewValidItem;
	int *invalid = (int *)g_Preview_State.Category[Root].Sub[SubItem].pListViewInValidItem;
	int nValidItem = 0, nInValidItem = 0;
	int nAllItem = g_Preview_State.Category[Root].Sub[SubItem].nListViewItem;
	for(int i=0; i<nAllItem; i++){
		if(rominfo[all[i]].Flag & 1){
			valid[nValidItem] = all[i];
			nValidItem++;
		}
		else{
			invalid[nInValidItem] = all[i];
			nInValidItem++;
		}
	}
	g_Preview_State.Category[Root].Sub[SubItem].nListValidItem = nValidItem;
	g_Preview_State.Category[Root].Sub[SubItem].nListInValidItem = nInValidItem;
}


void SetAllListViewValidItem(){
	int Root, SubItem;
	struct NES_ROM_Data *rominfo = (NES_ROM_Data *)g_Preview_State.DB.pListViewItem;

	for(Root=0; Root<g_Preview_State.nCategory; Root++){
		for(SubItem=0; SubItem<g_Preview_State.Category[Root].nSubCategory; SubItem++){
			int *all = (int *)g_Preview_State.Category[Root].Sub[SubItem].pListViewAllItem;
			int *valid = (int *)g_Preview_State.Category[Root].Sub[SubItem].pListViewValidItem;
			int *invalid = (int *)g_Preview_State.Category[Root].Sub[SubItem].pListViewInValidItem;
			int nValidItem = 0, nInValidItem = 0;
			int nAllItem = g_Preview_State.Category[Root].Sub[SubItem].nListViewItem;
			for(int i=0; i<nAllItem; i++){
				if(rominfo[all[i]].Flag & 1){
					valid[nValidItem] = all[i];
					nValidItem++;
				}
				else{
					invalid[nInValidItem] = all[i];
					nInValidItem++;
				}
			}
			g_Preview_State.Category[Root].Sub[SubItem].nListValidItem = nValidItem;
			g_Preview_State.Category[Root].Sub[SubItem].nListInValidItem = nInValidItem;
		}
	}
	MyTreeView_SetItemNum();
}


void DateStrInsSpace(char *Date){
	char str[16];
	int i, j;

	strcpy(str, Date);
	for(i=0; str[i]!='/'; i++);
	i++;
	j=i;
	if(str[i+1]=='/'){
		Date[j++] = ' ';
		Date[j++] = str[i++];
	}
	else{
		Date[j++] = str[i++];
		Date[j++] = str[i++];
	}
	Date[j++] = str[i++];
	if(str[i+1]=='\0'){
		Date[j++] = ' ';
		Date[j++] = str[i++];
		Date[j++] = str[i++];
	}
	else{
		Date[j++] = str[i++];
		Date[j++] = str[i++];
		Date[j++] = str[i++];
	}
}


int SetListViewItem_1st_NES(char *fn, struct ROM_DB *ROM_Data){
	int i=0,j=0,k;
	unsigned char *p=NULL;
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes, filesize, fp=0;

//	ClearListViewItemList();

	i = ROM_Data->nListViewItem;
/*
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, fn);
*/
	hFile = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return 0;
	}
	filesize = GetFileSize(hFile, NULL);
	p = (unsigned char *)malloc(filesize);
	if(p==NULL){
		CloseHandle(hFile);
		return 0;
	}
	ReadFile(hFile, p, filesize, &dwAccBytes, NULL);
	CloseHandle(hFile);

	while(1){
		if(i==ROM_Data->nListViewItemMem){
			struct NES_ROM_Data *nesi=NULL;
			nesi = (NES_ROM_Data *)realloc(ROM_Data->pListViewItem, sizeof(NES_ROM_Data)*(100+ROM_Data->nListViewItemMem));
			memset(&nesi[ROM_Data->nListViewItemMem], 0, sizeof(NES_ROM_Data)*100);
			ROM_Data->pListViewItem = nesi;
			ROM_Data->nListViewItemMem+=100;
		}
		for(j=0;;j++){
			for(k=0;fp<filesize;++k){
				if(p[fp]==';' || p[fp]==0x0d){
					break;
				}
				str[k]=p[fp++];
			}
			if(fp>=filesize)
				goto end_loop;
			if(j==0 && k<8){
				while(fp<filesize && p[fp++]!=0x0a);
				break;
			}
			str[k]=0;
			switch(j){
				case 0:		// All CRC
					{
						DWORD crc;
						MyStrAtoh(str, &crc);
						ROM_Data->pListViewItem[i].Crc = crc;
					}
					break;
				case 1:		// PROM CRC
					{
						DWORD crc;
						MyStrAtoh(str, &crc);
						ROM_Data->pListViewItem[i].PROM_Crc = crc;
					}
					break;
				case 2:		// Title
					strcpy(ROM_Data->pListViewItem[i].Title, str);
					break;
				case 3:		// Header Info 1
					ROM_Data->pListViewItem[i].HeaderInfo1 = atoi(str);
					break;
				case 4:		// Header Info 2
					ROM_Data->pListViewItem[i].HeaderInfo2 = atoi(str);
					break;
				case 5:		// PROM Size / 16KB
					ROM_Data->pListViewItem[i].PROM_Size = atoi(str);
					break;
				case 6:		// CROM Size / 8KB
					ROM_Data->pListViewItem[i].CROM_Size = atoi(str);
					break;
				case 7:		// Country
					strcpy(ROM_Data->pListViewItem[i].Country, str);
					break;
				case 8:		// Maker
					strcpy(ROM_Data->pListViewItem[i].Maker, str);
					break;
				case 9:		// Date
					DateStrInsSpace(str);
					strcpy(ROM_Data->pListViewItem[i].Release, str);
					break;
				case 10:	// Price
					strcpy(ROM_Data->pListViewItem[i].Price, str);
					break;
				case 11:	// Genre
					strcpy(ROM_Data->pListViewItem[i].Genre, str);
					break;
			}
			if(p[fp]==';'){
				fp++;
			}
			else if(p[fp]==0x0d){
				fp+=2;
				i++;
				break;
			}
		}
	}
end_loop:
	if(p)
		free(p);
	ROM_Data->nListViewItem = i;
	ROM_Data->nListViewNESItem= i;
	return 1;
}


void SetListViewItem_Add_NES(char *fn, struct ROM_DB *ROM_Data){
	int i=0,j=0,k,n;
	unsigned char *p=NULL;
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes, filesize, fp=0;
/*
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);

	PathCombine(str , str, fn);
*/
	p = NULL;
	hFile = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return;
	}
	filesize = GetFileSize(hFile, NULL);
	p = (unsigned char *)malloc(filesize);
	if(p==NULL){
		CloseHandle(hFile);
		return;
	}
	ReadFile(hFile, p, filesize, &dwAccBytes, NULL);
	CloseHandle(hFile);

	i = ROM_Data->nListViewItem;
	n = i;

	while(1){
		int DisableFlag=0;
		if(i==ROM_Data->nListViewItemMem){
			struct NES_ROM_Data *nesi=NULL;
			nesi = (NES_ROM_Data *)realloc(ROM_Data->pListViewItem, sizeof(NES_ROM_Data)*(100+ROM_Data->nListViewItemMem));
			memset(&nesi[ROM_Data->nListViewItemMem], 0, sizeof(NES_ROM_Data)*100);
			ROM_Data->pListViewItem = nesi;
			ROM_Data->nListViewItemMem+=100;
		}
		for(j=0;;j++){
			for(k=0;fp<filesize;++k){
				if(p[fp]==';' || p[fp]==0x0d){
					break;
				}
				str[k]=p[fp++];
			}
			if(fp>=filesize)
				goto end_loop_2;
			if(j==0 && k<8){
				while(fp<filesize && p[fp++]!=0x0a);
				break;
			}
			str[k]=0;
			if(!DisableFlag){
				switch(j){
				case 0:		// All CRC
					{
						DWORD crc;
						MyStrAtoh(str, &crc);
						for(int l=0; l<n; l++){
							if(crc == ROM_Data->pListViewItem[l].Crc){
								DisableFlag=1;
								break;
							}
						}
						if(!DisableFlag)
							ROM_Data->pListViewItem[i].Crc = crc;
					}
					break;
				case 1:		// PROM CRC
					{
						DWORD crc;
						MyStrAtoh(str, &crc);
						ROM_Data->pListViewItem[i].PROM_Crc = crc;
					}
					break;
				case 2:		// Title
					strcpy(ROM_Data->pListViewItem[i].Title, str);
					break;
				case 3:		// Header Info 1
					ROM_Data->pListViewItem[i].HeaderInfo1 = atoi(str);
					break;
				case 4:		// Header Info 2
					ROM_Data->pListViewItem[i].HeaderInfo2 = atoi(str);
					break;
				case 5:		// PROM Size / 16KB
					ROM_Data->pListViewItem[i].PROM_Size = atoi(str);
					break;
				case 6:		// CROM Size / 8KB
					ROM_Data->pListViewItem[i].CROM_Size = atoi(str);
					break;
				case 7:		// Country
					strcpy(ROM_Data->pListViewItem[i].Country, str);
					break;
				case 8:		// Maker
					strcpy(ROM_Data->pListViewItem[i].Maker, str);
					break;
				case 9:		// Date
					DateStrInsSpace(str);
					strcpy(ROM_Data->pListViewItem[i].Release, str);
					break;
				case 10:	// Price
					strcpy(ROM_Data->pListViewItem[i].Price, str);
					break;
				case 11:	// Genre
					strcpy(ROM_Data->pListViewItem[i].Genre, str);
					break;
				}
			}
			if(p[fp]==';'){
				fp++;
			}
			else if(p[fp]==0x0d){
				fp+=2;
				if(!DisableFlag)
					i++;
				break;
			}
		}
	}
end_loop_2:
	if(p)
		free(p);
	ROM_Data->nListViewItem = i;
	ROM_Data->nListViewNESItem= i;
}


void ApplyListViewItem_Fix_NES(char *fn, struct ROM_DB *ROM_Data){
	int i=0,j=0,k;
	unsigned char *p=NULL;
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes, filesize, fp=0;
	struct NES_ROM_Data TmpData;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);

	p = NULL;
	PathCombine(str , str, fn);
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return;
	}
	filesize = GetFileSize(hFile, NULL);
	p = (unsigned char *)malloc(filesize);
	if(p==NULL){
		CloseHandle(hFile);
		return;
	}
	ReadFile(hFile, p, filesize, &dwAccBytes, NULL);
	CloseHandle(hFile);

	i = 0;

	while(1){
		for(j=0;;j++){
			for(k=0;fp<filesize;++k){
				if(p[fp]==';' || p[fp]==0x0d){
					break;
				}
				str[k]=p[fp++];
			}
			if(fp>=filesize)
				goto end_loop_2;
			if(j==0 && k<8){
				while(fp<filesize && p[fp++]!=0x0a);
				break;
			}
			str[k]=0;
			switch(j){
				case 0:		// All CRC
					{
						DWORD crc;
						MyStrAtoh(str, &crc);
						TmpData.Crc = crc;
					}
					break;
				case 1:		// PROM CRC
					{
						DWORD crc;
						MyStrAtoh(str, &crc);
						TmpData.PROM_Crc = crc;
					}
					break;
				case 2:		// Title
					strcpy(TmpData.Title, str);
					break;
				case 3:		// Header Info 1
					TmpData.HeaderInfo1 = atoi(str);
					break;
				case 4:		// Header Info 2
					TmpData.HeaderInfo2 = atoi(str);
					break;
				case 5:		// PROM Size / 16KB
					TmpData.PROM_Size = atoi(str);
					break;
				case 6:		// CROM Size / 8KB
					TmpData.CROM_Size = atoi(str);
					break;
				case 7:		// Country
					strcpy(TmpData.Country, str);
					break;
				case 8:		// Maker
					strcpy(TmpData.Maker, str);
					break;
				case 9:		// Date
					DateStrInsSpace(str);
					strcpy(TmpData.Release, str);
					break;
				case 10:	// Price
					strcpy(TmpData.Price, str);
					break;
				case 11:	// Genre
					strcpy(TmpData.Genre, str);
					break;
			}
			if(p[fp]==';'){
				fp++;
			}
			else if(p[fp]==0x0d){
				int n = ROM_Data->nListViewNESItem;
				for(int m=0; m<n; m++){
					if(!strcmp(ROM_Data->pListViewItem[m].Title, TmpData.Title)){
						ROM_Data->pListViewItem[m].Crc = TmpData.Crc;
						ROM_Data->pListViewItem[m].PROM_Crc = TmpData.PROM_Crc;
						ROM_Data->pListViewItem[m].PROM_Size= TmpData.PROM_Size;
						ROM_Data->pListViewItem[m].CROM_Size= TmpData.CROM_Size;
						ROM_Data->pListViewItem[m].HeaderInfo1= TmpData.HeaderInfo1;
						ROM_Data->pListViewItem[m].HeaderInfo2= TmpData.HeaderInfo2;
						break;
					}
				}
				fp+=2;
				i++;
				break;
			}
		}
	}
end_loop_2:
	if(p)
		free(p);
}




void ApplyListViewItem_Fix_FDS(char *fn,  struct ROM_DB *ROM_Data){
	int i,j=0,k,n=0,m=0;
	unsigned char *p=NULL;
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes, filesize, fp=0;
	struct NES_ROM_Data TmpData;

	i = 0;
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);

	PathCombine(str , str, fn);		//ingredients.dat
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return;
	}
	filesize = GetFileSize(hFile, NULL);
	p = (unsigned char *)malloc(filesize);
	if(p==NULL){
		return;
	}
	ReadFile(hFile, p, filesize, &dwAccBytes, NULL);
	CloseHandle(hFile);

	while(1){
		for(j=0;;j++){
			for(k=0;fp<filesize;++k){
				if(p[fp]==';' || p[fp]==0x0d){
					break;
				}
				str[k]=p[fp++];
			}
			if(fp>=filesize)
				goto end_loop;
/*
			if(j==0 && k<8){
				while(fp<filesize && p[fp++]!=0x0a);
				break;
			}
*/
			str[k]=0;
			switch(j){
				case 0:		// ID
					{
/*
						DWORD crc;
						MyStrAtoh(str, &crc);
						g_Preview_State.DB.pListViewItem[i].Crc = crc;
*/
					}
					break;
				case 1:		// ?
				case 2:		// ?
				case 3:		// ?
				case 4:		// ?
					break;
				case 5:		// File Num
					n = atoi(str);
					m=0;
					TmpData.HeaderInfo1 = (char)n;
					break;
				default:
					if(m<n){
						DWORD crc;
						MyStrAtoh(str, &crc);
						TmpData.FDS_Crc[m++] = crc;
					}
					else{
						strcpy(TmpData.Title, str);
					}
					break;
			}
			if(p[fp]==';'){
				fp++;
			}
			else if(p[fp]==0x0d){
				int nItem = ROM_Data->nListViewItem;
				for(int o=ROM_Data->nListViewNESItem; o<nItem; o++){
					if(!strcmp(ROM_Data->pListViewItem[o].Title, TmpData.Title)){
						int fnum = (int)TmpData.HeaderInfo1;
						for(k=0; k<fnum; k++){
							ROM_Data->pListViewItem[o].FDS_Crc[k] = TmpData.FDS_Crc[k];
						}
						ROM_Data->pListViewItem[o].HeaderInfo1= TmpData.HeaderInfo1;
						ROM_Data->pListViewItem[o].HeaderInfo2= TmpData.HeaderInfo2;
						break;
					}
				}
				fp+=2;
				i++;
				n=0, m=0;
				break;
			}
		}
	}
end_loop:
	if(p)
		free(p);
}



// crc
int SetListViewItem_PCE(char *fn,  struct ROM_DB *ROM_Data){
	int i,j=0,k,n=0;
	unsigned char *p=NULL;
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes, filesize, fp=0;

//	ClearListViewItemList();
	i = ROM_Data->nListViewItem;


	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);

	PathCombine(str , str, fn);		//ingredients.dat

	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return 0;
	}
	filesize = GetFileSize(hFile, NULL);
	p = (unsigned char *)malloc(filesize);
	if(p==NULL){
		return 0;
	}
	ReadFile(hFile, p, filesize, &dwAccBytes, NULL);
	CloseHandle(hFile);

	while(1){
		if(i==ROM_Data->nListViewItemMem){
			struct NES_ROM_Data *nesi=NULL;
			nesi = (NES_ROM_Data *)realloc(ROM_Data->pListViewItem, sizeof(NES_ROM_Data)*(100+ROM_Data->nListViewItemMem));
			memset(&nesi[ROM_Data->nListViewItemMem], 0, sizeof(NES_ROM_Data)*100);
			ROM_Data->pListViewItem = nesi;
			ROM_Data->nListViewItemMem+=100;
		}
		for(j=0;;j++){
			for(k=0;fp<filesize;++k){
				if(p[fp]==';' || p[fp]==0x0d){
					break;
				}
				str[k]=p[fp++];
			}
			if(fp>=filesize)
				goto end_loop;
			if(j==0 && k!=8){
				while(fp<filesize && p[fp++]!=0x0a);
				break;
			}
			str[k]=0;
			switch(j){
				case 0:		// CRC
					{
						DWORD crc;
						MyStrAtoh(str, &crc);
						g_Preview_State.DB.pListViewItem[i].Crc = crc;
						ROM_Data->pListViewItem[i].Flag |= 6;
					}
					break;
				case 1:		// ?
					break;
				case 2:
					strcpy(ROM_Data->pListViewItem[i].Title, str);
					break;
			}
			if(p[fp]==';'){
				fp++;
			}
			else if(p[fp]==0x0d){
				fp+=2;
				i++;
				n++;
				break;
			}
		}
	}
end_loop:
	if(p)
		free(p);
	ROM_Data->nListViewPCEItem = n;
	ROM_Data->nListViewItem = i;
	return 1;
}



void SetListViewItem_NES(){
	if(g_Preview_State.DB.pListViewItem){
		free(g_Preview_State.DB.pListViewItem);
		g_Preview_State.DB.pListViewItem=NULL;
	}
	if(g_Preview_State.DB.pListViewSortItem){
		free(g_Preview_State.DB.pListViewSortItem);
		g_Preview_State.DB.pListViewSortItem=NULL;
	}
	g_Preview_State.DB.nListViewItem = 0;
	g_Preview_State.DB.nListViewItemMem= 0;
	g_Preview_State.DB.nListViewNESItem= 0;
	g_Preview_State.DB.nListViewFDSItem= 0;

	if(!g_Preview_State.NotLoadNESJ_DB){
		SetListViewItem_1st_NES(g_Preview_State.DATABASE_FileName[0]/*"famicom.dat"*/, &g_Preview_State.DB);
	}
	if(!g_Preview_State.NotLoadNESE_DB){
		if(!g_Preview_State.DB.nListViewNESItem){
			SetListViewItem_1st_NES(g_Preview_State.DATABASE_FileName[1]/*"nesdbase.dat"*/, &g_Preview_State.DB);
		}
		else{
			SetListViewItem_Add_NES(g_Preview_State.DATABASE_FileName[1]/*"nesdbase.dat"*/, &g_Preview_State.DB);
		}
	}
	if(!g_Preview_State.NotLoadNES_AddDB){
		char str[MAX_PATH];
		GetModuleFileName(NULL, str, MAX_PATH);
		PathRemoveFileSpec(str);
		PathCombine(str , str, "nesdbadd.dat");
		SetListViewItem_1st_NES(str, &g_Preview_State.DB);
	}
	if(!g_Preview_State.NotApplyNES_FixDB){
		ApplyListViewItem_Fix_NES("nesdbfix.dat", &g_Preview_State.DB);
	}
	if(!g_Preview_State.NotLoadFDS_DB){
		SetListViewItem_FDS(g_Preview_State.DATABASE_FileName[2]/*"disksys.dat"*/, &g_Preview_State.DB);
//			SetListViewItem_FDS("ingredients.dat", &g_Preview_State.DB);
	}
	if(!g_Preview_State.NotLoadFDS_AddDB){
		char str[MAX_PATH];
		GetModuleFileName(NULL, str, MAX_PATH);
		PathRemoveFileSpec(str);
		PathCombine(str , str, "fdsdbadd.dat");
		SetListViewItem_FDS(str, &g_Preview_State.DB);
	}
	if(!g_Preview_State.NotApplyFDS_FixDB){
		ApplyListViewItem_Fix_FDS("fdsdbfix.dat", &g_Preview_State.DB);
	}


#ifndef _NES_ONLY
	// PCE
	g_Preview_State.DB.nListViewPCE_Stpos = g_Preview_State.DB.nListViewItem;
	SetListViewItem_PCE("pce.dat", &g_Preview_State.DB);
#endif

	// sort
	g_Preview_State.DB.pListViewSortItem = (int *)malloc(g_Preview_State.DB.nListViewItem * sizeof(int *));
	for(int i=0; i<g_Preview_State.DB.nListViewItem; i++){
		g_Preview_State.DB.pListViewSortItem[i] = i;
	}
	PreviewDataStructSort(g_Preview_State.DB.pListViewSortItem, g_Preview_State.DB.nListViewItem, 0, 1);
	LoadListFile_NES();						// 所持リストを読み込み
}



int SetListViewItem_FDS(char *fn,  struct ROM_DB *ROM_Data){
	int i,j=0,k,n=0,m=0;
	unsigned char *p=NULL;
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes, filesize, fp=0;

//	ClearListViewItemList();
	i = ROM_Data->nListViewItem;
/*
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);

	PathCombine(str , str, fn);		//ingredients.dat
*/
	hFile = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return 0;
	}
	filesize = GetFileSize(hFile, NULL);
	p = (unsigned char *)malloc(filesize);
	if(p==NULL){
		return 0;
	}
	ReadFile(hFile, p, filesize, &dwAccBytes, NULL);
	CloseHandle(hFile);

	while(1){
		if(i==ROM_Data->nListViewItemMem){
			struct NES_ROM_Data *nesi=NULL;
			nesi = (NES_ROM_Data *)realloc(ROM_Data->pListViewItem, sizeof(NES_ROM_Data)*(100+ROM_Data->nListViewItemMem));
			memset(&nesi[ROM_Data->nListViewItemMem], 0, sizeof(NES_ROM_Data)*100);
			ROM_Data->pListViewItem = nesi;
			ROM_Data->nListViewItemMem+=100;
		}
		for(j=0;;j++){
			for(k=0;fp<filesize;++k){
				if(p[fp]==';' || p[fp]==0x0d){
					break;
				}
				str[k]=p[fp++];
			}
			if(fp>=filesize)
				goto end_loop;
/*
			if(j==0 && k<8){
				while(fp<filesize && p[fp++]!=0x0a);
				break;
			}
*/
			str[k]=0;
			switch(j){
				case 0:		// ID
					{
						if(strlen(str)<4){
							while(fp<filesize && p[fp]!=0x0d)fp++;
							if(fp>=filesize)
								goto end_loop;
						}
/*
						DWORD crc;
						MyStrAtoh(str, &crc);
						g_Preview_State.DB.pListViewItem[i].Crc = crc;
*/
					}
					break;
				case 1:		// ?
				case 2:		// ?
				case 3:		// ?
				case 4:		// ?
					break;
				case 5:		// File Num
					n = atoi(str);
					m=0;
					ROM_Data->pListViewItem[i].Flag |= 2;		// FDS Flag set
					ROM_Data->pListViewItem[i].HeaderInfo1 = (char)n;
					break;
				default:
					if(m<n){
						DWORD crc;
						MyStrAtoh(str, &crc);
						ROM_Data->pListViewItem[i].FDS_Crc[m++] = crc;
					}
					else {
						int sw = m-n;
						switch(sw){
							case 0:
								strcpy(ROM_Data->pListViewItem[i].Title, str);
								break;
							case 1:
								strcpy(ROM_Data->pListViewItem[i].Country, str);
								break;
							case 2:
								strcpy(ROM_Data->pListViewItem[i].Maker, str);
								break;
							case 3:
								strcpy(ROM_Data->pListViewItem[i].Release, str);
								break;
							case 4:
								strcpy(ROM_Data->pListViewItem[i].Price, str);
								break;
							case 5:
								strcpy(ROM_Data->pListViewItem[i].Genre, str);
								break;
						}
						m++;
					}
					break;
			}
			if(p[fp]==';'){
				fp++;
			}
			else if(p[fp]==0x0d){
				fp+=2;
				i++;
				n=0, m=0;
				break;
			}
		}
	}
end_loop:
	if(p)
		free(p);
	ROM_Data->nListViewFDSItem = i - ROM_Data->nListViewNESItem;
	ROM_Data->nListViewItem = i;
	return 1;
}




/*
void SavePreviewList(HWND lvhWnd){

	int i=-1,j;
	unsigned char CRLF[2] = { 0x0d, 0x0a};
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes;

	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "Preview.lst");
	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
 	if(hFile == INVALID_HANDLE_VALUE){
		hFile = CreateFile(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
	}
	while((i=ListView_GetNextItem(lvhWnd, i, LVNI_ALL)) != -1){
		for(j=0;j<6;++j){
			str[0]=0;
			ListView_GetItemText(lvhWnd, i, j, str, MAX_PATH);
			if(j!=5)
				strcat(str, ";");
			WriteFile(hFile, str, strlen(str), &dwAccBytes, NULL);
		}
		WriteFile(hFile, CRLF, 2, &dwAccBytes, NULL);
	}
	CloseHandle(hFile);
}
*/

void SetTreeViewSelectItem(HTREEITEM item){
	int i, nItem=-1, SubItem=-1;

//	SaveListViewItemList();
	for(i=0; i<g_Preview_State.nCategory; i++){
		int j;
		if(g_Preview_State.Category[i].TreeViewItemID == item){
			nItem=i;
			break;
		}
		for(j=0; j<g_Preview_State.Category[i].nSubCategory; j++){
			if(g_Preview_State.Category[i].Sub[j].TreeViewItemID == item){
				nItem = i;
				SubItem = j;
			}
		}
	}
	if(nItem==-1)
		return;

	g_Preview_State.nTreeSelect = nItem;
	g_Preview_State.nSubTreeSelect = SubItem;
	PreviewToolChangeDisplay();
}


void MyTreeView_SetItemNum(){
	char TreeStr[64], str[64];
	TVITEM tvItem;
	HWND hTreeView = g_Preview_State.hTreeView;

	memset((void *)&tvItem, 0, sizeof(tvItem));
	tvItem.mask =  TVIF_TEXT | TVIF_IMAGE| TVIF_SELECTEDIMAGE;

	tvItem.iImage = 2;
	tvItem.iSelectedImage = 2;
	LoadString(g_main_instance, IDS_STRING_PRVS0_01 , str, 64);
	wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[0].Sub[0].nListValidItem,g_Preview_State.DB.nListViewItem);
	tvItem.pszText = TreeStr;
	tvItem.hItem = g_Preview_State.Category[0].Sub[0].TreeViewItemID;
	TreeView_SetItem(hTreeView, &tvItem);	

	tvItem.iImage = 3;
	tvItem.iSelectedImage = 3;
	LoadString(g_main_instance, IDS_STRING_PRVS0_02 , str, 64);
	wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[0].Sub[1].nListValidItem, g_Preview_State.Category[0].Sub[1].nListViewItem);
	tvItem.pszText = TreeStr;
	tvItem.hItem = g_Preview_State.Category[0].Sub[1].TreeViewItemID;
	TreeView_SetItem(hTreeView, &tvItem);

	// FDS
	tvItem.iImage = 4;
	tvItem.iSelectedImage = 4;
	LoadString(g_main_instance, IDS_STRING_PRVS0_03 , str, 64);
	wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[0].Sub[2].nListValidItem, g_Preview_State.Category[0].Sub[2].nListViewItem);
	tvItem.pszText = TreeStr;
	tvItem.hItem = g_Preview_State.Category[0].Sub[2].TreeViewItemID;
	TreeView_SetItem(hTreeView, &tvItem);

	// PCE
	tvItem.iImage = 5;
	tvItem.iSelectedImage = 5;
	LoadString(g_main_instance, IDS_STRING_PRVS0_05 , str, 64);
	wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[0].Sub[3].nListValidItem, g_Preview_State.Category[0].Sub[2].nListViewItem);
	tvItem.pszText = TreeStr;
	tvItem.hItem = g_Preview_State.Category[0].Sub[3].TreeViewItemID;
	TreeView_SetItem(hTreeView, &tvItem);

	tvItem.iImage = 0;
	tvItem.iSelectedImage = 1;
	if(g_Preview_State.Category[1].nSubCategory){
		for(int i=0; i<g_Preview_State.Category[1].nSubCategory; i++){
			LoadString(g_main_instance, IDS_STRING_PRVS1_01 + i , str, 64);
			wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[1].Sub[i].nListValidItem, g_Preview_State.Category[1].Sub[i].nListViewItem);
			tvItem.pszText = TreeStr;
			tvItem.hItem = g_Preview_State.Category[1].Sub[i].TreeViewItemID;
			TreeView_SetItem(hTreeView, &tvItem);
		}
	}

}


void MyTreeView_CreateCategory(){
	TV_INSERTSTRUCT tv;
	HWND hTreeView = g_Preview_State.hTreeView;
	int i, j, *sortitem = g_Preview_State.DB.pListViewSortItem;
	char TreeStr[64], str[64];
/*
	const char *CountryStr[] = { "As", AsH, AsZ, Aus, E, EB, EM, EX, GH, J, JB, JH, JM, JMB, JU, JUE,
		JUEH, JUEZ, JUH, JUZ, JX, JXZ, JZ, JZB, O, P, PD, T, U, UB, UE, UEX, UH, UM, UMB,
		UX, UXB, UXH, UXZ, UZ, UZB, V, VB, Z, ZB, NULL};
*/

	int nAll = g_Preview_State.DB.nListViewItem;
	g_Preview_State.nCategory=1;
	g_Preview_State.Category = (struct RootCategory *)malloc(sizeof(struct RootCategory)*1);
	g_Preview_State.Category[0].Sub = (struct SubCategory *)malloc(sizeof(struct SubCategory)*4);
#ifndef _NES_ONLY
	g_Preview_State.Category[0].nSubCategory = 4;
#else
	g_Preview_State.Category[0].nSubCategory = 3;
#endif
	g_Preview_State.Category[0].Sub[0].pListViewAllItem = (int *)malloc(sizeof(int)*nAll);
	g_Preview_State.Category[0].Sub[0].pListViewValidItem = (int *)malloc(sizeof(int)*nAll);
	g_Preview_State.Category[0].Sub[0].pListViewInValidItem = (int *)malloc(sizeof(int)*nAll);
	g_Preview_State.Category[0].Sub[0].nListViewItem = nAll;
	for(j=0; j<nAll; j++){
		g_Preview_State.Category[0].Sub[0].pListViewAllItem[j] = j;
	}
	// sort
	PreviewDataStructSort(g_Preview_State.Category[0].Sub[0].pListViewAllItem, nAll, 0, 1);

//	memset(g_Preview_State.Category[0].Sub[0].pListViewAllItem, 0, sizeof(int)*i);
	memset(g_Preview_State.Category[0].Sub[0].pListViewValidItem, 0, sizeof(int)*nAll);
	memset(g_Preview_State.Category[0].Sub[0].pListViewInValidItem, 0, sizeof(int)*nAll);
	SetListViewValidItem(0, 0);

	memset((void *)&tv, 0, sizeof(tv));
	tv.hInsertAfter = TVI_LAST;
	tv.item.mask = TVIF_TEXT | TVIF_IMAGE| TVIF_SELECTEDIMAGE;
	tv.hParent = TVI_ROOT;
	tv.item.iImage = 0;
	tv.item.iSelectedImage = 1;
	LoadString(g_main_instance, IDS_STRING_PRVRC_01 , str, 64);
	tv.item.pszText = str;
	g_Preview_State.Category[0].TreeViewItemID = TreeView_InsertItem(hTreeView, &tv);

	tv.hParent = g_Preview_State.Category[0].TreeViewItemID;
	tv.item.iImage = 2;
	tv.item.iSelectedImage = 2;
	LoadString(g_main_instance, IDS_STRING_PRVS0_01 , str, 64);
	wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[0].Sub[0].nListValidItem,g_Preview_State.DB.nListViewItem);
	tv.item.pszText = TreeStr;
	g_Preview_State.Category[0].Sub[0].TreeViewItemID = TreeView_InsertItem(hTreeView, &tv);

	// MAIN->NES
	i = g_Preview_State.DB.nListViewNESItem;
	g_Preview_State.Category[0].Sub[1].pListViewAllItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[0].Sub[1].pListViewValidItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[0].Sub[1].pListViewInValidItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[0].Sub[1].nListViewItem = i;
/*
	for(j=0; j<i; j++){
		g_Preview_State.Category[0].Sub[1].pListViewAllItem[j] = j;
	}
*/
	{
		int k;
		for(j=0, k=0; j<nAll; j++){
			if(!(g_Preview_State.DB.pListViewItem[g_Preview_State.DB.pListViewSortItem[j]].Flag >> 1))
				g_Preview_State.Category[0].Sub[1].pListViewAllItem[k++] = g_Preview_State.DB.pListViewSortItem[j];
		}
	}
	memset(g_Preview_State.Category[0].Sub[1].pListViewValidItem, 0, sizeof(int)*i);
	memset(g_Preview_State.Category[0].Sub[1].pListViewInValidItem, 0, sizeof(int)*i);
	SetListViewValidItem(0, 1);

	tv.hParent = g_Preview_State.Category[0].TreeViewItemID;
	tv.item.iImage = 3;
	tv.item.iSelectedImage = 3;
	LoadString(g_main_instance, IDS_STRING_PRVS0_02 , str, 64);
	wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[0].Sub[1].nListValidItem, g_Preview_State.Category[0].Sub[1].nListViewItem);
	tv.item.pszText = TreeStr;
	g_Preview_State.Category[0].Sub[1].TreeViewItemID = TreeView_InsertItem(hTreeView, &tv);

	// MAIN->FDS
	i = g_Preview_State.DB.nListViewFDSItem;
	g_Preview_State.Category[0].Sub[2].pListViewAllItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[0].Sub[2].pListViewValidItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[0].Sub[2].pListViewInValidItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[0].Sub[2].nListViewItem = g_Preview_State.DB.nListViewFDSItem;
/*
	{
		int k;
		for(k=0, j=g_Preview_State.DB.nListViewNESItem; j<nAll; j++, k++){
			g_Preview_State.Category[0].Sub[2].pListViewAllItem[k] = j;
		}
	}
*/
	{
		int k;
		for(j=0, k=0; j<nAll && k!=g_Preview_State.DB.nListViewFDSItem; j++){
			if((g_Preview_State.DB.pListViewItem[g_Preview_State.DB.pListViewSortItem[j]].Flag >> 1)==1)
				g_Preview_State.Category[0].Sub[2].pListViewAllItem[k++] = g_Preview_State.DB.pListViewSortItem[j];
		}
	}
	memset(g_Preview_State.Category[0].Sub[2].pListViewValidItem, 0, sizeof(int)*i);
	memset(g_Preview_State.Category[0].Sub[2].pListViewInValidItem, 0, sizeof(int)*i);
	SetListViewValidItem(0, 2);

	tv.hParent = g_Preview_State.Category[0].TreeViewItemID;
	tv.item.iImage = 4;
	tv.item.iSelectedImage = 4;
	LoadString(g_main_instance, IDS_STRING_PRVS0_03 , str, 64);
	wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[0].Sub[2].nListValidItem, g_Preview_State.Category[0].Sub[2].nListViewItem);
	tv.item.pszText = TreeStr;
	g_Preview_State.Category[0].Sub[2].TreeViewItemID = TreeView_InsertItem(hTreeView, &tv);

#ifndef _NES_ONLY
	// MAIN->PCE
	i = g_Preview_State.DB.nListViewPCEItem;
	g_Preview_State.Category[0].Sub[3].pListViewAllItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[0].Sub[3].pListViewValidItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[0].Sub[3].pListViewInValidItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[0].Sub[3].nListViewItem = g_Preview_State.DB.nListViewPCEItem;
	{
		int k;
		for(j=0, k=0; j<nAll && k!=g_Preview_State.DB.nListViewPCEItem; j++){
			if((g_Preview_State.DB.pListViewItem[g_Preview_State.DB.pListViewSortItem[j]].Flag >> 1)==3)
				g_Preview_State.Category[0].Sub[3].pListViewAllItem[k++] = g_Preview_State.DB.pListViewSortItem[j];
		}
	}
	memset(g_Preview_State.Category[0].Sub[3].pListViewValidItem, 0, sizeof(int)*i);
	memset(g_Preview_State.Category[0].Sub[3].pListViewInValidItem, 0, sizeof(int)*i);
	SetListViewValidItem(0, 3);
	tv.hParent = g_Preview_State.Category[0].TreeViewItemID;
	tv.item.iImage = 5;
	tv.item.iSelectedImage = 5;
	LoadString(g_main_instance, IDS_STRING_PRVS0_05 , str, 64);
	wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[0].Sub[3].nListValidItem, g_Preview_State.Category[0].Sub[3].nListViewItem);
	tv.item.pszText = TreeStr;
	g_Preview_State.Category[0].Sub[3].TreeViewItemID = TreeView_InsertItem(hTreeView, &tv);
#endif


/*
	// Country
	g_Preview_State.Category[1].Sub = (struct SubCategory *)malloc(sizeof(struct SubCategory));
	g_Preview_State.Category[1].nSubCategory = 1;
	g_Preview_State.Category[1].Sub[0].pListViewAllItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[1].Sub[0].pListViewValidItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[1].Sub[0].pListViewInValidItem = (int *)malloc(sizeof(int)*i);
	g_Preview_State.Category[1].Sub[0].nListViewItem = i;
	for(j=0; j<i; j++){
		g_Preview_State.Category[1].Sub[0].pListViewAllItem[j] = j;
	}
//	memset(g_Preview_State.Category[0].Sub[0].pListViewAllItem, 0, sizeof(int)*i);
	memset(g_Preview_State.Category[0].Sub[0].pListViewValidItem, 0, sizeof(int)*i);
	memset(g_Preview_State.Category[0].Sub[0].pListViewInValidItem, 0, sizeof(int)*i);
	SetListViewValidItem(1, 0);
*/
#if 1
	// Genre
	i = g_Preview_State.nCategory;
	g_Preview_State.nCategory++;
	{
		struct RootCategory *tp;
		tp = (struct RootCategory *)realloc(g_Preview_State.Category , sizeof(struct RootCategory)*g_Preview_State.nCategory);
		g_Preview_State.Category = tp;
	}
	memset(&g_Preview_State.Category[i], 0, sizeof(struct RootCategory));
	for(j=0; GenreStr[j]; j++);
	g_Preview_State.Category[i].Sub = (struct SubCategory *)malloc(sizeof(struct SubCategory)*j);
//	g_Preview_State.Category[i].nSubCategory = j;
	memset((void *)&tv, 0, sizeof(tv));
	tv.hInsertAfter = TVI_LAST;
	tv.item.mask = TVIF_TEXT | TVIF_IMAGE| TVIF_SELECTEDIMAGE;
	tv.hParent = TVI_ROOT;
	tv.item.iImage = 0;
	tv.item.iSelectedImage = 1;
	LoadString(g_main_instance, IDS_STRING_PRVRC_02 , str, 64);
	tv.item.pszText = str;
	g_Preview_State.Category[i].TreeViewItemID = TreeView_InsertItem(hTreeView, &tv);

	{
		int k;
		tv.hParent = g_Preview_State.Category[i].TreeViewItemID;
		tv.item.iImage = 0;
		tv.item.iSelectedImage = 1;
		g_Preview_State.Category[i].nSubCategory = 0;
#if 0
		for(k=0; k<j; k++){
			int n=0;
			g_Preview_State.Category[i].Sub[k].pListViewAllItem = (int *)malloc(sizeof(int)*nAll);
			g_Preview_State.Category[i].Sub[k].pListViewValidItem = (int *)malloc(sizeof(int)*nAll);
			g_Preview_State.Category[i].Sub[k].pListViewInValidItem = (int *)malloc(sizeof(int)*nAll);
//			g_Preview_State.Category[i].Sub[k].nListViewItem = nAll;
//			memset(g_Preview_State.Category[i].Sub[k].pListViewAllItem, 0, sizeof(int)*nAll);
//			memset(g_Preview_State.Category[i].Sub[k].pListViewValidItem, 0, sizeof(int)*nAll);
//			memset(g_Preview_State.Category[i].Sub[k].pListViewInValidItem, 0, sizeof(int)*nAll);
			for(int l=0; l<nAll; l++){
				if(!strcmp(g_Preview_State.DB.pListViewItem[l].Genre, GenreStr[k])){
					g_Preview_State.Category[i].Sub[k].pListViewAllItem[n++] = l;
					break;
				}
			}
#if 1			// メモリ節約
			{
				int *tp;
				tp = (int *)realloc(g_Preview_State.Category[i].Sub[k].pListViewAllItem, sizeof(int)*n);
				g_Preview_State.Category[i].Sub[k].pListViewAllItem = tp;
				tp = (int *)realloc(g_Preview_State.Category[i].Sub[k].pListViewValidItem, sizeof(int)*n);
				g_Preview_State.Category[i].Sub[k].pListViewValidItem = tp;
				tp = (int *)realloc(g_Preview_State.Category[i].Sub[k].pListViewInValidItem, sizeof(int)*n);
				g_Preview_State.Category[i].Sub[k].pListViewInValidItem = tp;
			}

#endif
			g_Preview_State.Category[i].Sub[k].nListViewItem = n;
			SetListViewValidItem(i, k);
			LoadString(g_main_instance, IDS_STRING_PRVS1_01 + k , str, 64);
			wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[i].Sub[k].nListValidItem, n);
			tv.item.pszText = TreeStr;
			g_Preview_State.Category[i].Sub[k].TreeViewItemID = TreeView_InsertItem(hTreeView, &tv);
			g_Preview_State.Category[i].nSubCategory++;
		}
#else 
		for(k=0; k<j; k++){
			g_Preview_State.Category[i].Sub[k].pListViewAllItem = (int *)malloc(sizeof(int)*nAll);
			g_Preview_State.Category[i].Sub[k].pListViewValidItem = (int *)malloc(sizeof(int)*nAll);
			g_Preview_State.Category[i].Sub[k].pListViewInValidItem = (int *)malloc(sizeof(int)*nAll);
			g_Preview_State.Category[i].Sub[k].nListViewItem = 0;
		}
		int n=0;
		for(int l=0; l<nAll; l++){
			for(k=0; k<j; k++){
				if(!strcmp(g_Preview_State.DB.pListViewItem[g_Preview_State.DB.pListViewSortItem[l]].Genre, GenreStr[k])){
					g_Preview_State.Category[i].Sub[k].pListViewAllItem[g_Preview_State.Category[i].Sub[k].nListViewItem++] = g_Preview_State.DB.pListViewSortItem[l];
					break;
				}
			}
		}
#if 1			// メモリ節約
		for(k=0; k<j; k++){
			int *tp;
			int sn = g_Preview_State.Category[i].Sub[k].nListViewItem * sizeof(int);
			tp = (int *)realloc(g_Preview_State.Category[i].Sub[k].pListViewAllItem, sn);
			g_Preview_State.Category[i].Sub[k].pListViewAllItem = tp;
			tp = (int *)realloc(g_Preview_State.Category[i].Sub[k].pListViewValidItem, sn);
			g_Preview_State.Category[i].Sub[k].pListViewValidItem = tp;
			tp = (int *)realloc(g_Preview_State.Category[i].Sub[k].pListViewInValidItem, sn);
			g_Preview_State.Category[i].Sub[k].pListViewInValidItem = tp;
		}
#endif
		for(k=0; k<j; k++){
			SetListViewValidItem(i, k);
			LoadString(g_main_instance, IDS_STRING_PRVS1_01 + k , str, 64);
			wsprintf(TreeStr, "%s (%u/%u)", str, g_Preview_State.Category[i].Sub[k].nListValidItem, g_Preview_State.Category[i].Sub[k].nListViewItem);
			tv.item.pszText = TreeStr;
			g_Preview_State.Category[i].Sub[k].TreeViewItemID = TreeView_InsertItem(hTreeView, &tv);
		}
		g_Preview_State.Category[i].nSubCategory=j;
#endif
	}
#endif

#if 0
	///********----------  My Database  ----------*********////
	i = g_Preview_State.nCategory;
	g_Preview_State.nCategory++;
	{
		struct RootCategory *tp;
		tp = (struct RootCategory *)realloc(g_Preview_State.Category , sizeof(struct RootCategory)*g_Preview_State.nCategory);
		g_Preview_State.Category = tp;
	}
	memset(&g_Preview_State.Category[i], 0, sizeof(struct RootCategory));
	for(j=0; GenreStr[j]; j++);
	g_Preview_State.Category[i].Sub = (struct SubCategory *)malloc(sizeof(struct SubCategory)*j);
//	g_Preview_State.Category[i].nSubCategory = j;
	memset((void *)&tv, 0, sizeof(tv));
	tv.hInsertAfter = TVI_LAST;
	tv.item.mask = TVIF_TEXT | TVIF_IMAGE| TVIF_SELECTEDIMAGE;
	tv.hParent = TVI_ROOT;
	tv.item.iImage = 0;
	tv.item.iSelectedImage = 1;
	LoadString(g_main_instance, IDS_STRING_PRVRC_02 , str, 64);
	tv.item.pszText = str;
	g_Preview_State.Category[i].TreeViewItemID = TreeView_InsertItem(hTreeView, &tv);

	{
		int k;
		tv.hParent = g_Preview_State.Category[i].TreeViewItemID;
		tv.item.iImage = 0;
		tv.item.iSelectedImage = 1;
		g_Preview_State.Category[i].nSubCategory = 0;

		for(k=0; k<j; k++){
			g_Preview_State.Category[i].Sub[k].pListViewAllItem = (int *)malloc(sizeof(int)*nAll);
			g_Preview_State.Category[i].Sub[k].pListViewValidItem = (int *)malloc(sizeof(int)*nAll);
			g_Preview_State.Category[i].Sub[k].pListViewInValidItem = (int *)malloc(sizeof(int)*nAll);
			g_Preview_State.Category[i].Sub[k].nListViewItem = 0;
		}
		int n=0;
		for(int l=0; l<nAll; l++){
			for(k=0; k<j; k++){
				if(!strcmp(g_Preview_State.DB.pListViewItem[l].Genre, GenreStr[k])){
					g_Preview_State.Category[i].Sub[k].pListViewAllItem[g_Preview_State.Category[i].Sub[k].nListViewItem++] = l;
					break;
				}
			}
		}
	}
#endif

	//////////////------------------


	TreeView_SelectItem(hTreeView, g_Preview_State.Category[0].Sub[0].TreeViewItemID);	
//	PreviewToolChangeDisplay();
//	InvalidateRect(Settings.hListViewWindow, NULL, TRUE);
}



HWND CreateTreeWnd(HWND hWnd, HINSTANCE hInst){
	HWND tmphwnd;
	tmphwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, NULL, WS_CHILD | WS_BORDER | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
			0, 0, 0, 0, hWnd, (HMENU)ID_TREEVIEW, hInst, NULL);
//	SendMessage(tmphwnd, SB_SIMPLE, TRUE, 0L);
	{
		HIMAGELIST hITree;
		hITree = ImageList_Create(16, 16, ILC_COLOR8, 4, 0);
		ImageList_AddIcon(hITree, LoadIcon(hInst, MAKEINTRESOURCE(IDI_FOLDICON1)));
		ImageList_AddIcon(hITree, LoadIcon(hInst, MAKEINTRESOURCE(IDI_FOLDICON2)));
		ImageList_AddIcon(hITree, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ROMICON2)));
		ImageList_AddIcon(hITree, LoadIcon(hInst, MAKEINTRESOURCE(IDI_CARTICON)));
		ImageList_AddIcon(hITree, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ROMICON4)));
		ImageList_AddIcon(hITree, LoadIcon(hInst, MAKEINTRESOURCE(IDI_PCE)));

		TreeView_SetImageList(tmphwnd, hITree, TVSIL_NORMAL);
		g_Preview_State.hImageTree = hITree;
	}
	g_Preview_State.hTreeView = tmphwnd;
	return tmphwnd;
}



void MyChangeToolTipColor(HWND hWnd){
	SendMessage(hWnd, TTM_SETTIPBKCOLOR, g_Preview_State.ToolTipBGColor, (LPARAM)0);
	SendMessage(hWnd, TTM_SETTIPTEXTCOLOR, g_Preview_State.ToolTipTXTColor, (LPARAM)0);
}


void MyChangeAllToolTipColor(){
	MyChangeToolTipColor(TreeView_GetToolTips(g_Preview_State.hTreeView));
	MyChangeToolTipColor(ListView_GetToolTips(g_Preview_State.hListView));
	MyChangeToolTipColor(g_Preview_State.hToolTip);
}


void Create_MainPreviewWind(HWND hwnd, HINSTANCE hInst){
	DWORD dwStyle;
	RECT rect;
	Mainwindow_Preview = CreateWindowEx(0,WC_LISTVIEW, "",WS_BORDER | WS_CHILD | WS_VISIBLE | LVS_REPORT /*| LVS_EDITLABELS*/ | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_OWNERDATA,
				0, 0, 0, 0, hwnd,(HMENU)ID_PREVIELV, hInst,NULL);
	if(Mainwindow_Preview==NULL)
		return;
	g_Preview_State.hListView = Mainwindow_Preview;
	dwStyle = ListView_GetExtendedListViewStyle(Mainwindow_Preview);
	dwStyle |= LVS_EX_GRIDLINES| LVS_EX_FULLROWSELECT| LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP;// | LVS_EX_FULLROWSELECT ;
	ListView_SetExtendedListViewStyle(Mainwindow_Preview, dwStyle);
	ListView_SetBkColor(Mainwindow_Preview, g_Preview_State.BGColor);
	ListView_SetTextBkColor(Mainwindow_Preview, g_Preview_State.BGColor);
	ListView_SetTextColor(Mainwindow_Preview, g_Preview_State.FontColor);

	{	// screen bitmap load
		char str[MAX_PATH];
		g_Preview_State.hBitmap = NULL;
		GetModuleFileName(NULL, str, MAX_PATH);
		PathRemoveFileSpec(str);
		PathCombine(str , str, "screen.bmp");
		g_Preview_State.hBitmap = (HBITMAP)LoadImage(NULL, str, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	}

	g_Preview_State.hFont = CreateFontIndirect(&g_Preview_State.LogFont);
	if(g_Preview_State.hFont!=NULL){
		SendMessage(Mainwindow_Preview, WM_SETFONT, (WPARAM)g_Preview_State.hFont, MAKELPARAM(TRUE, 0));
	}
	GetClientRect(hwnd, &rect);
//	SendMessage(main_window_handle, WM_SIZE, (WPARAM)0, MAKELPARAM(rect.right - rect.left, rect.bottom - rect.top));
	MoveWindow(Mainwindow_Preview, 264, 0, rect.right - rect.left, rect.bottom - rect.top-Mainwindowsbhs, TRUE);
	InvalidateRect(Mainwindow_Preview, NULL, TRUE);
	InvalidateRect(hwnd, NULL, TRUE);


//	ListView_SetItemCount(Mainwindow_Preview, 0);
	g_Preview_State.DB.pListViewItem = NULL;
	g_Preview_State.DB.nListViewItemMem = 0;

	{
		int i;
		char str[64];

		for(i=0;i<COLUMN_NUM;i++){
			if(g_Preview_State.Column_w[i] < 10)
				g_Preview_State.Column_w[i] = 80;
		}

		LV_COLUMN lvcol;
		lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM ;

		for(i=0;i<COLUMN_NUM;i++){
			LoadString(g_main_instance, IDS_STRING_PREVCT1+i , str, 64);
			if(i==4 || i==6)
				lvcol.fmt = LVCFMT_RIGHT;
			else
				lvcol.fmt = LVCFMT_LEFT;
			lvcol.cx = g_Preview_State.Column_w[i];
			lvcol.pszText = str;
			lvcol.iSubItem = i;
			ListView_InsertColumn(Mainwindow_Preview, i, &lvcol);
		}
		ListView_SetColumnOrderArray(Mainwindow_Preview, COLUMN_NUM, g_Preview_State.Column_Pos);
	}
	g_PreviewMode=1;
	CreateTreeWnd(hwnd, hInst);
	CreateToolWnd(hwnd, hInst);

	g_Preview_State.hImageList = ImageList_Create(16, 16, ILC_COLOR8, 4, 0);
//	ImageList_AddIcon(g_Preview_State.hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ROMICON1)));
//	ImageList_AddIcon(g_Preview_State.hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ROMICON2)));
	ImageList_AddIcon(g_Preview_State.hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_CARTICON2)));
	ImageList_AddIcon(g_Preview_State.hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_CARTICON)));
	ImageList_AddIcon(g_Preview_State.hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ROMICON3)));
	ImageList_AddIcon(g_Preview_State.hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ROMICON4)));

	// GB
	ImageList_AddIcon(g_Preview_State.hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ROMICON1)));
	ImageList_AddIcon(g_Preview_State.hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ROMICON2)));
	// PCE
	ImageList_AddIcon(g_Preview_State.hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_PCEROM_ICON2)));
	ImageList_AddIcon(g_Preview_State.hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_PCEROM_ICON1)));

	g_Preview_State.nTreeSelect = 0;
	ListView_SetImageList(Mainwindow_Preview, g_Preview_State.hImageList, LVSIL_SMALL);
	SetListViewItem_NES();
	MyTreeView_CreateCategory();
	SetToolWindowItemState();
	if(g_Preview_State.window_w && g_Preview_State.window_h){
		SetWindowPos(hwnd, NULL, 0,0, g_Preview_State.window_w, g_Preview_State.window_h, SWP_NOMOVE | SWP_NOZORDER);
	}
	g_Preview_State.PrevSortItemp[0]=1;

	// Change tooltip color
	MyChangeAllToolTipColor();
//	PreviewListSort(0);
}


void MyTreeView_DestroyCategory(){
	ListView_SetItemCount(Mainwindow_Preview, 0);
	TreeView_DeleteAllItems(g_Preview_State.hTreeView);
	for(int i=0; i<g_Preview_State.nCategory; i++){
		int j;
		for(j=0; j<g_Preview_State.Category[i].nSubCategory; j++){
			if(g_Preview_State.Category[i].Sub[j].pListViewAllItem)
				free(g_Preview_State.Category[i].Sub[j].pListViewAllItem);
			if(g_Preview_State.Category[i].Sub[j].pListViewValidItem)
				free(g_Preview_State.Category[i].Sub[j].pListViewValidItem);
			if(g_Preview_State.Category[i].Sub[j].pListViewInValidItem)
				free(g_Preview_State.Category[i].Sub[j].pListViewInValidItem);
		}
		free(g_Preview_State.Category[i].Sub);
	}
	free(g_Preview_State.Category);
	g_Preview_State.nCategory = 0;
	g_Preview_State.Category = NULL;
}


void Destroy_MainPreviewWind(){
	int i;
	if(Mainwindow_Preview==NULL)
		return;
	for(i=0;i<6;i++){
		g_Preview_State.Column_w[i] = ListView_GetColumnWidth(Mainwindow_Preview, i);
	}
	ListView_GetColumnOrderArray(Mainwindow_Preview, COLUMN_NUM, g_Preview_State.Column_Pos);
	if(g_Preview_State.hBitmap)
		DeleteObject(g_Preview_State.hBitmap);
	DeleteObject(g_Preview_State.hFont);
//	SavePreviewList(Mainwindow_Preview);
	DestroyWindow(Mainwindow_Preview);
	Mainwindow_Preview=NULL;
	if(g_PreviewMode){
		WINDOWPLACEMENT wp;
		wp.length = sizeof( WINDOWPLACEMENT );
		GetWindowPlacement(main_window_handle, &wp);
		g_Preview_State.window_w = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
		g_Preview_State.window_h = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	}
	MyTreeView_DestroyCategory();
	g_PreviewMode=0;
	SaveListFile_NES();
	if(g_Preview_State.DB.pListViewSortItem){
		free(g_Preview_State.DB.pListViewSortItem);
		g_Preview_State.DB.pListViewSortItem=NULL;
	}
	if(g_Preview_State.DB.pListViewItem){
		free(g_Preview_State.DB.pListViewItem);
		g_Preview_State.DB.pListViewItem=NULL;
	}
/*
	for(i=0; i<PREVIEW_CATEGORYMAX; i++){
		if(g_Preview_State.pListViewAllItem[i])
			free(g_Preview_State.pListViewAllItem[i]);
		if(g_Preview_State.pListViewValidItem[i])
			free(g_Preview_State.pListViewValidItem[i]);
		if(g_Preview_State.pListViewInValidItem[i])
			free(g_Preview_State.pListViewInValidItem[i]);
	}
*/
	ImageList_Destroy(g_Preview_State.hImageList);
	ImageList_Destroy(g_Preview_State.hImageTree);
}


extern void setWindowedWindowStyle();

static int Save_Game_Window_Rect = 0;

void Return_PreviewMode(){
	if(Mainwindow_Preview==NULL){
		Create_MainPreviewWind(main_window_handle, g_main_instance);
		return;
	}
	if(!g_PreviewMode){
#if 0
		RECT rect;
		SetWindowPos(main_window_handle, NULL, 0,0, g_Preview_State.window_w, g_Preview_State.window_h, SWP_NOMOVE | SWP_NOZORDER);
		GetClientRect(main_window_handle, &rect);
		MoveWindow(Mainwindow_Preview, 264, 0, rect.right - rect.left-264, rect.bottom - rect.top-Mainwindowsbhs, TRUE);
		g_PreviewMode=1;
#else
		{
			WINDOWPLACEMENT wp;
			wp.length = sizeof( WINDOWPLACEMENT );
			GetWindowPlacement(main_window_handle, &wp);
			g_Preview_State.Game_window_xpos= wp.rcNormalPosition.left;
			g_Preview_State.Game_window_ypos= wp.rcNormalPosition.top;
			Save_Game_Window_Rect = 1;
		}
		RECT rect;
		SetWindowPos(main_window_handle, NULL, g_Preview_State.window_xpos, g_Preview_State.window_ypos, g_Preview_State.window_w, g_Preview_State.window_h, SWP_NOZORDER);
//		SetWindowPos(main_window_handle, NULL, window_xpos,window_ypos, g_Preview_State.window_w, g_Preview_State.window_h, SWP_NOMOVE | SWP_NOZORDER);
		GetClientRect(main_window_handle, &rect);
		MoveWindow(Mainwindow_Preview, 264, 0, rect.right - rect.left-264, rect.bottom - rect.top-Mainwindowsbhs, TRUE);
		ShowWindow(Mainwindow_Preview, SW_SHOW);
//		ShowWindow(g_Preview_State.hTreeView, SW_SHOW);
		ShowWindow(g_Preview_State.hToolTip, SW_SHOW);
		ShowWindow(g_Preview_State.hTool, SW_SHOW);
		g_PreviewMode=1;
		PreviewDisplayModeChange();
		setWindowedWindowStyle();
		GetClientRect(main_window_handle, &rect);
		SendMessage(main_window_handle, WM_SIZE, SIZE_RESTORED, (rect.bottom-rect.top)<<16 | (rect.right-rect.left));
		SetForegroundWindow(main_window_handle);
		if(emu && g_Preview_State.ScreenDispFlag){
			if(g_Preview_State.DisableSound){
				g_Preview_State.DisableSoundBackup = 1;
				emu->enable_sound(0);
			}
			if(g_Preview_State.EmuSpeed){
				emu->ToggleFastFPS();
				g_Preview_State.EmuSpeedBackup= 1;
			}
		}
//		InvalidateRect(main_window_handle, NULL, TRUE);
#endif
	}
}


void Goto_WindowGameMode(){
	if(g_PreviewMode){
#if 0
		RECT rect;
		g_PreviewMode=0;
		GetWindowRect(main_window_handle, &rect);
		g_Preview_State.window_w = rect.right - rect.left;
		g_Preview_State.window_h = rect.bottom - rect.top;
		MoveWindow(Mainwindow_Preview, 0, 0, 0, 0, TRUE);
#else
		{
			WINDOWPLACEMENT wp;
			wp.length = sizeof( WINDOWPLACEMENT );
			GetWindowPlacement(main_window_handle, &wp);
			g_Preview_State.window_xpos = wp.rcNormalPosition.left;
			g_Preview_State.window_ypos = wp.rcNormalPosition.top;
			g_Preview_State.window_w = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			g_Preview_State.window_h = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
		}
		if(Save_Game_Window_Rect)
			SetWindowPos(main_window_handle, NULL, g_Preview_State.Game_window_xpos, g_Preview_State.Game_window_ypos, g_Preview_State.window_w, g_Preview_State.window_h, SWP_NOZORDER);
		g_PreviewMode=0;
		ShowWindow(Mainwindow_Preview, SW_HIDE);
		ShowWindow(g_Preview_State.hTreeView, SW_HIDE);
		ShowWindow(g_Preview_State.hToolTip, SW_HIDE);
		ShowWindow(g_Preview_State.hTool, SW_HIDE);

		if(emu){
			if(g_Preview_State.DisableSoundBackup){
				g_Preview_State.DisableSoundBackup = 0;
				emu->enable_sound(1);
			}
			if(g_Preview_State.EmuSpeedBackup){
				g_Preview_State.EmuSpeedBackup = 0;
				emu->ToggleFastFPS();
			}
		}
		InvalidateRect(main_window_handle, NULL, TRUE);
#endif
	}
}


void PreviewDisplayModeChange(){
	if(g_PreviewMode){
/*
		if(g_Preview_State.ListDispFlag)
			;
		else
			;
*/
		if(g_Preview_State.TreeDispFlag)
			ShowWindow(g_Preview_State.hTreeView, SW_SHOW);
		else
			ShowWindow(g_Preview_State.hTreeView, SW_HIDE);
	}
}


void PreviewToolChangeDisplay(){
	int nItem=0;

	int Root = g_Preview_State.nTreeSelect;
	int SubItem = g_Preview_State.nSubTreeSelect;
	if(SubItem == -1){
		ListView_SetItemCount(Mainwindow_Preview, 0);
		return;
	}
	switch(g_Preview_State.CheckResaultDisplay){
		case 0:		//全て表示
			nItem = g_Preview_State.Category[Root].Sub[SubItem].nListViewItem;
			break;
		case 1:		//所持品のみ表示
			nItem = g_Preview_State.Category[Root].Sub[SubItem].nListValidItem;
			break;
		case 2:		//非所持品のみ表示
			nItem = g_Preview_State.Category[Root].Sub[SubItem].nListInValidItem;
			break;
		default:
			return;
	}
	ListView_SetItemCount(Mainwindow_Preview, nItem);
}





//  IDD_MAINPREVFSDIALOG 
LRESULT CALLBACK PreviewROMDirectoryDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
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
			if(g_Preview_State.nROMDirectory){
				for(i=0; i<g_Preview_State.nROMDirectory; ++i){
					item.mask = LVIF_TEXT | LVIF_PARAM;
					item.pszText = g_Preview_State.ROM_Directory[i];
					item.iItem = i;
					item.iSubItem = 0;
					item.lParam = i;
					ListView_InsertItem(hList, &item);
					if(g_Preview_State.ROM_SubDirectory[i]){
						ListView_SetItemState(hList, i, INDEXTOSTATEIMAGEMASK(2), LVIS_STATEIMAGEMASK);
					}
					else{
						ListView_SetItemState(hList, i, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
					}
				}
			}
			return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wp)) {
				case IDC_PVFSBUTTON1:		//Add
					if(MyFoldOpenDlg(hDlgWnd, Folder)){
						if(!AddPFoldchk(hList, Folder)){
							int n = ListView_GetItemCount(hList);
							item.mask = LVIF_TEXT | LVIF_PARAM;
							item.pszText = Folder;
							item.iItem = n;
							item.iSubItem = 0;
							item.lParam = n;
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
					g_Preview_State.nROMDirectory = 0;
					for(i=-1, j=0;(i=ListView_GetNextItem(hList, i,LVNI_ALL)) != -1; ++j){
						if(j==MAX_ROMDIRECTORY)
							break;
						ListView_GetItemText(hList, i, 0, g_Preview_State.ROM_Directory[j], MAX_PATH);
						g_Preview_State.ROM_SubDirectory[j]=(char)ListView_GetCheckState(hList, i);
						g_Preview_State.nROMDirectory++;
					}
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


void ListCheckResultClear(){
	int i;

	for(i=0; i< g_Preview_State.DB.nListViewItem; i++){
		g_Preview_State.DB.pListViewItem[i].Flag &= 0xfffffffe;
		g_Preview_State.DB.pListViewItem[i].FileName[0]= 0;
	}
	g_Preview_State.DB.ChangeHaveItem = 1;
	SetAllListViewValidItem();
//	SetListViewValidItem(0, 0);
}


// NES
// リストファイルをセーブ
int SaveListFile_NES(){
	int i=0,nMax;
	char *p=NULL;
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes;
	struct NES_ROM_Data *nesi = (NES_ROM_Data *)g_Preview_State.DB.pListViewItem;

	// 所持リストに変化なし
	if(g_Preview_State.DB.ChangeHaveItem == 0)
		return 0;
	p = (char *)malloc(2048);
	if(p==NULL)
		return 0;
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
//	PathCombine(str , str, "list");
	PathCombine(str , str, "nesrom.lst");
	hFile = CreateFile(str, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		hFile = CreateFile(str, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			free(p);
			return 0;
		}
	}

	nMax = g_Preview_State.DB.nListViewItem;
	{
		char *sig = "NNNesterJ\r\n";
		WriteFile(hFile, sig, strlen(sig), &dwAccBytes, NULL);
	}

	for(i=0; i<nMax; i++){
		if((nesi[i].Flag&1)==0)
			continue;
		if((nesi[i].Flag>>1) != 1){	// NES
			int j;
			j = wsprintf(p, "%08X;%08X;%08X;%s\r\n", nesi[i].Flag, nesi[i].nPlay, nesi[i].Crc, nesi[i].FileName);
			WriteFile(hFile, p, j, &dwAccBytes, NULL);
		}
		else{		// FDS
			int j, k;
			k = wsprintf(p, "%08X;%08X;%08X;", nesi[i].Flag, nesi[i].nPlay, nesi[i].HeaderInfo1);
			for(j=0; j<nesi[i].HeaderInfo1; j++){
				k+= wsprintf(&p[k], "%08X;", nesi[i].FDS_Crc[j]);
			}
			for(j=0; nesi[i].FileName[j]; j++){
				p[k++] = nesi[i].FileName[j];
			}
			p[k++] = 0x0d;
			p[k++] = 0x0a;
			p[k] = 0x00;
			WriteFile(hFile, p, k, &dwAccBytes, NULL);
		}
	}
	CloseHandle(hFile);
	free(p);
	return 1;
}


// NES
// リストファイルをロード
int LoadListFile_NES(){
	int i=0, k, nMax;
	unsigned char *p=NULL;
	char str[MAX_PATH];
	HANDLE hFile;
	DWORD dwAccBytes, filesize, fp=0;
	struct NES_ROM_Data TmpData;
	struct NES_ROM_Data *nesi = (NES_ROM_Data *)g_Preview_State.DB.pListViewItem;

	g_Preview_State.DB.ChangeHaveItem = 0;
	nMax = g_Preview_State.DB.nListViewItem;
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
//	PathCombine(str , str, "list");
	PathCombine(str , str, "nesrom.lst");
	hFile = CreateFile(str, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return 0;
	}
	filesize = GetFileSize(hFile, NULL);
	p = (unsigned char *)malloc(filesize);
	if(p==NULL){
		return 0;
	}
	ReadFile(hFile, p, filesize, &dwAccBytes, NULL);
	CloseHandle(hFile);

	for(i=0; p[i] != 0 && p[i] != 0x0d; i++);

	if(i>10){		// Old version
		while(1){
			DWORD dw;
			k=0;
			//		memset(&TmpData, 0, sizeof(TmpData));
			while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
				TmpData.Title[k++]=p[fp++];
			}
			if(fp>=filesize)
				break;
			if(p[fp]==0x0d){
				fp+=2;
				continue;
			}
			TmpData.Title[k]=0;
			
			k=0, fp++;
			while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
				str[k++]=p[fp++];
			}
			if(fp>=filesize)
				break;
			if(p[fp]==0x0d){
				fp+=2;
				continue;
			}
			str[k]=0;
			MyStrAtoh(str, &dw);
			TmpData.Flag = dw;
			
			k=0, fp++;
			while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
				str[k++]=p[fp++];
			}
			if(fp>=filesize)
				break;
			if(p[fp]==0x0d){
				fp+=2;
				continue;
			}
			str[k]=0;
			MyStrAtoh(str, &dw);
			TmpData.nPlay = dw;
			
			k=0, fp++;
			while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
				TmpData.FileName[k++]=p[fp++];
			}
			TmpData.FileName[k++]=0;
			
			for(k=0; k<nMax; k++){
				if(!strcmp(nesi[k].Title, TmpData.Title)){
					if((TmpData.Flag&2) == (nesi[k].Flag&2)){
						strcpy(nesi[k].FileName, TmpData.FileName);
						nesi[k].Flag = TmpData.Flag;
						nesi[k].nPlay= TmpData.nPlay;
						break;
					}
				}
			}
			while(fp<filesize && p[fp]!=0x0d)fp++;
			if(fp>=filesize)
				break;
			if(p[fp]==0x0d){
				fp+=2;
			}
		}
	}
	else{	// new
		while(1){
			DWORD dw;
			unsigned int type;
			k=0;
			//		memset(&TmpData, 0, sizeof(TmpData));
			while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
				str[k++]=p[fp++];
			}
			if(fp>=filesize)
				break;
			if(p[fp]==0x0d){
				fp+=2;
				continue;
			}
			str[k]=0;
			MyStrAtoh(str, &dw);
			TmpData.Flag = dw;

			k=0, fp++;
			while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
				str[k++]=p[fp++];
			}
			if(fp>=filesize)
				break;
			if(p[fp]==0x0d){
				fp+=2;
				continue;
			}
			str[k]=0;
			MyStrAtoh(str, &dw);
			TmpData.nPlay = dw;
/*
			k=0, fp++;
			while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
				str[k++]=p[fp++];
			}
			if(fp>=filesize)
				break;
			if(p[fp]==0x0d){
				fp+=2;
				continue;
			}
			str[k]=0;
			MyStrAtoh(str, &dw);
			TmpData.nPlay = dw;
*/
			type = TmpData.Flag>>1;
			if(type != 1){	// 
				k=0, fp++;
				while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
					str[k++]=p[fp++];
				}
				if(fp>=filesize)
					break;
				if(p[fp]==0x0d){
					fp+=2;
					continue;
				}
				str[k]=0;
				MyStrAtoh(str, &dw);
				TmpData.Crc= dw;
			}
			else{	// FDS
				int n;

				k=0, fp++;
				while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
					str[k++]=p[fp++];
				}
				if(fp>=filesize)
					break;
				if(p[fp]==0x0d){
					fp+=2;
					continue;
				}
				str[k]=0;
				MyStrAtoh(str, &dw);
				TmpData.HeaderInfo1= (char)dw;

				for(n=0; n<TmpData.HeaderInfo1; n++){
					k=0, fp++;
					while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
						str[k++]=p[fp++];
					}
					if(fp>=filesize)
						break;
					if(p[fp]==0x0d){
						fp+=2;
						continue;
					}
					str[k]=0;
					MyStrAtoh(str, &dw);
					TmpData.FDS_Crc[n]= dw;
				}
			}

			
			k=0, fp++;
			while(fp<filesize && p[fp]!=';' && p[fp]!=0x0d){
				TmpData.FileName[k++]=p[fp++];
			}
			TmpData.FileName[k++]=0;

			if(type != 1){	// NES
				for(k=0; k<nMax; k++){
					if(type == (nesi[k].Flag>>1)){
						if(nesi[k].Crc == TmpData.Crc){
							strcpy(nesi[k].FileName, TmpData.FileName);
							nesi[k].Flag = TmpData.Flag;
							nesi[k].nPlay= TmpData.nPlay;
							break;
						}
					}
				}
			}
			else{		// FDS
				for(k=0; k<nMax; k++){
					if((nesi[k].Flag>>1)==1){
						int n;
						if(TmpData.HeaderInfo1 == nesi[k].HeaderInfo1){
							for(n=0; n<TmpData.HeaderInfo1; n++){
								if(nesi[k].FDS_Crc[n] != TmpData.FDS_Crc[n]){
									break;
								}
							}
							if(n==TmpData.HeaderInfo1){
								strcpy(nesi[k].FileName, TmpData.FileName);
								nesi[k].Flag = TmpData.Flag;
								nesi[k].nPlay= TmpData.nPlay;
							}
						}
					}
				}
			}
			while(fp<filesize && p[fp]!=0x0d)fp++;
			if(fp>=filesize)
				break;
			if(p[fp]==0x0d){
				fp+=2;
			}
		}

	}
	if(p)
		free(p);
//	wsprintf(str, "NES [%u]", i);
//	SetWindowText(ghWnd, str);
	return 1;
}


//  result = { Header [0x10], ALL CRC [4], Filesize[4], }
int CheckOneROMFile(char *fn, unsigned char *result){
//	const char *nesExtensions[] = { "*.nes","*.fds","*.fam","*.unf","*.nsf",NULL };
//	const char *arcExtensions[] = { "zip","lzh","rar","gca",NULL };

	unsigned int crc=0;
	unsigned char *temp=NULL;
	HANDLE hFile;
	DWORD dwAccBytes, filesize, headerid;
	int arcflag=0, romtype;

#if 1	//	file extension check
	{
//		int i;

		romtype = CheckROMFile(fn, main_window_handle, &arcflag);
		if(romtype==-1)
			return 0;
#if 0
		char *extp = PathFindExtension(fn);
		if(extp==NULL)
			return 0;
		extp++;
		for(i=0; arcExtensions[i]; i++){
			if(MystrCmp(extp, (char *)arcExtensions[i]))
				break;
		}
		if(arcExtensions[i]){
			arcflag=1;
		}
#endif
	}
#endif

	if(!arcflag){
		hFile = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			return 0;
		}
		filesize = GetFileSize(hFile, NULL);
		temp = (unsigned char *)malloc(filesize);
		if(temp==NULL){
			CloseHandle(hFile);
			return 0;
		}
		ReadFile(hFile, temp, filesize, &dwAccBytes, NULL);
		CloseHandle(hFile);
	}
	else{
		if(!UncompressROMFile(fn, main_window_handle, &temp, &filesize, romtype)){
			return 0;
		}
	}

	headerid=*(DWORD *)&temp[0];
	if(romtype == EMUTYPE_NES){
		if(headerid==0x1A53454E){	//NES
			DWORD romsize;
			DWORD PROM_crc=0;
			
			romsize = temp[4] * 0x4000;
			romsize += temp[5] * 0x2000;
			if(temp[6]&4)
				romsize+=512;
			if(g_Preview_State.NESROM_CRCFrom_Filesize || (romsize > (filesize-0x10))){
				romsize = filesize-0x10;
			}
			crc = CrcCalc(temp+0x10, romsize);
			romsize = temp[4] * 0x4000;
			if(temp[6]&4)
				romsize+=512;
			if((filesize-0x10) >= romsize)
				PROM_crc = CrcCalc(temp+0x10, romsize);
			else
				PROM_crc = 0;
				/*
				
				  romsize = temp[4] * 0x4000;
				  if(temp[6]&4)
				  romsize += 512;
				  if((filesize-0x10) >= romsize){
				  PROM_crc = CrcCalc(temp+0x10, romsize);
				  }
				  if(g_Preview_State.NESROM_CRCFrom_Filesize){
				  romsize = filesize-0x10;
				  }
				  else{
				  romsize = temp[4] * 0x4000;
				  romsize += temp[5] * 0x2000;
				  if(temp[6]&4)
				  romsize+=512;
				  }
				  if((filesize-0x10) >= romsize){
				  crc = CrcCalc(temp+0x10, romsize);
				  }
			*/
			for(int i=0;i<0x10;i++){
				result[i] = temp[i];
			}
			*(DWORD *)&result[0x10] = crc;
			*(DWORD *)&result[0x14] = filesize;
			*(DWORD *)&result[0x18] = PROM_crc;
		}
		else if(headerid==0x1A534446){	//FDS
			struct MyPreviewFDSDiskData *pDisk;
			struct MyPreviewFDSBlockData *pBlock;
			DWORD blocksize, *FDS_Crc = (DWORD *)&result[12];
			int j=0, i = 0x4A;
			unsigned int cp = 0x10, nSide, imagep=0x10;
			nSide = (unsigned int)temp[4];

			for(unsigned int cs=0; cs<nSide; cs++){
				pDisk = (struct MyPreviewFDSDiskData *)&result[cp];
				
				imagep=cs*65500+0x10;
				pDisk->MakerID = temp[imagep+0x0f];
				*(DWORD *)&pDisk->GameID[0] = *(DWORD *)&temp[imagep+0x10];
				pDisk->GameVer = temp[imagep+0x14];
				pDisk->SideNum = temp[imagep+0x15];
				pDisk->AddNum1 = temp[imagep+0x16];
				pDisk->AddNum2 = temp[imagep+0x17];
				pDisk->AddNum3 = temp[imagep+0x18];
				pDisk->MakerDate[0] = temp[imagep+0x1f];
				pDisk->MakerDate[1] = temp[imagep+0x20];
				pDisk->MakerDate[2] = temp[imagep+0x21];
				pDisk->CreateDate[0] = temp[imagep+0x2c];
				pDisk->CreateDate[1] = temp[imagep+0x2d];
				pDisk->CreateDate[2] = temp[imagep+0x2e];
				pDisk->nBlock = (int)temp[imagep+0x39];
				imagep+=0x3a;
				cp+=sizeof(struct MyPreviewFDSDiskData);
				for(int cb=0; /*cb<nBlock &&*/ imagep<filesize; cb++){
					if(temp[imagep]!=3)
						break;
					pBlock = (struct MyPreviewFDSBlockData *)&result[cp];
					cp+=sizeof(struct MyPreviewFDSBlockData);

					blocksize= temp[13+imagep]&0xff |(int)temp[14+imagep]<<8;
					pBlock->FileSize = blocksize;
					pBlock->FileType = temp[15+imagep];
					pBlock->FileNum1 = temp[1+imagep];
					pBlock->FileNum2 = temp[2+imagep];
					*(DWORD *)&pBlock->FileName[0] = *(DWORD *)&temp[imagep+3];
					*(DWORD *)&pBlock->FileName[4] = *(DWORD *)&temp[imagep+7];
					pBlock->CRC = CrcCalc((unsigned char *)&temp[imagep+17], blocksize);
					imagep+=blocksize+17;
				}
				pDisk->nRealBlock = cb;
			}
			*(DWORD *)&result[0] = filesize;
			*(DWORD *)&result[4] = (DWORD)temp[4];
			*(DWORD *)&result[8] = j;
/*
	offset	byte
		0		4	filesize
		4		4	side
*/
		}
	}
	else if(romtype == EMUTYPE_PCE){
		DWORD romsize;
		romsize = filesize;
		*(DWORD *)&result[0] = filesize;
		*(DWORD *)&result[4] = CrcCalc(temp, romsize);
	}
	if(temp)
		free(temp);
	return 1;
}


void PCEROMCheckResultToText(unsigned char *Result, char *str, char *fn){
	char temp[32];

	strcpy(str, fn);
	wsprintf(temp, "\r\nFilesize = %u Byte\r\n", *((DWORD *)&Result[0]));
	strcat(str, temp);
	wsprintf(temp, "CRC32 = 0x%08X\r\n", *((DWORD *)&Result[4]));
	strcat(str, temp);
	strcat(str, "\r\n\r\n");
}



void BCDDatetoStr(unsigned char *date, char *s){
	char Dcstr[16] = "1925/00/00";

	if(date[0]>>4<=9)
		Dcstr[2]+=(date[0]>>4);
	if(date[0]&0x0f<=9)
		Dcstr[3]+=(date[0]&0xf);
	if(date[1]>>4<=9)
		Dcstr[5]+=(date[1]>>4);
	if(date[1]&0x0f<=9)
		Dcstr[6]+=(date[1]&0xf);
	if(date[2]>>4<=9)
		Dcstr[8]+=(date[2]>>4);
	if(date[2]&0x0f<=9)
		Dcstr[9]+=(date[2]&0xf);
	strcpy(s, Dcstr);
}



void FDSROMCheckResultToText(unsigned char *Result, char *str, char *fn){
//	int i;
	char temp[256];
	const char *sFileDataType[] = {"Data", "Character Data", "Name Table"};
//	DWORD *pCrc = (DWORD *)Result[12];

	strcpy(str, fn);
	wsprintf(temp, "\r\nFilesize = %u Byte\r\n", *((DWORD *)&Result[0]));
	strcat(str, temp);
	wsprintf(temp, "Side = %u\r\n", *((DWORD *)&Result[4]));
	strcat(str, temp);
//	wsprintf(temp, "Block = %u\r\n", *((DWORD *)&Result[8]));
//	strcat(str, temp);
/*
	wsprintf(temp, "CRC List\r\n", );
	strcat(str, temp);
	for(i=0; i<n; i++){
		wsprintf(temp, "Block %u = 0x%08X\r\n", i, pCrc[i]);
		strcat(str, temp);
	}
*/
	struct MyPreviewFDSDiskData *pDisk;
	struct MyPreviewFDSBlockData *pBlock;
	int j=0, i = 0x4A;
	unsigned int cp = 0x10, nSide, cs;

	nSide = *(DWORD *)&Result[4];

	for(cs=0; cs<nSide; cs++){
		int nBlock;
		char sDate[16];
		pDisk = (struct MyPreviewFDSDiskData *)&Result[cp];

		wsprintf(temp, "MakerID = 0x%X\r\n", pDisk->MakerID);
		strcat(str, temp);
		wsprintf(temp, "GameID = \"%c%c%c%c\"\r\n", pDisk->GameID[0], pDisk->GameID[1], pDisk->GameID[2], pDisk->GameID[3]);
		strcat(str, temp);

		wsprintf(temp, "MakerID = 0x%X\r\n", pDisk->GameVer);
		strcat(str, temp);
		wsprintf(temp, "SideNum = %u\r\n", pDisk->SideNum);
		strcat(str, temp);
		wsprintf(temp, "AddNum1 = 0x%X\r\n", pDisk->AddNum1);
		strcat(str, temp);
		wsprintf(temp, "AddNum2 = 0x%X\r\n", pDisk->AddNum2);
		strcat(str, temp);
		wsprintf(temp, "AddNum3 = 0x%X\r\n", pDisk->AddNum3);
		strcat(str, temp);

		BCDDatetoStr(pDisk->MakerDate, sDate);
		wsprintf(temp, "MakerDate = %s\r\n", sDate);
		strcat(str, temp);
		BCDDatetoStr(pDisk->CreateDate, sDate);
		wsprintf(temp, "MakerDate = %s\r\n", sDate);
		strcat(str, temp);

		wsprintf(temp, "File Block Num(Header) = %u\r\n", pDisk->nBlock);
		strcat(str, temp);
		wsprintf(temp, "File Block Num = %u\r\n", pDisk->nRealBlock);
		strcat(str, temp);

		wsprintf(temp, "\r\n");
		strcat(str, temp);

		nBlock = pDisk->nRealBlock;
		cp+=sizeof(struct MyPreviewFDSDiskData);
		for(int cb=0; cb<nBlock; cb++){
			pBlock = (struct MyPreviewFDSBlockData *)&Result[cp];
			cp+=sizeof(struct MyPreviewFDSBlockData);

			wsprintf(temp, "File Size = %u\r\n", pBlock->FileSize);
			strcat(str, temp);
			wsprintf(temp, "CRC = 0x%X\r\n", pBlock->CRC);
			strcat(str, temp);
			if(pBlock->FileType < 3)
				wsprintf(temp, "File Type = 0x%X (%s)\r\n", pBlock->FileType, sFileDataType[pBlock->FileType]);
			else
				wsprintf(temp, "File Type = 0x%X \r\n", pBlock->FileType, sFileDataType[pBlock->FileType]);
			strcat(str, temp);

			wsprintf(temp, "FileNum1 = %u\r\n", pBlock->FileNum1);
			strcat(str, temp);
			wsprintf(temp, "FileNum2 = %u\r\n", pBlock->FileNum2);
			strcat(str, temp);

			wsprintf(temp, "FileName = \"%c%c%c%c%c%c%c%c\"\r\n\r\n", pBlock->FileName[0], pBlock->FileName[1],
				pBlock->FileName[2],pBlock->FileName[3],pBlock->FileName[4],pBlock->FileName[5],pBlock->FileName[6],pBlock->FileName[7]);
			strcat(str, temp);
		}
	}
	strcat(str, "\r\n\r\n");
}


void NESROMCheckResultToText(unsigned char *Result, char *str, char *fn){
	int i = 0;
	char temp[32];

	strcpy(str, fn);
	wsprintf(temp, "\r\nFilesize = %u Byte\r\n", *((DWORD *)&Result[0x14]));
	strcat(str, temp);
	wsprintf(temp, "CRC = 0x%08X\r\n", *((DWORD *)&Result[0x10]));
	strcat(str, temp);
	wsprintf(temp, "PROM CRC = 0x%08X\r\n", *((DWORD *)&Result[0x18]));
	strcat(str, temp);
	wsprintf(temp, "PROM size = %u Byte\r\n", Result[4]*16384);
	strcat(str, temp);
	wsprintf(temp, "CROM size = %u Byte\r\n", Result[5]*8192);
	strcat(str, temp);
	strcat(str, "\r\nHeader:\r\n");
	for(;i<16;i++){
		wsprintf(temp, "%02X ", Result[i]);
		strcat(str, temp);
		if(i==7){
			strcat(str, ": ");
		}
	}
	strcat(str, "\r\n\r\n");
}


void FDSROMInfoToText(char *str, int nItem){
	int i, n;
	char temp[64];

	n = g_Preview_State.DB.pListViewItem[nItem].HeaderInfo1;
	wsprintf(temp, "******* Database Information *******\r\n");
	strcpy(str, temp);
	wsprintf(temp, "File Block = %u\r\n", n);
	strcat(str, temp);
	wsprintf(temp, "CRC List\r\n", n);
	strcat(str, temp);
	for(i=0; i<n; i++){
		wsprintf(temp, "Block %u = 0x%08X\r\n", i, g_Preview_State.DB.pListViewItem[nItem].FDS_Crc[i]);
		strcat(str, temp);
	}
}


void NESROMInfoToText(char *str, int nItem){
	int i = 0;
	char temp[64];

	wsprintf(temp, "******* Database Information *******\r\n");
	strcpy(str, temp);
	wsprintf(temp, "CRC = 0x%08X\r\n", g_Preview_State.DB.pListViewItem[nItem].Crc);
	strcat(str, temp);
	wsprintf(temp, "PROM CRC = 0x%08X\r\n", g_Preview_State.DB.pListViewItem[nItem].PROM_Crc);
	strcat(str, temp);
	wsprintf(temp, "PROM size = %u Byte\r\n", g_Preview_State.DB.pListViewItem[nItem].PROM_Size*16384);
	strcat(str, temp);
	wsprintf(temp, "CROM size = %u Byte\r\n", g_Preview_State.DB.pListViewItem[nItem].CROM_Size*8192);
	strcat(str, temp);
	wsprintf(temp, "\r\nHeader Info1 : 0x%02X\r\n", g_Preview_State.DB.pListViewItem[nItem].HeaderInfo1);
	strcat(str, temp);
	wsprintf(temp, "Header Info2 : 0x%02X\r\n", g_Preview_State.DB.pListViewItem[nItem].HeaderInfo2);
	strcat(str, temp);
	wsprintf(temp, "Mapper : %u\r\n", (g_Preview_State.DB.pListViewItem[nItem].HeaderInfo1>>4)|(g_Preview_State.DB.pListViewItem[nItem].HeaderInfo2&0xf0));
	strcat(str, temp);
	wsprintf(temp, "[%u] : Four-screen mirroring\r\n", (g_Preview_State.DB.pListViewItem[nItem].HeaderInfo1&8)?1:0);
	strcat(str, temp);
	wsprintf(temp, "[%u] : 512-byte trainer present\r\n", (g_Preview_State.DB.pListViewItem[nItem].HeaderInfo1&4)?1:0);
	strcat(str, temp);
	wsprintf(temp, "[%u] : SRAM enabled\r\n", (g_Preview_State.DB.pListViewItem[nItem].HeaderInfo1&2)?1:0);
	strcat(str, temp);
	wsprintf(temp, "[%u] : mirroring(0==H,1==V)\r\n", (g_Preview_State.DB.pListViewItem[nItem].HeaderInfo1&1)?1:0);
	strcat(str, temp);
}


void PCEROMInfoToText(char *str, int nItem){
	int i = 0;
	char temp[64];

	wsprintf(temp, "******* Database Information *******\r\n");
	strcpy(str, temp);
	wsprintf(temp, "CRC = 0x%08X\r\n", g_Preview_State.DB.pListViewItem[nItem].Crc);
	strcat(str, temp);
}


void PreviewCheckROMStateDlgProc1(HWND hDlg, int nItem){
	unsigned char *buf;
	char *str;

	str = (char *)malloc(80*64*4*2);
	if(str==NULL)
		return;
	buf = (unsigned char *)malloc(sizeof(struct MyPreviewFDSDiskData)*4+sizeof(struct MyPreviewFDSBlockData)*64*4 + 0x100);
	if(buf==NULL){
		free(str);
		return;
	}
	str[0] = 0;
	int i, Type = g_Preview_State.DB.pListViewItem[nItem].Flag>>1;
	if(0==CheckOneROMFile(g_Preview_State.DB.pListViewItem[nItem].FileName, buf)){
		const char *ErrMsg = "ROM File Not Found.";
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC2), ErrMsg);
		if(!Type){
			NESROMInfoToText(str, nItem);
			SetWindowText(GetDlgItem(hDlg, IDC_EDIT1), str);
		}
		else if(Type==1){
			FDSROMInfoToText(str, nItem);
			SetWindowText(GetDlgItem(hDlg, IDC_EDIT1), str);
		}
		else if(Type==3){
			PCEROMInfoToText(str, nItem);
			SetWindowText(GetDlgItem(hDlg, IDC_EDIT1), str);
		}
		free(buf);
		free(str);
		return;
	}
	if(Type == 0){		// NES
		NESROMCheckResultToText(buf, str, g_Preview_State.DB.pListViewItem[nItem].FileName);
		for(i=0; str[i]; i++);
		NESROMInfoToText(&str[i], nItem);
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT1), str);
		if(g_Preview_State.DB.pListViewItem[nItem].PROM_Size != buf[4] || g_Preview_State.DB.pListViewItem[nItem].CROM_Size != buf[5]){
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC2), "Header size infomation invalid.");
		}
		else if(g_Preview_State.DB.pListViewItem[nItem].HeaderInfo1 != buf[6] || g_Preview_State.DB.pListViewItem[nItem].HeaderInfo2 != buf[7]){
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC2), "Header flag infomation invalid.");
		}
		else{
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC2), "ROM file complete.");
		}
	}
	else if(Type==1){		// FDS
		FDSROMCheckResultToText(buf, str, g_Preview_State.DB.pListViewItem[nItem].FileName);
		for(i=0; str[i]; i++);
		FDSROMInfoToText(&str[i], nItem);
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT1), str);
//		SetWindowText(GetDlgItem(hDlg, IDC_STATIC2), "ROM file complete.");
	}
	else if(Type==3){		// PCE
		PCEROMCheckResultToText(buf, str, g_Preview_State.DB.pListViewItem[nItem].FileName);
		for(i=0; str[i]; i++);
		PCEROMInfoToText(&str[i], nItem);
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT1), str);
	}
	free(str);
	free(buf);

//	SetWindowText(GetDlgItem(hDlg, IDC_STATIC2), g_Preview_State.DB.pListViewItem[nItem].Title);
}


//  IDD_PRVROMCHKDIALOG 
LRESULT CALLBACK PreviewCheckROMStateDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static int nItem;

	switch(msg){
		case WM_INITDIALOG:
			nItem = (int)lp;
			{
				char str[260];
				strcpy(str, g_Preview_State.DB.pListViewItem[nItem].Title);
				MystrFnIcut(str);
				SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC1), str);
			}
			PreviewCheckROMStateDlgProc1(hDlgWnd, nItem);
			return TRUE;
		case WM_CTLCOLORSTATIC:
			if((HWND)lp == GetDlgItem(hDlgWnd, IDC_EDIT1))
				return (LRESULT)GetStockObject(WHITE_BRUSH);
		case WM_COMMAND:
			switch (LOWORD(wp)) {
				case IDC_BUTTON1:		//check
					PreviewCheckROMStateDlgProc1(hDlgWnd, nItem);
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


int GetListItemROMInfoStruct(int nItem){
	int *pROM_Data;
	int n=0;

	int Root = g_Preview_State.nTreeSelect;
	int SubItem = g_Preview_State.nSubTreeSelect;
	if(SubItem == -1){
		ListView_SetItemCount(Mainwindow_Preview, 0);
		return -1;
	}
	switch(g_Preview_State.CheckResaultDisplay){
		case 0:		//全て表示
			pROM_Data = g_Preview_State.Category[Root].Sub[SubItem].pListViewAllItem;
			n = g_Preview_State.Category[Root].Sub[SubItem].nListViewItem;
			break;
		case 1:		//所持品のみ表示
			pROM_Data = g_Preview_State.Category[Root].Sub[SubItem].pListViewValidItem;
			n = g_Preview_State.Category[Root].Sub[SubItem].nListValidItem;
			break;
		case 2:		//非所持品のみ表示
			pROM_Data = g_Preview_State.Category[Root].Sub[SubItem].pListViewInValidItem;
			n = g_Preview_State.Category[Root].Sub[SubItem].nListInValidItem;
			break;
		default:
			return -1;
	}
	if(nItem >= n)
		return -1;
	return pROM_Data[nItem];
}



int User_ROMDatabaseToFile(char *FileName, struct NES_ROM_Data *ROM_Data){
	HANDLE hFile=NULL;
	DWORD dwAccBytes;
	char fn[MAX_PATH], str[1024];

	GetModuleFileName(NULL, fn, MAX_PATH);
	PathRemoveFileSpec(fn);
	PathCombine(fn, fn, FileName);
	hFile = CreateFile(fn, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return 0;
	SetFilePointer(hFile, 0, 0, FILE_END);
	if(!(ROM_Data->Flag & 2)){
		wsprintf(str, "%08x;%08x;%s;%u;%u;%u;%u;%s;%s;%s;%s;%s\r\n", ROM_Data->Crc, ROM_Data->PROM_Crc,
			ROM_Data->Title, ROM_Data->HeaderInfo1, ROM_Data->HeaderInfo2, ROM_Data->PROM_Size, ROM_Data->CROM_Size,
			ROM_Data->Country, ROM_Data->Maker, ROM_Data->Release, ROM_Data->Price, ROM_Data->Genre);
		WriteFile(hFile, str, strlen(str), &dwAccBytes, NULL);
		CloseHandle(hFile);
		return 1;
	}
	else{
		char tmp[260];
		int i, j = (int)ROM_Data->HeaderInfo1;
		wsprintf(str, "    ;0;0;0;0;%02x;", j);
		for(i=0; i<j; i++){
			wsprintf(tmp, "%08x;", ROM_Data->FDS_Crc[i]);
			strcat(str, tmp);
		}
		wsprintf(tmp, "%s\r\n", ROM_Data->Title);
		strcat(str, tmp);
		WriteFile(hFile, str, strlen(str), &dwAccBytes, NULL);
		CloseHandle(hFile);
		return 1;
	}
	if(hFile)
		CloseHandle(hFile);
	return 0;
}


int User_FixROMDatabase(HWND hWnd, int n){
	char FileName[MAX_PATH];
	struct NES_ROM_Data ROM_data;
	int ret;
	if(MyFileOpenDlg(hWnd, FileName)){
		ret = MyPreviewCheckROMFile(FileName, &ROM_data);
		if(ret == 1 && !(g_Preview_State.DB.pListViewItem[n].Flag & 2)){
			g_Preview_State.DB.pListViewItem[n].CROM_Size = ROM_data.CROM_Size;
			g_Preview_State.DB.pListViewItem[n].PROM_Size = ROM_data.PROM_Size;
			g_Preview_State.DB.pListViewItem[n].HeaderInfo1 = ROM_data.HeaderInfo1;
			g_Preview_State.DB.pListViewItem[n].HeaderInfo2 = ROM_data.HeaderInfo2;
			g_Preview_State.DB.pListViewItem[n].PROM_Crc = ROM_data.PROM_Crc;
			g_Preview_State.DB.pListViewItem[n].Crc = ROM_data.Crc;
			g_Preview_State.DB.pListViewItem[n].Flag |= 1;
			strcpy(g_Preview_State.DB.pListViewItem[n].FileName, FileName);
			User_ROMDatabaseToFile("nesdbfix.dat", &g_Preview_State.DB.pListViewItem[n]);
			g_Preview_State.DB.ChangeHaveItem = 1;
			return 1;
		}
		else if(ret == 2 && (g_Preview_State.DB.pListViewItem[n].Flag & 2)){
			int i, j = (int)ROM_data.HeaderInfo1;
			for(i=0; i<j; i++){
				g_Preview_State.DB.pListViewItem[n].FDS_Crc[i] = ROM_data.FDS_Crc[i];
			}
			g_Preview_State.DB.pListViewItem[n].HeaderInfo1 = ROM_data.HeaderInfo1;
			g_Preview_State.DB.pListViewItem[n].Flag |= 1;
			strcpy(g_Preview_State.DB.pListViewItem[n].FileName, FileName);
			User_ROMDatabaseToFile("fdsdbfix.dat", &g_Preview_State.DB.pListViewItem[n]);
			g_Preview_State.DB.ChangeHaveItem = 1;
			return 1;
		}
	}
	return 0;
}



extern void LoadROM(const char* rom_name);
extern void FreeROM();

//extern void freeze(boolean really_freeze = TRUE);
//extern void thaw();


//IDD_USERADD_DB_DIALOG
LRESULT CALLBACK UserAdd_DatabaseDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	static struct NES_ROM_Data *pROM_data=NULL;
	static int bup;


    switch(msg) {
		case WM_COMMAND:
            switch(LOWORD(wp)){
				case IDC_BUTTON1:
					EndDialog(hDlg, IDC_BUTTON1);
					return TRUE;
					break;
                case IDOK:
					GetDlgItemText(hDlg, IDC_EDIT1, (LPTSTR)pROM_data->Title, sizeof(pROM_data->Title));
					GetDlgItemText(hDlg, IDC_EDIT2, (LPTSTR)pROM_data->Maker, sizeof(pROM_data->Maker));
					GetDlgItemText(hDlg, IDC_EDIT3, (LPTSTR)pROM_data->Release, sizeof(pROM_data->Release));
					GetDlgItemText(hDlg, IDC_EDIT4, (LPTSTR)pROM_data->Country, sizeof(pROM_data->Country));
					GetDlgItemText(hDlg, IDC_EDIT5, (LPTSTR)pROM_data->Price, sizeof(pROM_data->Price));
					GetDlgItemText(hDlg, IDC_EDIT6, (LPTSTR)pROM_data->Genre, sizeof(pROM_data->Genre));
					EndDialog(hDlg, IDOK);
					return TRUE;
					break;
				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
					return TRUE;
			}
			break;
		case WM_TIMER:
			if(emu)
			{
				emu->do_frame();
			}
			return TRUE;
		case WM_DESTROY:
			if(g_Preview_State.ScreenDispFlag){
				KillTimer(hDlg, ID_PROGRESSTIMER);
				if(bup)
					NESTER_settings.nes.preferences.run_in_background=0;
			}
			FreeROM();
			return TRUE;
		case WM_INITDIALOG:
			bup = 0;
			pROM_data = (struct NES_ROM_Data *)lp;
			SetDlgItemText(hDlg, IDC_EDIT7, (LPTSTR)pROM_data->FileName);
			if(g_Preview_State.ScreenDispFlag){
				if(!NESTER_settings.nes.preferences.run_in_background){
					NESTER_settings.nes.preferences.run_in_background = 1;
					bup = 1;
				}
				LoadROM(pROM_data->FileName);
				SetTimer(hDlg, ID_PROGRESSTIMER, 10, NULL);
			}
            return TRUE;
    }
    return FALSE;
}





int User_AddFileToROMDatabase(HWND hWnd, char *fn){
	struct NES_ROM_Data ROM_data;
	int ret;
	HWND hDlgWnd = NULL;
	memset(&ROM_data, 0, sizeof(ROM_data));
	ret = MyPreviewCheckROMFile(fn, &ROM_data);
	strcpy(ROM_data.FileName, fn); 
	if(ret == 1){
		{
			int i,j = g_Preview_State.DB.nListViewItem;
			for(i=0;i<j;i++){
				if(!(g_Preview_State.DB.pListViewItem[i].Flag&2)){
					if(g_Preview_State.DB.pListViewItem[i].Crc == ROM_data.Crc)
						return 0;
				}
			}
		}
		ret = DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_USERADD_DB_DIALOG),hWnd, (DLGPROC)UserAdd_DatabaseDlg, (LPARAM)&ROM_data);
		if(ret == IDCANCEL)
			return -1;
		if(ret == IDC_BUTTON1)
			return 0;
		User_ROMDatabaseToFile("nesdbadd.dat", &ROM_data);
		Add_FileToPreviewItemDatabase(&ROM_data);
		return 1;
	}
	else if(ret == 2){
		{
			int i,j = g_Preview_State.DB.nListViewItem;
			for(i=0;i<j;i++){
				if(g_Preview_State.DB.pListViewItem[i].Flag&2){
					int k, nFile = (int)g_Preview_State.DB.pListViewItem[i].HeaderInfo1;
					for(k=0; k<nFile; k++){
						if(g_Preview_State.DB.pListViewItem[i].FDS_Crc[k] != ROM_data.FDS_Crc[k])
							break;
					}
					if(k==nFile)
						return 0;
				}
			}
		}
		ret = DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_USERADD_DB_DIALOG),hWnd, (DLGPROC)UserAdd_DatabaseDlg, (LPARAM)&ROM_data);
		if(ret == IDCANCEL)
			return -1;
		if(ret == IDC_BUTTON1)
			return 0;
		User_ROMDatabaseToFile("fdsdbadd.dat", &ROM_data);
		Add_FileToPreviewItemDatabase(&ROM_data);
		return 1;
	}
	return 0;
}


void User_AddFileinFolderToROMDatabaseSub1(char *fn){
	if(-1 == User_AddFileToROMDatabase(main_window_handle, fn))
		CancelButton = 1;
}


int User_AddFileinFolderSearch(char *subd)
{
	char	dirstr[MAX_PATH];
	int		i=0,j=0;
	HANDLE	hFind;
	WIN32_FIND_DATA		fd;

	strcpy(dirstr, subd);
	PathCombine(dirstr, dirstr, "*.*");
	hFind = FindFirstFile(dirstr, &fd);
	if(hFind == INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return 0;
	}
	do {
		if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			char filestr[MAX_PATH];
			PathCombine(filestr, subd, fd.cFileName);
			User_AddFileinFolderToROMDatabaseSub1(filestr);
		}
		CheckMessage();
	} while(FindNextFile(hFind, &fd) && !CancelButton);
	FindClose(hFind);
	return 1;
}


int User_AddFileinSubFolderSearch(char *subd)
{
	char	dirstr[MAX_PATH];
	int		i=0,j=0;
	HANDLE	hFind;
	WIN32_FIND_DATA		fd;

	strcpy(dirstr, subd);
	PathCombine(dirstr, dirstr, "*.*");
	hFind = FindFirstFile(dirstr, &fd);
	if(hFind == INVALID_HANDLE_VALUE) {
		FindClose(hFind);
		return 0;
	}

	do {
		if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			char filestr[MAX_PATH];
			PathCombine(filestr, subd, fd.cFileName);
			User_AddFileinFolderToROMDatabaseSub1(filestr);
		}
		else{
			if(fd.cFileName[0] != '.'){
				char sfstr[MAX_PATH];
				PathCombine(sfstr, subd, fd.cFileName);
				User_AddFileinSubFolderSearch(sfstr);
			}
		}
		CheckMessage();
	} while(FindNextFile(hFind, &fd) && !CancelButton);
	FindClose(hFind);
	return 1;
}


int User_AddFileinFolderToROMDatabase(HWND hWnd, char *dn, int sff){
	if(ExeCheck_NESROM_Flag)
		return 0;
	CancelButton = 0;
	if(sff){
		User_AddFileinSubFolderSearch(dn);
	}
	else{
		User_AddFileinFolderSearch(dn);
	}
	return 1;
}


int Add_FileToPreviewItemDatabase(struct NES_ROM_Data *ROM_data){
	int j, i = g_Preview_State.DB.nListViewItem;
	int *tmp, type;
	struct ROM_DB *ROM_db = &g_Preview_State.DB;

	j = i;
	i++;

	//  Database
	if(j==ROM_db->nListViewItemMem){
		struct NES_ROM_Data *nesi=NULL;
		nesi = (NES_ROM_Data *)realloc(ROM_db->pListViewItem, sizeof(NES_ROM_Data)*(100+ROM_db->nListViewItemMem));
		memset(&nesi[ROM_db->nListViewItemMem], 0, sizeof(NES_ROM_Data)*100);
		ROM_db->pListViewItem = nesi;
		ROM_db->nListViewItemMem+=100;
	}
	ROM_db->pListViewItem[j] = *ROM_data;
	ROM_db->nListViewItem++;

	//  All
	tmp = (int *)realloc(g_Preview_State.Category[0].Sub[0].pListViewAllItem, sizeof(int)*i);
	g_Preview_State.Category[0].Sub[0].pListViewAllItem = tmp;
	tmp = (int *)realloc(g_Preview_State.Category[0].Sub[0].pListViewValidItem, sizeof(int)*i);
	g_Preview_State.Category[0].Sub[0].pListViewValidItem = tmp;
	tmp = (int *)realloc(g_Preview_State.Category[0].Sub[0].pListViewInValidItem, sizeof(int)*i);
	g_Preview_State.Category[0].Sub[0].pListViewInValidItem = tmp;
	g_Preview_State.Category[0].Sub[0].nListViewItem = i;
	g_Preview_State.Category[0].Sub[0].pListViewAllItem[j] = j;

//	SetListViewValidItem(0, 0);
	SetListViewValidItemOne(0, 0, j);

	type = ROM_data->Flag >> 1;
	if(!type){
		//  NES
		int nPreItem = g_Preview_State.DB.nListViewNESItem;
		int nItem = nPreItem+1;
		tmp = (int *)realloc(g_Preview_State.Category[0].Sub[1].pListViewAllItem, sizeof(int)*nItem);
		g_Preview_State.Category[0].Sub[1].pListViewAllItem = tmp;
		tmp = (int *)realloc(g_Preview_State.Category[0].Sub[1].pListViewValidItem, sizeof(int)*nItem);
		g_Preview_State.Category[0].Sub[1].pListViewValidItem = tmp;
		tmp = (int *)realloc(g_Preview_State.Category[0].Sub[1].pListViewInValidItem, sizeof(int)*nItem);
		g_Preview_State.Category[0].Sub[1].pListViewInValidItem = tmp;
		g_Preview_State.Category[0].Sub[1].nListViewItem = nItem;
		g_Preview_State.Category[0].Sub[1].pListViewAllItem[nPreItem] = j;
		SetListViewValidItemOne(0, 1, nPreItem);
	}
	else if(type == 1){
		// FDS
		int nPreItem = g_Preview_State.DB.nListViewFDSItem;
		int nItem = nPreItem+1;
		tmp = (int *)realloc(g_Preview_State.Category[0].Sub[2].pListViewAllItem, sizeof(int)*nItem);
		g_Preview_State.Category[0].Sub[2].pListViewAllItem = tmp;
		tmp = (int *)realloc(g_Preview_State.Category[0].Sub[2].pListViewValidItem, sizeof(int)*nItem);
		g_Preview_State.Category[0].Sub[2].pListViewValidItem = tmp;
		tmp = (int *)realloc(g_Preview_State.Category[0].Sub[2].pListViewInValidItem, sizeof(int)*nItem);
		g_Preview_State.Category[0].Sub[2].pListViewInValidItem = tmp;
		g_Preview_State.Category[0].Sub[2].nListViewItem = nItem;
		g_Preview_State.Category[0].Sub[2].pListViewAllItem[nPreItem] = j;
		SetListViewValidItemOne(0, 2, nPreItem);
	}

	// Genre
	{
		int k, nGenre=0;
		for(nGenre=0; GenreStr[nGenre]; nGenre++);
		for(k=0; k<nGenre; k++){
			if(!strcmp(GenreStr[k], ROM_data->Genre))
				break;
		}
		if(k!=nGenre){
			int nPreItem = g_Preview_State.Category[1].Sub[k].nListViewItem;
			int nItem = nPreItem+1;
			{
				int *tp;
				tp = (int *)realloc(g_Preview_State.Category[1].Sub[k].pListViewAllItem, nItem* sizeof(int));
				g_Preview_State.Category[1].Sub[k].pListViewAllItem = tp;
				tp = (int *)realloc(g_Preview_State.Category[1].Sub[k].pListViewValidItem, nItem* sizeof(int));
				g_Preview_State.Category[1].Sub[k].pListViewValidItem = tp;
				tp = (int *)realloc(g_Preview_State.Category[1].Sub[k].pListViewInValidItem, nItem* sizeof(int));
				g_Preview_State.Category[1].Sub[k].pListViewInValidItem = tp;
			}
			g_Preview_State.Category[1].Sub[k].pListViewAllItem[nPreItem] = j;
			g_Preview_State.Category[1].Sub[k].nListViewItem++;
			SetListViewValidItemOne(1, k, nPreItem);
		}
	}
	if(g_Preview_State.DB.pListViewSortItem){
		g_Preview_State.DB.pListViewSortItem[j] = j;
		PreviewDataStructSort(g_Preview_State.DB.pListViewSortItem, g_Preview_State.DB.nListViewItem, 0, 1);
	}

	PreviewToolChangeDisplay();
	MyTreeView_SetItemNum();
	return 1;
}



int CopySubCategory(struct SubCategory *src, struct SubCategory *dest){

	dest->pListViewAllItem = (int *)malloc(sizeof(int *)* src->nListViewItem);
	dest->pListViewValidItem = (int *)malloc(sizeof(int *)* src->nListValidItem);
	dest->pListViewInValidItem = (int *)malloc(sizeof(int *)* src->nListInValidItem);
	dest->nListViewItem = src->nListViewItem;
	dest->nListValidItem= src->nListValidItem;
	dest->nListInValidItem= src->nListInValidItem;

	memcpy(dest->pListViewAllItem, src->pListViewAllItem, sizeof(int *)* src->nListViewItem);
	memcpy(dest->pListViewValidItem, src->pListViewValidItem, sizeof(int *)* src->nListValidItem);
	memcpy(dest->pListViewInValidItem, src->pListViewInValidItem, sizeof(int *)* src->nListInValidItem);
	return 1;
}


void FreeMemSubCategory(struct SubCategory *dest){

	if(dest->pListViewAllItem){
		free(dest->pListViewAllItem);
		dest->pListViewAllItem = NULL;
	}
	if(dest->pListViewValidItem){
		free(dest->pListViewValidItem);
		dest->pListViewValidItem = NULL;
	}
	if(dest->pListViewInValidItem){
		free(dest->pListViewInValidItem);
		dest->pListViewInValidItem = NULL;
	}
	memset(dest, 0, sizeof(struct SubCategory));
}


struct MyResourceDLLFileInfo{
	char FileName[MAX_PATH];
	char CountryCode[32];
};



int ChangeResourceDLL(char *FileName){
	HMODULE tResDll=NULL;
	HMENU hMenu;
	tResDll	= LoadLibrary(FileName);
	if(tResDll == NULL)
		return 0;
	FreeLibrary(g_main_instance);
	g_main_instance = tResDll;
	hMenu = LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENU1));
	SetMenu(main_window_handle, hMenu);
	return 1;
}



void ChangeResourceDLLTmp(char *FileName, HMODULE *tmp){
	HMENU hMenu;
	if(*tmp)
		FreeLibrary(*tmp);
	*tmp = LoadLibrary(FileName);
	if(*tmp == NULL)
		return;
	hMenu = LoadMenu(*tmp, MAKEINTRESOURCE(IDR_MENU1));
	SetMenu(main_window_handle, hMenu);
}


void MyResourceDLLFileSelectDlgProc1(HMODULE *tmp){
	if(*tmp){
		HMENU hMenu;
		hMenu = LoadMenu(g_main_instance, MAKEINTRESOURCE(IDR_MENU1));
		SetMenu(main_window_handle, hMenu);
		FreeLibrary(*tmp);
	}
}




LRESULT CALLBACK MyResourceDLLFileSelectDlg(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp){

	static struct MyResourceDLLFileInfo **Info = NULL;
	static int SelectItem = -1;
	static HMODULE tResDll = NULL;

	switch(msg){
	case WM_INITDIALOG:
		{
			tResDll = NULL;
			SelectItem = -1;
			Info = (MyResourceDLLFileInfo **)lp;
			HWND hList = GetDlgItem(hWnd, IDC_LIST1);
			LV_COLUMN lvcol;
			lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvcol.fmt = LVCFMT_LEFT;
			lvcol.cx = 70;
			lvcol.pszText = "Language";
			lvcol.iSubItem = 0;
			ListView_InsertColumn(hList, 0, &lvcol);
			lvcol.cx = 250;
			lvcol.pszText = "FileName";
			lvcol.iSubItem = 1;
			ListView_InsertColumn(hList, 1, &lvcol);
			for(int i=0; Info[i]; i++){
				LV_ITEM item;
				item.mask = LVIF_TEXT | LVIF_PARAM;
				item.pszText = Info[i]->CountryCode;
				item.iItem = i;
				item.iSubItem = 0;
				item.lParam = i;
				ListView_InsertItem(hList, &item);
				item.mask = LVIF_TEXT;
				item.pszText = Info[i]->FileName;
				item.iItem = i;
				item.iSubItem = 1;
				ListView_SetItem(hList, &item);
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wp)){
			case IDOK:
				MyResourceDLLFileSelectDlgProc1(&tResDll);
				if((SelectItem != -1) && ChangeResourceDLL(Info[SelectItem]->FileName)){
					strcpy(NESTER_settings.ResourceDll, Info[SelectItem]->FileName);
				}
				EndDialog(hWnd, IDOK);
				break;
			case IDCANCEL:
				MyResourceDLLFileSelectDlgProc1(&tResDll);
				EndDialog(hWnd, IDCANCEL);
				break;
		}
		return TRUE;
	case WM_NOTIFY:
		{
			NMHDR *nmhdr = (NMHDR *)lp;
			if((int)wp == IDC_LIST1){
				if(nmhdr->code == LVN_ITEMCHANGED){
					int i=-1;
					HWND hList = GetDlgItem(hWnd, IDC_LIST1);
					if((i=ListView_GetNextItem(hList, i, LVNI_ALL | LVNI_SELECTED)) != -1){
						if(SelectItem != i){
							ChangeResourceDLLTmp(Info[i]->FileName, &tResDll);
							SelectItem = i;
						}
					}
				}
				return TRUE;
			}
			break;
		}
	}
    return FALSE;
}




//language DLL select
int LaungageResourceSelect(HWND hWnd){
	char	str[MAX_PATH];
	struct	MyResourceDLLFileInfo **DllInfo = NULL;
	int		i=0,j=0,n=0;
	HANDLE	hFind;
	WIN32_FIND_DATA		fd;

	DllInfo = (MyResourceDLLFileInfo **)malloc(sizeof(char *)*0x31);
	GetModuleFileName(NULL, str, MAX_PATH);
	PathRemoveFileSpec(str);
	PathCombine(str , str, "lang_*.dll");
	hFind = FindFirstFile(str, &fd);
	if(hFind == INVALID_HANDLE_VALUE){
		FindClose(hFind);
		return 0;
	}
	do {
		if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && n<0x30){
			HMODULE tResDll=NULL;
    		char resstr[64];
			tResDll	= LoadLibrary(fd.cFileName);
			if(tResDll == NULL)
				continue;
    		LoadString(tResDll, IDS_STRING_RESCODE , resstr, 64);
			if(strcmp(resstr, "NNNESTERJ_RESOURCE_DLL")){
				FreeLibrary(tResDll);
				continue;
			}
    		LoadString(tResDll, IDS_STRING_LANGCODE , resstr, 64);
			FreeLibrary(tResDll);
			DllInfo[n] = (MyResourceDLLFileInfo *)malloc(sizeof(MyResourceDLLFileInfo));
			strcpy(DllInfo[n]->FileName, str);
			PathRemoveFileSpec(DllInfo[n]->FileName);
			PathCombine(DllInfo[n]->FileName , DllInfo[n]->FileName, fd.cFileName);
			strcpy(DllInfo[n]->CountryCode, resstr);
			n++;
			DllInfo[n] = NULL;
		}
	} while(FindNextFile(hFind, &fd));
	FindClose(hFind);
	if(!n)
		return 0;

	if(IDOK == DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_RESOURCEDLL_SELECT_DIALOG), hWnd, (DLGPROC)MyResourceDLLFileSelectDlg, (LPARAM)DllInfo)){
		char str2[64], str3[64];
		LoadString(g_main_instance, IDS_STRING_MSGB_11 , str2, 64);
		LoadString(g_main_instance, IDS_STRING_MSGB_15 , str3, 64);
		MessageBox(hWnd, str3, str2, MB_OK);
	}

	for(i=0; i<n; i++){
		free(DllInfo[i]);
	}
	free(DllInfo);
	return 1;
}

