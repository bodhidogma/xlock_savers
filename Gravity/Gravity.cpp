/*
	Quix Screen Saver
	--------------------------------------------------------
	History:
		Ported from the xlockmore package to 16-bit Windows
		Ported to Win32 from 16-bit Windows
*/


#include <windows.h> 
#include <scrnsave.h>

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

#include "resource.h"
#include "Gravity.h"


LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	RECT	rc;
	HANDLE	hPalMem;	

    switch (msg)
    {                
        case WM_CREATE:
        {
			gravstruct	*gp = &gravs;
        	int		i, R,G,B;
			unsigned char ball;

			vLoadStrings();					// load strings from table
			GetIniSettings();				// get settings from ini file (control.ini)
        	
            wTimer = SetTimer(hWnd, ID_TIMER, wElapse, NULL);	// create timer to move image
                        
            GetClientRect(hWnd, &rc);							// find size of screen
			gp->width = rc.right;
			gp->height = rc.bottom;

			gp->sr = STARRADIUS;

			gp->nplanets = iPlanets;
			srand((unsigned)time(NULL));	// seed random w/ timer

			if (!gp->planets) {
				gp->planets = (planetstruct*)malloc((gp->nplanets+1)*sizeof(planetstruct));
				memset(gp->planets, 0, (gp->nplanets+1)*sizeof(planetstruct));
				gp->starcolor = rand() % (PALSIZE-21);
			}

			for (ball = 0; ball <= gp->nplanets; ball++)
				init_planet(&(gp->planets[ball]));

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
        }
        
        case WM_TIMER:                              // BOUNCER-specific
            MoveImage(hWnd);						// move image around
            break;

        case WM_DESTROY:                            // BOUNCER-specific
            if( wTimer )
            	KillTimer(hWnd, ID_TIMER);
			if (hPal)
				DeleteObject(hPal);
			free(gravs.planets);            
            break;

        case WM_ERASEBKGND:
			GetClientRect(hWnd,&rc);
			FillRect((HDC)wParam,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
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
			iScrlPos = 1000-wElapse;
			switch ((int)LOWORD(wParam))
			{
				case SB_LINERIGHT:					// determine specific movement type
					iScrlPos += 10;
					break;
				case SB_LINELEFT:
					iScrlPos -= 10;
					break;
				case SB_PAGERIGHT:				
					iScrlPos += 50;
					break;
				case SB_PAGELEFT:
					iScrlPos -= 50;
					break;
				case SB_THUMBPOSITION:			// grabbed thumb and moved it
					iScrlPos = (short int)HIWORD(wParam);
					break;
			}
			iScrlPos = min(1000,max(1,iScrlPos));		// check to see within desired range
			if (iScrlPos != 1000-(int)wElapse)
			{
				wElapse = 1000-iScrlPos;										// save new position
				SetScrollPos((HWND)lParam, SB_CTL, iScrlPos, TRUE);			// display position				
			}
			return TRUE;
		}
		case WM_COMMAND: {			
			switch (LOWORD(wParam))
			{
				case IDC_DEFAULTS: {
					wElapse = DEF_SPEEDSET;
					iPlanets = DEF_PLANETS;
					iTrails = DEF_TRAILS;
					iDamping = DEF_DAMPING;
					iColorCycle = DEF_CYCLE;
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

void init_planet(planetstruct *planet)
{
	gravstruct *gp = &gravs;

	planet->colors = rand() % (PALSIZE-21);

	// init positions
	POS(X) = FLOATRAND(-XR, XR);
	POS(Y) = FLOATRAND(-YR, YR);							 
	POS(Z) = FLOATRAND(-ZR, ZR);

	if (POS(Z) > -ALMOST) {
		planet->xi = (unsigned int)
			((double)gp->width * (HALF+POS(X) / (POS(Z) + DIST)));
		planet->yi = (unsigned int)
			((double)gp->height * (HALF+POS(Y) / (POS(Z) + DIST)));
	}
	else
		planet->xi = planet->yi = -1;

	planet->ri = RADIUS;

	// init velocities
	VEL(X) = FLOATRAND(-VR, VR);
	VEL(Y) = FLOATRAND(-VR, VR);
	VEL(Z) = FLOATRAND(-VR, VR);

}

static void MoveImage(HWND hWnd) {   

	HDC		hDC;
	HBRUSH	hbrColor, hbrOld;
	gravstruct *gp = &gravs;
	unsigned char	ball;
	
	hDC = GetDC(hWnd);									// get HDC to this window	
		
    SelectPalette(hDC, hPal, 0);
    RealizePalette(hDC);

	hbrOld = SelectObject(hDC, (HBRUSH)GetStockObject(BLACK_BRUSH));     // select the black pen			
	// Mask center point
	Ellipse(hDC, REC_CORNERS(gp->width/2, gp->height/2, gp->sr));   	

	// Resize center point
	switch (rand() % 4) {
	case 0:
		if (gp->sr < STARRADIUS)
			gp->sr++;
		break;
	case 1:
		if (gp->sr > 2)
			gp->sr--;
	}
	if (iColorCycle) {
		if (gp->starcolor+=6 > (PALSIZE-21))
			gp->starcolor = 1;
	}

	hbrColor = CreateSolidBrush(PALETTEINDEX(gp->starcolor));
	
	// Draw center point	
	SelectObject(hDC, hbrColor);
	Ellipse(hDC, REC_CORNERS(gp->width/2, gp->height/2, gp->sr));   	

	for (ball = 0; ball <= gp->nplanets; ball++)
		draw_planet(hDC, &gp->planets[ball]);	
	
	SelectObject(hDC, hbrOld);
	DeleteObject(hbrColor);								// delete the pen
	
	ReleaseDC(hWnd, hDC);
}

void draw_planet(HDC hDC, planetstruct *planet)
{
	HBRUSH hbrColor, hbrOld;
	gravstruct *gp = &gravs;
	double D;						// a DX variable to work with
	unsigned char cmpt;

	D = POS(X) * POS(X) + POS(Y) * POS(Y) + POS(Z) * POS(Z);
	if (D < COLLIDE)
		D = COLLIDE;
	D = sqrt(D);
	D = D * D * D;
	for (cmpt = X; cmpt < DIMENSIONS; cmpt++) {
		ACC(cmpt) = POS(cmpt) * GRAV / D;
		if (iDamping) {
			if (ACC(cmpt) > MaxA)
				ACC(cmpt) = MaxA;
			else if (ACC(cmpt) < -MaxA)
				ACC(cmpt) = -MaxA;
			VEL(cmpt) = VEL(cmpt) + ACC(cmpt);
			VEL(cmpt) *= DAMP;
		} else {
			// update velocity
			VEL(cmpt) = VEL(cmpt) + ACC(cmpt);
		}
		// update position
		POS(cmpt) = POS(cmpt) + VEL(cmpt);
	}

	gp->x = planet->xi;
	gp->y = planet->yi;

	if (POS(Z) > -ALMOST) {
		planet->xi = (unsigned int)
			((double) gp->width * (HALF + POS(X) / (POS(Z) + DIST)));
		planet->yi = (unsigned int)
			((double) gp->height * (HALF + POS(Y) / (POS(Z) + DIST)));
	}
	else
		planet->xi = planet->yi = -1;	

	// Mask
	hbrOld = (HBRUSH)SelectObject(hDC, (HBRUSH)GetStockObject(BLACK_BRUSH));
	Planet(gp->x, gp->y);

	if (iTrails)
		SetPixel(hDC, gp->x, gp->y, PALETTEINDEX(100));

	// Move
	gp->x = planet->xi;
	gp->y = planet->yi;
	planet->ri = RADIUS;

	if (iColorCycle) {
		if (planet->colors++ > (PALSIZE-21))
			planet->colors = 1;
	}

	// Redraw
	hbrColor = CreateSolidBrush(PALETTEINDEX(planet->colors));
	SelectObject(hDC, hbrColor);
	Planet(gp->x, gp->y);

	SelectObject(hDC, hbrOld);
	DeleteObject(hbrColor);
}

BOOL bInitDlgItems(HWND hDlg, HWND hScrl)
{
	SetDlgItemInt(hDlg,IDC_PLANETS, iPlanets, FALSE);
	CheckDlgButton(hDlg,IDC_TRAILS, iTrails);
	CheckDlgButton(hDlg,IDC_DAMPING, iDamping);
	CheckDlgButton(hDlg,IDC_CYCLE, iColorCycle);
	SetScrollRange(hScrl, SB_CTL, 1, 1000, FALSE);
	SetScrollPos(hScrl, SB_CTL, 1000-wElapse, TRUE);

	return TRUE;
}

BOOL bGetDlgItems(HWND hDlg)
{
	BOOL bRtn;

	iPlanets = GetDlgItemInt(hDlg, IDC_PLANETS, &bRtn, FALSE);
	iTrails = IsDlgButtonChecked(hDlg, IDC_TRAILS);
	iDamping = IsDlgButtonChecked(hDlg, IDC_DAMPING);
	iColorCycle = IsDlgButtonChecked(hDlg, IDC_CYCLE);
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
	LoadString(hMainInstance, IDS_PLANETS, szPlanetName, 20);
	LoadString(hMainInstance, IDS_TRAILS, szTrailName, 20);
	LoadString(hMainInstance, IDS_DAMP, szDampName, 20);
	LoadString(hMainInstance, IDS_COLOR, szCycleName, 20);
}

BOOL GetIniSettings(void)
{	
	// Retrieve settings from INI file (note: for compatability with 16 bit version)
    wElapse		= SetProfileVal(FALSE, szAppName, szSpeedName, DEF_SPEEDSET, szRegEntry);
	iPlanets	= SetProfileVal(FALSE, szAppName, szPlanetName, DEF_PLANETS, szRegEntry);
	iTrails		= SetProfileVal(FALSE, szAppName, szTrailName, DEF_TRAILS, szRegEntry);
    iDamping	= SetProfileVal(FALSE, szAppName, szDampName, DEF_DAMPING, szRegEntry);
    iColorCycle	= SetProfileVal(FALSE, szAppName, szCycleName, DEF_CYCLE, szRegEntry);
	return TRUE;
}

BOOL SetIniSettings(void)
{
	SetProfileVal(TRUE, szAppName, szSpeedName, wElapse, szRegEntry);
	SetProfileVal(TRUE, szAppName, szPlanetName, iPlanets, szRegEntry);
	SetProfileVal(TRUE, szAppName, szTrailName, iTrails, szRegEntry);
	SetProfileVal(TRUE, szAppName, szDampName, iDamping, szRegEntry);
	SetProfileVal(TRUE, szAppName, szCycleName, iColorCycle, szRegEntry);
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

