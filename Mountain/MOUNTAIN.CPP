/*
	Mountian.c  - Ported from the x-lock collection

	1995 Pascal Pensa

*/


#include <windows.h> 
#include <scrnsave.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "resource.h"
#include "Mountain.h"

#define BATCH 20

static void spread(int m[WORLDWIDTH][WORLDWIDTH], int x,int y)
{
	int x2,y2;
	int h = m[x][y];

	for(y2=y-1; y2<=y+1; y2++)
		for(x2=x-1; x2<=x+1; x2++)
			if(x2 >= 0 && y2 >= 0 && x2 < WORLDWIDTH && y2 < WORLDWIDTH)
				m[x2][y2] = (m[x2][y2] + h) / 2;
}


LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	HANDLE hPalMem;
    mountainstruct *mp = &mountains;
    int		i,R,G,B;
	                                                          
    switch (msg)
    {
    case WM_CREATE:
		vLoadStrings();					// load strings from table
		GetIniSettings();				// get settings from ini file (control.ini)
        
        wTimer = SetTimer(hWnd, ID_TIMER, wElapse, NULL);	// create timer to move image
                    
        GetClientRect(hWnd, &rc);							// find size of screen
        mp->width = rc.right;
        mp->height = rc.bottom;
        mp->pixelmode = (mp->width + mp->height < 200);

        mp->time = 0;

        //if (!sp->segs) {
        //    sp->segs = (lseg *)malloc(sizeof(lseg) * sp->beecount);
        //    sp->old_segs = (lseg *)malloc(sizeof(lseg) * sp->beecount);
        //    sp->x = (short *)malloc(sizeof(short) * sp->beecount * TIMES);
        //    sp->y = (short *)malloc(sizeof(short) * sp->beecount * TIMES);
        //    sp->xv = (short *)malloc(sizeof(short) * sp->beecount);
        //    sp->yv = (short *)malloc(sizeof(short) * sp->beecount);
        //}
        
		// Create a logical palette to play with
		hPalMem = LocalAlloc(LMEM_FIXED,
							sizeof(LOGPALETTE)
							+ PALSIZE * sizeof(PALETTEENTRY));
		if (!hPalMem) {
			// error("No memory for palette");
			return -1;
		}
		pLogPal = (NPLOGPALETTE) LocalLock(hPalMem);
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
        
    case WM_TIMER:                              // 
		if (iCls) {
	        GetClientRect(hWnd,&rc);
	        FillRect((HDC)wParam,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
		}
        srand((unsigned int)time( NULL ));
        MoveImage(hWnd);						// 
        break;

    case WM_DESTROY:                            // 
                    
        if( wTimer )
            KillTimer(hWnd, ID_TIMER);
		if (hPal)
			DeleteObject(hPal);
        
        break;

    case WM_ERASEBKGND:
        return 0L;

    default:
        break;
    }

    return DefScreenSaverProc(hWnd, msg, wParam, lParam);
}


BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND	hOK,
			hScrl;
	int 	iScrlPos;	

	hScrl = GetDlgItem(hDlg, IDC_SPEED);

	switch(message)
	{
	case WM_INITDIALOG:
		hOK = GetDlgItem(hDlg, IDOK);
		vLoadStrings();
		GetIniSettings();
		bInitDlgItems(hDlg, hScrl);
		return TRUE;
	
	case WM_HSCROLL:
		iScrlPos = MIN_SPEED-wElapse;
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
		iScrlPos = min(MIN_SPEED,max(1,iScrlPos));		// check to see within desired range
		if (iScrlPos != MIN_SPEED-(int)wElapse)
		{
			wElapse = MIN_SPEED-iScrlPos;										// save new position
			SetScrollPos((HWND)lParam, SB_CTL, iScrlPos, TRUE);			// display position				
		}
		return TRUE;
		
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_DEFAULT:
			wElapse = DEF_SPEED;
			//iBeeAcc = DEF_BEEACC;
			//iBeeVel = DEF_BEEVEL;
			//iBeeCount = DEF_BEECOUNT;
			//iWaspAcc = DEF_WASPACC;
			//iWaspVel = DEF_WASPVEL;
			iCls = DEF_CLS;

			bInitDlgItems(hDlg, hScrl);
			return TRUE;
		
		case IDOK:
			bGetDlgItems(hDlg);
			SetIniSettings();				
		
		case IDCANCEL:
            EndDialog(hDlg,LOWORD(wParam) == IDOK);
			return TRUE;		
		}
	}
	return FALSE;
}


