
#ifndef MY_EXT_WINDOW_H
#define MY_EXT_WINDOW_H

#include <windows.h>

#define WM_EXTRAWIN_01 (WM_USER+10)
#define WM_EXTRAWIN_02 (WM_USER+11)
#define WM_EXTRAWIN_03 (WM_USER+12)



// �A�C�e�����
struct MyCheatStructWriteItemInfo {
	int EDIT_X;
	int EDIT_Y;
	int TEXT_X;
	int TEXT_Y;

	int Width;

	int EditType;	// 0 = ������, 1 = ���lHex, 2 = ���lDec
	int InitValue;	// �����l���ݒ肳��Ă��邩
	int LoadValue;	// ���ݒl��ǂݍ��ނ�
	int DisableInput;	// ���͖���
//	int nPage;		// �y�[�W�ԍ�

	unsigned char Padding;	// ������̋󂫂𖄂߂鐔�l
	int Size;
	DWORD Address;
	DWORD Data;
	char Text[64];
	char Str[260];

	HWND hEdit;
	FARPROC OldWind_Proc;
};


// �e�[�u���p(�ꕶ��)
struct MyCheatStructWriteTableInfo_One {
	DWORD Out;
	int Out_Size;

//	int In;
	int In_s;
	int In_e;
	int In_Type;
};


// �e�[�u���p�i1�e�[�u��256���j
struct MyCheatStructWriteTableInfo {
	struct MyCheatStructWriteTableInfo_One Info[256];
	int nIn;
//	int nOut;
};


// �y�[�W�p
struct MyCheatStructWritePageInfo {
	HWND hWriteButton;	// �y�[�W���̂ݏ������݃{�^��

	char Title[64];
	char ButtonName[64];
	int  nItem;			// �y�[�W���̃A�C�e����
	struct MyCheatStructWriteItemInfo Item[64];
};


// 
struct MyCheatStructWriteInfo {
	int PosUserArrange;
	int nItem;
	int nCurPage;	// ���݂̃y�[�W

	char Title[260];
	char ButtonName[64];

	int WindowH;
	int WindowW;
	int nPage;		// �y�[�W�� -1
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