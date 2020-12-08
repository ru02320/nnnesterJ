

#include <windows.h>
#include "nnnkaillera.h"
#include "kailleraclient.h"


static int KailleraInit_Flag = 0;


BOOL nnnkailleraInit(){
	if(KailleraInit_Flag){
		kailleraShutdown();
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

