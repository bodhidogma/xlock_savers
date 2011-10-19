/*
	Quix Screen Saver
	--------------------------------------------------------
	History:
		Ported from the xlockmore package to 16-bit Windows
		Ported to Win32 from 16-bit Windows
*/


#include <windows.h> 
#include <scrnsave.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <memory.h>

#include "resource.h"
#include "Quix.h"

LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	RECT rc;
	HANDLE hPalMem;
                                                          
    switch (msg)
    {                
        case WM_CREATE:
        {
        	int		i, R,G,B;
			quixstruct	*qp = &quix;

			vLoadStrings();					// load strings from table
			GetIniSettings();				// get settings from ini file (control.ini)
        	
            wTimer = SetTimer(hWnd, ID_TIMER, wElapse, NULL);	// create timer to move image                        
            GetClientRect(hWnd, &rc);							// find size of screen

			qp->startTime = time(NULL);
			qp->nlines = ((iLines+1) * 2) * iSegs;
			if (!qp->lineq) {
				qp->lineq = (point*)malloc(qp->nlines * sizeof(point));
				(void)memset(qp->lineq, 0, qp->nlines * sizeof(point));

				qp->segs = (l_seg*)malloc((iSegs+1) * sizeof(l_seg));
				(void)memset(qp->segs, 0, (iSegs+1) * sizeof(l_seg));
			}
			qp->width = rc.right;
			qp->height = rc.bottom;
			qp->delta = iWidth;				// line distance

			if (qp->width < 100) {		// icon window
				qp->nlines /=4;
				qp->delta /= 4;
			}
			srand((unsigned)time(NULL));	// seed random w/ timer

			qp->offset = qp->delta / 3;
			qp->last = 0;
			qp->pix = 0;

			for (i = 0; i <= iSegs; i++) {				
				qp->segs[i].x = rand() % qp->width;					// init positions
				qp->segs[i].y = rand() % qp->height;
				qp->segs[i].dx = rand() % qp->delta + qp->offset;
				qp->segs[i].dy = rand() % qp->delta + qp->offset;
			}
			qp->ColorCount = 1;
                        
			// Create a logical palette to play with
			hPalMem = LocalAlloc(LMEM_FIXED,
				sizeof(LOGPALETTE) + PALSIZE * sizeof(PALETTEENTRY));
			if (!hPalMem) {	// error("No memory for palette");
				return -1;
			}
			pLogPal = (NPLOGPALETTE)LocalLock(hPalMem);
			pLogPal->palVersion = 0x300;
			pLogPal->palNumEntries = PALSIZE;
	        for (i=0, R=255,G=0,B=0; i<PALSIZE; i++) {
	        
	        	cycle_colors(&R,&G,&B,255,0,6);
	        	
	            pLogPal->palPalEntry[i].peRed = LOBYTE(R);
	            pLogPal->palPalEntry[i].peGreen = LOBYTE(G);
	            pLogPal->palPalEntry[i].peBlue = LOBYTE(B);
	            pLogPal->palPalEntry[i].peFlags = PC_RESERVED;
	        }
	        hPal = CreatePalette((LPLOGPALETTE)pLogPal);
	        if (!hPal) {
	            //Error("CreatePalette() failed");
	            return -1;
	        }
			break;
        }
        
        case WM_TIMER:                              // BOUNCER-specific
            MoveImage(hWnd);						// move image around
            break;

        case WM_DESTROY:                            // BOUNCER-specific
            if( wTimer )
            	KillTimer(hWnd, ID_TIMER);
			if (hPal)
				DeleteObject(hPal);
			free(quix.segs);
			free(quix.lineq);
            break;

        case WM_ERASEBKGND:
			if (iCLS) {
	            GetClientRect(hWnd,&rc);
	            FillRect((HDC)wParam,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
			}
            return 0L;

        default:
            break;
        }

    return DefScreenSaverProc(hWnd, msg, wParam, lParam);
}

BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND	hScrl;
	int 	iScrlPos;	

	hScrl = GetDlgItem(hDlg, IDC_SPEED);

	switch(message)
	{
		case WM_INITDIALOG: {
			vLoadStrings();
			GetIniSettings();
			bInitDlgItems(hDlg, hScrl);
			return TRUE;
		}
		case WM_HSCROLL: {
			iScrlPos = 100-wElapse;
			switch ((int)LOWORD(wParam))
			{
				case SB_LINERIGHT:					// determine specific movement type
					iScrlPos += 1;
					break;
				case SB_LINELEFT:
					iScrlPos -= 1;
					break;
				case SB_PAGERIGHT:				
					iScrlPos += 10;
					break;
				case SB_PAGELEFT:
					iScrlPos -= 10;
					break;
				case SB_THUMBPOSITION:			// grabbed thumb and moved it
					iScrlPos = (short int)HIWORD(wParam);
					break;
			}
			iScrlPos = min(100,max(1,iScrlPos));		// check to see within desired range
			if (iScrlPos != 100-(int)wElapse)
			{
				wElapse = 100-iScrlPos;										// save new position
				SetScrollPos((HWND)lParam, SB_CTL, iScrlPos, TRUE);			// display position				
			}
			return TRUE;
		}
		case WM_COMMAND: {			
			switch (LOWORD(wParam))
			{
				case IDC_DEFAULTS: {
					wElapse = DEF_SPEEDSET;
					iLines = DEF_LINECOUNT;
					iWidth = DEF_WIDTH;
					iSegs = DEF_SEGS;
					iCLS = DEF_CLS;
					iRainbow = DEF_RAINBOW;
					bInitDlgItems(hDlg, hScrl);
					return TRUE;
				}
				case IDOK: {					
					bGetDlgItems(hDlg);
					SetIniSettings();				
				}
				case IDCANCEL:
                    EndDialog(hDlg,LOWORD(wParam) == IDOK);
					return TRUE;
				default:
					break;
			}
		}
		default:
			break;
	}
	return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst) { return TRUE; }

void MoveImage(HWND hWnd) {   

	HDC		hDC;
	HPEN	hpenColor, hpenBlack, hpenOld;
	quixstruct	*qp = &quix;	
	int			i;
		
	hDC = GetDC(hWnd);									// get HDC to this window
	hpenBlack = (HPEN)GetStockObject(BLACK_PEN);				// get a black pen			
	//hpenWhite = GetStockObject(WHITE_PEN);				// get a black pen			

    SelectPalette(hDC, hPal, 0);
    RealizePalette(hDC);
	hpenOld = (HPEN)SelectObject(hDC, hpenBlack);     			// select the black pen			

	if (!iRainbow) {
		if (qp->ColorCount++ > (PALSIZE-21)) qp->ColorCount = 1;		
		hpenColor = CreatePen(PS_SOLID, 1, PALETTEINDEX(qp->ColorCount));
	}

	for (i = 0; i <= iSegs; i++) {
		qp->segs[i].x += qp->segs[i].dx;
		qp->segs[i].y += qp->segs[i].dy;
		check_bounds(qp, qp->segs[i].x, &qp->segs[i].dx, qp->width);
		check_bounds(qp, qp->segs[i].y, &qp->segs[i].dy, qp->height);
	}			
	
	for (i = 0; i <= (iSegs-1); i++) {
		if (iRainbow) {
			if (qp->ColorCount++ > (PALSIZE-21)) qp->ColorCount = 1;
			hpenColor = CreatePen(PS_SOLID, 1, PALETTEINDEX(qp->ColorCount));
		}

		SelectObject(hDC, hpenColor);						// select a new pen color
		MoveToEx(hDC, qp->segs[i].x, qp->segs[i].y, NULL);
		LineTo(hDC, qp->segs[i+1].x, qp->segs[i+1].y);

		SelectObject(hDC, hpenBlack);
		qp->first = (qp->last + 2) % qp->nlines;
		MoveToEx(hDC, qp->lineq[qp->first].x, qp->lineq[qp->first].y, NULL);// erase the old lines
		LineTo(hDC, qp->lineq[qp->first+1].x, qp->lineq[qp->first+1].y);		

		qp->lineq[qp->last].x = qp->segs[i].x;
		qp->lineq[qp->last].y = qp->segs[i].y;
		if (++qp->last >= qp->nlines) qp->last = 0;
		qp->lineq[qp->last].x = qp->segs[i+1].x;
		qp->lineq[qp->last].y = qp->segs[i+1].y;
		if (++qp->last >= qp->nlines) qp->last = 0;

		if (iRainbow)
			DeleteObject(hpenColor);
	}
		
	SelectObject(hDC, hpenOld);
	if (!iRainbow)
		DeleteObject(hpenColor);						// delete the pen
			
	ReleaseDC(hWnd, hDC);
}

BOOL bInitDlgItems(HWND hDlg, HWND hScrl)
{
	//SetEditLim(hDlg, IDC_NUMGALAXY, 1);

	SetDlgItemInt(hDlg,IDC_LINES, iLines, FALSE);
	SetDlgItemInt(hDlg,IDC_WIDTH, iWidth, FALSE);
	SetDlgItemInt(hDlg,IDC_SEGS, iSegs, FALSE);
	CheckDlgButton(hDlg,IDC_CLS, iCLS);
	CheckDlgButton(hDlg,IDC_RAINBOW, iRainbow);
	SetScrollRange(hScrl, SB_CTL, 1, 100, FALSE);
	SetScrollPos(hScrl, SB_CTL, 100-wElapse, TRUE);

	return TRUE;
}

BOOL bGetDlgItems(HWND hDlg)
{
	BOOL bRtn;

	iLines = GetDlgItemInt(hDlg, IDC_LINES, &bRtn, FALSE);
	iWidth = GetDlgItemInt(hDlg, IDC_WIDTH, &bRtn, FALSE);
	iSegs = GetDlgItemInt(hDlg, IDC_SEGS, &bRtn, FALSE);
	iCLS = IsDlgButtonChecked(hDlg, IDC_CLS);
	iRainbow = IsDlgButtonChecked(hDlg, IDC_RAINBOW);
	return TRUE;
}

