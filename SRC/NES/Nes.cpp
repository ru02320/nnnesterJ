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
#include <string.h>
#include <math.h>
#include "NES.h"
#include "NES_screen_mgr.h"
#include "NES_ROM.h"
#include "NES_PPU.h"
#include "pixmap.h"
#include "SNSS.h"

#include "mkutils.h"
#include "win32_netplay.h"
#include "ulunzip.h"

#include "nnnkaillera.h"
#include "mmemfile.h"

extern int nnnKailleraFlag;
extern int nnnKailleraplayer;

unsigned char nnnKailleraRecvLag;
unsigned char nnnKailleraSendLag;
extern void assertWindowSize();
extern int MovieAddedDialog();
extern int MovieDeleteMsgDialog();
extern void InfoDisplayt(char *, unsigned int );

#include "debug.h"


extern unsigned char editpalet[64*3];
extern uint8 netplay_status;
extern void MyCtRestore();
extern uint32 nes6502_getcycles(boolean reset_flag);

uint8 NES_preset_palette[64][3] =
    {
        // include the NES palette
#include "NES_pal.h"
    };

/*const uint8 NES::NES_preset_palette[NES_NUM_COLORS][3] =
{
// include the NES palette
#include "NES_pal.h"
};*/

//const float NES::CYCLES_PER_LINE = (float)113.6;
//const float NES::CYCLES_PER_LINE = (float)113.852;
//const float NES::CYCLES_PER_LINE = (float)113.75;
//const float NES::CYCLES_PER_LINE = (float)113.6666666666666666666;


extern void setNTSCMode();
extern void setPALMode();

NES::NES(const char* ROM_name, NES_screen_mgr* _screen_mgr, sound_mgr* _sound_mgr, HWND parent_window_handle)
{
	scr_mgr = _screen_mgr;
	snd_mgr = _sound_mgr;

	main_window_handle = parent_window_handle;

	scr_mgr->setParentNES(this);

#ifdef NESTER_DEBUG
	LOG("nester - NES emulator by Darren Ranalli, (c) 2000\n");
#endif

	cpu = NULL;
	ppu = NULL;
	apu = NULL;
//	mapper_extram = NULL;
	mapper_extramsize = 0;

//	fmovie = NULL;
	hFile_Movie = NULL;
	pmovie_play_stream = NULL;
//	ftape = NULL;
	movie_status = 0;
	pRewind = NULL;

	ex_controller = NULL;
//	memory_ssp[0][0]=memory_ssp[0][1]=memory_ssp[1][0]=memory_ssp[1][1] = NULL;


	try {
#ifdef NESTER_DEBUG
		LOG("Creating NES CPU...");
#endif
		cpu = new NES_6502(this);
//		cpu = new NES_6502ASM(this);

		if(!cpu) throw "error allocating cpu";
#ifdef NESTER_DEBUG
		LOG("Done.\n");
		LOG("Creating NES PPU...");
#endif
		ppu = new NES_PPU(this);
		if(!ppu) throw "error allocating ppu";
#ifdef NESTER_DEBUG
		LOG("Done.\n");
		LOG("Creating NES APU...");
#endif
		apu = new NES_APU(this);
		if(!apu) throw "error allocating apu";
#ifdef NESTER_DEBUG
		LOG("Done.\n");
#endif
		loadROM(ROM_name);
	} catch(...) {
		if(cpu) delete cpu;
		if(ppu) delete ppu;
		if(apu) delete apu;
		throw;
	}

	// set up palette and assert it
	use_vs_palette = 0;
	calculate_palette();
	scr_mgr->assert_palette();

	pad1 = NULL;
	pad2 = NULL;
	pad3 = NULL;
	pad4 = NULL;
	pad3_count=0, pad4_count=0;
	ZeroMemory(nnnKaillerapad, 4);
	is_frozen = FALSE;
/*
	Enable_PPU_Latch_Address = 0;
	Enable_PPU_Latch_FDFE = 0;
	Enable_PPU_Latch_RenderScreen = 0;
*/
	KailleraCounter = 0;
}

NES::~NES()
{
	freeROM();

	if(cpu) delete cpu;
	if(ppu) delete ppu;
	if(apu) delete apu;
}

void NES::new_snd_mgr(sound_mgr* _sound_mgr)
{
	snd_mgr = _sound_mgr;
	apu->snd_mgr_changed();
}

void NES::loadROM(const char* fn)
{
#ifdef NESTER_DEBUG
	LOG("Loading ROM...");
#endif

	ROM = new NES_ROM(fn);

	// set up the mapper
	long crc = crc32();
	#include "NES_set_Cycles.cpp"
	mapper = GetMapper(this, ROM);
	if(!mapper)
	{
		// unsupported mapper
#ifdef NESTER_DEBUG
		LOG("mapper #" << (int)ROM->get_mapper_num() << " not supported" << endl);
#endif

		delete ROM;
		ROM = NULL;

		throw "unsupported mapper";
	}
#ifdef NESTER_DEBUG
	LOG("Done\n");
	LOG(ROM->GetRomName() << ".nes: #" << (int)ROM->get_mapper_num() << " ");
	switch(ROM->get_mirroring())
	{
	case NES_PPU::MIRROR_HORIZ:
		LOG("H ");
		break;
	case NES_PPU::MIRROR_VERT:
		LOG("V ");
		break;
	case NES_PPU::MIRROR_FOUR_SCREEN:
		LOG("F ");
		break;
	}

	if(ROM->has_save_RAM())
	{
		LOG("S ");
	}
	if(ROM->has_trainer())
	{
		LOG("T ");
	}

	LOG(16*ROM->get_num_16k_ROM_banks() << "K/" << 8*ROM->get_num_8k_VROM_banks() << "K " << endl);
#endif

	// load datas to save it at the top of reset()
	Load_SaveRAM();
	Load_Disk();

	// load Game Genie Code
	Load_Genie();

	// set NTSC/PAL mode
	/*
	if(ROM->get_screen_mode() == 1)
{
	  setNTSCMode();
}
	else if(ROM->get_screen_mode() == 2)
{
	  setPALMode();
}
	*/
	scr_mgr->ScreenMode=ROM->get_screen_mode();
	if(NESTER_settings.nes.preferences.TV_Mode){
		nes_type = NESTER_settings.nes.preferences.TV_Mode;
	}
	else
		nes_type = ROM->get_screen_mode();
	reset(0);

	// TEST
	if(NESTER_settings.nes.preferences.UseRewind)
		pRewind = new NES_Rewind(NESTER_settings.nes.preferences.RewindKeyFrame, NESTER_settings.nes.preferences.RewindBFrame);

#ifdef NESTER_DEBUG
	LOG("Starting emulation...\n");
#endif
}

void NES::freeROM()
{
	Save_SaveRAM();
	Save_Disk();
	Save_TurboFile();

	if(nnnKailleraFlag)
		nnnKailleraFlag=0;
#ifdef NESTER_DEBUG
	LOG("Freeing ROM...");
#endif
	if(ROM)	{
		delete ROM;
		ROM = NULL;
	}
	if(mapper){
		delete mapper;
		mapper = NULL;
	}
/*
	if(mapper_extram){
		free(mapper_extram);
		mapper_extram = NULL;
	}
*/
//	if(fmovie) fclose(fmovie);
	if(hFile_Movie){
		CloseHandle(hFile_Movie);
		hFile_Movie =NULL;
		movie_status =0; 
	}
//	if(ftape) fclose(ftape);
	if(pmovie_play_stream){
		delete pmovie_play_stream;
		pmovie_play_stream = NULL;
	}
/*
	if(memory_ssp[0][0]){
		free(memory_ssp[0][0]);
		memory_ssp[0][0] = NULL;
	}
	if(memory_ssp[0][1]){
		free(memory_ssp[0][1]);
		memory_ssp[0][1] = NULL;
	}
	if(memory_ssp[1][0]){
		free(memory_ssp[1][0]);
		memory_ssp[1][0] = NULL;
	}
	if(memory_ssp[1][1]){
		free(memory_ssp[1][1]);
		memory_ssp[1][1] = NULL;
	}
*/

	if(ex_controller){
		delete ex_controller;
		ex_controller = NULL;
	}
	if(pRewind){
		delete pRewind;
		pRewind = NULL;
	}

	scr_mgr->clear(0x00);
	scr_mgr->blt();
#ifdef NESTER_DEBUG
	LOG("Done\n");
	LOG(endl);
#endif
}

const char* NES::getROMname()
{
	return ROM->GetRomName();
}

const char* NES::getROMpath()
{
	return ROM->GetRomPath();
}

