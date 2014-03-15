// Copyright (C) 2001-2002 Raven Software.
//
#ifndef __Q_SHARED_H
#define __Q_SHARED_H

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file

//#define	Q3_VERSION		"SOF2MP V0.01"	sent on 11/26/2001
//#define	Q3_VERSION		"SOF2MP V0.02"	sent on 12/10/2001
//#define	Q3_VERSION		"SOF2MP V0.03"  sent on 12/16/2001
//#define	Q3_VERSION		"SOF2MP V0.081" sent on 1/15/2002
//#define	Q3_VERSION		"SOF2MP V0.09"	sent on 1/24/2002
//#define	Q3_VERSION		"SOF2MP V0.10"	sent on 1/31/2002
//#define	Q3_VERSION		"SOF2MP V0.11"	sent on 2/7/2002
//#define	Q3_VERSION		"SOF2MP V0.12"	sent on 2/14/2002
//#define	Q3_VERSION		"SOF2MP V0.13"	sent on 2/21/2002
//#define	Q3_VERSION		"SOF2MP V0.13b"	public beta #1 on 3/1/3002
//#define	Q3_VERSION		"SOF2MP V0.14"	sent on 3/4/2002
//#define	Q3_VERSION		"SOF2MP V0.15"	sent on 3/11/2002
//#define	Q3_VERSION		"SOF2MP V0.15b" public beta #2 on 3/13/2002
//#define	Q3_VERSION		"SOF2MP V0.16"	sent on 3/18/2002
//#define	Q3_VERSION		"SOF2MP V0.16b"	public beta #3 on 3/20/2002
//#define	Q3_VERSION		"SOF2MP V0.17"	sent on 3/24/2002
//#define	Q3_VERSION		"SOF2MP TEST V1.01t"	sent on 3/28/2002
//#define	Q3_VERSION		"SOF2MP V0.18"	sent on 4/1/2002 - April Fools!
//#define	Q3_VERSION		"SOF2MP V1.02t"	sent on 4/5/2002
//#define	Q3_VERSION		"SOF2MP V0.19"	sent on 4/8/2002
//#define	Q3_VERSION		"SOF2MP V0.20"	sent on 4/15/2002 - Tax Day!
//#define	Q3_VERSION		"SOF2MP V0.21"	sent on 4/22/2002
//#define	Q3_VERSION		"SOF2MP V1.00.22"	sent on 4/26/2002
//#define	Q3_VERSION		"SOF2MP V1.00.23"	sent on 4/27/2002
#ifdef GERMAN_BUILD
	#define	Q3_VERSION		"SOF2MP V1.00g"
	#define SOF2_VERSION_ID	"1.00g"
#else
	#define	Q3_VERSION		"SOF2MP V1.00"
	#define SOF2_VERSION_ID	"1.00"
#endif

//#define SPECIAL_PRE_CACHE	1

//#ifdef _DEBUG
#define _SOF2_BOTS
//#endif
#define _spMaps

#define MAX_TEAMNAME 32

#include "../qcommon/disablewarnings.h"

/**********************************************************************
  VM Considerations

  The VM can not use the standard system headers because we aren't really
  using the compiler they were meant for.  We use bg_lib.h which contains
  prototypes for the functions we define for our own use in bg_lib.c.

  When writing mods, please add needed headers HERE, do not start including
  stuff like <stdio.h> in the various .c files that make up each of the VMs
  since you will be including system headers files can will have issues.

  Remember, if you use a C library function that is not defined in bg_lib.c,
  you will have to add your own version for support in the VM.

 **********************************************************************/
#ifdef Q3_VM

#include "bg_lib.h"

#define assert(exp)     ((void)0)

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))

#else
#define _CRT_SECURE_NO_WARNINGS
//#include <Windows.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <pthread.h>
#include "sqlite/sqlite3.h"
#endif

#ifdef _WIN32

//#pragma intrinsic( memset, memcpy )

#endif

#define EFFECT_SMOKE 0
#define EFFECT_EXPLOSION 1
#define EFFECT_SPARK_EXPLOSION 2

// this is the define for determining if we have an asm version of a C function
#if (defined _M_IX86 || defined __i386__) && !defined __sun__  && !defined __LCC__
#define id386	1
#else
#define id386	0
#endif

#if (defined(powerc) || defined(powerpc) || defined(ppc) || defined(__ppc) || defined(__ppc__)) && !defined(C_ONLY)
#define idppc	1
#else
#define idppc	0
#endif

// for windows fastcall option

#define	QDECL

short   ShortSwap (short l);
int		LongSwap (int l);
float	FloatSwap (const float *f);

//======================= WIN32 DEFINES =================================

#ifdef WIN32

#define	MAC_STATIC

#undef QDECL
#define	QDECL	__cdecl

// buildstring will be incorporated into the version string
#ifdef NDEBUG
#ifdef _M_IX86
#define	CPUSTRING	"win-x86"
#elif defined _M_ALPHA
#define	CPUSTRING	"win-AXP"
#endif
#else
#ifdef _M_IX86
#define	CPUSTRING	"win-x86-debug"
#elif defined _M_ALPHA
#define	CPUSTRING	"win-AXP-debug"
#endif
#endif

#define ID_INLINE __inline 

static ID_INLINE short BigShort( short l) { return ShortSwap(l); }
#define LittleShort
static ID_INLINE int BigLong(int l) { return LongSwap(l); }
#define LittleLong
static ID_INLINE float BigFloat(const float l) { return FloatSwap(&l); }
#define LittleFloat

#define	PATH_SEP '\\'

#endif

//======================= MAC OS X DEFINES =====================

#if defined(MACOS_X)

#define MAC_STATIC
#define __cdecl
#define __declspec(x)
#define stricmp strcasecmp
#define ID_INLINE inline 

#ifdef __ppc__
#define CPUSTRING	"MacOSX-ppc"
#elif defined __i386__
#define CPUSTRING	"MacOSX-i386"
#else
#define CPUSTRING	"MacOSX-other"
#endif

#define	PATH_SEP	'/'

#define __rlwimi(out, in, shift, maskBegin, maskEnd) asm("rlwimi %0,%1,%2,%3,%4" : "=r" (out) : "r" (in), "i" (shift), "i" (maskBegin), "i" (maskEnd))
#define __dcbt(addr, offset) asm("dcbt %0,%1" : : "b" (addr), "r" (offset))

