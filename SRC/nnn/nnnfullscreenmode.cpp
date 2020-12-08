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

#include "nnnfullscreenmode.h"
#include "nes.h"
#include "debug.h"
#include <ddraw.h>  // direct draw


#include "screenshot.h"
#include "recavi.h"
#include "ddblt.h"

extern char g_ScreenMsgStr[256];
extern int ddrawfullscreenblt_8bit(DDSURFACEDESC2 *ddsd, DWORD *palette, unsigned char *bmpd, DWORD height);
extern LOGFONT Default_Font;
int ddrawblt_8bit(DDSURFACEDESC2 *ddsd, DWORD *palette, unsigned char *bmpd, DWORD width, DWORD height, DWORD dPitch);
int ddrawblt_8bit2(DDSURFACEDESC2 *ddsd, DWORD *palette, unsigned char *bmpd, DWORD width, DWORD height, DWORD dPitch);



win32_nnnfullscreen_NES_screen_mgr::win32_nnnfullscreen_NES_screen_mgr(HWND wnd_handle,
                                                                 GUID* DeviceGUID,
                                                                 int do_scaling)
{
	PALETTEENTRY  ape[256];

	window_handle = wnd_handle;

	lpDD         = NULL;
	lpddsprimary  = NULL;
	lpddsback     = NULL;
	lpDDPal       = NULL;      // The primary surface palette
	int coope =NULL;
	clipper=NULL;
	
	dx_locked = 0;
	//  buffer    = NULL;
	MenuFlag=0;
	avi_rec = 0;
	avi_counter=0;
	Bitdepth=0;
	BitGreen=0;
	f_2xsai = 0;
	buffer_16bit = NULL;
	buffer_16bit_bouble=NULL;
	
  try {
//LOG("Fullscreen" << endl);
	memset(buffer, 0, (256+16)*(240+16));
	memset(Palette, 0, sizeof(RGBQUAD)*256);
	Bitdepth = NESTER_settings.nes.graphics.osd.fullscreen_BitDepth;
	pitch = get_width();
	m_viewable_width = get_viewable_width();
	m_viewable_height=get_viewable_height();
	m_width = get_width();
	m_height= get_height();
	m_widthdouble = m_width << 1;
	m_viewable_area_y_offset = get_viewable_area_y_offset();
	m_viewable_area_x_offset = get_viewable_area_x_offset();

	if(DirectDrawCreateEx(DeviceGUID,(void**)&lpDD, IID_IDirectDraw7,NULL)!=DD_OK){
		throw "Error initializing DirectDraw";
	}
	coope = DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE/* | DDSCL_NOWINDOWCHANGES*/;
	if(NESTER_settings.nes.graphics.osd.fullscreen_width==320)
		coope |= DDSCL_ALLOWMODEX;
	if(DD_OK!=(lpDD->SetCooperativeLevel(window_handle,coope))){
		throw "Error setting cooperative level";
	}
    // set the display mode
    if(setResolution())
    {
      throw "Error setting screen mode";
/*
      if(!checkResolution())
      {
        throw "Error setting screen mode";
      }
      if(setResolution())
      {
        throw "Error setting screen mode";
      }
*/
    }

    fullscreen_width = NESTER_settings.nes.graphics.osd.fullscreen_width;
    fullscreen_height = NESTER_settings.nes.graphics.osd.fullscreen_height; //(3*fullscreen_width)/4;

    magnification = 1;

    if(do_scaling==1)
    {
      // set magnification as high as possible
      // test until we're too large
      while(get_magnified_viewable_height() <= fullscreen_height)
      {
        magnification++;
      }
      // back off by one
      magnification--;
    }

    // Create the primary surface with several back buffers
    memset(&ddsd,0,sizeof(ddsd));
    ddsd.dwSize          = sizeof(ddsd);
	if(!NESTER_settings.nes.graphics.FullscreenModeNotUseFlip){
		ddsd.ddsCaps.dwCaps  = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
		ddsd.dwBackBufferCount = NESTER_settings.nes.graphics.FullscreenModeBackBuffM?VIDMEM_NUM_BACK_BUFFERS:SYSMEM_NUM_BACK_BUFFERS;
		ddsd.dwFlags         = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	}
	else{
		ddsd.ddsCaps.dwCaps  = DDSCAPS_PRIMARYSURFACE; // | DDSCAPS_VIDEOMEMORY;
		ddsd.dwFlags         = DDSD_CAPS;
	}

//	if(NESTER_settings.nes.graphics.osd.fullscreen_width==320)
//		ddsd.ddsCaps.dwCaps |= DDSCAPS_MODEX;


    while(1)
    {
      if(!FAILED(lpDD->CreateSurface(&ddsd,&lpddsprimary,NULL)))
      {
        break;
      }
	  if(NESTER_settings.nes.graphics.FullscreenModeNotUseFlip){
          throw "Error creating primary surface";
	  }

      // ideally, we want full back buffers in vid mem
      // if we can't get that, we try everything down to one back buffer in vid mem
      // if we can't get that, we try sys mem with 2 buffers, then one buffer

      // are we trying for vid mem?
      if(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
      {
        // cut down on the backbuffers
        ddsd.dwBackBufferCount--;
        if(ddsd.dwBackBufferCount <= 1)
        {
          // vid mem isn't cutting it
          // try 2 backbuffers in sys mem
          ddsd.dwBackBufferCount = SYSMEM_NUM_BACK_BUFFERS;

          ddsd.ddsCaps.dwCaps ^= DDSCAPS_VIDEOMEMORY;
          ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
        }
      }
      else if(ddsd.dwBackBufferCount)
      {
        // keep trying in sys mem
        // cut down on the backbuffers
        ddsd.dwBackBufferCount--;
        if(0 == ddsd.dwBackBufferCount)
        {
          throw "Error creating primary surface";
        }
      }
    }

	ZeroMemory(&ddscaps,sizeof(ddscaps));
	if(!NESTER_settings.nes.graphics.FullscreenModeNotUseFlip){
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		if(DD_OK!=(lpddsprimary->GetAttachedSurface(&ddscaps,&lpddsback))){
			throw "Error getting attached surface";
		}
	}

#if 1
	mode_2xsai = NESTER_settings.nes.graphics.FullscreenMode_2xsaiType;
	if(NESTER_settings.nes.graphics.FullscreenMode_2xsai){
		if(mode_2xsai == 3){
			Func_2xSai = New2xSaI_8bit;
			f_2xsai = 1;
			buffer_16bit = (unsigned short *)malloc((m_height+4)*m_width);
			if(Bitdepth!=8)
				buffer_16bit_bouble = (WORD *)malloc((m_width+4)*(m_height+4) * 4);
		}
		else if(Bitdepth!=8 /*&& magnification>=2*/){
			DDPIXELFORMAT ddpfPixel;
			ZeroMemory(&ddpfPixel, sizeof(ddpfPixel));
			ddpfPixel.dwSize = sizeof(ddpfPixel);
			if(lpddsprimary->GetPixelFormat(&ddpfPixel) != DD_OK)
				throw "Failed GetPixelFormat";
			buffer_16bit = (WORD *)malloc((m_width+4)*(m_height+4) * 2);
			if(Bitdepth==16){
				if(ddpfPixel.dwGBitMask == 0x7E0){
					BitGreen=1;
					Init_2xSaI(565);
				}
				else{
					Init_2xSaI(555);
				}
			}
			else{
				BitGreen=1;
				Init_2xSaI(565);
				buffer_16bit_bouble = (WORD *)malloc((m_width+4)*(m_height+4) * 2*4);
			}
			switch(NESTER_settings.nes.graphics.FullscreenMode_2xsaiType){
			case 0:
				Func_2xSai = _2xSaI;
				break;
			case 1:
				Func_2xSai = Super2xSaI;
				break;
			case 2:
				Func_2xSai = SuperEagle;
				break;
			default:
				Func_2xSai = _2xSaI;
				break;
			}
			f_2xsai = 1;
		}
	}
#endif

    // Create the back buffer for emu to draw in
    memset(&ddsd,0,sizeof(ddsd));
    ddsd.dwSize          = sizeof(ddsd);
    ddsd.dwFlags         = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    ddsd.dwWidth         = f_2xsai?get_width()<<1:get_width();
    ddsd.dwHeight        = f_2xsai?get_height()<<1:get_height();
    ddsd.ddsCaps.dwCaps  = DDSCAPS_OFFSCREENPLAIN; // | /*DDSCAPS_VIDEOMEMORY*/DDSCAPS_SYSTEMMEMORY;
	if(NESTER_settings.nes.graphics.FullscreenModeBackBuffM)
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

    if(FAILED(lpDD->CreateSurface(&ddsd,&lpddsbackbuf,NULL)))
    {
      throw "Error creating back buffer";
    }
//	lpDD->FlipToGDISurface();


    // create the rect structs
    /////////////////////////////////////////////////////////////////////////////
    // Note that RECT structures are defined so that the right and bottom members 
    // are exclusive--therefore, right - left equals the width of the rectangle, 
    // not one less than the width.
    /////////////////////////////////////////////////////////////////////////////

	if(do_scaling==2){			//nnn
		nes_screen_rect.top    = 0;
		nes_screen_rect.bottom = fullscreen_height-1; //(3 * NESTER_settings.nes.graphics.osd.fullscreen_width) / 4-1;
		nes_screen_rect.left   = 0;
		nes_screen_rect.right  = NESTER_settings.nes.graphics.osd.fullscreen_width-1;
	}
	else{
		nes_screen_rect.top    = (fullscreen_height - get_magnified_viewable_height()) / 2;
		nes_screen_rect.bottom = nes_screen_rect.top + get_magnified_viewable_height();
		nes_screen_rect.left   = (fullscreen_width - get_magnified_viewable_width()) / 2;
		nes_screen_rect.right  = nes_screen_rect.left + get_magnified_viewable_width();
	}

    back_buffer_nes_screen_rect.top = get_viewable_area_y_offset();
    back_buffer_nes_screen_rect.bottom = back_buffer_nes_screen_rect.top + get_viewable_height();
    back_buffer_nes_screen_rect.left = get_viewable_area_x_offset();
    back_buffer_nes_screen_rect.right = back_buffer_nes_screen_rect.left + get_viewable_width();

    top_rect.top      = 0;
    top_rect.bottom   = nes_screen_rect.top;
    top_rect.left     = 0;
    top_rect.right    = fullscreen_width;

    bot_rect.top      = nes_screen_rect.bottom;
    bot_rect.bottom   = fullscreen_height;
    bot_rect.left     = 0;
    bot_rect.right    = fullscreen_width;

    left_rect.top     = top_rect.bottom;
    left_rect.bottom  = bot_rect.top;
    left_rect.left    = 0;
    left_rect.right   = nes_screen_rect.left;

    right_rect.top    = top_rect.bottom;
    right_rect.bottom = bot_rect.top;
    right_rect.left   = nes_screen_rect.right;
    right_rect.right  = fullscreen_width;

    clear_margins();

	if(Bitdepth==8){
		
#ifdef NESTER_DEBUG
		// create a pink palette
		for(int i=0; i<256; i++)
		{
			ape[i].peRed   = (BYTE)0xff;
			ape[i].peGreen = (BYTE)0x9f;
			ape[i].peBlue  = (BYTE)0x9f;
			ape[i].peFlags = (BYTE)0;
		}
#else
		// create a black palette
		{
			HDC hdc = GetDC(window_handle);
			GetSystemPaletteEntries(hdc, 0, 256, ape);
			if(hdc)
				ReleaseDC(window_handle, hdc);
		}
		for(int i=10; i<0xf6; i++)  //for(int i=0; i<256; i++)
		{
			ape[i].peRed   = (BYTE)0;
			ape[i].peGreen = (BYTE)0;
			ape[i].peBlue  = (BYTE)0;
			ape[i].peFlags = (BYTE)0;
		}
#endif
		//    for(i=0; i<0xa; i++)
		/*    {
		ape[9].peRed   = (BYTE)0xe0;
		ape[9].peGreen = (BYTE)0xe0;
		ape[9].peBlue  = (BYTE)0xe0;
		
		  ape[10].peRed   = (BYTE)0x80;
		  ape[10].peGreen = (BYTE)0x80;
		  ape[10].peBlue  = (BYTE)0x80;
		  }
		*/
		// set color 0x00 to black
		ape[0].peRed   = (BYTE)0;
		ape[0].peGreen = (BYTE)0;
		ape[0].peBlue  = (BYTE)0;
		
		// set color 0xff to white
		ape[0xff].peRed   = (BYTE)0xff;
		ape[0xff].peGreen = (BYTE)0xff;
		ape[0xff].peBlue  = (BYTE)0xff;
		
		ddrval = lpDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, 
			ape, &lpDDPal, NULL);
		if(FAILED(ddrval) || (lpDDPal == NULL))
		{
			throw "Failed to create palette";
		}
		
		ddrval = lpddsprimary->SetPalette(lpDDPal);
		if(FAILED(ddrval))
		{
			throw "Failed to set palette";
		}
	}
  } catch(...) {
    if(lpDD)
    {
      lpDD->SetCooperativeLevel(window_handle, DDSCL_NORMAL);
      lpDD->RestoreDisplayMode();
	  lpDD->Release();
	  if(clipper){ clipper->Release(); clipper=NULL; }
	  if(buffer_16bit){
		  free(buffer_16bit);
		  buffer_16bit=NULL;
	  }
	  if(buffer_16bit_bouble){
		  free(buffer_16bit_bouble);
		  buffer_16bit_bouble=NULL;
	  }

//      delete lpDD;
    }
    throw;
  }
}

