
/***************************************************************************
 * Program:  lastlogin 			(c)1987 ICUS Computer Group        *
 * By:       Lenny Tropiano		...{ihnp4,mtune}!icus!lenny        *
 *                                                                         *
 * Program intent:   This will allow programs like 'finger' and 'last' to  *
 *                   lookup in the file /usr/adm/lastlogin.log and see     *
 *                   when a particular user has logged-in.   This saves    *
 *                   the necessity to keep /etc/wtmp around for a long     *
 *                   period of time.                                       *
 *                                                                         *
 *                   This program can be used/modified and redistributed   *
 *                   I declare it PUBLIC DOMAIN.  Please give me credit    *
 *                   when credit is due.                                   *
 *                                                                         *
 *      AT&T 3B1 compiling instructions for shared-libaries:               *
 *                                                                         *
 *      $ cc -c -O lastlogin.c                                             *
 *      $ ld -s -o lastlogin lastlogin.o /lib/shlib.ifile /lib/crt0s.o     *
 *      $ mv lastlogin /etc                                                *
 *      $ su                                                               *
 *      Password:                                                          *
 *      # chown adm /etc/lastlogin /usr/adm                                *
 *      # chgrp adm /etc/lastlogin /usr/adm                                *
 *      # chmod 4755 /etc/lastlogin                                        *
 *                                                                         *
 *      Place a call to /etc/lastlogin in your /etc/localprofile           *
 *      to be run on all user logins.                                      *
 ***************************************************************************/
/***************************************************************************
 *      Linux compiling instructions:                                      *
 *                                                                         *
 *      $ gcc -o lastlogin lastlogin.c utmp2.o                             *
 *      utmp2.o is compiled from poe-IGL (1.2)                             *
 *      $ mv lastlogin /etc                                                *
 *      $ su                                                               *
 *      Password:                                                          *
 *      # chown adm /etc/lastlogin /usr/adm                                *
 *      # chgrp adm /etc/lastlogin /usr/adm                                *
 *      # chmod 4755 /etc/lastlogin                                        *
 *                                                                         *
 *      Place a call to /etc/lastlogin in your /etc/profile                *
 *      to be run on all user logins.                                      *
 *                                                                         *
 *                                  B.Bergt@informatik.tu-chemnitz.de      *
 ***************************************************************************/

              /* Print the last login time and record the new time */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <utmp.h>

#define	LOGFILE	"/usr/adm/lastlog"

main()
{
	struct utmp *utent, *getutent();
	int    fd;
	long   hrs, min, sec;
	struct lastlog {
	   char ll_line[8];
	   time_t ll_time;
	} ll;

	if (access(LOGFILE, 0) == -1) {
	   if ((fd = creat(LOGFILE,0644)) == -1) {
		fprintf(stderr,"Cannot create file %s: ", LOGFILE);
		perror("creat()");
		exit(1);
	   }
	} else {
	   if ((fd = open(LOGFILE,O_RDWR)) == -1) {
		fprintf(stderr,"Cannot open file %s: ", LOGFILE);
		perror("open()");
		exit(1);
	   }
	}

	if (lseek(fd, (long)(getuid()*sizeof(struct lastlog)), 0) == -1) {
		fprintf(stderr,"Cannot position file %s: ", LOGFILE);
		perror("lseek()");
		exit(1);
	}

	if (read(fd, (char *) &ll, sizeof ll) == sizeof ll &&
	    ll.ll_time != 0L) {
		printf("Last login: %.*s on %.*s\n" , 19
			, (char *) ctime(&ll.ll_time) , sizeof(ll.ll_line)
			, ll.ll_line);
	} else  printf("Last login: [No Login information on record]\n");

	sprintf(ll.ll_line, "%.8s", strrchr(ttyname(0), '/')+1);
	setutent();
	while ((utent = getutent()) != NULL) 
	   if (strcmp(utent->ut_line, ll.ll_line) == 0)
		break;

	if (utent == NULL) {
		fprintf(stderr,"Cannot locate utmp entry for tty\n");
		exit(1);
	}
	ll.ll_time = utent->ut_time;
	endutent();

	lseek(fd, (long)(getuid()*sizeof(struct lastlog)), 0);
	write(fd, (char *) &ll, sizeof ll);
	close(fd);

	exit(0);
}
