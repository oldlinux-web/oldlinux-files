/*
**	$Id: funcs.c,v 2.0 90/09/19 19:53:19 paul Rel $
**
**	Miscellaneous routines.
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
**	$Log:	funcs.c,v $
**	Revision 2.0  90/09/19  19:53:19  paul
**	Initial 2.0 release
**	
*/


#include "getty.h"
#include "table.h"
#include <ctype.h>
#ifdef	I_TIME
#include <time.h>
#endif	/* I_TIME */
#ifdef	I_SYSTIME
#include <sys/time.h>
#endif	/* I_SYSTIME */
#ifdef	DOUNAME
#include <sys/utsname.h>
#endif	/* DOUNAME */
#include <setjmp.h>
#include <signal.h>

#if defined(RCSID) && !defined(lint)
static char *RcsId =
"@(#)$Id: funcs.c,v 2.0 90/09/19 19:53:19 paul Rel $";
#endif

#ifndef	MAXBUF
#define	MAXBUF	512	/* buffer size */
#endif	/* MAXBUF */

#ifndef	EXPFAIL
#define	EXPFAIL	30	/* default num seconds to wait for expected input */
#endif	/* EXPFAIL */

#define	EXPECT	0	/* states for chat() */
#define	SEND	1

/* SMR - gcc uses signed characters, so I cast the following */
#define	AUTOBD	((char)0376)	/* marker for AutoBaud digits */

char	*unquote();
int	expect(), send();
boolean	expmatch();
sig_t	expalarm();


/*
**	Fputs() - does fputs() with '\' and '@' expansion
**
**	Returns EOF if an error occurs.
*/

int
Fputs(s, stream)
register char *s;
register FILE *stream;
{
	char c, n, tbuf[16], ubuf[32];
	time_t clock;
	struct tm *lt, *localtime();
	char *month_name[] = { "January", "February", "March", "April",
			       "May", "June", "July", "August", "September",
			       "October", "November", "December" };

	while (c = *s++) {
		if ((c == '@') && (n = *s++)) {
			switch (n) {
			case 'B':	/* speed (baud rate) */
				if (*Speed && Fputs(Speed, stream) == EOF)
					return(EOF);
				break;
			case 'D':	/* date */
				(void) time(&clock);
				lt = localtime(&clock);
				(void) sprintf(tbuf, "%d %s %02d",
						lt->tm_mday,
						month_name[lt->tm_mon],
						lt->tm_year);
				if (Fputs(tbuf, stream) == EOF)
					return(EOF);
				break;
			case 'L':	/* line */
				if (*Device && Fputs(Device, stream) == EOF)
					return(EOF);
				break;
			case 'S':	/* system node name */
				if (*SysName && Fputs(SysName, stream) == EOF)
					return(EOF);
				break;
#ifdef	M_XENIX
			/* Special case applys here: SCO XENIX's
			 * /etc/gettydefs file has "\r\n@!login: " as
			 * the login field value, and replaces the "@"
			 * with the system node name.  This will do
			 * the same thing.
			 */
			case '!':
				if (*SysName && Fputs(SysName, stream) == EOF)
					return(EOF);
				(void) fputc(n, stream);
				break;
#endif	/* M_XENIX */
			case 'T':	/* time */
				(void) time(&clock);
				lt = localtime(&clock);
				(void) sprintf(tbuf, "%02d:%02d:%02d",
						lt->tm_hour,
						lt->tm_min, lt->tm_sec);
				if (Fputs(tbuf, stream) == EOF)
					return(EOF);
				break;
			case 'U':	/* number of active users */
				(void) sprintf(ubuf, "%d", Nusers);
				if (Fputs(ubuf, stream) == EOF)
					return(EOF);
				break;
			case 'V':	/* version */
				if (*Version && Fputs(Version, stream) == EOF)
					return(EOF);
				break;
			case '@':	/* in case '@@' was used */
				if (fputc(n, stream) == EOF)
					return(EOF);
				break;
			}
		} else {
			if (c == '\\')
				s = unquote(s, &c);
			/* we're in raw mode: send CR before every LF
			 */
			if (c == '\n' && (fputc('\r', stream) == EOF))
				return(EOF);
			if (c && fputc(c, stream) == EOF)
				return(EOF);
		}
	}
	return(SUCCESS);
}


