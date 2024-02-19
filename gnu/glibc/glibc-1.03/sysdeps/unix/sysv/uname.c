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
#include <errno.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

struct sysv_utsname
  {
    char sysname[9];
    char nodename[9];
    char release[9];
    char version[9];
    char machine[9];
  };

extern int EXFUN (__utssys, (struct sysv_utsname *buf, int dummy, int opcode));

/* Put information about the system in NAME.  */
int
DEFUN(uname, (name), struct utsname *name)
{
  struct sysv_utsname buf;
  if (__utssys (&buf, 0, 0) < 0)
    return -1;
  strcpy (name->sysname, buf.sysname);
  strcpy (name->nodename, buf.nodename);
  strcpy (name->release, buf.release);
  strcpy (name->version, buf.version);
  strcpy (name->machine, buf.machine);
  return 0;
}
