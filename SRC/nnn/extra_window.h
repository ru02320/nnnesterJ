
#ifndef MY_EXT_WINDOW_H
#define MY_EXT_WINDOW_H

#include <windows.h>

#define WM_EXTRAWIN_01 (WM_USER+10)
#define WM_EXTRAWIN_02 (WM_USER+11)
#define WM_EXTRAWIN_03 (WM_USER+12)



// アイテム情報
struct MyCheatStructWriteItemInfo {
	int EDIT_X;
	int EDIT_Y;
	int TEXT_X;
	int TEXT_Y;

	int Width;

	int EditType;	// 0 = 文字列, 1 = 数値Hex, 2 = 数値Dec
	int InitValue;	// 初期値が設定されているか
	int LoadValue;	// 現在値を読み込むか
	int DisableInput;	// 入力無効
//	int nPage;		// ページ番号

	unsigned char Padding;	// 文字列の空きを埋める数値
	int Size;
	DWORD Address;
	DWORD Data;
	char Text[64];
	char Str[260];

	HWND hEdit;
	FARPROC OldWind_Proc;
};


// テーブル用(一文字)
struct MyCheatStructWriteTableInfo_One {
	DWORD Out;
	int Out_Size;

//	int In;
	int In_s;
	int In_e;
	int In_Type;
};


// テーブル用（1テーブル256っ個）
struct MyCheatStructWriteTableInfo {
	struct MyCheatStructWriteTableInfo_One Info[256];
	int nIn;
//	int nOut;
};


// ページ用
struct MyCheatStructWritePageInfo {
	HWND hWriteButton;	// ページ内のみ書き込みボタン

	char Title[64];
	char ButtonName[64];
	int  nItem;			// ページ内のアイテム数
	struct MyCheatStructWriteItemInfo Item[64];
};


// 
struct MyCheatStructWriteInfo {
	int PosUserArrange;
	int nItem;
	int nCurPage;	// 現在のページ

	char Title[260];
	char ButtonName[64];

	int WindowH;
	int WindowW;
	int nPage;		// ページ数 -1
	HWND hWriteButton;
	RECT WriteButtonRect;

	HWND hTabWin;

	struct MyCheatStructWritePageInfo Page[16];
	struct MyCheatStructWriteTableInfo Table[10];
	int nTable;
};

void RunFrame_ExtraWindow();
void LoadROM_ExtraWindow();
void FreeROM_ExtraWindow();
int CheckModelessDialogMessage(MSG *lpmsg);

HWND CreateMyPaternWindow(HWND hWnd);
LRESULT CALLBACK MyNESDebugDialog(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
void Extrawin_FullscreentoWindow();
void Extrawin_ExitApp();
void Extrawin_WindowtoFullscreen();
void Extrawin_HideWin();
void Extrawin_ShowWin();


int MyCheatStWriteFileOpenDlg(HWND hWnd, char *fn);
HWND CreateMyCheatStWriteWindow(HWND hWnd);
int LoadCheatStructWriteFile(char *fn, struct MyCheatStructWriteInfo *Param);
HWND CreateMyCheatStWriteEditWindow(HWND hWnd);
int MyCheatStWriteFileOpenDlg(HWND hWnd, char *fn);

#endif