win32_nnnfullscreen_NES_screen_mgr::~win32_nnnfullscreen_NES_screen_mgr()
{
	if(avi_rec)
		end_avirec();
	if(buffer_16bit){
		free(buffer_16bit);
		buffer_16bit=NULL;
	}
	if(buffer_16bit_bouble){
		free(buffer_16bit_bouble);
		buffer_16bit_bouble=NULL;
	}
	if(clipper){ clipper->Release(); clipper=NULL; }
  if(lpDD)
  {
    if(dx_locked) unlock();
    lpDD->SetCooperativeLevel(window_handle, DDSCL_NORMAL);
    lpDD->RestoreDisplayMode();

    if(lpddsbackbuf)
    {
      lpddsbackbuf->Release();
    }
    if(lpddsprimary)
    {
      lpddsprimary->Release();
    }

	lpDD->Release();
//    delete ddraw;
  }
}

int win32_nnnfullscreen_NES_screen_mgr::setResolution()
{
  int width, height;
 // HRESULT result;

  width = NESTER_settings.nes.graphics.osd.fullscreen_width;
  height = NESTER_settings.nes.graphics.osd.fullscreen_height; //(3 * width) / 4;

//  LOG("w " << HEX(width) << "h " << HEX(height) << "D " << HEX(Bitdepth) << endl);

  if(NESTER_settings.nes.graphics.FullscreenModeUseDefRefreshRate){
	  if(DD_OK != lpDD->SetDisplayMode(width,height,Bitdepth,0,0))
		  return -1;
  }
  else{
	  if(DD_OK != lpDD->SetDisplayMode(width,height,Bitdepth,60,0))
	  	  if(DD_OK != lpDD->SetDisplayMode(width,height,Bitdepth,0,0))
			  return -1;
  }
  return 0;
}

