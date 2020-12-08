
/////////////////////////////////////////////////////////////
// Mapper 205  X
void  NES_mapper205::Reset(){
	m205_reg = 0;
	MMC3_Reset();
};

void NES_mapper205::MemoryWriteSaveRAM(uint32 addr, uint8 data){
//	LOG("-- " << HEX(addr,4) << " = " << HEX(data) << endl);
	if((addr & 0x6800) == 0x6800){
		m205_reg = data;
	}
	MMC3_set_CPU_banks();
	MMC3_set_PPU_banks();
}

void NES_mapper205::MemoryWriteLow(uint32 addr, uint8 data){
//	LOG("- " << HEX(addr,4) << " = " << HEX(data) << endl);
	MMC3_set_CPU_banks();
	MMC3_set_PPU_banks();
}

void NES_mapper205::SetPPU_Banks(uint32 addr, uint8 data){
	set_PPU_bank(addr, data | ((m205_reg & 3) * 128));
//	MMC3_set_PPU_banks();
}


void NES_mapper205::SetCPU_Banks(uint32 addr, uint8 data){
//	LOG("- " << HEX(addr,4) << " = " << HEX(data) << "  " << HEX(m205_reg) << endl);
	if(m205_reg & 0x02){
		set_CPU_bank(addr, (data & 0x0f)|((m205_reg&3)<<4));
	}
	else {
		set_CPU_bank(addr, (data & 0x1f)|((m205_reg&3)<<4));
	}
}


/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////
// Mapper 254  X
void  NES_mapper254::Reset(){

	wram = parent_NES->mapper_extram;
	parent_NES->mapper_extramsize = 0x2000;

	NES_6502::Context context;
	parent_NES->cpu->GetContext(&context);
	context.mem_page[3] = wram;
	parent_NES->cpu->SetContext(&context);

	memset(wram, 0, 0x2000);
	m254_regs[0] = 0;
	m254_regs[1] = 0;

	MMC3_Reset();
};


void NES_mapper254::MemoryWriteSaveRAM(uint32 addr, uint8 data){
//	LOG("W " << HEX(addr,4) << " = " << HEX(data) << endl);
}


uint8 NES_mapper254::MemoryReadHigh(uint32 addr){
	if(m254_regs[0]){
		return wram[addr - 0x6000];
	}
	else{
		return wram[addr - 0x6000]^m254_regs[1];
	}
}

boolean NES_mapper254::MemoryReadHighQuery(uint32 addr){
	if(addr >= 0x6000 && addr < 0x8000){
		return TRUE;
	}
	return FALSE;
}


void NES_mapper254::MemoryWrite(uint32 addr, uint8 data){
//	LOG("- " << HEX(addr,4) << " = " << HEX(data) << endl);
	switch(addr){
		case 0x8000:
			m254_regs[0] = 1;
//	LOG("WQW " << HEX(addr,4) << " = " << HEX(data) << endl);
			break;
		case 0xA001:
			m254_regs[1] = data;
//	LOG("WQW " << HEX(addr,4) << " = " << HEX(data) << endl);
			break;
	}
//	if(addr < 0xC000)
		MMC3_CMD_Write(addr, data);
}




/////////////////////////////////////////////////////////////
// Mapper 250  O

void NES_mapper250::MemoryWrite(uint32 addr, uint8 data){
	MMC3_CMD_Write((addr&0xE000)|((addr&0x400)>>10), addr&0xFF);
}


/////////////////////////////////////////////////////////////
// Mapper 249  x

void NES_mapper249::MemoryWriteLow(uint32 addr, uint8 data){
	if(addr == 0x5000){
		m249_reg = data;
		MMC3_set_CPU_banks();
		MMC3_set_PPU_banks();
	}
}

void NES_mapper249::SetPPU_Banks(uint32 addr, uint8 data){
	if(m249_reg & 2){
	}
}

void NES_mapper249::SetCPU_Banks(uint32 addr, uint8 data){
	if(m249_reg & 2){
	}

}


/////////////////////////////////////////////////////////////
// Mapper 245  x

void  NES_mapper245::Reset(){
	m245_reg = 0;
	MMC3_Reset();
}

void NES_mapper245::SetPPU_Banks(uint32 addr, uint8 data){
	m245_reg = data;
	set_PPU_bank(addr, data & 7);
	MMC3_set_CPU_banks();
}

void NES_mapper245::SetCPU_Banks(uint32 addr, uint8 data){
	set_CPU_bank(addr, (data & 0x3f)|((m245_reg&2)<<5));
}




