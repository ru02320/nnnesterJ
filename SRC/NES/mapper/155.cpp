
/////////////////////////////////////////////////////////////////////
// Mapper 155
void NES_mapper155::Reset()
{
	patch = 1;

	wram = parent_NES->mapper_extram;
	parent_NES->mapper_extramsize = 8*1024;
	patch = 1;
	wram_bank = 0;
	wram_flag = 0;
	wram_count = 0;
	NES_6502::Context context;
	parent_NES->cpu->GetContext(&context);
	context.mem_page[3] = wram;
	parent_NES->cpu->SetContext(&context);
	for(uint32 i = 0; i < 8*1024; i++){
		wram[i] = parent_NES->ReadSaveRAM(i);
	}

	write_count = 0;
	bits = 0x00;
	regs[0] = 0x0C; // reflects initial ROM state
	regs[1] = 0x00;
	regs[2] = 0x00;
	regs[3] = 0x00;
	{
		uint32 size_in_K = num_8k_ROM_banks * 8;

		if(size_in_K == 1024)
		{
			MMC1_Size = MMC1_1024K;
		}
		else if(size_in_K == 512)
		{
			MMC1_Size = MMC1_512K;
		}
		else
		{
			MMC1_Size = MMC1_SMALL;
		}
	}
	MMC1_256K_base = 0; // use first 256K
	MMC1_swap = 0;

	if(MMC1_Size == MMC1_SMALL)
	{
		// set two high pages to last two banks
		MMC1_HI1 = num_8k_ROM_banks-2;
		MMC1_HI2 = num_8k_ROM_banks-1;
	}
	else
	{
		// set two high pages to last two banks of current 256K region
		MMC1_HI1 = (256/8)-2;
		MMC1_HI2 = (256/8)-1;
	}

	// set CPU bank pointers
	MMC1_bank1 = 0;
	MMC1_bank2 = 1;
	MMC1_bank3 = MMC1_HI1;
	MMC1_bank4 = MMC1_HI2;

	MMC1_set_CPU_banks();
}

/////////////////////////////////////////////////////////////////////

