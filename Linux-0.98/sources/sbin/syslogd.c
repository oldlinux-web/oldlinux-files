#ifndef lint
static char sccsid[] = "@(#)syslogd.c";
#endif not lint

/*
 *  syslogd -- log system messages
 *
 * This program implements a system log. It takes a series of lines.
 * Each line may have a priority, signified as "<n>" as
 * the first characters of the line.  If this is
 * not present, a default priority is used.
 *
 * To kill syslogd, send a signal 15 (terminate).  A signal 1 (hup) will
 * cause it to reread its configuration file.
 *
 * Defined Constants:
 *
 * MAXLINE -- the maximimum line length that can be handled.
 * NLOGS   -- the maximum number of simultaneous log files.
 * DEFUPRI -- the default priority for user messages
 * DEFSPRI -- the default priority for kernel messages
 *
 */

#define	NLOGS		20		/* max number of log files */
#define	MAXLINE		1024		/* maximum line length */
#define DEFUPRI		(LOG_USER|LOG_NOTICE)
#define DEFSPRI		(LOG_KERN|LOG_CRIT)
#define MARKCOUNT	10		/* ratio of minor to major marks */

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <utmp.h>
#include <ctype.h>
#include <signal.h>
#include <sysexits.h>
#include <string.h>

#include <sys/syslog.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/socket.h>

typedef int (*sighand)();

char	*LogName = "/dev/log";
char	*ConfFile = "/etc/syslog.conf";
char	*PidFile = "/etc/syslog.pid";
char	ctty[] = "/dev/tty0";

#define FDMASK(fd)	(1 << (fd))

/*
 *	defines for BSD compat....I think!
 */

#define index(a, b)	strchr(a, b)
#define bzero(a, b)	memset(a, '\0', b)


#define	dprintf		if (Debug) printf

#define UNAMESZ		8	/* length of a login name */
#define MAXUNAMES	20	/* maximum number of user names */
#define MAXFNAME	200	/* max file pathname length */

#define NOPRI		0x10	/* the "no priority" priority */
#define	LOG_MARK	(LOG_NFACILITIES << 3)	/* mark "facility" */

/*
 * Flags to logmsg().
 */

#define IGN_CONS	0x001	/* don't print on console */
#define SYNC_FILE	0x002	/* do fsync on file after printing */
#define NOCOPY		0x004	/* don't suppress duplicate messages */
#define ADDDATE		0x008	/* add a date to the message */
#define MARK		0x010	/* this message is a mark */

/*
 * This structure represents the files that will have log
 * copies printed.
 */

struct filed {
	short	f_type;			/* entry type, see below */
	short	f_file;			/* file descriptor */
	time_t	f_time;			/* time this was last written */
	u_char	f_pmask[LOG_NFACILITIES+1];	/* priority mask */
	union {
		char	f_uname[MAXUNAMES][UNAMESZ+1];
		char	f_fname[MAXFNAME];
	} f_un;
};

/* values for f_type */
#define F_UNUSED	0		/* unused entry */
#define F_FILE		1		/* regular file */
#define F_TTY		2		/* terminal */
#define F_CONSOLE	3		/* console terminal */
#define F_FORW		4		/* remote machine */
#define F_USERS		5		/* list of users */
#define F_WALL		6		/* everyone logged on */

/* how often (in seconds) to check for more input */
#define GRANULARITY 10
#define MSG_BSIZE 1024

char	*TypeNames[7] = {
	"UNUSED",	"FILE",		"TTY",		"CONSOLE",
	"FORW",		"USERS",	"WALL"
};

struct filed	Files[NLOGS];

int	Debug;			/* debug flag */
char	LocalHostName[MAXHOSTNAMELEN+1];	/* our hostname */
char	*LocalDomain;		/* our local domain name */
int	InetInuse = 0;		/* non-zero if INET sockets are being used */
int	LogPort;		/* port number for INET connections */
char	PrevLine[MAXLINE + 1];	/* copy of last line to supress repeats */
char	PrevHost[MAXHOSTNAMELEN+1];		/* previous host */
int	PrevFlags;
int	PrevPri;
int	PrevCount = 0;		/* number of times seen */
int	Initialized = 0;	/* set when we have initialized ourselves */
int	MarkInterval = 20;	/* interval between marks in minutes */
int	MarkSeq = 0;		/* mark sequence number */
int	Gran = 0;
static int mask = 0;
int count;
int readfds;

