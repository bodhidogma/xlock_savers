
// Mountain.h

/*
** macros
*/
#define ID_TIMER 		1
#define PALETTESIZE		256

/* ~ 5000 Max mountain height (1000 - 10000) */
#define MAXHEIGHT  (3 * (mp->width + mp->height))
#define WORLDWIDTH 50    /* World size x * y */

#define DEF_SPEED	 	200
#define DEF_CLS			1

#define MIN_SPEED		1000

#define PALSIZE 250
#define MAX_COLOR	255
#define MIN_COLOR	1
#define INC_COLOR	1			// color cycle speed

#define MAX_RAND(max) (rand() % (max))
#define RANGE_RAND(min,max) ((min) + rand() % ((max) - (min)))

/*
** structures
*/
typedef struct {
  int		pixelmode;
  int		width;
  int		height;
  int       h[WORLDWIDTH][WORLDWIDTH];
  int		time;           /* up time */  
} mountainstruct;

/*
** global Vars
*/

TCHAR szAppName[APPNAMEBUFFERLEN];
TCHAR szIniFile[MAXFILELEN];
TCHAR szRegEntry[30];

TCHAR szDesc[30];
TCHAR szCls[20];
TCHAR szSpeed[20];

int	iCls;

WORD wTimer,        		// timer id
     wElapse;   			// speed parameter

static mountainstruct mountains;

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
