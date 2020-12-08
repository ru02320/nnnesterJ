
#ifndef _NNNCHEAT_H_
#define _NNNCHEAT_H_

#include <windows.h>
#define ID_EDIT			910
#define ID_LISTVIEW		911
#define ID_SBTIMER		912
#define ID_STATUS		913
#define ID_LUNCHASTIMER		914
#define ID_CKEYTIMER		915
#define ID_SCRMSGTIMER		916
#define ID_STATUSMW		917
#define ID_PREVIELV		918
#define ID_DCLKTIMER	919
#define ID_MYTOOLBAR	920
#define ID_TREEVIEW		921
#define ID_KEDIT1		922
#define ID_KEDIT2		923


#define PROG_NAME "NNNesteR"

#define WM_PREVIEW WM_USER+10
#define WM_PREVIEWS WM_USER+11
#define WM_PREVIEWSC WM_USER+12
#define WM_PREVIEWFS WM_USER+13

#define CHEATKEYWTT	4000

#define LUNCHFILEN	0		// okosama
#define LUNCHFOLDN	1
#define LUNCHMAPP	2
#define LUNCHPSIZE	3
#define LUNCHCSIZE	4
#define LUNCHRINFO	5
#define LUNCHALLCRC	6
#define LUNCHPCRC	7
#define LUNCHCCRC	8
#define LUNCHCRCR	9
#define LUNCHHEADS	10
#define LUNCHDBMAPP	11
#define LUNCHDBAROM	12
#define LUNCHDBNAME	13
#define LUNCHDBCOUN	14
#define LUNCHDBMAKE	15
#define LUNCHDBDATE	16
#define LUNCHDBPRIC	17
#define LUNCHDBJUN	18
//#define LUNCHDBRIDC	19
//#define LUNCHDBRSIZ	20

#define LUNCHCOLMAX	19




#if 0
struct ctrestoren {				//Cheat
	char ctrescomm[36];			//comment
	DWORD ctresadd;				//address
	DWORD ctresdata;			//value
	DWORD opcode;				//code(new)
	unsigned char ctrescflag;	//check
	unsigned char ctressflag;	//Type  0=Dec
	unsigned char ctresssize;	//Size
};
#else
struct ctrestoren {				//Cheat
	char ctrescomm[36];			//comment
	DWORD ctresadd;				//address
	DWORD ctresdata;			//value
	
	DWORD opcode;				//code(new)
	DWORD nSubcode;				//(new)
	struct ctrestoren *pSubcode;

	char ctrescflag;	//check
	char ctressflag;	//Type  0=Dec
	DWORD ctresssize;	//Size
};
#endif


#define freemem_macro(pmem){ \
	if(pmem){ \
		free(pmem); \
		pmem = NULL; \
	} \
}



LRESULT CALLBACK MyCheatDlg1(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg2(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg3(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg4(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg5(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg6(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg7(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg8(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg9(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg10(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg11(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg12(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg14(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyChild1Proc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyChild2Proc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyChild3Proc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyChild4Proc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg16(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK MyCheatDlg17(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK MyCheatDlg20(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK MyCheatDlg21(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK MyCheatDlg22(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);

HWND CreateMyChild(HWND, WNDPROC, LPCTSTR, LPCTSTR, int );
void InitCheatStruct();
void FreeCheatStruct();

//void SendNh(char *);
int CreateExtraWindow_01(HWND);
int CreateExtraWindow_02(HWND);
int CreateExtraWindow_03(HWND);
int CreateExtraWindow_04(HWND);

int pophexst(HWND , unsigned char *);
int MyStrAtoh(char *, DWORD *);
int MyHexCtoh(char *, DWORD *);
int CtCmpListO(HWND , int, DWORD, unsigned char *, unsigned char *, char *, char *, char *, char *);
void MyCtRestore();
//int MyCheat11s(HWND , unsigned char *, unsigned char *, int , int, DWORD *);
int MyCheat11s(HWND hWnd, int size, unsigned char *flagbuff, unsigned char *databuff, unsigned char *ndatabuff, int cmpsize, int type, DWORD pad);

void CheatfSave();
void CheatfLoad();
void CheatfSave2(HWND);
void CheatfLoad2(HWND);
void CheatfLoad3(HWND);
void CheatdfSave();
void CheatdfLoad();
int MystrJscmp(char *, char *, char);
int MystrCmp(char *,char *);
unsigned long CrcCalc(unsigned char *, int );
int RomCrcCheck(HWND);
void RomCrcCount(HWND, int *, int *);
int NesDbSrch(char *, char *, char *, char *);
void NesDbGetI(HWND, int, char *);
int MystrOr(char *, char *, int *);
int RomInfoSet(HWND, unsigned char *, int);
void RomCrcTout(HWND, HWND);
int ChengeRomH(HWND);
int ChengeRomHall(HWND);
int RomAndDbICmp(HWND );
int DbToRomIChange(HWND );
int FdsInfoSet(HWND , unsigned char *, int );
void FdsDbGetI(HWND , int , char *);
int FdsDbSrch(char *, char *, char *, char *);
int FdsCrcCheck(HWND );
int MystrCcutk(char *);
int RomFileNameC(HWND , int *, int *);
int MystrFnIcut(char *);
int LFileListSave(HWND );
int LFileListLoad(HWND );
void CheatCodeDisp(HWND , HWND );
int ChengeRomMin(HINSTANCE, HWND);
void CheatCodeACCopy(HWND , HWND );
void CheatCodeCCopy(HWND, HWND );
int	MystrJncmp(char *,char *, char);
int CheatCfSave(HWND);
void LunchAutoSSave();
void LunchAutoSLoad();
void ckreadmm(char *);
void NNNcfgsave();
void NNNcfgload();


void CheatWriteStringsMem(int size, DWORD adr, unsigned char *pdata);
void CheatWriteMem(int size, DWORD adr, DWORD data);
DWORD CheatReadMem(int size, DWORD adr);
void CheatReadStringsMem(int size, DWORD adr, unsigned char *pdata);


LRESULT CALLBACK MyLagTestDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);

#endif
