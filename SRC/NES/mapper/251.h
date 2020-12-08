
/////////////////////////////////////////////////////////////////////
// Mapper 251
class NES_mapper251 : public NES_mapper
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_mapper251(NES* parent) : NES_mapper(parent) {}
	~NES_mapper251() {}

	void  Reset();
	//  uint8 MemoryReadLow(uint32);
	void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
	void  MemoryWrite(uint32 addr, uint8 data);

protected:
	uint8 regs[11];
	uint8 bregs[4];
	void banksync();

private:
};
/////////////////////////////////////////////////////////////////////

