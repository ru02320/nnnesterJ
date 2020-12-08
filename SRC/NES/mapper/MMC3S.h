
/////////////////////////////////////////////////////////////////////


// mapper 205
class NES_mapper205 : public NES_mapperMMC3
{
//	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
//	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_mapper205(NES* parent) : NES_mapperMMC3(parent) {}
	~NES_mapper205() {}

	void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
	void  MemoryWriteLow(uint32 addr, uint8 data);
	void  Reset();

protected:
	void SetPPU_Banks(uint32 addr, uint8 data);
	void SetCPU_Banks(uint32 addr, uint8 data);

private:
	uint8 m205_reg;
};
/////////////////////////////////////////////////////////////////////


// mapper 254
class NES_mapper254 : public NES_mapperMMC3
{
//	friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
//	friend int extract_MPRD(SnssMapperBlock* block, NES* nes);

public:
	NES_mapper254(NES* parent) : NES_mapperMMC3(parent) {}
	~NES_mapper254() {}

	void  MemoryWrite(uint32 addr, uint8 data);
	void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
//	void  MemoryReadLow(uint32 addr);

	uint8 MemoryReadHigh(uint32 addr);
	boolean MemoryReadHighQuery(uint32 addr);

	void  Reset();

protected:
//	void SetPPU_Banks(uint32 addr, uint8 data);
//	void SetCPU_Banks(uint32 addr, uint8 data);

private:
	uint8 m254_regs[2];
	uint8 *wram;
};


/////////////////////////////////////////////////////////////////////
// mapper 250
class NES_mapper250 : public NES_mapperMMC3
{
public:
	NES_mapper250(NES* parent) : NES_mapperMMC3(parent) {}
	~NES_mapper250() {}

	void  MemoryWrite(uint32 addr, uint8 data);

protected:
private:
};


/////////////////////////////////////////////////////////////////////
// mapper 249
class NES_mapper249 : public NES_mapperMMC3
{
public:
	NES_mapper249(NES* parent) : NES_mapperMMC3(parent) {}
	~NES_mapper249() {}

	void  MemoryWriteLow(uint32 addr, uint8 data);

protected:
	void SetPPU_Banks(uint32 addr, uint8 data);
	void SetCPU_Banks(uint32 addr, uint8 data);

private:
	uint8 m249_reg;

};


/////////////////////////////////////////////////////////////////////
// mapper 245
class NES_mapper245 : public NES_mapperMMC3
{
public:
	NES_mapper245(NES* parent) : NES_mapperMMC3(parent) {}
	~NES_mapper245() {}

	void  Reset();

protected:
	void SetPPU_Banks(uint32 addr, uint8 data);
	void SetCPU_Banks(uint32 addr, uint8 data);

private:
	uint8 m245_reg;

};


/////////////////////////////////////////////////////////////////////
// mapper 115
class NES_mapper115 : public NES_mapperMMC3
{
public:
	NES_mapper115(NES* parent) : NES_mapperMMC3(parent) {}
	~NES_mapper115() {}

	void  Reset();

	void  MemoryWriteSaveRAM(uint32 addr, uint8 data);
	void  MemoryWriteLow(uint32 addr, uint8 data);

protected:
	void SetPPU_Banks(uint32 addr, uint8 data);
	void SetCPU_Banks(uint32 addr, uint8 data);

private:
	uint8 m115_regs[2];

};




/////////////////////////////////////////////////////////////////////
// mapper 217
class NES_mapper217 : public NES_mapperMMC3
{
public:
	NES_mapper217(NES* parent) : NES_mapperMMC3(parent) {}
	~NES_mapper217() {}

	void  Reset();

	void  MemoryWrite(uint32 addr, uint8 data);
	void  MemoryWriteLow(uint32 addr, uint8 data);

protected:
	void SetPPU_Banks(uint32 addr, uint8 data);
	void SetCPU_Banks(uint32 addr, uint8 data);

private:
	uint8 m217_regs[4];

};
