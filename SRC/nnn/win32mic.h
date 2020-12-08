#ifndef WIN32_BUTTON_H_
#define WIN32_BUTTON_H_

#include <windows.h>
#include "NES_settings.h"
#include "OSD_ButtonSettings.h"
#include "nnnextkeycfg.h"
#include "win32_directinput_input_mgr.h"


class win32_NES_button
{
public:
	win32_NES_button(OSD_ButtonSettings* btnsettings, unsigned char *btn_bit, win32_directinput_input_mgr* inp_mgr){
		inpm = inp_mgr;
		button = btn_bit;
		inpm->Setos(btnsettings, &OSD_Button);
	}
  void Poll(){
	if(inpm->Pressed(&OSD_Button))
		*button=4;
	else
		*button=0;
  }
protected:
  win32_directinput_input_mgr *inpm;
  unsigned char *button;
  OSD_ButtonSettings OSD_Button;
};

#endif
