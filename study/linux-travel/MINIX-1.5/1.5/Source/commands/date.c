/* date - print or set time and date		Author: Jan Looyen */

#include <sys/types.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>

#define	MIN	60L		/* # seconds in a minute */
#define	HOUR	(60 * MIN)	/* # seconds in an hour */
#define	DAY	(24 * HOUR)	/* # seconds in a day */
#define	YEAR	(365 * DAY)	/* # seconds in a year */

main(argc, argv)
int argc;
char **argv;
{
  int qflag;
  long t;
  char time_buf[15];

  if (argc > 2) usage();
  if (argc == 2) {
	if (*argv[1] == '-' && (argv[1][1] | 0x60) == 'q') {
		freopen("/dev/tty0", "r", stdin);
		printf("\nPlease enter date: MMDDYYhhmmss. Then hit the RETURN key.\n");
		gets(time_buf);
		set_time(time_buf);
	} else
		set_time(argv[1]);
  }
  time(&t);
  printf("%s", ctime(&t));
  exit(0);
}


set_time(t)
char *t;
{
  char *tp;
  long ct;
  int len;
  static int days_per_month[] = {
		      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
  };
  struct tm *p;

  time(&ct);
  p = localtime(&ct);
  p->tm_year -= 70;
  p->tm_mon++;
  len = strlen(t);
  if (len != 12 && len != 10 && len != 6 && len != 4) usage();
  tp = t;
  while (*tp)
	if (!isdigit(*tp++)) bad();
  if (len == 6 || len == 12) p->tm_sec = conv(&tp, 59);
  p->tm_min = conv(&tp, 59);
  p->tm_hour = conv(&tp, 23);
  if (len == 12 || len == 10) {
	p->tm_year = conv(&tp, 99);
	p->tm_mday = conv(&tp, 31);
	p->tm_mon = conv(&tp, 12);
	p->tm_year -= 70;
	if (p->tm_year < 0) p->tm_year += 100;
  }
  ct = p->tm_year * YEAR;
  ct += ((p->tm_year + 1) / 4) * DAY;
  days_per_month[1] = 28;
  if (((p->tm_year + 2) % 4) == 0) days_per_month[1]++;
  len = 0;
  p->tm_mon--;
  while (len < p->tm_mon) ct += days_per_month[len++] * DAY;
  ct += --p->tm_mday * DAY;
  ct += p->tm_hour * HOUR;
  ct += p->tm_min * MIN;
  ct += p->tm_sec;
  if (stime(&ct)) fprintf(stderr, "Set date not allowed\n");
}

conv(ptr, max)
char **ptr;
int max;
{
  int buf;

  *ptr -= 2;
  buf = atoi(*ptr);
  **ptr = 0;
  if (buf < 0 || buf > max) bad();
  return(buf);
}

bad()
{
  fprintf(stderr, "Date: bad conversion\n");
  exit(1);
}

usage()
{
  fprintf(stderr, "Usage: date [-q] [[MMDDYY]hhmm[ss]]\n");
  exit(1);
}

isdigit(c)
char c;
{
  if (c >= '0' && c <= '9')
	return(1);
  else
	return(0);
}
