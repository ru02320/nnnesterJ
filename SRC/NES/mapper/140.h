
/////////////////////////////////////////////////////////////////////
// Mapper 140
class NES_mapper140 : public NES_mapper
{

public:
	NES_mapper140(NES* parent) : NES_mapper(parent) {}
	~NES_mapper140() {}

	void  Reset();
	void  MemoryWriteSaveRAM(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

