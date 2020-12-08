#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 215
void NES_mapper215::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(num_8k_ROM_banks-4,num_8k_ROM_banks-3,num_8k_ROM_banks-2,num_8k_ROM_banks-1);

	// set PPU bank pointers
	set_PPU_banks(num_1k_VROM_banks-7,num_1k_VROM_banks-6,num_1k_VROM_banks-5,num_1k_VROM_banks-5,num_1k_VROM_banks-4,num_1k_VROM_banks-3,num_1k_VROM_banks-2,num_1k_VROM_banks-1);
	set_mirroring(NES_PPU::MIRROR_HORIZ);
}


void NES_mapper215::MemoryWrite(uint32 addr, uint8 data)
{
	int tmp2=addr&0x6;
	int tmp1=tmp2+((tmp2==0x06)?0:(addr&1));

	int v,n;
	n = tmp2 << 1;
	v = (tmp2+((tmp2==0x06)?0:(addr&0x01))) << 1;
	set_CPU_banks(n,n+1,v,v+1);
	v = tmp1 << 3;
	set_PPU_banks(v,v+1,v+2,v+3,v+4,v+5,v+6,v+7);
	set_mirroring((addr&0x10) ? NES_PPU::MIRROR_HORIZ: NES_PPU::MIRROR_VERT);


	//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) <<  endl);
}
/////////////////////////////////////////////////////////////////////

#endif