static inline unsigned int __lwbrx(register void *addr, register int offset) {
    register unsigned int word;
    
    asm("lwbrx %0,%2,%1" : "=r" (word) : "r" (addr), "b" (offset));
    return word;
}

static inline unsigned short __lhbrx(register void *addr, register int offset) {
    register unsigned short halfword;
    
    asm("lhbrx %0,%2,%1" : "=r" (halfword) : "r" (addr), "b" (offset));
    return halfword;
}

static inline float __fctiw(register float f) {
    register float fi;
    
    asm("fctiw %0,%1" : "=f" (fi) : "f" (f));

    return fi;
}

#define BigShort
static inline short LittleShort(short l) { return ShortSwap(l); }
#define BigLong
static inline int LittleLong (int l) { return LongSwap(l); }
#define BigFloat
static inline float LittleFloat (const float l) { return FloatSwap(&l); }

#endif

//======================= MAC DEFINES =================================

#ifdef __MACOS__

#include <MacTypes.h>
#define	MAC_STATIC
#define ID_INLINE inline 

#define	CPUSTRING	"MacOS-PPC"

#define	PATH_SEP ':'

void Sys_PumpEvents( void );

#define BigShort
static inline short LittleShort(short l) { return ShortSwap(l); }
#define BigLong
static inline int LittleLong (int l) { return LongSwap(l); }
#define BigFloat
static inline float LittleFloat (const float l) { return FloatSwap(&l); }

#endif

//======================= LINUX DEFINES =================================

// the mac compiler can't handle >32k of locals, so we
// just waste space and make big arrays static...
#ifdef __linux__

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// bk001205 - from Makefile
#define stricmp strcasecmp

#define	MAC_STATIC // bk: FIXME
#define ID_INLINE inline 

#ifdef __i386__
#define	CPUSTRING	"linux-i386"
#elif defined __axp__
#define	CPUSTRING	"linux-alpha"
#else
#define	CPUSTRING	"linux-other"
#endif

#define	PATH_SEP '/'

// bk001205 - try
#ifdef Q3_STATIC
#define	GAME_HARD_LINKED
#define	CGAME_HARD_LINKED
#define	UI_HARD_LINKED
#define	BOTLIB_HARD_LINKED
#endif

#if !idppc
inline static short BigShort( short l) { return ShortSwap(l); }
#define LittleShort
inline static int BigLong(int l) { return LongSwap(l); }
#define LittleLong
inline static float BigFloat(const float *l) { return FloatSwap(l); }
#define LittleFloat
#else
#define BigShort
inline static short LittleShort(short l) { return ShortSwap(l); }
#define BigLong
inline static int LittleLong (int l) { return LongSwap(l); }
#define BigFloat
inline static float LittleFloat (const float *l) { return FloatSwap(l); }
#endif

#endif

//======================= FreeBSD DEFINES =====================
#ifdef __FreeBSD__ // rb010123

#define stricmp strcasecmp

#define MAC_STATIC
#define ID_INLINE inline 

#ifdef __i386__
#define CPUSTRING       "freebsd-i386"
#elif defined __axp__
#define CPUSTRING       "freebsd-alpha"
#else
#define CPUSTRING       "freebsd-other"
#endif

#define	PATH_SEP '/'

// bk010116 - omitted Q3STATIC (see Linux above), broken target

#if !idppc
static short BigShort( short l) { return ShortSwap(l); }
#define LittleShort
static int BigLong(int l) { LongSwap(l); }
#define LittleLong
static float BigFloat(const float *l) { FloatSwap(l); }
#define LittleFloat
#else
#define BigShort
static short LittleShort(short l) { return ShortSwap(l); }
#define BigLong
static int LittleLong (int l) { return LongSwap(l); }
#define BigFloat
static float LittleFloat (const float *l) { return FloatSwap(l); }
#endif

#endif

//=============================================================

typedef unsigned char 		byte;
typedef unsigned short		word;
#ifndef __linux__
typedef unsigned long		ulong;
#endif

typedef enum {qfalse, qtrue}	qboolean;

typedef int		qhandle_t;
typedef int		thandle_t;
typedef int		fxHandle_t;
typedef int		sfxHandle_t;
typedef int		fileHandle_t;
typedef int		clipHandle_t;


#ifndef NULL
#define NULL ((void *)0)
#endif

#define	MAX_QINT			0x7fffffff
#define	MIN_QINT			(-MAX_QINT-1)


// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

// Boe!Man 10/10/11: MAX_INFO_STRING from 512 to 1024, fix for crash issues.
#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		1024
#define	MAX_INFO_VALUE		1024

#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		  8192
#define	BIG_INFO_VALUE		8192


#define	MAX_QPATH			64		// max length of a quake game pathname
#ifdef PATH_MAX
#define MAX_OSPATH			PATH_MAX
#else
#define	MAX_OSPATH			256		// max length of a filesystem pathname
#endif

#define	MAX_NAME_LENGTH		32		// max length of a client name

#define	MAX_SAY_TEXT	150

// paramters for command buffer stuffing
typedef enum {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
						// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
} cbufExec_t;


//
// these aren't needed by any of the VMs.  put in another header?
//
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility


#define LS_STYLES_START			0
#define LS_NUM_STYLES			32
#define	LS_SWITCH_START			(LS_STYLES_START+LS_NUM_STYLES)
#define LS_NUM_SWITCH			32
#define MAX_LIGHT_STYLES		64

// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum {
	PRINT_ALL,
	PRINT_DEVELOPER,		// only print when "developer 1"
	PRINT_WARNING,
	PRINT_ERROR
} printParm_t;


#ifdef ERR_FATAL
#undef ERR_FATAL			// this is be defined in malloc.h
#endif

// parameters to the main Error routine
typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
	ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;


// font rendering values used by ui and cgame

#define PROP_GAP_WIDTH			3
#define PROP_SPACE_WIDTH		8
#define PROP_HEIGHT				27
#define PROP_SMALL_SIZE_SCALE	0.75

#define BLINK_DIVISOR			200
#define PULSE_DIVISOR			75

#define UI_LEFT			0x00000000	// default
#define UI_CENTER		0x00000001
#define UI_RIGHT		0x00000002
#define UI_FORMATMASK	0x00000007
#define UI_DROPSHADOW	0x00000800
#define UI_BLINK		0x00001000
#define UI_INVERSE		0x00002000
#define UI_PULSE		0x00004000


// Draw text defines
#define DT_FORCECOLOR	0x00000001
#define DT_OUTLINE		0x00000002
#define DT_DROPSHADOW	0x00000004

