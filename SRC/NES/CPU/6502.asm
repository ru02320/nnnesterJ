

;; NMI VECTOR   0xFFFA
;; RESET VECTOR 0xFFFC
;; IRQ,BRK VECTOR   0xFFFE


%define F_CARRY  01h
%define F_ZERO   02h
%define F_INT    04h
%define F_DEC    08h
%define F_BREAK  10h
%define F_RSRVD  20h
%define F_OVER   40h
%define F_SIGN   80h

%define FN_CARRY  0feh
%define FN_ZERO   0fdh
%define FN_INT    0fbh
%define FN_DEC    0f7h
%define FN_BREAK  0efh
%define FN_RSRVD  0dfh
%define FN_OVER   0bfh
%define FN_SIGN   07fh

;%define FN_SIGNZERO   07dh	;SIGN & ZERO
%include 'macro.mac'

EXTERNL ASMDEBUG_LOG

EXTERNL ASMDEBUG_LOGN

section .data
ALIGN 8
PUBLICL READ_FUNCA
	DD 0
PUBLICL WRITE_FUNCA
	DD 0
PUBLICL MEM_PAGEP
PUBLICL MAIN_MEMORY
	DD DEAD_PAGE, DEAD_PAGE, DEAD_PAGE, DEAD_PAGE, DEAD_PAGE, DEAD_PAGE, DEAD_PAGE, DEAD_PAGE
ALIGN 8
;PUBLICL MAIN_MEMORY
;	DD 0
;ALIGN 8
;PUBLICL MAIN_MEMORY
;	times 800h db 0
ALIGN 8
PUBLICL DEAD_PAGE
	times 2004h db 0
ALIGN 8
PUBLICL CPU_INSTTABLE
	DD INST_00, INST_01, INST_02, INST_03, INST_04, INST_05, INST_06, INST_07
	DD INST_08, INST_09, INST_0A, INST_0B, INST_0C, INST_0D, INST_0E, INST_0F
	DD INST_10, INST_11, INST_12, INST_13, INST_14, INST_15, INST_16, INST_17
	DD INST_18, INST_19, INST_1A, INST_1B, INST_1C, INST_1D, INST_1E, INST_1F
	DD INST_20, INST_21, INST_22, INST_23, INST_24, INST_25, INST_26, INST_27
	DD INST_28, INST_29, INST_2A, INST_2B, INST_2C, INST_2D, INST_2E, INST_2F
	DD INST_30, INST_31, INST_32, INST_33, INST_34, INST_35, INST_36, INST_37
	DD INST_38, INST_39, INST_3A, INST_3B, INST_3C, INST_3D, INST_3E, INST_3F
	DD INST_40, INST_41, INST_42, INST_43, INST_44, INST_45, INST_46, INST_47
	DD INST_48, INST_49, INST_4A, INST_4B, INST_4C, INST_4D, INST_4E, INST_4F
	DD INST_50, INST_51, INST_52, INST_53, INST_54, INST_55, INST_56, INST_57
	DD INST_58, INST_59, INST_5A, INST_5B, INST_5C, INST_5D, INST_5E, INST_5F
	DD INST_60, INST_61, INST_62, INST_63, INST_64, INST_65, INST_66, INST_67
	DD INST_68, INST_69, INST_6A, INST_6B, INST_6C, INST_6D, INST_6E, INST_6F
	DD INST_70, INST_71, INST_72, INST_73, INST_74, INST_75, INST_76, INST_77
	DD INST_78, INST_79, INST_7A, INST_7B, INST_7C, INST_7D, INST_7E, INST_7F
	DD INST_80, INST_81, INST_82, INST_83, INST_84, INST_85, INST_86, INST_87
	DD INST_88, INST_89, INST_8A, INST_8B, INST_8C, INST_8D, INST_8E, INST_8F
	DD INST_90, INST_91, INST_92, INST_93, INST_94, INST_95, INST_96, INST_97
	DD INST_98, INST_99, INST_9A, INST_9B, INST_9C, INST_9D, INST_9E, INST_9F
	DD INST_A0, INST_A1, INST_A2, INST_A3, INST_A4, INST_A5, INST_A6, INST_A7
	DD INST_A8, INST_A9, INST_AA, INST_AB, INST_AC, INST_AD, INST_AE, INST_AF
	DD INST_B0, INST_B1, INST_B2, INST_B3, INST_B4, INST_B5, INST_B6, INST_B7
	DD INST_B8, INST_B9, INST_BA, INST_BB, INST_BC, INST_BD, INST_BE, INST_BF
	DD INST_C0, INST_C1, INST_C2, INST_C3, INST_C4, INST_C5, INST_C6, INST_C7
	DD INST_C8, INST_C9, INST_CA, INST_CB, INST_CC, INST_CD, INST_CE, INST_CF
	DD INST_D0, INST_D1, INST_D2, INST_D3, INST_D4, INST_D5, INST_D6, INST_D7
	DD INST_D8, INST_D9, INST_DA, INST_DB, INST_DC, INST_DD, INST_DE, INST_DF
	DD INST_E0, INST_E1, INST_E2, INST_E3, INST_E4, INST_E5, INST_E6, INST_E7
	DD INST_E8, INST_E9, INST_EA, INST_EB, INST_EC, INST_ED, INST_EE, INST_EF
	DD INST_F0, INST_F1, INST_F2, INST_F3, INST_F4, INST_F5, INST_F6, INST_F7
	DD INST_F8, INST_F9, INST_FA, INST_FB, INST_FC, INST_FD, INST_FE, INST_FF

PUBLICL CPU_PC
	DD 0
PUBLICL CPU_CYCLE
	DD 0
PUBLICL CPU_PENDING
	DD 0
PUBLICL CPU_REGA
	DD 0
PUBLICL CPU_REGX
	DD 0
