
// SSaver.h

/*
** macros
*/
#define ID_TIMER 		1
#define PALETTESIZE		256

#define DEF_SPEED	 	20
#define DEF_NUMGALAXY	3
#define DEF_STARSIZE	1
#define DEF_RANDOM		1
#define DEF_WRAP		0
#define DEF_ERASE		0
#define DEF_CLS			1

#define MIN_SPEED		100

#define PALSIZE 255
#define MAX_COLOR	255
#define MIN_COLOR	1
#define INC_COLOR	1			// color cycle speed


//#define WRAP		1
//#define BOUNCE	1

#define MAX_GALAXIES		8
#define MAX_STARS			350
#define MAX_HITITERATIONS	300
#define MAX_IDELTAT			50

//#define DEFAULT_GALAXIES		3
//#define DEFAULT_STARS			1000
//#define DEFAULT_HITITERATIONS	7500
//#define DEFAULT_IDELTAT			200		//.02

#define GALAKSIZE		3.0
#define QCONS			0.001

#define COLOROFFSET		0
#define MAXCOLORS		256
#define COLORBASE		8
#define COLORSTEP	(MAXCOLORS/COLORBASE)		// 8 colors / galaxy

#ifndef	PI
#	define PI 3.1415926535
#endif

#define FLOATRAND ((double) rand() / ((double) RAND_MAX))

#define MyXDrawPoint(h,x,y,c)\
	if (gp->starsize > 1) {	 \
			Ellipse(hDC, x,y,x+gp->starsize,y+gp->starsize);\
	} else {	\
            SetPixelV(h,x,y,c);\
	}

typedef double Vector[3];
typedef double Matrix[3][3];

typedef struct {
	Vector pos, vel;
	int px, py;
	int color;
	} Star;
typedef struct {
	int mass;
	int starscnt;
	Star *stars;
	int basecolor;
	Vector pos, vel;
	} Galaxy;

typedef struct {
    struct {
	int left;                             /* x minimum */
	int right;                            /* x maximum */
	int top;                              /* y minimum */
	int bottom;                           /* y maximum */
    } clip;				      
    int galcol[MAX_GALAXIES];                 /* colors */
    Matrix mat;                               /* Movement of stars(?) */
    double scale;                             /* Scale */
    int midx;                                 /* Middle of screen, x */
    int midy;                                 /* Middle of screen, y */
    double size;                              /* */
    Vector diff;                              /* */
    Galaxy galaxies[MAX_GALAXIES];            /* the Whole Universe */
    double f_deltat;                          /* quality of calculation, calc'd by d_ideltat */
    int f_galaxies;                           /* # galaxies */
    int f_stars;                              /* # stars per galaxy */
    int f_hititerations;                      /* # iterations before restart */
    int step;                                 /* */
    int init;                                 /* 1 -> re-initialize */
    int starsize;
} unistruct;


/*
** global Vars
*/

TCHAR szAppName[APPNAMEBUFFERLEN];
TCHAR szIniFile[MAXFILELEN];
TCHAR szRegEntry[30];

TCHAR szDesc[30];
TCHAR szSpeed[20];
TCHAR szRandom[20];
TCHAR szNumGalaxy[20];
TCHAR szStarSize[20];
TCHAR szWrap[20];
TCHAR szErase[20];
TCHAR szCls[20];

int iRandom;				// max bee acceleration = 3
int iStarSize;
int iNumGalaxy;				// max wasp acceleration = 5
int iWrap;					// max bee velocity = 12
int iErase;					// max wasp velocity = 12
int	iCls;

WORD wTimer,        		// timer id
     wElapse;   			// speed parameter


static unistruct universes;

NPLOGPALETTE pLogPal;	
HPALETTE hPal;				// our user defined palette handle

extern HINSTANCE	hMainInstance;	// instance handle

/*
**  function prototypes
*/
void MoveImage (HWND hWnd);
BOOL bInitDlgItems(HWND, HWND);
BOOL bGetDlgItems(HWND);
void vLoadStrings(void);
BOOL GetIniSettings(void);
BOOL SetIniSettings(void);
int SetProfileVal(BOOL, LPCTSTR, LPCTSTR, INT, LPCTSTR);
void WriteProfileInt(LPCTSTR szSection, LPCTSTR szKey, int i, LPCTSTR szRoot);
void cycle_colors(LPINT, LPINT, LPINT, INT, INT, INT);
