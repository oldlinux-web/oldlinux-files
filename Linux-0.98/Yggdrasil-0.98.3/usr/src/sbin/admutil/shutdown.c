/* shutdown.c by poe@daimi.aau.dk */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <utmp.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/param.h>
#include <termios.h>
/* #include <stdlib.h> */
#include "pathnames.h"

char	*prog;		/* name of the program */
int	opt_reboot;	/* true if -r option or reboot command */
int	timeout;	/* number of seconds to shutdown */
int	opt_quiet;	/* true if no message is wanted */
char	message[90];	/* reason for shutdown if any... */

/* #define DEBUGGING */

#define WR(s) write(fd, s, strlen(s))

usage()
{
	fprintf(stderr, "Usage: shutdown [-h|-r] [-q] [now|hh:ss|+mins]\n");
	exit(0);
}

void int_handler()
{
	unlink(_PATH_NOLOGIN);
	signal(SIGINT, SIG_DFL);
	puts("Shutdown process aborted\n");
	exit(1);
}

main(argc, argv)
	int argc;
	char *argv[];
{
	int c;	
	int fd;
	char *ptr;
	
#ifndef DEBUGGING
	if(geteuid()) {
		fprintf(stderr, "Only root can shut a system down.\n");
		exit(1);
	}
#endif

	prog = argv[0];
	if(ptr = strrchr(argv[0], '/')) prog = ++ptr;
	
	if(!strcmp("halt", prog)) {
		opt_reboot = 0;
		opt_quiet = 1;
		timeout = 0;
	} else if(!strcmp("reboot", prog)) {
		opt_reboot = 1;
		opt_quiet = 1;
		timeout = 0;
	} else {
		/* defaults */
		opt_reboot = 0;
		opt_quiet = 0;
		timeout = 2*60;
		
		c = 0;
		while(++c < argc) {
			if(argv[c][0] == '-') {
				switch(argv[c][1]) {
				case 'h': 
					opt_reboot = 0;
					break;
				case 'r':
					opt_reboot = 1;
					break;
				case 'q':
					opt_quiet = 1;
					break;
				default:
					usage();
				}
			} else if(!strcmp("now", argv[c])) {
				timeout = 0;
			} else if(argv[c][0] == '+') {
				timeout = 60 * atoi(&argv[c][1]);
			} else {
				char *colon;
				int hour, minute;
				time_t tics;
				struct tm *tt;
				int now, then;
				
				if(colon = strchr(argv[c], ':')) {
					*colon = '\0';
					hour = atoi(argv[c]);
					minute = atoi(++colon);
				} else usage();
				
				(void) time(&tics);
				tt = localtime(&tics);
				
				now = 3600 * tt->tm_hour + 60 * tt->tm_min;
				then = 3600 * hour + 60 * minute;
				timeout = then - now;
				if(timeout < 0) timeout = 0;
			}
		}
	}

	if(!opt_quiet) {
		/* now ask for message, gets() is insecure */
		int cnt = sizeof(message)-1;
		char *ptr;
		
		printf("Why? "); fflush(stdout);
		
		ptr = message;
		while(--cnt >= 0 && (*ptr = getchar()) && *ptr != '\n') { 
			ptr++;
		}
		*ptr = '\0';
	} else
		strcpy(message, "for maintainance; bounce, bounce");

#ifdef DEBUGGING
	printf("timeout = %d, quiet = %d, reboot = %d\n",
		timeout, opt_quiet, opt_reboot);
#endif
	
	/* so much for option-processing, now begin termination... */
	
	signal(SIGINT, int_handler);

	chdir("/");

	if(timeout > 5*60) {
		sleep(timeout - 5*60);
		timeout = 5*60;
	}

	
	if((fd = open(_PATH_NOLOGIN, O_WRONLY|O_CREAT)) >= 0) {
		WR("\r\nThe system is being shut down\r\n");
		write(fd, message, strlen(message));
		WR("\r\nLogin is therefore prohibited.\r\n");
		close(fd);
	}
	
	signal(SIGPIPE, SIG_IGN);

	if(timeout > 0) {
		wall();
		sleep(timeout);
	}

	timeout = 0;
	wall();
	sleep(3);

	/* now there's no turning back... */
	signal(SIGINT, SIG_IGN);
	kill(1, SIGTSTP);	/* tell init not to spawn more getty's */
	write_wtmp();
	sync();

	if(fork() > 0) sleep(1000); /* the parent will die soon... */
	setpgrp();		/* so the shell wont kill us in the fall */

#ifndef DEBUGGING
	/* a gentle kill of all other processes except init */
	kill(-1, SIGINT);
	sleep(1);

	/* now use brute force... */
	kill(-1, SIGKILL);
#endif
	sync();
	sleep(1);

	/* unmount disks... */
	unmount_disks();
	sync();
	sleep(1);
	
	if(opt_reboot) {
		reboot(0xfee1dead, 672274793, 0x1234567);
	} else {
		printf("\nNow you can turn off the power...\n");
	}
}

/*** end of main() ***/

write_user(struct utmp *ut)
{
	FILE *f;
	int fd;
	int minutes;
	char term[40] = {'/','d','e','v','/',0};
	char msg[100];

	minutes = timeout / 60;
	(void) strncat(term, ut->ut_line, sizeof(ut->ut_line));

	/* try not to get stuck on a mangled ut_line entry... */
	if((fd = open(term, O_RDWR|O_NONBLOCK)) < 0)
	        return;

	sprintf(msg, "\007\r\nURGENT: message from the sysadmin:\r\n");
	WR(msg);

	if(minutes == 0) {
	  sprintf(msg, "System going down IMMEDIATELY!\r\n\n");
	} else {
	  sprintf(msg, "System going down in %d minute%s\r\n\n",
		  minutes, minutes == 1 ? "" : "s");
	}
	WR(msg);

	sprintf(msg, "\t... %s ...\r\n\n", message);
	WR(msg);

	close(fd);
}

wall()
{
	/* write to all users, that the system is going down. */
	struct utmp *ut;
		
	utmpname(_PATH_UTMP);
	setutent();
	
	while(ut = getutent()) {
		if(ut->ut_type == USER_PROCESS)
			write_user(ut);
	}
	endutent();
}

write_wtmp()
{
	/* write in wtmp that we are dying */
	int fd;
	struct utmp ut;
	
	memset((char *)&ut, 0, sizeof(ut));
	strcpy(ut.ut_line, "~");
	if(opt_reboot)
		ut.ut_name[0] = 0;
	else
		memcpy(ut.ut_name, "shutdown", sizeof(ut.ut_name));

	time(&ut.ut_time);
	ut.ut_type = BOOT_TIME;
	
	if((fd = open(_PATH_WTMP, O_WRONLY|O_APPEND)) > 0) {
		write(fd, (char *)&ut, sizeof(ut));
		close(fd);
	}
}

unmount_disks()
{
	/* unmount all disks (except root) */
	FILE *mtab;
	char line[80];
	char *p;
	
	sync();
	if(!(mtab = fopen("/etc/mtab", "r")))
		return;
	
	while(fgets(line, 79, mtab)) {
		p = line;
		while(!isspace(*p)) p++;
		*p = '\0';

#ifdef DEBUGGING
		printf("umount %s\n", line);
#else
		/* we rely on umount() to check for root filesystem */
		if(umount(line) < 0)
			printf("Couldn't umount %s\n", line);
#endif
	}
	fclose(mtab);
}