BOOL WINAPI RegisterDialogClasses(HANDLE hInst) { return TRUE; }


void MoveImage(HWND hWnd)
{
	HDC		hDC;
	HBRUSH	hbColor, hbOld;
	//int		b;
	int		i,x,y,offset;
	mountainstruct *mp = &mountains;
	
	hDC = GetDC(hWnd);									// get HDC to this window
			
    SelectPalette(hDC, hPal, 0);
    RealizePalette(hDC);

	for(y=0; y < WORLDWIDTH; y++)
		for(x=0; x < WORLDWIDTH; x++)
			mp->h[x][y] = 0;

	for(i=0; i < BATCH; i++)
		mp->h[RANGE_RAND(1,WORLDWIDTH - 1)][RANGE_RAND(1,WORLDWIDTH - 1)] = 
		MAX_RAND(MAXHEIGHT);

	for(y=0; y < WORLDWIDTH; y++)
		for(x=0; x < WORLDWIDTH; x++)
			spread(mp->h,x,y);
 
	for(y=0; y < WORLDWIDTH; y++)
		for(x=0; x < WORLDWIDTH; x++){
			mp->h[x][y] = mp->h[x][y] + MAX_RAND(10) - 5;
			if(mp->h[x][y] < 10)
				mp->h[x][y] = 0;
		}

	offset = MAX_RAND( PALSIZE );	// colors avail

	for(y=0; y < WORLDWIDTH-1; y++)
		for(x=0; x < WORLDWIDTH-1; x++){
			int x2,y2,x3,y3,y4,y5,c;
			POINT p[5];

			c = (mp->h[x][y]+ mp->h[x+1][y] + mp->h[x][y+1] + mp->h[x+1][y+1]) / 4;
			c = (c + offset) % PALSIZE ;

			if(c >= PALSIZE)
				c = PALSIZE - 1;

			x2     = x * (2 * mp->width)  / (3 * WORLDWIDTH);
			y2     = y * (2 * mp->height) / (3 * WORLDWIDTH);
			p[0].x = (x2 - y2 / 2) + (mp->width / 4);
			p[0].y = (y2 - mp->h[x][y]) + mp->height / 4;

			x3     = (x+1) * (2 * mp->width)  / (3 * WORLDWIDTH);
			y3     = y     * (2 * mp->height) / (3 * WORLDWIDTH);
			p[1].x = (x3 - y3 / 2) + (mp->width / 4);
			p[1].y = (y3 - mp->h[x+1][y]) + mp->height / 4;

			y4     = (y+1) * (2 * mp->height) / (3 * WORLDWIDTH);
			p[2].x = (x3 - y4 / 2) + (mp->width / 4);
			p[2].y = (y4 - mp->h[x+1][y+1]) + mp->height / 4;

			y5     = (y+1) * (2 * mp->height) / (3 * WORLDWIDTH);
			p[3].x = (x2 - y5 / 2) + (mp->width / 4);
			p[3].y = (y5 - mp->h[x][y+1]) + mp->height / 4;

			p[4].x = p[0].x;
			p[4].y = p[0].y;

			hbColor = CreateSolidBrush(PALETTEINDEX(c));
			hbOld = SelectObject(hDC, hbColor);     // select the black pen			

			Polygon(hDC, p, 5);
			SelectObject(hDC, hbOld);     // select the black pen			
			DeleteObject(hbColor);								// delete the pen

			if (!mp->pixelmode) {
				//XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
				//XDrawLines(dsp, win, Scr[screen].gc, p, 5, CoordModeOrigin);
			}
		}
    	        
	//SelectObject(hDC, hpenOld);
	ReleaseDC(hWnd, hDC);
}


