#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 241
void NES_mapper241::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,2,3);
	regs[0]=0;
	// set PPU bank pointers
	if(num_1k_VROM_banks)
	{
		set_PPU_banks(0,1,2,3,4,5,6,7);
	}
}


void NES_mapper241::MemoryWrite(uint32 addr, uint8 data)
{
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(regs[0],2) << endl);
	//	char f=0;
	switch(addr/*&0xF801*/){
	case 0x8000:
		{
			DWORD n = data<<2;
			set_CPU_banks(n,n+1,n+2,n+3);
		}
		break;
	case 0x8001:
		break;
	case 0xF000:
//		set_PPU_bank7(data);
		break;
	}

}

#if 0
void NES_mapper241::HSync(uint32 scanline)
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
