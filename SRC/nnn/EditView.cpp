

#include <windows.h>
#include "EditView.h"


My_EditView::My_EditView(HWND hWnd, HINSTANCE hInst, RECT *rect, LOGFONT *pLogFont, unsigned char *pmem, int memsize){
	m_hWnd = hWnd;
	m_hInst= hInst;
	m_EnableCarat = 0;

	m_hFont			= NULL;
	m_nPageMaxLines	=0;
	m_nPageMaxCols	=0;
	m_nNumLines		=0;
	m_nFileLength	=0;
	m_pData			=NULL;
	m_nDataMem		=0;

	m_nVScrollPos	=0;
	m_nHScrollPos	=0;
	m_nVScrollMax	=0;
	m_nHScrollMax	=0;
	m_nCursorX = EDIT_NUM_STARTX;
	m_nCursorY = 0;

	m_nFileLength = memsize;
	m_pData = pmem;
	m_nColorDispBytes= 0;
	m_nColorDispNum = 0;

	SetFont(pLogFont);
	SetWindowSize(rect->right-rect->left, rect->bottom-rect->top);
}


My_EditView::~My_EditView(){
	if(m_EnableCarat){
		HideCaret(m_hWnd);
		DestroyCaret();
		m_EnableCarat=0;
	}
	if(m_hFont){
		DeleteObject(m_hFont);
		m_hFont=NULL;
	}
}


void My_EditView::SetFont(LOGFONT *pLogFont){
	TEXTMETRIC  tm;
	if(m_hFont){
		DeleteObject(m_hFont);
		m_hFont=NULL;
	}
	m_hFont = CreateFontIndirect(pLogFont);
	HDC hdc = GetDC(m_hWnd);
	HFONT hOldFont;
	hOldFont = (HFONT)SelectObject(hdc, m_hFont);
	GetTextMetrics(hdc, &tm);
	SelectObject(hdc, hOldFont);
	ReleaseDC(m_hWnd, hdc);
	m_nCharWidth = tm.tmAveCharWidth;
	m_nCharHeight = tm.tmHeight;
	SetScrollInfoParam();
}


void My_EditView::SetScrollInfoParam(){
	SCROLLINFO sInfo;

	m_nNumLines = m_nFileLength/0x10;
	m_nVScrollMax = m_nNumLines-(m_cyClient/m_nCharHeight);
	m_nHScrollMax = m_cxClient/m_nCharWidth-EDIT_CHARWIDTH;

//	m_nScreenDispBytes = m_nFileLength-(m_nVScrollPos*0x10);

	sInfo.cbSize = sizeof SCROLLINFO;
	sInfo.fMask = /*SIF_PAGE |*/ SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
	sInfo.nMin	= 0;
	sInfo.nMax	= m_nVScrollMax;
	sInfo.nPos	= m_nVScrollPos;
	sInfo.nPage	= m_nVScrollMax; //min(nPageMaxLines, nVScrollMax+1);
	SetScrollInfo (m_hWnd, SB_VERT, &sInfo, TRUE);

	sInfo.cbSize = sizeof SCROLLINFO;
	sInfo.fMask = /*SIF_PAGE |*/ SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
	sInfo.nMin	= 0;
	sInfo.nMax	= m_nHScrollMax;
	sInfo.nPos	= m_nHScrollPos;
	sInfo.nPage	= m_nHScrollMax; //min(nPageMaxLines, nVScrollMax+1);
//	SetScrollInfo (m_hWnd, SB_HORZ, &sInfo, TRUE);

}