extern	int errno, sys_nerr;
extern	char *sys_errlist[];
extern	char *ctime();
/* extern  char *index(); */

main(argc, argv)
	int argc;
	char **argv;
{
	register int i;
	register char *p;
	static int funix;
	int len, nlen, ns, nfds;
	struct sockaddr afunix, fromunix;
	int fd;
	FILE *fp;
	char *lp, *lpb;
	char errbuf[100];
	char line[MSG_BSIZE + 1];
	extern int die(), domark();

	while (--argc > 0) {
		p = *++argv;
		if (p[0] != '-')
			usage();
		switch (p[1]) {
		case 'f':		/* configuration file */
			if (p[2] != '\0')
				ConfFile = &p[2];
			break;

		case 'd':		/* debug */
			Debug++;
			break;

		case 'p':		/* path */
			if (p[2] != '\0')
				LogName = &p[2];
			break;

		case 'm':		/* mark interval */
			if (p[2] != '\0')
				MarkInterval = atoi(&p[2]);
			break;

		default:
			usage();
		}
	}

	if (!Debug) {
		if (fork())
			exit(0);
		for (i = 0; i < 10; i++)
			 close(i);
		open("/", 0);
		dup2(0, 1);
		dup2(0, 2);
		untty();
	} else
#ifdef linux
	;
#else
		setlinebuf(stdout);
#endif
	gethostname(LocalHostName, sizeof LocalHostName);
	if (p = index(LocalHostName, '.')) {
		*p++ = '\0';
		LocalDomain = p;
	}
	else
		LocalDomain = "";
	 signal(SIGTERM, (void *) die);
	 signal(SIGINT, Debug ? (void *) die : SIG_IGN);
	 signal(SIGQUIT, Debug ? (void *) die : SIG_IGN);
	 unlink(LogName);
/*
 *	create system logfile LogName mode 0666
 *	logerror and die if can't
 */

	funix = socket(AF_UNIX, SOCK_STREAM, 0);
	if (funix < 0) {
		logerror("unable to create socket");
		die(0);
	}
	afunix.sa_family = AF_UNIX;
	strcpy(afunix.sa_data, LogName);
	if (bind(funix, &afunix, sizeof(afunix)) < 0) {
		logerror("can't bind");
		die(0);
	}
	chmod(LogName, 0666);

	/* tuck my process id away */
	fp = fopen(PidFile, "w");
	if (fp != NULL) {
		fprintf(fp, "%d\n", getpid());
		 fclose(fp);
	}

	dprintf("off & running....\n");

	init();
	signal(SIGHUP, (void *) init);
	signal(SIGALRM, (void *) domark);

	if (listen(funix, 5) < 0) {
		perror("listen");
		exit(1);
	}

	alarm(MarkInterval * 60 / MARKCOUNT);

	for (;;) {
		errno = 0;
/*
 *	Change this so it only reads one line at a time
 *	otherwise if more than BUFSIZ messages are read
 *	we'll get lost---also reading char at a time would
 *	help us catch messages that are being written while
 *	we run .... oh for an atomic flock
 */

		readfds = FDMASK(funix) | mask;
		errno = 0;
		nfds = select(20, (fd_set *) &readfds, (fd_set *) NULL,
		    (fd_set *) NULL, (struct timeval *) NULL);
		count++;
		if (nfds == 0)
			continue;
		if (nfds < 0) {
			if (errno != EINTR)
				perror("select");
			continue;
		}
		if (readfds & (FDMASK(funix) | mask)) {
			len = sizeof fromunix;
			if ((i = recvfrom(funix, line, MSG_BSIZE, 0,
			    (struct sockaddr *) &fromunix, &len)) > 0) {
				line[i]='\0';
				printline(LocalHostName, line);
			} else {
				if ((errno == EAGAIN) || (errno == 0)) continue;
				sprintf(errbuf, "recvfrom unix %d: %s", errno, strerror(errno));
				logerror(errbuf);
			}
		} else {
			if (errno != EINTR) {
				logerror("accept failure");
				die(0);
			}
		}
	}
}