#if defined(_DEBUG) && !defined(BSPC)
	#define HUNK_DEBUG
#endif

typedef enum {
	h_high,
	h_low,
	h_dontcare
} ha_pref;

#ifdef HUNK_DEBUG
#define Hunk_Alloc( size, preference )				Hunk_AllocDebug(size, preference, #size, __FILE__, __LINE__)
void *Hunk_AllocDebug( int size, ha_pref preference, char *label, char *file, int line );
#else
void *Hunk_Alloc( int size, ha_pref preference );
#endif

void Com_Memset (void* dest, const int val, const size_t count);
void Com_Memcpy (void* dest, const void* src, const size_t count);

#define CIN_system	1
#define CIN_loop	2
#define	CIN_hold	4
#define CIN_silent	8
#define CIN_shader	16

/*
==============================================================

MATHLIB

==============================================================
*/


typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef vec3_t	vec3pair_t[2];

typedef int ivec3_t[3];
typedef int ivec4_t[4];
typedef int ivec5_t[5];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#ifndef M_PI
#define M_PI		3.14159265358979323846f	// matches value in gcc v2 math.h
#endif

#define SQRT3		1.732050808f

#define NUMVERTEXNORMALS	162
extern	vec3_t	bytedirs[NUMVERTEXNORMALS];

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		480

#define TINYCHAR_WIDTH		(SMALLCHAR_WIDTH)
#define TINYCHAR_HEIGHT		(SMALLCHAR_HEIGHT/2)

#define SMALLCHAR_WIDTH		8
#define SMALLCHAR_HEIGHT	16

#define BIGCHAR_WIDTH		12	// 16
#define BIGCHAR_HEIGHT		13  // 16

#define	GIANTCHAR_WIDTH		32
#define	GIANTCHAR_HEIGHT	48

typedef enum
{
CT_NONE,
CT_BLACK,
CT_RED,
CT_GREEN,
CT_BLUE,
CT_YELLOW,
CT_MAGENTA,
CT_CYAN,
CT_WHITE,
CT_LTGREY,
CT_MDGREY,
CT_DKGREY,
CT_DKGREY2,

CT_VLTORANGE,
CT_LTORANGE,
CT_DKORANGE,
CT_VDKORANGE,

CT_VLTBLUE1,
CT_LTBLUE1,
CT_DKBLUE1,
CT_VDKBLUE1,

CT_VLTBLUE2,
CT_LTBLUE2,
CT_DKBLUE2,
CT_VDKBLUE2,

CT_VLTBROWN1,
CT_LTBROWN1,
CT_DKBROWN1,
CT_VDKBROWN1,

CT_VLTGOLD1,
CT_LTGOLD1,
CT_DKGOLD1,
CT_VDKGOLD1,

CT_VLTPURPLE1,
CT_LTPURPLE1,
CT_DKPURPLE1,
CT_VDKPURPLE1,

CT_VLTPURPLE2,
CT_LTPURPLE2,
CT_DKPURPLE2,
CT_VDKPURPLE2,

CT_VLTPURPLE3,
CT_LTPURPLE3,
CT_DKPURPLE3,
CT_VDKPURPLE3,

CT_VLTRED1,
CT_LTRED1,
CT_DKRED1,
CT_VDKRED1,
CT_VDKRED,
CT_DKRED,

CT_VLTAQUA,
CT_LTAQUA,
CT_DKAQUA,
CT_VDKAQUA,

CT_LTPINK,
CT_DKPINK,
CT_LTCYAN,
CT_DKCYAN,
CT_LTBLUE3,
CT_BLUE3,
CT_DKBLUE3,

CT_MAX
} ct_table_t;

extern vec4_t colorTable[CT_MAX];

extern	vec4_t		colorBlack;
extern	vec4_t		colorRed;
extern	vec4_t		colorGreen;
extern	vec4_t		colorBlue;
extern	vec4_t		colorYellow;
extern	vec4_t		colorMagenta;
extern	vec4_t		colorCyan;
extern	vec4_t		colorWhite;
extern	vec4_t		colorLtGrey;
extern	vec4_t		colorMdGrey;
extern	vec4_t		colorDkGrey;
extern	vec4_t		colorLtBlue;
extern	vec4_t		colorDkBlue;

#define Q_COLOR_ESCAPE	'^'
#define Q_IsColorString(p)	( p && *(p) == Q_COLOR_ESCAPE && *((p)+1) && *((p)+1) != Q_COLOR_ESCAPE )

#define COLOR_BLACK		'0'
#define COLOR_RED		'1'
#define COLOR_GREEN		'2'
#define COLOR_YELLOW	'3'
#define COLOR_BLUE		'4'
#define COLOR_CYAN		'5'
#define COLOR_MAGENTA	'6'
#define COLOR_WHITE		'7'
#define ColorIndex(c)	( ( (c) - '0' ) & 7 )

#define S_COLOR_BLACK	"^0"
#define S_COLOR_RED		"^1"
#define S_COLOR_GREEN	"^2"
#define S_COLOR_YELLOW	"^3"
#define S_COLOR_BLUE	"^4"
#define S_COLOR_CYAN	"^5"
#define S_COLOR_MAGENTA	"^6"
#define S_COLOR_WHITE	"^7"

extern vec4_t	g_color_table[8];

#define	MAKERGB( v, r, g, b ) v[0]=r;v[1]=g;v[2]=b
#define	MAKERGBA( v, r, g, b, a ) v[0]=r;v[1]=g;v[2]=b;v[3]=a

#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )

struct cplane_s;

extern	vec3_t	vec3_origin;
extern	vec3_t	vec3_identity;
extern	vec3_t	axisDefault[3];

#define	nanmask (255<<23)

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

#if idppc

static inline float Q_rsqrt( float number ) {
		float x = 0.5f * number;
                float y;
#ifdef __GNUC__            
                asm("frsqrte %0,%1" : "=f" (y) : "f" (number));
#else
		y = __frsqrte( number );
#endif
		return y * (1.5f - (x * y * y));
	}

#ifdef __GNUC__            
static inline float Q_fabs(float x) {
    float abs_x;
    
    asm("fabs %0,%1" : "=f" (abs_x) : "f" (x));
    return abs_x;
}
#else
#define Q_fabs __fabsf
#endif

#else
float Q_fabs( float f );
float Q_rsqrt( float f );		// reciprocal square root
#endif

