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
#include <shlwapi.h>
#include <mmsystem.h>
#include "win32_emu.h"
#include "win32_directinput_input_mgr.h"
#include "win32_timing.h"
#include "mkutils.h"
#include "debug.h"
#include "NES_settings.h"
#include "win32_screen_mgr.h"

#include "savecfg.h"
#include "resource.h"
#include "extra_window.h"


extern void MyCtRestore();			//Cheat
extern void SocketClose();
extern int MovieAddedDialog();

//#include "recavi.h"

#include <dinput.h>

#define PROFILE

#define SPEED_THROTTLE

#define SPEED_THROTTLE_KEY  VK_ADD

// these read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

//char usedispfpsf;
extern int nnnKailleraFlag;
extern win32_screen_mgr* gscr_mgr;
extern win32_directinput_input_mgr* ginp_mgr;
extern win32_directsound_sound_mgr* g_snd_mgr;
extern uint8 netplay_status;
extern void InfoDisplayt(char *, unsigned int );
//extern struct Preview_state g_Preview_State;
//extern int g_PreviewMode;




#define NTSC_FRAMERATE (60000.0/1001.0)
//#define STD_FRAME_PERIOD   (1000.0/NTSC_FRAMERATE)
#define PAL_FRAMERATE  (50000.0/1001.0)

#define THROTTLE_SPEED  (NESTER_settings.nes.preferences.speed_throttling/* && KEY_UP(SPEED_THROTTLE_KEY)*/)
#define SKIP_FRAMES     (NESTER_settings.nes.preferences.auto_frameskip && THROTTLE_SPEED)

static double StdFramePeriodTime;
static double FastFramePeriodTime;


win32_emu::win32_emu(HWND parent_window_handle, HINSTANCE parent_instance_handle, const char* ROM_name)
{
	parent_wnd_handle = parent_window_handle;
	scr_mgr = NULL;
	inp_mgr = NULL;
	snd_mgr = &local_null_snd_mgr;
	emu = NULL;

	win32_pad1 = NULL;
	win32_pad2 = NULL;
	win32_pad3 = NULL;
	win32_pad4 = NULL;
	win32_mic  = NULL;
	win32_coin  = NULL;
	pad_bit[0]=pad_bit[1]=pad_bit[2]=pad_bit[3]=pad_bit[4]=pad_bit[5]=0;
	win32_extkey = NULL;		//nnn
	IsFastFPS = FALSE;
	UserPause = FALSE;
	SYS_TimeInit();

	frames_this_sec = 0;
	skipframes_this_sec = 0;
	recaviflag=0;


	try {
#if 0
		try {
			scr_mgr = new win32_NES_screen_mgr(parent_wnd_handle);
		} catch(...) {
			throw;// "error creating screen manager";
		}
#endif
		scr_mgr = gscr_mgr->Get_Nes_Screenmgr();
		if(scr_mgr==NULL)
			throw;// "";
#if 0
		try {
			inp_mgr = new win32_directinput_input_mgr(parent_wnd_handle, parent_instance_handle);
		} catch(...) {
			throw;// "error creating input manager";
		}
#endif
		inp_mgr=ginp_mgr;
		if(inp_mgr==NULL)
			throw;

		// get a null sound mgr
		snd_mgr = &local_null_snd_mgr;

		try {
			emu = new NES(ROM_name,scr_mgr,snd_mgr,parent_window_handle);
		} catch(...) {
			throw;// "error creating emulated NES";
		}

		SetFrameSpeed();

		scr_mgr->setParentNES((NES*)emu);
		scr_mgr->StartEmu();

		CreateWin32Pads();
		CreateWin32Extkey();

		// start the timer off right
		reset_last_frame_time();
		last_profile_sec_time=SYS_TimeInMilliseconds();

		// try to init dsound if appropriate
		enable_sound(NESTER_settings.nes.sound.enabled);

		// set up control pads
		emu->set_pad(pad_bit);

	} catch(...) {
		// careful of the order here
		DeleteWin32Extkey();
		DeleteWin32Pads();
		if(emu) delete emu;
		scr_mgr->EndEmu();
		//    if(scr_mgr) delete scr_mgr;
		//    if(inp_mgr) delete inp_mgr;
		if(snd_mgr != &local_null_snd_mgr){
			snd_mgr->freeze();
			snd_mgr->clear_buffer();
		}
		throw;
	}
}

