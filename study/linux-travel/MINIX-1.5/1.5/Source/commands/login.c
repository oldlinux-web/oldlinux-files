/* login - log into the system			Author: Patric van Kleef */

/* Original version by Patrick van Kleef.  History of modifications:
 *
 * Peter S. Housel   Jan. 1988
 *  - Set up $USER, $HOME and $TERM.
 *  - Set signals to SIG_DFL.
 *
 * Terrence W. Holm   June 1988
 *  - Allow a username as an optional argument.
 *  - Time out if a password is not typed within 30 seconds.
 *  - Perform a dummy delay after a bad username is entered.
 *  - Don't allow a login if "/etc/nologin" exists.
 *  - Cause a failure on bad "pw_shell" fields.
 *  - Record the login in "/usr/adm/wtmp".
 *
 * Peter S. Housel   Dec. 1988
 *  - Record the login in "/etc/utmp" also.
 *
 * F. van Kempen     June 1989
 *  - various patches for Minix V1.4a.
 *
 * F. van Kempen     September 1989
 *  - added login-failure administration (new utmp.h needed!).
 *  - support arguments in pw_shell field
 *  - adapted source text to MINIX Style Sheet
 *
 * F. van Kempen     October 1989
 *  - adapted to new utmp database.
 * F. van Kempen,    December 1989
 *  - fixed 'slot' assumption in wtmp()
 *  - fixed all MSS-stuff
 *  - adapted to POSIX (MINIX 1.5)
 * F. van Kempen,    January 1990
 *  - made all 'bad login accounting' optional by "#ifdef BADLOG".
 * F. van Kempen,    Februari 1990
 *  - fixed 'first argument' bug and added some casts.
 *
 * Andy Tanenbaum April 1990
 * - if /bin/sh cannot be located, try /usr/bin/sh
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sgtty.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <utmp.h>
#include <stdio.h>

#define DIGIT 			      3
#define MOTD 		    "/etc/motd"
#define TTY  		         "tty?"
#define CONS 		         "tty0"

static char *Version = "@(#) LOGIN 1.13 (02/10/90)";
int time_out;
char user[32];
char logname[35];
char home[64];
char shell[64];
char *env[] = {
  user,
  logname,
  home,
  shell,
  "TERM=minix",
  NULL
};

extern char *crypt();
extern struct passwd *getpwnam();
extern long time();
extern long lseek();
void Time_out();

void wtmp(line, user)
char *line;			/* tty device name */
char *user;			/* user name */
{
  /* Make entries in /usr/adm/wtmp and /etc/utmp. */
  struct utmp entry, oldent;
  char *blank = "               ";
  register int fd;
  register char *sp;
  int lineno;
  extern long time();

  /* Strip off the /dev part of the TTY name. */
  sp = strrchr(line, '/');
  if (sp == NULL)
	sp = line;
  else
	sp++;

  /* First, read the current UTMP entry. we need some of its
   * parameters! (like PID, ID etc...). */
  if ((fd = open(UTMP, O_RDONLY)) < 0) return;
  lineno = 0;
  while (read(fd, (char *) &oldent, sizeof(struct utmp))
					== sizeof(struct utmp)) {
	if (oldent.ut_pid == getpid()) break;
	lineno++;
  }
  lineno *= sizeof(struct utmp);

  if (lseek(fd, (long) lineno, SEEK_SET) >= 0) {
	read(fd, (char *) &oldent, sizeof(struct utmp));
  }
  close(fd);

  /* Clear out the new string fields. */
  strncpy(entry.ut_user, blank, sizeof(entry.ut_user));
  strncpy(entry.ut_id, blank, sizeof(entry.ut_id));
  strncpy(entry.ut_line, blank, sizeof(entry.ut_line));

  /* Enter new string fields. */
  strncpy(entry.ut_user, user, sizeof(entry.ut_user));
  strncpy(entry.ut_id, oldent.ut_id, sizeof(entry.ut_id));
  strncpy(entry.ut_line, sp, sizeof(entry.ut_line));

  /* Copy old numeric fields. */
  entry.ut_pid = oldent.ut_pid;

  /* Change new numeric fields. */
  entry.ut_type = USER_PROCESS;	/* we are past login... */
  time(&(entry.ut_time));

  /* Write a WTMP record. */
  if ((fd = open(WTMP, O_WRONLY)) > 0) {
	if (lseek(fd, 0L, SEEK_END) >= 0L) {
		write(fd, (char *) &entry, sizeof(struct utmp));
	}
	close(fd);
  }
  /* Rewrite the UTMP entry. */
  if ((fd = open(UTMP, O_WRONLY)) > 0) {
	if (lseek(fd, (long) lineno, SEEK_SET) >= 0) {
		write(fd, (char *) &entry, sizeof(struct utmp));
	}
	close(fd);
  }
}


