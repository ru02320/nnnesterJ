#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 250
void NES_mapper250::Reset()
{
	// set CPU bank pointers
	//  set_CPU_banks(0,1,2,3);
	set_CPU_banks(0,1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);

	// set PPU bank pointers
	set_PPU_banks(0,1,2,3,4,5,6,7);

	set_mirroring(NES_PPU::MIRROR_VERT);

	//regs[0] = regs[2] = 0x0F;
	//regs[1] = regs[3] = 0x00;
}


void NES_mapper250::MemoryWrite(uint32 addr, uint8 data)
{
	switch(addr & 0xE100){
	case 0x8000:
		//			set_CPU_bank4(data&0x0f);
		break;
	case 0x8100:
		//			set_CPU_bank5(data&0x0f);
		break;
	case 0xe000:
		//			set_CPU_bank6(data&0x0f);
		break;
	case 0xe100:
		//			set_CPU_bank7(data&0x0f);
		break;
	}
	//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) <<  endl);
}
/////////////////////////////////////////////////////////////////////

#endif
