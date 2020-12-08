#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 213
void NES_mapper213::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,2,3);

	// set PPU bank pointers
	set_PPU_banks(0,1,2,3,4,5,6,7);
	set_mirroring(NES_PPU::MIRROR_HORIZ);
}


void NES_mapper213::MemoryWrite(uint32 addr, uint8 data)
{
	int v;
	v = (addr & 0x06) << 1;
	set_CPU_banks(v, v+1, v+2, v+3);
	v =  addr & 0x38;
	set_PPU_banks(v,v+1,v+2,v+3,v+4,v+5,v+6,v+7);
	//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) <<  endl);
}
/////////////////////////////////////////////////////////////////////

#endif