#define SQRTFAST( x ) ( (x) * Q_rsqrt( x ) )

signed char ClampChar( int i );
signed short ClampShort( int i );
#ifdef Q3_VM 
float powf ( float x, int y );
#endif
// this isn't a real cheap function to call!
int DirToByte( vec3_t dir );
void ByteToDir( int b, vec3_t dir );

#if	1

#define minimum(x,y) ((x)<(y)?(x):(y))
#define maximum(x,y) ((x)>(y)?(x):(y))

#define DotProduct(x,y)					((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)			((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)				((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)					((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define VectorCopy4(a,b)				((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define	VectorScale(v, s, o)			((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)			((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))
#define VectorInc(v)					((v)[0] += 1.0f,(v)[1] += 1.0f,(v)[2] +=1.0f)
#define VectorDec(v)					((v)[0] -= 1.0f,(v)[1] -= 1.0f,(v)[2] -=1.0f)
#define VectorInverseScaleVector(a,b,c)	((c)[0]=(a)[0]/(b)[0],(c)[1]=(a)[1]/(b)[1],(c)[2]=(a)[2]/(b)[2])
#define VectorScaleVectorAdd(c,a,b,o)	((o)[0]=(c)[0]+((a)[0]*(b)[0]),(o)[1]=(c)[1]+((a)[1]*(b)[1]),(o)[2]=(c)[2]+((a)[2]*(b)[2]))
#define VectorAdvance(a,s,b,c)			(((c)[0]=(a)[0] + s * ((b)[0] - (a)[0])),((c)[1]=(a)[1] + s * ((b)[1] - (a)[1])),((c)[2]=(a)[2] + s * ((b)[2] - (a)[2])))
#define VectorAverage(a,b,c)			(((c)[0]=((a)[0]+(b)[0])*0.5f),((c)[1]=((a)[1]+(b)[1])*0.5f),((c)[2]=((a)[2]+(b)[2])*0.5f))
#define VectorScaleVector(a,b,c)		(((c)[0]=(a)[0]*(b)[0]),((c)[1]=(a)[1]*(b)[1]),((c)[2]=(a)[2]*(b)[2]))

#else

#define DotProduct(x,y)			_DotProduct(x,y)
#define VectorSubtract(a,b,c)	_VectorSubtract(a,b,c)
#define VectorAdd(a,b,c)		_VectorAdd(a,b,c)
#define VectorCopy(a,b)			_VectorCopy(a,b)
#define	VectorScale(v, s, o)	_VectorScale(v,s,o)
#define	VectorMA(v, s, b, o)	_VectorMA(v,s,b,o)

#endif

#ifdef __LCC__
#ifdef VectorCopy
#undef VectorCopy
// this is a little hack to get more efficient copies in our interpreter
typedef struct {
	float	v[3];
} vec3struct_t;
#define VectorCopy(a,b)	(*(vec3struct_t *)b=*(vec3struct_t *)a)
#define ID_INLINE static
#endif
#endif

#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define VectorSet5(v,x,y,z,a,b)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z), (v)[3]=(a), (v)[4]=(b))
#define Vector4Copy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define	SnapVector(v) {v[0]=((int)(v[0]));v[1]=((int)(v[1]));v[2]=((int)(v[2]));}
// just in case you do't want to use the macros
vec_t _DotProduct( const vec3_t v1, const vec3_t v2 );
void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorCopy( const vec3_t in, vec3_t out );
void _VectorScale( const vec3_t in, float scale, vec3_t out );
void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc );

unsigned ColorBytes3 (float r, float g, float b);
unsigned ColorBytes4 (float r, float g, float b, float a);

float NormalizeColor( const vec3_t in, vec3_t out );

float RadiusFromBounds( const vec3_t mins, const vec3_t maxs );
void ClearBounds( vec3_t mins, vec3_t maxs );
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );

#ifndef __LCC__
static ID_INLINE int VectorCompare( const vec3_t v1, const vec3_t v2 ) {
	if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) {
		return 0;
	}			
	return 1;
}

static ID_INLINE vec_t VectorLength( const vec3_t v ) {
	return (vec_t)sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static ID_INLINE vec_t VectorLengthSquared( const vec3_t v ) {
	return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static ID_INLINE vec_t Distance( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return VectorLength( v );
}

static ID_INLINE vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

// fast vector normalize routine that does not check to make sure
// that length != 0, nor does it return length, uses rsqrt approximation
static ID_INLINE void VectorNormalizeFast( vec3_t v )
{
	float ilength;

	ilength = Q_rsqrt( DotProduct( v, v ) );

	v[0] *= ilength;
	v[1] *= ilength;
	v[2] *= ilength;
}

static ID_INLINE void VectorInverse( vec3_t v ){
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

static ID_INLINE void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

#else
int VectorCompare( const vec3_t v1, const vec3_t v2 );

vec_t VectorLength( const vec3_t v );

vec_t VectorLengthSquared( const vec3_t v );

vec_t Distance( const vec3_t p1, const vec3_t p2 );

vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 );
 
void VectorNormalizeFast( vec3_t v );

void VectorInverse( vec3_t v );

void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );

#endif

vec_t VectorNormalize (vec3_t v);		// returns vector length
vec_t VectorNormalize2( const vec3_t v, vec3_t out );
void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out );
void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out );

#ifndef __LCC__
#ifndef	__GNUC__

static ID_INLINE int VectorToInt(vec3_t vec)
{
	int			tmp, retval;

	_asm
	{
		push	edx
		mov		edx, [vec]
		fld		dword ptr[edx + 0]
		fld		dword ptr[edx + 4]
		fld		dword ptr[edx + 8]

		mov		eax, 0xff00

		fistp	tmp	   
		mov		al, byte ptr [tmp]
		shl		eax, 16
		
		fistp	tmp
		mov		ah, byte ptr [tmp]

		fistp	tmp
		mov		al, byte ptr [tmp]

		mov		[retval], eax
		pop		edx
	}
	return(retval);
}

static ID_INLINE int qftol( float f ) 
{
	int		temp;

	_asm
	{
		fld f
		fistp temp
		mov eax, temp
	}
	return(temp);
}

#endif // __linux__
#endif


qboolean Q_TestRaySphere ( vec3_t origin, float radius, const vec3_t start, const vec3_t end );

int Q_log2(int val);

float Q_acos(float c);
float Q_asin(float c);

int		Q_rand( int *seed );
float	Q_random( int *seed );
float	Q_crandom( int *seed );

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

