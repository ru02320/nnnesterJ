
// 
class NES_UNIFmapper4 : public NES_mapper
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_UNIFmapper4(NES* parent) : NES_mapper(parent) {}
	~NES_UNIFmapper4() {}

	void  Reset();
	void  MemoryWrite(uint32 addr, uint8 data);
	void  MemoryWriteLow(uint32 addr, uint8 data);
	void  HSync(uint32 scanline);

protected:
	uint8 regs[5], dregs[8];
	int irq_counter, irq_latch;
	uint8 irq_enabled;

	void oMMC3PRG(uint8);
	void oMMC3CHR(uint8);
	void setchr1(uint8, uint8);
	void setchr2(uint8, uint8);

private:
};
/////////////////////////////////////////////////////////////////////

