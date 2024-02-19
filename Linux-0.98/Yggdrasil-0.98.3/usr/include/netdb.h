/*-
 * Copyright (c) 1980, 1983, 1988 Regents of the University of California.
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
 *	@(#)netdb.h	5.11 (Berkeley) 5/21/90
 */

#ifndef _NETDB_H
#define _NETDB_H

#define __PATH_ETC	"/usr/etc/inet"

#define	_PATH_HEQUIV	__PATH_ETC"/hosts.equiv"
#define	_PATH_HOSTS	__PATH_ETC"/hosts"
#define	_PATH_NETWORKS	__PATH_ETC"/networks"
#define	_PATH_PROTOCOLS	__PATH_ETC"/protocols"
#define	_PATH_SERVICES	__PATH_ETC"/services"
#define _PATH_RESCONF   __PATH_ETC"/resolv.conf"

/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */
struct	hostent {
#if 1
	__const__
#else
#ifdef __STDC__
	const
#endif
#endif
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

/*
 * Assumption here is that a network number
 * fits in 32 bits -- probably a poor one.
 */
struct	netent {
	char		*n_name;	/* official name of net */
	char		**n_aliases;	/* alias list */
	int		n_addrtype;	/* net address type */
	unsigned long	n_net;		/* network # */
};

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	int	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

struct	protoent {
	char	*p_name;	/* official protocol name */
	char	**p_aliases;	/* alias list */
	int	p_proto;	/* protocol # */
};

#ifdef __STDC__

#ifdef __cplusplus
extern "C" {
#endif

extern struct hostent *gethostbyname(const char *);
extern struct hostent *gethostbyaddr(const char *, int, int);
extern struct hostent *gethostent(void);
extern void sethostent(int);
extern void endhostent(void);
extern struct netent *getnetbyname(const char *);
extern struct netent *getnetbyaddr(int, int);
extern struct netent *getnetent(void);
extern void setnetent(int);
extern void endnetent(void);
extern struct servent *getservbyname(const char *, const char *);
extern struct servent *getservbyport(int, const char *);
extern struct servent *getservent(void);
extern void setservent(int);
extern void endservent(void);
extern struct protoent *getprotobyname(const char *);
extern struct protoent *getprotobynumber(int);
extern struct protoent *getprotoent(void);
extern void setprotoent(int);
extern void endprotoent(void);
extern struct rpcent *getrpcent(void);
extern struct rpcent *getrpcbyname(const char *);
extern struct rpcent *getrpcbynumber(int);
extern void setrpcent(int);
extern void endrpcent(void);

#ifdef __cplusplus
}
#endif

#else /* not __STDC__ */

extern struct hostent *gethostbyname();
extern struct hostent *gethostbyaddr();
extern struct hostent *gethostent();
extern void sethostent();
extern void endhostent();
extern struct netent *getnetbyname();
extern struct netent *getnetbyaddr();
extern struct netent *getnetent();
extern void setnetent();
extern void endnetent();
extern struct servent *getservbyname();
extern struct servent *getservbyport();
extern struct servent *getservent();
extern void setservent();
extern void endservent();
extern struct protoent *getprotobyname();
extern struct protoent *getprotobynumber();
extern struct protoent *getprotoent();
extern void setprotoent();
extern void endprotoent();
extern struct rpcent *getrpcent();
extern struct rpcent *getrpcbyname();
extern struct rpcent *getrpcbynumber();
extern void setrpcent();
extern void endrpcent();

#endif /* not __STDC__ */

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */

#define	HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA		4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS	NO_DATA		/* no address, look for MX record */

#endif /* _NETDB_H */
