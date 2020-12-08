
/////////////////////////////////////////////////////////////////////
// Mapper 212
class NES_mapper212 : public NES_mapper
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_mapper212(NES* parent) : NES_mapper(parent) {}
	~NES_mapper212() {}

	void  Reset();
	//  uint8 MemoryReadLow(uint32);
	//  void  MemoryWriteLow(uint32 addr, uint8 data);
	void  MemoryWrite(uint32 addr, uint8 data);

protected:
	//  uint8 regs[1];

private:
};
/////////////////////////////////////////////////////////////////////

