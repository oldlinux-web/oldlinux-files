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

struct portcache {
	address PE_location;
	struct task *PE_link;
} tk_portcache;

#ifdef PORTCACHE

#define pe_location	tk_portcache.PE_location
#define pe_link		tk_portcache.PE_link

#else 

#define tk_portcache	tk_dummy	/* other modules must not touch it */

#endif

#define NOWAIT		0
#define WAIT		1

#define LOOK		0
#define DELETE		1
