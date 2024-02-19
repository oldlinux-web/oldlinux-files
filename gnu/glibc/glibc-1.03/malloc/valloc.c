/* Allocate memory on a page boundary.
   Copyright (C) 1991 Free Software Foundation, Inc.

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

/* IGNORE(@ */
#include <ansidecl.h>
#include <stdlib.h>
/* @) */

#include <malloc.h>

#ifdef	__GNU_LIBRARY__
extern size_t EXFUN(__getpagesize, (NOARGS));
#else
#ifndef	USG
extern size_t EXFUN(getpagesize, (NOARGS));
#define	__getpagesize()	getpagesize()
#else
#include <sys/param.h>
#ifdef	EXEC_PAGESIZE
#define	__getpagesize()	EXEC_PAGESIZE
#else	/* No EXEC_PAGESIZE.  */
#ifdef	NBPG
#ifndef	CLSIZE
#define	CLSIZE	1
#endif	/* No CLSIZE.  */
#define	__getpagesize()	(NBPG * CLSIZE)
#else	/* No NBPG.  */
#define	__getpagesize()	NBPC
#endif	/* NBPG.  */
#endif	/* EXEC_PAGESIZE.  */
#endif	/* USG.  */
#endif

static size_t pagesize;

PTR
DEFUN(valloc, (size), size_t size)
{
  if (pagesize == 0)
    pagesize = __getpagesize();

  return memalign (pagesize, size);
}
