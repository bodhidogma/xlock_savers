/* Quix.h
*/


/* Function prototypes
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


/* Macros
*/
#define ID_TIMER 		1
#define PALETTESIZE		256
#define DEF_SPEEDSET 	25

#define DEF_LINECOUNT	15
#define DEF_WIDTH		5
#define DEF_SEGS		3
#define DEF_CLS			0
#define DEF_RAINBOW		1

#define MAX_COLOR	255
#define MIN_COLOR	1
#define INC_COLOR	1			// color cycle speed

#define PALSIZE 255

#define check_bounds(qp, val, del, max)						\
{															\
	if ((val) < 0) {										\
		*(del) = (rand() % (qp)->delta) + (qp)->offset;		\
	} else if ((val) > (max)) {								\
		*(del) = -(rand() % (qp)->delta) - (qp)->offset;	\
	}														\
}


/*
** globals
*/
int		iLines,
		iWidth,
		iSegs,
		iCLS,
		iRainbow;

TCHAR szAppName[APPNAMEBUFFERLEN];
TCHAR szIniFile[MAXFILELEN];
TCHAR szRegEntry[30];

TCHAR szDesc[30];
TCHAR  szSpeedName[20];
TCHAR  szLinesName[20];
TCHAR  szWidthName[20];
TCHAR  szSegsName[20];
TCHAR  szClsName[20];
TCHAR  szRainbowName[20];

WORD wTimer,        		// timer id
     wElapse;   			// speed parameter

NPLOGPALETTE pLogPal;	
HPALETTE hPal;				// our user defined palette handle

/*
** Structures
*/
typedef struct {
	int		x;
	int		y;
} point;

typedef struct {
	int		x,	y,
			dx, dy;
} l_seg;

typedef struct {
	int		pix;
	long	startTime;
	int		first,
			last,
			offset,
			delta,
			width,
			height,
			nlines,
			ColorCount;			
	l_seg*  segs;
	point*	lineq;
} quixstruct;

static quixstruct quix;

