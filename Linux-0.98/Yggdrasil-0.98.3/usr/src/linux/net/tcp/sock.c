/* sock.c */
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
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/sock_ioctl.h>
#include "../kern_sock.h"
#include "timer.h"
#include "ip.h"
#include "tcp.h"
#include "udp.h"
#include "sock.h"
#include <asm/segment.h>
#include <asm/system.h>
#include <linux/fcntl.h>

#ifdef MEM_DEBUG
#define MPRINTK printk
#else
#define MPRINTK dummy_routine
#endif

#define min(a,b) ((a)<(b)?(a):(b))
#define swap(a,b) {unsigned long c; c=a; a=b; b=c;}

extern struct proto tcp_prot;
extern struct proto udp_prot;
extern struct proto raw_prot;
extern struct proto packet_prot;

static int ip_proto_init(void);
static int ip_proto_create(struct socket *sock, int protocol);
static int ip_proto_dup(struct socket *newsock, struct socket *oldsock);
static int ip_proto_release(struct socket *sock, struct socket *peer);
static int ip_proto_bind(struct socket *sock, struct sockaddr *umyaddr,
			   int sockaddr_len);
static int ip_proto_connect(struct socket *sock, struct sockaddr *uservaddr,
			      int sockaddr_len, int flags);
static int ip_proto_socketpair(struct socket *sock1, struct socket *sock2);
static int ip_proto_accept(struct socket *sock, struct socket *newsock, int flags);
static int ip_proto_getname(struct socket *sock, struct sockaddr *usockaddr,
			      int *usockaddr_len, int peer);
static int ip_proto_read(struct socket *sock, char *ubuf, int size,
			   int nonblock);
static int ip_proto_write(struct socket *sock, char *ubuf, int size,
			    int nonblock);
static int ip_proto_select(struct socket *sock, int which, select_table *wait);
static int ip_proto_ioctl(struct socket *sock, unsigned int cmd,
			    unsigned long arg);
static int ip_proto_listen(struct socket *sock, int backlog);

static int ip_proto_send (struct socket *sock, void *buff, int len,
			  int nonblock, unsigned flags);
static int ip_proto_recv (struct socket *sock, void *buff, int len,
			  int nonblock, unsigned flags);
static int ip_proto_sendto (struct socket *sock, void *buff, int len,
			    int nonblock, unsigned flags,
			    struct sockaddr *addr, int addr_len);
static int ip_proto_recvfrom (struct socket *sock, void *buff, int len,
			      int nonblock, unsigned flags,
			      struct sockaddr *addr, int *addr_len);

static int ip_proto_shutdown (struct socket *sock, int how);


static int ip_proto_setsockopt (struct socket *sock, int level, int optname,
				char *optval, int optlen);
static int ip_proto_getsockopt (struct socket *sock, int level, int optname,
				char *optval, int *optlen);
static int ip_proto_fcntl (struct socket *sock, unsigned int cmd,
			   unsigned long arg);


struct proto_ops inet_proto_ops = 
{
  ip_proto_init,
  ip_proto_create,
  ip_proto_dup,
  ip_proto_release,
  ip_proto_bind,
  ip_proto_connect,
  ip_proto_socketpair,
  ip_proto_accept,
  ip_proto_getname, 
  ip_proto_read,
  ip_proto_write,
  ip_proto_select,
  ip_proto_ioctl,
  ip_proto_listen,
  ip_proto_send,
  ip_proto_recv,
  ip_proto_sendto,
  ip_proto_recvfrom,
  ip_proto_shutdown,
  ip_proto_setsockopt,
  ip_proto_getsockopt,
  ip_proto_fcntl
};

void
print_sk (volatile struct sock *sk)
{
  if (!sk) {
    PRINTK ("  print_sk(NULL)\n");
    return;
  }
  PRINTK ("  wmem_alloc = %d\n", sk->wmem_alloc);
  PRINTK ("  rmem_alloc = %d\n", sk->rmem_alloc);
  PRINTK ("  send_head = %X\n", sk->send_head);
  PRINTK ("  state = %d\n",sk->state);
  PRINTK ("  wback = %X, rqueue = %X\n", sk->wback, sk->rqueue);
  PRINTK ("  wfront = %X\n", sk->wfront);
  PRINTK ("  daddr = %X, saddr = %X\n", sk->daddr,sk->saddr);
  PRINTK ("  num = %d", sk->num);
  PRINTK (" next = %X\n", sk->next);
  PRINTK ("  send_seq = %d, acked_seq = %d, copied_seq = %d\n",
	  sk->send_seq, sk->acked_seq, sk->copied_seq);
  PRINTK ("  rcv_ack_seq = %d, window_seq = %d, fin_seq = %d\n",
	  sk->rcv_ack_seq, sk->window_seq, sk->fin_seq);
  PRINTK ("  prot = %X\n", sk->prot);
  PRINTK ("  pair = %X, back_log = %X\n", sk->pair,sk->back_log);
  PRINTK ("  inuse = %d , blog = %d\n", sk->inuse, sk->blog);
  PRINTK ("  dead = %d delay_acks=%d\n", sk->dead, sk->delay_acks);
  PRINTK ("  retransmits = %d, timeout = %d\n", sk->retransmits, sk->timeout);
  PRINTK ("  cong_window = %d, packets_out = %d\n", sk->cong_window,
	  sk->packets_out);
}

void
print_skb(struct sk_buff *skb)
{
  if (!skb) {
    PRINTK ("  print_skb(NULL)\n");
    return;
  }
  PRINTK ("  prev = %X, next = %X\n", skb->prev, skb->next);
  PRINTK ("  sk = %X link3 = %X\n", skb->sk, skb->link3);
  PRINTK ("  mem_addr = %X, mem_len = %d\n", skb->mem_addr, skb->mem_len);
  PRINTK ("  used = %d free = %d\n", skb->used,skb->free);
}

/* just used to reference some pointers to keep gcc from over optimizing
   my code so that it doesn't work. */
void dummy_routine(void *dummy, ...)
{
   return;
}

