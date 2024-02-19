/*
**	$Id: main.c,v 2.0 90/09/19 20:02:06 paul Rel $
**
**	Main body of program.
*/

/*
**	Copyright 1989,1990 by Paul Sutcliffe Jr.
**
**	Permission is hereby granted to copy, reproduce, redistribute,
**	or otherwise use this software as long as: there is no monetary
**	profit gained specifically from the use or reproduction or this
**	software, it is not sold, rented, traded or otherwise marketed,
**	and this copyright notice is included prominently in any copy
**	made.
**
**	The author make no claims as to the fitness or correctness of
**	this software for any use whatsoever, and it is provided as is. 
**	Any use of this software is at the user's own risk.
*/

/*
**	$Log:	main.c,v $
**	Revision 2.0  90/09/19  20:02:06  paul
**	Initial 2.0 release
**	
*/


#define	MAIN

#include "getty.h"
#include "defaults.h"
#include "table.h"
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef	PWD
#include <pwd.h>
#endif	/* PWD */

#if defined(RCSID) && !defined(lint)
static char *RcsId =
"@(#)$Id: main.c,v 2.0 90/09/19 20:02:06 paul Rel $";
#endif

#if !defined(lint)
#include "release.h"
static char *Release = RELEASE;
static char *RelDate = DATE;
#endif

/* how does this thing work
 */
#define	USAGE	"Usage:\t%s [options] %s\n\t%s -c checkfile\n"
#ifdef	TRS16
#define	UOPT	"speed [defaultfile]"		/* Tandy Xenix/68k 3.2 */
#else	/* TRS16 */
#define	UOPT	"line [speed [type [lined]]]"	/* real System V */
#endif	/* TRS16 */

#define	VALUE(cptr)	((cptr == (char *) NULL) ? "NULL" : cptr)

struct	speedtab {
	ushort	cbaud;		/* baud rate */
	int	nspeed;		/* speed in numeric format */
	char	*speed;		/* speed in display format */
} speedtab[] = {
	{ B50,	  50,	 "50"	 },
	{ B75,	  75,	 "75"	 },
	{ B110,	  110,	 "110"	 },
	{ B134,	  134,	 "134"	 },
	{ B150,	  150,	 "150"	 },
	{ B200,	  200,	 "200"	 },
	{ B300,	  300,	 "300"	 },
	{ B600,	  600,	 "600"	 },
	{ B1200,  1200,	 "1200"	 },
	{ B1800,  1800,	 "1800"	 },
	{ B2400,  2400,	 "2400"	 },
	{ B4800,  4800,	 "4800"	 },
	{ B9600,  9600,	 "9600"	 },
#ifdef	B19200
	{ B19200, 19200, "19200" },
#endif	/* B19200 */
#ifdef	B38400
	{ B38400, 38400, "38400" },
#endif	/* B38400 */
	{ EXTA,	  0,	 "EXTA"	 },
	{ EXTB,	  0,	 "EXTB"	 },
	{ 0,	  0,	 ""	 }
};

extern	int	errno;

sig_t		timeout();
int		tputc();
void		exit_usage();
struct	passwd	*getpwuid();
struct	utmp	*getutent();
#ifdef	DEBUG
char		*ctime();
#endif	/* DEBUG */


#ifdef	UUGETTY

char	*lock, *altlock;

int	makelock(), readlock();
boolean	checklock();
sig_t	rmlocks();

#endif	/* UUGETTY */


#ifdef	WARNCASE
char	*bad_case[] = {
	"\r\n",
	"If your terminal supports lower case letters, please\r\n",
	"use them.  Login again, using lower case if possible.\r\n",
	(char *) NULL
};
#endif	/* WARNCASE */


