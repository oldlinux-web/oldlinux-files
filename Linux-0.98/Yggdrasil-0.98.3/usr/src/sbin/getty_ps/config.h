/*
**	config.h
**
**	Getty configuration.
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


#include "tune.h"			/* defs needed below */


/*  These are set by config.sh.
 *  If you change them here, they will be reset
 *  the next time you run Configure.
 */

#define	PORTABLE		/* compile for more than one site */
#define	GETUTENT		/* we have getutent() and friends */
#define	STRDUP			/* we have strdup() */
#define	PUTENV			/* we have putenv() */

#undef	TTYTYPE   "x"	/* file used to identify terminals */
#define	GETTYTAB  "/etc/gettydefs"	/* file used for speed/termio table */

#define	STDCHAR   char	/* signed or unsigned chars in stdio */
#define	UIDTYPE	  uid_t	/* storage type of UID's */
#define	GIDTYPE	  gid_t	/* storage type of GID's */

#define	FCNTL			/* include fcntl.h? */
#define	IOCTL			/* include sys/ioctl.h? */
#define	PWD			/* include pwd.h? */
#undef	I_TIME			/* include time.h? */
#define	I_SYSTIME		/* include sys/time.h? */
#undef	SYSTIMEKERNEL
#define	VARARGS			/* include varargs.h? */

#define	index	  strchr	/* use these instead */
#define	rindex	  strrchr

#define	VOIDSIG			/* you have 'void (*signal)()' */

#ifdef	VOIDSIG				/* define sig_t appropriately */
typedef	void	sig_t;
#else	/* VOIDSIG */
typedef	int	sig_t;
#endif	/* VOIDSIG */

#ifndef	VOIDUSED
#define	VOIDUSED  7
#endif	/* VOIDUSED */
#define	VOIDFLAGS 7
#if (VOIDFLAGS & VOIDUSED) != VOIDUSED
#define	void	  int		/* is void to be avoided? */
#define	M_VOID			/* Xenix strikes again */
#endif	/* VOIDFLAGS & VOIDUSED */

#ifndef	PORTABLE
#define	HOSTNAME  "nyongwa"	/* compile node name in */
#else	/* PORTABLE */
#define	DOUNAME			/* use uname() to get node name */
#undef	PHOSTNAME 	/* get node name from this command */
#endif	/* PORTABLE */

#ifndef	UTMP_FILE
#define	UTMP_FILE "/etc/utmp"	/* name of the utmp file */
#endif	/* UTMP_FILE */

#ifdef	LOGUTMP
#ifndef	WTMP_FILE
#define	WTMP_FILE "/etc/wtmp"	/* name of the wtmp file */
#endif	/* WTMP_FILE */
#endif	/* LOGUTMP */

#ifdef	TRYMAIL
#define	MAILER	  "/bin/mail"	/* mail agent */
#endif	/* TRYMAIL */

#ifdef	UUGETTY
#define	ASCIIPID		/* PID stored in ASCII in lock file */
#define	BOTHPID			/* ... or perhaps not */
#define	LOCK	  "/usr/spool/uucp/LCK..%s"	/* lock file name */
#define	UUCPID	  5	/* uid of UUCP account */
#endif	/* UUGETTY */


/* end of config.h */
