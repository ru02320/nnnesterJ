
/////////////////////////////////////////////////////////////////////
// Mapper 155
class NES_mapper155 : public NES_mapper1
{
	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_mapper155(NES* parent) : NES_mapper1(parent) {}
	~NES_mapper155() {}

	void Reset();

protected:
private:
};
/////////////////////////////////////////////////////////////////////

