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

#include "NES_6502ASM.h"
#include "NES.h"
#include <stdio.h>
#include "debug.h"


#ifdef __cplusplus
extern "C" {
#endif
	extern unsigned char (*READ_FUNCA)(unsigned int);
	extern void (*WRITE_FUNCA)(unsigned int,unsigned char);
	extern unsigned char *MEM_PAGEP[8];
	extern unsigned char MAIN_MEMORY[0x800];
	extern unsigned char DEAD_PAGE[0x2000];
	extern unsigned char CPU_IFLAG,CPU_PENDING;
	extern int __cdecl WORD_READ();
	extern void __cdecl CPU_IRQ();
	extern unsigned int CPU_CYCLE,CPU_PC;
#ifdef __cplusplus
} 
#endif


// NOT SAFE FOR MULTIPLE NES_6502ASM'S
static NES_6502ASM *NES_6502ASM_nes = NULL;

static void NES_write(uint32 address, uint8 value)
{
  NES_6502ASM_nes->MemoryWrite(address, value);
}

static uint8 NES_read(uint32 address)
{
  return NES_6502ASM_nes->MemoryRead(address);
}


NES_6502ASM::NES_6502ASM(NES* parent) : ParentNES(parent)
{
  if(NES_6502ASM_nes) throw "error: multiple NES_6502's";

  try {
    NES_6502ASM_nes = this;

    Init();
  } catch(...) {
    NES_6502ASM_nes = NULL;
    throw;
  }
  READ_FUNCA   = NES_read;
  WRITE_FUNCA = NES_write;
	MEM_PAGEP[0] = DEAD_PAGE;
	MEM_PAGEP[1] = DEAD_PAGE;
	MEM_PAGEP[2] = DEAD_PAGE;
	MEM_PAGEP[3] = DEAD_PAGE;
	MEM_PAGEP[4] = DEAD_PAGE;
	MEM_PAGEP[5] = DEAD_PAGE;
	MEM_PAGEP[6] = DEAD_PAGE;
	MEM_PAGEP[7] = DEAD_PAGE;
}

NES_6502ASM::~NES_6502ASM()
{
  NES_6502ASM_nes = NULL;
}




// Context get/set
void NES_6502ASM::SetContext(nes6502_context *context)
{
//  ASSERT(0x00000000 == (cpu->pc_reg & 0xFFFF0000));
//  cpu->read_handler = NESReadHandler;
//  cpu->write_handler = NESWriteHandler;
//  nes6502_setcontext(cpu);
	MEM_PAGEP[0] = context->mem_page[0];
	MEM_PAGEP[1] = context->mem_page[1];
	MEM_PAGEP[2] = context->mem_page[2];
	MEM_PAGEP[3] = context->mem_page[3];
	MEM_PAGEP[4] = context->mem_page[4];
	MEM_PAGEP[5] = context->mem_page[5];
	MEM_PAGEP[6] = context->mem_page[6];
	MEM_PAGEP[7] = context->mem_page[7];
/*
	for(int loop = 0; loop < NES6502_NUMBANKS; loop++)
   {
      if(NULL == cpu->mem_page[loop])
		  MEM_PAGEP[loop] = DEAD_PAGE;
   }
*/
}

void NES_6502ASM::GetContext(nes6502_context *context)
{
//  nes6502_getcontext(cpu);
//  cpu->read_handler = NESReadHandler;
//  cpu->write_handler = NESWriteHandler;
	context->mem_page[0] = MEM_PAGEP[0];
	context->mem_page[1] = MEM_PAGEP[1];
	context->mem_page[2] = MEM_PAGEP[2];
	context->mem_page[3] = MEM_PAGEP[3];
	context->mem_page[4] = MEM_PAGEP[4];
	context->mem_page[5] = MEM_PAGEP[5];
	context->mem_page[6] = MEM_PAGEP[6];
	context->mem_page[7] = MEM_PAGEP[7];
}

uint8 NES_6502ASM::MemoryRead(uint32 addr)
{
  return ParentNES->MemoryRead(addr);
}

void NES_6502ASM::MemoryWrite(uint32 addr, uint8 data)
{
  ParentNES->MemoryWrite(addr, data);
}

uint32 NES_6502ASM::GetCycles(boolean reset_flag){
	uint32 tc=CPU_CYCLE;
	if(reset_flag)
		CPU_CYCLE=0;
	return tc;
}


uint8 NES_6502ASM::GetByte(uint32 address){
	uint8 data;
	_asm{
		mov		eax,address
		call	WORD_READ
		mov		data,al
	}
	return data;
}


void NES_6502ASM::SetDMA(int cycles){
	_asm{
		mov		eax,cycles
		add		CPU_CYCLE,eax
	}
}


void NES_6502ASM::DoPendingIRQ(void){
	_asm{
		test	CPU_IFLAG,1
		jnz		l1
		call	CPU_IRQ
		ret
	l1:
		mov		CPU_PENDING,1
	}
}


void NES_6502ASM::DoIRQ(void){
	_asm{
		test	CPU_IFLAG,1
		jnz		l1
		call	CPU_IRQ
	l1:
	}
}
