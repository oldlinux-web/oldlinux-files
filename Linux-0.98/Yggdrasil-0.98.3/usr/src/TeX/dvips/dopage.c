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
 *   dopage.c of dvisw software package.  This code is copyright (C) 1986
 *   by Radical Eye Software.
 *
 *   Main page drawing procedure.  Interprets the page commands.  A simple
 *   (if lengthy) case statement interpreter.  
 */
#include "structures.h"
/*
 *   The external routines we use:
 */
extern void error() ;
extern void pageinit() ;
extern void abortpage() ;
extern void drawrule() ;
extern shalfword dvibyte() ;
extern halfword twobytes() ;
extern integer threebytes() ;
extern integer signedquad() ;
extern shalfword signedbyte() ;
extern shalfword signedpair() ;
extern integer signedtrio() ;
extern void dospecial() ;
extern void drawchar() ;
extern double floor() ;
extern void fontdef() ;
extern void pageend() ;
/*
 *   Now the external variables.
 */
extern fontdesctype *curfnt ;
extern shalfword fnt ;
extern shalfword hh, vv ;
extern fontdesctype *TeXfonts[] ;
extern real conv ;
extern FILE *bitfile ;
/*
 *   Now we have the dopage procedure.
 */
void
dopage()
{
   register shalfword cmd ;
   register integer p ;
   shalfword sp=0 ;
   integer h, v, w, x, y, z ;
   shalfword hhstack[STACKSIZE], vvstack[STACKSIZE] ;
   integer hstack[STACKSIZE], vstack[STACKSIZE], wstack[STACKSIZE] ;
   integer xstack[STACKSIZE], ystack[STACKSIZE], zstack[STACKSIZE] ;
   shalfword hhh ;
   integer thinspace=0 ;

   pageinit() ;
   hh = vv = h = v = w = x = y = z = fnt = 0 ;
   curfnt = NULL ;
   while ((cmd=dvibyte())!=140) {   /* 140 is eop command */
      if (cmd < 129) {              /* < 129 is character command */
        if (cmd == 128) cmd = dvibyte() ;
        if (curfnt->chardesc[cmd].flags & EXISTS) {
           drawchar(curfnt->chardesc+cmd, cmd) ;
        }
        h += curfnt->chardesc[cmd].TFMwidth ;
        hh += curfnt->chardesc[cmd].pixelwidth ;
        goto setmotion ;
      } else if (cmd < 171) {
        switch (cmd) {
/* illegal options */
case 129: case 130: case 131: case 134: case 135: case 136: case 139:
case 140:
   error("! bad command in dvi file") ;
/* rules */
case 132: case 137:
 { integer ry, rx ;
   shalfword rxx, ryy ;
   ry = signedquad() ; rx = signedquad() ;
   rxx = (conv * rx + 0.9999999) ;
   ryy = (conv * ry + 0.9999999) ;
   drawrule(rxx, ryy) ;
   if (cmd == 132) {
      h += rx ; hh += rxx ;
      goto setmotion ;
   }
 }
   break ;
/* put char */
case 133:
   cmd = dvibyte() ;
   if (curfnt->chardesc[cmd].flags & EXISTS) {
      drawchar(curfnt->chardesc+cmd, cmd) ;
   }
   break ;
/* nop, eop */
case 138:
   break ;
/* push */
case 141:
   if (sp+1 >= STACKSIZE) error("! Out of stack space") ;
   hhstack[sp] = hh ; vvstack[sp] = vv ;
   hstack[sp] = h ; vstack[sp] = v ;
   wstack[sp] = w ; xstack[sp] = x ;
   ystack[sp] = y ; zstack[sp] = z ;
   sp++ ; break ;
/* pop */
case 142:
   sp-- ;
   if (sp < 0) abortpage() ;
   hh = hhstack[sp] ; vv = vvstack[sp] ;
   h = hstack[sp] ; v = vstack[sp] ;
   w = wstack[sp] ; x = xstack[sp] ;
   y = ystack[sp] ; z = zstack[sp] ;
   break ;
/* right */
case 143:
   p = signedbyte() ; goto horizontalmotion ;
case 144:
   p = signedpair() ; goto horizontalmotion ;
case 145:
   p = signedtrio() ; goto horizontalmotion ;
case 146:
   p = signedquad() ; goto horizontalmotion ;
/* w moves */
case 147:
   p = w ; goto horizontalmotion ;
case 148:
   p = w = signedbyte() ; goto horizontalmotion ;
case 149:
   p = w = signedpair() ; goto horizontalmotion ;
case 150:
   p = w = signedtrio() ; goto horizontalmotion ;
case 151:
   p = w = signedquad() ; goto horizontalmotion ;
/* x moves */
case 152:
   p = x ; goto horizontalmotion ;
case 153:
   p = x = signedbyte() ; goto horizontalmotion ;
case 154:
   p = x = signedpair() ; goto horizontalmotion ;
case 155:
   p = x = signedtrio() ; goto horizontalmotion ;
case 156:
   p = x = signedquad() ; goto horizontalmotion ;
/* down moves */
case 157:
   p = signedbyte() ; goto verticalmotion ;
case 158:
   p = signedpair() ; goto verticalmotion ;
case 159:
   p = signedtrio() ; goto verticalmotion ;
case 160:
   p = signedquad() ; goto verticalmotion ;
/* y moves */
case 161:
   p = y ; goto verticalmotion ;
case 162:
   p = y = signedbyte() ; goto verticalmotion ;
case 163:
   p = y = signedpair() ; goto verticalmotion ;
case 164:
   p = y = signedtrio() ; goto verticalmotion ;
case 165:
   p = y = signedquad() ; goto verticalmotion ;
/* z moves */
case 166:
   p = z ; goto verticalmotion ;
case 167:
   p = z = signedbyte() ; goto verticalmotion ;
case 168:
   p = z = signedpair() ; goto verticalmotion ;
case 169:
   p = z = signedtrio() ; goto verticalmotion ;
case 170:
   p = z = signedquad() ; goto verticalmotion ;
}
         goto endofloop ;
      } else if (cmd < 236) {   /* font selection command */
         if (cmd < 235) fnt = cmd - 171 ;
         else fnt = dvibyte() ;
         curfnt = TeXfonts[fnt] ;
         thinspace = curfnt->thinspace ;
      } else {
      switch (cmd) {
/* illegal options */
case 236: case 237: case 238: case 244: case 245: case 246: case 247: 
case 248: case 249: case 250: case 251: case 252: case 253: case 254: 
case 255:
   error("! illegal command in dvi file") ;
/* font definition */
case 243:
   fontdef() ;
   break ;
/* specials */   /* this should eventually call a do special routine. */
case 239: p = dvibyte() ; dospecial(p) ; break ;
case 240: p = twobytes() ; dospecial(p) ; break ;
case 241: p = threebytes() ; dospecial(p) ; break ;
case 242: p = signedquad() ; dospecial(p) ; break ;
}
      }
      goto endofloop ;
/*
 *   The calculations here are crucial to the appearance of the document.
 *   If the motion is less than a thinspace, we round the motion; otherwise,
 *   we update the position and round the new position.  Then we check to
 *   insure that the rounded position didn't accumulate an error that was
 *   greater than MAXDRIFT.
 */
verticalmotion:
/* vertical motion cases */
      v += p ;
      if (p >= thinspace) vv = floor(v * conv + 0.5) ;
      else if (p <= -thinspace) vv = floor(v * conv + 0.5) ;
      else 
      { vv += floor(p * conv + 0.5) ;
        hhh = floor(v * conv + 0.5) ;
        if (hhh - vv > MAXDRIFT) vv = hhh - MAXDRIFT ;
        else if (vv - hhh > MAXDRIFT) vv = hhh + MAXDRIFT ;
      }
      goto endofloop ;
/*
 *   The horizontal kerning is done exactly analogously to the vertical
 *   motion, only characters are handled automatically; there kern is
 *   not dependent directly on their width in the dvi units.  Thus, we
 *   do the hh and h motion in the drawchar part, and only check the
 *   rounding here.
 */
horizontalmotion:
/* horizontal motion cases */
      h += p ;
      if (p >= thinspace || p <= -4 * thinspace) hh = floor(h * conv + 0.5) ;
      else hh += floor(p * conv + 0.5) ;
setmotion:
      hhh = floor(h * conv + 0.5) ;
      if (hhh - hh > MAXDRIFT) { hh = hhh - MAXDRIFT ; }
      else if (hh - hhh > MAXDRIFT) { hh = hhh + MAXDRIFT ; }
endofloop: ;
   }
   pageend() ;
}
