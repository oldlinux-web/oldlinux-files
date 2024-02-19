#ifndef LINT
/* @(#) nixmode.i 1.2 88/01/24 12:48:57 */
static char modeid[]="@(#) nixmode.i 1.2 88/01/24 12:48:57";
#endif

/*
(C) Copyright 1988 Rahul Dhesi -- All rights reserved

UNIX-specific routines to get and set file attribute.  These might be 
usable on other systems that have the following identical things:
fileno(), fstat(), chmod(), sys/types.h and sys/stat.h.
*/

/*
Get file attributes.  Currently only the lowest nine of the
**IX mode bits are used.  Also we return bit 23=0 and bit 22=1,
which means use portable attribute format, and use attribute
value instead of using default at extraction time.
*/

unsigned long getfattr (f)
ZOOFILE f;
{
	int fd;
   struct stat buf;           /* buffer to hold file information */
	fd = fileno(f);
   if (fstat (fd, &buf) == -1)
      return (NO_FATTR);      /* inaccessible -- no attributes */
	else
		return (unsigned long) (buf.st_mode & 0x1ffL) | (1L << 22);
}

/*
Set file attributes.  Only the lowest nine bits are used.
*/

int setfattr (f, a)
char *f;							/* filename */
unsigned long a;				/* atributes to set */
{
	return (chmod (f, (int) (a & 0x1ff)));
}