void NES::reset(unsigned char softreset)
{
	// save SaveRAM
	Save_SaveRAM();
	Save_Disk();
	Save_TurboFile();

	if(!softreset || ROM->get_mapper_num()==20){
		// clear RAM
		memset(RAM, 0x00, sizeof(RAM));
	}

	// set up CPU
	{
		NES_6502::Context context;

		memset((void*)&context, 0x00, sizeof(context));
		cpu->GetContext(&context);

		context.mem_page[0] = RAM;
		context.mem_page[3] = SaveRAM;

		cpu->SetContext(&context);
	}

	// reset the PPU
	ppu->reset();

	// VROM write protect
	ppu->vram_write_protect = ROM->get_num_8k_VROM_banks() ? 1 : 0;
	ppu->vram_size = 0x2000;

	// reset the APU
	apu->reset();

	frame_irq_enabled = 0xFF;
	frame_irq_disenabled = 0;

	if(mapper)
	{
		// reset the mapper
		mapper->Reset();
	}

	// reset the CPU
	cpu->Reset();



	// load SaveRAM
	Load_SaveRAM();
	Load_Disk();
	Load_TurboFile();

	// set up the trainer if present
	if(ROM->has_trainer())
	{
		// trainer is located at 0x7000; SaveRAM is 0x2000 bytes at 0x6000
		memcpy(&SaveRAM[0x1000], ROM->get_trainer(), NES_ROM::TRAINER_LEN);
	}
	long crc = crc32();
  #include "NES_set_Controller.cpp"
  #include "NES_set_VSPalette.cpp"

	ideal_cycle_count  = 0.0;
	emulated_cycle_count = 0;

//	if(fmovie){fclose(fmovie); fmovie=NULL;}
#if 0
	if(hFile_Movie){
		CloseHandle(hFile_Movie);
		hFile_Movie =NULL;
		movie_status = 0;
	}
	if(pmovie_play_stream){
		delete pmovie_play_stream;
		pmovie_play_stream = NULL;
	}
	movie_status = 0;
#endif
//	if(ftape){fclose(ftape); ftape=NULL;}
//	tape_status = 0;
	if(ex_controller)
		ex_controller->StopTape();

	disk_side_flag = 0;

	// pad1 & pad2 reset
	net_pad1_bits = 0;
	net_pad2_bits = 0;
	net_past_pad1_bits = 0;
	net_past_pad2_bits = 0;
	net_past_disk_side = 0;
	net_syncframe = 0;

	pad_strobe = FALSE;
	pad1_bits = 0;
	pad2_bits = 0;

	pad3_bits=0;
	pad4_bits=0;
	pad3_bitsnes=0;
	pad4_bitsnes=0;

	// external device reset
	if(ex_controller)
		ex_controller->Reset();
#if 0
	arkanoid_byte = 0;
	arkanoid_bits = 0;

	doremi_out = 0;
	doremi_scan = 0;
	doremi_reg = 0;

	excitingboxing_byte = 0;

	kb_out = 0;
	kb_scan = 0;
	kb_graph = 0;

	familytrainer_byte = 0;

	hypershot_byte = 0;

	mahjong_bits = 0;

	if(ex_controller_type == EX_OEKAKIDS_TABLET)
	{
		POINT p;
		p.x = 16;
		p.y = 16;
		ClientToScreen (main_window_handle, &p);
		SetCursorPos ( p.x, p.y );
	}
	tablet_byte = 0;
	tablet_data = 0;
	tablet_pre_flag = 0;

	pokkunmoguraa_byte = 0;

	powerpad_bits1 = 0;
	powerpad_bits2 = 0;

	spaceshadow_bits = 0;

	tf_pre_flag = 0;
	tf_pointer = 0;
	tf_bit = 1;

	vszapper_strobe = 0;
	vszapper_count = 0;
#endif

	// record moviefile
	if(movie_status == 2 && hFile_Movie){
		unsigned char buff[2]={ 0xff, 0};
		DWORD dw;
		if(softreset)
			buff[1] = 4;
		else
			buff[1] = 5;
		WriteFile(hFile_Movie, buff, 2, &dw, NULL);
	}
	if(pRewind){
		unsigned char *s1=NULL;
		if(MemSaveSNSS(&s1, this)){
			pRewind->AddStateData(s1);
		}
	}
}

boolean NES::emulate_frame(boolean draw)
{
	int res;
	MyCtRestore();
	Movie();
	if(nnnKailleraFlag)
		nnnKailleraClient();
	if(1==ROM->get_screen_mode()) //NESTER_settings.nes.graphics.show_all_scanlines
		res = emulate_NTSC_frame(draw);
	else
		res = emulate_PAL_frame(draw);
	if(pRewind){
		if(pRewind->NeedStateData()){
			unsigned char *s1=NULL;
			if(MemSaveSNSS(&s1, this)){
				pRewind->AddStateData(s1);
			}
		}
		else {
			pRewind->AddFrameData(pad1);
		}
	}
	return res;
}


boolean NES::emulate_frame2(boolean draw){
	int res;
	MyCtRestore();
	if(1==ROM->get_screen_mode()) //NESTER_settings.nes.graphics.show_all_scanlines
		res = emulate_NTSC_frame(draw);
	else
		res = emulate_PAL_frame(draw);
	return res;
}



int NES::Rewind(uint32 n){
	if(pRewind){
		pRewind->Rewind(n);
		NES_Rewind_FrameData *pRewindData;
		pRewindData = pRewind->GetCur();
		if(pRewindData){
			uint32 nFrame = pRewindData->GetInputDataNum();
			uint8 *pData = pRewindData->GetStateData();
			MemLoadSNSS(pData, this);
			for(uint32 i=0; i<nFrame; i++){
				NES_Rewind_InputData *pInput;
				pInput = pRewindData->GetInputData(i);
				*pad1 = pInput->InputData[0];
				*pad2 = pInput->InputData[1];
				*pad3 = pInput->InputData[2];
				*pad4 = pInput->InputData[3];
				if(i == (nFrame-1)){
					emulate_frame2(TRUE);
				}
				else {
					emulate_frame2(FALSE);
				}
			}
			return 0;
		}
	}
	return 1;
}


void NES::ChangeRewindStatus(boolean b){
	if(b && pRewind == NULL){
		pRewind = new NES_Rewind(NESTER_settings.nes.preferences.RewindKeyFrame, NESTER_settings.nes.preferences.RewindBFrame);
	}
	else {
		if(pRewind){
			delete pRewind;
			pRewind = NULL;
		}
	}
}



boolean NES::emulate_NTSC_frame(boolean draw)
{
	uint32 i;
	pixmap p;
	uint8* cur_line; // ptr to screen buffer
	boolean retval = draw;

	trim_cycle_counts();

	// disk change and update network pad info
	/*
	  if(!GetNetplayStatus())
	  {

		if(disk_side_flag)
	    {
	      mapper->SetDiskSide(disk_side_flag & 0x0F);
	      disk_side_flag = 0;
	    }

	  }
	  else
	*/
	if(netplay_status){
		net_syncframe++;

		if(net_syncframe == GetNetplayLatency())
		{
			uint8 socket_data;
			net_syncframe = 0;

			// get socket data
			socket_data = SocketGetByte();
			while((socket_data & 0xC0) == 0xC0)
			{
				switch(socket_data & 0xF0)
				{
				case 0xC0:
				case 0xD0:
				case 0xE0:
					// reserved
					break;
				case 0xF0:
					// change disk
					net_past_disk_side = socket_data&0x0f;
					break;
				}
				socket_data = SocketGetByte();
			}

			// set past info
			if(netplay_status == 1)
			{
				// server
				net_pad1_bits = net_past_pad1_bits;
				net_pad2_bits = socket_data;
			}
			else if(netplay_status == 2)
			{
				// client
				net_pad1_bits = socket_data;
				net_pad2_bits = net_past_pad2_bits;
			}
			if(net_past_disk_side != (disk_side_flag&0x0f))
			{
				mapper->SetDiskSide(net_past_disk_side & 0x0F);
				disk_side_flag = net_past_disk_side & 0x0F;
			}

			net_past_pad1_bits = 0;
			net_past_pad2_bits = 0;
			//      net_past_disk_side = 0;

			// get current info
#if 0
			if(pad1) net_past_pad1_bits = pad1->get_inp_state();
			if(pad2) net_past_pad2_bits = pad2->get_inp_state();
			if(GetAsyncKeyState('M') & 0x8000) net_past_pad1_bits = 0x30;
#else
			net_past_pad1_bits = *pad1;
			net_past_pad2_bits = *pad2;
			if(*mic_bits) net_past_pad1_bits = 0x30;
#endif
			if(disk_side_flag != (net_past_disk_side&0x0f))
			{
				// only server can change disk side
				if(netplay_status == 1)
				{
					net_past_disk_side = (disk_side_flag & 0x0F);
					SocketSendByte((net_past_disk_side & 0x0F) | 0xF0);
				}
				//        disk_side_flag = 0;
			}

			// send current info
			/*
			      if(net_past_disk_side)
			      {
			        SocketSendByte((net_past_disk_side & 0x0F) | 0xF0);
			        // change disk
			      }
			*/
			if(netplay_status == 1)
			{
				// server
				SocketSendByte(net_past_pad1_bits);
			}
			else if(netplay_status == 2)
			{
				// client
				net_past_pad2_bits = net_past_pad1_bits;
				SocketSendByte(net_past_pad2_bits);
			}
		}
	}

	// do frame
	ppu->start_frame();

	if(retval)
	{
		if(!scr_mgr->lock(p))
			retval = FALSE;
		else
			cur_line = p.data;
	}

	// LINES 0-239
	for(i = 0; i < NES_NUM_FRAME_LINES; i++)
	{
		// do one line's worth of CPU cycles
		//emulate_CPU_cycles(CYCLES_PER_LINE);
		//mapper->HSync(i);

		if(retval)
		{
			// Bankswitch per line Support for Mother
			if(BANKSWITCH_PER_TILE)
			{
				// render line
				ppu->do_scanline_and_draw(cur_line, CYCLES_PER_LINE * 32 / 42);
				// do half line's worth of CPU cycles (hblank)
				emulate_CPU_cycles(13);
				mapper->HSync(i);
				emulate_CPU_cycles(CYCLES_PER_LINE * 10 / 42 - 13);
				if(i == 0)
				{
					emulate_CPU_cycles(CYCLES_PER_LINE * 32 / 42 + 13);
					mapper->HSync(i);
					emulate_CPU_cycles(CYCLES_PER_LINE * 10 / 42 - 13);
				}
			}
			else
			{
				// do one line's worth of CPU cycles
				emulate_CPU_cycles(CYCLES_PER_LINE);
				mapper->HSync(i);
				// render line
				ppu->do_scanline_and_draw(cur_line, 0);
			}
			// point to next line
			cur_line += p.pitch;
		}
		else
		{
			// do one line's worth of CPU cycles
			emulate_CPU_cycles(CYCLES_PER_LINE);
			mapper->HSync(i);
			ppu->do_scanline_and_dont_draw();
		}
	}

	if(retval)
	{
		scr_mgr->unlock();
	}

	ppu->end_frame();

	// fram_IRQ

	if(!(frame_irq_enabled & 0xC0))
	{
		cpu->DoPendingIRQ();
	}

	for(i = 240; i <= 261; i++)
	{
	    if(i == 261){
	      ppu->end_vblank();
	    }
		else if(i == 241)
		{
			// do v-blank
			ppu->start_vblank();
			mapper->VSync();

			// 1 instruction between vblank flag and NMI
			emulate_CPU_cycles(CYCLES_BEFORE_NMI);
			if(ppu->NMI_enabled()) cpu->DoNMI();
			emulate_CPU_cycles(CYCLES_PER_LINE - CYCLES_BEFORE_NMI);
			mapper->HSync(i);
			continue;
		}

		emulate_CPU_cycles(CYCLES_PER_LINE);
		mapper->HSync(i);
	}
	// HALF-LINE 262.5
	//emulate_CPU_cycles(CYCLES_PER_LINE/2);

	apu->DoFrame();
	apu->SyncAPURegister();

	return retval;
}

