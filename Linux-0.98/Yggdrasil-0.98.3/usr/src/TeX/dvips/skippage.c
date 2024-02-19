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
 *   skippage.c of dvisw software package.  This code is copyright (C) 1986
 *   by Radical Eye Software.
 *
 *   Skips over a page, collecting possible font definitions.  A very simple
 *   case statement insures we maintain sync with the dvi file by collecting
 *   the necessary parameters; font definitions must be processed normally,
 *   however.
 */
#include "structures.h"
/*
 *   These are the external routines called.
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
extern void error() ;
/*
 *   These are the external variables accessed.
 */
#ifdef DEBUG
extern integer debug_flag;
#endif	/* DEBUG */
extern integer pagenum ;

/*
 *   And now the big routine.
 */
void
skippage()
{
   register shalfword cmd ;
   register integer i ;

#ifdef DEBUG
   if (dd(D_PAGE))
#ifdef SHORTINT
   fprintf(stderr,"Skipping page %ld\n", pagenum) ;
#else	/* ~SHORTINT */
   fprintf(stderr,"Skipping page %d\n", pagenum) ;
#endif	/* ~SHORTINT */
#endif	/* DEBUG */
   while ((cmd=dvibyte())!=140) {
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
case 128: case 133: case 143: case 148: case 153: case 157: case 162: 
case 167: case 235:
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
