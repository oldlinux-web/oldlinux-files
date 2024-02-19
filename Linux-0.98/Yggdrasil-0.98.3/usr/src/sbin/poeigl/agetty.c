/*++
/* NAME
/*	agetty 8
/* SUMMARY
/*	alternative System V/SunOS 4 getty
/* SYSTEM V SYNOPSIS
/*	agetty [-i] [-l login_program] [-m] [-t timeout] port baud_rate,...
/* SUNOS 4 SYNOPSIS
/*	agetty [-h] [-l login_program] [-m] [-t timeout] baud_rate,... port
/* DESCRIPTION
/*	\fIagetty\fP opens a tty port, prompts for a login name and invokes
/*	the /bin/login command. It is normally invoked by \fIinit(8)\fP.
/*
/*	\fIagetty\fP has several \fInon-standard\fP features that are useful
/*	for hard-wired and for dial-in lines:
/* .IP o
/*	Adapts the tty settings to parity bits and to erase, kill,
/*	end-of-line and uppercase characters when it reads a login name.
/*	The program can handle 7-bit characters with even, odd, none or space
/*	parity, and 8-bit characters with no parity. The following special
/*	characters are recognized: @ and Control-U (kill); #, DEL and
/*	back space (erase); carriage return and line feed (end of line).
/* .IP o
/*	Optionally deduces the baud rate from the CONNECT messages produced by 
/*	Hayes(tm)-compatible modems.
/* .IP o
/*	Optionally does not hang up when it is given an already opened line 
/*	(useful for call-back applications).
/* .IP o
/*	Optionally does not display the contents of the \fI/etc/issue\fP file
/*	(System V only).
/* .IP o
/*	Optionally invokes a non-standard login program instead of
/*	\fI/bin/login\fP.
/* .IP o
/*	Optionally turns on hard-ware flow control (SunOS 4 only).
/* .PP
/*	This program does not use the \fI/etc/gettydefs\fP (System V) or
/*	\fI/etc/gettytab\fP (SunOS 4) files.
/* ARGUMENTS
/* .fi
/* .ad
/* .TP
/* port
/*	A path name relative to the \fI/dev\fP directory. If a "-" is 
/*	specified, \fIagetty\fP assumes that its standard input is 
/*	already connected to a tty port and that a connection to a 
/*	remote user has already been established. 
/* .sp
/*	Under System V, a "-" \fIport\fP argument should be preceded 
/*	by a "--".
/* .TP
/* baud_rate,...
/*	A comma-separated list of one or more baud rates. Each time 
/*	\fIagetty\fP receives a BREAK character it advances through 
/*	the list, which is treated as if it were circular. 
/* .sp
/*	Baud rates should be specified in descending order, so that the 
/*	null character (Ctrl-@) can also be used for baud rate switching.
/* OPTIONS
/* .fi
/* .ad
/* .TP
/* -h (SunOS 4 only)
/*	Enable hardware (RTS/CTS) flow control. It is left up to the
/*	application to disable software (XON/XOFF) flow protocol where
/*	appropriate.
/* .TP
/* -i (System V only)
/*	Do not display the contents of \fI/etc/issue\fP before writing the
/*	login prompt. Terminals or communications hardware may become confused
/*	when receiving lots of text at the wrong baud rate; dial-up scripts
/*	may fail if the login prompt is preceded by too much text.
/* .TP
/* -l login_program
/*	Invoke the specified \fIlogin_program\fP instead of /bin/login.
/*	This allows the use of a non-standard login program (for example,
/*	one that asks for a dial-up password or that uses a different 
/*	password file).
/* .TP
/* -m
/*	Try to extract the baud rate the \fIconnect\fP status message 
/*	produced by some Hayes(tm)-compatible modems. These status 
/*	messages are of the form: "<junk><speed><junk>".
/*	\fIagetty\fP assumes that the modem emits its status message at 
/*	the same speed as specified with (the first) \fIbaud_rate\fP value
/*	on the command line.
/* .sp
/*	Since the \fI-m\fP feature may fail on heavily-loaded systems, 
/*	you still should enable BREAK processing by enumerating all 
/*	expected baud rates on the command line.
/* .TP
/* -t timeout
/*	Terminate if no user name could be read within \fItimeout\fP 
/*	seconds. This option should probably not be used with hard-wired 
/*	lines.
/* SYSTEM V EXAMPLES
/*	This section shows sample entries for the \fI/etc/inittab\fP file.
/*
/*	For a hard-wired line:
/* .ti +5
/*	t0:2:respawn:/etc/agetty ttyM0 9600
/*
/*	For a dial-in line with a 2400/1200/300 baud modem:
/* .ti +5
/*	t1:2:respawn:/etc/agetty -mt60 ttyM1 2400,1200,300
/* SUNOS 4 EXAMPLES
/* .ad
/* .fi
/*	This section show sample entries for the \fI/etc/ttytab\fP file.
/*	Note that init(8) appends the port name to the command
/*	specified in the inittab file.
/*
/*	For a hard-wired line:
/* .ti +5
/*	ttya  "/usr/etc/agetty 9600"  vt100  on local
/*
/*	For a dial-in line with a 2400/1200/300 baud modem:
/* .ti +5
/*	ttyb  "/usr/etc/agetty -mt60 2400,1200,300"  unknown  on modem
/*
/*	The latter also requires that the \fIDTR\fP and \fICD\fP modem 
/*	control lines are enabled (see the eeprom(8) manual page).
/* FILES
/*	/etc/utmp, the system status file (System V only).
/*	/etc/issue, printed before the login prompt (System V only).
/*	/dev/console, problem reports (if syslog(3) is not used).
/*	/etc/inittab (System V init(8) configuration file).
/*	/etc/ttytab (SunOS 4 init(8) configuration file).
/* BUGS
/*	The baud-rate detection feature (the \fI-m\fP option) requires that
/*	\fIagetty\fP be scheduled soon enough after completion of a dial-in
/*	call (within 30 ms with modems that talk at 2400 baud). For robustness,
/*	always use the \fI-m\fP option in combination with a multiple baud
/*	rate command-line argument, so that BREAK processing is enabled.
/*
/*	The text in the /etc/issue file (System V only) and the login prompt
/*	are always output with 7-bit characters and space parity.
/*
/*	The baud-rate detection feature (the \fI-m\fP option) requires that 
/*	the modem emits its status message \fIafter\fP raising the DCD line.
/* DIAGNOSTICS
/*	Depending on how the program was configured, all diagnostics are
/*	written to the console device or reported via the syslog(3) facility.
/*	Error messages are produced if the \fIport\fP argument does not
/*	specify a terminal device; if there is no /etc/utmp entry for the
/*	current process (System V only); and so on.
/* AUTHOR(S)
/*	W.Z. Venema <wietse@wzv.win.tue.nl>
/*	Eindhoven University of Technology
/*	Department of Mathematics and Computer Science
/*	Den Dolech 2, P.O. Box 513, 5600 MB Eindhoven, The Netherlands
/* CREATION DATE
/*	Sat Nov 25 22:51:05 MET 1989
/* LAST MODIFICATION
/*	91/09/01 23:22:00
/* VERSION/RELEASE
/*	1.29
/*--*/

