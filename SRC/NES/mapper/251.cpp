#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 251
void NES_mapper251::Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0,1,num_8k_ROM_banks-2,num_8k_ROM_banks-1);
//	set_CPU_banks(0x30, 0x31, 0x3E, 0x3F);

	// set PPU bank pointers
//	set_PPU_banks(0,1,2,3,4,5,6,7);

	set_mirroring(NES_PPU::MIRROR_VERT);

	memset(regs, 0, 10);
	memset(bregs, 0, 4);
	//regs[0] = regs[2] = 0x0F;
	//regs[1] = regs[3] = 0x00;
}

void NES_mapper251::banksync()
{
	uint32 chr[6];
	uint32 prg[4];


	for(int i=0; i<6; i++){
		chr[i] = (regs[i] | (bregs[1] << 4)) & ((bregs[2] << 4) | 0x0F);
	}

	if(regs[8]&0x80){
		set_PPU_banks(chr[2],chr[3],chr[4],chr[5],chr[0],chr[0]+1,chr[1],chr[1]+1);
	}else{
		set_PPU_banks(chr[0],chr[0]+1,chr[1],chr[1]+1,chr[2],chr[3],chr[4],chr[5]);
	}

	prg[0] = (regs[6] & ((bregs[3] & 0x3F) ^ 0x3F)) | (bregs[1]);
	prg[1] = (regs[7] & ((bregs[3] & 0x3F) ^ 0x3F)) | (bregs[1]);
	prg[2] = prg[3] = ((bregs[3] & 0x3F) ^ 0x3F) | (bregs[1]);
	prg[2] &= (num_8k_ROM_banks-1);

	if(regs[8]&0x40){
		set_CPU_banks(prg[2],prg[1],prg[0],prg[3]);
	}else{
		set_CPU_banks(prg[0],prg[1],prg[2],prg[3]);
	}
}


void NES_mapper251::MemoryWriteSaveRAM(uint32 addr, uint8 data)
{
	switch(addr & 0xE001){
		case 0x6000:
			if(regs[9]){
				bregs[regs[10]++] = data;
				if(regs[10] == 4){
					regs[10] = 0;
					banksync();
				}
			}
			break;
	}
}

void NES_mapper251::MemoryWrite(uint32 addr, uint8 data)
{
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2) <<  endl);
	switch(addr & 0xE001){
#if 1
	case 0x8000:
		regs[8]=data;
		banksync();
		break;
	case 0x8001:
		regs[regs[8]&7] = data;
		banksync();
		break;
	case 0xA001:
		if(data & 0x80){
			regs[9] = 1;
			regs[10] = 0;
		}else{
			regs[9] = 0;
		}
		break;
#endif
	}

}
/////////////////////////////////////////////////////////////////////

#endif
