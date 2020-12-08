
#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <stddef.h>
#include <shlobj.h>

#include "resource.h"
#include "settings.h"
#include "netplay.h"
#include "types.h"


extern HWND main_window_handle;
extern HINSTANCE g_main_instance;



///////////////////////////////////////////////////////////
// network
SOCKET sock = INVALID_SOCKET; // client socket
SOCKET sv_sock = INVALID_SOCKET; // server socket
uint8 netplay_status = 0;
uint8 netplay_latency = 0;
uint8 netplay_disconnect = 0;
unsigned long ip_address;
unsigned long ip_port;



BOOL CALLBACK NetplayServerDialogProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static boolean bNetLatency;
	static boolean bIPPort;

	switch(msg)
	{
	case WM_CLOSE:
		{
			EndDialog(hDlg, IDCANCEL);
		}
		break;

	case WM_INITDIALOG:
		{
			Edit_SetText(GetDlgItem(hDlg, IDC_NETLATENCY), "0");
			Edit_SetText(GetDlgItem(hDlg, IDC_IPPORT), "10000");
		}
		break;

	case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
			case IDC_NETLATENCY:
				{
					int result = SendMessage((HWND)lparam, WM_GETTEXTLENGTH, 0, 0);
					bNetLatency = (result == 1) ? TRUE : FALSE;
					EnableWindow(GetDlgItem(hDlg, IDOK), bNetLatency & bIPPort );
				}
				break;

			case IDC_IPPORT:
				{
					int result = SendMessage((HWND)lparam, WM_GETTEXTLENGTH, 0, 0);
					bIPPort = (result == 4 || result == 5) ? TRUE : FALSE;
					EnableWindow(GetDlgItem(hDlg, IDOK), bNetLatency & bIPPort );
				}
				break;

			case IDOK:
				{
					char sz[64];
					GetDlgItemText(hDlg, IDC_NETLATENCY, sz, 64);
					netplay_latency = atoi(sz);
					GetDlgItemText(hDlg, IDC_IPPORT, sz, 64);
					ip_port = atol(sz);
					EndDialog(hDlg, IDOK);
				}
				break;

			case IDCANCEL:
				{
					EndDialog(hDlg, IDCANCEL);
				}
				break;

			default:
				return FALSE;
			}
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK NetplayClientDialogProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static boolean bIPAddress;
	static boolean bIPPort;

	switch(msg)
	{
	case WM_CLOSE:
		{
			EndDialog(hDlg, IDCANCEL);
		}
		break;

	case WM_INITDIALOG:
		{
			Edit_SetText(GetDlgItem(hDlg, IDC_IPADDRESS), "192.168.0.1");
			Edit_SetText(GetDlgItem(hDlg, IDC_IPPORT), "10000");
		}
		break;

	case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
			case IDC_IPADDRESS:
				{
					int result = SendMessage((HWND)lparam, WM_GETTEXTLENGTH, 0, 0);
					bIPAddress = (result != 0) ? TRUE : FALSE;
					EnableWindow(GetDlgItem(hDlg, IDOK), bIPAddress & bIPPort );
				}
				break;

			case IDC_IPPORT:
				{
					int result = SendMessage((HWND)lparam, WM_GETTEXTLENGTH, 0, 0);
					bIPPort = (result == 4 || result == 5) ? TRUE : FALSE;
					EnableWindow(GetDlgItem(hDlg, IDOK), bIPAddress & bIPPort );
				}
				break;

			case IDOK:
				{
					char sz[64];
					GetDlgItemText(hDlg, IDC_IPADDRESS, sz, 64);
					ip_address = inet_addr(sz);
					GetDlgItemText(hDlg, IDC_IPPORT, sz, 64);
					ip_port = atol(sz);
					EndDialog(hDlg, IDOK);
				}
				break;

			case IDCANCEL:
				{
					EndDialog(hDlg, IDCANCEL);
				}
				break;

			default:
				return FALSE;
			}
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

uint8 GetNetplayStatus()
{
	return netplay_status;
}

uint8 GetNetplayLatency()
{
	return netplay_latency;
}

