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
 *   scalewidth.c of dvisw software package.  This code is copyright (C) 1985
 *   by Radical Eye Software.
 *
 *   Scales the width values.  Takes two thirty-two bit integers, multiplies
 *   them, divides them by 2^20, and returns the thirty-two bit result.
 *   The first integer, the width in FIXes, can lie between -2^24 and 2^24-1.
 *   The second integer, the scale factor, can lie between 0 and 2^27-1.  The
 *   arithmetic must be exact.  The answer is truncated to an integer.
 *
 *   Since this math is special, we put it in its own file.  It is the only
 *   place in the program where such accuracy is required.
 */
#include "structures.h"

integer
scalewidth(a, b)
	register integer a, b ;
{
  register integer al, bl ;
  al = a & 32767 ;
  bl = b & 32767 ;
  a >>= 15 ;
  b >>= 15 ;
  return ( ((al*bl/32768) + a*bl+al*b)/32 + a*b*1024) ;
}
