

#include <windows.h>

#include "types.h"
#include "debug.h"
#include "scale2x.h"




#ifdef __cplusplus
extern "C" {
#endif

void scale2x_8_mmx(scale2x_uint8* dst0, scale2x_uint8* dst1, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count);
void scale2x_16_mmx(scale2x_uint16* dst0, scale2x_uint16* dst1, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count);
void scale2x_32_mmx(scale2x_uint32* dst0, scale2x_uint32* dst1, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count);
	
#ifdef __cplusplus
}
#endif




//#define MMX


#ifdef MMX
EXTERN_C void _2xSaILine  (uint8 *srcPtr, uint8 *deltaPtr, uint32 srcPitch, uint32 width,
                        uint8 *dstPtr, uint32 dstPitch);
EXTERN_C void _2xSaISuperEagleLine  (uint8 *srcPtr, uint8 *deltaPtr, uint32 srcPitch, uint32 width,
                        uint8 *dstPtr, uint32 dstPitch);
EXTERN_C int Init_2xSaIMMX (uint32 BitFormat);
#endif

bool mmx_cpu = false;

static uint32 colorMask = 0xF7DEF7DE;
static uint32 lowPixelMask = 0x08210821;
static uint32 qcolorMask = 0xE79CE79C;
static uint32 qlowpixelMask = 0x18631863;


//static uint8 Dammybuff[256*256];


int Init_2xSaI(uint32 BitFormat)
{
	if (BitFormat == 565)
	{
		colorMask = 0xF7DEF7DE;
		lowPixelMask = 0x08210821;
		qcolorMask = 0xE79CE79C;
		qlowpixelMask = 0x18631863;
	}
	else
		if (BitFormat == 555)
		{
			colorMask = 0x7BDE7BDE;
			lowPixelMask = 0x04210421;
			qcolorMask = 0x739C739C;
			qlowpixelMask = 0x0C630C63;
		}
		else
		{
			return 0;
		}
#ifdef MMX
	Init_2xSaIMMX(BitFormat);
#endif
	return 1;
}

inline int GetResult1(uint32 A, uint32 B, uint32 C, uint32 D, uint32 E)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C) x+=1; else if (B == C) y+=1;
	if (A == D) x+=1; else if (B == D) y+=1;
	if (x <= 1) r+=1;
	if (y <= 1) r-=1;
	return r;
}

inline int GetResult2(uint32 A, uint32 B, uint32 C, uint32 D, uint32 E)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C) x+=1; else if (B == C) y+=1;
	if (A == D) x+=1; else if (B == D) y+=1;
	if (x <= 1) r-=1;
	if (y <= 1) r+=1;
	return r;
}


inline int GetResult(uint32 A, uint32 B, uint32 C, uint32 D)
{
	int x = 0;
	int y = 0;
	int r = 0;
	if (A == C) x+=1; else if (B == C) y+=1;
	if (A == D) x+=1; else if (B == D) y+=1;
	if (x <= 1) r+=1;
	if (y <= 1) r-=1;
	return r;
}


