#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper NSF - private mapper number = 12 (decimal)
void NES_mapperNSF::Reset()
{
	uint8 i;
	uint32 j;

	// Init ExSound
	chip_type = ROM_banks[0x7B] & 0x3F;
	parent_NES->apu->SelectExSound(chip_type);
/*
	if(chip_type & 1)
		parent_NES->apu->SelectExSound(1);
	if(chip_type & 2)
		parent_NES->apu->SelectExSound(2);
	if(chip_type & 4)
		parent_NES->apu->SelectExSound(4);
*/
	parent_NES->MemoryWrite(0x4015, 0x1f);

	// Init Banks
	uint8 bank_switch = 0;
	for(i = 0; i < 8; i++)
	{
		bank_switch |= ROM_banks[0x70+i];
	}
	if(bank_switch)
	{
		uint8 start_bank = ROM_banks[0x9] >> 4;
		for(i = 0; (start_bank + i) < 8; i++)
		{
			BankSwitch(start_bank+i, i);
		}
		for(i = 0; i < 8; i++)
		{
			BankSwitch(i+8, ROM_banks[0x70+i]);
		}
		if(chip_type & 4)
		{
			BankSwitch(6, ROM_banks[0x76]);
			BankSwitch(7, ROM_banks[0x77]);
		}
	}
	else
	{
		uint32 nsf_size = ROM_banks[0x0] | (ROM_banks[0x1]<<8) | (ROM_banks[0x2]<<16);
		uint32 load_addr = ROM_banks[0x8] | (ROM_banks[0x9]<<8);
		for(j = 0; j < nsf_size-0x80; j++)
		{
			wram2[(load_addr+j) & 0x7FFF] = ROM_banks[j+0x80];
		}
	}

	// Load Player Program
	LoadPlayer();

	// Map WRAM
	NES_6502::Context context;
	parent_NES->cpu->GetContext(&context);
	context.mem_page[3] = wram1 + 0x0000; // $6000-$7FFF (FDS)
	context.mem_page[4] = wram2 + 0x0000; // $8000-$9FFF
	context.mem_page[5] = wram2 + 0x2000; // $A000-$BFFF
	context.mem_page[6] = wram2 + 0x4000; // $C000-$DFFF
	context.mem_page[7] = wram2 + 0x6000; // $E000-$FFFF
	parent_NES->cpu->SetContext(&context);
}

void NES_mapperNSF::MemoryWriteLow(uint32 addr, uint8 data)
{
	if(addr >= 0x5FF6 && addr <= 0x5FFF)
	{
		BankSwitch(addr & 0xF, data);
	}
	parent_NES->apu->ExWrite(addr, data);
}

void NES_mapperNSF::MemoryWriteSaveRAM(uint32 addr, uint8 data)
{
	if((chip_type & 4) || addr < 0x7E40 || addr >= 0x7FF0)
	{
		wram1[addr - 0x6000] = data;
	}
	parent_NES->apu->ExWrite(addr, data);
}

void NES_mapperNSF::MemoryWrite(uint32 addr, uint8 data)
{
	if((chip_type & 4) && addr >= 0xFFF0 && addr <= 0xFFF2)
	{
		wram2[addr & 0x7FFF] = data;
	}
	parent_NES->apu->ExWrite(addr, data);
}

uint8 NES_mapperNSF::MemoryReadLow(uint32 addr)
{
	return parent_NES->apu->ExRead(addr);
}

void NES_mapperNSF::BankSwitch(uint8 num, uint8 bank)
{
	uint32 load_start = (ROM_banks[0x8] | (ROM_banks[0x9] << 8)) & 0x0FFF;
	if(num == 6 || num == 7)
	{
		for(uint32 i = 0; i < 0x1000; i++)
		{
			int32 adr = 0x1000 * bank + i + 0x80 - load_start;
			if(adr < 0)
			{
				wram1[0x1000*(num&1)+i] = 0;
			}
			else
			{
				wram1[0x1000*(num&1)+i] = ROM_banks[adr];
			}
		}
	}
	else if(num >= 8 && num <= 14)
	{
		for(uint32 i = 0; i < 0x1000; i++)
		{
			int32 adr = 0x1000 * bank + i + 0x80 - load_start;
			if(adr < 0)
			{
				wram2[0x1000*(num&7)+i] = 0;
			}
			else
			{
				wram2[0x1000*(num&7)+i] = ROM_banks[adr];
			}
		}
	}
	else if(num == 15)
	{
		uint32 adr_max;
		if(chip_type & 4)
		{
			adr_max = 0xE40;
		}
		else
		{
			adr_max = 0xFFA;
		}
		for(uint32 i = 0; i < adr_max; i++)
		{
			int32 adr = 0x1000 * bank + i + 0x80 - load_start;
			if(adr < 0)
			{
				wram2[0x1000*(num&7)+i] = 0;
			}
			else
			{
				wram2[0x1000*(num&7)+i] = ROM_banks[adr];
			}
		}
	}
}

