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

#include <sysdeps/unix/sysdep.h>

#ifdef	__STDC__
#define	ENTRY(name)							      \
  .global _##name;							      \
  .align 2;								      \
  _##name##:
#else
#define	ENTRY(name)							      \
  .global _/**/name;							      \
  .align 2;								      \
  _/**/name/**/:
#endif

#ifdef	__STDC__
#define	PSEUDO(name, syscall_name, args)				      \
  ENTRY (name)								      \
  mov SYS_##syscall_name, %g1;					   	      \
  ta 0;									      \
  bcc 1f;								      \
  sethi %hi(_errno), %g1;						      \
  st %o0, [%g1 + %lo(_errno)];						      \
  sub %g0, 1, %o0;							      \
1:
#else
#define	PSEUDO(name, syscall_name, args)				      \
  ENTRY (name)								      \
  mov SYS_/**/syscall_name, %g1;				   	      \
  ta 0;									      \
  bcc 1f;								      \
  sethi %hi(___errno), %g1;						      \
  st %o0, [%g1 + %lo(___errno)];					      \
  sub %g0, 1, %o0;							      \
1:
#endif

#define	ret	retl; nop
#define	r0	%o0
#define	r1	%o1
#define	MOVE(x,y)	mov x , y