win32_emu::~win32_emu()
{
	DeleteWin32Extkey();
	DeleteWin32Pads();
	if(emu) delete emu;
	if(scr_mgr){ scr_mgr->EndEmu(); scr_mgr->end_avirec(); }
	//  if(scr_mgr) delete scr_mgr;
	//	if(inp_mgr) delete inp_mgr;
	if(snd_mgr != &local_null_snd_mgr){
		snd_mgr->freeze();
		snd_mgr->clear_buffer();
		snd_mgr->end_sndrec();
	}
#if 0
	if(recaviflag){
		recavifilerelease();
	}
#endif
}

void win32_emu::PollInput(float gt)
{
#if 0
	// if we don't have the input focus, release all buttons
	if(GetForegroundWindow() != parent_wnd_handle)
	{
		pad1.release_all_buttons();
		pad2.release_all_buttons();
		pad3.release_all_buttons();
		pad4.release_all_buttons();
		return;
	}
#endif

	{
		inp_mgr->Poll();

		if(win32_pad1){
			win32_pad1->Poll(gt);
			pad_bit[0] = pad1.get_inp_state();
		}
		if(win32_pad2){
			win32_pad2->Poll(gt);
			pad_bit[1] = pad2.get_inp_state();
		}
		if(win32_pad3){ win32_pad3->Poll(gt);
			pad_bit[2] = pad3.get_inp_state();
		}
		if(win32_pad4){
			win32_pad4->Poll(gt);
			pad_bit[3] = pad4.get_inp_state();
		}
		if(win32_mic){
			win32_mic->Poll();
		}
		if(win32_coin){
			win32_coin->Poll();
		}
	}
}


void win32_emu::input_settings_changed()
{
	DeleteWin32Pads();
	CreateWin32Pads();
	DeleteWin32Extkey();			//nnn
	CreateWin32Extkey();
}

void win32_emu::CreateWin32Pads()
{
	win32_directinput_input_mgr* win32_inp_mgr;

	DeleteWin32Pads();

	win32_inp_mgr = (win32_directinput_input_mgr*)inp_mgr; // naughty

	try {
		win32_pad1 = new win32_NES_pad(&NESTER_settings.nes.input.player1, &pad1, win32_inp_mgr);
	} catch(const char* IFDEBUG(s)) {
		LOG("Error creating pad 1 - " << s << endl);
		win32_pad1 = NULL;
	}

	try {
		win32_pad2 = new win32_NES_pad(&NESTER_settings.nes.input.player2, &pad2, win32_inp_mgr);
	} catch(const char* IFDEBUG(s)) {
		LOG("Error creating pad 2 - " << s << endl);
		win32_pad2 = NULL;
	}

	try {
		win32_pad3 = new win32_NES_pad(&NESTER_settings.nes.input.player3, &pad3, win32_inp_mgr);
	} catch(const char* IFDEBUG(s)) {
		LOG("Error creating pad 3 - " << s << endl);
		win32_pad3 = NULL;
	}

	try {
		win32_pad4 = new win32_NES_pad(&NESTER_settings.nes.input.player4, &pad4, win32_inp_mgr);
	} catch(const char* IFDEBUG(s)) {
		LOG("Error creating pad 4 - " << s << endl);
		win32_pad4 = NULL;
	}
	try {
		win32_mic = new win32_NES_button(&NESTER_settings.nes.input.MicButton, &pad_bit[4], win32_inp_mgr);
	} catch(const char* IFDEBUG(s)) {
		LOG("Error creating mic - " << s << endl);
		win32_mic = NULL;
	}
	try {
		win32_coin = new win32_NES_button(&NESTER_settings.nes.input.extkeycfg[8], &pad_bit[5], win32_inp_mgr);
	} catch(const char* IFDEBUG(s)) {
		LOG("Error creating coin button - " << s << endl);
		win32_coin = NULL;
	}
}

