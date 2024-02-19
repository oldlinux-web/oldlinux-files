/* raw.c - implements raw ip sockets. */
/*
    Copyright (C) 1992  Ross Biro

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

    The Author may be reached as bir7@leland.stanford.edu or
    C/O Department of Mathematics; Stanford University; Stanford, CA 94305
*/

#include <linux/types.h>
#include <linux/sched.h>
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

extern struct proto raw_prot;

static  unsigned long
min(unsigned long a, unsigned long b)
{
   if (a < b) return (a);
   return (b);
}

/* this should be the easiest of all, all we do is copy it into
   a buffer. */
int
raw_rcv (struct sk_buff *skb, struct device *dev, struct options *opt,
	 unsigned long daddr, unsigned short len, unsigned long saddr,
	 int redo, struct ip_protocol *protocol)
{

   volatile struct sock *sk;

   sk = protocol->data;
   
   /* now we need to copy this into memory. */
   if (!redo )
     {
	skb->dev = dev;
	skb->saddr = daddr;
	skb->daddr = saddr;
	/* now see if we are in use. */
	cli();
	if (sk->inuse)
	  {
	     PRINTK ("raw_rcv adding to backlog. \n");
	     if (sk->back_log == NULL)
	       {
		  sk->back_log = skb;
		  skb->next = skb;
		  skb->prev = skb;
	       }
	     else
	       {
		  skb->next = sk->back_log;
		  skb->prev = sk->back_log->prev;
		  skb->prev->next = skb;
		  skb->next->prev = skb;
	       }
	     sti();
	     return (0);
	  }
	sk->inuse = 1;
	sti();
     }

   skb->sk = sk;
   skb->len = len;

   /* charge it too the socket. */
   if (sk->rmem_alloc + skb->mem_len >= SK_RMEM_MAX)
     {
	skb->sk = NULL;
	free_skb (skb, FREE_READ);
	return (0);
     }
	     
   sk->rmem_alloc += skb->mem_len;

   /* now just put it onto the queue. */
   if (sk->rqueue == NULL)
     {
	sk->rqueue = skb;
	skb->next = skb;
	skb->prev = skb;
     }
   else
     {
	skb->next = sk->rqueue;
	skb->prev = sk->rqueue->prev;
	skb->prev->next = skb;
	skb->next->prev = skb;
     }
   skb->len = len;
   wake_up (sk->sleep);
   release_sock (sk);
   return (0);
}

static  int
raw_loopback (volatile struct sock *sk, int prot, char *from, int len, 
	      unsigned long daddr)
{
   /* just pretend it just came in. */
   struct sk_buff *skb;
   int err;
   skb = malloc (len+sizeof (*skb));
   if (skb == NULL) return (-ENOMEM);

   skb->mem_addr = skb;
   skb->mem_len = len + sizeof (*skb);
   skb->h.raw = (unsigned char *)(skb+1);
   verify_area (from, len);
   memcpy_fromfs (skb+1, from, len);
   err = raw_rcv (skb, NULL, NULL, daddr, len, sk->saddr, prot, 0);
   return (err);
}

/* this will do terrible things if len + ipheader + devheader > dev->mtu */
static int
raw_sendto (volatile struct sock *sk, unsigned char *from, int len,
	    int noblock,
	    unsigned flags, struct sockaddr_in *usin, int addr_len)
{
   struct sk_buff *skb;
   struct device *dev=NULL;
   struct sockaddr_in sin;
   int tmp;

   /* check the flags. */
   if (flags) return (-EINVAL);
   if (len < 0) return (-EINVAL);

   /* get and verify the address. */
   if (usin)
     {
	if (addr_len < sizeof (sin))
	  return (-EINVAL);
	verify_area (usin, sizeof (sin));
	memcpy_fromfs (&sin, usin, sizeof(sin));
	if (sin.sin_family &&
	    sin.sin_family != AF_INET)
	  return (-EINVAL);
     }
   else
     {
	if (sk->state != TCP_ESTABLISHED)
	  return (-EINVAL);
	sin.sin_family = AF_INET;
	sin.sin_port = sk->protocol;
	sin.sin_addr.s_addr = sk->daddr;
     }
   if (sin.sin_port == 0) sin.sin_port = sk->protocol;

   if ((sin.sin_addr.s_addr & 0xff000000) == 0)
     {
	int err;
	err = raw_loopback (sk, sin.sin_port, from,  len,
			    sin.sin_addr.s_addr);
	if (err < 0) return (err);
     }

   sk->inuse = 1;
   skb = sk->prot->wmalloc (sk, len+sizeof (*skb) + sk->prot->max_header, 0);
   /* this shouldn't happen, but it could. */
   if (skb == NULL)
     {
	PRINTK ("raw_sendto: write buffer full?\n");
	print_sk (sk);
	release_sock (sk);
	return (-EAGAIN);
     }
   skb->mem_addr = skb;
   skb->mem_len = len + sizeof (*skb) +sk->prot->max_header;
   skb->sk = sk;
   skb->free = 1; /* these two should be unecessary. */
   skb->arp = 0;
   tmp = sk->prot->build_header (skb, sk->saddr, 
				 sin.sin_addr.s_addr, &dev,
				 sk->protocol, sk->opt, skb->mem_len);
   if (tmp < 0)
     {
	sk->prot->wfree (sk, skb->mem_addr, skb->mem_len);
	release_sock (sk);
	return (tmp);
     }
   verify_area (from, len);
   memcpy_fromfs (skb+1, from, len);
   skb->len = tmp + len;
   sk->prot->queue_xmit (sk, dev, skb, 1);
   return (len);
}

