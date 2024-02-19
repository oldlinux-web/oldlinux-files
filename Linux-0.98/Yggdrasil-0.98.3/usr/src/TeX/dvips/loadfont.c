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
 *   loadfont.c of dvisw software package, (C) 1986 Radical Eye Software.
 *   loads a pk file into memory.
 */
#include "structures.h"
/*
 *   These are the external routines we use.
 */
extern void error() ;
extern integer scalewidth() ;
extern void tfmload() ;
extern FILE *search() ;
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
extern char *pkpath ;
/*
 *   We use malloc here.
 */
char *malloc() ;
/*
 *   Now we have some routines to get stuff from the pk file.  pkbyte returns
 *   the next byte from the pk file.
 */
shalfword
pkbyte()
{
   register shalfword i ;

   if ((i=getc(pkfile))==EOF)
      error("! unexpected eof in pk file") ;
   return(i) ;
}

integer
pkquad()
{
   register integer i ;

   i = pkbyte() ;
   if (i > 127)
      i -= 256 ;
   i = i * 256 + pkbyte() ;
   i = i * 256 + pkbyte() ;
   i = i * 256 + pkbyte() ;
   return(i) ;
}

integer
pktrio()
{
   register integer i ;

   i = pkbyte() ;
   if (i > 127)
      i -= 256 ;
   i = i * 256 + pkbyte() ;
   i = i * 256 + pkbyte() ;
   return(i) ;
}

static char errbuf[80] ;
/*
 *   pkopen opens the pk file.  This is system dependent.
 */
Boolean
pkopen(fn)
	halfword fn ;
{
   register char *d, *n ;
   char name[50] ;

   n = TeXfonts[fn]->name ;
   if (*n==0) {
      n++ ;
      d = pkpath ;
   } else {
      d = n ;
      while (*n) n++ ;
   }
   (void)sprintf(name, "%s.%dpk", n, TeXfonts[fn]->dpi) ;
   if ((pkfile=search(d, name))==NULL) {
      (void)sprintf(errbuf, "Could not open %s, loading tfm", name) ;
      error(errbuf) ;
      return(0) ;
   } else
      return(1) ;
}

/*
 *   Now our loadfont routine.
 */
void
loadfont(fn)
	halfword fn ;
{
   register shalfword i ;
   register shalfword cmd ;
   register fontdesctype *curfnt ;
   register integer k ;
   register shalfword length ;
   register shalfword cc ;
   register integer scaledsize = TeXfonts[fn]->scaledsize ;
   register quarterword *tempr ;
   register chardesctype *cd ;

/*
 *   We clear out some pointers:
 */
   curfnt = TeXfonts[fn] ;
   for (i=0; i<256; i++) {
      curfnt->chardesc[i].TFMwidth = 0 ;
      curfnt->chardesc[i].packptr = NULL ;
      curfnt->chardesc[i].pixelwidth = 0 ;
      curfnt->chardesc[i].flags = 0 ;
   }
   if (!pkopen(fn)) {
      tfmload(fn) ;
      return ;
   }
   if (pkbyte()!=247)
      error("! bad pk file, expected pre") ;
   if (pkbyte()!=89)
      error("! bad version of pk file") ;
   for(i=pkbyte(); i>0; i--)
      (void)pkbyte() ;
   if (pkquad()!=16*TeXfonts[fn]->designsize)
      error("design size mismatch") ;
/*
 *   For now, we turn off the checksum test.  We must turn it back on later,
 *   though, and don't forget to remove the signed quad after this.
   if (pkquad()!=TeXfonts[fn]->checksum)
      error("checksum error") ;
 */
   k = pkquad() ;
   k = pkquad() ;
   k = pkquad() ;
/*
 *   Now we get down to the serious business of reading character definitions.
 */
   while ((cmd=pkbyte())!=245) {
      if (cmd < 240) {
         switch (cmd & 7) {
case 0: case 1: case 2: case 3:
            length = (cmd & 7) * 256 + pkbyte() - 4 ;
            cc = pkbyte() ;
            cd = curfnt->chardesc+cc ;
            cd->TFMwidth = scalewidth(pktrio(), scaledsize) ;
            cd->pixelwidth = pkbyte() ;
            break ;
case 4:
            length = pkbyte() * 256 ;
            length = length + pkbyte() - 5 ;
            cc = pkbyte() ;
            cd = curfnt->chardesc+cc ;
            cd->TFMwidth = scalewidth(pktrio(), scaledsize) ;
            i = pkbyte() ;
            cd->pixelwidth = i * 256 + pkbyte() ;
            break ;
case 5: case 6:
            error("! lost sync in pk file (character too big)") ;
case 7:
            length = pkquad() - 12 ;
            cc = pkquad() ;
            cd = curfnt->chardesc+cc ;
            cd->TFMwidth = scalewidth(pkquad(), scaledsize) ;
            cd->pixelwidth = pkquad() ;
            k = pkquad() ;
            for (; length>0; length--)
               i = pkbyte() ;
         }
         if (bytesleft < length + 1) {
#ifdef DEBUG
             if (dd(D_FONTS))
		fprintf(stderr,"Allocating new raster memory\n") ;
#endif /* DEBUG */
             if (length + 1 > MINCHUNK) {
                tempr = (quarterword *)malloc((unsigned int)length + 1) ;
             } else {
                raster = (quarterword *)malloc(RASTERCHUNK) ;
                tempr = raster ;
                bytesleft = RASTERCHUNK - length - 1 ;
                raster += length + 1 ;
            }
            if (tempr == NULL)
               error("! out of memory while allocating raster") ;
         } else {
            tempr = raster ;
            bytesleft -= length + 1 ;
            raster += length + 1 ;
         }
         cd->packptr = tempr ;
         *tempr++ = cmd ;
         for (; length>0; length--)
            *tempr++ = pkbyte() ;
      } else {
         k = 0 ;
         switch (cmd) {
case 243:
            k = pkbyte() ;
            if (k > 127)
               k -= 256 ;
case 242:
            k = k * 256 + pkbyte() ;
case 241:
            k = k * 256 + pkbyte() ;
case 240:
            k = k * 256 + pkbyte() ;
            while (k-- > 0)
               i = pkbyte() ;
            break ;
case 244:
            k = pkquad() ;
            break ;
case 246:
            break ;
default:
            error("! lost sync in pk file") ;
         }
      }
   }
   (void)fclose(pkfile) ;
   curfnt->loaded = 1 ;
}