#ifndef	lint
char sccsid[] = "@(#) agetty.c 1.29 9/1/91 23:22:00";
#endif

#include <stdio.h>

#include <termio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <varargs.h>
#include <ctype.h>
#include <utmp.h>
#include <getopt.h>

#ifdef linux
#include "pathnames.h"
#include <sys/param.h>
#undef USE_SYSLOG
#endif

 /* If USE_SYSLOG is undefined all diagnostics go directly to /dev/console. */

#ifdef	USE_SYSLOG
#include <syslog.h>
extern void closelog();
#endif

extern void exit();

 /*
  * Some heuristics to find out what environment we are in: if it is not
  * System V, assume it is SunOS 4.
  */

#ifdef LOGIN_PROCESS			/* defined in System V utmp.h */
#define	SYSV_STYLE			/* select System V style getty */
#endif

 /*
  * Things you may want to modify.
  * 
  * If ISSUE is not defined, agetty will never display the contents of the
  * /etc/issue file. You will not want to spit out large "issue" files at the
  * wrong baud rate. Relevant for System V only.
  * 
  * You may disagree with the default line-editing etc. characters defined
  * below. Note, however, that DEL cannot be used for interrupt generation
  * and for line editing at the same time.
  */

#ifdef	SYSV_STYLE
#define	ISSUE "/etc/issue"		/* displayed before the login prompt */
#endif

#define LOGIN " login: "		/* login prompt */

/* Some shorthands for control characters. */