void win32_emu::DeleteWin32Pads()
{
	if(win32_pad1)
	{
		delete win32_pad1;
		win32_pad1 = NULL;
	}
	if(win32_pad2)
	{
		delete win32_pad2;
		win32_pad2 = NULL;
	}
	if(win32_pad3)
	{
		delete win32_pad3;
		win32_pad3 = NULL;
	}
	if(win32_pad4)
	{
		delete win32_pad4;
		win32_pad4 = NULL;
	}
	if(win32_mic){
		delete win32_mic;
		win32_mic = NULL;
	}
	if(win32_coin){
		delete win32_coin;
		win32_coin = NULL;
	}
}

//nnn
void win32_emu::CreateWin32Extkey()
{
	win32_directinput_input_mgr* win32_inp_mgr;

	DeleteWin32Extkey();

	win32_inp_mgr = (win32_directinput_input_mgr*)inp_mgr; // naughty

	try {
		win32_extkey = new win32_EXT_key(NESTER_settings.nes.input.extkeycfg, win32_inp_mgr);
	} catch(const char* IFDEBUG(s)) {
		LOG("Error creating extkey - " << s << endl);
		win32_extkey = NULL;
	}
}

void win32_emu::DeleteWin32Extkey()
{
	if(win32_extkey)
	{
		delete win32_extkey;
		win32_extkey = NULL;
	}
}

boolean win32_emu::emulate_frame(boolean draw)
{
	return emu->emulate_frame(draw);
}


void win32_emu::freeze()
{
	if(!frozen()) emu->freeze();
}

void win32_emu::thaw()
{
	if(frozen())
	{
		emu->thaw();
		reset_last_frame_time();
	}
}

void win32_emu::reset_last_frame_time()
{
	last_frame_time = SYS_TimeInMilliseconds();

#ifdef PROFILE
	last_profile_sec_time = cur_time;
	frames_this_sec = 0;
#endif

}

const char* win32_emu::getROMname()
{
	const char* name;

	name = emu->getROMname();

	return name;
}

const char* win32_emu::getROMpath()
{
	const char* path;

	path = emu->getROMpath();

	return path;
}

boolean win32_emu::loadState(const char* fn)
{
	boolean result;

	freeze();
	result = emu->loadState(fn);
	thaw();

	return result;
}

boolean win32_emu::saveState(const char* fn)
{
	boolean result;

	freeze();
	result = emu->saveState(fn);
	thaw();

	return result;
}

void win32_emu::reset(unsigned char sr)
{
	freeze();
	emu->reset(sr);
	thaw();
}

void win32_emu::blt()
{
	scr_mgr->blt();
}

void win32_emu::flip()
{
	scr_mgr->flip();
}

void win32_emu::assert_palette()
{
	scr_mgr->assert_palette();
}

/*
boolean win32_emu::toggle_fullscreen()
{
  return scr_mgr->toggle_fullscreen();
}
*/
void win32_emu::sound_settings_changed()
{
	snd_mgr = &local_null_snd_mgr;
	enable_sound(NESTER_settings.nes.sound.enabled);
}


void win32_emu::enable_sound(boolean enable)
{
	freeze();

	if(snd_mgr != &local_null_snd_mgr)
	{
		snd_mgr->freeze();
		snd_mgr = &local_null_snd_mgr;
	}

	if(enable)
	{
		snd_mgr = g_snd_mgr->set_mono();
	}

	((NES*)emu)->new_snd_mgr(snd_mgr);
	snd_mgr->thaw();

	thaw();
}

boolean win32_emu::sound_enabled()
{
	return !snd_mgr->IsNull();
}

boolean win32_emu::set_sample_rate(int sample_rate)
{
	if(!sound_enabled()) return FALSE;
	if(get_sample_rate() == sample_rate) return TRUE;
	return TRUE;
}

int win32_emu::get_sample_rate()
{
	return snd_mgr->get_sample_rate();
}

// STATIC FUNCTIONS
#if 1
static inline void SleepUntil(long time)
{
	if(!NESTER_settings.nes.preferences.NotUseSleep){
//		timeBeginPeriod(1);
		long timeleft = time - (long)(SYS_TimeInMilliseconds());
		if( timeleft > 2)			//if( timeleft > 2)
		{
			Sleep( timeleft - 1 );
		}
		while( time - (long)(SYS_TimeInMilliseconds()) > 0 )
			Sleep(0);
//		timeEndPeriod(1);
	}
	else{
//		MSG      msg;
		while(1){
			long timeleft = time - (long)(SYS_TimeInMilliseconds());
			if(timeleft <= 1)
				break;
/*
			if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
				if(msg.message == WM_QUIT)
					return;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
*/
		}
	}
}

