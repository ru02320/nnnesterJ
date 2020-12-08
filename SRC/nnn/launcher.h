void SetLauncherListName();
void LauncherROMInfoCopyC(HWND);
void LauncherROMFileCopy(HWND);
void LauncherFDSSaveDateClear(HWND);

HWND CreateMyPwind(HWND, WNDPROC, LPCTSTR, LPCTSTR);
int MCPreview(HWND);
LRESULT CALLBACK MyPrevProc(HWND, UINT, WPARAM, LPARAM);
void PrevScanFold(HWND,HWND);
int PrevScanSubFold(HWND , char *);
void PreviewFSave(HWND);
int PreviewFLoad();
int MyFoldOpenDlg(HWND , char *);
void LunchDInitMode();
void LunchDDispMode();
void LunchDHideMode();

void LauncherListNoRemove(HWND );
void LauncherListNoRomMove(HWND );
void LauncherListGoodRomMove(HWND );
void LauncherListFileCheck(HWND );
int Romchrexp(HWND hWnd);
int WINAPI kGameCallback(char *game, int player, int maxplayers);
void WINAPI kchatReceived(char *nick, char *text);
void WINAPI kclientDropped(char *nick, int playernb);
void WINAPI kmoreInfos(char *gamename);
void LauncherROMInfoCopyC(HWND);
void LunchWidthBuckup();
void LunchWidthRestore();
void FdsCrcTout(HWND , HWND );
int ChengeRomoInfo(HINSTANCE, HWND);
void MyListVclr(int , int );
void LauncherWindowTset(HWND );
void NNNlunchcfgload();
void NNNlunchcfgsave();


LRESULT CALLBACK LauncherFDSSaveClearDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK MyCheatDlg13(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg15(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg23(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg24(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MyCheatDlg19(HWND, UINT, WPARAM, LPARAM);
//int CALLBACK MyListSort(LPARAM, LPARAM, LPARAM);
int AddPFoldchk(HWND , char *);
int RomInfoScan(HWND, HWND);
void LauncherListSort(int nItem);

struct LauncherText {
	char FileName[MAX_PATH];
	char FolderName[MAX_PATH];
	char Mapper[8];
	char PromSize[8];
	char CromSize[12];		//8
	char Info[8];
	char AllCRC[260];
	char PROMCRC[32];		//12
	char CROMCRC[12];
	char Resault[8];
	char Header[4];
	char DBInfo[64];
	char RomDbCmp[4];
	char DBTitle[128];
	char DBCountry[8];
	char DBMaker[64];
	char DBRelease[24];
	char DBPrice[8];
	char DBGenre[8];
};

#if 0
struct Launcher_ROMInfo {
	char FileName[MAX_PATH];
	char FolderName[MAX_PATH];

	DWORD All_CRC;
	DWORD PROM_CRC;
	DWORD CROM_CRC;

	int Mapper;
	int PROM_Size;
	int CROM_Size;

	DWORD Flag;

	char Header[16];
	char DBTitle[128];

//	char Resault[8];
//	char Header[4];
	char DBHeader[16];

	char DBCountry[8];
	char DBMaker[64];
	char DBRelease[24];
	char DBPrice[8];
	char DBGenre[8];
};
#endif

