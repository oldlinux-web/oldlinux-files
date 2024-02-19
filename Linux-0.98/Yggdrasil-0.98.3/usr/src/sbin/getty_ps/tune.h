/*
**	$Id: tune.H,v 2.0 90/09/19 20:19:47 paul Rel $
**
**	Getty tuneable parameters.
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
**	$Log:	tune.H,v $
**	Revision 2.0  90/09/19  20:19:47  paul
**	Initial 2.0 release
**	
*/


#define	boolean	 int			/* does your cc know about boolean? */

#define	DEF_CFL	 (CS8)			/* default word-len/parity */

#define	DEF_CONNECT  "CONNECT\\s\\A\r\n"	/* default CONNECT string */


/*  Feature selection
 */

#define	DEBUG				/* include debugging code */
#define	LOGUTMP				/* need to update utmp/wtmp files */
#undef	MY_CANON			/* use my own ERASE and KILL chars */
#define	RCSID				/* include RCS ID info in objects */
#undef	SETTERM				/* need to set TERM in environment */
#undef	TELEBIT				/* include Telebit FAST parsing */
#define	TRYMAIL				/* mail errors if CONSOLE unavailable */
#define	WARNCASE			/* warn user if login is UPPER case */

/*  define your ERASE and KILL characters here
 */
#ifdef	MY_CANON
#define	MY_ERASE '\010'			/* 010 = ^H, backspace */
#define	MY_KILL	 '\025'			/* 025 = ^U, nak */
#endif

/*  define your Telebit FAST speed here
 */
#ifdef	TELEBIT
#define	TB_FAST	 "19200"		/* CONNECT FAST == this speed */
#endif	/* TELEBIT */

/*  who should be notified of errors?
 */
#ifdef	TRYMAIL
#define	NOTIFY	 "root"
#endif


/*  Where to find things
 */

#define	CONSOLE	 "/dev/console"		/* place to log errors */
#define	DEFAULTS "/etc/default/%s"	/* name of defaults file */
#define	ISSUE	 "/etc/issue"		/* name of the issue file;
					   say "#undef ISSUE" to turn off
					   the issue feature */
#define	LOGIN	 "/bin/login"		/* name of login program */


/*  Special cases
 */

#undef	TRS16				/* you are a Tandy 6000 or equivilent */


/*  You probably shouldn't fool with these
 */

#define	MAXDEF	 100			/* max # lines in defaults file */
#define	MAXLINE	 256			/* max # chars in a line */
#define	MAXID	 12			/* max # chars in Gtab Id */
#define	MAXLOGIN 80			/* max # chars in Gtab Login */


/* end of tune.h */