#else
static inline void SleepUntil(long time)
{
	long timeleft;

	while(1)
	{
		timeleft = time - long(SYS_TimeInMilliseconds());
		if(timeleft <= 0) break;

		if(timeleft > 2)
		{
			Sleep((timeleft) - 1);
		}
	}
}
#endif
/*
When the NTSC standard was designed, certain frequencies involved
in the color subcarrier were interfering with the 60 Hz power lines.  So
the NTSC engineers set the framerate to 60000/1001 Hz.  See also
"drop frame timecode" on any search engine for the full story.
*/


void win32_emu::do_frame()
{
	uint32 frames_since_last, i, skip_frameflag, kailleraf=0;

	if(frozen()) return;

	// skip frames while disk accessed
	while(!NESTER_settings.nes.preferences.NotUseFDSDiskASkip && DiskAccessed())
	{
		emulate_frame(FALSE);
		last_frame_time = cur_time = SYS_TimeInMilliseconds();
		skipframes_this_sec++;
	}

	// at this point, last_frame_time is set to the time when the last frame was drawn.

	// get the current time
	cur_time = SYS_TimeInMilliseconds();

	skip_frameflag = (SKIP_FRAMES && !nnnKailleraFlag && !netplay_status);
	// make up for missed frames
//	if(SKIP_FRAMES && !nnnKailleraFlag && !netplay_status)  //&&!(IsFastFPS)
	{
		if(IsFastFPS){
			if(NESTER_settings.nes.preferences.FastSpeedState==0){		//FPS
				FramePeriod = FastFramePeriodTime;
				frames_since_last = (uint32)((cur_time - last_frame_time) / FramePeriod );
			}
			else if(NESTER_settings.nes.preferences.FastSpeedState==1){	//SkipFrame
				FramePeriod = FastFramePeriodTime;
				if(skip_frameflag)
				{
					for(i=0;i<NESTER_settings.nes.preferences.FastSkipFrame;++i){
						emulate_frame(FALSE);
						skipframes_this_sec++;
					}
				}
				frames_since_last = 0;
			}
			else{
				frames_since_last = 0;
			}
		}
		else if(NESTER_settings.nes.preferences.StdSpeedState==1 ||NESTER_settings.nes.preferences.StdSpeedState==0){		//FPS
			FramePeriod = StdFramePeriodTime;
			frames_since_last = (uint32)((cur_time - last_frame_time) / FramePeriod );
		}
		else if(NESTER_settings.nes.preferences.StdSpeedState==2){		//SkipFrame
			FramePeriod = StdFramePeriodTime;
			if(skip_frameflag)
			{
				for(i=0;i<NESTER_settings.nes.preferences.StdSkipFrame;++i){
					emulate_frame(FALSE);
					skipframes_this_sec++;
				}
			}
			frames_since_last = 0;
		}

		// are there extra frames?
		if(skip_frameflag){
			if(frames_since_last > 1)
			{
				for(i = 1; i < frames_since_last; i++)
				{
					last_frame_time += FramePeriod;
					emulate_frame(FALSE);
					skipframes_this_sec++;
				}
			}
		}
	}

	//  nnnKailleraClient();

	//  MyCtRestore();			//Cheat

	PollInput((float)cur_time);
	// emulate current frame
	if(win32_extkey->Buttonp[0x16]!=1)
		emulate_frame(TRUE);

	if(nnnKailleraFlag){
		uint32 n_time = SYS_TimeInMilliseconds();
		if(last_frame_time + FramePeriod < n_time)
			kailleraf = 1;
	}

	// sleep until this frame's target time
	if(THROTTLE_SPEED)	//THROTTLE_SPEED && !(IsFastFPS)
	{
		if(!(IsFastFPS && NESTER_settings.nes.preferences.FastSpeedState==2))
			SleepUntil(long(last_frame_time+ FramePeriod ));
	}

	// draw frame
	blt();
	flip();

	if(NESTER_settings.nes.preferences.ShowFPS){
		frames_this_sec++;
		if((cur_time - last_profile_sec_time) > (2.0*1000.0)){
			char str[64];
			float cfps, vfps;
			cfps=(float)(frames_this_sec * (1000.0/((double)cur_time - (double)last_profile_sec_time)));
			vfps=(float)((skipframes_this_sec+frames_this_sec) * (1000.0/((double)cur_time - (double)last_profile_sec_time)));
			sprintf(str, "FPS : %5.2f , RFPS : %5.2f", cfps, vfps);
			InfoDisplayt(str, 2000);
			frames_this_sec = 0;
			skipframes_this_sec =0;
			last_profile_sec_time = cur_time;
		}
	}

	// get ready for next frame
	if(nnnKailleraFlag){
		if(!kailleraf)
			last_frame_time += FramePeriod;
		else
			last_frame_time = cur_time;
	}
	else if(THROTTLE_SPEED /*&& !nnnKailleraFlag*/)	//THROTTLE_SPEED   && !(IsFastFPS)
	{
//		if(!nnnKailleraFlag && !netplay_status)
			last_frame_time += FramePeriod;
/*		else{
			DWORD lcur_time = SYS_TimeInMilliseconds();
			uint32 o = (uint32)((lcur_time - (last_frame_time+FramePeriod)) / FramePeriod );
			if(o > 1){
				last_frame_time  = lcur_time;
			}
			else
				last_frame_time += FramePeriod;
		}
*/
	}
	else
	{
		last_frame_time = cur_time;
	}
	ExtKeyExe();			//Ext Key Function
//	RunFrame_ExtraWindow();
}

