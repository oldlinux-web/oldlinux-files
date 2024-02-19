/* strdup.c -- return a newly allocated copy of a string
   Copyright (C) 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifdef STRDUP_MISSING
/* If you have the STDC headers, you surely have strdup, too. */
char *malloc ();
char *strcpy ();

/* Return a newly allocated copy of string S;
   return 0 if out of memory. */

char *
strdup (s)
     char *s;
{
  char *p;

  p = malloc ((unsigned) (strlen (s) + 1));
  if (p)
    strcpy (p, s);
  return p;
}
#endif
