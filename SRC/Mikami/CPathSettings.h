#ifndef CSAVEDIRS_H_
#define CSAVEDIRS_H_

#include <windows.h>
#include <shlwapi.h>

class CPathSettings
{
public:
	char szSramPath[MAX_PATH];
	char szStatePath[MAX_PATH];
    char szShotPath[MAX_PATH];
	char szWavePath[MAX_PATH];
	char szLastStatePath[MAX_PATH];
	char szAppPath[MAX_PATH];
	char szMoviePath[MAX_PATH];			// Movie file folder
	char szGameGeniePath[MAX_PATH];		// game genie file folder
	char szNNNcheatPath[MAX_PATH];		// NNN cheat file folder
	char szIPSPatchPath[MAX_PATH];		// IPS Patch file folder
	unsigned char UseSramPath;
	unsigned char UseStatePath;
	unsigned char UseShotPath;
	unsigned char UseWavePath;
	unsigned char UseMoviePath;		//Movie
	unsigned char UseGeniePath;		//Genie
	unsigned char UseNNNchtPath;		//NNN cht
	unsigned char UseIPSPath;		//IPS
	CPathSettings()
	{
	}
	
	~CPathSettings()
	{
	}

};
#endif