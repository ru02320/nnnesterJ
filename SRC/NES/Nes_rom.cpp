/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
** 
** 圧縮ファイル対応のため、みかみかな によって、Win32 APIの File IO に
** 書きかえられています。
**
** 武田によってFAM,FDS形式の読み込みに関するコードが追加されています。
*/

#include <windows.h>
#include <shlwapi.h>
#include "NES_ROM.h"
#include "arc.h"
#include "debug.h"
#include "crc32.h"
#include "resource.h"


extern void LoadIPSPatch(unsigned char *, char *);
extern int MyStrAtoh(char *, DWORD *);
extern HWND main_window_handle;
extern int MystrFnIcut(char *);
extern HINSTANCE g_main_instance;

#define CopyMemIncPtr(o,i,s) \
{\
	memcpy(o,i,s);\
	i+=s;\
}

NES_ROM::NES_ROM(const char* fn)
{
	HANDLE hf = NULL;
	unsigned char *buf = NULL;
	unsigned char *p = NULL;

	trainer    = NULL;
	ROM_banks  = NULL;
	VROM_banks = NULL;

	rom_name = NULL;
	rom_path = NULL;
	crc = 0;
	crc_all = 0;
	fds = 0;
	unif_mapper=0;
	GameTitle[0] =0;
	char str[64];
	dbcorrect[0]=dbcorrect[1]=dbcorrect[2]=0;

	uint8 image_type = 0;
	const char *nesExtensions[] = { "*.nes","*.fds","*.fam","*.unf","*.nsf",NULL };

	try
	{
		// store filename and path
		rom_name = (char*)malloc(strlen(fn)+1);
		rom_path = (char*)malloc(strlen(fn)+1);
		if( !rom_name || !rom_path)
			throw "Error loading file: out of memory";

		GetPathInfo( fn );

		hf = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		         FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if( hf == INVALID_HANDLE_VALUE ) throw "Error opening file";

		int filesize, readsize;
		filesize = GetFileSize( hf, NULL );
		if( filesize == -1 ) throw "Error opening file";
		if( sizeof(NES_header) > filesize )
			throw "Error reading from file";
		if( !( buf = p = (unsigned char*)malloc(sizeof(NES_header)) ) )
			throw "Error loading file: out of memory";
		if( !ReadFile( hf, p, sizeof(NES_header), (DWORD*)&readsize, NULL ) ||
		        ( readsize != sizeof(NES_header) ) )
			throw "Error reading from file";
		CopyMemIncPtr( &header, p, sizeof(NES_header) );
		if( ( !strncmp( (char*)header.id, "NES", 3) && ( header.ctrl_z == 0x1A) ) ||
		        ( !strncmp( (char*)header.id, "NEZ", 3) && ( header.ctrl_z == 0x1A) ) ||
		        ( !strncmp( (char*)header.id, "FDS", 3) && ( header.ctrl_z == 0x1A) ) ||
		        ( header.id[0] <= 0x1A && header.id[1] == 0x00 && (header.id[2] || header.id[3] || header.ctrl_z)&& header.num_8k_vrom_banks == 0x00 ) ||
		        ( !strncmp( (char*)header.id, "NES", 3) && (header.ctrl_z == 'M'  ) ||
		        ( !strncmp( (char*)header.id, "UNIF", 4)))
		  )
		{
			if(!(buf = (unsigned char*)realloc(buf, filesize)))
				throw "Error loading file: out of memory";
			p=buf+sizeof(NES_header);
			if( !ReadFile( hf, p, filesize-sizeof(NES_header), (DWORD*)&readsize, NULL) ||
			        ( ( readsize += sizeof(NES_header) ) != filesize ) )
				throw "Error Reading from file";
			CloseHandle( hf );
			hf = NULL;
		}
		else
		{
			CloseHandle( hf );
			hf = NULL;
			if(buf){
				free(buf);
				buf=NULL;
			}
			if( !Uncompress( NULL, &buf, fn, (LPDWORD)&filesize, nesExtensions) ){
//				LoadString(g_main_instance, IDS_STRING_ERRM_02 , str, 64);
				throw 0;
//				throw str;
			}
			p = buf;
			if( p+sizeof(NES_header)-buf > filesize ){
				LoadString(g_main_instance, IDS_STRING_ERRM_03 , str, 64);
				throw str;
			}
			CopyMemIncPtr( &header, p, sizeof(NES_header) );
		}

		if( !strncmp( (char*)header.id, "NES", 3) && ( header.ctrl_z == 0x1A) ||
		        !strncmp( (char*)header.id, "NEZ", 3) && ( header.ctrl_z == 0x1A) )
		{
			if(!NESTER_settings.nes.preferences.DisableIPSPatch)
				LoadIPSPatch((unsigned char *)p, (char *)fn);		//IPS PATCH
			// allocate memory
			ROM_banks = (uint8*)malloc(header.num_16k_rom_banks * (16*1024));
			if( !ROM_banks ) throw "Out of memory";

			VROM_banks = (uint8*)malloc(header.num_8k_vrom_banks * (8*1024));
			if( !VROM_banks ) throw "Out of memory";

			// load trainer if present
			if( has_trainer() )
			{
				if( !( trainer = (uint8*)malloc(TRAINER_LEN) ) )
					throw "Out of memory";
				if( p+TRAINER_LEN-buf > filesize ){
					LoadString(g_main_instance, IDS_STRING_ERRM_04 , str, 64);
					throw str;
				}
				CopyMemIncPtr( trainer, p, TRAINER_LEN );
				crc = CrcCalc(trainer, TRAINER_LEN);
			}

			if( p + (16*1024) * header.num_16k_rom_banks - buf > filesize ){
				LoadString(g_main_instance, IDS_STRING_ERRM_05 , str, 64);
				throw str;
			}
			CopyMemIncPtr( ROM_banks, p, (16*1024) * header.num_16k_rom_banks );

			if( p + (8*1024) * header.num_8k_vrom_banks - buf > filesize ){
				LoadString(g_main_instance, IDS_STRING_ERRM_06 , str, 64);
				throw str;
			}
			CopyMemIncPtr( VROM_banks, p, (8*1024) * header.num_8k_vrom_banks );
			if(((header.flags_1 >> 4) | (header.flags_2 & 0xF0)) == 20)
			{
				image_type = 1;
				// convert NES disk image
				uint32 i;
				uint8 disk_num = header.num_16k_rom_banks >> 2;
				if(disk_num > 4)
				{
					disk_num = 4;
				}

				uint8 disk_header[15] =
				    {
				        0x01,0x2A,0x4E,0x49,0x4E,0x54,0x45,0x4E,0x44,0x4F,0x2D,0x48,0x56,0x43,0x2A
				    };
#if 0
				uint8 disk[0x40000];
				for (i = 0; i < disk_num; i++)
				{
					uint32 file_num = ROM_banks[0x10000*i+0x3F];
					uint32 rom_pointer = 0x10000*i+0x40;
					uint32 disk_pointer = 0x10000*i+0x3A;
					uint32 file_size = 0;
					uint32 j;
					uint32 k;
					for(j = 0x00; j <= 0x0E; j++)
					{
						disk[0x10000*i+j] = disk_header[j];
					}
					for(j = 0x0F; j <= 0x37; j++)
					{
						disk[0x10000*i+j] = ROM_banks[0x10000*i+j-0x0F];
					}
					disk[0x10000*i+0x38] = 0x02;
					disk[0x10000*i+0x39] = file_num;
					for(j = 0; j < file_num; j++)
					{
						if(disk_pointer < 0x10000*(i+1))
						{
							file_size = ROM_banks[rom_pointer+13]+ROM_banks[rom_pointer+14]*256;
							for(k = 0; k < 16; k++)
							{
								disk[disk_pointer] = ROM_banks[rom_pointer];
								disk_pointer++;
								rom_pointer++;
							}
							disk[disk_pointer] = 0x04;
							disk_pointer++;
							for(k = 0; k < file_size; k++)
							{
								disk[disk_pointer] = ROM_banks[rom_pointer];
								disk_pointer++;
								rom_pointer++;
							}
						}
					}
				}
				for(i = 0; i < disk_num; i++)
				{
					for(uint32 j = 0; j < 65500; j++)
					{
						ROM_banks[65500*i+j+16] = disk[0x10000*i+j];
					}
				}
#else
				uint8 disk[0x10000];
				for (i = 0; i < disk_num; i++)
				{
					int rpos, wpos;
					wpos = i*65500+16;
					rpos = i*0x10000;

					memcpy(disk, &ROM_banks[rpos], 65500);
					memcpy(&ROM_banks[wpos], disk_header, 15);
					wpos+=15;
					memcpy(&ROM_banks[wpos], disk, 65500-15);
				}
#endif

				//header.id[0] = 'N';
				//header.id[1] = 'E';
				//header.id[2] = 'S';
				//header.num_16k_rom_banks = disk_num*4;
				//header.num_8k_vrom_banks = 0;
				//header.flags_1 = 0x40;
				//header.flags_2 = 0x10;
				//header.reserved[0] = header.reserved[1] = header.reserved[2] = header.reserved[3] = 0;
				//header.reserved[4] = header.reserved[5] = header.reserved[6] = header.reserved[7] = 0;
				ROM_banks[0] = 'F';
				ROM_banks[1] = 'D';
				ROM_banks[2] = 'S';
				ROM_banks[3] = 0x1A;
				ROM_banks[4] = disk_num;
			}
		}
		else if( !strncmp( (char*)header.id, "FDS", 3) && ( header.ctrl_z == 0x1A) )
		{
			if(!NESTER_settings.nes.preferences.DisableIPSPatch)
				LoadIPSPatch((unsigned char *)p, (char *)fn);		//IPS PATCH

			image_type = 1;
			uint8 disk_num = header.num_16k_rom_banks;

			header.id[0] = 'N';
			header.id[1] = 'E';
			header.id[2] = 'S';
			header.num_16k_rom_banks *= 4;
			header.num_8k_vrom_banks = 0;
			header.flags_1 = 0x40;
			header.flags_2 = 0x10;
			header.reserved[0] = header.reserved[1] = header.reserved[2] = header.reserved[3] = 0;
			header.reserved[4] = header.reserved[5] = header.reserved[6] = header.reserved[7] = 0;

			// allocate memory
			ROM_banks = (uint8*)malloc(16+65500*disk_num);
			if(!ROM_banks) throw "Out of memory";

			VROM_banks = (uint8*)malloc(0);
			if(!VROM_banks) throw "Out of memory";

			if( p + 65500 * disk_num - buf > filesize ){
				LoadString(g_main_instance, IDS_STRING_ERRM_07 , str, 64);
				throw str;
			}
			CopyMemIncPtr( ROM_banks + 16, p, 65500 * disk_num );

			ROM_banks[0] = 'F';
			ROM_banks[1] = 'D';
			ROM_banks[2] = 'S';
			ROM_banks[3] = 0x1A;
			ROM_banks[4] = disk_num;
		}
		else if(header.id[0] <= 0x1A && header.id[1] == 0x00 && header.num_8k_vrom_banks == 0x00)
		{
			image_type = 1;
			uint8 fam[6];
			fam[0] = header.id[0];
			fam[1] = header.id[1];
			fam[2] = header.id[2];
			fam[3] = header.ctrl_z;
			fam[4] = header.num_16k_rom_banks;
			fam[5] = header.num_8k_vrom_banks;

			p = 6 + buf;

			while(!((fam[0] == 0x13 || fam[0] == 0x1A) && fam[1] == 0x00))
			{
				if(p + (uint32)fam[2]+((uint32)fam[3]<<8)+((uint32)fam[4]<<16)-6 - buf > filesize)
					throw 0; //"Error reading FAM image";
				p += (uint32)fam[2]+((uint32)fam[3]<<8)+((uint32)fam[4]<<16)-6;
				if(p + 6 - buf > filesize)
					throw 0; //"Error reading FAM image";
				CopyMemIncPtr( fam, p, 6 );
			}

			uint8 disk_num = (uint8)(((uint32)fam[2]+((uint32)fam[3]<<8)+((uint32)fam[4]<<16))/65500);

			header.id[0] = 'N';
			header.id[1] = 'E';
			header.id[2] = 'S';
			header.num_16k_rom_banks = disk_num*4;
			header.num_8k_vrom_banks = 0;
			header.flags_1 = 0x40;
			header.flags_2 = 0x10;
			header.reserved[0] = header.reserved[1] = header.reserved[2] = header.reserved[3] = 0;
			header.reserved[4] = header.reserved[5] = header.reserved[6] = header.reserved[7] = 0;

			// allocate memory
			ROM_banks = (uint8*)malloc(16+65500*disk_num);
			if(!ROM_banks) throw "Out of memory";

			VROM_banks = (uint8*)malloc(0);
			if(!VROM_banks) throw "Out of memory";

			if(fam[0] == 0x1A)
			{
				if( p + 16 - buf > filesize )
					throw 0;//"FAM イメージの読み込みに失敗しました";
				p += 16;
			}

			if( p + 65500 * disk_num - buf > filesize )
				throw 0; //"FAM イメージの読み込みに失敗しました";
			CopyMemIncPtr( ROM_banks + 16, p, 65500 * disk_num );

			ROM_banks[0] = 'F';
			ROM_banks[1] = 'D';
			ROM_banks[2] = 'S';
			ROM_banks[3] = 0x1A;
			ROM_banks[4] = disk_num;
		}
		else if(!strncmp((const char*)header.id, "NES", 3) && (header.ctrl_z == 'M'))
		{
			image_type = 2;
			ROM_banks = (uint8*)malloc(0x40000);
			if(!ROM_banks) throw "Out of memory";

			VROM_banks = (uint8*)malloc(0);
			if(!VROM_banks) throw "Out of memory";

			if( filesize > 0x40000 ){
				char str[64];
				LoadString(g_main_instance, IDS_STRING_ERRM_09 , str, 64);
				throw str;
			}
			CopyMemIncPtr( ROM_banks + 0x10, p, filesize - ( p - buf ) );

			*(int*)ROM_banks = filesize;
			ROM_banks[0x4] = header.num_16k_rom_banks;
			ROM_banks[0x5] = header.num_8k_vrom_banks;
			ROM_banks[0x6] = header.flags_1;
			ROM_banks[0x7] = header.flags_2;
			ROM_banks[0x8] = header.reserved[0];
			ROM_banks[0x9] = header.reserved[1];
			ROM_banks[0xA] = header.reserved[2];
			ROM_banks[0xB] = header.reserved[3];
			ROM_banks[0xC] = header.reserved[4];
			ROM_banks[0xD] = header.reserved[5];
			ROM_banks[0xE] = header.reserved[6];
			ROM_banks[0xF] = header.reserved[7];

			header.id[0] = 'N';
			header.id[1] = 'E';
			header.id[2] = 'S';
			header.ctrl_z = 0x1A;
			header.num_16k_rom_banks = 1;
			header.num_8k_vrom_banks = 0;
			header.flags_1 = 0xC0;
			header.flags_2 = 0x00;
			header.reserved[0] = header.reserved[1] = header.reserved[2] = header.reserved[3] = 0;
			header.reserved[4] = header.reserved[5] = header.reserved[6] = header.reserved[7] = 0;
		}
		else if(!strncmp((char*)header.id, "UNIF", 4)){		// UNIF
			int promcn=0, cromcn=0, promtsize=0, cromtsize=0, unif_pos=0x10;
			unsigned char *prommp[16], *crommp[16];
			unsigned int prommsize[16], crommsize[16], promcsize[16], cromcsize[16];
			const char *mapperstr[4]={ "BMC-NovelDiamond9999999" ,"BTL-MARIO1-MALEE" , "BMC-Supervision16in1","BMC-Super24in1"};
			ROM_banks = VROM_banks = NULL;
			header.num_16k_rom_banks = header.num_8k_vrom_banks = 0;
			header.flags_1 = header.flags_2 = 0;
			unif_psize_16k = unif_csize_8k =0;
			memset(prommp, 0, sizeof(prommp));
			memset(crommp, 0, sizeof(crommp));

			image_type = 3;
			unif_mapper = 0xFF;
			while(unif_pos < (filesize-0x10)){
				DWORD ch = *((DWORD *)&p[unif_pos]);
				DWORD chunksize = *((DWORD *)&p[unif_pos+4]);
				if(ch==0x5250414D){						//MAPR
					if(chunksize>=23 && !memcmp(&p[unif_pos+8], mapperstr[0], 23)){
						unif_mapper = 1;
						strcpy(GameTitle, mapperstr[0]);
					}
					else if(chunksize>=16 && !memcmp(&p[unif_pos+8], mapperstr[1], 16)){
						unif_mapper = 2;
						strcpy(GameTitle, mapperstr[1]);
					}
					else if(chunksize>=20 && !memcmp(&p[unif_pos+8], mapperstr[2], 20)){
						unif_mapper = 3;
						strcpy(GameTitle, mapperstr[2]);
					}
					else if(chunksize>=14 && !memcmp(&p[unif_pos+8], mapperstr[3], 14)){
						unif_mapper = 4;
						strcpy(GameTitle, mapperstr[3]);
					}
				}
				else if(ch==0x5252494D){				//MIRR
				}
				else if(ch==0x52544142){				//BATR
				}
				else if((ch&0x00ffffff)==0x00475250){	//PRG?
					int msize, pagen;
					pagen = (ch>>24)-'0';
					if(chunksize<0x4000){
						msize=0x4000;
					}
					else{
						msize=chunksize&0xFFFFC000;
						if(chunksize&0x3FFF)
							msize+=0x4000;
					}
					unif_psize_16k += (unsigned char)(msize/0x4000);
					prommp[pagen] = &p[unif_pos+8];
					promcsize[pagen] = chunksize;
					prommsize[pagen] = msize;
					promtsize += msize;
				}
				else if((ch&0x00ffffff)==0x00524843){	//CHR?
					int msize, pagen;
					pagen = (ch>>24)-'0';
					if(chunksize<0x2000){
						msize=0x2000;
					}
					else{
						msize=chunksize&0xFFFFE000;
						if(chunksize&0x1FFF)
							msize+=0x2000;
					}
					unif_csize_8k += (unsigned char)(msize/0x2000);
					crommp[pagen] = &p[unif_pos+8];
					cromcsize[pagen] = chunksize;
					crommsize[pagen] = msize;
					cromtsize += msize;
				}
				unif_pos += (chunksize + 8);
			}
			{
				int i, tsize;
				ROM_banks = (unsigned char *)malloc(promtsize);
				if(ROM_banks==NULL)
					throw "Memory error";
				VROM_banks = (unsigned char *)malloc(cromtsize);
				if(VROM_banks==NULL)
					throw "Memory error";

				for(tsize=0, i=0; prommp[i]&&i<16; i++){
					memcpy(&ROM_banks[tsize], prommp[i], promcsize[i]);
					tsize+=prommsize[i];
				}
				for(tsize=0, i=0; crommp[i]&&i<16; i++){
					memcpy(&VROM_banks[tsize], crommp[i], cromcsize[i]);
					tsize+=crommsize[i];
				}
			}
		}
		else
		{
			char str[64];
			LoadString(g_main_instance, IDS_STRING_ERRM_08 , str, 64);
			throw str;
		}
		free(buf);
	}
	catch(...)
	{
		if(hf)			CloseHandle(hf);
		if(buf)			free(buf);
		if(VROM_banks)	free(VROM_banks);
		if(ROM_banks)	free(ROM_banks);
		if(trainer)		free(trainer);
		if(rom_name)	free(rom_name);
		if(rom_path)	free(rom_path);
		throw;
	}

	//	uint32 j;
	screen_mode = 1;

	// figure out mapper number
	mapper = ( header.flags_1 >> 4);

	// if there is anything in the reserved bytes,
	// don't trust the high nybble of the mapper number
	//	for( i = 0; i < sizeof(header.reserved); i++ )
	//	{
	//		if(header.reserved[i] != 0x00) throw "Invalid NES header ($8-$F)";
	//	}
	mapper |= ( header.flags_2 & 0xF0 );

	dbcorrect[0]=0;
//	if(unif_mapper)
//		return;

	if(image_type == 1)
	{
		//	screen_mode = 1;
		mapper = 20;

		fds = (ROM_banks[0x1f] << 24) | (ROM_banks[0x20] << 16) |
		      (ROM_banks[0x21] <<  8) | (ROM_banks[0x22] <<  0);
		for(int i = 0; i < ROM_banks[4]; i++)
		{
			uint8 file_num = 0;
			uint32 pt = 16+65500*i+0x3a;
			while(ROM_banks[pt] == 0x03)
			{
				pt += 0x0d;
				pt += ROM_banks[pt] + ROM_banks[pt+1] * 256 + 4;
				file_num++;
			}
			ROM_banks[16+65500*i+0x39] = file_num;
		}
	}
	else if(image_type == 2)
	{
		//    screen_mode = 1;
		mapper = 12; // 12 is private mapper number
	}
	else if(image_type == 0)
	{
//		crc = CrcCalc(ROM_banks, header.num_16k_rom_banks * 0x4000);
		crc = CrcCalca(ROM_banks, header.num_16k_rom_banks * 0x4000, crc);
		crc_all = CrcCalca(VROM_banks, header.num_8k_vrom_banks * 0x2000, crc);
#if 1

		if(!NESTER_settings.nes.preferences.UseRomDataBase)
			// nestoy database
		{
			FILE* fp2;
			unsigned int c, pt, i, db=2;
			char fn2[256], buf[256];
			unsigned char theader[2], dbflag=0;
			GetModuleFileName(NULL, fn2, 256);
			while(db){
				if(db==2){
					PathRemoveFileSpec(fn2);
					PathCombine(fn2 , fn2, "famicom.dat");
				}
				else{
					PathRemoveFileSpec(fn2);
					PathCombine(fn2 , fn2, "nesdbase.dat");
				}
				fp2 = fopen(fn2, "r");
				if(fp2 != NULL){
					while(fgets(buf, 256, fp2)){
						pt = 0;
						// All CRC
						for(;buf[pt] != ';' && buf[pt]!='\0';pt++);
						if(buf[pt]=='\0')
							continue;
						pt++;
						// PROM CRC
						c = 0;
						{
							char buf2[16];
							for(i = 0; i < 8 && buf[pt] != ';' && buf[pt]; i++, pt++){
								buf2[i] = buf[pt];
							}
							if(buf[pt]=='\0')
								continue;
							++pt;
							buf2[i] = 0;
							MyStrAtoh(buf2, (DWORD *)&c);
						}
						if(crc == c && crc != 0 && c != 0){
							// Title
							for(i=0; buf[pt] != ';' && buf[pt]!='\0'; ++i, ++pt){
								GameTitle[i] = buf[pt];
							}
							GameTitle[i]=0;
							MystrFnIcut(GameTitle);
							pt++;
							char buf2[16];
							// Header 1
							i = 0;
							while(buf[pt] != ';') buf2[i++] = buf[pt++];
							pt++;
							buf2[i] = '\0';
							//						header.flags_1 = atoi(buf2);
							theader[0] = atoi(buf2);
							// Header 2
							i = 0;
							while(buf[pt] != ';') buf2[i++] = buf[pt++];
							pt++;
							buf2[i] = '\0';
							//						header.flags_2 = atoi(buf2);
							theader[1] = atoi(buf2);
							// PROM Size
							while(buf[pt] != ';') pt++;
							pt++;
							// CROM Size
							while(buf[pt] != ';') pt++;
							pt++;
							// Country
							if(/*buf[pt] == 'A' ||*/ buf[pt] == 'E' || (buf[pt] == 'P' && buf[pt+1] == 'D') || buf[pt] == 'S')
							{
								// Asia, Europe, PD, Swedish
								screen_mode = 2;
							}
							/*						else if(buf[pt] == 'J' || buf[pt] == 'U' || buf[pt] == 'V'){
														screen_mode = 1;
													}*/
							//						mapper = (header.flags_1 >> 4) | (header.flags_2 & 0xF0);
							for(i = 0; i < 8; i++) header.reserved[i] = 0;
							//						fseek(fp2, 0, SEEK_END);
							dbflag=1;
							break;
						}
					}
					fclose(fp2);
				}
				if(dbflag)
					break;
				--db;
			}

			if(dbflag){
				if(theader[0]!=header.flags_1 || ((theader[1]&0xf0)!=(header.flags_2&0xf0))){
					if(NESTER_settings.nes.preferences.AutoRomCorrect){
						char str2[64], str3[64];
						LoadString(g_main_instance, IDS_STRING_LMSG_05 , str2, 64);
						LoadString(g_main_instance, IDS_STRING_MSGB_10 , str3, 64);
						if(IDYES== MessageBox(main_window_handle,(LPCSTR)str3,(LPCSTR)str2, MB_YESNO))
						{
							dbcorrect[0]=1, dbcorrect[1]= header.flags_1, dbcorrect[2]= header.flags_2;
							header.flags_1 = theader[0];
							header.flags_2 = theader[1];
							mapper = (header.flags_1 >> 4) | (header.flags_2 & 0xF0);
						}
					}
					else{
						dbcorrect[0]=1, dbcorrect[1]= header.flags_1, dbcorrect[2]= header.flags_2;
						header.flags_1 = theader[0];
						header.flags_2 = theader[1];
						mapper = (header.flags_1 >> 4) | (header.flags_2 & 0xF0);
					}
				}
			}
		}
#endif
	}
	#include "NES_rom_Correct.cpp"
}

