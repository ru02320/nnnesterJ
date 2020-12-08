#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 212
void NES_mapper212::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(num_8k_ROM_banks-4,num_8k_ROM_banks-3,num_8k_ROM_banks-2,num_8k_ROM_banks-1);

	// set PPU bank pointers
	set_PPU_banks(num_1k_VROM_banks-7,num_1k_VROM_banks-6,num_1k_VROM_banks-5,num_1k_VROM_banks-5,num_1k_VROM_banks-4,num_1k_VROM_banks-3,num_1k_VROM_banks-2,num_1k_VROM_banks-1);
	set_mirroring(NES_PPU::MIRROR_HORIZ);
}


void NES_mapper212::MemoryWrite(uint32 addr, uint8 data)
{
	int v;
	if((addr &0x4000) == 0x4000){
		v = (addr & 0x06) << 1;
		set_CPU_banks(v,v+1,v+2,v+3);
	}
	else {
		v = (addr & 0x07) << 1;
		set_CPU_banks(v,v+1,v,v+1);
	}
	v = (addr & 7) << 3;
	set_PPU_banks(v,v+1,v+2,v+3,v+4,v+5,v+6,v+7);
	set_mirroring((addr&0x08) ? NES_PPU::MIRROR_HORIZ: NES_PPU::MIRROR_VERT);


	//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) <<  endl);
}
/////////////////////////////////////////////////////////////////////

#endif
