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
 *   fontdef.c of dvisw software package.  This code is copyright (C) 1986
 *   by Radical Eye Software.
 *
 *  Stores the data from a font definition into the global data structures.
 *  A routine skipnop is also included to skip over nops and font definitions.
 *  It returns the first byte not a nop or font definition.
 */
#include "structures.h"
/*
 *   These are the external routines it calls.
 */
extern shalfword dvibyte() ;
extern integer signedquad() ;
extern void error() ;
extern halfword dpicheck() ;
/*
 *   The external variables it uses:
 */
extern fontdesctype *TeXfonts[256] ;
extern char *nextstring, *maxstring ;
extern Boolean orientationset ;
extern integer mag ;
#ifdef DEBUG
extern integer debug_flag;
#endif	/* DEBUG */
/*
 *   We use malloc here:
 */
char *malloc() ;
/*
 *   fontdef takes a font definition in the dvi file and loads the data
 *   into its data structures.
 */
void
fontdef()
{
   register integer i, j ;
   register shalfword fn ;
   register fontdesctype *fp ;

   orientationset = 1 ;
   fn = dvibyte() ;
   if (TeXfonts[fn] == NULL)
   {
      fp = (fontdesctype *)malloc(sizeof(fontdesctype)) ;
      if (fp==NULL)
         error("! ran out of memory") ;
      TeXfonts[fn] = fp ;
      fp->loaded = 0 ;
      fp->checksum = signedquad() ;
      fp->scaledsize = signedquad() ;
      fp->designsize = signedquad() ;
      fp->thinspace = fp->scaledsize / 6 ;
      fp->name = nextstring ;
      fp->resfont = NULL ;
      fp->dpi = dpicheck(((halfword)((float)mag*(float)fp->scaledsize*DPI/
            ((float)fp->designsize*1000.0)+0.5))) ;
      i = dvibyte() ; j = dvibyte() ;
      if (nextstring + i + j > maxstring)
         error("! out of string space") ;
      for (; i>0; i--)
         *nextstring++ = dvibyte() ;
      *nextstring++ = 0 ;
      for (; j>0; j--)
         *nextstring++ = dvibyte() ;
      *nextstring++ = 0 ;
#ifdef DEBUG
      if (dd(D_FONTS))
	fprintf(stderr,"Defining font %s\n", fp->name+1) ;
#endif /* DEBUG */
   } else {
      for (i = 1; i<=12; i++) (void)dvibyte() ;
      i = dvibyte() + dvibyte() ;
      for (; i>0; i--) (void)dvibyte() ;
   }
}
/*
 *   Skips any nops or font definitions between pages in a dvi file.  Returns
 *   the first command which is not a nop or font definition.
 */
int
skipnop()
{
  register int cmd ;
  while ((cmd=dvibyte())==138||cmd==243)
    if (cmd==243) fontdef() ;
  return(cmd) ;
}