inline uint32 INTERPOLATE(uint32 A, uint32 B)
{
	if (A !=B)
	{
		return ( ((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask) );
	}
	else return A;
}


inline uint32 Q_INTERPOLATE(uint32 A, uint32 B, uint32 C, uint32 D)
{
	register uint32 x = ((A & qcolorMask) >> 2) +
	                    ((B & qcolorMask) >> 2) +
	                    ((C & qcolorMask) >> 2) +
	                    ((D & qcolorMask) >> 2);
	register uint32 y = (A & qlowpixelMask) +
	                    (B & qlowpixelMask) +
	                    (C & qlowpixelMask) +
	                    (D & qlowpixelMask);
	y = (y>>2) & qlowpixelMask;
	return x+y;
}




#define HOR
#define VER
void Super2xSaI(uint8 *srcPtr, uint32 srcPitch,
                uint8 *deltaPtr,
                uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
	uint32 *dP;
	uint16 *bP;

#ifdef MMX_BLA  //no MMX version yet
	if (cpu_mmx && width != 512)
	{
		for (height; height; height-=1)
		{
			bP = (uint16 *) srcPtr;
			xP = (uint16 *) deltaPtr;
			dP = (uint32 *) dstPtr;
			_2xSaISuperEagleLine  ((uint8 *) bP, (uint8 *) xP, srcPitch, width, (uint8 *) dP, dstPitch);
			dstPtr += dstPitch << 1;
			srcPtr += srcPitch;
			deltaPtr += srcPitch;
		}
	}
	else
	{
#endif
		uint32 Nextline = srcPitch >> 1;

		for (height; height; height-=1)
		{
			bP = (uint16 *) srcPtr;
			dP = (uint32 *) dstPtr;
			for (uint32 finish = width; finish; finish -= 1 )
			{
				uint32 color4, color5, color6;
				uint32 color1, color2, color3;
				uint32 colorA0, colorA1, colorA2, colorA3,
				colorB0, colorB1, colorB2, colorB3,
				colorS1, colorS2;
				uint32 product1a, product1b,
				product2a, product2b;

				//---------------------------------------    B1 B2
				//                                         4  5  6 S2
				//                                         1  2  3 S1
				//                                           A1 A2

				colorB0 = *(bP- Nextline - 1);
				colorB1 = *(bP- Nextline);
				colorB2 = *(bP- Nextline + 1);
				colorB3 = *(bP- Nextline + 2);

				color4 = *(bP - 1);
				color5 = *(bP);
				color6 = *(bP + 1);
				colorS2 = *(bP + 2);

				color1 = *(bP + Nextline - 1);
				color2 = *(bP + Nextline);
				color3 = *(bP + Nextline + 1);
				colorS1 = *(bP + Nextline + 2);

				colorA0 = *(bP + Nextline + Nextline - 1);
				colorA1 = *(bP + Nextline + Nextline);
				colorA2 = *(bP + Nextline + Nextline + 1);
				colorA3 = *(bP + Nextline + Nextline + 2);


				//--------------------------------------
				if (color2 == color6 && color5 != color3)
				{
					product2b = product1b = color2;
				}
				else
					if (color5 == color3 && color2 != color6)
					{
						product2b = product1b = color5;
					}
					else
						if (color5 == color3 && color2 == color6 && color5 != color6)
						{
							register int r = 0;

							r += GetResult (color6, color5, color1, colorA1);
							r += GetResult (color6, color5, color4, colorB1);
							r += GetResult (color6, color5, colorA2, colorS1);
							r += GetResult (color6, color5, colorB2, colorS2);

							if (r > 0)
								product2b = product1b = color6;
							else
								if (r < 0)
									product2b = product1b = color5;
								else
								{
									product2b = product1b = INTERPOLATE (color5, color6);
								}

						}
						else
						{

#ifdef VER
							if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
								product2b = Q_INTERPOLATE (color3, color3, color3, color2);
							else
								if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
									product2b = Q_INTERPOLATE (color2, color2, color2, color3);
								else
#endif
									product2b = INTERPOLATE (color2, color3);

#ifdef VER
							if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
								product1b = Q_INTERPOLATE (color6, color6, color6, color5);
							else
								if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
									product1b = Q_INTERPOLATE (color6, color5, color5, color5);
								else
#endif
									product1b = INTERPOLATE (color5, color6);
						}

#ifdef HOR
				if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
					product2a = INTERPOLATE (color2, color5);
				else
					if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
						product2a = INTERPOLATE(color2, color5);
					else
#endif
						product2a = color2;

#ifdef HOR
				if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
					product1a = INTERPOLATE (color2, color5);
				else
					if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
						product1a = INTERPOLATE(color2, color5);
					else
#endif
						product1a = color5;


				product1a = product1a | (product1b << 16);
				product2a = product2a | (product2b << 16);

				*(dP) = product1a;
				*(dP+(dstPitch>>2)) = product2a;

				bP += 1;
				dP += 1;
			}//end of for ( finish= width etc..)

			dstPtr += dstPitch << 1;
			srcPtr += srcPitch;
			deltaPtr += srcPitch;
		}; //endof: for (height; height; height--)
#ifdef MMX_BLA
	}
#endif
}







/*ONLY use with 640x480x16 or higher resolutions*/
/*Only use this if 2*width * 2*height fits on the current screen*/
void SuperEagle(uint8 *srcPtr, uint32 srcPitch,
                uint8 *deltaPtr,
                uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
	uint32 *dP;
	uint16 *bP;
	uint16 *xP;

#ifdef MMX
	if (mmx_cpu && width != 512)
	{
		for (height; height; height-=1)
		{
			bP = (uint16 *) srcPtr;
			xP = (uint16 *) deltaPtr;
			dP = (uint32 *) dstPtr;
			_2xSaISuperEagleLine  ((uint8 *) bP, (uint8 *) xP, srcPitch, width, (uint8 *)dP, dstPitch);
			dstPtr += dstPitch << 1;
			srcPtr += srcPitch;
			deltaPtr += srcPitch;
		}
	}
	else
	{
#endif
		uint32 Nextline = srcPitch >> 1;

		for (height; height; height-=1)
		{
			bP = (uint16 *) srcPtr;
			dP = (uint32 *) dstPtr;
			for (uint32 finish = width; finish; finish -= 1 )
			{

				uint32 color4, color5, color6;
				uint32 color1, color2, color3;
				uint32 colorA0, colorA1, colorA2, colorA3,
				colorB0, colorB1, colorB2, colorB3,
				colorS1, colorS2;
				uint32 product1a, product1b,
				product2a, product2b;

				colorB0 = *(bP- Nextline - 1);
				colorB1 = *(bP- Nextline);
				colorB2 = *(bP- Nextline + 1);
				colorB3 = *(bP- Nextline + 2);

				color4 = *(bP - 1);
				color5 = *(bP);
				color6 = *(bP + 1);
				colorS2 = *(bP + 2);

				color1 = *(bP + Nextline - 1);
				color2 = *(bP + Nextline);
				color3 = *(bP + Nextline + 1);
				colorS1 = *(bP + Nextline + 2);

				colorA0 = *(bP + Nextline + Nextline - 1);
				colorA1 = *(bP + Nextline + Nextline);
				colorA2 = *(bP + Nextline + Nextline + 1);
				colorA3 = *(bP + Nextline + Nextline + 2);


				//--------------------------------------
				if (color2 == color6 && color5 != color3)
				{
					product1b = product2a = color2;
					if ((color1 == color2 && color6 == colorS2) ||
					        (color2 == colorA1 && color6 == colorB2))
					{
						product1a = INTERPOLATE (color2, color5);
						product1a = INTERPOLATE (color2, product1a);
						product2b = INTERPOLATE (color2, color3);
						product2b = INTERPOLATE (color2, product2b);
						//                       product1a = color2;
						//                       product2b = color2;
					}
					else
					{
						product1a = INTERPOLATE (color5, color6);
						product2b = INTERPOLATE (color2, color3);
					}
				}
				else
					if (color5 == color3 && color2 != color6)
					{
						product2b = product1a = color5;
						if ((colorB1 == color5 && color3 == colorA2) ||
						        (color4 == color5 && color3 == colorS1))
						{
							product1b = INTERPOLATE (color5, color6);
							product1b = INTERPOLATE (color5, product1b);
							product2a = INTERPOLATE (color5, color2);
							product2a = INTERPOLATE (color5, product2a);
							//                       product1b = color5;
							//                       product2a = color5;
						}
						else
						{
							product1b = INTERPOLATE (color5, color6);
							product2a = INTERPOLATE (color2, color3);
						}
					}
					else
						if (color5 == color3 && color2 == color6 && color5 != color6)
						{
							register int r = 0;

							r += GetResult (color6, color5, color1, colorA1);
							r += GetResult (color6, color5, color4, colorB1);
							r += GetResult (color6, color5, colorA2, colorS1);
							r += GetResult (color6, color5, colorB2, colorS2);

							if (r > 0)
							{
								product1b = product2a = color2;
								product1a = product2b = INTERPOLATE (color5, color6);
							}
							else
								if (r < 0)
								{
									product2b = product1a = color5;
									product1b = product2a = INTERPOLATE (color5, color6);
								}
								else
								{
									product2b = product1a = color5;
									product1b = product2a = color2;
								}
						}
						else
						{

							if ((color2 == color5) || (color3 == color6))
							{
								product1a = color5;
								product2a = color2;
								product1b = color6;
								product2b = color3;

							}
							else
							{
								product1b = product1a = INTERPOLATE (color5, color6);
								product1a = INTERPOLATE (color5, product1a);
								product1b = INTERPOLATE (color6, product1b);

								product2a = product2b = INTERPOLATE (color2, color3);
								product2a = INTERPOLATE (color2, product2a);
								product2b = INTERPOLATE (color3, product2b);
							}
						}


				product1a = product1a | (product1b << 16);
				product2a = product2a | (product2b << 16);

				*(dP) = product1a;
				*(dP+(dstPitch>>2)) = product2a;

				bP += 1;
				dP += 1;
			}//end of for ( finish= width etc..)

			dstPtr += dstPitch << 1;
			srcPtr += srcPitch;
			deltaPtr += srcPitch;
		}; //endof: for (height; height; height--)
#ifdef MMX
	}
#endif
}




/*ONLY use with 640x480x16 or higher resolutions*/
/*Only use this if 2*width * 2*height fits on the current screen*/
void _2xSaI(uint8 *srcPtr, uint32 srcPitch,
            uint8 *deltaPtr,
            uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
	uint32 *dP;
	uint16 *bP;
	uint16 *xP;

//	uint32 dgb1,dgb2,dgb3,dgb4,dgb5;
//	dgb1=dgb2=dgb3=dgb4=dgb5=0;

#ifdef MMX
	if (mmx_cpu && width != 512)
	{
		for (height; height; height-=1)
		{

			bP = (uint16 *) srcPtr;
			xP = (uint16 *) deltaPtr; //deltaPtr;
			dP = (uint32 *) dstPtr;
			_2xSaILine  ((uint8 *) bP, (uint8 *) xP, srcPitch, width, (uint8 *)dP, dstPitch);
			dstPtr += dstPitch << 1;
			srcPtr += srcPitch;
			deltaPtr += srcPitch;
		}
	}
	else
	{
#endif
		uint32 Nextline = srcPitch >> 1;

		for (height; height; height-=1)
		{
			bP = (uint16 *) srcPtr;
			dP = (uint32 *) dstPtr;
			for (uint32 finish = width; finish; finish -= 1 )
			{


				register uint32 colorA, colorB;
				uint32 colorC, colorD,
				colorE, colorF, colorG, colorH,
				colorI, colorJ, colorK, colorL,
				colorM, colorN, colorO, colorP;
				uint32 product, product1, product2;


				//---------------------------------------
				// Map of the pixels:                    I|E F|J
				//                                       G|A B|K
				//                                       H|C D|L
				//                                       M|N O|P
				colorI = *(bP- Nextline - 1);
				colorE = *(bP- Nextline);
				colorF = *(bP- Nextline + 1);
				colorJ = *(bP- Nextline + 2);

				colorG = *(bP - 1);
				colorA = *(bP);
				colorB = *(bP + 1);
				colorK = *(bP + 2);

				colorH = *(bP + Nextline - 1);
				colorC = *(bP + Nextline);
				colorD = *(bP + Nextline + 1);
				colorL = *(bP + Nextline + 2);

				colorM = *(bP + Nextline + Nextline - 1);
				colorN = *(bP + Nextline + Nextline);
				colorO = *(bP + Nextline + Nextline + 1);
				colorP = *(bP + Nextline + Nextline + 2);

				if ((colorA == colorD) && (colorB != colorC))
				{
					if ( ((colorA == colorE) && (colorB == colorL)) ||
					        ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ)) )
					{
						product = colorA;
					}
					else
					{
						product = INTERPOLATE(colorA, colorB);
					}

					if (((colorA == colorG) && (colorC == colorO)) ||
					        ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM)) )
					{
						product1 = colorA;
					}
					else
					{
						product1 = INTERPOLATE(colorA, colorC);
					}
					product2 = colorA;
				}
				else
					if ((colorB == colorC) && (colorA != colorD))
					{
						if (((colorB == colorF) && (colorA == colorH)) ||
						        ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)) )
						{
							product = colorB;
						}
						else
						{
							product = INTERPOLATE(colorA, colorB);
						}

						if (((colorC == colorH) && (colorA == colorF)) ||
						        ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)) )
						{
							product1 = colorC;
						}
						else
						{
							product1 = INTERPOLATE(colorA, colorC);
						}
						product2 = colorB;
					}
					else
						if ((colorA == colorD) && (colorB == colorC))
						{
							if (colorA == colorB)
							{
								product = colorA;
								product1 = colorA;
								product2 = colorA;
							}
							else
							{
								register int r = 0;
								product1 = INTERPOLATE(colorA, colorC);
								product = INTERPOLATE(colorA, colorB);

								r += GetResult1 (colorA, colorB, colorG, colorE, colorI);
								r += GetResult2 (colorB, colorA, colorK, colorF, colorJ);
								r += GetResult2 (colorB, colorA, colorH, colorN, colorM);
								r += GetResult1 (colorA, colorB, colorL, colorO, colorP);

								if (r > 0)
									product2 = colorA;
								else
									if (r < 0)
										product2 = colorB;
									else
									{
										product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
									}

								product2 = colorB;
							}
						}
						else
						{
							product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);

							if ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ))
							{
								product = colorA;
							}
							else
								if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI))
								{
									product = colorB;
								}
								else
								{
									product = INTERPOLATE(colorA, colorB);
								}

							if ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM))
							{
								product1 = colorA;
							}
							else
								if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI))
								{
									product1 = colorC;
								}
								else
								{
									product1 = INTERPOLATE(colorA, colorC);
								}
						}
				product = colorA | (product << 16);
				product1 = product1 | (product2 << 16);
				*(dP) = product;
				*(dP+(dstPitch>>2)) = product1;

				bP += 1;
				dP += 1;
			}//end of for ( finish= width etc..)

			dstPtr += dstPitch << 1;
			srcPtr += srcPitch;
			deltaPtr += srcPitch;
		}; //endof: for (height; height; height--)
