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
 *   Main routine for dvips.c.  (C) 1987 Radical Eye Software.
 */
#include "structures.h"
/*
 *   First we define some globals.
 */
fontdesctype *TeXfonts[256] ; /* font pointers */
fontdesctype *curfnt ;        /* the currently selected font */
sectiontype *sections ;       /* sections to process document in */
Boolean landscape ;           /* is this document in landscape? */
integer maxpages ;            /* the maximum number of pages */
Boolean notfirst ;            /* true if a first page was specified */
integer firstpage ;           /* the number of the first page if specified */
Boolean orientationset ;      /* after this goes true, too late to landscape */
int numcopies ;          /* number of copies of this document to print */
char *oname ;                 /* output file name */
char *iname ;                 /* dvi file name */
char strings[STRINGSIZE] ;    /* strings for program */
char *nextstring, *maxstring ; /* string pointers */
FILE *dvifile, *bitfile ;     /* dvi and bit files */
FILE *pkfile ;                /* font file */
Boolean prescan ;             /* is this the prescan? */
integer curpos ;              /* dvi file location */
real conv ;                   /* conversion ratio from TeX units to pixels */
integer mag ;                 /* the magnification of this document */
integer fontmem = 800000 ;    /* memory remaining in printer */
integer pagecount ;           /* page counter for the sections */
integer pagenum ;             /* the page number we currently look at */
halfword bytesleft ;          /* number of bytes left in raster */
quarterword *raster ;         /* area for raster manipulations */
shalfword hh, vv ;            /* horizontal and vertical pixel positions */
shalfword fnt ;               /* current font */
char *tfmpath ;               /* pointer to directories for tfm files */
char *pkpath ;                /* pointer to directories for pk files */
integer swmem ;               /* memory in smartwriter */
int quiet ;                   /* should we only print errors to stderr? */
int filter ;                  /* act as filter default output to stdout,
                                               default input to stdin? */
Boolean reverse ;             /* are we going reverse? */
Boolean usesPSfonts ;         /* do we use local PostScript fonts? */
Boolean headers_off ;         /* do we send headers or not? */
#ifdef DEBUG
integer debug_flag = 0;
#endif /* DEBUG */
/*
 *   This routine calls the following externals:
 */
extern void prescanpages() ;
extern void initprinter() ;
extern void cleanprinter() ;
extern void dosection() ;
extern void getdefaults() ;
extern void cmdout() ;
extern void znumout() ;
extern void add_header() ;
extern void send_headers() ;
extern char *strcpy() ;
/*
 *   This error routine prints the error message; if the first
 *   character is !, it exits.
 */
static char *progname ;
void
error(s)
	char *s ;
{
   fprintf(stderr, "%s: %s\n", progname, s) ;
   if (*s=='!') {
      if (bitfile != NULL) {
         cleanprinter() ;
      }
      exit(1) ;
   }
}
/*
 *   Initialize sets up all the globals and data structures.
 */
void
initialize()
{
   int i ;

   for (i=0; i<256; i++)
      TeXfonts[i] = NULL ;
   sections = NULL ;
   landscape = 0 ;
   maxpages = 100000 ;
   firstpage = 0 ;
   notfirst = 0 ;
   orientationset = 0 ;
   numcopies = 1 ;
   nextstring = strings ;
   oname = strings ;
   iname = strings ;
   *nextstring++ = 0 ;
   maxstring = strings + STRINGSIZE - 200 ;
   bitfile = NULL ;
   bytesleft = 0 ;
   pkpath = PKPATH ;
   tfmpath = TFMPATH ;
   swmem = SWMEM ;
   oname = OUTPATH ;
#ifdef DEBUG
   debug_flag = 0;
#endif /* DEBUG */
}
/*
 *   This routine copies a string into the string `pool', safely.
 */
char *
newstring(s)
   char *s ;
{
   int l = strlen(s) ;

   if (nextstring + l >= maxstring)
      error("! out of string space") ;
   (void)strcpy(nextstring, s) ;
   s = nextstring ;
   nextstring += l + 1 ;
   return(s) ;
}
/*
 *   Finally, our main routine.
 */
