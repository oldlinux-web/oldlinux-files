/* leave - tell the user when to go home	Author: Terrence W. Holm */

/* Usage:	leave [ [+] hhmm ]
 *
 * Author:	Terrence W. Holm
 *
 * Revision:
 *		Fred van Kempen, MINIX User Group Holland
 *		 -adapted to MSS
 *		 -adapted to new utmp database
 *		 -adapted to POSIX (MINIX 1.5)
 */

#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <utmp.h>
#include <stdio.h>


#define Min(a,b)  ((a<b) ? a : b)

#define STRING	   80		/* lots of room for an argument */
#define MIN	   60L		/* seconds per minute */
#define HOUR      (60L*60L)	/* seconds per hour */
#define HALF_DAY  (12L*HOUR)	/* seconds per half day */
#define DAY	   (24L*HOUR)	/* seconds per day */

/* Set the following to your personal preferences for the
 * time and contents of warnings.
 */
#define INTERVALS 13		/* size of intervals[] */
#define WARNINGS  4		/* size of warnings[] */


static char *Version = "@(#) LEAVE 1.4 (01/09/90)";
static int intervals[INTERVALS] = {
  -5 * MIN,
  -1 * MIN,
  0,
  MIN,
  2 * MIN,
  3 * MIN,
  4 * MIN,
  5 * MIN,
  6 * MIN,
  7 * MIN,
  8 * MIN,
  9 * MIN,
  10 * MIN
};
static char *warnings[WARNINGS] = {
  "You have to leave within 5 minutes",
  "Just one more minute!",
  "Time to leave!",
  "You're going to be late!"	/* for all subsequent warnings */
};


#ifdef _BSD
long timezone;
#else
extern long timezone;
#endif


extern char *ttyname();
extern char *cuserid();


void Usage()
{
   fprintf(stderr, "Usage: leave [[+]hhmm]\n");
  exit(1);
}


void Get_Hour_Min(when, hour, min)
char *when;
int *hour;
int *min;
{
  int hour_min;
  int just_min = 0;

  switch (sscanf(when, "%d:%d", &hour_min, &just_min)) {
      case 1:
	*hour = hour_min / 100;
	*min = hour_min % 100;
	break;
      case 2:
	*hour = hour_min;
	*min = just_min;
	break;
      default:
	Usage();
  }

  if (hour_min < 0 || just_min < 0 || *min > 59) Usage();
}


int Still_Logged_On(user, tty)
char *user;
char *tty;
{
  FILE *f;
  struct utmp login;

  if ((f = fopen(UTMP, "r")) == (FILE *) NULL)
	/* no login/logout records kept */
	return(1);

  while (fread(&login, sizeof(struct utmp), 1, f) == 1) {
	if (!strncmp(login.ut_line, tty, 8))
		if (!strncmp(login.ut_name, user, 8)) {
			fclose(f);
			return(1);
		} else {
			fclose(f);
			return(0);
		}
  }
  fclose(f);
  return(0);
}


main(argc, argv)
int argc;
char *argv[];
{
  char when[STRING];
  long now = time((time_t *)0);
  long leave, delta;
  int hour, min;
  int pid, i;
  char *user = cuserid( (char *) NULL);
  char *tty = ttyname(0) + 5;

  /* get the argument string "when" either from stdin, or argv */
  if (argc <= 1) {
	printf("When do you have to leave? ");
	if (fgets(when, STRING, stdin) == NULL || when[0] == '\n') exit(0);
  } else {
	strcpy(when, argv[1]);
	if (argc > 2) strcat(when, argv[2]);
  }

  /* determine the leave time from the current time and "when" */
  if (when[0] == '+') {
	Get_Hour_Min(&when[1], &hour, &min);
	leave = now + hour * HOUR + min * MIN;
  } else {
	/* user entered an absolute time */
#ifdef _BSD
	timezone = -localtime(&now)->tm_gmtoff;
#endif
	Get_Hour_Min(&when[0], &hour, &min);
	if (hour >= 1 && hour <= 12) {
		/* 12-hour format: relative to previous midnight or noon */
		leave = now - (now - timezone) % HALF_DAY +
			hour % 12 * HOUR + min * MIN;
		if (leave < now - HOUR)
			leave = leave + HALF_DAY;
		else if (leave < now) {
			printf("That time has already passed!\n");
			exit(1);
		}
	} else if (hour <= 24) {
		/* 24-hour format: relative to previous midnight */
		leave = now - (now - timezone) % DAY +
			hour * HOUR + min * MIN;
		if (leave < now - HOUR)
			leave = leave + DAY;
		else if (leave < now) {
			printf("That time has already passed!\n");
			exit(1);
		}
	} else
		Usage();
  }

  printf("Alarm set for %s", ctime(&leave));

  if ((pid = fork()) == -1) {
	fprintf(stderr, "leave: can not fork\n");
	exit(1);
  }
  if (pid != 0) exit(0);

  /* only the child continues on */
  if (user == NULL || tty == NULL) {
	fprintf(stderr, "leave: Can not determine user and terminal name\n");
	exit(1);
  }
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);

  for (;;) {
	if (!Still_Logged_On(user, tty)) exit(0);

	/* how much longer until the leave time? */
	delta = leave - time((time_t *)0);

	/* which interval are we currently in? */
	for (i = 0; i < INTERVALS; ++i)
		if (delta + intervals[i] > 0) break;

	/* if we are within intervals[0] then print a warning If
	 * there are more intervals than messages, then use/
	 * warnings[WARNINGS-1] for all subsequent messages. */
	if (i > 0)
		printf("\007%s\n", warnings[i > WARNINGS ? WARNINGS - 1 : i - 1]);

	if (i == INTERVALS) {
		printf("That was the last time I'll tell you. Bye.\n");
		exit(0);
	}
	/* Sleep until the next interval. For long periods, wake up
	 * every hour to check if the user is still on (also required
	 * because 16 bit ints don't allow long waits). */
	sleep((int) Min(delta + intervals[i], HOUR));
  }
}