void
lock_skb (struct sk_buff *skb)
{
   if (skb->lock)
     {
	printk ("*** bug more than one lock on sk_buff. \n");
     }
   skb->lock = 1;
}


void
free_skb (struct sk_buff *skb, int rw)
{
   if (skb->lock)
     {
	skb->free = 1;
	return;
     }
   if (skb->sk)
     {
	if (rw)
	  {
	     skb->sk->prot->rfree (skb->sk, skb->mem_addr, skb->mem_len);
	  }
	else
	  {
	     skb->sk->prot->wfree (skb->sk, skb->mem_addr, skb->mem_len);
	  }
     }
   else
     {
	free_s (skb->mem_addr, skb->mem_len);
     }
}

void
unlock_skb (struct sk_buff *skb, int rw)
{
   if (skb->lock != 1)
     {
	printk ("*** bug unlocking non-locked sk_buff. \n");
     }
   skb->lock = 0;
   if (skb->free)
     free_skb (skb, rw);
}

static  int
sk_inuse( struct proto *prot, int num)
{
  volatile struct sock *sk;
  for (sk = prot->sock_array[num & (SOCK_ARRAY_SIZE -1 )];
       sk != NULL; sk=sk->next)
    {
      if (sk->dummy_th.source == num) return (1);
    }
  return (0);
}

unsigned short
get_new_socknum(struct proto *prot, unsigned short base)
{
  static int start=0;
  /* used to cycle through the port numbers so the chances of
     a confused connection drop. */

  int i,j;
  int best=0;
  int size=32767; /* a big num. */
  volatile struct sock *sk;
  start++;
  if (base == 0) base = PROT_SOCK+1+(start % 1024);
  if (base <= PROT_SOCK)
    {
      base += PROT_SOCK+(start % 1024);
    }

  /* now look through the entire array and try to find an empty
     ptr. */
  for (i = 0; i < SOCK_ARRAY_SIZE; i++)
    {
      j = 0;
      sk = prot->sock_array[(i+base+1) & (SOCK_ARRAY_SIZE -1)];
      while (sk != NULL)
	{
	  sk = sk->next;
	  j++;
	}
      if (j == 0) return (i+base+1);
      if (j < size) 
	{
	  best = i;
	  size = j;
	}
    }
  /* now make sure the one we want is not in use. */
  while (sk_inuse (prot, base +best+1))
    {
      best += SOCK_ARRAY_SIZE;
    }
  return (best+base+1);
  
}

void
put_sock(unsigned short num, volatile struct sock *sk)
{
   volatile struct sock *sk1;
   volatile struct sock *sk2;
   int mask;

   PRINTK ("put_sock (num = %d, sk = %X\n", num, sk);
   sk->num = num;
   sk->next = NULL;
   num = num & (SOCK_ARRAY_SIZE -1);

   /* we can't have an interupt renter here. */
   cli();
   if (sk->prot->sock_array[num] == NULL)
     {
	sk->prot->sock_array[num] = sk;
	sti();
	return;
     }
   sti();
   for (mask = 0xff000000; mask != 0xffffffff; mask = (mask >> 8) | mask)
     {
	if (mask & sk->saddr)
	  {
	     mask = mask << 8;
	     break;
	  }
     }

   PRINTK ("mask = %X\n", mask);

   cli();
   sk1 = sk->prot->sock_array[num];
   for (sk2 = sk1; sk2 != NULL; sk2=sk2->next)
     {
	if (!(sk2->saddr & mask))
	  {
	     if (sk2 == sk1)
	       {
		  sk->next = sk->prot->sock_array[num];
		  sk->prot->sock_array[num] = sk;
		  sti();
		  return;
	       }
	     sk->next = sk2;
	     sk1->next= sk;
	     sti();
	     return;
	  }
	sk1 = sk2;
     }
   /* goes at the end. */
   sk->next = NULL;
   sk1->next = sk;
   sti();
}


static void
remove_sock(volatile struct sock *sk1)
{
  volatile struct sock *sk2;
  PRINTK ("remove_sock(sk1=%X)\n",sk1);

  if (!sk1)
    {
      printk ("sock.c: remove_sock: sk1 == NULL\n");
      return;
    }

  if (!sk1->prot)
    {
      printk ("sock.c: remove_sock: sk1->prot == NULL\n");
      return;
    }

  /* we can't have this changing out from under us. */
  cli();
  sk2 = sk1->prot->sock_array[sk1->num & (SOCK_ARRAY_SIZE -1)];
  if (sk2 == sk1)
    {
       sk1->prot->sock_array[sk1->num & (SOCK_ARRAY_SIZE -1)] = sk1->next;
       sti();
       return;
    }

  while (sk2 && sk2->next != sk1)
    sk2 = sk2->next;

  if (sk2)
    {
      sk2->next = sk1->next;
      sti();
      return;
    }
  sti();

  if (sk1->num != 0)
    PRINTK ("remove_sock: sock  not found.\n");
}

