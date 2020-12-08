#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 201
void NES_mapper201::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,2,3);
	// set PPU bank pointers
	set_PPU_banks(0,1,2,3,4,5,6,7);
	set_mirroring(NES_PPU::MIRROR_HORIZ);
}


void NES_mapper201::MemoryWrite(uint32 addr, uint8 data)
{
	int v,n = addr & 0x07;
	v = n << 2;
	set_CPU_banks(v,v+1,v+2,v+3);
	v = n << 3;
	set_PPU_banks(v,v+1,v+2,v+3,v+4,v+5,v+6,v+7);
#if 0
	if(addr & 0x08){
		int v,n = addr & 0x03;
		v = n << 2;
		set_CPU_banks(v,v+1,v+2,v+3);
		v = n << 3;
		set_PPU_banks(v,v+1,v+2,v+3,v+4,v+5,v+6,v+7);
	}
	else{
		set_CPU_banks(0,1,2,3);
		set_PPU_banks(0,1,2,3,4,5,6,7);
	}
#endif
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2) <<  endl);
}
/////////////////////////////////////////////////////////////////////

#endif