boolean NES::emulate_PAL_frame(boolean draw)
{
	uint32 i;
	pixmap p;
	uint8* cur_line; // ptr to screen buffer
	boolean retval = draw;

	trim_cycle_counts();

	// disk change and update network pad info
	/*
	if(!GetNetplayStatus())
{
	  if(disk_side_flag)
	  {
	    mapper->SetDiskSide(disk_side_flag & 0x0F);
	    disk_side_flag = 0;
	  }
}
	else
	*/
	if(netplay_status)
	{
		net_syncframe++;

		if(net_syncframe == GetNetplayLatency())
		{
			uint8 socket_data;
			net_syncframe = 0;

			// get socket data
			socket_data = SocketGetByte();
			while((socket_data & 0xC0) == 0xC0)
			{
				switch(socket_data & 0xF0)
				{
				case 0xC0:
				case 0xD0:
				case 0xE0:
					// reserved
					break;
				case 0xF0:
					// change disk
					net_past_disk_side = socket_data&0x0f;
					break;
				}
				socket_data = SocketGetByte();
			}

			// set past info
			if(netplay_status == 1)
			{
				// server
				net_pad1_bits = net_past_pad1_bits;
				net_pad2_bits = socket_data;
			}
			else if(netplay_status == 2)
			{
				// client
				net_pad1_bits = socket_data;
				net_pad2_bits = net_past_pad2_bits;
			}
			if(net_past_disk_side != (disk_side_flag&0x0f))
			{
				mapper->SetDiskSide(net_past_disk_side & 0x0F);
				disk_side_flag = net_past_disk_side&0x0f;
			}

			net_past_pad1_bits = 0;
			net_past_pad2_bits = 0;
			//      net_past_disk_side = 0;

			// get current info
#if 0
			if(pad1) net_past_pad1_bits = pad1->get_inp_state();
			if(pad2) net_past_pad2_bits = pad2->get_inp_state();
			if(GetAsyncKeyState('M') & 0x8000) net_past_pad1_bits = 0x30;
#else
			net_past_pad1_bits = *pad1;
			net_past_pad2_bits = *pad2;
			if(*mic_bits) net_past_pad1_bits = 0x30;
#endif
			if(disk_side_flag != (net_past_disk_side&0x0f))
			{
				// only server can change disk side
				if(netplay_status == 1)
				{
					net_past_disk_side = (disk_side_flag & 0x0F);
					SocketSendByte((net_past_disk_side & 0x0F) | 0xF0);
				}
				//        disk_side_flag = 0;
			}
			/*
			      // send current info
			      if(net_past_disk_side)
			      {
			        // change disk
			        SocketSendByte((net_past_disk_side & 0x0F) | 0xF0);
			      }
			*/
			if(netplay_status == 1)
			{
				// server
				SocketSendByte(net_past_pad1_bits);
			}
			else if(netplay_status == 2)
			{
				// client
				net_past_pad2_bits = net_past_pad1_bits;
				SocketSendByte(net_past_pad2_bits);
			}
		}
	}

	// do frame
	ppu->start_frame();

	if(retval)
	{
		if(!scr_mgr->lock(p))
			retval = FALSE;
		else
			cur_line = p.data;
	}

	// LINES 0-239
	for(i = 0; i < 240; i++)
	{
		if(retval)
		{
			// do one line's worth of CPU cycles
			emulate_CPU_cycles(CYCLES_PER_LINE);
			mapper->HSync(i);
			// render line
			ppu->do_scanline_and_draw(cur_line, 0);
			cur_line += p.pitch;
		}
		else
		{
			// do one line's worth of CPU cycles
			emulate_CPU_cycles(CYCLES_PER_LINE);
			mapper->HSync(i);
			ppu->do_scanline_and_dont_draw();
		}
	}

	if(retval)
	{
		scr_mgr->unlock();
	}

	ppu->end_frame();

	// fram_IRQ
	if(!(frame_irq_enabled & 0xC0))
	{
		cpu->DoPendingIRQ();
	}

	for(i = 240; i <= 311; i++)
	{
		if(i == 311)
		{
			ppu->end_vblank();
		}
		else if(i == 241)
		{
			// do v-blank
			ppu->start_vblank();
			mapper->VSync();
			// 1 instruction between vblank flag and NMI
			emulate_CPU_cycles(CYCLES_BEFORE_NMI);
			if(ppu->NMI_enabled()) cpu->DoNMI();
			emulate_CPU_cycles(CYCLES_PER_LINE - CYCLES_BEFORE_NMI);
			mapper->HSync(i);
			continue;
		}

		emulate_CPU_cycles(CYCLES_PER_LINE);
		mapper->HSync(i);
	}

	apu->DoFrame();
	apu->SyncAPURegister();

	return retval;
}



void NES::freeze()
{
	apu->freeze();
	is_frozen = TRUE;
}

void NES::thaw()
{
	apu->thaw();
	is_frozen = FALSE;
}

boolean NES::frozen()
{
	return is_frozen;
}

uint8 NES::MemoryRead(uint32 addr)
{
	//  LOG("Read " << HEX(addr,4) << endl);

	if(addr < 0x2000) // RAM
	{
		return ReadRAM(addr);
	}
	else if(addr < 0x4000) // low registers
	{
		return ReadLowRegs(addr);
	}
	else if(addr < 0x4018) // high registers
	{
		return ReadHighRegs(addr);
	}
	else if(addr < 0x6000) // mapper low
	{
		//    LOG("MAPPER LOW READ: " << HEX(addr,4) << endl);
		//    return((uint8)(addr >> 8)); // what's good for conte is good for me
		return mapper->MemoryReadLow(addr);
	}
	else // save RAM, or ROM (mapper 40)
	{
		mapper->MemoryReadSaveRAM(addr);
		return cpu->GetByte(addr);
	}
}

void NES::MemoryWrite(uint32 addr, uint8 data)
{
	//  LOG("Write " << HEX(addr,4) << " " << HEX(data,2) << endl);

	if(addr < 0x2000) // RAM
	{
		WriteRAM(addr, data);
	}
	else if(addr < 0x4000) // low registers
	{
		WriteLowRegs(addr, data);
	}
	else if(addr < 0x4018) // high registers
	{
		WriteHighRegs(addr, data);
		mapper->WriteHighRegs(addr, data);
	}
	else if(addr < 0x6000) // mapper low
	{
		mapper->MemoryWriteLow(addr, data);
	}
	else if(addr < 0x8000) // save RAM
	{
		SaveRAM[addr - 0x6000] = data;
		mapper->MemoryWriteSaveRAM(addr, data);
	}
	else // mapper
	{
		mapper->MemoryWrite(addr, data);
	}
}


uint8 NES::ReadRAM(uint32 addr)
{
	return RAM[addr & 0x7FF];
}

void NES::WriteRAM(uint32 addr, uint8 data)
{
	RAM[addr & 0x7FF] = data;
}


uint8 NES::ReadLowRegs(uint32 addr)
{
	//return ppu->ReadLowRegs(addr & 0xE007);
	if(vstopgun_ppu)
	{
		if(addr == 0x2002)
		{
			return ppu->ReadLowRegs(addr & 0x2002) | 0x1b;
		}
	}
	return ppu->ReadLowRegs(addr & 0xE007);
}

void NES::WriteLowRegs(uint32 addr, uint8 data)
{
	if(vstopgun_ppu)
	{
		if(addr == 0x2000)
		{
			vstopgun_value = data & 0x7F;
			ppu->WriteLowRegs(0x2000, data & 0x7F);
			return;
		}
		if(addr == 0x2001)
		{
			ppu->WriteLowRegs(0x2000, vstopgun_value | (data & 0x80));
			ppu->WriteLowRegs(0x2001, data);
			return;
		}
	}
	ppu->WriteLowRegs(addr & 0xE007, data);
}


uint8 NES::ReadHighRegs(uint32 addr)
{
	if(addr == 0x4014) // SPR-RAM DMA
	{
#ifdef NESTER_DEBUG
		LOG("Read from SPR-RAM DMA reg??" << endl);
#endif
		return ppu->Read0x4014();
	}
	else if(addr == 0x4015 && !(frame_irq_enabled & 0xC0)) // frame_IRQ
	{
		return apu->Read(0x4015) | 0x40;
	}
	else if(addr < 0x4016) // APU
	{
		//    LOG("APU READ:" << HEX(addr,4) << endl);
		return apu->Read(addr);
	}
	else // joypad regs
	{
		uint8 retval=0;

		if(addr == 0x4016)
		{
			{
				if(pad3_count<8){
					// joypad 1
					retval |=  pad1_bits & 0x01;
					pad1_bits >>= 1;

					// joypad 3 (Famicom)
					retval |=  (pad3_bits & 0x01)<<1;
					pad3_bits >>= 1;
				}
				else if(pad3_count<16){		//NES 4play (joypad 3)
					retval |=  (pad3_bitsnes & 0x01);
					pad3_bitsnes >>= 1;
				}
//				else if(pad3_count==19)		//NES 4play adapter info?
//					retval |= 1;

				// mic on joypad 2
				retval |= *mic_bits;
				retval |= *coin_bits;

				retval |= 0x40;		// ?
			}
			if(ex_controller)
				retval |= ex_controller->ReadReg4016();
			//	  LOG("4016READ: " << HEX(addr,4) << "  "<< HEX(pad3_count,2)<<"  "<< HEX(retval,2)<< endl);
			++pad3_count;
		}
		else if(addr == 0x4017)
		{
			{
				if(pad4_count<8){		// joypad 2
					retval |=  (pad2_bits & 0x01);
					pad2_bits >>= 1;
					//joypad 4 (Famicom)
					retval |= ((pad4_bits & 0x01) <<1);
					pad4_bits >>= 1;
				}
				else if(pad4_count<16){		//NES 4play (joypad 4)
					retval |=  (pad4_bitsnes & 0x01);
					pad4_bitsnes >>= 1;
				}
//				else if(pad4_count==18)		//NES 4play adapter info?
//					retval |= 1;
			}
			if(ex_controller)
				retval |= ex_controller->ReadReg4017();
			//	  LOG("4017READ: " << HEX(addr,4) << "  "<< HEX(pad4_count,2)<<"  "<< HEX(retval,2)<< endl);
			++pad4_count;
		}
		return retval;
	}
}