void vectoangles( const vec3_t value1, vec3_t angles);
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );

void AxisClear( vec3_t axis[3] );
void AxisCopy( vec3_t in[3], vec3_t out[3] );

void SetPlaneSignbits( struct cplane_s *out );
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *plane);

#ifdef Q3_VM
double fmod( double x, double y );
#endif
float	AngleMod(float a);
float	LerpAngle (float from, float to, float frac);
void	LerpVector ( vec3_t from, vec3_t to, float lerp, vec3_t out );
float	AngleSubtract( float a1, float a2 );
void	AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 );

float AngleNormalize360 ( float angle );
float AngleNormalize180 ( float angle );
float AngleDelta ( float angle1, float angle2 );

qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );
void RotateAroundDirection( vec3_t axis[3], float yaw );
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up );
// perpendicular vector could be replaced by this

//int	PlaneTypeForNormal (vec3_t normal);

void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void PerpendicularVector( vec3_t dst, const vec3_t src );
void NormalToLatLong( const vec3_t normal, byte bytes[2] );


//=============================================

float Com_Clampf( float min, float max, float value );
int Com_Clamp( int min, int max, int value );

char	*COM_SkipPath( char *pathname );
void	COM_StripExtension( const char *in, char *out );
void	COM_DefaultExtension( char *path, int maxSize, const char *extension );

void	COM_BeginParseSession( const char *name );
int		COM_GetCurrentParseLine( void );
const char	*SkipWhitespace( const char *data, qboolean *hasNewLines );
char	*COM_Parse( const char **data_p );
char	*COM_ParseExt( const char **data_p, qboolean allowLineBreak );
int		COM_Compress( char *data_p );
void	COM_ParseError( char *format, ... );
void	COM_ParseWarning( char *format, ... );
//int		COM_ParseInfos( char *buf, int max, char infos[][MAX_INFO_STRING] );

#define MAX_TOKENLENGTH		1024

#ifndef TT_STRING
//token types
#define TT_STRING					1			// string
#define TT_LITERAL					2			// literal
#define TT_NUMBER					3			// number
#define TT_NAME						4			// name
#define TT_PUNCTUATION				5			// punctuation
#endif

typedef struct pc_token_s
{
	int type;
	int subtype;
	int intvalue;
	float floatvalue;
	char string[MAX_TOKENLENGTH];
} pc_token_t;

// data is an in/out parm, returns a parsed out token

void	COM_MatchToken( const char**buf_p, char *match );

void SkipBracedSection (const char **program);
void SkipRestOfLine ( const char **data );

void Parse1DMatrix (const char **buf_p, int x, float *m);
void Parse2DMatrix (const char **buf_p, int y, int x, float *m);
void Parse3DMatrix (const char **buf_p, int z, int y, int x, float *m);

int	QDECL Com_sprintf (char *dest, int size, const char *fmt, ...);


// mode parm for FS_FOpenFile
typedef enum {
	FS_READ,
	FS_WRITE,
	FS_APPEND,
	FS_APPEND_SYNC,
	FS_READ_TEXT,
	FS_WRITE_TEXT,
	FS_APPEND_TEXT,
	FS_APPEND_SYNC_TEXT
} fsMode_t;

typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

// Henk 12/01/10 -> Added current_gametypes cvar
typedef enum {
	GT_NONE,
	GT_HS,
	GT_NONE1,
	GT_INF,
	GT_CTF,
	GT_DM,
	GT_TDM,
	GT_ELIM,
	GT_HZ
} gameTypes_t;
// Henk 12/01/10 -> End

// Henk 08/03/10 -> Added option to enable/disable nades and stuff for H&S
typedef enum {
	MDN11,
	F1,
	L2A2,
	GOGGLES,
	BRIEFCASE,
	RANDOMGRENADE,
} hideseekExtra_t;
// Henk 08/03/10 -> End

#ifdef _DEBUG
// Boe!Man 10/9/13: Add options for 1fx_debug CVAR.
typedef enum {
	MODDBG_ALL = 1,
	MODDBG_HIDESEEK = 2,
	MODDBG_ADMCMDS = 4,
	MODDBG_CM = 8
} debugMode_t;
#endif

//=============================================

int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );

// portable case insensitive compare
int		Q_stricmp (const char *s1, const char *s2);
int		Q_strncmp (const char *s1, const char *s2, int n);
int		Q_stricmpn (const char *s1, const char *s2, int n);
char	*Q_strlwr( char *s1 );
char	*Q_strupr( char *s1 );
char	*Q_strrchr( const char* string, int c );

// buffer size safe library replacements
void	Q_strncpyz( char *dest, const char *src, int destsize );
void	Q_strcat( char *dest, int size, const char *src );

// strlen that discounts Quake color sequences
int Q_PrintStrlen( const char *string );
// removes color sequences from string
char *Q_CleanStr( char *string );

//=============================================

// 64-bit integers for global rankings interface
// implemented as a struct for qvm compatibility
typedef struct
{
	byte	b0;
	byte	b1;
	byte	b2;
	byte	b3;
	byte	b4;
	byte	b5;
	byte	b6;
	byte	b7;
} qint64;

//=============================================
/*
short	BigShort(short l);
short	LittleShort(short l);
int		BigLong (int l);
int		LittleLong (int l);
qint64  BigLong64 (qint64 l);
qint64  LittleLong64 (qint64 l);
float	BigFloat (const float *l);
float	LittleFloat (const float *l);

void	Swap_Init (void);
*/
char	* QDECL va(char *format, ...);

//=============================================

