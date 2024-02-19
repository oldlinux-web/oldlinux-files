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

#include <ansidecl.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <hurd.h>

/* Read the contents of the symbolic link PATH into no more than
   LEN bytes of BUF.  The contents are not null-terminated.
   Returns the number of characters read, or -1 for errors.  */
int
DEFUN(__readlink, (path, buf, len),
      CONST char *path AND char *buf AND size_t len)
{
  error_t err;
  file_t file;

  file = __path_lookup (path, FS_LOOKUP_READ|FS_LOOKUP_NOTRANS, 0);
  if (file == MACH_PORT_NULL)
    return -1;

  p = buf;
  while (len > 0)
    {
      char *s = p;
      size_t nread = len;
      err = __io_read (file, &s, &nread);
      if (err)
	break;
      if (nread == 0)
	break;
      if (s != p)
	{
	  memcpy (p, s, nread);
	  __vm_deallocate (__mach_task_self (), s, nread);
	}
      len -= nread;
      p += nread;
    }

  __mach_port_deallocate (__mach_task_self (), file);

  if (err)
    return __hurd_fail (err);

  return p - buf;
}