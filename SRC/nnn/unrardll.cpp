
/*
  unrar.RAROpenArchive
  unrar.RAROpenArchiveEx
  unrar.RARCloseArchive
  unrar.RARReadHeader
  unrar.RARReadHeaderEx
  unrar.RARProcessFile
  unrar.RARProcessFileW
  unrar.RARSetChangeVolProc
  unrar.RARSetProcessDataProc
  unrar.RARSetCallback
  unrar.RARSetPassword
*/


#define STRICT
#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include "unrar.h"
#include <shlwapi.h>

extern int MystrCmp(char *,char *);


struct FileExtract_Info {
	unsigned char *p;
	int CurPos;
	int Size;
};



int CmpExtensions(const char **pExt, const char *p){
	char *extp;
	extp=PathFindExtension(p);
	++extp;
	for(int j=0; pExt[j]!=NULL; ++j){
		if(MystrCmp(extp, (char *)&pExt[j][2])){
			return 1;
		}
	}
	return 0;
}


int CALLBACK CallbackProc(UINT msg,LONG UserData,LONG P1,LONG P2){
	switch(msg){
		case UCM_PROCESSDATA:
			if (UserData!=0){
				FileExtract_Info *pFE = (FileExtract_Info *)UserData;
				if(pFE->Size - pFE->CurPos >= P2){
					memcpy(&pFE->p[pFE->CurPos], (const void *)P1, P2);
					pFE->CurPos += P2;
				}
//				fwrite((char *)P1,1,P2,stdout);
			}
			return(0);
	}
	return(0);
}


int UseUnrarDllExeLoad(char *fn, unsigned char **buff, unsigned long *ucsize, const char **szExtensions){
	HMODULE unrardll=NULL;

	HANDLE (WINAPI *RAROpenArchiveEx)(struct RAROpenArchiveDataEx *ArchiveData)=NULL;
	int	   (WINAPI *RARCloseArchive)(HANDLE hArcData) = NULL;
	int    (WINAPI *RARReadHeader)(HANDLE hArcData,struct RARHeaderData *HeaderData) = NULL;
	int    (WINAPI *RARProcessFile)(HANDLE hArcData,int Operation,char *DestPath,char *DestName) = NULL;
	void   (WINAPI *RARSetCallback)(HANDLE hArcData,UNRARCALLBACK Callback,LONG UserData) = NULL;

	if(!(unrardll=LoadLibrary("unrar")))
		return FALSE;
	RAROpenArchiveEx = (HANDLE (WINAPI *)(struct RAROpenArchiveDataEx *ArchiveData))GetProcAddress(unrardll, (LPCSTR)"RAROpenArchiveEx");
	RARCloseArchive = (int (WINAPI *)(HANDLE hArcData))GetProcAddress(unrardll, (LPCSTR)"RARCloseArchive");
	RARReadHeader = (int (WINAPI *)(HANDLE hArcData,struct RARHeaderData *HeaderData))GetProcAddress(unrardll, (LPCSTR)"RARReadHeader");
	RARProcessFile = (int (WINAPI *)(HANDLE hArcData,int Operation,char *DestPath,char *DestName))GetProcAddress(unrardll, (LPCSTR)"RARProcessFile");
	RARSetCallback = (void (WINAPI *)(HANDLE hArcData,UNRARCALLBACK Callback,LONG UserData))GetProcAddress(unrardll, (LPCSTR)"RARSetCallback");

	if(!RAROpenArchiveEx || !RARCloseArchive || !RARSetCallback || !RARProcessFile || !RARReadHeader){
		FreeLibrary(unrardll);
		return FALSE;
	}

	HANDLE hArcData;
	int RHCode,PFCode;
	struct RARHeaderData HeaderData;
	struct RAROpenArchiveDataEx OpenArchiveData;

	memset(&OpenArchiveData,0,sizeof(OpenArchiveData));
	OpenArchiveData.ArcName=fn;
	OpenArchiveData.CmtBuf=NULL;
	OpenArchiveData.CmtBufSize=0;
	OpenArchiveData.OpenMode=RAR_OM_EXTRACT;
	hArcData=RAROpenArchiveEx(&OpenArchiveData);

	if (OpenArchiveData.OpenResult!=0){
//		OutOpenArchiveError(OpenArchiveData.OpenResult,ArcName);
		FreeLibrary(unrardll);
		return FALSE;
	}

	FileExtract_Info FE;
	FE.CurPos = 0;
	FE.Size   = 0;
	FE.p = NULL;
	RARSetCallback(hArcData, CallbackProc,(LONG)&FE);


	while ((RHCode=RARReadHeader(hArcData,&HeaderData))==0){
		// 64bit–¢‘Î‰ž
//		int64 UnpSize=HeaderData.UnpSize+(((__int64)HeaderData.UnpSizeHigh)<<32);

		if(CmpExtensions(szExtensions, HeaderData.FileName)){
			FE.Size   = HeaderData.UnpSize;
			FE.p = (unsigned char *)malloc(HeaderData.UnpSize);
			if(FE.p==NULL)
				break;
			PFCode=RARProcessFile(hArcData, RAR_TEST, NULL, NULL);
			if(PFCode == 0){
				if(ucsize)
					*ucsize = HeaderData.UnpSize;
				*buff = FE.p;
				RARCloseArchive(hArcData);
				FreeLibrary(unrardll);
				return TRUE;
			}
			else {
				free(FE.p);
			}
			break;
		}
		else {
			PFCode=RARProcessFile(hArcData, RAR_SKIP, NULL, NULL);
		}
		if (PFCode!=0)
			break;
	}
	RARCloseArchive(hArcData);
	FreeLibrary(unrardll);
	unrardll=NULL;
	return FALSE;
}




