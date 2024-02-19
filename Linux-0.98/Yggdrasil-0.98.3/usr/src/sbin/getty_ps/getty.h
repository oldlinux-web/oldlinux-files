/*
**	$Id: getty.h,v 2.0 90/09/19 19:59:15 paul Rel $
**
**	Included by all getty modules
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
**	$Log:	getty.h,v $
**	Revision 2.0  90/09/19  19:59:15  paul
**	Initial 2.0 release
**	
*/


#include "config.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <termio.h>
#ifdef	IOCTL
#include <sys/ioctl.h>
#endif	/* IOCTL */
#ifdef	FCNTL
#include <fcntl.h>
#endif	/* FCNTL */
#include <utmp.h>

#include "extern.h"
#include "funcs.h"
#include "mytermio.h"		/* SMR - gcc was missing a few definitions */

/*	General purpose defines
 */

#ifndef	FALSE
#define	FALSE	(0)
#endif	/* FALSE */
#ifndef	TRUE
#define	TRUE	(1)
#endif	/* TRUE */

#define OK	(0)

#define SUCCESS	(0)		/* normal return */
#define FAIL	(-1)		/* error return */

#define	STDIN	fileno(stdin)
#define	STDOUT	fileno(stdout)

#define strequal(s1, s2)	(strcmp(s1, s2) == 0)
#define strnequal(s1, s2, n)	(strncmp(s1, s2, n) == 0)
#define	strncopy(s1, s2)	(strncpy(s1, s2, sizeof(s1)))

typedef	struct termio	TERMIO;


#ifdef	DEBUG

/* debug levels
 */
#define	D_OPT	0001		/* option settings */
#define	D_DEF	0002		/* defaults file processing */
#define	D_UTMP	0004		/* utmp/wtmp processing */
#define	D_INIT	0010		/* line initialization (INIT) */
#define	D_GTAB	0020		/* gettytab file processing */
#define	D_GETL	0040		/* get login name routine */
#define	D_RUN	0100		/* other runtime diagnostics */
#define D_STTY	0400		/* output of stty -a in various places */

#ifdef	UUGETTY
#define	D_LOCK	0200		/* uugetty lockfile processing */
#endif	/* UUGETTY */

/* debug defs
 */
#define	debug1(a,b)		dprint(a,b)
#define	debug2(a,b)		debug(a,b)
#define	debug3(a,b,c)		debug(a,b,c)
#define	debug4(a,b,c,d)		debug(a,b,c,d)
#define	debug5(a,b,c,d,e)	debug(a,b,c,d,e)
#define	debug6(a,b,c,d,e,f)	debug(a,b,c,d,e,f)

#else	/* DEBUG */

#define	debug1(a,b)		/* define to nothing, disables debugging */
#define	debug2(a,b)
#define	debug3(a,b,c)
#define	debug4(a,b,c,d)
#define	debug5(a,b,c,d,e)
#define	debug6(a,b,c,d,e,f)

#endif	/* DEBUG */


/* end of getty.h */