main(argc, argv)
int argc;
char *argv[];
{
	register int c, i, fd;
	int cbaud, nspeed, flags;
	char ch, *p, *speed, devname[MAXLINE+1];
	STDCHAR buf[MAXLINE+1];
	char termcap[1024], tbuf[64];
	char *tgetstr();
	DEF **def;
	TERMIO termio;
	GTAB *gtab, *gt;
	FILE *fp;
	struct utmp *utmp;
	struct stat st;

#if defined(DEBUG) || defined(LOGUTMP)
	time_t clock;
#endif	/* DEBUG || LOGUTMP */

#ifndef	TRS16
	char lined[MAXLINE+1];
#endif	/* TRS16 */

#ifdef	UUGETTY
	struct passwd *pwd;
	UIDTYPE uucpuid = 0;
	GIDTYPE uucpgid = 0;
#endif	/* UUGETTY */

#ifdef	TTYTYPE
	char name[16], line[16];
#endif	/* TTYTYPE */

#ifdef	LOGUTMP
	int pid;
#endif	/* LOGUTMP */

#ifdef	ISSUE
	char *issue = ISSUE;			/* default issue file */
#endif	/* ISSUE */

	boolean clear = TRUE;			/* clear screen flag */
	char *login = LOGIN;			/* default login program */
	char *clrscr = (char *) NULL;		/* string to clear screen */
	char *defname = (char *) NULL;		/* defaults file name */
	char *init = (char *) NULL;		/* value of INIT */
	char term[16];				/* terminal type */
	boolean waitchar = FALSE;		/* wait for char flag */
	unsigned int delay = 0;			/* #sec's delay before prompt */
	char *waitfor = (char *) NULL;		/* string to wait for */
	char *connect = (char *) NULL;		/* connect chat string */

	extern int optind;
	extern char *optarg;
	extern int expect();

	/* startup
	 */
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGTERM, SIG_DFL);

	(void) strcpy(term, "unknown");
	AutoBaud = FALSE;
	AutoRate[0] = '\0';
	Check = FALSE;
	CheckFile = (char *) NULL;
#ifdef	DEBUG
	Debug = 0;
#endif	/* DEBUG */
	Device = "unknown";
	GtabId = (char *) NULL;
	LineD = (char *) NULL;
	NoHangUp = FALSE;
	TimeOut = 0;
#ifdef	WARNCASE
	WarnCase = TRUE;
#endif	/* WARNCASE */

	/* who am I?
	 */
#ifdef	UUGETTY
	MyName = "uugetty";
#else
	MyName = "getty";
#endif	/* UUGETTY */

	/* process the command line
	 */

	while ((c = getopt(argc, argv, "C:D:c:d:ht:w:")) != EOF) {
		switch (c) {
		case 'C':
			connect = optarg;
			break;
		case 'D':
#ifdef	DEBUG
			(void) sscanf(optarg, "%o", &Debug);
			Dfp = stderr;
#else	/* DEBUG */
			logerr("DEBUG not compiled in");
#endif	/* DEBUG */
			break;
		case 'c':
			Check = TRUE;
			CheckFile = optarg;
			break;
		case 'd':
			defname = optarg;
			break;
		case 'h':
			NoHangUp = TRUE;
			break;
		case 'r':
			waitchar = TRUE;
			delay = (unsigned) atoi(optarg);
			break;
		case 't':
			TimeOut = atoi(optarg);
			break;
		case 'w':
			waitchar = TRUE;
			waitfor = optarg;
			break;
		case '?':
			exit_usage(2);
		}
	}

	/* just checking?
	 */
	if (Check) {
		(void) signal(SIGINT, SIG_DFL);
		(void) gtabvalue((char *) NULL, G_CHECK);
		exit(0);
	}

#ifdef	TRS16
	
	/* special handling for v7-based init
	 */

	if (optind < argc)
		GtabId = argv[optind++];
	else {
		logerr("no speed given");
		exit_usage(2);
	}

	/* Tandy Xenix/68k 3.2 /etc/inittab allows one optional argument
	 * after the speed flag.  The best use I could do with it here is
	 * to assume that it's the name of the defaults file to be used.
	 *
	 * Sigh.  Actually, it's not optional -- if none is given in
	 * /etc/inittab, then it appears here as a 0-length string.
	 */
	if (optind < argc)
		if (strlen(argv[optind]) > 0)
			defname = argv[optind++];

	if ((p = ttyname(STDIN)) != (char *) NULL)
		Device = p+5;		/* strip off "/dev/" */
	else {
		logerr("cannot determine line");
		exit_usage(2);
	}

