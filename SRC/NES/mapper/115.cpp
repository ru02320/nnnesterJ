#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 115
void NES_mapper115::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0, 1, num_8k_ROM_banks-2, num_8k_ROM_banks-1);
	regs[0]=0;
	// set PPU bank pointers
	if(num_1k_VROM_banks)
	{
		set_PPU_banks(0,1,2,3,4,5,6,7);
	}
	irq_latch=irq_counter=irq_enabled=0;
}


void NES_mapper115::MemoryWrite(uint32 addr, uint8 data)
{
	char f=0;
	switch(addr&0xf001){
	case 0x8000:
		regs[0]=data;
		f=1;
		break;
	case 0x8001:
		switch(regs[0]&7){
		case 0:
			set_PPU_bank0(data&0x1f);
			set_PPU_bank1(data&0x1f+1);
			break;
		case 1:
			set_PPU_bank2(data&0x1f);
			set_PPU_bank3(data&0x1f+1);
			break;
		case 2:
			set_PPU_bank4(data&0x1f);
			break;
		case 3:
			set_PPU_bank5(data&0x1f);
			break;
		case 4:
			set_PPU_bank6(data&0x1f);
			break;
		case 5:
			set_PPU_bank7(data&0x1f);
			break;
		case 6:
			set_CPU_bank4(data&0x0f);
			break;
		case 7:
			set_CPU_bank5(data&0x0f);
			break;
		}
		f=1;
		break;
	case 0xA000:
		//			irq_enabled=data;
		break;
	case 0xC000:
		data&=0x1f;
		//			irq_counter=data;
		//			set_PPU_bank1(data);
		break;
	case 0xE000:
		//			set_CPU_bank4(data&0x0f);
		set_PPU_banks(data, data+1, data+2, data+3, data+4, data+5, data+6, data+7);
		break;
	}
	if(f==0){
		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(regs[0],2) << endl);
	}

}


void NES_mapper115::HSync(uint32 scanline)
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

/////////////////////////////////////////////////////////////////////

#endif
