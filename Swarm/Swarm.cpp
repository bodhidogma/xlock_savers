/*
	Swarm Screen Saver (rotor.scr) - rotor.c

	Swarming little thingies

	History:
		Ported from the xlockmore package to 16-bit Windows
		Ported to Win32 from 16-bit Windows

*/


#include <windows.h> 
#include <scrnsave.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "resource.h"
#include "Swarm.h"


LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	HANDLE hPalMem;
    swarmstruct	*sp = &swarms;
    int		b,
        	i, R,G,B;
                                                          
    switch (msg)
    {
    case WM_CREATE:
		vLoadStrings();					// load strings from table
		GetIniSettings();				// get settings from ini file (control.ini)
        
        wTimer = SetTimer(hWnd, ID_TIMER, wElapse, NULL);	// create timer to move image
                    
        GetClientRect(hWnd, &rc);							// find size of screen
        sp->width = rc.right;
        sp->height = rc.bottom;
        sp->border = (sp->width + sp->height) / 50;
        
        sp->beecount = iBeeCount;							// # of bees
        
        srand((unsigned int)time( NULL ));
        
        sp->r = 0;
                    
        if (!sp->segs) {
            sp->segs = (lseg *)malloc(sizeof(lseg) * sp->beecount);
            sp->old_segs = (lseg *)malloc(sizeof(lseg) * sp->beecount);
            sp->x = (short *)malloc(sizeof(short) * sp->beecount * TIMES);
            sp->y = (short *)malloc(sizeof(short) * sp->beecount * TIMES);
            sp->xv = (short *)malloc(sizeof(short) * sp->beecount);
            sp->yv = (short *)malloc(sizeof(short) * sp->beecount);
        }
        
        // ------ WASP configuration -------
        sp->wx[0] = sp->border + rand() % (sp->width - 2 * sp->border);
        sp->wy[0] = sp->border + rand() % (sp->height - 2 * sp->border);
        sp->wx[1] = sp->wx[0];
        sp->wy[1] = sp->wy[0];
        sp->wxv = 0;
        sp->wyv = 0;
        
        // ------ BEES configuration -------
        for (b = 0; b < sp->beecount; b++) {
            X(0,b) = rand() % sp->width;
            X(1,b) = X(0,b);
            Y(0,b) = rand() % sp->height;
            Y(1,b) = Y(0,b);
            sp->xv[b] = RAND(7);
            sp->yv[b] = RAND(7);
        }
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
        
    case WM_TIMER:                              // BOUNCER-specific
        MoveImage(hWnd);						// move image around
        break;

    case WM_DESTROY:                            // BOUNCER-specific
                    
        if( wTimer )
            KillTimer(hWnd, ID_TIMER);
		if (hPal)
			DeleteObject(hPal);
        
        free( swarms.yv );
        free( swarms.xv );
        free( swarms.y );
        free( swarms.x );
        free( swarms.old_segs );
        free( swarms.segs );	
        break;

    case WM_ERASEBKGND:
		if (iCls) {
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
			iBeeAcc = DEF_BEEACC;
			iBeeVel = DEF_BEEVEL;
			iBeeCount = DEF_BEECOUNT;
			iWaspAcc = DEF_WASPACC;
			iWaspVel = DEF_WASPVEL;
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
	HPEN	hpenColor, hpenBlack, hpenWhite, hpenOld;
	int		b;	
	swarmstruct *sp = &swarms;
	
	hDC = GetDC(hWnd);									// get HDC to this window
	hpenBlack = GetStockObject(BLACK_PEN);				// get a black pen			
	hpenWhite = GetStockObject(WHITE_PEN);				// get a black pen			
		
    SelectPalette(hDC, hPal, 0);
    RealizePalette(hDC);

	// ----- WASP, age the arrays ------
	sp->wx[2] = sp->wx[1];
	sp->wx[1] = sp->wx[0];
	sp->wy[2] = sp->wy[1];
   	sp->wy[1] = sp->wy[0];
    // ----- Accellerate -----
    sp->wxv += RAND(iWaspAcc);
    sp->wyv += RAND(iWaspAcc);    
    // ----- Speed limit checks -----
    if (sp->wxv > iWaspVel)
    	sp->wxv = iWaspVel;
    if (sp->wxv < -iWaspVel)
    	sp->wxv = -iWaspVel;
    if (sp->wyv > iWaspVel)
    	sp->wyv = iWaspVel;
    if (sp->wyv < -iWaspVel)
    	sp->wyv = -iWaspVel;
    // ----- Move -----
    sp->wx[0] = sp->wx[1] + sp->wxv;
    sp->wy[0] = sp->wy[1] + sp->wyv;    
    // ----- Bounce checks -----
    if ((sp->wx[0] < sp->border) || (sp->wx[0] > sp->width - sp->border - 1)) {
    	sp->wxv = -sp->wxv;
    	sp->wx[0] += sp->wxv;
    	}
    if ((sp->wy[0] < sp->border) || (sp->wy[0] > sp->height - sp->border - 1)) {
    	sp->wyv = -sp->wyv;                       
    	sp->wy[0] += sp->wyv;
    	}
    // ----- Don't let things settle down -----
    sp->xv[rand() % sp->beecount] += RAND(3);
    sp->yv[rand() % sp->beecount] += RAND(3);
    
    // ----- BEES -----
    for (b = 0; b < sp->beecount; b++) {
    	int		distance,
    			dx, dy;
    			
    	// ----- Age the arrays -----
    	X(2,b) = X(1,b);
    	X(1,b) = X(0,b);
    	Y(2,b) = Y(1,b);
    	Y(1,b) = Y(0,b);    	
    	// ----- Accellerate -----
    	dx = sp->wx[1] - X(1,b);
    	dy = sp->wy[1] - Y(1,b);
    	distance = abs(dx) + abs(dy);		// approximation
    	if (distance == 0)
    		distance = 1;
    	sp->xv[b] += (dx * iBeeAcc) / distance;
    	sp->yv[b] += (dy * iBeeAcc) / distance;
    	// ----- Speed limit checks -----
    	if (sp->xv[b] > iBeeVel)
    		sp->xv[b] = iBeeVel;
    	if (sp->xv[b] < -iBeeVel)
    		sp->xv[b] = -iBeeVel;
    	if (sp->yv[b] > iBeeVel)
    		sp->yv[b] = iBeeVel;
    	if (sp->yv[b] < -iBeeVel)
    		sp->yv[b] = -iBeeVel;
    	// ----- Move -----
    	X(0,b) = X(1,b) + sp->xv[b];
    	Y(0,b) = Y(1,b) + sp->yv[b];
    	// ----- Fill the segment lists -----
    	sp->segs[b].x1 = X(0,b);
    	sp->segs[b].y1 = Y(0,b);
    	sp->segs[b].x2 = X(1,b);
    	sp->segs[b].y2 = Y(1,b);
    	sp->old_segs[b].x1 = X(1,b);
    	sp->old_segs[b].y1 = Y(1,b);
    	sp->old_segs[b].x2 = X(2,b);
    	sp->old_segs[b].y2 = Y(2,b);
    }
    	        
	hpenOld = SelectObject(hDC, hpenBlack);     // select the black pen			
	MoveToEx(hDC, sp->wx[1], sp->wy[1], NULL);  // erase the old lines
	LineTo(hDC, sp->wx[2], sp->wy[2]);	
	DrawSegments(hDC, sp->old_segs, sp->beecount);
	
	SelectObject(hDC, hpenWhite);
	MoveToEx(hDC, sp->wx[0], sp->wy[0], NULL);
	LineTo(hDC, sp->wx[1], sp->wy[1]);
	
	if (sp->r++ > (PALSIZE-21))
		sp->r = 0;

	hpenColor = CreatePen(PS_SOLID, 1, PALETTEINDEX(sp->r));
	SelectObject(hDC, hpenColor);						// select a new pen color
	DrawSegments(hDC, sp->segs, sp->beecount);
	SelectObject(hDC, hpenOld);
	DeleteObject(hpenColor);								// delete the pen
			
	ReleaseDC(hWnd, hDC);
}

void DrawSegments(HDC hDC, lseg *segs, int seg_cnt)
{

	int iCnt;
	
	for (iCnt = 0; iCnt < seg_cnt; iCnt++) {
		MoveToEx(hDC, segs[iCnt].x1, segs[iCnt].y1, NULL);
		LineTo(hDC, segs[iCnt].x2, segs[iCnt].y2);
	}
}


BOOL bInitDlgItems(HWND hDlg, HWND hScrl)
{
	SetDlgItemInt(hDlg,IDC_BEEACC, iBeeAcc, FALSE);
	SetDlgItemInt(hDlg,IDC_BEEVEL, iBeeVel, FALSE);
	SetDlgItemInt(hDlg,IDC_BEECOUNT, iBeeCount, FALSE);
	SetDlgItemInt(hDlg,IDC_WASPACC, iWaspAcc, FALSE);
	SetDlgItemInt(hDlg,IDC_WASPVEL, iWaspVel, FALSE);
	CheckDlgButton(hDlg, IDC_CLS, iCls);

	SetScrollRange(hScrl, SB_CTL, 1, 100, FALSE);
	SetScrollPos(hScrl, SB_CTL, MIN_SPEED-wElapse, TRUE);

	return TRUE;
}

BOOL bGetDlgItems(HWND hDlg)
{
	BOOL bRtn;

	iBeeAcc = GetDlgItemInt(hDlg, IDC_BEEACC, &bRtn, FALSE);
	iBeeVel = GetDlgItemInt(hDlg, IDC_BEEVEL, &bRtn, FALSE);
	iBeeCount = GetDlgItemInt(hDlg, IDC_BEECOUNT, &bRtn, FALSE);
	iWaspAcc = GetDlgItemInt(hDlg, IDC_WASPACC, &bRtn, FALSE);
	iWaspVel = GetDlgItemInt(hDlg, IDC_WASPVEL, &bRtn, FALSE);
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
	LoadString(hMainInstance, IDS_BEEACC, szBeeAcc, 20);
	LoadString(hMainInstance, IDS_WASPACC, szWaspAcc, 20);
	LoadString(hMainInstance, IDS_BEEVEL, szBeeVel, 20);
	LoadString(hMainInstance, IDS_WASPVEL, szWaspVel, 20);
	LoadString(hMainInstance, IDS_BEECOUNT, szBeeCount, 20);
	LoadString(hMainInstance, IDS_CLS, szCls, 20);
}


BOOL GetIniSettings(void)
{	
	// Retrieve settings from INI file (note: for compatability with 16 bit version)
    wElapse		= SetProfileVal(FALSE, szAppName, szSpeed, DEF_SPEED, szRegEntry);
	iBeeAcc		= SetProfileVal(FALSE, szAppName, szBeeAcc, DEF_BEEACC, szRegEntry);
	iBeeVel		= SetProfileVal(FALSE, szAppName, szBeeVel, DEF_BEEVEL, szRegEntry);
    iBeeCount	= SetProfileVal(FALSE, szAppName, szBeeCount, DEF_BEECOUNT, szRegEntry);    
	iWaspAcc	= SetProfileVal(FALSE, szAppName, szWaspAcc, DEF_WASPACC, szRegEntry);
	iWaspVel	= SetProfileVal(FALSE, szAppName, szWaspAcc, DEF_WASPVEL, szRegEntry);
	iCls		= SetProfileVal(FALSE, szAppName, szCls, DEF_CLS, szRegEntry);

	return TRUE;
}


BOOL SetIniSettings(void)
{
	SetProfileVal(TRUE, szAppName, szSpeed, wElapse, szRegEntry);
	SetProfileVal(TRUE, szAppName, szBeeAcc, iBeeAcc, szRegEntry);
	SetProfileVal(TRUE, szAppName, szBeeVel, iBeeVel, szRegEntry);
	SetProfileVal(TRUE, szAppName, szBeeCount, iBeeCount, szRegEntry);
	SetProfileVal(TRUE, szAppName, szWaspAcc, iWaspAcc, szRegEntry);
	SetProfileVal(TRUE, szAppName, szWaspVel, iWaspVel, szRegEntry);
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
