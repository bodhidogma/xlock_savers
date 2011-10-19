/*-------------
	Rotor - rotor spirograph screen saver port
			from the XLock package.

  Rev History:
	15-dec-95: ported from 16 bit version to Win'95 compliant (paulmcav@silcom.com)
	23-dec-94: ported from xlock package to win16
	01-dec-93: added path from AIXV3 from
			(Tom McConnell, tmcconne@sedona.intel.com)
	11-Nov-90: put into xlock (by Steve AZellers, zellers@sun.com)
	16-oct-90: received from Tom Lawrence (tcl@cs.brown.edu: 'flight' simulator
  -------------*/

#include <windows.h>
#include <scrnsave.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "resource.h"
#include "rotor.h"


LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	HANDLE hPalMem;

	flightstruct *fs = &flights;
	int		x, R,G,B;
	struct elem *pelem;
	
	switch (message)
	{
	case WM_CREATE:
		vLoadStrings();
		GetIniSettings();

		wTimer = SetTimer(hWnd, ID_TIMER, wElapse, NULL);	// create timer for image refresh

		GetClientRect(hWnd, &rc);
		fs->centerx = rc.right / 2;
		fs->centery = rc.bottom / 2;
		fs->smallscrn = (rc.right < 320);

		fs->num = iRotorCnt;

		if (fs->elements == NULL) {							// allocate memory
			if ((fs->elements = (struct elem *)
					malloc(sizeof(struct elem) * fs->num)) == 0) {
						exit(1);
			}
			if ((fs->savex = (LPINT)malloc(sizeof(INT) * iSegLen)) == 0)
					exit(1);
			if ((fs->savey = (LPINT)malloc(sizeof(INT) * iSegLen)) == 0)
					exit(1);
		}
		(void) memset(fs->savex,0, sizeof(fs->savex));

		pelem = fs->elements;

		for (x = fs->num; --x >= 0; pelem++) {			// define drift for segments
			pelem->radius_drift_max = (FLOAT)1.0;
			pelem->radius_drift_now = (FLOAT)1.0;

			pelem->end_radius = (FLOAT)100.0;			// define end radius

			pelem->ratio_drift_max = (FLOAT)1.0;		// define ratio drift
			pelem->ratio_drift_now = (FLOAT)1.0;
			pelem->end_ratio = (FLOAT)10.0;
		}

		srand((unsigned)time(NULL));					// seed random # generator

		fs->rotor = 0;
		fs->prev = 1;
		fs->lastx = fs->centerx;
		fs->lasty = fs->centery;
		fs->angle = (float)(rand() % (long)MAXANGLE) / 3;	// define initial angle
		fs->r = iClrStart;									// set start color value
		fs->firsttime = TRUE;								// first time through

		hPalMem = LocalAlloc(LMEM_FIXED, sizeof(LOGPALETTE) + PALSIZE * sizeof(PALETTEENTRY));
		if (!hPalMem) {
			return -1;
		}
		pLogPal = (NPLOGPALETTE) LocalLock(hPalMem);
		pLogPal->palVersion = 0x300;
		pLogPal->palNumEntries = PALSIZE;
		for (x=0, R=255, G=0, B=0; x < PALSIZE; x++) {
			cycle_colors(&R, &G, &B, 255, 0, 6);

			pLogPal->palPalEntry[x].peRed = LOBYTE(R);
			pLogPal->palPalEntry[x].peGreen = LOBYTE(G);
			pLogPal->palPalEntry[x].peBlue = LOBYTE(B);
			pLogPal->palPalEntry[x].peFlags = PC_RESERVED;
		}
		hPal = CreatePalette((LPLOGPALETTE)pLogPal);
		if (!hPal) {
			return -1;
		}
		break;

	case WM_ERASEBKGND:
        GetClientRect(hWnd,&rc);
		FillRect((HDC)wParam,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
        return 0L;
		break;

	case WM_TIMER:
		MoveImage(hWnd);
		break;
	
	case WM_DESTROY:
		if( wTimer )							// if timer was defined
			KillTimer(hWnd, ID_TIMER);			// then kill it
		free( flights.savey );					// free any allocated memory
		free( flights.savex );
		free( flights.elements );

		if (hPal) {								// free palette memory
			DeleteObject(hPal);
		}
		break;
	}

	return DefScreenSaverProc(hWnd, message, wParam, lParam);
}


BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND	hOK,
			hScrl;
	INT		iScrlPos;
	
	hScrl = GetDlgItem(hDlg, IDC_ROTORSPEED);

	switch (message)
	{
	case WM_INITDIALOG:
		hOK = GetDlgItem(hDlg, IDOK);
		vLoadStrings();
		GetIniSettings();
		bInitDlgItems(hDlg, hScrl);
		return TRUE;

	case WM_HSCROLL:
		iScrlPos = MIN_SPEED-wElapse;
			switch ((int)LOWORD(wParam))			// message from scrollbar
			{
				case SB_LINERIGHT:					// determine specific movement type
					iScrlPos += 5;
					break;
				case SB_LINELEFT:
					iScrlPos -= 5;
					break;
				case SB_PAGERIGHT:				
					iScrlPos += 20;
					break;
				case SB_PAGELEFT:
					iScrlPos -= 20;
					break;
				case SB_THUMBPOSITION:			// grabbed thumb and moved it
					iScrlPos = (short int)HIWORD(wParam);
					break;
			}
			iScrlPos = min(MIN_SPEED,max(1,iScrlPos));		// check to see within desired range
			if (iScrlPos != MIN_SPEED-(int)wElapse)
			{
				wElapse = MIN_SPEED-iScrlPos;								// save new position
				SetScrollPos((HWND)lParam, SB_CTL, iScrlPos, TRUE);			// display position				
			}
			break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{		
		case IDC_DEFAULT:
			iSegLen = DEF_SEGLEN;
			iSegCnt = DEF_SEGCNT;
			iRotorCnt = DEF_ROTORCNT;
			iClrStart = DEF_CLRSTART;
			iClrInc = DEF_CLRINC;
			wElapse = DEF_SPEED;				

			bInitDlgItems(hDlg, hScrl);				
			return TRUE;

		case IDOK:
			bGetDlgItems(hDlg);
			SetIniSettings();

		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam) == IDOK);
			return TRUE;
		}
	}
	return FALSE;
}


BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
	return TRUE;
}


