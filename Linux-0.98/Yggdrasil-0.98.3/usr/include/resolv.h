/*
 * Copyright (c) 1983, 1987, 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)resolv.h	5.10 (Berkeley) 6/1/90
 */

#ifndef _RESOLVE_H
#define	_RESOLVE_H

#include <traditional.h>
#include <sys/types.h>

#include <netinet/in.h>		/* struct sockaddr_in declaration */
#include <arpa/nameser.h>	/* MAXDNAME definition */

/*
 * Resolver configuration file.
 * Normally not present, but may contain the address of the
 * inital name server(s) to query and the domain search list.
 */

#ifndef __PATH_ETC
#define __PATH_ETC	"/usr/etc/inet"
#endif

#ifndef _PATH_RESCONF
#define _PATH_RESCONF	__PATH_ETC"/resolv.conf"
#endif

/*
 * Global defines and variables for resolver stub.
 */
#define	MAXNS		3		/* max # name servers we'll track */
#define	MAXDFLSRCH	3		/* # default domain levels to try */
#define	MAXDNSRCH	6		/* max # domains in search path */
#define	LOCALDOMAINPARTS 2		/* min levels in name that is "local" */

#define	RES_TIMEOUT	5		/* min. seconds between retries */

struct state {
	int	retrans;	 	/* retransmition time interval */
	int	retry;			/* number of times to retransmit */
	long	options;		/* option flags - see below. */
	int	nscount;		/* number of name servers */
	struct	sockaddr_in nsaddr_list[MAXNS];	/* address of name server */
#define	nsaddr	nsaddr_list[0]		/* for backward compatibility */
	u_short	id;			/* current packet id */
	char	defdname[MAXDNAME];	/* default domain */
	char	*dnsrch[MAXDNSRCH+1];	/* components of domain to search */
};

/*
 * Resolver options
 */
#define RES_INIT	0x0001		/* address initialized */
#define RES_DEBUG	0x0002		/* print debug messages */
#define RES_AAONLY	0x0004		/* authoritative answers only */
#define RES_USEVC	0x0008		/* use virtual circuit */
#define RES_PRIMARY	0x0010		/* query primary server only */
#define RES_IGNTC	0x0020		/* ignore trucation errors */
#define RES_RECURSE	0x0040		/* recursion desired */
#define RES_DEFNAMES	0x0080		/* use default domain name */
#define RES_STAYOPEN	0x0100		/* Keep TCP socket open */
#define RES_DNSRCH	0x0200		/* search up local domain tree */

#define RES_DEFAULT	(RES_RECURSE | RES_DEFNAMES | RES_DNSRCH)

extern struct state _res;

extern int h_errno;

#ifdef __cplusplus
extern "C" {
#endif

int res_query _ARGS ((const char *__dname, int __class, int __type,
	u_char *__answer, int __anslen));
int res_search _ARGS ((const char *__dname, int __class, int __type,
	u_char *__answer, int __anslen));
int res_mkquery _ARGS ((int __op, const char *__dname, int __class, int __type,
	char *__data, int __datalen, struct rrec *__newrr, char *__buf,
	int __buflen));
int res_querydomain _ARGS ((const char *__name, const char *__domain,
	int __class, int __type, u_char *__answer, int __anslen));
int res_send _ARGS ((const char *__msg, int __msglen, char *__answer,
	int __anslen));
int res_init _ARGS ((void));

int dn_comp _ARGS ((u_char *__exp_dn, u_char *__comp_dn, int __length,
	 u_char **__dnptrs, u_char **__lastdnptr));
int dn_expand _ARGS ((u_char *__msg, u_char *__eomorig, u_char *__comp_dn, 
	u_char *__exp_dn, int __length));
int dn_skipname _ARGS ((u_char *__comp_dn, u_char *__eom));

void herror _ARGS((char *__s));

#ifdef __cplusplus
}
#endif

#endif /* _RESOLVE_H */
