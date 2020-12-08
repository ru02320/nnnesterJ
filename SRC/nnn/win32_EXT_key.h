#ifndef WIN32_EXTKEY_H_
#define WIN32_EXTKEY_H_

#include <windows.h>
#include "NES_settings.h"
//#include "INPButton.h"
#include "OSD_ButtonSettings.h"
#include "nnnextkeycfg.h"
#include "win32_directinput_input_mgr.h"


class win32_EXT_key
{
public:
  win32_EXT_key(OSD_ButtonSettings* , win32_directinput_input_mgr* );
  ~win32_EXT_key();

  void Poll();
  unsigned char Buttonp[EXTKEYALLNUM];
  OSD_ButtonSettings ExtButton[EXTKEYALLNUM];
protected:
  win32_directinput_input_mgr *inpm;
  unsigned char Buttonf[EXTKEYALLNUM];


  void CreateButtons(OSD_ButtonSettings* , win32_directinput_input_mgr* );
  void DeleteButtons();

private:
};

#endif