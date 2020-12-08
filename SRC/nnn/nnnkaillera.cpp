

#include <windows.h>
#include "nnnkaillera.h"


#ifndef NNN_KAILLERA



HMODULE nnnkailleradll=NULL;

static int (WINAPI *ModifyPlay)(void *, int )=NULL;
static int (WINAPI *ChatSend)(char *) = NULL;


BOOL nnnkailleraInit(){
	void (WINAPI *Init)(VOID);
	if(nnnkailleradll != NULL)
		return TRUE;		//FALSE
	if(!(nnnkailleradll=LoadLibrary("kailleraclient")))
		return FALSE;
	if(Init = (void(WINAPI *)(VOID))GetProcAddress(nnnkailleradll, (LPCSTR)"_kailleraInit@0")){
		Init();
		ModifyPlay = (int(WINAPI *)(void *, int ))GetProcAddress(nnnkailleradll, (LPCSTR)"_kailleraModifyPlayValues@8");
		ChatSend = (int(WINAPI *)(char *))GetProcAddress(nnnkailleradll, (LPCSTR)"_kailleraChatSend@4");
	}
	else{
		FreeLibrary(nnnkailleradll);
		nnnkailleradll=NULL;
		return FALSE;
	}
	return TRUE;
}


void nnnkailleraShutdown(){
	void (WINAPI *Shutdown)(VOID);
	if(nnnkailleradll){
		if(Shutdown = (void(WINAPI *)(VOID))GetProcAddress(nnnkailleradll, (LPCSTR)"_kailleraShutdown@0")){
			Shutdown();
		}
		FreeLibrary(nnnkailleradll);
		nnnkailleradll=NULL;
	}
	return;
}


void nnnkailleraSetInfos(kailleraInfos *kInfo){
	void (WINAPI *SetInfos)(kailleraInfos *);
	if(nnnkailleradll){
		if(SetInfos = (void(WINAPI *)(kailleraInfos *))GetProcAddress(nnnkailleradll, (LPCSTR)"_kailleraSetInfos@4")){
			SetInfos(kInfo);
		}
	}
	return;
}


void nnnkailleraSelectServerDialog(HWND hwnd){
	void (WINAPI *SelectServer)(HWND );
	if(nnnkailleradll){
		if(SelectServer = (void(WINAPI *)(HWND ))GetProcAddress(nnnkailleradll, (LPCSTR)"_kailleraSelectServerDialog@4")){
			SelectServer(hwnd);
		}
	}
	return;
}


int nnnkailleraModifyPlayValues(void *values, int size){
	if(nnnkailleradll){
		if(ModifyPlay != NULL){
			return ModifyPlay(values, size);
		}
	}
	return -1;
}


void nnnkailleraEndGame(){
	int (WINAPI *EndGame)();
	if(nnnkailleradll){
		if(EndGame = (int(WINAPI *)())GetProcAddress(nnnkailleradll, (LPCSTR)"_kailleraEndGame@0")){
			EndGame();
		}
	}
	return;
}


void nnnkailleraChatSend(char *str){
	if(nnnkailleradll){
		if(ChatSend){
			ChatSend(str);
		}
	}
	return;
}



/*
_kailleraGetVersion@4
kailleraGetVersion(char *version);
*/

#else



#include <windows.h>
#include "nnnkaillera.h"
#include "kailleraclient.h"


static int KailleraInit_Flag = 0;


BOOL nnnkailleraInit(){
	if(KailleraInit_Flag){
		return TRUE;
//		kailleraShutdown();
	}
	kailleraInit();
	KailleraInit_Flag = 1;
	return TRUE;
}


void nnnkailleraShutdown(){
	if(KailleraInit_Flag){
		kailleraShutdown();
	}
	KailleraInit_Flag = 0;
	return;
}


void nnnkailleraSetInfos(kailleraInfos *kInfo){
	if(!KailleraInit_Flag){
		return;
	}
	kailleraSetInfos(kInfo);
	return;
}


void nnnkailleraSelectServerDialog(HWND hwnd){
	if(!KailleraInit_Flag){
		return;
	}
	kailleraSelectServerDialog(hwnd);
	return;
}


int nnnkailleraModifyPlayValues(void *values, int size){
	if(!KailleraInit_Flag){
		return -1;
	}
	return kailleraModifyPlayValues(values, size);
}


void nnnkailleraEndGame(){
	if(!KailleraInit_Flag){
		return;
	}
	kailleraEndGame();
	return;
}


void nnnkailleraChatSend(char *str){
	if(!KailleraInit_Flag){
		return;
	}
	kailleraChatSend(str);
	return;
}



/*
_kailleraGetVersion@4
kailleraGetVersion(char *version);
*/



#endif