void NES::WriteHighRegs(uint32 addr, uint8 data)
{
	if(addr == 0x4014) // SPR-RAM DMA
	{
		ppu->Write0x4014(data);
		cpu->SetDMA(514);		//CYCLES_PER_DMA
	}
	else if(addr < 0x4016) // APU
	{
		apu->Write(addr, data);
	}
	else if(addr == 0x4016) // joy pad
	{
		// bit 0 == joypad strobe
		if(data & 0x01)
		{
			pad_strobe = TRUE;
		}
		else
		{
			if(pad_strobe)
			{
				pad_strobe = FALSE;
				pad3_count=0, pad4_count=0;		//Pad read counter clear

				// get input states
#if 0
				if(pad1) pad1_bits = pad1->get_inp_state();
				if(pad2) pad2_bits = pad2->get_inp_state();
				if(pad3) pad3_bitsnes = pad3_bits = pad3->get_inp_state();
				if(pad4) pad4_bitsnes = pad4_bits = pad4->get_inp_state();
				//        if(GetAsyncKeyState('M') & 0x8000)
				//			pad2_bits = 0x30;
#else
				pad1_bits = *pad1;
				pad2_bits = *pad2;
				pad3_bitsnes = pad3_bits = *pad3;
				pad4_bitsnes = pad4_bits = *pad4;
#endif

				if(netplay_status)
				{
					// network
					pad1_bits = net_pad1_bits;
					pad2_bits = net_pad2_bits;
				}
/*
				else if(movie_status == 1)
				{
					// play movie
					int pad_tmp, eof=0;
					if(movie_flag&0x80){
						pad_tmp = fgetc(fmovie);
						if(pad_tmp==EOF)
							eof=1;
						else
							pad1_bits = pad_tmp;
					}
					if(movie_flag&0x40){
						pad_tmp = fgetc(fmovie);
						if(pad_tmp==EOF)
							eof=1;
						else if(pad_tmp==0x30)
							*mic_bits=4;
						else
							pad2_bits = pad_tmp;
					}
					if(movie_flag&0x20){
						pad_tmp = fgetc(fmovie);
						if(pad_tmp==EOF)
							eof=1;
						else
							pad3_bits = pad_tmp;
					}
					if(movie_flag&0x10){
						pad_tmp = fgetc(fmovie);
						if(pad_tmp==EOF)
							eof=1;
						else
							pad4_bits = pad_tmp;
					}
					if(movie_flag&8){
						pad_tmp = fgetc(fmovie);
						if(pad_tmp==EOF)
							eof=1;
						else{
							if(disk_side_flag != pad_tmp){
								disk_side_flag = pad_tmp;
								mapper->SetDiskSide(disk_side_flag & 0x0F);
							}
						}
					}
					if(eof){
						if(IDYES==MovieAddedDialog()){
							movie_status = 2;
							fseek(fmovie,0,SEEK_CUR);
						}
						else{
							movie_status = 0;
							fclose(fmovie);
						}
					}
				}
				else if(movie_status == 2)
				{
					// rec movie
					if(movie_flag&0x80)
						fputc(pad1_bits, fmovie);
					if(movie_flag&0x40){
						if(*mic_bits){
							fputc(0x30, fmovie);
						}
						else
							fputc(pad2_bits, fmovie);
					}
					if(movie_flag&0x20)
						fputc(pad3_bits, fmovie);
					if(movie_flag&0x10)
						fputc(pad4_bits, fmovie);
					if(movie_flag&8)
						fputc(disk_side_flag, fmovie);
				}
*/

				// MIC, Insert Coin
#if 0
				if(pad1_bits == 0x30)
				{
					pad1_bits = 0x00;
					mic_bits = 0x04;
				}
				if(pad2_bits == 0x30)
				{
					pad2_bits = 0x00;
					mic_bits = 0x04;
				}
#else
				if(*mic_bits){
					pad2_bits = 0x00;
				}
				if(*coin_bits){
					*coin_bits = 0x20;
				}
#endif

				// swap pad_bits for VS Super Sky Kid, VS Dr.Mario
				if(pad_swap == 1)
				{
					pad_swap = pad1_bits;
					pad1_bits = (pad1_bits & 0x0C) | (pad2_bits & 0xF3);
					pad2_bits = (pad2_bits & 0x0C) | (pad_swap & 0xF3);
					pad_swap = 1;
				}
				// swap pad_bits for VS Pinball (Alt)
				if(pad_swap == 2)
				{
					pad_swap = pad1_bits;
					pad1_bits = (pad1_bits & 0xFD) | ((pad2_bits & 0x01) << 1);
					pad2_bits = (pad2_bits & 0xFE) | ((pad_swap & 0x02) >> 1);
					pad_swap = 2;
				}
				// swap pad_bits for Nintendo World Championship (#105)
				if(crc32() == 0x0b0e128f)
				{
					pad2_bits |= pad1_bits & 0x08;
				}

				if(ex_controller)
					ex_controller->WriteReg4016_strobe();
			}
		}
		if(ex_controller)
			ex_controller->WriteReg4016(data);
	}
	else if(addr == 0x4017) // frame-IRQ
	{
		if(!frame_irq_disenabled)
		{
			frame_irq_enabled = data;
		}
		apu->Write(addr, data);
	}
}

//#include "NES_external_device.cpp"

void NES::emulate_CPU_cycles(float num_cycles)
{
	int32 cycle_deficit;

	ideal_cycle_count += num_cycles;
	cycle_deficit = ((uint32)ideal_cycle_count) - emulated_cycle_count;

	if((uint32)ideal_cycle_count > emulated_cycle_count)
	{
		if(ex_controller_type == EX_FAMILY_KEYBOARD && ex_controller->GetTapeStatus != 0)
		{
			uint32 local_emulated_cycles;
			int32 tapewait;
			tapewait = ex_controller->GetTapeWait();
			while (tapewait <= 0)
			{
				ex_controller->RotateTape();
				tapewait += 8;
			}
			while ( cycle_deficit >= tapewait)
			{
				local_emulated_cycles = cpu->Execute(tapewait);
				emulated_cycle_count += local_emulated_cycles;
				cycle_deficit -= local_emulated_cycles;
				tapewait -= local_emulated_cycles;
				while ( tapewait <= 0)
				{
					ex_controller->RotateTape();
					tapewait += 8;
				}
			}
			ex_controller->SetTapeWait(tapewait);
			local_emulated_cycles = cpu->Execute(cycle_deficit);
			emulated_cycle_count += local_emulated_cycles;
			tapewait -= local_emulated_cycles;
		}
		else
		{
			emulated_cycle_count += cpu->Execute(cycle_deficit);
			if(apu->SyncDMCRegister(cycle_deficit) && DPCM_IRQ)
			{
				cpu->DoPendingIRQ();
			}
		}
	}
}


// call every once in a while to avoid cycle count overflow
void NES::trim_cycle_counts()
{
	uint32 trim_amount;

	trim_amount = (uint32)floor(ideal_cycle_count);
	if(trim_amount > emulated_cycle_count) trim_amount = emulated_cycle_count;

	ideal_cycle_count  -= (float)trim_amount;
	emulated_cycle_count -= trim_amount;
}


void NES::Save_SaveRAM()
{
	// does the ROM use save ram?
	if(!ROM->has_save_RAM()) return;

	// has anything been written to Save RAM?
	for(uint32 i = 0; i < sizeof(SaveRAM); i++)
	{
		if(SaveRAM[i] != 0x00) break;
	}
	if(i < sizeof(SaveRAM))
	{
		FILE* fp = NULL;
		char fn[MAX_PATH], *extp;

#ifdef NESTER_DEBUG
		LOG("Saving Save RAM...");
#endif

		if( NESTER_settings.path.UseSramPath )
		{
			strcpy( fn, NESTER_settings.path.szSramPath );
			PathAddBackslash( fn );
		}
		else
			strcpy( fn, ROM->GetRomPath() );

		if( GetFileAttributes( fn ) == 0xFFFFFFFF )
			MKCreateDirectories( fn );
		strcat(fn, ROM->GetRomName());
		for(extp=fn;*extp;extp++);
		try
		{
			uint8 flag=0;
			if(NESTER_settings.nes.preferences.SaveFileToZip){
				char inpfn[MAX_PATH];
				strcpy(extp, ".zsv");
				strcpy(inpfn, ROM->GetRomName());
				strcat(inpfn, ".sav");
				if(!UsezipLibMemToZip(SaveRAM, ROM->get_size_SaveRAM(), fn, inpfn))
					throw "can't save";
				flag = 1;
			}
			if(!flag){
				strcpy(extp, ".sav");
				fp = fopen(fn, "wb");
				if(!fp) throw "can't open save RAM file";
	
				if(fwrite(SaveRAM, ROM->get_size_SaveRAM(), 1, fp) != 1)
					throw "can't open save RAM file";

				fclose(fp);
			}
#ifdef NESTER_DEBUG
			LOG("Done." << endl);
#endif
		} catch(...) {
#ifdef NESTER_DEBUG
			LOG("can't save" << endl);
#endif
			if(fp) fclose(fp);
		}
	}
}

void NES::Load_SaveRAM()
{
	memset(SaveRAM, 0x00, sizeof(SaveRAM));

	// does the ROM use save ram?
	if(!ROM->has_save_RAM()) return;

	{
		FILE* fp = NULL;
		char fn[MAX_PATH], *extp;

		if( NESTER_settings.path.UseSramPath )
		{
			strcpy( fn, NESTER_settings.path.szSramPath );
			PathAddBackslash( fn );
		}
		else
			strcpy( fn, ROM->GetRomPath() );


		strcat(fn, ROM->GetRomName());
		for(extp=fn;*extp;extp++);

		try
		{
			uint8 flag = 0;
			if(NESTER_settings.nes.preferences.SaveFileToZip){
				const char *savextentions[] = { "*.sav", NULL};
				DWORD rdw;
				strcpy(extp, ".zsv");
				if(!UseUnzipLibExeLoad_P(fn, SaveRAM, 0, &rdw, savextentions)){
					flag = 0;
				}
				else{
					flag = 1;
				}
			}
			if(flag==0){
				strcpy(extp, ".sav");
				fp = fopen(fn, "rb");
				if(!fp) throw "none found.";

#ifdef NESTER_DEBUG
				LOG("Loading Save RAM...");
#endif
				if(fread(SaveRAM, ROM->get_size_SaveRAM(), 1, fp) != 1)
				{
#ifdef NESTER_DEBUG
					LOG("error reading Save RAM file" << endl);
#endif
					throw "error reading Save RAM file";
				}
				fclose(fp);
			}
#ifdef NESTER_DEBUG
			LOG("Done." << endl);
#endif
		} catch(...) {
			if(fp) fclose(fp);
		}
	}
}

