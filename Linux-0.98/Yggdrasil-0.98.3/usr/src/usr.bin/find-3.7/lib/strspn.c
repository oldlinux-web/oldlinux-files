/* strspn.c -- return numbers of chars at start of string in a class
   Copyright (C) 1987, 1990 Free Software Foundation, Inc.

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

#if defined(USG)
#include <string.h>
#define index strchr
#else
#include <strings.h>
#endif

int
strspn (str, class)
     char *str, *class;
{
  register char *st = str;

  while (*st && index (class, *st))
    ++st;
  return st - str;
}
