#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 249
void NES_mapper249::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0, 1, num_8k_ROM_banks-2, num_8k_ROM_banks-1);
	regs[0]=0;
	// set PPU bank pointers
	if(num_1k_VROM_banks)
	{
		set_PPU_banks(0,1,2,3,4,5,6,7);
	}
}


void NES_mapper249::MemoryWrite(uint32 addr, uint8 data)
{
	//	char f=0;
	switch(addr&0xf001){
	case 0x8000:
		set_CPU_bank4(data);
		set_CPU_bank5(data+1);
		break;
	case 0x8001:
		set_CPU_bank6(data);
		set_CPU_bank7(data+1);
		break;
	case 0x8800:
		set_PPU_bank0(data);
		break;
	case 0x8801:
		set_PPU_bank1(data);
		break;
		/*		case 0x9000:
					set_CPU_bank6(data);
					break;
				case 0xA400:
					set_CPU_bank7(data);
					break;
				case 0xB000:
					set_PPU_bank0(data);
					break;
				case 0xB004:
					set_PPU_bank1(data);
					break;
				case 0xB008:
					set_PPU_bank2(data);
					break;
				case 0xB00C:
					set_PPU_bank3(data);
					break;
				case 0xC000:
					set_PPU_bank0(data);
					break;
				case 0xC004:
					set_PPU_bank1(data);
					break;
				case 0xC008:
					set_PPU_bank2(data);
					break;
				case 0xC00C:
					set_PPU_bank3(data);
					break;
				case 0xD000:
					set_PPU_bank0(data);
					break;
				case 0xD004:
					set_PPU_bank1(data);
					break;
				case 0xD008:
					set_PPU_bank2(data);
					break;
				case 0xD00C:
					set_PPU_bank3(data);
					break;
				case 0xE000:
					set_PPU_bank4(data);
					break;
				case 0xE004:
					set_PPU_bank5(data);
					break;
				case 0xE008:
					set_PPU_bank6(data);
					break;
				case 0xE00C:
					set_PPU_bank7(data);
					break;
		*/	}
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(regs[0],2) << endl);

}

#if 0
void NES_mapper249::HSync(uint32 scanline)
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
