#ifndef _NES_EX_CONTROLLER_H_
#define _NES_EX_CONTROLLER_H_

#include <windows.h>
#include <stdio.h>
#include "types.h"
#include "mmemfile.h"

//#include "nes.h"
class NES;  // class prototypes

class NES_EX_Controller_base
{
public:
	NES_EX_Controller_base(NES* pNES){ parent_NES = pNES; };
	virtual ~NES_EX_Controller_base(){};

	virtual void Reset(){};

	virtual uint8 ReadReg4016(){ return 0; };
	virtual uint8 ReadReg4017(){ return 0; };
	
	virtual void WriteReg4016_strobe(){};
	virtual void WriteReg4016(uint8 data){};

	// for Family basic
	virtual void StopTape(){};
	virtual void StartPlayTape(const char* fn){};
	virtual void StartRecTape(const char* fn){};
	virtual uint8 GetTapeStatus(){ return 0; };
	virtual void RotateTape(){};
	virtual int32 GetTapeWait(){ return 0; };
	virtual void SetTapeWait(int32 wait){};
	virtual int StartAutoFileInput(my_memread_stream *stream){ return 0; };

	// for Turbo File
	virtual void Save_TurboFile(char* fn){};
	virtual void Load_TurboFile(char* fn){};
	virtual void SetTurboFile_Bank(uint32){};

protected:
	NES* parent_NES;
	HWND m_hWnd;
private:

};




// Arkanoid Paddle
class NES_EX_Controller_Arkanoid_Paddle : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Arkanoid_Paddle(NES* pNES);
	~NES_EX_Controller_Arkanoid_Paddle(){};

	void Reset();
	uint8 ReadReg4016();
	uint8 ReadReg4017();
	
	void WriteReg4016_strobe();
	void WriteReg4016(uint8 data);
protected:
private:
	uint8 arkanoid_byte;
	uint32 arkanoid_bits;

};



// Crazy Climber
class NES_EX_Controller_Crazy_Climber : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Crazy_Climber(NES* pNES);
	~NES_EX_Controller_Crazy_Climber(){};

	void WriteReg4016_strobe();
protected:
private:

};


// Doremikko Keyboard
class NES_EX_Controller_Doremikko_Keyboard : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Doremikko_Keyboard(NES* pNES);
	~NES_EX_Controller_Doremikko_Keyboard(){};

	void Reset();
	uint8 ReadReg4017();
	
	void WriteReg4016(uint8 data);
protected:
private:
	uint8 doremi_out;
	uint8 doremi_scan;
	uint8 doremi_reg;

};


// Exciting_Boxing
class NES_EX_Controller_Exciting_Boxing : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Exciting_Boxing(NES* pNES);
	~NES_EX_Controller_Exciting_Boxing(){};

	void Reset();
	uint8 ReadReg4017();
	
	void WriteReg4016(uint8 data);
protected:
private:
	uint8 excitingboxing_byte;

};


// Family Basic
class NES_EX_Controller_Family_Basic : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Family_Basic(NES* pNES);
	~NES_EX_Controller_Family_Basic();

	void Reset();
	uint8 ReadReg4016();
	uint8 ReadReg4017();
	
	void WriteReg4016(uint8 data);

	void StopTape();
	void StartPlayTape(const char* fn);
	void StartRecTape(const char* fn);
	uint8 GetTapeStatus();
	int32 GetTapeWait();
	void SetTapeWait(int32 wait);
	void RotateTape();

	int StartAutoFileInput(my_memread_stream *stream);
	uint8 Autoinput();

protected:
private:
	uint8 kb_out;
	uint8 kb_scan;
	uint8 kb_graph;
	uint8 tape_data;
	uint8 tape_bit;
	uint8 tape_in;
	uint8 tape_out;
	int32 tape_wait;

	uint8 tape_status;
	FILE* ftape;

	my_memread_stream *mstream;
	int32 auto_input_data;
	int32 auto_input_state;
	int32 auto_input_kanamode;
	int32 auto_input_kana_tmp;
	int32 auto_input_counter;
	int32 auto_input_shift;
	int32 auto_input_daku;
};


