#ifndef MKUTILS_H_
#define MKUTILS_H_

//#ifdef _USENH
//#include "nh.h"
//#endif

extern void MKCreateDirectories( const char *path );
extern int MKGetLongFileName( char *longPath, const char *sourcePath );

#endif