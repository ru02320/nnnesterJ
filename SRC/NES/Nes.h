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

#ifndef _NES_H_
#define _NES_H_

#include <stdio.h>

#include "types.h"
#include "emulator.h"
#include "NES_CPU.h"
#include "NES_6502.h"
#include "NES_6502ASM.h"
#include "NES_mapper.h"
#include "NES_ROM.h"
#include "NES_PPU.h"
#include "NES_APU_wrapper.h"
#include "NES_pad.h"
#include "NES_settings.h"

#include "NES_external_device.h"
#include "NES_Rewind.h"

#include "libsnss.h"

#include "mmemfile.h"


// External Device
#define EX_NONE                   0
#define EX_ARKANOID_PADDLE        2
#define EX_CRAZY_CLIMBER          3
#define EX_DATACH_BARCODE_BATTLER 4
#define EX_DOREMIKKO_KEYBOARD     5
#define EX_EXCITING_BOXING        6
#define EX_FAMILY_KEYBOARD        7
#define EX_FAMILY_TRAINER_A       8
#define EX_FAMILY_TRAINER_B       9
#define EX_HYPER_SHOT             10
#define EX_MAHJONG                11
#define EX_OEKAKIDS_TABLET        12
#define EX_OPTICAL_GUN            13
#define EX_POKKUN_MOGURAA         14
#define EX_POWER_PAD_A            15
#define EX_POWER_PAD_B            16
#define EX_SPACE_SHADOW_GUN       17
#define EX_TOP_RIDER              18
#define EX_TURBO_FILE             19
#define EX_VS_ZAPPER              20


class NES_screen_mgr;

class NES //: public emulator
{
	// friend classes
	friend NES_screen_mgr;
	friend NES_6502;
	friend NES_6502ASM;
	friend NES_CPU;

	friend NES_PPU;
	friend NES_APU;
	friend NES_mapper;
	friend NES_mapper1;
	friend NES_mapper4;
	friend NES_mapper5;
	friend NES_mapper6;
	friend NES_mapper13;
	friend NES_mapper16;
	friend NES_mapper17;
	friend NES_mapper18;
	friend NES_mapper19;
	friend NES_mapper20;
	friend NES_mapper21;
	friend NES_mapper23;
	friend NES_mapper24;
	friend NES_mapper25;
	friend NES_mapper26;
	friend NES_mapper33;
	friend NES_mapper40;
	friend NES_mapper42;
	friend NES_mapper43;
	friend NES_mapper44;
	friend NES_mapper45;
	friend NES_mapper47;
	friend NES_mapper48;
	friend NES_mapper49;
	friend NES_mapper50;
	friend NES_mapper51;
	friend NES_mapper52;
	friend NES_mapper64;
	friend NES_mapper65;
	friend NES_mapper67;
	friend NES_mapper69;
	friend NES_mapper73;
	friend NES_mapper77;
	friend NES_mapper80;
	friend NES_mapper83;
	friend NES_mapper85;
	friend NES_mapper90;
	friend NES_mapper91;
	friend NES_mapper95;
	friend NES_mapper96;
	friend NES_mapper100;
	friend NES_mapper105;
	friend NES_mapper112;
	friend NES_mapper115;
	friend NES_mapper117;
	friend NES_mapper118;
	friend NES_mapper119;
	friend NES_mapper155;
	friend NES_mapper160;
	friend NES_mapper182;
	friend NES_mapper183;
	friend NES_mapper185;
	friend NES_mapper187;
	friend NES_mapper188;
	friend NES_mapper189;
	friend NES_mapper211;
	friend NES_mapper234;
	friend NES_mapper235;
	friend NES_mapper237;
	friend NES_mapper245;
	friend NES_mapper246;
	friend NES_mapper248;
	friend NES_mapper254;
	friend NES_mapperNSF;
	friend NES_mapperMMC3;


	friend NES_UNIFmapper2;
	friend NES_UNIFmapper4;

