/*-----------------
	Rotor.H
  -----------------*/

/*
** function prototypes
*/
void cycle_colors(LPINT, LPINT, LPINT, INT, INT, INT);
void MoveImage (HWND hWnd);
BOOL bInitDlgItems(HWND, HWND);
BOOL bGetDlgItems(HWND);
void vLoadStrings(void);
BOOL GetIniSettings(void);
BOOL SetIniSettings(void);
int SetProfileVal(BOOL, LPCTSTR, LPCTSTR, INT, LPCTSTR);

/*
** macros
*/
#define ID_TIMER    1

#define MAXANGLE    10000.0
#define MAX_COLOR   255
#define MIN_COLOR   1
#define CAT(X,Y)    X##Y

#define SCALE(W,N)		CAT(W,N) /= 4; CAT(W,N) += (CAT(fs->center,W)-18)
#define SCALEIFSMALL()	if (fs->smallscrn) {		\
							SCALE(x,1); SCALE(x,2);	\
							SCALE(y,1); SCALE(y,2);	\
						}

#define DEF_SEGLEN		100
#define DEF_SEGCNT		45
#define DEF_ROTORCNT	3
#define DEF_CLRSTART	1
#define DEF_CLRINC		1
#define DEF_SPEED		30

#define MIN_SPEED		1000

#define PALSIZE			255

/*
** structures
*/
struct elem {           // rotor segment
	float	angle;
	float	radius;
	float	start_radius;
	float	end_radius;
	float	radius_drift_max;
	float	radius_drift_now;
	float	ratio;
	float	start_ratio;
	float	end_ratio;
	float	ratio_drift_max;
	float	ratio_drift_now;
};

typedef struct tsflightstr {
	struct elem *elements;  // rotor elements
	int   r;
	int   lastx, lasty;
	int   num, rotor, prev,
	      *savex, *savey,
	      centerx, centery;
	float angle;
	BOOL  firsttime, smallscrn, forward, unused;
} flightstruct;

/*
** variables
*/
TCHAR szAppName[APPNAMEBUFFERLEN];
TCHAR szIniFile[MAXFILELEN];
TCHAR szRegEntry[30];

TCHAR szDesc[30];
TCHAR szSegLen[30];
TCHAR szSegCnt[30];
TCHAR szRotorCnt[30];
TCHAR szClrStart[30];
TCHAR szClrInc[30];
TCHAR szSpeed[30];

int	iSegLen;	        	// modifiable: 100	(save)
int iSegCnt;         		// modifiable: 45	(reps)
int iRotorCnt;			  	// modifiable: 2	(count)
int iClrStart;		  		// modifiable: 1
int	iClrInc;				// modifiable: 1

WORD  wTimer,  			// timer handle
      wElapse; 			// timer speed

static flightstruct flights;

NPLOGPALETTE	pLogPal;
HPALETTE		hPal;				// user defined palette

extern HINSTANCE	hMainInstance;	// instance handle
