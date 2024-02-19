/* Copyright (C) 1991 Free Software Foundation, Inc.
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

#ifdef	__GNUC__

static __inline int
DEFUN(is_neg_68881, (num), register LONG_DOUBLE num)
{
  if (num == 0.0)
    {
      char negative_p;
      asm("ftst%.x %1\n"
	  "fslt %0" : "=g" (negative_p) : "f" (num));
      return negative_p;
    }
  return num < 0.0;
}

#define	IS_NEGATIVE(num)	is_neg_68881(num)

#endif

#include <../sysdeps/ieee754/printf_fp.c>
