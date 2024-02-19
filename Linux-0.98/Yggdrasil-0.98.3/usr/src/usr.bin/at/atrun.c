/*
 *
 * atrun - runs jobs queued by /bin/at
 *
 * Written 11/10/89 by Ed Carp (erc@apple.com)
 * Adapted for linux 08/11/92.
 *
 * Copyright 1992 by Ed Carp
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include "at.h"
extern int errno;
extern char *sys_errlist[];
extern int sys_nerr;
main (argc, argv, envp)
     int argc;
     char *argv[];
     char *envp[];
{
   char atfile[128], execfile[40];
   struct stat buf;
   struct tm *tm;
   int userid, groupid, curtime, date, i, j, uid;
   long secs;

   /* get current time and date */

   uid = getuid ();
   secs = time ((long *) 0);
   tm = localtime (&secs);
   curtime = (tm->tm_hour * 100) + tm->tm_min;
   date = ((1 + tm->tm_mon) * 10000) + (tm->tm_mday * 100) + tm->tm_year;
   /* process all files for the specified date/time */
   for (j = 0; j <= ATLIM; j++)
     {
	sprintf (atfile, "%s/%04d%06d.%d",
		 ATJOBS, curtime, date, j);
	if (EOF == stat (atfile, &buf))
	   continue;
	/* if(buf.st_uid != uid) continue; */
	setuid (buf.st_uid);	/* set uid */
	if (-1 == system (atfile))
	   perror ("atrun exec");
	setuid (uid);		/* be root (or whoever) again */
	unlink (atfile);
     }
}