#define CTL(x)		(x ^ 0100)	/* Assumes ASCII dialect */
#define	CR		CTL('M')	/* carriage return */
#define	NL		CTL('J')	/* line feed */
#define	BS		CTL('H')	/* back space */
#define	DEL		CTL('?')	/* delete */

/* Defaults for line-editing etc. characters; you may want to change this. */

#define DEF_ERASE	DEL		/* default erase character */
#define DEF_INTR	CTL('C')	/* default interrupt character */
#define DEF_QUIT	CTL('\\')	/* default quit char */
#define DEF_KILL	CTL('U')	/* default kill char */
#define DEF_EOF		CTL('D')	/* default EOF char */
#define DEF_EOL		0
#define DEF_SWITCH	0		/* default switch char */

 /*
  * SunOS 4.1.1 termio is broken. We must use the termios stuff instead,
  * because the termio -> termios translation does not clear the termios
  * CIBAUD bits. Therefore, the tty driver would sometimes report that input
  * baud rate != output baud rate. I did not notice that problem with SunOS
  * 4.1. We will use termios where available, and termio otherwise.
  */

/* linux 0.12 termio is broken too, if we use it c_cc[VERASE] isn't set
   properly, but all is well if we use termios?! */

#ifdef	TCGETS
#undef	TCGETA
#undef	TCSETA
#undef	TCSETAW
#define	termio	termios
#define	TCGETA	TCGETS
#define	TCSETA	TCSETS
#define	TCSETAW	TCSETSW
#endif

 /*
  * This program tries to not use the standard-i/o library.  This keeps the
  * executable small on systems that do not have shared libraries (System V
  * Release <3).
  */

#define	BUFSIZ		1024

 /*
  * When multiple baud rates are specified on the command line, the first one
  * we will try is the first one specified.
  */

#define	FIRST_SPEED	0

/* Storage for command-line options. */

#define	MAX_SPEED	10		/* max. nr. of baud rates */

struct options {
    int     flags;			/* toggle switches, see below */
    int     timeout;			/* time-out period */
    char   *login;			/* login program */
    int     numspeed;			/* number of baud rates to try */
    int     speeds[MAX_SPEED];		/* baud rates to be tried */
    char   *tty;			/* name of tty */
};

#define	F_PARSE		(1<<0)		/* process modem status messages */
#define	F_ISSUE		(1<<1)		/* display /etc/issue */
#define	F_RTSCTS	(1<<2)		/* enable RTS/CTS flow control */

/* Storage for things detected while the login name was read. */

struct chardata {
    int     erase;			/* erase character */
    int     kill;			/* kill character */
    int     eol;			/* end-of-line character */
    int     parity;			/* what parity did we see */
    int     capslock;			/* upper case without lower case */
};

/* Initial values for the above. */

struct chardata init_chardata = {
    DEF_ERASE,				/* default erase character */
    DEF_KILL,				/* default kill character */
    0,					/* always filled in at runtime */
    0,					/* space parity */
    0,					/* always filled in at runtime */
};

/* The following is used for understandable diagnostics. */

char *progname;

/* ... */
#ifdef DEBUGGING
#define debug(s) fprintf(dbf,s); fflush(dbf)
FILE *dbf;
#else
#define debug(s) /* nothing */
#endif

main(argc, argv)
int     argc;
char  **argv;
{
    char   *logname;			/* login name, given to /bin/login */
    char   *get_logname();
    struct chardata chardata;		/* set by get_logname() */
    struct termio termio;		/* terminal mode bits */
    static struct options options = {
	F_ISSUE,			/* show /etc/issue (SYSV_STYLE) */
	0,				/* no timeout */
	_PATH_LOGIN,			/* default login program */
	0,				/* no baud rates known yet */
    };

    /* The BSD-style init command passes us a useless process name. */

#ifdef	SYSV_STYLE
    progname = argv[0];
#else
    progname = "agetty";
#endif

#ifdef DEBUGGING
	dbf = fopen("/dev/tty1", "w");

	{	int i;
	
		for(i = 1; i < argc; i++) {
			debug(argv[i]);
		}
	}
#endif

    /* Parse command-line arguments. */

    parse_args(argc, argv, &options);

#ifdef linux
	setsid();
#endif
	
    /* Update the utmp file. */

#ifdef	SYSV_STYLE
    update_utmp(options.tty);
#endif

    /* Open the tty as standard { input, output, error }. */
    open_tty(options.tty, &termio);

#ifdef linux
	{
		int iv;
		
		iv = getpid();
		(void) ioctl(0, TIOCSPGRP, &iv);
	}
#endif
    /* Initialize the termio settings (raw mode, eight-bit, blocking i/o). */

    termio_init(&termio, options.speeds[FIRST_SPEED]);
	
    /* Optionally detect the baud rate from the modem status message. */

    if (options.flags & F_PARSE)
	auto_baud(&termio);

    /* Set the optional timer. */

    if (options.timeout)
	(void) alarm((unsigned) options.timeout);
 
    /* Read the login name. */

    while ((logname = get_logname(&options, &chardata, &termio)) == 0)
	next_speed(&termio, &options);

    /* Disable timer. */

    if (options.timeout)
	(void) alarm(0);

    /* Finalize the termio settings. */

    termio_final(&options, &termio, &chardata);

    /* Now the newline character should be properly written. */

    (void) write(1, "\n", 1);

    /* Let the login program take care of password validation. */

    (void) execl(options.login, options.login, logname, (char *) 0);
    error("%s: can't exec %s: %m", options.tty, options.login);
    /* NOTREACHED */
}