NES_ROM::~NES_ROM()
{
	if(VROM_banks)	free(VROM_banks);
	if(ROM_banks)	free(ROM_banks);
	if(trainer)		free(trainer);
	if(rom_name)	free(rom_name);
	if(rom_path)	free(rom_path);
}

void NES_ROM::GetPathInfo(const char* fn)
{
	strcpy( rom_path, fn );
	PathRemoveFileSpec( rom_path );
	PathAddBackslash( rom_path );

	strcpy( rom_name, PathFindFileName( fn ) );
	PathRemoveExtension( rom_name );
}


void NES_ROM::GetROMInfoStr(char *h){
	char headerflag[5],headerflag2[5];
	unsigned char *th = (unsigned char *)&header;
//	h[0x10]= dbcorrect[0], h[0x11]= dbcorrect[1], h[0x12]= dbcorrect[2];
	for(int i=0,j=1; i<4; ++i, j<<=1){
		if(th[6] & j)
			headerflag[i]='1';
		else
			headerflag[i]='0';
		if(th[0x11] & j)
			headerflag2[i]='1';
		else
			headerflag2[i]='0';
	}
	headerflag[4]=0, headerflag2[4]=0;
	if(dbcorrect[0]){
		wsprintf(h, " Mapper [ %u -> %u ], PROM %uKB, CROM %uKB, FLAG[ %s -> %s ], PROM CRC 0x%08X, ROM CRC 0x%08X", (dbcorrect[1] >> 4)|(dbcorrect[2] & 0xF0),(th[6] >> 4)|(th[7] & 0xF0), th[4]*16, th[5]*8, headerflag2, headerflag, crc, crc_all);
	}
	else{
		wsprintf(h, " Mapper [ %u ], PROM %uKB, CROM %uKB, FLAG %s, PROM CRC 0x%08X, ROM CRC 0x%08X", (th[6] >> 4)|(th[7] & 0xF0), th[4]*16, th[5]*8, headerflag, crc, crc_all);
	}
}
