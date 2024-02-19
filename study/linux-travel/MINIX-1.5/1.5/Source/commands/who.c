/* who - see who is logged in			Author: Terrence W. Holm */

/*
 *		The user log-in name, terminal port and log-in time
 *		are displayed for all current users, or restricted
 *		to the specified <USER>, <DEVICE> or the current user.
 *
 * Usage:	who
 *		who <USER>
 *		who <DEVICE>
 *		who am i
 *
 * Version:	1.5	01/09/90
 *
 * Author:	Terrence W. Holm	June 1988
 *		revised for UTMP use	Feb 1989
 *
 *		Fred van Kempen, October 1989
 *		Fred van Kempen, December 1989
 *		Fred van Kempen, January 1990
 */

#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <utmp.h>
#include <stdio.h>

static char *Version = "@(#) WHO 1.5 (01/09/90)";

extern char *ttyname();

void usage()
{
  fprintf(stderr, "Usage: who [USER | DEVICE | am i]\n");
  exit(-1);
}


main(argc, argv)
int argc;
char *argv[];
{
  struct utmp entry;
  struct tm *tm;
  char *fmt = "%02.2d:%02.2d:%02.2d";
  char logstr[16], actstr[16];
  char *arg, *sp;
  long login, active;
  int fd, size, found;

  switch (argc) {
	case 1:	
		arg = NULL;	break;
	case 2:	
		arg = argv[1];	break;
	case 3:
		if (!strcmp(argv[1], "am") && (!strcmp(argv[2], "i") ||
				       !strcmp(argv[2], "I")))
			arg = ttyname(0) + 5;
		else
			usage();
		break;
	default:
		usage();
  }

  size = sizeof(struct utmp);
  found = 0;
  if ((fd = open(UTMP, O_RDONLY)) < 0) {
	fprintf(stderr, "%s: user-accouting is not active.\n", argv[0]);
	exit(0);
  }
  while (read(fd, &entry, size) == size) {
	if (entry.ut_type == USER_PROCESS) {
		if (found == 0) {
			found++;
			printf("USER     LINE       TIME     ACTIVE   PID\n");
		}
		login = entry.ut_time;
		tm = localtime(&login);
		sprintf(logstr, fmt, tm->tm_hour, tm->tm_min, tm->tm_sec);
		time(&active);
		active -= login;
		tm = localtime(&active);
		sprintf(actstr, fmt, tm->tm_hour, tm->tm_min, tm->tm_sec);
		printf("%-8.8s %-8.8s %-8.8s  %-8.8s  %d\n",
		       entry.ut_name, entry.ut_line, logstr,
		       actstr, entry.ut_pid);
	}
  }
  close(fd);
  if (found == 0) printf("No active users.\n");

  exit(0);
}
