:
# $Id: config.h.SH,v 2.0 90/09/19 19:37:01 paul Rel $
#
# Creates config.h file for getty distribution
#
# $Log:	config.h.SH,v $
# Revision 2.0  90/09/19  19:37:01  paul
# Initial 2.0 release
# 
#

case $CONFIG in
'')
    if test ! -f config.sh; then
	ln ../config.sh . || \
	ln ../../config.sh . || \
	ln ../../../config.sh . || \
	(echo "Can't find config.sh."; exit 1)
    fi
    . config.sh
    ;;
esac
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac

case "$d_phostname" in
$define) phostname='\"$phostname\"' ;;
$undef)  phostname='' ;;
esac

echo "Extracting config.h (with variable substitutions)"
$spitshell >config.h <<!GROK!THIS!
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

#$d_portable	PORTABLE		/* compile for more than one site */
#$d_getutent	GETUTENT		/* we have getutent() and friends */
#$d_strdup	STRDUP			/* we have strdup() */
#$d_putenv	PUTENV			/* we have putenv() */

#$d_ttytype	TTYTYPE   "$ttytype"	/* file used to identify terminals */
#$define	GETTYTAB  "$gettytab"	/* file used for speed/termio table */

#$define	STDCHAR   $stdchar	/* signed or unsigned chars in stdio */
#$define	UIDTYPE	  $uidtype	/* storage type of UID's */
#$define	GIDTYPE	  $gidtype	/* storage type of GID's */

#$d_fcntl	FCNTL			/* include fcntl.h? */
#$d_ioctl	IOCTL			/* include sys/ioctl.h? */
#$i_pwd		PWD			/* include pwd.h? */
#$i_time	I_TIME			/* include time.h? */
#$i_systime	I_SYSTIME		/* include sys/time.h? */
#$d_systimekernel	SYSTIMEKERNEL
#$d_varargs	VARARGS			/* include varargs.h? */

#$d_index	index	  strchr	/* use these instead */
#$d_index	rindex	  strrchr

#$d_voidsig	VOIDSIG			/* you have 'void (*signal)()' */

#ifdef	VOIDSIG				/* define sig_t appropriately */
typedef	void	sig_t;
#else	/* VOIDSIG */
typedef	int	sig_t;
#endif	/* VOIDSIG */

#ifndef	VOIDUSED
#$define	VOIDUSED  $defvoidused
#endif	/* VOIDUSED */
#$define	VOIDFLAGS $voidflags
#if (VOIDFLAGS & VOIDUSED) != VOIDUSED
#$define	void	  int		/* is void to be avoided? */
#$define	M_VOID			/* Xenix strikes again */
#endif	/* VOIDFLAGS & VOIDUSED */

#ifndef	PORTABLE
#$define	HOSTNAME  "$hostname"	/* compile node name in */
#else	/* PORTABLE */
#$d_douname	DOUNAME			/* use uname() to get node name */
#$d_phostname	PHOSTNAME $phostname	/* get node name from this command */
#endif	/* PORTABLE */

#ifndef	UTMP_FILE
#$define	UTMP_FILE "$utmp"	/* name of the utmp file */
#endif	/* UTMP_FILE */

#ifdef	LOGUTMP
#ifndef	WTMP_FILE
#$define	WTMP_FILE "$wtmp"	/* name of the wtmp file */
#endif	/* WTMP_FILE */
#endif	/* LOGUTMP */

#ifdef	TRYMAIL
#$define	MAILER	  "$mailer"	/* mail agent */
#endif	/* TRYMAIL */

#ifdef	UUGETTY
#$d_asciipid	ASCIIPID		/* PID stored in ASCII in lock file */
#$d_asciipid	BOTHPID			/* ... or perhaps not */
#$define	LOCK	  "$lock/LCK..%s"	/* lock file name */
#$define	UUCPID	  $uucpid	/* uid of UUCP account */
#endif	/* UUGETTY */


/* end of config.h */
!GROK!THIS!
chmod 644 config.h
$eunicefix config.h