usage()
{
	fprintf(stderr, "usage: syslogd [-d] [-mmarkinterval] [-ppath] [-fconffile]\n");
	exit(1);
}

untty()
{
	int i;

	if (!Debug) {
#ifdef linux
		setpgrp();
#endif
		i = open("/dev/tty", O_RDWR);
		if (i >= 0) {
#ifndef linux
			 ioctl(i, (int) TIOCNOTTY, (char *)0);
#endif linux
			 close(i);
		}
	}
}

/*
 * Take a raw input line, decode the message, and print the message
 * on the appropriate log files.
 */

printline(hname, msg)
	char *hname;
	char *msg;
{
	register char *p, *q;
	register int c;
	char line[MAXLINE + 1];
	int pri;

	/* test for special codes */
	pri = DEFUPRI;
	p = msg;
	if (*p == '<') {
		pri = 0;
		while (isdigit(*++p))
			pri = 10 * pri + (*p - '0');
		if (*p == '>')
			++p;
		if (pri <= 0 || pri >= (LOG_NFACILITIES << 3))
			pri = DEFUPRI;
	}

	/* don't allow users to log kernel messages */
	if ((pri & LOG_PRIMASK) == LOG_KERN)
		pri |= LOG_USER;

	q = line;

	while ((c = *p++ & 0177) != '\0' && c != '\n' &&
	    q < &line[sizeof(line) - 1]) {
		if (iscntrl(c)) {
			*q++ = '^';
			*q++ = c ^ 0100;
		} else
			*q++ = c;
	}
	*q = '\0';

	logmsg(pri, line, hname, 0);
}


/*
 * Log a message to the appropriate log files, users, etc. based on
 * the priority.
 */

