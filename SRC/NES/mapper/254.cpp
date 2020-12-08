#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 254
void NES_mapper254::Reset()
{
	// set CPU bank pointers
	set_CPU_bank3(0);
	set_CPU_bank4(num_8k_ROM_banks-4);
	set_CPU_bank5(num_8k_ROM_banks-3);
	set_CPU_bank6(num_8k_ROM_banks-2);
	set_CPU_bank7(num_8k_ROM_banks-1);
	regs[0]=0;
	// set PPU bank pointers
//	if(num_1k_VROM_banks)
	{
		set_PPU_banks(0,1,2,3,4,5,6,7);
	}
	irq_latch=irq_counter=irq_enabled=0;
}


void NES_mapper254::MemoryWriteSaveRAM(uint32 addr, uint8 data)
{
	//	if(addr>=0x6000 && addr < 0x8000){
//	parent_NES->WriteSaveRAM(0, 0);
	//    }
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
}


void NES_mapper254::MemoryReadSaveRAM(uint32 addr)
{
	LOG("R " << HEX(addr,4)  << endl);
}


void NES_mapper254::MemoryWrite(uint32 addr, uint8 data)
{
	//	char f=0;
	switch(addr & 0xE003){
	case 0x8000:
		{
			int pb = (data&0x1f)<<3;
			set_PPU_banks(pb,pb+1,pb+2,pb+3,pb+4,pb+5,pb+6,pb+7);
		}
		break;
	case 0xE000:
//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  "  << endl);
		set_CPU_bank3(data&0x0f);

//		irq_counter= (irq_counter&0xff00)|data;
		//			set_CPU_bank4(data);
		break;
	case 0xE001:
		{
			if(data & 0x08)
			{
				set_mirroring(NES_PPU::MIRROR_HORIZ);
			}
			else
			{
				set_mirroring(NES_PPU::MIRROR_VERT);
			}
		}
//		irq_counter=(irq_counter&0xff)|(data<<8);
		//			set_CPU_bank5(data);
		break;
//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(regs[0],2) << endl);
	}
}

void NES_mapper254::HSync(uint32 scanline)
{
#if 0
	if(irq_enabled)
	{
		if(scanline<241 /*&& (parent_NES->ppu->spr_enabled() || parent_NES->ppu->bg_enabled())*/){
			irq_counter-=114;
			if(irq_counter<=0){
				parent_NES->cpu->DoIRQ();
				irq_enabled = 0;
				//			  irq_counter = irq_latch;
			}
		}
	}
#endif
}


/////////////////////////////////////////////////////////////////////

#endif