static int
raw_write (volatile struct sock *sk, unsigned char *buff, int len, int noblock,
	   unsigned flags)
{
   return (raw_sendto (sk, buff, len, noblock, flags, NULL, 0));
}

static void
raw_close (volatile struct sock *sk, int timeout)
{
   sk->inuse = 1;
   sk->state = TCP_CLOSE;
   delete_ip_protocol ((struct ip_protocol *)sk->pair);
   free_s ((void *)sk->pair, sizeof (struct ip_protocol));
   release_sock (sk);
}

static int
raw_init (volatile struct sock *sk)
{
   struct ip_protocol *p;
   p = malloc (sizeof (*p));
   if (p == NULL) return (-ENOMEM);

   p->handler = raw_rcv;
   p->protocol = sk->protocol;
   p->data = (void *)sk;
   add_ip_protocol (p);
   
   /* we need to remember this somewhere. */
   sk->pair = (volatile struct sock *)p;

   return (0);
}


int
raw_recvfrom (volatile struct sock *sk, unsigned char *to, int len,
	      int noblock,
	      unsigned flags, struct sockaddr_in *sin, int *addr_len)
{
	/* this should be easy, if there is something there we
	   return it, otherwise we block. */
	int copied=0;
	struct sk_buff *skb;
	if (len == 0) return (0);
	if (len < 0) return (-EINVAL);
	if (addr_len)
	  {
		  verify_area (addr_len, sizeof(*addr_len));
		  put_fs_long (sizeof (*sin), addr_len);
	  }
	sk->inuse = 1;
	while (sk->rqueue == NULL)
	  {
	     if (noblock)
	       {
		  release_sock (sk);
		  return (-EAGAIN);
	       }
	     release_sock (sk);
	     cli();
	     if (sk->rqueue == NULL)
	       {
		  interruptible_sleep_on (sk->sleep);
		  if (current->signal & ~current->blocked)
		    {
		       return (-ERESTARTSYS);
		    }
	       }
	     sti();
	  }
	skb = sk->rqueue;

	if (!(flags & MSG_PEEK))
	  {
		  if (skb->next == skb )
		    {
			    sk->rqueue = NULL;
		    }
		  else
		    {
			    sk->rqueue = sk->rqueue ->next;
			    skb->prev->next = skb->next;
			    skb->next->prev = skb->prev;
		    }
	  }
	copied = min (len, skb->len);
	verify_area (to, copied);
	memcpy_tofs (to, skb->h.raw,  copied);
	/* copy the address. */
	if (sin)
	  {
		  struct sockaddr_in addr;
		  addr.sin_family = AF_INET;
		  addr.sin_addr.s_addr = skb->daddr;
		  verify_area (sin, sizeof (*sin));
		  memcpy_tofs(sin, &addr, sizeof (*sin));
	  }

	if (!(flags & MSG_PEEK))
	  {
	     free_skb (skb, FREE_READ);
	  }
	release_sock (sk);
	return (copied);

}

int
raw_read (volatile struct sock *sk, unsigned char *buff, int len, int noblock,
	  unsigned flags)
{
	return (raw_recvfrom (sk, buff, len, noblock, flags, NULL, NULL));
}


int udp_connect (volatile struct sock *sk, struct sockaddr_in *usin,
		 int addr_len);

int udp_select (volatile struct sock *sk, int sel_type, select_table *wait);


struct proto raw_prot =
{
  sock_wmalloc,
  sock_rmalloc,
  sock_wfree,
  sock_rfree,
  sock_rspace,
  sock_wspace,
  raw_close,
  raw_read,
  raw_write,
  raw_sendto,
  raw_recvfrom,
  ip_build_header,
  udp_connect,
  NULL,
  ip_queue_xmit,
  ip_retransmit,
  NULL,
  NULL,
  raw_rcv,
  udp_select,
  NULL,
  raw_init,
  128,
  0,
  {NULL,}
};
