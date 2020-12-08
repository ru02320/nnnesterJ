#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 245
void NES_mapper245::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);

	regs[0] = 0;
	irq_counter = irq_latch = irq_enabled = 0;
}

void NES_mapper245::MemoryWrite(uint32 addr, uint8 data)
{
	switch(addr&0xF007)
	{
	case 0x8000:
		{
			regs[0] = data;
		}
		break;

	case 0x8001:
		{
			switch(regs[0] & 7)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				break;

			case 6:
				{
					set_CPU_bank4(data);
				}
				break;

			case 7:
				{
					set_CPU_bank5(data);
				}
				break;
			}
		}
		break;
	case 0xA000:
		break;
	case 0xA001:
		irq_enabled=data;
		break;
	case 0xE000:
		irq_counter= (irq_counter&0xff00)|data;
		//			set_CPU_bank4(data);
		break;
	case 0xE001:
		irq_counter=(irq_counter&0xff)|(data<<8);
		//			set_CPU_bank5(data);
		break;
	}
//	if(addr>=0x8000 ){
//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(regs[0],2) << endl);
//	}
}

void NES_mapper245::HSync(uint32 scanline)
{
#if 1
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
