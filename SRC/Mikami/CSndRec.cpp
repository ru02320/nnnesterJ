#include <stdlib.h>
#include <memory.h>
#include "CSndRec.h"



CSndRec::CSndRec( const char *szFileName, dword nSampleBits, dword nSampleRate, int nChannel)
{
	status=0;
	fwav = fopen( szFileName, "wb" );
	if(fwav==NULL)
		return;
	memset( &WaveHead, 0x00, sizeof(RIFFHEADER) );
	fwrite( &WaveHead, sizeof(RIFFHEADER), 1, fwav );
	WaveHead.nSample = nSampleRate;
    WaveHead.nBitrate = (word)nSampleBits;
	WaveHead.nChannel = nChannel;
	status=1;
}

CSndRec::~CSndRec()
{
	if(status){
		memcpy( WaveHead.typeFile, "RIFF", 0x04 );
		WaveHead.sizeFile = (dword)( ftell(fwav)-8 );
		memcpy( WaveHead.typeSub, "WAVEfmt ", 0x08 );
		WaveHead.offsetFirstTag		= 0x00000010;
		WaveHead.typeCoding			=	  0x0001;
//		WaveHead.nChannel			=	  nChannel;
		WaveHead.nBytesPerSec       = (WaveHead.nSample << ( WaveHead.nBitrate >> 4 )) * WaveHead.nChannel;
		WaveHead.nBytesPerSample	= (WaveHead.nBitrate >> 3) * WaveHead.nChannel;
		memcpy( WaveHead.tagFirst, "data", 0x04 );
		WaveHead.sizeData = (dword)( ftell(fwav) - sizeof(RIFFHEADER) );
		fseek( fwav, 0, SEEK_SET );
		fwrite( &WaveHead, sizeof(RIFFHEADER), 1, fwav );
		fflush( fwav );
		fclose( fwav );
	}
}

void CSndRec::write( void *lpBuf, dword size )
{
	fwrite( lpBuf, 1, size, fwav );
}



