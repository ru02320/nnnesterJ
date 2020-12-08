
#include <windows.h>
#include "myconfig.h"

unsigned char Hexstrtouc(char *str){
	unsigned char ret=0, *p = (unsigned char *)str;
	for(int i=0;i<2 && *p!=0;i++, p++){
		if(i)
			ret<<=4;
		if(*p >= 0x30 && *p <= 0x39){
			ret|=(DWORD)*p-'0';
		}
		else if(*p >= 0x41 && *p <= 0x5a){
			ret|=(DWORD)*p-0x37;
		}
		else if(*p >= 0x61 && *p <= 0x7a){
			ret|=(DWORD)*p-0x57;
		}
	}
	return ret;
}


// WindowsのAPIを使用したINI保存
SaveConfigWinIni::SaveConfigWinIni(char *fn){
	int n;

	n = strlen(fn)+1; 
	m_fn = (char *)malloc(n);
	if(m_fn == NULL)
		throw;
	strcpy(m_fn, fn);
}


SaveConfigWinIni::~SaveConfigWinIni(){
	if(m_fn){
		free(m_fn);
		m_fn = NULL;
	}
}


void SaveConfigWinIni::WriteString(char *sec, char *key, char *val){
	WritePrivateProfileString(sec, key, val, m_fn);
}


void SaveConfigWinIni::WriteInt(char *sec, char *key, int val){
	char ts[64];

	wsprintf(ts, "%u", val);
	WritePrivateProfileString(sec, key, ts, m_fn);
}


void SaveConfigWinIni::WriteHexString(char *sec, char *key, unsigned char *val, int size){
	unsigned char *p;
	int i;

	p = (unsigned char *)malloc(size*2+1);
	if(p==NULL)
		return;
	for(i=0; i<size; i++, val++){
		wsprintf((char *)&p[i*2], "%02X", *val);
	}
	WritePrivateProfileString(sec, key, (const char *)p, m_fn);
	free(p);
}


// WindowsのAPIを使用したINI読み込み
LoadConfigWinIni::LoadConfigWinIni(char *fn){
	int n;

	n = strlen(fn)+1; 
	m_fn = (char *)malloc(n);
	if(m_fn == NULL)
		throw;
	strcpy(m_fn, fn);
}


LoadConfigWinIni::~LoadConfigWinIni(){
	if(m_fn){
		free(m_fn);
		m_fn = NULL;
	}
}


int LoadConfigWinIni::ReadString(char *sec, char *key, char *pval, int max, char *defval){
	GetPrivateProfileString(sec, key, defval, pval, max, m_fn);
	return 1;
}


int LoadConfigWinIni::ReadInt(char *sec, char *key, int defval){
	return GetPrivateProfileInt(sec, key, defval, m_fn);
}


//unsigned char Hexstrtouc(char *str){
int LoadConfigWinIni::ReadHexString(char *sec, char *key, unsigned char *pval, int size){
	unsigned char *p;
	int i;

	p = (unsigned char *)malloc(size*2+1);
	if(p==NULL)
		return 0;
	if(GetPrivateProfileString(sec, key, "" , (char *)p, size*2+1, m_fn)){
		for(i=0; i<size; i++, pval++){
			*pval = Hexstrtouc((char *)&p[i*2]);
		}
	}
	free(p);
	return 1;
}


// WindowsのAPIを使用したレジストリ保存
SaveConfigWinReg::SaveConfigWinReg(char *softname){
	DWORD dw;
	HKEY software_key;

	softname_key= 0;
	section_key	= 0;

	if(RegCreateKeyEx(HKEY_CURRENT_USER, "Software", 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &software_key, &dw) != ERROR_SUCCESS){
		throw;
	}
	if(RegCreateKeyEx(software_key, softname, 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &softname_key, &dw) != ERROR_SUCCESS){
		throw;
	}
	RegCloseKey(software_key);
	cur_keyname[0] = 0;
}


