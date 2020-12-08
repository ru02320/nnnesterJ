#ifdef _NES_MAPPER_CPP_

// UNIF   Novel

void NES_UNIFmapper1::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,2,3);
	// set PPU bank pointers
	set_PPU_banks(0,1,2,3,4,5,6,7);
	set_mirroring(NES_PPU::MIRROR_VERT);
}


void NES_UNIFmapper1::MemoryWrite(uint32 addr, uint8 data)
{
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);

	unsigned char n = (unsigned char)addr;
	unsigned char pn = n&7;
	pn <<= 3;
	set_PPU_banks(pn,pn+1,pn+2,pn+3,pn+4,pn+5,pn+6,pn+7);
	n&=3;
	n<<=2;
	set_CPU_banks(n,n+1,n+2,n+3);
}

#endif
