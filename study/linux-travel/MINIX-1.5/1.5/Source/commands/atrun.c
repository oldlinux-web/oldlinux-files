/* atrun - perform the work 'at' has squirreled away	Author: Jan Looyen */

/*-------------------------------------------------------------------------*
 *	atrun scans directory /usr/spool/at for 'at' jobs to be executed.  *
 *	Finished jobs have been moved to directory /usr/spool/at/past.     *
 *-------------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <stdio.h>

main()
{
  int fd, nr;
  char realtime[15], procname[35], procpast[35];
  struct direct dirbuf;
  struct tm *p, *localtime();
  struct stat sbuf;
  time_t clock;

/*-------------------------------------------------------------------------*
 *	Compute real time,  move 'at' jobs whose filenames < real time to  *
 *	/usr/spool/at/past and start a sh for each job.			   *
 *-------------------------------------------------------------------------*/
  time(&clock);
  p = localtime(&clock);
  sprintf(realtime, "%02d.%03d.%02d%02d.00",
	p->tm_year % 100, p->tm_yday, p->tm_hour, p->tm_min);
  if ((fd = open("/usr/spool/at", O_RDONLY)) > 0)
	while (read(fd, (char *) &dirbuf, sizeof(dirbuf)) > 0)
		if (dirbuf.d_ino > 0 &&
		    dirbuf.d_name[0] != '.' &&
		    dirbuf.d_name[0] != 'p' &&
		    strncmp(dirbuf.d_name, realtime, 11) <= 0) {

			sprintf(procname, "/usr/spool/at/%.14s", dirbuf.d_name);
			sprintf(procpast, "/usr/spool/at/past/%.14s", dirbuf.d_name);

			if (fork() == 0)	/* code for child */
				if (link(procname, procpast) == 0) {	/* link ok? */
					unlink(procname);
					stat(procpast, &sbuf);
					setgid(sbuf.st_uid);
					setuid(sbuf.st_gid);
					execl("/bin/sh", "sh", procpast, (char *) 0);
					fprintf(stderr, "proc %s can't start\n", procpast);
					exit(1);
				}
		}
}
