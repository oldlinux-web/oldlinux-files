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

/*
 *	ANSI Standard: 2.2.4.2 Characteristics of floating types <float.h>
 */

#if	!defined(_FLOAT_H) ||	\
	(defined(__need_HUGE_VAL) && !defined(__HUGE_VAL_defined))

#ifndef	__need_HUGE_VAL
#define	_FLOAT_H	1
#include <features.h>
#endif	/* Don't need HUGE_VAL.  */

#ifdef	_FLOAT_H
/* ANSI values for FLT_ROUNDS (the names are nonstandard).  */
#define	_FLT_ROUNDS_INDETERMINATE	(-1)
#define	_FLT_ROUNDS_TOZERO		0
#define	_FLT_ROUNDS_TONEAREST		1
#define	_FLT_ROUNDS_TOPOSINF		2
#define	_FLT_ROUNDS_TONEGINF		3
#endif

#ifdef	__HUGE_VAL_defined
#undef	__need_HUGE_VAL
#endif	/* HUGE_VAL defined.  */

#if	defined(_FLOAT_H) || defined(__need_HUGE_VAL)
/* Include the machine-specific <float.h> file.  */
#include <fl.h>

#ifdef	__need_HUGE_VAL
#define	__HUGE_VAL_defined	1
#undef	__need_HUGE_VAL
#endif	/* Need HUGE_VAL.  */
#endif	/* <float.h> included or need HUGE_VAL.  */

#endif	/* float.h  */
