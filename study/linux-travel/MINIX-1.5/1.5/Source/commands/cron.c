/* Cron - clock daemon			Author: S.R. Sampson */

/*	Cron is the clock daemon.  It is typically started up from the
 *	/etc/rc file by the line:
 *		/usr/bin/cron
 *	Cron automatically puts itself in the background, so no & is needed.
 *	If cron is used, it runs all day, spending most of its time asleep.
 *	Once a minute it wakes up and examines /usr/lib/crontab to see if there
 *	are any commands to be executed.  The format of this table is the same
 *	as in UNIX, except that % is not allowed to indicate 'new line.'
 *
 *	Each crontab entry has six fields:
 *	   minute    hour  day-of-the-month  month  day-of-the-week  command
 *	Each entry is checked in turn, and any entry matching the current time
 *	is executed.  The entry * matches anything.  Some examples:
 *
 *   min hr dat mo day   command
 *    *  *   *  *   *    /usr/bin/date >/dev/tty0   #print date every minute
 *    0  *   *  *   *    /usr/bin/date >/dev/tty0   #print date on the hour
 *   30  4   *  *  1-5   /bin/backup /dev/fd1       #do backup Mon-Fri at 0430
 *   30 19   *  *  1,3,5 /etc/backup /dev/fd1       #Mon, Wed, Fri at 1930
 *    0  9  25 12   *    /usr/bin/sing >/dev/tty0   #Xmas morning at 0900 only
 *
 * Version 1.6  SrT  90/04/08
 *      Added casting fixes by Ralf Wenk, and integrated net
 *      changes that release current directory, and use the
 *      1.5.5 include files.  Altered assign, so no temporary
 *	buffer is needed.
 *
 * Version 1.5  SrT  89/04/10
 *	Changed sleep code, to type SRS sent me.
 *
 * Version 1.4  SrT  89/03/17
 *	Fixed a pointer problem, when reloading crontab.
 *
 * Version 1.3  SrT  89/03/16
 *	Loads crontab, into memory and only rereads the disk
 *	version if it changes.  (Free up those clock cycles!)
 *
 * Fixed 03/10/89, by Simmule Turner, simmy@nu.cs.fsu.edu
 *	Now correctly cleans up zombie processes
 *	Logs actions to /usr/adm/cronlog
 *	Syncs with clock after each minute
 *	Comments allowed in crontab
 *	Fixed bug that prevented month, from matching
*/


#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#ifndef DEBUG
#define CRONTAB "/usr/lib/crontab"
#define LOGFILE "/usr/adm/cronlog"
#else
#define LOGFILE "/usr/adm/cronlog.dbg"
#define CRONTAB "/usr/adm/crontab.dbg"
#endif

#define NULLDEV "/dev/null"
#define SEPARATOR " \t"
#define CRONSIZE  2048
#define CRONSTRUCT struct cron_entry

#define	TRUE	1
#define	FALSE	0

struct cron_entry {
  char *mn;
  char *hr;
  char *day;
  char *mon;
  char *wkd;
  char *cmd;
  struct cron_entry *next;
} *head, *entry_ptr;

char crontab[CRONSIZE];
FILE *cronlog;

int wakeup(), ret();

time_t previous_time = 0L;

main()
{
  int status;
  time_t clock;

  status = fork();
  if (status == -1) {
	fprintf(stderr, "Can't fork cron\n");
	exit(1);
  }

  if (status > 0) exit(0);

  signal(SIGINT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);

  close(0);
  close(1);
  close(2);

  chdir("/");

  open(NULLDEV, O_RDONLY);
  if ((cronlog = fopen(LOGFILE, "a")) == (FILE *) NULL) {
	open(NULLDEV, O_WRONLY);
	open(NULLDEV, O_WRONLY);
  } else {
	setbuf(cronlog, (char *) NULL);
	dup(fileno(cronlog));
  }

  entry_ptr = (CRONSTRUCT *) malloc(sizeof(CRONSTRUCT));
  entry_ptr->next = (CRONSTRUCT *) NULL;
  head = entry_ptr;

  while (TRUE) {
	signal(SIGALRM, wakeup);
	time(&clock);
	alarm((unsigned) (60 - clock % 60));
	pause();

	signal(SIGALRM, ret);
	alarm(1);
	while (wait((int *) NULL) != (-1));
  }
}

int ret() {}