void SocketClose()
{
	if(sock != INVALID_SOCKET)
	{
		shutdown(sock, 2);
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
	if(sv_sock != INVALID_SOCKET)
	{
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
	}
	netplay_disconnect = 0;
	netplay_status = 0;
	EnableMenuItem(GetSystemMenu(main_window_handle, FALSE), SC_CLOSE, MF_ENABLED);
}

uint8 SocketGetByte()
{
	if(netplay_disconnect)
	{
		return 0;
	}
	if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_READ | FD_CLOSE) == SOCKET_ERROR)
	{
		netplay_disconnect = 1;
		netplay_status = 0;
		return 0;
	}
	while(1)
	{
		MSG msg;
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			switch(msg.message)
			{
			case WM_SOCKET:
				{
					if(WSAGETSELECTERROR(msg.lParam) != 0)
					{
						netplay_disconnect = 1;
						netplay_status = 0;
						return 0;
					}
					else if(WSAGETSELECTEVENT(msg.lParam) == FD_READ)
					{
						char buf[1];
						int status = recv(sock, buf, 1, 0);
						if(status == SOCKET_ERROR)
						{
							netplay_disconnect = 1;
							netplay_status = 0;
							return 0;
						}
						else if(status > 0)
						{
							if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_CLOSE) == SOCKET_ERROR)
							{
								netplay_disconnect = 1;
								netplay_status = 0;
								return 0;
							}
							else
							{
								return buf[0];
							}
						}
					}
					else if(WSAGETSELECTEVENT(msg.lParam) == FD_CLOSE)
					{
						netplay_disconnect = 1;
						netplay_status = 0;
						return 0;
					}
				}
				break;

			case WM_COMMAND:
				{
					if(LOWORD(msg.wParam) == ID_FILE_NETPLAY_CLOSE)
					{
						netplay_disconnect = 1;
						netplay_status = 0;
						return 0;
					}
				}
				break;

			default:
				{
					//DefWindowProc(main_window_handle, msg.message, msg.wParam, msg.lParam);
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				break;
			}
		}
	}
	return 0;
}

void SocketSendByte(uint8 data)
{
	if(netplay_disconnect)
	{
		return;
	}
	if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
	{
		netplay_disconnect = 1;
		netplay_status = 0;
		return;
	}
	while(1)
	{
		MSG msg;
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			switch(msg.message)
			{
			case WM_SOCKET:
				{
					if(WSAGETSELECTERROR(msg.lParam) != 0)
					{
						netplay_disconnect = 1;
						netplay_status = 0;
						return;
					}
					else if(WSAGETSELECTEVENT(msg.lParam) == FD_WRITE)
					{
						char buf[1];
						buf[0] = data;
						int status = send(sock, buf, 1, 0);
						if(status == SOCKET_ERROR)
						{
							netplay_disconnect = 1;
							netplay_status = 0;
							return;
						}
						else if(status > 0)
						{
							if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_CLOSE) == SOCKET_ERROR)
							{
								netplay_disconnect = 1;
								netplay_status = 0;
								return;
							}
							else
							{
								return;
							}
						}
					}
					else if(WSAGETSELECTEVENT(msg.lParam) == FD_CLOSE)
					{
						netplay_disconnect = 1;
						netplay_status = 0;
						return;
					}
				}
				break;

			case WM_COMMAND:
				{
					if(LOWORD(msg.wParam) == ID_FILE_NETPLAY_CLOSE)
					{
						netplay_disconnect = 1;
						netplay_status = 0;
						return;
					}
				}
				break;

			default:
				{
					//DefWindowProc(main_window_handle, msg.message, msg.wParam, msg.lParam);
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				break;
			}
		}
	}
	return;
}

