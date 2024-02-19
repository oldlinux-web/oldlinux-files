#ifndef LINT
/* @(#) sysv.c 2.5 88/01/10 14:47:24 */
static char sysvid[]="@(#) sysv.c 2.5 88/01/10 14:47:24";
#endif /* LINT */

/* machine.c for System V */

/*
The contents of this file are hereby released to the public domain.

                                    -- Rahul Dhesi  1986/12/31
*/

#ifdef UNBUF_IO	/* do not use */
/*
Function tell() returns the current seek position for a file 
descriptor.  Microport System V/AT has an undocumented tell()
library function (why?) but the **IX PC doesn't, so we code
one here.  It is needed for unbuffered I/O only.
*/
long lseek PARMS ((int, long, int));
long tell (fd)
int fd;
{ return (lseek (fd, 0L, 1)); }
#endif /* UNBUF_IO */

/****************
Date and time functions are standard **IX-style functions.  "nixtime.i"
will be included by machine.c.
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
   return (fname); /* default is no-op */
}

extern long timezone;   /* defined by library routine */
long time ();
struct tm *localtime ();

/* Function gettz(), returns the offset from GMT in seconds of the
local time, taking into account daylight savings time */

#if 1		/* Following should work for System V */
long gettz()
{
#define SEC_IN_DAY	(24L * 60L * 60L)
#define INV_VALUE		(SEC_IN_DAY + 1L)
	static long retval = INV_VALUE;	     /* cache, init to impossible value */
	struct tm *tm;
	long clock;
	if (retval != INV_VALUE)				 /* if have cached value, return it */
		return retval;
	clock = time ((long *) 0);
	tm = localtime (&clock);
	retval = timezone - tm->tm_isdst*3600;
	return retval;
}
#else
/* This version of gettz should be portable to all Unices, although it can't
   be described as elegant. Users immediately west of the International
   Date Line (Polynesia, Soviet Far East) may get times out by 24 hours.
   Contributed by: Ian Phillipps <igp@camcon.co.uk> */

/* Function gettz(), returns the offset from GMT in seconds */
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
#endif

/* Standard **IX-compatible time functions */
#include "nixtime.i"

/* Standard **IX-specific file attribute routines */
#include "nixmode.i"

/* 
Make a directory.  System V has no system call accessible to 
ordinary users to make a new directory.  Hence we spawn a shell 
and hope /bin/mkdir is there.  Since /bin/mkdir gives a nasty 
error message if it fails, we call it only if nothing already 
exists by the name of the needed directory.
*/

int mkdir(dirname)
char *dirname;
{
   char cmd[PATHSIZE+11+1]; /* room for "/bin/mkdir " used below  + 1 spare */
   if (!exists(dirname)) {
      strcpy(cmd, "/bin/mkdir ");
      strcat(cmd, dirname);
      return (system(cmd));
   }
	return (0);
}

/* No file truncate system call in older System V.  If yours has one,
add it here -- see bsd.c for example.  It's ok for zootrunc to be
a no-op. */
/*ARGSUSED*/
int zootrunc(f) FILE *f; { return 0; }
	
