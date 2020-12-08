
#include <ddraw.h>
#include <windows.h>

void convto16_565bitpalette(DWORD *spalette, WORD *dpalette);
void convto16_555bitpalette(DWORD *spalette, WORD *dpalette);

int ddrawblt_8bit(DDSURFACEDESC2 *ddsd, DWORD *palette, unsigned char *bmpd, DWORD width, DWORD height, DWORD dPitch){
	DWORD pitch = ddsd->lPitch;
	DWORD *surfp = (DWORD *)ddsd->lpSurface;
	DWORD *palettep=palette;
	dPitch-=width;
	switch(ddsd->ddpfPixelFormat.dwRGBBitCount){
		case 8:
			pitch-=width;
			_asm{
				push es
				mov ax,ds
				mov es,ax
				mov ecx, height
				mov esi, bmpd
				mov edi, surfp
//				mov ebx, palettep
				add esi, 8
				cld
			blt8_label1:
				push ecx
				mov ecx, width
				shr ecx,2
//			blt8_label2:
				rep movsd
				pop ecx
				add edi, pitch
				add esi, dPitch
				dec ecx
				jnz blt8_label1
				pop es
			}
			break;
		case 16:
			{
				WORD *palette_wp = (WORD *)malloc(2*256);
				if(palette_wp==NULL)
					break;
				pitch-=(2*width);
				if(ddsd->ddpfPixelFormat.dwGBitMask==0x7E0)
					convto16_565bitpalette(palettep, palette_wp);
				else
					convto16_555bitpalette(palettep, palette_wp);
				_asm{
					cld
					mov ecx, height
					mov esi, bmpd
					mov edi, surfp
					mov ebx, palette_wp
					add esi, 8
			blt16_label2:
					push ecx
					mov ecx, width
			blt16_label3:
					movzx eax, BYTE PTR [esi]
					shl eax,1
					inc esi
					mov dx, WORD PTR [ebx+eax]
					mov WORD PTR [edi], dx
					lea edi,[edi+2]
					dec ecx
					jnz blt16_label3
					pop ecx
					add edi, pitch
					add esi, dPitch
					dec ecx
					jnz blt16_label2
				}
				free(palette_wp);
			}
			break;
		case 24:
			pitch-=(3*width);
			_asm{
				cld
				mov ecx, height
				mov esi, bmpd
				mov edi, surfp
				mov ebx, palettep
				add esi, 8
			blt24_label1:
				push ecx
				mov ecx, width
			blt24_label2:
				movzx eax, BYTE PTR [esi]
				inc esi
				mov dx, WORD PTR [ebx+eax*4]
				mov WORD PTR [edi], dx
				mov dl, BYTE PTR [ebx+eax*4+2]
				mov BYTE PTR [edi+2], dl
				lea edi,[edi+3]
				dec ecx
				jnz blt24_label2
				pop ecx
				add edi, pitch
				add esi, dPitch
				dec ecx
				jnz blt24_label1
			}
			break;
		case 32:
			pitch-=(4*width);
			_asm{
				cld
				mov ecx, height
				mov esi, bmpd
				mov edi, surfp
				mov ebx, palettep
				add esi, 8
			blt32_label1:
				push ecx
				mov ecx, width
			blt32_label2:
				movzx eax, BYTE PTR [esi]
				inc esi
				mov edx, DWORD PTR [ebx+eax*4]
				mov DWORD PTR [edi], edx
				lea edi,[edi+4]
				dec ecx
				jnz blt32_label2
				pop ecx
				add edi, pitch
				add esi, dPitch
				dec ecx
				jnz blt32_label1
			}
			break;
	}
	return 1;
}