// added by Mikami Kana
void win32_emu::shot_screen(char *sfn)
{
	char fn[MAX_PATH];
	if( NESTER_settings.path.UseShotPath )
	{
		strcpy( fn, NESTER_settings.path.szShotPath );
		PathAddBackslash( fn );
	}
	else
		strcpy( fn, getROMpath() );

	if( GetFileAttributes( fn ) == 0xFFFFFFFF )
		MKCreateDirectories( fn );

	strcat( fn, getROMname() );
	int p = strlen(fn);
	for( int i=0; i<=10000; i++ )
	{
		if(NESTER_settings.nes.graphics.ScreenShotUseBmp){
			sprintf( fn + p, "%04d.bmp", i );
		}
		else{
			sprintf( fn + p, "%04d.png", i );
		}
		if( GetFileAttributes( fn ) == 0xFFFFFFFF )
			break;
	}
	if( i >= 10000 )
		return;

	scr_mgr->shot_screen( fn );
	strcpy(sfn, PathFindFileName(fn));
}

// added by Mikami Kana
bool win32_emu::start_sndrec(char *wfn)
{
	if( snd_mgr == &local_null_snd_mgr )
		return FALSE;
	if(wfn!=NULL){
		return snd_mgr->start_sndrec(wfn, NESTER_settings.nes.sound.sample_bits, NESTER_settings.nes.sound.sample_rate );
	}
	char fn[MAX_PATH];
	if( NESTER_settings.path.UseWavePath )
	{
		strcpy( fn, NESTER_settings.path.szWavePath );
		PathAddBackslash( fn );
	}
	else
		strcpy( fn, getROMpath() );

	if( GetFileAttributes( fn ) == 0xFFFFFFFF )
		MKCreateDirectories( fn );

	strcat( fn, getROMname() );
	int p = strlen(fn);
	for( int i=0; i<=100; i++ )
	{
		sprintf( fn + p, "%02d.wav", i );
		if( GetFileAttributes( fn ) == 0xFFFFFFFF )
			break;
	}
	if( i>=100 )
		return FALSE;
	bool result = snd_mgr->start_sndrec(
	                  fn,
	                  NESTER_settings.nes.sound.sample_bits,
	                  NESTER_settings.nes.sound.sample_rate );
	return result;
}

// added by Mikami Kana
void win32_emu::end_sndrec()
{
	snd_mgr->end_sndrec();
}

bool win32_emu::IsRecording()
{
	return snd_mgr->IsRecording();
}

void win32_emu::ToggleFastFPS()
{
	//	if(SKIP_FRAMES)
	{
		if(IsFastFPS){
			last_frame_time = SYS_TimeInMilliseconds();
		}
		IsFastFPS = ~IsFastFPS;
	}
}

uint8 win32_emu::GetDiskSideNum()
{
	return emu->GetDiskSideNum();
}

uint8 win32_emu::GetDiskSide()
{
	return emu->GetDiskSide();
}

