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

#define PACKETSIZE	1490	/* network packet size - sizeof(framehdr) */

#define BROADCAST	((address) 0xFF)

#define TYPE		0x0F	/* message types */
#define   LOCATE	0x01
#define   HERE		0x02
#define   REQUEST	0x03
#define   REPLY		0x04
#define   ACK		0x05
#define   NAK		0x06
#define	  ENQUIRY	0x07
#define   ALIVE		0x08
#define	  DEAD		0x09

#define LAST		0x10	/* flags */
#define RETRANS		0x20

struct pktheader {
	char	ph_dstnode;	/* 0: destination node */
	char	ph_srcnode;	/* 1: source node */
	char	ph_dsttask;	/* 2: destination task */
	char	ph_srctask;	/* 3: source task */
	char	ph_ident;	/* 4: transaction identifier */
	char	ph_seq;		/* 5: fragment no. */
	unshort	ph_size;	/* 6: total size of this packet */
	char	ph_flags;	/* 8: some flags (not used) */
	char	ph_type;	/* 9: locate, here, data, ack or nak (!= 0) */
};

#define ph_signal	ph_seq

#define NOSEND		0
#define SEND		1

#define DONTKNOW	0
#define LOCAL		1
#define GLOBAL		2

#define siteaddr(x)	lobyte(x)
#define tasknum(x)	hibyte(x)

#define pktfrom(ph)	((unshort) (ph->ph_srctask<<8 | ph->ph_srcnode & 0xFF))
#define pktto(ph)	((unshort) (ph->ph_dsttask<<8 | ph->ph_dstnode & 0xFF))
