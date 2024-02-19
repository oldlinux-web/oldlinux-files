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

#define NDEBUG
#define PORTCACHE

/*
 * This module does the port management. It keeps track of the local servers
 * doing a ``getreq'' on a port, local clients waiting for a server on some
 * port, and interlocal servers addressed by some port. This last category of
 * ports may be forgotten, or may be incorrect.
 *
 * The following routines are defined:
 *	portinstall(port, where, wait);
 *	portlookup(port, wait, delete);
 *	portremove(port, location);
 *	portquit(port, task);
 *
 * ``Portinstall'' is called when a port is assumed to be at location
 * ``where.'' If ``wait'' is set, the port is local.
 * ``Portlookup'' is called to find a port in the cache. If ``wait'' is
 * set, the routine will block until the port is found. If ``delete'' is
 * set, the port must be removed when it is found.
 * ``Portremove'' removes a port from the cache which is thought
 * of to be at the specified location.
 * When a port doesn't have to be located anymore for some task, ``portquit''
 * takes care of that.
 */

#include "kernel.h"
#include "amoeba.h"
#include "global.h"
#include "task.h"
#include "internet.h"
#include "assert.h"

extern struct task	task[];

#ifdef STATISTICS
#include "portstat.h"

struct portstat portstat;
#define STINC(x)	portstat.x++
#else
#define STINC(x)
#endif

#include "conf.h"

#define LOGHASH		   5	/* log sizeof hash table of local ports */

struct porttab {
	port p_port;			/* the port this entry is about */
	unshort p_idle;			/* idle timer */
	address p_location;		/* where is it? (0 = being located) */
	address p_asker;		/* address of someone interested */
	struct porttab *p_nextport;	/* port with same hash value */
	struct task *p_tasklist;	/* list of tasks */
};

#define NILPORTTAB	((struct porttab *) 0)

#define NHASH		(1 << LOGHASH)
#define HASHMASK	(NHASH - 1)

#define hash(p)		(* (unshort *) (p) & HASHMASK)

/* MINIX can't allocate memory in the kernel at run-time
extern unshort nport;
PRIVATE struct porttab *porttab, *lastport, *hashtab[NHASH], *portfree;
*/
PRIVATE struct porttab porttab[NPORTS];
PRIVATE struct porttab *lastport, *hashtab[NHASH], *portfree;

#ifndef NONET
#ifndef NOCLIENT

#define NLOCATE            8    /* max. number of ports to locate */
PRIVATE port loctab[NLOCATE];
PRIVATE unshort loccnt, loctim, locthissweep;
extern unshort minloccnt, maxloccnt;

#endif
#endif

/* Allocate an entry in the hash table at location ``ht.'' */
static struct porttab *allocate(ht, p)
struct porttab **ht;
port *p;
{
  register struct porttab *pt;
  STINC(pts_alloc);
  if ((pt=portfree) == 0) {
	STINC(pts_full);
	portpurge();		/* total cleanup, not likely to happen */
	if ((pt=portfree) == 0)
		return 0;
  }
  portfree = pt->p_nextport;
  pt->p_nextport = *ht;
  *ht = pt;
  pt->p_port = *p;
  return pt;
}

/* Install a port in the hash table.  If ``wait'' is set, the location will
 * be this machine and is certain.  If not, the location is somewhere else
 * and uncertain.
 */
portinstall(p, where, wait)
register port *p;
address where;
{
  register struct porttab **ht, *pt;
  register struct task *t;
  extern address local;

  ht = &hashtab[hash(p)];
  for (pt = *ht; pt != NILPORTTAB; pt = pt->p_nextport)
	if (PortCmp(&pt->p_port, p)) {
		if (pt->p_location == SOMEWHERE) {
			compare(pt->p_tasklist, !=, NILTASK);
			do {
				t = pt->p_tasklist;
				t->pe_location = where;
				STINC(pts_wakeup);
				wakeup((event_t) &t->pe_location);
			} while ((pt->p_tasklist = t->pe_link) != NILTASK);
		}
#ifndef NOCLIENT
		else if (siteaddr(pt->p_location) == local && !wait &&
							pt->p_tasklist != 0)
			return;
#endif
		break;
	}
  if (pt == NILPORTTAB && (pt = allocate(ht, p)) == NILPORTTAB)
#ifndef NOCLIENT
	if (wait)
#endif
		panic("portcache full for servers", 0x8000);
#ifndef NOCLIENT
	else		/* no room left, so forget it */
		return;
#endif
  pt->p_location = where;
#ifndef NOCLIENT
  if (wait) {	/* task is going to await a client, so leave it immortal */
#endif
	compare(area(where), ==, LOCAL);
	t = &task[tasknum(where)];
	t->pe_location = where;
	t->pe_link = pt->p_tasklist;
	pt->p_tasklist = t;
#ifndef NONET
	if (pt->p_asker != NOWHERE) {
		STINC(pts_here);
		hereport(pt->p_asker, p, (unsigned )1);
		pt->p_asker = NOWHERE;
	}
#endif
#ifndef NOCLIENT
  }
#endif
  pt->p_idle = 0;
}

