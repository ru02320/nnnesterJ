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

#ifndef _WIN32_EMU_H_
#define _WIN32_EMU_H_

#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include <process.h>

#include "emulator.h"
#include "NES.h"
#include "win32_NES_screen_mgr.h"
//#include "input_mgr.h"
#include "win32_directsound_sound_mgr.h"
#include "null_sound_mgr.h"
#include "NES_pad.h"
#include "win32_NES_pad.h"
#include "win32_EXT_key.h"			//nnn
#include "win32mic.h"			//nnn


class win32_emu : public emulator
{
public:
	win32_emu(HWND parent_window_handle, HINSTANCE parent_instance_handle, const char* ROM_name);
	~win32_emu();

	void PollInput(float);

	// emulator interface
	const char* getROMname();
	const char* getROMpath();
	boolean loadState(const char* fn);
	boolean saveState(const char* fn);

	void writeBaram(unsigned char* adr);		//Cheat
	void readBaram(unsigned char* adr);
	void ReadBoneb(unsigned int addr, unsigned char *data);
	void WriteBoneb(unsigned int addr, unsigned char data);

	void writeSraram(unsigned char* adr);
	void readSraram(unsigned char* adr);
	void ReadSroneb(unsigned int addr, unsigned char *data);
	void WriteSroneb(unsigned int addr, unsigned char data);

	uint32 GetMainMemp(uint8 **p){ return emu->GetMainMemp(p); };
	uint32 GetSramMemp(uint8 **p){ return emu->GetSramMemp(p); };
	uint32 GetExtMemp(uint8 **p){ return emu->GetExtMemp(p); };
	uint32 Get_PPUMemp(uint8 **p){ return emu->Get_PPUMemp(p); };
	void  emulate_CPU_cycles(float num_cycles){ emu->emulate_CPU_cycles(num_cycles); };

	void GetCPUContext(unsigned char *context){ emu->GetCPUContext(context);}
	void SetCPUContext(unsigned char *context){ emu->SetCPUContext(context);}

	int get_emutype(){ return EMUTYPE_NES; }

	void ExtKeyExe();
	unsigned char GetScreenMode();
	void SetScreenMode(uint8 mode){
		emu->SetScreenMode(mode);
		SetFrameSpeed(); }
	void nnnKailleraClient();	//nnn
	void nnnManuEnable(unsigned char);
	void GetROMInfoStr(char *wt);
	uint32 crc32(){return emu->crc32();}
	void SetFrameSpeed();
	void Load_Genie(){emu->Load_Genie();}
	void GetGameTitleName(char *s){emu->GetGameTitleName(s);}


	void reset(unsigned char);
	void freeze();
	void thaw();
	void do_frame();
	boolean frozen()  { return emu->frozen(); }

	// screen manager interface
	void blt();
	void flip();
	void assert_palette();
	//  boolean toggle_fullscreen();
	//  boolean is_fullscreen()      { return scr_mgr->is_fullscreen(); }

	// sound interface
	void enable_sound(boolean enable);
	boolean sound_enabled();
	boolean set_sample_rate(int sample_rate);
	int get_sample_rate();
	void sound_settings_changed();

	// called when input settings are changed
	void input_settings_changed();

	// output *.wav, written by Mikami Kana
	bool start_sndrec(char *wfn);
	void end_sndrec();
	bool IsRecording();
	void ToggleFastFPS();

	bool IsUserPause(){ return UserPause; };
	void SetUserPause(bool sw){ UserPause = sw; };

	//for Disk System
	uint8 GetDiskSideNum();
	uint8 GetDiskSide();
	void SetDiskSide(uint8 side);
	uint8 DiskAccessed();

	// for Expand Controllers
	void SetExControllerType(uint8 num);
	uint8 GetExControllerType();

	int StartFamilyBasicAutoInput(my_memread_stream *stream){ return emu->StartFamilyBasicAutoInput(stream); };

	// for Screen Shot /by mikami
	void shot_screen(char *);

	void SetBarcodeValue(uint32 value_low, uint32 value_high);
	void StopTape();
	void StartPlayTape(const char* fn);
	void StartRecTape(const char* fn);
	uint8 GetTapeStatus();

	// for Movie
	void StopMovie(){ emu->StopMovie();}
	void StartPlayMovie(const char* fn, unsigned char mflag){ emu->StartPlayMovie(fn, mflag); }
	void StartRecMovie(const char* fn, unsigned char mflag){ emu->StartRecMovie(fn, mflag); }
	void StartPlayToRecMovie(){ emu->StartPlayToRecMovie(); }
	uint8 GetMovieStatus(){ return emu->GetMovieStatus();}
	void InsertMovieMsg(char *msg){ emu->InsertMovieMsg(msg); }
	void DeleteMovieMsg(){ emu->DeleteMovieMsg(); }
	void MovieRecPause(){ emu->MovieRecPause(); }
	void MoviePreStateRec(){ emu->MoviePreStateRec(); }

	//  void SetScreenmgr();		//
	int start_avirec(char *fn){ return scr_mgr->start_avirec(fn); }
	void end_avirec(){ scr_mgr->end_avirec(); }
	int is_avirec(){ return scr_mgr->is_avirec(); }

	void StopAPULog(){ emu->StopAPULog(); };
	int StartAPULog(char *fn, char **pptext){ return emu->StartAPULog(fn, pptext);};
	boolean IsAPULog(){ return emu->IsAPULog(); };
	void ChangeRewindStatus(boolean b){ emu->ChangeRewindStatus(b); };


protected:
	HWND parent_wnd_handle;

	win32_NES_screen_mgr* scr_mgr;
	win32_directinput_input_mgr* inp_mgr;
	sound_mgr *snd_mgr;
	NES *emu;		//emulator* emu;
	NES_pad pad1;
	NES_pad pad2;
	NES_pad pad3;
	NES_pad pad4;
	unsigned char pad_bit[6];

	bool UserPause;

	// token, local null sound manager; always there
	null_sound_mgr local_null_snd_mgr;

	void CreateWin32Pads();
	void DeleteWin32Pads();
	win32_NES_pad* win32_pad1;
	win32_NES_pad* win32_pad2;
	win32_NES_pad* win32_pad3;
	win32_NES_pad* win32_pad4;
	win32_NES_button* win32_mic;
	win32_NES_button* win32_coin;

	void CreateWin32Extkey();
	void DeleteWin32Extkey();
	win32_EXT_key* win32_extkey;

	boolean emulate_frame(boolean draw);
//	boolean emulate_frame_debug(boolean draw);

	double frames_per_sec;
	double last_frame_time;
	double cur_time;


	// profiling vars
	uint32 frames_this_sec;
	double last_profile_sec_time;
	uint32 skipframes_this_sec;

	void reset_last_frame_time();

	double FramePeriod;
	BOOL IsFastFPS;

	BOOL recaviflag;

	//Movie
/*
	HANDLE hFile_Movie;
	uint8 movie_flag;
	uint8 movie_status;
*/
private:
};

#endif
