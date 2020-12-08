/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#include <windows.h>
#include <windowsx.h> 

#include <ddraw.h>  // direct draw

#include "types.h"
#include "NES_screen_mgr.h"
#include "pixmap.h"

class win32_nnnfullscreen_NES_screen_mgr : public NES_screen_mgr
{
public:
  win32_nnnfullscreen_NES_screen_mgr(HWND wnd_handle, GUID* DeviceGUID, int do_scaling);
  ~win32_nnnfullscreen_NES_screen_mgr();

  boolean lock(pixmap& p);
  boolean unlock();

  void blt();
  void flip();

  void clear(PIXEL color);

  boolean set_palette(const uint8 pal[256][3]);
  boolean get_palette(uint8 pal[256][3]);
  boolean set_palette_section(uint8 start, uint8 len, const uint8 pal[][3]);
  boolean get_palette_section(uint8 start, uint8 len, uint8 pal[][3]);

  void assert_palette();

  void clear_margins();

  enum {
    VIDMEM_NUM_BACK_BUFFERS = 4,
    SYSMEM_NUM_BACK_BUFFERS = 2,
  };

  void shot_screen( char *szFileName );
  int start_avirec(char *fn);
  void frame_avirec();
  void end_avirec();
  int is_avirec(){ return avi_rec; }


  void nnnManuEnable(unsigned char);
  void EndEmu();
  void StartEmu();
protected:
  LPDIRECTDRAW7 lpDD; // DirectDraw object

  LPDIRECTDRAWSURFACE7  lpddsprimary; // dd primary surface
  LPDIRECTDRAWSURFACE7  lpddsback;    // dd back surface
  LPDIRECTDRAWPALETTE  lpDDPal;      // The primary surface palette
  DDSURFACEDESC2        ddsd;         // a direct draw surface description struct
  DDSCAPS2              ddscaps;      // a direct draw surface capabilities struct
  HRESULT              ddrval;       // result back from dd calls
  LPDIRECTDRAWCLIPPER  clipper;     // Clipper for primary

  LPDIRECTDRAWSURFACE7  lpddsbackbuf; // this is where the emu draws
  RECT nes_screen_rect; // rectangle of actual NES screen
  RECT back_buffer_nes_screen_rect; // rectangle of actual NES screen on back buffer
  RECT top_rect;   // top margin
  RECT left_rect;  // left margin
  RECT right_rect; // right margin
  RECT bot_rect;   // bottom margin

  pixmap dx_pmap; // info on directx buffer
  int dx_locked;

  // dimensions of device surface (320x240, 640x480...)
  uint32 fullscreen_width;
  uint32 fullscreen_height;

  int setResolution();
  int checkResolution();

  HWND   window_handle;
  uint32 pitch;
//  PIXEL* buffer;
  BOOL MenuFlag;
  int avi_rec;
  int avi_counter;
  RGBQUAD Palette[256];

  unsigned char buffer[(256+16)*(240+16)];

  int m_viewable_width;
  int m_viewable_height;
  int m_width;
  int m_widthdouble;
  int m_height;
  int m_viewable_area_y_offset;
  int m_viewable_area_x_offset;


  int f_2xsai;
  int mode_2xsai;
  int BitGreen;
  WORD *buffer_16bit;
  WORD palette_16bit[256];
  WORD *buffer_16bit_bouble;
  void (*Func_2xSai)(uint8 *, uint32 , uint8 *,uint8 *, uint32 , int , int );

/*
	  NES_SCREEN_WIDTH  = 256,
	  NES_SCREEN_HEIGHT = 240,
	  SIDE_MARGIN = 8,
*/

  uint32 magnification;
  uint8 Bitdepth;
//  unsigned char menuflag; //menu flag

  uint32 get_magnified_viewable_width() { return (magnification * get_viewable_width()); }
  uint32 get_magnified_viewable_height() { return (magnification * get_viewable_height()); }
private:
};