#ifndef NONET
#ifndef NOCLIENT
/* Broadcast a locate message
 */
static sendloc(){
  register struct porttab *pt;
  register unsigned n = 0;

  if (locthissweep) {
	/* During this clocktick we already sent out a broadcast packet.
	 * To prevent buggy userprograms from creating a broadcast storm
	 * we do not send another one, we just prepare for it to be done 
	 */
	 STINC(pts_nolocate);
	 loccnt = maxloccnt;
	 return;
  }
  for (pt = porttab; pt < lastport; pt++)
	if (pt->p_location == SOMEWHERE) {
		loctab[n++] = pt->p_port;
		if (n == NLOCATE)
			break;
	}
  if (n) {
	locthissweep = 1;
	whereport(loctab, n);	/* send out the broadcast locate */
  } else
	loctim = 0;	/* No locates necessary anymore */
  loccnt = 0;
}

#endif NOCLIENT
#endif NONET

/* Look whether port p is in the portcache.  You can specify whether you
 * want to wait for the information and whether you want to delete it.
 */
address portlookup(p, wait, del)
register port *p;
{
  register struct porttab **ht, *pt;
  register struct task *c, *t;
  register address location;

  STINC(pts_lookup);
  ht = &hashtab[hash(p)];
  for (pt = *ht; pt != NILPORTTAB; pt = pt->p_nextport)
	if (PortCmp(&pt->p_port, p)) {	/* found it */
		location = pt->p_location;
		switch (area(location)) {
		case LOCAL:		/* local server */
			if (pt->p_tasklist == 0)
				break;
			if (del) {	/* remove it */
				pt->p_tasklist = pt->p_tasklist->pe_link;
				if ((t = pt->p_tasklist) != NILTASK)
					pt->p_location = t->pe_location;
			}
			pt->p_idle = 0;
			STINC(pts_flocal);
			return(location);

		case GLOBAL:		/* remote server */
			compare(pt->p_tasklist, ==, NILTASK);
			pt->p_idle = 0;
			STINC(pts_fglobal);
			return(location);
		
		case DONTKNOW:		/* somebody else wants to know too */
			compare(pt->p_tasklist, !=, NILTASK);
			break;
		
		default:
			assert(0);
		}
		break;
	}
  /* The port wasn't in the port cache */
#ifndef NOCLIENT
  if (wait) {		/* wait for it */
	if (pt == NILPORTTAB && (pt = allocate(ht, p)) == NILPORTTAB)
		panic("portcache full for clients", 0x8000);
	pt->p_location = SOMEWHERE;
	c = curtask;
	c->pe_link = pt->p_tasklist;
	pt->p_tasklist = c;
#ifndef NONET
	STINC(pts_locate);
	sendloc();
	loctim = minloccnt;
#endif
	c->pe_location = SOMEWHERE;
	if (sleep((event_t) &c->pe_location))
		assert(pt->p_tasklist != c);
	pt->p_idle = 0;
	return(c->pe_location);
  }
  else
#endif NOCLIENT
	return(NOWHERE);
}

/* Port p isn't at location ``location'' anymore */
portremove(p, location)
port *p;
address location;
{
  register struct porttab *pt, **ht;
  register struct task *t;

  for (ht = &hashtab[hash(p)]; (pt= *ht) != NILPORTTAB; ht = &pt->p_nextport)
	if (PortCmp(&pt->p_port, p)) {
		if (pt->p_location == location) {
			if ((t = pt->p_tasklist) != NILTASK) {
				compare(area(location), ==, LOCAL);
				compare(t->pe_location, ==, location);
				if ((pt->p_tasklist = t->pe_link) != NILTASK) {
					pt->p_location =
						pt->p_tasklist->pe_location;
					break;
				}
			} else {
				*ht = pt->p_nextport;
				pt->p_location = NOWHERE;	/* for dump */
				pt->p_nextport = portfree;
				portfree = pt;
			}
		}
		else if ((t = pt->p_tasklist) != NILTASK)
			while (t->pe_link != NILTASK)
				if (t->pe_link->pe_location == location) {
					t->pe_link = t->pe_link->pe_link;
					break;
				}
				else
					t = t->pe_link;
		return;
	}
}