/* parse-args - parse command-line arguments */

parse_args(argc, argv, op)
int     argc;
char  **argv;
struct options *op;
{
    extern char *optarg;		/* getopt */
    extern int optind;			/* getopt */
    int     c;

    while (isascii(c = getopt(argc, argv, "hil:mt:"))) {
	switch (c) {
	case 'h':				/* enable h/w flow control */
	    op->flags |= F_RTSCTS;
	    break;
	case 'i':				/* do not show /etc/issue */
	    op->flags &= ~F_ISSUE;
	    break;
	case 'l':
	    op->login = optarg;			/* non-default login program */
	    break;
	case 'm':				/* parse modem status message */
	    op->flags |= F_PARSE;
	    break;
	case 't':				/* time out */
	    if ((op->timeout = atoi(optarg)) <= 0)
		error("bad timeout value: %s", optarg);
	    break;
	default:
	    usage();
	}
    }
 	debug("after getopt loop\n");
    if (argc != optind + 2)			/* check parameter count */
	usage();

#ifdef linux
    /* while we use a BSD like init and ttytab */
    parse_speeds(op, argv[optind++]);		/* baud rate(s) */
    op->tty = argv[optind];			/* tty name */
#else    
#ifdef	SYSV_STYLE
    op->tty = argv[optind++];			/* tty name */
    parse_speeds(op, argv[optind]);		/* baud rate(s) */
#else
    parse_speeds(op, argv[optind++]);		/* baud rate(s) */
    op->tty = argv[optind];			/* tty name */
#endif
#endif /* linux */
	debug("exiting parseargs\n");
}

/* parse_speeds - parse alternate baud rates */

parse_speeds(op, arg)
struct options *op;
char   *arg;
{
    char   *strtok();
    char   *cp;

	debug("entered parse_speeds\n");
    for (cp = strtok(arg, ","); cp != 0; cp = strtok((char *) 0, ",")) {
	if ((op->speeds[op->numspeed++] = bcode(cp)) <= 0)
	    error("bad speed: %s", cp);
	if (op->numspeed > MAX_SPEED)
	    error("too many alternate speeds");
    }
    debug("exiting parsespeeds\n");
}

#ifdef	SYSV_STYLE

/* update_utmp - update our utmp entry */

update_utmp(line)
char   *line;
{
    struct utmp ut;
    long    ut_size = sizeof(ut);	/* avoid nonsense */
    int     ut_fd;
    int     mypid = getpid();
    long    time();
    long    lseek();
    char   *strncpy();

    /*
     * The utmp file holds miscellaneous information about things started by
     * /etc/init and other system-related events. Our purpose is to update
     * the utmp entry for the current process, in particular the process type
     * and the tty line we are listening to. Return successfully only if the
     * utmp file can be opened for update, and if we are able to find our
     * entry in the utmp file.
     */

#ifdef linux
	utmpname(_PATH_UTMP);
	(void) strncpy(ut.ut_user, "LOGIN", sizeof(ut.ut_user));
	(void) strncpy(ut.ut_line, line, sizeof(ut.ut_line));
	(void) strncpy(ut.ut_id, line + 3, sizeof(ut.ut_id));
	(void) time(&ut.ut_time);
	ut.ut_type = LOGIN_PROCESS;
	ut.ut_pid = mypid;

	pututline(&ut);
	endutent();
#else
    if ((ut_fd = open(UTMP_FILE, 2)) < 0) {
	error("%s: open for update: %m", UTMP_FILE);
    } else {
	while (read(ut_fd, (char *) &ut, sizeof(ut)) == sizeof(ut)) {
	    if (ut.ut_type == INIT_PROCESS && ut.ut_pid == mypid) {
		ut.ut_type = LOGIN_PROCESS;
		ut.ut_time = time((long *) 0);
		(void) strncpy(ut.ut_name, "LOGIN", sizeof(ut.ut_name));
		(void) strncpy(ut.ut_line, line, sizeof(ut.ut_line));
		(void) lseek(ut_fd, -ut_size, 1);
		(void) write(ut_fd, (char *) &ut, sizeof(ut));
		(void) close(ut_fd);
		return;
	    }
	}
	error("%s: no utmp entry", line);
    }
#endif /* linux */
}

