#include <windows.h>
#include <windowsx.h> 

#include "settings.h"
#include "png.h"


BOOL screenshot_save_8bit(char *szFileName, int width, int height, PALETTEENTRY *pal, unsigned char *bmpd){
  if(NESTER_settings.nes.graphics.ScreenShotUseBmp){
	  FILE *fp;
	  if(!(fp = fopen( szFileName, "wb" )))
		  return FALSE;

	  BITMAPINFOHEADER bmi;
	  bmi.biSize            = sizeof(BITMAPINFOHEADER);
	  bmi.biWidth           = width;	
	  bmi.biHeight          = height; 
	  bmi.biPlanes          = 0x0001; 
	  bmi.biBitCount        = 0x0008;
	  bmi.biCompression     = BI_RGB;   
	  bmi.biSizeImage       = width * height;
	  bmi.biXPelsPerMeter   = 0x00000060; 
	  bmi.biYPelsPerMeter   = 0x00000060;
	  bmi.biClrUsed         = 0x00000100; 
	  bmi.biClrImportant    = 0x00000100; 

	  BITMAPFILEHEADER bmf;
	  bmf.bfType      = *(WORD*)"BM";
	  bmf.bfSize      = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 0x400 + width * height;
	  bmf.bfReserved1 = 0x0000;
	  bmf.bfReserved2 = 0x0000;
	  bmf.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 0x400;
	  fwrite( &bmf, sizeof(BITMAPFILEHEADER), 1, fp );
	  fwrite( &bmi, sizeof(BITMAPINFOHEADER), 1, fp );
	  int i,j;
	  for(i = 0; i < 256; i++ )
	  {
		  fputc( (char)(pal[i].peBlue ), fp );
		  fputc( (char)(pal[i].peGreen), fp );
		  fputc( (char)(pal[i].peRed  ), fp );
		  fputc( 0x00, fp );
	  }
	  for(i=0, j=(height-1)*width; i<height; i++, j-=width){
		  fwrite(&bmpd[j], width, 1, fp);
	  }
	  fclose(fp);
  }
  else{
	  FILE *fp;
	  if(!(fp = fopen( szFileName, "wb")))
		  return FALSE;

	  int i, j;
	  png_structp png_ptr;
	  png_infop info_ptr;
	  unsigned char **imagep;
	  png_color_struct pngplt[256];

	  png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
	  if( png_ptr==NULL ){
		  return FALSE;
	  }
	  info_ptr = png_create_info_struct( png_ptr );
	  if( info_ptr==NULL ){
		  png_destroy_write_struct( &png_ptr, NULL );
		  return FALSE;
	  }
	  if(setjmp(png_ptr->jmpbuf)){
		  png_destroy_write_struct(&png_ptr, NULL);
		  return FALSE;
	  }

	  imagep = (png_bytepp)malloc(height * sizeof(png_bytep));
	  for(i=0, j=0; i<height; ++i, j+=width){
		  imagep[i] = &bmpd[j];
	  }

	  for(i=0, j; i<0x100; ++i){
		  pngplt[i].red= pal[i].peRed;
		  pngplt[i].green= pal[i].peGreen;
		  pngplt[i].blue= pal[i].peBlue;
	  }
	  png_init_io( png_ptr, fp );
	  png_set_compression_level( png_ptr, 6);
	  png_set_PLTE( png_ptr, info_ptr, pngplt, 256);
	  png_set_IHDR( png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_NONE );
	  png_write_info( png_ptr, info_ptr );
	  png_write_image( png_ptr, imagep);
	  png_write_end( png_ptr, info_ptr );
	  png_destroy_write_struct( &png_ptr, &info_ptr );
	  fclose(fp);
	  free(imagep);
  }
  return TRUE;
}