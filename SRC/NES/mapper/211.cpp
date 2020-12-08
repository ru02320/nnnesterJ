#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 211
void NES_mapper211::Reset()
{
	tekker ^= 0xFF;
	// set CPU bank pointers
//	set_CPU_bank3(0);
/*
	set_CPU_bank4(num_8k_ROM_banks-4);
	set_CPU_bank5(num_8k_ROM_banks-3);
	set_CPU_bank6(num_8k_ROM_banks-2);
	set_CPU_bank7(num_8k_ROM_banks-1);
*/
	set_CPU_bank4(0);
	set_CPU_bank5(1);
	set_CPU_bank6(2);
	set_CPU_bank7(num_8k_ROM_banks-1);

	// set PPU bank pointers
//	if(num_1k_VROM_banks)
	{
		set_PPU_banks(0,1,2,3,4,5,6,7);
	}
//	memset(chr, 0, 8);
	int i;
	for(i=0; i<8;i++){
		chr[i] = i;
	}
	memset(prg, 0, 4);
	memset(cregs, 0, 2);
	memset(nregs, 0, 4);
	memset(mregs, 0, 3);

	irq_latch=irq_counter=irq_enabled=0;
}


void NES_mapper211::SetPRG(){
	switch(cregs[0]&3){
		case 1:
			set_CPU_bank4(prg[0] << 1);
			set_CPU_bank5(prg[0] << 1 + 1);
			set_CPU_bank6(prg[2] << 1);
			set_CPU_bank7(prg[2] << 1 + 1);
			break;
		case 2:
			set_CPU_bank4(prg[0]);
			set_CPU_bank5(prg[1]);
			set_CPU_bank6(prg[2]);
			set_CPU_bank7(num_8k_ROM_banks-1);
			break;
	}
}


void NES_mapper211::SetCHR(){
	uint32 tmp;
	switch(cregs[0]&0x18){
		case 0x00:
			tmp = chr[0]<<3;
			set_PPU_banks(tmp, tmp+1, tmp+2, tmp+3, tmp+4, tmp+5, tmp+6, tmp+7);
			break;
		case 0x08:
			set_PPU_banks(chr[0]<<2,chr[0]<<2+1,chr[0]<<2+2,chr[0]<<2+3,
				chr[4]<<2,chr[4]<<2+1,chr[4]<<2+2,chr[4]<<2+3);
			break;
		case 0x10:
			set_PPU_banks(chr[0]<<1,chr[0]<<1+1,chr[2]<<1,chr[2]<<1+1,
				chr[4]<<1,chr[4]<<1+1,chr[6]<<1,chr[6]<<1+1);
			break;
		case 0x18:
			set_PPU_banks(chr[0],chr[1],chr[2],chr[3],chr[4],chr[5],chr[6],chr[7]);
			break;
	}
}


void NES_mapper211::SetMIR(){
	if(cregs[0]&0x40){
	}
	else {
	}
}


uint8 NES_mapper211::MemoryReadLow(uint32 addr){
	switch(addr){
		case 0x5000:
			return tekker;
		case 0x5800:
			return mregs[0] * mregs[1];
			break;
		case 0x5801:
			return (mregs[0] * mregs[1]) >> 8;
			break;
		case 0x5803:
			return mregs[2];
			break;
	}
	return 0;
}


void NES_mapper211::MemoryWriteLow(uint32 addr, uint8 data)
{
	switch(addr){
		case 0x5800:
			mregs[0] = data;
			break;
		case 0x5801:
			mregs[1] = data;
			break;
		case 0x5803:
			mregs[2] = data;
			break;
	}
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
}


void NES_mapper211::MemoryWrite(uint32 addr, uint8 data)
{
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
	//	char f=0;
	switch(addr & 0xF007){
	case 0x8000:
	case 0x8001:
	case 0x8002:
	case 0x8003:
		prg[addr & 3] = data;
		SetPRG();
		break;

	case 0x9000:
	case 0x9001:
	case 0x9002:
	case 0x9003:
	case 0x9004:
	case 0x9005:
	case 0x9006:
	case 0x9007:
		chr[addr & 7] = data;
		SetCHR();
		break;

	case 0xB000:
	case 0xB001:
	case 0xB002:
	case 0xB003:
	case 0xB004:
	case 0xB005:
	case 0xB006:
	case 0xB007:
		nregs[addr & 3] = data & 3;
		SetMIR();
		break;
	case 0xC002:
		irq_enabled = 0;
		break;
	case 0xC003:
	case 0xC004:
		if(!irq_enabled){
			irq_enabled = 1;
			irq_counter = irq_latch;
		}
		break;
	case 0xC005:
		irq_counter = irq_latch = data;
		break;
	case 0xE000:
		cregs[0] = data;
		SetMIR();
		break;
	case 0xE001:
		cregs[1] = data;
		SetMIR();
		break;
	}
}

void NES_mapper211::HSync(uint32 scanline)
{
	if(irq_counter){
		irq_counter--;
	}
	if(irq_counter == 0){
		if(irq_enabled){
			parent_NES->cpu->DoIRQ();
			//			  irq_counter = irq_latch;
		}
		irq_enabled = 0;
	}
}


/////////////////////////////////////////////////////////////////////

#endif