#else	/* TRS16 */

	/* normal System V handling
	 */

	if (optind < argc)
		Device = argv[optind++];
	else {
		logerr("no line given");
		exit_usage(2);
	}
	if (optind < argc)
		GtabId = argv[optind++];
	if (optind < argc)
		(void) strncpy(term, argv[optind++], sizeof(term));
	if (optind < argc) {
		(void) strncpy(lined, argv[optind++], sizeof(lined));
		LineD = lined;
	}

#endif	/* TRS16 */

#ifdef	TTYTYPE

	if (strequal(term, "unknown")) {
		if ((fp = fopen(TTYTYPE, "r")) == (FILE *) NULL) {
			(void) sprintf(MsgBuf, "cannot open %s", TTYTYPE);
			logerr(MsgBuf);
		} else {
			while ((fscanf(fp, "%s %s", name, line)) != EOF) {
				if (strequal(line, Device)) {
					(void) strncpy(term,name,sizeof(term));
					break;
				}
			}
			(void) fclose(fp);
		}
	}

#endif	/* TTYTYPE */

	/* need full name of the device
	 */
	(void) sprintf(devname, "/dev/%s", Device);

	/* command line parsed, now build the list of
	 * runtime defaults; this may override things set above.
	 */
	def = defbuild(defname);

#ifdef	DEBUG

	/* debugging on?
	 */
	if ((p = defvalue(def, "DEBUG")) != (char *) NULL)
		(void) sscanf(p, "%o", &Debug);

	if (Debug) {
		(void) sprintf(buf, "/tmp/getty:%s", Device);
		if ((Dfp = fopen(buf, "a+")) == (FILE *) NULL) {
			logerr("cannot open debug file");
			exit(FAIL);
		} else {
			if (fileno(Dfp) < 3) {
				if ((fd = fcntl(fileno(Dfp), F_DUPFD, 3)) > 2) {
					(void) fclose(Dfp);
					Dfp = fdopen(fd, "a+");
				}
			}
			(void) time(&clock);
			(void) fprintf(Dfp, "%s Started: %s",
					MyName, ctime(&clock));
		}
	}

	debug(D_OPT, "command line values:\n");
	debug(D_OPT, " [-C] connect  = (%s)\n", VALUE(connect));
	debug(D_OPT, " [-d] defname  = (%s)\n", VALUE(defname));
	debug(D_OPT, " [-h] NoHangUp = (%s)\n", (NoHangUp) ? "TRUE" : "FALSE");
	debug(D_OPT, " [-r] waitchar = (%s)\n", (waitchar) ? "TRUE" : "FALSE");
	debug(D_OPT, "      delay    = (%u)\n", delay);
	debug(D_OPT, " [-t] TimeOut  = (%d)\n", TimeOut);
	debug(D_OPT, " [-w] waitfor  = (%s)\n", VALUE(waitfor));
	debug(D_OPT, " line  = (%s)\n", VALUE(Device));
	debug(D_OPT, " speed = (%s)\n", VALUE(GtabId));
	debug(D_OPT, " type  = (%s)\n", term);
	debug(D_OPT, " lined = (%s)\n", VALUE(LineD));
	debug(D_RUN, "loading defaults\n");

#endif	/* DEBUG */

	/* setup all runtime values
	 */

	if ((SysName = defvalue(def, "SYSTEM")) == (char *) NULL)
		SysName = getuname();

	if ((Version = defvalue(def, "VERSION")) != (char *) NULL)
		if (*Version == '/') {
			if ((fp = fopen(Version, "r")) != (FILE *) NULL) {
				(void) fgets(buf, sizeof(buf), fp);
				(void) fclose(fp);
				buf[strlen(buf)-1] = '\0';
				Version = strdup(buf);
			}
		}

	if ((p = defvalue(def, "LOGIN")) != (char *) NULL)
		login = p;
	if ((p = defvalue(def, "INIT")) != (char *) NULL)
		init = p;
