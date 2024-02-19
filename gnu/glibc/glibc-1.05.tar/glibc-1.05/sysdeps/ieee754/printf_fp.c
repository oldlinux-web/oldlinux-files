/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <stdio.h>
#include <math.h>
#include <printf.h>
#include "ieee754.h"

#define	outchar(x)							      \
  do									      \
    {									      \
      register CONST int outc = (x);					      \
      if (putc (outc, s) == EOF)					      \
	return -1;							      \
      else								      \
	++done;								      \
    } while (0)

#ifndef	HANDLE_SPECIAL

#ifdef	__GNUC__
 __inline
#endif
static int
DEFUN(special_ieee754, (s, info, fpnum),
      FILE *s AND CONST struct printf_info *info AND LONG_DOUBLE fpnum)
{
  int is_neg;
  CONST char *string;

  if (__isnan ((double) fpnum))
    {
      string = "NaN";
      is_neg = 0;
    }
  else if (__isinf ((double) fpnum))
    {
      string = "Inf";
      is_neg = fpnum < 0;
    }
  else
    return 0;

  {
    size_t done = 0;
    int width = info->prec > info->width ? info->prec : info->width;

    if (is_neg || info->showsign || info->space)
      --width;
    width -= 3;

    if (!info->left)
      while (width-- > 0)
	outchar (' ');

    if (is_neg)
      outchar ('-');
    else if (info->showsign)
      outchar ('+');
    else if (info->space)
      outchar (' ');

    {
      register size_t len = 3;
      while (len-- > 0)
	outchar (*string++);
    }

    if (info->left)
      while (width-- > 0)
	outchar (' ');

    return done;
  }
}

#define	HANDLE_SPECIAL(done, s, info, fpnum)				      \
  {									      \
    int more_done = special_ieee754 (s, info, fpnum);			      \
    if (more_done == -1)						      \
      return -1;							      \
    else if (more_done != 0)						      \
      return done + more_done;						      \
  }

#endif

#ifndef	IS_NEGATIVE

#ifdef	__GNUC__
__inline
#endif
static int
DEFUN(is_neg_ieee754, (num), register LONG_DOUBLE num)
{
  union ieee754_double u;

  u.d = (double) num;
  return u.ieee.negative;
}

#define	IS_NEGATIVE(num)	is_neg_ieee754 (num)

#endif

#undef	outchar

#include <../sysdeps/generic/printf_fp.c>
