
/////////////////////////////////////////////////////////////////////
// Mapper 191
class NES_mapper191 : public NES_mapper
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_mapper191(NES* parent) : NES_mapper(parent) {}
	~NES_mapper191() {}

	void  Reset();
	void  Sync();

	//  uint8 MemoryReadLow(uint32);

	void  WriteHighRegs(uint32 addr, uint8 data);
//	void  MemoryWriteLow(uint32 addr, uint8 data);
//	void  MemoryWrite(uint32 addr, uint8 data);

protected:
	uint8 regs[4];

private:
};
/////////////////////////////////////////////////////////////////////