/////////////////////////////////////////////////////////////
// Mapper 115  x

void  NES_mapper115::Reset(){
	m115_regs[0] = m115_regs[1] = 0;
	MMC3_Reset();
}

void NES_mapper115::SetPPU_Banks(uint32 addr, uint8 data){
	set_PPU_bank(addr, (uint32)data | (uint32)((m115_regs[1]&1)<<8));
}

void NES_mapper115::SetCPU_Banks(uint32 addr, uint8 data){
	set_CPU_bank(addr, data);
	if(m115_regs[0] & 0x80){
		uint32 n = (m115_regs[0] & 7) << 1;
		set_CPU_bank4(n);
		set_CPU_bank5(n+1);
	}
}

void NES_mapper115::MemoryWriteSaveRAM(uint32 addr, uint8 data){
	if(addr == 0x6000){
		m115_regs[0] = data;
	}
	else if(addr == 0x6001){
		m115_regs[1] = data;
	}
	MMC3_set_CPU_banks();
}


void NES_mapper115::MemoryWriteLow(uint32 addr, uint8 data){
	MMC3_set_CPU_banks();
}



/////////////////////////////////////////////////////////////
// Mapper 217  x

void  NES_mapper217::Reset(){
	m217_regs[0] = 0;
	m217_regs[1] = 0xFF;
	m217_regs[2] = 3;
	m217_regs[3] = 0;
	MMC3_Reset();
}

void NES_mapper217::SetPPU_Banks(uint32 addr, uint8 data){
	if(m217_regs[1] & 8)
		set_PPU_bank(addr, (uint32)data | ((m217_regs[1]&3)<<8));
	else
		set_PPU_bank(addr, (uint32)data&0x7F | ((m217_regs[1]&3)<<8) | ((m217_regs[1]&0x10)<<3));
}

void NES_mapper217::SetCPU_Banks(uint32 addr, uint8 data){
	if(m217_regs[0] & 0x80){
	LOG("P1 " << HEX(addr,4) << " = " << HEX(data)  << " m217_regs " << HEX(m217_regs[0]) << endl);
		uint32 n;
		n = ((m217_regs[0]&0x0F)|((m217_regs[1]&3)<<4) << 1);
		set_CPU_bank4(n);
		set_CPU_bank5(n+1);
		set_CPU_bank6(n);
		set_CPU_bank7(n+1);
	}
	else if(m217_regs[1] & 8){
		uint32 n= (data & 0x1F) | ((m217_regs[1] & 3) << 5);
	LOG("P2 " << HEX(addr,4) << " = " << HEX(data) << " n " << HEX(n) << endl);
		set_CPU_bank(addr, n);
	}
	else {
	LOG("P3 " << HEX(addr,4) << " = " << HEX(data) << endl);
		set_CPU_bank(addr, (data & 0x0F) | ((m217_regs[1] & 3) << 5) | (m217_regs[1] & 0x10));
	}
}

void NES_mapper217::MemoryWrite(uint32 addr, uint8 data){
	const uint8 perm[8] = {0, 6, 3, 7, 5, 2, 4, 1};

	if(!m217_regs[2]){
		MMC3_CMD_Write(addr, data);
		return;
	}
	switch(addr & 0xE001){
		case 0x8000:
			irq_counter = data;
			break;
		case 0x8001:
			MMC3_CMD_Write(0x8000, data);
			m217_regs[3] = 1;
			break;
		case 0xA000:
			if(m217_regs[3]){
				MMC3_CMD_Write(0x8001, (data&0xC0)|(perm[data&7]));
				m217_regs[3] = 0;
			}
			break;
		case 0xA001:
			set_mirroring((data&1)?(NES_PPU::MIRROR_HORIZ):(NES_PPU::MIRROR_VERT));
			break;
		case 0xC001:
			irq_enabled = 1;
			break;
		case 0xE000:
			irq_enabled = 0;
			break;
	}
}


void NES_mapper217::MemoryWriteLow(uint32 addr, uint8 data){
	LOG("WL " << HEX(addr,4) << " = " << HEX(data) << endl);
	switch(addr){
		case 0x5000:
			m217_regs[0] = data;
			MMC3_set_CPU_banks();
			break;
		case 0x5001:
			m217_regs[1] = data;
			MMC3_set_CPU_banks();
			break;
		case 0x5007:
			m217_regs[2] = data;
			break;
	}
}