#ifdef	ISSUE
	if ((p = defvalue(def, "ISSUE")) != (char *) NULL)
		issue = p;
#endif	/* ISSUE */
	if ((p = defvalue(def, "CLEAR")) != (char *) NULL)
		if (strequal(p, "NO"))
			clear = FALSE;
	if ((p = defvalue(def, "HANGUP")) != (char *) NULL)
		if (strequal(p, "NO"))
			NoHangUp = TRUE;
	if ((p = defvalue(def, "WAITCHAR")) != (char *) NULL)
		if (strequal(p, "YES"))
			waitchar = TRUE;
	if ((p = defvalue(def, "DELAY")) != (char *) NULL)
		delay = (unsigned) atoi(p);
	if ((p = defvalue(def, "TIMEOUT")) != (char *) NULL)
		TimeOut = atoi(p);
	if ((p = defvalue(def, "CONNECT")) != (char *) NULL)
		connect = p;
	if ((p = defvalue(def, "WAITFOR")) != (char *) NULL) {
		waitchar = TRUE;
		waitfor = p;
	}

	/* find out how to clear the screen
	 */
	if (!strequal(term, "unknown")) {
		p = tbuf;
		if ((tgetent(termcap, term)) == 1)
			if ((clrscr = tgetstr("cl", &p)) == (char *) NULL)
				clrscr = "";
	}

#ifdef	UUGETTY

	debug2(D_RUN, "check for lockfiles\n");

	/* deal with the lockfiles; we don't want to charge
	 * ahead if uucp, kermit or something is already
	 * using the line.
	 */

	/* name the lock file(s)
	 */
	(void) sprintf(buf, LOCK, Device);
	lock = strdup(buf);
	altlock = defvalue(def, "ALTLOCK");
	if (altlock != (char *) NULL) {
		(void) sprintf(buf, LOCK, altlock);
		altlock = strdup(buf);
	}
	debug3(D_LOCK, "lock = (%s)\n", lock);
	debug3(D_LOCK, "altlock = (%s)\n", VALUE(altlock));

	/* check for existing lock file(s)
	 */
	if (checklock(lock) == TRUE) {
		while (checklock(lock) == TRUE)
			(void) sleep(60);
		exit(0);
	}

	/* there's a race condition just asking for trouble here  :-(
	 */
	if (altlock != (char *) NULL && checklock(altlock) == TRUE) {
		while (checklock(altlock) == TRUE)
			(void) sleep(60);
		exit(0);
	}

	/* allow uucp to access the device
	 */
	(void) chmod(devname, 0666);
	if ((pwd = getpwuid(UUCPID)) != (struct passwd *) NULL) {
		uucpuid = pwd->pw_uid;
		uucpgid = pwd->pw_gid;
	}
	(void) chown(devname, uucpuid, uucpgid);

#else	/* UUGETTY */

	(void) chmod(devname, 0622);
	if (stat(devname, &st) == 0)
		(void) chown(devname, 0, st.st_gid);
	else
		(void) chown(devname, 0, 0);