static uint32 suggested_mode, s_Height;

static HRESULT WINAPI DDEnumCallback_Modes(LPDDSURFACEDESC2 lpDDSurfaceDesc,
                                           LPVOID lpContext)
{
  // check the vid mode

  // 8 bit?
//  if(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount != 8) return DDENUMRET_OK;

  // square ratio?
//  if(/*((lpDDSurfaceDesc->dwWidth * 3) / 4)*/NESTER_settings.nes.graphics.osd.fullscreen_height == lpDDSurfaceDesc->dwHeight) return DDENUMRET_OK;

  // vid mode is OK

  // is it the current mode?
  if(lpDDSurfaceDesc->dwWidth == NESTER_settings.nes.graphics.osd.fullscreen_width && lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == NESTER_settings.nes.graphics.osd.fullscreen_BitDepth)
  {
    // mode is supported; bail
    return DDENUMRET_OK;  //DDENUMRET_CANCEL;
  }

  // if this resolution is bigger than the current, and smaller than other
  // enum'd resolutions, set it
  if(lpDDSurfaceDesc->dwWidth > NESTER_settings.nes.graphics.osd.fullscreen_width)
  {
    if((suggested_mode == 0) || (suggested_mode > lpDDSurfaceDesc->dwWidth))
    {
      suggested_mode = lpDDSurfaceDesc->dwWidth;
	  s_Height = lpDDSurfaceDesc->dwHeight;
    }
  }

  return DDENUMRET_OK;
}


