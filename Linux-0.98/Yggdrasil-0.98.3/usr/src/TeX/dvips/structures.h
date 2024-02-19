/*
 *   This is dvips, a freely redistributable PostScript driver
 *   for dvi files.  It is (C) Copyright 1987 by Tomas Rokicki.
 *   You may modify and use this program to your heart's content,
 *   so long as you send modifications to Tomas Rokicki.  It can
 *   be included in any distribution, commercial or otherwise, so
 *   long as the banner string defined in structures.h is not
 *   modified (except for the version number) and this banner is
 *   printed on program invocation, or can be printed on program
 *   invocation with the -? option.
 */
/*
 *   dvips structures file.  This software (C) 1987 Radical Eye Software.
 */
#define BANNER "This is dvips, version 0.92 (C) 1987 Radical Eye Software\n" 
#include <stdio.h>
#include "paths.h"
#include "debug.h"

/*
 *   Constants, used to expand or decrease the capacity of this program.
 */
#define STRINGSIZE (3141)   /* maximum number of strings in program */

#if	defined(ScriptPrinter) || defined(UltraScript)
#define SWMEM (409776)	    /* font memory in ScriptPrinter */
#else
#define SWMEM (100000)      /* font memory in smartwriter */
#endif

#define RASTERCHUNK (8192)  /* size of chunk of raster */
#define MINCHUNK (256)      /* minimum size char to get own raster */
#define MAXDRIFT (3)        /* maximum drift in pixels */
#define STACKSIZE (100)     /* maximum stack size for dvi files */
/*
 *   Other constants, which define printer dependent stuff.
 */
#define DPI (300)           /* dots per inch */
#define FONTCOST (2372)     /* overhead cost of each sw font */
#define PSFONTCOST (3372)   /* overhead cost for PostScript fonts */
#define CHARCOST (70)       /* overhead cost for each character */
#define STRINGCOST (10)     /* the cost of a string */
/*
 *   Type declarations.  integer must be a 32-bit signed; shalfword must
 *   be a sixteen-bit signed; halfword must be a sixteen-bit unsigned;
 *   quarterword must be an eight-bit unsigned.
 */
typedef int integer ;
typedef short shalfword ;
typedef unsigned short halfword ;
typedef unsigned char quarterword ;
typedef float real ;
typedef short Boolean ;
/*
 *   If the machine has a default integer size of 16 bits, and 32-bit
 *   integers must be manipulated with %ld, set the macro SHORTINT.
 */
/*
 *   This is the structure definition for resident fonts.  We use
 *   a small and simple hash table to handle these.  We don't need
 *   a big hash table.
 */
#define RESHASHPRIME (23)
struct resfont {
   char *TeXname ;
   char *PSname ;
   halfword dpi ;
   struct resfont *next ;
} ;
/*
 *   A chardesc describes an individual character.  Before the fonts are
 *   downloaded, swfont indicates that the character has already been used
 *   with the following meanings:
 */
typedef struct {
   integer TFMwidth ;
   quarterword *packptr ;
   halfword pixelwidth ;
   halfword flags ;
} chardesctype ;
#define EXISTS (1)
#define PREVPAGE (2)
#define THISPAGE (4)
#define TOOBIG (8)
/*
 *   A fontdesc describes a font.  The name is located in the string pool.
 */
typedef struct {
   integer checksum, scaledsize, designsize, thinspace ;
   halfword dpi ;
   halfword psname ;
   char loaded, defined ;
   chardesctype chardesc[256] ;
   char *name ;
   struct resfont *resfont ;
} fontdesctype ;
/*
 *   This type holds the font usage information for a section.  A section is
 *   a largest portion of the document whose font usage does not overflow
 *   the capacity of the printer.  If a single page does overflow the
 *   capacity all by itself, it is made into its own section and a warning
 *   message is printed; the page is still printed.
 *
 *   The sections are in a linked list, built during the prescan phase and
 *   processed in LIFO order (so pages stack correctly on output) during
 *   the second phase.
 */
typedef struct {
   halfword fontnum, bitmap[16] ;
} charusetype ;
/*
 *   This type describes a section.  The actual section layout is as follows:
 *      sectiontype sect ;
 *      charusetype charuse[numfonts] ;
 *      halfword guard = 256 ;
 *   but since we can't declare this or take a sizeof, we build it and
 *   manipulate it ourselves.  numfonts is the number of fonts currently
 *   defined.
 */
typedef struct t {
   integer bos ;
   struct t *next ;
   halfword numpages ;
} sectiontype ;
/*
 *   This next section is somewhat of a kludge, but we need to be able
 *   to handle the 16-bit compilers somehow.
 */
#define numout(a) znumout((integer)(a))