void
destroy_sock(volatile struct sock *sk)
{

  struct sk_buff *skb;
  PRINTK ("destroying socket %X\n",sk);
  /* just to be safe. */
  sk->inuse = 1;

  remove_sock (sk);
  /* now we can no longer get new packets. */

  delete_timer((struct timer *)&sk->time_wait);

  /* cleanup up the write buffer. */
  for (skb = sk->wfront; skb != NULL; )
    {
      struct sk_buff *skb2;
      skb2=skb->next;
      free_skb(skb, FREE_WRITE);
      skb=skb2;
    }

  sk->wfront = NULL;

  if (sk->rqueue != NULL)
    {
       skb = sk->rqueue;
       do {
	  struct sk_buff *skb2;
	  skb2=skb->next;
	  /* this will take care of closing sockets that were
	     listening and didn't accept everything. */

	  if (skb->sk != NULL && skb->sk != sk)
	    {
	       skb->sk->dead = 1;
	       skb->sk->prot->close (skb->sk, 0);
	    }
	  free_skb(skb, FREE_READ);
	  skb=skb2;
       } while (skb != sk->rqueue);
    }

  sk->rqueue = NULL;

  /* now we need to clean up the send head. */
  for (skb = sk->send_head; skb != NULL; )
    {
      struct sk_buff *skb2;
      /* we need to remove skb from the transmit queue. */
      cli();
      /* see if it's in a transmit queue. */
      if (skb->next != NULL)
	{
	   if (skb->next != skb)
	     {
		skb->next->prev = skb->prev;
		skb->prev->next = skb->next;
	     }
	   else
	     {
		int i;
		for (i = 0; i < DEV_NUMBUFFS; i++)
		  {
		     if (skb->dev && skb->dev->buffs[i] == skb)
		       {
			  skb->dev->buffs[i]= NULL;
			  break;
		       }
		  }
	     }
	}
      sti();
      skb2=skb->link3;
      free_skb(skb, FREE_WRITE);
      skb=skb2;
    }

  sk->send_head = NULL;

  /* and now the backlog. */

  if (sk->back_log != NULL)
    {
       /* this should never happen. */
       printk ("cleaning back_log. \n");
       cli();
       skb = sk->back_log;
       do {
	  struct sk_buff *skb2;
	  skb2=skb->next;
	  free_skb(skb, FREE_READ);
	  skb=skb2;
       } while (skb != sk->back_log);
       sti();
    }

  sk->back_log = NULL;

  /* now if everything is gone we can free the socket structure, 
     otherwise we need to keep it around until everything is gone. */
  if (sk->rmem_alloc == 0 && sk->wmem_alloc == 0)
    {
       free_s ((void *)sk,sizeof (*sk));
    }
  else
    {
       /* this should never happen. */
       /* actually it can if an ack has just been sent. */
       PRINTK ("possible memory leak in socket = %X\n", sk);
       print_sk (sk);
       sk->destroy = 1;
       sk->ack_backlog = 0;
       sk->inuse = 0;
       sk->time_wait.len = SOCK_DESTROY_TIME;
       sk->timeout = TIME_DESTROY;
       reset_timer ((struct timer *)&sk->time_wait);
    }
  
}


static int
ip_proto_fcntl (struct socket *sock, unsigned int cmd, unsigned long arg)
{
   volatile struct sock *sk;
   sk=sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
   switch (cmd)
     {
       case F_SETOWN:
	sk->proc = arg;
	return (0);

       case F_GETOWN:
	return (sk->proc);

       default:
	return (-EINVAL);
     }
}

static int
ip_proto_setsockopt(struct socket *sock, int level, int optname,
		    char *optval, int optlen)
{
    volatile struct sock *sk;
    int val;
    /* This should really pass things on to the other levels. */
    if (level != SOL_SOCKET) return (-EOPNOTSUPP);
    sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
    verify_area (optval, sizeof (int));
    val = get_fs_long ((unsigned long *)optval);
    switch (optname)
      {
	case SO_TYPE:
	case SO_ERROR:
	default:
	  return (-ENOPROTOOPT);

	case SO_DEBUG: /* not implemented. */
	case SO_DONTROUTE:
	case SO_BROADCAST:
	case SO_SNDBUF:
	case SO_RCVBUF:
	  return (0);

	case SO_REUSEADDR:
	  if (val)
	    sk->reuse = 1;
	  else 
	    sk->reuse = 1;
	  return (0);

	case SO_KEEPALIVE:
	  if (val)
	    sk->keepopen = 1;
	  else
	    sk->keepopen = 0;
	  return (0);

	 case SO_OOBINLINE:
	  if (val)
	    sk->urginline = 1;
	  else
	    sk->urginline = 0;
	  return (0);

	 case SO_NO_CHECK:
	  if (val)
	    sk->no_check = 1;
	  else
	    sk->no_check = 0;
	  return (0);

	 case SO_PRIORITY:
	  if (val >= 0 && val < DEV_NUMBUFFS)
	    {
	       sk->priority = val;
	    }
	  else
	    {
	       return (-EINVAL);
	    }
	  return (0);

      }
}

static int
ip_proto_getsockopt(struct socket *sock, int level, int optname,
		    char *optval, int *optlen)
{
    volatile struct sock *sk;
    int val;
    /* This should really pass things on to the other levels. */
    if (level != SOL_SOCKET) return (-EOPNOTSUPP);
    sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
    switch (optname)
      {
	default:
	  return (-ENOPROTOOPT);

	case SO_DEBUG: /* not implemented. */
	case SO_DONTROUTE:
	case SO_BROADCAST:
	case SO_SNDBUF:
	case SO_RCVBUF:
	  val = 0;
	  break;

	case SO_REUSEADDR:
	  val = sk->reuse;
	  break;

	case SO_KEEPALIVE:
	  val = sk->keepopen;
	  break;

	case SO_TYPE:
	  if (sk->prot == &tcp_prot)
	    val = SOCK_STREAM;
	  else
	    val = SOCK_DGRAM;
	  break;

	case SO_ERROR:
	  val = sk->err;
	  sk->err = 0;
	  break;

	 case SO_OOBINLINE:
	  val = sk->urginline;
	  break;

	 case SO_NO_CHECK:
	  val = sk->no_check;
	  break;

	 case SO_PRIORITY:
	  val = sk->priority;
	  break;
      }
    verify_area (optlen, sizeof (int));
    put_fs_long (sizeof(int),(unsigned long *) optlen);

    verify_area(optval, sizeof (int));
    put_fs_long (val, (unsigned long *)optval);
    return (0);
}

static int
ip_proto_listen(struct socket *sock, int backlog)
{
  volatile struct sock *sk;
  sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }

  /* we may need to bind the socket. */
  if (sk->num == 0)
    {
      sk->num = get_new_socknum (sk->prot, 0);
      if (sk->num == 0) return (-EAGAIN);
      put_sock (sk->num, sk);
      sk->dummy_th.source = net16(sk->num);
    }

  sk->state = TCP_LISTEN;
  return (0);
}