#ifdef MMX
	}
#endif
//    LOG("1 =" << HEX(dgb1) << " , 2 = " << HEX(dgb2) << " , 3 = " << HEX(dgb3) << " , 4 = " << HEX(dgb4)<< " , 5 = " << HEX(dgb5) <<endl);
}




void New2xSaI_8bit(uint8 *srcPtr, uint32 srcPitch,	uint8 *deltaPtr,
		uint8 *dstPtr, uint32 dstPitch, int width, int height){
	int y, dstPitch2x;
	unsigned char *src0, *src1, *src2, *dest0, *dest1;

	src0 = srcPtr-srcPitch;
	src1 = srcPtr;
	src2 = srcPtr+srcPitch;

	dest0 = dstPtr;
	dest1 = dstPtr + dstPitch;
	dstPitch2x = dstPitch << 1;
	for(y=0; y < height; y++){
		scale2x_8_def(dest0, dest1, (const scale2x_uint8*)src0, (const scale2x_uint8*)src1, (const scale2x_uint8*)src2, width);
		src0 += srcPitch;
		src1 += srcPitch;
		src2 += srcPitch;
		dest0 += dstPitch2x;
		dest1 += dstPitch2x;
	}
}


void New2xSaI_8bit_mmx(uint8 *srcPtr, uint32 srcPitch,	uint8 *deltaPtr,
		uint8 *dstPtr, uint32 dstPitch, int width, int height){
	int y, dstPitch2x;
	unsigned char *src0, *src1, *src2, *dest0, *dest1;

	src0 = srcPtr-srcPitch;
	src1 = srcPtr;
	src2 = srcPtr+srcPitch;

	dest0 = dstPtr;
	dest1 = dstPtr + dstPitch;
	dstPitch2x = dstPitch << 1;
	for(y=0; y < height; y++){
		scale2x_8_mmx(dest0, dest1, (const scale2x_uint8*)src0, (const scale2x_uint8*)src1, (const scale2x_uint8*)src2, width);
		src0 += srcPitch;
		src1 += srcPitch;
		src2 += srcPitch;
		dest0 += dstPitch2x;
		dest1 += dstPitch2x;
	}
}




void New2xSaI_16bit(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
		uint8 *dstPtr, uint32 dstPitch, int width, int height){
	int y, dstPitch2x;
	scale2x_uint16 *src0, *src1, *src2, *dest0, *dest1;

	src0 = (scale2x_uint16 *)srcPtr-srcPitch;
	src1 = (scale2x_uint16 *)srcPtr;
	src2 = (scale2x_uint16 *)srcPtr+srcPitch;

	dest0 = (scale2x_uint16 *)dstPtr;
	dest1 = (scale2x_uint16 *)dstPtr + dstPitch;
	dstPitch2x = dstPitch << 1;
	for(y=0; y < height; y++){
		scale2x_16_def(dest0, dest1, src0, src1, src2, width);
		src0 += srcPitch;
		src1 += srcPitch;
		src2 += srcPitch;
		dest0 += dstPitch2x;
		dest1 += dstPitch2x;
	}
}




