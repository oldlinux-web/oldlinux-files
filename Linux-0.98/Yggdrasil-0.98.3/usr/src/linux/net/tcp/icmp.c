/* Internet Control Message Protocol (ICMP) icmp.c */

/*
    Copyright (C) 1992  Bob Harris

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 

    The Author of tcpip package may be reached as bir7@leland.stanford.edu or
    C/O Department of Mathematics; Stanford University; Stanford, CA 94305

    The author of this file may be reached at rth@sparta.com or Sparta, Inc.
    7926 Jones Branch Dr. Suite 900, McLean Va 22102.
*/

/* modified by Ross Biro bir7@leland.stanford.edu to do more than just
   echo responses. */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kernel.h>	/* free_s */
#include <linux/fcntl.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include "timer.h"
#include "ip.h"
#include "tcp.h"
#include "sock.h"
#include <linux/errno.h>
#include <linux/timer.h>
#include <asm/system.h>
#include <asm/segment.h>
#include "../kern_sock.h" /* for PRINTK */
#include "icmp.h"

#define min(a,b) ((a)<(b)?(a):(b))

/* an array of errno for error messages from dest unreach. */
struct icmp_err icmp_err_convert[]=
{
   {ENETUNREACH, 1},
   {EHOSTUNREACH, 1},
   {ENOPROTOOPT, 1},
   {ECONNREFUSED, 1},
   {EOPNOTSUPP, 0},
   {EOPNOTSUPP, 0},
   {ENETUNREACH, 1},
   {EHOSTDOWN, 1},
   {ENONET, 1},
   {ENETUNREACH, 1},
   {EHOSTUNREACH, 1},
   {EOPNOTSUPP, 0},
   {EOPNOTSUPP, 0}
};

void
print_icmph (struct icmp_header *icmph)
{
   PRINTK ("  type = %d, code = %d, checksum = %X\n", icmph->type,
	   icmph->code, icmph->checksum);
   PRINTK (" gateway = %X\n", icmph->un.gateway);
}

/* sends an icmp message in response to a packet. */
void
icmp_reply (struct sk_buff *skb_in,  int type, int code, struct device *dev)
{
   struct sk_buff *skb;
   struct ip_header *iph;
   int offset;
   struct icmp_header *icmph;

   int len;
   /* get some memory for the replay. */
   len = sizeof (*skb) + 8 /* amount of header to return. */ +
         sizeof (struct icmp_header) +
	 64 /* enough for an ip header. */ +
	 dev->hard_header_len;
	   
   skb = malloc (len);
   if (skb == NULL) return;

   skb->mem_addr = skb;
   skb->mem_len = len;

   len -= sizeof (*skb);

   /* find the ip header. */
   iph = (struct ip_header *)(skb_in+1);
   iph = (struct ip_header *)((unsigned char *)iph + dev->hard_header_len);

   /* Build Layer 2-3 headers for message back to source */
   offset = ip_build_header( skb, iph->daddr, iph->saddr,
			    &dev, IPPROTO_ICMP, NULL, len );

   if (offset < 0)
     {
	skb->sk = NULL;
	free_skb (skb, FREE_READ);
	return;
     }

   /* Readjust length according to actual IP header size */
   skb->len = offset + sizeof (struct icmp_header) + 8;
   
   icmph = (struct icmp_header *)((unsigned char *)(skb+1) + offset);
   icmph->type = type;
   icmph->code = code;
   icmph->checksum = 0; /* we don't need to compute this. */
   icmph->un.gateway = 0; /* might as well 0 it. */
   memcpy (icmph+1, iph+1, 8);
   /* send it and free it. */
   ip_queue_xmit (NULL, dev, skb, 1);
   
}

/* deals with incoming icmp packets. */