SaveConfigWinReg::~SaveConfigWinReg(){
	if(softname_key)
		RegCloseKey(softname_key);
	if(section_key)
		RegCloseKey(section_key);
}


void SaveConfigWinReg::WriteString(char *sec, char *key, char *val){
	OpenSectionKey(sec);
	RegSetValueEx(section_key, key, 0, REG_SZ, (CONST BYTE *)val, strlen(val));
}


void SaveConfigWinReg::WriteInt(char *sec, char *key, int val){
	OpenSectionKey(sec);
	RegSetValueEx(section_key, key, 0, REG_DWORD, (CONST BYTE*)&val, 4);
}


void SaveConfigWinReg::WriteHexString(char *sec, char *key, unsigned char *val, int size){
	unsigned char *p;
	int i;

	p = (unsigned char *)malloc(size*2+1);
	if(p==NULL)
		return;
	for(i=0; i<size; i++, val++){
		wsprintf((char *)&p[i*2], "%02X", *val);
	}
	WriteString(sec, key, (char *)p);
	free(p);
}


void SaveConfigWinReg::OpenSectionKey(char *sec){
	DWORD dw;
	if(!strcmp(cur_keyname, sec))
		return;
	if(section_key){
		RegCloseKey(section_key);
		section_key = 0;
	}
	if(RegCreateKeyEx(softname_key, sec, 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &section_key, &dw) != ERROR_SUCCESS){
		throw;
	}
	strcpy(cur_keyname, sec);
}



// WindowsのAPIを使用したREG読み込み
LoadConfigWinReg::LoadConfigWinReg(char *softname){
	DWORD dw;
	HKEY software_key;

	softname_key= 0;
	section_key	= 0;

	if(RegCreateKeyEx(HKEY_CURRENT_USER, "Software", 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &software_key, &dw) != ERROR_SUCCESS){
		throw;
	}
	if(RegCreateKeyEx(software_key, softname, 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &softname_key, &dw) != ERROR_SUCCESS){
		throw;
	}
	RegCloseKey(software_key);
	cur_keyname[0] = 0;
}


LoadConfigWinReg::~LoadConfigWinReg(){
	if(softname_key)
		RegCloseKey(softname_key);
	if(section_key)
		RegCloseKey(section_key);
}


int LoadConfigWinReg::ReadString(char *sec, char *key, char *pval, int max, char *defval){
	DWORD dwType = REG_SZ;
	DWORD dwByte = max;

	strcpy(pval, defval);
	OpenSectionKey(sec);
	if(RegQueryValueEx(section_key, key, NULL, &dwType, (BYTE *)pval, &dwByte)!=ERROR_SUCCESS)
		return 0;
	return 1;
}


int LoadConfigWinReg::ReadInt(char *sec, char *key, int defval){
	DWORD dwType = REG_DWORD;
	DWORD dwByte = 4;
	DWORD dwVal = defval;

	OpenSectionKey(sec);
	RegQueryValueEx(section_key, key, NULL, &dwType, (BYTE *)&dwVal, &dwByte);
	return dwVal;
}


//unsigned char Hexstrtouc(char *str){
int LoadConfigWinReg::ReadHexString(char *sec, char *key, unsigned char *pval, int size){
	unsigned char *p;
	int i;

	p = (unsigned char *)malloc(size*2+1);
	if(p==NULL)
		return 0;
	memset(p, '0', size*2);
	if(ReadString(sec, key, (char *)p, size*2+1, "")){
		for(i=0; i<size; i++, pval++){
			*pval = Hexstrtouc((char *)&p[i*2]);
		}
	}
	free(p);
	return 1;
}


void LoadConfigWinReg::OpenSectionKey(char *sec){
	DWORD dw;
	if(!strcmp(cur_keyname, sec))
		return;
	if(section_key){
		RegCloseKey(section_key);
		section_key = 0;
	}
	if(RegCreateKeyEx(softname_key, sec, 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &section_key, &dw) != ERROR_SUCCESS){
		throw;
	}
	strcpy(cur_keyname, sec);
}




