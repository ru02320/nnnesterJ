#ifdef _NES_MAPPER_CPP_

// UNIF   Supervision 16 in 1

void NES_UNIFmapper3::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0x100,0x101,0x102,0x103);
	// set PPU bank pointers
//	set_PPU_banks(0,1,2,3,4,5,6,7);
//	set_mirroring(NES_PPU::MIRROR_VERT);
	regs[0] = regs[1] = 0;
}


void NES_UNIFmapper3::Setprg(){
	set_CPU_bank_unif(((regs[0]&0xF)<<4)|0xF, 3);
	if(regs[0]&0x10){
		uint32 tmb = (regs[0]&0xF)<<4;
		set_CPU_bank_unif(tmb|((regs[1]&7)<<1), 4);
		set_CPU_bank_unif(tmb|((regs[1]&7)<<1)+1, 5);
		set_CPU_bank_unif(tmb|0x0E, 6);
		set_CPU_bank_unif((tmb|0x0E)+1, 7);
	}
	else{
		set_CPU_banks(0x100,0x101,0x102,0x103);
	}

	if(regs[0]&0x20){
		set_mirroring(NES_PPU::MIRROR_HORIZ);
	}
	else{
		set_mirroring(NES_PPU::MIRROR_VERT);
	}
}


void NES_UNIFmapper3::MemoryWriteSaveRAM(uint32 addr, uint8 data)
{
	if(!(regs[0]&0x10)){
		regs[0]=data;
		Setprg();
	}
}

void NES_UNIFmapper3::MemoryWrite(uint32 addr, uint8 data)
{
	regs[1]=data;
	Setprg();
}

#endif
