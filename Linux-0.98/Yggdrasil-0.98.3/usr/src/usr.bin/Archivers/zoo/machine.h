/* @(#) machine.h 2.1 87/12/25 12:22:43 */

/*
The contents of this file are hereby released to the public domain.

                           -- Rahul Dhesi 1986/11/14
*/

/* 
This file holds definitions that usually do not change
between different systems, except when using REALLY strange systems.  But
options.h and machine.c hold stuff that does change quite a bit.
*/

/* 
MAXLONG is the maximum size of a long integer.  Right now it doesn't have to
be accurate since it's only used within zooext() to fake infinite disk space.
*/
#define  MAXLONG  ((unsigned long) (~0L))

/* 
Type BYTE must hold exactly 8 bits.  The code will collapse badly if BYTE is
anything other than exactly 8 bits. To avoid sign extension when casting
BYTE to a longer size, it must be declared unsigned.  For machine-
independence, Zoo does all I/O of archive headers and directory entries 
in units of BYTE.  The actual file data are not written in units of
BYTE, however, so portability may not be absolute.
*/
typedef  unsigned char BYTE;  /* type corresponding to an 8-bit byte */

