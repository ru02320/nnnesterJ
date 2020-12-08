#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 193
void NES_mapper193::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(num_8k_ROM_banks-4,num_8k_ROM_banks-3,num_8k_ROM_banks-2,num_8k_ROM_banks-1);

	// set PPU bank pointers
	set_PPU_banks(num_1k_VROM_banks-7,num_1k_VROM_banks-6,num_1k_VROM_banks-5,num_1k_VROM_banks-5,num_1k_VROM_banks-4,num_1k_VROM_banks-3,num_1k_VROM_banks-2,num_1k_VROM_banks-1);
//	set_PPU_banks(0,1,2,3,4,5,6,7);
	set_mirroring(NES_PPU::MIRROR_HORIZ);
}


void  NES_mapper193::WriteHighRegs(uint32 addr, uint8 data){
	MemoryWrite193(addr, data);
}


void  NES_mapper193::MemoryWriteLow(uint32 addr, uint8 data){
	MemoryWrite193(addr, data);
}


void NES_mapper193::MemoryWrite193(uint32 addr, uint8 data)
{
	int v;
	switch(addr&0x03){
		case 0:
			v = data;
			set_PPU_bank0(v);
			set_PPU_bank1(v+1);
			set_PPU_bank2(v+2);
			set_PPU_bank3(v+3);
			break;
		case 1:
			v = data;
			set_PPU_bank4(v);
			set_PPU_bank5(v+1);
			break;
		case 2:
			v = data;
			set_PPU_bank6(v);
			set_PPU_bank7(v+1);
			break;
		case 3:
			set_CPU_bank4(data);
			break;
	}
	//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) <<  endl);
}
/////////////////////////////////////////////////////////////////////


#endif