/* Hardware should be inited here. */
static int ip_proto_init(void)
{
  int i;
  struct device *dev;
  struct ip_protocol *p;
  seq_offset = CURRENT_TIME*250;
  /* add all the protocols. */
  for (i = 0; i < SOCK_ARRAY_SIZE; i++)
    {
       tcp_prot.sock_array[i] = NULL;
       udp_prot.sock_array[i] = NULL;
       raw_prot.sock_array[i] = NULL;
    }

  for (p = ip_protocol_base; p != NULL;)
    {
       struct ip_protocol *tmp;
       /* add all the protocols. */
       tmp = p->next;
       add_ip_protocol (p);
       p = tmp;
    }

  /* add the devices */
  for (dev = dev_base; dev != NULL; dev=dev->next)
    {
       if (dev->init)
	 dev->init(dev);
    }
  timer_table[NET_TIMER].fn = net_timer;
  return (0);
}

static int
ip_proto_create (struct socket *sock, int protocol)
{
  volatile struct sock *sk;
  struct proto *prot;
  int err;

  sk = malloc (sizeof (*sk));
  if (sk == NULL)
    return (-ENOMEM);
  sk->num = 0;


  switch (sock->type)
    {
    case SOCK_STREAM:
    case SOCK_SEQPACKET:
       if (protocol && protocol != IPPROTO_TCP)
	 {
	    free_s ((void *)sk, sizeof (*sk));
	    return (-EPROTONOSUPPORT);
	 }
       sk->no_check = TCP_NO_CHECK;
       prot = &tcp_prot;
       break;

    case SOCK_DGRAM:
       if (protocol && protocol != IPPROTO_UDP)
	 {
	    free_s ((void *)sk, sizeof (*sk));
	    return (-EPROTONOSUPPORT);
	 }
       sk->no_check = UDP_NO_CHECK;
       prot=&udp_prot;
       break;
      
     case SOCK_RAW:
       if (!suser())
	 {
	    free_s ((void *)sk, sizeof (*sk));
	    return (-EPERM);
	 }

       if (!protocol)
	 {
	    free_s ((void *)sk, sizeof (*sk));
	    return (-EPROTONOSUPPORT);
	 }
       prot = &raw_prot;
       sk->reuse = 1;
       sk->no_check = 0; /* doesn't matter no checksum is preformed
			    anyway. */
       sk->num = protocol;
       break;

    case SOCK_PACKET:
       if (!suser())
	 {
	    free_s ((void *)sk, sizeof (*sk));
	    return (-EPERM);
	 }

       if (!protocol)
	 {
	    free_s ((void *)sk, sizeof (*sk));
	    return (-EPROTONOSUPPORT);
	 }
       prot = &packet_prot;
       sk->reuse = 1;
       sk->no_check = 0; /* doesn't matter no checksum is preformed
			    anyway. */
       sk->num = protocol;
       break;

      
    default:
       free_s ((void *)sk, sizeof (*sk));
       return (-ESOCKTNOSUPPORT);

    }
  sk->protocol = protocol;
  sk->wmem_alloc = 0;
  sk->rmem_alloc = 0;
  sk->pair = NULL;
  sk->opt = NULL;
  sk->send_seq = 0;
  sk->acked_seq = 0;
  sk->copied_seq = 0;
  sk->fin_seq = 0;
  sk->proc = 0;
  sk->rtt = TCP_WRITE_TIME;
  sk->packets_out = 0;
  sk->cong_window = 1; /* start with only sending one packet at a time. */
  sk->exp_growth = 1;  /* if set cong_window grow exponentially every time
			  we get an ack. */
  sk->urginline = 0;
  sk->intr = 0;
  sk->linger = 0;
  sk->destroy = 0;
  sk->reuse = 0;
  sk->priority = 1;
  sk->shutdown = 0;
  sk->urg = 0;
  sk->keepopen = 0;
  sk->done = 0;
  sk->ack_backlog = 0;
  sk->window = 0;
  sk->bytes_rcv = 0;
  sk->state = TCP_CLOSE;
  sk->dead = 0;
  sk->ack_timed = 0;

  /* this is how many unacked bytes we will accept for
     this socket.  */

  sk->max_unacked = 2048; /* needs to be at most 2 full packets. */

  /* how many packets we should send before forcing an ack. 
     if this is set to zero it is the same as sk->delay_acks = 0 */

  sk->max_ack_backlog = MAX_ACK_BACKLOG;
  sk->inuse = 0;
  sk->delay_acks = 1; /* default to waiting a while before sending
			 acks.  */
  sk->wback = NULL;
  sk->wfront = NULL;
  sk->rqueue = NULL;
  sk->mtu = 576;
  sk->prot = prot;
  sk->sleep = sock->wait;
  sk->daddr = 0;
  sk->saddr = MY_IP_ADDR;
  sk->err = 0;
  sk->next = NULL;
  sk->pair = NULL;
  sk->send_tail = NULL;
  sk->send_head = NULL;
  sk->time_wait.len = TCP_CONNECT_TIME;
  sk->time_wait.when = 0;
  sk->time_wait.sk = sk;
  sk->time_wait.next = NULL;
  sk->timeout = 0;
  sk->back_log = NULL;
  sk->blog = 0;
  sock->data =(void *) sk;
  sk->dummy_th.doff = sizeof (sk->dummy_th)/4;
  sk->dummy_th.res1=0;
  sk->dummy_th.res2=0;
  sk->dummy_th.urg_ptr = 0;
  sk->dummy_th.fin = 0;
  sk->dummy_th.syn = 0;
  sk->dummy_th.rst = 0;
  sk->dummy_th.psh = 0;
  sk->dummy_th.ack = 0;
  sk->dummy_th.urg = 0;
  sk->dummy_th.dest = 0;

  if (sk->num)
    {
      /* it assumes that any protocol which allows
	 the user to assign a number at socket
	 creation time automatically
	 shares. */
      put_sock (sk->num, sk);
      sk->dummy_th.source = net16(sk->num);
    }

  if (sk->prot->init)
    {
       err = sk->prot->init(sk);
       if (err != 0)
	 {
	    destroy_sock (sk);
	    return (err);
	 }
    }
  return (0);
}

