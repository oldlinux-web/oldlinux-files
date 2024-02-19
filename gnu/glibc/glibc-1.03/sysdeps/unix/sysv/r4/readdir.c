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
#include <assert.h>
#include <stddef.h>
#include "bsddir.h"

#define D_NAMLEN(d)	(strlen ((d)->d_name))

/* Put into BUF up to NBYTES bytes of `struct direct's read from FD.
   Return the number of bytes actually read.
   This is similar in spirit but not quite the same as the Sun
   getdirentries call.  */
extern int EXFUN(__getdents, (int fd, struct direct *buf, size_t nbytes));

#define	READ_DIRECTORY							      \
{									      \
  int bytes = __getdents (dirp->__fd,					      \
			  (struct direct *) dirp->__data,		      \
			  dirp->__allocation);	      			      \
  if (bytes <= 0)							      \
    return NULL;							      \
  dirp->__size = (size_t) bytes;					      \
}

#include <sysdeps/unix/bsd/readdir.c>
