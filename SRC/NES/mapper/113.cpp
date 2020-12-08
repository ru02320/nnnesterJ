
/////////////////////////////////////////////////////////////////////
// Mapper 113
void NES_mapper113::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,2,3);

	// set PPU bank pointers
	if(num_1k_VROM_banks)
	{
		set_PPU_banks(0,1,2,3,4,5,6,7);
	}
}

void NES_mapper113::MemoryWriteLow(uint32 addr, uint8 data)
{
	uint32 v;
	v = (((data >> 3) & 0x08) + (data & 0x07)) << 3;
	set_PPU_banks(v+0,v+1,v+2,v+3,v+4,v+5,v+6,v+7);
	v = (data >> 1) & 0x1c;
	set_CPU_banks(v+0, v+1, v+2, v+3);
}

void NES_mapper113::MemoryWrite(uint32 addr, uint8 data)
{
	switch(addr)
	{
	case 0x8008:
	case 0x8009:
/*		
		{
			uint8 prg_bank, chr_bank;

			prg_bank = data >> 3;

			if (num_8k_ROM_banks <= 8 && num_1k_VROM_banks == 8*16)
			{
				chr_bank = ((data >> 3) & 0x08) + (data & 0x07);
			}
			else
			{
				chr_bank = data & 0x07;
			}
			//uint8 prg_bank = (data & 0x03) | ((data & 0x80) >> 5);
			//uint8 chr_bank = (data & 0x70) >> 4;

			set_CPU_bank4(prg_bank*4+0);
			set_CPU_bank5(prg_bank*4+1);
			set_CPU_bank6(prg_bank*4+2);
			set_CPU_bank7(prg_bank*4+3);

			set_PPU_bank0(chr_bank*8+0);
			set_PPU_bank1(chr_bank*8+1);
			set_PPU_bank2(chr_bank*8+2);
			set_PPU_bank3(chr_bank*8+3);
			set_PPU_bank4(chr_bank*8+4);
			set_PPU_bank5(chr_bank*8+5);
			set_PPU_bank6(chr_bank*8+6);
			set_PPU_bank7(chr_bank*8+7);
		}
*/
		{
			uint32 v;
			v = (((data >> 3) & 0x08) + (data & 0x07)) << 3;
			set_PPU_banks(v+0,v+1,v+2,v+3,v+4,v+5,v+6,v+7);
			v = (data >> 1);
			set_CPU_banks(v+0, v+1, v+2, v+3);
		}
		break;
	case 0x8E66:
	case 0x8E67:
		{
			uint32 v = (data&0x07)?0:8;
			set_PPU_banks(v+0,v+1,v+2,v+3,v+4,v+5,v+6,v+7);
		}
		break;
	case 0xE00A:
//		MIRROR_SET2( 2 );
		break;
	}
}
/////////////////////////////////////////////////////////////////////

