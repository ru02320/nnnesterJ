/**
 * みかみかなによる拡張
 * GPL2 に従ってください
 */
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include "arc.h"
#define M_ERROR_MESSAGE_OFF		0x00800000L

#include "ulunzip.h"
#include "settings.h"
#include "unrarp.h"

extern int MystrCmp(char *,char *);
int UseUnrarDllExeLoad(char *fn, unsigned char **buff, unsigned long *ucsize, const char **szExtensions);
int UnRarDllFindFileFromExtention(char *fn, const char **szExtensions);


static const char *szLibNames[] = {
		"Unlha32",
		"UnZip32",
		"Unrar32",
		"Cab32",
		"Tar32",
		"Unarj32j",
		"Bga32",
		"UnGCA32",
		"7-zip32"
	};

static const char *szFuncNamePreFixes[] = {
		"Unlha",
		"UnZip",
		"Unrar",
		"Cab",
		"Tar",
		"Unarj",
		"Bga",
		"UnGCA",
		"SevenZip"
	};

static const char *szCommands[] = {
		NULL,
		NULL,
		"-e -u \"%s\" \"%s\" \"%s\"",
		"-x -j \"%s\" \"%s\" \"%s\"",
		"-x -f \"%s\" -o \"%s\" \"%s\"",
		"e \"%s\" \"%s\" \"%s\"",
		"x -n \"%s\" \"%s\" \"%s\"",
		NULL,
		"e \"%s\" -hide -o%s \"%s\""
//		"e \"%s\" -o\"%s\" \"%s\""
	};

static const char *ArcExtstr[] = {
		"lzh", "zip", "rar", "cab",
		"tar", "arj", "bza", "gca",
		"lha", "jar", "tgz", "tbz",
		"gz" , "bz2", "gza", "7z"
	};

#define ARC_EXT_MAX 16

#define LIB_UNLHA	0
#define LIB_UNZIP	1
#define LIB_UNRAR	2
#define LIB_CAB		3
#define LIB_TAR		4
#define LIB_UNARJ	5
#define LIB_BGA		6
#define LIB_GCA		7
#define LIB_7ZIP	8


//"Unlha32",	LZH、LHA
//"UnZip32",	ZIP、JAR
//"Unrar32",	RAR
//"Cab32",		CAB
//"Tar32",		TAR、TGZ(tar.gz)、TAZ(tar.gz)、GZ、Z、BZ2
//"Unarj32j",	ARJ
//"Bga32"		GZA、BZA
//"UnGCA32"		GCA