#endif

/* open_tty - set up tty as standard { input, output, error } */

open_tty(tty, tp)
char   *tty;
struct termio *tp;
{
    /* Get rid of the present standard { output, error} if any. */

    (void) close(1);
    (void) close(2);
    errno = 0;					/* ignore above errors */

    /* Set up new standard input, unless we are given an already opened port. */

    if (strcmp(tty, "-")) {
	struct stat st;

	/* Sanity checks... */

	if (chdir("/dev"))
	    error("/dev: chdir() failed: %m");
	if (stat(tty, &st) < 0)
	    error("/dev/%s: %m", tty);
	if ((st.st_mode & S_IFMT) != S_IFCHR)
	    error("/dev/%s: not a character device", tty);

	/* Open the tty as standard input. */

	(void) close(0);
	errno = 0;				/* ignore close(2) errors */

	if (open(tty, O_RDWR, 0) != 0)
	    error("/dev/%s: cannot open as standard input: %m", tty);

    } else {

	/*
	 * Standard input should already be connected to an open port. Make
	 * sure it is open for read/write.
	 */

	if ((fcntl(0, F_GETFL, 0) & O_RDWR) != O_RDWR)
	    error("%s: not open for read/write", tty);
    }

    /* Set up standard output and standard error file descriptors. */

    if (dup(0) != 1 || dup(0) != 2)		/* set up stdout and stderr */
	error("%s: dup problem: %m", tty);	/* we have a problem */

    /*
     * The following ioctl will fail if stdin is not a tty, but also when
     * there is noise on the modem control lines. In the latter case, the
     * common course of action is (1) fix your cables (2) give the modem more
     * time to properly reset after hanging up. SunOS users can achieve (2)
     * by patching the SunOS kernel variable "zsadtrlow" to a larger value;
     * 5 seconds seems to be a good value.
     */

    if (ioctl(0, TCGETA, tp) < 0)
	error("%s: ioctl: %m", tty);

    /*
     * It seems to be a terminal. Set proper protections and ownership. Mode
     * 0622 is suitable for SYSV <4 because /bin/login does not change
     * protections. SunOS 4 login will change the protections to 0620 (write
     * access for group tty) after the login has succeeded.
     */

    (void) chown(tty, 0, 0);			/* root, sys */
    (void) chmod(tty, 0622);			/* crw--w--w- */
    errno = 0;					/* ignore above errors */
}

/* termio_init - initialize termio settings */

char gbuf[1024];
char area[1024];

termio_init(tp, speed)
struct termio *tp;
int     speed;
{

    /*
     * Initial termio settings: 8-bit characters, raw-mode, blocking i/o.
     * Special characters are set after we have read the login name; all
     * reads will be done in raw mode anyway. Errors will be dealt with
     * lateron.
     */
#ifdef linux
	/* flush input and output queues, important for modems! */
	(void) ioctl(0, TCFLSH, 2);
#endif

    tp->c_cflag = CS8 | HUPCL | CREAD | speed;
    tp->c_iflag = tp->c_lflag = tp->c_oflag = tp->c_line = 0;
    tp->c_cc[VMIN] = 1;
    tp->c_cc[VTIME] = 0;
    (void) ioctl(0, TCSETA, tp);
    debug("term_io 2\n");
}

/* auto_baud - extract baud rate from modem status message */

auto_baud(tp)
struct termio *tp;
{
    int     speed;
    int     vmin;
    unsigned iflag;
    char    buf[BUFSIZ];
    char   *bp;
    int     nread;