/*
**	getuname() - retrieve the system's node name
**
**	Returns pointer to name or a zero-length string if not found.
*/

char *
getuname()
{
#ifdef	HOSTNAME			/* hardwire the name */

	static char name[] = HOSTNAME;

	return(name);

#else	/* HOSTNAME */

#ifdef	M_XENIX
#define	SYSTEMID "/etc/systemid"
	static FILE *fp;
#endif	/* M_XENIX */

	struct utsname uts;
	static char name[80];

	name[0] = '\0';

#ifdef	DOUNAME				/* dig it out of the kernel */

	if (uname(&uts) != FAIL)
		(void) strcpy(name, uts.nodename);

#endif	/* DOUNAME */

#ifdef	M_XENIX				/* if Xenix's uts.nodename is empty */
	if (strlen(name) == 0) {
		if ((fp = fopen(SYSTEMID, "r")) != (FILE *) NULL) {
			(void) fgets(name, sizeof(name), fp);
			(void) fclose(fp);
			name[strlen(name)-1] = '\0';
		}
	}
#endif	/* M_XENIX */

#ifdef	PHOSTNAME			/* get it from the shell */

	if (strlen(name) == 0) {
		FILE *cmd;
		if ((cmd = popen(PHOSTNAME, "r")) != (FILE *) NULL) {
			(void) fgets(name, sizeof(name), cmd);
			(void) pclose(cmd);
			name[strlen(name)-1] = '\0';
		}
	}

#endif	/* PHOSTNAME */

	return(name);

#endif	/* HOSTNAME */
}


/*
**	settermio() - setup tty according to termio values
*/

void
settermio(termio, state)
register TERMIO *termio;
int state;
{
	register int i;
	static TERMIO setterm;

#ifdef	TRS16
	/* Tandy 16/6000 console's BREAK key sends ^C
	 */
	char Cintr = (strequal(Device, "console")) ? '\003' : CINTR;
#else
	char Cintr = CINTR;
#endif	/* TRS16 */

#ifdef	MY_ERASE
	char Cerase = MY_ERASE;
#else
	char Cerase = CERASE;
#endif	/* MY_ERASE */

#ifdef	MY_KILL
	char Ckill = MY_KILL;
#else
	char Ckill = CKILL;
#endif	/* MY_KILL */

	(void) ioctl(STDIN, TCGETA, &setterm);

	switch (state) {
	case INITIAL:
		setterm.c_iflag = termio->c_iflag;
		setterm.c_oflag = termio->c_oflag;
		setterm.c_cflag = termio->c_cflag;
		setterm.c_lflag = termio->c_lflag;
		setterm.c_line  = termio->c_line;

		/* single character processing
		 */
		setterm.c_lflag &= ~(ICANON);
		setterm.c_cc[VMIN] = 1;
		setterm.c_cc[VTIME] = 0;

		/* sanity check
		 */
		if ((setterm.c_cflag & CBAUD) == 0)
			setterm.c_cflag |= B9600;
		if ((setterm.c_cflag & CSIZE) == 0)
			setterm.c_cflag |= DEF_CFL;
		setterm.c_cflag |= (CREAD | HUPCL);

		(void) ioctl(STDIN, TCSETAF, &setterm);
		break;

	case FINAL:
		setterm.c_iflag = termio->c_iflag;
		setterm.c_oflag = termio->c_oflag;
		setterm.c_cflag = termio->c_cflag;
		setterm.c_lflag = termio->c_lflag;
		setterm.c_line  = termio->c_line;

		/* sanity check
		 */
		if ((setterm.c_cflag & CBAUD) == 0)
			setterm.c_cflag |= B9600;
		if ((setterm.c_cflag & CSIZE) == 0)
			setterm.c_cflag |= DEF_CFL;
		setterm.c_cflag |= CREAD;

		/* set c_cc[] chars to reasonable values
		 */
		for (i=0; i < NCC; i++)
			setterm.c_cc[i] = CNUL;
		setterm.c_cc[VINTR] = Cintr;
		setterm.c_cc[VQUIT] = CQUIT;
		setterm.c_cc[VERASE] = Cerase;
		setterm.c_cc[VKILL] = Ckill;
		setterm.c_cc[VEOF] = CEOF;
#ifdef	CEOL
		setterm.c_cc[VEOL] = CEOL;
#endif	/* CEOL */

/*
 *  SMR - Linux does funny things if VMIN is zero (like more doesn't work),
 *        so I put it to one here.
 */
		setterm.c_cc[VMIN] = 1;

		(void) ioctl(STDIN, TCSETAW, &setterm);
		break;

	}
}


