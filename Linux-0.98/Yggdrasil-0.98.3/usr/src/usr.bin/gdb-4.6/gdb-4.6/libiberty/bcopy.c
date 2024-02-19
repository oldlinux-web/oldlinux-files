/* bcopy -- copy memory regions of arbitary length
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

	bcopy -- copy memory regions of arbitrary length

SYNOPSIS

	void bcopy (char *in, char *out, int length)

DESCRIPTION

	Copy LENGTH bytes from memory region pointed to by IN to memory
	region pointed to by OUT.

BUGS
	Significant speed improvements can be made in some cases by
	implementing copies of multiple bytes simultaneously, or unrolling
	the copy loop.

*/

void
bcopy (src, dest, len)
  register char *src, *dest;
  int len;
{
  if (dest < src)
    while (len--)
      *dest++ = *src++;
  else
    {
      char *lasts = src + (len-1);
      char *lastd = dest + (len-1);
      while (len--)
        *(char *)lastd-- = *(char *)lasts--;
    }
}
