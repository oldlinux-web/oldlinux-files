#ifndef _LINUX_SOCKET_H
#define _LINUX_SOCKET_H

struct sockaddr {
	unsigned short sa_family;	/* address family, AF_xxx */
	char sa_data[14];		/* 14 bytes of protocol address */
};

/*
 * socket types
 */
#define SOCK_STREAM	1		/* stream (connection) socket */
#define SOCK_DGRAM	2		/* datagram (connectionless) socket */
#define SOCK_RAW	3		/* raw socket */
#define SOCK_RDM	4		/* reliably-delivered message */
#define SOCK_SEQPACKET	5		/* sequential packet socket */
#define SOCK_PACKET    10		/* linux specific way of getting
					   packets at the dev level.  For
					   writing rarp and other similiar
					   things on the user level. */

/*
 * supported address families
 */
#define AF_UNSPEC	0
#define AF_UNIX		1
#define AF_INET		2

/*
 * protocol families, same as address families
 */
#define PF_UNIX		AF_UNIX
#define PF_INET		AF_INET

/* flags we can use with send/ and recv. */
#define MSG_OOB		1
#define MSG_PEEK	2

/* ioctl's */
#define FIOSETOWN 	0x8901	 /* the 89 is for uniqueness.
				    This should be somewhere else. */
#define SIOCSPGRP	0x8902
#define FIOGETOWN	0x8903   /* this too. */
#define SIOCGPGRP	0x8904
#define SIOCATMARK	0x8905


/* for setsockoptions */
#define SO_DEBUG	1
#define SO_REUSEADDR	2
#define SO_TYPE		3
#define SO_ERROR	4
#define SO_DONTROUTE	5
#define SO_BROADCAST	6
#define SO_SNDBUF	7
#define SO_RCVBUF	8
#define SO_KEEPALIVE	9
#define SO_OOBINLINE   10
#define SO_NO_CHECK    11
#define SO_PRIORITY    12
#define SO_LINGER      13

/* the different priorities */
#define SOPRI_INTERACTIVE	0
#define SOPRI_NORMAL		1
#define SOPRI_BACKGROUND	2

/* setsockoptions level */
#define SOL_SOCKET	1

#endif /* _LINUX_SOCKET_H */