BOOL Uncompress( HWND hwnd, unsigned char **ptrlpBuf, LPCSTR ArcName, LPDWORD lpdwSize , const char **szExtensions)
{


	int i,j,result;
	HMODULE hLib=NULL;
	HANDLE hf;
	HARC harc = NULL;
	char szFuncName[256], *extp, flag;
	char szCommand[512];
	char szTempPath[512];
	char szTempFullName[512];
	EXECUTECOMMAND ExecuteCommand;
//	CHECKARCHIVE CheckArchive;
	EXTRACTMEM ExtractMem;
	OPENARCHIVE OpenArchive;
	CLOSEARCHIVE CloseArchive;
	FINDFIRST FindFirst;
	INDIVIDUALINFO idvinfo;
	GetTempPath( MAX_PATH + 1, szTempPath );

	extp=PathFindExtension(ArcName);
	++extp;
	for(i=0,flag=0; i<ARC_EXT_MAX; ++i){
		if(MystrCmp(extp, (char *)ArcExtstr[i])){
			flag=1;
			break;
		}
	}
	if(!flag)
		return FALSE;
	if(i==14)
		i=LIB_BGA;
	else if(i==8)
		i=LIB_UNLHA;
	else if(i==9)
		i=LIB_UNZIP;
	else if(i>=10 && i < 15)
		i=LIB_TAR;
	else if(i==15)
		i=LIB_7ZIP;
	if(i==1 && !NESTER_settings.nes.preferences.ZipDLLUseFlag){
		if(UseUnzipLibExeLoad((char *)ArcName, (unsigned char **)(ptrlpBuf), lpdwSize, szExtensions))
			return TRUE;
//		else
//			return FALSE;
	}
	else if(i==LIB_UNRAR && !NESTER_settings.nes.preferences.ZipDLLUseFlag){
//		if(UnRarToMemExec((char *)ArcName, (unsigned char **)(ptrlpBuf), lpdwSize, szExtensions))
//			return TRUE;
		if(UseUnrarDllExeLoad((char *)ArcName, (unsigned char **)(ptrlpBuf), lpdwSize, szExtensions))
			return TRUE;
	}

//	for( i=0; i<7; i++ )
	{
		if(!(hLib = LoadLibrary(szLibNames[i])))
			return FALSE;
/*		sprintf( szFuncName, "%sCheckArchive", szFuncNamePreFixes[i] );
		if( !(CheckArchive = (CHECKARCHIVE)GetProcAddress( hLib, szFuncName ) ) )
			return FALSE;
		if( !CheckArchive( ArcName, 1 ) )
			return FALSE;
*/
		sprintf( szFuncName, "%sOpenArchive", szFuncNamePreFixes[i] );
		OpenArchive = (OPENARCHIVE)GetProcAddress( hLib, szFuncName );

		sprintf( szFuncName, "%sFindFirst", szFuncNamePreFixes[i] );
		FindFirst = (FINDFIRST)GetProcAddress( hLib, szFuncName );

		sprintf( szFuncName, "%sCloseArchive", szFuncNamePreFixes[i] );
		CloseArchive = (CLOSEARCHIVE)GetProcAddress( hLib, szFuncName );

		result = -1;
		for( j=0; szExtensions[j]; j++ ){
			if( !(harc = OpenArchive( hwnd, ArcName, M_ERROR_MESSAGE_OFF ) ) ){	
				CloseArchive( harc );
				break;
			}
			result = FindFirst( harc, szExtensions[j], &idvinfo );
			CloseArchive( harc );
			if( result == 0 )
				break;
		}
		
		if( result )
			return FALSE;

		if( szCommands[i] ){
			sprintf( szCommand, szCommands[i], ArcName,  szTempPath, idvinfo.szFileName );
			ExecuteCommand = (EXECUTECOMMAND)GetProcAddress( hLib, szFuncNamePreFixes[i] );
			ExecuteCommand( hwnd, szCommand, NULL, 0 );

			sprintf( szTempFullName, "%s%s", szTempPath, idvinfo.szFileName );
			hf = CreateFile(szTempFullName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL );
			if( hf == INVALID_HANDLE_VALUE ){
				FreeLibrary(hLib);
				return FALSE;
			}
			*lpdwSize = GetFileSize( hf, NULL );
			*ptrlpBuf = (unsigned char*)malloc(*lpdwSize);
			DWORD rsize;
			ReadFile( hf, *ptrlpBuf, *lpdwSize, &rsize, NULL );
			CloseHandle( hf );
			DeleteFile( szTempFullName );
			if( *lpdwSize != rsize ){
				FreeLibrary(hLib);
				return FALSE;
			}
			FreeLibrary(hLib);
			return TRUE;
		}
		else
		{
			char sz[FNAME_MAX32 + 1];
			if( i == LIB_UNZIP ){
				int p1=0, p2=0;
				while(true){
					if( IsDBCSLeadByte( idvinfo.szFileName[p1] ) ){
						sz[p2++] = idvinfo.szFileName[p1++];
					}
					else switch( idvinfo.szFileName[p1] ){
						case '[':case ']':
							sz[p2++] = '\\';
					}
					if( !( sz[p2++] = idvinfo.szFileName[p1++] ) )
						break;
				}
			}
			else
				strcpy( sz, idvinfo.szFileName );
			*lpdwSize = idvinfo.dwOriginalSize;
			*ptrlpBuf = (unsigned char*)malloc(*lpdwSize);
			if(i==LIB_GCA)
				sprintf( szCommand, " ex \"%s\" %s \"%s\"", ArcName, szTempPath, sz);
			else
				sprintf( szCommand, "\"%s\" \"%s\"", ArcName, sz );
			sprintf( szFuncName, "%sExtractMem", szFuncNamePreFixes[i] );
			ExtractMem = (EXTRACTMEM)GetProcAddress( hLib, szFuncName );
			result = ExtractMem(hwnd, szCommand, (LPBYTE)(*ptrlpBuf), *lpdwSize,NULL, NULL, NULL ); 
			if( result == 0 ){
				FreeLibrary(hLib);
				return TRUE;
			}
		}
	}
	if(hLib)
		FreeLibrary(hLib);
	return FALSE;
}