void NES::Save_Disk()
{
	// must not save before load disk image to disk[] in mapper reset
	uint8 *diskp = mapper->GetDiskDatap();
	if(!diskp)
		return;
	if(diskp[0] == 0x01)
	{
		FILE* fp = NULL;
		char fn[MAX_PATH], *extp;

		GetModuleFileName(NULL, fn, 256);
		int pt = strlen(fn);
		while(fn[pt] != '\\') pt--;
		fn[pt+1] = '\0';
		if( NESTER_settings.path.UseSramPath )
		{
			strcpy( fn, NESTER_settings.path.szSramPath );
			PathAddBackslash( fn );
		}
		else
			strcpy( fn, ROM->GetRomPath() );
		//    strcat(fn, "save\\");
		if( GetFileAttributes( fn ) == 0xFFFFFFFF )
			MKCreateDirectories( fn );
		strcat(fn, ROM->GetRomName());
		for(extp=fn;*extp;extp++);


		try
		{
			unsigned char headbuff[0x10] = {'F','D','S',0x1a};
			headbuff[4] = mapper->GetDiskSideNum();
			uint8 flag = 0;
			if(NESTER_settings.nes.preferences.SaveFileToZip){
				char inpfn[MAX_PATH];
				unsigned char *p;
				strcpy(extp, ".zsv");
				strcpy(inpfn, ROM->GetRomName());
				strcat(inpfn, ".fds");
				uint32 sn = mapper->GetDiskSideNum();
				sn=sn*65500+0x10;
				p = (unsigned char *)malloc(sn);
				if(p!=NULL){
					memcpy(p, headbuff, 0x10);
					memcpy(p+0x10, diskp, sn-0x10);
					if(!UsezipLibMemToZip(p, sn, fn, inpfn))
						throw "can't save";
					flag=1;
					free(p);
				}
			}
			if(!flag){
				strcpy(extp, ".sdk");
				fp = fopen(fn, "wb");
				if(!fp){
					throw "can't open save disk file";
				}
				else
				{
					fwrite(headbuff, 0x10, 1, fp);
					uint8 sn = mapper->GetDiskSideNum();
//					for(uint8 i = 0; i < sn; i++)
					{
						fwrite(diskp, 65500*sn, 1, fp);
					}
					fclose(fp);
				}
			}
		} catch(...) {
			if(fp) fclose(fp);
		}
	}
}

void NES::Load_Disk()
{
	// must not load before load disk image to disk[] in mapper reset
	uint8 *diskp = mapper->GetDiskDatap();
	if(!diskp)
		return;
	if(diskp[0] == 0x01)
	{
		FILE* fp = NULL;
		char fn[256];

		GetModuleFileName(NULL, fn, 256);
		int pt = strlen(fn);
		while(fn[pt] != '\\') pt--;
		fn[pt+1] = '\0';
		if( NESTER_settings.path.UseSramPath )
		{
			strcpy( fn, NESTER_settings.path.szSramPath );
			PathAddBackslash( fn );
		}
		else
			strcpy( fn, ROM->GetRomPath() );
		strcat(fn, ROM->GetRomName());

		try
		{
			uint8 flag =0;
			if(NESTER_settings.nes.preferences.SaveFileToZip){
				const char *savextentions[] = { "*.fds", NULL};
				DWORD rdw;
				unsigned char *sdp;
				strcat(fn, ".zsv");
				if(!UseUnzipLibExeLoad(fn, &sdp, &rdw, savextentions)){
					flag = 0;
				}
				else{
					memcpy(diskp, sdp+0x10, rdw);
					free(sdp);
					flag = 1;
				}
			}
			if(!flag){
				strcat(fn, ".sdk");
				fp = fopen(fn, "rb");
				if(!fp){
					throw "none found.";
				}
				else
				{
					int d0 = fgetc(fp);
					int d1 = fgetc(fp);
					int d2 = fgetc(fp);
					int d3 = fgetc(fp);

					if(d0 == 'F' && d1 == 'D' && d2 == 'S' && d3 == 0x1a)
					{
						// new disk save format
						fseek(fp, 16, SEEK_SET);
						uint32 sn= mapper->GetDiskSideNum();
						fread(diskp, sn*65500, 1, fp);
					}
					else
						throw "error.";
/*
					else
					{
						// old disk save format
						fseek(fp, 0, SEEK_SET);
						while((d0 = fgetc(fp)) != EOF)
						{
							d1 = fgetc(fp);
							d2 = fgetc(fp);
							d3 = fgetc(fp);
							mapper->SetDiskData(d1+d2*256+d3*65536, d0);
						}
					}
*/
					fclose(fp);
				}
			}
		} catch(...) {
			if(fp) fclose(fp);
		}
	}
}




void NES::Save_TurboFile()
{
	if(ex_controller_type == EX_TURBO_FILE/* && tf_write*/)
	{
		char fn[256];

		GetModuleFileName(NULL, fn, 256);
		int pt = strlen(fn);
		while(fn[pt] != '\\') pt--;
		fn[pt+1] = '\0';
		strcat(fn, "tb_file.dat");

		ex_controller->Save_TurboFile(fn);
	}
}

void NES::Load_TurboFile()
{
	if(ex_controller_type == EX_TURBO_FILE)
	{
		char fn[256];

		GetModuleFileName(NULL, fn, 256);
		int pt = strlen(fn);
		while(fn[pt] != '\\') pt--;
		fn[pt+1] = '\0';
		strcat(fn, "tb_file.dat");

		ex_controller->Load_TurboFile(fn);
	}
}

void NES::Load_Genie()
{
	FILE* fp = NULL;
	char fn[256];

	GetModuleFileName(NULL, fn, 256);
	int pt = strlen(fn);
	while(fn[pt] != '\\') pt--;
	fn[pt+1] = '\0';
	if(NESTER_settings.path.UseGeniePath){
		strcpy(fn, NESTER_settings.path.szGameGeniePath);
	}
	else{
		strcat(fn, "genie\\");
		CreateDirectory(fn, NULL);
	}
	strcat(fn, ROM->GetRomName());
	strcat(fn, ".gen");

	//  try
	{
		genie_num = 0;

		fp = fopen(fn, "rb");
		if(!fp) return;

		int c;
		while((c = fgetc(fp)) != EOF)
		{
			uint8 code[9], p = 0;
			memset(code, 0x00, sizeof(code));
			code[0] = c;
			if(!(c == 0x0D || c == 0x0A || c == EOF))
			{
				for(;;)
				{
					c = fgetc(fp);
					if(c == 0x0D || c == 0x0A || c == EOF) break;
					p++;
					if(p < 8) code[p] = c;
				}
			}
			for(uint8 i = 0; i < 9; i++)
			{
				switch(code[i])
				{
				case 'A': code[i] = 0x00; break;
				case 'P': code[i] = 0x01; break;
				case 'Z': code[i] = 0x02; break;
				case 'L': code[i] = 0x03; break;
				case 'G': code[i] = 0x04; break;
				case 'I': code[i] = 0x05; break;
				case 'T': code[i] = 0x06; break;
				case 'Y': code[i] = 0x07; break;
				case 'E': code[i] = 0x08; break;
				case 'O': code[i] = 0x09; break;
				case 'X': code[i] = 0x0A; break;
				case 'U': code[i] = 0x0B; break;
				case 'K': code[i] = 0x0C; break;
				case 'S': code[i] = 0x0D; break;
				case 'V': code[i] = 0x0E; break;
				case 'N': code[i] = 0x0F; break;
				default:  p = i; i = 9; break;
				}
			}
			if(p == 6)
			{
				uint32 addr = 0x0000,data = 0x0000;
				// address
				addr |= (code[3] & 0x4) ? 0x4000 : 0x0000;
				addr |= (code[3] & 0x2) ? 0x2000 : 0x0000;
				addr |= (code[3] & 0x1) ? 0x1000 : 0x0000;
				addr |= (code[4] & 0x8) ? 0x0800 : 0x0000;
				addr |= (code[5] & 0x4) ? 0x0400 : 0x0000;
				addr |= (code[5] & 0x2) ? 0x0200 : 0x0000;
				addr |= (code[5] & 0x1) ? 0x0100 : 0x0000;
				addr |= (code[1] & 0x8) ? 0x0080 : 0x0000;
				addr |= (code[2] & 0x4) ? 0x0040 : 0x0000;
				addr |= (code[2] & 0x2) ? 0x0020 : 0x0000;
				addr |= (code[2] & 0x1) ? 0x0010 : 0x0000;
				addr |= (code[3] & 0x8) ? 0x0008 : 0x0000;
				addr |= (code[4] & 0x4) ? 0x0004 : 0x0000;
				addr |= (code[4] & 0x2) ? 0x0002 : 0x0000;
				addr |= (code[4] & 0x1) ? 0x0001 : 0x0000;
				// value
				data |= (code[0] & 0x8) ? 0x0080 : 0x0000;
				data |= (code[1] & 0x4) ? 0x0040 : 0x0000;
				data |= (code[1] & 0x2) ? 0x0020 : 0x0000;
				data |= (code[1] & 0x1) ? 0x0010 : 0x0000;
				data |= (code[5] & 0x8) ? 0x0008 : 0x0000;
				data |= (code[0] & 0x4) ? 0x0004 : 0x0000;
				data |= (code[0] & 0x2) ? 0x0002 : 0x0000;
				data |= (code[0] & 0x1) ? 0x0001 : 0x0000;
				genie_code[genie_num] = (addr << 16) | data;
				genie_num++;
			}
			else if(p == 8)
			{
				uint32 addr = 0x0000,data = 0x0000;
				// address
				addr |= (code[3] & 0x4) ? 0x4000 : 0x0000;
				addr |= (code[3] & 0x2) ? 0x2000 : 0x0000;
				addr |= (code[3] & 0x1) ? 0x1000 : 0x0000;
				addr |= (code[4] & 0x8) ? 0x0800 : 0x0000;
				addr |= (code[5] & 0x4) ? 0x0400 : 0x0000;
				addr |= (code[5] & 0x2) ? 0x0200 : 0x0000;
				addr |= (code[5] & 0x1) ? 0x0100 : 0x0000;
				addr |= (code[1] & 0x8) ? 0x0080 : 0x0000;
				addr |= (code[2] & 0x4) ? 0x0040 : 0x0000;
				addr |= (code[2] & 0x2) ? 0x0020 : 0x0000;
				addr |= (code[2] & 0x1) ? 0x0010 : 0x0000;
				addr |= (code[3] & 0x8) ? 0x0008 : 0x0000;
				addr |= (code[4] & 0x4) ? 0x0004 : 0x0000;
				addr |= (code[4] & 0x2) ? 0x0002 : 0x0000;
				addr |= (code[4] & 0x1) ? 0x0001 : 0x0000;
				// value
				data |= (code[0] & 0x8) ? 0x0080 : 0x0000;
				data |= (code[1] & 0x4) ? 0x0040 : 0x0000;
				data |= (code[1] & 0x2) ? 0x0020 : 0x0000;
				data |= (code[1] & 0x1) ? 0x0010 : 0x0000;
				data |= (code[7] & 0x8) ? 0x0008 : 0x0000;
				data |= (code[0] & 0x4) ? 0x0004 : 0x0000;
				data |= (code[0] & 0x2) ? 0x0002 : 0x0000;
				data |= (code[0] & 0x1) ? 0x0001 : 0x0000;
				// compare value
				data |= (code[6] & 0x8) ? 0x8000 : 0x0000;
				data |= (code[7] & 0x4) ? 0x4000 : 0x0000;
				data |= (code[7] & 0x2) ? 0x2000 : 0x0000;
				data |= (code[7] & 0x1) ? 0x1000 : 0x0000;
				data |= (code[5] & 0x8) ? 0x0800 : 0x0000;
				data |= (code[6] & 0x4) ? 0x0400 : 0x0000;
				data |= (code[6] & 0x2) ? 0x0200 : 0x0000;
				data |= (code[6] & 0x1) ? 0x0100 : 0x0000;
				genie_code[genie_num] = (addr << 16) | data | 0x80000000;
				genie_num++;
			}
		}

		fclose(fp);

		//  } catch(...) {
		//    if(fp) fclose(fp);
	}
}

