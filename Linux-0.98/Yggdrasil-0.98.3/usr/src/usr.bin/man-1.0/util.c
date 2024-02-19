/*
 * util.c
 *
 * Copyright (c) 1991, John W. Eaton.
 *
 * You may distribute under the terms of the GNU General Public
 * License as specified in the README file that comes with the man 1.0
 * distribution.  
 *
 * John W. Eaton
 * jwe@che.utexas.edu
 * Department of Chemical Engineering
 * The University of Texas at Austin
 * Austin, Texas  78712
 */

#ifdef STD_HEADERS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#else
#include <stdio.h>
#include <string.h>
extern char *strdup ();
#endif

/*
 * Extract last element of a name like /foo/bar/baz.
 */
char *
mkprogname (s)
     register char *s;
{
  char *t;

  t = strrchr (s, '/');
  if (t == NULL)
    t = s;
  else
    t++;

  return strdup (t);
}
