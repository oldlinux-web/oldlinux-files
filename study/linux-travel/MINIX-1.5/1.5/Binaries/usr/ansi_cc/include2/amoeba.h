#ifndef _AMOEBA_H
#define _AMOEBA_H

/****************************************************************************/
/*									    */
/* (c) Copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands */
/*									    */
/*    This product is part of the  Amoeba  distributed operating system.    */
/*									    */
/*    Permission to use, sell, duplicate or disclose this software must be  */
/* obtained in writing.  Requests for such permissions may be sent to	    */
/*									    */
/*									    */
/*		Dr. Andrew S. Tanenbaum					    */
/*		Dept. of Mathematics and Computer Science		    */
/*		Vrije Universiteit					    */
/*		Postbus 7161						    */
/*		1007 MC Amsterdam					    */
/*		The Netherlands						    */
/*									    */
/****************************************************************************/

/*
** If the host operating system for amoeba transactions is not amoeba
** then several things may need to be done to make the amoeba code
** compatible.  These are provided in host_os.h.
*/
#include "host_os.h"

#define PORTSIZE	 6
#define OOBSIZE		20
#define HEADERSIZE	32
#define PRIVSIZE	10
#define CAPSIZE		16

#define FAIL		((unshort) -1)
#define NOTFOUND	((unshort) -2)
#define BADADDRESS	((unshort) -3)
#define ABORTED		((unshort) -4)
#define TRYAGAIN	((unshort) -5)

#define sizeoftable(t)	(sizeof(t) / sizeof((t)[0]))

#define NILPORT		((port *) 0)
#define NILBUF		((bufptr) 0)

#define _FP(p)		((struct _fakeport *) (p))
#ifndef lint
#define PortCmp(p, q)	(_FP(p)->_p1==_FP(q)->_p1 && _FP(p)->_p2==_FP(q)->_p2)
#define NullPort(p)	(_FP(p)->_p1==0L && _FP(p)->_p2==0)
#else
#define PortCmp(p, q)	((p)->_portbytes[0] == (q)->_portbytes[0])
#define NullPort(p)	((p)->_portbytes[0] == 0)
#endif

typedef char *bufptr;
#ifndef MAX_BLOCK_NR
typedef unsigned short unshort;
#endif
typedef char *event_t;

typedef struct {
	char	_portbytes[PORTSIZE];
} port;

struct _fakeport {
	long	_p1;
	short	_p2;
};

typedef struct {	/* private part of capability */
	char	prv_object[3];
	char	prv_rights;
	port	prv_random;
} private;

typedef struct {
	port	cap_port;
	private	cap_priv;
} capability;

typedef struct {
	port	h_port;
	port	h_signature;
	private	h_priv;
	unshort	h_command;
	long	h_offset;
	unshort	h_size;
	unshort	h_extra;
} header;

#define h_status	h_command	/* alias: reply status */

/*
** Some function declarations that people tend to forget
** because they are lazy.
*/
extern unshort trans(), getreq(), putrep(), timeout();

#endif /* _AMOEBA_H */