void My_EditView::DrawBinEdit(HDC hdc){
	int yMax,xMax,y, x, xpos, cflag, xrem, yrem, nrem, address;
	char str[256];

	if(!m_pData)
		return;
	cflag = m_EnableCarat;
	if(cflag)
		HideCaret(m_hWnd);

	nrem = m_nFileLength-(m_nVScrollPos*0x10);
	yrem = nrem/0x10;
	xrem = nrem%0x10;

//	nwidth = m_nCharWidth*3;
	yMax = m_cyClient/m_nCharHeight;
	if(yMax > yrem)
		yMax = yrem;
	xMax = 16; //m_cxClient/(m_nCharWidth*EDIT_CHARWIDTH);
	xpos = m_nHScrollPos*m_nCharWidth;
//	ypos = m_nVScrollPos*(m_nCharHeight+m_nLineSpaceHeight);
//	yadd = m_nVScrollPos;
	HFONT hOldFont;
//	SetBkMode(hdc, 0);
//	SetTextColor(hdc, RGB(255, 255, 255));
	hOldFont = (HFONT)SelectObject(hdc, m_hFont);
	address = (m_nVScrollPos)*16;
	for(y = 0; y<yMax; y++, address+=0x10){
		int i=0;
		wsprintf(str, " %06X: ", (m_nVScrollPos+y)*0x10);
		SetTextColor(hdc, RGB(0, 0, 0));
		TextOutA(hdc, xpos, (y*m_nCharHeight), str, strlen(str));
		i=9;
		{
			for(x=0; x<16; x++, i+=3){
//				wsprintf(&str[i], "%02X ", m_pData[(m_nVScrollPos+y)*16+x]);
				wsprintf(str, "%02X ", m_pData[address+x]);
				SetTextColor(hdc, RGB(0, 0, 0));
				switch(m_nColorDispBytes){
					case 1:
						if(m_nColorDispNum == m_pData[address+x])
							SetTextColor(hdc, RGB(255, 0, 255));
						break;
					case 2:
						if(address+1 < m_nFileLength && m_nColorDispNum == *(WORD *)&m_pData[address+x])
							SetTextColor(hdc, RGB(255, 0, 255));
						break;
					case 3:
						if(address+2 < m_nFileLength && m_nColorDispNum == ((*(DWORD *)&m_pData[address+x])&0x0ffffff))
							SetTextColor(hdc, RGB(255, 0, 255));
						break;
					case 4:
						if(address+3 < m_nFileLength && m_nColorDispNum == *(DWORD *)&m_pData[address+x])
							SetTextColor(hdc, RGB(255, 0, 255));
						break;
				}
				TextOutA(hdc, xpos+(m_nCharWidth*i), (y*m_nCharHeight), str, strlen(str));
			}
		}
#if 1
		{
			i = 0;
			unsigned char t;
			wsprintf(&str[i], " : ");
			i+=3;
			SetTextColor(hdc, RGB(0, 0, 0));
			for(x=0; x<16; x++, i++){
				t = m_pData[(m_nVScrollPos+y)*16+x];
				if(t>=0x80 || t<0x20)
					t=0x20;
				str[i] = t;
			}
			str[i]=0;
			TextOutA(hdc, xpos+(EDIT_NUM_ENDX*m_nCharWidth), (y*m_nCharHeight), str, strlen(str));
		}

#endif
//		TextOutA(hdc, xpos, (y*m_nCharHeight), str, strlen(str));
	}
	SelectObject(hdc, hOldFont);
	if(cflag)
		ShowCaret(m_hWnd);
}


void My_EditView::DrawBinEdit_1byte(HDC hdc){
	int y, x, cflag;
	char str[16];

	HFONT hOldFont;
	if(!m_pData)
		return;
	cflag = m_EnableCarat;
	if(cflag)
		HideCaret(m_hWnd);

//	SetBkMode(hdc, 0);
//	SetTextColor(hdc, RGB(255, 255, 255));
	hOldFont = (HFONT)SelectObject(hdc, m_hFont);
	x = ((m_nCursorX-EDIT_NUM_STARTX)/3*3+EDIT_NUM_STARTX) * m_nCharWidth;
	y = m_nCursorY * m_nCharHeight;

	int cx = m_nCursorX-EDIT_NUM_STARTX;
	int cxa = cx%3;
	int cy = (m_nCursorY + m_nVScrollPos) * 0x10;

	wsprintf(str, "%02X ", m_pData[cx/3+cy]);
	TextOutA(hdc, x, y, str, strlen(str));
	SelectObject(hdc, hOldFont);
	if(cflag)
		ShowCaret(m_hWnd);
}


void My_EditView::SetWindowSize(int w, int h){
	m_cxClient = w;
	m_cyClient = h;
//	m_cxClient -= (h%m_nCharHeight);
	SetScrollInfoParam();
	if(m_nCursorY>=m_cyClient/m_nCharHeight){
		m_nCursorY = m_cyClient/m_nCharHeight-1;
	}
	if(m_EnableCarat)
		SetCaretPos(m_nCursorX*m_nCharWidth, m_nCursorY*m_nCharHeight);
}


void My_EditView::LButtonDown(int x, int y){
	x/=m_nCharWidth;
	y/=m_nCharHeight;
	if(x>=EDIT_NUM_STARTX && x<=EDIT_NUM_ENDX){
		int cx = x-EDIT_NUM_STARTX;
		if(cx%3!=2){
			m_nCursorX = x;
			m_nCursorY = y;
			SetCaretPos(x*m_nCharWidth, y*m_nCharHeight);
		}
	}
}


void My_EditView::KillFocus(){
	if(m_EnableCarat){
		HideCaret(m_hWnd);
		DestroyCaret();
		m_EnableCarat = 0;
	}
}


