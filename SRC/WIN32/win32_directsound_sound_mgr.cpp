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
#include "win32_directsound_sound_mgr.h"
#include "debug.h"
#include "iDirectX.h"
#include "CSndRec.h"
#include <math.h>

win32_directsound_sound_mgr::win32_directsound_sound_mgr(HWND window_handle,
        int sample_bits, int sample_rate, int buffer_length_in_frames, int st, int Volume)
//		: sound_mgr(sample_bits, sample_rate, buffer_length_in_frames)
{
	stereo = st;
	rate =sample_rate;
	s_bits=sample_bits;
	buffer_length=buffer_length_in_frames;
	window = window_handle;

	if(!Volume)
		DSVolume = -10000;
	else{
		DSVolume = (int)(1000 * (float)log((float)Volume / 100.0));
	}

	if(st==-1){
		sound_mgr = new null_sound_mgr();
	}
	else if(st){
		sound_mgr = new win32_directsound_sound_mgr_stereo(window_handle,
						sample_bits, sample_rate, buffer_length_in_frames, DSVolume);
	}
	else{
		sound_mgr = new win32_directsound_sound_mgr_mono(window_handle,
						sample_bits, sample_rate, buffer_length_in_frames, DSVolume);
	}
}

win32_directsound_sound_mgr::~win32_directsound_sound_mgr()
{
	if(sound_mgr)
		delete sound_mgr;
}

void win32_directsound_sound_mgr::reset()
{
	sound_mgr->reset();
}

// lock down for a period of inactivity
void win32_directsound_sound_mgr::freeze()
{
	sound_mgr->freeze();
}

void win32_directsound_sound_mgr::thaw()
{
	sound_mgr->thaw();
}

void win32_directsound_sound_mgr::clear_buffer()
{
	sound_mgr->thaw();
}

boolean win32_directsound_sound_mgr::lock(sound_buf_pos which, void** buf, uint32* buf_len)
{
	return sound_mgr->lock(which, buf, buf_len);
}

void win32_directsound_sound_mgr::unlock()
{
	sound_mgr->unlock();
}

bool win32_directsound_sound_mgr::start_sndrec( char *fn, uint32 nSampleBits, uint32 nSampleRate )
{
	return sound_mgr->start_sndrec(fn, nSampleBits, nSampleRate );
}

void win32_directsound_sound_mgr::end_sndrec()
{
	sound_mgr->end_sndrec();
}

bool win32_directsound_sound_mgr::IsRecording()
{
	return sound_mgr->IsRecording();
}


// returns SOUND_BUF_LOW or SOUND_BUF_HIGH
sound_buf_pos win32_directsound_sound_mgr::get_currently_playing_half()
{
	return sound_mgr->get_currently_playing_half();
}


sound_mgr *win32_directsound_sound_mgr::set_null(){
	if(stereo==-1)
		return sound_mgr;
	stereo = -1;
	delete sound_mgr;
	sound_mgr = new null_sound_mgr();
	return sound_mgr;
}


sound_mgr *win32_directsound_sound_mgr::set_mono(){
	if(stereo==0)
		return sound_mgr;
	stereo = 0;
	delete sound_mgr;
	sound_mgr = new win32_directsound_sound_mgr_mono(window, s_bits, rate, buffer_length, DSVolume);
	return sound_mgr;
}


sound_mgr *win32_directsound_sound_mgr::set_stereo(){
	if(stereo==1)
		return sound_mgr;
	stereo = 1;
	delete sound_mgr;
	sound_mgr = new win32_directsound_sound_mgr_stereo(window, s_bits, rate, buffer_length, DSVolume);
	return sound_mgr;
}