void NES_mapperNSF::LoadPlayer()
{
	// Load Player (thanx Chris Covell)
	uint8 play_prg1[0x1C0] =
	    {
	        0x78,0xd8,0xa2,0xff,0x9a,0xad,0x02,0x20,0x10,0xfb,0xa9,0x00,0xaa,0x95,0x00,0x9d,
	        0x00,0x01,0x9d,0x00,0x02,0x9d,0x00,0x03,0x9d,0x00,0x04,0x9d,0x00,0x05,0x9d,0x00,
	        0x06,0x9d,0x00,0x07,0xe8,0xd0,0xe6,0xa9,0x80,0x8d,0x00,0x20,0xa9,0x00,0x8d,0x01,
	        0x20,0xa9,0x00,0x8d,0x06,0x20,0x8d,0x06,0x20,0xaa,0x8d,0x07,0x20,0xe8,0xe0,0x10,
	        0xd0,0xf8,0xa9,0x01,0x8d,0x06,0x20,0xa9,0x00,0x8d,0x06,0x20,0xa2,0x00,0xa0,0x00,
	        0xbd,0x70,0x7f,0x8d,0x07,0x20,0xe8,0xc8,0xc0,0x08,0xd0,0xf4,0xa0,0x00,0x8c,0x07,
	        0x20,0x8c,0x07,0x20,0x8c,0x07,0x20,0x8c,0x07,0x20,0x8c,0x07,0x20,0x8c,0x07,0x20,
	        0x8c,0x07,0x20,0x8c,0x07,0x20,0xe0,0x80,0xd0,0xd6,0xa9,0x3f,0x8d,0x06,0x20,0xa9,
	        0x00,0x8d,0x06,0x20,0xa9,0x0e,0x8d,0x07,0x20,0xa9,0x30,0x8d,0x07,0x20,0xa9,0x00,
	        0x8d,0xf1,0x7f,0xa9,0x0e,0x8d,0x01,0x20,0xa9,0x00,0x8d,0xf1,0x7f,0x20,0x00,0x80,
	        0xa9,0x01,0x8d,0xf0,0x7f,0x20,0x4d,0x7f,0x29,0x10,0xf0,0xf9,0xee,0xf1,0x7f,0xa9,
	        0xff,0xcd,0xf1,0x7f,0xd0,0x05,0xa9,0x00,0x8d,0xf1,0x7f,0xa9,0x00,0x8d,0xf0,0x7f,
	        0xad,0xf1,0x7f,0x20,0x00,0x80,0xa9,0x01,0x8d,0xf0,0x7f,0x4c,0xe5,0x7e,0x48,0x8a,
	        0x48,0x98,0x48,0xa9,0x20,0x8d,0x06,0x20,0x8d,0x06,0x20,0xad,0xf1,0x7f,0x4a,0x4a,
	        0x4a,0x4a,0x09,0x10,0x8d,0x07,0x20,0xad,0xf1,0x7f,0x29,0x0f,0x09,0x10,0x8d,0x07,
	        0x20,0xa9,0x00,0x8d,0x06,0x20,0x8d,0x06,0x20,0x8d,0x05,0x20,0x8d,0x05,0x20,0xad,
	        0xf0,0x7f,0xf0,0x03,0x20,0x00,0x80,0x68,0xa8,0x68,0xaa,0x68,0x40,0xa0,0x08,0xa2,
	        0x01,0x8e,0x16,0x40,0xca,0x8e,0x16,0x40,0xad,0x16,0x40,0x6a,0x8a,0x2a,0xaa,0x88,
	        0xd0,0xf6,0xcd,0xf2,0x7f,0xf0,0x04,0x8d,0xf2,0x7f,0x60,0xa9,0x00,0x60,0x00,0x00,
	        0x38,0x4c,0xc6,0xc6,0xc6,0x64,0x38,0x00,0x18,0x38,0x18,0x18,0x18,0x18,0x7e,0x00,
	        0x7c,0xc6,0x0e,0x3c,0x78,0xe0,0xfe,0x00,0x7e,0x0c,0x18,0x3c,0x06,0xc6,0x7c,0x00,
	        0x1c,0x3c,0x6c,0xcc,0xfe,0x0c,0x0c,0x00,0xfc,0xc0,0xfc,0x06,0x06,0xc6,0x7c,0x00,
	        0x3c,0x60,0xc0,0xfc,0xc6,0xc6,0x7c,0x00,0xfe,0xc6,0x0c,0x18,0x30,0x30,0x30,0x00,
	        0x7c,0xc6,0xc6,0x7c,0xc6,0xc6,0x7c,0x00,0x7c,0xc6,0xc6,0x7e,0x06,0x0c,0x78,0x00,
	        0x38,0x6c,0xc6,0xc6,0xfe,0xc6,0xc6,0x00,0xfc,0xc6,0xc6,0xfc,0xc6,0xc6,0xfc,0x00,
	        0x3c,0x66,0xc0,0xc0,0xc0,0x66,0x3c,0x00,0xf8,0xcc,0xc6,0xc6,0xc6,0xcc,0xf8,0x00,
	        0xfe,0xc0,0xc0,0xfc,0xc0,0xc0,0xfe,0x00,0xfe,0xc0,0xc0,0xfc,0xc0,0xc0,0xc0,0x00,
	        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0e,0x7f,0x40,0x7e,0x0e,0x7f
	    };

	uint8 play_prg2[0x1C0] =
	    {
	        0x78,0xd8,0xa2,0xff,0x9a,0xad,0x02,0x20,0x10,0xfb,0xa9,0x00,0xaa,0x95,0x00,0x9d,
	        0x00,0x01,0x9d,0x00,0x02,0x9d,0x00,0x03,0x9d,0x00,0x04,0x9d,0x00,0x05,0x9d,0x00,
	        0x06,0x9d,0x00,0x07,0xe8,0xd0,0xe6,0xa9,0x80,0x8d,0x00,0x20,0xa9,0x00,0x8d,0x01,
	        0x20,0xa9,0x00,0x8d,0x06,0x20,0x8d,0x06,0x20,0xaa,0x8d,0x07,0x20,0xe8,0xe0,0x10,
	        0xd0,0xf8,0xa9,0x01,0x8d,0x06,0x20,0xa9,0x00,0x8d,0x06,0x20,0xa2,0x00,0xa0,0x00,
	        0xbd,0x70,0xff,0x8d,0x07,0x20,0xe8,0xc8,0xc0,0x08,0xd0,0xf4,0xa0,0x00,0x8c,0x07,
	        0x20,0x8c,0x07,0x20,0x8c,0x07,0x20,0x8c,0x07,0x20,0x8c,0x07,0x20,0x8c,0x07,0x20,
	        0x8c,0x07,0x20,0x8c,0x07,0x20,0xe0,0x80,0xd0,0xd6,0xa9,0x3f,0x8d,0x06,0x20,0xa9,
	        0x00,0x8d,0x06,0x20,0xa9,0x0e,0x8d,0x07,0x20,0xa9,0x30,0x8d,0x07,0x20,0xa9,0x00,
	        0x8d,0xf1,0xff,0xa9,0x0e,0x8d,0x01,0x20,0xa9,0x00,0x8d,0xf1,0xff,0x20,0x00,0x80,
	        0xa9,0x01,0x8d,0xf0,0xff,0x20,0x4d,0xff,0x29,0x10,0xf0,0xf9,0xee,0xf1,0xff,0xa9,
	        0xff,0xcd,0xf1,0xff,0xd0,0x05,0xa9,0x00,0x8d,0xf1,0xff,0xa9,0x00,0x8d,0xf0,0xff,
	        0xad,0xf1,0xff,0x20,0x00,0x80,0xa9,0x01,0x8d,0xf0,0xff,0x4c,0xe5,0xfe,0x48,0x8a,
	        0x48,0x98,0x48,0xa9,0x20,0x8d,0x06,0x20,0x8d,0x06,0x20,0xad,0xf1,0xff,0x4a,0x4a,
	        0x4a,0x4a,0x09,0x10,0x8d,0x07,0x20,0xad,0xf1,0xff,0x29,0x0f,0x09,0x10,0x8d,0x07,
	        0x20,0xa9,0x00,0x8d,0x06,0x20,0x8d,0x06,0x20,0x8d,0x05,0x20,0x8d,0x05,0x20,0xad,
	        0xf0,0xff,0xf0,0x03,0x20,0x00,0x80,0x68,0xa8,0x68,0xaa,0x68,0x40,0xa0,0x08,0xa2,
	        0x01,0x8e,0x16,0x40,0xca,0x8e,0x16,0x40,0xad,0x16,0x40,0x6a,0x8a,0x2a,0xaa,0x88,
	        0xd0,0xf6,0xcd,0xf2,0xff,0xf0,0x04,0x8d,0xf2,0xff,0x60,0xa9,0x00,0x60,0x00,0x00,
	        0x38,0x4c,0xc6,0xc6,0xc6,0x64,0x38,0x00,0x18,0x38,0x18,0x18,0x18,0x18,0x7e,0x00,
	        0x7c,0xc6,0x0e,0x3c,0x78,0xe0,0xfe,0x00,0x7e,0x0c,0x18,0x3c,0x06,0xc6,0x7c,0x00,
	        0x1c,0x3c,0x6c,0xcc,0xfe,0x0c,0x0c,0x00,0xfc,0xc0,0xfc,0x06,0x06,0xc6,0x7c,0x00,
	        0x3c,0x60,0xc0,0xfc,0xc6,0xc6,0x7c,0x00,0xfe,0xc6,0x0c,0x18,0x30,0x30,0x30,0x00,
	        0x7c,0xc6,0xc6,0x7c,0xc6,0xc6,0x7c,0x00,0x7c,0xc6,0xc6,0x7e,0x06,0x0c,0x78,0x00,
	        0x38,0x6c,0xc6,0xc6,0xfe,0xc6,0xc6,0x00,0xfc,0xc6,0xc6,0xfc,0xc6,0xc6,0xfc,0x00,
	        0x3c,0x66,0xc0,0xc0,0xc0,0x66,0x3c,0x00,0xf8,0xcc,0xc6,0xc6,0xc6,0xcc,0xf8,0x00,
	        0xfe,0xc0,0xc0,0xfc,0xc0,0xc0,0xfe,0x00,0xfe,0xc0,0xc0,0xfc,0xc0,0xc0,0xc0,0x00,
	        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0e,0xff,0x40,0xfe,0x0e,0xff
	    };

	if(chip_type & 4)
	{
		// $FE40 - $FFFF (FDS)
		for(uint32 i = 0; i < 0x1C0; i++)
		{
			wram2[0x7E40+i] = play_prg2[i];
		}
		wram2[0x7E40+0x09E] = ROM_banks[0xA]; // INIT_ADR
		wram2[0x7E40+0x09F] = ROM_banks[0xB];
		wram2[0x7E40+0x0C4] = ROM_banks[0xA]; // INIT_ADR
		wram2[0x7E40+0x0C5] = ROM_banks[0xB];
		wram2[0x7E40+0x105] = ROM_banks[0xC]; // PLAY_ADR
		wram2[0x7E40+0x106] = ROM_banks[0xD];
		wram2[0x7E40+0x0B0] = ROM_banks[0x6]; // MAX_SONG
	}
	else
	{
		// $7E40 - $7FFF (non FDS)
		for(uint32 i = 0; i < 0x1C0; i++)
		{
			wram1[0x1E40+i] = play_prg1[i];
		}
		wram1[0x1E40+0x09E] = ROM_banks[0xA]; // INIT_ADR
		wram1[0x1E40+0x09F] = ROM_banks[0xB];
		wram1[0x1E40+0x0C4] = ROM_banks[0xA]; // INIT_ADR
		wram1[0x1E40+0x0C5] = ROM_banks[0xB];
		wram1[0x1E40+0x105] = ROM_banks[0xC]; // PLAY_ADR
		wram1[0x1E40+0x106] = ROM_banks[0xD];
		wram1[0x1E40+0x0B0] = ROM_banks[0x6]; // MAX_SONG
		wram2[0x7FFA] = 0x0E;
		wram2[0x7FFB] = 0x7F;
		wram2[0x7FFC] = 0x40;
		wram2[0x7FFD] = 0x7E;
		wram2[0x7FFE] = 0x0E;
		wram2[0x7FFF] = 0x7F;
	}
}
/////////////////////////////////////////////////////////////////////

#endif
