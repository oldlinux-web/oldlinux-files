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
 *   download.c of dvisw software package.  This code (C) 1986
 *   Radical Eye Software.  Downloads a font to the
 *   smartwriter.  If passed a NULL pointer, it finishes up the last
 *   definition.
 */
#include "structures.h"
/*
 *   These are the external routines we call.
 */
extern void unpack() ;
extern void flip() ;
extern void error() ;
extern void znumout() ;
extern void hexout() ;
extern void specialout() ;
extern void cmdout() ;
/*
 *   These are the external variables we access.
 */
extern fontdesctype *TeXfonts[] ;
extern FILE *bitfile ;
extern fontdesctype *curfnt ;
extern halfword bytesleft ;
extern quarterword *raster ;
extern Boolean landscape ;
extern char *nextstring ;
/*
 *   We might use malloc here.
 */
char *malloc() ;
/*
 *   We have a routine which downloads an individual character.
 */
static halfword bftest = 1 ;
void
downchar(c, cc)
	chardesctype *c ;
	shalfword cc ;
{
   register halfword cheight, cwidth ;
   register shalfword i, j ;
   register quarterword *p ;
   register halfword cmd ;
   register shalfword xoff, yoff ;
   register halfword numchars ;
   halfword wwidth ;

   p = c->packptr ;
   cmd = *p++ ;
   if (cmd & 4) {
      cwidth = p[0] * 256 + p[1] ;
      cheight = p[2] * 256 + p[3] ;
      xoff = p[4] * 256 + p[5] ;
      yoff = p[6] * 256 + p[7] ;
      p += 8 ;
   } else {
      cwidth = *p++ ;
      cheight = *p++ ;
      xoff = *p++ ;
      yoff = *p++ ;
      if (xoff > 127)
         xoff -= 256 ;
      if (yoff > 127)
         yoff -= 256 ;
   }
/*
 *   Now we insure we have enough memory for this character before
 *   exiting.
 */
   wwidth = (cwidth + 15) / 16 ;
   i = 2 * cheight * wwidth ;
   numchars = i ;
   if (bytesleft < i) {
      if (bytesleft >= RASTERCHUNK)
         free((char *)raster) ;
      if (RASTERCHUNK > i) {
         raster = (quarterword *)malloc(RASTERCHUNK) ;
         bytesleft = RASTERCHUNK ;
      } else {
         raster = (quarterword *)malloc((unsigned int)i) ;
         bytesleft = i ;
      }
      if (raster == NULL) {
         error("! out of memory during allocation") ;
      }
   }
   for (; i>=0; i--) 
      raster[i]=0 ;
   unpack(p, (halfword *)raster, cwidth, cheight, cmd) ;
/*
 *   Now we actually send out the data.
 */
   specialout('[') ;
   specialout('<') ;
/*
 *   This is somewhat of a kludge.  If the first byte of bftest has
 *   a 1 in it, then this is a byte-flipped machine.  So we flip
 *   the bytes around.
 */
   if (*(char *)&bftest) {
      for (i=0; i<numchars; i+=2) {
         j = raster[i] ;
         raster[i] = raster[i+1] ;
         raster[i+1] = j ;
      }
   }
/*
 *   The funny thing is, we need to output the characters from the
 *   bottom up!  Ha ha ha he he ho ho ho ha ha . . . uh . . .
 */
   p = raster + (cheight - 1) * 2 * wwidth ;
   for (j=0; j<cheight; j++) {
      for (i=(cwidth + 7)/ 8; i>0; i--)
         hexout((int)*p++) ;
      if (((cwidth - 1) & 8) == 0)
         p++ ;
      p -= 4 * wwidth ;
   }
   specialout('>') ;
   numout(cwidth) ;
   numout(cheight) ;
   numout(xoff) ;
   numout((cheight - yoff - 1)) ;
   numout(c->pixelwidth) ;
   specialout(']') ;
   numout(cc) ;
   cmdout("dc") ;
}
/*
 *   And the download procedure.
 */
void
download(p, psfont)
	charusetype *p ;
	int psfont ;
{
   register halfword b, bit ;
   register chardesctype *c ;
   int cc ;

   curfnt = TeXfonts[p->fontnum] ;
   curfnt->psname = psfont ;
   if (psfont < 27)
      (void)sprintf(nextstring, "/f%c", 'a'+psfont-1) ;
   else
      (void)sprintf(nextstring, "/f%d", psfont-27) ;
   cmdout(nextstring) ;
   if (curfnt->resfont != NULL) {
      (void)sprintf(nextstring, "/%s", curfnt->resfont->PSname) ;
      cmdout(nextstring) ;
      numout(curfnt->scaledsize) ;
      cmdout("pf") ;
   } else {
      cmdout("df") ;
      c = curfnt->chardesc ;
      cc = 0 ;
      for (b=0; b<16; b++) {
         for (bit=32768; bit!=0; bit>>=1) {
            if (p->bitmap[b] & bit) {
               downchar(c, cc) ;
               c->flags |= EXISTS ;
            } else
               c->flags &= ~EXISTS ;
            c++ ;
            cc++ ;
         }
      }
      cmdout("dfend");
   }
}
