
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

int MyFileOpenDlg(HWND hWnd, char *fn);
int MyFolderOpenDlg(HWND hWnds, char *folds);
void Add_PreviewListToFile(char *fn);
int Preview_Folderadd(HWND hWnd);
void Preview_sFolderadd(char *dirname);
void Add_PreviewListToFile(char *fn);
void OnGetDispInfo(NMLVDISPINFO* pInfo);
int OnOdfinditem(LPNMLVFINDITEM );
int PreviewSorts(const void *p1 , const void *p2);
int PreviewSortn(const void *p1 , const void *p2);
void PreviewListSort(int nItem);
//void LoadPreviewList(HWND lvhWnd);
//void SavePreviewList(HWND lvhWnd);
void Create_MainPreviewWind(HWND hwnd, HINSTANCE hInst);
void Destroy_MainPreviewWind();
void Return_PreviewMode();
void Goto_WindowGameMode();
HWND CreateToolWnd(HWND hWnd, HINSTANCE hInst);
void SetTreeViewSelectItem(HTREEITEM item);
void PreviewToolChangeDisplay();
void PreviewDisplayModeChange();
LRESULT CALLBACK PreviewROMDirectoryDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
void ListCheckResultClear();
void ExeCheck_NESROM();
void SetListViewValidItem(int , int );
int LoadListFile_NES();
int SaveListFile_NES();
int GetListItemROMInfoStruct(int nItem);
LRESULT CALLBACK PreviewCheckROMStateDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
int SetListViewItem_FDS(char *,  struct ROM_DB *);
void SetAllListViewValidItem();
void MyTreeView_SetItemNum();

int User_FixROMDatabase(HWND hWnd, int n);
int User_AddFileToROMDatabase(HWND hWnd, char *fn);
int User_AddFileinFolderToROMDatabase(HWND hWnd, char *dn, int sff);
int Add_FileToPreviewItemDatabase(struct NES_ROM_Data *ROM_data);
void ExeCheck_NESROM_Folder(int sff);

void FreeMemSubCategory(struct SubCategory *dest);
int CopySubCategory(struct SubCategory *src, struct SubCategory *dest);
void PreviewDataStructSort(int *dest, int Num, int nItem, int Sortd);
int LaungageResourceSelect(HWND hWnd);

void MyTreeView_DestroyCategory();
void SetListViewItem_NES();
void MyTreeView_CreateCategory();

void MyChangeAllToolTipColor();
int PreviewSearchItem(char *sstr, int start);
void PreviewRandamSelectItem();




