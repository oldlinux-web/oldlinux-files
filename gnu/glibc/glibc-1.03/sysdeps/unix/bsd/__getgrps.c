/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <sysdep.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>


extern int EXFUN(__bsd_getgroups, (int n, int list[]));

/* If SIZE is zero, return the number of supplementary groups
   the calling process is in.  Otherwise, fill in the group IDs
   of its supplementary groups in LIST and return the number written.  */
int
DEFUN(__getgroups, (size, list), int size AND gid_t list[])
{
  int *groups;
  int n;
  register int i;

  if (sizeof (gid_t) == sizeof (int) || size == 0)
    return __bsd_getgroups (size, (int *) NULL);

  if (size < 0 || size > NGROUPS_MAX || list == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  groups = (int *) __alloca (size * sizeof (int));
  n = __bsd_getgroups(size, groups);
  if (n != -1)
    for (i = 0; i < n; ++i)
      list[i] = (gid_t) groups[i];
  return n;
}
