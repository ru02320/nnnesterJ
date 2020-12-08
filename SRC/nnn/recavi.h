
#include <windows.h>
#include <windowsx.h> 
#include <commctrl.h>
#include <commdlg.h>
#include <winuser.h>

#include <vfw.h>

int recavifilestart(char *fn, int w, int h);
void recavifilerelease();
void recavifilewriteflame(unsigned char *);
