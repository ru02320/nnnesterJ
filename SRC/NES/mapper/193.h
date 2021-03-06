
/////////////////////////////////////////////////////////////////////
// Mapper 193
class NES_mapper193 : public NES_mapper
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_mapper193(NES* parent) : NES_mapper(parent) {}
	~NES_mapper193() {}

	void  Reset();
	//  uint8 MemoryReadLow(uint32);
	void  MemoryWrite193(uint32 addr, uint8 data);

	void  WriteHighRegs(uint32 addr, uint8 data);
	void  MemoryWriteLow(uint32 addr, uint8 data);
//	void  MemoryWrite(uint32 addr, uint8 data);

protected:
	//  uint8 regs[1];

private:
};
/////////////////////////////////////////////////////////////////////

