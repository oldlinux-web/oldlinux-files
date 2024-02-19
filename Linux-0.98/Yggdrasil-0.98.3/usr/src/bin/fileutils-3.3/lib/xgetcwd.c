/* xgetcwd.c -- return current directory with unlimited length
   Copyright (C) 1992 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Written by David MacKenzie, djm@gnu.ai.mit.edu. */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include "pathmax.h"

#if !defined(_POSIX_VERSION) && !defined(USG)
char *getwd ();
#define getcwd(buf, max) getwd (buf)
#else
char *getcwd ();
#endif

/* Amount to increase buffer size by in each try. */
#define PATH_INCR 32

char *xmalloc ();
char *xrealloc ();

/* Return the current directory, newly allocated, arbitrarily long.
   Return NULL and set errno on error. */

char *
xgetcwd ()
{
  char *cwd;
  char *ret;
  long path_max;

  errno = 0;
  path_max = PATH_MAX;
  path_max += 2;		/* The getcwd docs say to do this. */

  cwd = (char *) xmalloc (path_max);

  errno = 0;
  while ((ret = getcwd (cwd, path_max)) == NULL && errno == ERANGE)
    {
      path_max += PATH_INCR;
      cwd = xrealloc (cwd, path_max);
      errno = 0;
    }

  if (ret == NULL)
    {
      int save_errno = errno;
      free (cwd);
      errno = save_errno;
      return NULL;
    }
  return cwd;
}