int
icmp_rcv(struct sk_buff *skb1, struct device *dev, struct options *opt,
	unsigned long daddr, unsigned short len,
	unsigned long saddr, int redo, struct ip_protocol *protocol )
{
   int size, offset;
   struct icmp_header *icmph, *icmphr;
   struct sk_buff *skb;
   unsigned char *buff;


   /* drop broadcast packets.  */
   if ((daddr & 0xff000000) == 0 || (daddr & 0xff000000) == 0xff000000)
     {
	skb1->sk = NULL;
	free_skb (skb1, FREE_READ);
	return (0);
     }

   buff = skb1->h.raw;

   icmph = (struct icmp_header *)buff;

   /* Validate the packet first */
   if( icmph->checksum )
     { /* Checksums Enabled? */
	if( ip_compute_csum( (unsigned char *)icmph, len ) )
	  {
	     /* Failed checksum! */
	     PRINTK("\nICMP ECHO failed checksum!");
	     skb1->sk = NULL;
	     free_skb (skb1, FREE_READ);
	     return (0);
	  }
     }

   print_icmph(icmph);

   /* Parse the ICMP message */
   switch( icmph->type )
     {
       case ICMP_DEST_UNREACH:
       case ICMP_SOURCE_QUENCH:
	{
	   struct ip_header *iph;
	   struct ip_protocol *ipprot;
	   unsigned char hash;
	   int err;

	   err = icmph->type << 8 | icmph->code;

	   /* we need to cause the socket to be closed and the error message
	      to be set appropriately. */
	   iph = (struct ip_header *)(icmph+1);

	   /* get the protocol(s) */
	   hash = iph->protocol & (MAX_IP_PROTOS -1 );
	   for (ipprot = ip_protos[hash]; ipprot != NULL; ipprot=ipprot->next)
	     {
		/* pass it off to everyone who wants it. */
		ipprot->err_handler (err, (unsigned char *)iph+4*iph->ihl,
				     iph->daddr, iph->saddr, ipprot);
	     }
	   skb1->sk = NULL;
	   free_skb (skb1, FREE_READ);
	   return (0);
	}

       case ICMP_REDIRECT:
	{
	   /* we need to put a new route in the routing table. */
	   struct rtable *rt; /* we will add a new route. */
	   struct ip_header *iph;

	   iph = (struct ip_header *)(icmph+1);
	   rt = malloc (sizeof (*rt));
	   if (rt != NULL)
	     {
		rt->net = iph->daddr;
		/* assume class C network.  Technically this is incorrect,
		   but will give it a try. */
		if ((icmph->code & 1) == 0) rt->net &= 0x00ffffff;
		rt->dev = dev;
		rt->router = icmph->un.gateway;
		add_route (rt);
	     }
	   skb1->sk = NULL;
	   free_skb (skb1, FREE_READ);
	   return (0);
	}

       case ICMP_ECHO: 
	
	/* Allocate an sk_buff response buffer (assume 64 byte IP header) */

	size = sizeof( struct sk_buff ) + dev->hard_header_len + 64 + len;
	skb = malloc( size );
	if (skb == NULL)
	  {
	     skb1->sk = NULL;
	     free_skb (skb1, FREE_READ);
	     return (0);
	  }
	skb->sk = NULL;
	skb->mem_addr = skb;
	skb->mem_len = size;

	/* Build Layer 2-3 headers for message back to source */
	offset = ip_build_header( skb, daddr, saddr, &dev, IPPROTO_ICMP, opt, len );
	if (offset < 0)
	  {
	     /* Problems building header */
	     PRINTK("\nCould not build IP Header for ICMP ECHO Response");
	     free_s (skb->mem_addr, skb->mem_len);
	     skb1->sk = NULL;
	     free_skb (skb1, FREE_READ);
	     return( 0 ); /* just toss the received packet */
	  }

	/* Readjust length according to actual IP header size */
	skb->len = offset + len;

	/* Build ICMP_ECHO Response message */
	icmphr = (struct icmp_header *)( (char *)( skb + 1 ) + offset );
	memcpy( (char *)icmphr, (char *)icmph, len );
	icmphr->type = ICMP_ECHOREPLY;
	icmphr->code = 0;
	icmphr->checksum = 0;

	if( icmph->checksum )
	  { /* Calculate Checksum */
	     icmphr->checksum = ip_compute_csum( (void *)icmphr, len );
	  }

	/* Ship it out - free it when done */
	ip_queue_xmit( (volatile struct sock *)NULL, dev, skb, 1 );
	
	skb1->sk = NULL;
	free_skb (skb1, FREE_READ);
	return( 0 );

	default:
	PRINTK("\nUnsupported ICMP type = x%x", icmph->type );
	skb1->sk = NULL;
	free_skb (skb1, FREE_READ);
	return( 0 ); /* just toss the packet */
     }

   /* should be unecessary, but just in case. */
   skb1->sk = NULL;
   free_skb (skb1, FREE_READ);
   return( 0 ); /* just toss the packet */
}