int UnRarDllFindFileFromExtention(char *fn, const char **szExtensions){
	HMODULE unrardll=NULL;

	HANDLE (WINAPI *RAROpenArchiveEx)(struct RAROpenArchiveDataEx *ArchiveData)=NULL;
	int	   (WINAPI *RARCloseArchive)(HANDLE hArcData) = NULL;
	int    (WINAPI *RARReadHeader)(HANDLE hArcData,struct RARHeaderData *HeaderData) = NULL;
	int    (WINAPI *RARProcessFile)(HANDLE hArcData,int Operation,char *DestPath,char *DestName) = NULL;
	void   (WINAPI *RARSetCallback)(HANDLE hArcData,UNRARCALLBACK Callback,LONG UserData) = NULL;

	if(!(unrardll=LoadLibrary("unrar")))
		return FALSE;
	RAROpenArchiveEx = (HANDLE (WINAPI *)(struct RAROpenArchiveDataEx *ArchiveData))GetProcAddress(unrardll, (LPCSTR)"RAROpenArchiveEx");
	RARCloseArchive = (int (WINAPI *)(HANDLE hArcData))GetProcAddress(unrardll, (LPCSTR)"RARCloseArchive");
	RARReadHeader = (int (WINAPI *)(HANDLE hArcData,struct RARHeaderData *HeaderData))GetProcAddress(unrardll, (LPCSTR)"RARReadHeader");
	RARProcessFile = (int (WINAPI *)(HANDLE hArcData,int Operation,char *DestPath,char *DestName))GetProcAddress(unrardll, (LPCSTR)"RARProcessFile");
	RARSetCallback = (void (WINAPI *)(HANDLE hArcData,UNRARCALLBACK Callback,LONG UserData))GetProcAddress(unrardll, (LPCSTR)"RARSetCallback");

	if(!RAROpenArchiveEx || !RARCloseArchive || !RARSetCallback || !RARProcessFile || !RARReadHeader){
		FreeLibrary(unrardll);
		return FALSE;
	}

	HANDLE hArcData;
	int RHCode,PFCode;
	struct RARHeaderData HeaderData;
	struct RAROpenArchiveDataEx OpenArchiveData;

	memset(&OpenArchiveData,0,sizeof(OpenArchiveData));
	OpenArchiveData.ArcName=fn;
	OpenArchiveData.CmtBuf=NULL;
	OpenArchiveData.CmtBufSize=0;
	OpenArchiveData.OpenMode=RAR_OM_LIST;
	hArcData=RAROpenArchiveEx(&OpenArchiveData);

	if (OpenArchiveData.OpenResult!=0){
//		OutOpenArchiveError(OpenArchiveData.OpenResult,ArcName);
		FreeLibrary(unrardll);
		return FALSE;
	}

//	RARSetCallback(hArcData, CallbackProc,(LONG)&FE);


	while ((RHCode=RARReadHeader(hArcData,&HeaderData))==0){
		// 64bit–¢‘Î‰ž
//		int64 UnpSize=HeaderData.UnpSize+(((__int64)HeaderData.UnpSizeHigh)<<32);

		if(CmpExtensions(szExtensions, HeaderData.FileName)){
			RARCloseArchive(hArcData);
			FreeLibrary(unrardll);
			return TRUE;
		}
		PFCode=RARProcessFile(hArcData, RAR_SKIP, NULL, NULL);
		if (PFCode!=0)
			break;
	}
	RARCloseArchive(hArcData);
	FreeLibrary(unrardll);
	unrardll=NULL;
	return FALSE;
}




