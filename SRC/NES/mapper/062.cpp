#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 62
void NES_mapper62::Reset()
{
	set_CPU_banks(0,1,2,3);
	set_PPU_banks(0,1,2,3,4,5,6,7);
}

void NES_mapper62::MemoryWriteSaveRAM(uint32 addr, uint8 data){
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
}

void NES_mapper62::MemoryWriteLow(uint32 addr, uint8 data){
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
}

void NES_mapper62::MemoryWrite(uint32 addr, uint8 data)
{
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
	switch(addr & 0xFF00){
		case 0x8100:
			set_CPU_bank4(data);
			set_CPU_bank5(data+1);
			break;
		case 0x8500:
			set_CPU_bank4(data);
			break;
		case 0x8700:
			set_CPU_bank5(data);
			break;
		set_PPU_bank0(data);
		set_PPU_bank1(data+1);
		set_PPU_bank2(data + 2);
		set_PPU_bank3(data + 3);
		set_PPU_bank4(data + 4);
		set_PPU_bank5(data + 5);
		set_PPU_bank6(data + 6);
		set_PPU_bank7(data + 7);
	}

}
/////////////////////////////////////////////////////////////////////

#endif
