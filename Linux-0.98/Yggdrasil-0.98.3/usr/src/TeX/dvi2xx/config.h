#define USEPXL

/*
 *  default font path
 *  can be overridden by environment variable TEXPXL
 *  or -a option
 *  popular choice:
 * #define  FONTAREA       "/usr/TeX/lib/tex/fonts" */
#ifdef vms
#define  FONTAREA	"tex$pkdir:"
#else
#define  FONTAREA       "/usr/TeX/lib/tex/fonts"
#endif

/* 
 * if your LaserJet II P or LaserJet III or LaserJet 2000
 * complains about not enough memory, you might try to reduce 
 * the value below or uncomment to use the default settings
 */  
#ifdef LJ2P
#define  MAX_FONTS_PER_PAGE 255         /* maximum number of fonts per page */
#endif

/* Unix user: remove the following definition if you cannot access the
 * appropriate C library functions
 */
#ifndef u3b2
#define TIMING
#endif

#ifdef unix
/* name of the program which is called to generate pk files which are missing
 */
#define MAKETEXPK "MakeTeXPK"
#endif

/*
 * assure that LJ2 is defined when LJ2P is defined
 */
#ifdef LJ2P
#ifndef LJ2
#define LJ2
#endif
#endif

/*
 * assure that LJ is defined when LJ2 is defined
 */
#ifdef LJ2
#ifndef LJ
#define LJ
#endif
#endif

/*
 * assure that IBM3812 is not defined when LJ is defined
 */ 
#ifdef LJ
#ifdef IBM3812
#undef IBM3812
#endif
#endif


#define  TRUE      (bool) 1
#define  FALSE     (bool) 0
#define  UNKNOWN     -1

#define  STRSIZE         255     /* stringsize for file specifications  */

typedef  char    bool;

#ifdef hpux
typedef  char    signed_char;
#else
#ifdef vms
typedef  char    signed_char;
#else
#ifdef u3b2
typedef  short signed_char;
#else
/* this is the default !! */
typedef  signed char    signed_char; 
#endif 
#endif
#endif


#ifndef u3b2
#define  ARITHMETIC_RIGHT_SHIFT
#endif


bool findfile();


/* 
 * maximal number of characters in font file
 * #define  LASTFNTCHAR  127        7-bit classic version
 * #define  LASTFNTCHAR  255        8-bit fonts
 */

#ifdef SEVENBIT 
#define LASTFNTCHAR 127
#else
#define  LASTFNTCHAR  255
#endif



/* this information is needed in findfile.c and dvi2xx.c, NO CUSTOMIZATION */
#ifdef LJ
#define  RESOLUTION    300
#define MODE "localfont"    /* mode definition for metafont */
#endif

#ifdef IBM3812
#define  RESOLUTION    240
#define MODE "IBMThreeEightOneTwo"    /* mode definition for metafont */
#endif






