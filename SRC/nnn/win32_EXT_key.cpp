
#include "win32_EXT_key.h"
#include "nnnextkeycfg.h"

win32_EXT_key::win32_EXT_key(OSD_ButtonSettings* btnsettings, win32_directinput_input_mgr* inp_mgr)
{
	inpm = inp_mgr;
	for(int i=0; i<EXTKEYALLNUM; ++i){
		Buttonp[i]=0;
		Buttonf[i]=0;
	}
	CreateButtons(btnsettings, inp_mgr);
}

win32_EXT_key::~win32_EXT_key()
{
  DeleteButtons();
}

void win32_EXT_key::Poll()
{
	for(int i=0; i<EXTKEYALLNUM; ++i){
		if(Buttonf[i]==0){
			if(inpm->Pressed(&ExtButton[i])){
				Buttonp[i]=1;
				Buttonf[i]=1;
			}
		}
		else{
			if(!inpm->Pressed(&ExtButton[i])){
				Buttonp[i]=2;
				Buttonf[i]=0;
			}
		}
	}
}


void win32_EXT_key::CreateButtons(OSD_ButtonSettings* bsettings, win32_directinput_input_mgr* inp_mgr)
{
	DeleteButtons();
	for(int i=0; i<EXTKEYALLNUM; ++i){
		inpm->Setos(&bsettings[i], &ExtButton[i]);
	}
}


void win32_EXT_key::DeleteButtons()
{
	for(int i=0; i<EXTKEYALLNUM; ++i){
		ExtButton[i].type = D_NONE;
	}
}