int win32_nnnfullscreen_NES_screen_mgr::checkResolution()
{
  suggested_mode = 0;

  lpDD->EnumDisplayModes(0, NULL, (LPVOID)NULL, DDEnumCallback_Modes);
  if(suggested_mode == 0)
  {
    return 0;
  }
  NESTER_settings.nes.graphics.osd.fullscreen_width = suggested_mode;
  NESTER_settings.nes.graphics.osd.fullscreen_height = s_Height;
  return -1;
}


boolean win32_nnnfullscreen_NES_screen_mgr::lock(pixmap& p)
{
#if 0
  if(dx_locked) throw "Error: surface already locked";

  // set up the surface description to lock the surface
  memset(&ddsd,0,sizeof(ddsd)); 
  ddsd.dwSize  = sizeof(ddsd);

  // lock the backbuffer surface
  while(1)
  {
    ddrval = lpddsbackbuf->Lock(NULL, &ddsd,
      0/*DDLOCK_NOSYSLOCK breaks NT for some stupid reason*/, NULL);
    if(!FAILED(ddrval))
    {
      break;
    }
    if(ddrval == DDERR_SURFACELOST)
    {
      ddrval = lpddsbackbuf->Restore();
      if(FAILED(ddrval))
      {
        break;
      }
      continue; // this fixes a bug where while() was
                // terminating before lock() was called
    }
    if(ddrval != DDERR_WASSTILLDRAWING)
    {
      break;
    }
  }
  if(FAILED(ddrval))
  {
    return FALSE;
  }

  buffer = (PIXEL*)ddsd.lpSurface;

  pitch  = ddsd.lPitch;

  dx_locked = 1;

  p.width  = get_width();
  p.height = get_height();
  p.pitch  = pitch;
  p.data   = buffer;
#else
//  pitch  = get_width();
#if 0
  if(Bitdepth==8){
	  if(dx_locked) throw "Error: surface already locked";
	  
	  // set up the surface description to lock the surface
	  memset(&ddsd,0,sizeof(ddsd)); 
	  ddsd.dwSize  = sizeof(ddsd);
	  
	  // lock the backbuffer surface
	  while(1)
	  {
		  ddrval = lpddsbackbuf->Lock(NULL, &ddsd,
			  0/*DDLOCK_NOSYSLOCK breaks NT for some stupid reason*/, NULL);
		  if(!FAILED(ddrval))
		  {
			  break;
		  }
		  if(ddrval == DDERR_SURFACELOST)
		  {
			  ddrval = lpddsbackbuf->Restore();
			  if(FAILED(ddrval))
			  {
				  break;
			  }
			  continue; // this fixes a bug where while() was
			  // terminating before lock() was called
		  }
		  if(ddrval != DDERR_WASSTILLDRAWING)
		  {
			  break;
		  }
	  }
	  if(FAILED(ddrval))
	  {
		  return FALSE;
	  }
	  
	  
	  pitch  = ddsd.lPitch;

	  dx_locked = 1;

	  p.width  = get_width();
	  p.height = get_height();
	  p.pitch  = pitch;
	  p.data   = (unsigned char *)ddsd.lpSurface;
	  return TRUE;
  }
#endif 
  p.width  = get_width();
  p.height = get_height();
  p.pitch  = pitch;

  if(f_2xsai && mode_2xsai == 3){
	  p.data   = (uint8 *)buffer_16bit + (m_width);
  }
  else
	  p.data   = buffer;


#endif
  return TRUE;
}

boolean win32_nnnfullscreen_NES_screen_mgr::unlock()
{
#if 1
/*
	if(Bitdepth==8 && dx_locked){
		lpddsbackbuf->Unlock(NULL);
		dx_locked = 0;
	}
*/
	return TRUE;
#else
  if(dx_locked) throw "Error: surface already locked";

  // set up the surface description to lock the surface
  memset(&ddsd,0,sizeof(ddsd)); 
  ddsd.dwSize  = sizeof(ddsd);

  // lock the backbuffer surface
  while(1)
  {
    ddrval = lpddsbackbuf->Lock(NULL, &ddsd,
      0/*DDLOCK_NOSYSLOCK breaks NT for some stupid reason*/, NULL);
    if(!FAILED(ddrval))
    {
      break;
    }
    if(ddrval == DDERR_SURFACELOST)
    {
      ddrval = lpddsbackbuf->Restore();
      if(FAILED(ddrval))
      {
        break;
      }
      continue; // this fixes a bug where while() was
                // terminating before lock() was called
    }
    if(ddrval != DDERR_WASSTILLDRAWING)
    {
      break;
    }
  }
  if(FAILED(ddrval))
  {
    return FALSE;
  }

  buffer = (PIXEL*)ddsd.lpSurface;
//  pitch  = ddsd.lPitch;
  int w = get_width();
  int h = get_height();
  memcpy(buffer, graphbuff, w*h);
  lpddsbackbuf->Unlock(NULL);
//  p.pitch  = pitch;
#endif

  return TRUE;
}

