
#ifndef __MYCFGSAVE_H
#define __MYCFGSAVE_H

#include <windows.h>
#include <shlwapi.h>


class SaveConfigBase
{
public:
	SaveConfigBase(){};
	virtual ~SaveConfigBase(){};
	virtual void WriteString(char *sec, char *key, char *val) = 0;
	virtual void WriteHexString(char *sec, char *key, unsigned char *val, int size) = 0;
	virtual void WriteInt(char *sec, char *key, int val) = 0;
protected:
//	virtual int Open(char *fn);
//	virtual void Close();
private:

};



class LoadConfigBase
{
public:
	LoadConfigBase(){};
	virtual ~LoadConfigBase(){};
// �Z�N�V�����A�L�[���A�󂯎��|�C���^�A�󂯎�鑤�̍ő�T�C�Y�A�f�t�H���g�l
	virtual int ReadString(char *sec, char *key, char *pval, int max, char *defval) = 0;
	virtual int ReadInt(char *sec, char *key, int defval) = 0;
	virtual int ReadHexString(char *sec, char *key, unsigned char *pval, int size) = 0;
protected:
//	virtual int Open(char *fn);
//	virtual void Close();
private:

};



class SaveConfigWinIni : public SaveConfigBase
{
public:
	SaveConfigWinIni(char *fn);
	~SaveConfigWinIni();
	void WriteString(char *sec, char *key, char *val);
	void WriteInt(char *sec, char *key, int val);
	void WriteHexString(char *sec, char *key, unsigned char *val, int size);
protected:
private:
	char *m_fn;

};


class LoadConfigWinIni : public LoadConfigBase
{
public:
	LoadConfigWinIni(char *fn);
	~LoadConfigWinIni();
//	virtual int Open(char *fn);
//	virtual void Close();
// �Z�N�V�����A�L�[���A�󂯎��|�C���^�A�󂯎�鑤�̍ő�T�C�Y�A�f�t�H���g�l
	int ReadString(char *sec, char *key, char *pval, int max, char *defval);
	int ReadHexString(char *sec, char *key, unsigned char *pval, int size);
	int ReadInt(char *sec, char *key, int defval);
protected:
private:
	char *m_fn;

};



class SaveConfigWinReg : public SaveConfigBase
{
public:
	SaveConfigWinReg(char *swname);
	~SaveConfigWinReg();
	void WriteString(char *sec, char *key, char *val);
	void WriteInt(char *sec, char *key, int val);
	void WriteHexString(char *sec, char *key, unsigned char *val, int size);
protected:
private:
	void OpenSectionKey(char *sec);

	HKEY softname_key;
	HKEY section_key;
	char cur_keyname[260];
};


class LoadConfigWinReg : public LoadConfigBase
{
public:
	LoadConfigWinReg(char *swname);
	~LoadConfigWinReg();
//	virtual int Open(char *fn);
//	virtual void Close();
// �Z�N�V�����A�L�[���A�󂯎��|�C���^�A�󂯎�鑤�̍ő�T�C�Y�A�f�t�H���g�l
	int ReadString(char *sec, char *key, char *pval, int max, char *defval);
	int ReadHexString(char *sec, char *key, unsigned char *pval, int size);
	int ReadInt(char *sec, char *key, int defval);
protected:
private:
	void OpenSectionKey(char *sec);

	HKEY softname_key;
	HKEY section_key;
	char cur_keyname[260];
};



unsigned char Hexstrtouc(char *str);




#endif