static int
ip_proto_dup (struct socket *newsock, struct socket *oldsock)
{
  return (ip_proto_create (newsock,
			   ((volatile struct sock *)(oldsock->data))->protocol));
}

/* the peer socket should always be NULL. */
static int
ip_proto_release(struct socket *sock, struct socket *peer)
{
  volatile struct sock *sk;
  sk = sock->data;
  if (sk == NULL) return (0);
  wake_up (sk->sleep);
  /* start closing the connection.  This may take a while. */
  /* if linger is set, we don't return until the close is
     complete.  Other wise we return immediately.  The
     actually closing is done the same either way. */
  if (sk->linger == 0)
    {
       sk->prot->close(sk,0);
       sk->dead = 1;
    }
  else
    {
       sk->prot->close(sk, 0);
       cli();
       while (sk->state != TCP_CLOSE)
	 {
	    interruptible_sleep_on (sk->sleep);
	    if (current->signal & ~current->blocked)
	      {
		 sti();
		 return (-ERESTARTSYS);
	      }
	 }
       sti();
       sk->dead = 1;
    }

  sk->inuse = 1;
  /* this will destroy it. */
  release_sock (sk);
  sock->data = NULL;
  return (0);
}


/* this needs to be changed to dissallow
   the rebinding of sockets.   What error
   should it return? */

static int
ip_proto_bind (struct socket *sock, struct sockaddr *uaddr,
	       int addr_len)
{
  struct sockaddr_in addr;
  volatile struct sock *sk, *sk2;
  unsigned short snum;

  sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
  /* check this error. */
  if (sk->state != TCP_CLOSE) return (-EIO);
  if (sk->num != 0) return (-EINVAL);

  verify_area (uaddr, addr_len);
  memcpy_fromfs (&addr, uaddr, min (sizeof (addr), addr_len));
  if (addr.sin_family && addr.sin_family != AF_INET)
    return (-EINVAL); /* this needs to be changed. */

  snum = net16(addr.sin_port);
  PRINTK ("bind sk =%X to port = %d\n", sk, snum);
  print_sk (sk);
  sk = sock->data;

  /* we can't just leave the socket bound wherever it is, it might be bound
     to a priveledged port. However, since there seems to be a bug here,
     we will leave it if the port is not priveledged(sp?) */

  if (snum == 0)
    {
       if ( sk->num > PROT_SOCK) return (0);
       snum = get_new_socknum (sk->prot, 0);
    }

  if (snum <= PROT_SOCK && !suser())
    return (-EACCES);

  if (my_ip_addr(addr.sin_addr.s_addr) || addr.sin_addr.s_addr == 0)
    sk->saddr = addr.sin_addr.s_addr;

  PRINTK ("sock_array[%d] = %X:\n", snum & (SOCK_ARRAY_SIZE -1),
	  sk->prot->sock_array[snum & (SOCK_ARRAY_SIZE -1)]);
  print_sk (sk->prot->sock_array[snum & (SOCK_ARRAY_SIZE -1)]);

  /* make sure we are allowed to bind here. */
  for (sk2 = sk->prot->sock_array[snum & (SOCK_ARRAY_SIZE -1)];
       sk2 != NULL;
       sk2 = sk2->next)
    {
       if (sk2->num != snum) continue;
       if (sk2->saddr != sk->saddr) continue;
       if (!sk->reuse) return (-EADDRINUSE);
       if (!sk2->reuse) return (-EADDRINUSE);
    }
  remove_sock (sk);
  put_sock(snum, sk);
  sk->dummy_th.source = net16(sk->num);
  sk->daddr = 0;
  sk->dummy_th.dest = 0;
  return (0);
}

static int
ip_proto_connect (struct socket *sock, struct sockaddr * uaddr,
		  int addr_len, int flags)
{
  volatile struct sock *sk;
  int err;
  sock->conn = NULL;
  sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }

  /* we may need to bind the socket. */
  if (sk->num == 0)
    {
      sk->num = get_new_socknum (sk->prot, 0);
      if (sk->num == 0) return (-EAGAIN);
      put_sock (sk->num, sk);
      sk->dummy_th.source = net16(sk->num);
    }

  if (sk->prot->connect == NULL)
    return (-EOPNOTSUPP);

  if (sk->intr == 0)
    {
      err = sk->prot->connect (sk, (struct sockaddr_in *)uaddr, addr_len);
      if (err < 0) return (err);
    }

  sock->state = SS_CONNECTED;

  if (flags & O_NONBLOCK) return (0);

  cli(); /* avoid the race condition */

  while (sk->state != TCP_ESTABLISHED && sk->state < TCP_CLOSING)
    {
      interruptible_sleep_on (sk->sleep);
      if (current->signal & ~current->blocked)
	{
	   sti();
	   sk->intr = 1;
	   return (-ERESTARTSYS);
	}
    }
  sti();
  sk->intr = 0;
  if (sk->state != TCP_ESTABLISHED && sk->err)
    {
      return (-sk->err);
    }
  return (0);
}

static int
ip_proto_socketpair (struct socket *sock1, struct socket *sock2)
{
  return (-EOPNOTSUPP);
}

