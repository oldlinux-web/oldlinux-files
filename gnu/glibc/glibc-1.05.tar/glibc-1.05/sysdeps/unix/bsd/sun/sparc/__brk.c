/* Copyright (C) 1992 Free Software Foundation, Inc.
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
#include <errno.h>

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif

#include <sysdep.h>

#ifndef	SYS_brk
#define	SYS_brk	17
#endif

#ifndef	HAVE_GNU_LD
#define	_end	end
#endif

extern char _end;
char *__curbrk = &_end;

/* Set the end of the process's data space to ADDR.
   Return 0 if successful, -1 if not.  */
int
DEFUN(__brk, (addr), PTR addr)
{
  __label__ error asm ("error");
  int err;

  addr = (PTR) (((unsigned long int) addr + 7) & ~7);

  asm ("	mov %1, %%o0\n"
       "	mov %2, %%g1\n"
       "	ta 0\n"
       "	mov %%o0, %0\n"
       "	bcs error"
       : "=r" (err) : "r" (addr), "r" (SYS_brk) : "%o0");      

  __curbrk = addr;
  return 0;

 error:
  errno = err;
  return -1;
}
