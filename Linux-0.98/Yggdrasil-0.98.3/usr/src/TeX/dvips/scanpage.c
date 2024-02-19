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
 *   scanpage.c of dvisw software package.  This code is copyright (C) 1986
 *   by Radical Eye Software.
 *
 *   This routine scans a page for font definitions.  It keeps track of the
 *   memory remaining in the printer, and when that memory is exhausted,
 *   returns with false, unless it is the first page in a section.  If it
 *   is the first page and it still requires too many fonts, it returns 2;
 *   otherwise, it returns 1.
 */
#include "structures.h"
/*
 *   These are the external routines we need.
 */
extern shalfword dvibyte() ;
extern halfword twobytes() ;
extern integer threebytes() ;
extern integer signedquad() ;
extern shalfword signedbyte() ;
extern shalfword signedpair() ;
extern integer signedtrio() ;
extern void skipover() ;
extern void fontdef() ;
extern void dospecial() ;
extern Boolean residentfont() ;
extern void loadfont() ;
extern void error() ;
/*
 *   And, of course, the globals it uses.
 */
#ifdef DEBUG
extern integer debug_flag;
#endif	/* DEBUG */
extern fontdesctype *TeXfonts[] ;
extern fontdesctype *curfnt ;
extern integer fontmem ;
extern integer pagecount ;
extern integer pagenum ;
extern Boolean landscape ;

/*
 *   Now our scanpage routine.
 */
short
scanpage()
{
   register shalfword cmd ;
   register integer i ;
   register quarterword *cras ;
   register halfword thischar ;
   register halfword delchar=0 ;
   register halfword fnt ;
   register long pagecost ;
   integer instring ;

   instring = 0 ;
   pagecost = 0 ;
#ifdef DEBUG
   if (dd(D_PAGE))
#ifdef SHORTINT
   fprintf(stderr,"Scanning page %ld\n", pagenum) ;
#else	/* ~SHORTINT */
   fprintf(stderr,"Scanning page %d\n", pagenum) ;
#endif	/* ~SHORTINT */
#endif	/* DEBUG */
   curfnt = NULL ;
   while ((cmd=dvibyte())!=140) {
/*
 *   We are going to approximate that each character requires
 *   1 byte in PostScript, and each string STRINGCOST bytes.
 */
      if (cmd < 129 || cmd==133) {
         pagecost++ ;
         if (! instring) {
            pagecost += STRINGCOST ;
            instring = 1 ;
         }
/* character set command */
         if (cmd == 133 || cmd == 128)
            cmd = dvibyte() ;
         if (curfnt==NULL)
            error("! no font selected") ;
         if ((curfnt->chardesc[cmd].flags & (PREVPAGE | THISPAGE)) == 0) {
            cras = curfnt->chardesc[cmd].packptr ;
            if (cras != NULL && ((*cras)&7)==7)
               error("illegal character") ;
            else if (cras != NULL) {
               thischar = CHARCOST ;
               if ((*cras & 4) == 0)
                  thischar += 2 * cras[2] * ((cras[1] + 15) / 16) ;
               else
                  thischar += 2 * (cras[3] * 256 + cras[4]) *
                     ((cras[1] * 256 + cras[2] + 15) / 16) ;
               if (fontmem > thischar + pagecost) {
                  fontmem -= thischar ;
                  curfnt->chardesc[cmd].flags |= THISPAGE ;
               } else {
                  if (pagecount > 0)
                     return(0) ;
                  delchar++ ;
                  curfnt->chardesc[cmd].flags |= TOOBIG ;
               }
            }
         }
      } else {
         instring = 0 ;
         if (cmd < 236 && cmd >= 171) {
/* font definition command */
            if (cmd == 235)
               fnt = dvibyte() ;
            else
               fnt = cmd - 171 ;
            curfnt = TeXfonts[fnt] ;
            if (curfnt==NULL)
               error("! undefined font selected") ;
            if (curfnt->loaded==0) {
               if (!residentfont(fnt))
                  loadfont(fnt) ;
            }
            if (curfnt->defined==0) {
               curfnt->defined = 1 ;
               if (curfnt->resfont == NULL)
                  thischar = FONTCOST ;
               else
                  thischar = PSFONTCOST ;
               fontmem -= thischar ;
               if (fontmem <= 0)
                  return(0) ;
            }
         } else {
            switch (cmd) {
/* illegal options */
case 129: case 130: case 131: case 134: case 135: case 136: case 139: 
case 236: case 237: case 238: case 244: case 245: case 246: case 247: 
case 248: case 249: case 250: case 251: case 252: case 253: case 254: 
case 255:
      error("! unexpected command in dvi file") ;
/* eight byte commands */
case 132: case 137:
      cmd = dvibyte() ;
      cmd = dvibyte() ;
      cmd = dvibyte() ;
      cmd = dvibyte() ;
/* four byte commands */
case 146: case 151: case 156: case 160: case 165: case 170:
      cmd = dvibyte() ;
/* three byte commands */
case 145: case 150: case 155: case 159: case 164: case 169:
      cmd = dvibyte() ;
/* two byte commands */
case 144: case 149: case 154: case 158: case 163: case 168:
      cmd = dvibyte() ;
/* one byte commands */
case 128: case 143: case 148: case 153: case 157: case 162: case 167: 
      cmd = dvibyte() ;
      break ;
/* specials */
case 239: i = dvibyte() ; dospecial(i) ; break ;
case 240: i = twobytes() ; dospecial(i) ; break ;
case 241: i = threebytes() ; dospecial(i) ; break ;
case 242: i = signedquad() ; dospecial(i) ; break ;
/* font definition */
case 243:
      fontdef() ;
      break ;
default: ;
            }
         }
      }
   }
/*
 *   On a postscript printer, this should never happen.  Even with only
 *   200K VM, at 11 bytes per char, you can have 18K characters on a
 *   page.
 */
   if (delchar != 0) {
#ifdef SHORTINT
      fprintf(stderr,
	      "%d characters deleted on page %ld\n", delchar, pagenum) ;
#else	/* ~SHORTINT */
      fprintf(stderr,
	     "%d characters deleted on page %d\n", delchar, pagenum) ;
#endif	/* ~SHORTINT */
      return(2) ;
   }
   if (fontmem > pagecost)
      return(1) ;
   else
      return(0) ;
}
