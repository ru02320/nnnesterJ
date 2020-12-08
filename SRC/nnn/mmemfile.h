
#ifndef __MMEMREAD_H__
#define __MMEMREAD_H__

#include <stdlib.h>
#include <memory.h>

class my_memread_stream
{
private:
  unsigned char *pMem;
  int nSize;
  int nPos;

public:

  my_memread_stream(unsigned char *p, int size)
  {
  	pMem = p;
  	nSize = size;
  	nPos = 0;
  }

  void close(void)
  {
    if (pMem)
    {
      free(pMem);
      pMem = NULL;
    }
    nSize = 0;
  	nPos = 0;
  }

  ~my_memread_stream()
  {
    close();
  }

  unsigned char *get_mem_p(){
	  return pMem;
  }

  bool open(unsigned char *p, int size)
  {
    close();
  	
  	pMem = p;
  	nSize = size;
  	nPos = 0;
    return (false);
  }

  int get_ch(/*int *eof*/){
  	if(nPos >= nSize){
/*
		if(eof)
			*eof = 1;
*/
		return -1;
  	}
	return pMem[nPos++];
  }

  int get_data(/*int *eof*/){
  	if(nPos >= nSize){
/*
		if(eof)
			*eof = 1;
*/
		return -1;
  	}
	return pMem[nPos];
  }

  int read(unsigned char *Pbuf, int nread/*, int *eof*/)
  {
    if (!pMem)
      return 0;

  	if(nread > nSize-nPos){
  		nread = nSize-nPos;
/*
		if(eof)
			*eof = 1;
*/
  	}
	if(!nread){
/*
		if(eof)
			*eof = 1;
*/
		return 0;
	}
  	memcpy(Pbuf, &pMem[nPos], nread);
  	nPos += nread;
    return (nread);
  }

  bool reset(void)
  {
    if(!pMem)
      return (true);
  	nPos = 0;
    return (false);
  }

  int get_size(void)
  {
    if(!pMem)
      return (-1);
    return (nSize);
  }
/*
  int get_pointer(void)
  {
    if(!pMem)
      return (-1);
    return pMem;
  }
*/
  int set_pos(int newpos)
  {
    if(!pMem)
      return -1;
	if(newpos >= nSize)
		return -1;
	nPos = newpos;
    return 0;
  }

  int move_pos(int apos)
  {
    if(!pMem)
      return -1;
	nPos += apos;
	if(nPos >= nSize){
		nPos = nSize-1;
	}
	else if(nPos < 0){
		nPos = 0;
	}
    return 0;
  }

  int get_pos()
  {
	return nPos;
  }
};



#endif