//
// key / value info strings
//
char *Info_ValueForKey( const char *s, const char *key );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_big( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
qboolean Info_Validate( const char *s );
void Info_NextPair( const char **s, char *key, char *value );

void	QDECL Com_Error( int level, const char *error, ... );
void	QDECL Com_Printf( const char *msg, ... );


/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

#define	CVAR_ARCHIVE		0x00000001		// set to cause it to be saved to vars.rc
											// used for system variables, not for player
											// specific configurations
#define	CVAR_USERINFO		0x00000002		// sent to server on connect or change
#define	CVAR_SERVERINFO		0x00000004		// sent in response to front end requests
#define	CVAR_SYSTEMINFO		0x00000008		// these cvars will be duplicated on all clients
#define	CVAR_INIT			0x00000010		// don't allow change from console at all,
											// but can be set from the command line
#define	CVAR_LATCH			0x00000020		// will only change when C code next does
											// a Cvar_Get(), so it can't be changed
											// without proper initialization.  modified
											// will be set, even though the value hasn't
											// changed yet
#define	CVAR_ROM			0x00000040		// display only, cannot be set by user at all (can be set by code)
#define	CVAR_USER_CREATED	0x00000080		// created by a set command
#define	CVAR_TEMP			0x00000100		// can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT			0x00000200		// can not be changed if cheats are disabled
#define CVAR_NORESTART		0x00000400		// do not clear when a cvar_restart is issued
#define CVAR_INTERNAL		0x00000800		// cvar won't be displayed, ever (for passwords and such)
#define	CVAR_PARENTAL		0x00001000		// lets cvar system know that parental stuff needs to be updated
#define CVAR_LOCK_RANGE		0x00002000		// enforces the mins / maxs

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s {
	char		*name;
	char		*string;
	char		*resetString;		// cvar_restart will reset to this value
	char		*latchedString;		// for CVAR_LATCH vars
	float		mMinValue, mMaxValue;
	int			flags;
	qboolean	modified;			// set each time the cvar is changed
	int			modificationCount;	// incremented each time the cvar is changed
	float		value;				// atof( string )
	int			integer;			// atoi( string )
	struct cvar_s *next;
	struct cvar_s *hashNext;
} cvar_t;

#define	MAX_CVAR_VALUE_STRING	256

typedef int	cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct {
	cvarHandle_t	handle;
	int			modificationCount;
	float		value;
	int			integer;
	char		string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#include "surfaceflags.h"			// shared with the q3map utility

// plane types are used to speed some tests
// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2
#define	PLANE_NON_AXIAL	3


/*
=================
PlaneTypeForNormal
=================
*/

#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL) ) )

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s {
	vec3_t	normal;
	float	dist;
	byte	type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
	byte	signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
	byte	pad[2];
} cplane_t;
/*
Ghoul2 Insert Start
*/
typedef struct
{
	float		mDistance;
	int			mEntityNum;
	int			mModelIndex;
	int			mPolyIndex;
	int			mSurfaceIndex;
	vec3_t		mCollisionPosition;
	vec3_t		mCollisionNormal;
	int			mFlags;
	int			mMaterial;
	int			mLocation;
	float		mBarycentricI; // two barycentic coodinates for the hit point
	float		mBarycentricJ; // K = 1-I-J
}CollisionRecord_t;

#define MAX_G2_COLLISIONS 16

typedef CollisionRecord_t G2Trace_t[MAX_G2_COLLISIONS];	// map that describes all of the parts of ghoul2 models that got hit

/*
Ghoul2 Insert End
*/
// a trace is returned when a box is swept through the world
typedef struct {
	qboolean	allsolid;	// if true, plane is not valid
	qboolean	startsolid;	// if true, the initial point was in a solid area
	float		fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;		// final position
	cplane_t	plane;		// surface normal at impact, transformed to world space
	int			surfaceFlags;	// surface hit
	int			contents;	// contents on other side of surface hit
	int			entityNum;	// entity the contacted sirface is a part of
} trace_t;

// calling defines for the trace function
enum EG2_Collision
{
	G2_NOCOLLIDE,
	G2_COLLIDE,
	G2_RETURNONHIT
};

typedef enum 
{
	WF_BASEMODEL =					(1<<0),
	WF_CLIENTONLY =					(1<<1),
	WF_SERVERONLY =					(1<<2),
	WF_NPC =						(1<<3),
	WF_DM_SKELETON =				(1<<4),

} EWraithInstFlags;

// trace->entityNum can also be 0 to (MAX_GENTITIES-1)
// or ENTITYNUM_NONE, ENTITYNUM_WORLD


// markfragments are returned by CM_MarkFragments()
typedef struct 
{
	int		firstPoint;
	int		numPoints;

} markFragment_t;


typedef struct 
{
	vec3_t		origin;
	vec3_t		axis[3];

} orientation_t;

typedef struct 
{
	float		yawAngle;
	qboolean	yawing;

	float		pitchAngle;
	qboolean	pitching;

	int			anim;
	int			animTime;

} animInfo_t;

//=====================================================================


// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define KEYCATCH_CONSOLE		0x0001
#define	KEYCATCH_UI				0x0002
#define	KEYCATCH_MESSAGE		0x0004
#define	KEYCATCH_CGAME			0x0008
#define KEYCATCH_NUMBERSONLY	0x0010

// sound channels
// channel 0 never willingly overrides
// other channels will allways override a playing sound on that channel
typedef enum {
	CHAN_AUTO,
	CHAN_LOCAL,		// menu sounds, etc
	CHAN_WEAPON,
	CHAN_VOICE,
	CHAN_ITEM,
	CHAN_BODY,
	CHAN_LOCAL_SOUND,	// chat messages, etc
	CHAN_ANNOUNCER,		// announcer voices, etc
	CHAN_AMBIENT,		//# ingame NPC weapons only, do not use in scripts!
} soundChannel_t;


/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define	SNAPFLAG_RATE_DELAYED	1
#define	SNAPFLAG_NOT_ACTIVE		2	// snapshot used during connection and for zombies
#define SNAPFLAG_SERVERCOUNT	4	// toggled every map_restart so transitions can be detected

//
// per-level limits
//
#define	MAX_CLIENTS			64		// absolute limit
#define MAX_LOCATIONS		64
#define MAX_TERRAINS		32
#define MAX_LADDERS			64

#define MAX_INSTANCE_TYPES		16

#define	GENTITYNUM_BITS			10		// don't need to send any more
#define	MAX_GENTITIES			(1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values thatare going to be communcated over the net need to
// also be in this range
#define	ENTITYNUM_NONE			(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD			(MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)


// these are also in be_aas_def.h - argh (rjr)
#define	MAX_MODELS				256		// these are sent over the net as 8 bits
#define	MAX_SOUNDS				256		// so they cannot be blindly increased
#define MAX_AMBIENT_SOUNDSETS	64
#define MAX_FX					64		// max effects strings, I'm hoping that 64 will be plenty
#define MAX_SUB_BSP				32
#define MAX_ICONS				32
#define	MAX_CHARSKINS			64		// character skins

#define	MAX_CONFIGSTRINGS	1400

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define	CS_SERVERINFO		0		// an info string with all the serverinfo cvars
#define	CS_SYSTEMINFO		1		// an info string for server system to client system configuration (timescale, etc)
#define CS_PLAYERS			2		// info string for player user info
#define CS_CUSTOM			(CS_PLAYERS + MAX_CLIENTS )

#define	RESERVED_CONFIGSTRINGS	2	// game can't modify below this, only the system can

#define	MAX_GAMESTATE_CHARS	16000
typedef struct {
	int			stringOffsets[MAX_CONFIGSTRINGS];
	char		stringData[MAX_GAMESTATE_CHARS];
	int			dataCount;
} gameState_t;

//=========================================================

// bit field limits
#define	MAX_STATS				16
#define	MAX_PERSISTANT			16
#define	MAX_AMMO				16
#define	MAX_WEAPONS				32
#define MAX_GAMETYPE_ITEMS		5

#define	MAX_PS_EVENTS			4

#define PS_PMOVEFRAMECOUNTBITS	6

typedef enum
{
	ATTACK_NORMAL,
	ATTACK_ALTERNATE,
	ATTACK_MAX

} attackType_t;

// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.

typedef struct playerState_s 
{
	int			commandTime;	// cmd->serverTime of last executed command
	int			pm_type;
	int			bobCycle;		// for view bobbing and footstep generation
	int			pm_flags;		// ducked, etc
	int			pm_debounce;	// debounce buttons
	int			pm_time;

	vec3_t		origin;
	vec3_t		velocity;

	int			weaponTime;
	int			weaponFireBurstCount;
	int			weaponAnimId;
	int			weaponAnimIdChoice;
	int			weaponAnimTime;
	int			weaponCallbackTime;
	int			weaponCallbackStep;

	int			gravity;
	int			speed;
	int			delta_angles[3];				// add to command angles to get view direction
												// changed by spawns, rotating objects, and teleporters
	int			groundEntityNum;				// ENTITYNUM_NONE = in air
												
	int			legsAnim;						// mask off ANIM_TOGGLEBIT
												
	int			torsoTimer;						// don't change low priority animations until this runs out
	int			torsoAnim;						// mask off ANIM_TOGGLEBIT
												
	int			movementDir;					// a number 0 to 7 that represents the reletive angle
												// of movement to the view angle (axial and diagonals)
												// when at rest, the value will remain unchanged
												// used to twist the legs during strafing
												
	int			eFlags;							// copied to entityState_t->eFlags
												
	int			eventSequence;					// pmove generated events
	int			events[MAX_PS_EVENTS];			
	int			eventParms[MAX_PS_EVENTS];		
												
	int			externalEvent;					// events set on player from another source
	int			externalEventParm;				
	int			externalEventTime;				
												
	int			clientNum;						// ranges from 0 to MAX_CLIENTS-1
	int			weapon;							// copied to entityState_t->weapon
	int			weaponstate;					
												
	vec3_t		viewangles;						// for fixed views
	int			viewheight;						
												
	// damage feedback							
	int			damageEvent;					// when it changes, latch the other parms
	int			damageYaw;						
	int			damagePitch;					
	int			damageCount;					
												
	int			painTime;						// used for both game and client side to process the pain twitch - NOT sent across the network
	int			painDirection;					// NOT sent across the network
										
	int			stats[MAX_STATS];				
	int			persistant[MAX_PERSISTANT];		// stats that aren't cleared on death
	int			ammo[MAX_AMMO];
	int			clip[ATTACK_MAX][MAX_WEAPONS];
	int			firemode[MAX_WEAPONS];

	int			generic1;
	int			loopSound;

	// Incaccuracy values for firing
	int			inaccuracy;
	int			inaccuracyTime;
	int			kickPitch;

	// not communicated over the net at all
	int			ping;							// server to game info for scoreboard
	int			pmove_framecount;				// FIXME: don't transmit over the network
	int			jumppad_frame;
	int			entityEventSequence;
	vec3_t		pvsOrigin;						// view origin used to calculate PVS (also the lean origin)
												// THIS VARIABLE MUST AT LEAST BE THE PLAYERS ORIGIN ALL OF THE 
												// TIME OR THE PVS CALCULATIONS WILL NOT WORK.

	// Zooming
	int			zoomTime;
	int			zoomFov;

	// LAdders
	int			ladder;
	int			leanTime;

	// Timers 
	int			grenadeTimer;
	int			respawnTimer;
	int			oldTimer;
	int			RealSpawnTimer;

} playerState_t;


typedef enum 
{
	TEAM_FREE,
	TEAM_RED,
	TEAM_BLUE,
	TEAM_SPECTATOR,

	TEAM_NUM_TEAMS

} team_t;

//====================================================================


//
// usercmd_t->button bits, many of which are generated by the client system,
// so they aren't game/cgame only definitions
//
#define	BUTTON_ATTACK		(1<<0)
#define	BUTTON_TALK			(1<<1)			// displays talk balloon and disables actions	
#define BUTTON_GOGGLES		(1<<2)			// turns nv or therm goggles on/off
#define BUTTON_LEAN			(1<<3)			// lean modifier, when held strafe left and right will lean
#define	BUTTON_WALKING		(1<<4)			// walking can't just be infered from MOVE_RUN
											// because a key pressed late in the frame will
											// only generate a small move value for that frame
											// walking will use different animations and
											// won't generate footsteps
#define	BUTTON_USE			(1<<5)			// the ol' use key returns!
#define	BUTTON_RELOAD		(1<<6)			// reloads current weapon
#define BUTTON_ALT_ATTACK	(1<<7)
#define	BUTTON_ANY			(1<<8)			// any key whatsoever
#define BUTTON_ZOOMIN		(1<<9)
#define BUTTON_ZOOMOUT		(1<<10)
#define BUTTON_FIREMODE		(1<<11)

#define BUTTON_LEAN_RIGHT	(1<<12)
#define BUTTON_LEAN_LEFT	(1<<13)


// usercmd_t is sent to the server each client frame
typedef struct usercmd_s 
{
	int				serverTime;
	int				angles[3];
	int 			buttons;
	byte			weapon;
	signed char		forwardmove;
	signed char		rightmove;
	signed char		upmove;

} usercmd_t;

//===================================================================

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define	SOLID_BMODEL	0xffffff

typedef enum {
	TR_STATIONARY,
	TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	TR_SINE,					// value = base + sin( time / duration ) * delta
	TR_GRAVITY,
	TR_HEAVYGRAVITY,
	TR_LIGHTGRAVITY
} trType_t;

typedef struct {
	trType_t	trType;
	int		trTime;
	int		trDuration;			// if non 0, trTime + trDuration = stop time
	vec3_t	trBase;
	vec3_t	trDelta;			// velocity, etc
} trajectory_t;

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

typedef struct entityState_s 
{
	int				number;			// entity index
	int				eType;			// entityType_t
	int				eFlags;

	trajectory_t	pos;			// for calculating position
	trajectory_t	apos;			// for calculating angles

	int				time;
	int				time2;
					
	vec3_t			origin;
	vec3_t			origin2;
					
	vec3_t			angles;
	vec3_t			angles2;
					
	int				otherEntityNum;	// shotgun sources, etc
	int				otherEntityNum2;
					
	int				groundEntityNum;	// -1 = in air
					
	int				loopSound;		// constantly loop this sound
	int				mSoundSet;
										
	int				modelindex;
	int				modelindex2;
	int				clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses
	int				frame;
					
	int				solid;			// for client side prediction, trap_linkentity sets this properly
					
	int				event;			// impulse events -- muzzle flashes, footsteps, etc
	int				eventParm;

	int				generic1;
	// for players
	// these fields are only transmitted for client entities!!!!!
	int				gametypeitems;	// bit flags indicating which items are carried
	int				weapon;			// determines weapon and flash model, etc
	int				legsAnim;		// mask off ANIM_TOGGLEBIT
	int				torsoAnim;		// mask off ANIM_TOGGLEBIT
	int				torsoTimer;		// time the animation will play for
	int				leanOffset;		// Lean direction
} entityState_t;



typedef enum {
	CA_UNINITIALIZED,
	CA_DISCONNECTED, 	// not talking to a server
	CA_AUTHORIZING,		// not used any more, was checking cd key 
	CA_CONNECTING,		// sending request packets to the server
	CA_CHALLENGING,		// sending challenge packets to the server
	CA_CONNECTED,		// netchan_t established, getting gamestate
	CA_LOADING,			// only during cgame initialization, never during main loop
	CA_PRIMED,			// got gamestate, waiting for first frame
	CA_ACTIVE,			// game views should be displayed
	CA_CINEMATIC		// playing a cinematic or a static pic, not connected to a server
} connstate_t;

#define Square(x) ((x)*(x))

// real time
//=============================================


typedef struct qtime_s {
	int tm_sec;     /* seconds after the minute - [0,59] */
	int tm_min;     /* minutes after the hour - [0,59] */
	int tm_hour;    /* hours since midnight - [0,23] */
	int tm_mday;    /* day of the month - [1,31] */
	int tm_mon;     /* months since January - [0,11] */
	int tm_year;    /* years since 1900 */
	int tm_wday;    /* days since Sunday - [0,6] */
	int tm_yday;    /* days since January 1 - [0,365] */
	int tm_isdst;   /* daylight savings time flag */
} qtime_t;


// server browser sources
#define AS_LOCAL			0
#define AS_GLOBAL			1
#define AS_FAVORITES		2


// cinematic states
typedef enum {
	FMV_IDLE,
	FMV_PLAY,		// play
	FMV_EOF,		// all other conditions, i.e. stop/EOF/abort
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
} e_status;


#define	MAX_GLOBAL_SERVERS			2048
#define	MAX_OTHER_SERVERS			128
#define MAX_PINGREQUESTS			32
#define MAX_SERVERSTATUSREQUESTS	16

#define SAY_ALL		0
#define SAY_TEAM	1
#define SAY_TELL	2

// Boe!Man 3/30/10
#define ADM_TALK	3
#define ADM_CHAT	4
#define REF_TALK	5
#define REF_CHAT    6
#define SADM_CHAT	7
#define CLAN_TALK	8
#define CLAN_CHAT   9
#define CADM_CHAT   10

#define CDKEY_LEN 16
#define CDCHKSUM_LEN 2

void Rand_Init(int seed);
float flrand(float min, float max);
int irand(int min, int max);
int Q_irand(int value1, int value2);

#ifdef __cplusplus
	#define RAND_TABLE_SIZE		1024

	extern	int		randIndex;
	extern	float	randTable[RAND_TABLE_SIZE];

	inline float flrand(void) { return(randTable[randIndex++ & (RAND_TABLE_SIZE - 1)]); }
#endif


/*
Ghoul2 Insert Start
*/

typedef struct {
	float		matrix[3][4];
} mdxaBone_t;

// For ghoul2 axis use

typedef enum
{
	ORIGIN = 0, 
	POSITIVE_X,
	POSITIVE_Z,
	POSITIVE_Y,
	NEGATIVE_X,
	NEGATIVE_Z,
	NEGATIVE_Y
} Eorientations;
/*
Ghoul2 Insert End
*/


// define the new memory tags for the zone, used by all modules now
//
#define TAGDEF(blah) TAG_ ## blah
typedef enum {
	#include "../qcommon/tags.h"
} memtag_t;

typedef struct 
{
	int		isValid;	
	void	*ghoul2;
	int		modelNum;
	int		boltNum;
	vec3_t	angles;
	vec3_t	origin;
	vec3_t	scale;
	vec3_t	dir;
	vec3_t	forward;

} CFxBoltInterface;


/*
========================================================================

Version Download

========================================================================
*/

#define VD_IGNORE			-3
#define VD_CANCEL			-2
#define VD_BROWSER_DOWNLOAD	-1


/*
========================================================================

Generic string hash table

========================================================================
*/
#define KEYWORDHASH_SIZE	512

typedef struct keywordHash_s
{
	char					*name;		// Name of keyword
	struct keywordHash_s	*next;		// Used for collisions

} keywordHash_t;

int				KeywordHash_Key		( char *keyword );
void			KeywordHash_Add		( keywordHash_t *table[], keywordHash_t *key );
keywordHash_t*	KeywordHash_Find	( keywordHash_t *table[], char *keyword );

/*
========================================================================

String ID Tables

========================================================================
*/
#define ENUM2STRING(arg)   #arg,arg
typedef struct stringID_table_s
{
	char	*name;
	int		id;
} stringID_table_t;

int GetIDForString ( stringID_table_t *table, const char *string );
const char *GetStringForID( stringID_table_t *table, int id );


// stuff to help out during development process, force reloading/uncacheing of certain filetypes...
//
typedef enum
{
	eForceReload_NOTHING,
//	eForceReload_BSP,	// not used in MP codebase
	eForceReload_MODELS,
	eForceReload_ALL

} ForceReload_e;

#endif	// __Q_SHARED_H
