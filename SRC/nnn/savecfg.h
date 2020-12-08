
#ifndef _SAVECFG_H_
#define _SAVECFG_H_



#include <commctrl.h>
#include "settings.h"
#include "cheat.h"


void SaveNesMainSettings(NES_preferences_settings *nesp);
void SaveSoundSettings(NES_sound_settings *nss);
void SaveGraphics(NES_graphics_settings *ngs);
void SaveAllControllerSettings(NES_input_settings *nesis);
//void SaveGraphicsOnMenu(NES_settings *ngs);

void SetUseRegistry(int use);
int CheckUseRegistry();


void SaveHoyKeyCfg();
void LoadHoyKeyCfg();


struct NES_ROM_Data {
	char Title[128];		//Title
	char Maker[64];			//Maker
	char Release[24];		//Release
	char Price[16];			//Price
	char Genre[16];			//Genre
	char Country[8];
	DWORD FDS_Crc[64];
	DWORD Crc;
	DWORD PROM_Crc;

	int  PROM_Size;
	int  CROM_Size;
	unsigned char HeaderInfo1;
	unsigned char HeaderInfo2;

	DWORD	Flag;				// program
								// 1 Have , 2 FDS/*, 4 User Added data, */
	char	FileName[MAX_PATH];
	DWORD	nPlay;
};


#define MAX_ROMDIRECTORY 64
#define COLUMN_NUM 8


struct SubCategory {
	HTREEITEM TreeViewItemID;

	int *pListViewAllItem;				//�S�ẴA�C�e���ւ̃|�C���^
	int *pListViewValidItem;			//�L���ȃA�C�e���ւ̃|�C���^
	int *pListViewInValidItem;			//�����ȃA�C�e���ւ̃|�C���^

	int nListViewItem;				//���A�C�e����
	int nListValidItem;				//�L���ȃA�C�e�����i�����Ă�ROM���j
	int nListInValidItem;			//�����ȃA�C�e�����i�����Ė���ROM���j
};


struct RootCategory {
	HTREEITEM TreeViewItemID;
	int		nSubCategory;
	struct	SubCategory *Sub;
};


struct ROM_DB {
	int nListViewItem;					//���A�C�e����
	int nListViewItemMem;				//�������m�ۂ��ꂽ��
	int nListViewNESItem;				//NES�A�C�e����
	int nListViewFDSItem;				//FDS�A�C�e����
	int nListViewGBItem;				//GB�A�C�e����
	int nListViewPCEItem;				//PCE�A�C�e����

	int nListViewNES_Stpos;				//NES�J�n�ʒu
	int nListViewFDS_Stpos;				//FDS
	int nListViewGB_Stpos;				//GB
	int nListViewPCE_Stpos;				//PCE

	int ChangeHaveItem;					// �������X�g�ɕω� = 1

	struct NES_ROM_Data *pListViewItem;	//�f�[�^�\���̂̃|�C���^
	int *pListViewSortItem;				//�\�[�g�ς݃|�C���^
};


struct Preview_state {
	int		NotLoadNESJ_DB;
	int		NotLoadNESE_DB;
	int		NotLoadFDS_DB;
	int		NotApplyNES_FixDB;
	int		NotApplyFDS_FixDB;
	int		NotLoadNES_AddDB;
	int		NotLoadFDS_AddDB;

	int		NESROM_CRCFrom_Filesize;

	int		DisableSound;
	int		EmuSpeed;
	int		DisableSoundBackup;
	int		EmuSpeedBackup;

	int		window_w;
	int		window_h;
	int		window_xpos;
	int		window_ypos;
	int		Game_window_xpos;
	int		Game_window_ypos;

	DWORD	Column_w[COLUMN_NUM];
	DWORD	Column_Pos[COLUMN_NUM];
	char	PrevSortItemp[COLUMN_NUM];

	int		nROMDirectory;
	char	ROM_Directory[MAX_ROMDIRECTORY][MAX_PATH];
	char	ROM_SubDirectory[MAX_ROMDIRECTORY];
	char	DATABASE_FileName[3][MAX_PATH];

	int		nToolTipHeight;
	HWND	hTool;
	HWND	hToolTip;
	HWND	hListView;
	HWND	hTreeView;
	HIMAGELIST hImageList;		//
	HIMAGELIST hImageTree;		//

	HFONT	hFont;
	DWORD	FontColor;
	DWORD	BGColor;
	LOGFONT LogFont;

	DWORD	ToolTipTXTColor;
	DWORD	ToolTipBGColor;

	HBITMAP hBitmap;

//preview List Font

	struct	RootCategory *Category;
	int		nCategory;						// �J�e�S���[��

//	HTREEITEM TreeViewItemID[PREVIEW_CATEGORYMAX];

	int nList_Width;
	int nTree_Width;
	int ListDispFlag;
	int TreeDispFlag;
	int ScreenDispFlag;

	RECT	PreviewScreenRect;
	RECT	PreviewScreenDrawRect;

	// Data

	struct ROM_DB DB;
//	struct ROM_DB MyDB;

//	void *pListViewItemExtra;			//�g���f�[�^�\���̂̃|�C���^


	char SearchString[260];