// ---------------------------------
// Cycle colors in an RGB pattern
//
void cycle_colors(int *r, int *g, int *b, int iMaxC, int iMinC, int iInc_Color)
{
	if ((*r >= iMaxC) && (*b <= iMinC)) {
		 *r = iMaxC; *b = iMinC; *g += iInc_Color;		
		 }
	if ((*g >= iMaxC) && (*b <= iMinC)) {
		 *g = iMaxC; *b = iMinC; *r -= iInc_Color;
		 }
	if ((*g >= iMaxC) && (*r <= iMinC)) {
		 *g = iMaxC; *r = iMinC; *b += iInc_Color;
		 }
	if ((*b >= iMaxC) && (*r <= iMinC)) {
		 *b = iMaxC; *r = iMinC; *g -= iInc_Color;
		 }
	if ((*b >= iMaxC) && (*g <= iMinC)) {
		 *b = iMaxC; *g = iMinC; *r += iInc_Color;
		 }
	if ((*r >= iMaxC) && (*g <= iMinC)) {
		 *r = iMaxC; *g = iMinC; *b -= iInc_Color;
		 }			
}

void vLoadStrings(void)
{
	// Load Common Strings from stringtable...
	LoadString(hMainInstance, idsAppName, szAppName, APPNAMEBUFFERLEN);
		LoadString(hMainInstance, idsIniFile, szRegEntry, 30);
	// check to see if should use ini file or registry
	LoadString(hMainInstance, IDS_DESCRIPTION, szDesc, 30);
	
	LoadString(hMainInstance, IDS_SPEED, szSpeedName, 20);
	LoadString(hMainInstance, IDS_LINES, szLinesName, 20);
	LoadString(hMainInstance, IDS_WIDTH, szWidthName, 20);
	LoadString(hMainInstance, IDS_SEGS, szSegsName, 20);
	LoadString(hMainInstance, IDS_CLS, szClsName, 20);
	LoadString(hMainInstance, IDS_RAINBOW, szRainbowName, 20);
}

BOOL GetIniSettings(void)
{	
	// Retrieve settings from INI file (note: for compatability with 16 bit version)
    wElapse		= SetProfileVal(FALSE, szAppName, szSpeedName, DEF_SPEEDSET, szRegEntry);
	iLines		= SetProfileVal(FALSE, szAppName, szLinesName, DEF_LINECOUNT, szRegEntry);
	iWidth		= SetProfileVal(FALSE, szAppName, szWidthName, DEF_WIDTH, szRegEntry);
    iSegs		= SetProfileVal(FALSE, szAppName, szSegsName, DEF_SEGS, szRegEntry);
    iCLS		= SetProfileVal(FALSE, szAppName, szClsName, DEF_CLS, szRegEntry);
    iRainbow	= SetProfileVal(FALSE, szAppName, szRainbowName, DEF_RAINBOW, szRegEntry);
	
	return TRUE;
}

BOOL SetIniSettings(void)
{
	SetProfileVal(TRUE, szAppName, szSpeedName, wElapse, szRegEntry);
	SetProfileVal(TRUE, szAppName, szLinesName, iLines, szRegEntry);
	SetProfileVal(TRUE, szAppName, szWidthName, iWidth, szRegEntry);
	SetProfileVal(TRUE, szAppName, szSegsName, iSegs, szRegEntry);
	SetProfileVal(TRUE, szAppName, szClsName, iCLS, szRegEntry);
	SetProfileVal(TRUE, szAppName, szRainbowName, iRainbow, szRegEntry);
	
	return TRUE;
}

void WriteProfileInt(LPCTSTR szSection, LPCTSTR szKey, int i, LPCTSTR szRoot)
{
    char achBuf[40];

    /* write out as unsigned because GetPrivateProfileInt() can't
     * cope with signed values!
     */
    wsprintf(achBuf, "%u", i);
    WritePrivateProfileString(szSection, szKey, achBuf, szRoot);
}

int SetProfileVal(BOOL iMode, LPCTSTR szApp, LPCTSTR szKey, INT iDefVal, LPCTSTR szRoot)
{
	// if iMode = TRUE, then write the given value
	// if       = FALSE, then read the from given key with default value
	int	iCtrl = 0;

	iCtrl = (szRoot[7]=='.');	// Control.ini

	if (iMode == TRUE) {		// Write the given value
		if (iCtrl) {
			WriteProfileInt(szApp, szKey, iDefVal, szRoot);
		}
	} else {					// Read the given value
		if (iCtrl) {
			iDefVal = GetPrivateProfileInt(szApp, szKey, iDefVal, szRoot);
		}
	}
	return iDefVal;
}