PUBLICL CPU_REGY
	DD 0
PUBLICL CPU_REGS
	DD 0
PUBLICL CPU_CFLAG
	DD 0
PUBLICL CPU_ZFLAG
	DD 0
PUBLICL CPU_IFLAG
	DD 0
PUBLICL CPU_DFLAG
	DD 0
PUBLICL CPU_BFLAG
	DD 0
PUBLICL CPU_VFLAG
	DD 0
PUBLICL CPU_NFLAG
	DD 0
PUBLICL DEBUG_FLAG		;debug
	DD 0
PUBLICL CPU_RENDLABEL


;;;;;     Bit No.       7   6   5   4   3   2   1   0
;;;;;                   S   V       B   D   I   Z   C


section .text code align=4 use32


PUBLICL INST_00		; BRK		*
	inc word[CPU_PC]
	call CPU_IRQ
	SETBFLAG
	ret



;********STACK**********/

PUBLICL INST_08		;PHP
	CONVFLAG
	S_PUSH
	ENDINST1 3


PUBLICL INST_28		;PLP
	S_POP
	RCONVFLAG
	ENDINST1 4

PUBLICL INST_48		;PHA
	mov		al,byte[CPU_REGA],
	S_PUSH
	ENDINST1 3

PUBLICL INST_68		;PLA
	S_POP
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST1 4




;***************AND***************/

PUBLICL INST_21		;AND - (Indirect,X)
	INDEXEDINDIRECT CPU_REGX
	and		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,6

PUBLICL INST_25		;AND - Zero Page
	ZEROPAGER
	and		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,3

PUBLICL INST_29		;AND - Immediate
	and		ah,byte[CPU_REGA]
	mov		byte[CPU_REGA],ah
	SETFLAG1 ah
	ENDINST 2,2

PUBLICL INST_2D		;AND - Absolute
	ABSOLUTE
	and		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 3,4

PUBLICL INST_31		;AND - (Indirect),Y
	INDIRECTINDEXED CPU_REGY
	and		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,5

PUBLICL INST_35		;AND - Zero Page,X
	ZEROPAGEIND_RD CPU_REGX
	and		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,4

PUBLICL INST_39		;AND - Absolute,Y
	ABSOLUTEINDEXEDC CPU_REGY
	and		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 3,4

PUBLICL INST_3D		;AND - Absolute,X
	ABSOLUTEINDEXEDC CPU_REGX
	and		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 3,4



;****************BIT********************/

PUBLICL INST_24		;BIT - Zero Page
	ZEROPAGER
	mov	ah,al
	shl	ah,1
	mov	byte[CPU_VFLAG],ah
	mov	byte[CPU_NFLAG],al
	and al,byte[CPU_REGA]
	lahf
	mov		byte[CPU_ZFLAG],ah
	ENDINST 2,3

PUBLICL INST_2C		;BIT - Absolute
	ABSOLUTE
	mov	ah,al
	shl	ah,1
	mov	byte[CPU_VFLAG],ah
	mov	byte[CPU_NFLAG],al
	and al,byte[CPU_REGA]
	lahf
	mov		byte[CPU_ZFLAG],ah
	ENDINST 3,4



;*****************EOR***********************/

PUBLICL INST_41		;EOR - (Indirect,X)
	INDEXEDINDIRECT CPU_REGX
	xor		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,4

PUBLICL INST_45		;EOR - Zero Page
	ZEROPAGER
	xor		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,3

PUBLICL INST_49		;EOR - Immediate
	xor		ah,byte[CPU_REGA]
	mov		byte[CPU_REGA],ah
	SETFLAG1 ah
	ENDINST 2,2

PUBLICL INST_4D		;EOR - Absolute
	ABSOLUTE
	xor		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 3,4

PUBLICL INST_51		;EOR - (Indirect),Y
	INDIRECTINDEXEDC CPU_REGY
	xor		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,5

PUBLICL INST_55		;EOR - Zero Page,X
	ZEROPAGEIND_RD CPU_REGX
	xor		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,4

PUBLICL INST_59		;EOR - Absolute,Y
	ABSOLUTEINDEXEDC CPU_REGY
	xor		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 3,4

PUBLICL INST_5D		;EOR - Absolute,X
	ABSOLUTEINDEXEDC CPU_REGX
	xor		al,byte[CPU_REGA]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 3,4



;******************ADC*******************/

PUBLICL INST_61		;ADC - (Indirect,X)
	INDIRECTINDEXED CPU_REGX
	ADC_C
	ENDINST 2,6

PUBLICL INST_65		;ADC - Zero Page
	ZEROPAGER
	ADC_C
	ENDINST 2,3

PUBLICL INST_69		;ADC - Immediate
	mov	al,ah
	ADC_C
	ENDINST 2,2

PUBLICL INST_6D		;ADC - Absolute
	ABSOLUTE
	ADC_C
	ENDINST 3,4

PUBLICL INST_71		;ADC - (Indirect),Y
	INDIRECTINDEXEDC CPU_REGY
	ADC_C
	ENDINST 2,5

PUBLICL INST_75		;ADC - Zero Page,X
	ZEROPAGEIND_RD CPU_REGX
	ADC_C
	ENDINST 2,5

PUBLICL INST_79		;ADC - Absolute,Y
	ABSOLUTEINDEXEDC CPU_REGY
	ADC_C
	ENDINST 3,4

PUBLICL INST_7D		;ADC - Absolute,X
	ABSOLUTEINDEXEDC CPU_REGX
	ADC_C
	ENDINST 3,4



;******************Trans**********************/
PUBLICL INST_8A		;TXA	*
	mov		al,byte[CPU_REGX]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST1 2

PUBLICL INST_98		;TYA	*
	mov		al,byte[CPU_REGY]
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST1 2

