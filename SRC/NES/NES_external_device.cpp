

#include <windows.h>
#include "NES_external_device.h"
#include "NES_screen_mgr.h"
#include "nes.h"


extern int Mainwindowsbhs;
extern HWND Mainwindowsbh;
extern HWND main_window_handle;

// Arkanoid Paddle
NES_EX_Controller_Arkanoid_Paddle::NES_EX_Controller_Arkanoid_Paddle(NES* pNES)
 : NES_EX_Controller_base(pNES){
	Reset();
}

void NES_EX_Controller_Arkanoid_Paddle::Reset(){
	arkanoid_byte = 0;
	arkanoid_bits = 0;
}

uint8 NES_EX_Controller_Arkanoid_Paddle::ReadReg4016(){
	return arkanoid_byte;
}

uint8 NES_EX_Controller_Arkanoid_Paddle::ReadReg4017(){
	uint8 retval = (arkanoid_bits & 0x01) << 1;
	arkanoid_bits >>= 1;
	return retval;
}

void NES_EX_Controller_Arkanoid_Paddle::WriteReg4016(uint8 data){
	arkanoid_byte = 0x00;
	arkanoid_bits = 0x00;

	if(GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		arkanoid_byte = 0x02;
	try
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(main_window_handle, &pt);
#if 0
		if(NESTER_settings.nes.graphics.osd.double_size)
		{
			pt.x >>= 1;
		}
#else
		RECT rect;
		float xp,yp;
		GetClientRect(main_window_handle, &rect);
		if(Mainwindowsbh){
			rect.bottom-=Mainwindowsbhs;
		}
		yp=(float)rect.bottom / (float)parent_NES->scr_mgr->get_viewable_height();
		xp=(float)rect.right / (float)parent_NES->scr_mgr->get_viewable_width();
		pt.y = (int)(pt.y/yp);
		pt.x = (int)(pt.x/xp);
#endif
		if(pt.x > 176) pt.x = 176;
		if(pt.x < 32) pt.x = 32;
		uint8 px = 0xFF - (uint8)(0x52+172*(pt.x-32)/144);
		arkanoid_bits |= (px & 0x01) << 7;
		arkanoid_bits |= (px & 0x02) << 5;
		arkanoid_bits |= (px & 0x04) << 3;
		arkanoid_bits |= (px & 0x08) << 1;
		arkanoid_bits |= (px & 0x10) >> 1;
		arkanoid_bits |= (px & 0x20) >> 3;
		arkanoid_bits |= (px & 0x40) >> 5;
		arkanoid_bits |= (px & 0x80) >> 7;
	} catch(...) { }
}

void NES_EX_Controller_Arkanoid_Paddle::WriteReg4016_strobe(){
	if(GetAsyncKeyState(VK_RBUTTON) & 0x8000)
		parent_NES->pad1_bits |= 0x04;
}


//Crazy Climber
NES_EX_Controller_Crazy_Climber::NES_EX_Controller_Crazy_Climber(NES* pNES)
 : NES_EX_Controller_base(pNES){
}

void NES_EX_Controller_Crazy_Climber::WriteReg4016_strobe(){
	parent_NES->pad1_bits = parent_NES->pad1_bits & 0x0F;
	parent_NES->pad2_bits = parent_NES->pad2_bits & 0x0F;
	if(GetAsyncKeyState('D') & 0x8000) parent_NES->pad1_bits |= 0x10;
	if(GetAsyncKeyState('A') & 0x8000) parent_NES->pad1_bits |= 0x20;
	if(GetAsyncKeyState('W') & 0x8000) parent_NES->pad1_bits |= 0x40;
	if(GetAsyncKeyState('S') & 0x8000) parent_NES->pad1_bits |= 0x80;
	if(GetAsyncKeyState('J') & 0x8000) parent_NES->pad2_bits |= 0x10;
	if(GetAsyncKeyState('G') & 0x8000) parent_NES->pad2_bits |= 0x20;
	if(GetAsyncKeyState('Y') & 0x8000) parent_NES->pad2_bits |= 0x40;
	if(GetAsyncKeyState('H') & 0x8000) parent_NES->pad2_bits |= 0x80;
}


//Doremikko Keyboard
NES_EX_Controller_Doremikko_Keyboard::NES_EX_Controller_Doremikko_Keyboard(NES* pNES)
 : NES_EX_Controller_base(pNES){
	Reset();
}

void NES_EX_Controller_Doremikko_Keyboard::Reset(){
	doremi_out = 0;
	doremi_scan= 0;
	doremi_reg = 0;
}

uint8 NES_EX_Controller_Doremikko_Keyboard::ReadReg4017(){
	uint8 retval = 0x00;

	switch(doremi_scan)
	{
	case 1:
		{
			if(GetAsyncKeyState('Z') & 0x8000) retval |= 0x10;
			if(GetAsyncKeyState('S') & 0x8000) retval |= 0x20;
		}
		break;
		
	case 2:
		{
			if(GetAsyncKeyState('X') & 0x8000) retval |= 0x01;
			if(GetAsyncKeyState('D') & 0x8000) retval |= 0x02;
			if(GetAsyncKeyState('C') & 0x8000) retval |= 0x04;
			if(GetAsyncKeyState('V') & 0x8000) retval |= 0x08;
			if(GetAsyncKeyState('G') & 0x8000) retval |= 0x10;
			if(GetAsyncKeyState('B') & 0x8000) retval |= 0x20;
		}
		break;
		
	case 3:
		{
			if(GetAsyncKeyState('H') & 0x8000) retval |= 0x01;
			if(GetAsyncKeyState('N') & 0x8000) retval |= 0x02;
			if(GetAsyncKeyState('J') & 0x8000) retval |= 0x04;
			if(GetAsyncKeyState('M') & 0x8000) retval |= 0x08;
			if(GetAsyncKeyState(','+0x90) & 0x8000) retval |= 0x10;
			if(GetAsyncKeyState('L') & 0x8000) retval |= 0x20;
		}
		break;
		
	case 4:
		{
			if(GetAsyncKeyState('.'+0x90) & 0x8000) retval |= 0x01;
			if(GetAsyncKeyState(';'+0x80) & 0x8000) retval |= 0x02;
			if(GetAsyncKeyState('/'+0x90) & 0x8000) retval |= 0x04;
			if(GetAsyncKeyState('Q') & 0x8000) retval |= 0x08;
			if(GetAsyncKeyState('2') & 0x8000) retval |= 0x10;
			if(GetAsyncKeyState('W') & 0x8000) retval |= 0x20;
		}
		break;
		
	case 5:
		{
			if(GetAsyncKeyState('3') & 0x8000) retval |= 0x01;
			if(GetAsyncKeyState('E') & 0x8000) retval |= 0x02;
			if(GetAsyncKeyState('4') & 0x8000) retval |= 0x04;
			if(GetAsyncKeyState('R') & 0x8000) retval |= 0x08;
			if(GetAsyncKeyState('T') & 0x8000) retval |= 0x10;
			if(GetAsyncKeyState('6') & 0x8000) retval |= 0x20;
		}
		break;
		
	case 6:
		{
			if(GetAsyncKeyState('Y') & 0x8000) retval |= 0x01;
			if(GetAsyncKeyState('7') & 0x8000) retval |= 0x02;
			if(GetAsyncKeyState('U') & 0x8000) retval |= 0x04;
			if(GetAsyncKeyState('I') & 0x8000) retval |= 0x08;
			if(GetAsyncKeyState('9') & 0x8000) retval |= 0x10;
			if(GetAsyncKeyState('O') & 0x8000) retval |= 0x20;
		}
		break;
		
	case 7:
		{
			if(GetAsyncKeyState('0') & 0x8000) retval |= 0x01;
			if(GetAsyncKeyState('P') & 0x8000) retval |= 0x02;
			if(GetAsyncKeyState('-'+0x90) & 0x8000) retval |= 0x04;
			if(GetAsyncKeyState('@'+0x80) & 0x8000) retval |= 0x08;
		}
		break;
	}
	if(doremi_out)
	{
		retval = (retval & 0xF0) >> 3;
	}
	else
	{
		retval = (retval & 0x0F) << 1;
	}
	//retval = (0xFF - retval) & 0x1e;
	doremi_out = 1 - doremi_out;
	return retval;
}