    /*
     * This works only if the modem produces its status code AFTER raising
     * the DCD line, and if the computer is fast enough to set the proper
     * baud rate before the message has gone by. We expect a message of the
     * following format:
     * 
     * <junk><number><junk>
     * 
     * The number is interpreted as the baud rate of the incoming call. If the
     * modem does not tell us the baud rate within one second, we will keep
     * using the current baud rate. It is advisable to enable BREAK
     * processing (comma-separated list of baud rates) if the processing of
     * modem status messages is enabled.
     */

    /*
     * Use 7-bit characters, don't block if input queue is empty. Errors will
     * be dealt with lateron.
     */

    iflag = tp->c_iflag;
    tp->c_iflag |= ISTRIP;			/* enable 8th-bit stripping */
    vmin = tp->c_cc[VMIN];
    tp->c_cc[VMIN] = 0;				/* don't block if queue empty */
    (void) ioctl(0, TCSETA, tp);

    /*
     * Wait for a while, then read everything the modem has said so far and
     * try to extract the speed of the dial-in call.
     */

    (void) sleep(1);
    if ((nread = read(0, buf, sizeof(buf) - 1)) > 0) {
	buf[nread] = '\0';
	for (bp = buf; bp < buf + nread; bp++) {
	    if (isascii(*bp) && isdigit(*bp)) {
		if (speed = bcode(bp)) {
		    tp->c_cflag &= ~CBAUD;
		    tp->c_cflag |= speed;
		}
		break;
	    }
	}
    }
    /* Restore terminal settings. Errors will be dealt with lateron. */

    tp->c_iflag = iflag;
    tp->c_cc[VMIN] = vmin;
    (void) ioctl(0, TCSETA, tp);
}

/* do_prompt - show login prompt, optionally preceded by /etc/issue contents */

do_prompt(op, tp)
struct options *op;
struct termio *tp;
{
#ifdef	ISSUE
    int     fd;
    int     oflag;
    int     n;
    char    buf[BUFSIZ];
#endif

    (void) write(1, "\r\n", 2);			/* start a new line */
#ifdef	ISSUE					/* optional: show /etc/issue */
    if ((op->flags & F_ISSUE) && (fd = open(ISSUE, 0)) >= 0) {
	oflag = tp->c_oflag;			/* save current setting */
	tp->c_oflag |= (ONLCR | OPOST);		/* map NL in output to CR-NL */
	(void) ioctl(0, TCSETAW, tp);
	while ((n = read(fd, buf, sizeof(buf))) > 0)
	    (void) write(1, buf, n);
	tp->c_oflag = oflag;			/* restore settings */
	(void) ioctl(0, TCSETAW, tp);		/* wait till output is gone */
	(void) close(fd);
    }
#endif
#ifdef linux
	{
		char hn[MAXHOSTNAMELEN+1];

		(void) gethostname(hn, MAXHOSTNAMELEN);
		write(1, hn, strlen(hn));
	}
#endif		
    (void) write(1, LOGIN, sizeof(LOGIN) - 1);	/* always show login prompt */
}

/* next_speed - select next baud rate */

next_speed(tp, op)
struct termio *tp;
struct options *op;
{
    static int baud_index = FIRST_SPEED;/* current speed index */

    baud_index = (baud_index + 1) % op->numspeed;
    tp->c_cflag &= ~CBAUD;
    tp->c_cflag |= op->speeds[baud_index];
    (void) ioctl(0, TCSETA, tp);
}

/* get_logname - get user name, establish parity, speed, erase, kill, eol */

char   *get_logname(op, cp, tp)
struct options *op;
struct chardata *cp;
struct termio *tp;
{
    char    logname[BUFSIZ];
    char   *bp;
    char    c;				/* input character, full eight bits */
    char    ascval;			/* low 7 bits of input character */
    int     bits;			/* # of "1" bits per character */
    int     mask;			/* mask with 1 bit up */
    static char *erase[] = {		/* backspace-space-backspace */
	"\010\040\010",			/* space parity */
	"\010\040\010",			/* odd parity */
	"\210\240\210",			/* even parity */
	"\210\240\210",			/* no parity */
    };

    /* Initialize kill, erase, parity etc. (also after switching speeds). */

    *cp = init_chardata;

    /* Flush pending input (esp. after parsing or switching the baud rate). */

    (void) sleep(1);
    (void) ioctl(0, TCFLSH, (struct termio *) 0);

    /* Prompt for and read a login name. */

