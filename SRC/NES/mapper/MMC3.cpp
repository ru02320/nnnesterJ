
/////////////////////////////////////////////////////////////////////
// MMC3
// much of this is based on the DarcNES source. thanks, nyef :)
void NES_mapperMMC3::MMC3_Reset()
{
	patch = 0;
	irq_reload = 0;

	// clear registers FIRST!!!
	for(int i = 0; i < 8; i++) regs[i] = 0x00;

	// set CPU bank pointers
	prg0 = 0;
	prg1 = 1;
	MMC3_set_CPU_banks();

	// set VROM banks
	if(num_1k_VROM_banks)
	{
		chr01 = 0;
		chr23 = 2;
		chr4  = 4;
		chr5  = 5;
		chr6  = 6;
		chr7  = 7;
		MMC3_set_PPU_banks();
	}
	else
	{
		chr01 = chr23 = chr4 = chr5 = chr6 = chr7 = 0;
	}

	irq_enabled = 0;
	irq_counter = 0;
	irq_latch = 0;
}

uint8 NES_mapperMMC3::MemoryReadLow(uint32 addr)
{
	return  (uint8)(addr >> 8);
}

void NES_mapperMMC3::MMC3_CMD_Write(uint32 addr, uint8 data)
{
//	LOG(" " << HEX(addr,4) << " = " << HEX(data) << endl);
	switch(addr & 0xE001)
	{
	case 0x8000:
		{
			uint8 old;
			old = regs[0];
			regs[0] = data;
//			if((old&0x40) != (data&0x40))
				MMC3_set_CPU_banks();
//			if((old&0x80) != (data&0x80))
				MMC3_set_PPU_banks();
		}
		break;

	case 0x8001:
		{
			uint32 bank_num;
			int32 base = (regs[0] & 0x80)?4:0;

			regs[1] = data;
			bank_num = regs[1];

			switch(regs[0] & 0x07)
			{
			case 0x00:
				{
					//if(num_1k_VROM_banks)
					{
						bank_num &= 0xfe;
						chr01 = bank_num;
//						SetPPU_Banks(0^base, bank_num);
//						SetPPU_Banks(1^base, bank_num | 1);
						MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x01:
				{
					//if(num_1k_VROM_banks)
					{
						bank_num &= 0xfe;
						chr23 = bank_num;
//						SetPPU_Banks(2^base, bank_num);
//						SetPPU_Banks(3^base, bank_num | 1);
						MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x02:
				{
					//if(num_1k_VROM_banks)
					{
						chr4 = bank_num;
//						SetPPU_Banks(4^base, bank_num);
						MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x03:
				{
					//if(num_1k_VROM_banks)
					{
						chr5 = bank_num;
//						SetPPU_Banks(5^base, bank_num);
						MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x04:
				{
					//if(num_1k_VROM_banks)
					{
						chr6 = bank_num;
//						SetPPU_Banks(6^base, bank_num);
						MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x05:
				{
					//if(num_1k_VROM_banks)
					{
						chr7 = bank_num;
//						SetPPU_Banks(7^base, bank_num);
						MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x06:
				{
					prg0 = bank_num;
					MMC3_set_CPU_banks();
/*
					if(regs[0] & 0x40){
						SetCPU_Banks(6, bank_num);
					}
					else{
						SetCPU_Banks(4, bank_num);
					}
*/
				}
				break;

			case 0x07:
				{
					prg1 = bank_num;
//					SetCPU_Banks(5, bank_num);
					MMC3_set_CPU_banks();
				}
				break;
			}
		}
		break;

	case 0xA000:
		{
			regs[2] = data;

			if(data & 0x40)
			{
//				LOG("MAP4 MIRRORING: 0x40 ???" << endl);
			}
			SetMir(data);

		}
		break;

	case 0xA001:
		{
			regs[3] = data;

			if(data & 0x80)
			{
				// enable save RAM $6000-$7FFF
			}
			else
			{
				// disable save RAM $6000-$7FFF
			}
		}
		break;

	case 0xC000:
		regs[4] = data;
//		irq_counter = regs[4];
		irq_latch = regs[4];
		break;

	case 0xC001:
		irq_reload = 1;
//2
//		irq_counter = irq_latch;

		regs[5] = data;
//-		irq_latch = regs[5];
//		irq_counter = 0;	//-
		break;

	case 0xE000:
		regs[6] = data;
		irq_enabled = 0;
		break;

	case 0xE001:
		regs[7] = data;
		irq_enabled = 1;
		break;

	default:
//		LOG("MAP4: UNKNOWN: " << HEX(addr,4) << " = " << HEX(data) << endl);
		break;

	}
}

void NES_mapperMMC3::HSync(uint32 scanline)
{
//	if(irq_enabled)
//	if((parent_NES->ppu->get_reg0()&0x38)!=0x18)
	{
/*		uint8 tmp = irq_counter;
		if(!irq_counter || irq_reload){
			irq_counter = irq_latch;
			irq_reload = 0 ;
		}
		else 
			irq_counter--;
*/
		if((scanline >= 0) && (scanline <= 239))
		{
			if(parent_NES->ppu->spr_enabled() || parent_NES->ppu->bg_enabled())
			{
				{
					if(!(irq_counter--))
					{
						irq_counter = irq_latch;
						if(irq_enabled)
							parent_NES->cpu->DoIRQ();
					}
					if(irq_reload){
						irq_counter = irq_latch;
						irq_reload = 0;
					}
				}
			}
		}
	}
}

void NES_mapperMMC3::MMC3_set_CPU_banks()
{
	if(prg_swap())
	{
		SetCPU_Banks(4, num_8k_ROM_banks-2);
		SetCPU_Banks(6, prg0);
	}
	else
	{
		SetCPU_Banks(4, prg0);
		SetCPU_Banks(6, num_8k_ROM_banks-2);
	}
	SetCPU_Banks(5, prg1);
	SetCPU_Banks(7, num_8k_ROM_banks-1);
}

void NES_mapperMMC3::MMC3_set_PPU_banks()
{
	if(num_1k_VROM_banks)
	{
		if(chr_swap())
		{
			SetPPU_Banks(0, chr4);
			SetPPU_Banks(1, chr5);
			SetPPU_Banks(2, chr6);
			SetPPU_Banks(3, chr7);
			SetPPU_Banks(4, chr01);
			SetPPU_Banks(5, chr01+1);
			SetPPU_Banks(6, chr23);
			SetPPU_Banks(7, chr23+1);
		}
		else
		{
			SetPPU_Banks(0, chr01);
			SetPPU_Banks(1, chr01+1);
			SetPPU_Banks(2, chr23);
			SetPPU_Banks(3, chr23+1);
			SetPPU_Banks(4, chr4);
			SetPPU_Banks(5, chr5);
			SetPPU_Banks(6, chr6);
			SetPPU_Banks(7, chr7);
		}
	}
	else
	{
		if(chr_swap())
		{
			set_VRAM_bank(0, chr4);
			set_VRAM_bank(1, chr5);
			set_VRAM_bank(2, chr6);
			set_VRAM_bank(3, chr7);
			set_VRAM_bank(4, chr01+0);
			set_VRAM_bank(5, chr01+1);
			set_VRAM_bank(6, chr23+0);
			set_VRAM_bank(7, chr23+1);
		}
		else
		{
			set_VRAM_bank(0, chr01+0);
			set_VRAM_bank(1, chr01+1);
			set_VRAM_bank(2, chr23+0);
			set_VRAM_bank(3, chr23+1);
			set_VRAM_bank(4, chr4);
			set_VRAM_bank(5, chr5);
			set_VRAM_bank(6, chr6);
			set_VRAM_bank(7, chr7);
		}
	}
}

#define MAP4_ROM(ptr)  (((ptr)-parent_NES->ROM->get_ROM_banks())  >> 13)
#define MAP4_VROM(ptr) (((ptr)-parent_NES->ROM->get_VROM_banks()) >> 10)
#define MAP4_VRAM(ptr) (((ptr)-parent_NES->ppu->get_patt()) >> 10)

void NES_mapperMMC3::SNSS_fixup() // HACK HACK HACK HACK
{
	NES_6502::Context context;
	parent_NES->cpu->GetContext(&context);

	prg0 = MAP4_ROM(context.mem_page[prg_swap() ? 6 : 4]);
	prg1 = MAP4_ROM(context.mem_page[5]);
	if(num_1k_VROM_banks)
	{
		if(chr_swap())
		{
			chr01 = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[4]);
			chr23 = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[6]);
			chr4  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[0]);
			chr5  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[1]);
			chr6  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[2]);
			chr7  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[3]);
		}
		else
		{
			chr01 = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[0]);
			chr23 = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[2]);
			chr4  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[4]);
			chr5  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[5]);
			chr6  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[6]);
			chr7  = MAP4_VROM(parent_NES->ppu->PPU_VRAM_banks[7]);
		}
	}
	else
	{
		if(chr_swap())
		{
			chr01 = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[4]);
			chr23 = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[6]);
			chr4  = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[0]);
			chr5  = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[1]);
			chr6  = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[2]);
			chr7  = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[3]);
		}
		else
		{
			chr01 = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[0]);
			chr23 = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[2]);
			chr4  = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[4]);
			chr5  = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[5]);
			chr6  = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[6]);
			chr7  = MAP4_VRAM(parent_NES->ppu->PPU_VRAM_banks[7]);
		}
	}
}



void NES_mapperMMC3::MMC3_set_Mir(uint8 data){
	if(parent_NES->ROM->get_mirroring() != NES_PPU::MIRROR_FOUR_SCREEN)
	{
		if(data & 0x01)
		{
			set_mirroring(NES_PPU::MIRROR_HORIZ);
		}
		else
		{
			set_mirroring(NES_PPU::MIRROR_VERT);
		}
	}
}


void NES_mapperMMC3::SetPPU_Banks(uint32 addr, uint8 data){
	set_PPU_bank(addr, data);
}


void NES_mapperMMC3::SetCPU_Banks(uint32 addr, uint8 data){
	set_CPU_bank(addr, (data & 0x3f));
}




#if 0
void NES_mapperMMC3::PPU_A12_Rising(){
/*
	if(irq_reload){
		irq_counter = regs[4];
		irq_reload = 0;
	}
*/
}
#endif
/////////////////////////////////////////////////////////////////////