boolean NES::loadState(const char* fn)
{
	uint8 flag = 0;
	FILE* fp = NULL;
	fp = fopen(fn, "rb");
	if(!fp) return 0;
	if('P' == getc(fp))
		flag=1;
	fclose(fp);
	if(flag){			//NESTER_settings.nes.preferences.StateFileToZip){
		const char *savextentions[] = { "*.std", NULL};
		unsigned char *stated=NULL;
		DWORD rdw;

		if(UseUnzipLibExeLoad((char *)fn, &stated, &rdw, savextentions)){
			MemLoadSNSS(stated, this);
			free(stated);
		}
		else{
			flag = 0;
		}
	}
	if(!flag)
		return LoadSNSS(fn, this);
	return flag;
}

boolean NES::saveState(const char* fn)
{
	uint8 flag = 0;
	if(NESTER_settings.nes.preferences.StateFileToZip){
		unsigned char *s1=NULL;
		if(MemSaveSNSS(&s1, this)){
			char inpfn[MAX_PATH], ssfn[MAX_PATH], *p;
			DWORD sn;
			strcpy(ssfn, fn);
			p = PathFindExtension(ssfn);
			*++p = 'z';
			strcpy(inpfn, ROM->GetRomName());
			strcat(inpfn, ".std");
			sn = (*(DWORD *)&s1[4])+0x20;
			if(UsezipLibMemToZip(s1, sn, ssfn, inpfn))
				flag=1;
		}
		if(s1)
			free(s1);
	}
	if(!flag)
		return SaveSNSS(fn, this);
	return flag;
}

void NES::calculate_palette()
{
	if( use_vs_palette ){
		memcpy(NES_RGB_pal, vs_palette, sizeof(NES_RGB_pal));
	}
	else if(NESTER_settings.nes.graphics.calculate_palette == 1 && !use_vs_palette)
	{
		int x,z;
		float tint = ((float)NESTER_settings.nes.graphics.tint) / 256.0f;
		float hue = 332.0f + (((float)NESTER_settings.nes.graphics.hue - (float)0x80) * (20.0f / 256.0f));
		float s,y;
		int cols[16] = {0,240,210,180,150,120,90,60,30,0,330,300,270,0,0,0};
		float theta;
		float br1[4] = {0.5f, 0.75f, 1.0f, 1.0f};
		float br2[4] = {0.29f, 0.45f, 0.73f, 0.9f};
		float br3[4] = {0.0f, 0.24f, 0.47f, 0.77f};
		float r,g,b;

		for(x = 0; x <= 3; x++)
		{
			for(z = 0; z <= 15; z++)
			{
				s = tint;
				y = br2[x];
				if(z == 0)
				{
					s = 0;
					y = br1[x];
				}
				else if(z == 13)
				{
					s = 0;
					y = br3[x];
				}
				else if((z == 14) || (z == 15))
				{
					s = 0;
					y = 0;
				}

				theta = 3.14159265f * (((float)(cols[z] + hue)) / 180.0f);

				r = y + (s * (float)sin(theta));
				g = y - ((27.0f / 53.0f) * s * (float)sin(theta)) + ((10.0f / 53.0f) * s * (float)cos(theta));
				b = y - (s * (float)cos(theta));

				r = r * 256.0f;
				g = g * 256.0f;
				b = b * 256.0f;

				if(r > 255.0f) r = 255.0f;
				if(g > 255.0f) g = 255.0f;
				if(b > 255.0f) b = 255.0f;

				if(r < 0.0f) r = 0.0;
				if(g < 0.0f) g = 0.0;
				if(b < 0.0f) b = 0.0;

				NES_RGB_pal[(x*16) + z][0] = (uint8)r;
				NES_RGB_pal[(x*16) + z][1] = (uint8)g;
				NES_RGB_pal[(x*16) + z][2] = (uint8)b;
			}
		}
	}
	else if( NESTER_settings.nes.graphics.calculate_palette == 2 )
	{
		memset(NES_RGB_pal, 0x00, sizeof(NES_RGB_pal));
		if( FILE *pf = fopen( NESTER_settings.nes.graphics.szPaletteFile, "rb" ) )
		{
			fread( NES_RGB_pal, 1, sizeof(NES_RGB_pal), pf );
			fclose(pf);
		}
	}
	else if(NESTER_settings.nes.graphics.calculate_palette == 3){ //nnn
		memcpy(NES_RGB_pal, editpalet, NES_NUM_COLORS*3);
	}
	else{
		memcpy(NES_RGB_pal, NES_preset_palette, sizeof(NES_RGB_pal));
	}
#if 0
	else
	{
		if( use_vs_palette )
		{
			memcpy(NES_RGB_pal, vs_palette, sizeof(NES_RGB_pal));
		}
		else
		{
			memcpy(NES_RGB_pal, NES_preset_palette, sizeof(NES_RGB_pal));
		}
	}
#endif
	if(ppu->rgb_pal())
	{
		for(int i = 0; i < NES_NUM_COLORS; i++)
		{
			switch(ppu->rgb_pal())
			{
			case 0x20:
				NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.8);
				NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.73);
				break;
			case 0x40:
				NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.73);
				NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.7);
				break;
			case 0x60:
				NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.76);
				NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.78);
				NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.58);
				break;
			case 0x80:
				NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.86);
				NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.8);
				break;
			case 0xA0:
				NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.83);
				NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.68);
				NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.85);
				break;
			case 0xC0:
				NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.67);
				NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.77);
				NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.83);
				break;
			case 0xE0:
				NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.68);
				NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.68);
				NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.68);
				break;
			}
		}
	}

	if(NESTER_settings.nes.graphics.black_and_white)
	{
		int i;

		for(i = 0; i < NES_NUM_COLORS; i++)
		{
			uint8 Y;
			Y = (uint8)(((float)NES_RGB_pal[i][0] * 0.299) +
			            ((float)NES_RGB_pal[i][1] * 0.587) +
			            ((float)NES_RGB_pal[i][2] * 0.114));
			NES_RGB_pal[i][0] = Y;
			NES_RGB_pal[i][1] = Y;
			NES_RGB_pal[i][2] = Y;
		}
	}
}

void NES::ppu_rgb()
{
	calculate_palette();
	scr_mgr->assert_palette();
}

uint8 NES::GetDiskSideNum()
{
	return mapper->GetDiskSideNum();
}
uint8 NES::GetDiskSide()
{
	return mapper->GetDiskSide();
}
void NES::SetDiskSide(uint8 side)
{
	disk_side_flag = side;// | 0x10;
	mapper->SetDiskSide(disk_side_flag & 0x0F);
	if(movie_status == 2 && hFile_Movie){
		unsigned char buff[3]={ 0xff, 1};
		DWORD dw;
		buff[2] = side&0x0f;
		WriteFile(hFile_Movie, buff, 3, &dw, NULL);
	}
}

uint8 NES::DiskAccessed()
{
	return mapper->DiskAccessed();
}

void NES::SetExControllerType(uint8 num)
{
	switch(num){
		case EX_NONE:
		case EX_ARKANOID_PADDLE:
		case EX_CRAZY_CLIMBER:
		case EX_DATACH_BARCODE_BATTLER:
		case EX_DOREMIKKO_KEYBOARD:
		case EX_EXCITING_BOXING:
		case EX_FAMILY_KEYBOARD:
		case EX_FAMILY_TRAINER_A:
		case EX_FAMILY_TRAINER_B:
		case EX_HYPER_SHOT:
		case EX_MAHJONG:
		case EX_OEKAKIDS_TABLET:
		case EX_OPTICAL_GUN:
		case EX_POKKUN_MOGURAA:
		case EX_POWER_PAD_A:
		case EX_POWER_PAD_B:
		case EX_SPACE_SHADOW_GUN:
		case EX_TOP_RIDER:
		case EX_TURBO_FILE:
		case EX_VS_ZAPPER:
			break;
		default:
			return;
	}
	if(ex_controller_type == num)
		return;
	if(ex_controller){
		delete ex_controller;
		ex_controller = NULL;
	}
	try{
		switch(num){
		case EX_NONE:
			break;
		case EX_ARKANOID_PADDLE:
			ex_controller = new NES_EX_Controller_Arkanoid_Paddle(this);
			break;
		case EX_CRAZY_CLIMBER:
			ex_controller = new NES_EX_Controller_Crazy_Climber(this);
			break;
		case EX_DATACH_BARCODE_BATTLER:
//			ex_controller = new NES_EX_Controller_Doremikko_Keyboard();
			break;
		case EX_DOREMIKKO_KEYBOARD:
			ex_controller = new NES_EX_Controller_Doremikko_Keyboard(this);
			break;
		case EX_EXCITING_BOXING:
			ex_controller = new NES_EX_Controller_Exciting_Boxing(this);
			break;
		case EX_FAMILY_KEYBOARD:
			ex_controller = new NES_EX_Controller_Family_Basic(this);
			break;
		case EX_FAMILY_TRAINER_A:
			ex_controller = new NES_EX_Controller_Family_Trainer(this);
			break;
		case EX_FAMILY_TRAINER_B:
			ex_controller = new NES_EX_Controller_Family_Trainer(this);
			break;
		case EX_HYPER_SHOT:
			ex_controller = new NES_EX_Controller_Hyper_Shot(this);
			break;
		case EX_MAHJONG:
			ex_controller = new NES_EX_Controller_Mahjong(this);
			break;
		case EX_OEKAKIDS_TABLET:
			ex_controller = new NES_EX_Controller_OekaKids_Tablet(this);
			break;
		case EX_OPTICAL_GUN:
			ex_controller = new NES_EX_Controller_Optical_Gun(this);
			break;
		case EX_POKKUN_MOGURAA:
			ex_controller = new NES_EX_Controller_Pokkun_Moguraa(this);
			break;
		case EX_POWER_PAD_A:
			ex_controller = new NES_EX_Controller_Power_Pad(this);
			break;
		case EX_POWER_PAD_B:
			ex_controller = new NES_EX_Controller_Power_Pad(this);
			break;
		case EX_SPACE_SHADOW_GUN:
			ex_controller = new NES_EX_Controller_Space_Shadow_Gun(this);
			break;
		case EX_TOP_RIDER:
//			ex_controller = new NES_EX_Controller_Space_Shadow_Gun();
			break;
		case EX_TURBO_FILE:
			ex_controller = new NES_EX_Controller_Turbo_File(this);
			break;
		case EX_VS_ZAPPER:
			ex_controller = new NES_EX_Controller_VS_Zapper(this);
			break;
		default:
			return;
		}
	}catch(...){
		if(ex_controller){
			delete ex_controller;
			ex_controller = NULL;
		}
		ex_controller_type = EX_NONE;
	}
	ex_controller_type = num;
}