/*
**	chat() - handle expect/send sequence to Device
**
**	Returns FAIL if an error occurs.
*/

int
chat(s)
char *s;
{
	register int state = EXPECT;
	boolean finished = FALSE, if_fail = FALSE;
	char c, *p;
	char word[MAXLINE+1];		/* buffer for next word */

	debug3(D_INIT, "chat(%s) called\n", s);

	while (!finished) {
		p = word;
		while (((c = (*s++ & 0177)) != '\0') && c != ' ' && c != '-')
			/*
			 *  SMR - I don't understand this, because if c is \0
			 *  then it is 0, isn't it?  If so we end the loop and
			 *  terminate the word anyway.
			 *
			*p++ = (c) ? c : '\177';
			 */
			*p++ = c;

		finished = (c == '\0');
		if_fail = (c == '-');
		*p = '\0';

		switch (state) {
		case EXPECT:
			if (expect(word) == FAIL) {
				if (if_fail == FALSE)
					return(FAIL);	/* no if-fail seq */
			} else {
				/* eat up rest of current sequence
				 */
				if (if_fail == TRUE) {
					while ((c = (*s++ & 0177)) != '\0' &&
						c != ' ')
						;
					if (c == '\0')
						finished = TRUE;
					if_fail = FALSE;
				}
			}
			state = SEND;
			break;
		case SEND:
			if (send(word) == FAIL)
				return(FAIL);
			state = EXPECT;
			break;
		}
		continue;
	}
	debug2(D_INIT, "chat() successful\n");
	return (SUCCESS);
}


/*
**	unquote() - decode char(s) after a '\' is found.
**
**	Returns the pointer s; decoded char in *c.
*/

char	valid_oct[] = "01234567";
char	valid_dec[] = "0123456789";
char	valid_hex[] = "0123456789aAbBcCdDeEfF";

char *
unquote(s, c)
char *s, *c;
{
	int value, base;
	char n, *valid;

	n = *s++;
	switch (n) {
	case 'b':
		*c = '\b';	break;
	case 'c':
		if ((n = *s++) == '\n')
			*c = '\0';
		else
			*c = n;
		break;
	case 'f':
		*c = '\f';	break;
	case 'n':
		*c = '\n';	break;
	case 'r':
		*c = '\r';	break;
	case 's':
		*c = ' ';	break;
	case 't':
		*c = '\t';	break;
	case '\n':
		*c = '\0';	break;	/* ignore NL which follows a '\' */
	case '\\':
		*c = '\\';	break;	/* '\\' will give a single '\' */
	default:
		if (isdigit(n)) {
			value = 0;
			if (n == '0') {
				if (*s == 'x') {
					valid = valid_hex;
					base = 16;
					s++;
				} else {
					valid = valid_oct;
					base = 8;
				}
			} else {
				valid = valid_dec;
				base = 10;
				s--;
			}
			while (strpbrk(s, valid) == s) {
				value = (value * base) + (int) (n - '0');
				s++;
			}
			*c = (char) (value & 0377);
		} else {
			*c = n;
		}
		break;
	}
	return(s);
}


