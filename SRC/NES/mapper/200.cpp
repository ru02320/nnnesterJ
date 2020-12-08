#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 200
void NES_mapper200::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,2,3);

	// set PPU bank pointers
	set_PPU_banks(0,1,2,3,4,5,6,7);

	set_mirroring(NES_PPU::MIRROR_HORIZ);
}


void NES_mapper200::MemoryWrite(uint32 addr, uint8 data)
{
	int v,n = addr & 0x07;

/*	ROM_BANK16(0x8000,A&0x07);
	ROM_BANK16(0xC000,A&0x07);
	VROM_BANK8(A&0x07);*/

	v = n << 1;
	set_CPU_banks(v,v+1,v,v+1);
	v = n << 3;
	set_PPU_banks(v,v+1,v+2,v+3,v+4,v+5,v+6,v+7);

	set_mirroring((addr&0x08) ? NES_PPU::MIRROR_HORIZ: NES_PPU::MIRROR_VERT);

	//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) <<  endl);
}
/////////////////////////////////////////////////////////////////////

#endif
