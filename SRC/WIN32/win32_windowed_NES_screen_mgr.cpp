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

#include "win32_windowed_NES_screen_mgr.h"
#include "debug.h"

#include "screenshot.h"
#include "recavi.h"
#include "savecfg.h"
#include "ddblt.h"

/*
extern unsigned char nnnKailleraRecvLag;
extern unsigned char nnnKailleraSendLag;
extern unsigned char nnnKailleraLagDisp;
*/
extern char g_ScreenMsgStr[256];
extern int Mainwindowsbhs;
extern HWND Mainwindowsbh;
extern int g_PreviewMode;
extern struct Preview_state g_Preview_State;
extern RECT g_cscreen_rect;



win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr(HWND wnd_handle)
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
	avi_rec = 0;
	avi_counter=0;
	f_2xsai = 0;
	buffer_16bit = NULL;
	buffer_8bit_bouble=NULL;

	try {
		if(NESTER_settings.nes.graphics.WindowMode_2xsai && NESTER_settings.nes.graphics.osd.zoom_size!=1){
			f_2xsai = 1;
			mode_2xsai = NESTER_settings.nes.graphics.WindowMode_2xsaiType;
		}
		m_viewable_width = get_viewable_width();
		m_viewable_height=get_viewable_height();
		m_width = get_width();
		m_height= get_height();
		m_widthdouble = m_width << 1;
		m_viewable_area_y_offset = get_viewable_area_y_offset();
		m_viewable_area_x_offset = get_viewable_area_x_offset();
		bmInfo = new bitmapInfo;
		if(bmInfo == NULL)
			throw "Out of memory in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr";

		palInfo = new logPalette;
		if(palInfo == NULL)
			throw "Out of memory in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr";

		memset(&bmInfo->bmiHeader, 0x00, sizeof(bmInfo->bmiHeader));
		bmInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmInfo->bmiHeader.biWidth = get_width()<<1; // double size
		bmInfo->bmiHeader.biHeight = -(abs(get_height())<<1); // top-down bitmap
		bmInfo->bmiHeader.biPlanes = 1;
		bmInfo->bmiHeader.biBitCount = (f_2xsai && mode_2xsai < 3)?16:8;
		bmInfo->bmiHeader.biCompression = BI_RGB;
		bmInfo->bmiHeader.biSizeImage = NULL;
		bmInfo->bmiHeader.biXPelsPerMeter = NULL;
		bmInfo->bmiHeader.biYPelsPerMeter = NULL;
		bmInfo->bmiHeader.biClrUsed = (f_2xsai && mode_2xsai < 3)?0:256;
		bmInfo->bmiHeader.biClrImportant = (f_2xsai && mode_2xsai < 3)?0:256;

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
			throw "GetDC failed in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr";


		if(!(f_2xsai && mode_2xsai < 3)){
			palHan = CreatePalette((LOGPALETTE*)palInfo);
			if(!palHan)
				throw("CreatePalette failed in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr");
			if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
				throw("SelectPalette failed in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr");
			
			if(RealizePalette(screenDC) == GDI_ERROR)
				throw("RealizePalette failed in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr");
			DeleteObject(palHan);
			palHan = NULL;
		}

		hBmp = CreateDIBSection(screenDC, (BITMAPINFO*)bmInfo,
		                        DIB_RGB_COLORS,
		                        (void**)&buffer, NULL, NULL);
		if(!hBmp)
			throw("CreateDIBSection failed in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr");
		if(f_2xsai){
			if(mode_2xsai < 3){
				Init_2xSaI(555);
				buffer_16bit = (WORD *)malloc((m_width+4)*(m_height+4) * 2);
				buffer_8bit_bouble = (unsigned char *)malloc(m_height*m_width*4);
				convto16_555bitpalette((DWORD *)&bmInfo->bmiPalette, palette_16bit);
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
			}
			else{
				if(mode_2xsai == 4)
					Func_2xSai = New2xSaI_8bit_mmx;
				else
					Func_2xSai = New2xSaI_8bit;
				buffer_8bit_bouble = (unsigned char *)malloc((m_height+4)*m_width*4);
				mode_2xsai = 3;
			}
		}

	if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

	} catch(...) {
		if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
		else { DeleteObject(oldPal); oldPal = NULL; }
		if(hBmp)     { DeleteObject(hBmp); }
		if(palHan)   { DeleteObject(palHan); palHan = NULL; }
		if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
		if(bmInfo)   { delete bmInfo; bmInfo = NULL; }
		if(palInfo)  { delete palInfo; palInfo = NULL; }
		if(buffer_16bit){
			free(buffer_16bit);
			buffer_16bit=NULL;
		}
		if(buffer_8bit_bouble){
			free(buffer_8bit_bouble);
			buffer_8bit_bouble=NULL;
		}
		throw;
	}
}