void win32_emu::SetDiskSide(uint8 side)
{
	emu->SetDiskSide(side);
}

uint8 win32_emu::DiskAccessed()
{
	return emu->DiskAccessed();
}

void win32_emu::SetExControllerType(uint8 num)
{
	emu->SetExControllerType(num);
}
uint8 win32_emu::GetExControllerType()
{
	return emu->GetExControllerType();
}
void win32_emu::SetBarcodeValue(uint32 value_low, uint32 value_high)
{
	emu->SetBarcodeValue(value_low, value_high);
}
void win32_emu::StopTape()
{
	emu->StopTape();
}
void win32_emu::StartPlayTape(const char* fn)
{
	emu->StartPlayTape(fn);
}
void win32_emu::StartRecTape(const char* fn)
{
	emu->StartRecTape(fn);
}
uint8 win32_emu::GetTapeStatus()
{
	return emu->GetTapeStatus();
}




//Cheat
void win32_emu::readBaram(unsigned char* adr)
{
	emu->readBaram(adr);
	return;
}


void win32_emu::writeBaram(unsigned char* adr)
{
	emu->writeBaram(adr);
	return;
}


void win32_emu::ReadBoneb(unsigned int addr, unsigned char *data){
	emu->ReadBoneb(addr, data);
	return;
}


void win32_emu::WriteBoneb(unsigned int addr, unsigned char data){
	emu->WriteBoneb(addr, data);
	return;
}



//Sram
void win32_emu::readSraram(unsigned char* adr){
	emu->readSraram(adr);
}

void win32_emu::writeSraram(unsigned char* adr){
	emu->writeSraram(adr);
}

void win32_emu::ReadSroneb(unsigned int addr, unsigned char *data){
	emu->ReadSroneb(addr, data);
}


void win32_emu::WriteSroneb(unsigned int addr, unsigned char data){
	emu->WriteSroneb(addr, data);
}


void win32_emu::GetROMInfoStr(char *wt){
	emu->GetROMInfoStr(wt);
}


