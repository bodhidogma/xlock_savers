/*
** Gravity.h
 */


/*
** macros
*/

#define ID_TIMER 		1
#define PALETTESIZE		256
#define DEF_SPEEDSET 	250

#define MAX_COLOR	255
#define MIN_COLOR	1
#define INC_COLOR	1		// color cycle speed

#define DEF_PLANETS		10
#define DEF_TRAILS		0
#define DEF_DAMPING		1
#define DEF_CYCLE 		1

#define PALSIZE 255

#define GRAV				-0.02		// gravitational constant
#define DIST				16.0
#define COLLIDE				0.0001
#define ALMOST				15.99
#define HALF				0.5
#define INTRINSIC_RADIUS	(float)(gp->height/5)
#define STARRADIUS			(int)(gp->height/(2*DIST))
#define AVG_RADIUS			(INTRINSIC_RADIUS/DIST)
#define RADIUS				(int)(INTRINSIC_RADIUS/(float)(POS(Z)+DIST))

#define XR					HALF*ALMOST
#define YR					HALF*ALMOST
#define ZR					HALF*ALMOST

#define VR					0.04

#define DIMENSIONS			3
#define X					0
#define Y					1
#define Z					2

#define DAMP		0.999999
#define MaxA		0.1			// max accelleration

#define POS(c)				planet->P[c]
#define VEL(c)				planet->V[c]
#define ACC(c)				planet->A[c]

#define REC_CORNERS(x,y,r)			(x-r),(y-r),(x+r),(y+r)

#define Planet(x,y)\
	if ((x) >= 0 && (y) >= 0 && (x) <= gp->width && (y) <= gp->height) {\
		if (planet->ri < 2)\
			SetPixel(hDC, (x),(y), PALETTEINDEX(planet->colors));\
		else\
			Ellipse(hDC, REC_CORNERS((x),(y),planet->ri/2));\
	}

#define FLOATRAND(min,max)		(min)+(rand()/(float)RAND_MAX)*((max)-(min))

/*
** structures
*/
typedef struct {
	double
		P[DIMENSIONS],
		V[DIMENSIONS],
		A[DIMENSIONS];
	int
		xi,
		yi,
		ri,
		colors;
} planetstruct;

typedef struct {
	int
		width,
		height,
		x, y,
		sr,
		nplanets,	
		starcolor;
	planetstruct *planets;
} gravstruct;

static gravstruct gravs;

/*
** global Vars
*/

TCHAR szAppName[APPNAMEBUFFERLEN];
TCHAR szIniFile[MAXFILELEN];
TCHAR szRegEntry[30];
TCHAR szDesc[30];
TCHAR szSpeedName[20];
TCHAR szPlanetName[20];
TCHAR szTrailName[20];
TCHAR szDampName[20];
TCHAR szCycleName[20];

WORD wTimer,        		// timer id
     wElapse;   			// speed parameter

NPLOGPALETTE pLogPal;	
HPALETTE hPal;				// our user defined palette handle

int	iPlanets = 10;			// # planets
int iTrails = 0;			// obit trails
int	iDamping = 1;			// Decaying orbits
int iColorCycle = 1;		// Color Cycling


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

void init_planet(planetstruct*);
void draw_planet(HDC, planetstruct*);