/*
**	send() - send a string to stdout
*/

int
send(s)
register char *s;
{
	register int retval = SUCCESS;
	char ch;

	debug2(D_INIT, "SEND: (");

	if (strequal(s, "\"\"")) {	/* ("") used as a place holder */
		debug2(D_INIT, "[nothing])\n");
		return(retval);
	}

	while (ch = *s++) {
		if (ch == '\\') {
			switch (*s) {
			case 'p':		/* '\p' == pause */
				debug2(D_INIT, "[pause]");
				(void) sleep(1);
				s++;
				continue;
			case 'd':		/* '\d' == delay */
				debug2(D_INIT, "[delay]");
				(void) sleep(2);
				s++;
				continue;
			case 'K':		/* '\K' == BREAK */
				debug2(D_INIT, "[break]");
				(void) ioctl(STDOUT, TCSBRK, 0);
				s++;
				continue;
			default:
				s = unquote(s, &ch);
				break;
			}
		}
		debug3(D_INIT, ((ch < ' ') ? "^%c" : "%c"),
			       ((ch < ' ') ? ch | 0100 : ch));
		if (write(STDOUT, &ch, 1) == FAIL) {
			retval = FAIL;
			break;
		}
	}
	debug3(D_INIT, ") -- %s\n", (retval == SUCCESS) ? "OK" : "Failed");
	return(retval);
}


/*
**	expect() - look for a specific string on stdin
*/

jmp_buf env;	/* here so expalarm() sees it */

int
expect(s)
register char *s;
{
	register int i;
	register int expfail = EXPFAIL;
	register retval = FAIL;
	char ch, *p, word[MAXLINE+1], buf[MAXBUF];
	sig_t (*oldalarm)();

	if (strequal(s, "\"\"")) {	/* ("") used as a place holder */
		debug2(D_INIT, "EXPECT: ([nothing])\n");
		return(SUCCESS);
	}

#ifdef	lint
	/* shut lint up about 'warning: oldalarm may be used before set' */
	oldalarm = signal(SIGALRM, SIG_DFL);
#endif	/* lint */

	/* look for escape chars in expected word
	 */
	for (p = word; ch = (*s++ & 0177);) {
		if (ch == '\\') {
			if (*s == 'A') {	/* spot for AutoBaud digits */
				*p++ = AUTOBD;
				s++;
				continue;
			} else if (*s == 'T') {	/* change expfail timeout */
				if (isdigit(*++s)) {
					s = unquote(s, &ch);
					/* allow 3 - 255 second timeout */
					if ((expfail = (int) ch) < 3)
						expfail = 3;
				}
				continue;
			} else
				s = unquote(s, &ch);
		}
		*p++ = (ch) ? ch : '\177';
	}
	*p = '\0';

	if (setjmp(env)) {	/* expalarm returns non-zero here */
		debug3(D_INIT, "[timed out after %d seconds]\n", expfail);
		(void) signal(SIGALRM, oldalarm);
		return(FAIL);
	}

	oldalarm = signal(SIGALRM, expalarm);
	(void) alarm((unsigned) expfail);

	debug3(D_INIT, "EXPECT: <%d> (", expfail);
	debug1(D_INIT, word);
	debug2(D_INIT, "), GOT:\n");
	p = buf;
	while (read(STDIN, &ch, 1) == 1) {
		debug3(D_INIT, ((ch < ' ') ? "^%c" : "%c"),
			       ((ch < ' ') ? ch | 0100 : ch));
		*p++ = (char) ((int) ch & 0177);
		*p = '\0';
		if (strlen(buf) >= strlen(word)) {
			for (i=0; buf[i]; i++)
				if (expmatch(&buf[i], word)) {
					retval = SUCCESS;
					break;
				}
		}
		if (retval == SUCCESS)
			break;
	}
	(void) alarm((unsigned) 0);
	(void) signal(SIGALRM, oldalarm);
	debug3(D_INIT, " -- %s\n", (retval == SUCCESS) ? "got it" : "Failed");
	return(retval);
}