void My_EditView::SetFocus(){
	if(!m_EnableCarat){
		CreateCaret(m_hWnd, 0, m_nCharWidth, m_nCharHeight);
		SetCaretPos(m_nCursorX * m_nCharWidth, m_nCursorY * m_nCharHeight);
		SetCaretBlinkTime(500);
		ShowCaret(m_hWnd);
		m_EnableCarat = 1;
	}
}


void My_EditView::VScroll(DWORD wp){
	int ScrollInc=0;

	switch (LOWORD(wp)){
		case SB_LINEUP:
			ScrollInc = -1;
			break;
		case SB_LINEDOWN:
			ScrollInc = 1;
			break;
		case SB_PAGEUP:
			ScrollInc = -8;
			break;
		case SB_PAGEDOWN:
			ScrollInc = 8;
			break;
		case SB_THUMBPOSITION:
			ScrollInc = HIWORD(wp) - m_nVScrollPos;
			break;
		case SB_THUMBTRACK:
			ScrollInc = HIWORD(wp) - m_nVScrollPos;
			break;
			
		default:
			break;
	}
	ScrollInc = max(-(int)m_nVScrollPos, min((int)ScrollInc, (int)((m_nVScrollMax /*- nPageMaxCols + 1*/) - m_nVScrollPos)));
				/*
				if(m_nVScrollPos+ScrollInc > m_nVScrollMax - m_nNumLines+1){
				m_nVScrollPos = m_nVScrollMax - m_nNumLines+1;
				}
				else if(m_nVScrollPos+ScrollInc < 0)
				ScrollInc = 0;
				else
				m_nVScrollPos += ScrollInc;
				*/
	if(ScrollInc){
		m_nVScrollPos += ScrollInc;
		if(ScrollInc>0){
			m_nCursorY-=ScrollInc;
			if(m_nCursorY<0)
				m_nCursorY=0;
		}
		else{
			m_nCursorY+=abs(ScrollInc);
			if(m_nCursorY>=m_cyClient/m_nCharHeight){
				m_nCursorY = m_cyClient/m_nCharHeight-1;
			}
		}
		if(ScrollInc > 0){
			RECT arse;
//			SetRect(&arse, 0, m_cyClient, m_cxClient, m_cyClient-(ScrollInc*m_nCharHeight));
			SetRect(&arse, 0, 0, m_cxClient, m_cyClient);
			InvalidateRect(m_hWnd, &arse, FALSE);
		}
		else{
			RECT arse;
//			SetRect(&arse, 0, m_nNumLines*m_nCharHeight, m_cxClient, m_cyClient);
			SetRect(&arse, 0, m_cyClient-m_nCharHeight*abs(ScrollInc), m_cxClient, m_cyClient);
			InvalidateRect(m_hWnd, &arse, FALSE);
		}
//		InvalidateRect(m_hWnd, NULL, TRUE);
		SetCaretPos(m_nCursorX*m_nCharWidth, m_nCursorY*m_nCharHeight);
		SCROLLINFO sInfo;
		sInfo.cbSize = sizeof SCROLLINFO;
		sInfo.fMask = /*SIF_PAGE |*/ SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
		sInfo.nMin	= 0;
		sInfo.nMax	= m_nVScrollMax;
		sInfo.nPos	= m_nVScrollPos;
		sInfo.nPage	= m_nVScrollMax; //min(nPageMaxLines, nVScrollMax+1);
		//					SetScrollPos(hWnd, SB_VERT, y, TRUE);
		SetScrollInfo (m_hWnd, SB_VERT, &sInfo, TRUE);
		ScrollWindowEx(m_hWnd, 0, -m_nCharHeight * ScrollInc, NULL, NULL,NULL,NULL, SW_INVALIDATE);
		UpdateWindow(m_hWnd);


//		InvalidateRect(m_hWnd, NULL, TRUE);
	}
}


int My_EditView::GetCursorAddress(){
	int cx = (m_nCursorX-EDIT_NUM_STARTX)/3;
	int cy = (m_nCursorY + m_nVScrollPos) * 0x10;
	return cx+cy;
}