void win32_nnnfullscreen_NES_screen_mgr::blt()
{
	if(MenuFlag)
	{
//		if(clipper){ clipper->Release(); clipper=NULL; }
//		if(lpDD->CreateClipper(0,&clipper,NULL)!=DD_OK)
//			return;
//		if(clipper->SetHWnd(0,window_handle)!=DD_OK)
//			return;
//		if(lpddsprimary->SetClipper(clipper)!=DD_OK)
//			return;
//		lpDD->FlipToGDISurface();
//	  lpddsprimary->Blt(NULL /*&nes_screen_rect*/, lpddsbackbuf, NULL, DDBLT_WAIT /*| DDBLT_ASYNC*/, NULL);
	  return;
  }
	if(avi_rec)
		frame_avirec();
  if(f_2xsai){
	  if(mode_2xsai == 3){
//		  Func_2xSai((unsigned char *)&buffer_16bit_bouble[m_widthdouble], m_widthdouble, NULL, (unsigned char *)buffer, m_widthdouble, m_width, m_height);
//		  ddrawfullscreenblt_8bit(&ddsd, (DWORD *)Palette, buffer/*[get_viewable_area_y_offset()*272]*/, get_height());
//		  ddrawblt_8bit(&ddsd, (DWORD *)&bmInfo->bmiPalette, &buffer[m_viewable_area_x_offset + m_viewable_area_y_offset*m_widthdouble], m_viewable_width<<1, m_viewable_height<<1, m_widthdouble);
	  }
	  else{
		  if(BitGreen)
			  convto16_565bitpalette((DWORD *)Palette, palette_16bit);
		  else
			  convto16_555bitpalette((DWORD *)Palette, palette_16bit);
		  scrconv8_16for2xsai(&buffer[m_viewable_area_y_offset*m_width], (unsigned char *)&buffer_16bit[m_viewable_width*2], palette_16bit,
			  m_viewable_width, m_viewable_height, m_width, m_viewable_width<<1);
		  switch(Bitdepth){
		  case 16:
			  //		  _2xSaI((unsigned char *)&buffer_16bit[(m_viewable_width*2)], (m_viewable_width*2), NULL, (unsigned char *)ddsd.lpSurface, ddsd.lPitch, m_viewable_width, m_viewable_height);
			  break;
		  case 24:
			  Func_2xSai((unsigned char *)&buffer_16bit[(m_viewable_width*2)], (m_viewable_width*2), NULL, (unsigned char *)buffer_16bit_bouble, m_viewable_width<<2, m_viewable_width, m_viewable_height);
			  //		  scrconv16_24((unsigned char *)buffer_16bit_bouble, (unsigned char *)ddsd.lpSurface, m_viewable_width<<1, m_viewable_height<<1, m_viewable_width<<2, ddsd.lPitch);
			  break;
		  case 32:
			  Func_2xSai((unsigned char *)&buffer_16bit[(m_viewable_width*2)], (m_viewable_width*2), NULL, (unsigned char *)buffer_16bit_bouble, m_viewable_width<<2, m_viewable_width, m_viewable_height);
			  //		  scrconv16_32((unsigned char *)buffer_16bit_bouble, (unsigned char *)ddsd.lpSurface, m_viewable_width<<1, m_viewable_height<<1, m_viewable_width<<2, ddsd.lPitch);
			  break;
		  default:
			  break;
		  }
	  }
  }
//  if(Bitdepth!=8)
  {
  // lock the backbuffer surface
  while(1)
  {
    ddrval = lpddsbackbuf->Lock(NULL, &ddsd, 0/*DDLOCK_NOSYSLOCK breaks NT for some stupid reason*/, NULL);
    if(!FAILED(ddrval)){
		break;
    }
    if(ddrval == DDERR_SURFACELOST)
    {
      ddrval = lpddsbackbuf->Restore();
      if(FAILED(ddrval)){
		  break;
      }
      continue; // this fixes a bug where while() was
                // terminating before lock() was called
    }
    if(ddrval != DDERR_WASSTILLDRAWING){
		break;
    }
  }
  if(FAILED(ddrval)){
	  return;
  }

#if 1
  if(f_2xsai){
	  if(mode_2xsai == 3){
		  if(Bitdepth==8){
			  Func_2xSai(((unsigned char *)buffer_16bit + ((1+m_viewable_area_y_offset)*m_width)) +(m_viewable_area_x_offset), m_width, NULL, (unsigned char *)ddsd.lpSurface, ddsd.lPitch, m_viewable_width, m_viewable_height);
		  }
		  else{
			  Func_2xSai(((unsigned char *)buffer_16bit + ((1+m_viewable_area_y_offset)*m_width)) +(m_viewable_area_x_offset), m_width, NULL, (unsigned char *)buffer_16bit_bouble, m_width*2, m_viewable_width, m_viewable_height);
			  ddrawblt_8bit2(&ddsd, (DWORD *)Palette, (unsigned char *)buffer_16bit_bouble, m_width*2, m_height*2, m_width*2);
//			  ddrawfullscreenblt_8bit(&ddsd, (DWORD *)Palette, buffer_16bit_bouble, get_height());
		  }
//		  ddrawfullscreenblt_8bit(&ddsd, (DWORD *)Palette, buffer/*[get_viewable_area_y_offset()*272]*/, get_height());
	  }
	  else{
		  switch(Bitdepth){
		  case 16:
			  Func_2xSai((unsigned char *)&buffer_16bit[m_viewable_width*2], (m_viewable_width*2), NULL, (unsigned char *)ddsd.lpSurface, ddsd.lPitch, m_viewable_width, m_viewable_height);
			  //		  LOG("_2xcsai b" << HEX(Bitdepth)<< endl);
			  //		  LOG("_2xcsai b" << endl);
			  break;
		  case 24:
			  //		  _2xSaI((unsigned char *)&buffer_16bit[(m_viewable_width*2)], (m_viewable_width*2), NULL, (unsigned char *)buffer_16bit_bouble, m_viewable_width<<2, m_viewable_width, m_viewable_height);
			  scrconv16_24((unsigned char *)buffer_16bit_bouble, (unsigned char *)ddsd.lpSurface, m_viewable_width<<1, m_viewable_height<<1, m_viewable_width<<2, ddsd.lPitch);
			  break;
		  case 32:
			  //		  _2xSaI((unsigned char *)&buffer_16bit[(m_viewable_width*2)], (m_viewable_width*2), NULL, (unsigned char *)buffer_16bit_bouble, m_viewable_width<<2, m_viewable_width, m_viewable_height);
			  scrconv16_32((unsigned char *)buffer_16bit_bouble, (unsigned char *)ddsd.lpSurface, m_viewable_width<<1, m_viewable_height<<1, m_viewable_width<<2, ddsd.lPitch);
			  break;
		  default:
			  break;
		  }
	  }
  }
  else{
	  ddrawfullscreenblt_8bit(&ddsd, (DWORD *)Palette, buffer/*[get_viewable_area_y_offset()*272]*/, get_height());
  }
#else
  ddrawfullscreenblt_8bit(&ddsd, (DWORD *)Palette, buffer/*[get_viewable_area_y_offset()*272]*/, get_height());
#endif
  lpddsbackbuf->Unlock(NULL);
  }


  if(f_2xsai){
//	  if(mode_2xsai == 3){
//	  }
//	  else
	  {
		  RECT sr;
		  SetRect(&sr, 0, 0, m_viewable_width<<1, m_viewable_height<<1);
		  lpddsback->Blt(&nes_screen_rect, lpddsbackbuf, &sr, DDBLT_WAIT /*|DDBLT_ASYNC */, NULL);
	  }
  }
  else if(!NESTER_settings.nes.graphics.FullscreenModeNotUseFlip){
	  lpddsback->Blt(&nes_screen_rect, lpddsbackbuf, &back_buffer_nes_screen_rect, DDBLT_WAIT /*| DDBLT_ASYNC*/, NULL);
  }
  else{
	  clear_margins();
	  lpddsprimary->Blt(&nes_screen_rect, lpddsbackbuf, &back_buffer_nes_screen_rect, DDBLT_WAIT, NULL);
	  return;
  }

  if( NESTER_settings.nes.graphics.EmulateTVScanline && !NESTER_settings.nes.graphics.FullscreenModeNotUseFlip) //&& nes_screen_rect.right - nes_screen_rect.left == 512)
  {
    memset( &ddsd, 0, sizeof(ddsd) ); 
    ddsd.dwSize  = sizeof(ddsd);

    if( SUCCEEDED( lpddsback->Lock( NULL, &ddsd, DDLOCK_WRITEONLY, NULL ) ) )
    {
      const DWORD width = ddsd.dwWidth*(Bitdepth/8);
      const DWORD pitch_twoline = ddsd.lPitch << 1;
      DWORD y_count = ddsd.dwHeight >> 1;
      BYTE *p;
      p = (BYTE*)(ddsd.lpSurface);
      while( --y_count )
      {
        memset( p, 0x00, width );
        p += pitch_twoline;
      }
      lpddsback->Unlock(NULL);// ddsd.lpSurface 
    }
  }
  if(g_ScreenMsgStr[0]){
	  HDC hdc;
	  HFONT hFont, hOldFont;
	  HRESULT hr;
	  RECT sr=nes_screen_rect;
	  sr.left+=10, sr.top+= 10;
	  if(NESTER_settings.nes.graphics.FullscreenModeNotUseFlip)
		  hr = lpddsprimary->GetDC(&hdc);
	  else
		  hr = lpddsback->GetDC(&hdc);
	  if(FAILED(hr))
		  return;
	  hFont = CreateFontIndirect(&Default_Font);
	  SetBkMode(hdc, 0);
	  SetTextColor(hdc, RGB(255, 255, 255));
	  hOldFont = (HFONT)SelectObject(hdc, hFont);
	  DrawText(hdc /*screenDC*/, g_ScreenMsgStr, -1, &sr, DT_WORDBREAK);
	  SelectObject(hdc, hOldFont);
	  if(NESTER_settings.nes.graphics.FullscreenModeNotUseFlip)
		  lpddsprimary->ReleaseDC(hdc);
	  else
		  lpddsback->ReleaseDC(hdc);
	  DeleteObject(hFont);
  }
}


