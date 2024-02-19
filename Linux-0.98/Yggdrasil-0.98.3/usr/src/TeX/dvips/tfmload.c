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
 *   tfmload.c of dvisw software package.  This code is copyright (C) 1986
 *   by Radical Eye Software.
 *
 *   Loads a tfm file.  It marks the characters as undefined.
 */
#include "structures.h"
/*
 *   These are the external routines it calls:
 */
extern void error() ;
extern integer scalewidths() ;
extern FILE *search() ;
/*
 *   Here are the external variables we use:
 */
extern fontdesctype *TeXfonts[] ;
extern real conv ;
extern char *tfmpath ;
/*
 *   Our static variables:
 */
static FILE *tfmfile ; 

/*
 *   Tries to open a tfm file.  Returns true if found.
 */
Boolean
tfmopen(fn)
	halfword fn ;
{
   register char *d, *n ;
   char name[50] ;

   n = TeXfonts[fn]->name ;
   if (*n==0) {
      n++ ;
      d = tfmpath ;
   } else {
      printf ("AJR null non-leading pathname component.\n");
      d = n ;
      while (*n++) ;
   }
   (void)sprintf(name, "%s.tfm", n) ;
   if ((tfmfile=search(d, name))!=NULL)
      return(1) ;
   fprintf(stderr, "(%s) %s\n", d, name) ;
   return(0) ;
}

shalfword
tfmbyte ()
{
  return(getc(tfmfile)) ;
}

halfword
tfm16 ()
{
  register halfword a ; 
  a = tfmbyte () ; 
  return ( a * 256 + tfmbyte () ) ; 
} 

integer
tfm32 ()
{
  register integer a ; 
  a = tfm16 () ; 
  if (a > 32767) a -= 65536 ;
  return ( a * 65536 + tfm16 () ) ; 
} 

void
tfmload(font)
	halfword font ;
{
   register shalfword i ;
   register integer li ;
   register fontdesctype *curfnt ;
/*   float ratio ; unused */
   integer scaledsize ;
   shalfword nw, hd ;
   shalfword bc, ec ;
   integer scaled[128] ;
   halfword chardat[128] ;

   curfnt = TeXfonts[font] ;
   if (!tfmopen(font))
      error("! could not open tfm file") ;
/*
 *   Next, we read the font data from the tfm file, and store it in
 *   our own arrays.
 */
   li = tfm16() ; hd = tfm16() ;
   bc = tfm16() ; ec = tfm16() ;
   nw = tfm16() ;
   li = tfm32() ; li = tfm32() ; li = tfm32() ; li = tfm16() ;
   if (tfm32()!=curfnt->checksum)
       error("bad checksum in tfm file") ;
   if ((li=tfm32())!=16*curfnt->designsize)
      error("bad design size in tfm file") ;
   for (i=2; i<hd; i++)
      li = tfm32() ;
   for (i=0; i<128; i++)
      chardat[i] = 256 ;
   for (i=bc; i<=ec; i++) {
      chardat[i] = tfmbyte() ;
      li = tfm16() ;
      li = tfmbyte() ;
   }
   scaledsize = curfnt->scaledsize ;
   for (i=0; i<nw; i++)
      scaled[i] = scalewidth(tfm32(), scaledsize) ;
   (void)fclose(tfmfile) ;
   for (i=0; i<128; i++)
      if (chardat[i]!= 256) {
         li = scaled[chardat[i]] ;
         curfnt->chardesc[i].TFMwidth = li ;
         curfnt->chardesc[i].pixelwidth = ((integer)(conv*li+0.5)) ;
         curfnt->chardesc[i].flags = EXISTS ;
      } else
         curfnt->chardesc[i].flags = 0 ;
   curfnt->loaded = 1 ;
}
