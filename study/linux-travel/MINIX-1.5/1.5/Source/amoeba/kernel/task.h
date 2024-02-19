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

#ifdef BUFFERED				/* HACK */
#define BUFSIZE		100
#define NETBUF		((buffer) -1)

typedef unshort buffer;
#endif

struct task {

#include "mpx.H"			/* mpx module */
#include "trans.H"			/* trans module */
#include "portcache.H"			/* portcache module */
	char *tk_aux;			/* auxiliary pointer */
	/* really a hack to make process task more efficient */

};

extern struct task *curtask, *uppertask;
extern unshort ntask;

#define NILTASK		((struct task *) 0)