    for (*logname = 0; *logname == 0; /* void */ ) {

	/* Write issue file and prompt, with "parity" bit == 0. */

	do_prompt(op, tp);

	/* Read name, watch for break, parity, erase, kill, end-of-line. */

	for (bp = logname, cp->eol = 0; cp->eol == 0; /* void */ ) {

	    /* Do not report trivial EINTR/EIO errors. */

	    if (read(0, &c, 1) < 1) {
		if (errno == EINTR || errno == EIO)
		    exit(0);
		error("%s: read: %m", op->tty);
	    }
	    /* Do BREAK handling elsewhere. */

	    if ((c == 0) && op->numspeed > 1)
		return (0);

	    /* Do parity bit handling. */

	    if (c != (ascval = (c & 0177))) {	/* "parity" bit on ? */
		for (bits = 1, mask = 1; mask & 0177; mask <<= 1)
		    if (mask & ascval)
			bits++;			/* count "1" bits */
		cp->parity |= ((bits & 1) ? 1 : 2);
	    }
	    /* Do erase, kill and end-of-line processing. */

	    switch (ascval) {
	    case CR:
	    case NL:
		*bp = 0;			/* terminate logname */
		cp->eol = ascval;		/* set end-of-line char */
		break;
	    case BS:
	    case DEL:
	    case '#':
		cp->erase = ascval;		/* set erase character */
		if (bp > logname) {
		    (void) write(1, erase[cp->parity], 3);
		    bp--;
		}
		break;
	    case CTL('U'):
	    case '@':
		cp->kill = ascval;		/* set kill character */
		while (bp > logname) {
		    (void) write(1, erase[cp->parity], 3);
		    bp--;
		}
		break;
	    case CTL('D'):
		exit(0);
	    default:
		if (!isascii(ascval) || !isprint(ascval)) {
		     /* ignore garbage characters */ ;
		} else if (bp - logname >= sizeof(logname) - 1) {
		    error("%s: input overrun", op->tty);
		} else {
		    (void) write(1, &c, 1);	/* echo the character */
		    *bp++ = ascval;		/* and store it */
		}
		break;
	    }
	}
    }
    /* Handle names with upper case and no lower case. */

    if (cp->capslock = caps_lock(logname)) {
	for (bp = logname; *bp; bp++)
	    if (isupper(*bp))
		*bp = tolower(*bp);		/* map name to lower case */
    }
    return (logname);
}

/* termio_final - set the final tty mode bits */

termio_final(op, tp, cp)
struct options *op;
struct termio *tp;
struct chardata *cp;
{
    /* General terminal-independent stuff. */

    tp->c_iflag |= IXON | IXOFF;		/* 2-way flow control */
    tp->c_lflag |= ICANON | ISIG | ECHO | ECHOKE | ECHOCTL | ECHOPRT;
    tp->c_oflag |= OPOST;
    /* tp->c_cflag = 0; */
    tp->c_cc[VINTR] = DEF_INTR;			/* default interrupt */
    tp->c_cc[VQUIT] = DEF_QUIT;			/* default quit */
    tp->c_cc[VEOF] = DEF_EOF;			/* default EOF character */
    tp->c_cc[VEOL] = DEF_EOL;
#ifdef linux
    tp->c_cc[VSWTC] = DEF_SWITCH;		/* default switch character */
#else
    tp->c_cc[VSWTCH] = DEF_SWITCH;		/* default switch character */
#endif

    /* Account for special characters seen in input. */

    if (cp->eol == CR) {
	tp->c_iflag |= ICRNL;			/* map CR in input to NL */
	tp->c_oflag |= ONLCR;			/* map NL in output to CR-NL */
    }
    tp->c_cc[VERASE] = cp->erase;		/* set erase character */
    tp->c_cc[VKILL] = cp->kill;			/* set kill character */

    /* Account for the presence or absence of parity bits in input. */

    switch (cp->parity) {
    case 0:					/* space (always 0) parity */
	break;
    case 1:					/* odd parity */
	tp->c_cflag |= PARODD;
	/* FALLTHROUGH */
    case 2:					/* even parity */
	tp->c_cflag |= PARENB;
	tp->c_iflag |= INPCK | ISTRIP;
	/* FALLTHROUGH */
    case (1 | 2):				/* no parity bit */
	tp->c_cflag &= ~CSIZE;
	tp->c_cflag |= CS7;
	break;
    }
    /* Account for upper case without lower case. */