int ddrawblt_8bit2(DDSURFACEDESC2 *ddsd, DWORD *palette, unsigned char *bmpd, DWORD width, DWORD height, DWORD dPitch){
	DWORD pitch = ddsd->lPitch;
	DWORD *surfp = (DWORD *)ddsd->lpSurface;
	DWORD *palettep=palette;
	dPitch-=width;
	switch(ddsd->ddpfPixelFormat.dwRGBBitCount){
		case 8:
			pitch-=width;
			_asm{
				push es
				mov ax,ds
				mov es,ax
				mov ecx, height
				mov esi, bmpd
				mov edi, surfp
//				mov ebx, palettep
//				add esi, 8
				cld
			blt8_label1:
				push ecx
				mov ecx, width
				shr ecx,2
//			blt8_label2:
				rep movsd
				pop ecx
				add edi, pitch
				add esi, dPitch
				dec ecx
				jnz blt8_label1
				pop es
			}
			break;
		case 16:
			{
				WORD *palette_wp = (WORD *)malloc(2*256);
				if(palette_wp==NULL)
					break;
				pitch-=(2*width);
				if(ddsd->ddpfPixelFormat.dwGBitMask==0x7E0)
					convto16_565bitpalette(palettep, palette_wp);
				else
					convto16_555bitpalette(palettep, palette_wp);
				_asm{
					cld
					mov ecx, height
					mov esi, bmpd
					mov edi, surfp
					mov ebx, palette_wp
//					add esi, 8
			blt16_label2:
					push ecx
					mov ecx, width
			blt16_label3:
					movzx eax, BYTE PTR [esi]
					shl eax,1
					inc esi
					mov dx, WORD PTR [ebx+eax]
					mov WORD PTR [edi], dx
					lea edi,[edi+2]
					dec ecx
					jnz blt16_label3
					pop ecx
					add edi, pitch
					add esi, dPitch
					dec ecx
					jnz blt16_label2
				}
				free(palette_wp);
			}
			break;
		case 24:
			pitch-=(3*width);
			_asm{
				cld
				mov ecx, height
				mov esi, bmpd
				mov edi, surfp
				mov ebx, palettep
//				add esi, 8
			blt24_label1:
				push ecx
				mov ecx, width
			blt24_label2:
				movzx eax, BYTE PTR [esi]
				inc esi
				mov dx, WORD PTR [ebx+eax*4]
				mov WORD PTR [edi], dx
				mov dl, BYTE PTR [ebx+eax*4+2]
				mov BYTE PTR [edi+2], dl
				lea edi,[edi+3]
				dec ecx
				jnz blt24_label2
				pop ecx
				add edi, pitch
				add esi, dPitch
				dec ecx
				jnz blt24_label1
			}
			break;
		case 32:
			pitch-=(4*width);
			_asm{
				cld
				mov ecx, height
				mov esi, bmpd
				mov edi, surfp
				mov ebx, palettep
//				add esi, 8
			blt32_label1:
				push ecx
				mov ecx, width
			blt32_label2:
				movzx eax, BYTE PTR [esi]
				inc esi
				mov edx, DWORD PTR [ebx+eax*4]
				mov DWORD PTR [edi], edx
				lea edi,[edi+4]
				dec ecx
				jnz blt32_label2
				pop ecx
				add edi, pitch
				add esi, dPitch
				dec ecx
				jnz blt32_label1
			}
			break;
	}
	return 1;
}



#define FULLSCREEN_W 272