void MoveImage(HWND hWnd) 
{   
	HDC		hDC;
	HPEN	hpenColor, hpenBlack, hpenOld;		// define some pens

	flightstruct *fs = &flights;
	struct elem *pelem;
	
	int		thisx,
			thisy;
	int		i,
			rp;
	int		x1, y1,
			x2, y2;
			
	hDC = GetDC(hWnd);						// get HDC to this window

	SelectPalette(hDC, hPal, 0);
	RealizePalette(hDC);

	hpenBlack = (HPEN)GetStockObject(BLACK_PEN);	// get a black pen			
	hpenOld = (HPEN)SelectObject(hDC, hpenBlack);
	        
	for (rp = 0; rp < iSegCnt; rp++) {		// loop for REPS times
		thisx = fs->centerx;				// save center x & y
		thisy = fs->centery;
		
		for (i= fs->num, pelem = fs->elements; --i >= 0; pelem++) {		// loop elements
			if (pelem->radius_drift_max <= pelem->radius_drift_now) {	// track radius drift
				pelem->start_radius = pelem->end_radius;				// start = end
				pelem->end_radius =										// define end radius
					(float)(rand() % 40000) / (float)100.0 - (float)200.0;
				pelem->radius_drift_max =								// define drift max
					(float)(rand() % 100000) + (float)10000.0;
				pelem->radius_drift_now = (float)0.0;					// reset drift now
			}
			if (pelem->ratio_drift_max <= pelem->ratio_drift_now) {		// track ratio drift
				pelem->start_ratio = pelem->end_ratio;					// start = end
				pelem->end_ratio =										// define end ratio
					(float)(rand() % 2000) / (float)100.0 - (float)10.0;
				pelem->ratio_drift_max =								// define drift max
					(float)(rand() % 100000) + (float)1000.0;
				pelem->ratio_drift_now = (float)0.0;					// reset drift now
			}
			pelem->ratio = pelem->start_ratio +							// set ratio
				(pelem->end_ratio - pelem->start_ratio) /
				pelem->ratio_drift_max * pelem->ratio_drift_now;
			pelem->angle = fs->angle * pelem->ratio;					// set angle
			pelem->radius = pelem->start_radius +						// set radius
				(pelem->end_radius - pelem->start_radius) /
				pelem->radius_drift_max * pelem->radius_drift_now;
				
			thisx += (int)(cos(pelem->angle) * pelem->radius);			// get X & Y position
			thisy += (int)(sin(pelem->angle) * pelem->radius);
			
			pelem->ratio_drift_now += (float)1.0;					// increment ratio drift
			pelem->radius_drift_now += (float)1.0;                  // increment radius drift
		}
		if (fs->firsttime)
			fs->firsttime = FALSE;
		else {                             							// if firsttime
			x1 = (int)fs->savex[fs->rotor];							// save previous positions
			y1 = (int)fs->savey[fs->rotor];
			x2 = (int)fs->savex[fs->prev];
			y2 = (int)fs->savey[fs->prev];
	
			SCALEIFSMALL();

			MoveToEx(hDC, x1, y1, NULL);							// erase the old lines
			LineTo(hDC, x2, y2);
			SelectObject(hDC, hpenOld);								// select the old pen
			
			if ((fs->r += iClrInc) > (PALSIZE-21))
				fs->r = 0;

			hpenColor = CreatePen(PS_SOLID, 1, PALETTEINDEX(fs->r));
			hpenOld = (HPEN)SelectObject(hDC, hpenColor);
			
			x1 = fs->lastx;											// get new positions
			y1 = fs->lasty;
			x2 = thisx;
			y2 = thisy;

			SCALEIFSMALL();
			
			MoveToEx(hDC, x1, y1, NULL);							// draw thew new line
			LineTo(hDC, x2, y2);
			SelectObject(hDC, hpenOld);
			DeleteObject(hpenColor);								// delete the defined pen
		}
		fs->savex[fs->rotor] = fs->lastx = thisx;					// save old positions
		fs->savey[fs->rotor] = fs->lasty = thisy;
		
		++fs->rotor;												// increment rotor count
		fs->rotor %= iSegLen;
		++fs->prev;
		fs->prev %= iSegLen;
		if (fs->forward) {											// moving forward?
			fs->angle += (float)0.01;								// increment angle
			if (fs->angle >= MAXANGLE) {
				fs->angle = (float)MAXANGLE;
				fs->forward = FALSE;
			}
		} else {													// else moving back
			fs->angle -= (float)0.01;								// decrement angle
			if (fs->angle <= 0) {
				fs->angle = (float)0.0;
				fs->forward = TRUE;
			}
		}
	}	    
	
	ReleaseDC(hWnd, hDC);
}

void SetEditLim(HWND hDlg, int iDlgItem, WORD wLim)
{
	HWND hCtrl;

	hCtrl = GetDlgItem(hDlg, iDlgItem);
	SendMessage(hCtrl, EM_LIMITTEXT, wLim, 0L);
}

