#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 191
void NES_mapper191::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,2,3);

	// set PPU bank pointers
	set_PPU_banks(num_1k_VROM_banks-7,num_1k_VROM_banks-6,num_1k_VROM_banks-5,num_1k_VROM_banks-5,num_1k_VROM_banks-4,num_1k_VROM_banks-3,num_1k_VROM_banks-2,num_1k_VROM_banks-1);
//	set_PPU_banks(0,1,2,3,4,5,6,7);
	set_mirroring(NES_PPU::MIRROR_HORIZ);
}


void  NES_mapper191::Sync(){
	int v;

	v = ((regs[1]<<3)+regs[0])<<2;
	set_PPU_bank0(v);
	set_PPU_bank1(v+1);
	v = ((regs[1]<<3)+regs[1])<<2 + 2;
	set_PPU_bank2(v);
	set_PPU_bank3(v+1);
	v = ((regs[1]<<3)+regs[2])<<2;
	set_PPU_bank4(v);
	set_PPU_bank5(v+1);
	v = ((regs[1]<<3)+regs[3])<<2 + 2;
	set_PPU_bank6(v);
	set_PPU_bank7(v+1);

}


void  NES_mapper191::WriteHighRegs(uint32 addr, uint8 data){
	int v;
	switch(addr){
		case 0x4100:
			regs[0] = data/* & 0x07*/;
			break;
		case 0x4101:
			if(regs[0] < 4){
				regs[regs[0]] = data & 0x07;
				Sync();
			}
			else{
				switch(regs[0]){
					case 4:
						regs[1] = data & 0x07;
						Sync();
						break;
					case 5:
						v = (data & 0x07) << 2;
						set_CPU_banks(v,v+1,v+2,v+3);
						break;
					case 7:
						set_mirroring((data ^ 2) ? NES_PPU::MIRROR_HORIZ: NES_PPU::MIRROR_VERT);
						break;
				}
			}
			break;
	}
}

/////////////////////////////////////////////////////////////////////


#endif