static int
ip_proto_accept (struct socket *sock, struct socket *newsock, int flags)
{
  volatile struct sock *sk1, *sk2;
  sk1= sock->data;
   if (sk1 == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
  newsock->data = NULL;
  if (sk1->prot->accept == NULL) return (-EOPNOTSUPP);
  /* restore the state if we have been interrupted, and
     then returned. */
  if (sk1->pair != NULL )
    {
      sk2 = sk1->pair;
      sk1->pair = NULL;
    }
  else
    {
      sk2 = sk1->prot->accept (sk1,flags);
      if (sk2 == NULL)
	{
	  if (sk1->err <= 0)
	    printk ("Warning sock.c:sk1->err <= 0.  Returning non-error.\n");
	  return (-sk1->err);
	}
    }
  newsock->data = (void *)sk2;
  sk2->sleep = (void *)newsock->wait;
  newsock->conn = NULL;
  if (flags & O_NONBLOCK)
    return (0);

  cli(); /* avoid the race. */
  while (sk2->state == TCP_SYN_RECV)
    {
      interruptible_sleep_on (sk2->sleep);
      if (current->signal & ~current->blocked)
	{
	   sti();
	   sk1->pair = sk2;
	   sk2->sleep = NULL;
	   newsock->data = NULL;
	   return (-ERESTARTSYS);
	}
    }
  sti();

  if (sk2->state != TCP_ESTABLISHED && sk2->err > 0)
    {
      int err;
      err = -sk2->err;
      destroy_sock (sk2);
      newsock->data = NULL;
      return (err);
    }
  newsock->state = SS_CONNECTED;
  return (0);
}

static int
ip_proto_getname(struct socket *sock, struct sockaddr *uaddr,
		 int *uaddr_len, int peer)
{
  struct sockaddr_in sin;
  volatile struct sock *sk;
  int len;
  verify_area(uaddr_len, sizeof (len));
  len = get_fs_long(uaddr_len);
  /* check this error. */
  if (len < sizeof (sin)) return (-EINVAL);
  verify_area (uaddr, len);
  sin.sin_family=AF_INET;
  sk = sock->data;
  if (sk == NULL)
    {
       printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
       return (0);
    }
  if (peer)
    {
      if (sk->state != TCP_ESTABLISHED)
	return (-ENOTCONN);
      sin.sin_port = sk->dummy_th.dest;
      sin.sin_addr.s_addr = sk->daddr;
      }
  else
    {
      sin.sin_port = sk->dummy_th.source;
      sin.sin_addr.s_addr = sk->saddr;
    }
  len = sizeof (sin);
  memcpy_tofs(uaddr, &sin, sizeof (sin));
  put_fs_long (len, uaddr_len);
  return (0);
}

static int
ip_proto_read (struct socket *sock, char *ubuf, int size, int noblock)
{
  volatile struct sock *sk;
  sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
  if (sk->shutdown & RCV_SHUTDOWN)
    return (-EIO);

  /* we may need to bind the socket. */
  if (sk->num == 0)
    {
      sk->num = get_new_socknum (sk->prot, 0);
      if (sk->num == 0) return (-EAGAIN);
      put_sock (sk->num, sk);
      sk->dummy_th.source = net16(sk->num);
    }

  return (sk->prot->read (sk, ubuf, size, noblock,0));
}

static int
ip_proto_recv (struct socket *sock, void *ubuf, int size, int noblock,
	       unsigned flags)
{
  volatile struct sock *sk;
  sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
  if (sk->shutdown & RCV_SHUTDOWN)
    return (-EIO);

  /* we may need to bind the socket. */
  if (sk->num == 0)
    {
      sk->num = get_new_socknum (sk->prot, 0);
      if (sk->num == 0) return (-EAGAIN);
      put_sock (sk->num, sk);
      sk->dummy_th.source = net16(sk->num);
    }

  return (sk->prot->read (sk, ubuf, size, noblock, flags));
}

static int
ip_proto_write (struct socket *sock, char *ubuf, int size, int noblock)
{
  volatile struct sock *sk;
  sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
  if (sk->shutdown & SEND_SHUTDOWN)
    return (-EIO);

  /* we may need to bind the socket. */
  if (sk->num == 0)
    {
      sk->num = get_new_socknum (sk->prot, 0);
      if (sk->num == 0) return (-EAGAIN);
      put_sock (sk->num, sk);
      sk->dummy_th.source = net16(sk->num);
    }

  return (sk->prot->write (sk, ubuf, size, noblock, 0));
}


static int
ip_proto_send (struct socket *sock, void *ubuf, int size, int noblock, 
	       unsigned flags)
{
  volatile struct sock *sk;
  sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
  if (sk->shutdown & SEND_SHUTDOWN)
    return (-EIO);

  /* we may need to bind the socket. */
  if (sk->num == 0)
    {
      sk->num = get_new_socknum (sk->prot, 0);
      if (sk->num == 0) return (-EAGAIN);
      put_sock (sk->num, sk);
      sk->dummy_th.source = net16(sk->num);
    }

  return (sk->prot->write (sk, ubuf, size, noblock, flags));
}


static int
ip_proto_sendto (struct socket *sock, void *ubuf, int size, int noblock, 
		 unsigned flags, struct sockaddr *sin, int addr_len )
{
  volatile struct sock *sk;
  sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
  if (sk->shutdown & SEND_SHUTDOWN)
    return (-EIO);
  if (sk->prot->sendto == NULL) return (-EOPNOTSUPP);

  /* we may need to bind the socket. */
  if (sk->num == 0)
    {
      sk->num = get_new_socknum (sk->prot, 0);
      if (sk->num == 0) return (-EAGAIN);
      put_sock (sk->num, sk);
      sk->dummy_th.source = net16(sk->num);
    }

  return (sk->prot->sendto (sk, ubuf, size, noblock, flags, 
			    (struct sockaddr_in *)sin, addr_len));
}

static int
ip_proto_recvfrom (struct socket *sock, void *ubuf, int size, int noblock, 
		   unsigned flags, struct sockaddr *sin, int *addr_len )
{
  volatile struct sock *sk;
  sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }
  if (sk->shutdown & RCV_SHUTDOWN)
    return (-EIO);
  if (sk->prot->recvfrom == NULL) return (-EOPNOTSUPP);

  /* we may need to bind the socket. */
  if (sk->num == 0)
    {
      sk->num = get_new_socknum (sk->prot, 0);
      if (sk->num == 0) return (-EAGAIN);
      put_sock (sk->num, sk);
      sk->dummy_th.source = net16(sk->num);
    }

  return (sk->prot->recvfrom (sk, ubuf, size, noblock, flags,
			      (struct sockaddr_in*)sin, addr_len));
}

