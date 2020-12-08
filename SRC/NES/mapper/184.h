
/////////////////////////////////////////////////////////////////////
// Mapper 184
class NES_mapper184 : public NES_mapper
{

public:
	NES_mapper184(NES* parent) : NES_mapper(parent) {}
	~NES_mapper184() {}

	void  Reset();
	void  MemoryWriteSaveRAM(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