void NES_EX_Controller_Doremikko_Keyboard::WriteReg4016(uint8 data){
	if((data & 0x02) && !(doremi_reg & 0x02)){
		doremi_scan = 0;
		doremi_out = 0;
	}
	if((data & 0x01) && !(doremi_reg & 0x01)){
		doremi_scan++;
		doremi_out = 0;
	}
	doremi_reg = data;
}



//Exciting_Boxing
NES_EX_Controller_Exciting_Boxing::NES_EX_Controller_Exciting_Boxing(NES* pNES)
 : NES_EX_Controller_base(pNES){
	Reset();
}

void NES_EX_Controller_Exciting_Boxing::Reset(){
	excitingboxing_byte = 0;
}

uint8 NES_EX_Controller_Exciting_Boxing::ReadReg4017(){
	return excitingboxing_byte;
}

void NES_EX_Controller_Exciting_Boxing::WriteReg4016(uint8 data){
	excitingboxing_byte = 0x00;

	if(data & 0x02){
		if(!(GetAsyncKeyState('D') & 0x8000)) excitingboxing_byte |= 0x10;
		if(!(GetAsyncKeyState('E') & 0x8000)) excitingboxing_byte |= 0x08;
		if(!(GetAsyncKeyState('S') & 0x8000)) excitingboxing_byte |= 0x04;
		if(!(GetAsyncKeyState('W') & 0x8000)) excitingboxing_byte |= 0x02;
	}
	else{
		if(!(GetAsyncKeyState('R') & 0x8000)) excitingboxing_byte |= 0x10;
		if(!(GetAsyncKeyState('F') & 0x8000)) excitingboxing_byte |= 0x08;
		if(!(GetAsyncKeyState('A') & 0x8000)) excitingboxing_byte |= 0x04;
		if(!(GetAsyncKeyState('Q') & 0x8000)) excitingboxing_byte |= 0x02;
	}
}


//Family Basic
NES_EX_Controller_Family_Basic::NES_EX_Controller_Family_Basic(NES* pNES)
 : NES_EX_Controller_base(pNES){
	ftape = NULL;
	mstream = NULL;
	Reset();
}

NES_EX_Controller_Family_Basic::~NES_EX_Controller_Family_Basic(){
	StopTape();
	if(mstream){
		delete mstream;
		mstream = NULL;
	}
}

void NES_EX_Controller_Family_Basic::Reset(){
	kb_out = 0;
	kb_scan = 0;
	kb_graph = 0;
	tape_data = 0;
	tape_bit = 0;
	tape_in = 0;
	tape_out = 0;
	tape_wait = 0;
	tape_status = 0;
	StopTape();
	auto_input_counter = 0;
	auto_input_kanamode = 0;
	auto_input_kana_tmp = 0;
	if(mstream){
		delete mstream;
		mstream = NULL;
	}
}

uint8 NES_EX_Controller_Family_Basic::ReadReg4016(){
	if(tape_status == 1 && tape_in){
		return 0x02;
	}
	return 0;
}