static int
ip_proto_shutdown (struct socket *sock, int how)
{
	volatile struct sock *sk;
	/* this should really check to make sure the socket is
	   a tcp socket. */
	how++; /* maps 0->1 has the advantage of making bit 1 rcvs and
		       1->2 bit 2 snds.
		       2->3 */
	if (how & ~SHUTDOWN_MASK) return (-EINVAL);
	sk = sock->data;
	if (sk == NULL)
	  {
	     printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	     return (0);
	  }
	if (sk->state != TCP_ESTABLISHED) return (-ENOTCONN);
	sk->shutdown |= how;
	return (0);
}

static int
ip_proto_select (struct socket *sock, int sel_type, select_table *wait )
{
  volatile struct sock *sk;
  sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }

  if (sk->prot->select == NULL)
    {
       PRINTK ("select on non-selectable socket. \n");
       return (0);
    }
  return (sk->prot->select(sk, sel_type, wait));
}

/* these should be distributed to the different protocol routines. */
static int
ip_proto_ioctl (struct socket *sock, unsigned int cmd, 
		unsigned long arg)
{
   volatile struct sock *sk;
   sk = sock->data;
   if (sk == NULL)
     {
	printk ("Warning: sock->data = NULL: %d\n" ,__LINE__);
	return (0);
     }

  PRINTK ("in ip_proto_ioctl\n");
  switch (cmd)
    {

      case IP_SET_DEV:
       if (!suser())
	 return (-EPERM);
       return (ip_set_dev((struct ip_config *)arg));

     case FIOSETOWN:
     case SIOCSPGRP:
        {
	 long user;
	 user = get_fs_long ((void *) arg);
	 sk->proc = user;
	 return (0);
       }

     case FIOGETOWN:
     case SIOCGPGRP:
       {
	 verify_area ((void *)arg, sizeof (long));
	 put_fs_long (sk->proc, (void *)arg);
	 return (0);
       }

    default:
       if (!sk->prot->ioctl)
	 return (-EINVAL);
       return (sk->prot->ioctl (sk, cmd, arg));
    }
}

#ifdef MEM_DEBUG

struct mem
{
   unsigned long check;
   struct mem *other;
   unsigned long len;
   unsigned short buff[10];
};

static void
print_mem (struct mem *m)
{
   int i;
   MPRINTK("mem:\n");
   MPRINTK("  check=%X, other = %X\n", m->check, m->other);
   MPRINTK("  len=%d buff:\n " , m->len);
   for (i = 0; i < 10; i++)
     {
	MPRINTK ("0x%02X ",m->buff[i]);
     }
   MPRINTK ("\n");
}

static void *
smalloc (unsigned long size)
{
   struct mem *head, *tail;
   static unsigned short count;
   int i;
   int sum;
   unsigned char *ptr;

   MPRINTK ("smalloc (size = %d)\n",size);
   head = malloc (size + 2*sizeof (*head));
   if (head == NULL) return (NULL);
   tail = (struct mem *)((unsigned char *)(head+1) + size); 

   head->other = tail;
   tail->other = head;

   tail->len = size;
   head->len = size;
   for (i = 0; i < 10; i++)
     {
	tail->buff[i]=count++;
	head->buff[i]=count;
     }

   ptr = (unsigned char *)head;
   head->check = 0;
   sum = 0;

   for (i = 0; i < sizeof (*head); i ++)
     {
	sum+= ptr[i]; 
     }

   head->check = ~sum;
   ptr = (unsigned char *)tail;
   tail->check = 0;
   sum = 0;

   for (i = 0; i < sizeof (*head); i ++)
     {
	sum+= ptr[i]; 
     }

   tail->check = ~sum;
   MPRINTK ("head = %X:\n", head);
   print_mem(head);
   MPRINTK ("tail = %X:\n", tail);
   print_mem(tail);
   return (head+1);
}

void
sfree (void *data, unsigned long len)
{
   int i;
   int sum;
   int csum;
   unsigned char *ptr;
   int bad = 0;
   struct mem *head, *tail;
   MPRINTK ("sfree(data=%X, len = %d)\n", data, len);
   head = data;
   head--;
   tail = (struct mem *)((unsigned char *)(head+1) + len);
   print_mem (head);
   print_mem (tail);
   if (head->other != tail)
     {
	MPRINTK ("sfree: head->other != tail:\n");
	bad = 1;
     }
   if (tail->other != head)
     {
	MPRINTK ("sfree: tail->other != head:\n");
	bad =1 ;
     }
   if (head ->len != len)
     {
	MPRINTK ("sfree: head->len != len");
	bad = 1;
     }
   if (tail ->len != len)
     {
	MPRINTK ("sfree: tail->len != len");
	bad = 1;
     }
   csum = head->check;
   ptr = (unsigned char *)head;
   head->check = 0;
   sum = 0;
   for (i = 0; i < sizeof (*head); i ++)
     {
	sum+= ptr[i]; 
     }
   if (csum != ~sum)
     {
	MPRINTK ("sfree: head failed checksum\n");
	bad = 1;
     }
   csum = tail->check;
   ptr = (unsigned char *)tail;
   tail->check = 0;
   sum = 0;
   for (i = 0; i < sizeof (*head); i ++)
     {
	sum+= ptr[i]; 
     }
   if (csum != ~sum)
     {
	MPRINTK ("sfree: tail failed checksum\n");
	bad = 1;
     }
   if (!bad)
     free_s (head, len+2*sizeof (*head));
   else
     schedule();
}
#else
static  void *
smalloc (unsigned long size)
{
   return (malloc (size));
}
static  void
sfree(void *data, unsigned long len)
{
   free_s(data,len);
}
#endif

void *
sock_wmalloc(volatile struct sock *sk, unsigned long size, int force)
{
  void *tmp;
  if (sk)
    {
       if (sk->wmem_alloc + size >= SK_WMEM_MAX && !force)
	 {
	    MPRINTK ("sock_wmalloc(%X,%d,%d) returning NULL\n",
		     sk, size, force);
	    return (NULL);
	 }
      cli();
      sk->wmem_alloc+= size;
      sti();
    }
   if (sk)
     tmp = smalloc (size);
   else
     tmp = malloc (size);

  MPRINTK ("sock_wmalloc(%X,%d,%d) returning %X\n",sk, size, force, tmp);
  return (tmp);
}