/*
**	expmatch() - compares expected string with the one gotten
*/

#ifdef	TELEBIT
char	valid[] = "0123456789FAST";
#else	/* TELEBIT */
char	valid[] = "0123456789";
#endif	/* TELEBIT */

boolean
expmatch(got, exp)
register char *got;
register char *exp;
{
	register int ptr = 0;

	while (*exp) {
		if (*exp == AUTOBD) {	/* substitute real digits gotten */
			while (*got && strpbrk(got, valid) == got) {
				AutoBaud = TRUE;
				if (ptr < (sizeof(AutoRate) - 2))
					AutoRate[ptr++] = *got;
				got++;
			}
			if (*got == '\0')
				return(FALSE);	/* didn't get it all yet */
			AutoRate[ptr] = '\0';
			exp++;
			continue;
		}
		if (*got++ != *exp++)
			return(FALSE);		/* no match */
	}
	return(TRUE);
}


/*
**	expalarm() - called when expect()'s SIGALRM goes off
*/

sig_t
expalarm()
{
	longjmp(env, 1);
}


/*
**	getlogname() - get the users login response
**
**	Returns int value indicating success.
*/

int
getlogname(termio, name, size)
TERMIO *termio;
register char *name;
int size;
{
	register int count;
	register int lower = 0;
	register int upper = 0;
	char ch, *p;
	ushort lflag;

#ifdef	MY_ERASE
	char Erase = MY_ERASE;
#else
	char Erase = CERASE;
#endif	/* MY_ERASE */
#ifdef	MY_KILL
	char Kill = MY_KILL;
#else
	char Kill = CKILL;
#endif	/* MY_KILL */

	debug2(D_GETL, "getlogname() called\n");
	/*
	 *  SMR - Linux didn't flush the line in 0.95; this isn't necessary
	 *        with 0.96a, but it shouldn't hurt.
	 */
	fflush(stdout);

	debug2(D_STTY, "Getlogname:\n");
	if (Debug & D_STTY) system("stty -a >&3");

	(void) ioctl(STDIN, TCGETA, termio);
	lflag = termio->c_lflag;

	termio->c_iflag = 0;
	termio->c_oflag = 0;
	termio->c_cflag = 0;
	termio->c_lflag = 0;

	p = name;	/* point to beginning of buffer */
	count = 0;	/* nothing entered yet */

	do {
		if (read(STDIN, &ch, 1) != 1)	/* nobody home */
			exit(0);
		debug4(D_GETL, "Character entered = %c (%d).\n", ch, ch);
		if ((ch = (char) ((int) ch & 0177)) == CEOF)
			if (p == name)		/* ctrl-d was first char */
				exit(0);
		if (ch == CQUIT)		/* user wanted out, i guess */
			exit(0);
		if (ch == '\0') {
			debug2(D_GETL, "returned (BADSPEED)\n");
			return(BADSPEED);
		}
		if (!(lflag & ECHO)) {
			(void) putc(ch, stdout);
			(void) fflush(stdout);
		}
		if (ch == Erase) {
			if (count) {
				if (!(lflag & ECHOE)) {
					(void) fputs(" \b", stdout);
					(void) fflush(stdout);
				}
				--p;
				--count;
			}
		} else if (ch == Kill) {
			if (!(lflag & ECHOK)) {
				(void) fputs("\r\n", stdout);
				(void) fflush(stdout);
			}
			p = name;
			count = 0;
		} else {
			*p++ = ch;
			count++;
			if (islower(ch))
				lower++;
			if (isupper(ch))
				upper++;
		}
	} while ((ch != '\n') && (ch != '\r') && (count < size));

	*(--p) = '\0';	/* terminate buffer */

	if (ch == '\r') {
		(void) putc('\n', stdout);
		(void) fflush(stdout);
		termio->c_iflag |= ICRNL;	/* turn on cr/nl xlate */
		termio->c_oflag |= ONLCR;
	} else if (ch == '\n') {
		(void) putc('\r', stdout);
		(void) fflush(stdout);
	}

	if (strlen(name) == 0) {
		debug2(D_GETL, "returned (NONAME)\n");
		return(NONAME);
	}

	if (upper && !lower) {
#ifdef	WARNCASE
		if (WarnCase) {
			WarnCase = FALSE;
			debug2(D_GETL, "returned (BADCASE)\n");
			return(BADCASE);
		}
#endif	/* WARNCASE */
		for (p=name; *p; p++)		/* make all chars UC */
			*p = toupper(*p);
		termio->c_iflag |= IUCLC;
		termio->c_oflag |= OLCUC;
		termio->c_lflag |= XCASE;
	}

	debug3(D_GETL, "returned (SUCCESS), name=(%s)\n", name);
	return(SUCCESS);
}


