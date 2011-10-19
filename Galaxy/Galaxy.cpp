/*
 * galaxy.c - Spinning galaxies for xlockmore
 
   
 *
 * Originally done by Uli Siegmund (uli@wombat.okapi.sub.org) on Amiga
 *   for EGS in Cluster
 * Port from Cluster/EGS to C/Intuition by Harald Backert
 * Port to X11 and incorporation into xlockmore by Hubert Feyrer
 *   (hubert.feyrer@rz.uni-regensburg.de)
 *
 * Revision History:
 * 09-Mar-94: VAX can generate a random number 0.0 which results in a
 *            division by zero, corrected by Jouk Jansen
 *            <joukj@crys.chem.uva.nl>
 * 30-Sep-94: Initial port by Hubert Feyer
 * 10-Oct-94: Add colors by Hubert Feyer
 * 23-Oct-94: Modified by David Bagley <bagleyd@source.asset.com>

 * 14-Feb-96: ported to Win32, Paul McAvoy (paulmcav@silcom.com)
 */


#include <windows.h> 
#include <scrnsave.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "resource.h"
#include "Galaxy.h"


LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	HANDLE hPalMem;
    unistruct *gp = &universes;
    int		i, R,G,B;
                                                          
    switch (msg)
    {
    case WM_CREATE:
		vLoadStrings();					// load strings from table
		GetIniSettings();				// get settings from ini file (control.ini)
        
        wTimer = SetTimer(hWnd, ID_TIMER, wElapse, NULL);	// create timer to move image
		srand((unsigned)time(NULL));	// seed random w/ timer
                    
        GetClientRect(hWnd, &rc);							// find size of screen
		          
		if (iRandom) {
			gp->f_galaxies       = (rand() % MAX_GALAXIES)+2;
		} else {
			gp->f_galaxies       = iNumGalaxy;
		}
		gp->f_stars          = MAX_STARS / iStarSize;
		gp->f_hititerations  = MAX_HITITERATIONS;
		gp->f_deltat         = ((double) MAX_IDELTAT)/10000.0;

		gp->clip.left    = 5;
		gp->clip.top     = 5;
		gp->clip.right   = rc.right - 10;
		gp->clip.bottom  = rc.bottom - 10;
	
		gp->scale        = (double)(gp->clip.right)/4.0;
		gp->midx         = gp->clip.right/2;
		gp->midy         = gp->clip.bottom/2;
		gp->init         = 1;
	
		gp->starsize   = iStarSize;
	

		if(!gp->galaxies[0].stars){
			for (i=0; i < gp->f_galaxies; ++i) {
				gp->galaxies[i].starscnt=0;	/* 0 valid entries */
				gp->galaxies[i].stars=(Star *)malloc(gp->f_stars*sizeof(Star));
			}
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
        
		for (i=0; i < gp->f_galaxies; ++i)			
			free(gp->galaxies[i].stars);		
        break;

    case WM_ERASEBKGND:
		//if (iCls) {
	    //    GetClientRect(hWnd,&rc);
	    //    FillRect((HDC)wParam,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
		//}
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
			iRandom = DEF_RANDOM;
			iNumGalaxy = DEF_NUMGALAXY;
			iStarSize = DEF_STARSIZE;
			iWrap = DEF_WRAP;
			iErase = DEF_ERASE;
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
	HBRUSH	hpenColor, hpenBlack, hpenOld;
	
	unistruct *gp = &universes;
    double d;                                  /* tmp */
    int i, j, k;                               /* more tmp */
	RECT rc;

	hDC = GetDC(hWnd);									// get HDC to this window
	hpenBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);				// get a black pen			
	
	SelectPalette(hDC, hPal, 0);
    RealizePalette(hDC);
    	        
	hpenOld = (HBRUSH)SelectObject(hDC, hpenBlack);     // select the black pen			

    if(gp->init){
		double w1, w2, w3;                     /* more tmp */
		double v,w, h;                         /* yet more tmp */

		gp->init=0;
		gp->step=0;
		
		for (i=0; i<gp->f_galaxies; ++i){
			gp->galcol[i]= rand() % 255 + 2;
		}
		
		for (i=0; i<gp->f_galaxies; ++i) {
			gp->galaxies[i].basecolor=gp->galcol[i];
		
			gp->galaxies[i].starscnt=(rand() % (gp->f_stars/2))+gp->f_stars/2;
		
			w1=2.0*PI*FLOATRAND;
			w2=2.0*PI*FLOATRAND;

			gp->mat[0][0]= cos(w2);
			gp->mat[0][1]= -sin(w1)*sin(w2);
			gp->mat[0][2]= cos(w1)*sin(w2);
			gp->mat[1][0]= 0.0;
			gp->mat[1][1]= cos(w1);
			gp->mat[1][2]= sin(w1);
			gp->mat[2][0]= -sin(w2);
			gp->mat[2][1]= -sin(w1)*cos(w2);
			gp->mat[2][2]= cos(w1)*cos(w2);
		
			gp->galaxies[i].vel[0]= FLOATRAND*2.0-1.0;
			gp->galaxies[i].vel[1]= FLOATRAND*2.0-1.0;
			gp->galaxies[i].vel[2]= FLOATRAND*2.0-1.0;
			gp->galaxies[i].pos[0]= -gp->galaxies[i].vel[0]*gp->f_deltat*
				  gp->f_hititerations+FLOATRAND-0.5;
			gp->galaxies[i].pos[1]= -gp->galaxies[i].vel[1]*gp->f_deltat*
				  gp->f_hititerations+FLOATRAND-0.5;
			gp->galaxies[i].pos[2]= -gp->galaxies[i].vel[2]*gp->f_deltat*
				  gp->f_hititerations+FLOATRAND-0.5;
		
			gp->galaxies[i].mass= (int)(FLOATRAND*1000.0+1.0);
		
			/*w3=FLOATRAND;*/
				w3 = 0.0;
			gp->size= w3*w3*GALAKSIZE+0.1;
		
			for (j=0; j<gp->galaxies[i].starscnt; ++j) {
				w=2.0*PI*FLOATRAND;
				d=FLOATRAND*gp->size;
				h=FLOATRAND*exp(-2.0*(d/gp->size))/5.0*gp->size;
				if (FLOATRAND<0.5) h= -h;
				gp->galaxies[i].stars[j].pos[0]= gp->mat[0][0]*d*cos(w)+
						  gp->mat[1][0]*d*sin(w)+gp->mat[2][0]*h+gp->galaxies[i].pos[0];
				gp->galaxies[i].stars[j].pos[1]= gp->mat[0][1]*d*cos(w)+gp->mat[1][1]*d*sin(w)+gp->mat[2][1]*h+gp->galaxies[i].pos[1];
				gp->galaxies[i].stars[j].pos[2]= gp->mat[0][2]*d*cos(w)+gp->mat[1][2]*d*sin(w)+gp->mat[2][2]*h+gp->galaxies[i].pos[2];
				
				v=sqrt(gp->galaxies[i].mass*QCONS/sqrt(d*d+h*h));
				gp->galaxies[i].stars[j].vel[0]= -gp->mat[0][0]*v*sin(w)+gp->mat[1][0]*v*cos(w)+gp->galaxies[i].vel[0];
				gp->galaxies[i].stars[j].vel[1]= -gp->mat[0][1]*v*sin(w)+gp->mat[1][1]*v*cos(w)+gp->galaxies[i].vel[1];
				gp->galaxies[i].stars[j].vel[2]= -gp->mat[0][2]*v*sin(w)+gp->mat[1][2]*v*cos(w)+gp->galaxies[i].vel[2];
				
				gp->galaxies[i].stars[j].color= gp->galaxies[i].basecolor + (j % COLORSTEP);
								
				gp->galaxies[i].stars[j].px= 0;
				gp->galaxies[i].stars[j].py= 0;
			}
		}
		
        if (iCls) {
			GetClientRect(hWnd, &rc);							// find size of screen
			FillRect(hDC,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
		}
		iCls = 1;
    }
    
    for (i=0; i<gp->f_galaxies; ++i){
		for (j=0; j<gp->galaxies[i].starscnt; ++j) {
			for (k=0; k<gp->f_galaxies; ++k) {
				gp->diff[0]=gp->galaxies[k].pos[0]-gp->galaxies[i].stars[j].pos[0];
				gp->diff[1]=gp->galaxies[k].pos[1]-gp->galaxies[i].stars[j].pos[1];
				gp->diff[2]=gp->galaxies[k].pos[2]-gp->galaxies[i].stars[j].pos[2];
				d=gp->diff[0]*gp->diff[0]+gp->diff[1]*gp->diff[1]+gp->diff[2]*gp->diff[2];
				d=gp->galaxies[k].mass/(d*sqrt(d))*gp->f_deltat*QCONS;
				gp->diff[0]*=d;
				gp->diff[1]*=d;
				gp->diff[2]*=d;
				gp->galaxies[i].stars[j].vel[0]+=gp->diff[0];
				gp->galaxies[i].stars[j].vel[1]+=gp->diff[1];
				gp->galaxies[i].stars[j].vel[2]+=gp->diff[2];
			}
			gp->galaxies[i].stars[j].pos[0]+=gp->galaxies[i].stars[j].vel[0]*
				  gp->f_deltat;
			gp->galaxies[i].stars[j].pos[1]+=gp->galaxies[i].stars[j].vel[1]*
				  gp->f_deltat;
			gp->galaxies[i].stars[j].pos[2]+=gp->galaxies[i].stars[j].vel[2]*
				  gp->f_deltat;
			
			if (gp->starsize > 1)
				SelectObject(hDC, hpenBlack);     // select the black pen

			if (gp->galaxies[i].stars[j].px>=gp->clip.left
				&& gp->galaxies[i].stars[j].px<=gp->clip.right-gp->starsize
				&& gp->galaxies[i].stars[j].py>=gp->clip.top
				&& gp->galaxies[i].stars[j].py<=gp->clip.bottom-gp->starsize){

				if (!iErase) {
					MyXDrawPoint(hDC, gp->galaxies[i].stars[j].px, gp->galaxies[i].stars[j].py, 0);
				}
			}
	    
			gp->galaxies[i].stars[j].px=(int)(gp->galaxies[i].stars[j].pos[0]*
				  gp->scale)+gp->midx;
			gp->galaxies[i].stars[j].py=(int)(gp->galaxies[i].stars[j].pos[1]*
				  gp->scale)+gp->midy;
	    
			if (iWrap) {
				if(gp->galaxies[i].stars[j].px < gp->clip.left){
					//(void) printf("wrap l -> r\n");
					gp->galaxies[i].stars[j].px = gp->clip.right;
				}
				if(gp->galaxies[i].stars[j].px > gp->clip.right){
					//(void) printf("wrap r -> l\n");
					gp->galaxies[i].stars[j].px = gp->clip.left;
				}
				if(gp->galaxies[i].stars[j].py > gp->clip.bottom){
					//(void) printf("wrap b -> t\n");
					gp->galaxies[i].stars[j].py = gp->clip.top;
				}
				if(gp->galaxies[i].stars[j].py < gp->clip.top){
					//(void) printf("wrap t -> b\n");
					gp->galaxies[i].stars[j].py = gp->clip.bottom;
				}
			}
	    
			if (gp->galaxies[i].stars[j].px>=gp->clip.left
				&& gp->galaxies[i].stars[j].px<=gp->clip.right-gp->starsize
				&& gp->galaxies[i].stars[j].py>=gp->clip.top
				&& gp->galaxies[i].stars[j].py<=gp->clip.bottom-gp->starsize){


				if (gp->starsize > 1) {
					hpenColor = CreateSolidBrush(PALETTEINDEX(gp->galaxies[i].stars[j].color));
					SelectObject(hDC, hpenColor);				// select a new pen color
				}

				MyXDrawPoint(hDC, gp->galaxies[i].stars[j].px,
						  gp->galaxies[i].stars[j].py,
						  PALETTEINDEX(gp->galaxies[i].stars[j].color));
 
				if (gp->starsize > 1)
					DeleteObject(hpenColor);					// delete the pen
			}
		}
	
		for (k=i+1; k<gp->f_galaxies; ++k) {
			gp->diff[0]=gp->galaxies[k].pos[0]-gp->galaxies[i].pos[0];
			gp->diff[1]=gp->galaxies[k].pos[1]-gp->galaxies[i].pos[1];
			gp->diff[2]=gp->galaxies[k].pos[2]-gp->galaxies[i].pos[2];
			d=gp->diff[0]*gp->diff[0]+gp->diff[1]*gp->diff[1]+gp->diff[2]*gp->diff[2];
			d=gp->galaxies[i].mass*gp->galaxies[k].mass/(d*sqrt(d))*gp->f_deltat*QCONS;
			gp->diff[0]*=d;
			gp->diff[1]*=d;
			gp->diff[2]*=d;
			gp->galaxies[i].vel[0]+=gp->diff[0]/gp->galaxies[i].mass;
			gp->galaxies[i].vel[1]+=gp->diff[1]/gp->galaxies[i].mass;
			gp->galaxies[i].vel[2]+=gp->diff[2]/gp->galaxies[i].mass;
			gp->galaxies[k].vel[0]-=gp->diff[0]/gp->galaxies[k].mass;
			gp->galaxies[k].vel[1]-=gp->diff[1]/gp->galaxies[k].mass;
			gp->galaxies[k].vel[2]-=gp->diff[2]/gp->galaxies[k].mass;
		}
		gp->galaxies[i].pos[0]+=gp->galaxies[i].vel[0]*gp->f_deltat;
		gp->galaxies[i].pos[1]+=gp->galaxies[i].vel[1]*gp->f_deltat;
		gp->galaxies[i].pos[2]+=gp->galaxies[i].vel[2]*gp->f_deltat;
    }

    gp->step++;
    if(gp->step > gp->f_hititerations*4) gp->init=1;
	
	SelectObject(hDC, hpenOld);
			
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
	SetDlgItemInt(hDlg,IDC_NUMGALAXY, iNumGalaxy, FALSE);
	SetEditLim(hDlg, IDC_NUMGALAXY, 1);
	SetDlgItemInt(hDlg,IDC_STARSIZE, iStarSize, FALSE);
	SetEditLim(hDlg, IDC_STARSIZE, 1);

	CheckDlgButton(hDlg, IDC_RANDOM, iRandom);
	CheckDlgButton(hDlg, IDC_WRAP, iWrap);
	CheckDlgButton(hDlg, IDC_ERASE, iErase);	
	CheckDlgButton(hDlg, IDC_CLS, iCls);

    SetScrollRange(hScrl, SB_CTL, 1, MIN_SPEED, FALSE);
	SetScrollPos(hScrl, SB_CTL, MIN_SPEED-wElapse, TRUE);

return TRUE;
}

BOOL bGetDlgItems(HWND hDlg)
{
	BOOL bRtn;

	iNumGalaxy = GetDlgItemInt(hDlg, IDC_NUMGALAXY, &bRtn, FALSE);
	iStarSize = GetDlgItemInt(hDlg, IDC_STARSIZE, &bRtn, FALSE);

	iRandom = IsDlgButtonChecked(hDlg, IDC_RANDOM);
	iWrap = IsDlgButtonChecked(hDlg, IDC_WRAP);
	iErase = IsDlgButtonChecked(hDlg, IDC_ERASE);
	
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
	LoadString(hMainInstance, IDS_RANDOM, szRandom, 20);
	LoadString(hMainInstance, IDS_NUMGALAXY, szNumGalaxy, 20);
	LoadString(hMainInstance, IDS_STARSIZE, szStarSize, 20);
	LoadString(hMainInstance, IDS_WRAP, szWrap, 20);
	LoadString(hMainInstance, IDS_ERASE, szErase, 20);
	LoadString(hMainInstance, IDS_CLS, szCls, 20);

}


BOOL GetIniSettings(void)
{	
	// Retrieve settings from INI file (note: for compatability with 16 bit version)

    wElapse		= SetProfileVal(FALSE, szAppName, szSpeed, DEF_SPEED, szRegEntry);
	iRandom		= SetProfileVal(FALSE, szAppName, szRandom, DEF_RANDOM, szRegEntry);
	iNumGalaxy	= SetProfileVal(FALSE, szAppName, szNumGalaxy, DEF_NUMGALAXY, szRegEntry);
	iStarSize	= SetProfileVal(FALSE, szAppName, szStarSize, DEF_STARSIZE, szRegEntry);
    iWrap		= SetProfileVal(FALSE, szAppName, szWrap, DEF_WRAP, szRegEntry);    
	iErase		= SetProfileVal(FALSE, szAppName, szErase, DEF_ERASE, szRegEntry);
	iCls		= SetProfileVal(FALSE, szAppName, szCls, DEF_CLS, szRegEntry);
	return TRUE;
}


BOOL SetIniSettings(void)
{
	SetProfileVal(TRUE, szAppName, szSpeed, wElapse, szRegEntry);
	SetProfileVal(TRUE, szAppName, szRandom, iRandom, szRegEntry);
	SetProfileVal(TRUE, szAppName, szNumGalaxy, iNumGalaxy|1, szRegEntry);
	SetProfileVal(TRUE, szAppName, szStarSize, iStarSize|1, szRegEntry);
	SetProfileVal(TRUE, szAppName, szWrap, iWrap, szRegEntry);
	SetProfileVal(TRUE, szAppName, szErase, iErase, szRegEntry);
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
