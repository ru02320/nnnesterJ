
#include "debug.h"
#include "resource.h"
#include <windows.h>
#include <stdio.h>

extern "C"{
	void ASMDEBUG_LOG(unsigned int n, unsigned int *d);
	void ASMDEBUG_LOGN(unsigned int n);
	extern DWORD CPU_PC;
	extern DWORD CPU_CYCLE;
	extern DWORD CPU_PENDING;
	extern DWORD CPU_REGA;
	extern DWORD CPU_REGX;
	extern DWORD CPU_REGY;
	extern DWORD CPU_REGS;
	extern DWORD CPU_CFLAG;
	extern DWORD CPU_ZFLAG;
	extern DWORD CPU_IFLAG;
	extern DWORD CPU_DFLAG;
	extern DWORD CPU_BFLAG;
	extern DWORD CPU_VFLAG;
	extern DWORD CPU_NFLAG;
	extern DWORD DEBUG_FLAG;		//debug

}


extern HWND main_window_handle;
extern HINSTANCE g_main_instance;

DWORD debugdf=0, breakp=0x10000, codedw, *pdword, one_step;
LRESULT CALLBACK TestDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
extern MyStrAtoh(char *, DWORD *);
HWND cpu_dwnd;


static char *nes6502_mnemonic_table[256] = {
	"BRK", "ORA - (Indirect,X)", NULL, NULL, NULL, "ORA - Zero Page", "ASL - Zero Page", NULL, 
	"PHP", "ORA - Immediate", "ASL - Accumulator", NULL, NULL, "ORA - Absolute", "ASL - Absolute", NULL, 
// 10H
	"BPL", "ORA - (Indirect),Y", NULL, NULL, NULL, "ORA - Zero Page,X", "ASL - Zero Page,X", NULL, 
	"CLC", "ORA - Absolute,Y", NULL, NULL, NULL, "ORA - Absolute,X", "ASL - Absolute,X", NULL, 
// 20H
	"JSR", "AND - (Indirect,X)", NULL, NULL, "BIT - Zero Page", "AND - Zero Page", "ROL - Zero Page", NULL, 
	"PLP", "AND - Immediate", "ROL - Accumulator", NULL, "BIT - Absolute", "AND - Absolute", "ROL - Absolute", NULL, 
// 30H
	"BMI", "AND - (Indirect),Y", NULL, NULL, NULL, "AND - Zero Page,X", "ROL - Zero Page,X", NULL, 
	"SEC", "AND - Absolute,Y", NULL, NULL, NULL, "AND - Absolute,X", "ROL - Absolute,X", NULL, 
// 40H
	"RTI", "EOR - (Indirect,X)", NULL, NULL, NULL, "EOR - Zero Page", "LSR - Zero Page ", NULL, 
	"PHA", "EOR - Immediate", "LSR - Accumulator", "Future Expansion", "JMP - Absolute", "EOR - Absolute", "LSR - Absolute", NULL, 
// 50H
	"BVC", "EOR - (Indirect),Y", NULL, NULL, NULL, "EOR - Zero Page,X ", "LSR - Zero Page,X", NULL, 
	"CLI", "EOR - Absolute,Y", NULL, NULL, NULL, "EOR - Absolute,X", "LSR - Absolute,X", NULL, 
// 60H
	"RTS", "ADC - (Indirect,X)", NULL, NULL, NULL, "ADC - Zero Page", "ROR - Zero Page", NULL, 
	"PLA", "ADC - Immediate", "ROR - Accumulator", NULL, "JMP - Indirect", "ADC - Absolute", "ROR - Absolute", NULL, 
// 70H
	"BVS", "ADC - (Indirect),Y", NULL, NULL, NULL, "ADC - Zero Page,X", "ROR - Zero Page,X", NULL, 
	"SEI", "ADC - Absolute,Y", NULL, NULL, NULL, "ADC - Absolute,X", "ROR - Absolute,X", NULL, 
// 80H
	NULL, "STA - (Indirect,X)", NULL, NULL, "STY - Zero Page", "STA - Zero Page", "STX - Zero Page", NULL, 
	"DEY", NULL, "TXA", NULL, "STY - Absolute", "STA - Absolute", "STX - Absolute", NULL, 
// 90H
	"BCC", "STA - (Indirect),Y", NULL, NULL, "STY - Zero Page,X", "STA - Zero Page,X", "STX - Zero Page,Y ", NULL, 
	"TYA", "STA - Absolute,Y", "TXS", NULL, NULL, "STA - Absolute,X", NULL, NULL, 
// A0H
	"LDY - Immediate", "LDA - (Indirect,X)", "LDX - Immediate", "Future Expansion", "LDY - Zero Page", "LDA - Zero Page", "LDX - Zero Page", NULL, 
	"TAY", "LDA - Immediate", "TAX", NULL, "LDY - Absolute", "LDA - Absolute", "LDX - Absolute", NULL, 
// B0H
	"BCS", "LDA - (Indirect),Y", NULL, NULL, "LDY - Zero Page,X", "LDA - Zero Page,X", "LDX - Zero Page,Y", NULL, 
	"CLV", "LDA - Absolute,Y", "TSX", NULL, "LDY - Absolute,X", "LDA - Absolute,X", "LDX - Absolute,Y", NULL, 
// C0H
	"Cpy - Immediate", "CMP - (Indirect,X)", NULL, NULL, "CPY - Zero Page", "CMP - Zero Page", "DEC - Zero Page", NULL, 
	"INY", "CMP - Immediate", "DEX", NULL, "CPY - Absolute", "CMP - Absolute", "DEC - Absolute ", NULL, 
// D0H
	"BNE", "CMP   (Indirect@,Y)", NULL, NULL, NULL, "CMP - Zero Page,X", "DEC - Zero Page,X", NULL, 
	"CLD", "CMP - Absolute,Y", NULL, NULL, NULL, "CMP - Absolute,X", "DEC - Absolute,X", NULL, 
// E0H
	"CPX - Immediate", "SBC - (Indirect,X)", NULL, "Future Expansion", "CPX - Zero Page", "SBC - Zero Page", "INC - Zero Page", NULL, 
	"INX", "SBC - Immediate", "NOP", NULL, "CPX - Absolute", "SBC - Absolute", "INC - Absolute", NULL, 
// F0H
	"BEQ", "SBC - (Indirect),Y", NULL, NULL, NULL, "SBC - Zero Page,X", "INC - Zero Page,X", NULL, 
	"SED", "SBC - Absolute,Y", NULL, NULL, NULL, "SBC - Absolute,X", "INC - Absolute,X", NULL
};



