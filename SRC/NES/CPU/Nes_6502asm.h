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

// NES_6502 interface class
// created to cut down on extra work required for retro-fitting of
// new releases of Matt Conte's nes6502

#ifndef _NES_6502ASM_H_
#define _NES_6502ASM_H_

#include "types.h"
#include "NES_CPU.h"

#include "nes6502.h"

//***********************************************
/*
#define  NES6502_NUMBANKS  8
#define  NES6502_BANKSHIFT 13
#define  NES6502_BANKSIZE  (0x10000 / NES6502_NUMBANKS)
#define  NES6502_BANKMASK  (NES6502_BANKSIZE - 1)
*/
/* cycle counts for interrupts */
/*
#define  INT_CYCLES     7
#define  RESET_CYCLES   6

#define  NMI_MASK       0x01
#define  IRQ_MASK       0x02

*/
#ifdef __cplusplus
extern "C" {
#endif
	extern unsigned char *MEM_PAGEP[8];
//	extern unsigned char MAIN_MEMORY[0x800];
	extern unsigned char DEAD_PAGE[0x2000];

	extern void __cdecl CPU_RESET(void);
	extern int __cdecl CPU_EXECUTE(int);
	extern void __cdecl CPU_NMI(void);
	extern void __cdecl CPU_IRQ(void);

	extern void ASMDEBUG_LOGN(unsigned int n);

#ifdef __cplusplus
}
#endif


#if 0
typedef struct
{
   uint8 *mem_page[NES6502_NUMBANKS];  /* memory page pointers */

   uint32 pc_reg;

   uint8 a_reg, p_reg;
   uint8 x_reg, y_reg;
   uint8 s_reg, __padding;
   uint8 int_pending, jammed;

   int32 total_cycles, burn_cycles;
} nes6502_context;
#endif

//***********************************************

class NES;  // prototype of NES class

class NES_6502ASM : public NES_CPU
{
public:
  struct Context : public nes6502_context {};

public:
  NES_6502ASM(NES* parent);
  ~NES_6502ASM();

  // Functions that govern the 6502's execution
  void Init()                           { /*nes6502_init();*/ }
  void Reset()                          { CPU_RESET(); }
  int  Execute(int total_cycles)        {
//	  int n;
//	  	ASMDEBUG_LOGN(0x50);
		CPU_EXECUTE(total_cycles);
		return total_cycles;
//		n = CPU_EXECUTE(total_cycles);
//		ASMDEBUG_LOGN(0x51);
//		return n;
}
  void DoNMI(void)                      { CPU_NMI(); }
  void DoIRQ(void);
  void DoPendingIRQ(void);
  void SetDMA(int cycles);
  uint8  GetByte(uint32 address);
  uint32 GetCycles(boolean reset_flag);

  // Context get/set
  void SetContext(nes6502_context *cpu);
  void GetContext(nes6502_context *cpu);

protected:

  NES* ParentNES;

  uint8 MemoryRead(uint32 addr);
  void  MemoryWrite(uint32 addr, uint8 data);

  friend void NES_write(uint32 address, uint8 value);
  friend uint8 NES_read(uint32 address);

};

#endif
