

#include "NES_settings.h"
#include "OSD_ButtonSettings.h"



void NES_controller_input_settings::OSD_SetDefaults(int n){
	Clear();
}


void NES_input_settings::OSD_EKey_SetDefaults(){
	ExtBtnClear();
	extkeycfg[3].type = D_KEYBOARD_KEY;		// FDS 1A
	extkeycfg[3].j_offset = DIK_A;
	extkeycfg[4].type = D_KEYBOARD_KEY;		// FDS 1B
	extkeycfg[4].j_offset = DIK_B;
	extkeycfg[5].type = D_KEYBOARD_KEY;		// FDS 2A
	extkeycfg[5].j_offset = DIK_C;
	extkeycfg[6].type = D_KEYBOARD_KEY;		// FDS 2B
	extkeycfg[6].j_offset = DIK_D;
	extkeycfg[7].type = D_KEYBOARD_KEY;		// FDS EJECT
	extkeycfg[7].j_offset = DIK_E;

	extkeycfg[12].type = D_KEYBOARD_KEY;		// Screen Shot
	extkeycfg[12].j_offset = DIK_F12;
	extkeycfg[14].type = D_KEYBOARD_KEY;		// Wave Record
	extkeycfg[14].j_offset = DIK_F11;

	extkeycfg[0x10].type = D_KEYBOARD_KEY;		// Quick Save
	extkeycfg[0x10].j_offset = DIK_F5;
	extkeycfg[0x11].type = D_KEYBOARD_KEY;		// Quick Load
	extkeycfg[0x11].j_offset = DIK_F7;

	MicButton.type = D_KEYBOARD_KEY;
	MicButton.j_offset = DIK_M;
}
