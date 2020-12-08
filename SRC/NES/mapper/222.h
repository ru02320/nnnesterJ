
/////////////////////////////////////////////////////////////////////
// Mapper 222
class NES_mapper222 : public NES_mapper
{

public:
	NES_mapper222(NES* parent) : NES_mapper(parent) {}
	~NES_mapper222() {}

	void  Reset();
	void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};
/////////////////////////////////////////////////////////////////////