#ifdef BADLOG
void addlog(nam, pwd, tty)
char *nam;			/* name of attempting user */
char *pwd;			/* attempted password */
char *tty;			/* name of terminal line */
{
/* Register a failed login if the logfile exists. */
  struct utmp entry;
  int fd;

  strncpy(entry.ut_name, nam, sizeof(entry.ut_name));
  strncpy(entry.ut_line, tty, sizeof(entry.ut_line));
  entry.ut_time = time((time_t *)0);

  if ((fd = open(BTMP, O_WRONLY)) < 0) return;

  if (lseek(fd, 0L, SEEK_END) >= 0L) {
	/* Append the entry to the btmp file. */
	write(fd, (char *) &entry, sizeof(struct utmp));
  }
  close(fd);
}
#endif /* BADLOG */


void show_file(nam)
char *nam;
{
/* Read a textfile and show it on the desired terminal. */
  register int fd, len;
  char buf[80];

  if ((fd = open(nam, O_RDONLY)) > 0) {
	len = 1;
	while (len > 0) {
		len = read(fd, buf, 80);
		write(1, buf, len);
	}
	close(fd);
  }
}


int main(argc, argv)
int argc;
char *argv[];
{
  char name[30];
  char password[30];
  char ttyname[16];
  int bad, n, ttynr, ap;
  struct sgttyb args;
  struct passwd *pwd;
  struct stat statbuf;
  char *bp, *argx[8];		/* pw_shell arguments */
  char *sh = "/bin/sh";		/* sh/pw_shell field value */
  char *sh2= "/usr/bin/sh";	/* other possibility */

  /* Reset some of the line parameters in case they have been mashed. */
  if (ioctl(0, TIOCGETP, &args) < 0) exit(1);
#ifdef NOGETTY
  args.sg_kill = '\030';	/* CTRL-X */
  args.sg_erase = '\b';
  args.sg_flags |= (XTABS | CRMOD | ECHO);
  ioctl(0, TIOCSETP, &args);
#endif /* NOGETTY */

  /* Look up /dev/tty number. */
  fstat(0, &statbuf);
  ttynr = statbuf.st_rdev & 0377;
  if (ttynr == 0)
	strcpy(ttyname, CONS);	/* system console */
  else {
	strcpy(ttyname, TTY);
	ttyname[DIGIT] = '0' + ttynr;
  }

  /* Get login name and passwd. */
  for (;;) {
	bad = 0;

	if (argc > 1) {
		strcpy(name, argv[1]);
		argc = 1;
	} else {
		do {
			write(1, "login: ", 7);
			n = read(0, name, 30);
		} while (n < 2);
		name[n - 1] = 0;
	}

	/* Look up login/passwd. */
	if ((pwd = getpwnam(name)) == (struct passwd *) NULL) bad++;

	/* If login name wrong or password exists, ask for pw. */
	if (bad || strlen(pwd->pw_passwd) != 0) {
		args.sg_flags &= ~ECHO;
		ioctl(0, TIOCSETP, &args);
		write(1, "Password: ", 10);

		time_out = 0;
		signal(SIGALRM, Time_out);
		alarm(30);

		n = read(0, password, 30);

		alarm(0);
		if (time_out) {
			n = 1;
			bad++;
		}
		password[n - 1] = 0;
		write(1, "\n", 1);
		args.sg_flags |= ECHO;
		ioctl(0, TIOCSETP, &args);

		if (bad && crypt(password, "aaaa") ||
		    strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd))) {
#ifdef BADLOG
			addlog(name, password, ttyname);
#endif /* BADLOG */
			write(1, "Login incorrect\n", 16);
			continue;
		}
	}
	/* Check if the system is going down  */
	if (access("/etc/nologin", 0) == 0 && strcmp(name, "root") != 0) {
		write(1, "System going down\n\n", 19);
		continue;
	}
	/* Write login record to /usr/adm/wtmp and /etc/utmp */
	wtmp(ttyname, name);

	/* Create the argv[] array from the pw_shell field. */
	ap = 0;
	argx[ap++] = "-";	/* most shells need it for their .profile */
	if (pwd->pw_shell[0]) {
		sh = pwd->pw_shell;
		bp = sh;
		while (*bp) {
			while (*bp && *bp != ' ' && *bp != '\t') bp++;
			if (*bp == ' ' || *bp == '\t') {
				*bp++ = '\0';	/* mark end of string */
				argx[ap++] = bp;
			}
		}
	} else
	argx[ap] = (char *) NULL;

	/* Set the environment */
	strcpy(user, "USER=");
	strcat(user, name);
	strcpy(logname, "LOGNAME=");
	strcat(logname, name);
	strcpy(home, "HOME=");
	strcat(home, pwd->pw_dir);
	strcpy(shell, "SHELL=");
	strcat(shell, sh);

	chdir(pwd->pw_dir);

	/* Reset signals to default values. */
	for (n = 1; n <= _NSIG; ++n) signal(n, SIG_DFL);

	/* Show the message-of-the-day. */
	show_file(MOTD);

	/* Assign the terminal to this user. */
	strcpy(name, "/dev/");
	strcat(name, ttyname);
	chown(name, pwd->pw_uid, pwd->pw_gid);

	setgid(pwd->pw_gid);
	setuid(pwd->pw_uid);
	execve(sh, argx, env);
	execve(sh2, argx, env);		/* if /bin/sh absent, try /usr/bin/sh*/

	write(1, "exec failure\n", 13);
	exit(1);
  }
}


void Time_out()
{
   time_out = 1;
}


