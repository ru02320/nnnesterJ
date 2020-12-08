
#ifndef _NES_CPU_H_
#define _NES_CPU_H_

#include "types.h"

#include "nes6502.h"

class NES;  // prototype of NES class

class NES_CPU
{
//public:
//	struct Context : public nes6502_context {};

public:
	NES_CPU(){};

  virtual ~NES_CPU(){}

  // Functions that govern the 6502's execution
  virtual void Init() = 0;
  virtual void Reset() = 0;
  virtual int  Execute(int total_cycles) = 0;
  virtual void DoNMI(void) = 0;
  virtual void DoIRQ(void) = 0;
  virtual void DoPendingIRQ(void) = 0;
  virtual void SetDMA(int cycles) = 0;
  virtual uint8  GetByte(uint32 address) = 0;
  virtual uint32 GetCycles(boolean reset_flag) = 0;

  // Context get/set
  virtual void SetContext(nes6502_context *cpu) = 0;
  virtual void GetContext(nes6502_context *cpu) = 0;

protected:

  NES* ParentNES;

  virtual uint8 MemoryRead(uint32 addr) = 0;
  virtual void  MemoryWrite(uint32 addr, uint8 data) = 0;

//  friend void NES_write(uint32 address, uint8 value);
//  friend uint8 NES_read(uint32 address);

};



#endif


