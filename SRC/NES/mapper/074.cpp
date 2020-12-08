#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 74
void NES_mapper74::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0, 1, num_8k_ROM_banks-2, num_8k_ROM_banks-1);
//	set_CPU_banks(0, 1, 2, 3);
	regs[0]=0;
	// set PPU bank pointers
	if(num_1k_VROM_banks)
	{
		set_PPU_banks(0,1,2,3,4,5,6,7);
	}
}


void NES_mapper74::MemoryWrite(uint32 addr, uint8 data)
{
	//	char f=0;
	switch(addr&0xff0f){
	case 0x8000:
		set_CPU_bank4(data&0x3f);
		break;
	case 0x8001:
		set_CPU_bank5(data&0x3f);
		break;
	case 0xE000:
		set_PPU_bank1(data);
		break;
	}
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(regs[0],2) << endl);

}

#if 0
void NES_mapper74::HSync(uint32 scanline)
{
	if(irq_enabled)
	{
		if(scanline<241){
			irq_counter--;
			if(irq_counter<=0){
				parent_NES->cpu->DoIRQ();
				irq_enabled = 0;
				irq_counter = irq_latch;
			}
		}
	}
}
#endif

/////////////////////////////////////////////////////////////////////

#endif
