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

struct trans {
	char		TS_state;	/* see below */
	char		TS_flags;	/* several flags - see below */
	char		TS_clident;	/* ident number for client */
	char		TS_svident;	/* ident number for server */
	char		TS_ident;	/* transaction identifier */
	char		TS_seq;		/* fragment sequence number */
	char		TS_count;	/* max. number timer may expire */
	char		TS_signal;	/* signal being sent to the server */
	unshort		TS_addr;	/* network address of this task */
	unshort		TS_timer;	/* timer, decremented every sweep */
	unshort		TS_cltim;	/* client crash timer */
	address		TS_client;	/* if serving: who's the client */
	address		TS_server;	/* if in trans: who's the server */
	port		TS_portcache;	/* this port was used the last time */
	header	       *TS_rhdr;	/* saved param in getreq or trans */
	header	       *TS_xhdr;	/* saved param in putrep or trans */
	bufptr		TS_rbuf;	/* receiver buffer */
	bufptr		TS_xbuf;	/* transmitter buffer */
	unshort		TS_rcnt;	/* size of rbuf */
	unshort		TS_xcnt;	/* size of xbuf */
	unshort		TS_offset;	/* offset in buffer */
	unshort		TS_maxloc;	/* max. location time in seconds */
	long		TS_totloc;	/* total location time in ticks */
	long		TS_totsvr;	/* total server time in ticks */
#ifdef BUFFERED
	address		TS_sender;	/* task that sent the buffer */
	char	       *TS_savehdr;	/* saved header pointer */
	buffer		TS_buffer;	/* buffer */
	unshort		TS_bufcnt;	/* buffer size */
	unshort		TS_what;	/* REQUEST or REPLY */
#endif
} tk_trans;

#ifdef TRANS

#define ts_state	tk_trans.TS_state
#define ts_flags	tk_trans.TS_flags
#define ts_clident	tk_trans.TS_clident
#define ts_svident	tk_trans.TS_svident
#define ts_ident	tk_trans.TS_ident
#define ts_seq		tk_trans.TS_seq
#define ts_timer	tk_trans.TS_timer
#define ts_count	tk_trans.TS_count
#define ts_signal	tk_trans.TS_signal
#define ts_addr		tk_trans.TS_addr
#define ts_cltim	tk_trans.TS_cltim
#define ts_client	tk_trans.TS_client
#define ts_server	tk_trans.TS_server
#define ts_portcache	tk_trans.TS_portcache
#define ts_rhdr		tk_trans.TS_rhdr
#define ts_xhdr		tk_trans.TS_xhdr
#define ts_rbuf		tk_trans.TS_rbuf
#define ts_xbuf		tk_trans.TS_xbuf
#define ts_rcnt		tk_trans.TS_rcnt
#define ts_xcnt		tk_trans.TS_xcnt
#define ts_offset	tk_trans.TS_offset
#define ts_maxloc	tk_trans.TS_maxloc
#define	ts_totloc	tk_trans.TS_totloc
#define ts_totsvr	tk_trans.TS_totsvr
#define ts_sender	tk_trans.TS_sender
#define ts_savehdr	tk_trans.TS_savehdr
#define ts_buffer	tk_trans.TS_buffer
#define ts_bufcnt	tk_trans.TS_bufcnt
#define ts_what		tk_trans.TS_what

/* possible values of ts_state */
#define IDLE		0
#define SENDING		1
#define DONE		2
#define ACKED		3
#define NACKED		4
#define FAILED		5
#define WAITBUF		6
#define RECEIVING	7
#define ABORT		8
#define MEMFAULT	9

/* possible flags in ts_flags */
#define LOCATING	bit(0)	/* blocked in trans locating a port */
#define PUTREQ		bit(1)	/* blocked in trans sending a request */
#define GETREQ		bit(2)	/* blocked in getreq */
#define PUTREP		bit(3)	/* blocked in putrep */
#define GETREP		bit(4)	/* blocked in trans getting a reply */
#define SERVING		bit(5)	/* running between getreq and putrep */

#else 

#define tk_trans	tk_dummy	/* other modules must not touch it */

#endif