void win32_nnnfullscreen_NES_screen_mgr::flip()
{
  if(MenuFlag!=0){
	  return;
  }
  if(NESTER_settings.nes.graphics.FullscreenModeNotUseFlip){
	  return;
  }
  // Flip the surfaces
  while(1)
  {
    ddrval = lpddsprimary->Flip(NULL, DDFLIP_WAIT /*| DDFLIP_NOVSYNC*/);
    if(!FAILED(ddrval))
    {
      break;
    }
    if(ddrval == DDERR_SURFACELOST)
    {
      ddrval = lpddsprimary->Restore();
      if(FAILED(ddrval))
      {
        return;
      }
      continue;
    }
    if(ddrval != DDERR_WASSTILLDRAWING)
    {
      break;
    }
  }

  clear_margins();
}

void win32_nnnfullscreen_NES_screen_mgr::clear(PIXEL color)
{

  if(NESTER_settings.nes.graphics.FullscreenModeNotUseFlip){
	  return;
  }

  DDBLTFX ddbltfx;

  memset(&ddbltfx, 0x00, sizeof(ddbltfx));
  ddbltfx.dwSize = sizeof(ddbltfx);
  ddbltfx.dwFillColor = color;

  lpddsback->Blt(&nes_screen_rect, NULL, NULL, DDBLT_COLORFILL | /*DDBLT_ASYNC*/DDBLT_WAIT, &ddbltfx);
}