wakeup()
{
  register struct tm *tm;
  time_t cur_time;
  CRONSTRUCT *this_entry = head;

  load_crontab();

  time(&cur_time);
  tm = localtime(&cur_time);

  while (this_entry->next && this_entry->mn) {
	if (match(this_entry->mn, tm->tm_min) &&
	    match(this_entry->hr, tm->tm_hour) &&
	    match(this_entry->day, tm->tm_mday) &&
	    match(this_entry->mon, tm->tm_mon + 1) &&
	    match(this_entry->wkd, tm->tm_wday)) {
		fprintf(cronlog, "%02d/%02d-%02d:%02d  %s\n",
			tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
			tm->tm_min, this_entry->cmd);
		if (fork() == 0) {
			execl("/bin/sh", "/bin/sh", "-c",
	                      this_entry->cmd, (char *) 0);
			exit(1);
		}
	}
	this_entry = this_entry->next;
  }
}

/*
 *	This routine will match the left string with the right number.
 *
 *	The string can contain the following syntax *
 *	*		This will return TRUE for any number
 *	x,y [,z, ...]	This will return TRUE for any number given.
 *	x-y		This will return TRUE for any number within
 *			the range of x thru y.
 */

match(left, right)
register char *left;
register int right;
{
  register int n;
  register char c;

  n = 0;
  if (!strcmp(left, "*")) return(TRUE);

  while ((c = *left++) && (c >= '0') && (c <= '9')) n = (n * 10) + c - '0';

  switch (c) {
      case '\0':
	return(right == n);

      case ',':
	if (right == n) return(TRUE);
	do {
		n = 0;
		while ((c = *left++) && (c >= '0') && (c <= '9'))
			n = (n * 10) + c - '0';

		if (right == n) return(TRUE);
	} while (c == ',');
	return(FALSE);

      case '-':
	if (right < n) return(FALSE);

	n = 0;
	while ((c = *left++) && (c >= '0') && (c <= '9'))
		n = (n * 10) + c - '0';

	return(right <= n);
  }
}

load_crontab()
{
  int pos = 0;
  FILE *cfp;
  struct stat buf;

  if (stat(CRONTAB, &buf)) {
	if (previous_time == 0L) printf("Can't stat crontab\n");
	previous_time = 0L;
	return;
  }
#ifdef DEBUG
  printf("Crontab Time:%ld In_Core:%ld\n", buf.st_mtime, previous_time);
#endif

  if (buf.st_mtime <= previous_time) return;

  if ((cfp = fopen(CRONTAB, "r")) == (FILE *) NULL) {
	if (previous_time == 0L) printf("Can't open crontab\n");
	previous_time = 0L;
	return;
  }
  previous_time = buf.st_mtime;

  entry_ptr = head;
  while (fgets(&crontab[pos], CRONSIZE - pos, cfp) != (char *) NULL) {
	int len;

	if (crontab[pos] == '#') continue;
	len = strlen(&crontab[pos]);
	if (crontab[pos + len - 1] == '\n') {
		len--;
		crontab[pos + len] = '\0';
	}
	assign(entry_ptr, &crontab[pos]);
	if (entry_ptr->next == (CRONSTRUCT *) NULL) {
		entry_ptr->next = (CRONSTRUCT *) malloc(sizeof(CRONSTRUCT));
		entry_ptr->next->next = (CRONSTRUCT *) NULL;
	}
	entry_ptr = entry_ptr->next;
	pos += ++len;
	if (pos >= CRONSIZE) break;
  }
  fclose(cfp);

  while (entry_ptr) {
	entry_ptr->mn = (char *) NULL;
	entry_ptr = entry_ptr->next;
  }

#ifdef DEBUG
  printf("Crontab uses %d/%d bytes\n", pos, CRONSIZE);
  {
	CRONSTRUCT *start = head;
	dumptable(start);
  }
#endif
}

assign(entry, line)
CRONSTRUCT *entry;
char *line;
{
  entry->mn = strtok(line, SEPARATOR);
  entry->hr = strtok((char *) NULL, SEPARATOR);
  entry->day = strtok((char *) NULL, SEPARATOR);
  entry->mon = strtok((char *) NULL, SEPARATOR);
  entry->wkd = strtok((char *) NULL, SEPARATOR);
  entry->cmd = strchr(entry->wkd,'\0') + 1;
}

#ifdef DEBUG
dumptable(table)
CRONSTRUCT *table;
{
  time_t clock;
  time(&clock);

  printf("\nContents of crontab at: %s", ctime(&clock));
  printf("Minute\tHour\tDay\tMonth\tWeekday\tCommand\n");
  while (table->next && table->mn) {
	printf("%s\t%s\t%s\t%s\t%s\t%s\n",
	       table->mn, table->hr, table->day, table->mon,
	       table->wkd, table->cmd);
	table = table->next;
  }
}
#endif