#endif	/* UUGETTY */

	/* the line is mine now ...
	 */

	debug2(D_RUN, "open stdin, stdout and stderr\n");

	/* open the device; don't wait around for carrier-detect
	 */
	if ((fd = open(devname, O_RDWR | O_NDELAY)) < 0) {
		logerr("cannot open line");
		exit(FAIL);
	}

	/* make new fd == stdin if it isn't already
	 */
	if (fd > 0) {
		(void) close(0);
		if (dup(fd) != 0) {
			logerr("cannot open stdin");
			exit(FAIL);
		}
	}

	/* make stdout and stderr, too
	 */
	(void) close(1);
	(void) close(2);
	if (dup(0) != 1) {
		logerr("cannot open stdout");
		exit(FAIL);
	}
	if (dup(0) != 2) {
		logerr("cannot open stderr");
		exit(FAIL);
	}

	if (fd > 0)
		(void) close(fd);

	/* no buffering
	 */
	setbuf(stdin, (char *) NULL);
	setbuf(stdout, (char *) NULL);
	setbuf(stderr, (char *) NULL);

	debug2(D_STTY, "Stdin just opened:\n");
	if (Debug & D_STTY) system("stty -a >&3");
	debug2(D_RUN, "setup terminal\n");

	/* get the required info from the gettytab file
	 */
	gtab = gtabvalue(GtabId, G_FORCE);

	/* setup terminal
	 */
	if (!NoHangUp) {
		(void) ioctl(STDIN, TCGETA, &termio);
		termio.c_cflag &= ~CBAUD;	/* keep all but CBAUD bits */
		termio.c_cflag |= B0;		/* set speed == 0 */
		(void) ioctl(STDIN, TCSETAF, &termio);
	}
	settermio(&(gtab->itermio), INITIAL);

	debug2(D_STTY, "After processing gettydefs:\n");
	if (Debug & D_STTY) system("stty -a >&3");

	/* clear O_NDELAY flag now
	 */
	flags = fcntl(STDIN, F_GETFL, 0);
	(void) fcntl(STDIN, F_SETFL, flags & ~O_NDELAY);

	/* handle init sequence if requested
	 */
	if (init != (char *) NULL) {
		debug2(D_RUN, "perform line initialization\n");
		if (chat(init) == FAIL){
			logerr("warning: INIT sequence failed");
			debug2(D_RUN, "Modem initialization failed -- aborting\n");
			exit(FAIL);
			}
	}

#ifdef	LOGUTMP

	debug2(D_RUN, "update utmp/wtmp files\n");

	pid = getpid();
#ifndef linux
	while ((utmp = getutent()) != (struct utmp *) NULL) 
		if (utmp->ut_type == INIT_PROCESS && utmp->ut_pid == pid)
#endif
			{
			debug2(D_UTMP, "logutmp entry made\n");
			/* show login process in utmp
			 */
			strncopy(utmp->ut_line, Device);
			strncopy(utmp->ut_id, Device+3);
			utmp->ut_host[0] = '\0';
			utmp->ut_addr = 0;
			strncopy(utmp->ut_user, "LOGIN");
			utmp->ut_pid = pid;
			utmp->ut_type = LOGIN_PROCESS;
			(void) time(&clock);
			utmp->ut_time = clock;
			pututline(utmp);

			/* write same record to end of wtmp
			 * if wtmp file exists
			 */
#ifndef linux
			if (stat(WTMP_FILE, &st) && errno == ENOENT)
				break;
#endif
			if ((fp = fopen(WTMP_FILE, "a")) != (FILE *) NULL) {
				(void) fseek(fp, 0L, 2);
				(void) fwrite((char *)utmp,sizeof(*utmp),1,fp);
				(void) fclose(fp);
			}
		}
	endutent();

#endif	/* LOGUTMP */

	/* do we need to wait ?
	 */
	if (waitchar) {

		debug2(D_RUN, "waiting for any char ...\n");

		(void) ioctl(STDIN, TCFLSH, 0);
		(void) read(STDIN, &ch, 1);		/* this will block */

		debug2(D_RUN, "... got one!\n");

#ifdef	UUGETTY
		/* check to see if line is locked, we don't want to
		 * read more chars if that's the case
		 */
		if (checklock(lock) == TRUE) {
			debug2(D_RUN, "line locked now, stopping\n");
			while (checklock(lock) == TRUE)
				(void) sleep(60);
			exit(0);
		}
		if (altlock != (char *) NULL && checklock(altlock) == TRUE) {
			debug2(D_RUN, "line locked now, stopping\n");
			while (checklock(altlock) == TRUE)
				(void) sleep(60);
			exit(0);
		}
#endif	/* UUGETTY */

		if (waitfor != (char *) NULL) {
			if (ch == *waitfor) {	/* first char equal ? */
				waitfor++;
				debug3(D_RUN, "matched waitfor[0] (%c)\n", ch);
				if (!(*waitfor)) {
					debug2(D_RUN, "match complete\n");
					goto wait_cont;
				}
			}
			if (expect(waitfor) == FAIL)
				exit(0);
		}

	    wait_cont:
		if (delay) {
			debug3(D_RUN, "delay(%d)\n", delay);
			(void) sleep(delay);
			/* eat up any garbage from the line (modem) */
			(void) fcntl(STDIN, F_SETFL, flags | O_NDELAY);
			while (read(STDIN, &ch, 1) == 1)
				;
			(void) fcntl(STDIN, F_SETFL, flags & ~O_NDELAY);
		}

	}