// Family_Trainer
class NES_EX_Controller_Family_Trainer : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Family_Trainer(NES* pNES);
	~NES_EX_Controller_Family_Trainer(){};

	void Reset();
	uint8 ReadReg4017();
	
	void WriteReg4016_strobe();
	void WriteReg4016(uint8 data);
protected:
private:
	uint8 familytrainer_byte;

};


// Hyper_Shot
class NES_EX_Controller_Hyper_Shot : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Hyper_Shot(NES* pNES);
	~NES_EX_Controller_Hyper_Shot(){};

	void Reset();
	uint8 ReadReg4017();
	
	void WriteReg4016_strobe();
protected:
private:
	uint8 hypershot_byte;

};


// Mahjong
class NES_EX_Controller_Mahjong : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Mahjong(NES* pNES);
	~NES_EX_Controller_Mahjong(){};

	void Reset();
	uint8 ReadReg4017();
	
	void WriteReg4016(uint8 data);
protected:
private:
	uint32 mahjong_bits;

};


// OekaKids_Tablet
class NES_EX_Controller_OekaKids_Tablet : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_OekaKids_Tablet(NES* pNES);
	~NES_EX_Controller_OekaKids_Tablet(){};

	void Reset();
	uint8 ReadReg4017();
	
	void WriteReg4016(uint8 data);
protected:
private:
	uint8 tablet_byte;
	uint32 tablet_data;
	uint8 tablet_pre_flag;

};


// Optical Gun
class NES_EX_Controller_Optical_Gun : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Optical_Gun(NES* pNES);
	~NES_EX_Controller_Optical_Gun(){};

	uint8 ReadReg4017();
	
protected:
private:

};


// Pokkun Moguraa
class NES_EX_Controller_Pokkun_Moguraa : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Pokkun_Moguraa(NES* pNES);
	~NES_EX_Controller_Pokkun_Moguraa(){};

	void Reset();
	uint8 ReadReg4017();
	
	void WriteReg4016_strobe();
	void WriteReg4016(uint8 data);
protected:
private:
	uint8 pokkunmoguraa_byte;

};





// Power Pad
class NES_EX_Controller_Power_Pad : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Power_Pad(NES* pNES);
	~NES_EX_Controller_Power_Pad(){};

	void Reset();
	uint8 ReadReg4017();
	
	void WriteReg4016_strobe();
protected:
private:
	uint32 powerpad_bits1;
	uint32 powerpad_bits2;

};


// Space Shadow Gun
class NES_EX_Controller_Space_Shadow_Gun : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Space_Shadow_Gun(NES* pNES);
	~NES_EX_Controller_Space_Shadow_Gun(){};

	void Reset();
	uint8 ReadReg4016();
	uint8 ReadReg4017();
	
	void WriteReg4016_strobe();
protected:
private:
	uint32 spaceshadow_bits;

};



// Turbo File
class NES_EX_Controller_Turbo_File : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_Turbo_File(NES* pNES);
	~NES_EX_Controller_Turbo_File(){};


	void Reset();
	uint8 ReadReg4017();

	void WriteReg4016(uint8 data);
	void Save_TurboFile(char* fn);
	void Load_TurboFile(char* fn);
	void SetTurboFile_Bank(uint32);
protected:
private:
	uint8 tf_byte;
	uint8 tf_data[13*0x2000];
	uint8 tf_pre_flag;
	uint8 tf_bit;
	uint8 tf_write;
	uint32 tf_pointer;
	uint32 tf_bank;
};


// VS Unisystem Zapper
class NES_EX_Controller_VS_Zapper : public NES_EX_Controller_base
{
public:
	NES_EX_Controller_VS_Zapper(NES* pNES);
	~NES_EX_Controller_VS_Zapper(){};

	void Reset();
	uint8 ReadReg4016();
	
	void WriteReg4016_strobe();
	void WriteReg4016(uint8 data);
protected:
private:
	uint8 vszapper_strobe;
	uint8 vszapper_count;

};







#endif
