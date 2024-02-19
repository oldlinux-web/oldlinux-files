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

/* Compatibility with BSD bstring(3).  */

#ifndef	_BSTRING_H

#define	_BSTRING_H	1
#include <features.h>

#define	__need_size_t
#include <stddef.h>

/* Copy N bytes of SRC to DEST.  */
extern void EXFUN(bcopy, (CONST PTR __src, PTR __dest, size_t __n));
/* Set N bytes of S to 0.  */
extern void EXFUN(bzero, (PTR __s, size_t __n));
/* Compare N bytes of S1 and S2.  */
extern int EXFUN(bcmp, (CONST PTR __s1, CONST PTR __s2, size_t __n));

/* Return the position of the first bit set in I, or 0 if none are set.
   The least-significant bit is position 1, the most-significant 32.  */
extern int EXFUN(ffs, (int __i));

#if	defined(__GNUC__) && defined(__OPTIMIZE__)
#define	ffs()	__builtin_ffs()
#endif	/* GCC and optimizing.  */


#endif	/* bstring.h  */
