
#ifndef __MMEMFILE_H__
#define __MMEMFILE_H__


class my_stream_base
{
public:

  my_stream_base(){}
  virtual ~my_stream_base(){}
  virtual int read(unsigned char *Pbuf, int nread) = 0;
  virtual int get_ch(void) = 0;
  virtual int get_size(void) = 0;
  virtual bool set_pos(int newpos) = 0;
};


class my_file_stream : public my_stream_base
{
  FILE *Pfile;
  bool eof_flag, error_flag;

public:

  my_file_stream(FILE *fp)
  {
    Pfile = fp;
    eof_flag = false;
    error_flag = false;
 }

  ~my_file_stream()
  {
    close();
  }

  void close(void)
  {
    if (Pfile)
    {
      fclose(Pfile);
      Pfile = NULL;
    }

    eof_flag = false;
    error_flag = false;
  }

  int get_ch(){
	  return fgetc(Pfile);
  }

  int read(unsigned char *Pbuf, int nread)
  {
    if (!Pfile)
      return 0;

    if (eof_flag)
    {
      return 0;
    }

    if (error_flag)
      return 0;

    int bytes_read = fread(Pbuf, 1, nread, Pfile);

    if (bytes_read < nread)
    {
      if (ferror(Pfile))
      {
        error_flag = true;
        return (-1);
      }

      eof_flag = true;
    }

    return (bytes_read);
  }

  bool get_error_status(void)
  {
    return (error_flag);
  }

  bool reset(void)
  {
    if (error_flag)
      return (true);

    fseek(Pfile, 0, SEEK_SET);

    eof_flag = false;

    return (false);
  }

  int get_size(void)
  {
    if (!Pfile)
      return (-1);

    int loc = ftell(Pfile);

    fseek(Pfile, 0, SEEK_END);

    int size = ftell(Pfile);

    fseek(Pfile, loc, SEEK_SET);

    return (size);
  }

  bool set_pos(int newpos)
  {
    if (!Pfile)
      return (true);
    fseek(Pfile, newpos, SEEK_SET);
    return (false);
  }

};



class my_mem_stream : public my_stream_base
{
  unsigned char *pMem;
  unsigned int nSize;
  unsigned int nPos;
	
public:

  my_mem_stream(unsigned char *p, int size)
  {
    pMem = NULL;
    nSize = 0;
  	nPos = 0;
  	open(p, size);
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

  ~my_mem_stream()
  {
    close();
  }

  bool open(unsigned char *p, int size)
  {
    close();
  	
  	pMem = p;
  	nSize = size;
  	nPos = 0;
    return (false);
  }

  int get_ch(){
	  if(nPos >= nSize){
		  return -1;
	  }
	  return (int)pMem[nPos++];
  }

  int read(unsigned char *Pbuf, int nread)
  {
    if (!pMem)
      return 0;

  	if(nread > (int)nSize-nPos){
  		nread = nSize-nPos;
  	}
	if(!nread)
		return 0;
  	memcpy(Pbuf, &pMem[nPos], nread);

  	nPos += nread;
    return (nread);
  }


  bool set_pos(int newpos)
  {
    if(!pMem || (unsigned int)newpos >= nSize)
      return (true);
  	nPos = newpos;
    return (false);
  }


  bool reset(void)
  {
    if (!pMem)
      return (true);
  	nPos = 0;
    return (false);
  }

  int get_size(void)
  {
    if (!pMem)
      return (-1);
    return (nSize);
  }
};



#endif