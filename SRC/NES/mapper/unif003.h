
// 
class NES_UNIFmapper3 : public NES_mapper
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_UNIFmapper3(NES* parent) : NES_mapper(parent) {}
	~NES_UNIFmapper3() {}

	void  Reset();
	void  MemoryWrite(uint32 addr, uint8 data);
//	void  MemoryWriteLow(uint32 addr, uint8 data);
	void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
protected:
	uint8 regs[1];

	void Setprg();
private:
};
/////////////////////////////////////////////////////////////////////