int ddrawfullscreenblt_8bit(DDSURFACEDESC2 *ddsd, DWORD *palette, unsigned char *bmpd, DWORD height){
	DWORD pitch = ddsd->lPitch;
	DWORD *surfp = (DWORD *)ddsd->lpSurface;
	DWORD *palettep=palette;
	switch(ddsd->ddpfPixelFormat.dwRGBBitCount){
		case 8:
			{
#if 1
/*
				unsigned int i;
				char *ps, *pd;
				ps = (char *)bmpd;
				pd = (char *)surfp;
				for(i=0; i<height; i++, pd+=pitch, ps+=FULLSCREEN_W){
					memcpy(pd, ps, FULLSCREEN_W);
				}
*/
			pitch-=FULLSCREEN_W;
			_asm{
				push es
				mov ax,ds
				mov es,ax
				mov edx, height
				mov esi, bmpd
				mov edi, surfp
				mov ebx, palettep
//				add esi, 0
				cld
			blt8_label1:
//				push ecx
				mov ecx, FULLSCREEN_W
//			blt8_label2:
				rep movsb
//				pop ecx
				add edi, pitch
//				add esi, 16
				dec edx
				jnz blt8_label1
				pop es
			}
#else
			pitch-=FULLSCREEN_W;
			_asm{
				push es
				mov ax,ds
				mov es,ax
				mov edx, height
				mov esi, bmpd
				mov edi, surfp
				mov ebx, palettep
//				add esi, 0
				cld
			blt8_label1:
//				push ecx
				mov ecx, FULLSCREEN_W/4
//			blt8_label2:
				rep movsd
//				pop ecx
				add edi, pitch
//				add esi, 16
				dec edx
				jnz blt8_label1
				pop es
#endif
			}
			break;
		case 16:
			{
				WORD *palette_wp = (WORD *)malloc(2*256);
				if(palette_wp==NULL)
					break;
				pitch-=(2*FULLSCREEN_W);
				DWORD gpxelf = ddsd->ddpfPixelFormat.dwGBitMask==0x7E0;
				_asm{
					cld
					mov esi, palettep
					mov edi, palette_wp
					mov ebx, gpxelf
//					cmp ebx, 7E0h
//					jnz blt16_label_G1
//					mov bx,1
//					jmp short blt16_label_G3
//			blt16_label_G1:
//					xor ebx, ebx
//			blt16_label_G2:
//			blt16_label_G3:
					mov ecx, 100h
			blt16_label1:
					mov eax, DWORD PTR [esi]
					add esi, 4
					shr eax, 3
					xor edx, edx
					and eax, 1F1F1Fh
					mov dh, ah
					shr dx, 2
					test bx,bx
					jnz blt16_label_ng
					shr dx, 1
			blt16_label_ng:
					or  dl, al
					shr eax, 5
					or  dh, ah
					mov WORD PTR [edi], dx
					lea edi,[edi+2]
					dec ecx
					jnz blt16_label1

					mov ecx, height
					mov esi, bmpd
					mov edi, surfp
					mov ebx, palette_wp
//					add esi, 8
			blt16_label2:
					push ecx
					mov ecx, FULLSCREEN_W
			blt16_label3:
					movzx eax, BYTE PTR [esi]
					shl eax,1
					inc esi
					mov dx, WORD PTR [ebx+eax]
					mov WORD PTR [edi], dx
					add edi,2
					dec ecx
					jnz blt16_label3
					pop ecx
					add edi, pitch
//					add esi, 16
					dec ecx
					jnz blt16_label2
				}
				free(palette_wp);
			}
			break;
		case 24:
			pitch-=(3*FULLSCREEN_W);
			_asm{
				cld
				mov ecx, height
				mov esi, bmpd
				mov edi, surfp
				mov ebx, palettep
//				add esi, 8
			blt24_label1:
				push ecx
				mov ecx, FULLSCREEN_W
			blt24_label2:
				movzx eax, BYTE PTR [esi]
				inc esi
				mov dx, WORD PTR [ebx+eax*4]
				mov WORD PTR [edi], dx
				mov dl, BYTE PTR [ebx+eax*4+2]
				mov BYTE PTR [edi+2], dl
				lea edi,[edi+3]
				dec ecx
				jnz blt24_label2
				pop ecx
				add edi, pitch
//				add esi, 16
				dec ecx
				jnz blt24_label1
			}
			break;
		case 32:
			pitch-=(4*FULLSCREEN_W);
			_asm{
				cld
				mov ecx, height
				mov esi, bmpd
				mov edi, surfp
				mov ebx, palettep
//				add esi, 8
			blt32_label1:
				push ecx
				mov ecx, FULLSCREEN_W
			blt32_label2:
				movzx eax, BYTE PTR [esi]
				inc esi
				mov edx, DWORD PTR [ebx+eax*4]
				mov DWORD PTR [edi], edx
				lea edi,[edi+4]
				dec ecx
				jnz blt32_label2
				pop ecx
				add edi, pitch
//				add esi, 16
				dec ecx
				jnz blt32_label1
			}
			break;
	}
	return 1;
}




//  8bit -> 16bit (RGB 565)
int scrconv8_16for2xsai(unsigned char *src, unsigned char *dest, WORD *palette, DWORD width, DWORD height, DWORD srcPitch, DWORD destPitch){

	{
		srcPitch -= width;
		destPitch-=(2*width);
		_asm{
				cld

				mov esi, src
				mov edi, dest
/*
				mov eax, 2
				mov ecx, width
				add ecx, destPitch
				mul ecx
				add ecx, 2*2
				add edi, ecx
*/
				mov ebx, palette
				add esi, 8
				mov ecx, height
scrconv8_16_label2:
				push ecx
				mov ecx, width
scrconv8_16_label3:
				movzx eax, BYTE PTR [esi]
				shl eax,1
				inc esi
				mov dx, WORD PTR [ebx+eax]
				mov WORD PTR [edi], dx
				add edi,2
				dec ecx
				jnz scrconv8_16_label3
				pop ecx
				add edi, destPitch
				add esi, srcPitch
				dec ecx
				jnz scrconv8_16_label2
		}
	}
	return 1;
}

#if 1
//  16bit(565) -> 24bit
int scrconv16_24(unsigned char *src, unsigned char *dest, DWORD width, DWORD height, DWORD srcPitch, DWORD destPitch){
	{
		srcPitch -= (2*width);
		destPitch-=(3*width);
		_asm{
			cld
			mov esi, src
			mov edi, dest
			mov ecx, height
scrconv16_24_label1:
			push ecx
			mov ecx, width
scrconv16_24_label2:
			movzx eax, WORD PTR [esi]
			lea esi,[esi+2]
			mov edx, eax
			shl eax, 3
			shr edx, 8
			shl ah, 2
			mov WORD PTR [edi], ax
			mov BYTE PTR [edi+2], dl
			lea edi,[edi+3]
			dec ecx
			jnz scrconv16_24_label2
			pop ecx
			add esi,srcPitch
			add edi,destPitch
			dec ecx
			jnz scrconv16_24_label1
		}
	}
	return 1;
}
#endif

//  16bit(565) -> 32bit
int scrconv16_32(unsigned char *src, unsigned char *dest, DWORD width, DWORD height, DWORD srcPitch, DWORD destPitch){
	{
		srcPitch -= (2*width);
		destPitch-=(4*width);
		DWORD Dammy[2]={0,0};
		_asm{
			cld

#if 1
			mov esi, src
			mov edi, dest

			mov ecx, height
scrconv16_32_label1:
//			push ecx
			mov ebx, width
scrconv16_32_label2:
			movzx eax, WORD PTR [esi]
			lea esi,[esi+2]

			mov edx, eax
			shl eax, 3
			shl edx, 8
			shl ah, 2
			mov dx, ax

			mov DWORD PTR [edi], edx
			lea edi,[edi+4]
			dec ebx
			jnz scrconv16_32_label2
//			pop ecx
			add esi,srcPitch
			add edi,destPitch
			dec ecx
			jnz scrconv16_32_label1
#else
			mov esi, src
			mov edi, dest

			mov ecx, height
scrconv16_32_label1:
//			push ecx
			mov ebx, width
scrconv16_32_label2:
			movzx eax, WORD PTR [esi]
			lea esi,[esi+2]
			mov edx, eax
			shl eax, 3
			shl edx, 8
			shl ah, 2
			mov dx, ax
			mov DWORD PTR [edi], edx
			lea edi,[edi+4]
			dec ebx
			jnz scrconv16_32_label2
//			pop ecx
			add esi,srcPitch
			add edi,destPitch
			dec ecx
			jnz scrconv16_32_label1
#endif
		}
	}
	return 1;
}


//RGBQUAD -> 16bit(565) palette   RRRRR GGGGGG BBBBB
void convto16_565bitpalette(DWORD *spalette, WORD *dpalette){
	_asm{
			cld
			mov esi, spalette
			mov edi, dpalette
			mov ecx, 100h
convto16_565_label1:
			mov eax, DWORD PTR [esi]
			lea esi,[esi+4]
			shr eax, 3
			xor edx, edx
			and eax, 1F1F1Fh
			mov dh, ah
			shr dx, 2				//555 = 3
			or  dl, al
			shr eax, 5
			or  dh, ah
			mov WORD PTR [edi], dx
			lea edi,[edi+2]
			dec ecx
			jnz convto16_565_label1
	}
}


//RGBQUAD -> 16bit(555) palette   xRRRRR GGGGG BBBBB
void convto16_555bitpalette(DWORD *spalette, WORD *dpalette){
	_asm{
		cld
			mov esi, spalette
			mov edi, dpalette
			mov ecx, 100h
convto16_555_label1:
			mov eax, DWORD PTR [esi]
			lea esi,[esi+4]
			shr eax, 3
			xor edx, edx
			and eax, 1F1F1Fh
			mov dh, ah
			shr dx, 3				//565 = 2
			or  dl, al
			shr eax, 6				//565 = 5
			or  dh, ah
			mov WORD PTR [edi], dx
			lea edi,[edi+2]
			dec ecx
			jnz convto16_555_label1
	}
}



//  16bit(555) -> 32bit
int scrconv15_32(unsigned char *src, unsigned char *dest, DWORD width, DWORD height, DWORD srcPitch, DWORD destPitch){
	{
		srcPitch -= (2*width);
		destPitch-=(4*width);
		DWORD Dammy[2]={0,0};
		_asm{
			cld
			mov esi, src
			mov edi, dest
			mov ecx, height
scrconv15_32_label1:
//			push ecx
			mov ebx, width
scrconv15_32_label2:
			movzx eax, WORD PTR [esi]
			lea esi,[esi+2]

// xRRRRR GGGGG BBBBB
// 
			mov edx, eax
			shl eax, 3		// AL = B
			shl edx, 9		// EDX = "??RR????" H
			shl ah, 3		// AH = G
			mov dx, ax

			mov DWORD PTR [edi], edx
			lea edi,[edi+4]
			dec ebx
			jnz scrconv15_32_label2
//			pop ecx
			add esi,srcPitch
			add edi,destPitch
			dec ecx
			jnz scrconv15_32_label1
		}
	}
	return 1;
}