void ASMDEBUG_LOGN(unsigned int n){
	FILE *file=NULL;
	char str[1024];
	file = fopen("nes_disasm.txt", "a+b");
	if(file==NULL)
		return;
	sprintf(str," Call %04X\r\n", n	);
	fwrite(str, strlen(str), 1, file);
	fclose(file);
	return;
}


void ASMDEBUG_LOG(unsigned int n, unsigned int *d){
	{
		FILE *file=NULL;
		char str[1024];
		file = fopen("nes_disasm.txt", "a+b");
		if(file==NULL)
			return;
//		LOG("PC " << HEX(d[0],4) << " , CODE  " << HEX(n,8) << "" << nes6502_mnemonic_table[n&0xff] <<  endl);
//		LOG(" A" << HEX(CPU_REGA,4) <<  " , X  " << HEX(CPU_REGX,8) <<  endl);
//		LOG(" Y " << HEX(CPU_REGY,4) << " , S  " << HEX(CPU_REGS,8) <<  endl);
		sprintf(str," PC=%04X OP =%04X ops =%s A =%02X X =%02X Y=%02X S=%02X \r\n",
			CPU_PC, n, nes6502_mnemonic_table[n&0xff], CPU_REGA, CPU_REGX, CPU_REGY, CPU_REGS
		);
		fwrite(str, strlen(str), 1, file);
		fclose(file);
	}
	return;


	if((breakp>=0x10000 || breakp==d[0]) && !debugdf){
//	LOG("CODE  " << HEX(d,8) << ",  " << HEX(n,8) << endl);
		codedw = (DWORD)n;
		pdword = (DWORD *)d;
/*
		if(!cpu_dwnd){
			cpu_dwnd=CreateDialog(g_main_instance, MAKEINTRESOURCE(IDD_CPUDEBUGDIALOG), main_window_handle, (DLGPROC)TestDlg);
			ShowWindow(cpu_dwnd, SW_SHOW);
			UpdateWindow(cpu_dwnd);
		}
*/
		if(IDCANCEL==DialogBox(g_main_instance, MAKEINTRESOURCE(IDD_CPUDEBUGDIALOG), main_window_handle, (DLGPROC)TestDlg)){
			debugdf=1;
		}

  }
}



void SetDebugInfo(HWND hDlgWnd){
	char tstr[256];
			wsprintf(tstr, "%4X", CPU_PC/*pdword[0]*/);

			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT1), tstr);
			wsprintf(tstr, "%2X", CPU_REGA/*pdword[3]*/);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT2), tstr);
			wsprintf(tstr, "%2X", CPU_REGX/*pdword[4]*/);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT3), tstr);
			wsprintf(tstr, "%2X", CPU_REGY/*pdword[5]*/);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT4), tstr);
			wsprintf(tstr, "%2X", CPU_REGS/*pdword[6]*/);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT5), tstr);
			wsprintf(tstr, "%X", breakp);		//break point
			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT6), tstr);
			for(int i=0,s=0x80; i<8;++i,s>>=1){
				if(s&pdword[14])
					tstr[i]='1';
				else
					tstr[i]='0';
			}
			tstr[8]=0;
//			wsprintf(str, "F %2X" ,d[14]);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_EDIT7), tstr);

			wsprintf(tstr, "%8X" , codedw);
			SetWindowText(GetDlgItem(hDlgWnd, IDC_STATIC1), tstr);
}


//IDD_CPUDEBUGDIALOG
LRESULT CALLBACK TestDlg(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	char tstr[256];
	DWORD h;
    switch(msg) {
		case WM_COMMAND:
            switch(LOWORD(wp)){
                case IDOK:
					GetDlgItemText(hDlgWnd, IDC_EDIT6, (LPTSTR)tstr, 25);
					MyStrAtoh(tstr,&h);
					breakp=h;
					one_step=1;
					EndDialog(hDlgWnd, IDOK);
					return TRUE;
				case IDCANCEL:
					debugdf=1;
					one_step=1;
					EndDialog(hDlgWnd, IDCANCEL);
					return TRUE;
			}
            break;
        case WM_INITDIALOG:
			SetDebugInfo(hDlgWnd);
            return TRUE;
    }
    return FALSE;
}
