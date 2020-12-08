#include <windows.h>

LRESULT CALLBACK nnnEditPaletDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
void defaultPaletteFileRead();
void defaultPaletteFileSave();
LRESULT CALLBACK nnnEditCromDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK nnnROMHeaderInfoDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK nnnMovieRecDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK nnnMoviePlayDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK MovieMessageDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK Preview_ConfigDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK UserAdd_DatabaseDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK MyMemViewColorDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);


void DrawMenuOwnerDraw(DRAWITEMSTRUCT *);
void SetMenuOwnerDraw(HWND );
struct MenuOD{
	unsigned int MenuID;
	unsigned int IconID;
	unsigned int cIconID;
	char MenuStr[64];
};
LRESULT CALLBACK GameGenieEditDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
void MyRegistShortCutKey(HWND);
void MyUnRegistShortCutKey(HWND);
LRESULT CALLBACK MyShortCutKeyDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK MyOnyoNiPuuDlg(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK PreviewSearchStringDlg(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);