void win32_emu::ExtKeyExe(){
	if(!win32_extkey)
		return;
	int32 cur_ctrl = emu->GetExControllerType();
	int32 use_key = cur_ctrl == EX_FAMILY_KEYBOARD || cur_ctrl == EX_DOREMIKKO_KEYBOARD;
	win32_extkey->Poll();
	if(win32_extkey->Buttonp[0]==1){	//soft reset
		if(!(use_key && win32_extkey->ExtButton[0].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_SOFTRESET, 0);
		}
		win32_extkey->Buttonp[0]=0;
	}
	else if(win32_extkey->Buttonp[0]==2)
		win32_extkey->Buttonp[0]=0;
	if(win32_extkey->Buttonp[1]==1){		//Hard Reset
		if(!(use_key && win32_extkey->ExtButton[1].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_RESET, 0);
		}
		win32_extkey->Buttonp[1]=0;
	}
	else if(win32_extkey->Buttonp[1]==2)
		win32_extkey->Buttonp[1]=0;
	if(win32_extkey->Buttonp[2]==1){		// FDS side change
		if(!(use_key && win32_extkey->ExtButton[2].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_USEEXTKEY4, 0);
		}
		win32_extkey->Buttonp[2]=0;
	}
	else if(win32_extkey->Buttonp[2]==2)
		win32_extkey->Buttonp[2]=0;
	if(win32_extkey->Buttonp[3]==1){
		if(!(use_key && win32_extkey->ExtButton[3].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_DISK_1A, 0);
		}
		win32_extkey->Buttonp[3]=0;
	}
	else if(win32_extkey->Buttonp[3]==2)
		win32_extkey->Buttonp[3]=0;
	if(win32_extkey->Buttonp[4]==1){
		if(!(use_key && win32_extkey->ExtButton[4].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_DISK_1B, 0);
		}
		win32_extkey->Buttonp[4]=0;
	}
	else if(win32_extkey->Buttonp[4]==2)
		win32_extkey->Buttonp[4]=0;
	if(win32_extkey->Buttonp[5]==1){
		if(!(use_key && win32_extkey->ExtButton[5].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_DISK_2A, 0);
		}
		win32_extkey->Buttonp[5]=0;
	}
	else if(win32_extkey->Buttonp[5]==2)
		win32_extkey->Buttonp[5]=0;
	if(win32_extkey->Buttonp[6]==1){
		if(!(use_key && win32_extkey->ExtButton[6].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_DISK_2B, 0);
		}
		win32_extkey->Buttonp[6]=0;
	}
	else if(win32_extkey->Buttonp[6]==2)
		win32_extkey->Buttonp[6]=0;
	if(win32_extkey->Buttonp[7]==1){
		if(!(use_key && win32_extkey->ExtButton[7].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_DISK_EJECT, 0);
		}
		win32_extkey->Buttonp[7]=0;
	}
	else if(win32_extkey->Buttonp[7]==2)
		win32_extkey->Buttonp[7]=0;

	if(win32_extkey->Buttonp[9]==1){
		if(!(use_key && win32_extkey->ExtButton[8].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_USEEXTKEY1, 0);
		}
		win32_extkey->Buttonp[9]=0;
	}
	else if(win32_extkey->Buttonp[9]==2){
		if(!(use_key && win32_extkey->ExtButton[9].type == D_KEYBOARD_KEY)){
			if(!NESTER_settings.nes.input.extkeycfgFastFPSToggle)
				PostMessage(parent_wnd_handle, WM_COMMAND, ID_USEEXTKEY1, 0);
		}
		win32_extkey->Buttonp[9]=0;
	}
	if(win32_extkey->Buttonp[10]==1){		//Fast Speed (Toggle)
		if(!(use_key && win32_extkey->ExtButton[10].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_USEEXTKEY1, 0);
		}
		win32_extkey->Buttonp[10]=0;
	}
	else if(win32_extkey->Buttonp[0x10]==2)
		win32_extkey->Buttonp[10]=0;
	if(win32_extkey->Buttonp[11]==1){		//Pause
		if(!(use_key && win32_extkey->ExtButton[11].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_USEEXTKEY5, 0);
		}
		win32_extkey->Buttonp[11]=0;
	}
	else if(win32_extkey->Buttonp[11]==2)
		win32_extkey->Buttonp[11]=0;
	if(win32_extkey->Buttonp[12]==1){
		if(!(use_key && win32_extkey->ExtButton[12].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_SCREENSHOT, 0);
		}
		win32_extkey->Buttonp[12]=0;
	}
	else if(win32_extkey->Buttonp[12]==2)
		win32_extkey->Buttonp[12]=0;
	if(win32_extkey->Buttonp[13]==1){		//Screen Shot (While pushing)
		if(!(use_key && win32_extkey->ExtButton[13].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_SCREENSHOT, 0);
		}
	}
	else if(win32_extkey->Buttonp[13]==2)
		win32_extkey->Buttonp[13]=0;
	if(win32_extkey->Buttonp[14]==1){
		if(!(use_key && win32_extkey->ExtButton[14].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_SNDREC, 0);
		}
		win32_extkey->Buttonp[14]=0;
	}
	else if(win32_extkey->Buttonp[14]==2)
		win32_extkey->Buttonp[14]=0;
	if(win32_extkey->Buttonp[15]==1){
		if(!(use_key && win32_extkey->ExtButton[15].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_OPTIONS_FULLSCREEN, 0);
		}
		win32_extkey->Buttonp[15]=0;
	}
	else if(win32_extkey->Buttonp[15]==2)
		win32_extkey->Buttonp[15]=0;

	if(win32_extkey->Buttonp[0x10]==1){
		if(!(use_key && win32_extkey->ExtButton[0x10].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_QUICK_SAVE, 0);
		}
		win32_extkey->Buttonp[0x10]=0;
	}
	else if(win32_extkey->Buttonp[0x10]==2)
		win32_extkey->Buttonp[0x10]=0;
	if(win32_extkey->Buttonp[0x11]==1){
		if(!(use_key && win32_extkey->ExtButton[0x11].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_QUICK_LOAD, 0);
		}
		win32_extkey->Buttonp[0x11]=0;
	}
	else if(win32_extkey->Buttonp[0x11]==2)
		win32_extkey->Buttonp[0x11]=0;
	if(win32_extkey->Buttonp[0x12]==1){
		if(!(use_key && win32_extkey->ExtButton[0x12].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_USEEXTKEY2, 0);
		}
		win32_extkey->Buttonp[0x12]=0;
	}
	else if(win32_extkey->Buttonp[0x12]==2)
		win32_extkey->Buttonp[0x12]=0;
	if(win32_extkey->Buttonp[0x13]==1){
		if(!(use_key && win32_extkey->ExtButton[0x13].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_USEEXTKEY3, 0);
		}
		win32_extkey->Buttonp[0x13]=0;
	}
	else if(win32_extkey->Buttonp[0x13]==2)
		win32_extkey->Buttonp[0x13]=0;

	if(win32_extkey->Buttonp[0x14]==1){		//FPS Display(Toggle)
		if(!(use_key && win32_extkey->ExtButton[0x14].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_CHEATFPS, 0);
		}
		win32_extkey->Buttonp[0x14]=0;
	}
	else if(win32_extkey->Buttonp[0x14]==2)
		win32_extkey->Buttonp[0x14]=0;
	if(win32_extkey->Buttonp[0x15]==1){		//Cheat Apply Flag reverse(Toggle)
		if(!(use_key && win32_extkey->ExtButton[0x15].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_CHEAP_CHEAT2, 0);
		}
		win32_extkey->Buttonp[0x15]=0;
	}
	else if(win32_extkey->Buttonp[0x15]==2)
		win32_extkey->Buttonp[0x15]=0;
	if(win32_extkey->Buttonp[0x16]==1){		//Rewind
/*		if(RewindFirst == FALSE){
			RewindFirst = TRUE;
//			freeze();
		}*/
		if(!(use_key && win32_extkey->ExtButton[0x16].type == D_KEYBOARD_KEY)){
			emu->Rewind(1);
//			blt();
//			flip();
		}
	}
	else if(win32_extkey->Buttonp[0x16]==2){
		win32_extkey->Buttonp[0x16]=0;
//		RewindFirst=FALSE;
//		thaw();
	}
	if(win32_extkey->Buttonp[0x17]==1){		//Movie Stop.
		if(!(use_key && win32_extkey->ExtButton[0x17].type == D_KEYBOARD_KEY)){
			PostMessage(parent_wnd_handle, WM_COMMAND, ID_FILE_MOVIE_STOP, 0);
		}
		win32_extkey->Buttonp[0x17]=0;
	}
	else if(win32_extkey->Buttonp[0x17]==2)
		win32_extkey->Buttonp[0x17]=0;
	return;
}


