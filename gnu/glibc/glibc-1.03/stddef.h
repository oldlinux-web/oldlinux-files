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
 *	ANSI Standard: 4.1.4 Common definitions	<stddef.h>
 */

#ifndef	_STDDEF_H

/* All this #ifdef madness is so that only things which certain
   headers need will be defined by them.  The ANSI standard says
   this must be so.  Each header that needs the definitions of
   `wchar_t', `size_t' or `NULL' #defines `__need_XXX' (where XXX
   is what it needs) and it gets only the definitions it asks for.
   The `__need_XXX' macro is automagically #undef'd before control
   returns to the calling header.  The header need not check to
   see if things are already defined, but it may do so using
   `__XXX_defined' macros.  */
#if	!defined(__need_wchar_t) && !defined(__need_size_t) &&	\
	!defined(__need_ptrdiff_t) && !defined(__need_NULL)
#define	_STDDEF_H	1
#include <features.h>
#endif


#if	!defined(__ptrdiff_t_defined) &&			\
	(defined(_STDDEF_H) || defined(__need_ptrdiff_t))
#define	__ptrdiff_t_defined	1

/* The result of subtracting two pointers.  */
typedef long int ptrdiff_t;

#endif	/* ptrdiff_t not defined and <stddef.h> or need ptrdiff_t.  */
#undef	__need_ptrdiff_t


#if	!defined(__size_t_defined) &&			\
	(defined(_STDDEF_H) || defined(__need_size_t))
#define	__size_t_defined	1

/* The result of the `sizeof' operator.  */
typedef unsigned long int size_t;

#endif	/* size_t not defined and <stddef.h> or need size_t.  */
#undef	__need_size_t


#if	!defined(__wchar_t_defined) &&			\
	(defined(_STDDEF_H) || defined(__need_wchar_t))
#define	__wchar_t_defined	1

/* Wide character type.
   Unless the implementation of multibyte functions and the
   corresponding locale information is changed, this must be
   an unsigned integral type.
   Locale-writers should change this as necessary to
   be big enough to hold unique values not between 0 and 127,
   and not the type's maximum value (unsigned version of -1),
   for each defined multibyte character.  */
typedef	unsigned char	wchar_t;

#endif	/* wchar_t not defined and <stddef.h> or need wchar_t.  */
#undef	__need_wchar_t


#if	!defined(NULL) &&			\
	(defined(_STDDEF_H) || defined(__need_NULL))

/* Null pointer constant.  */
/* IGNORE($ This cannot be just ((PTR) 0) because ((char *) 0) is wrong.  */
#ifdef	__STDC__
/* $) IFANSI($	*/
#define	NULL	((void *) 0)
/* $) IGNORE($	*/
#else	/* Not ANSI C.  */
/* $) IFTRAD($	*/
#define	NULL	0
/* $) IGNORE($	*/
#endif	/* ANSI C.  */
/* $)	*/

#endif	/* NULL not defined and <stddef.h> or need NULL.  */
#undef	__need_NULL


#ifdef	_STDDEF_H
/* Offset, in bytes, of member M in an S structure.  */
#define	offsetof(s, m)	((size_t) &((s *) NULL)->m)
#endif	/* <stddef.h> included.  */


#endif	/* <stddef.h> not already included.  */
