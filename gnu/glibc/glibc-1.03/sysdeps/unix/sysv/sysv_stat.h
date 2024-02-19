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

struct sysv_stat
  {
    short int st_dev;
    unsigned short int st_ino;
    unsigned short int st_mode;
    short int st_nlink;
    unsigned short int st_uid;
    unsigned short int st_gid;
    short int st_rdev;
    long int st_size;
    long int st_atime;
    long int st_mtime;
    long int st_ctime;
  };

extern int EXFUN(__sys_stat, (CONST char *filename, struct sysv_stat *buf));
extern int EXFUN(__sys_fstat, (int fd, struct sysv_stat *buf));


#ifdef	__GNUC__
__inline
#endif
static void
DEFUN(translate_stat, (out, in),
      struct stat *out AND CONST struct sysv_stat *in)
{
  out->st_dev = in->st_dev;
  out->st_ino = in->st_ino;
  out->st_mode = in->st_mode;
  out->st_nlink = in->st_nlink;
  out->st_uid = in->st_uid;
  out->st_gid = in->st_gid;
  out->st_rdev = in->st_rdev;
  out->st_size = in->st_size;
  out->st_atime = in->st_atime;
  out->st_atime_usec = 0;
  out->st_mtime = in->st_mtime;
  out->st_mtime_usec = 0;
  out->st_ctime = in->st_ctime;
  out->st_ctime_usec = 0;
  out->st_blksize = 0;
  out->st_blocks = -1;
}
