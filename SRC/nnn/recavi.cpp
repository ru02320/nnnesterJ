
#include <windows.h>
#include <windowsx.h> 

#include <commctrl.h>
#include <commdlg.h>
#include <winuser.h>


#include <vfw.h>

#include "recavi.h"

extern unsigned char NES_preset_palette[64][3];

AVISTREAMINFO si;
BITMAPINFO *bmih;
PAVIFILE pavi;
PAVISTREAM pstm,ptmp;
unsigned long sizeimage, imagecounter, s=0;
//unsigned char *databuffer=NULL;
AVICOMPRESSOPTIONS *p_ptp, opt;
COMPVARS cv;


int recavifilestart(char *fn, int w, int h){
	RGBQUAD *paldata=NULL;

	p_ptp=&opt;

	ZeroMemory(&si, sizeof(AVISTREAMINFO));
	si.fccType = streamtypeVIDEO;
	si.fccHandler = comptypeDIB;
	si.dwScale = 1;
	si.dwRate = 30;
	si.dwLength = 10;
	si.dwQuality = (DWORD)-1;
	si.rcFrame.top = 0;
	si.rcFrame.left = 0;
	si.rcFrame.right = w;
	si.rcFrame.bottom = h;
	si.dwSuggestedBufferSize = w*h;
	strcpy(si.szName, "Video #1");

	bmih = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
	if(bmih==NULL)
		return 0;
	imagecounter=0;
	sizeimage = w*h;
//-	databuffer = (unsigned char *)malloc(sizeimage * 3);
//	paldata = (RGBQUAD *)malloc((256 * sizeof(RGBQUAD)));
//-	if(databuffer==NULL)
//-		return 0;
	memset(&bmih->bmiHeader, 0, sizeof(BITMAPINFOHEADER));
	bmih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmih->bmiHeader.biWidth           = w; 
    bmih->bmiHeader.biHeight          = h; 
    bmih->bmiHeader.biPlanes          = 0x0001; 
    bmih->bmiHeader.biBitCount        = 8;
    bmih->bmiHeader.biCompression     = BI_RGB;   
    bmih->bmiHeader.biSizeImage       = sizeimage;
    bmih->bmiHeader.biXPelsPerMeter   = 0;
    bmih->bmiHeader.biYPelsPerMeter   = 0;
    bmih->bmiHeader.biClrUsed         = 0;
    bmih->bmiHeader.biClrImportant    = 0;
//	memcpy(&bmih[sizeof(BITMAPINFOHEADER)], pal, (256 * sizeof(RGBQUAD)));

	paldata = bmih->bmiColors;
	memset(paldata,0,sizeof(256* sizeof(RGBQUAD)));
	for(int i = 64; i<128; i++){
		paldata[i].rgbRed = NES_preset_palette[i-64][0];
		paldata[i].rgbGreen = NES_preset_palette[i-64][1];
		paldata[i].rgbBlue = NES_preset_palette[i-64][2];
	}

//	memcpy(paldata, pal, (256 * sizeof(RGBQUAD)));

	memset(&cv,0,sizeof(COMPVARS));
	cv.cbSize=sizeof(COMPVARS);
	cv.dwFlags=ICMF_COMPVARS_VALID;
	cv.fccHandler=comptypeDIB;
	cv.lQ=ICQUALITY_DEFAULT;
	AVIFileInit();

	if (!ICCompressorChoose(NULL,ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME /*| ICMF_CHOOSE_ALLCOMPRESSORS*/,&bmih->bmiHeader,NULL,&cv,NULL))
		return 0;
	si.fccHandler=cv.fccHandler;
//	si.fccType = cv.fccType;
/*
	si.dwRate = cv.hic;
	si.dwLength = 10;
	si.dwQuality = (DWORD)-1;
*/
	opt.fccType=streamtypeVIDEO; //
	opt.fccHandler=cv.fccHandler;
	opt.dwKeyFrameEvery=cv.lKey;
	opt.dwQuality=cv.lQ;
	opt.dwBytesPerSecond=cv.lDataRate;
	opt.dwFlags=(cv.lDataRate>0?AVICOMPRESSF_DATARATE:0)
										|(cv.lKey>0?AVICOMPRESSF_KEYFRAMES:0);
	opt.lpFormat=NULL;
	opt.cbFormat=0;
	opt.lpParms=cv.lpState;
	opt.cbParms=cv.cbState;
	opt.dwInterleaveEvery=0;

	if(AVIFileOpen(&pavi, fn,OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE,NULL)!=0)
		return 0;
	if(AVIFileCreateStream(pavi,&pstm,&si)!=0)
		return 0;
//	if(!AVISaveOptions(NULL, 0, 1, &pstm, &p_ptp))
//		return 0;
	if(AVIMakeCompressedStream(&ptmp,pstm,&opt,NULL)!=AVIERR_OK)
		return 0;
	if(AVIStreamSetFormat(ptmp, 0, &bmih->bmiHeader, sizeof(BITMAPINFOHEADER)+ (256 * sizeof(RGBQUAD))) !=0)
		return 0;
	return 1;
}



void recavifilerelease(){
//	AVISaveOptionsFree(s,popt);
	AVIStreamRelease(ptmp);
	AVIStreamRelease(pstm);
	AVIFileRelease(pavi);
	ICCompressorFree(&cv);
	AVIFileExit();
/*
	if(databuffer){
		free(databuffer);
		databuffer = NULL;
	}
*/
	if(bmih){
		free(bmih);
		bmih = NULL;
	}
}


void recavifilewriteflame(unsigned char *ip){
//	int i, x=w>>1, d;
//	unsigned char *p=databuffer;
/*
	for(i=0; i<h; ++i){
		for(int j=0; j<x; ++j){
			d = ip[i*w+j];
			*p++=paldata[d].rgbBlue;
			*p++=paldata[d].rgbGreen;
			*p++=paldata[d].rgbRed;
		}
	}
*/
	if(AVIStreamWrite(ptmp, imagecounter, 1, ip, sizeimage, AVIIF_KEYFRAME, NULL, NULL)!=0)
		return;
	++imagecounter;
	return;
}
