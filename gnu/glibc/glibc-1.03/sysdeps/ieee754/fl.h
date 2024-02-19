/* Floating-point constants for IEEE 754 machines,
   where `float' is single-precision, and `double' and `long double'
   are both double-precision.

Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

#ifndef	__need_HUGE_VAL

#define	FLT_RADIX	2

#define	FLT_ROUNDS	_FLT_ROUNDS_TONEAREST	/* IEEE default.  */

#define	FLT_MANT_DIG	24
#define	DBL_MANT_DIG	53
#define	LDBL_MANT_DIG	DBL_MANT_DIG

#define	FLT_DIG		6
#define	DBL_DIG		15
#define	LDBL_DIG	DBL_DIG

#define	FLT_MIN_EXP	(-125)
#define	DBL_MIN_EXP	(-1021)
#define	LDBL_MIN_EXP	DBL_MIN_EXP

#define	FLT_MIN_10_EXP	(-37)
#define	DBL_MIN_10_EXP	(-307)
#define	LDBL_MIN_10_EXP	DBL_MIN_10_EXP

#define	FLT_MAX_EXP	128
#define	DBL_MAX_EXP	1024
#define	LDBL_MAX_EXP	DBL_MAX_EXP

#define	FLT_MAX_10_EXP	38
#define	DBL_MAX_10_EXP	308
#define	LDBL_MAX_10_EXP	DBL_MAX_10_EXP

#define	FLT_MAX		3.40282347e38
#define	DBL_MAX		1.7976931348623157e308
#define	LDBL_MAX	DBL_MAX

#define	FLT_EPSILON	1.19209290e-7
#define	DBL_EPSILON	2.2204460492503131e-16
#define	LDBL_EPSILON	DBL_EPSILON

#define	FLT_MIN		1.17549435e-38
#define	DBL_MIN		2.2250738585072014e-308
#define	LDBL_MIN	DBL_MIN

#ifdef	__USE_GNU
/* IEEE Not A Number.  */

#include <endian.h>

#ifdef __BIG_ENDIAN
#define	__nan_bytes		{ 0x7f, 0xf8, 0, 0, 0, 0, 0, 0 }
#else
#define	__nan_bytes		{ 0, 0, 0, 0, 0, 0, 0xf8, 0x7f }
#endif

#ifdef	__GNUC__
#define	NAN \
  (__extension__ ((union { unsigned char __c[8];			      \
			   double __d; })				      \
		  { __nan_bytes }).__d)
#else	/* Not GCC.  */
static CONST char __nan[8] = __nan_bytes;
#define	NAN	(*(CONST double *) __nan)
#endif	/* GCC.  */

#endif	/* Use GNU.  */

#else	/* Need HUGE_VAL.  */

/* Used by <stdlib.h> and <math.h> functions for overflow.  */

/* IEEE positive infinity.  */

#include <endian.h>

#ifdef __BIG_ENDIAN
#define	__huge_val_bytes	{ 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 }
#else
#define	__huge_val_bytes	{ 0, 0, 0, 0, 0, 0, 0xf0, 0x7f }
#endif

#ifdef	__GNUC__
#define	HUGE_VAL \
  (__extension__ ((union { unsigned char __c[8];			      \
			   double __d; })				      \
		  { __huge_val_bytes }).__d)
#else	/* Not GCC.  */
static CONST char __huge_val[8] = __huge_val_bytes;
#define	HUGE_VAL	(*(CONST double *) __huge_val)
#endif	/* GCC.  */

#endif	/* Don't need HUGE_VAL.  */
