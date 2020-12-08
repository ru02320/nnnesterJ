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
#define DIRECTDRAW_VERSION 0x0700
#include <DDraw.h>

#include "types.h"
#include "NES_screen_mgr.h"

// NOTE: this class uses a nasssty little hack to provide fast double-size
// operation; calling blt() will cause the buffer to "explode" to
// double-size, changing the contents of the buffer
// in the future, may want to make two buffers, one single size, the other
// double size

class win32_overlaywindowed_NES_screen_mgr : public NES_screen_mgr
{
public:
  win32_overlaywindowed_NES_screen_mgr(HWND wnd_handle);
  ~win32_overlaywindowed_NES_screen_mgr();

  boolean lock(pixmap& p);
  boolean unlock();

  void blt();
  void flip() {}

  void clear(PIXEL color);

  boolean set_palette(const uint8 pal[256][3]);
  boolean get_palette(uint8 pal[256][3]);
  boolean set_palette_section(uint8 start, uint8 len, const uint8 pal[][3]);
  boolean get_palette_section(uint8 start, uint8 len, uint8 pal[][3]);

  void assert_palette();

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
  LPDIRECTDRAWSURFACE7 lpDDSPrimary;
  LPDIRECTDRAWSURFACE7  lpDDSOverlay; // DirectDraw overlay surface
  DDSURFACEDESC2        ddsd;         // a direct draw surface description struct
  DDCAPS             ddcaps;      // a direct draw surface capabilities struct
  LPDIRECTDRAWCLIPPER	clipper;
  LPDIRECTDRAWSURFACE7  lpDDSBack;   // DirectDraw back surface
  LPDIRECTDRAWSURFACE7  lpDDSMsg;   // DirectDraw back surface   (Message Display)

  struct bitmapInfo {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiPalette[256];
  } *bmInfo;

  struct logPalette {
    WORD         palVersion;
    WORD         palNumEntries;
    PALETTEENTRY palPalEntry[256];
  } *palInfo;

  HDC screenDC, bmpDC;
  HPALETTE palHan, oldPal;
  HBITMAP oldBmp;

  RECT rs, rd;
  DWORD dwUpdateFlags;
  DDOVERLAYFX     ovfx;

  DWORD g_dwXRatio,g_dwYRatio;
  unsigned char Menuflag;

  HWND window_handle;

  HBITMAP hBmp;
  PIXEL* buffer;

  int m_viewable_width;
  int m_viewable_height;
  int m_width;
  int m_widthdouble;
  int m_height;
  int m_viewable_area_y_offset;
  int m_viewable_area_x_offset;


  int f_2xsai;
  int mode_2xsai;
  int BitDepth;
  int BitGreen;
  WORD *buffer_16bit;
  WORD palette_16bit[256];
  WORD *buffer_16bit_bouble;
  void (*Func_2xSai)(uint8 *, uint32 , uint8 *,uint8 *, uint32 , int , int );


  int avi_rec;
  int avi_counter;

  int blown_up;
  void scaling(unsigned char *, int, int);
  void emulateTVScanline();
int CheckOverlaySupport();
int DDOverlayInit();
int DDPrimaryInit();
void CheckBoundries();
int create_backsurface();
int create_backsurfacemsg();
private:
};