main(argc, argv)
	int argc ;
	char *argv[] ;
{
   int i, lastext ;

   progname = argv[0] ;
   initialize() ;
   getdefaults() ;
/*
 *   This next whole big section of code is straightforward; we just scan
 *   the options.  Allowable options are:  n (number of pages), c (number
 *   of copies), p (first page), and o (output file name.)  The argument
 *   can either immediately follow the option letter or be separated by
 *   spaces.  Any argument not preceeded by '-' is considered a file name;
 *   the program complains if more than one is given, and uses stdin 
 *   if none is given.
 */
   for (i=1; i<argc; i++) {
      if (*argv[i]=='-') {
         char *p=argv[i]+2 ;
         char c=argv[i][1] ;
         switch (c) {
case 'n' : case 'N' :
            if (*p == 0)
               p = argv[++i] ;
#ifdef SHORTINT
            if (sscanf(p, "%ld", &maxpages)==0)
#else	/* ~SHORTINT */
            if (sscanf(p, "%d", &maxpages)==0)
#endif	/* ~SHORTINT */
               error("! bad number of pages option.") ;
            break ;
case 'p' : case 'P' :
            if (*p == 0)
               p = argv[++i] ;
#ifdef SHORTINT
            if (sscanf(p, "%ld", &firstpage)==0)
#else	/* ~SHORTINT */
            if (sscanf(p, "%d", &firstpage)==0)
#endif	/* ~SHORTINT */
               error("! bad first page option.") ;
            else notfirst = 1 ;
            break ;
case 'o' : case 'O' :
            if (*p == 0)
               p = argv[++i] ;
            oname = p ;
            break ;
case 'h' : case 'H' :
            if (*p == 0)
               p = argv[++i] ;
            if (strcmp(p, "-") == 0)
               headers_off = 1 ;
            else
               add_header(p) ;
            break ;
case 'c' : case 'C' :
            if (*p == 0)
               p = argv[++i] ;
            if (sscanf(p, "%d", &numcopies)==0)
               error("! bad number of copies option.") ;
            break ;
case 'q' : case 'Q' :
            quiet = 1 ;
            break ;
case 'f' : case 'F' :
            filter = 1 ;
            break ;
case 'r' : case 'R' :
            reverse = 1 ;
            break ;
#ifdef DEBUG
case 'd' : case 'D' :
	    if (*p == 0)
	       p = argv[++i];
	    if (sscanf(p, "%d", &debug_flag)==0)
	       error("! bad debug option.");
	    break;
#endif /* DEBUG */
case '?' :
            fprintf(stderr, BANNER) ;
            break ;
default:
#ifdef DEBUG
            error("! bad option, not one of cdfnopq") ;
#else	/* ~DEBUG */
            error("! bad option, not one of cfnopq") ;
#endif	/* ~DEBUG */
         }
      } else {
         if (*iname == 0) {
            register char *p ;

            lastext = 0 ;
            iname = nextstring ;
            p = argv[i] ;
            while (*p) {
               *nextstring = *p++ ;
               if (*nextstring == '.')
                  lastext = nextstring - iname ;
               else if (*nextstring == '/' || *nextstring == ':')
                  lastext = 0 ;
               nextstring++ ;
            }
            if (lastext == 0) {
               lastext = nextstring - iname ;
               *nextstring++ = '.' ;
               *nextstring++ = 'd' ;
               *nextstring++ = 'v' ;
               *nextstring++ = 'i' ;
            }
            *nextstring++ = 0 ;
         } else
            error("! two input file names specified on input") ;
      }
   }
   if (!quiet)
      fprintf(stderr, BANNER) ;
   if (*oname == 0 && ! filter) {
      oname = nextstring ;
      for (i=0; i<=lastext; i++)
         *nextstring++ = iname[i] ;
      *nextstring++ = 'b' ;
      *nextstring++ = 'i' ;
      *nextstring++ = 't' ;
      *nextstring++ = 0 ;
   }
#ifdef DEBUG
   if (dd(D_PATHS)) {
#ifdef SHORTINT
	fprintf(stderr,"input file %s output file %s swmem %ld\n",
#else /* ~SHORTINT */
   	fprintf(stderr,"input file %s output file %s swmem %d\n",
#endif /* ~SHORTINT */
           iname, oname, swmem) ;
   fprintf(stderr,"tfm path %s pk path %s\n", tfmpath, pkpath) ;
   } /* dd(D_PATHS) */
#endif /* DEBUG */
/*
 *   Now we try to open the dvi file.
 */
   add_header(HEADERFILE) ;
   if (*iname != 0)
      dvifile = fopen(iname, "r") ;
   else
      dvifile = stdin ;
   if (dvifile==NULL)
      error("! couldn't open dvi file") ;
   curpos = 0 ;
/*
 *   Now we do our main work.
 */
   prescan = 1 ;
   prescanpages() ;
   initprinter() ;
   if (usesPSfonts)
      add_header(PSFONTHEADER) ;
   if (! headers_off)
      send_headers() ;
   cmdout("TeXDict") ;
   cmdout("begin") ;
   cmdout("@start") ;
   if (numcopies != 1) {
      numout(numcopies) ;
      cmdout("@copies") ;
   }
   prescan = 0 ;
   while (sections != NULL) {
      if (! quiet)
         fprintf(stderr, ". ") ;
      (void)fflush(stderr) ;
      dosection(sections) ;
      sections = sections->next ;
   }
   cleanprinter() ;
   if (! quiet)
      fprintf(stderr, "\n") ;
   exit(0) ;
}
