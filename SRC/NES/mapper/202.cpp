#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 202
void NES_mapper202::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,0,1);

	// set PPU bank pointers
	set_PPU_banks(0,1,2,3,4,5,6,7);
	set_mirroring(NES_PPU::MIRROR_HORIZ);
}


void NES_mapper202::MemoryWrite(uint32 addr, uint8 data)
{
	int tmp=addr&0x0e;

	int v, n;
	v = tmp;
	n = tmp+(((tmp&0x0c)==0x0c)?2:0);
	set_CPU_banks(v,v+1,n,n+1);
	v = tmp << 2;
	set_PPU_banks(v,v+1,v+2,v+3,v+4,v+5,v+6,v+7);
	set_mirroring((addr&0x01) ? NES_PPU::MIRROR_HORIZ: NES_PPU::MIRROR_VERT);

	//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) <<  endl);
}
/////////////////////////////////////////////////////////////////////

#endif
