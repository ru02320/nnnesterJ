
#include <windows.h>
#include "win32_emu.h"
#include "arc.h"
#include <shlwapi.h>


extern int MystrCmp(char *,char *);

const char *nesromext[] = { "*.nes", "*.fds", "*.fam", "*.nsf", "*.unif", "*.unf", NULL};
const char *pceromext[] = { "*.pce", NULL};
const char *gbromext[] = { "*.gb", "*.gbc", NULL};
const char *snesromext[] = { "*.smc", NULL};

int CheckROMFile(char *fn, HWND hWnd, int *Flag){
	int i;
	char *extp;

	if(Flag != NULL)
		*Flag = 0;
	extp=PathFindExtension(fn);
	if(extp==NULL)
		return -1;
	++extp;
	for(i=0; nesromext[i]; i++){
		if(MystrCmp(extp, (char *)&nesromext[i][2])){
			break;
		}
	}
	if(nesromext[i]!=NULL)
		return EMUTYPE_NES;
	for(i=0; gbromext[i]; i++){
		if(MystrCmp(extp, (char *)&gbromext[i][2])){
			break;
		}
	}
	if(gbromext[i]!=NULL)
		return EMUTYPE_GB;
	for(i=0; pceromext[i]; i++){
		if(MystrCmp(extp, (char *)&pceromext[i][2])){
			break;
		}
	}
	if(pceromext[i]!=NULL)
		return EMUTYPE_PCE;
	for(i=0; snesromext[i]; i++){
		if(MystrCmp(extp, (char *)&snesromext[i][2])){
			break;
		}
	}
	if(snesromext[i]!=NULL)
		return EMUTYPE_SNES;

	if(Flag != NULL)
		*Flag = 1;
	if(ArcFindFile_FromExtention(hWnd, fn, nesromext))
		return EMUTYPE_NES;
	if(ArcFindFile_FromExtention(hWnd, fn, gbromext))
		return EMUTYPE_GB;
	if(ArcFindFile_FromExtention(hWnd, fn, pceromext))
		return EMUTYPE_PCE;
	if(ArcFindFile_FromExtention(hWnd, fn, snesromext))
		return EMUTYPE_SNES;
	return -1;
}


int UncompressROMFile(char *fn, HWND hWnd, unsigned char **buff, unsigned long *size, int emutype){
	switch(emutype){
		case EMUTYPE_NES:
			return Uncompress(hWnd, buff, fn, size, nesromext);
			break;
		case EMUTYPE_GB:
			return Uncompress(hWnd, buff, fn, size, gbromext);
			break;
		case EMUTYPE_PCE:
			return Uncompress(hWnd, buff, fn, size, pceromext);
			break;
	}
	return 0;
}