PUBLICL INST_9A		;TXS	*
	mov		al,byte[CPU_REGX]
	mov		byte[CPU_REGS],al
;	SETFLAG1 al
	ENDINST1 2

PUBLICL INST_A8		;TAY	*
	mov		al,byte[CPU_REGA]
	mov		byte[CPU_REGY],al
	SETFLAG1 al
	ENDINST1 2

PUBLICL INST_AA		;TAX	*
	mov		al,byte[CPU_REGA]
	mov		byte[CPU_REGX],al
	SETFLAG1 al
	ENDINST1 2

PUBLICL INST_BA		;TSX	*
	mov		al,byte[CPU_REGS]
	mov		byte[CPU_REGX],al
	SETFLAG1 al
	ENDINST1 2


;******************SUB*****************/
PUBLICL INST_E1		;SBC - (Indirect,X)
	INDIRECTINDEXED CPU_REGX
	SBC_C
	ENDINST 2,6

PUBLICL INST_E5		;SBC - Zero Page
	ZEROPAGER
	SBC_C
	ENDINST 2,3

PUBLICL INST_E9		;SBC - Immediate
	mov	al,ah
	SBC_C
	ENDINST 2,2

PUBLICL INST_ED		;SBC - Absolute
	ABSOLUTE
	SBC_C
	ENDINST 3,4

PUBLICL INST_F1		;SBC - (Indirect),Y
	INDIRECTINDEXEDC CPU_REGY
	SBC_C
	ENDINST 2,5

PUBLICL INST_F5		;SBC - Zero Page,X
	ZEROPAGEIND_RD CPU_REGX
	SBC_C
	ENDINST 2,5

PUBLICL INST_F9		;SBC - Absolute,Y
	ABSOLUTEINDEXEDC CPU_REGY
	SBC_C
	ENDINST 3,4

PUBLICL INST_FD		;SBC - Absolute,X
	ABSOLUTEINDEXEDC CPU_REGX
	SBC_C
	ENDINST 3,4



;**************DEC************/

PUBLICL INST_88		;DEY		*
	dec		byte[CPU_REGY]
	mov		al,byte[CPU_REGY]
	SETFLAG1 al
	ENDINST1 2

PUBLICL INST_C6		;DEC - Zero Page	*
	ZEROPAGEA
	dec		byte[ebx]
	mov		al,byte[ebx]
	SETFLAG1 al
	ENDINST 2,5

PUBLICL INST_CA		;DEX		*
	dec		byte[CPU_REGX]
	mov		al,byte[CPU_REGX]
	SETFLAG1 al
	ENDINST1 2

PUBLICL INST_CE		;DEC - Absolute
	shr		eax,8
	push	ax
	call	READ_FUNC
	dec		al
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,6

PUBLICL INST_D6		;DEC - Zero Page,X
	ZEROPAGEA
	dec		byte[ebx]
	mov		al,byte[ebx]
	SETFLAG1 al
	ENDINST 2,6

PUBLICL INST_DE		;DEC - Absolute,X 
	shr		eax,8
	push	ax
	call	READ_FUNC
	dec		al
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,7



;**************INC***********/

PUBLICL INST_C8		;INY	*
	inc		byte[CPU_REGY]
	mov		al,[CPU_REGY]
	SETFLAG1 al
	ENDINST1 2

PUBLICL INST_E6		;INC - Zero Page	*
	ZEROPAGEA
	inc		byte[ebx]
	mov		al,byte[ebx]
	SETFLAG1 al
	ENDINST 2,5

PUBLICL INST_E8		;INX	*
	inc		byte[CPU_REGX]
	mov		al,byte[CPU_REGX]
	SETFLAG1 al
	ENDINST1 2

PUBLICL INST_EE		;INC - Absolute		*
	shr		eax,8
	push	ax
	call	READ_FUNC
	inc		al
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,2

PUBLICL INST_F6		;INC - Zero Page,X
	ZEROPAGEA
	add		ebx,dword[CPU_REGX]
	inc		byte[ebx]
	mov		al,byte[ebx]
	SETFLAG1 al
	ENDINST 2,6

PUBLICL INST_FE		;INC - Absolute,X
	shr		eax,8
	push	ax
	call	READ_FUNC
	inc		al
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,7





;**************shift************/

PUBLICL INST_06		;ASL - Zero Page		*
	ZEROPAGER
	shl		al,1
	SETSHIFTCFLAG
	mov		byte[ebx],al
	SETFLAG1 al
	ENDINST 2,5

PUBLICL INST_0A		;ASL - Accumulator		*
	shl		byte[CPU_REGA],1
	SETSHIFTCFLAG
	mov		al,byte[CPU_REGA]
	SETFLAG1 al
	ENDINST 1,2

PUBLICL INST_0E		;ASL - Absolute 		*
	shr		eax,8
	push	ax
	call	READ_FUNC
	shl		al,1
	SETSHIFTCFLAG
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,6

PUBLICL INST_16		;ASL - Zero Page,X		*
	ZEROPAGEIND_RD CPU_REGX
	shl		al,1
	SETSHIFTCFLAG
	mov		byte[ebx],al
	SETFLAG1 al
	ENDINST 2,6

PUBLICL INST_1E		;ASL - Absolute,X
	shr		eax,8
	add		ax,word[CPU_REGX]
	push	ax
	call	READ_FUNC
	shl		al,1
	SETSHIFTCFLAG
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,7



PUBLICL INST_26		;ROL - Zero Page
	ZEROPAGER
	mov		ah,byte[CPU_CFLAG]
	sahf
	rcl		al,1
	SETSHIFTCFLAG
	mov		byte[ebx],al
	SETFLAG1 al
	ENDINST 2,5

