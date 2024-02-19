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

#include <ansidecl.h>
#include <sys/types.h>

extern int EXFUN(__getdirentries, (int fd, char *buf,
				   size_t nbytes, off_t *basep));

/* Sun Unix lets directories be read only by the `getdirentries' call.  */
#define	READ_DIRECTORY							      \
{									      \
  long int offset;							      \
  int bytes = __getdirentries(dirp->__fd, dirp->__data,			      \
			      (int) dirp->__allocation, &offset);	      \
  if (bytes <= 0)							      \
    return NULL;							      \
  dirp->__size = (size_t) bytes;					      \
}

#include <../sysdeps/unix/bsd/readdir.c>
