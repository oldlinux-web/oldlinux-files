/*
**	$Id: extern.h,v 2.0 90/09/19 19:48:33 paul Rel $
**
**	Defines all external values.
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
**	$Log:	extern.h,v $
**	Revision 2.0  90/09/19  19:48:33  paul
**	Initial 2.0 release
**	
*/


#ifdef	MAIN			/* define as "extern", except for MAIN,	*/
#define EXTERN			/* which is defined only in main.c	*/
#else
#define EXTERN	extern
#endif	/* MAIN */


/*	Global variables
 */

#ifdef	MAIN
EXTERN	STDCHAR	MsgBuf[80];	/* message buffer */
#else
EXTERN	STDCHAR	MsgBuf[];
#endif	/* MAIN */

EXTERN	boolean	AutoBaud;	/* autobauding requested? */
EXTERN	char	AutoRate[16];	/* AutoBaud digits buffer */
EXTERN	boolean	Check;		/* check a gettytab file? */
EXTERN	char	*CheckFile;	/* gettytab-like file to check */
EXTERN	char	*Device;	/* controlling line (minus "/dev/") */
EXTERN	char	*GtabId;	/* current gettytab id */
EXTERN	boolean	NoHangUp;	/* don't hangup line before setting speed */
EXTERN	char	*LineD;		/* line discipline */
EXTERN	char	*MyName;	/* this program name */
EXTERN	int	Nusers;		/* number of users currently logged in */
EXTERN	char	*Speed;		/* current baud rate (string literal) */
EXTERN	char	*SysName;	/* nodename of system */
EXTERN	int	TimeOut;	/* timeout value from command line */
EXTERN	char	*Version;	/* value of VERSION */

#ifdef	WARNCASE
EXTERN	boolean	WarnCase;	/* controls display of bad case message */
#endif	/* WARNCASE */

#ifdef	DEBUG
EXTERN	int	Debug;		/* debug value from command line */
EXTERN	FILE	*Dfp;		/* debug output file pointer */
#endif	/* DEBUG */


/*	System routines
 */

#ifndef __STDC__
extern	int	fputc();
extern	char	*malloc(), *ttyname();
extern	unsigned alarm(), sleep();
extern	time_t	time();
#endif

#ifndef	STRDUP			/* Is There In Truth No Strdup() ? */
extern	char	*strdup();
#endif	/* STRDUP */

#ifndef	GETUTENT		/* How about getutent() ? */
extern	struct utmp	*getutent();
extern	void		setutent(), endutent();
#endif	/* GETUTENT */

#ifndef	PUTENV			/* putenv() ? */
extern	int	putenv();
#endif	/* PUTENV */

/* end of extern.h */