#ifdef	UUGETTY

	debug2(D_RUN, "locking the line\n");

	/* try to lock the line
	 */
	if (makelock(lock) == FAIL) {
		while (checklock(lock) == TRUE)
			(void) sleep(60);
		exit(0);
	}
	if (altlock != (char *) NULL && makelock(altlock) == FAIL) {
		while (checklock(altlock) == TRUE)
			(void) sleep(60);
		exit(0);
	}

	/* set to remove lockfile(s) on certain signals
	 */
	(void) signal(SIGHUP, rmlocks);
	(void) signal(SIGINT, rmlocks);
	(void) signal(SIGQUIT, rmlocks);
	(void) signal(SIGTERM, rmlocks);

#endif	/* UUGETTY */

	if (connect != (char *) NULL) {

		debug2(D_RUN, "perform connect sequence\n");

		cbaud = 0;
		if (strequal(connect, "DEFAULT"))
			connect = DEF_CONNECT;
		if (chat(connect) == FAIL){
			logerr("warning: CONNECT sequence failed");
			debug2(D_RUN, "Failed connect sequence -- aborting\n");
			exit(FAIL);
			}
		if (AutoBaud) {
			debug3(D_RUN, "AutoRate = (%s)\n", AutoRate);
#ifdef	TELEBIT
			if (strequal(AutoRate, "FAST"))
				(void) strcpy(AutoRate, TB_FAST);
#endif	/* TELEBIT */
			if ((nspeed = atoi(AutoRate)) > 0)
				for (i=0; speedtab[i].nspeed; i++)
					if (nspeed == speedtab[i].nspeed) {
						cbaud = speedtab[i].cbaud;
						speed = speedtab[i].speed;
						break;
					}
		}
		if (cbaud) {	/* AutoBaud && match found */
			debug3(D_RUN, "setting speed to %s\n", speed);
			if ((gt = gtabvalue(speed, G_FIND)) != (GTAB *) NULL) {
				/* use matching line from gettytab */
				if (strequal(gt->cur_id, speed)) {
					gtab = gt;
					goto set_term;
				}
			}
			/* change speed of existing gettytab line
			 */
			gtab->itermio.c_cflag =
				(gtab->itermio.c_cflag & ~CBAUD) | cbaud;
			gtab->ftermio.c_cflag =
				(gtab->ftermio.c_cflag & ~CBAUD) | cbaud;
		    set_term:
			settermio(&(gtab->itermio), INITIAL);
		}
	}

	debug2(D_RUN, "entering login loop\n");

	/* loop until a successful login is made
	 */
	for (;;) {

		/* set Nusers value
		 */
		Nusers = 0;
		setutent();
		while ((utmp = getutent()) != (struct utmp *) NULL) {
#ifdef	USER_PROCESS
			if (utmp->ut_type == USER_PROCESS)
#endif	/* USER_PROCESS */
			{
				Nusers++;
				debug3(D_UTMP, "utmp entry (%s)\n",
						utmp->ut_name);
			}
		}
		endutent();
		debug3(D_UTMP, "Nusers=%d\n", Nusers);

		/* set Speed value
		 */
		cbaud = gtab->itermio.c_cflag & CBAUD;
		for (i=0; speedtab[i].cbaud != cbaud; i++)
			;
		Speed = speedtab[i].speed;

#ifdef	ISSUE
		if (clear && *clrscr)		/* clear screen */
			(void) tputs(clrscr, 1, tputc);

		(void) fputc('\r', stdout);	/* just in case */

		/* display ISSUE, if present
		 */
		if (*issue != '/') {
			(void) Fputs(issue, stdout);
			(void) fputs("\r\n", stdout);
		} else if ((fp = fopen(issue, "r")) != (FILE *) NULL) {
			while (fgets(buf, sizeof(buf), fp) != (char *) NULL)
				(void) Fputs(buf, stdout);
			(void) fclose(fp);
		}
#endif	/* ISSUE */

	    login_prompt:

		/* display login prompt
		 */
		(void) Fputs("@S ", stdout);
		(void) Fputs(gtab->login, stdout);

		/* eat any chars from line noise
		 */
		(void) ioctl(STDIN, TCFLSH, 0);

		/* start timer, if required
		 */
		if (TimeOut > 0) {
			(void) signal(SIGALRM, timeout);
			(void) alarm((unsigned) TimeOut);
		}

		/* handle the login name
		 */
		switch (getlogname(&termio, buf, MAXLINE)) {
		case SUCCESS:
			/* stop alarm clock
			 */
			if (TimeOut > 0)
				(void) alarm((unsigned) 0);

			/* setup terminal
			 */
			termio.c_iflag |= gtab->ftermio.c_iflag;
			termio.c_oflag |= gtab->ftermio.c_oflag;
			termio.c_cflag |= gtab->ftermio.c_cflag;
			termio.c_lflag |= gtab->ftermio.c_lflag;
			termio.c_line  |= gtab->ftermio.c_line;
			settermio(&termio, FINAL);
#ifdef	DEBUG
			debug2(D_STTY, "Final settings:\n");
			if (Debug & D_STTY) system("stty -a >&3");
			if (Debug)
				(void) fclose(Dfp);
#endif	/* DEBUG */

#ifdef	SETTERM
			(void) sprintf(MsgBuf, "TERM=%s", term);
			(void) putenv(strdup(MsgBuf));
#endif	/* SETTERM */

			/* hand off to login, which can be a shell script!
			 */
			(void) execl(login, "login", buf, (char *) NULL);
			(void) execl("/bin/sh", "sh", "-c",
					login, buf, (char *) NULL);
			(void) sprintf(MsgBuf, "cannot execute %s", login);
			logerr(MsgBuf);
			exit(FAIL);

		case BADSPEED:
			/* go to next entry
			 */
			GtabId = gtab->next_id;
			debug3(D_RUN, "Bad Speed; trying %s\n", GtabId);
			gtab = gtabvalue(GtabId, G_FORCE);
			settermio(&(gtab->itermio), INITIAL);
			break;

#ifdef	WARNCASE
		case BADCASE:
			/* first try was all uppercase
			 */
			for (i=0; bad_case[i] != (char *) NULL; i++)
				(void) fputs(bad_case[i], stdout);
			goto login_prompt;
#endif	/* WARNCASE */

		case NONAME:
			/* no login name entered
			 */
			break;
		}
	}
}