uint8 NES::GetExControllerType()
{
	return ex_controller_type;
}

void NES::SetBarcodeValue(uint32 value_low, uint32 value_high)
{
	mapper->SetBarcodeValue(value_low, value_high);
}

int NES::StartFamilyBasicAutoInput(my_memread_stream *stream){
	if(ex_controller)
		return ex_controller->StartAutoFileInput(stream);
	return 0;
};

/*
void NES::StopMovie()
{
	if(fmovie) fclose(fmovie);
	movie_status = 0;
}
void NES::StartPlayMovie(const char* fn)
{
	uint32 i;
	if(fmovie) fclose(fmovie);
	fmovie = fopen(fn, "r+b");		//rb
	fseek(fmovie, 4, SEEK_SET);
	uint32 fsize = 0;
	fsize |= fgetc(fmovie);
	fsize |= fgetc(fmovie) << 8;
	fsize |= fgetc(fmovie) << 16;
	fsize |= fgetc(fmovie) << 24;
	movie_flag = fgetc(fmovie);
	movie_flag = fgetc(fmovie);
	movie_flag = fgetc(fmovie);
	if(movie_flag==0){
		fseek(fmovie, fsize + 16, SEEK_SET);
		movie_flag = 0xc0;
		movie_status = 1;
		return;
	}
	i=16;
	fseek(fmovie, 16, SEEK_SET);
	i += fgetc(fmovie)+1;
	fseek(fmovie, i, SEEK_SET);
	i += fgetc(fmovie)+1;
	fseek(fmovie, i, SEEK_SET);
	i += fgetc(fmovie)+1;
	fsize+=i;
	fseek(fmovie, fsize + 16, SEEK_SET);
	movie_status = 1;
}
void NES::StartPlayToRecMovie()
{
	fseek(fmovie,0,SEEK_CUR);
	movie_status = 2;
}
void NES::StartRecMovie(const char* fn, unsigned char mflag)
{
	if(fmovie) fclose(fmovie);
	fmovie = fopen(fn, "ab");
	movie_status = 2;
	movie_flag = mflag;
}
uint8 NES::GetMovieStatus()
{
	return movie_status;
}
*/

uint8 contoller_bitconv(uint8 bbit){
	uint8 rbit=0;

	if(bbit&0x80)
		rbit|=0x08;
	if(bbit&0x40)
		rbit|=0x04;
	if(bbit&0x20)
		rbit|=0x01;
	if(bbit&0x10)
		rbit|=0x02;
	if(bbit&0x8)
		rbit|=0x20;
	if(bbit&0x4)
		rbit|=0x10;
	if(bbit&0x2)
		rbit|=0x40;
	if(bbit&0x1)
		rbit|=0x80;
	return rbit;
}


void NES::Movie(){
	// Movie
	if(pmovie_play_stream && movie_status==1){
		// play movie
		DWORD dw, i=0;
		unsigned char buff[4];
		if(movie_fam_f){

			dw = pmovie_play_stream->read(buff, movie_play_bn);
			if(!dw){
				StopMovie();
				return;
			}
			if(movie_flag&0x80){
				*pad1 = contoller_bitconv(buff[i++]);
			}
			if(movie_flag&0x40){
				*pad2 = contoller_bitconv(buff[i++]);
			}
			if(movie_flag&0x20){
			}
		}
		else{
	read_lbl:
		dw = pmovie_play_stream->read(buff, movie_play_bn);
		if(!dw){
			if(!movie_lplay_f){
				if(IDYES==MovieAddedDialog()){
					StartPlayToRecMovie();
				}
				else{
					StopMovie();
				}
				return;
			}
			else{
				uint32 hsize, tmp;
				
				pmovie_play_stream->set_pos(8);
				tmp = pmovie_play_stream->get_ch();
				hsize = tmp;
				hsize <<=4;

				tmp = pmovie_play_stream->get_ch();

				pmovie_play_stream->set_pos(hsize);
				hsize += pmovie_play_stream->get_ch();
				pmovie_play_stream->set_pos(hsize);
				hsize += pmovie_play_stream->get_ch();
				pmovie_play_stream->set_pos(hsize);
				hsize += pmovie_play_stream->get_ch();
				pmovie_play_stream->set_pos(hsize);
				goto read_lbl;
			}
		}
		while(buff[0]==0xff){
			if(movie_play_bn!=1){
				pmovie_play_stream->move_pos(-movie_play_bn + 1);
			}
			buff[0] = pmovie_play_stream->get_ch();
			if(buff[0]==1){			//disk
				buff[0] = pmovie_play_stream->get_ch();
				SetDiskSide(buff[0]);
			}
			else if(buff[0]==2){	//state
				unsigned char *pmem, *state=NULL;
				int cp;
				DWORD ss;

				pmem = pmovie_play_stream->get_mem_p();
				cp = pmovie_play_stream->get_pos();
				state = &pmem[cp];
				ss = *((DWORD *)&state[4]);
				MemLoadSNSS(state, this);
				pmovie_play_stream->move_pos(ss);
			}
			else if(buff[0]==3){	//message
				char sbuff[256];
				unsigned int n;

				movie_pre_msgp = pmovie_play_stream->get_pos() -2;
				buff[0] = pmovie_play_stream->get_ch();
				if(buff[0]){
					n = (unsigned int)buff[0];
					dw = pmovie_play_stream->read((unsigned char *)sbuff, n);
					sbuff[n]=0;
					InfoDisplayt(sbuff, 5000);
				}
			}
			else if(buff[0]==4){	// soft reset
				reset(1);
			}
			else if(buff[0]==5){	// hard reset
				reset(0);
			}
			goto read_lbl;
/*
			dw = pmovie_play_stream->read(buff, movie_play_bn);
			if(!dw){
				if(IDYES==MovieAddedDialog()){
					StartPlayToRecMovie();
				}
				else{
					StopMovie();
				}
				return;
			}
*/
		}
		*((DWORD *)pad1)=0;
		if(movie_flag&0x80){
			*pad1 = buff[i++];
		}
		if(movie_flag&0x40){
			*pad2 = buff[i++];
			if(*pad2==0x30){
				*mic_bits=4;
				*pad2 = 0;
			}
		}
		if(movie_flag&0x20){
			*pad3 = buff[i++];
		}
		if(movie_flag&0x10){
			*pad4 = buff[i++];
		}
		}
	}
	else if(movie_status==2){
		DWORD dw, i=0;
		unsigned char buff[4];
		// rec movie
		if(movie_flag&0x80)
			buff[i++]=*pad1;
		if(movie_flag&0x40){
			if(*mic_bits){
				buff[i++]=0x30;
			}
			else{
				buff[i++]=*pad2;
			}
		}
		if(movie_flag&0x20)
			buff[i++]=*pad3;
		if(movie_flag&0x10)
			buff[i++]=*pad4;
		WriteFile(hFile_Movie, buff, i, &dw, NULL);
	}
}


void NES::StopMovie(){
	if(hFile_Movie){
		CloseHandle(hFile_Movie);
		hFile_Movie = NULL;
		movie_status = 0;
	}
	if(pmovie_play_stream){
		delete pmovie_play_stream;
		pmovie_play_stream =NULL;
		movie_status = 0;
	}
}

