
#include <ddraw.h>
#include "win32_screen_mgr.h"
#include "win32_GUID.h"


win32_screen_mgr::win32_screen_mgr(HWND window_handle)
{
	wnd_handle = window_handle;
	nes_screen = NULL;
#ifndef _NES_ONLY
	gb_screen = NULL;
	pce_screen = NULL;
#endif
	fullscreen = FALSE;
	emutype = EMUTYPE_NES;

	if(!SetScreenmgr(emutype))
	{
		throw "Error initializing win32 windowed screen manager";
	}
}


win32_screen_mgr::~win32_screen_mgr()
{
	if(nes_screen)
		delete nes_screen;
#ifndef _NES_ONLY
	if(gb_screen)
		delete gb_screen;
	if(pce_screen)
		delete pce_screen;
#endif
}


boolean win32_screen_mgr::toggle_fullscreen()
{
	switch(emutype){
	case EMUTYPE_NES:
		return nes_screen->toggle_fullscreen();
		break;
	}
	return FALSE;
}


boolean win32_screen_mgr::is_fullscreen(){
	switch(emutype){
	case EMUTYPE_NES:
		return nes_screen->is_fullscreen();
		break;
	}
	return FALSE;
}


boolean win32_screen_mgr::SetScreenmgr(int emu_type){
	if(emu_type==-1)
		emu_type = emutype;
	try {
		switch(emu_type){
		case EMUTYPE_NES:
			if(nes_screen){
				nes_screen->SetScreenmgr();
			}
			else{
				win32_NES_screen_mgr* sm;
				sm = new win32_NES_screen_mgr(wnd_handle);
				nes_screen = sm;
				emutype = EMUTYPE_NES;
			}
			break;
#ifndef _NES_ONLY
		case EMUTYPE_GB:
			if(gb_screen){
				gb_screen->SetScreenmgr();
			}
			else{
				win32_GB_screen_mgr* sm;
				sm = new win32_GB_screen_mgr(wnd_handle);
				gb_screen = sm;
				emutype = EMUTYPE_GB;
			}
			break;
		case EMUTYPE_PCE:
			if(pce_screen){
				pce_screen->SetScreenmgr();
			}
			else{
				win32_PCE_screen_mgr* sm;
				sm = new win32_PCE_screen_mgr(wnd_handle);
				pce_screen = sm;
				emutype = EMUTYPE_PCE;
			}
			break;
#endif
		default:
			throw;
			break;
		}
	} catch(const char* IFDEBUG(s)) {
		LOG(s << endl);
		return FALSE;
	} catch(...) {
		return FALSE;
	}
	emutype = emu_type;
	//  assert_palette();
	return TRUE;
}


uint32 win32_screen_mgr::get_width(){
	switch(emutype){
	case EMUTYPE_NES:
		return nes_screen->get_viewable_width();
		break;
#ifndef _NES_ONLY
	case EMUTYPE_GB:
		return GB_SCREEN_WIDTH;
		break;
	case EMUTYPE_PCE:
		return PCE_SCREEN_WIDTH;
		break;
#endif
	}
	return 100;
}

uint32 win32_screen_mgr::get_height(){
	switch(emutype){
	case EMUTYPE_NES:
		return nes_screen->get_viewable_height();
		break;
#ifndef _NES_ONLY
	case EMUTYPE_GB:
		return GB_SCREEN_HEIGHT;
		break;
	case EMUTYPE_PCE:
		return PCE_SCREEN_HEIGHT;
		break;
#endif
	}
	return 100;
}

void win32_screen_mgr::nnnManuEnable(unsigned char f){
	switch(emutype){
	case EMUTYPE_NES:
		nes_screen->nnnManuEnable(f);
		break;
#ifndef _NES_ONLY
	case EMUTYPE_GB:
		gb_screen->nnnManuEnable(f);
		break;
#endif
	}
}


win32_NES_screen_mgr* win32_screen_mgr::Get_Nes_Screenmgr(){
	switch(emutype){
	case EMUTYPE_NES:
		return nes_screen;
		break;
#ifndef _NES_ONLY
	case EMUTYPE_GB:
		delete gb_screen;
		gb_screen = NULL;
		SetScreenmgr(EMUTYPE_NES);
		return nes_screen;
		break;
	case EMUTYPE_PCE:
		delete pce_screen;
		pce_screen = NULL;
		SetScreenmgr(EMUTYPE_NES);
		return nes_screen;
		break;
#endif
	}
	return nes_screen;
}


#ifndef _NES_ONLY
win32_GB_screen_mgr* win32_screen_mgr::Get_GB_Screenmgr(){
	switch(emutype){
	case EMUTYPE_NES:
		delete nes_screen;
		nes_screen = NULL;
		SetScreenmgr(EMUTYPE_GB);
		return gb_screen;
		break;
	case EMUTYPE_GB:
		return gb_screen;
		break;
	case EMUTYPE_PCE:
		delete pce_screen;
		pce_screen = NULL;
		SetScreenmgr(EMUTYPE_GB);
		return gb_screen;
		break;
	}
	return gb_screen;
}
#endif


#ifndef _NES_ONLY
win32_PCE_screen_mgr* win32_screen_mgr::Get_PCE_Screenmgr(){
	switch(emutype){
	case EMUTYPE_NES:
		delete nes_screen;
		nes_screen = NULL;
		SetScreenmgr(EMUTYPE_PCE);
		return pce_screen;
		break;
	case EMUTYPE_GB:
		delete gb_screen;
		gb_screen = NULL;
		SetScreenmgr(EMUTYPE_PCE);
		return pce_screen;
		break;
	case EMUTYPE_PCE:
		return pce_screen;
		break;
	}
	return pce_screen;
}
#endif


/*
void win32_screen_mgr::set_screenmode(unsigned char sm)
{
	if(ScreenMode != sm){
		ScreenMode = sm;
		screen->set_screenmode(sm);
	}
}
*/
