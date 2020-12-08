
/////////////////////////////////////////////////////////////////////
// Mapper 211
class NES_mapper211 : public NES_mapper
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_mapper211(NES* parent) : NES_mapper(parent) {}
	~NES_mapper211() {}

	void  Reset();
	
	void  MemoryWriteLow(uint32 addr, uint8 data);
	uint8 MemoryReadLow(uint32 addr);
//	void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
	void  MemoryWrite(uint32 addr, uint8 data);
	void  HSync(uint32 scanline);

protected:
	uint8 cregs[2];
	uint8 nregs[4];
	uint8 mregs[3];
	uint8 tekker;
	uint8 chr[8];
	uint8 prg[4];

	uint32 irq_counter, irq_latch;
	uint8 irq_enabled;

private:
	void SetPRG();
	void SetCHR();
	void SetMIR();

};
/////////////////////////////////////////////////////////////////////