boolean win32_nnnfullscreen_NES_screen_mgr::set_palette(const uint8 pal[256][3])
{
  PALETTEENTRY  pe[256];

  if(Bitdepth == 8){
	  for(int i = 0; i < 256; i++)
	  {
		  pe[i].peRed   = (BYTE)pal[i][0];
		  pe[i].peGreen = (BYTE)pal[i][1];
		  pe[i].peBlue  = (BYTE)pal[i][2];
		  pe[i].peFlags = (BYTE)0;
	  }
	  
	  lpDDPal->SetEntries(0, 0, 256, pe);
  }
  else{
	  for(int i=0; i<256; i++){
		  Palette[i].rgbRed   = (BYTE)pal[i][0];
		  Palette[i].rgbGreen = (BYTE)pal[i][1];
		  Palette[i].rgbBlue  = (BYTE)pal[i][2];
		  Palette[i].rgbReserved=0;
	  }
  }

  return TRUE;
}

boolean win32_nnnfullscreen_NES_screen_mgr::get_palette(uint8 pal[256][3])
{
  if(Bitdepth == 8){
	  PALETTEENTRY  pe[256];
	  lpDDPal->GetEntries(0, 0, 256, pe);
	  for(int i = 0; i < 256; i++)
	  {
		  pal[i][0] = pe[i].peRed;
		  pal[i][1] = pe[i].peGreen;
		  pal[i][2] = pe[i].peBlue;
	  }
  }
  else{
	  for(int i=0; i<256; i++){
		pal[i][0]=	Palette[i].rgbRed;
		pal[i][1]=	Palette[i].rgbGreen;
		pal[i][2]=	Palette[i].rgbBlue;
	  }
  }
  return TRUE;
}

boolean win32_nnnfullscreen_NES_screen_mgr::set_palette_section(uint8 start, uint8 len, const uint8 pal[][3])
{
  if(Bitdepth == 8){
	  PALETTEENTRY  pe[256];
	  for(int i = 0; i < len; i++)
	  {
		  pe[i].peRed   = (BYTE)pal[i][0];
		  pe[i].peGreen = (BYTE)pal[i][1];
		  pe[i].peBlue  = (BYTE)pal[i][2];
		  pe[i].peFlags = (BYTE)0;
	  }
	  lpDDPal->SetEntries(0, start, len, pe);
  }
  else{
	  for(int i=0, n = start; i<len; i++, n++){
		  Palette[n].rgbRed   = (BYTE)pal[i][0];
		  Palette[n].rgbGreen = (BYTE)pal[i][1];
		  Palette[n].rgbBlue  = (BYTE)pal[i][2];
		  Palette[n].rgbReserved=0;
	  }
  }
  return TRUE;
}

boolean win32_nnnfullscreen_NES_screen_mgr::get_palette_section(uint8 start, uint8 len, uint8 pal[][3])
{

  if(Bitdepth == 8){
	  PALETTEENTRY  pe[256];
	  lpDDPal->GetEntries(0, start, len, pe);
	  for(int i = 0; i < len; i++)
	  {
		  pal[i][0] = pe[i].peRed;
		  pal[i][1] = pe[i].peGreen;
		  pal[i][2] = pe[i].peBlue;
	  }
  }
  else{
	  for(int i=0, n=start; i<len; i++, n++){
		pal[i][0]=	Palette[n].rgbRed;
		pal[i][1]=	Palette[n].rgbGreen;
		pal[i][2]=	Palette[n].rgbBlue;
	  }
  }
  return TRUE;
}

void win32_nnnfullscreen_NES_screen_mgr::assert_palette()
{
	set_NES_palette();
}

