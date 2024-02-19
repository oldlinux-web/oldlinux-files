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

#ifndef	__need_HUGE_VAL

/* These are the minimum magnitudes given in the ANSI Standard.  */

#define	FLT_RADIX	2

#define	FLT_ROUNDS	-1	/* Indeterminable.  */

#define	FLT_MANT_DIG	0	/* Unspecified.  */
#define	DBL_MANT_DIG	0	/* Unspecified.  */
#define	LDBL_MANT_DIG	0	/* Unspecified.  */

#define	FLT_DIG		6
#define	DBL_DIG		10
#define	LDBL_DIG	10

#define	FLT_MIN_EXP	0	/* Unspecified.  */
#define	DBL_MIN_EXP	0	/* Unspecified.  */
#define	LDBL_MIN_EXP	0	/* Unspecified.  */

#define	FLT_MIN_10_EXP	37
#define	DBL_MIN_10_EXP	37
#define	LDBL_MIN_10_EXP	37

#define	FLT_MAX_EXP	0	/* Unspecified.  */
#define	DBL_MAX_EXP	0	/* Unspecified.  */
#define	LDBL_MAX_EXP	0	/* Unspecified.  */

#define	FLT_MAX_10_EXP	37
#define	DBL_MAX_10_EXP	37
#define	LDBL_MAX_10_EXP	37

#define	FLT_MAX		1e37
#define	DBL_MAX		1e37
#define	LDBL_MAX	1e37

#define	FLT_EPSILON	1e-5
#define	DBL_EPSILON	1e-9
#define	LDBL_EPSILON	1e-9

#define	FLT_MIN		1e-37
#define	DBL_MIN		1e-37
#define	LDBL_MIN	1e-37

#else	/* Need HUGE_VAL.  */

/* Used by <stdlib.h> and <math.h> functions for overflow.	*/
#define	HUGE_VAL	1e37

#endif	/* Don't need HUGE_VAL.  */