logmsg(pri, msg, from, flags)
	int pri;
	char *msg, *from;
	int flags;
{
	register struct filed *f;
	register int l;
	int fac, prilev;
	time_t now;
	int omask;
	sighand s1, s2;
	struct iovec iov[6];
	register struct iovec *v = iov;
	char line[MAXLINE + 1];

	dprintf("logmsg: pri %o, flags %x, from %s, msg %s\n", pri, flags, from, msg);

	s1 = signal(SIGHUP, SIG_IGN);
	s2 = signal(SIGALRM, SIG_IGN);

	/*
	 * Check to see if msg looks non-standard.
	 */
	if (strlen(msg) < 16 || msg[3] != ' ' || msg[6] != ' ' ||
	    msg[9] != ':' || msg[12] != ':' || msg[15] != ' ')
		flags |= ADDDATE;

	if (!(flags & NOCOPY)) {
		if (flags & (ADDDATE|MARK))
			flushmsg();
		else if (!strcmp(msg + 16, PrevLine + 16)) {
			/* we found a match, update the time */
			 strncpy(PrevLine, msg, 15);
			PrevCount++;
			signal(SIGHUP, (void *) s1);
			signal(SIGALRM, (void *) s2);
			return;
		} else {
			/* new line, save it */
			flushmsg();
			 strcpy(PrevLine, msg);
			 strcpy(PrevHost, from);
			PrevFlags = flags;
			PrevPri = pri;
		}
	}

	 time(&now);
	if (flags & ADDDATE)
		v->iov_base = ctime(&now) + 4;
	else
		v->iov_base = msg;
	v->iov_len = 15;
	v++;
	v->iov_base = " ";
	v->iov_len = 1;
	v++;
	v->iov_base = from;
	v->iov_len = strlen(v->iov_base);
	v++;
	v->iov_base = " ";
	v->iov_len = 1;
	v++;
	if (flags & ADDDATE)
		v->iov_base = msg;
	else
		v->iov_base = msg + 16;
	v->iov_len = strlen(v->iov_base);
	v++;

	/* extract facility and priority level */
	fac = (pri & LOG_FACMASK) >> 3;
	if (flags & MARK)
		fac = LOG_NFACILITIES;
	prilev = pri & LOG_PRIMASK;

	/* log the message to the particular outputs */
	if (!Initialized) {
		int cfd = open(ctty, O_WRONLY);

		if (cfd >= 0) {
			v->iov_base = "\r\n";
			v->iov_len = 2;
			 writev(cfd, iov, 6);
			 close(cfd);
		}
		untty();
		signal(SIGHUP, (void *) s1);
		signal(SIGALRM, (void *) s2);
		return;
	}
	for (f = Files; f < &Files[NLOGS]; f++) {
		/* skip messages that are incorrect priority */
		if (f->f_pmask[fac] < prilev || f->f_pmask[fac] == NOPRI)
			continue;

		/* don't output marks to recently written files */
		if ((flags & MARK) && (now - f->f_time) < (MarkInterval * 60 / 2))
			continue;

		dprintf("Logging to %s", TypeNames[f->f_type]);
		f->f_time = now;
		switch (f->f_type) {
		case F_UNUSED:
			dprintf("\n");
			break;

		case F_FORW:
			dprintf("Forward (unsupported)\n");
			break;

		case F_CONSOLE:
			if (flags & IGN_CONS) {
				dprintf(" (ignored)\n");
				break;
			}

		case F_TTY:
		case F_FILE:
			dprintf(" %s\n", f->f_un.f_fname);
			if (f->f_type != F_FILE) {
				v->iov_base = "\r\n";
				v->iov_len = 2;
			} else {
				v->iov_base = "\n";
				v->iov_len = 1;
			}
			if (writev(f->f_file, iov, 6) < 0) {
				int e = errno;
				 close(f->f_file);
				/*
				 * Check for EBADF on TTY's due to vhangup() XXX
				 */
				if (e == EBADF && f->f_type != F_FILE) {
					f->f_file = open(f->f_un.f_fname, O_WRONLY|O_APPEND);
					if (f->f_file < 0) {
						f->f_type = F_UNUSED;
						logerror(f->f_un.f_fname);
					}
				} else {
					f->f_type = F_UNUSED;
					errno = e;
					logerror(f->f_un.f_fname);
				}
			} else if (flags & SYNC_FILE)
#ifndef linux
				 fsync(f->f_file);
#else
				 sync();
#endif
			break;

		case F_USERS:
		case F_WALL:
			dprintf("\n");
			v->iov_base = "\r\n";
			v->iov_len = 2;
			wallmsg(f, iov);
			break;
		}
	}
	signal(SIGHUP, (void *) s1);
	signal(SIGALRM, (void *) s2);
}


/*
 *  WALLMSG -- Write a message to the world at large
 *
 *	Write the specified message to either the entire
 *	world, or a list of approved users.
 */

wallmsg(f, iov)
	register struct filed *f;
	struct iovec *iov;
{
	register char *p;
	register int i;
	int ttyf, len, pid, wstat, xpid;
	FILE *uf;
	static int reenter = 0;
	struct utmp ut;
	time_t now;
	char greetings[200];

	if (reenter++)
		return;

	/* open the user login file */
	if ((uf = fopen("/etc/utmp", "r")) == NULL) {
		logerror("/etc/utmp");
		reenter = 0;
		return;
	}

	 time(&now);
	 sprintf(greetings,
	    "\r\n\7Message from syslogd@%s at %.24s ...\r\n",
		iov[2].iov_base, ctime(&now));
	len = strlen(greetings);