	friend NES_EX_Controller_Arkanoid_Paddle;
	friend NES_EX_Controller_Crazy_Climber;
//	friend NES_EX_Controller_Doremikko_Keyboard;
//	friend NES_EX_Controller_Exciting_Boxing;
//	friend NES_EX_Controller_Family_Basic;
	friend NES_EX_Controller_Family_Trainer;
	friend NES_EX_Controller_Hyper_Shot;
	friend NES_EX_Controller_Mahjong;
	friend NES_EX_Controller_OekaKids_Tablet;
	friend NES_EX_Controller_Optical_Gun;
	friend NES_EX_Controller_Pokkun_Moguraa;
	friend NES_EX_Controller_Power_Pad;
	friend NES_EX_Controller_Space_Shadow_Gun;
//	friend NES_EX_Controller_Turbo_File;
//	friend NES_EX_Controller_VS_Zapper;
	

	// SNSS friend functions
	friend void adopt_BASR(SnssBaseBlock* block, NES* nes);
	friend void adopt_VRAM(SnssVramBlock* block, NES* nes);
	friend void adopt_SRAM(SnssSramBlock* block, NES* nes);
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend void adopt_SOUN(SnssSoundBlock* block, NES* nes);
	friend int extract_BASR(SnssBaseBlock* block, NES* nes);
	friend int extract_VRAM(SnssVramBlock* block, NES* nes);
	friend int extract_SRAM(SnssSramBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_SOUN(SnssSoundBlock* block, NES* nes);

	friend void adopt_ExMPRD(const char* fn, NES* nes);
	friend void extract_ExMPRD(const char* fn, NES* nes);

	friend int adoptMem_ExMPRD(unsigned char *extm, uint32 nsize, NES* nes);
	friend int extractMem_ExMPRD(unsigned char **extm, uint32 nsize, NES* nes);

public:
	NES(const char* ROM_name, NES_screen_mgr* _screen_mgr, sound_mgr* _sound_mgr,HWND parent_window_handle);
	~NES();

	void new_snd_mgr(sound_mgr* _sound_mgr);

	void set_pad(unsigned char* c) {
		pad1 = c; pad2 = c+1; pad3 = c+2; pad4 = c+3;
		mic_bits = c+4; coin_bits = c+5;
	}
	/*
	  void set_pad2(controller* c) { pad2 = (NES_pad*)c; }
	  void set_pad3(controller* c) { pad3 = (NES_pad*)c; }
	  void set_pad4(controller* c) { pad4 = (NES_pad*)c; }
	*/

	boolean emulate_frame(boolean draw);
	boolean emulate_frame2(boolean draw);
	boolean emulate_NTSC_frame(boolean draw);
	boolean emulate_PAL_frame(boolean draw);


	void reset(unsigned char);

	const char* getROMname();
	const char* getROMpath();

	boolean loadState(const char* fn);
	boolean saveState(const char* fn);

	void readBaram(unsigned char* adr);			//Cheat
	void writeBaram(unsigned char* adr);
	void ReadBoneb(unsigned int addr, unsigned char *data);
	void WriteBoneb(unsigned int addr, unsigned char data);
	void readSraram(unsigned char* adr);
	void writeSraram(unsigned char* adr);
	void ReadSroneb(unsigned int addr, unsigned char *data);
	void WriteSroneb(unsigned int addr, unsigned char data);

	uint32 GetMainMemp(uint8 **p){ *p = RAM; return 0x800; };
	uint32 GetSramMemp(uint8 **p){ *p = SaveRAM; return ROM->get_size_SaveRAM(); };
	uint32 GetExtMemp(uint8 **p){ *p = mapper_extram; return mapper_extramsize; };
	uint32 Get_PPUMemp(uint8 **p){ 
		for(int i=0; i<12; i++){
			p[i] = ppu->PPU_VRAM_banks[i];
		}
		p[12] = ppu->bg_pal;
		p[13] = ppu->spr_pal;
		return 14;
	}

	void GetCPUContext(unsigned char *context){ cpu->GetContext((NES_6502::Context *)context);}
	void SetCPUContext(unsigned char *context){ cpu->SetContext((NES_6502::Context *)context);}


	unsigned char GetScreenMode(){return nes_type;};
	void SetScreenMode(uint8 mode){ nes_type = mode; };
	void nnnKailleraClient();
	void GetROMInfoStr(char *wt);
	void GetGameTitleName(char *s){ strcpy(s, ROM->get_GameTitleName()); };

	void freeze();
	void thaw();
	boolean frozen();

	void calculate_palette();

	uint8 getBGColor() { return ppu->getBGColor(); }
	void  ppu_rgb();

	enum {
	    NES_NUM_VBLANK_LINES = 20,
	    NES_NUM_FRAME_LINES = 240,

	    // these are 0-based, and actions occur at start of line
	    NES_NMI_LINE = 241,
	    NES_VBLANK_FLAG_SET_LINE = 241,
	    NES_VBLANK_FLAG_RESET_LINE = 261,
	    NES_SPRITE0_FLAG_RESET_LINE = 261,

	    NES_COLOR_BASE = 0x40, // NES palette is set starting at color 0x40 (64)
	    NES_NUM_COLORS = 64    // 64 colors in the NES palette
	};

	uint32 crc32() { return ROM->crc32(); }
	uint32 crc32_all() { return ROM->crc32_all(); }
	uint32 fds_id() { return ROM->fds_id(); }

	//  uint32 fds_mk_id() { return ROM->fds_mk_id(); }
	//  uint32 fds_gm_id() { return ROM->fds_gm_id(); }

	float CYCLES_PER_LINE;
	float CYCLES_BEFORE_NMI;
	boolean BANKSWITCH_PER_TILE;
	boolean DPCM_IRQ;
/*
	boolean Enable_PPU_Latch_Address;
	boolean Enable_PPU_Latch_FDFE;
	boolean Enable_PPU_Latch_RenderScreen;
*/

	// Disk System
	uint8 GetDiskSideNum();
	uint8 GetDiskSide();
	void SetDiskSide(uint8 side);
	uint8 DiskAccessed();

	// Expand Controllers
	void SetExControllerType(uint8 num);
	uint8 GetExControllerType();
	void SetBarcodeValue(uint32 value_low, uint32 value_high);
	uint8 ex_controller_type;

	int StartFamilyBasicAutoInput(my_memread_stream *stream);

	// Data Recorder
	void StopTape();
	void StartPlayTape(const char* fn);
	void StartRecTape(const char* fn);
	uint8 GetTapeStatus();

	// Game Genie
	uint8 GetGenieCodeNum() { return genie_num; }
	uint32 GetGenieCode(uint8 num) { return genie_code[num]; }

	// Movie
	void StopMovie();
	void StartPlayMovie(const char* fn, unsigned char mflag);
	void StartRecMovie(const char* fn, unsigned char mflag);
	uint8 GetMovieStatus(){return movie_status; }
	void StartPlayToRecMovie();
	void InsertMovieMsg(char *);
	void DeleteMovieMsg();
	void MovieRecPause();
	void MoviePreStateRec();

	// frame-IRQ
	uint8 frame_irq_enabled;
	uint8 frame_irq_disenabled;

	// SaveRAM control
	void  WriteSaveRAM(uint32 addr, uint8 data) { SaveRAM[addr] = data;}
	uint8 ReadSaveRAM(uint32 addr) { return SaveRAM[addr]; }

	void  emulate_CPU_cycles(float num_cycles);
	void Load_Genie();

	void StopAPULog(){
		if(apu){
			apu->StopAPULog();
		}
	}
	int StartAPULog(char *fn, char **pptext){
		if(apu){
			return apu->StartAPULog(fn, pptext);
		}
		return 0;
	}

	boolean IsAPULog(){
		if(apu)
			return apu->IsAPULog();
		return false;
	}

	int Rewind(uint32 n);
	void ChangeRewindStatus(boolean b);

protected:
	uint8 NES_RGB_pal[NES_NUM_COLORS][3];
	//  static const uint8 NES_preset_palette[NES_NUM_COLORS][3];

	NES_screen_mgr* scr_mgr;
	sound_mgr* snd_mgr;
//	NES_6502* cpu;
	NES_CPU *cpu;
	NES_PPU* ppu;
	NES_APU* apu;
	NES_ROM* ROM;
	NES_mapper* mapper;

	NES_settings settings;

	boolean is_frozen;

	uint8 nes_type;		// 1 = NTSC, 2 = PAL

	float  ideal_cycle_count;   // number of cycles that should have executed so far
	uint32 emulated_cycle_count;  // number of cycles that have executed so far

	// internal memory
	uint8 RAM[0x800];
	uint8 SaveRAM[0x10000];

	uint8 mapper_extram[0x10000];
	uint32 mapper_extramsize;




	// Rewind
	NES_Rewind *pRewind;



	// joypad stuff
	unsigned char *pad1,*pad2,*pad3,*pad4;
	/*
	  NES_pad* pad1;
	  NES_pad* pad2;
	  NES_pad* pad3;
	  NES_pad* pad4;
	*/
	boolean  pad_strobe;
	uint8 pad1_bits;
	uint8 pad2_bits;

	//  uint8 mic_bits;
	uint8 *mic_bits;
	uint8 *coin_bits;


	uint8 pad3_count;
	uint8 pad4_count;
	uint8 pad3_bits;
	uint8 pad4_bits;
	uint8 pad3_bitsnes;
	uint8 pad4_bitsnes;


	// network joypad stuff
	uint8 net_pad1_bits;
	uint8 net_pad2_bits;
	uint8 net_past_pad1_bits;
	uint8 net_past_pad2_bits;
	uint8 net_past_disk_side;
	uint8 net_syncframe;

	uint8 net_pad3_bits;
	uint8 net_pad4_bits;
	uint8 net_past_pad3_bits;
	uint8 net_past_pad4_bits;

	unsigned char	nnnKaillerapad[4];
	unsigned int	KailleraCounter;

	// Disk System
	uint8 disk_side_flag;


	HWND main_window_handle;

	void loadROM(const char* fn);
	void freeROM();

	// these are called by the CPU
	uint8 MemoryRead(uint32 addr);
	void  MemoryWrite(uint32 addr, uint8 data);

	// internal read/write functions
	uint8 ReadRAM(uint32 addr);
	void  WriteRAM(uint32 addr, uint8 data);

	uint8 ReadLowRegs(uint32 addr);
	void  WriteLowRegs(uint32 addr, uint8 data);

	uint8 ReadHighRegs(uint32 addr);
	void  WriteHighRegs(uint32 addr, uint8 data);

	void  trim_cycle_counts();

	// file stuff
	void Save_SaveRAM();
	void Load_SaveRAM();
	void Save_Disk();
	void Load_Disk();
	void Save_TurboFile();
	void Load_TurboFile();

//	FILE* fmovie;
	uint8 movie_status;
	uint8 movie_flag;
	uint8 movie_lplay_f;
	uint8 movie_fam_f;
	long movie_play_bn;
	long movie_pre_msgp;
	long movie_pre_stap;
	HANDLE hFile_Movie;
	my_memread_stream *pmovie_play_stream;
	char movie_filename[260];
	int movie_archive;
	void Movie();

//	uint8 tape_status;
//	FILE* ftape;

	NES_EX_Controller_base *ex_controller;

//	unsigned char *memory_ssp[2][2];


	// Game Genie
	uint8 genie_num;
	uint32 genie_code[256];

	// VS-Unisystem
	uint8 vs_palette[192];
	uint8 use_vs_palette;
	uint8 pad_swap;
	uint8 vstopgun_ppu;
	uint8 vstopgun_value;

#if 0
	// Arkanoid Paddle
	uint8 ReadReg4016_ARKANOID_PADDLE();
	uint8 ReadReg4017_ARKANOID_PADDLE();
	void WriteReg4016_strobe_ARKANOID_PADDLE();
	void WriteReg4016_ARKANOID_PADDLE(uint8 data);
	uint8 arkanoid_byte;
	uint32 arkanoid_bits;

	// Crazy Climber
	void WriteReg4016_strobe_CRAZY_CLIMBER();

	// Doremikko Keyboard
	uint8 ReadReg4017_DOREMIKKO_KEYBOARD();
	void WriteReg4016_DOREMIKKO_KEYBOARD(uint8 data);
	uint8 doremi_out;
	uint8 doremi_scan;
	uint8 doremi_reg;

	// Exciting Boxing Controller
	uint8 ReadReg4017_EXCITING_BOXING();
	void WriteReg4016_EXCITING_BOXING(uint8 data);
	uint8 excitingboxing_byte;

	// Family Basic Keyboard, Data Recorder
	uint8 ReadReg4016_FAMILY_KEYBOARD();
	uint8 ReadReg4017_FAMILY_KEYBOARD();
	void WriteReg4016_FAMILY_KEYBOARD(uint8 data);
	void RotateTape();
	uint8 kb_out;
	uint8 kb_scan;
	uint8 kb_graph;
	uint8 tape_data;
	uint8 tape_bit;
	uint8 tape_in;
	uint8 tape_out;
	int32 tape_wait;

	// Family Trainer
	uint8 ReadReg4017_FAMILY_TRAINER();
	void WriteReg4016_strobe_FAMILY_TRAINER();
	void WriteReg4016_FAMILY_TRAINER(uint8 data);
	uint8 familytrainer_byte;

	// Hyper Shot
	uint8 ReadReg4017_HYPER_SHOT();
	void WriteReg4016_strobe_HYPER_SHOT();
	uint8 hypershot_byte;

	// Mahjong Controller
	uint8 ReadReg4017_MAHJONG();
	void WriteReg4016_MAHJONG(uint8 data);
	uint32 mahjong_bits;

	// OekaKids Tablet
	uint8 ReadReg4017_OEKAKIDS_TABLET();
	void WriteReg4016_OEKAKIDS_TABLET(uint8 data);
	uint8 tablet_byte;
	uint32 tablet_data;
	uint8 tablet_pre_flag;

	// Optical Gun (Zapper)
	uint8 ReadReg4017_OPTICAL_GUN();

	// Pokkun Moguraa
	uint8 ReadReg4017_POKKUN_MOGURAA();
	void WriteReg4016_strobe_POKKUN_MOGURAA();
	void WriteReg4016_POKKUN_MOGURAA(uint8 data);
	uint8 pokkunmoguraa_byte;

	// Power Pad
	uint8 ReadReg4017_POWER_PAD();
	void WriteReg4016_strobe_POWER_PAD();
	uint32 powerpad_bits1;
	uint32 powerpad_bits2;

	// Space Shadow Gun
	uint8 ReadReg4016_SPACE_SHADOW_GUN();
	uint8 ReadReg4017_SPACE_SHADOW_GUN();
	void WriteReg4016_strobe_SPACE_SHADOW_GUN();
	uint32 spaceshadow_bits;

	// Turbo File
	uint8 ReadReg4017_TURBO_FILE();
	void WriteReg4016_TURBO_FILE(uint8 data);
	uint8 tf_byte;
	uint8 tf_data[13*0x2000];
	uint8 tf_pre_flag;
	uint8 tf_bit;
	uint8 tf_write;
	uint32 tf_pointer;
	uint32 tf_bank;

	// VS Unisystem Zapper
	void WriteReg4016_strobe_VS_ZAPPER();
	void WriteReg4016_VS_ZAPPER(uint8 data);
	uint8 ReadReg4016_VS_ZAPPER();
	uint8 vszapper_strobe;
	uint8 vszapper_count;
#endif

private:
};

#endif
