
// swarm.h

/*
** macros
*/
#define ID_TIMER 		1
#define PALETTESIZE		256

#define DEF_SPEED	 	15
#define DEF_BEECOUNT 	30
#define DEF_BEEACC		3
#define DEF_WASPACC		5
#define DEF_BEEVEL		12
#define DEF_WASPVEL		12
#define DEF_CLS			1

#define MIN_SPEED		100

#define PALSIZE 255
#define MAX_COLOR	255
#define MIN_COLOR	1
#define INC_COLOR	1			// color cycle speed

#define	X(t,b)	(sp->x[(t)*sp->beecount+(b)])
#define	Y(t,b)	(sp->y[(t)*sp->beecount+(b)])
#define RAND(v)	((rand()%(v))-((v)/2))				// rand number around 0

#define TIMES 			4  // max time positions recorded

/*
** structures
*/
typedef struct {
	int	x1, y1,
		x2,	y2;
} lseg;
            
typedef struct {
	int		r;
	long	startTime;
	int		width;
	int		height;
	int		border;
	int		beecount;
	lseg	*segs;			//	bee lines	
	lseg	*old_segs;		//	old bee lines
	short	*x;
	short	*y;
	short	*xv;
	short	*yv;
	short	wx[3];
	short	wy[3];
	short	wxv;
	short	wyv;
} swarmstruct;

/*
** global Vars
*/

TCHAR szAppName[APPNAMEBUFFERLEN];
TCHAR szIniFile[MAXFILELEN];
TCHAR szRegEntry[30];

TCHAR szDesc[30];
TCHAR szBeeAcc[20];
TCHAR szWaspAcc[20];
TCHAR szBeeVel[20];
TCHAR szWaspVel[20];
TCHAR szBeeCount[20];
TCHAR szCls[20];
TCHAR szSpeed[20];

int iBeeAcc;				// max bee acceleration = 3
int iWaspAcc;				// max wasp acceleration = 5
int iBeeVel;				// max bee velocity = 12
int iWaspVel;				// max wasp velocity = 12
int iBeeCount;	  			// number of bees
int	iCls;

WORD wTimer,        		// timer id
     wElapse;   			// speed parameter

static swarmstruct swarms;

NPLOGPALETTE pLogPal;	
HPALETTE hPal;				// our user defined palette handle

extern HINSTANCE	hMainInstance;	// instance handle

/*
**  function prototypes
*/
void MoveImage (HWND hWnd);
void DrawSegments(HDC hDC, lseg *segs, int seg_cnt);
BOOL bInitDlgItems(HWND, HWND);
BOOL bGetDlgItems(HWND);
void vLoadStrings(void);
BOOL GetIniSettings(void);
BOOL SetIniSettings(void);
int SetProfileVal(BOOL, LPCTSTR, LPCTSTR, INT, LPCTSTR);
void WriteProfileInt(LPCTSTR szSection, LPCTSTR szKey, int i, LPCTSTR szRoot);
void cycle_colors(LPINT, LPINT, LPINT, INT, INT, INT);
