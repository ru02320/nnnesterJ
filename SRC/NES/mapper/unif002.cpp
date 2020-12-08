#ifdef _NES_MAPPER_CPP_

// UNIF  Mario

void NES_UNIFmapper2::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,2,3);
	set_CPU_bank_unif(4, 3);
	// set PPU bank pointers
	set_PPU_banks(0,1,2,3,4,5,6,7);
	set_mirroring(NES_PPU::MIRROR_VERT);
	memset(wram, 0, 0x1000);
}

uint8 NES_UNIFmapper2::MemoryReadLow(uint32 addr)
{
//	if(addr >= 0x7000 && addr < 0x8000)
	if((addr&0xFFF) < 0x800)
		return wram[addr&0x7FF];
	return parent_NES->cpu->GetByte(addr);
}

void NES_UNIFmapper2::MemoryWriteLow(uint32 addr, uint8 data)
{
	if(addr >= 0x7000 && addr < 0x8000)
		wram[addr&0xFFF] = data;
}

void NES_UNIFmapper2::MemoryWrite(uint32 addr, uint8 data)
{
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);

}

#endif
