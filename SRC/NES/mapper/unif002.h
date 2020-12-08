
// 
class NES_UNIFmapper2 : public NES_mapper
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_UNIFmapper2(NES* parent) : NES_mapper(parent) {}
	~NES_UNIFmapper2() {}

	void  Reset();
	void  MemoryWrite(uint32 addr, uint8 data);
	void  MemoryWriteLow(uint32 addr, uint8 data);
	uint8  MemoryReadLow(uint32 addr);

protected:
	uint8 wram[0x1000];

private:
};
/////////////////////////////////////////////////////////////////////

