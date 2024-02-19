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
#include <stdlib.h>
/* @) */

#define _MALLOC_INTERNAL
#include <malloc.h>

PTR
DEFUN(memalign, (alignment, size),
      size_t alignment AND size_t size)
{
  PTR result;
  unsigned long int adj;

  size = ((size + alignment - 1) / alignment) * alignment;

  result = malloc (size);
  if (result == NULL)
    return NULL;
  adj = (unsigned long int) ((unsigned long int) ((char *) result -
						  (char *) NULL)) % alignment;
  if (adj != 0)
    {
      struct alignlist *l;
      for (l = _aligned_blocks; l != NULL; l = l->next)
	if (l->aligned == NULL)
	  /* This slot is free.  Use it.  */
	  break;
      if (l == NULL)
	{
	  l = (struct alignlist *) malloc (sizeof (struct alignlist));
	  if (l == NULL)
	    {
	      free (result);
	      return NULL;
	    }
	}
      l->exact = result;
      result = l->aligned = (char *) result + alignment - adj;
      l->next = _aligned_blocks;
      _aligned_blocks = l;
    }

  return result;
}
