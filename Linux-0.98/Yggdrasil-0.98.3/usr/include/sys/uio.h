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

#ifndef _SYS_UIO_H

#define _SYS_UIO_H	1

#include <features.h>
#include <traditional.h>

#ifdef __STDC__
#define	__need_size_t
#include <stddef.h>
#else
#include <sys/types.h>
#endif


/* Structure describing a section of memory.  */

struct iovec
  {
    /* Starting address.  */
    void *iov_base;
    /* Length in bytes.  */
    size_t iov_len;
  };


#ifdef __cplusplus
extern "C" {
#endif

/* Read data from file descriptor FD, and put the result in the
   buffers described by VECTOR, which is a vector of COUNT `struct iovec's.
   The buffers are filled in the order specified.
   Operates just like `read' (see <unistd.h>) except that data are
   put in VECTOR instead of a contiguous buffer.  */
extern int readv _ARGS ((int __fd, const struct iovec *__vector,
		size_t __count));

/* Write data pointed by the buffers described by VECTOR, which
   is a vector of COUNT `struct iovec's, to file descriptor FD.
   The data is written in the order specified.
   Operates just like `write' (see <unistd.h>) except that the data
   are taken from VECTOR instead of a contiguous buffer.  */
extern int writev _ARGS ((int __fd, const struct iovec *__vector,
		size_t __count));

#ifdef __cplusplus
}
#endif

#endif /* sys/uio.h */