BOOL bInitDlgItems(HWND hDlg, HWND hScrl)
{
	//SetDlgItemInt(hDlg,IDC_BEEACC, iBeeAcc, FALSE);
	//SetDlgItemInt(hDlg,IDC_BEEVEL, iBeeVel, FALSE);
	//SetDlgItemInt(hDlg,IDC_BEECOUNT, iBeeCount, FALSE);
	//SetDlgItemInt(hDlg,IDC_WASPACC, iWaspAcc, FALSE);
	//SetDlgItemInt(hDlg,IDC_WASPVEL, iWaspVel, FALSE);
	//CheckDlgButton(hDlg, IDC_CLS, iCls);

	SetScrollRange(hScrl, SB_CTL, 1, MIN_SPEED, FALSE);
	SetScrollPos(hScrl, SB_CTL, MIN_SPEED-wElapse, TRUE);

	return TRUE;
}

BOOL bGetDlgItems(HWND hDlg)
{
	BOOL bRtn;

	//iBeeAcc = GetDlgItemInt(hDlg, IDC_BEEACC, &bRtn, FALSE);
	//iBeeVel = GetDlgItemInt(hDlg, IDC_BEEVEL, &bRtn, FALSE);
	//iBeeCount = GetDlgItemInt(hDlg, IDC_BEECOUNT, &bRtn, FALSE);
	//iWaspAcc = GetDlgItemInt(hDlg, IDC_WASPACC, &bRtn, FALSE);
	//iWaspVel = GetDlgItemInt(hDlg, IDC_WASPVEL, &bRtn, FALSE);
	iCls = IsDlgButtonChecked(hDlg, IDC_CLS);

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

	LoadString(hMainInstance, IDS_SPEED, szSpeed, 20);
	//LoadString(hMainInstance, IDS_BEEACC, szBeeAcc, 20);
	//LoadString(hMainInstance, IDS_WASPACC, szWaspAcc, 20);
	//LoadString(hMainInstance, IDS_BEEVEL, szBeeVel, 20);
	//LoadString(hMainInstance, IDS_WASPVEL, szWaspVel, 20);
	//LoadString(hMainInstance, IDS_BEECOUNT, szBeeCount, 20);
	LoadString(hMainInstance, IDS_CLS, szCls, 20);
}


BOOL GetIniSettings(void)
{	
	// Retrieve settings from INI file (note: for compatability with 16 bit version)
    wElapse		= SetProfileVal(FALSE, szAppName, szSpeed, DEF_SPEED, szRegEntry);
	//iBeeAcc		= SetProfileVal(FALSE, szAppName, szBeeAcc, DEF_BEEACC, szRegEntry);
	//iBeeVel		= SetProfileVal(FALSE, szAppName, szBeeVel, DEF_BEEVEL, szRegEntry);
    //iBeeCount	= SetProfileVal(FALSE, szAppName, szBeeCount, DEF_BEECOUNT, szRegEntry);    
	//iWaspAcc	= SetProfileVal(FALSE, szAppName, szWaspAcc, DEF_WASPACC, szRegEntry);
	//iWaspVel	= SetProfileVal(FALSE, szAppName, szWaspAcc, DEF_WASPVEL, szRegEntry);
	iCls		= SetProfileVal(FALSE, szAppName, szCls, DEF_CLS, szRegEntry);

	return TRUE;
}


BOOL SetIniSettings(void)
{
	SetProfileVal(TRUE, szAppName, szSpeed, wElapse, szRegEntry);
	//SetProfileVal(TRUE, szAppName, szBeeAcc, iBeeAcc, szRegEntry);
	//SetProfileVal(TRUE, szAppName, szBeeVel, iBeeVel, szRegEntry);
	//SetProfileVal(TRUE, szAppName, szBeeCount, iBeeCount, szRegEntry);
	//SetProfileVal(TRUE, szAppName, szWaspAcc, iWaspAcc, szRegEntry);
	//SetProfileVal(TRUE, szAppName, szWaspVel, iWaspVel, szRegEntry);
	SetProfileVal(TRUE, szAppName, szCls, iCls, szRegEntry);

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