#ifndef NOCLIENT
/* Task ``s'' isn't interested anymore */
portquit(p, s)
register port *p;
register struct task *s;
{
  register struct porttab *pt, **ht;
  register struct task *t;

  for (ht = &hashtab[hash(p)]; (pt= *ht) != NILPORTTAB; ht = &pt->p_nextport)
	if (PortCmp(&pt->p_port, p)) {
		if (pt->p_location != SOMEWHERE)
			return;
		if ((t = pt->p_tasklist) == s) {
			if ((pt->p_tasklist = s->pe_link) == NILTASK) {
				*ht = pt->p_nextport;
				pt->p_location = NOWHERE;	/* for dump */
				pt->p_nextport = portfree;
				portfree = pt;
			}
			s->pe_location = NOWHERE;
			wakeup((event_t) &s->pe_location);
		}
		else {
			while (t != NILTASK)
				if (t->pe_link == s) {
					t->pe_link = s->pe_link;
					s->pe_location = NOWHERE;
					wakeup((event_t) &s->pe_location);
					break;
				}
				else
					t = t->pe_link;
		}
		return;
	}
}
#endif NOCLIENT

#ifndef NONET

#define NHERE 8
PRIVATE port heretab[NHERE];

portask(asker, ptab, nports)		/* handle locate request */
address asker;
register port *ptab;
unsigned nports;
{
  register port *p,*q;
  register struct porttab **ht, *pt;
  register unsigned nfound;

  STINC(pts_portask);
  nfound = 0; q = heretab;
  for(p=ptab; nports--; p++) {
	ht = &hashtab[hash(p)];
	for (pt = *ht; pt != NILPORTTAB; pt = pt->p_nextport)
		if (PortCmp(&pt->p_port, p)) {	/* found it */
			if (area(pt->p_location) == LOCAL) {
				if (pt->p_tasklist == 0) {
					/* just record someone was interested */
					pt->p_asker = asker;
					break;
				}
				if (nfound < NHERE) {
					*q++ = *p;
					nfound++;
				}
				pt->p_idle = 0;
			}
		}
  }
  if (nfound) {
	STINC(pts_portyes);
	hereport(asker, heretab, nfound);
  }
}
#endif

#ifndef NDEBUG
portdump(){
  register struct porttab *pt;
  register struct task *t;

  printf("\n PORT  LOCATION IDLE TASKS\n");
  for (pt = porttab; pt < lastport; pt++)
	if (pt->p_location != NOWHERE) {
		prport(&pt->p_port);
		printf("   %4x   %4d", pt->p_location, pt->p_idle);
		for (t = pt->p_tasklist; t != NILTASK; t = t->pe_link)
			printf(" {%d, %x}", t - task, t->pe_location);
		printf("\n");
	}
}
#endif

/* Initialize tables and free list */
portinit(){
  register struct porttab *pt;

/*  MINIX can't allocate data in the kernel at run-time
  extern char *aalloc();

  porttab = (struct porttab *) aalloc(nport * sizeof(struct porttab), 0);
  lastport = &porttab[nport];
*/
  lastport = &porttab[NPORTS];
  for (pt = porttab; pt < lastport; pt++) {
	pt->p_nextport = portfree;
	portfree = pt;
  }
}

/* called when freelist was empty, will throw away all mortal ports */
portpurge() {
  register struct porttab *pt,**ht,**htp;
 
  for (htp=hashtab; htp< &hashtab[NHASH]; htp++) {
	ht = htp;
	while ((pt = *ht) != 0) {
		if (pt->p_tasklist == 0){
			*ht = pt->p_nextport;
			pt->p_location = NOWHERE;	/* for dump */
			pt->p_nextport = portfree;
			portfree = pt;
		} else
			ht = &pt->p_nextport;
	}
  }
}

#define MAXSWEEP	 3000	/* dseconds maximum idle time for port */
#define SWEEPRESOLUTION	  100	/* accuracy */

portsweep() {
  register struct porttab *pt,**ht,**htp;
  static unshort cnt;

#ifndef NOCLIENT
#ifndef NONET
  locthissweep = 0;
  if (loctim && ++loccnt > loctim) {              /* send a locate message */
	STINC(pts_relocate);
        sendloc();
        loctim <<= 1;
        if (loctim > maxloccnt)
                loctim = maxloccnt;
	locthissweep = 0;
  }
#endif NONET
#endif
  if (++cnt<SWEEPRESOLUTION)
	return;
  for (htp=hashtab; htp< &hashtab[NHASH]; htp++) {
	ht = htp;
	while ((pt = *ht) != 0) {
		if (pt->p_tasklist == 0 && (pt->p_idle += cnt) > MAXSWEEP) {
			*ht = pt->p_nextport;
			pt->p_location = NOWHERE;	/* for dump */
			pt->p_nextport = portfree;
			portfree = pt;
			STINC(pts_aged);
		} else
			ht = &pt->p_nextport;
	}
  }
  cnt=0;
}
