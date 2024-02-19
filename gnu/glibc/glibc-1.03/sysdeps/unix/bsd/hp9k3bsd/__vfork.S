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

#ifndef	SYS_vfork
#define	SYS_vfork	66
#endif

/* Clone the calling process, but without copying the whole address space.
   The the calling process is suspended until the the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
.globl ___vfork
___vfork:
	/* Pop the return PC value into A0.  */
	moveal sp@+, a0

	/* Stuff the syscall number in D0 and trap into the kernel.  */
	movel #SYS_vfork, d0
	trap #0
	bcs error		/* Branch forward if it failed.  */

	/* It succeeded.  See which fork we're in.  */
	tstl d1			/* If D1 is zero, we're the parent.  */
	beq parent

	/* We're the child.  Clear D0.  */
	clrl d0

	/* Jump to the return PC.  */
parent:	jmp a0@

error:
	movel d0, _errno
	moveq #-1, d0
	jmp a0@