boolean SocketAccept()
{
	SOCKADDR_IN sv_sock_addr;
	MSG msg;

	int result = DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_NETPLAY_SERVER),
								main_window_handle, NetplayServerDialogProc, NULL);

	if(result != IDOK)
	{
		return FALSE;
	}
	if(ip_port < 1024 || ip_port == 8080 || 65535 < ip_port)
	{
		MessageBox(main_window_handle, "Invalid Port number", "NETWORK ERROR", MB_OK);
		return FALSE;
	}

	if((sv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		MessageBox(main_window_handle, "socket() failed", "NETWORK ERROR", MB_OK);
		return FALSE;
	}

	memset(&sv_sock_addr, 0x00, sizeof(sv_sock_addr));
	sv_sock_addr.sin_family = AF_INET;
	sv_sock_addr.sin_port = (unsigned short)ip_port;
	sv_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sv_sock, (LPSOCKADDR)&sv_sock_addr, sizeof(sv_sock_addr)) == SOCKET_ERROR)
	{
		MessageBox(main_window_handle, "bind() failed", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	if(listen(sv_sock, 0) == SOCKET_ERROR)
	{
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
		MessageBox(main_window_handle, "listen() failed", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	if(WSAAsyncSelect(sv_sock, main_window_handle, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR)
	{
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
		MessageBox(main_window_handle, "WSAAsyncSelect() failed", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	while(1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			switch(msg.message)
			{
			case WM_SOCKET:
				{
					if(WSAGETSELECTERROR(msg.lParam) != 0)
					{
						shutdown(sv_sock, 2);
						closesocket(sv_sock);
						sv_sock = INVALID_SOCKET;
						MessageBox(main_window_handle, "socket error", "NETWORK ERROR", MB_OK);
						return FALSE;
					}
					else if(WSAGETSELECTEVENT(msg.lParam) == FD_ACCEPT)
					{
						SOCKADDR_IN sock_addr;
						int len = sizeof(sock_addr);
						if((sock = accept(sv_sock, (LPSOCKADDR)&sock_addr, &len)) == INVALID_SOCKET)
						{
							closesocket(sv_sock);
							sv_sock=INVALID_SOCKET;
							MessageBox(main_window_handle, "accept() failed", "NETWORK ERROR", MB_OK);
							return FALSE;
						}
						if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_CLOSE) == SOCKET_ERROR)
						{
							closesocket(sv_sock);
							sv_sock=INVALID_SOCKET;
							closesocket(sock);
							sock=INVALID_SOCKET;
							MessageBox(main_window_handle, "WSAAsyncSelect() failed", "NETWORK ERROR", MB_OK);
							return FALSE;
						}
						netplay_status = 1;
						return TRUE;
					}
				}
				break;

			case WM_COMMAND:
				{
					if(LOWORD(msg.wParam) == ID_FILE_NETPLAY_CANCEL)
					{
						shutdown(sv_sock, 2);
						closesocket(sv_sock);
						sv_sock = INVALID_SOCKET;
						MessageBox(main_window_handle, "accepting canceled", "NETWORK ERROR", MB_OK);
						return FALSE;
					}
				}
				break;

			default:
				{
					//DefWindowProc(main_window_handle, msg.message, msg.wParam, msg.lParam);
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				break;
			}
		}
	}
	return FALSE;
}

boolean SocketConnect()
{
	SOCKADDR_IN sock_addr;
	MSG msg;

	int result = DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_NETPLAY_CLIENT),
								main_window_handle, NetplayClientDialogProc, NULL);

	if(result != IDOK)
	{
		return FALSE;
	}
	if(ip_address == INADDR_NONE)
	{
		MessageBox(main_window_handle, "Invalid IP Address", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	if(ip_port < 1024 || ip_port == 8080 || 65535 < ip_port)
	{
		MessageBox(main_window_handle, "Invalid Port number", "NETWORK ERROR", MB_OK);
		return FALSE;
	}

	memset(&sock_addr, 0x00, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = (unsigned short)ip_port;
	sock_addr.sin_addr.s_addr = ip_address;

	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		MessageBox(main_window_handle, "socket() failed", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_CONNECT) == SOCKET_ERROR)
	{
		closesocket(sock);
		sock = INVALID_SOCKET;
		MessageBox(main_window_handle, "WSAAsyncSelect() failed", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	if(connect(sock, (LPSOCKADDR)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSAEWOULDBLOCK)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			MessageBox(main_window_handle, "connect() failed", "NETWORK ERROR", MB_OK);
			return FALSE;
		}
	}
	while(1)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			switch(msg.message)
			{
			case WM_SOCKET:
				{
					if(WSAGETSELECTERROR(msg.lParam) != 0)
					{
						shutdown(sock, 2);
						closesocket(sock);
						sock = INVALID_SOCKET;
						MessageBox(main_window_handle, "socket error", "NETWORK ERROR", MB_OK);
						return FALSE;
					}
					else if(WSAGETSELECTEVENT(msg.lParam) == FD_CONNECT)
					{
						if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_CLOSE) == SOCKET_ERROR)
						{
							closesocket(sock);
							sock=INVALID_SOCKET;
							MessageBox(main_window_handle, "WSAAsyncSelect() failed", "NETWORK ERROR", MB_OK);
							return FALSE;
						}
						netplay_status = 2;
						return TRUE;
					}
				}
				break;

			case WM_COMMAND:
				{
					if(LOWORD(msg.wParam) == ID_FILE_NETPLAY_CANCEL)
					{
						shutdown(sock, 2);
						closesocket(sock);
						sock = INVALID_SOCKET;
						MessageBox(main_window_handle, "connectiong canceled", "ACCEPTING ERROR", MB_OK);
						return FALSE;
					}
				}
				break;

			default:
				{
					//DefWindowProc(main_window_handle, msg.message, msg.wParam, msg.lParam);
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				break;
			}
		}
	}
	return FALSE;
}

boolean SocketServer()
{
	uint8 client_protocol_type, server_protocol_type = 0x02;
	DWORD dwFirst, dwLast;

	if(!SocketAccept())
	{
		return FALSE;
	}
	// check protocol type
	SocketSendByte(server_protocol_type);
	client_protocol_type = SocketGetByte();
	if(netplay_disconnect)
	{
		SocketClose();
		MessageBox(main_window_handle, "network disconnect", "NETWORK_ERROR", MB_OK);
		return FALSE;
	}
	if(server_protocol_type != client_protocol_type)
	{
		SocketClose();
		MessageBox(main_window_handle, "different version", "NETWORK_ERROR", MB_OK);
		return FALSE;
	}
	// check network latency
	dwFirst = timeGetTime();
	for(uint8 i = 0; i < 4; i++)
	{
		SocketSendByte(0x00);
		SocketGetByte();
	}
	dwLast = timeGetTime();
	if(netplay_disconnect)
	{
		SocketClose();
		MessageBox(main_window_handle, "network disconnect", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	// set and send netplay_latency
	if(netplay_latency == 0)
	{
		netplay_latency = (uint8)((dwLast - dwFirst) * 60 / 8000) + 1;
	}
	if(netplay_latency > 30)
	{
		SocketClose();
		MessageBox(main_window_handle, "network is too heavy", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	SocketSendByte(netplay_latency);
	if(netplay_disconnect)
	{
		SocketClose();
		MessageBox(main_window_handle, "network disconnect", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	return TRUE;
}

boolean SocketClient()
{
	uint8 server_protocol_type, client_protocol_type = 0x02;

	if(!SocketConnect())
	{
		return FALSE;
	}
	// check protocol type
	SocketSendByte(client_protocol_type);
	server_protocol_type = SocketGetByte();
	if(netplay_disconnect)
	{
		SocketClose();
		MessageBox(main_window_handle, "network disconnect", "NETWORK_ERROR", MB_OK);
		return FALSE;
	}
	if(server_protocol_type != client_protocol_type)
	{
		SocketClose();
		MessageBox(main_window_handle, "different version", "NETWORK_ERROR", MB_OK);
		return FALSE;
	}
	// check network latency
	for(uint8 i = 0; i < 4; i++)
	{
		SocketGetByte();
		SocketSendByte(0x00);
	}
	if(netplay_disconnect)
	{
		SocketClose();
		MessageBox(main_window_handle, "network disconnect", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	// get netplay_latency
	netplay_latency = SocketGetByte();
	if(netplay_disconnect)
	{
		SocketClose();
		MessageBox(main_window_handle, "network disconnect", "NETWORK ERROR", MB_OK);
		return FALSE;
	}
	return TRUE;
}
