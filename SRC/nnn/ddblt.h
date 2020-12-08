

//int ddrawblt_8bit(DDSURFACEDESC2 *ddsd, DWORD *palette, unsigned char *bmpd, DWORD width, DWORD height, DWORD dPitch);


int scrconv8_16for2xsai(unsigned char *src, unsigned char *dest, WORD *palette, DWORD width, DWORD height, DWORD srcPitch, DWORD destPitch);
void convto16_565bitpalette(DWORD *spalette, WORD *dpalette);
void convto16_555bitpalette(DWORD *spalette, WORD *dpalette);
int scrconv16_24(unsigned char *src, unsigned char *dest, DWORD width, DWORD height, DWORD srcPitch, DWORD destPitch);
int scrconv16_32(unsigned char *src, unsigned char *dest, DWORD width, DWORD height, DWORD srcPitch, DWORD destPitch);
int scrconv15_32(unsigned char *src, unsigned char *dest, DWORD width, DWORD height, DWORD srcPitch, DWORD destPitch);


int Init_2xSaI(uint32 BitFormat);
void Super2xSaI(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void SuperEagle(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void _2xSaI(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void nnn_2xSaI(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void New2xSaI_8bit(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void New2xSaI_8bit_mmx(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void New2xSaI_16bit(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);