void win32_nnnfullscreen_NES_screen_mgr::clear_margins()
{
  DDBLTFX ddbltfx;

  memset(&ddbltfx, 0x00, sizeof(ddbltfx));
  ddbltfx.dwSize = sizeof(ddbltfx);

  if(parent_NES && NESTER_settings.nes.graphics.draw_overscan)
  {
    ddbltfx.dwFillColor = parent_NES->getBGColor();
  } else {
    ddbltfx.dwFillColor = 0x00;
  }

  if(NESTER_settings.nes.graphics.FullscreenModeNotUseFlip){
	  lpddsprimary->Blt(&top_rect,   NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	  lpddsprimary->Blt(&left_rect,  NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	  lpddsprimary->Blt(&right_rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	  lpddsprimary->Blt(&bot_rect,   NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
//	  lpddsprimary->Blt(NULL, NULL, NULL, DDBLT_WAIT, NULL);
	  return;
  }

  lpddsback->Blt(&top_rect,   NULL, NULL, DDBLT_COLORFILL | /*DDBLT_ASYNC*/DDBLT_WAIT, &ddbltfx);
  lpddsback->Blt(&left_rect,  NULL, NULL, DDBLT_COLORFILL | /*DDBLT_ASYNC*/DDBLT_WAIT, &ddbltfx);
  lpddsback->Blt(&right_rect, NULL, NULL, DDBLT_COLORFILL | /*DDBLT_ASYNC*/DDBLT_WAIT, &ddbltfx);
  lpddsback->Blt(&bot_rect,   NULL, NULL, DDBLT_COLORFILL | /*DDBLT_ASYNC*/DDBLT_WAIT, &ddbltfx);
}


void win32_nnnfullscreen_NES_screen_mgr::shot_screen( char *szFileName )
{
  const DWORD x_offset = get_viewable_area_x_offset();
  const DWORD y_offset = get_viewable_area_y_offset();
  int w, h;
  PALETTEENTRY pal[256];
  unsigned char *sdt = NULL, *psrc;
  w = get_viewable_width();
  h = get_viewable_height();
  sdt = (unsigned char *)malloc(w*h);
  if(sdt == NULL)
	  return;
  if(Bitdepth==8){
	  lpDDPal->GetEntries( 0, 0, 256, pal );
  }
  else{
	  for(int i=0;i<0x100;i++){
		  pal[i].peBlue	=Palette[i].rgbBlue;
		  pal[i].peGreen=Palette[i].rgbGreen;
		  pal[i].peRed	=Palette[i].rgbRed;
		  pal[i].peFlags=0;
	  }
  }
  if(f_2xsai && mode_2xsai == 3){
	  psrc = (uint8 *)buffer_16bit + (m_width);
  }
  else
	  psrc = buffer;
  for(int i=0,j=0,k=y_offset*pitch+x_offset; i<h; i++, j+=w, k+=pitch){
	  memcpy(&sdt[j], &psrc[k], w);
  }
  screenshot_save_8bit(szFileName, w, h, pal, sdt);
  free(sdt);
}

void win32_nnnfullscreen_NES_screen_mgr::nnnManuEnable(unsigned char flag){
	MenuFlag=flag;
	if(flag && !NESTER_settings.nes.graphics.FullscreenModeNotUseFlip){
//		lpddsprimary->SetPalette(NULL);
/*
		if(clipper){ clipper->Release(); clipper=NULL; }
		if(lpDD->CreateClipper(0,&clipper,NULL)!=DD_OK)
			return;
		if(clipper->SetHWnd(0,window_handle)!=DD_OK)
			return;
		if(lpddsprimary->SetClipper(clipper)!=DD_OK)
			return;
*/
		lpDD->FlipToGDISurface();
	}
/*
	else{
		if(clipper){ clipper->Release(); clipper=NULL; }
//		ddrval = lpddsprimary->SetPalette(lpDDPal);
	}
*/
}

void win32_nnnfullscreen_NES_screen_mgr::EndEmu(){
//	MenuFlag=0;
//	for(int i=0; i<3; i++)
	{
//		lpddsback->Blt(&nes_screen_rect, NULL, NULL, DDBLT_ASYNC/*DDBLT_WAIT*/, NULL);
//		clear(0);
//		flip();
	}
	{
		HBRUSH hBrush, hOldBrush;
		HDC hdc = GetDC(window_handle);
		hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
		hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
		Rectangle(hdc, 0, 0, NESTER_settings.nes.graphics.osd.fullscreen_width-1, NESTER_settings.nes.graphics.osd.fullscreen_height-1);//fullscreen_width*3/4);
		SelectObject(hdc, hOldBrush);
		ReleaseDC(window_handle, hdc);
	}
	if(MenuFlag)
		nnnManuEnable(1);
	InvalidateRect(window_handle,NULL,FALSE);
}


void win32_nnnfullscreen_NES_screen_mgr::StartEmu(){
}


int win32_nnnfullscreen_NES_screen_mgr::start_avirec(char *fn){
	avi_rec = recavifilestart(fn, get_viewable_width(), get_viewable_height());
	return avi_rec;
}


void win32_nnnfullscreen_NES_screen_mgr::frame_avirec(){
  avi_counter^=1;
  if(!avi_counter){
	  return;
  }
  const DWORD x_offset = get_viewable_area_x_offset();
  const DWORD y_offset = get_viewable_area_y_offset();
  const DWORD pitch = get_width() << 1;
  int w, h, i,j,k;
  unsigned char *sdt = NULL, *psrc;
  w = get_viewable_width();
  h = get_viewable_height();
  sdt = (unsigned char *)malloc(w*h);
  if(sdt == NULL)
	  return;
  if(f_2xsai && mode_2xsai == 3){
	  psrc = (uint8 *)buffer_16bit + (m_width);
  }
  else
	  psrc = buffer;
  for(i=0,j=0,k=(y_offset+h-1)*pitch+x_offset; i<h; i++, j+=w, k-=pitch){
	  memcpy(&sdt[j], &psrc[k], w);
  }
  recavifilewriteflame(sdt);
  free(sdt);
}


void win32_nnnfullscreen_NES_screen_mgr::end_avirec(){
	if(avi_rec)
		recavifilerelease();
}

