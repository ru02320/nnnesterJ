

#ifndef MY_EDITVIEWCLASS
#define MY_EDITVIEWCLASS

#include <windows.h>
#include "EditView.h"

#define EDIT_CHARWIDTH	56		// 8+1+(3*16)-1
#define EDIT_ADDRNCHAR	8		// 8+1+(3*16)
#define EDIT_NUM_STARTX	9		// 8+1+(3*16)
#define EDIT_NUM_ENDX	56		// 8+1+(3*16)-1

class My_EditView
{
public:
	My_EditView(HWND hWnd, HINSTANCE hInst, RECT *rect, LOGFONT *pLogFont, unsigned char *pmem, int memsize);
	~My_EditView();

	void DrawBinEdit_1byte(HDC hdc);
	void SetScrollInfoParam();
	void DrawBinEdit(HDC hdc);
	void SetWindowSize(int w, int h);
	void SetFont(LOGFONT *pLogFont);
	void KillFocus();
	void SetFocus();
	void LButtonDown(int x, int y);
	void VScroll(DWORD wp);
	int GetCursorAddress();
	int KeyDown(char wp);
	int CharKeyDown(char wp);
	void SetMemPointer(unsigned char *pmem, int memsize);
	void SetColorDisplay(int Bytes, int Num);


protected:
	HWND		m_hWnd;
	HINSTANCE	m_hInst;
	int		m_EnableCarat;
	int		m_nColorDispBytes;
	DWORD		m_nColorDispNum;

	HFONT	m_hFont;			// handle of fixed display font
//	HWND 
	int	m_nCursorX;			//カーソル現在位置X
	int	m_nCursorY;			//  〃  Y
	int	m_cxClient;
	int	m_cyClient;			//width, height of hex view window

	int	m_nCharHeight;		//
	int m_nCharWidth;			//
	int m_nNumAddrCols;			//numer of chars taken by address
//	int m_nLineSpaceHeight;	// スペース（高さ
//	int m_nCurrentLinePos;	// 現在表示開始位置ライン数

	//Scroll bar data
	int m_nVScrollPos;		//スクロールバー現時位置V
	int m_nHScrollPos;		//  〃  H
	int m_nVScrollMax;		//  〃 最大V
	int m_nHScrollMax;		//    〃   H

	int m_nPageMaxLines;		//
	int m_nPageMaxCols;		//
	int m_nNumLines;			//現在ライン数 (nFileLength/0x10)
	int m_nFileLength;		//ファイルサイズ
//	int m_nScreenDispBytes;		//ファイルサイズ

	unsigned char	*m_pData;		//データ
	int				m_nDataMem;	//データメモリ確保サイズ

};



//#include "nes6502.h"

class My_NES_EditView
{
public:
	My_NES_EditView(HWND hWnd, HINSTANCE hInst, RECT *rect, LOGFONT *pLogFont, unsigned char **pmem);
	~My_NES_EditView();

	void DrawBinEdit_1byte(HDC hdc);
	void SetScrollInfoParam();
	void DrawBinEdit(HDC hdc);
	void SetWindowSize(int w, int h);
	void SetFont(LOGFONT *pLogFont);
	void KillFocus();
	void SetFocus();
	void LButtonDown(int x, int y);
	void VScroll(DWORD wp);
	int GetCursorAddress();
	int KeyDown(char wp);
	int CharKeyDown(char wp);
	void SetMemPointer(unsigned char **pmem);
	void SetColorDisplay(int Bytes, int Num);

	unsigned char GetDataByte(int Address);
	WORD GetDataWord(int Address);
	DWORD GetDataDWord(int Address);
	void SetDataByte(int Address, unsigned char Data);

protected:
	HWND		m_hWnd;
	HINSTANCE	m_hInst;
	int		m_EnableCarat;
	int		m_nColorDispBytes;
	DWORD		m_nColorDispNum;

	HFONT	m_hFont;			// handle of fixed display font
//	HWND 
	int	m_nCursorX;			//カーソル現在位置X
	int	m_nCursorY;			//  〃  Y
	int	m_cxClient;
	int	m_cyClient;			//width, height of hex view window

	int	m_nCharHeight;		//
	int m_nCharWidth;			//
	int m_nNumAddrCols;			//numer of chars taken by address
//	int m_nLineSpaceHeight;	// スペース（高さ
//	int m_nCurrentLinePos;	// 現在表示開始位置ライン数

	//Scroll bar data
	int m_nVScrollPos;		//スクロールバー現時位置V
	int m_nHScrollPos;		//  〃  H
	int m_nVScrollMax;		//  〃 最大V
	int m_nHScrollMax;		//    〃   H

	int m_nPageMaxLines;		//
	int m_nPageMaxCols;		//
	int m_nNumLines;			//現在ライン数 (nFileLength/0x10)
	int m_nFileLength;		//ファイルサイズ
//	int m_nScreenDispBytes;		//ファイルサイズ

	unsigned char	*m_pData[8];		//データ
	int				m_nDataMem;	//データメモリ確保サイズ

};

#endif