void *
sock_rmalloc(volatile struct sock *sk, unsigned long size, int force)
{
   struct mem *tmp;
   if (sk )
     {
	if (sk->rmem_alloc + size >= SK_RMEM_MAX && !force)
	  {
	     MPRINTK ("sock_rmalloc(%X,%d,%d) returning NULL\n",sk,size,force);
	     return (NULL);
	  }
	cli();
	sk->rmem_alloc+= size;
	sti();
     }
   if (sk)
     tmp = smalloc (size);
   else
     tmp = malloc (size);

   MPRINTK ("sock_rmalloc(%X,%d,%d) returning %X\n",sk, size, force, tmp);
   return (tmp);
}


unsigned long
sock_rspace (volatile struct sock *sk)
{
   int amt;
   if (sk != NULL)
     {
	if (sk->rmem_alloc >= SK_RMEM_MAX-2*MIN_WINDOW) return (0);
	amt = min ((SK_RMEM_MAX-sk->rmem_alloc)/2-MIN_WINDOW, MAX_WINDOW);
	if (amt < 0) return (0);
	return (amt);
     }
   return (0);
}

unsigned long
sock_wspace (volatile struct sock *sk)
{
  if (sk != NULL)
    {
       if (sk->shutdown & SEND_SHUTDOWN) return (0);
       if (sk->wmem_alloc >= SK_WMEM_MAX) return (0);
       return (SK_WMEM_MAX-sk->wmem_alloc );
    }
  return (0);
}


void
sock_wfree (volatile struct sock *sk, void *mem, unsigned long size)
{
   MPRINTK ("sock_wfree (sk=%X, mem=%X, size=%d)\n",sk, mem, size);
   if (sk)
     {
	sk->wmem_alloc -= size;
	sfree(mem,size);
	/* in case it might be waiting for more memory. */
	if (!sk->dead && sk->wmem_alloc > SK_WMEM_MAX/2) wake_up(sk->sleep);
	if (sk->destroy && sk->wmem_alloc == 0 && sk->rmem_alloc == 0)
	  {
	     MPRINTK ("recovered lost memory, destroying sock = %X\n",sk);
	     delete_timer ((struct timer *)&sk->time_wait);
	     free_s ((void *)sk, sizeof (*sk));
	  }
     }
   else
     {
	free_s (mem, size);
     }
}

void
sock_rfree (volatile struct sock *sk, void *mem, unsigned long size)
{
   MPRINTK ("sock_rfree (sk=%X, mem=%X, size=%d)\n",sk, mem, size);
   if (sk)
     {
	sk->rmem_alloc -= size;
	sfree(mem,size);
	if (sk->destroy && sk->wmem_alloc == 0 && sk->rmem_alloc == 0)
	  {
	     delete_timer ((struct timer *)&sk->time_wait);
	     free_s ((void *)sk, sizeof (*sk));
	  }
     }
   else
     {
	free_s (mem, size);
     }
}


/* This routine must find a socket given a tcp header.  Everyhting
   is assumed to be in net order. */

volatile struct sock *get_sock (struct proto *prot, unsigned short num,
				unsigned long raddr,
				unsigned short rnum, unsigned long laddr)
{
  volatile struct sock *s;
  PRINTK ("get_sock (prot=%X, num=%d, raddr=%X, rnum=%d, laddr=%X)\n",
	  prot, num, raddr, rnum, laddr);

  /* SOCK_ARRAY_SIZE must be a power of two.  This will work better
     than a prime unless 3 or more sockets end up using the same
     array entry.  This should not be a problem because most
     well known sockets don't overlap that much, and for
     the other ones, we can just be careful about picking our
     socket number when we choose an arbitrary one. */

  for (s=prot->sock_array[num&(SOCK_ARRAY_SIZE-1)]; s != NULL; s=s->next)
    {
      if (s->num == num)
	{
	  /* we need to see if this is the socket that we want. */
	  if (!ip_addr_match (s->daddr, raddr))
	    continue;
	  if (s->dummy_th.dest != rnum && s->dummy_th.dest != 0)
	    continue;
	  if (!ip_addr_match (s->saddr, laddr))
	    continue;
	  return (s);
	}
    }
  return (NULL);
}

void release_sock (volatile struct sock *sk)
{
  if (!sk)
    {
      printk ("sock.c: release_sock sk == NULL\n");
      return;
    }

  if (!sk->prot)
    {
      printk ("sock.c: release_sock sk->prot == NULL\n");
      return;
    }

  if (sk->blog) return;
  /* see if we have any packets built up. */

  cli();
  sk->inuse = 1;
  while (sk->back_log != NULL)
    {
      struct sk_buff *skb;
      sk->blog = 1;
      skb = sk->back_log;
      PRINTK ("release_sock: skb = %X:\n",skb);
      print_skb(skb);
      if (skb->next != skb)
	{
	  sk->back_log = skb->next;
	  skb->prev->next = skb->next;
	  skb->next->prev = skb->prev;
	}
      else
	{
	  sk->back_log = NULL;
	}
      sti();
      PRINTK ("sk->back_log = %X\n",sk->back_log);
      if (sk->prot->rcv)
	sk->prot->rcv(skb, skb->dev, sk->opt,
		      skb->saddr, skb->len, skb->daddr, 1,
		      /* only used for/by raw sockets. */
		      (struct ip_protocol *)sk->pair); 
      cli();
    }
  sk->blog = 0;
  sk->inuse = 0;
  sti();
  if (sk->dead && sk->state == TCP_CLOSE)
    {
        /* should be about 2 rtt's */
       sk->time_wait.len = min (sk->rtt * 2, TCP_DONE_TIME);
       sk->timeout = TIME_DONE;
       reset_timer ((struct timer *)&sk->time_wait);
    }
}
