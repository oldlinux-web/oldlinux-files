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

#include <sysdep.h>

#ifndef	SYS_brk
#define	SYS_brk	17
#endif

#if/*ndef	HAVE_GNU_LD*/1
#define	__end		_end
#endif

.data
.global ___curbrk
___curbrk:
	.long __end

.text
ENTRY (__brk)
	add %o0, 7, %o0
	andn %o0, 7, %o0
	mov %o0, %o2		/* Save arg where it won't be clobbered.  */
	mov SYS_brk, %g1
	ta 0
	bcs error
	sethi %hi(___curbrk), %g1
	st %o2, [%g1 + %lo(___curbrk)]
	retl
error:	sethi %hi(_errno), %g1
	st %o0, [%g1 + %lo(_errno)]
	retl
	sub %g0, 1, %o0