	/* scan the user login file */
	xpid = 0;
	while (fread((char *) &ut, sizeof ut, 1, uf) == 1) {
		/* is this slot used? */
		if (ut.ut_name[0] == '\0')
			continue;

		/* should we send the message to this user? */
		if (f->f_type == F_USERS) {
			for (i = 0; i < MAXUNAMES; i++) {
				if (!f->f_un.f_uname[i][0]) {
					i = MAXUNAMES;
					break;
				}
				if (strncmp(f->f_un.f_uname[i], ut.ut_name,
				    UNAMESZ) == 0)
					break;
			}
			if (i >= MAXUNAMES)
				continue;
		}

		/* compute the device name */
		p = "/dev/12345678";
		strncpy(&p[5], ut.ut_line, UNAMESZ);

		/*
		 * Might as well   instead of using nonblocking I/O
		 * and doing notty().
		 */
		pid = fork();
/*		if (pid != 0) {
			while ((xpid = wait(&wstat)) != pid) {
				if (xpid == -1)
					break;
			}
		} else
*/		if (pid == 0) {
			if (f->f_type == F_WALL) {
				iov[0].iov_base = greetings;
				iov[0].iov_len = len;
				iov[1].iov_len = 0;
			}
			 signal(SIGALRM, SIG_DFL);
			 alarm(30);
			/* open the terminal */
			ttyf = open(p, O_WRONLY);
			if (ttyf >= 0)
				 writev(ttyf, iov, 6);
			exit(0);
		}
	}
	/* close the user login file */
	 fclose(uf);
	while ((xpid = wait(&wstat)) != -1) ;
	reenter = 0;
}


domark()
{
	int pri;

	if ((++MarkSeq % MARKCOUNT) == 0)
		logmsg(LOG_INFO, "-- MARK --", LocalHostName, ADDDATE|MARK);
	else
		flushmsg();
	signal(SIGALRM, (void *) domark);
	alarm(MarkInterval * 60 / MARKCOUNT);
}

flushmsg()
{
	if (PrevCount == 0)
		return;
	if (PrevCount > 1)
		 sprintf(PrevLine+16, "last message repeated %d times", PrevCount);
	PrevCount = 0;
	logmsg(PrevPri, PrevLine, PrevHost, PrevFlags|NOCOPY);
	PrevLine[0] = '\0';
}

/*
 * Print syslogd errors some place.
 */
logerror(type)
	char *type;
{
	char buf[100];

	if (errno == 0)
		 sprintf(buf, "syslogd: %s", type);
	else if ((unsigned) errno > sys_nerr)
		 sprintf(buf, "syslogd: %s: error %d", type, errno);
	else
		 sprintf(buf, "syslogd: %s: %s", type, sys_errlist[errno]);
	errno = 0;
	dprintf("%s\n", buf);
	logmsg(LOG_SYSLOG|LOG_ERR, buf, LocalHostName, ADDDATE);
}

die(sig)
{
	char buf[100];

	if (sig) {
		dprintf("syslogd: going down on signal %d\n", sig);
		flushmsg();
		 sprintf(buf, "going down on signal %d", sig);
		logerror(buf);
	}
	 unlink(LogName);
	exit(0);
}

/*
 *  INIT -- Initialize syslogd from configuration table
 */

