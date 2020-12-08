#include <windows.h>
#include <windowsx.h> 

#include "types.h"
//#include "screen_mgr.h"
#include "win32_NES_screen_mgr.h"
#include "win32_windowed_NES_screen_mgr.h"

#ifndef _NES_ONLY

#include "win32_GB_screen_mgr.h"
#include "win32_PCE_screen_mgr.h"

#endif

#include "emulator.h"


//#define EMUTYPE_NONE	0


class win32_screen_mgr
{
public:
	win32_screen_mgr(HWND window_handle);
	~win32_screen_mgr();

	boolean toggle_fullscreen();
	boolean is_fullscreen(); // { return fullscreen; }
	boolean SetScreenmgr(int);
	win32_NES_screen_mgr* Get_Nes_Screenmgr();
#ifndef _NES_ONLY
	win32_GB_screen_mgr* Get_GB_Screenmgr();
	win32_PCE_screen_mgr* Get_PCE_Screenmgr();
#endif
	void assert_palette(){if(nes_screen) nes_screen->assert_palette();}
	uint32 get_width();
	uint32 get_height();
	void nnnManuEnable(unsigned char);

protected:
	HWND wnd_handle;
	boolean fullscreen;

	win32_NES_screen_mgr* nes_screen; // ptr windowed/fullscreen screen manager
#ifndef _NES_ONLY
	win32_GB_screen_mgr* gb_screen;
	win32_PCE_screen_mgr* pce_screen;
#endif

	int emutype;
	boolean GoWindowed();
	boolean GoFullscreen();
private:
};

