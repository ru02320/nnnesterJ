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

#include <stdlib.h>

#include "nnnwindowmode.h"
#include "debug.h"

#include "screenshot.h"
#include "recavi.h"
#include "savecfg.h"
#include "ddblt.h"

/*
extern char usedispfpsf;

extern unsigned char nnnKailleraRecvLag;
extern unsigned char nnnKailleraSendLag;
extern unsigned char nnnKailleraLagDisp;
*/
extern int Mainwindowsbhs;
extern HWND Mainwindowsbh;
int DDColorMatch(IDirectDrawSurface7 * pdds, COLORREF rgb);
extern int g_PreviewMode;
extern struct Preview_state g_Preview_State;
extern RECT g_cscreen_rect;

int ddrawblt_8bit(DDSURFACEDESC2 *ddsd, DWORD *palette, unsigned char *bmpd, DWORD width, DWORD height, DWORD dPitch);



win32_overlaywindowed_NES_screen_mgr::win32_overlaywindowed_NES_screen_mgr(HWND wnd_handle)
{
  int i;
  window_handle = wnd_handle;

  blown_up = 0;

  oldPal = NULL;
  oldBmp = NULL;
  hBmp = NULL;
  buffer = NULL;

  palHan = NULL;
  bmpDC  = NULL;
  screenDC = NULL;
  bmInfo   = NULL;
  palInfo  = NULL;
  lpDD = NULL;
  lpDDSOverlay = NULL;
  lpDDSPrimary = NULL;
  lpDDSBack = NULL;
  lpDDSMsg = NULL;
  clipper=NULL;
  avi_rec = 0;
  avi_counter=0;
  buffer_16bit = NULL;
  BitDepth=0;
  BitGreen=0;
  f_2xsai = 0;
  buffer_16bit_bouble=NULL;

  try {

	f_2xsai = NESTER_settings.nes.graphics.WindowMode_2xsai;
	mode_2xsai = NESTER_settings.nes.graphics.WindowMode_2xsaiType;

	if(DD_OK != DirectDrawCreateEx(NULL, (VOID**)&lpDD, IID_IDirectDraw7, NULL))
		throw "Failed Direct Draw create";
	if(DD_OK != lpDD->SetCooperativeLevel(window_handle, DDSCL_NORMAL))
		throw "Failed SetCooperativeLevel";
	if(!DDPrimaryInit())
		throw "Failed DDPrimaryInit";
	if(NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay){
		if(CheckOverlaySupport() == FALSE){
			LOG("not support overlay" << endl);
			NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay=0;
//			throw "not support overlay";
		}
		if(!DDOverlayInit()){
			LOG("Failed DDOverlayInit" << endl);
			NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay=0;
//			throw "Failed DDOverlayInit";
		}
	}
	if(!NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay){
		if(!create_backsurface())
			throw "Failed DDBackInit";
	}

	if(lpDD->CreateClipper(0,&clipper,NULL)!=DD_OK)
		throw "Failed CreateClipper";
	if(clipper->SetHWnd(0,window_handle)!=DD_OK)
		throw "Failed clipper->SetHWnd";
	if(lpDDSPrimary->SetClipper(clipper)!=DD_OK)
		throw "Failed SetClipper";

    bmInfo = new bitmapInfo;
    if(bmInfo == NULL)
      throw "Out of memory in win32_overlaywindowed_NES_screen_mgr::win32_overlaywindowed_NES_screen_mgr";

    palInfo = new logPalette;
    if(palInfo == NULL)
      throw "Out of memory in win32_overlaywindowed_NES_screen_mgr::win32_overlaywindowed_NES_screen_mgr";

    memset(&bmInfo->bmiHeader, 0x00, sizeof(bmInfo->bmiHeader));
    bmInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmInfo->bmiHeader.biWidth = get_width()<<1; // double size 
    bmInfo->bmiHeader.biHeight = -(abs(get_height())<<1); // top-down bitmap 
    bmInfo->bmiHeader.biPlanes = 1;
    bmInfo->bmiHeader.biBitCount = 8;
    bmInfo->bmiHeader.biCompression = BI_RGB;
    bmInfo->bmiHeader.biSizeImage = NULL;
    bmInfo->bmiHeader.biXPelsPerMeter = NULL;
    bmInfo->bmiHeader.biYPelsPerMeter = NULL;
    bmInfo->bmiHeader.biClrUsed = 256;
    bmInfo->bmiHeader.biClrImportant = 256;

    palInfo->palVersion = 0x300;
    palInfo->palNumEntries = 256;

#ifdef NESTER_DEBUG
    // set up a pink palette
    for(i = 0; i < 256; i++)
    {
      palInfo->palPalEntry[i].peRed   = (BYTE)0xff;
      palInfo->palPalEntry[i].peGreen = (BYTE)0x9f;
      palInfo->palPalEntry[i].peBlue  = (BYTE)0x9f;
      palInfo->palPalEntry[i].peFlags = (BYTE)PC_NOCOLLAPSE;// | PC_RESERVED;

      bmInfo->bmiPalette[i].rgbRed   = palInfo->palPalEntry[i].peRed;
      bmInfo->bmiPalette[i].rgbGreen = palInfo->palPalEntry[i].peGreen;
      bmInfo->bmiPalette[i].rgbBlue  = palInfo->palPalEntry[i].peBlue;
      bmInfo->bmiPalette[i].rgbReserved = 0;
    }

    // set color 0 to black
    palInfo->palPalEntry[0].peRed   = (BYTE)0;
    palInfo->palPalEntry[0].peGreen = (BYTE)0;
    palInfo->palPalEntry[0].peBlue  = (BYTE)0;
    bmInfo->bmiPalette[0].rgbRed   = palInfo->palPalEntry[0].peRed;
    bmInfo->bmiPalette[0].rgbGreen = palInfo->palPalEntry[0].peGreen;
    bmInfo->bmiPalette[0].rgbBlue  = palInfo->palPalEntry[0].peBlue;

#else
    // set up a black palette
    for(i = 0; i < 256; i++)
    {
      palInfo->palPalEntry[i].peRed   = (BYTE)0;
      palInfo->palPalEntry[i].peGreen = (BYTE)0;
      palInfo->palPalEntry[i].peBlue  = (BYTE)0;
      palInfo->palPalEntry[i].peFlags = (BYTE)PC_NOCOLLAPSE;// | PC_RESERVED;

      bmInfo->bmiPalette[i].rgbRed   = palInfo->palPalEntry[i].peRed;
      bmInfo->bmiPalette[i].rgbGreen = palInfo->palPalEntry[i].peGreen;
      bmInfo->bmiPalette[i].rgbBlue  = palInfo->palPalEntry[i].peBlue;
      bmInfo->bmiPalette[i].rgbReserved = 0;
    }
#endif

    // set color 0xff to white
    palInfo->palPalEntry[0xff].peRed   = (BYTE)0xff;
    palInfo->palPalEntry[0xff].peGreen = (BYTE)0xff;
    palInfo->palPalEntry[0xff].peBlue  = (BYTE)0xff;
    bmInfo->bmiPalette[0xff].rgbRed   = palInfo->palPalEntry[0xff].peRed;
    bmInfo->bmiPalette[0xff].rgbGreen = palInfo->palPalEntry[0xff].peGreen;
    bmInfo->bmiPalette[0xff].rgbBlue  = palInfo->palPalEntry[0xff].peBlue;

    for(i=0;i<10;i++)
    {
      palInfo->palPalEntry[i].peRed = i;
      palInfo->palPalEntry[i+246].peRed = i+246;
      palInfo->palPalEntry[i].peGreen = palInfo->palPalEntry[i].peBlue =
      palInfo->palPalEntry[i+246].peGreen = palInfo->palPalEntry[i+246].peBlue = 0;
      palInfo->palPalEntry[i].peFlags = palInfo->palPalEntry[i+246].peFlags = PC_EXPLICIT;
    }

    screenDC = GetDC(window_handle);
    if(screenDC == NULL)
      throw "GetDC failed in win32_overlaywindowed_NES_screen_mgr::win32_overlaywindowed_NES_screen_mgr";

    palHan = CreatePalette((LOGPALETTE*)palInfo);
    if(!palHan)
      throw("CreatePalette failed in win32_overlaywindowed_NES_screen_mgr::win32_overlaywindowed_NES_screen_mgr");

    if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
      throw("SelectPalette failed in win32_overlaywindowed_NES_screen_mgr::win32_overlaywindowed_NES_screen_mgr");

    if(RealizePalette(screenDC) == GDI_ERROR)
      throw("RealizePalette failed in win32_overlaywindowed_NES_screen_mgr::win32_overlaywindowed_NES_screen_mgr");

    DeleteObject(palHan);
    palHan = NULL;

    hBmp = CreateDIBSection(screenDC, (BITMAPINFO*)bmInfo,
                             DIB_RGB_COLORS,
                             (void**)&buffer, NULL, NULL);
    if(!hBmp)
      throw("CreateDIBSection failed in win32_overlaywindowed_NES_screen_mgr::win32_overlaywindowed_NES_screen_mgr");

    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

	if(NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay)
		{
		RECT dest;
		DDCAPS          capsDrv;
		unsigned int    uStretchFactor1000;
		unsigned int    uDestSizeAlign, uSrcSizeAlign;
		POINT pt;
		GetClientRect(window_handle, &dest);
		pt.x=dest.left;
		pt.y=dest.top;
		ClientToScreen(window_handle, &pt);
		ZeroMemory(&ovfx, sizeof(ovfx));
		ovfx.dwSize=sizeof(ovfx);
		ZeroMemory(&capsDrv, sizeof(capsDrv));
		capsDrv.dwSize=sizeof(capsDrv);
		lpDD->GetCaps(&capsDrv, NULL);
		uStretchFactor1000 = capsDrv.dwMinOverlayStretch>1000 ? capsDrv.dwMinOverlayStretch : 1000;
		uDestSizeAlign = capsDrv.dwAlignSizeDest;
		uSrcSizeAlign =  capsDrv.dwAlignSizeSrc;
		GetClientRect(window_handle, &dest);
		//		rs.left=0,rs.top=0, rs.bottom=dest.bottom-dest.top, rs.right=dest.right-dest.left;
		rs.left=0,rs.top=0, rs.bottom=get_viewable_height(), rs.right=get_viewable_width();
		rd.left=pt.x,rd.top=pt.y, rd.bottom=rs.bottom+pt.y, rd.right=rs.right+pt.x;
		
		/*    if(capsDrv.dwCaps & DDCAPS_ALIGNSIZESRC && uSrcSizeAlign)
        rs.right -= rs.right % uSrcSizeAlign;
		rd.right  = (rs.right*uStretchFactor1000+999)/1000; // adding 999 takes care of integer truncation problems.
		rd.bottom = rs.bottom*uStretchFactor1000/1000;*/
		g_dwXRatio = (rd.right - rd.left) * 1000 / (rs.right - rs.left);
		g_dwYRatio = (rd.bottom - rd.top) * 1000 / (rs.bottom - rs.top);
		ovfx.dckSrcColorkey.dwColorSpaceLowValue=0;
		ovfx.dckSrcColorkey.dwColorSpaceHighValue=0;
		dwUpdateFlags = DDOVER_SHOW;// | DDOVER_DDFX;
		if(capsDrv.dwCKeyCaps & DDCKEYCAPS_SRCOVERLAY)
			dwUpdateFlags |= DDOVER_KEYSRCOVERRIDE;

		if (capsDrv.dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY){
			ovfx.dckDestColorkey.dwColorSpaceLowValue = DDColorMatch(lpDDSPrimary,RGB(0, 0, 0));
			ovfx.dckDestColorkey.dwColorSpaceHighValue = DDColorMatch(lpDDSPrimary,RGB(0, 0, 0));
			dwUpdateFlags |= DDOVER_DDFX | DDOVER_KEYDESTOVERRIDE;
		}

		CheckBoundries();
		lpDDSOverlay->Blt(&rs, NULL, &rd, DDBLT_ASYNC, NULL);
		if(DD_OK!=(lpDDSOverlay->UpdateOverlay(&rs, lpDDSPrimary, &rd, dwUpdateFlags, &ovfx)))
			throw "Failed DDOverlayUpdate";
		lpDDSOverlay->UpdateOverlay(NULL, lpDDSPrimary, NULL, DDOVER_HIDE, NULL);
		}
	m_viewable_width = get_viewable_width();
	m_viewable_height=get_viewable_height();
	m_width = get_width();
	m_height= get_height();
	m_widthdouble = m_width << 1;
	m_viewable_area_y_offset = get_viewable_area_y_offset();
	m_viewable_area_x_offset = get_viewable_area_x_offset();
#if 1

	if(NESTER_settings.nes.graphics.WindowMode_2xsai)
	{
		int ret;
		if(lpDDSOverlay)
			ret = lpDDSOverlay->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		else
			ret = lpDDSBack->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		if(DD_OK == ret){
			if(lpDDSOverlay)
				lpDDSOverlay->Unlock(NULL);
			else
				lpDDSBack->Unlock(NULL);
			BitDepth = ddsd.ddpfPixelFormat.dwRGBBitCount;

			if(mode_2xsai < 3){
				if(BitDepth!=8){
					buffer_16bit = (WORD *)malloc((m_width+4)*(m_height+4) * 2);
					if(BitDepth==16){
						if(ddsd.ddpfPixelFormat.dwGBitMask == 0x7E0){
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
					switch(mode_2xsai){
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
			else{
				if(mode_2xsai == 4)
					Func_2xSai = New2xSaI_8bit_mmx;
				else
					Func_2xSai = New2xSaI_8bit;
				buffer_16bit_bouble = (unsigned short *)malloc((m_height+4)*m_width*4);
				f_2xsai = 1;
			}
		}
	}
#endif
  } catch(...) {
    if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
      else { DeleteObject(oldPal); oldPal = NULL; }
    if(hBmp)     { DeleteObject(hBmp); }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
    if(bmInfo)   { delete bmInfo; bmInfo = NULL; }
    if(palInfo)  { delete palInfo; palInfo = NULL; }
	if(clipper){ clipper->Release(); clipper=NULL; }
	if(lpDDSOverlay){ lpDDSOverlay->Release(); lpDDSOverlay=NULL; }
	if(lpDDSMsg){ lpDDSMsg->Release(); lpDDSMsg=NULL; }
	if(lpDDSBack){ lpDDSBack->Release(); lpDDSBack=NULL; }
	if(lpDDSPrimary){ lpDDSPrimary->Release(); lpDDSPrimary=NULL; }
	if(lpDD){ lpDD->Release(); lpDD = NULL; }
	if(buffer_16bit){ free(buffer_16bit); buffer_16bit=NULL; }
	if(buffer_16bit_bouble){ free(buffer_16bit_bouble); buffer_16bit_bouble=NULL; }
    throw;
  }
}

win32_overlaywindowed_NES_screen_mgr::~win32_overlaywindowed_NES_screen_mgr()
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
	
	if(clipper){
		clipper->Release();
		clipper=NULL;
	}
	if(lpDDSOverlay)
    {
      lpDDSOverlay->UpdateOverlay(NULL, lpDDSPrimary, NULL, DDOVER_HIDE, NULL);
      lpDDSOverlay->Release();
      lpDDSOverlay=NULL;
    }
	if(lpDDSMsg){
		lpDDSMsg->Release();
		lpDDSMsg=NULL;
	}
	if(lpDDSBack){
		lpDDSBack->Release();
		lpDDSBack=NULL;
	}
	if(lpDDSPrimary){
		lpDDSPrimary->Release();
		lpDDSPrimary=NULL;
	}
	if(lpDD){
		lpDD->Release();
		lpDD = NULL;
	}

  if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
  if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
      else { DeleteObject(oldPal); oldPal = NULL; }
  if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
  if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
  if(hBmp)     { DeleteObject(hBmp); }
  if(palHan)   { DeleteObject(palHan); palHan = NULL; }
  if(bmInfo)   { delete bmInfo; bmInfo = NULL; }
  if(palInfo)  { delete palInfo; palInfo = NULL; }
}


boolean win32_overlaywindowed_NES_screen_mgr::lock(pixmap& p)
{
  p.height = m_height;
  p.width  = m_width;
  p.pitch  = m_width<<1;

  if(f_2xsai && mode_2xsai >= 3){
	  p.data   = (uint8 *)buffer_16bit_bouble + (m_width << 1);
  }
  else
	  p.data   = buffer;
  return TRUE;
}


boolean win32_overlaywindowed_NES_screen_mgr::unlock()
{
  blown_up = 0;
  return TRUE;
}


void win32_overlaywindowed_NES_screen_mgr::blt()
{
  RECT dest;

  oldBmp = 0;
  bmpDC = 0;
  screenDC = 0;
  HDC hdcSurf, hdcSurf2;    // The handle to the Overlay's DC

  try {
	  // avi record
	  if(avi_rec)
		  frame_avirec();

    screenDC = GetDC(window_handle);
    if(screenDC == NULL)
      throw "GetDC failed in win32_overlaywindowed_NES_screen_mgr::blt";

    bmpDC = CreateCompatibleDC(screenDC);
    if(!bmpDC)
      throw "CreateCompatibleDC failed in win32_overlaywindowed_NES_screen_mgr::blt";

    oldBmp = SelectBitmap(bmpDC, hBmp);
    if(oldBmp == NULL)
      throw "SelectBitmap failed in win32_overlaywindowed_NES_screen_mgr::blt";
/*
    if(!GetClientRect(window_handle, &dest))
      throw "GetClientRect failed in win32_overlaywindowed_NES_screen_mgr::blt";

	if(Mainwindowsbh){
		dest.bottom-=Mainwindowsbhs;
	}
*/
	dest = g_cscreen_rect;

//    if( NESTER_settings.nes.graphics.osd.double_size )
    {
      if( NESTER_settings.nes.graphics.EmulateTVScanline && !g_PreviewMode)
      {
		  emulateTVScanline();
      }
	  else if(f_2xsai&& !g_PreviewMode){
		  int ret;
		  if(mode_2xsai < 3){
			  if(BitGreen)
				  convto16_565bitpalette((DWORD *)&bmInfo->bmiPalette, palette_16bit);
			  else
				  convto16_555bitpalette((DWORD *)&bmInfo->bmiPalette, palette_16bit);
			  scrconv8_16for2xsai(&buffer[m_viewable_area_y_offset*m_widthdouble], (unsigned char *)&buffer_16bit[m_viewable_width*2], palette_16bit,
				  m_viewable_width, m_viewable_height, m_widthdouble, m_viewable_width<<1);
//					Func_2xSai((unsigned char *)&buffer_16bit[m_viewable_width], m_viewable_width<<1, NULL, (unsigned char *)buffer, m_widthdouble<<1, m_viewable_width, m_viewable_height);
		  }
		  if(lpDDSOverlay)
			  ret = lpDDSOverlay->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		  else
			  ret = lpDDSBack->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		  if(DD_OK == ret){

			if(mode_2xsai >= 3){
					Func_2xSai((unsigned char *)buffer_16bit_bouble +m_widthdouble, m_widthdouble, NULL, (unsigned char *)buffer, m_widthdouble, m_width, m_height);
					ddrawblt_8bit(&ddsd, (DWORD *)&bmInfo->bmiPalette, &buffer[m_viewable_area_x_offset + (m_viewable_area_y_offset*m_widthdouble<<1) ], m_viewable_width<<1, m_viewable_height<<1, m_widthdouble);
			}
			else{
				switch(BitDepth){
					case 16:
						Func_2xSai((unsigned char *)&buffer_16bit[m_viewable_width<<1], m_viewable_width<<1, NULL, (unsigned char *)ddsd.lpSurface, ddsd.lPitch, m_viewable_width, m_viewable_height);
						break;
					case 24:
						Func_2xSai((unsigned char *)&buffer_16bit[m_viewable_width<<1], m_viewable_width<<1, NULL, (unsigned char *)buffer_16bit_bouble, m_widthdouble<<1, m_viewable_width, m_viewable_height);
						scrconv16_24((unsigned char *)buffer_16bit_bouble, (unsigned char *)ddsd.lpSurface, m_viewable_width<<1, m_viewable_height<<1, m_viewable_width<<2, ddsd.lPitch);
						break;
					case 32:
						Func_2xSai((unsigned char *)&buffer_16bit[m_viewable_width<<1], m_viewable_width<<1, NULL, (unsigned char *)buffer_16bit_bouble, m_widthdouble<<1, m_viewable_width, m_viewable_height);
						scrconv16_32((unsigned char *)buffer_16bit_bouble, (unsigned char *)ddsd.lpSurface, m_viewable_width<<1, m_viewable_height<<1, m_widthdouble<<1, ddsd.lPitch);
						break;
				}
			}
			  if(lpDDSOverlay){
		  		  lpDDSOverlay->Unlock(NULL);
				  POINT pt;
				  pt.x=dest.left;
				  pt.y=dest.top;
				  ClientToScreen(window_handle, &pt);

				  if(NESTER_settings.nes.graphics.EmulateTVScanline||f_2xsai){
					  rs.right=m_viewable_width<<1;
					  rs.bottom=m_viewable_height<<1;
				  }
				  else{
					  rs.right=m_viewable_width;
					  rs.bottom=m_viewable_height;
				  }
				  rd.top=pt.y, rd.left=pt.x, rd.bottom=pt.y+dest.bottom, rd.right=pt.x+dest.right;
				  CheckBoundries();
				  lpDDSOverlay->UpdateOverlay(&rs, lpDDSPrimary, &rd, dwUpdateFlags, &ovfx);
				  goto _Blt_Finally;
			  }
			  else {
				  lpDDSBack->Unlock(NULL);
				  POINT pt;
				  RECT rsrc, d;
				  pt.x=dest.left;
				  pt.y=dest.top;
				  d=dest;
				  ClientToScreen(window_handle, &pt);
				  if(NESTER_settings.nes.graphics.EmulateTVScanline||f_2xsai){
					  rsrc.right=m_viewable_width<<1;
					  rsrc.bottom=m_viewable_height<<1;
				  }
				  else{
					  rsrc.right=m_viewable_width;
					  rsrc.bottom=m_viewable_height;
				  }
				  rsrc.top=0, rsrc.left=0;
				  d.top=pt.y, d.left=pt.x, d.bottom+=pt.y, d.right+=pt.x;
				  //			  lpDDSPrimary->BltFast(pt.x, pt.y, lpDDSBack, &rsrc, DDBLTFAST_WAIT );
				  lpDDSPrimary->Blt(&d, lpDDSBack, &rsrc, DDBLT_ASYNC, NULL);
				  goto _Blt_Finally;
			  }
		  }
	  }
	}
	if(lpDDSOverlay && NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay){//
		POINT pt;
//		HRESULT hRet;

		pt.x=dest.left;
		pt.y=dest.top;
		ClientToScreen(window_handle, &pt);

		lpDDSOverlay->GetDC(&hdcSurf2);
//		if(window_handle == GetFocus()){
		if(g_PreviewMode){
			rs.top=0, rs.left=0, rs.right=m_viewable_width, rs.bottom=m_viewable_height;
			rd.top=pt.y+g_Preview_State.PreviewScreenDrawRect.top, rd.left=pt.x+g_Preview_State.PreviewScreenDrawRect.left;
			rd.bottom=rd.top+rs.bottom, rd.right=rd.left+rs.right;
			BitBlt(hdcSurf2, 0, //, // // upper left dest x
			  0, // // upper left dest y
			  rs.right,
			  rs.bottom,
			  bmpDC,
			  get_viewable_area_x_offset(), // upper left source x
			  get_viewable_area_y_offset(), // upper left source y
			  SRCCOPY);
		}
		else{
			if(NESTER_settings.nes.graphics.EmulateTVScanline){
				rs.right=m_viewable_width<<1;
				rs.bottom=m_viewable_height<<1;
			}
			else{
				rs.right=m_viewable_width;
				rs.bottom=m_viewable_height;
			}
			rd.top=pt.y, rd.left=pt.x, rd.bottom=pt.y+dest.bottom, rd.right=pt.x+dest.right;
			BitBlt(hdcSurf2, 0, // upper left dest x
				  0, // upper left dest y
				  rs.right,
				  rs.bottom,
//				  (NESTER_settings.nes.graphics.EmulateTVScanline)? get_viewable_width()<<1:get_viewable_width(), //dest.right-dest.left, // width of dest
//				  (NESTER_settings.nes.graphics.EmulateTVScanline)? get_viewable_height()<<1:get_viewable_height(), //dest.bottom-dest.top, // height of dest
				  bmpDC,
				  get_viewable_area_x_offset(), //get_viewable_area_x_offset(), // upper left source x
				  get_viewable_area_y_offset(), //get_viewable_area_y_offset(), // upper left source y
				  SRCCOPY);
		}
//		lpDDSOverlay->SetOverlayPosition(pt.x, pt.y);
		lpDDSOverlay->ReleaseDC(hdcSurf2);
		CheckBoundries();
        lpDDSOverlay->UpdateOverlay(&rs, lpDDSPrimary, &rd, dwUpdateFlags, &ovfx);
//		}
//		rs.left=0,rs.top=0, rs.bottom=get_viewable_area_y_offset(), rs.right=get_viewable_area_x_offset();
//		rd.left=pt.x,rd.top=pt.y, rd.bottom=dest.bottom-dest.top+pt.y, rd.right=dest.right-dest.left+pt.x;
//		CheckBoundries();
//		lpDDSOverlay->SetOverlayPosition(pt.x, pt.y);
//		lpDDSOverlay->Flip(NULL, DDFLIP_WAIT);
	}
	else{
		POINT pt;
		HRESULT hRet;
		RECT rsrc, d;

		pt.x=dest.left;
		pt.y=dest.top;
		d=dest;
		ClientToScreen(window_handle, &pt);
#if 1
		hRet = lpDDSBack->GetDC(&hdcSurf);
		if(hRet!=DD_OK)
			throw "getdc";

		if(g_PreviewMode){
			rsrc.top=0, rsrc.left=0, rsrc.right=m_viewable_width, rsrc.bottom=m_viewable_height;
			d.top=pt.y+g_Preview_State.PreviewScreenDrawRect.top, d.left=pt.x+g_Preview_State.PreviewScreenDrawRect.left;
			d.bottom=d.top+rsrc.bottom, d.right=d.left+rsrc.right;
			BitBlt(hdcSurf, 0, //, // // upper left dest x
			  0, // // upper left dest y
			  rsrc.right,
			  rsrc.bottom,
			  bmpDC,
			  get_viewable_area_x_offset(), // upper left source x
			  get_viewable_area_y_offset(), // upper left source y
			  SRCCOPY);
		}
		else{
#if 1
			if(NESTER_settings.nes.graphics.EmulateTVScanline||f_2xsai){
				rsrc.right=m_viewable_width<<1;
				rsrc.bottom=m_viewable_height<<1;
			}
			else{
				rsrc.right=m_viewable_width;
				rsrc.bottom=m_viewable_height;
			}
			rsrc.top=0, rsrc.left=0; //, rsrc.right=(NESTER_settings.nes.graphics.EmulateTVScanline)? get_viewable_width()<<1:get_viewable_width(), /*dest.right,*/ rsrc.bottom=(NESTER_settings.nes.graphics.EmulateTVScanline)? get_viewable_height()<<1:get_viewable_height(); //dest.bottom;
			d.top=pt.y, d.left=pt.x, d.bottom+=pt.y, d.right+=pt.x;
			BitBlt(hdcSurf, 0, //, // // upper left dest x
				  0, // // upper left dest y
				  rsrc.right,
				  rsrc.bottom,
//				  (NESTER_settings.nes.graphics.EmulateTVScanline)? m_viewable_width<<1:m_viewable_width, //dest.right-dest.left, // width of dest
//				  (NESTER_settings.nes.graphics.EmulateTVScanline)? m_viewable_height<<1:m_viewable_height, //dest.bottom-dest.top, // height of dest
				  bmpDC,
				  get_viewable_area_x_offset(), // upper left source x
				  get_viewable_area_y_offset(), // upper left source y
				  SRCCOPY);
		}
		lpDDSBack->ReleaseDC(hdcSurf);
#else
		lpDDSBack->ReleaseDC(hdcSurf);
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize=sizeof(ddsd);
		if(DD_OK == lpDDSBack->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL)){
			ddrawblt_8bit(&ddsd, (DWORD *)&bmInfo->bmiPalette, (unsigned char *)buffer, get_viewable_height());
			lpDDSBack->Unlock(NULL);
		}
		}
#endif
		lpDDSPrimary->Blt(&d, lpDDSBack, &rsrc, /*DDBLT_ASYNC*/ DDBLT_WAIT , NULL);
#else
		unsigned char *tb=NULL;
		int width,height;
		height = dest.bottom-dest.top, width = dest.right-dest.left;
		tb= (unsigned char*)malloc(width*height);
		if(tb){
			scaling(tb, height, width);
			lpDDSPrimary->Lock(NULL, &ddsd, 0, NULL);
			memcpy(ddsd.lpSurface, tb, width*height);
			lpDDSPrimary->Unlock(NULL);
			free(tb);
		}
#endif
	}

_Blt_Finally:

    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

  } catch(const char* IFDEBUG(s)) {
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
    LOG(s << endl);
  }
}

void win32_overlaywindowed_NES_screen_mgr::clear(PIXEL color)
{
  PIXEL* p;
  p = buffer;
  uint32 height = m_height << 1;
  uint32 width = m_width << 1;
  for(uint32 i = 0; i < height; i++)
  {
    memset(p, color, width);
    p += width;
  }
}

boolean win32_overlaywindowed_NES_screen_mgr::set_palette(const uint8 pal[256][3])
{
  oldBmp = 0;
  bmpDC  = 0;
  palHan = 0;
  bmpDC  = 0;
  screenDC = 0;

  try {
    screenDC = GetDC(window_handle);
    if(screenDC == NULL)
      throw "GetDC failed in win32_overlaywindowed_NES_screen_mgr::set_palette";

    palHan = CreatePalette((LOGPALETTE*)palInfo);
    if(!palHan)
      throw "CreatePalette failed in win32_overlaywindowed_NES_screen_mgr::set_palette";

    if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
      throw "SelectPalette failed in win32_overlaywindowed_NES_screen_mgr::set_palette";

    bmpDC = CreateCompatibleDC(screenDC);
    if(!bmpDC)
      throw "CreateCompatibleDC failed in win32_overlaywindowed_NES_screen_mgr::set_palette";

    oldBmp = SelectBitmap(bmpDC, hBmp);
    if(oldBmp == NULL)
      throw "SelectBitmap failed in win32_overlaywindowed_NES_screen_mgr::set_palette";

    for(int i = 0; i < 256; i++)
    {
      palInfo->palPalEntry[i].peRed   = (BYTE)pal[i*3    ];
      palInfo->palPalEntry[i].peGreen = (BYTE)pal[i*3 + 1];
      palInfo->palPalEntry[i].peBlue  = (BYTE)pal[i*3 + 2];
      palInfo->palPalEntry[i].peFlags = (BYTE)PC_NOCOLLAPSE;// | PC_RESERVED;

      bmInfo->bmiPalette[i].rgbRed   = palInfo->palPalEntry[i].peRed;
      bmInfo->bmiPalette[i].rgbGreen = palInfo->palPalEntry[i].peGreen;
      bmInfo->bmiPalette[i].rgbBlue  = palInfo->palPalEntry[i].peBlue;
      bmInfo->bmiPalette[i].rgbReserved = 0;
    }
/*
	if(f_2xsai || BitDepth == 16){
		if(BitGreen)
			convto16_565bitpalette((DWORD *)&bmInfo->bmiPalette, palette_16bit);
		else
			convto16_555bitpalette((DWORD *)&bmInfo->bmiPalette, palette_16bit);
	}
*/

    if(!SetDIBColorTable(bmpDC, 0, 256, bmInfo->bmiPalette))
      throw "SetDIBColorTable failed in win32_overlaywindowed_NES_screen_mgr::set_palette";

    if(!AnimatePalette(palHan, 0, 256, &palInfo->palPalEntry[0]))
      if(!SetPaletteEntries(palHan, 0, 256, &palInfo->palPalEntry[0]))
        throw "SetPaletteEntries failed in win32_overlaywindowed_NES_screen_mgr::set_palette";

    if(RealizePalette(screenDC) == GDI_ERROR)
      throw "RealizePalette failed in win32_overlaywindowed_NES_screen_mgr::set_palette";

    if(oldPal)   { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

  } catch(const char* IFDEBUG(s)) {
    if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
      else { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
    LOG(s << endl);
    return FALSE;
  }

  return TRUE;
}

boolean win32_overlaywindowed_NES_screen_mgr::get_palette(uint8 pal[256][3])
{
  return FALSE;
}

boolean win32_overlaywindowed_NES_screen_mgr::set_palette_section(uint8 start, uint8 len, const uint8 pal[][3])
{
  oldBmp = 0;
  bmpDC  = 0;
  palHan = 0;
  bmpDC  = 0;
  screenDC = 0;

  try {
    screenDC = GetDC(window_handle);
    if(screenDC == NULL)
      throw "GetDC failed in win32_overlaywindowed_NES_screen_mgr::set_palette_section";

    palHan = CreatePalette((LOGPALETTE*)palInfo);
    if(!palHan)
      throw "CreatePalette failed in win32_overlaywindowed_NES_screen_mgr::set_palette_section";

    if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
      throw "SelectPalette failed in win32_overlaywindowed_NES_screen_mgr::set_palette_section";

    bmpDC = CreateCompatibleDC(screenDC);
    if(!bmpDC)
      throw "CreateCompatibleDC failed in win32_overlaywindowed_NES_screen_mgr::set_palette_section";

    oldBmp = SelectBitmap(bmpDC, hBmp);
    if(oldBmp == NULL)
      throw "SelectBitmap failed in win32_overlaywindowed_NES_screen_mgr::set_palette_section";

    for(int i = 0; i < len; i++)
    {
      palInfo->palPalEntry[start+i].peRed   = (BYTE)pal[i][0];
      palInfo->palPalEntry[start+i].peGreen = (BYTE)pal[i][1];
      palInfo->palPalEntry[start+i].peBlue  = (BYTE)pal[i][2];
      palInfo->palPalEntry[start+i].peFlags = (BYTE)PC_NOCOLLAPSE;// | PC_RESERVED;

      bmInfo->bmiPalette[start+i].rgbRed   = palInfo->palPalEntry[start+i].peRed;
      bmInfo->bmiPalette[start+i].rgbGreen = palInfo->palPalEntry[start+i].peGreen;
      bmInfo->bmiPalette[start+i].rgbBlue  = palInfo->palPalEntry[start+i].peBlue;
      bmInfo->bmiPalette[start+i].rgbReserved = 0;
    }
/*
	if(f_2xsai || BitDepth == 16){
		if(BitGreen)
			convto16_565bitpalette((DWORD *)&bmInfo->bmiPalette, palette_16bit);
		else
			convto16_555bitpalette((DWORD *)&bmInfo->bmiPalette, palette_16bit);
	}
*/
    if(!SetDIBColorTable(bmpDC, 0, 256, bmInfo->bmiPalette))
      throw "SetDIBColorTable failed in win32_overlaywindowed_NES_screen_mgr::set_palette_section";

    if(!AnimatePalette(palHan, start, len, &palInfo->palPalEntry[start]))
      if(!SetPaletteEntries(palHan, start, len, &palInfo->palPalEntry[start]))
        throw "SetPaletteEntries failed in win32_overlaywindowed_NES_screen_mgr::set_palette_section";

    if(RealizePalette(screenDC) == GDI_ERROR)
      throw "RealizePalette failed in win32_overlaywindowed_NES_screen_mgr::set_palette_section";

    if(oldPal)   { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

  } catch(const char* IFDEBUG(s)) {
    if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
      else { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
    LOG(s << endl);
    return FALSE;
  }
  return TRUE;
}

boolean win32_overlaywindowed_NES_screen_mgr::get_palette_section(uint8 start, uint8 len, uint8 pal[][3])
{
  return FALSE;
}

void win32_overlaywindowed_NES_screen_mgr::assert_palette()
{
  oldBmp = 0;
  bmpDC  = 0;
  palHan = 0;
  bmpDC  = 0;
  screenDC = 0;

  try {
    screenDC = GetDC(window_handle);
    if(screenDC == NULL)
      throw "GetDC failed in win32_overlaywindowed_NES_screen_mgr::assert_palette";

    palHan = CreatePalette((LOGPALETTE*)palInfo);
    if(!palHan)
      throw "CreatePalette failed in win32_overlaywindowed_NES_screen_mgr::assert_palette";

    if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
      throw "SelectPalette failed in win32_overlaywindowed_NES_screen_mgr::assert_palette";

    bmpDC = CreateCompatibleDC(screenDC);
    if(!bmpDC)
      throw "CreateCompatibleDC failed in win32_overlaywindowed_NES_screen_mgr::assert_palette";

    oldBmp = SelectBitmap(bmpDC, hBmp);
    if(oldBmp == NULL)
      throw "SelectBitmap failed in win32_overlaywindowed_NES_screen_mgr::assert_palette";

    if(!SetDIBColorTable(bmpDC, 0, 256, bmInfo->bmiPalette))
      throw "SetDIBColorTable failed in win32_overlaywindowed_NES_screen_mgr::assert_palette";

    if(!AnimatePalette(palHan, 0, 256, &palInfo->palPalEntry[0]))
      if(!SetPaletteEntries(palHan, 0, 256, &palInfo->palPalEntry[0]))
        throw "SetPaletteEntries failed in win32_overlaywindowed_NES_screen_mgr::set_palette_section";

    if(RealizePalette(screenDC) == GDI_ERROR)
      throw "RealizePalette failed in win32_overlaywindowed_NES_screen_mgr::assert_palette";

    if(oldPal)   { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

  } catch(const char* IFDEBUG(s)) {
    if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
      else { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
    LOG(s << endl);
  }
}


void win32_overlaywindowed_NES_screen_mgr::scaling(unsigned char *tbuff, int wheight, int wwidth)
{
 
  unsigned char *d_base;
  unsigned char *s_base;

  uint32 i,j, dwSLength, dwLength;
  int viewable_width	=get_width();
  int viewable_heigh	=get_viewable_height(); 

  // source pointer
  s_base = buffer;
  // destination pointer
  d_base = tbuff;
	if ((viewable_width*3) % 4==0)
		dwSLength=viewable_width*3;
	else
		dwSLength=viewable_width*3+(4-(viewable_width*3) % 4);

	if ((wwidth*3) % 4==0)
		dwLength=wwidth*3;
	else
		dwLength=wwidth*3+(4-(wwidth*3) % 4);

  int x,y;
  for(i = 0; i < (unsigned int)wheight; i++){
//	  s = s_base;
//	  d = d_base;
	  for(j = 0; j < (unsigned int)wwidth; j++){
			x=(int)(((double)viewable_width/(double)wwidth)*j);
			y=(int)(((double)viewable_heigh/(double)wheight)*i);
//			CopyMemory(d_base+j*3+i*dwLength, s_base+x*3+y*dwSLength,3);
	  }
/*	  memcpy(d_base-pitch, d_base, pitch);
	  d_base -= pitch<<1;
	  s_base -= pitch;
*/}
}


void win32_overlaywindowed_NES_screen_mgr::emulateTVScanline()
{
  if(blown_up) return;
  
  unsigned char *d_base;
  unsigned char *s_base;
  unsigned char *d;
  unsigned char *s;
  uint32 i,j;
  const uint32 viewable_height = get_viewable_height();
  const uint32 viewable_width = get_viewable_width();
  const uint32 pitch = get_width()<<1;

  // source pointer
  s_base = &buffer[get_viewable_area_x_offset() + 
    (pitch * (get_viewable_area_y_offset() + (viewable_height-1)))];

  // destination pointer
  d_base = &buffer[get_viewable_area_x_offset() + pitch +
    (pitch * (get_viewable_area_y_offset() + ( (viewable_height-1) << 1 ) ) )];

  for(i = 0; i < viewable_height; i++)
  {
    // stretch the source line to the destination line
    s = s_base;
    d = d_base;
    for(j = 0; j < viewable_width; j++)
    {
      *(d++) = *s;
      *(d++) = *s;
      s++;
    }
    d_base -= pitch;
    
	// draw TV scanline
    memset( d_base, 0x00, pitch );
    d_base -= pitch;
    s_base -= pitch;
  }
  blown_up = 1;
}


void win32_overlaywindowed_NES_screen_mgr::shot_screen( char *szFileName )
{
  const DWORD x_offset = get_viewable_area_x_offset();
  const DWORD y_offset = get_viewable_area_y_offset();
  const DWORD pitch = get_width() << 1;
  int w, h, i,j,k;
  unsigned char *sdt = NULL, *psrc;
  PALETTEENTRY pal[256];
  w = get_viewable_width();
  h = get_viewable_height();
  sdt = (unsigned char *)malloc(w*h);
  if(sdt == NULL)
	  return;
  if(f_2xsai && mode_2xsai >= 3)
	  psrc	= (uint8 *)buffer_16bit_bouble + (m_width << 1);
  else
	  psrc	= buffer;

  if(blown_up){
	  int dp=pitch<<1;
	  for(i=0,j=0,k=y_offset*dp+x_offset+pitch; i<h; i++, j+=w, k+=dp){
		  memcpy(&sdt[j], &psrc[k], w);
	  }
  }
  else{
	  for(i=0,j=0,k=y_offset*pitch+x_offset; i<h; i++, j+=w, k+=pitch){
		  memcpy(&sdt[j], &psrc[k], w);
	  }
  }
  for(i=0;i<0x100;i++){
	  pal[i].peBlue	=bmInfo->bmiPalette[i].rgbBlue;
	  pal[i].peGreen=bmInfo->bmiPalette[i].rgbGreen;
	  pal[i].peRed	=bmInfo->bmiPalette[i].rgbRed;
	  pal[i].peFlags=0;
  }
  screenshot_save_8bit(szFileName, w, h, pal, sdt);
  free(sdt);
}

int win32_overlaywindowed_NES_screen_mgr::CheckOverlaySupport(){
    memset(&ddcaps, 0, sizeof(ddcaps));
    ddcaps.dwSize = sizeof(ddcaps);
    if (lpDD->GetCaps(&ddcaps, 0))
        return FALSE;
    if (!(ddcaps.dwCaps & DDCAPS_OVERLAY))
        return FALSE;
    if (!(ddcaps.dwCaps & DDCAPS_OVERLAYSTRETCH))
        return FALSE;
    return TRUE;
}


int win32_overlaywindowed_NES_screen_mgr::DDOverlayInit(){
//	DDCOLORKEY	ddck;
    DDSURFACEDESC2              ddsd;  // DirectDraw surface descriptor
    HRESULT                     hRet;
    DDPIXELFORMAT               ddpfOverlayFormat =	{
		sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16, 0xF800, 0x07e0, 0x001F,   // 16 bit RGB 5:6:5
			 0};
		RECT dest;
		GetClientRect(window_handle, &dest);

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY |DDCAPS_OVERLAYSTRETCH ;
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    ddsd.dwBackBufferCount = 0;
	ddsd.dwWidth	=(NESTER_settings.nes.graphics.EmulateTVScanline||f_2xsai)?get_viewable_width()<<1:get_viewable_width(); //(NESTER_settings.nes.graphics.EmulateTVScanline)? get_viewable_width()<<1:get_viewable_width();
	ddsd.dwHeight	=(NESTER_settings.nes.graphics.EmulateTVScanline||f_2xsai)?get_viewable_height()<<1:get_viewable_height(); //(NESTER_settings.nes.graphics.EmulateTVScanline)? get_viewable_height()<<1:get_viewable_height(); 
    ddsd.ddpfPixelFormat = ddpfOverlayFormat;  // Use 16 bit RGB 5:6:5 pixel format

    // Attempt to create the surface with theses settings
    hRet = lpDD->CreateSurface(&ddsd, &lpDDSOverlay, NULL);
    if (hRet != DD_OK)
        return FALSE;
/*	ZeroMemory(&ddck, sizeof(DDCOLORKEY));
	ddck.dwColorSpaceLowValue	=0;
	ddck.dwColorSpaceHighValue	=0;
	lpDDSOverlay->SetColorKey(DDCKEY_SRCBLT,&ddck);
*/    return TRUE;
}


int win32_overlaywindowed_NES_screen_mgr::DDPrimaryInit(){
    HRESULT                     hRet;
//    DDSURFACEDESC2              ddsd;

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    hRet = lpDD->CreateSurface(&ddsd, &lpDDSPrimary, NULL);
    if (hRet != DD_OK)
        return FALSE;
    return TRUE;
}


void win32_overlaywindowed_NES_screen_mgr::CheckBoundries(){
/*
    if ((ddcaps.dwCaps & DDCAPS_OVERLAYSTRETCH) && (ddcaps.dwMinOverlayStretch)
        && (g_dwXRatio < ddcaps.dwMinOverlayStretch))
    {
        rd.right = 2 * GetSystemMetrics(SM_CXSIZEFRAME) + rd.left + (get_viewable_width()
                                 * (ddcaps.dwMinOverlayStretch + 1)) / 1000;
    }
    if ((ddcaps.dwCaps & DDCAPS_OVERLAYSTRETCH) && (ddcaps.dwMaxOverlayStretch)
        && (g_dwXRatio > ddcaps.dwMaxOverlayStretch))
    {
        rd.right = 2 * GetSystemMetrics(SM_CXSIZEFRAME) + rd.left + (get_viewable_height()
                               * (ddcaps.dwMaxOverlayStretch + 999)) / 1000;
    }

    // Recalculate the ratio's for the upcoming calculations
    g_dwXRatio = (rd.right - rd.left) * 1000 / (rs.right - rs.left);
    g_dwYRatio = (rd.bottom - rd.top) * 1000 / (rs.bottom - rs.top);

    // Check to make sure we're within the screen's boundries, if not then fix
    // the problem by adjusting the source rectangle which we draw from.
    if (rd.left < 0)
    {
        rs.left = -rd.left * 1000 / g_dwXRatio;
        rd.left = 0;
    }
    if (rd.right > GetSystemMetrics(SM_CXSCREEN))
    {
        rs.right = get_viewable_width()- ((rd.right - GetSystemMetrics(SM_CXSCREEN)) *
                                1000 / g_dwXRatio);
        rd.right = GetSystemMetrics(SM_CXSCREEN);
    }
    if (rd.bottom > GetSystemMetrics(SM_CYSCREEN))
    {
        rs.bottom = get_viewable_height() - ((rd.bottom - GetSystemMetrics(SM_CYSCREEN))
                                 * 1000 / g_dwYRatio);
        rd.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    // I don't know how useful this is... but just in case someone can do it - here's the check.
    if (rd.top < 0)
    {
        rs.top = -rd.top * 1000 / g_dwYRatio;
        rd.top = 0;
    }
*/
    if ((ddcaps.dwCaps & DDCAPS_ALIGNBOUNDARYSRC) && ddcaps.dwAlignBoundarySrc)
        rs.left = (rs.left + ddcaps.dwAlignBoundarySrc / 2) & -(signed)
            (ddcaps.dwAlignBoundarySrc);
    if ((ddcaps.dwCaps & DDCAPS_ALIGNSIZESRC) && ddcaps.dwAlignSizeSrc)
        rs.right = rs.left + (rs.right - rs.left + ddcaps.dwAlignSizeSrc
                                   / 2) & -(signed) (ddcaps.dwAlignSizeSrc);
    if ((ddcaps.dwCaps & DDCAPS_ALIGNBOUNDARYDEST) && ddcaps.dwAlignBoundaryDest)
        rd.left = (rd.left + ddcaps.dwAlignBoundaryDest / 2) & -(signed)
            (ddcaps.dwAlignBoundaryDest);
    if ((ddcaps.dwCaps & DDCAPS_ALIGNSIZEDEST) && ddcaps.dwAlignSizeDest)
        rd.right = rd.left + (rd.right - rd.left) & -(signed) (ddcaps.dwAlignSizeDest);
}


int win32_overlaywindowed_NES_screen_mgr::create_backsurface()
{
	DDSURFACEDESC2	ddsd;
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize	=sizeof(ddsd);
	ddsd.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH; 
	ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN; //| DDSCAPS_SYSTEMMEMORY;
	if(NESTER_settings.nes.graphics.WindowModeBackBuffM)
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

	ddsd.dwWidth	=get_viewable_width()*2;//(NESTER_settings.nes.graphics.EmulateTVScanline )? get_viewable_width()<<1:get_viewable_width();
	ddsd.dwHeight	=get_viewable_height()*2;//(NESTER_settings.nes.graphics.EmulateTVScanline )? get_viewable_height()<<1:get_viewable_height(); 
	if(lpDD->CreateSurface(&ddsd,&lpDDSBack,NULL)!=DD_OK)
		return 0;
	return 1;
}



void win32_overlaywindowed_NES_screen_mgr::nnnManuEnable(unsigned char flag){
	Menuflag=flag;
	if(flag && lpDDSOverlay && NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay){
		lpDDSOverlay->UpdateOverlay(NULL, lpDDSPrimary, NULL, DDOVER_HIDE, NULL);
	}
}

void win32_overlaywindowed_NES_screen_mgr::EndEmu(){
	if(lpDDSOverlay && NESTER_settings.nes.graphics.WindowModeUseDdrawOverlay){
		lpDDSOverlay->UpdateOverlay(NULL, lpDDSPrimary, NULL, DDOVER_HIDE, NULL);
	}
}


void win32_overlaywindowed_NES_screen_mgr::StartEmu(){
}




int DDColorMatch(IDirectDrawSurface7 * pdds, COLORREF rgb)
{
    COLORREF                rgbT;
    HDC                     hdc;
    DWORD                   dw = CLR_INVALID;
    DDSURFACEDESC2          ddsd;
    HRESULT                 hres;

    //
    //  Use GDI SetPixel to color match for us
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel value
        SetPixel(hdc, 0, 0, rgb);       // Set our value
        pdds->ReleaseDC(hdc);
    }
    //
    // Now lock the surface so we can read back the converted color
    //
    ddsd.dwSize = sizeof(ddsd);
    while ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;
    if (hres == DD_OK)
    {
        dw = *(DWORD *) ddsd.lpSurface;                 // Get DWORD
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;  // Mask it to bpp
        pdds->Unlock(NULL);
    }
    //
    //  Now put the color that was there back.
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }
    return dw;
}



int win32_overlaywindowed_NES_screen_mgr::start_avirec(char *fn){
	avi_rec = recavifilestart(fn, get_viewable_width(), get_viewable_height());
	return avi_rec;
}


void win32_overlaywindowed_NES_screen_mgr::frame_avirec(){
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
  if(f_2xsai && mode_2xsai >= 3)
	  psrc	= (uint8 *)buffer_16bit_bouble + (m_width << 1);
  else
	  psrc	= buffer;
  if(blown_up){
	  int dp=pitch<<1;
	  for(i=0,j=0,k=(y_offset+h-1)*dp+x_offset; i<h; i++, k-=dp){
		  for(int x=0,dk=k; x<w; x++, dk+=2){
		  	  sdt[j++] = psrc[dk];
		  }
	  }
  }
  else{
	  for(i=0,j=0,k=(y_offset+h-1)*pitch+x_offset; i<h; i++, j+=w, k-=pitch){
		  memcpy(&sdt[j], &psrc[k], w);
	  }
  }
  recavifilewriteflame(sdt);
  free(sdt);
}


void win32_overlaywindowed_NES_screen_mgr::end_avirec(){
	if(avi_rec)
		recavifilerelease();
}


