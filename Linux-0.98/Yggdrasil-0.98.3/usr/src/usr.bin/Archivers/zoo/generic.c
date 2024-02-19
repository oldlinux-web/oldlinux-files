#ifndef LINT
static char genericid[]="@(#) generic.c 2.2 88/01/24 12:44:03";
#endif /* LINT */

/* 
Generic template for machine-dependent functions.

The contents of this file are hereby released to the public domain

											-- Rahul Dhesi 1991/07/05
*/

/****************
Date and time functions are assumed to be standard **IX-style functions.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

/* Function isadir() returns 1 if the supplied handle is a directory, 
else it returns 0.  
*/

int isadir (file)
ZOOFILE file;
{
   int handle = fileno(file);
   struct stat buf;           /* buffer to hold file information */
   if (fstat (handle, &buf) == -1) {
      return (0);             /* inaccessible -- assume not dir */
   } else {
      if (buf.st_mode & S_IFDIR)
         return (1);
      else
         return (0);
   }
}

/****************
Function fixfname() converts the supplied filename to a syntax
legal for the host system.  It is used during extraction.
*/

char *fixfname(fname)
char *fname;
{
	fname[FNLIMIT] = '\0';		/* Just truncate at filename size limit */
	return fname;
}

/* The function gettz() returns the offset from GMT in seconds, taking
into account the local timezone and any daylight savings time. */

/* This version of gettz should be portable to all Unices, although it
can't be described as elegant.  Users immediately west of the International
Date Line (Polynesia, Soviet Far East) may get times out by 24 hours.
Contributed by: Ian Phillipps <igp@camcon.co.uk> */

long gettz()
{
#define NOONOFFSET 43200
#define SEC_IN_DAY	(24L * 60L * 60L)
#define INV_VALUE		(SEC_IN_DAY + 1L)
	static long retval = INV_VALUE;	     /* cache, init to impossible value */
	extern long time();
	extern struct tm *localtime();
	long now;
	long noon;
	struct tm *noontm;
	if (retval != INV_VALUE)				 /* if have cached value, return it */
		return retval;
   now = time((long *) 0);
   /* Find local time for GMT noon today */
   noon = now - now % SEC_IN_DAY + NOONOFFSET ;
   noontm = localtime( &noon );
   retval = NOONOFFSET - 60 * ( 60 * noontm->tm_hour - noontm->tm_min );
	return retval;
#undef NOONOFFSET
}

/* Standard UNIX-compatible time functions */
#include "nixtime.i"

/* Standard UNIX-specific file attribute routines */
#include "nixmode.i"

/* Assume no file truncate system call.  Ok to be a no-op. */
/*ARGSUSED*/
int zootrunc(f) FILE *f; { return 0; }
	