BOOL ArcFindFile_FromExtention( HWND hwnd, LPCSTR ArcName, const char **szExtensions){
	int i,j,result;
	HMODULE hLib=NULL;
	HARC harc = NULL;
	char szFuncName[256], *extp, flag;
	char szTempPath[512];
	OPENARCHIVE OpenArchive;
	CLOSEARCHIVE CloseArchive;
	FINDFIRST FindFirst;
	INDIVIDUALINFO idvinfo;
	GetTempPath( MAX_PATH + 1, szTempPath );

	extp=PathFindExtension(ArcName);
	++extp;
	for(i=0,flag=0; i<ARC_EXT_MAX; ++i){
		if(MystrCmp(extp, (char *)ArcExtstr[i])){
			flag=1;
			break;
		}
	}
	if(!flag)
		return FALSE;
	if(i==14)
		i=LIB_BGA;
	else if(i==8)
		i=LIB_UNLHA;
	else if(i==9)
		i=LIB_UNZIP;
	else if(i>=10 && i < 15)
		i=LIB_TAR;
	else if(i==15)
		i=LIB_7ZIP;
	if(i==1 && !NESTER_settings.nes.preferences.ZipDLLUseFlag){
		if(UsezipLibFindFileFromExtention((char *)ArcName, szExtensions))
			return TRUE;
//		else
//			return FALSE;
	}
	else if(i==LIB_UNRAR && !NESTER_settings.nes.preferences.ZipDLLUseFlag){
		if(UnRarDllFindFileFromExtention((char *)ArcName, szExtensions))
			return TRUE;
	}

//	for( i=0; i<7; i++ )
	{
		if(!(hLib = LoadLibrary(szLibNames[i])))
			return FALSE;
/*		sprintf( szFuncName, "%sCheckArchive", szFuncNamePreFixes[i] );
		if( !(CheckArchive = (CHECKARCHIVE)GetProcAddress( hLib, szFuncName ) ) )
			return FALSE;
		if( !CheckArchive( ArcName, 1 ) )
			return FALSE;
*/
		sprintf( szFuncName, "%sOpenArchive", szFuncNamePreFixes[i] );
		OpenArchive = (OPENARCHIVE)GetProcAddress( hLib, szFuncName );

		sprintf( szFuncName, "%sFindFirst", szFuncNamePreFixes[i] );
		FindFirst = (FINDFIRST)GetProcAddress( hLib, szFuncName );

		sprintf( szFuncName, "%sCloseArchive", szFuncNamePreFixes[i] );
		CloseArchive = (CLOSEARCHIVE)GetProcAddress( hLib, szFuncName );

		result = -1;
		for( j=0; szExtensions[j]; j++ ){
			if( !(harc = OpenArchive( hwnd, ArcName, M_ERROR_MESSAGE_OFF ) ) ){	
				CloseArchive( harc );
				break;
			}
			result = FindFirst( harc, szExtensions[j], &idvinfo );
			CloseArchive( harc );
			if( result == 0 )
				break;
		}
		if( result ){
			FreeLibrary(hLib);
			return FALSE;
		}
	}
	if(hLib)
		FreeLibrary(hLib);
	return TRUE;
}