init()
{
	register int i;
	register FILE *cf;
	register struct filed *f;
	register char *p;
	char cline[BUFSIZ];

	dprintf("init\n");

	/* flush any pending output */
	flushmsg();

	/*
	 *  Close all open log files.
	 */
	for (f = Files; f < &Files[NLOGS]; f++) {
		if (f->f_type == F_FILE || f->f_type == F_TTY)
			 close(f->f_file);
		f->f_type = F_UNUSED;
	}

	/* open the configuration file */
	if ((cf = fopen(ConfFile, "r")) == NULL) {
		dprintf("cannot open %s\n", ConfFile);
		cfline("*.ERR\t/dev/tty0", &Files[0]);
		cfline("*.PANIC\t*", &Files[1]);
		return;
	}

	/*
	 *  Foreach line in the conf table, open that file.
	 */
	f = Files;
	while (fgets(cline, sizeof cline, cf) != NULL && f < &Files[NLOGS]) {
		/* check for end-of-section */
		if (cline[0] == '\n' || cline[0] == '#')
			continue;

		/* strip off newline character */
		p = index(cline, '\n');
		if (p)
			*p = '\0';

		cfline(cline, f++);
	}

	/* close the configuration file */
	 fclose(cf);

	Initialized = 1;

	if (Debug) {
		for (f = Files; f < &Files[NLOGS]; f++) {
			for (i = 0; i <= LOG_NFACILITIES; i++)
				if (f->f_pmask[i] == NOPRI)
					printf("X ");
				else
					printf("%d ", f->f_pmask[i]);
			printf("%s: ", TypeNames[f->f_type]);
			switch (f->f_type) {
			case F_FILE:
			case F_TTY:
			case F_CONSOLE:
				printf("%s", f->f_un.f_fname);
				break;

			case F_FORW:
				break;

			case F_USERS:
				for (i = 0; i < MAXUNAMES && *f->f_un.f_uname[i]; i++)
					printf("%s, ", f->f_un.f_uname[i]);
				break;
			}
			printf("\n");
		}
	}

	logmsg(LOG_SYSLOG|LOG_INFO, "syslogd: restart", LocalHostName, ADDDATE);
	dprintf("syslogd: restarted\n");
}

/*
 * Crack a configuration file line
 */

struct code {
	char	*c_name;
	int	c_val;
};

struct code	PriNames[] = {
	"panic",	LOG_EMERG,
	"emerg",	LOG_EMERG,
	"alert",	LOG_ALERT,
	"crit",		LOG_CRIT,
	"err",		LOG_ERR,
	"error",	LOG_ERR,
	"warn",		LOG_WARNING,
	"warning",	LOG_WARNING,
	"notice",	LOG_NOTICE,
	"info",		LOG_INFO,
	"debug",	LOG_DEBUG,
	"none",		NOPRI,
	NULL,		-1
};

struct code	FacNames[] = {
	"kern",		LOG_KERN,
	"user",		LOG_USER,
	"mail",		LOG_MAIL,
	"daemon",	LOG_DAEMON,
	"auth",		LOG_AUTH,
	"security",	LOG_AUTH,
	"mark",		LOG_MARK,
	"syslog",	LOG_SYSLOG,
	"lpr",		LOG_LPR,
	"local0",	LOG_LOCAL0,
	"local1",	LOG_LOCAL1,
	"local2",	LOG_LOCAL2,
	"local3",	LOG_LOCAL3,
	"local4",	LOG_LOCAL4,
	"local5",	LOG_LOCAL5,
	"local6",	LOG_LOCAL6,
	"local7",	LOG_LOCAL7,
	NULL,		-1
};