PUBLICL INST_2A		;ROL - Accumulator
	mov		ah,byte[CPU_CFLAG]
	sahf
	rcl		byte[CPU_REGA],1
	SETSHIFTCFLAG
	mov		al,byte[CPU_REGA]
	SETFLAG1 al
	ENDINST 1,2

PUBLICL INST_2E		;ROL - Absolute
	shr		eax,8
	push	ax
	call	READ_FUNC
	mov		ah,byte[CPU_CFLAG]
	sahf
	rcl		al,1
	SETSHIFTCFLAG
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,6

PUBLICL INST_36		;ROL - Zero Page,X
	ZEROPAGEIND_RD CPU_REGX
	mov		ah,byte[CPU_CFLAG]
	sahf
	rcl		al,1
	SETSHIFTCFLAG
	mov		byte[ebx],al
	SETFLAG1 al
	ENDINST 2,6

PUBLICL INST_3E		;ROL - Absolute,X
	shr		eax,8
	add		ax,word[CPU_REGX]
	push	ax
	call	READ_FUNC
	mov		ah,byte[CPU_CFLAG]
	sahf
	rcl		al,1
	SETSHIFTCFLAG
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,7




PUBLICL INST_46		;LSR - Zero Page
	ZEROPAGER
	shr		al,1
	SETSHIFTCFLAG
	mov		byte[ebx],al
	SETFLAG1 al
	ENDINST 2,5

PUBLICL INST_4A		;LSR - Accumulator
	shr		byte[CPU_REGA],1
	SETSHIFTCFLAG
	mov		al,byte[CPU_REGA]
	SETFLAG1 al
	ENDINST 1,2

PUBLICL INST_4E		;LSR - Absolute
	shr		eax,8
	push	ax
	call	READ_FUNC
	shr		al,1
	SETSHIFTCFLAG
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,6

PUBLICL INST_56		;LSR - Zero Page,X
	ZEROPAGEIND_RD CPU_REGX
	shr		al,1
	SETSHIFTCFLAG
	mov		byte[ebx],al
	SETFLAG1 al
	ENDINST 2,6

PUBLICL INST_5E		;LSR - Absolute,X
	shr		eax,8
	add		ax,word[CPU_REGX]
	push	ax
	call	READ_FUNC
	shr		al,1
	SETSHIFTCFLAG
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,7




PUBLICL INST_66		;ROR - Zero Page
	ZEROPAGER
	mov		ah,byte[CPU_CFLAG]
	sahf
	rcr		al,1
	SETSHIFTCFLAG
	mov		byte[ebx],al
	SETFLAG1 al
	ENDINST 2,5

PUBLICL INST_6A		;ROR - Accumulator
	mov		ah,byte[CPU_CFLAG]
	sahf
	rcr		byte[CPU_REGA],1
	SETSHIFTCFLAG
	mov		al,byte[CPU_REGA]
	SETFLAG1 al
	ENDINST 1,2

PUBLICL INST_6E		;ROR - Absolute
	shr		eax,8
	push	ax
	call	READ_FUNC
	mov		ah,byte[CPU_CFLAG]
	sahf
	rcr		al,1
	SETSHIFTCFLAG
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,6

PUBLICL INST_76		;ROR - Zero Page,X
	ZEROPAGEIND_RD CPU_REGX
	mov		ah,byte[CPU_CFLAG]
	sahf
	rcr		al,1
	SETSHIFTCFLAG
	mov		byte[ebx],al
	SETFLAG1 al
	ENDINST 2,6

PUBLICL INST_7E		;ROR - Absolute,X
	shr		eax,8
	add		ax,word[CPU_REGX]
	push	ax
	call	READ_FUNC
	mov		ah,byte[CPU_CFLAG]
	sahf
	rcr		al,1
	SETSHIFTCFLAG
	SETFLAG1 al
	mov		dl,al
	pop		ax
	call	WRITE_FUNC
	ENDINST 3,7













;********************OR**************************/
PUBLICL INST_01		; ORA - (Indirect,X)		*
	INDEXEDINDIRECT CPU_REGX
	or		byte[CPU_REGA],al
	mov		dl,byte[CPU_REGA]
	SETFLAG1 dl
	ENDINST 2,6



PUBLICL INST_05		;ORA - Zero Page		*
	ZEROPAGER
	or		byte[CPU_REGA],al
	mov		dl,byte[CPU_REGA]
	SETFLAG1 dl
	ENDINST 2,3

PUBLICL INST_09		;ORA - Immediate
	or		byte[CPU_REGA],ah
	mov		dl,[CPU_REGA]
	SETFLAG1 dl
	ENDINST 2,2


PUBLICL INST_0D		;ORA - Absolute
	ABSOLUTE
	or		byte[CPU_REGA],al
	mov		dl,byte[CPU_REGA]
	SETFLAG1 dl
	ENDINST 3,4


PUBLICL INST_11		;ORA - (Indirect),Y
	INDIRECTINDEXED CPU_REGY
	or		byte[CPU_REGA],al
	mov		dl,byte[CPU_REGA]
	SETFLAG1 dl
	ENDINST 2,5


PUBLICL INST_15		;ORA - Zero Page,X
	ZEROPAGEIND_RD CPU_REGX
	or		byte[CPU_REGA],al
	mov		dl,byte[CPU_REGA]
	SETFLAG1 dl
	ENDINST 2,4

PUBLICL INST_19		;ORA - Absolute,Y
	ABSOLUTEINDEXEDC CPU_REGY
	or		byte[CPU_REGA],al
	mov		dl,byte[CPU_REGA]
	SETFLAG1 dl
	ENDINST 3,4

PUBLICL INST_1D		;ORA - Absolute,X
	ABSOLUTEINDEXEDC CPU_REGX
	or		byte[CPU_REGA],al
	mov		dl,byte[CPU_REGA]
	SETFLAG1 dl
	ENDINST 3,4