uint8 NES_EX_Controller_Family_Basic::ReadReg4017(){
	uint8 retval;

	if(mstream){
		return Autoinput();
	}
	retval = 0xFF;
	
	if(GetAsyncKeyState(VK_NEXT) & 0x8000) kb_graph = 1;
	if(GetAsyncKeyState(VK_PRIOR) & 0x8000) kb_graph = 0;
	
	if(kb_out)
	{
		switch(kb_scan)
		{
		case 1:
			{
				if(GetAsyncKeyState(VK_F8) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState(VK_RETURN) & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('[' + 0x80) & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState(']' + 0x80) & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 2:
			{
				if(GetAsyncKeyState(VK_F7) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('@' + 0x80) & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState(':' + 0x80) & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState(';' + 0x80) & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 3:
			{
				if(GetAsyncKeyState(VK_F6) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('O') & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('L') & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('K') & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 4:
			{
				if(GetAsyncKeyState(VK_F5) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('I') & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('U') & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('J') & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 5:
			{
				if(GetAsyncKeyState(VK_F4) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('Y') & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('G') & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('H') & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 6:
			{
				if(GetAsyncKeyState(VK_F3) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('T') & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('R') & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('D') & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 7:
			{
				if(GetAsyncKeyState(VK_F2) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('W') & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('S') & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('A') & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 8:
			{
				if(GetAsyncKeyState(VK_F1) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState(VK_ESCAPE) & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('Q') & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState(VK_CONTROL)) retval &= 0xEF;
			}
			break;
			
		case 9:
			{
				if(GetAsyncKeyState(VK_HOME) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState(VK_UP) & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState(VK_RIGHT) & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState(VK_LEFT) & 0x8000) retval &= 0xEF;
			}
			break;
		}
	}
	else
	{
		switch(kb_scan)
		{
		case 1:
			{
				if(GetAsyncKeyState(VK_F12) & 0x8000) retval &= 0xFD;
				//if(GetAsyncKeyState(VK_SHIFT)) retval &= 0xFB;  //shift(right)
				if(GetAsyncKeyState('\\' + 0x80) & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState(VK_END) & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 2:
			{
				if(GetAsyncKeyState(226) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('/' + 0x90) & 0x8000 || GetAsyncKeyState(VK_DIVIDE) & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('-' + 0x90) & 0x8000 || GetAsyncKeyState(VK_SUBTRACT) & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('^' + 0x80) & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 3:
			{
				if(GetAsyncKeyState('.' + 0x90) & 0x8000 || GetAsyncKeyState(VK_DECIMAL) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState(',' + 0x90) & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('P') & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('0') & 0x8000 || GetAsyncKeyState(VK_NUMPAD0) & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 4:
			{
				if(GetAsyncKeyState('M') & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('N') & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('9') & 0x8000 || GetAsyncKeyState(VK_NUMPAD9) & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('8') & 0x8000 || GetAsyncKeyState(VK_NUMPAD8) & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 5:
			{
				if(GetAsyncKeyState('B') & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('V') & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('7') & 0x8000 || GetAsyncKeyState(VK_NUMPAD7) & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('6') & 0x8000 || GetAsyncKeyState(VK_NUMPAD6) & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 6:
			{
				if(GetAsyncKeyState('F') & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('C') & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('5') & 0x8000 || GetAsyncKeyState(VK_NUMPAD5) & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('4') & 0x8000 || GetAsyncKeyState(VK_NUMPAD4) & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 7:
			{
				if(GetAsyncKeyState('X') & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState('Z') & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState('E') & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('3') & 0x8000 || GetAsyncKeyState(VK_NUMPAD3) & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 8:
			{
				if(GetAsyncKeyState(VK_SHIFT)) retval &= 0xFD;
				//if(GetAsyncKeyState(VK_MENU)) retval &= 0xFB;
				if(kb_graph) retval &= 0xFB;
				if(GetAsyncKeyState('1') & 0x8000 || GetAsyncKeyState(VK_NUMPAD1) & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState('2') & 0x8000 || GetAsyncKeyState(VK_NUMPAD2) & 0x8000) retval &= 0xEF;
			}
			break;
			
		case 9:
			{
				if(GetAsyncKeyState(VK_DOWN) & 0x8000) retval &= 0xFD;
				if(GetAsyncKeyState(VK_SPACE) & 0x8000) retval &= 0xFB;
				if(GetAsyncKeyState(VK_DELETE) & 0x8000) retval &= 0xF7;
				if(GetAsyncKeyState(VK_INSERT) & 0x8000) retval &= 0xEF;
			}
			break;
		}
	}
	return retval;
}

// 自動入力
#if 1
uint8 NES_EX_Controller_Family_Basic::Autoinput(){
	uint8 retval;

	retval = 0xFF;
	
	if(kb_out)
	{
		switch(kb_scan)
		{
		case 1:
			{
				if(auto_input_data == 0x0d) retval &= 0xFB;
				if(auto_input_data == '[' || auto_input_data == 0x0db/*'ﾛ'*/ || auto_input_data == 0x0a2) retval &= 0xF7;
				if(auto_input_data == ']' || auto_input_data == 0x0a1/*'｡'*/ || auto_input_data == 0x0a3) retval &= 0xEF;
			}
			break;
			
		case 2:
			{
				if(auto_input_data == '@' || auto_input_data == '`' || auto_input_data == 0x0da/*'ﾚ'*/) retval &= 0xFB;
				if(auto_input_data == ':' || auto_input_data == '*' || auto_input_data == '-') retval &= 0xF7;
				if(auto_input_data == ';' || auto_input_data == '+' || auto_input_data == 0x0d3/*'ﾓ'*/) retval &= 0xEF;
			}
			break;
			
		case 3:
			{
				if(auto_input_data == 'O' || auto_input_data == 'o' || auto_input_data == 0x0cd/*'ﾍ'*/) retval &= 0xFB;
				if(auto_input_data == 'L' || auto_input_data == 'l' || auto_input_data == 0x0d2/*'ﾒ'*/) retval &= 0xF7;
				if(auto_input_data == 'K' || auto_input_data == 'k' || auto_input_data == 0x0d1/*'ﾑ'*/) retval &= 0xEF;
			}
			break;
			
		case 4:
			{
				if(auto_input_data == 'I' || auto_input_data == 'i' || auto_input_data == 0x0cc/*'ﾌ'*/) retval &= 0xFB;
				if(auto_input_data == 'U' || auto_input_data == 'u' || auto_input_data == 0x0cb/*'ﾋ'*/) retval &= 0xF7;
				if(auto_input_data == 'J' || auto_input_data == 'j' || auto_input_data == 0x0d0/*'ﾐ'*/) retval &= 0xEF;
			}
			break;
			
		case 5:
			{
				if(auto_input_data == 'Y' || auto_input_data == 'y' || auto_input_data == 0x0ca/*'ﾊ'*/) retval &= 0xFB;
				if(auto_input_data == 'G' || auto_input_data == 'g' || auto_input_data == 0x0bf/*'ｿ'*/) retval &= 0xF7;
				if(auto_input_data == 'H' || auto_input_data == 'h' || auto_input_data == 0x0cf/*'ﾏ'*/) retval &= 0xEF;
			}
			break;
			
		case 6:
			{
				if(auto_input_data == 'T' || auto_input_data == 't' || auto_input_data == 0x0ba/*'ｺ'*/) retval &= 0xFB;
				if(auto_input_data == 'R' || auto_input_data == 'r' || auto_input_data == 0x0b9/*'ｹ'*/) retval &= 0xF7;
				if(auto_input_data == 'D' || auto_input_data == 'd' || auto_input_data == 0x0bd/*'ｽ'*/) retval &= 0xEF;
			}
			break;
			
		case 7:
			{
				if(auto_input_data == 'W' || auto_input_data == 'w' || auto_input_data == 0x0b7/*'ｷ'*/) retval &= 0xFB;
				if(auto_input_data == 'S' || auto_input_data == 's' || auto_input_data == 0x0bc/*'ｼ'*/) retval &= 0xF7;
				if(auto_input_data == 'A' || auto_input_data == 'a' || auto_input_data == 0x0bb/*'ｻ'*/) retval &= 0xEF;
			}
			break;
			
		case 8:
			{
//				if(auto_input_data == 'I' || auto_input_data == 'i') retval &= 0xFB;
				if(auto_input_data == 'Q' || auto_input_data == 'q' || auto_input_data == 0x0b6/*'ｶ'*/) retval &= 0xF7;
//				if(auto_input_data == 'I' || auto_input_data == 'i') retval &= 0xEF;
			}
			break;
			
		case 9:
			{
			}
			break;
		}
	}
	else
	{
		switch(kb_scan)
		{
		case 1:
			{
				// katakana mode
				if(auto_input_data == 0x1000) retval &= 0xFD;

				if(auto_input_data == '\\' /*|| auto_input_data == '_'*/ || auto_input_data == '|' || auto_input_data == 0x0d9/*'ﾙ'*/) retval &= 0xF7;

			}
			break;
			
		case 2:
			{
				if(auto_input_data == '_' || auto_input_data == 0x0dd /*'ﾝ'*/) retval &= 0xFD;
				if(auto_input_data == '/' || auto_input_data == '?' || auto_input_data == 0x0a6/*'ｦ'*/) retval &= 0xFB;
				if(auto_input_data == '-' || auto_input_data == '=' || auto_input_data == 0x0d7/*'ﾗ'*/) retval &= 0xF7;
				if(auto_input_data == '^' || auto_input_data == '~' || auto_input_data == 0x0d8/*'ﾘ'*/) retval &= 0xEF;
			}
			break;
			
		case 3:
			{
				if(auto_input_data == '.' || auto_input_data == '>' || auto_input_data == 0x0dc/*'ﾜ'*/) retval &= 0xFD;
				if(auto_input_data == ',' || auto_input_data == '<' || auto_input_data == 0x0d6/*'ﾖ'*/) retval &= 0xFB;
				if(auto_input_data == 'P' || auto_input_data == 'p' || auto_input_data == 0x0ce/*'ﾎ'*/) retval &= 0xF7;
				if(auto_input_data == '0' || auto_input_data == 0x0c9/*'ﾉ'*/) retval &= 0xEF;
			}
			break;
			
		case 4:
			{
				if(auto_input_data == 'M' || auto_input_data == 'm' || auto_input_data == 0x0d5/*'ﾕ'*/) retval &= 0xFD;
				if(auto_input_data == 'N' || auto_input_data == 'n' || auto_input_data == 0x0d4/*'ﾔ'*/) retval &= 0xFB;
				if(auto_input_data == '9' || auto_input_data == ')' || auto_input_data == 0x0c8/*'ﾈ'*/) retval &= 0xF7;
				if(auto_input_data == '8' || auto_input_data == '(' || auto_input_data == 0x0c7/*'ﾇ'*/) retval &= 0xEF;
			}
			break;
			
		case 5:
			{
				if(auto_input_data == 'B' || auto_input_data == 'b' || auto_input_data == 0x0c4/*'ﾄ'*/) retval &= 0xFD;
				if(auto_input_data == 'V' || auto_input_data == 'v' || auto_input_data == 0x0c3/*'ﾃ'*/) retval &= 0xFB;
				if(auto_input_data == '7' || auto_input_data == '\'' || auto_input_data == 0x0c6/*'ﾆ'*/) retval &= 0xF7;
				if(auto_input_data == '6' || auto_input_data == '&' || auto_input_data == 0x0c5/*'ﾅ'*/) retval &= 0xEF;
			}
			break;
			
		case 6:
			{
				if(auto_input_data == 'F' || auto_input_data == 'f' || auto_input_data == 0x0be/*'ｾ'*/) retval &= 0xFD;
				if(auto_input_data == 'C' || auto_input_data == 'c' || auto_input_data == 0x0c2/*'ﾂ'*/) retval &= 0xFB;
				if(auto_input_data == '5' || auto_input_data == '%' || auto_input_data == 0x0b5/*'ｵ'*/) retval &= 0xF7;
				if(auto_input_data == '4' || auto_input_data == '$' || auto_input_data == 0x0b4/*'ｴ'*/) retval &= 0xEF;
			}
			break;
			
		case 7:
			{
				if(auto_input_data == 'X' || auto_input_data == 'x' || auto_input_data == 0x0c1/*'ﾁ'*/) retval &= 0xFD;
				if(auto_input_data == 'Z' || auto_input_data == 'z' || auto_input_data == 0x0c0/*'ﾀ'*/) retval &= 0xFB;
				if(auto_input_data == 'E' || auto_input_data == 'e' || auto_input_data == 0x0b8/*'ｸ'*/) retval &= 0xF7;
				if(auto_input_data == '3' || auto_input_data == '#' || auto_input_data == 0x0b3/*'ｳ'*/) retval &= 0xEF;
			}
			break;
			
		case 8:
			{
				switch(auto_input_data){
					case '!':
					case '"':
					case '#':
					case '$':
					case '%':
					case '&':
					case '\'':
					case '(':
					case ')':
					case '=':
					case '~':
					case '|':
					case '{':
					case '+':
					case '*':
					case '}':
					case '<':
					case '>':
					case '?':
					case '_':
						retval &= 0xFD;
						break;
				}
				if(auto_input_shift){
					retval &= 0xFD;
				}

				if(auto_input_data == '1' || auto_input_data == '!' || auto_input_data == 0x0b1/*'ｱ'*/) retval &= 0xF7;
				if(auto_input_data == '2' || auto_input_data == '"' || auto_input_data == 0x0b2/*'ｲ'*/) retval &= 0xEF;

				if(kb_graph) retval &= 0xFB;
			}
			break;
			
		case 9:
			{
				if(auto_input_data == ' ') retval &= 0xFB;

				// 次のデータ読み込み
				auto_input_counter++;
				if(auto_input_state == 0){	// 押してる状態
					if(auto_input_counter == 8){
						auto_input_counter = 0;
						auto_input_state = 1;
						auto_input_data = 0;
					}
				}
				else{						// 押してない状態
					if(auto_input_counter == 4){
						auto_input_counter = 0;
						auto_input_state = 0;
						if(auto_input_kana_tmp){
							auto_input_data = auto_input_kana_tmp;
							auto_input_kana_tmp = 0;
						}
						else{
							if((auto_input_data = mstream->get_ch())==-1){
								delete mstream;
								mstream = NULL;
								kb_graph = 0;
								return retval;
							}

							// 全角スペース
							if(auto_input_data == 0x81 && mstream->get_data() == 0x40){
								mstream->get_ch();
								auto_input_data = 0x20;
							}

							// カタカナ？
							if(auto_input_data >= 0xa1/*0xb0*/ && auto_input_data <= 0xdf){
								unsigned char nd = mstream->get_data();

								if(auto_input_data == 0x0a2 || auto_input_data == 0x0a3){
									auto_input_data |= 0x20000;
								}
								else if(auto_input_data >= 0xa7 && auto_input_data <= 0xaf){	// 'ｧ'-
									if(auto_input_data <= 0xab){
										auto_input_data += 0x0a;
									}
									else if(auto_input_data <= 0xae){
										auto_input_data += 0x0028;
									}
									else {	// ｯ
//										auto_input_data += 0x0013;
										auto_input_data = 0x0c2;
									}
									auto_input_data |= 0x20000;
								}
								if(nd == 0xde){	// 'ﾞ'
									unsigned char ttt = mstream->get_ch();
									auto_input_data |= 0x90000;
								}
								else if(nd == 0xdf){ // 'ﾟ'
									unsigned char ttt = mstream->get_ch();
									auto_input_data |= 0xa0000;
								}
								else if(auto_input_daku){	// 濁音->非濁音ｶﾅ
									mstream->move_pos(-1);
									auto_input_data = 0;
									auto_input_daku = 0;
								}
								if(auto_input_kanamode==0){
									// カタカナモードに変更
									auto_input_kana_tmp = auto_input_data;
									auto_input_data = 0x1000;
									auto_input_kanamode = 1;
								}
							}
							else{
								if(auto_input_kanamode){
									// 非カタカナモードに変更
									auto_input_kana_tmp = auto_input_data;
									auto_input_data = 0x1000;
									auto_input_kanamode = 0;
								}
							}
						}
//						LOG("auto_input_data  " << HEX(auto_input_data,4) << endl);
						if(!(auto_input_data & 0x30000)){
							auto_input_daku = 0;
						}
						if(auto_input_data & 0x10000){
							if(auto_input_data & 0x80000)
								auto_input_daku = 1;
							else 
								auto_input_daku = 0;
							auto_input_data &= 0xffff;
							kb_graph = 1;
						}
						else {
							kb_graph = 0;
						}
						if(auto_input_data & 0x20000){
							if(auto_input_data & 0x80000)
								auto_input_daku = 1;
							else 
								auto_input_daku = 0;
							auto_input_data &= 0xffff;
							auto_input_shift = 1;
						}
						else {
							auto_input_shift = 0;
						}
					}
				}
			}
			break;
		}
	}
//	LOG("kb_out  " << HEX(kb_out,2) << ", kb_scan " << HEX(kb_scan,2) << endl);
	return retval;
}
#endif

void NES_EX_Controller_Family_Basic::WriteReg4016(uint8 data){
	if (data == 0x05){
		kb_out = 0;
		kb_scan = 0;
	}
	else if (data  == 0x04){
		kb_scan++;
		if (kb_scan > 9) kb_scan = 0;
		kb_out = 1 - kb_out;
	}
	else if (data == 0x06){
		kb_out = 1 - kb_out;
	}
	if (tape_status == 2){
		tape_out = data & 0x02;
	}
}

void NES_EX_Controller_Family_Basic::RotateTape()
{
	if(tape_status == 1)
	{
		if(tape_bit == 0x80)
		{
			tape_bit = 0x01;
			int tmp_data;
			if((tmp_data = fgetc(ftape)) == EOF)
			{
				tape_status = 0;
				tape_in = 0;
			}
			else
			{
				tape_data = tmp_data;
				tape_in = tape_data & tape_bit;
			}
		}
		else
		{
			tape_bit <<= 1;
			tape_in = tape_data & tape_bit;
		}
	}
	else if(tape_status == 2)
	{
		if(tape_out)
		{
			tape_data |= tape_bit;
		}
		else
		{
			tape_data &= 0xFF - tape_bit;
		}
		if(tape_bit == 0x80)
		{
			fputc(tape_data, ftape);
			tape_bit = 0x01;
		}
		else
		{
			tape_bit <<= 1;
		}
	}
}

void NES_EX_Controller_Family_Basic::StopTape()
{
	if(ftape){
		fclose(ftape);
		ftape = NULL;
	}
	tape_status = 0;
}

void NES_EX_Controller_Family_Basic::StartPlayTape(const char* fn)
{
	StopTape();
	ftape = fopen(fn, "rb");
	tape_status = 1;
	tape_wait = 0;
	tape_bit = 0x01;
	tape_data = fgetc(ftape);
	tape_in = tape_data & tape_bit;
}

void NES_EX_Controller_Family_Basic::StartRecTape(const char* fn)
{
	StopTape();
	ftape = fopen(fn, "wb");
	tape_status = 2;
	tape_wait = 0;
	tape_bit = 0x01;
	tape_data = 0;
	tape_out = 0;
}

uint8 NES_EX_Controller_Family_Basic::GetTapeStatus()
{
	return tape_status;
}

int32 NES_EX_Controller_Family_Basic::GetTapeWait(){
	return tape_wait;
}

void NES_EX_Controller_Family_Basic::SetTapeWait(int32 wait){
	tape_wait = wait;
}

int NES_EX_Controller_Family_Basic::StartAutoFileInput(my_memread_stream *stream){
	if(mstream){
		delete mstream;
		mstream = NULL;
	}
	mstream = stream;
/*
	if((auto_input_data = mstream->get_ch())==-1){
		delete mstream;
		mstream = NULL;
		return 0;
	}
*/
	auto_input_data = 0x0100000;
	auto_input_counter = 0;
	auto_input_state = 0;
	auto_input_kanamode = 0;
	auto_input_kana_tmp = 0;
	auto_input_shift = 0;
	auto_input_daku = 0;
	return 1;
}


//Family_Trainer
NES_EX_Controller_Family_Trainer::NES_EX_Controller_Family_Trainer(NES* pNES)
 : NES_EX_Controller_base(pNES){
	Reset();
}

void NES_EX_Controller_Family_Trainer::Reset(){
	familytrainer_byte = 0;
}

uint8 NES_EX_Controller_Family_Trainer::ReadReg4017(){
	return familytrainer_byte;
}

void NES_EX_Controller_Family_Trainer::WriteReg4016(uint8 data){
	familytrainer_byte = 0x00;

	if(parent_NES->ex_controller_type == EX_FAMILY_TRAINER_A)
	{
		if(!(data & 0x04))
		{
			if(!(GetAsyncKeyState('R') & 0x8000)) familytrainer_byte |= 0x10;
			if(!(GetAsyncKeyState('E') & 0x8000)) familytrainer_byte |= 0x08;
			if(!(GetAsyncKeyState('W') & 0x8000)) familytrainer_byte |= 0x04;
			if(!(GetAsyncKeyState('Q') & 0x8000)) familytrainer_byte |= 0x02;
		}
		if(!(data & 0x02))
		{
			if(!(GetAsyncKeyState('F') & 0x8000)) familytrainer_byte |= 0x10;
			if(!(GetAsyncKeyState('D') & 0x8000)) familytrainer_byte |= 0x08;
			if(!(GetAsyncKeyState('S') & 0x8000)) familytrainer_byte |= 0x04;
			if(!(GetAsyncKeyState('A') & 0x8000)) familytrainer_byte |= 0x02;
		}
		if(!(data & 0x01))
		{
			if(!(GetAsyncKeyState('V') & 0x8000)) familytrainer_byte |= 0x10;
			if(!(GetAsyncKeyState('C') & 0x8000)) familytrainer_byte |= 0x08;
			if(!(GetAsyncKeyState('X') & 0x8000)) familytrainer_byte |= 0x04;
			if(!(GetAsyncKeyState('Z') & 0x8000)) familytrainer_byte |= 0x02;
		}
	}
	if(parent_NES->ex_controller_type == EX_FAMILY_TRAINER_B)
	{
		if(!(data & 0x04))
		{
			if(!(GetAsyncKeyState('Q') & 0x8000)) familytrainer_byte |= 0x10;
			if(!(GetAsyncKeyState('W') & 0x8000)) familytrainer_byte |= 0x08;
			if(!(GetAsyncKeyState('E') & 0x8000)) familytrainer_byte |= 0x04;
			if(!(GetAsyncKeyState('R') & 0x8000)) familytrainer_byte |= 0x02;
		}
		if(!(data & 0x02))
		{
			if(!(GetAsyncKeyState('A') & 0x8000)) familytrainer_byte |= 0x10;
			if(!(GetAsyncKeyState('S') & 0x8000)) familytrainer_byte |= 0x08;
			if(!(GetAsyncKeyState('D') & 0x8000)) familytrainer_byte |= 0x04;
			if(!(GetAsyncKeyState('F') & 0x8000)) familytrainer_byte |= 0x02;
		}
		if(!(data & 0x01))
		{
			if(!(GetAsyncKeyState('Z') & 0x8000)) familytrainer_byte |= 0x10;
			if(!(GetAsyncKeyState('X') & 0x8000)) familytrainer_byte |= 0x08;
			if(!(GetAsyncKeyState('C') & 0x8000)) familytrainer_byte |= 0x04;
			if(!(GetAsyncKeyState('V') & 0x8000)) familytrainer_byte |= 0x02;
		}
	}
}

void NES_EX_Controller_Family_Trainer::WriteReg4016_strobe(){
	if(GetAsyncKeyState('T') & 0x8000) parent_NES->pad1_bits |= 0x08;
	if(GetAsyncKeyState('G') & 0x8000) parent_NES->pad1_bits |= 0x04;
}


//Hyper_Shot
NES_EX_Controller_Hyper_Shot::NES_EX_Controller_Hyper_Shot(NES* pNES)
 : NES_EX_Controller_base(pNES){
	Reset();
}

void NES_EX_Controller_Hyper_Shot::Reset(){
	hypershot_byte = 0;
}

uint8 NES_EX_Controller_Hyper_Shot::ReadReg4017(){
	return 	hypershot_byte;
}

void NES_EX_Controller_Hyper_Shot::WriteReg4016_strobe(){
	hypershot_byte = 0x00;
	if(parent_NES->pad1_bits & 0x01) hypershot_byte |= 0x02;
	if(parent_NES->pad1_bits & 0x02) hypershot_byte |= 0x04;
	if(parent_NES->pad2_bits & 0x01) hypershot_byte |= 0x08;
	if(parent_NES->pad2_bits & 0x02) hypershot_byte |= 0x10;
}


//Mahjong
NES_EX_Controller_Mahjong::NES_EX_Controller_Mahjong(NES* pNES)
 : NES_EX_Controller_base(pNES){

	Reset();
}

void NES_EX_Controller_Mahjong::Reset(){
	mahjong_bits = 0;
}

uint8 NES_EX_Controller_Mahjong::ReadReg4017(){
	uint8 retval = (mahjong_bits & 0x01) << 1;
	mahjong_bits >>= 1;
	return retval;
}

void NES_EX_Controller_Mahjong::WriteReg4016(uint8 data){
	mahjong_bits = 0x00;

	if((data & 0x06) == 0x02)
	{
		if(GetAsyncKeyState('J') & 0x8000) mahjong_bits |= 0x04;
		if(GetAsyncKeyState('H') & 0x8000) mahjong_bits |= 0x08;
		if(GetAsyncKeyState('G') & 0x8000) mahjong_bits |= 0x10;
		if(GetAsyncKeyState('F') & 0x8000) mahjong_bits |= 0x20;
		if(GetAsyncKeyState('D') & 0x8000) mahjong_bits |= 0x40;
		if(GetAsyncKeyState('S') & 0x8000) mahjong_bits |= 0x80;
	}
	else if((data & 0x06) == 0x04)
	{
		if(GetAsyncKeyState('A') & 0x8000) mahjong_bits |= 0x01;
		if(GetAsyncKeyState('U') & 0x8000) mahjong_bits |= 0x02;
		if(GetAsyncKeyState('Y') & 0x8000) mahjong_bits |= 0x04;
		if(GetAsyncKeyState('T') & 0x8000) mahjong_bits |= 0x08;
		if(GetAsyncKeyState('R') & 0x8000) mahjong_bits |= 0x10;
		if(GetAsyncKeyState('E') & 0x8000) mahjong_bits |= 0x20;
		if(GetAsyncKeyState('W') & 0x8000) mahjong_bits |= 0x40;
		if(GetAsyncKeyState('Q') & 0x8000) mahjong_bits |= 0x80;
	}
	else if((data & 0x06) == 0x06)
	{
		if(GetAsyncKeyState('M') & 0x8000) mahjong_bits |= 0x02;
		if(GetAsyncKeyState('N') & 0x8000) mahjong_bits |= 0x04;
		if(GetAsyncKeyState('B') & 0x8000) mahjong_bits |= 0x08;
		if(GetAsyncKeyState('V') & 0x8000) mahjong_bits |= 0x10;
		if(GetAsyncKeyState('C') & 0x8000) mahjong_bits |= 0x20;
		if(GetAsyncKeyState('Z') & 0x8000) mahjong_bits |= 0x40;
		if(GetAsyncKeyState('X') & 0x8000) mahjong_bits |= 0x80;
	}
}


//OekaKids_Tablet
NES_EX_Controller_OekaKids_Tablet::NES_EX_Controller_OekaKids_Tablet(NES* pNES)
 : NES_EX_Controller_base(pNES){
	Reset();
}

void NES_EX_Controller_OekaKids_Tablet::Reset(){
	POINT p;
	tablet_byte = 0;
	tablet_data = 0;
	tablet_pre_flag = 0;

	p.x = 16;
	p.y = 16;
	ClientToScreen(main_window_handle, &p);
	SetCursorPos( p.x, p.y );
}

uint8 NES_EX_Controller_OekaKids_Tablet::ReadReg4017(){
	return tablet_byte;
}

void NES_EX_Controller_OekaKids_Tablet::WriteReg4016(uint8 data){
	if(!(data & 0x01)){
		tablet_byte = 0;
		tablet_data = 0;
		if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) tablet_data |= 0x0001;
		try
		{
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(main_window_handle, &pt);
#if 0
			if(NESTER_settings.nes.graphics.osd.double_size)
			{
				pt.x >>= 1;
				pt.y >>= 1;
			}
			if(!NESTER_settings.nes.graphics.show_all_scanlines)
			{
				pt.y += 8;
			}
#else
			RECT rect;
			float xp,yp;
			GetClientRect(main_window_handle, &rect);
			if(Mainwindowsbh){
				rect.bottom-=Mainwindowsbhs;
			}
			yp=(float)rect.bottom / (float)parent_NES->scr_mgr->get_viewable_height();
			xp=(float)rect.right / (float)parent_NES->scr_mgr->get_viewable_width();
			pt.y = (int)(pt.y/yp);
			pt.x = (int)(pt.x/xp);
#endif
			if(pt.y > 48)
			{
				if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) tablet_data |= 0x0001;
				tablet_data |= 0x0002;
			}
			else
			{
				if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) tablet_data |= 0x0003;
			}
			pt.x = (int32)((240*pt.x)/256) +  8; if(pt.x < 0) pt.x = 0;
			pt.y = (int32)((256*pt.y)/240) - 12; if(pt.y < 0) pt.y = 0;
			tablet_data = tablet_data | (pt.x << 10) | (pt.y << 2);
			} catch(...) { }
	}
	else
	{
		if(!(tablet_pre_flag & 0x02) && (data & 0x02))
		{
			tablet_data <<= 1;
		}
		if(!(data & 0x02))
		{
			tablet_byte = 0x04;
		}
		else
		{
			if(tablet_data & 0x40000)
			{
				tablet_byte = 0x00;
			}
			else
			{
				tablet_byte = 0x08;
			}
		}
	}
	tablet_pre_flag = data;
}


//Optical Gun
NES_EX_Controller_Optical_Gun::NES_EX_Controller_Optical_Gun(NES* pNES)
 : NES_EX_Controller_base(pNES){
}


uint8 NES_EX_Controller_Optical_Gun::ReadReg4017(){
	uint8 retval;

	retval = 0x08;
	if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) retval |= 0x10;
	try
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(main_window_handle, &pt);
#if 0
		if(NESTER_settings.nes.graphics.osd.double_size)
		{
			pt.x >>= 1;
			pt.y >>= 1;
		}
		if(!NESTER_settings.nes.graphics.show_all_scanlines)
		{
			pt.y += 8;
		}
#else
		RECT rect;
		float xp,yp;
		GetClientRect(main_window_handle, &rect);
		if(Mainwindowsbh){
			rect.bottom-=Mainwindowsbhs;
		}
		yp=(float)rect.bottom / (float)parent_NES->scr_mgr->get_viewable_height();
		xp=(float)rect.right / (float)parent_NES->scr_mgr->get_viewable_width();
		pt.y = (int)(pt.y/yp);
		pt.x = (int)(pt.x/xp);
#endif
		if(0 < pt.x && pt.x < 255 && 0 < pt.y && pt.y < 239)
		{
			uint8 c = parent_NES->ppu->GetPointColor(pt.x, pt.y) & 0x3F;
			if(parent_NES->crc32() == 0xbc1dce96) // Chiller (#11)
			{
				if(parent_NES->NES_RGB_pal[c][0] > 0 && parent_NES->NES_RGB_pal[c][1] == 0 && parent_NES->NES_RGB_pal[c][2] > 0)
				{
					retval &= 0xF7;
				}
			}
			else
			{
				if(c == 32 || c == 48)
					//if(NES_RGB_pal[c][0] == 0xFF && NES_RGB_pal[c][1] == 0xFF && NES_RGB_pal[c][2] == 0xFF)
				{
					retval &= 0xF7;
				}
			}
		}
	} catch(...) { }
	return retval;
}



// Pokkun Moguraa
NES_EX_Controller_Pokkun_Moguraa::NES_EX_Controller_Pokkun_Moguraa(NES* pNES)
 : NES_EX_Controller_base(pNES){
	Reset();
}

void NES_EX_Controller_Pokkun_Moguraa::Reset(){
	pokkunmoguraa_byte = 0;
}

uint8 NES_EX_Controller_Pokkun_Moguraa::ReadReg4017(){
	return pokkunmoguraa_byte;
}

void NES_EX_Controller_Pokkun_Moguraa::WriteReg4016(uint8 data){
	pokkunmoguraa_byte = 0x00;

	if(!(data & 0x04))
	{
		if(!(GetAsyncKeyState('R') & 0x8000)) pokkunmoguraa_byte |= 0x10;
		if(!(GetAsyncKeyState('E') & 0x8000)) pokkunmoguraa_byte |= 0x08;
		if(!(GetAsyncKeyState('W') & 0x8000)) pokkunmoguraa_byte |= 0x04;
		if(!(GetAsyncKeyState('Q') & 0x8000)) pokkunmoguraa_byte |= 0x02;
	}
	if(!(data & 0x02))
	{
		if(!(GetAsyncKeyState('F') & 0x8000)) pokkunmoguraa_byte |= 0x10;
		if(!(GetAsyncKeyState('D') & 0x8000)) pokkunmoguraa_byte |= 0x08;
		if(!(GetAsyncKeyState('S') & 0x8000)) pokkunmoguraa_byte |= 0x04;
		if(!(GetAsyncKeyState('A') & 0x8000)) pokkunmoguraa_byte |= 0x02;
	}
	if(!(data & 0x01))
	{
		if(!(GetAsyncKeyState('V') & 0x8000)) pokkunmoguraa_byte |= 0x10;
		if(!(GetAsyncKeyState('C') & 0x8000)) pokkunmoguraa_byte |= 0x08;
		if(!(GetAsyncKeyState('X') & 0x8000)) pokkunmoguraa_byte |= 0x04;
		if(!(GetAsyncKeyState('Z') & 0x8000)) pokkunmoguraa_byte |= 0x02;
	}
}

void NES_EX_Controller_Pokkun_Moguraa::WriteReg4016_strobe(){
	if(GetAsyncKeyState('T') & 0x8000) parent_NES->pad1_bits |= 0x08;
	if(GetAsyncKeyState('G') & 0x8000) parent_NES->pad1_bits |= 0x04;
}


// Power Pad
NES_EX_Controller_Power_Pad::NES_EX_Controller_Power_Pad(NES* pNES)
 : NES_EX_Controller_base(pNES){
	Reset();
}

void NES_EX_Controller_Power_Pad::Reset(){
	powerpad_bits1 = 0;
	powerpad_bits2 = 0;
}

uint8 NES_EX_Controller_Power_Pad::ReadReg4017(){
	uint8 retval = 0x00;
	retval |= (powerpad_bits1 & 0x01) << 3;
	retval |= (powerpad_bits2 & 0x01) << 4;
	powerpad_bits1 >>= 1;
	powerpad_bits2 >>= 1;
	return retval;
}

void NES_EX_Controller_Power_Pad::WriteReg4016_strobe(){
	powerpad_bits1 = powerpad_bits2 = 0x00;

	if(parent_NES->ex_controller_type == EX_POWER_PAD_A)
	{
		if(GetAsyncKeyState('R') & 0x8000) powerpad_bits1 |= 0x02;
		if(GetAsyncKeyState('E') & 0x8000) powerpad_bits1 |= 0x01;
		if(GetAsyncKeyState('W') & 0x8000) powerpad_bits2 |= 0x02;
		if(GetAsyncKeyState('Q') & 0x8000) powerpad_bits2 |= 0x01;
		if(GetAsyncKeyState('F') & 0x8000) powerpad_bits1 |= 0x04;
		if(GetAsyncKeyState('D') & 0x8000) powerpad_bits1 |= 0x10;
		if(GetAsyncKeyState('S') & 0x8000) powerpad_bits1 |= 0x80;
		if(GetAsyncKeyState('A') & 0x8000) powerpad_bits2 |= 0x08;
		if(GetAsyncKeyState('V') & 0x8000) powerpad_bits1 |= 0x08;
		if(GetAsyncKeyState('C') & 0x8000) powerpad_bits1 |= 0x20;
		if(GetAsyncKeyState('X') & 0x8000) powerpad_bits1 |= 0x40;
		if(GetAsyncKeyState('Z') & 0x8000) powerpad_bits2 |= 0x04;
		if(GetAsyncKeyState('T') & 0x8000) parent_NES->pad1_bits |= 0x08;
		if(GetAsyncKeyState('G') & 0x8000) parent_NES->pad1_bits |= 0x04;
	}
	if(parent_NES->ex_controller_type == EX_POWER_PAD_B)
	{
		if(GetAsyncKeyState('Q') & 0x8000) powerpad_bits1 |= 0x02;
		if(GetAsyncKeyState('W') & 0x8000) powerpad_bits1 |= 0x01;
		if(GetAsyncKeyState('E') & 0x8000) powerpad_bits2 |= 0x02;
		if(GetAsyncKeyState('R') & 0x8000) powerpad_bits2 |= 0x01;
		if(GetAsyncKeyState('A') & 0x8000) powerpad_bits1 |= 0x04;
		if(GetAsyncKeyState('S') & 0x8000) powerpad_bits1 |= 0x10;
		if(GetAsyncKeyState('D') & 0x8000) powerpad_bits1 |= 0x80;
		if(GetAsyncKeyState('F') & 0x8000) powerpad_bits2 |= 0x08;
		if(GetAsyncKeyState('Z') & 0x8000) powerpad_bits1 |= 0x08;
		if(GetAsyncKeyState('X') & 0x8000) powerpad_bits1 |= 0x20;
		if(GetAsyncKeyState('C') & 0x8000) powerpad_bits1 |= 0x40;
		if(GetAsyncKeyState('V') & 0x8000) powerpad_bits2 |= 0x04;
		if(GetAsyncKeyState('T') & 0x8000) parent_NES->pad1_bits |= 0x08;
		if(GetAsyncKeyState('G') & 0x8000) parent_NES->pad1_bits |= 0x04;
	}
}



// Space Shadow Gun
NES_EX_Controller_Space_Shadow_Gun::NES_EX_Controller_Space_Shadow_Gun(NES* pNES)
 : NES_EX_Controller_base(pNES){
	Reset();
}

void NES_EX_Controller_Space_Shadow_Gun::Reset(){
	spaceshadow_bits = 0;
}

uint8 NES_EX_Controller_Space_Shadow_Gun::ReadReg4016(){
	uint8 retval = (spaceshadow_bits & 0x01) << 1;
	spaceshadow_bits >>= 1;
	return retval;
}

uint8 NES_EX_Controller_Space_Shadow_Gun::ReadReg4017(){
	uint8 retval;

	retval = 0x08;
	if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) retval |= 0x10;
	try
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(main_window_handle, &pt);
#if 0
		if(NESTER_settings.nes.graphics.osd.double_size)
		{
			pt.x >>= 1;
			pt.y >>= 1;
		}
		if(!NESTER_settings.nes.graphics.show_all_scanlines)
		{
			pt.y += 8;
		}
#else
		RECT rect;
		float xp,yp;
		GetClientRect(main_window_handle, &rect);
		if(Mainwindowsbh){
			rect.bottom-=Mainwindowsbhs;
		}
		yp=(float)rect.bottom / (float)parent_NES->scr_mgr->get_viewable_height();
		xp=(float)rect.right / (float)parent_NES->scr_mgr->get_viewable_width();
		pt.y = (int)(pt.y/yp);
		pt.x = (int)(pt.x/xp);
#endif
		
		if(0 < pt.x && pt.x < 255 && 0 < pt.y && pt.y < 239)
		{
			uint8 c = parent_NES->ppu->GetPointColor(pt.x, pt.y) & 0x3F;
			if(parent_NES->NES_RGB_pal[c][0] == 0xFF && parent_NES->NES_RGB_pal[c][1] == 0xFF && parent_NES->NES_RGB_pal[c][2] == 0xFF)
			{
				retval &= 0xF7;
			}
		}
	} catch(...) { }
	return retval;
}

void NES_EX_Controller_Space_Shadow_Gun::WriteReg4016_strobe(){
	spaceshadow_bits = 0x00;
	spaceshadow_bits = parent_NES->pad1_bits & 0xFC;
	spaceshadow_bits |= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 0x02 : 0x00;
}


// Turbo File
NES_EX_Controller_Turbo_File::NES_EX_Controller_Turbo_File(NES* pNES)
 : NES_EX_Controller_base(pNES){
	Reset();
}

void NES_EX_Controller_Turbo_File::Reset(){
	tf_byte = 0;
//	tf_data[13*0x2000];
	tf_pre_flag = 0;
	tf_bit = 1;
	tf_write = 0;
	tf_pointer = 0;
	tf_bank = 0;
}

uint8 NES_EX_Controller_Turbo_File::ReadReg4017(){
	return tf_byte;
}

void NES_EX_Controller_Turbo_File::WriteReg4016(uint8 data){
	tf_byte = 0x00;

	if(!(data & 0x02)){
		tf_pointer = 0x00;
		tf_bit = 0x01;
	}
	if(data & 0x04){
		tf_data[tf_bank*0x2000 + tf_pointer] &= 0xFF - tf_bit;
		tf_data[tf_bank*0x2000 + tf_pointer] |= tf_bit * (data & 0x01);
		tf_write = 1;
	}
	if((tf_pre_flag & 0x04) && !(data & 0x04)){
		if(tf_bit == 0x80){
			tf_pointer = (tf_pointer + 1) & 0x1FFF;
			tf_bit = 0x01;
		}
		else{
			tf_bit <<= 1;
		}
	}
	if(tf_data[tf_bank*0x2000 + tf_pointer] & tf_bit){
		tf_byte = 0x04;
	}
	tf_pre_flag = data;
}

void NES_EX_Controller_Turbo_File::Save_TurboFile(char* fn){
	FILE* fp = NULL;
	if(tf_write){
		if((fp = fopen(fn, "wb")) != NULL)
		{
			fwrite(tf_data, 13*0x2000, 1, fp);
			fclose(fp);
		}
		tf_write = 0;
	}
}

void NES_EX_Controller_Turbo_File::Load_TurboFile(char* fn){
	FILE* fp = NULL;
	if((fp = fopen(fn, "rb")) != NULL)
	{
		fread(tf_data, 13*0x2000, 1, fp);
		fclose(fp);
	}
	tf_write = 0;
}

void NES_EX_Controller_Turbo_File::SetTurboFile_Bank(uint32 nbank){
	tf_bank = nbank;
}


// VS Unisystem Zapper
NES_EX_Controller_VS_Zapper::NES_EX_Controller_VS_Zapper(NES* pNES)
 : NES_EX_Controller_base(pNES){

	Reset();
}

void NES_EX_Controller_VS_Zapper::Reset(){
	vszapper_strobe = 0;
	vszapper_count  = 0;
}

uint8 NES_EX_Controller_VS_Zapper::ReadReg4016(){
	uint8 retval = 0x00;

	if(vszapper_count == 4){
		retval = 0x01;
	}
	vszapper_count++;
	return retval;
}

void NES_EX_Controller_VS_Zapper::WriteReg4016(uint8 data){
	if((vszapper_strobe & 0x01) && !(data & 0x01)){
		vszapper_count = 0;
	}
	vszapper_strobe = data;
}

void NES_EX_Controller_VS_Zapper::WriteReg4016_strobe(){
}

