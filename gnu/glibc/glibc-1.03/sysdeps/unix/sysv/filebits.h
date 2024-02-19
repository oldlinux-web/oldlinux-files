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

/* These values should be changed as appropriate for your system.  */

#ifndef	_GNU_FILE_H

#define	_GNU_FILE_H	1


/* File access modes for open and fcntl.  */
#define	__O_RDONLY	0	/* Open read-only.  */
#define	__O_WRONLY	1	/* Open write-only.  */
#define	__O_RDWR	2	/* Open read/write.  */


/* Bits OR'd into the second argument to open.  */
#define	__O_CREAT	0x100	/* Create file if it doesn't exist.  */
#define	__O_EXCL	0x400	/* Fail if file already exists.  */
#define	__O_TRUNC	0x200	/* Truncate file to zero length.  */
#define	__O_NOCTTY	0x800	/* Don't assign a controlling terminal.  */

/* File status flags for `open' and `fcntl'.  */
#define	__O_APPEND	000010	/* Writes append to the file.  */
#define	__O_NONBLOCK	000004	/* Non-blocking I/O.  */

/* BSD before 4.4 doesn't support POSIX.1 O_NONBLOCK,
   but O_NDELAY is close.  */
#define	__O_NDELAY	__O_NONBLOCK

/* Mask for file access modes.  */
#define	__O_ACCMODE	(__O_RDONLY|__O_WRONLY|__O_RDWR)

/* Alternate names for values for the WHENCE argument to `lseek'.  */
#define	__L_SET		0	/* Seek from beginning of file.  */
#define	__L_INCR	1	/* Seek from current position.  */
#define	__L_XTND	2	/* Seek from end of file.  */

/* Operations for the `flock' call.  */
#define	__LOCK_SH	1    /* Shared lock.  */
#define	__LOCK_EX	2    /* Exclusive lock.  */
#define	__LOCK_UN	8    /* Unlock.  */

/* Can be OR'd in to one of the above.  */
#define	__LOCK_NB	4    /* Don't block when locking.  */

#endif	/* gnu/file.h */
