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
 *   dosection.c of dvisw software package.  This code is (C) 1986
 *   Radical Eye Software.
 */
#include "structures.h"
/*
 *   These are the external routines we call.
 */
extern void dopage() ;
extern void download() ;
extern integer signedquad() ;
extern void skipover() ;
extern void cmdout() ;
/*
 *   These are the external variables we access.
 */
extern FILE *dvifile ;
extern FILE *bitfile ;
extern integer curloc ;
extern integer pagenum ;
extern integer curpos ;
extern Boolean landscape ;
extern halfword bytesleft ;
extern quarterword *raster ;
extern int quiet ;
extern Boolean reverse ;
static int psfont ;
/*
 *   Now we have the main procedure.
 */
void
dosection(s)
	sectiontype *s ;
{
   charusetype *cu ;
   integer prevptr ;

   cmdout("bos") ;
/*
 *   We insure raster is even-word aligned.
 */
   if (bytesleft & 1) {
      bytesleft-- ;
      raster++ ;
   }
   cu = (charusetype *) (s + 1) ;
   psfont = 1 ;
   while (cu->fontnum != 256)
      download(cu++, psfont++) ;
   prevptr = s->bos ;
   if (! reverse) {
      (void)fseek(dvifile, (long)prevptr, 0) ;
      curpos = prevptr ;
   }
   while (s->numpages-- != 0) {
      if (reverse) {
         (void)fseek(dvifile, (long)prevptr, 0) ;
         curpos = prevptr ;
      }
      pagenum = signedquad() ;
      if (! quiet) {
#ifdef SHORTINT
         fprintf(stderr, "[%ld", pagenum) ;
#else  /* ~SHORTINT */
         fprintf(stderr, "[%d", pagenum) ;
#endif /* ~SHORTINT */
         (void)fflush(stderr) ;
      }
      skipover(36) ;
      prevptr = signedquad()+1 ;
      dopage() ;
      if (! quiet) {
         fprintf(stderr, "] ") ;
         (void)fflush(stderr) ;
      }
      if (! reverse)
         (void)skipnop() ;
   }
   cmdout("eos") ;
}
