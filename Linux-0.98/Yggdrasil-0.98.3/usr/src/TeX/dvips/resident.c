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
 *   resident.c of dvisw software package.  (C) 1986 Radical Eye Software.
 *   This code reads in and handles the defaults for the program from the
 *   file config.sw.  This entire file is a bit kludgy, sorry.
 */
#include "structures.h"
/*
 *   This is the structure definition for resident fonts.  We use
 *   a small and simple hash table to handle these.  We don't need
 *   a big hash table.
 */
#define RESHASHPRIME (23)
struct resfont *reshash[RESHASHPRIME] ;
/*
 *   These are the external routines we use.
 */
extern void error() ;
extern integer scalewidth() ;
extern void tfmload() ;
extern FILE *search() ;
extern shalfword pkbyte() ;
extern integer pkquad() ;
extern integer pktrio() ;
extern Boolean pkopen() ;
extern char *strcpy() ;
extern char *newstring() ;
extern void add_header() ;
/*
 *   These are the external variables we use.
 */
#ifdef DEBUG
extern integer debug_flag;
#endif	/* DEBUG */
extern fontdesctype *TeXfonts[] ;
extern integer mag ;
extern halfword bytesleft ;
extern quarterword *raster ;
extern FILE *pkfile ;
extern char *oname ;
extern integer swmem ;
extern char *tfmpath ;
extern char *pkpath ;
extern char *nextstring ;
extern char *maxstring ;
extern Boolean landscape ;
extern int quiet ;
extern int filter ;
extern Boolean reverse ;
extern Boolean usesPSfonts ;
/*
 *   We use malloc here.
 */
char *malloc() ;
/*
 *   Our hash routine.
 */
int
hash(s)
   char *s ;
{
   int h = 12 ;

   while (*s != 0)
      h = (h + h + *s++) % RESHASHPRIME ;
   return(h) ;
}
/*
 *   The routine which looks up a font name (and size.)
 */
struct resfont *
lookup(name, dpi)
   char *name ;
   int dpi ;
{
   struct resfont *p ;

   for (p=reshash[hash(name)]; p!=NULL; p=p->next)
      if (strcmp(p->TeXname, name)==0 && (p->dpi == 0 || p->dpi == dpi))
         return(p) ;
   return(NULL) ;
}
/*
 *   This routine adds an entry.
 */
void
add_entry(TeXname, PSname, dpi)
   char *TeXname, *PSname ;
   int dpi ;
{
   struct resfont *p ;
   int h ;

   p = (struct resfont *)malloc((unsigned int)sizeof(struct resfont)) ;
   if (p==NULL)
      error("! out of memory") ;
   p->TeXname = TeXname ;
   p->PSname = PSname ;
   p->dpi = dpi ;
   h = hash(TeXname) ;
   p->next = reshash[h] ;
   reshash[h] = p ;
}
/*
 *   Now our residentfont routine.
 */
Boolean
residentfont(fn)
	halfword fn ;
{
   register shalfword i ;
   register fontdesctype *curfnt ;
   char *nm ;
   struct resfont *p ;

/*
 *   First we determine if we can find this font in the resident list.
 */
   for (nm=TeXfonts[fn]->name; *nm != 0; nm++) ;
   nm++ ;
   if ((p=lookup(nm, (int)TeXfonts[fn]->dpi))==NULL)
      return 0 ;
/*
 *   We clear out some pointers:
 */
#ifdef DEBUG
   if (dd(D_FONTS))
	fprintf(stderr,"This font is resident.\n") ;
#endif	/* DEBUG */
   curfnt = TeXfonts[fn] ;
   curfnt->resfont = p ;
   for (i=0; i<256; i++) {
      curfnt->chardesc[i].TFMwidth = 0 ;
      curfnt->chardesc[i].packptr = NULL ;
      curfnt->chardesc[i].pixelwidth = 0 ;
      curfnt->chardesc[i].flags = EXISTS ;
   }
   tfmload((halfword)fn) ;
   usesPSfonts = 1 ;
   return(1) ;
}
static char iinline[100] ;
void bad_config() {
   error("Error in config file:") ;
   error(iinline) ;
}
/*
 *   Now we have the getdefaults routine.
 */
void
getdefaults()
{
   FILE *deffile ;
   int i, j ;
   char fontname[50], PSname[100] ;

   if ((deffile=search(CONFIGPATH,CONFIGFILE))==NULL) {
      checkenv() ;
      return ;
   }
   while (fgets(iinline, 100, deffile)!=NULL) {
      switch (iinline[0]) {
case 'm' : case 'M' :
#ifdef SHORTINT
         if (sscanf(iinline+1, "%ld", &swmem) != 1) bad_config() ;
#else	/* ~SHORTINT */
         if (sscanf(iinline+1, "%d", &swmem) != 1) bad_config() ;
#endif	/* ~SHORTINT */
         break ;
case 'o' : case 'O' :
         if (sscanf(iinline+1, "%s", PSname) != 1) bad_config ;
         oname = newstring(PSname) ;
         break ;
case 't' : case 'T' :
         if (sscanf(iinline+1, "%s", PSname) != 1) bad_config() ;
         tfmpath = newstring(PSname) ;
         break ;
case 'p' : case 'P' :
         if (sscanf(iinline+1, "%s", PSname) != 1) bad_config() ;
         pkpath = newstring(PSname) ;
         break ;
case ' ' : case '*' : case '#' : case ';' :
         break ;
case 'r' : case 'R' :
         reverse = 1 ;
         break ;
case 'D' : case 'd' :
         if ((j=sscanf(iinline+1, "%s %s %d", fontname, PSname, &i))>=2) {
            if (j==2)
               i = 0 ;
            add_entry(newstring(fontname), newstring(PSname), i) ;
         }
         break ;
case 'q' : case 'Q' :
         quiet = 1 ;
         break ;
case 'f' : case 'F' :
         filter = 1 ;
         break ;
case 'h' : case 'H' :
         if (sscanf(iinline+1, "%s", PSname) != 1) bad_config() ;
         add_header(PSname) ;
         break ;
default:
         bad_config() ;
      }
   }
   checkenv() ;
}
/*
 *   Get environment variables!
 */
checkenv() {
   char *p, *getenv() ;

   if (p=getenv("TEXFONTS"))
      tfmpath = newstring(p) ;
   if (p=getenv("TEXPKS"))
      pkpath = newstring(p) ;
}