;*******CMP*******/
PUBLICL INST_C0		;Cpy - Immediate	*
	xor		bx,bx
	mov		dx,word[CPU_REGY]
	mov		bl,ah
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 2,2

PUBLICL INST_C1		;CMP - (Indirect,X)	*
	INDEXEDINDIRECT CPU_REGX
	xor		bx,bx
	mov		dx,word[CPU_REGA]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 2,6


PUBLICL INST_C4		;CPY - Zero Page	*
	ZEROPAGER
	xor		bx,bx
	mov		dx,word[CPU_REGY]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 2,3

PUBLICL INST_C5		;CMP - Zero Page	*
	ZEROPAGER
	xor		bx,bx
	mov		dx,word[CPU_REGA]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 2,3

PUBLICL INST_C9		;CMP - Immediate	*
	xor		bx,bx
	mov		dx,word[CPU_REGA]
	mov		bl,ah
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 2,2

PUBLICL INST_CC		;CPY - Absolute			*
	ABSOLUTE
	xor		bx,bx
	mov		dx,word[CPU_REGY]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 3,4

PUBLICL INST_CD		;CMP - Absolute			*
	ABSOLUTE
	xor		bx,bx
	mov		dx,word[CPU_REGA]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 3,4

PUBLICL INST_D1		;CMP (Indirect@,Y 		*
	INDIRECTINDEXEDC CPU_REGY
	xor		bx,bx
	mov		dx,word[CPU_REGA]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 2,5

PUBLICL INST_D5		;CMP - Zero Page,X 	*
	ZEROPAGEIND_RD CPU_REGX
	xor		bx,bx
	mov		dx,word[CPU_REGA]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 2,4

PUBLICL INST_D9		;CMP - Absolute,Y	*
	ABSOLUTEINDEXED CPU_REGY
	xor		bx,bx
	mov		dx,word[CPU_REGA]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 3,4

PUBLICL INST_DD		;CMP - Absolute,X	*
	ABSOLUTEINDEXED CPU_REGX
	xor		bx,bx
	mov		dx,word[CPU_REGA]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 3,4

PUBLICL INST_E0		;CPX - Immediate	*
	xor		bx,bx
	mov		dx,word[CPU_REGX]
	mov		bl,ah
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 2,2

PUBLICL INST_E4		;CPX - Zero Page	*
	ZEROPAGER
	xor		bx,bx
	mov		dx,word[CPU_REGX]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 2,3

PUBLICL INST_EC		;CPX - Absolute		*
	ABSOLUTE
	xor		bx,bx
	mov		dx,word[CPU_REGX]
	mov		bl,al
	sub		dx,bx
	not		dh
	mov		byte[CPU_CFLAG],dh
	SETFLAG1 dl
	ENDINST 3,4




;*******JMP*******/

PUBLICL INST_10		;BPL			*
	IFNFLAGSET .l1
	RELATIVEJMP
	add		word[CPU_CYCLE],2
	ret
.l1
	ENDINST 2,2


PUBLICL INST_20		;JSR			*
	shr		eax,8
	mov		dx,ax
	mov		ebx,dword[CPU_REGS]
	mov		ax,word[CPU_PC]
	add		ebx,dword[MAIN_MEMORY]
	add		ebx,100h
	add		ax,2
	dec		ebx
	mov		word[ebx],ax
	mov		word[CPU_PC],dx
	sub		byte[CPU_REGS],2
	add		word[CPU_CYCLE],6
	ret


PUBLICL INST_30		;BMI			*
	IFNFLAGCLEAR .l1
	RELATIVEJMP
	add		word[CPU_CYCLE],2
	ret
.l1
	ENDINST 2,2


PUBLICL INST_40		; RTI			*
	mov		ebx,dword[CPU_REGS]
	add		ebx,dword[MAIN_MEMORY]
	add		ebx,100h
	inc		ebx
	mov		al,byte[ebx]
	RCONVFLAG
	inc		ebx
	mov		ax,word[ebx]
	mov		word[CPU_PC],ax
	add		byte[CPU_REGS],3
	add		word[CPU_CYCLE],6
	test	byte[CPU_PENDING],0ffh
	jz		.l1
	call	CPU_IRQ
	mov		byte[CPU_PENDING],0
.l1
	ret

PUBLICL INST_4C		;JMP <Absolute>		*
	shr		eax,8
	mov		word[CPU_PC],ax
	add		dword[CPU_CYCLE],3
	ret


PUBLICL INST_50		;BVC				*
	IFVFLAGSET .l1
	RELATIVEJMP
	add		word[CPU_CYCLE],2
	ret
.l1
	ENDINST 2,2

PUBLICL INST_60		;RTS				*
	mov		ebx,dword[CPU_REGS]
	add		ebx,dword[MAIN_MEMORY]
	add		ebx,100h
	inc		ebx
	mov		ax,word[ebx]
	inc		ax
	mov		word[CPU_PC],ax
	add		byte[CPU_REGS],2
	add		word[CPU_CYCLE],6
	ret


PUBLICL INST_6C		;JMP <Indirect>		*
	shr		eax,8
	call	WORD_READ
	mov		word[CPU_PC],ax
	add		dword[CPU_CYCLE],5
	ret

PUBLICL INST_70		;BVS			*
	IFVFLAGCLEAR .l1
	RELATIVEJMP
	add		word[CPU_CYCLE],2
	ret
.l1
	ENDINST 2,2

PUBLICL INST_90		;BCC			*
	IFCFLAGSET .l1
	RELATIVEJMP
	add		word[CPU_CYCLE],2
	ret
.l1
	ENDINST 2,2

PUBLICL INST_B0		;BCS			*
	IFCFLAGCLEAR .l1
	RELATIVEJMP
	add		word[CPU_CYCLE],2
	ret
.l1
	ENDINST 2,2

PUBLICL INST_D0		;BNE			*
	IFZFLAGSET .l1
	RELATIVEJMP
	add		word[CPU_CYCLE],2
	ret
.l1
	ENDINST 2,2

PUBLICL INST_F0		;BEQ			*
	IFZFLAGCLEAR .l1
	RELATIVEJMP
	add		word[CPU_CYCLE],2
	ret
.l1
	ENDINST 2,2






;********store********/

PUBLICL INST_81		;STA - (Indirect,X)	*
	INDEXEDINDIRECTW CPU_REGX,CPU_REGA
	ENDINST 2,6

PUBLICL INST_84		;STY - Zero Page	*
	mov		dl,byte[CPU_REGY]
	ZEROPAGEW
	ENDINST 2,3

PUBLICL INST_85		;STA - Zero Page	*
	mov		dl,byte[CPU_REGA]
	ZEROPAGEW
	ENDINST 2,3

PUBLICL INST_86		;STX - Zero Page	*
	mov		dl,byte[CPU_REGX]
	ZEROPAGEW
	ENDINST 2,3

PUBLICL INST_8C		;STY - Absolute		*
	mov		dl,byte[CPU_REGY]
	ABSOLUTEW
	ENDINST 3,4

PUBLICL INST_8D		;STA - Absolute		*
	mov		dl,byte[CPU_REGA]
	ABSOLUTEW
	ENDINST 3,4

PUBLICL INST_8E		;STX - Absolute		*
	mov		dl,byte[CPU_REGX]
	ABSOLUTEW
	ENDINST 3,4

PUBLICL INST_91		;STA - (Indirect),Y	*
	INDIRECTINDEXEDW CPU_REGY,CPU_REGA
	ENDINST 2,6

PUBLICL INST_94		;STY - Zero Page,X	*
	mov		dl,byte[CPU_REGY]
	ZEROPAGEIND_WT	CPU_REGX
	ENDINST 2,4

PUBLICL INST_95		;STA - Zero Page,X	*
	mov		dl,byte[CPU_REGA]
	ZEROPAGEIND_WT	CPU_REGX
	ENDINST 2,4

PUBLICL INST_96		;STX - Zero Page,Y	*
	mov		dl,byte[CPU_REGX]
	ZEROPAGEIND_WT	CPU_REGY
	ENDINST 2,4

PUBLICL INST_99		;STA - Absolute,Y	*
	mov		dl,byte[CPU_REGA]
	ABSOLUTEINDEXEDW CPU_REGY
	ENDINST 3,5

PUBLICL INST_9D		;STA - Absolute,X	*
	mov		dl,byte[CPU_REGA]
	ABSOLUTEINDEXEDW CPU_REGX
	ENDINST 3,5






;********LD*******/
PUBLICL INST_A0		;LDY - Immediate		*
	IMMEDIATE	CPU_REGY
	SETFLAG1 ah
	ENDINST 2,2

PUBLICL INST_A2		;LDX - Immediate		*
	IMMEDIATE	CPU_REGX
	SETFLAG1 ah
	ENDINST 2,2

PUBLICL INST_A9		;LDA - Immediate		*
	IMMEDIATE	CPU_REGA
	SETFLAG1 ah
	ENDINST 2,2

PUBLICL INST_A4		;LDY - Zero Page		*
	ZEROPAGER
	mov		byte[CPU_REGY],al
	SETFLAG1 al
	ENDINST 2,3

PUBLICL INST_A5		;LDA - Zero Page		*
	ZEROPAGER
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,3

PUBLICL INST_A6		;LDX - Zero Page		*
	ZEROPAGER
	mov		byte[CPU_REGX],al
	SETFLAG1 al
	ENDINST 2,3

PUBLICL INST_AC		;LDY - Absolute			*
	ABSOLUTE
	mov		byte[CPU_REGY],al
	SETFLAG1 al
	ENDINST 3,4

PUBLICL INST_AD		;LDA - Absolute			*
	ABSOLUTE
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 3,4

PUBLICL INST_AE		;LDX - Absolute			*
	ABSOLUTE
	mov		byte[CPU_REGX],al
	SETFLAG1 al
	ENDINST 3,4

PUBLICL INST_B1		;LDA - (Indirect),Y		*
	INDIRECTINDEXEDC CPU_REGY
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,5

PUBLICL INST_A1		;LDA - (Indirect,X)		*
	INDEXEDINDIRECT CPU_REGX
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,6

PUBLICL INST_B4		;LDY - Zero Page,X		*
	ZEROPAGEIND_RD	CPU_REGX
	mov		byte[CPU_REGY],al
	SETFLAG1 al
	ENDINST 2,2

PUBLICL INST_B5		;LDA - Zero Page,X		*
	ZEROPAGEIND_RD CPU_REGX
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 2,4

PUBLICL INST_B6		;LDX - Zero Page,Y		*
	ZEROPAGEIND_RD CPU_REGY
	mov		byte[CPU_REGX],al
	SETFLAG1 al
	ENDINST 2,4

PUBLICL INST_B9		;LDA - Absolute,Y		*
	ABSOLUTEINDEXEDC CPU_REGY
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 3,4

PUBLICL INST_BC		;LDY - Absolute,X		*
	ABSOLUTEINDEXEDC CPU_REGX
	mov		byte[CPU_REGY],al
	SETFLAG1 al
	ENDINST 3,4

PUBLICL INST_BD		;LDA - Absolute,X		*
	ABSOLUTEINDEXEDC CPU_REGX
	mov		byte[CPU_REGA],al
	SETFLAG1 al
	ENDINST 3,4

PUBLICL INST_BE		;LDX - Absolute,Y		*
	ABSOLUTEINDEXEDC CPU_REGY
	mov		byte[CPU_REGX],al
	SETFLAG1 al
	ENDINST 3,4


;********FLAG*******/

PUBLICL INST_18		;CLC
	CLEARCFLAG
	ENDINST1 2

PUBLICL INST_38		;SEC
	SETCFLAG
	ENDINST1 2

PUBLICL INST_58		;CLI
	CLEARIFLAG
	test	byte[CPU_PENDING],0ffh
	jz		.l1
	call	CPU_IRQ
	mov		byte[CPU_PENDING],0
.l1
	ENDINST1 2

PUBLICL INST_78		;SEI
	SETIFLAG
	ENDINST1 2

PUBLICL INST_B8		;CLV
	CLEARVFLAG
	ENDINST1 2

PUBLICL INST_D8		;CLD
	CLEARDFLAG
	ENDINST1 2

PUBLICL INST_F8		;SED
	SETDFLAG
	ENDINST1 2


;***************************************************************************


;ax=address , [ret] eax=data
PUBLICL READ_FUNC
	and		eax,0ffffh
	push	eax
	mov		esi,dword[READ_FUNCA]
	call	esi
	pop		edx
	ret


;ax=address , dl=data;
PUBLICL WRITE_FUNC
	and		eax,0ffffh
	and		edx,0ffh
	push	edx
	push	eax
	mov		esi,dword[WRITE_FUNCA]
	call	esi
	pop		eax
	pop		eax
	ret

PUBLICL WORD_READ
;	push	ebx
;	push	edx
;	push	esi
	xor		ebx,ebx
	xor		edx,edx
	mov		dx,ax
	mov		bl,ah
	shr		bx,5
	shl		bx,2
	and		dx,1fffh
	add		ebx,MEM_PAGEP
	mov		esi,dword[ebx]
	add		esi,edx
	mov		ax,word[esi]
;	pop		esi
;	pop		edx
;	pop		ebx
	ret


PUBLICL DWORD_READ
;	push	ebx
;	push	edx
;	push	esi
	xor		ebx,ebx
	xor		edx,edx
	mov		dx,ax
	mov		bl,ah
	shr		bx,5
	shl		bx,2
	and		dx,1fffh
	add		ebx,MEM_PAGEP
	mov		esi,dword[ebx]
	add		esi,edx
	mov		eax,dword[esi]
;	pop		esi
;	pop		edx
;	pop		ebx
	ret



PUBLICL CPU_NMI
	push	10
	call	ASMDEBUG_LOGN 
	pop		eax
	
	pusha
	mov		ebx,dword[MAIN_MEMORY]
	add		ebx,100h
	mov		ax,word[CPU_PC]
	add		ebx,dword[CPU_REGS]
	dec		ebx
	mov		word[ebx],ax
	dec		ebx
	CONVFLAG
	mov		byte[ebx],al
	sub		byte[CPU_REGS],3
	CLEARBFLAG
	SETIFLAG
	add		dword[CPU_CYCLE],7
	mov		ax,0fffah
	call	WORD_READ
	mov		word[CPU_PC],ax
	popa
	ret

PUBLICL CPU_IRQ
	push	11
	call	ASMDEBUG_LOGN 
	pop		eax
	
	pusha
	mov		ebx,dword[MAIN_MEMORY]
	add		ebx,100h
	mov		ax,word[CPU_PC]
	add		ebx,dword[CPU_REGS]
	dec		ebx
	mov		word[ebx],ax
	dec		ebx
	CONVFLAG
	mov		byte[ebx],al
	sub		byte[CPU_REGS],3
	CLEARBFLAG
	SETIFLAG
	add		dword[CPU_CYCLE],7
	mov		ax,0fffeh
	call	WORD_READ
	mov		word[CPU_PC],ax
	popa
	ret




PUBLICL CPU_EXECUTE
	push	es
	mov		eax,[esp+8]
	mov		dx,ds
	mov		es,dx
	
;	pusha
.l1
	CMP		eax,dword[CPU_CYCLE]
	jbe		.l2
	test	byte[CPU_PENDING],0ffh
	jz		.lirq
	call	CPU_IRQ
	mov		byte[CPU_PENDING],0
.lirq
	push	eax
	mov		eax,[CPU_PC]
%if 1							;debug
	mov		ebx,CPU_PC		;debug
	push	ebx				;debug
%endif
	call	DWORD_READ
%if 1							;debug
	push	eax
	CONVFLAG
	mov		byte[DEBUG_FLAG],al
;	call	ASMDEBUG_LOG
	pop		eax
	pop		ebx
%endif
	xor		ebx,ebx
	mov		bl,al
	shl		ebx,2
	add		ebx,CPU_INSTTABLE
	call	[ebx]
	pop eax
	jmp .l1
.l2
	sub		dword[CPU_CYCLE],eax
;	popa
	mov		eax,[esp+8]
	pop		es
	ret


PUBLICL CPU_RESET
	push	4
	call	ASMDEBUG_LOGN 
	pop		eax

	pusha
	mov		ax,ds
	mov		es,ax
	mov		ecx,CPU_RENDLABEL - CPU_CYCLE
	mov		edi,CPU_CYCLE
	xor		eax,eax
	rep		stosb
	mov		byte[CPU_REGS],0ffh
	mov		dword[CPU_CYCLE],6
	mov		ax,0fffch
	call	WORD_READ
	mov		word[CPU_PC],ax
	SETIFLAG
	SETZFLAG
	popa
	ret


PUBLICL CPU_BURN
	push	3
	call	ASMDEBUG_LOGN 
	pop		eax
	
	mov		eax,[esp+4]
	add		dword[CPU_CYCLE],eax
	ret


PUBLICL CPU_GETCYCLES
	push	2
	call	ASMDEBUG_LOGN 
	pop		eax

;	push	edx
	mov		eax,[esp+4]
	mov		edx,dword[CPU_CYCLE]
	test	eax,eax
	jz		.l1
	mov		dword[CPU_CYCLE],0
.l1
;	pop		edx
	ret


PUBLICL CPU_GETBYTE
	push	1
	call	ASMDEBUG_LOGN 
	pop		eax
	
	mov		eax,[esp+4]
	call	WORD_READ
	ret



;*******TOP********
PUBLICL INST_0C
PUBLICL INST_1C
PUBLICL INST_3C
PUBLICL INST_5C
PUBLICL INST_7C
PUBLICL INST_DC
PUBLICL INST_FC
	ENDINST 2,4

;*******DOP(3)********
PUBLICL INST_04
PUBLICL INST_44
PUBLICL INST_64
	ENDINST1 3


;*******DOP(2)********
PUBLICL INST_80
PUBLICL INST_82
PUBLICL INST_89
PUBLICL INST_C2
PUBLICL INST_E2
	ENDINST1 2

;*******DOP(4)********
PUBLICL INST_14
PUBLICL INST_34
PUBLICL INST_54
PUBLICL INST_74
PUBLICL INST_D4
PUBLICL INST_F4
	ENDINST1 4


;**********NOP************
PUBLICL INST_EA		;NOP
PUBLICL INST_1A		;--
PUBLICL INST_3A		;--
PUBLICL INST_5A		;--
PUBLICL INST_7A		;--
PUBLICL INST_DA		;--
PUBLICL INST_FA		;--
	ENDINST1 2





;**********----****************
PUBLICL INST_02		;--
PUBLICL INST_12		;--
PUBLICL INST_22		;--
PUBLICL INST_32		;--
PUBLICL INST_42		;--
PUBLICL INST_52		;--
PUBLICL INST_62		;--
PUBLICL INST_72		;--
PUBLICL INST_92		;--
PUBLICL INST_B2		;--
PUBLICL INST_D2		;--
PUBLICL INST_F2		;--
	inc		word[CPU_PC]
	ret


PUBLICL INST_03		;SLO ($nn,X) 
PUBLICL INST_07		;SLO $nn
PUBLICL INST_0B		;ANC #$nn
PUBLICL INST_0F		;SLO $nnnn
PUBLICL INST_13		;SLO ($nn),Y
PUBLICL INST_17		;SLO $nn,X
PUBLICL INST_1B		;SLO $nnnn,Y
PUBLICL INST_1F		;SLO $nnnn,X
PUBLICL INST_23		;RLA ($nn,X)
PUBLICL INST_27		;RLA $nn
PUBLICL INST_2B		;ANC #$nn
PUBLICL INST_2F		;RLA $nnnn
PUBLICL INST_33		;RLA ($nn),Y
PUBLICL INST_37		;RLA $nn,X
PUBLICL INST_3B		;RLA $nnnn,Y
PUBLICL INST_3F		;RLA $nnnn,X 
PUBLICL INST_43		;SRE ($nn,X)
PUBLICL INST_47		;SRE $nn
PUBLICL INST_4B		;ASR #$nn
PUBLICL INST_4F		;SRE $nnnn 
PUBLICL INST_53		;SRE ($nn),Y
PUBLICL INST_57		;SRE $nn,X
PUBLICL INST_5B		;SRE $nnnn,Y
PUBLICL INST_5F		;SRE $nnnn,X
PUBLICL INST_63		;RRA ($nn,X)
PUBLICL INST_67		;RRA $nn
PUBLICL INST_6B		;ARR #$nn
PUBLICL INST_6F		;RRA $nnnn
PUBLICL INST_73		;RRA ($nn),Y
PUBLICL INST_77		;RRA $nn,X
PUBLICL INST_7B		;RRA $nnnn,Y
PUBLICL INST_7F		;RRA $nnnn,X
PUBLICL INST_83		;SAX ($nn,X)
PUBLICL INST_87		;SAX $nn
PUBLICL INST_8B		;ANE #$nn 
PUBLICL INST_8F		;SAX $nnnn
PUBLICL INST_93		;SHA ($nn),Y
PUBLICL INST_97		;SAX $nn,Y
PUBLICL INST_9B		;SHS $nnnn,Y
PUBLICL INST_9C		;SHY $nnnn,X
PUBLICL INST_9E		;SHX $nnnn,Y
PUBLICL INST_9F		;SHA $nnnn,Y
PUBLICL INST_A3		;LAX ($nn,X)
PUBLICL INST_A7		;LAX $nn
PUBLICL INST_AB		;LXA #$nn 
PUBLICL INST_AF		;LAX $nnnn
PUBLICL INST_B3		;LAX ($nn),Y
PUBLICL INST_B7		;LAX $nn,Y
PUBLICL INST_BB		;LAS $nnnn,Y
PUBLICL INST_BF		;LAX $nnnn,Y
PUBLICL INST_C3		;DCP ($nn,X)
PUBLICL INST_C7		;DCP $nn
PUBLICL INST_CB		;SBX #$nn
PUBLICL INST_CF		;DCP $nnnn
PUBLICL INST_D3		;DCP ($nn),Y
PUBLICL INST_D7		;DCP $nn,X
PUBLICL INST_DB		;DCP $nnnn,Y
PUBLICL INST_DF		;DCP $nnnn,X
PUBLICL INST_E3		;ISB ($nn,X)
PUBLICL INST_E7		;ISB $nn
PUBLICL INST_EB		;USBC #$nn
PUBLICL INST_EF		;ISB $nnnn
PUBLICL INST_F3		;ISB ($nn),Y
PUBLICL INST_F7		;ISB $nn,X
PUBLICL INST_FB		;ISB $nnnn,Y
PUBLICL INST_FF		;ISB $nnnn,X
	inc		word[CPU_PC]
	ret


section .bss
