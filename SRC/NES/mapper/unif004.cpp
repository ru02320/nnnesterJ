#ifdef _NES_MAPPER_CPP_

// UNIF   Super24 in 1


void NES_UNIFmapper4::Reset()
{
	irq_counter = irq_latch =0;
	irq_enabled = 0;
	memset(dregs, 0, 8);
	regs[0] = regs[1] =0;
	regs[2]=0x24;
	regs[3]=159;
	regs[4]=0;
	dregs[7]=1;
//	parent_NES->CYCLES_PER_LINE=120;
	oMMC3PRG(0);
	oMMC3CHR(0);
//	set_mirroring(NES_PPU::MIRROR_VERT);
}


void NES_UNIFmapper4::setchr1(uint8 pn, uint8 data)
{
	if(regs[2]&0x20){
//		set_VRAM_bank_unif(data, pn);
	}
	else{
		uint32 tdata= (uint32)data;
		tdata+=(regs[4]<<3);
		set_VRAM_bank_unif(tdata/*&0x7FF*/, pn);
	}
}

void NES_UNIFmapper4::setchr2(uint8 pn, uint8 data)
{
	if(regs[2]&0x20){
//		set_VRAM_bank_unif(data, pn);
//		set_VRAM_bank_unif(data+1, pn+1);
	}
	else{
		uint32 tdata = (uint32)data;
		tdata+=(regs[4]<<3);
		set_VRAM_bank_unif(tdata/*&0x7FF*/, pn);
		set_VRAM_bank_unif(tdata+1/*&0x7FF+1*/, pn+1);
	}
}


static unsigned char mask8[8]={63,31,15,1,3,0,0,0};


void NES_UNIFmapper4::oMMC3PRG(uint8 data)
{
	uint32 tmp;

	tmp = dregs[7];
	tmp&=mask8[regs[2]&7];
	tmp|=(regs[3]<<1);
	set_CPU_bank_unif(tmp, 5);
	tmp = 0xff;
	tmp&=mask8[regs[2]&7];
	tmp|=(regs[3]<<1);
	set_CPU_bank_unif(tmp, 7);
	if(data&0x40){
		tmp = dregs[6];
		tmp&=mask8[regs[2]&7];
		tmp|=(regs[3]<<1);
		set_CPU_bank_unif(tmp, 6);

		tmp = 0xfe;
		tmp&=mask8[regs[2]&7];
		tmp|=(regs[3]<<1);
		set_CPU_bank_unif(tmp, 4);
	}
	else{
		tmp = dregs[6];
		tmp&=mask8[regs[2]&7];
		tmp|=(regs[3]<<1);
		set_CPU_bank_unif(tmp, 4);	//

		tmp = 0xfe;
		tmp&=mask8[regs[2]&7];
		tmp|=(regs[3]<<1);
		set_CPU_bank_unif(tmp, 6);	//
	}
}

void NES_UNIFmapper4::oMMC3CHR(uint8 data)
{
	uint8 cm=(data&0x80)>>5;
	setchr2(cm^0,dregs[0]);
	setchr2(cm^2,dregs[1]);
	setchr1(cm^4,dregs[2]);
	setchr1(cm^5,dregs[3]);
	setchr1(cm^6,dregs[4]);
	setchr1(cm^7,dregs[5]);
}

void NES_UNIFmapper4::MemoryWriteLow(uint32 addr, uint8 data)
{
	switch(addr){
		case 0x5ff0:
			regs[2]=data;
			oMMC3PRG(regs[1]);
			oMMC3CHR(regs[1]);
			break;
		case 0x5FF1:
			regs[3]=data;
			oMMC3PRG(regs[1]);
			break;
		case 0x5FF2:
			regs[4]=data;
			oMMC3CHR(regs[1]);
			break;
	}
}


void NES_UNIFmapper4::MemoryWrite(uint32 addr, uint8 data)
{
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
	switch(addr&0xE001){
		case 0x8000:
			if((data&0x40) != (regs[1]&0x40))
				oMMC3PRG(data);
			if((data&0x80) != (regs[1]&0x80))
				oMMC3CHR(data);
			regs[1] = data;
			break;
		case 0x8001:
		{
			uint8 pm = (regs[1]&0x80)>>5;
			dregs[regs[1]&0x7] = data;
			switch(regs[1]&0x07){
				case 0:
					setchr2(pm^0,data);
					break;
				case 1:
					setchr2(pm^2, data);
					break;
				case 2:
					setchr1(pm^4, data);
					break;
				case 3:
					setchr1(pm^5, data);
					break;
				case 4:
					setchr1(pm^6, data);
					break;
				case 5:
					setchr1(pm^7, data);
					break;
				case 6:
					if(regs[1]&0x40){
						uint32 tmp = data;
						tmp&=mask8[regs[2]&7];
						tmp|=(regs[3]<<1);
						set_CPU_bank_unif(tmp, 6);	//
					}
					else{
						uint32 tmp = data;
						tmp&=mask8[regs[2]&7];
						tmp|=(regs[3]<<1);
						set_CPU_bank_unif(tmp, 4);
					}
					break;
				case 7:
					{
						uint32 tmp = data;
						tmp&=mask8[regs[2]&7];
						tmp|=(regs[3]<<1);
						set_CPU_bank_unif(tmp, 5);
					}
					break;
			}
		}
			break;
		case 0xA000:
			if(data&1)
				set_mirroring(NES_PPU::MIRROR_HORIZ);
			else
				set_mirroring(NES_PPU::MIRROR_VERT);
			break;

		case 0xc000:
			irq_latch=data;
			if(regs[0]==1)
				irq_counter=irq_latch;
			break;
		case 0xc001:
			regs[0]=1;
			irq_counter=irq_latch;
			break;
		case 0xE000:
			irq_enabled=0;
			if(regs[0]==1)
				irq_counter=irq_latch;
			break;
		case 0xE001:
			irq_enabled=1;
			if(regs[0]==1)
				irq_counter=irq_latch;
			break;
	}
}


void NES_UNIFmapper4::HSync(uint32 scanline)
{
	if(irq_enabled){
		if(parent_NES->ppu->spr_enabled() || parent_NES->ppu->bg_enabled()){
			if(scanline >= 0 && scanline <= 240){
				regs[0]=0;
				if(irq_counter-- == 0){
					parent_NES->cpu->DoPendingIRQ();
					regs[0]=1;
//					irq_counter = irq_latch;
//					irq_enabled = 0;
				}
			}
		}
	}
}

#endif