void NES::StartPlayMovie(const char* fn, unsigned char mflag){
	const char *mvext[] = { "*.mv5", NULL};
	unsigned char *zbuff = NULL;
	DWORD nzsize;
	HANDLE hFile;

	StopMovie();

	movie_archive = 0;
#if 1
	if(UseUnzipLibExeLoad((char *)fn, &zbuff, &nzsize, mvext)){
		movie_archive = 1;
	}
	else
	{
		DWORD dw;
		hFile = CreateFile(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		nzsize = GetFileSize(hFile, NULL);
		zbuff = (unsigned char *)malloc(nzsize);
		if(zbuff == NULL){
			CloseHandle(hFile);
			return;
		}
		ReadFile(hFile, zbuff, nzsize, &dw, NULL);
		CloseHandle(hFile);
	}
#endif

	pmovie_play_stream = new my_memread_stream(zbuff, nzsize);

	uint32 fsize = 0;
	DWORD rdw;
	uint8 sf;
	movie_play_bn=0;
	{
		uint8 mc;
		mc = (uint8)pmovie_play_stream->get_ch();
		if(mc=='F')
			movie_fam_f = 1;
		else
			movie_fam_f = 0;
	}
	if(movie_fam_f){
		pmovie_play_stream->set_pos(5);
		movie_flag = pmovie_play_stream->get_ch();
		if(movie_flag&0x80)
			movie_play_bn++;
		if(movie_flag&0x40)
			movie_play_bn++;
		if(movie_flag&0x20)
			movie_play_bn++;
		reset(0);

		pmovie_play_stream->set_pos(144);
		CYCLES_PER_LINE = 110;
	}
	else{
		pmovie_play_stream->set_pos(0);
		rdw = pmovie_play_stream->read((unsigned char *)&fsize, 4);
		if(rdw != 4 || fsize != 0x1A35564D){
			delete pmovie_play_stream;
			pmovie_play_stream = NULL;
			return;
		}
//		pmovie_play_stream->set_pos(4);
		rdw = pmovie_play_stream->read((unsigned char *)&fsize, 4);
		if(rdw != 4){
			delete pmovie_play_stream;
			pmovie_play_stream = NULL;
			return;
		}
		pmovie_play_stream->set_pos(8);
		sf = pmovie_play_stream->get_ch();
		pmovie_play_stream->set_pos(0x0A);
		movie_flag = pmovie_play_stream->get_ch();

		if(movie_flag==0){
			delete pmovie_play_stream;
			pmovie_play_stream = NULL;
			return;
		}
		if(movie_flag&0x80)
			movie_play_bn++;
		if(movie_flag&0x40)
			movie_play_bn++;
		if(movie_flag&0x20)
			movie_play_bn++;
		if(movie_flag&0x10)
			movie_play_bn++;
		
		uint32 hsize, tmp;

		tmp = pmovie_play_stream->get_ch();
		hsize = tmp;
		hsize <<=4;

		tmp = pmovie_play_stream->get_ch();

		pmovie_play_stream->set_pos(hsize);
		hsize += pmovie_play_stream->get_ch();
		pmovie_play_stream->set_pos(hsize);
		hsize += pmovie_play_stream->get_ch();
		pmovie_play_stream->set_pos(hsize);
		hsize += pmovie_play_stream->get_ch();
		pmovie_play_stream->set_pos(hsize);

		movie_lplay_f = 0;
		if(sf==0){
			reset(0);
		}
		else if(sf==2 && mflag){
			movie_lplay_f=1;
		}
	}

	strcpy(movie_filename, fn);
	movie_status = 1;
	movie_pre_msgp = 0;
	movie_pre_stap = 0;
}

void NES::StartRecMovie(const char* fn, unsigned char mflag){
	if(hFile_Movie && movie_status==3 && !mflag){
		unsigned char *s1=NULL;
		unsigned char buff[2]={ 0xff, 2};
		DWORD dw, ss;
		if(!MemSaveSNSS(&s1, this)){
			StopMovie();
//			MessageBox(main_window_handle,(LPCSTR)"Error",(LPCSTR)"Error",MB_OK);
			return;
		}
		movie_pre_stap = SetFilePointer(hFile_Movie, 0, 0, FILE_CURRENT);
		WriteFile(hFile_Movie, buff, 2, &dw, NULL);
		ss = *((DWORD *)&s1[4]);
		WriteFile(hFile_Movie, s1, ss+0x20, &dw, NULL);
		if(s1)
			free(s1);
		movie_status = 2;
		return;
	}
	StopMovie();
	hFile_Movie = CreateFile(fn, GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile_Movie == INVALID_HANDLE_VALUE){
		return;
    }
	unsigned char sf;
	DWORD dw;
	SetFilePointer(hFile_Movie, (long)8, 0, FILE_BEGIN);
	ReadFile(hFile_Movie, &sf, 1, &dw, NULL);
	SetFilePointer(hFile_Movie, (long)0x14, 0, FILE_BEGIN);
	WriteFile(hFile_Movie, &CYCLES_PER_LINE, 4, &dw, NULL);
	SetFilePointer(hFile_Movie, (long)0, 0, FILE_END);
	if(sf==1){
		movie_status = 3;
		StartRecMovie(NULL, 0);
	}
	else if(sf==0){
		reset(0);
/*
		hFile_Movie = CreateFile(fn, GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile_Movie == INVALID_HANDLE_VALUE){
			return;
	    }
*/
		movie_pre_stap = 0;
		if(mflag&1){
			SetFilePointer(hFile_Movie, (long)0, 0, FILE_END);
			movie_status = 3;
			StartRecMovie(NULL, 0);
		}
		SetFilePointer(hFile_Movie, (long)0, 0, FILE_END);
	}
	movie_status = 2;
	movie_flag = mflag&0xF0;
	movie_pre_msgp = 0;
}



void NES::MoviePreStateRec(){
	if(hFile_Movie && movie_pre_stap && movie_status==2){
		DWORD dw, ss;
		unsigned char *s1=NULL, *s2=NULL;
		SetFilePointer(hFile_Movie, movie_pre_stap, 0, FILE_BEGIN);
		SetFilePointer(hFile_Movie, 2, 0, FILE_CURRENT);
		s1 = (unsigned char *)malloc(0x20);
		if(s1==NULL){
			StopMovie();
			return;
		}
		ReadFile(hFile_Movie, s1, 0x20, &dw, NULL);
		ss = *((DWORD *)&s1[8]);
		s1 = (unsigned char *)realloc(s1, ss+0x20);
		if(s1==NULL){
			StopMovie();
			return;
		}
		ReadFile(hFile_Movie, &s1[0x20], ss, &dw, NULL);
		MemLoadSNSS(s1, this);
		if(s1)
			free(s1);
	}
}


void NES::MovieRecPause(){
	movie_status = 3;
}


//-?
void NES::StartPlayToRecMovie(){
	if(pmovie_play_stream == NULL || movie_status != 1 || movie_archive == 1){
		StopMovie();
		return;
	}
	int curpos;
	curpos = pmovie_play_stream->get_pos();
	StopMovie();

	hFile_Movie = CreateFile(movie_filename, GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile_Movie == INVALID_HANDLE_VALUE){
		return;
    }
	SetFilePointer(hFile_Movie, (long)curpos, 0, FILE_BEGIN);
	SetEndOfFile(hFile_Movie);
	DWORD dw, n;
	SetFilePointer(hFile_Movie, (long)0x10, 0, FILE_BEGIN);
	ReadFile(hFile_Movie, &n, 4, &dw, NULL);
	n++;
	SetFilePointer(hFile_Movie, (long)0x10, 0, FILE_BEGIN);
	WriteFile(hFile_Movie, &n, 4, &dw, NULL);
	SetFilePointer(hFile_Movie, (long)0, 0, FILE_END);
	movie_status = 2;
}


//-?
void NES::InsertMovieMsg(char *msg){
	if(hFile_Movie == NULL || movie_status==0)
		return;
	int n = strlen(msg);
	unsigned char buff[3]={ 0xff, 3};
	DWORD dw;
	buff[2] = (unsigned char)n;
	if(movie_status ==2){
		WriteFile(hFile_Movie, buff, 3, &dw, NULL);
		WriteFile(hFile_Movie, msg, n, &dw, NULL);
	}else if(movie_status ==1){
		if(movie_archive == 1)
			return;
		HANDLE hFile;
		DWORD i, cp;
		unsigned char *p=NULL;
		hFile = CreateFile(movie_filename, GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		i = GetFileSize(hFile, NULL);
		cp= pmovie_play_stream->get_pos();
		i-=cp;
		p = (unsigned char *)malloc(i);
		if(p==NULL)
			return;
		ReadFile(hFile, p, i, &dw, NULL);
		SetFilePointer(hFile, cp, NULL, FILE_BEGIN);
		cp+=(3+n);
		WriteFile(hFile, buff, 3, &dw, NULL);
		WriteFile(hFile, msg, n, &dw, NULL);
		WriteFile(hFile, p, i, &dw, NULL);
		free(p);
		CloseHandle(hFile);
	}
}



//-?
void NES::DeleteMovieMsg(){
	if(hFile_Movie == NULL || movie_status!=1 || movie_pre_msgp==0)
		return;
	DWORD dw;
	if(IDYES == MovieDeleteMsgDialog()){
		if(movie_archive == 1)
			return;
		DWORD i, cp, bp;
		unsigned char buff[3];
		unsigned char *p=NULL;

		HANDLE hFile;
		hFile = CreateFile(movie_filename, GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			return;
		}
		i = GetFileSize(hFile, NULL);
		bp= pmovie_play_stream->get_pos();
		SetFilePointer(hFile, movie_pre_msgp, 0, FILE_BEGIN);
		i-=movie_pre_msgp;
		p = (unsigned char *)malloc(i);
		if(p==NULL)
			return;
		ReadFile(hFile, p, i, &dw, NULL);
		SetFilePointer(hFile, movie_pre_msgp, NULL, FILE_BEGIN);
		ReadFile(hFile, buff, 3, &dw, NULL);
		SetFilePointer(hFile, movie_pre_msgp, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);
		cp=(DWORD)buff[2];
		cp+=3;
		WriteFile(hFile, &p[cp], i-cp, &dw, NULL);
		free(p);
		movie_pre_msgp = 0;
		CloseHandle(hFile);
	}
}

/******************************/


void NES::StopTape()
{
	if(ex_controller)
		ex_controller->StopTape();
}

void NES::StartPlayTape(const char* fn)
{
	if(ex_controller)
		ex_controller->StartPlayTape(fn);
}

void NES::StartRecTape(const char* fn)
{
	if(ex_controller)
		ex_controller->StartRecTape(fn);
}

uint8 NES::GetTapeStatus()
{
	if(ex_controller)
		return ex_controller->GetTapeStatus();
	return 0;
}


//////Cheat
void NES::readBaram(unsigned char* adr)
{
	memcpy(adr, RAM, 0x800);
	return;
}

void NES::writeBaram(unsigned char* adr)
{
	memcpy(RAM, adr, 0x800);
	return;
}


void NES::ReadBoneb(unsigned int addr, unsigned char *data)
{
	*data = RAM[addr & 0x7FF];
	return;
}

void NES::WriteBoneb(unsigned int addr, unsigned char data)
{
	RAM[addr & 0x7FF] = data;
	return;
}


//SRAM
void NES::readSraram(unsigned char* adr)
{
	memcpy(adr, SaveRAM, 0x2000);
	return;
}


void NES::writeSraram(unsigned char* adr)
{
	memcpy(SaveRAM, adr, 0x2000);
	return;
}


void NES::ReadSroneb(unsigned int addr, unsigned char *data)
{
	*data = SaveRAM[addr & 0x1fff];
	return;
}


void NES::WriteSroneb(unsigned int addr, unsigned char data)
{
	SaveRAM[addr & 0x1fff] = data;
	return;
}


void NES::GetROMInfoStr(char *wh)
{
	ROM->GetROMInfoStr(wh);
	return;
}


void NES::nnnKailleraClient(){
	if(nnnKailleraFlag){
		unsigned char buff[8];
		int recv;

		memset(buff, 0, sizeof(buff));
		buff[0] = *pad1;
		if(nnnKailleraplayer){
			if(-1 == (recv= nnnkailleraModifyPlayValues(&buff,1))){
//				nnnkailleraEndGame();
				nnnKailleraFlag=0;
				return;
			}
		}
		else{
			if(-1 == (recv= nnnkailleraModifyPlayValues(&buff,0))){
//				nnnkailleraEndGame();
				nnnKailleraFlag=0;
				return;
			}
		}
		*pad1 = nnnKaillerapad[0] = buff[0];
		*pad2 = nnnKaillerapad[1] = buff[1];
		*pad3 = nnnKaillerapad[2] = buff[2];
		*pad4 = nnnKaillerapad[3] = buff[3];
//		KailleraCounter++;
	}
	return;
}