cfline(line, f)
	char *line;
	register struct filed *f;
{
	register char *p;
	register char *q;
	register int i;
	char *bp;
	int pri;
	struct hostent *hp;
	char buf[MAXLINE];

	dprintf("cfline(%s)\n", line);

	/* clear out file entry */
	bzero((char *) f, sizeof *f);
	for (i = 0; i <= LOG_NFACILITIES; i++)
		f->f_pmask[i] = NOPRI;

	/* scan through the list of selectors */
	for (p = line; *p && *p != '\t';) {

		/* find the end of this facility name list */
		for (q = p; *q && *q != '\t' && *q++ != '.'; )
			continue;

		/* collect priority name */
		for (bp = buf; *q && !index("\t,;", *q); )
			*bp++ = *q++;
		*bp = '\0';

		/* skip cruft */
		while (index(", ;", *q))
			q++;

		/* decode priority name */
		pri = decode(buf, PriNames);
		if (pri < 0) {
			char xbuf[200];

			 sprintf(xbuf, "unknown priority name \"%s\"", buf);
			logerror(xbuf);
			return;
		}

		/* scan facilities */
		while (*p && !index("\t.;", *p)) {
			int i;

			for (bp = buf; *p && !index("\t,;.", *p); )
				*bp++ = *p++;
			*bp = '\0';
			if (*buf == '*')
				for (i = 0; i < LOG_NFACILITIES; i++)
					f->f_pmask[i] = pri;
			else {
				i = decode(buf, FacNames);
				if (i < 0) {
					char xbuf[200];

					 sprintf(xbuf, "unknown facility name \"%s\"", buf);
					logerror(xbuf);
					return;
				}
				f->f_pmask[i >> 3] = pri;
			}
			while (*p == ',' || *p == ' ')
				p++;
		}

		p = q;
	}

	/* skip to action part */
	while (*p == '\t')
		p++;

fprintf(stderr, "syslogd: cfline on %s\n", p);

	switch (*p)
	{
	case '@':
		if (!InetInuse)
			break;
		 strcpy(f->f_un.f_uname, ++p);

		f->f_type = F_FORW;
		break;

	case '/':
		 strcpy(f->f_un.f_fname, p);
		if ((f->f_file = open(p, O_WRONLY|O_APPEND)) < 0) {
			logerror(p);
			break;
		}

		if (isatty(f->f_file)) {
			f->f_type = F_TTY;
			untty();
		}
		else
			f->f_type = F_FILE;
		if (strcmp(p, ctty) == 0)
			f->f_type = F_CONSOLE;
		break;

	case '*':
		f->f_type = F_WALL;
		break;

	default:
		for (i = 0; i < MAXUNAMES && *p; i++) {
			for (q = p; *q && *q != ','; )
				q++;
			 strncpy(f->f_un.f_uname[i], p, UNAMESZ);
			if ((q - p) > UNAMESZ)
				f->f_un.f_uname[i][UNAMESZ] = '\0';
			else
				f->f_un.f_uname[i][q - p] = '\0';
			while (*q == ',' || *q == ' ')
				q++;
			p = q;
		}
		f->f_type = F_USERS;
		break;
	}
}


/*
 *  Decode a symbolic name to a numeric value
 */

decode(name, codetab)
	char *name;
	struct code *codetab;
{
	register struct code *c;
	register char *p;
	char buf[40];

	if (isdigit(*name))
		return (atoi(name));

	 strcpy(buf, name);
	for (p = buf; *p; p++)
		if (isupper(*p))
			*p = tolower(*p);
	for (c = codetab; c->c_name; c++)
		if (!strcmp(buf, c->c_name))
			return (c->c_val);

	return (-1);
}

readline(fd, s, size)
int fd;
char *s;
int size;
{
	int c, i, j;
	static long cur_pos;
	struct stat st;

	fstat(fd, &st);
	if (cur_pos < st.st_size) {
		i = 0; j = 0;
		do {
			j = read(fd, &c, 1);
			*s++ = c;
			i++;
		} while (j > 0 && c != '\n');
		cur_pos = lseek(fd, 0L, 1);
		return(i);
	}
	return(0);
}

int
recvfrom(s, buf, buflen, flags, sock, socklen)
int s, buflen, flags;
char *buf;
struct sockaddr *sock;
int *socklen;
{
	int i, fd, nfds, cc;
	long newmask;

	/* check for dead connections */

	/* I think this is done below now */

	/* check for data on current connections and return */

	if (mask | FDMASK(s)) {
		newmask = mask | FDMASK(s);
		nfds = select(20, (fd_set *) &newmask, (fd_set *) NULL,
		    (fd_set *) NULL, (struct timeval *) NULL);
	
		fd = 0;
		while (nfds > 0) {
	/* accept new connection and read data */

			if (newmask & FDMASK(s)) {
				listen(s, 5);
				fd = accept(s, sock, socklen);
				if (fd < 0) {
					return -1;
				}
				mask |= FDMASK(fd);
				fcntl(fd, F_SETFL, O_NONBLOCK);
				cc = read(fd, buf, buflen);
				return(cc);
			} else {
				while (!(newmask & 1)) {
					newmask = newmask >> 1;
					fd++;
				}
				cc = read(fd, buf, buflen);
				if (cc <= 0) {
					mask ^= FDMASK(fd);
					newmask = newmask >> 1;
					close(fd);
					fd++;
					nfds--;
				} else {
					return(cc);
				}
			}
		}
	}
}