int My_EditView::KeyDown(char wp){
	switch(wp){
	case VK_UP:
		if(m_nCursorY)
			m_nCursorY--;
		else{
			if(m_nVScrollPos)
				SendMessage(m_hWnd, WM_VSCROLL, SB_LINEUP, 0);
		}
		break;
	case VK_DOWN:
		{
			int nrem = m_nFileLength-(m_nVScrollPos*0x10);
			int yrem = nrem/0x10;
			int xrem = nrem%0x10;
			if(!(m_nCursorY == yrem-1 && m_nCursorX < xrem)){
				if(m_nCursorY>=m_cyClient/m_nCharHeight)
					SendMessage(m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
				else if(m_nCursorY != yrem-1)
					m_nCursorY++;
			}
		}
		break;
	case VK_LEFT:
		if(m_nCursorX != EDIT_NUM_STARTX){
			int cx = m_nCursorX-EDIT_NUM_STARTX;
			int cxa = cx%3;
			if(cxa==0){
				m_nCursorX-=2;
			}
			else
				m_nCursorX--;
		}
		else{
			if(m_nCursorY)
				m_nCursorY--;
			else{
				if(m_nVScrollPos)
					SendMessage(m_hWnd, WM_VSCROLL, SB_LINEUP, 0);
			}
			m_nCursorX = EDIT_NUM_ENDX-1;
		}
		break;
	case VK_RIGHT:
		{
			int nrem = m_nFileLength-(m_nVScrollPos*0x10);
			int yrem = nrem/0x10;
			int xrem = nrem%0x10;
			if(!(m_nCursorY == yrem-1 && m_nCursorX < xrem+1)){
				if(m_nCursorX == EDIT_NUM_ENDX-1){
					m_nCursorX = EDIT_NUM_STARTX;
					if(m_nCursorY>=m_cyClient/m_nCharHeight)
						SendMessage(m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
					else
						m_nCursorY++;
				}
				else{
					int cx = m_nCursorX-EDIT_NUM_STARTX;
					int cxa = cx%3;
					if(cxa==0){
						m_nCursorX++;
					}
					else
						m_nCursorX+=2;
				}
			}
		}
		break;
	default:
		return 0;
	}
	SetCaretPos(m_nCursorX*m_nCharWidth, m_nCursorY*m_nCharHeight);
	return 0;
}


int My_EditView::CharKeyDown(char wp){
	int n;
	if(wp>='A' && wp<='F'){
		n=wp-('A'-0x0a);
	}	
	else if(wp>='a' && wp<='f'){
		n=wp-('a'-0x0a);
	}	
	else if(wp>='0' && wp<='9'){
		n=wp-'0';
	}
	else {
		return 0;
	}
	if(m_nCursorX >=EDIT_NUM_STARTX && m_nCursorX <=EDIT_NUM_ENDX){
		int cx = m_nCursorX-EDIT_NUM_STARTX;
		int cxa = cx%3;
		if(cxa!=2){
			int y = (m_nCursorY + m_nVScrollPos) * 0x10;
			int address = cx/3+y;
			if(address >= m_nFileLength)
				return 0;
			if(cxa==0){
				m_pData[address]&=0x0f;
				m_pData[address]|=(unsigned char)(n<<4);
			}
			else{
				m_pData[address]&=0xf0;
				m_pData[address]|=(unsigned char)(n);
			}
			HDC hdc = GetDC(m_hWnd);
			DrawBinEdit_1byte(hdc);
			ReleaseDC(m_hWnd, hdc);
			if(GetCursorAddress() == m_nFileLength-1 && cxa!=0)
				return 1;
			m_nCursorX++;
			if(m_nCursorX>=EDIT_NUM_ENDX){
				m_nCursorX=EDIT_NUM_STARTX;
				m_nCursorY++;
				if(m_nCursorY>=m_cyClient/m_nCharHeight){
					SendMessage(m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
				}
			}
			else if((m_nCursorX-EDIT_NUM_STARTX)%3==2){
				m_nCursorX++;
			}
			SetCaretPos(m_nCursorX*m_nCharWidth, m_nCursorY*m_nCharHeight);
		}
	}
	return 1;
}


void My_EditView::SetMemPointer(unsigned char *pmem, int memsize){
	m_nFileLength = memsize;
	m_pData = pmem;
	SetScrollInfoParam();
}


void My_EditView::SetColorDisplay(int Bytes, int Num){
	m_nColorDispBytes = Bytes;
	m_nColorDispNum = Num;
}















/// nes


My_NES_EditView::My_NES_EditView(HWND hWnd, HINSTANCE hInst, RECT *rect, LOGFONT *pLogFont, unsigned char **pmem){
	m_hWnd = hWnd;
	m_hInst= hInst;
	m_EnableCarat = 0;

	m_hFont			= NULL;
	m_nPageMaxLines	=0;
	m_nPageMaxCols	=0;
	m_nNumLines		=0;
	m_nFileLength	=0;
	m_nDataMem		=0;

	m_nVScrollPos	=0;
	m_nHScrollPos	=0;
	m_nVScrollMax	=0;
	m_nHScrollMax	=0;
	m_nCursorX = EDIT_NUM_STARTX;
	m_nCursorY = 0;
	for(int i=0;i<8;i++){
		m_pData[i] = NULL;
	}

	m_nFileLength = 0x10000;
	m_nColorDispBytes= 0;
	m_nColorDispNum = 0;


	SetFont(pLogFont);
	SetWindowSize(rect->right-rect->left, rect->bottom-rect->top);
	SetMemPointer(pmem);

}


My_NES_EditView::~My_NES_EditView(){
	if(m_EnableCarat){
		HideCaret(m_hWnd);
		DestroyCaret();
		m_EnableCarat=0;
	}
	if(m_hFont){
		DeleteObject(m_hFont);
		m_hFont=NULL;
	}
}


void My_NES_EditView::SetFont(LOGFONT *pLogFont){
	TEXTMETRIC  tm;
	if(m_hFont){
		DeleteObject(m_hFont);
		m_hFont=NULL;
	}
	m_hFont = CreateFontIndirect(pLogFont);
	HDC hdc = GetDC(m_hWnd);
	HFONT hOldFont;
	hOldFont = (HFONT)SelectObject(hdc, m_hFont);
	GetTextMetrics(hdc, &tm);
	SelectObject(hdc, hOldFont);
	ReleaseDC(m_hWnd, hdc);
	m_nCharWidth = tm.tmAveCharWidth;
	m_nCharHeight = tm.tmHeight;
	SetScrollInfoParam();
}


void My_NES_EditView::SetScrollInfoParam(){
	SCROLLINFO sInfo;

	m_nNumLines = m_nFileLength/0x10;
	m_nVScrollMax = m_nNumLines-(m_cyClient/m_nCharHeight);
	m_nHScrollMax = m_cxClient/m_nCharWidth-EDIT_CHARWIDTH;

//	m_nScreenDispBytes = m_nFileLength-(m_nVScrollPos*0x10);

	sInfo.cbSize = sizeof SCROLLINFO;
	sInfo.fMask = /*SIF_PAGE |*/ SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
	sInfo.nMin	= 0;
	sInfo.nMax	= m_nVScrollMax;
	sInfo.nPos	= m_nVScrollPos;
	sInfo.nPage	= m_nVScrollMax; //min(nPageMaxLines, nVScrollMax+1);
	SetScrollInfo (m_hWnd, SB_VERT, &sInfo, TRUE);

	sInfo.cbSize = sizeof SCROLLINFO;
	sInfo.fMask = /*SIF_PAGE |*/ SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
	sInfo.nMin	= 0;
	sInfo.nMax	= m_nHScrollMax;
	sInfo.nPos	= m_nHScrollPos;
	sInfo.nPage	= m_nHScrollMax; //min(nPageMaxLines, nVScrollMax+1);
//	SetScrollInfo (m_hWnd, SB_HORZ, &sInfo, TRUE);

}



void My_NES_EditView::DrawBinEdit(HDC hdc){
	int yMax,xMax,y, x, xpos, cflag, xrem, yrem, nrem, address;
	char str[256];

//	if(!m_pData)
//		return;
	cflag = m_EnableCarat;
	if(cflag)
		HideCaret(m_hWnd);

	nrem = m_nFileLength-(m_nVScrollPos*0x10);
	yrem = nrem/0x10;
	xrem = nrem%0x10;

//	nwidth = m_nCharWidth*3;
	yMax = m_cyClient/m_nCharHeight;
	if(yMax > yrem)
		yMax = yrem;
	xMax = 16; //m_cxClient/(m_nCharWidth*EDIT_CHARWIDTH);
	xpos = m_nHScrollPos*m_nCharWidth;
//	ypos = m_nVScrollPos*(m_nCharHeight+m_nLineSpaceHeight);
//	yadd = m_nVScrollPos;
	HFONT hOldFont;
//	SetBkMode(hdc, 0);
//	SetTextColor(hdc, RGB(255, 255, 255));
	hOldFont = (HFONT)SelectObject(hdc, m_hFont);
	address = (m_nVScrollPos)*16;
	for(y = 0; y<yMax; y++, address+=0x10){
		int i=0;
		wsprintf(str, " %06X: ", (m_nVScrollPos+y)*0x10);
		SetTextColor(hdc, RGB(0, 0, 0));
		TextOutA(hdc, xpos, (y*m_nCharHeight), str, strlen(str));
		i=9;
		{
			for(x=0; x<16; x++, i+=3){
//				wsprintf(&str[i], "%02X ", m_pData[(m_nVScrollPos+y)*16+x]);
				wsprintf(str, "%02X ", GetDataByte(address+x));
				SetTextColor(hdc, RGB(0, 0, 0));
				switch(m_nColorDispBytes){
					case 1:
						if(m_nColorDispNum == GetDataByte(address+x))
							SetTextColor(hdc, RGB(255, 0, 255));
						break;
					case 2:
						if(address+1 < m_nFileLength && m_nColorDispNum == GetDataWord(address+x))
							SetTextColor(hdc, RGB(255, 0, 255));
						break;
					case 3:
						if(address+2 < m_nFileLength && m_nColorDispNum == GetDataDWord(address+x))
							SetTextColor(hdc, RGB(255, 0, 255));
						break;
					case 4:
						if(address+3 < m_nFileLength && m_nColorDispNum == GetDataDWord(address+x))
							SetTextColor(hdc, RGB(255, 0, 255));
						break;
				}
				TextOutA(hdc, xpos+(m_nCharWidth*i), (y*m_nCharHeight), str, strlen(str));
			}
		}
#if 1
		{
			i = 0;
			unsigned char t;
			wsprintf(&str[i], " : ");
			i+=3;
			SetTextColor(hdc, RGB(0, 0, 0));
			for(x=0; x<16; x++, i++){
				t = GetDataByte((m_nVScrollPos+y)*16+x);
				if(t>=0x80 || t<0x20)
					t=0x20;
				str[i] = t;
			}
			str[i]=0;
			TextOutA(hdc, xpos+(EDIT_NUM_ENDX*m_nCharWidth), (y*m_nCharHeight), str, strlen(str));
		}

#endif
//		TextOutA(hdc, xpos, (y*m_nCharHeight), str, strlen(str));
	}
	SelectObject(hdc, hOldFont);
	if(cflag)
		ShowCaret(m_hWnd);
}


void My_NES_EditView::DrawBinEdit_1byte(HDC hdc){
	int y, x, cflag;
	char str[16];

	HFONT hOldFont;
//	if(!m_pData)
//		return;
	cflag = m_EnableCarat;
	if(cflag)
		HideCaret(m_hWnd);

//	SetBkMode(hdc, 0);
//	SetTextColor(hdc, RGB(255, 255, 255));
	hOldFont = (HFONT)SelectObject(hdc, m_hFont);
	x = ((m_nCursorX-EDIT_NUM_STARTX)/3*3+EDIT_NUM_STARTX) * m_nCharWidth;
	y = m_nCursorY * m_nCharHeight;

	int cx = m_nCursorX-EDIT_NUM_STARTX;
	int cxa = cx%3;
	int cy = (m_nCursorY + m_nVScrollPos) * 0x10;

	wsprintf(str, "%02X ", GetDataByte(cx/3+cy));
	TextOutA(hdc, x, y, str, strlen(str));
	SelectObject(hdc, hOldFont);
	if(cflag)
		ShowCaret(m_hWnd);
}


void My_NES_EditView::SetWindowSize(int w, int h){
	m_cxClient = w;
	m_cyClient = h;
//	m_cxClient -= (h%m_nCharHeight);
	SetScrollInfoParam();
	if(m_nVScrollMax<=0 && m_nVScrollPos){
		m_nVScrollPos = 0;
	}
	if(m_nCursorY>=m_cyClient/m_nCharHeight){
		m_nCursorY = m_cyClient/m_nCharHeight-1;
	}
	if(m_EnableCarat)
		SetCaretPos(m_nCursorX*m_nCharWidth, m_nCursorY*m_nCharHeight);
}


void My_NES_EditView::LButtonDown(int x, int y){
	x/=m_nCharWidth;
	y/=m_nCharHeight;
	if(x>=EDIT_NUM_STARTX && x<=EDIT_NUM_ENDX){
		int cx = x-EDIT_NUM_STARTX;
		if(cx%3!=2){
			m_nCursorX = x;
			m_nCursorY = y;
			SetCaretPos(x*m_nCharWidth, y*m_nCharHeight);
		}
	}
}


void My_NES_EditView::KillFocus(){
	if(m_EnableCarat){
		HideCaret(m_hWnd);
		DestroyCaret();
		m_EnableCarat = 0;
	}
}


void My_NES_EditView::SetFocus(){
	if(!m_EnableCarat){
		CreateCaret(m_hWnd, 0, m_nCharWidth, m_nCharHeight);
		SetCaretPos(m_nCursorX * m_nCharWidth, m_nCursorY * m_nCharHeight);
		SetCaretBlinkTime(500);
		ShowCaret(m_hWnd);
		m_EnableCarat = 1;
	}
}


void My_NES_EditView::VScroll(DWORD wp){
	int ScrollInc=0;

	switch (LOWORD(wp)){
		case SB_LINEUP:
			ScrollInc = -1;
			break;
		case SB_LINEDOWN:
			ScrollInc = 1;
			break;
		case SB_PAGEUP:
			ScrollInc = -8;
			break;
		case SB_PAGEDOWN:
			ScrollInc = 8;
			break;
		case SB_THUMBPOSITION:
			ScrollInc = HIWORD(wp) - m_nVScrollPos;
			break;
		case SB_THUMBTRACK:
			ScrollInc = HIWORD(wp) - m_nVScrollPos;
			break;
			
		default:
			break;
	}
	ScrollInc = max(-(int)m_nVScrollPos, min((int)ScrollInc, (int)((m_nVScrollMax /*- nPageMaxCols + 1*/) - m_nVScrollPos)));
				/*
				if(m_nVScrollPos+ScrollInc > m_nVScrollMax - m_nNumLines+1){
				m_nVScrollPos = m_nVScrollMax - m_nNumLines+1;
				}
				else if(m_nVScrollPos+ScrollInc < 0)
				ScrollInc = 0;
				else
				m_nVScrollPos += ScrollInc;
				*/
	if(ScrollInc){
		m_nVScrollPos += ScrollInc;
		if(ScrollInc>0){
			m_nCursorY-=ScrollInc;
			if(m_nCursorY<0)
				m_nCursorY=0;
		}
		else{
			m_nCursorY+=abs(ScrollInc);
			if(m_nCursorY>=m_cyClient/m_nCharHeight){
				m_nCursorY = m_cyClient/m_nCharHeight-1;
			}
		}
		if(ScrollInc > 0){
			RECT arse;
//			SetRect(&arse, 0, m_cyClient, m_cxClient, m_cyClient-(ScrollInc*m_nCharHeight));
			SetRect(&arse, 0, 0, m_cxClient, m_cyClient);
			InvalidateRect(m_hWnd, &arse, FALSE);
		}
		else{
			RECT arse;
//			SetRect(&arse, 0, m_nNumLines*m_nCharHeight, m_cxClient, m_cyClient);
			SetRect(&arse, 0, m_cyClient-m_nCharHeight*abs(ScrollInc), m_cxClient, m_cyClient);
			InvalidateRect(m_hWnd, &arse, FALSE);
		}
//		InvalidateRect(m_hWnd, NULL, TRUE);
		SetCaretPos(m_nCursorX*m_nCharWidth, m_nCursorY*m_nCharHeight);
		SCROLLINFO sInfo;
		sInfo.cbSize = sizeof SCROLLINFO;
		sInfo.fMask = /*SIF_PAGE |*/ SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
		sInfo.nMin	= 0;
		sInfo.nMax	= m_nVScrollMax;
		sInfo.nPos	= m_nVScrollPos;
		sInfo.nPage	= m_nVScrollMax; //min(nPageMaxLines, nVScrollMax+1);
		//					SetScrollPos(hWnd, SB_VERT, y, TRUE);
		SetScrollInfo (m_hWnd, SB_VERT, &sInfo, TRUE);
		ScrollWindowEx(m_hWnd, 0, -m_nCharHeight * ScrollInc, NULL, NULL,NULL,NULL, SW_INVALIDATE);
		UpdateWindow(m_hWnd);


//		InvalidateRect(m_hWnd, NULL, TRUE);
	}
}


int My_NES_EditView::GetCursorAddress(){
	int cx = (m_nCursorX-EDIT_NUM_STARTX)/3;
	int cy = (m_nCursorY + m_nVScrollPos) * 0x10;
	return cx+cy;
}


int My_NES_EditView::KeyDown(char wp){
	switch(wp){
	case VK_UP:
		if(m_nCursorY)
			m_nCursorY--;
		else{
			if(m_nVScrollPos)
				SendMessage(m_hWnd, WM_VSCROLL, SB_LINEUP, 0);
		}
		break;
	case VK_DOWN:
		{
			int nrem = m_nFileLength-(m_nVScrollPos*0x10);
			int yrem = nrem/0x10;
			int xrem = nrem%0x10;
			if(!(m_nCursorY == yrem-1 && m_nCursorX < xrem)){
				if(m_nCursorY>=m_cyClient/m_nCharHeight)
					SendMessage(m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
				else if(m_nCursorY != yrem-1)
					m_nCursorY++;
			}
		}
		break;
	case VK_LEFT:
		if(m_nCursorX != EDIT_NUM_STARTX){
			int cx = m_nCursorX-EDIT_NUM_STARTX;
			int cxa = cx%3;
			if(cxa==0){
				m_nCursorX-=2;
			}
			else
				m_nCursorX--;
		}
		else{
			if(m_nCursorY)
				m_nCursorY--;
			else{
				if(m_nVScrollPos)
					SendMessage(m_hWnd, WM_VSCROLL, SB_LINEUP, 0);
			}
			m_nCursorX = EDIT_NUM_ENDX-1;
		}
		break;
	case VK_RIGHT:
		{
			int nrem = m_nFileLength-(m_nVScrollPos*0x10);
			int yrem = nrem/0x10;
			int xrem = nrem%0x10;
			if(!(m_nCursorY == yrem-1 && m_nCursorX < xrem+1)){
				if(m_nCursorX == EDIT_NUM_ENDX-1){
					m_nCursorX = EDIT_NUM_STARTX;
					if(m_nCursorY>=m_cyClient/m_nCharHeight)
						SendMessage(m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
					else
						m_nCursorY++;
				}
				else{
					int cx = m_nCursorX-EDIT_NUM_STARTX;
					int cxa = cx%3;
					if(cxa==0){
						m_nCursorX++;
					}
					else
						m_nCursorX+=2;
				}
			}
		}
		break;
	default:
		return 0;
	}
	SetCaretPos(m_nCursorX*m_nCharWidth, m_nCursorY*m_nCharHeight);
	return 0;
}


int My_NES_EditView::CharKeyDown(char wp){
	int n;
	if(wp>='A' && wp<='F'){
		n=wp-('A'-0x0a);
	}	
	else if(wp>='a' && wp<='f'){
		n=wp-('a'-0x0a);
	}	
	else if(wp>='0' && wp<='9'){
		n=wp-'0';
	}
	else {
		return 0;
	}
	if(m_nCursorX >=EDIT_NUM_STARTX && m_nCursorX <=EDIT_NUM_ENDX){
		int cx = m_nCursorX-EDIT_NUM_STARTX;
		int cxa = cx%3;
		if(cxa!=2){
			int y = (m_nCursorY + m_nVScrollPos) * 0x10;
			int address = cx/3+y;
			if(address >= m_nFileLength)
				return 0;
			if(cxa==0){
				unsigned char Data = GetDataByte(address);
				Data&=0x0f;
				Data|=(unsigned char)(n<<4);
				SetDataByte(address, Data);
			}
			else{
				unsigned char Data = GetDataByte(address);
				Data&=0xf0;
				Data|=(unsigned char)(n);
				SetDataByte(address, Data);
			}
			HDC hdc = GetDC(m_hWnd);
			DrawBinEdit_1byte(hdc);
			ReleaseDC(m_hWnd, hdc);
			if(GetCursorAddress() == m_nFileLength-1 && cxa!=0)
				return 1;
			m_nCursorX++;
			if(m_nCursorX>=EDIT_NUM_ENDX){
				m_nCursorX=EDIT_NUM_STARTX;
				m_nCursorY++;
				if(m_nCursorY>=m_cyClient/m_nCharHeight){
					SendMessage(m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
				}
			}
			else if((m_nCursorX-EDIT_NUM_STARTX)%3==2){
				m_nCursorX++;
			}
			SetCaretPos(m_nCursorX*m_nCharWidth, m_nCursorY*m_nCharHeight);
		}
	}
	return 1;
}


void My_NES_EditView::SetMemPointer(unsigned char **pmem){
	int i;
	for(i=0; i<8; i++){
		m_pData[i] = pmem[i];
	}
	SetScrollInfoParam();
}


void My_NES_EditView::SetColorDisplay(int Bytes, int Num){
	m_nColorDispBytes = Bytes;
	m_nColorDispNum = Num;
}


unsigned char My_NES_EditView::GetDataByte(int Address){
	int Page = Address>>13;
	Address&=0x01fff;
	if(m_pData[Page]==NULL)
		return 0;
	return m_pData[Page][Address];
}


WORD My_NES_EditView::GetDataWord(int Address){
	return (GetDataByte(Address) | GetDataByte(Address+1) << 8);
}


DWORD My_NES_EditView::GetDataDWord(int Address){
	return (GetDataByte(Address) | GetDataByte(Address+1) << 8| GetDataByte(Address+3) << 16| GetDataByte(Address+1) << 24);
}


void My_NES_EditView::SetDataByte(int Address, unsigned char Data){
	int Page = Address>>13;
	Address&=0x01fff;
	if(m_pData[Page]==NULL)
		return;
	m_pData[Page][Address] = Data;
}

