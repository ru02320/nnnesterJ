
/////////////////////////////////////////////////////////////////////
// Mapper 205
class NES_mapper205 : public NES_mapper
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_mapper205(NES* parent) : NES_mapper(parent) {}
	~NES_mapper205() {}

	void  Reset();
	//  uint8 MemoryReadLow(uint32);
	//  void  MemoryWriteLow(uint32 addr, uint8 data);
	void  MemoryWrite(uint32 addr, uint8 data);

protected:
	//  uint8 regs[1];

private:
};
/////////////////////////////////////////////////////////////////////