/*
**	timeout() - handles SIGALRM
*/

sig_t
timeout()
{
	TERMIO termio;

	/* say bye-bye
	 */
	(void) sprintf(MsgBuf, "\nTimed out after %d seconds.\n", TimeOut);
	(void) Fputs(MsgBuf, stdout);
	(void) Fputs("Bye Bye.\n", stdout);

	/* force a hangup
	 */
	(void) ioctl(STDIN, TCGETA, &termio);
	termio.c_cflag &= ~CBAUD;
	termio.c_cflag |= B0;
	(void) ioctl(STDIN, TCSETAF, &termio);

	exit(1);
}


/*
**	tputc() - output a character for tputs()
*/

int
tputc(c)
char c;
{
	fputc(c, stdout);
}


/*
**	exit_usage() - exit with usage display
*/

void
exit_usage(code)
int code;
{
	FILE *fp;

	if ((fp = fopen(CONSOLE, "w")) != (FILE *) NULL) {
		(void) fprintf(fp, USAGE, MyName, UOPT, MyName);
		(void) fclose(fp);
	}
	exit(code);
}


#ifdef	UUGETTY

/*
**	makelock() - attempt to create a lockfile
**
**	Returns FAIL if lock could not be made (line in use).
*/

int
makelock(name)
char *name;
{
	int fd, pid;
	char *temp, buf[MAXLINE+1];
#ifdef	ASCIIPID
	char apid[16];
#endif	/* ASCIIPID */
	int getpid();
	char *mktemp();

	debug3(D_LOCK, "makelock(%s) called\n", name);

	/* first make a temp file
	 */
	(void) sprintf(buf, LOCK, "TM.XXXXXX");
	if ((fd = creat((temp=mktemp(buf)), 0444)) == FAIL) {
		(void) sprintf(MsgBuf, "cannot create tempfile (%s)", temp);
		logerr(MsgBuf);
		return(FAIL);
	}
	debug3(D_LOCK, "temp = (%s)\n", temp);

	/* put my pid in it
	 */
#ifdef	ASCIIPID
	(void) sprintf(apid, "%09d", getpid());
	(void) write(fd, apid, strlen(apid));
#else
	pid = getpid();
	(void) write(fd, (char *)&pid, sizeof(pid));
#endif	/* ASCIIPID */
	(void) close(fd);

	/* link it to the lock file
	 */
	while (link(temp, name) == FAIL) {
		debug3(D_LOCK, "link(temp,name) failed, errno=%d\n", errno);
		if (errno == EEXIST) {		/* lock file already there */
			if ((pid = readlock(name)) == FAIL)
				continue;
			if ((kill(pid, 0) == FAIL) && errno == ESRCH) {
				/* pid that created lockfile is gone */
				(void) unlink(name);
				continue;
			}
		}
		debug2(D_LOCK, "lock NOT made\n");
		(void) unlink(temp);
		return(FAIL);
	}
	debug2(D_LOCK, "lock made\n");
	(void) unlink(temp);
	return(SUCCESS);
}

