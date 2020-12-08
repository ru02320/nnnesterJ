#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 222
void NES_mapper222::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0, 1, num_8k_ROM_banks-2, num_8k_ROM_banks-1);
	// set PPU bank pointers
	if(num_1k_VROM_banks)
	{
		set_PPU_banks(0,1,2,3,4,5,6,7);
	}
//	set_mirroring(NES_PPU::MIRROR_HORIZ);
	// Dragon Ninja Pirates
	set_mirroring(NES_PPU::MIRROR_VERT);
}



void NES_mapper222::MemoryWrite(uint32 addr, uint8 data)
{
	switch(addr&0xf003){
	case 0x8000:
		set_CPU_bank4(data);
		break;
	case 0xA000:
		set_CPU_bank5(data);
		break;

	case 0xB000:
		set_PPU_bank0(data);
		break;
	case 0xB002:
		set_PPU_bank1(data);
		break;
	case 0xC000:
		set_PPU_bank2(data);
		break;
	case 0xC002:
		set_PPU_bank3(data);
		break;
	case 0xD000:
		set_PPU_bank4(data);
		break;
	case 0xD002:
		set_PPU_bank5(data);
		break;
	case 0xE000:
		set_PPU_bank6(data);
		break;
	case 0xE002:
		set_PPU_bank7(data);
		break;
	}
}


/////////////////////////////////////////////////////////////////////

#endif
