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

struct mpx {
	short	MX_active;		/* is a transaction in progress */
	unshort	MX_flags;		/* flags - see below */
	int	MX_proc_nr;		/* task identifier */
	header	MX_hdr;			/* storage space for header */
} tk_mpx;

#ifdef MPX

#define mx_flags	tk_mpx.MX_flags
#define mx_active	tk_mpx.MX_active
#define mx_proc_nr	tk_mpx.MX_proc_nr
#define	mx_hdr		tk_mpx.MX_hdr


/* bits in flags: */
#define RUNNABLE	bit(0)		/* task is runnable */
#define NESTED		bit(1)		/* nested getreq, trans or putrep */
#define	BETWEEN		bit(2)		/* between getreq and putrep */
#else

#define tk_mpx		tk_dummy	/* other modules must not touch it */

#endif
