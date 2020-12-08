#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <stddef.h>
#include <shlobj.h>

#define WM_SOCKET (WM_USER+1)
#include "types.h"


BOOL CALLBACK NetplayServerDialogProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK NetplayClientDialogProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam);
uint8 GetNetplayStatus();
uint8 GetNetplayLatency();
void SocketClose();
uint8 SocketGetByte();
void SocketSendByte(uint8 data);
boolean SocketAccept();
boolean SocketConnect();
boolean SocketServer();
boolean SocketClient();