/*
**	logerr() - display an error message
*/

void
logerr(msg)
register char *msg;
{
	register FILE *co;
	char *errdev;
	time_t clock;

	errdev = (Check) ? "/dev/tty" : CONSOLE;

	time (&clock);
	if ((co = fopen(errdev, "w")) != (FILE *) NULL) {
		(void) fprintf(co, "\r\n%s - %s (%s): %s\r\n", 
			asctime(localtime(&clock)), MyName, Device, msg);
		(void) fclose(co);
	}

#ifdef	TRYMAIL
	else {
		char buf[MAXLINE];
		FILE *popen();

		(void) sprintf(buf, "%s %s", MAILER, NOTIFY);
		if ((co = popen(buf, "w")) != (FILE *) NULL) {
			(void) fprintf(co, "To: %s\n", NOTIFY);
			(void) fprintf(co, "Subject: %s problem\n\n", MyName);
			(void) fprintf(co, "%s: %s\n", Device, msg);
			(void) pclose(co);
		}
	}
#endif	/* TRYMAIL */

}


#ifdef	DEBUG

/*
**	debug() - an fprintf to the debug file
**
**	Only does the output if the requested level is "set."
*/

#ifdef	VARARGS

#include <varargs.h>

/*VARARGS2*/
void
debug(lvl, fmt, va_alist)
int lvl;
char *fmt;
va_dcl
{
	va_list args;

	va_start(args);
	if (Debug & lvl) {
		(void) vfprintf(Dfp, fmt, args);
		(void) fflush(Dfp);
	}
	va_end(args);
}

#else	/* VARARGS */

/*VARARGS2*/
void
debug(lvl, fmt, arg1, arg2, arg3, arg4)
int lvl;
char *fmt;
{
	if (Debug & lvl) {
		(void) fprintf(Dfp, fmt, arg1, arg2, arg3, arg4);
		(void) fflush(Dfp);
	}
}

#endif	/* VARARGS */

/*
**	dprint() - like debug(), but shows control chars
*/

void
dprint(lvl, word)
int lvl;
char *word;
{
	char *p, *fmt, ch;

	if (Debug & lvl) {
		p = word;
		while (ch = *p++) {
			if (ch == AUTOBD) {
				(void) fputs("[speed]", Dfp);
				(void) fflush(Dfp);
				continue;
			} else if (ch < ' ') {
				fmt = "^%c";
				ch = ch | 0100;
			} else {
				fmt = "%c";
			}
			(void) fprintf(Dfp, fmt, ch);
		}
		(void) fflush(Dfp);
	}
}

#endif	/* DEBUG */


/* end of funcs.c */
