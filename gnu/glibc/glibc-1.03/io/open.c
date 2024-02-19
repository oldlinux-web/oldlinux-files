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
#include <fcntl.h>

/* The `open' function might take three arguments.  */

#undef	open

#ifdef	HAVE_GNU_LD

#include <gnu-stabs.h>

symbol_alias(__open, open);

#else	/* No GNU stabs.  */

#include <stdarg.h>

int
DEFUN(open, (file, oflag), CONST char *file AND int oflag DOTS)
{
  if (oflag & O_CREAT)
    {
      va_list args;
      int mode;
      va_start(args, oflag);
      mode = va_arg(args, int);
      va_end(args);
      return __open(file, oflag, mode);
    }

  return __open(file, oflag);
}

#endif	/* GNU stabs.  */