win32_windowed_NES_screen_mgr::~win32_windowed_NES_screen_mgr()
{
	if(avi_rec)
		end_avirec();
	if(buffer_16bit){
		free(buffer_16bit);
		buffer_16bit=NULL;
	}
	if(buffer_8bit_bouble){
		free(buffer_8bit_bouble);
		buffer_8bit_bouble=NULL;
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

boolean win32_windowed_NES_screen_mgr::lock(pixmap& p)
{
	p.height = get_height();
	p.width  = get_width();
	p.pitch  = get_width()<<1;
	if(f_2xsai){
		if(mode_2xsai == 3){
			p.data = buffer_8bit_bouble + (get_width() << 1);
		}
		else{
			p.data   = buffer_8bit_bouble;
		}
	}
	else
		p.data   = buffer;

	return TRUE;
}

boolean win32_windowed_NES_screen_mgr::unlock()
{
	blown_up = 0;
	return TRUE;
}

void win32_windowed_NES_screen_mgr::displaymessage()
{
#if 0
	if(g_ScreenMsgStr[0]){
		RECT sr;
		HFONT hFont;
#ifdef _NNNESTERJ_ENGLISH
		HFONT font = CreateFont(12,9,0,0,FW_THIN,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
		                        DEFAULT_PITCH|FF_SCRIPT,"Courier");
#else
		hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,	SHIFTJIS_CHARSET,
						   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "‚l‚r ƒSƒVƒbƒN");
#endif
		sr.left=5, sr.top= 5, sr.bottom=29, sr.right=get_width();
		SetBkMode(screenDC, 0);
		SetTextColor(screenDC, RGB(255, 255, 255));
		SelectObject(screenDC, hFont);
		DrawText(screenDC /*screenDC*/, g_ScreenMsgStr, -1, &sr, DT_WORDBREAK);
		DeleteObject(hFont);
	}
#endif
	return;
}


void win32_windowed_NES_screen_mgr::blt()
{
	RECT dest;

	oldBmp = 0;
	bmpDC = 0;
	screenDC = 0;

	try {
		// avi record
	  if(avi_rec)
		  frame_avirec();

		screenDC = GetDC(window_handle);
		if(screenDC == NULL)
			throw "GetDC failed in win32_windowed_NES_screen_mgr::blt";

		bmpDC = CreateCompatibleDC(screenDC);
		if(!bmpDC)
			throw "CreateCompatibleDC failed in win32_windowed_NES_screen_mgr::blt";

		oldBmp = SelectBitmap(bmpDC, hBmp);
		if(oldBmp == NULL)
			throw "SelectBitmap failed in win32_windowed_NES_screen_mgr::blt";

		if(!GetClientRect(window_handle, &dest))
			throw "GetClientRect failed in win32_windowed_NES_screen_mgr::blt";

		if(Mainwindowsbh){
			dest.bottom-=Mainwindowsbhs;
		}
		if(g_PreviewMode){
			BitBlt(screenDC, g_Preview_State.PreviewScreenDrawRect.left, g_Preview_State.PreviewScreenDrawRect.top, get_viewable_width(), get_viewable_height(),
		    bmpDC, get_viewable_area_x_offset(), get_viewable_area_y_offset(), SRCCOPY);
			goto _Blt_Finally;
		}

		if( NESTER_settings.nes.graphics.osd.zoom_size!=1 )
		{
			if( NESTER_settings.nes.graphics.UseStretchBlt )
			{
				StretchBlt(
				    screenDC,
				    0, // upper left dest x
				    0, // upper left dest y
				    dest.right-dest.left, // width of dest
				    dest.bottom-dest.top, // height of dest
				    bmpDC,
				    get_viewable_area_x_offset(), // upper left source x
				    get_viewable_area_y_offset(), // upper left source y
				    get_viewable_width(),
				    get_viewable_height(),
				    SRCCOPY);
				goto _Blt_Finally;
			}
			else if( NESTER_settings.nes.graphics.EmulateTVScanline )
			{
				emulateTVScanline();
			}
			else if(f_2xsai){
				if(mode_2xsai != 3){
					convto16_555bitpalette((DWORD *)&bmInfo->bmiPalette, palette_16bit);
/*
					for(int t = 0; t<m_height; t++){
						buffer_16bit[t* m_viewable_width+ 8]=0xffff;
					}
*/
/*
					for(int t = 0; t<m_viewable_width; t++){
						buffer_16bit[t]=0xffff;
					}
*/
//					memcpy(buffer_8bit_bouble, buffer, m_height*m_width);
					scrconv8_16for2xsai(&buffer_8bit_bouble[m_viewable_area_y_offset*(m_widthdouble)], (unsigned char *)&buffer_16bit[m_viewable_width*2], palette_16bit,
						m_viewable_width, m_viewable_height, m_widthdouble, m_viewable_width<<1);
					Func_2xSai((unsigned char *)&buffer_16bit[m_viewable_width*2], m_viewable_width<<1, NULL, (unsigned char *)buffer, m_widthdouble<<1, m_viewable_width, m_viewable_height);
					BitBlt(
						screenDC,
						0, // upper left dest x
						0, // upper left dest y
						dest.right-dest.left, // width of dest
						dest.bottom-dest.top, // height of dest
						bmpDC,
						0, // upper left source x
						0, // upper left source y
						SRCCOPY);
				}
				else{
					Func_2xSai((unsigned char *)&buffer_8bit_bouble[(m_widthdouble)], m_widthdouble, NULL, (unsigned char *)buffer, m_widthdouble, m_width, m_height);
					BitBlt(
						screenDC,
						0, // upper left dest x
						0, // upper left dest y
						dest.right-dest.left, // width of dest
						dest.bottom-dest.top, // height of dest
						bmpDC,
						get_viewable_area_x_offset()<<1, // upper left source x
						get_viewable_area_y_offset()<<1, // upper left source y
						SRCCOPY);
				}
				goto _Blt_Finally;
			}
			else
				doubleSizeBlowup();
		}

		BitBlt(
		    screenDC,
		    0, // upper left dest x
		    0, // upper left dest y
		    dest.right-dest.left, // width of dest
		    dest.bottom-dest.top, // height of dest
		    bmpDC,
		    get_viewable_area_x_offset(), // upper left source x
		    get_viewable_area_y_offset(), // upper left source y
		    SRCCOPY);

_Blt_Finally:
//		displaymessage();

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

void win32_windowed_NES_screen_mgr::clear(PIXEL color)
{
	PIXEL* p;

	p = buffer;
	for(uint32 i = 0; i < (get_height() << 1); i++)
	{
		memset(p, color, get_width()<<1);
		p += (get_width()<<1);
	}
}

boolean win32_windowed_NES_screen_mgr::set_palette(const uint8 pal[256][3])
{
	oldBmp = 0;
	bmpDC  = 0;
	palHan = 0;
	bmpDC  = 0;
	screenDC = 0;

	try {
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
		if(f_2xsai && mode_2xsai != 3){
			return TRUE;
		}
		screenDC = GetDC(window_handle);
		if(screenDC == NULL)
			throw "GetDC failed in win32_windowed_NES_screen_mgr::set_palette";

		palHan = CreatePalette((LOGPALETTE*)palInfo);
		if(!palHan)
			throw "CreatePalette failed in win32_windowed_NES_screen_mgr::set_palette";
		if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
			throw "SelectPalette failed in win32_windowed_NES_screen_mgr::set_palette";

		bmpDC = CreateCompatibleDC(screenDC);
		if(!bmpDC)
			throw "CreateCompatibleDC failed in win32_windowed_NES_screen_mgr::set_palette";
		oldBmp = SelectBitmap(bmpDC, hBmp);
		if(oldBmp == NULL)
			throw "SelectBitmap failed in win32_windowed_NES_screen_mgr::set_palette";

		if(!SetDIBColorTable(bmpDC, 0, 256, bmInfo->bmiPalette))
			throw "SetDIBColorTable failed in win32_windowed_NES_screen_mgr::set_palette";

		if(!AnimatePalette(palHan, 0, 256, &palInfo->palPalEntry[0]))
			if(!SetPaletteEntries(palHan, 0, 256, &palInfo->palPalEntry[0]))
				throw "SetPaletteEntries failed in win32_windowed_NES_screen_mgr::set_palette";

		if(RealizePalette(screenDC) == GDI_ERROR)
			throw "RealizePalette failed in win32_windowed_NES_screen_mgr::set_palette";
 
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

boolean win32_windowed_NES_screen_mgr::get_palette(uint8 pal[256][3])
{
	return FALSE;
}

boolean win32_windowed_NES_screen_mgr::set_palette_section(uint8 start, uint8 len, const uint8 pal[][3])
{
	oldBmp = 0;
	bmpDC  = 0;
	palHan = 0;
	bmpDC  = 0;
	screenDC = 0;

	try {
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
		if(f_2xsai && mode_2xsai != 3){
			return TRUE;
		}
		screenDC = GetDC(window_handle);
		if(screenDC == NULL)
			throw "GetDC failed in win32_windowed_NES_screen_mgr::set_palette_section";

		palHan = CreatePalette((LOGPALETTE*)palInfo);
		if(!palHan)
			throw "CreatePalette failed in win32_windowed_NES_screen_mgr::set_palette_section";
		if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
			throw "SelectPalette failed in win32_windowed_NES_screen_mgr::set_palette_section";

		bmpDC = CreateCompatibleDC(screenDC);
		if(!bmpDC)
			throw "CreateCompatibleDC failed in win32_windowed_NES_screen_mgr::set_palette_section";
		oldBmp = SelectBitmap(bmpDC, hBmp);
		if(oldBmp == NULL)
			throw "SelectBitmap failed in win32_windowed_NES_screen_mgr::set_palette_section";


		if(!SetDIBColorTable(bmpDC, 0, 256, bmInfo->bmiPalette))
			throw "SetDIBColorTable failed in win32_windowed_NES_screen_mgr::set_palette_section";

		if(!AnimatePalette(palHan, start, len, &palInfo->palPalEntry[start]))
			if(!SetPaletteEntries(palHan, start, len, &palInfo->palPalEntry[start]))
				throw "SetPaletteEntries failed in win32_windowed_NES_screen_mgr::set_palette_section";

		if(RealizePalette(screenDC) == GDI_ERROR)
			throw "RealizePalette failed in win32_windowed_NES_screen_mgr::set_palette_section";

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

boolean win32_windowed_NES_screen_mgr::get_palette_section(uint8 start, uint8 len, uint8 pal[][3])
{
	return FALSE;
}

void win32_windowed_NES_screen_mgr::assert_palette()
{
	oldBmp = 0;
	bmpDC  = 0;
	palHan = 0;
	bmpDC  = 0;
	screenDC = 0;

	try {
		set_NES_palette();
		if(f_2xsai && mode_2xsai != 3){
			return;
		}

		screenDC = GetDC(window_handle);
		if(screenDC == NULL)
			throw "GetDC failed in win32_windowed_NES_screen_mgr::assert_palette";

		palHan = CreatePalette((LOGPALETTE*)palInfo);
		if(!palHan)
			throw "CreatePalette failed in win32_windowed_NES_screen_mgr::assert_palette";

		if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
			throw "SelectPalette failed in win32_windowed_NES_screen_mgr::assert_palette";

		bmpDC = CreateCompatibleDC(screenDC);
		if(!bmpDC)
			throw "CreateCompatibleDC failed in win32_windowed_NES_screen_mgr::assert_palette";

		oldBmp = SelectBitmap(bmpDC, hBmp);
		if(oldBmp == NULL)
			throw "SelectBitmap failed in win32_windowed_NES_screen_mgr::assert_palette";

		if(!SetDIBColorTable(bmpDC, 0, 256, bmInfo->bmiPalette))
			throw "SetDIBColorTable failed in win32_windowed_NES_screen_mgr::assert_palette";

		if(!AnimatePalette(palHan, 0, 256, &palInfo->palPalEntry[0]))
			if(!SetPaletteEntries(palHan, 0, 256, &palInfo->palPalEntry[0]))
				throw "SetPaletteEntries failed in win32_windowed_NES_screen_mgr::set_palette_section";

		if(RealizePalette(screenDC) == GDI_ERROR)
			throw "RealizePalette failed in win32_windowed_NES_screen_mgr::assert_palette";
 
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

void win32_windowed_NES_screen_mgr::doubleSizeBlowup()
{
	if(blown_up) return;
/*
	/// test
	{
		uint8 *tmp = malloc(256*(240+16+2)+2);
		const uint32 viewable_height = get_viewable_height();
		const uint32 viewable_width = get_viewable_width();
		memset(tmp, 0, 256*(240+16+2)+2);

		const DWORD x_offset = get_viewable_area_x_offset();
		const DWORD y_offset = get_viewable_area_y_offset();
		const DWORD pitch = get_width() << 1;
		int w, h, i,j,k;
		unsigned char *sdt = NULL;
		w = get_viewable_width();
		h = get_viewable_height();
	  for(i=0,j=0,k=y_offset*pitch+x_offset; i<h; i++, j+=w, k+=pitch){
		  memcpy(&sdt[j], &buffer[k], w);
	  }
		Super2xSaI(tmp, viewable_width, NULL, buffer, viewable_width*2, viewable_width, get_viewable_height);
		free(tmp);
		return;
	}
*/

	unsigned char *d_base;
	unsigned char *s_base;

	uint32 i;
	const uint32 viewable_height = get_viewable_height();
	const uint32 viewable_width = get_viewable_width();
	const uint32 pitch = get_width()<<1;

	// source pointer
	s_base = &buffer[get_viewable_area_x_offset() +
	                 (pitch * (get_viewable_area_y_offset() + (viewable_height-1)))];

	// destination pointer
	d_base = &buffer[get_viewable_area_x_offset() + pitch +
	                 (pitch * (get_viewable_area_y_offset() + ( (viewable_height-1) << 1 ) ) )];

#if 1
	unsigned char *d;
	unsigned char *s;
	for(i = 0; i < viewable_height; i++)
	{
		// stretch the source line to the destination line
		s = s_base;
		d = d_base;
		for(int j = 0; j < viewable_width; j++)
		{
			*(d++) = *s;
			*(d++) = *s;
			s++;
		}
		// duplicate the stretched on the previous line
		memcpy(d_base-pitch, d_base, pitch);
		d_base -= pitch<<1;
		s_base -= pitch;
	}
#else
	for(i = 0; i < viewable_height; i++)
	{
		// stretch the source line to the destination line
		//    s = s_base;
		//    d = d_base;
		_asm {
			cld
			mov ecx,viewable_width
			mov esi,s_base
			mov edi,d_base
	loops:
			lodsb
			mov ah,al
			stosw
			loop loops
		}
		// duplicate the stretched on the previous line
		memcpy(d_base-pitch, d_base, pitch);
		d_base -= pitch<<1;
		s_base -= pitch;
	}
#endif
	blown_up = 1;
}

void win32_windowed_NES_screen_mgr::emulateTVScanline()
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


void win32_windowed_NES_screen_mgr::shot_screen( char *szFileName )
{
  const DWORD x_offset = get_viewable_area_x_offset();
  const DWORD y_offset = get_viewable_area_y_offset();
  const DWORD pitch = get_width() << 1;
  int w, h, i,j,k;
  unsigned char *sdt = NULL;
  PALETTEENTRY pal[256];
  unsigned char *tbuff;
  if(f_2xsai){
		if(mode_2xsai == 3){
			tbuff = buffer_8bit_bouble + (get_width() << 1);
		}
		else{
			tbuff = buffer_8bit_bouble;
		}
	}
  else
	  tbuff = buffer;
  w = get_viewable_width();
  h = get_viewable_height();
  sdt = (unsigned char *)malloc(w*h);
  if(sdt == NULL)
	  return;
  if(blown_up){
	  int dp=pitch<<1;
	  for(i=0,j=0,k=y_offset*pitch/*dp*/+x_offset/*+pitch*/; i<h; i++, k+=dp){
		  for(int x=0,dk=k; x<w; x++, dk+=2){
		  	  sdt[j++] = tbuff[dk];
		  }
	  }
  }
  else{
	  for(i=0,j=0,k=y_offset*pitch+x_offset; i<h; i++, j+=w, k+=pitch){
		  memcpy(&sdt[j], &tbuff[k], w);
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

void win32_windowed_NES_screen_mgr::nnnManuEnable(unsigned char){
}


void win32_windowed_NES_screen_mgr::EndEmu(){
}


void win32_windowed_NES_screen_mgr::StartEmu(){
}



int win32_windowed_NES_screen_mgr::start_avirec(char *fn){
	avi_rec = recavifilestart(fn, get_viewable_width(), get_viewable_height());
	return avi_rec;
}


void win32_windowed_NES_screen_mgr::frame_avirec(){
  avi_counter^=1;
  if(!avi_counter){
	  return;
  }
  const DWORD x_offset = get_viewable_area_x_offset();
  const DWORD y_offset = get_viewable_area_y_offset();
  const DWORD pitch = get_width() << 1;
  int w, h, i,j,k;
  unsigned char *sdt = NULL;
  unsigned char *tbuff;
  if(f_2xsai){
		if(mode_2xsai == 3){
			tbuff = buffer_8bit_bouble + (get_width() << 1);
		}
		else{
			tbuff = buffer_8bit_bouble;
		}
  }
  else
	  tbuff = buffer;
  w = get_viewable_width();
  h = get_viewable_height();
  sdt = (unsigned char *)malloc(w*h);
  if(sdt == NULL)
	  return;
  if(blown_up){
	  int dp=pitch<<1;
	  for(i=0,j=0,k=(y_offset+h-1)*dp+x_offset; i<h; i++, k-=dp){
		  for(int x=0,dk=k; x<w; x++, dk+=2){
		  	  sdt[j++] = tbuff[dk];
		  }
	  }
  }
  else{
	  for(i=0,j=0,k=(y_offset+h-1)*pitch+x_offset; i<h; i++, j+=w, k-=pitch){
		  memcpy(&sdt[j], &tbuff[k], w);
	  }
  }
  recavifilewriteflame(sdt);
  free(sdt);
}


void win32_windowed_NES_screen_mgr::end_avirec(){
	if(avi_rec)
		recavifilerelease();
}