/*
**	checklock() - test for presense of valid lock file
**
**	Returns TRUE if lockfile found, FALSE if not.
*/

boolean
checklock(name)
char *name;
{
	int pid;
	struct stat st;

	debug3(D_LOCK, "checklock(%s) called\n", name);

	if ((stat(name, &st) == FAIL) && errno == ENOENT) {
		debug2(D_LOCK, "stat failed, no file\n");
		return(FALSE);
	}

	if ((pid = readlock(name)) == FAIL) {
		debug2(D_LOCK, "couldn't read lockfile\n");
		return(FALSE);
	}

	if ((kill(pid, 0) == FAIL) && errno == ESRCH) {
		debug2(D_LOCK, "no active process has lock, will remove\n");
		(void) unlink(name);
		return(FALSE);
	}

	debug2(D_LOCK, "active process has lock, return(TRUE)\n");
	return(TRUE);
}

/*
**	readlock() - read contents of lockfile
**
**	Returns pid read or FAIL on error.
*/

int
readlock(name)
char *name;
{
	int fd, pid, n;
#ifdef	ASCIIPID
	char apid[16];
#endif	/* ASCIIPID */

	if ((fd = open(name, O_RDONLY)) == FAIL)
		return(FAIL);

#ifdef	ASCIIPID
	(void) read(fd, apid, sizeof(apid));
	n = sscanf(apid, "%d", &pid);
#else
	(void) read(fd, (char *)&pid, sizeof(pid));
#endif	/* ASCIIPID */

#ifdef  BOTHPID
	if (n != 1){
		(void) close(fd);
		fd = open(name, O_RDONLY);
		(void) read(fd, (char *)&pid, sizeof(pid));
		}
#endif

	(void) close(fd);
	debug3(D_LOCK, "read %d from the lockfile\n", pid);
	return(pid);
}

/*
**	rmlocks() - remove lockfile(s)
*/

sig_t
rmlocks()
{
	if (altlock != (char *) NULL)
		(void) unlink(altlock);

	(void) unlink(lock);
}

#endif	/* UUGETTY */


/* end of main.c */
