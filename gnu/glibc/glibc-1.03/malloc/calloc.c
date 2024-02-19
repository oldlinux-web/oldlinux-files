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

/* IGNORE(@ */
#include <ansidecl.h>
/* @) */
#include <malloc.h>

#if	defined(__GNU_LIBRARY__) || defined(STDC_HEADERS) || defined(USG)
#include <string.h>
#else
#define	memset(s, zero, n)	bzero ((s), (n))
#endif

/* Allocate an array of NMEMB elements each SIZE bytes long.
   The entire array is initialized to zeros.  */
PTR
DEFUN(calloc, (nmemb, size), register size_t nmemb AND register size_t size)
{
  register PTR result = malloc (nmemb * size);

  if (result != NULL)
    (void) memset (result, 0, nmemb * size);

  return result;
}