BOOL bInitDlgItems(HWND hDlg, HWND hScrl)
{	
	SetDlgItemInt(hDlg,IDC_SEGCOUNT, iSegCnt, FALSE);
	SetEditLim(hDlg, IDC_SEGCOUNT, 2);
	SetDlgItemInt(hDlg,IDC_SEGLEN, iSegLen, FALSE);
	SetEditLim(hDlg, IDC_SEGLEN, 3);
	SetDlgItemInt(hDlg,IDC_CLRSTART, iClrStart, FALSE);
	SetEditLim(hDlg, IDC_CLRSTART, 3);
	SetDlgItemInt(hDlg,IDC_CLRINC, iClrInc, FALSE);
	SetEditLim(hDlg, IDC_CLRINC, 2);
	SetDlgItemInt(hDlg,IDC_ROTORCNT, iRotorCnt, FALSE);
	SetEditLim(hDlg, IDC_ROTORCNT, 1);

	SetScrollRange(hScrl, SB_CTL, 1, MIN_SPEED, FALSE);
	SetScrollPos(hScrl, SB_CTL, MIN_SPEED-wElapse, TRUE);

	return TRUE;
}

BOOL bGetDlgItems(HWND hDlg)
{
	BOOL bRtn;

	iSegCnt = GetDlgItemInt(hDlg, IDC_SEGCOUNT, &bRtn, FALSE);
	iSegLen = GetDlgItemInt(hDlg, IDC_SEGLEN, &bRtn, FALSE);
	iClrStart = GetDlgItemInt(hDlg, IDC_CLRSTART, &bRtn, FALSE);
	iClrInc = GetDlgItemInt(hDlg, IDC_CLRINC, &bRtn, FALSE);
	iRotorCnt = GetDlgItemInt(hDlg, IDC_ROTORCNT, &bRtn, FALSE);

	if (iClrStart > 255)
		iClrStart = 255;
	
	return TRUE;
}


void cycle_colors(LPINT r, LPINT g, LPINT b, INT iMaxC, INT iMinC, INT iInc_Color)
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
	// Load Common Strings from stringtable...//
	LoadString(hMainInstance, idsAppName, szAppName, APPNAMEBUFFERLEN);
	LoadString(hMainInstance, IDS_INIFILE, szRegEntry, 30);
	// check to see if should use ini file or registry
	LoadString(hMainInstance, IDS_DESCRIPTION, szDesc, 30);

	LoadString(hMainInstance, IDS_SEGLEN, szSegLen, 30);
	LoadString(hMainInstance, IDS_SEGCOUNT, szSegCnt, 30);
	LoadString(hMainInstance, IDS_ROTORCNT, szRotorCnt, 30);
	LoadString(hMainInstance, IDS_CLRSTART, szClrStart, 30);
	LoadString(hMainInstance, IDS_CLRSPEED, szClrInc, 30);
	LoadString(hMainInstance, IDS_SPEED, szSpeed, 30);
}

BOOL GetIniSettings(void)
{
	wElapse		= SetProfileVal(FALSE, szAppName, szSpeed, DEF_SPEED, szRegEntry);
	iSegLen		= SetProfileVal(FALSE, szAppName, szSegLen, DEF_SEGLEN, szRegEntry);
	iSegCnt		= SetProfileVal(FALSE, szAppName, szSegCnt, DEF_SEGCNT, szRegEntry);
	iRotorCnt	= SetProfileVal(FALSE, szAppName, szRotorCnt, DEF_ROTORCNT, szRegEntry);
	iClrStart	= SetProfileVal(FALSE, szAppName, szClrStart, DEF_CLRSTART, szRegEntry);
	iClrInc		= SetProfileVal(FALSE, szAppName, szClrInc, DEF_CLRINC, szRegEntry);

	return TRUE;
}

BOOL SetIniSettings(void)
{
	SetProfileVal(TRUE, szAppName, szSpeed, wElapse, szRegEntry);
	SetProfileVal(TRUE, szAppName, szSegLen, iSegLen, szRegEntry);
	SetProfileVal(TRUE, szAppName, szSegCnt, iSegCnt, szRegEntry);
	SetProfileVal(TRUE, szAppName, szRotorCnt, iRotorCnt, szRegEntry);
	SetProfileVal(TRUE, szAppName, szClrStart, iClrStart, szRegEntry);
	SetProfileVal(TRUE, szAppName, szClrInc, iClrInc, szRegEntry);

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

