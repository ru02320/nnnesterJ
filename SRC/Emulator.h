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

#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <stdio.h>
#include "types.h"
#include "screen_mgr.h"
#include "sound_mgr.h"
#include "controller.h"
#include "mmemfile.h"

#include "emudef.h"

#define EMUTYPE_NES		0
#define EMUTYPE_GB		1
#define EMUTYPE_PCE		2
#define EMUTYPE_SNES	3


class emulator
{
public:
  emulator(const char* ROM_name = NULL) {};
  virtual ~emulator() {};

  virtual const char* getROMname() = 0; // returns ROM name without extension
  virtual const char* getROMpath() = 0;

  virtual boolean loadState(const char* fn) = 0;
  virtual boolean saveState(const char* fn) = 0;

  virtual void writeBaram(unsigned char* adr) = 0;
  virtual void readBaram(unsigned  char* adr) = 0;		//Cheat
  virtual void ReadBoneb(unsigned int addr, unsigned char *data) =0;
  virtual void WriteBoneb(unsigned int addr, unsigned char data) =0;
  virtual void writeSraram(unsigned char* adr) = 0;
  virtual void readSraram(unsigned  char* adr) = 0;
  virtual void ReadSroneb(unsigned int addr, unsigned char *data) =0;
  virtual void WriteSroneb(unsigned int addr, unsigned char data) =0;

  virtual uint32 GetMainMemp(uint8 **) =0;
  virtual uint32 GetSramMemp(uint8 **) =0;
  virtual uint32 GetExtMemp(uint8 **) =0;
  virtual uint32 Get_PPUMemp(uint8 **p) =0;
  virtual void  emulate_CPU_cycles(float num_cycles)=0;
  virtual void GetCPUContext(uint8 *context) = 0;
  virtual void SetCPUContext(uint8 *context) = 0;

  virtual unsigned char GetScreenMode()=0;
  virtual void SetScreenMode(uint8 )=0;
  virtual void nnnKailleraClient()=0;	//nnn
  virtual void GetROMInfoStr(char *wt)=0;
  virtual void GetGameTitleName(char *s)=0;
  virtual uint32 crc32()=0;
  virtual void Load_Genie()=0;

  virtual int start_avirec(char *fn)=0;
  virtual void end_avirec()=0;
  virtual int is_avirec()=0;

  virtual void do_frame()=0;
  virtual void ToggleFastFPS()=0;
  virtual void nnnManuEnable(unsigned char)=0;
  virtual bool start_sndrec(char *wfn)=0;
  virtual void end_sndrec()=0;
  virtual bool IsRecording()=0;
  virtual bool IsUserPause()=0;
  virtual void SetUserPause(bool sw)=0;

  virtual boolean emulate_frame(boolean draw) = 0;
//  virtual boolean emulate_frame_debug(boolean draw) = 0;

  virtual int get_emutype() = 0;

  virtual void SetFrameSpeed()=0;
  virtual void blt()=0;
  virtual void flip()=0;
  virtual void assert_palette()=0;
  virtual void ExtKeyExe()=0;

  virtual void reset(unsigned char) = 0;

  virtual void set_pad(unsigned char* c) {}
/*
  virtual void set_pad2(controller* c) {}
  virtual void set_pad3(controller* c) {}
  virtual void set_pad4(controller* c) {}
*/
//  virtual void set_micbits(unsigned char* c) {}

  virtual void input_settings_changed() {}
  virtual void shot_screen(char *)=0;

  // sound
  virtual void enable_sound(boolean enable) {};
  virtual boolean sound_enabled() { return FALSE; };
  virtual boolean set_sample_rate(int sample_rate) { return FALSE; };
  virtual int get_sample_rate() { return 0; };
  virtual void sound_settings_changed() {}

  // freeze() is called when the emulator should
  // shut down for a period of inactivity;
  virtual void freeze() = 0;
  // thaw() signals the end of the inactive period
  virtual void thaw()   = 0;

  virtual boolean frozen() = 0;

  // for Disk System
  virtual uint8 GetDiskSideNum() = 0;
  virtual uint8 GetDiskSide() = 0;
  virtual void SetDiskSide(uint8 side) = 0;
  virtual uint8 DiskAccessed() = 0;

  // for Expand Controllers
  virtual void SetExControllerType(uint8 num) = 0;
  virtual uint8 GetExControllerType() = 0;
  virtual void SetBarcodeValue(uint32 value_low, uint32 value_high) = 0;

  virtual void StopTape() = 0;
  virtual void StartPlayTape(const char* fn) = 0;
  virtual void StartRecTape(const char* fn) = 0;
  virtual uint8 GetTapeStatus() = 0;
  virtual int StartFamilyBasicAutoInput(my_memread_stream *stream){ return 0; };

  // for Movie
  virtual void StopMovie() = 0;
  virtual void StartPlayMovie(const char* fn, unsigned char mflag) = 0;
  virtual void StartRecMovie(const char* fn, unsigned char mflag) = 0;
  virtual uint8 GetMovieStatus() = 0;
  virtual void StartPlayToRecMovie()=0;
  virtual void InsertMovieMsg(char *)=0;
  virtual void DeleteMovieMsg()=0;
  virtual void MovieRecPause()=0;
  virtual void MoviePreStateRec()=0;

  virtual void StopAPULog(){};
  virtual int StartAPULog(char *fn, char **pptext){ return 0; };
  virtual boolean IsAPULog(){ return false; };
  virtual void ChangeRewindStatus(boolean b){ };


protected:
private:
};

#endif