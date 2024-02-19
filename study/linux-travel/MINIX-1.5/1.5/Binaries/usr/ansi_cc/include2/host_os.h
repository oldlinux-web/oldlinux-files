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
**	This is the host_os.h file for Minix
*/

#define	SIGAMOEBA	SIGEMT

#ifdef AM_KERNEL
/*
** to avoid altering trans.c, portcache.c, etc we need the following
*/

#ifndef PRIVATE
#define	PRIVATE		static
#endif

/*
** The following sequences of undefs and defines is to avoid clashes in the
** naming of variables and constants in Amoeba and Minix.
*/

#undef ABORT
#undef ABORTED	
#undef ACK
#undef ACKED
#undef ALIVE
#undef BADADDRESS
#undef BROADCAST
#undef BUFSIZE
#undef CRASH
#undef DEAD
#undef DELETE
#undef DONE
#undef DONTKNOW
#undef ENQUIRY
#undef FAIL
#undef FAILED
#undef GLOBAL
#undef HASHMASK
#undef HEADERSIZE
#undef HERE
#undef IDLE
#undef IMMORTAL
#undef LAST
#undef LOCAL
#undef LOCATE
#undef LOCATING
#undef LOOK
#undef MEMFAULT
#undef MORTAL
#undef NAK
#undef NESTED
#undef NHASH
#undef NILVECTOR
#undef NOSEND
#undef NOTFOUND
#undef NOWAIT
#undef NOWHERE
#undef PACKETSIZE
#undef PORT
#undef RECEIVING
#undef REPLY
#undef REQUEST
#undef RETRANS
#undef RUNNABLE
#undef SEND
#undef SENDING
#undef SERVING
#undef SOMEWHERE
#undef TASK
#undef TYPE
#undef WAIT

#undef bit
#undef concat
#undef disable
#undef enable
#undef hash
#undef hibyte
#undef lobyte
#undef siteaddr
#undef sizeoftable

#define allocbuf	am_allocbuf
#define append		am_append
#define area		am_area
#define badassertion	am_badassertion
#define cleanup		am_cleanup
#define debug		am_debug
#define destroy		am_destroy
#define freebuf		am_freebuf
#define getall		am_gall
#define getbuf		am_gbuf
#define getreq		am_greq
#define getsig		am_gsig
#define handle		am_handle
#define locate		am_locate
#define netenable	am_netenable
#define netsweep	am_sweep
#define ntask		am_ntsk
#define	pickoff		am_pickoff
#define porttab		am_ptab
#define puthead		am_puthead
#define putbuf		am_pbuf
#define putrep		am_prep
#define putsig		am_psig
#define sendsig		am_sendsig
#define sleep		am_sleep
#define task		am_task
#define ticker		am_ticker
#define timeout		am_timeout
#define trans		am_trans
#define umap		am_umap
#define uniqport	am_uniqport
#define uppertask	am_uppertask
#define wakeup		am_wakeup

#endif /* AM_KERNEL */
