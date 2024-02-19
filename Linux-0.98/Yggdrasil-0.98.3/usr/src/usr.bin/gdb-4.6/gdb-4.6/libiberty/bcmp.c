/* Emulate bcmp using memcmp
   Copyright (C) 1991 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


/*

NAME

	bcmp -- compare two memory regions

SYNOPSIS

	int bcmp (char *from, char *to, int count)

DESCRIPTION

	Compare two memory regions and return zero if they are identical,
	non-zero otherwise.  If count is zero, return zero.

NOTES

	No guarantee is made about the non-zero returned value.  In
	particular, the results may be signficantly different than
	strcmp(), where the return value is guaranteed to be less than,
	equal to, or greater than zero, according to lexicographical
	sorting of the compared regions.

BUGS

*/


int
bcmp (from, to, count)
  char *from, *to;
  int count;
{
  int rtnval = 0;

  while (count-- > 0)
    {
      if (*from++ != *to++)
	{
	  rtnval = 1;
	  break;
	}
    }
  return (rtnval);
}

