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

#ifndef _WIN32_DIALOGS_H_
#define _WIN32_DIALOGS_H_

#include <windows.h>   // include important windows stuff
#include <windowsx.h>

BOOL CALLBACK PreferencesOptions_DlgProc(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK PreferencesOptions_DlgProc2(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK PreferencesOptions_DlgProc3(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);

BOOL CALLBACK GraphicsOptions_DlgProc(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK GraphicsOptions_DlgProc2(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK GraphicsOptions_DlgProc3(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);

BOOL CALLBACK SoundOptions_DlgProc(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SoundOptions_DlgProc2(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);


BOOL CALLBACK ControllersOptions_DlgProc1(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ControllersOptions_DlgProc2(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ControllersOptions_DlgProc3(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ControllersOptions_DlgProc4(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);


BOOL CALLBACK AboutNester_DlgProc(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK PathsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK ExtButtonControllersOptions_DlgProc(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ExtButtonControllersOptions_DlgProc2(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ControllersJoyTest_DlgProc(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam);

#endif
