
/////////////////////////////////////////////////////////////////////
// MMC3
class NES_mapperMMC3 : public NES_mapper
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_mapperMMC3(NES* parent) : NES_mapper(parent) {}
	~NES_mapperMMC3() {}

	virtual void Reset(){
		MMC3_Reset();
	};

	virtual uint8 MemoryReadLow(uint32 addr);
	virtual void MemoryReadSaveRAM(uint32 addr){};

	virtual uint8 MemoryReadHigh(uint32 addr) { return 0; };
	virtual boolean MemoryReadHighQuery(uint32 addr) { return FALSE; }

	virtual void  MemoryWriteLow(uint32 addr, uint8 data){};
	virtual void  MemoryWriteSaveRAM(uint32 addr, uint8 data){};
	virtual void  MemoryWrite(uint32 addr, uint8 data){
		MMC3_CMD_Write(addr, data);
	}

	virtual void  HSync(uint32 scanline);


//	void  PPU_A12_Rising();

protected:
	uint8  patch;
	uint8  regs[8];

	uint32 prg0,prg1;
	uint32 chr01,chr23,chr4,chr5,chr6,chr7;

	uint32 chr_swap() { return regs[0] & 0x80; }
	uint32 prg_swap() { return regs[0] & 0x40; }

	uint8 irq_enabled; // IRQs enabled
	uint8 irq_counter; // IRQ scanline counter, decreasing
	uint8 irq_latch;   // IRQ scanline counter latch

	uint8 irq_reload;

	uint8 vs_index; // VS Atari RBI Baseball and VS TKO Boxing

	void MMC3_Reset();

	void MMC3_set_CPU_banks();
	void MMC3_set_PPU_banks();
	void MMC3_set_Mir(uint8 data);

	void SNSS_fixup(); // HACK HACK HACK HACK

	void MMC3_CMD_Write(uint32 addr, uint8 data);
	
	virtual void SetPPU_Banks(uint32 addr, uint8 data);
	virtual void SetCPU_Banks(uint32 addr, uint8 data);
	virtual void SetMir(uint8 data){
		MMC3_set_Mir(data);
	};

private:
};
/////////////////////////////////////////////////////////////////////