    if (cp->capslock) {
	tp->c_iflag |= IUCLC;
	tp->c_lflag |= XCASE;
	tp->c_oflag |= OLCUC;
    }
    /* Optionally enable hardware flow control */

#ifdef	CRTSCTS
    if (op->flags & F_RTSCTS)
	tp->c_cflag |= CRTSCTS;
#endif

    /* Finally, make the new settings effective */

    if (ioctl(0, TCSETA, tp) < 0)
	error("%s: ioctl: TCSETA: %m", op->tty);
}

/* caps_lock - string contains upper case without lower case */

caps_lock(s)
char   *s;
{
    int     capslock;

    for (capslock = 0; *s; s++) {
	if (islower(*s))
	    return (0);
	if (capslock == 0)
	    capslock = isupper(*s);
    }
    return (capslock);
}

/* bcode - convert speed string to speed code; return 0 on failure */

bcode(s)
char   *s;
{
    struct Speedtab {
	long    speed;
	int     code;
    };
    static struct Speedtab speedtab[] = {
	50, B50,
	75, B75,
	110, B110,
	134, B134,
	150, B150,
	200, B200,
	300, B300,
	600, B600,
	1200, B1200,
	1800, B1800,
	2400, B2400,
	4800, B4800,
	9600, B9600,
#ifdef	B19200
	19200, B19200,
#endif
#ifdef	B38400
	38400, B38400,
#endif
#ifdef	EXTA
	19200, EXTA,
#endif
#ifdef	EXTB
	38400, EXTB,
#endif
	0, 0,
    };
    struct Speedtab *sp;
    long    atol();
    long    speed = atol(s);

    for (sp = speedtab; sp->speed; sp++)
	if (sp->speed == speed)
	    return (sp->code);
    return (0);
}

/* usage - explain */

usage()
{
#if defined(SYSV_STYLE) && !defined(linux)
    static char msg[] =
    "[-i] [-l login_program] [-m] [-t timeout] line baud_rate,...";
#else
    static char msg[] =
    "[-h] [-l login_program] [-m] [-t timeout] baud_rate,... line";
#endif

    error("usage: %s %s", progname, msg);
}

/* error - report errors to console or syslog; only understands %s and %m */

#define	str2cpy(b,s1,s2)	strcat(strcpy(b,s1),s2)

/* VARARGS */

error(va_alist)
va_dcl
{
    va_list ap;
    char   *fmt;
#ifndef	USE_SYSLOG
    int     fd;
#endif
    char    buf[BUFSIZ];
    char   *bp;
    extern char *sys_errlist[];
    char   *strcpy();
    char   *strcat();

    /*
     * If the diagnostic is reported via syslog(3), the process name is
     * automatically prepended to the message. If we write directly to
     * /dev/console, we must prepend the process name ourselves.
     */

#ifdef USE_SYSLOG
    buf[0] = '\0';
    bp = buf;
#else
    (void) str2cpy(buf, progname, ": ");
    bp = buf + strlen(buf);
#endif

    /*
     * %s expansion is done by hand. On a System V Release 2 system without
     * shared libraries and without syslog(3), linking with the the stdio
     * library would make the program three times as big...
     *
     * %m expansion is done here as well. Too bad syslog(3) does not have a
     * vsprintf() like interface.
     */

    va_start(ap);
    fmt = va_arg(ap, char *);
    while (*fmt) {
	if (strncmp(fmt, "%s", 2) == 0) {
	    (void) strcpy(bp, va_arg(ap, char *));
	    bp += strlen(bp);
	    fmt += 2;
	} else if (strncmp(fmt, "%m", 2) == 0) {
	    (void) strcpy(bp, sys_errlist[errno]);
	    bp += strlen(bp);
	    fmt += 2;
	} else {
	    *bp++ = *fmt++;
	}
    }
    *bp = 0;
    va_end(ap);

    /*
     * Write the diagnostic directly to /dev/console if we do not use the
     * syslog(3) facility.
     */

#ifdef	USE_SYSLOG
    (void) openlog(progname, LOG_PID, LOG_AUTH);
    (void) syslog(LOG_ERR, "%s", buf);
    closelog();
#else
    /* Terminate with CR-LF since the console mode is unknown. */
    (void) strcat(bp, "\r\n");
    if ((fd = open("/dev/console", 1)) >= 0) {
	(void) write(fd, buf, strlen(buf));
	(void) close(fd);
    }
#endif
    (void) sleep((unsigned) 10);			/* be kind to init(8) */
    exit(1);
}