unsigned char win32_emu::GetScreenMode(){
	return emu->GetScreenMode();
}

void win32_emu::nnnKailleraClient(){	//nnn
	emu->nnnKailleraClient();
	return;
}

void win32_emu::nnnManuEnable(unsigned char flag){
	scr_mgr->nnnManuEnable(flag);
}

/*
void win32_emu::SetScreenmgr(){
	scr_mgr->SetScreenmgr();
}
*/

void win32_emu::SetFrameSpeed(){
	if(NESTER_settings.nes.preferences.StdSpeedState==0){
		if(1==emu->GetScreenMode()){
			StdFramePeriodTime = 1000.0/NTSC_FRAMERATE;
		}
		else{
			StdFramePeriodTime = 1000.0/PAL_FRAMERATE;
		}
	}
	else if(NESTER_settings.nes.preferences.StdSpeedState==1){
		StdFramePeriodTime = 1000.0/NESTER_settings.nes.preferences.StdFPS;
	}
	else{
		float tf;
		if(1==emu->GetScreenMode())
			tf = (float)NTSC_FRAMERATE;
		else
			tf = (float)PAL_FRAMERATE;
		StdFramePeriodTime = 1000.0/((NESTER_settings.nes.preferences.StdSkipFrame+1)*tf);
	}
	if(NESTER_settings.nes.preferences.FastSpeedState==0){
		FastFramePeriodTime= 1000.0/NESTER_settings.nes.preferences.FastFPS;
	}
	else if(NESTER_settings.nes.preferences.FastSpeedState==1){
		float tf;
		if(1==emu->GetScreenMode())
			tf = (float)NTSC_FRAMERATE;
		else
			tf = (float)PAL_FRAMERATE;
		FastFramePeriodTime = 1000.0/((NESTER_settings.nes.preferences.FastSkipFrame+1)*tf);
	}
	else{
		FastFramePeriodTime = 0;
	}
}