	int nTreeSelect;					// 
	int nSubTreeSelect;					// 
	int CheckResaultDisplay;			// 0 = �S�ĕ\���A1 = �L���ȕ��̂ݕ\���A2 = �����ȕ��̂ݕ\��
	int GameTitleDetailDisplay;			// �ڍו\��

};


struct extra_window_struct {
	// Dialog
	HWND CheatView[4];
	HWND CheatCompare;
	HWND CheatSearch;
	HWND CheatListEdit;

	HWND hOnyoniDlg;

	// Window
	HWND DebugWindow;
	HWND PatternWindow;
	HWND KailleraWindow;		// Kaillera Dammy window for Preview

	HWND CheatStWriteWindow;
	HWND CheatStWriteEditWindow;

};


#define CTRESTMAX 512

struct cheat_info_struct {

	struct ctrestoren ctrestds[CTRESTMAX];
	int DisableCheat;

	// cheat dialog radio button state
	int cc_clistedit1;
	int cc_clistedit2;
	int cc_clistedit3;		// opcode
	int cc_clistedit4;		// subcode num

	int cc_listorder[7];
	int cc_listwidth[7];


	int ctcmpdtf1;						//Compare Checkbox
	int ctcmpdtf2;						//Size
	int ctcmpdtf3;						//In Type 0=Dec
	int ctcmpdtf4;						//Out Type 0=Dec
	int ctcmpdtf5;						//left
	int ctcmpdtf6;						//Use Compare 0=use

	int cc_csrchrb1;					// ==...
	int cc_csrchrb2;					// 0=Dec
	int cc_csrchrb3;					// size


	int ctrestoreaf;					//Cheat Apply Flag
	int ctrestnum;						//

	int ctcmp2datarf;				//Compare radio botton
//	int ctcmpflag2;
//	int ctcmpflag2_BUP;						//backup

	// search
	unsigned char *pMainMemData;
	unsigned char *pSRAMData;
	unsigned char *pExtraMemData;
	unsigned char *pMainMemFlag;
	unsigned char *pSRAMFlag;
	unsigned char *pExtraMemFlag;

	int *pSearchOData;	// for ownar data   sizeof(int)* (mainmem_size+sram_size+extramem_size)
	int nSearchOData;	// for ownar data   item num

	// backup for search undo
	unsigned char *pBUP_MainMem;
	unsigned char *pBUP_SRAM;
	unsigned char *pBUP_ExtraMem;


	// cmp
	unsigned char *pMainCmp;
	unsigned char *pSRAMCmp;
	unsigned char *pExtraCmp;

	int *pCmpOData;	// for ownar data   sizeof(int)* (mainmem_size+sram_size+extramem_size)
	int nCmpOData;		// for ownar data   item num

	DWORD CheatTempMemory[32];

	int mainmem_size;
	int sram_size;
	int extramem_size;

};


/*
  *** Disk ***
	 0x00		4	Block num
	 0x04		1	���[�J�[���ʔԍ�
	 0x05		1	�Q�[�����ʂh�c
	 0x06		1	�Q�[���o�[�W�����ԍ�
	 0x07		1	�f�B�X�N�̖ʔԍ�(0:side-A 1:side-B)
	 0x08		1	�f�B�X�N�t���ԍ�1	Disk Number	0x26
	 0x09		1	�f�B�X�N�t���ԍ�2		0x27
	 0x0A		1	�f�B�X�N�t���ԍ�3		0x28
	 0x0B		3	���[�J�[�F��(?)BCD�����ŋL�^�B�N���͏��a(+1925)  0x2F
	 0x0E		3	Created Date BCD�����ŋL�^�B�N���͏��a(+1925)	 0x3C
	 0x11
*/
struct MyPreviewFDSDiskData {
	int		nBlock;
	int		nRealBlock;
	unsigned char MakerID;
	unsigned char GameID[4];
	unsigned char GameVer;
	unsigned char SideNum;
	unsigned char AddNum1;
	unsigned char AddNum2;
	unsigned char AddNum3;
	unsigned char MakerDate[3];
	unsigned char CreateDate[3];
};

/*
(size 1byte)  �t�@�C���ԍ�(DiskBIOS�͂��̃f�[�^���g�p���Ȃ�)
(size 1byte)  �t�@�C���ԍ�2(DiskBIOS�͂��̃f�[�^���g�p���Ȃ�)
(size 8byte)  �t�@�C����(DiskBIOS�͂��̃f�[�^���g�p���Ȃ�)
(size 2byte)  �t�@�C���i�[�ꏊ
(size 2byte)  �t�@�C���T�C�Y    0x57
(size 1byte)  �t�@�C���̎�� (0:�f�[�^ 1:�L�����N�^�f�[�^ 2:�l�[���e�[�u���f�[�^)
                        (DiskBIOS��1,2�̋�ʂ����Ȃ�)
*/
struct MyPreviewFDSBlockData {
	unsigned char FileNum1;
	unsigned char FileNum2;
	unsigned char FileName[8];
	unsigned char FileType;
	int		FileSize;
	int		CRC;

};




struct MyWindowShortCutKey {
	int KeyConfig;
	int Active;
	int Regist;
};




#endif