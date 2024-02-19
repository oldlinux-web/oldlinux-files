 /* tcp.c */
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
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include <linux/fcntl.h>
#include "timer.h"
#include "ip.h"
#include "icmp.h"
#include "tcp.h"
#include "sock.h"
#include <linux/errno.h>
#include <linux/timer.h>
#include <asm/system.h>
#include <asm/segment.h>
/* #include <signal.h>*/
#include <linux/termios.h> /* for ioctl's */
#include "../kern_sock.h" /* for PRINTK */

#define tmax(a,b) (before ((a),(b)) ? (b) : (a))
#define swap(a,b) {unsigned long c; c=a; a=b; b=c;}

extern struct proto tcp_prot;

static  int 
min (unsigned int a, unsigned int b)
{
   if (a < b) return (a);
   return (b);
}

void
print_th (struct tcp_header *th)
{
   unsigned char *ptr;
   ptr = (unsigned char *)(th + 1);
   PRINTK ("tcp header:\n");
   PRINTK ("  source=%d, dest=%d, seq =%d, ack_seq = %d\n",
	   net16(th->source), net16(th->dest), net32(th->seq),
	   net32(th->ack_seq));
   PRINTK ("  fin=%d, syn=%d, rst=%d, psh=%d, ack=%d, urg=%d res1=%d res2=%d\n"
	   ,th->fin, th->syn, th->rst, th->psh, th->ack, th->urg,
	   th->res1, th->res2);
   PRINTK ("  window = %d, check = %d urg_ptr = %d\n",
	   net16(th->window), net16(th->check), net16(th->urg_ptr));
   PRINTK ("  doff = %d\n",th->doff);
   PRINTK ("options = %d %d %d %d\n", ptr[0], ptr[1], ptr[2], ptr[3]);
 }

 /* This routine grabs the first thing off of a rcv queue. */
static  struct sk_buff *
get_firstr(volatile struct sock *sk)
{
   struct sk_buff *skb;
   skb = sk->rqueue;
   if (skb == NULL) return (NULL);
   sk->rqueue = skb->next;
   if (sk->rqueue == skb)
     {
       sk->rqueue = NULL;
     }
   else
     {
       sk->rqueue->prev=skb->prev;
       sk->rqueue->prev->next = sk->rqueue;
     }
   return (skb);
}

static  long
diff (unsigned long seq1, unsigned long seq2)
{
   long d;
   d=seq1-seq2;
   if (d > 0) return (d);
   /* I hope this returns what I want. */
   return (~d+1);
}

 /* enter the time wait state. */
static  void
tcp_time_wait (volatile struct sock *sk)
{
   sk->state = TCP_TIME_WAIT;
   sk->time_wait.len = TCP_TIMEWAIT_LEN;
   sk->timeout = TIME_CLOSE;
   reset_timer ((struct timer *)&sk->time_wait);
}

static void
tcp_retransmit (volatile struct sock *sk, int all)
{
   if (all) 
     {
	ip_retransmit (sk, all);
	return;
     }
   sk->rtt *= 2; /* exponential back off. */
   if (sk->cong_window > 1)
     sk->cong_window = sk->cong_window / 2;
   sk->exp_growth = 0;

   /* do the actuall retransmit. */
   ip_retransmit (sk, all);
   
}

/* this routine is called by the icmp module when it gets some
   sort of error condition.  If err < 0 then the socket should
   be closed and the error returned to the user.  If err > 0
   it's just the icmp type << 8 | icmp code.  
   header points to the first 8 bytes of the tcp header.  We need
   to find the appropriate port. */

void
tcp_err (int err, unsigned char *header, unsigned long daddr,
	 unsigned long saddr, struct ip_protocol *protocol)
{
   struct tcp_header *th;
   volatile struct sock *sk;
   
   th = (struct tcp_header *)header;
   sk = get_sock (&tcp_prot, net16(th->dest), saddr, th->source, daddr);

   if (sk == NULL) return;

   if (err & 0xff00 == (ICMP_SOURCE_QUENCH << 8))
     {
	/* for now we will just trigger a linear backoff. The slow start
	   code should cause a real backoff here. */

	if (sk->cong_window > 1)
	  sk->cong_window --;

	return;
     }

   sk->err = icmp_err_convert[err & 0xff].errno;
   if (icmp_err_convert[err & 0xff].fatal)
     {
	if (sk->state != TCP_ESTABLISHED)
	  sk->state = TCP_CLOSE;
	sk->prot->close(sk, 0);
     }

   return;
   
}

static int
tcp_select (volatile struct sock *sk, int sel_type, select_table *wait)
{
  switch (sel_type)
    {
    case SEL_IN:
       select_wait (sk->sleep, wait);
       if (sk->rqueue != NULL &&
	   (between (sk->copied_seq, sk->rqueue->next->h.th->seq - 1,
		     sk->rqueue->next->h.th->seq + sk->rqueue->next->len) ||
	    sk->state == TCP_LISTEN))
	 {
	    return (1);
	 }

      switch (sk->state)
	{
	case TCP_LISTEN:
	case TCP_ESTABLISHED:
	case TCP_SYN_SENT:
	case TCP_SYN_RECV:
	   return (0);
	default:
	   return (1);
	}

    case SEL_OUT:
       select_wait (sk->sleep, wait);
       if (sk->state != TCP_ESTABLISHED) return (1);
      /* hack so it will probably be able to write something
	 if it says it's ok to write. */
      if (sk->prot->wspace(sk) >= MIN_WRITE_SPACE) return (1);
      return (0);

    case SEL_EX:
      select_wait(sk->sleep,wait);
      if (sk->err) return (1);
      if (sk->state == TCP_FIN_WAIT1 ||
	  sk->state == TCP_FIN_WAIT2 ||
	  sk->state == TCP_TIME_WAIT ||
	  sk->state == TCP_LAST_ACK)
	return (1);
      return (0);
    }
  return (0);
}

static int
tcp_ioctl (volatile struct sock *sk, int cmd, unsigned long arg)
{
   switch (cmd)
     {
       default:
	return (-EINVAL);

      case TIOCINQ:
/*      case FIONREAD:*/
	{
	  unsigned long amount;
	  struct sk_buff *skb;

	  if (sk->state == TCP_LISTEN)
	    return (-EINVAL);

	  amount = 0;
	  if (sk->rqueue != NULL)
	    {
	      skb = sk->rqueue->next;
	      /* go until a push or until we are out of data. */
	      do {
		amount += skb -> len;
		if (skb->h.th->psh) break;
		skb = skb->next;
	      } while (skb != sk->rqueue->next);
	    }

	  verify_area ((void *)arg, sizeof (unsigned long));
	  put_fs_long (amount, (unsigned long *)arg);
	  return (0);
	}

       case SIOCATMARK:
	{
	  struct sk_buff *skb;
	  int answ=0;
	  /* try to figure out if we need to read some urgent data. */
	  if (sk->rqueue != NULL)
	    {
	      skb = sk->rqueue->next;
	      if (sk->copied_seq+1 == skb->h.th->seq && skb->h.th->urg)
		answ = 1;
	    }
	  verify_area ((void *) arg, sizeof (unsigned long));
	  put_fs_long (answ, (void *) arg);
	  return (0);
	}
       
      case TIOCOUTQ:
	{
	  unsigned long amount;
	  if (sk->state == TCP_LISTEN)
	    return (-EINVAL);
	  amount = sk->prot->wspace(sk)/2;
	  verify_area ((void *)arg, sizeof (unsigned long));
	  put_fs_long (amount, (unsigned long *)arg);
	  return (0);
	}

     }
}


/* this routine computes a tcp checksum */
static  unsigned short
tcp_check (struct tcp_header *th, int len, unsigned long saddr,
	   unsigned long daddr)
{     
   unsigned long sum;
   
   if (saddr == 0) saddr = MY_IP_ADDR;
   print_th (th);
   __asm__("\t addl %%ecx,%%ebx\n"
	   "\t adcl %%edx,%%ebx\n"
	   "\t adcl $0, %%ebx\n"
	   : "=b" (sum)
	   : "0" (daddr), "c" (saddr), "d" ((net16(len) << 16) + IPPROTO_TCP*256)
	   : "cx","bx","dx" );
   
   if (len > 3)
     {
	__asm__(
		"\tclc\n"
		"1:\n"
		"\t lodsl\n"
		"\t adcl %%eax, %%ebx\n"
		"\t loop 1b\n"
		"\t adcl $0, %%ebx\n"
		: "=b" (sum) , "=S" (th)
		: "0" (sum), "c" (len/4) ,"1" (th)
		: "ax", "cx", "bx", "si" );
     }
   
   /* convert from 32 bits to 16 bits. */
   __asm__(
	   "\t movl %%ebx, %%ecx\n"
	   "\t shrl $16,%%ecx\n"
	   "\t addw %%cx, %%bx\n"
	   "\t adcw $0, %%bx\n"
	   : "=b" (sum)
	   : "0" (sum)
	   : "bx", "cx");
   
   /* check for an extra word. */
   if ((len & 2) != 0)
     {
	__asm__("\t lodsw\n"
		"\t addw %%ax,%%bx\n"
		"\t adcw $0, %%bx\n"
		: "=b" (sum), "=S" (th)
		: "0" (sum) ,"1" (th)
		: "si", "ax", "bx");
     }
   
   /* now check for the extra byte. */
   if ((len & 1) != 0)
     {
	__asm__("\t lodsb\n"
		"\t movb $0,%%ah\n"
		"\t addw %%ax,%%bx\n"
		"\t adcw $0, %%bx\n"
		: "=b" (sum)
		: "0" (sum) ,"S" (th)
		: "si", "ax", "bx");
     }
   
   /* we only want the bottom 16 bits, but we never cleared
      the top 16. */
   return ((~sum) & 0xffff);
}


static  void
tcp_send_check (struct tcp_header *th, unsigned long saddr, 
		 unsigned long daddr, int len, volatile struct sock *sk)
 {

   th->check = 0;
   if (sk && sk->no_check) return;
   th->check = tcp_check (th, len, saddr, daddr);
   return;
}

 /* This routine sends an ack and also updates the window. */
 static  void
 tcp_send_ack (unsigned long sequence, unsigned long ack,
	       volatile struct sock *sk,
	       struct tcp_header *th, unsigned long daddr)
 {
   struct sk_buff *buff;
   struct tcp_header *t1;
   struct device *dev=NULL;
   int tmp;

   /* we need to grab some memory, and put together an ack, and then
      put it into the queue to be sent. */

   buff=sk->prot->wmalloc(sk,MAX_ACK_SIZE,1);
   if (buff == NULL) 
     {
	/* force it to send an ack. */
	sk->ack_backlog++;
	if (sk->timeout != TIME_WRITE && sk->state < TCP_CLOSING)
	  {
	     sk->timeout = TIME_WRITE;
	     sk->time_wait.len = 10; /* got to do it quickly. */
	     reset_timer ((struct timer *)&sk->time_wait);
	  }
	return;
     }

   buff->mem_addr = buff;
   buff->mem_len = MAX_ACK_SIZE;
   buff->len=sizeof (struct tcp_header);
   buff->sk = sk;
   t1 = (struct tcp_header *)(buff + 1);
   /* put in the ip_header and routing stuff. */
   tmp = sk->prot->build_header (buff, sk->saddr, daddr, &dev,
				 IPPROTO_TCP, sk->opt, MAX_ACK_SIZE);
   if (tmp < 0)
     {
       sk->prot->wfree(sk, buff->mem_addr, buff->mem_len);
       return;
     }
   buff->len += tmp;
   t1 = (struct tcp_header *)((char *)t1 +tmp);

   memcpy (t1, th, sizeof (*t1)); /* this should probably be removed. */

   /* swap the send and the receive. */
   t1->dest = th->source;
   t1->source = th->dest;
   t1->seq = net32(sequence);
   t1->ack = 1;
   sk->window = sk->prot->rspace(sk);
   t1->window = net16(sk->window);
   t1->res1=0;
   t1->res2=0;
   t1->rst = 0;
   t1->urg = 0;
   t1->syn = 0;
   t1->psh = 0;
   t1->fin = 0;
   if (ack == sk->acked_seq)
     {
	sk->ack_backlog = 0;
	sk->bytes_rcv = 0;
	sk->ack_timed = 0;
	if (sk->send_head == NULL &&
	    sk->wfront == NULL)
	  {
	     delete_timer((struct timer *)&sk->time_wait);
	     sk->timeout = 0;
	  }
	
     }
   t1->ack_seq = net32(ack);
   t1->doff = sizeof (*t1)/4;
   tcp_send_check (t1, sk->saddr, daddr, sizeof (*t1), sk);
   sk->prot->queue_xmit(sk, dev, buff, 1);
}

/* this routine builds a generic tcp header. */
static  int
tcp_build_header(struct tcp_header *th, volatile struct sock *sk, int push)
{

 /* want to get rid of this. */
  memcpy (th,(void *) &(sk->dummy_th), sizeof (*th));
  th->seq = net32(sk->send_seq);
  th->psh = (push == 0) ? 1 : 0;
  th->doff = sizeof (*th)/4;
  th->ack = 1;
  th->fin = 0;
  sk->ack_backlog = 0;
  sk->bytes_rcv = 0;
  sk->ack_timed = 0;
  th->ack_seq = net32(sk->acked_seq);
  sk->window = sk->prot->rspace(sk);
  th->window = net16(sk->window);

  return (sizeof (*th));
}

/* This routine copies from a user buffer into a socket, and starts
   the transmit system. */

static int
tcp_write(volatile struct sock *sk, unsigned char *from,
	  int len, int nonblock, unsigned flags)
{
  int copied=0;
  int copy;
  int tmp;
  struct sk_buff *skb;
  unsigned char *buff;
  struct proto *prot;
  struct device *dev=NULL;

  PRINTK ("in TCP_WRITE sk = %X:\n",sk);
  print_sk (sk);

  sk->inuse = 1; /* no one else will use this socket. */
  prot = sk->prot;
  while (len > 0)
    {
       /* first thing we do is make sure that we are established. */	 

      while (sk->state != TCP_ESTABLISHED)
	{
	  if (sk->state != TCP_SYN_SENT &&
	      sk->state != TCP_SYN_RECV)
	   {
	      release_sock (sk);
	      if (sk->keepopen)
		{
		    send_sig (SIGPIPE, current, 0);
		    return (-EINTR);
		}
	      if (copied) return (copied);
	      if (sk->err) return (-sk->err);
	      return (-ENOTCONN);
	    }

	  if (nonblock)
	    {
	      release_sock (sk);
	      return (-EAGAIN);
	    }

	  /* now here is a race condition.
	     release_sock could cause the connection to
	     enter the established mode, if that is the
	     case, then we will block here for ever, because
	     we will have gotten our wakeup call before we
	     go to sleep. */
	  release_sock (sk);
	  cli();
	  if (sk->state != TCP_ESTABLISHED)
	    {
	      interruptible_sleep_on (sk->sleep);
	      if (current->signal & ~current->blocked)
		{
		   sti();
		   if (copied) return (copied);
		   return (-ERESTARTSYS);
		}
	    }
	  sti();
	  sk->inuse = 1;
	}
      /* we also need to worry about the window.  The smallest we
	 will send is about 200 bytes. */


      copy = min (sk->mtu, diff(sk->window_seq, sk->send_seq));

      /* redundent check here. */
      if (copy < 200 || copy > sk->mtu) copy = sk->mtu;
      copy = min (copy, len);

      skb=prot->wmalloc (sk, copy + prot->max_header+sizeof (*skb),0);

      /* if we didn't get any memory, we need to sleep. */
      if (skb == NULL)
	{
	  if (nonblock ||  copied)
	    {
	      break;
	    }
	  /* here is another race condition. */
	  tmp = sk->wmem_alloc;
	  release_sock (sk);
	  /* again we will try to avoid it. */
	  cli ();
	  if (tmp <= sk->wmem_alloc)
	    {
	      interruptible_sleep_on (sk->sleep);
	      if (current->signal & ~current->blocked)
		{
		   sti();
		   if (copied) return (copied);
		   return (-ERESTARTSYS);
		}
	    }
	  sti();
	  sk->inuse = 1;
	  continue;
	}
      skb->mem_addr = skb;
      skb->mem_len = copy+prot->max_header+sizeof (*skb);
      skb->len = 0;
      skb->sk = sk;
      buff =(unsigned char *)( skb+1);
       /* we need to optimize this.  Perhaps some hints here
	  would be good. */

      tmp = prot->build_header (skb, sk->saddr, sk->daddr, &dev,
				IPPROTO_TCP, sk->opt, skb->mem_len);
      if (tmp < 0 )
	{
	  prot->wfree (sk, skb->mem_addr, skb->mem_len);
	  release_sock (sk);
	  return (tmp);
	}
      skb->len += tmp;
      skb->dev = dev;
      buff+=tmp;
      tmp = tcp_build_header((struct tcp_header *)buff, sk, len-copy);
      if (tmp < 0)
	{
	  prot->wfree (sk, skb->mem_addr, skb->mem_len);
	  release_sock (sk);
	  return (tmp);
	}

      if (flags & MSG_OOB)
	{
		((struct tcp_header *)buff)->urg = 1;
		((struct tcp_header *)buff)->urg_ptr = copy;
	}
      skb->len += tmp;
      memcpy_fromfs (buff+tmp, from, copy);

      tcp_send_check ((struct tcp_header *)buff, sk->saddr, sk->daddr,
		       copy +sizeof (struct tcp_header), sk);

      from += copy;
      copied += copy;
      len -= copy;
      skb->len += copy;
      skb->free = 0;
      sk->send_seq += copy;
      skb->h.seq = sk->send_seq;
      if (after (sk->send_seq , sk->window_seq) ||
	  sk->packets_out >= sk->cong_window)
	{
	  PRINTK ("sk->cong_window = %d, sk->packets_out = %d\n",
		  sk->cong_window, sk->packets_out);
	  PRINTK ("sk->send_seq = %d, sk->window_seq = %d\n",
		  sk->send_seq, sk->window_seq);
	  skb->next = NULL;
	  if (sk->wback == NULL)
	    {
	      sk->wfront=skb;
	    }
	  else
	    {
	      sk->wback->next = skb;
	    }
	  sk->wback = skb;
	}
      else
	{
	  prot->queue_xmit (sk, dev, skb,0);
	}
    }
  sk->err = 0;
  release_sock (sk);
  return (copied);
}


static  void
tcp_read_wakeup(volatile struct sock *sk)
{
  int tmp;
  struct device *dev = NULL;
  struct tcp_header *t1;
  struct sk_buff *buff;

  if (!sk->ack_backlog ) return;
  PRINTK ("in tcp read wakeup\n");
  /* we need to put code here to prevent this routine from being called. */
  /* being called once in a while is ok, so only check if this is the
     second time in a row. */

  /* we need to grab some memory, and put together an ack, and then
     put it into the queue to be sent. */

  buff=sk->prot->wmalloc(sk,MAX_ACK_SIZE,1);
  if (buff == NULL) 
    {
       /* try again real soon. */
       sk->timeout = TIME_WRITE;
       sk->time_wait.len = 10;
       reset_timer((struct timer *) &sk->time_wait);
       return;
    }

  buff->mem_addr = buff;
  buff->mem_len = MAX_ACK_SIZE;
  buff->len=sizeof (struct tcp_header);
  buff->sk = sk;

  /* put in the ip_header and routing stuff. */
  tmp = sk->prot->build_header (buff, sk->saddr, sk->daddr, &dev,
				IPPROTO_TCP, sk->opt, MAX_ACK_SIZE);
  if (tmp < 0)
    {
      sk->prot->wfree(sk, buff->mem_addr, buff->mem_len);
      return;
    }

  buff->len += tmp;
  t1 = (struct tcp_header *)((char *)(buff+1) +tmp);

  memcpy (t1,(void *) &sk->dummy_th, sizeof (*t1));
  t1->seq = net32(sk->send_seq);
  t1->ack = 1;
  t1->res1=0;
  t1->res2=0;
  t1->rst = 0;
  t1->urg = 0;
  t1->syn = 0;
  t1->psh = 0;
  sk->ack_backlog = 0;
  sk->bytes_rcv = 0;
  sk->window = sk->prot->rspace(sk);
  t1->window = net16(sk->window);
  t1->ack_seq = net32(sk->acked_seq);
  t1->doff = sizeof (*t1)/4;
  tcp_send_check (t1, sk->saddr, sk->daddr, sizeof (*t1), sk);
  sk->prot->queue_xmit(sk, dev, buff, 1);
}


/* This routine frees used buffers. */
/* It should consider sending an ack to let the
   other end know we now have a bigger window. */

static  void
cleanup_rbuf (volatile struct sock *sk)
{
   PRINTK ("cleaning rbuf for sk=%X\n",sk);
  /* we have to loop through all the buffer headers, and 
     try to free up all the space we can. */
  while (sk->rqueue != NULL )
    {
      struct sk_buff *skb;
      skb=sk->rqueue->next;
      if (!skb->used) break;
      if (sk->rqueue == skb)
	{
	  sk->rqueue = NULL;
	}
      else
	{
	  skb->next->prev = skb->prev;
	  skb->prev->next = skb->next;
	}
      skb->sk = sk;
      free_skb (skb, FREE_READ);
    }
   /* at this point we should send an ack if the difference in
      the window, and the amount of space is bigger than
      TCP_WINDOW_DIFF */
   PRINTK ("sk->window left = %d, sk->prot->rspace(sk)=%d\n",
	   sk->window - sk->bytes_rcv, sk->prot->rspace(sk));

   if ((sk->prot->rspace(sk) >
	(sk->window - sk->bytes_rcv + TCP_WINDOW_DIFF)) ||
       (sk->window - sk->bytes_rcv < 2*sk->mtu)) 
     {
	/* force it to send an ack. */
	sk->ack_backlog++;
	if (sk->timeout != TIME_WRITE && sk->state == TCP_ESTABLISHED)
	  {
	     sk->time_wait.len = TCP_ACK_TIME;
	     sk->timeout=TIME_WRITE;
	     reset_timer ((struct timer *)&sk->time_wait);
	  }
     }

}

/* handle reading urgent data. */
static  int
tcp_read_urg(volatile struct sock * sk,
	     unsigned char *to, int len, unsigned flags)
{
    int copied = 0;
    struct sk_buff *skb;
    PRINTK ("tcp_read_urg(sk=%X, to=%X, len=%d, flags=%X)\n",
	    sk, to, len, flags);
    print_sk(sk);
    while (len > 0)
      {
	 sk->inuse = 1;
	  while (sk->urg==0 || sk->rqueue == NULL)
	    {
	       /* now at this point, we may have gotten some data. */
	       release_sock (sk);
		if (sk->state > TCP_CLOSING)
		  {
		      if (copied) return (copied);
		      return (-ENOTCONN);
		  }
	       cli();
	       if (sk->urg == 0 || sk->rqueue == NULL)
		 {
		    interruptible_sleep_on (sk->sleep);
		    if (current->signal & ~current->blocked)
		      {
			 sti();
			 if (copied) return (copied);
			 return (-ERESTARTSYS);
		      }
		 }
	       sti();
	       sk->inuse = 1;
	    }
	  /* now we have some urgent data, we must find it.*/
	  for (skb = sk->rqueue->next; skb->next != sk->rqueue;
	       skb = skb->next)
	    {
		int offset;
		int amt;
		if (!skb->h.th->urg) continue;
		offset = 0;
		amt = min(skb->h.th->urg_ptr,len);
		verify_area (to, amt);
		memcpy_tofs (to, (unsigned char *)(skb->h.th) +
			     skb->h.th->doff*4
			     + offset, amt);

		if (!(flags & MSG_PEEK))
		  {
		     skb->urg_used = 1;
		     sk->urg --;
		  }
		release_sock (sk);
		copied += amt;
		return (copied);
	    }
      }
    return (0);
}

/* This routine copies from a sock struct into the user buffer. */
static  int
tcp_read(volatile struct sock *sk, unsigned char *to,
	 int len, int nonblock, unsigned flags)
{
    int copied=0; /* will be used to say how much has been copied. */
    struct sk_buff *skb;
    unsigned long offset;
    unsigned long used;

    if (len == 0) return (0);
    if (len < 0) 
      {
	 return (-EINVAL);
      }
    
    /* this error should be checked. */
    if (sk->state == TCP_LISTEN) return (-ENOTCONN);

    /* will catch some errors. */
    if (sk->err)
      {
	int err;
	err = -sk->err;
	sk->err = 0;
	return (err);
      }

    /* urgent data needs to be handled specially. */
    if ((flags & MSG_OOB))
      return (tcp_read_urg (sk, to, len, flags));

    /* so no-one else will use this socket. */
    sk->inuse = 1;
    if (sk->rqueue != NULL)
      skb=sk->rqueue->next;
    else
      skb = NULL;

    while ( len > 0)
      {
	 PRINTK("tcp_read (sk=%X, to=%X, len=%d, nonblock=%d, flags=%X)\n",
		sk, to, len, nonblock, flags);
	  while ( skb == NULL || before (sk->copied_seq+1, skb->h.th->seq) ||
		 skb->used) /* skb->used just checks to see if we've
			       gone all the way around. */
	    {

	       PRINTK("skb = %X:\n",skb);
	       print_skb(skb);
	       print_sk (sk);

	       cleanup_rbuf(sk);

		release_sock (sk); /* now we may have some data waiting. */


	       PRINTK ("tcp_read about to sleep. state = %d\n",sk->state);
	       cli();

	       if (sk->state == TCP_CLOSE || sk->state == TCP_TIME_WAIT)
		   {
		     sti();
		     if (copied) return (copied);
		     if (sk->err) return (-sk->err);
		     if (!sk->done)
		       {
			  sk->done = 1;
			  return (0);
		       }
		     return (-ENOTCONN);
		  }
			

		if (nonblock || ((flags & MSG_PEEK) && copied))
		  {
		    sti();
		    release_sock (sk);
		    if (copied) return (copied);
		    return (-EAGAIN);
		  }

		if ( sk->rqueue == NULL ||
		    before (sk->copied_seq+1, sk->rqueue->next->h.th->seq))
		  {
		     interruptible_sleep_on (sk->sleep);
		     if (current->signal & ~current->blocked)
		       {
			  sti ();
			  if (copied) return (copied);
			  return (-ERESTARTSYS);
		       }
		  }
		sti();
		PRINTK ("tcp_read woke up. \n");

		sk->inuse = 1;

		if (sk->rqueue != NULL)
		  skb=sk->rqueue->next;
		else
		  skb = NULL;

	    }

	  /* Copy anything from the current block that needs to go
	     into the user buffer. */

	 offset = sk->copied_seq+1 - skb->h.th->seq;

	 if (skb->h.th->syn) offset --;
	 if (offset < skb->len )
	   {
	      /* if there is urgent data we must either return or skip
		 over it. */
	      if (skb->h.th->urg)
		{
		   if (skb->urg_used)
		     {
			if (flags & MSG_PEEK) break;
			sk->copied_seq += skb->h.th->urg_ptr;
			offset += skb->h.th->urg_ptr;
			if (offset > skb->len)
			  {
			     skb->used = 1;
			     skb=skb->next;
			     continue;
			  }
		     }
		   else
		     {
			break;
		     }
		}
	      used = min(skb->len - offset, len);

	      verify_area (to, used);
	      memcpy_tofs(to, ((unsigned char *)skb->h.th) +
			  skb->h.th->doff*4 +
			  offset,
			  used);
	      copied += used;
	      len -= used;
	      to += used;
	      if (!(flags & MSG_PEEK))
		sk->copied_seq += used;
	      
	      /* mark this data used if we are really reading it, and if
		 it doesn't contain any urgent data. And we have used all
		 the data. */
	      if (!(flags & MSG_PEEK) &&
		  (!skb->h.th->urg || skb->urg_used) &&
		  (used + offset >= skb->len) )
		skb->used = 1;
	      
	      /* see if this is the end of a message or if the remaining data
		 is urgent. */
	      if ( skb->h.th->psh || skb->h.th->urg)
		{
		   break;
		}
	   }
	 else /* already used this data, must be a retransmit. */
	   {
	      skb->used = 1;
	   }
	 skb=skb->next;
      }
    cleanup_rbuf (sk);
    release_sock (sk);
    if (copied == 0 && nonblock) return (-EAGAIN);
    return (copied);
}

/* this routine will send a reset to the other tcp. */
static  void
tcp_reset(unsigned long saddr, unsigned long daddr, struct tcp_header *th,
	   struct proto *prot, struct options *opt, struct device *dev)
{
  /* we need to grab some memory, and put together a reset, and then
     put it into the queue to be sent. */
  struct sk_buff *buff;
  struct tcp_header *t1;
  int tmp;
  buff=prot->wmalloc(NULL, MAX_RESET_SIZE,1);
  if (buff == NULL) return;

  PRINTK("tcp_reset buff = %X\n", buff);
  buff->mem_addr = buff;
  buff->mem_len = MAX_RESET_SIZE;
  buff->len = sizeof (*t1);
  buff->sk = NULL;
  buff->dev = dev;

  t1=(struct tcp_header *)(buff + 1);
  /* put in the ip_header and routing stuff. */
  tmp = prot->build_header (buff, saddr, daddr, &dev, IPPROTO_TCP, opt,
			    sizeof(struct tcp_header));
  if (tmp < 0)
    {
      prot->wfree (NULL,buff->mem_addr, buff->mem_len);
      return;
    }
  t1 = (struct tcp_header *)((char *)t1 +tmp);
  buff->len += tmp;
  memcpy (t1, th, sizeof (*t1));
  /* swap the send and the receive. */
  t1->dest = th->source;
  t1->source = th->dest;
  t1->seq = th->ack_seq; /* add one so it will be in
			    the right range.*/
  t1->rst = 1;
  t1->ack = 0;
  t1->syn = 0;
  t1->urg = 0;
  t1->fin = 0;
  t1->psh = 0;
  t1->doff = sizeof (*t1)/4;
  tcp_send_check (t1, saddr, daddr, sizeof (*t1), NULL);
  prot->queue_xmit(NULL, dev, buff, 1);
  
}


/* This routine handles a connection request.  This should make sure
   we haven't already responded. */
/* Because of the way BSD works, we have to send a syn/ack now. This also
 means it will be harder to close a socket which is listening. */

static  void
tcp_conn_request(volatile struct sock *sk, struct sk_buff *skb,
		 unsigned long daddr,
		 unsigned long saddr, struct options *opt, struct device *dev)
{
  struct sk_buff *buff;
  struct tcp_header *t1;
  unsigned char *ptr;
  volatile struct sock *newsk;
  struct tcp_header *th;
  int tmp;
  th = skb->h.th;

  PRINTK ("tcp_conn_request (sk = %X, skb = %X, daddr = %X, sadd4= %X, \n"
	  "                  opt = %X, dev = %X)\n",
	  sk, skb, daddr, saddr, opt, dev);
  
  /* if the socket is dead, don't accept the connection. */
  if (!sk->dead)
    {
       wake_up(sk->sleep);
    }
  else
    {
       PRINTK ("tcp_conn_request on dead socket\n");
       tcp_reset (daddr, saddr, th, sk->prot, opt, dev);
       free_skb (skb, FREE_READ);
       return;
    }


  /* we need to build a new sock struct. */
  /* It is sort of bad to have a socket without an inode attached to
     it, but the wake_up's will just wake up the listening socket,
     and if the listening socket is destroyed before this is taken
     off of the queue, this will take care of it. */

  newsk = malloc(sizeof (struct sock));
  if (newsk == NULL) 
    {
       /* just ignore the syn.  It will get retransmitted. */
       free_skb (skb, FREE_READ);
       return;
    }


  PRINTK ("newsk = %X\n", newsk);
  memcpy ((void *)newsk, (void *)sk, sizeof (*newsk));
  newsk->wback = NULL;
  newsk->wfront = NULL;
  newsk->rqueue = NULL;
  newsk->send_head = NULL;
  newsk->send_tail = NULL;
  newsk->back_log = NULL;
  newsk->blog = 0;
  newsk->intr = 0;
  newsk->proc = 0;
  newsk->done = 0;

  newsk->pair = NULL;
  newsk->wmem_alloc = 0;
  newsk->rmem_alloc = 0;

  newsk->max_unacked = MAX_WINDOW - TCP_WINDOW_DIFF;

  newsk->err = 0;
  newsk->shutdown = 0;
  newsk->ack_backlog = 0;
  newsk->acked_seq = skb->h.th->seq+1;
  newsk->fin_seq = skb->h.th->seq;
  newsk->copied_seq = skb->h.th->seq;
  newsk->state = TCP_SYN_RECV;
  newsk->timeout = 0;
  newsk->send_seq = timer_seq*SEQ_TICK-seq_offset;
  newsk->rcv_ack_seq = newsk->send_seq;
  newsk->urg =0;
  newsk->retransmits = 0;
  newsk->destroy = 0;
  newsk->time_wait.sk = newsk;
  newsk->time_wait.next = NULL;
  newsk->dummy_th.source = skb->h.th->dest;
  newsk->dummy_th.dest = skb->h.th->source;
  /* swap these two, they are from our point of view. */
  newsk->daddr=saddr;
  newsk->saddr=daddr;

  put_sock (newsk->num,newsk);
  newsk->dummy_th.res1=0;
  newsk->dummy_th.doff=6;
  newsk->dummy_th.fin=0;
  newsk->dummy_th.syn=0;
  newsk->dummy_th.rst=0;
  newsk->dummy_th.psh=0;
  newsk->dummy_th.ack=0;
  newsk->dummy_th.urg=0;
  newsk->dummy_th.res2=0;
  newsk->acked_seq = skb->h.th->seq+1;
  newsk->copied_seq = skb->h.th->seq;

  if (skb->h.th->doff == 5)
    {
      newsk->mtu=576-HEADER_SIZE;
    }
  else
    {
      ptr = (unsigned char *)(skb+1);
      if (ptr[0] != 2 || ptr[1] != 4)
	{
	   newsk->mtu=576-HEADER_SIZE;
	}
      else
	{
	  newsk->mtu = min (ptr[2]*256+ptr[3]-HEADER_SIZE,
			    dev->mtu-HEADER_SIZE);
	}
    }

  print_sk (newsk);
  buff=newsk->prot->wmalloc(newsk,MAX_SYN_SIZE,1);
  if (buff == NULL)
    {
       sk->err = -ENOMEM;
       newsk->dead = 1;
       release_sock (newsk);
       free_skb (skb, FREE_READ);
       return;
    }
  
  buff->mem_addr = buff;
  buff->mem_len = MAX_SYN_SIZE;
  buff->len=sizeof (struct tcp_header)+4;
  buff->sk = newsk;
  
  t1=(struct tcp_header *)(buff + 1);
  /* put in the ip_header and routing stuff. */

  tmp = sk->prot->build_header (buff, newsk->saddr, newsk->daddr, &dev,
				IPPROTO_TCP, NULL, MAX_SYN_SIZE);

  /* something went wrong. */
  if (tmp < 0)
    {
       sk->err = tmp;
       sk->prot->wfree(newsk, buff->mem_addr, buff->mem_len);
       newsk->dead = 1;
       release_sock (newsk);
       skb->sk = sk;
       free_skb (skb, FREE_READ);
       return;
    }

  buff->len += tmp;
  t1 = (struct tcp_header *)((char *)t1 +tmp);
  
  memcpy (t1, skb->h.th, sizeof (*t1));
  buff->h.seq = newsk->send_seq;
  /* swap the send and the receive. */
  t1->dest = skb->h.th->source;
  t1->source = newsk->dummy_th.source;
  t1->seq = net32(newsk->send_seq++);
  t1->ack = 1;
  newsk->window = sk->prot->rspace(newsk);
  t1->window = net16(newsk->window);
  t1->res1=0;
  t1->res2=0;
  t1->rst = 0;
  t1->urg = 0;
  t1->psh = 0;
  t1->syn = 1;
  t1->ack_seq = net32(skb->h.th->seq+1);
  t1->doff = sizeof (*t1)/4+1;

  ptr = (unsigned char *)(t1+1);
  ptr[0]=2;
  ptr[1]=4;
  ptr[2]=((dev->mtu - HEADER_SIZE) >> 8) & 0xff;
  ptr[3]=(dev->mtu - HEADER_SIZE) & 0xff;

  tcp_send_check (t1, daddr, saddr, sizeof (*t1)+4, newsk);
  newsk->prot->queue_xmit(newsk, dev, buff, 0);

  newsk->time_wait.len = TCP_CONNECT_TIME;
  PRINTK ("newsk->time_wait.sk = %X\n", newsk->time_wait.sk);
  reset_timer ((struct timer *)&newsk->time_wait);
  skb->sk = newsk;
  /* charge the sock_buff to newsk. */
  sk->rmem_alloc -= skb->mem_len;
  newsk->rmem_alloc += skb->mem_len;

  if (sk->rqueue == NULL)
    {
      skb->next = skb;
      skb->prev = skb;
      sk->rqueue = skb;
    }
  else
    {
      skb->next = sk->rqueue;
      skb->prev = sk->rqueue->prev;
      sk->rqueue->prev = skb;
      skb->prev->next = skb;
    }
  release_sock (newsk);
}

static  void
tcp_close (volatile struct sock *sk, int timeout)
{
  /* we need to grab some memory, and put together a fin, and then
     put it into the queue to be sent. */
  struct sk_buff *buff;
  int need_reset = 0;
  struct tcp_header *t1,*th;
  struct proto *prot;
  struct device *dev=NULL;
  int tmp;
  PRINTK ("tcp_close ((struct sock *)%X, %d)\n",sk, timeout);
  sk->inuse = 1;
  sk->keepopen = 0;
  sk->shutdown = SHUTDOWN_MASK;

  if (!sk->dead)
    wake_up (sk->sleep);


  /* we need to flush the recv. buffs. */
  
  if (sk->rqueue != NULL)
    {
       struct sk_buff *skb;
       struct sk_buff *skb2;
       skb = sk->rqueue;
       do {
	  skb2=skb->next;
	  free_skb (skb, FREE_READ);
	  skb=skb2;
       } while (skb != sk->rqueue);
       need_reset = 1;
    }
  sk->rqueue = NULL;


  switch (sk->state)
    {
      case TCP_FIN_WAIT1:
      case TCP_FIN_WAIT2:
      case TCP_LAST_ACK:
       if (timeout)
	 tcp_time_wait(sk);
       release_sock (sk);
       if (!need_reset)
	 return;
       break;

      case TCP_TIME_WAIT:
       if (timeout)
	 sk->state = TCP_CLOSE;
       release_sock (sk);
       return;

      case TCP_LISTEN:
       sk->state = TCP_CLOSE;
       release_sock(sk);
       return;

      case TCP_CLOSE:

       release_sock(sk);
       return;
       

    case TCP_ESTABLISHED:
    case TCP_SYN_SENT:
    case TCP_SYN_RECV:

      prot = (struct proto *)sk->prot;
      th=(struct tcp_header *)&sk->dummy_th;

       buff=prot->wmalloc(sk, MAX_FIN_SIZE,1);
       if (buff == NULL)
	 {
	    /* this will force it to try again later. */
	    sk->state = TCP_ESTABLISHED;
	    sk->timeout = TIME_CLOSE;
	    sk->time_wait.len = 100; /* wait a second. */
	    reset_timer ((struct timer *)&sk->time_wait);
	    return;
	 }

      buff->mem_addr = buff;
      buff->mem_len = MAX_FIN_SIZE;
      buff->sk = sk;
      buff->len = sizeof (*t1);
      t1=(struct tcp_header *)(buff + 1);
      /* put in the ip_header and routing stuff. */
      tmp = prot->build_header (buff,sk->saddr, sk->daddr, &dev,
				IPPROTO_TCP, sk->opt,
				sizeof(struct tcp_header));
      if (tmp < 0)
	{
	  prot->wfree (sk,buff->mem_addr, buff->mem_len);
	  PRINTK ("Unable to build header for fin.\n");
	  release_sock(sk);
	  return;
	}
      t1 = (struct tcp_header *)((char *)t1 +tmp);
      buff ->len += tmp;
      buff->dev = dev;
      memcpy (t1, th, sizeof (*t1));
      t1->seq = net32(sk->send_seq);
      sk->send_seq++;
      buff->h.seq = sk->send_seq;
      t1->ack = 1;
       /* ack everything immediately from now on. */
      sk->delay_acks = 0;
      t1->ack_seq = net32(sk->acked_seq);
      t1->window = net16(sk->prot->rspace(sk));
      t1->fin = 1;
      t1->rst = need_reset;
      t1->doff = sizeof (*t1)/4;
      tcp_send_check (t1, sk->saddr, sk->daddr, sizeof (*t1), sk);

      if (sk->wfront == NULL)
	{
	  prot->queue_xmit(sk, dev, buff, 0);
	}
      else
	{
	   sk->time_wait.len = sk->rtt;
	   sk->timeout = TIME_WRITE;
	   reset_timer ((struct timer *)&sk->time_wait);
	   buff->next = NULL;
	   if (sk->wback == NULL)
	     {
		sk->wfront=buff;
	     }
	   else
	     {
		sk->wback->next = buff;
	     }
	   sk->wback = buff;
	   
	}
      sk->state = TCP_FIN_WAIT1;
    }
  release_sock (sk);
}


/* This routine takes stuff off of the write queue, and puts it in the
   xmit queue. */
static  void
tcp_write_xmit (volatile struct sock *sk)
{
  struct sk_buff *skb;
  while (sk->wfront != NULL && before (sk->wfront->h.seq, sk->window_seq) &&
	 sk->packets_out < sk->cong_window)
    {
      skb = sk->wfront;
      sk->wfront = skb->next;
      if (sk->wfront == NULL)
	sk->wback = NULL;
      sk->prot->queue_xmit (sk, skb->dev, skb, skb->free);
    }
}



/* This routine deals with incoming acks, but not outgoing ones. */

static  int
tcp_ack (volatile struct sock *sk, struct tcp_header *th, unsigned long saddr)
{
  unsigned long ack;
  ack = net32(th->ack_seq);

  if (!between (ack , sk->rcv_ack_seq, sk->send_seq)) 
    {
      if (after (ack, sk->send_seq) || sk->state != TCP_ESTABLISHED) 
	{
	  return (0);
	}
      if (sk->keepopen)
	reset_timer ((struct timer *)&sk->time_wait);
      sk->retransmits = 0;
      return (1);
    }

  sk->window_seq = ack + net16(th->window);


  /* we don't want too many packets out there. */
  if (sk->cong_window < 2048 && ack != sk->rcv_ack_seq)
    {
       if (sk->exp_growth)
	 sk->cong_window *= 2;
       else
	 sk->cong_window++;
    }

  sk->rcv_ack_seq = ack;

  /* see if we can take anything off of the retransmit queue. */
  while (sk->send_head != NULL)
    {
      if (before (sk->send_head->h.seq, ack+1))
	{
	  struct sk_buff *oskb;
	  /* we have one less packet out there. */
	  sk->packets_out --;
	  cli();
	  oskb = sk->send_head;
	  /* estimate the rtt. */
	  sk->rtt += ((jiffies - oskb->when) - sk->rtt)/2;
	  if (sk->rtt < 30) sk->rtt = 30;
	  sk->send_head = oskb->link3;
	  if (sk->send_head == NULL) 
	    {
	      sk->send_tail = NULL;
	    }
	  /* we may need to remove this from the dev send list. */
	  if (oskb->next != NULL)
	    {
	       if (oskb->next != oskb)
		 {
		    oskb->next->prev = oskb->prev;
		    oskb->prev->next = oskb->next;
		 }
	       else
		 {
		    int i;
		    for (i = 0; i < DEV_NUMBUFFS; i++)
		      {
			 if (oskb->dev->buffs[i] = oskb)
			   {
			      oskb->dev->buffs[i]= NULL;
			      break;
			   }
		      }
		 }
	    }
	  free_skb  (oskb, FREE_WRITE); /* write. */
	  sti();
	  if (!sk->dead)
	    wake_up(sk->sleep);
	}
      else
	{
	  break;
	}

    }


  /* at this point we need to check to see if we have anything
     which needs to be retransmiteed.  If we have failed to get
     some acks i.e. had to retransmit something, and we succeded, we
     should then attempt to retransmit everything right now. */

  if (sk->retransmits && sk->send_head != NULL)
    {
      sk->prot->retransmit (sk,1);
    }
  sk->retransmits = 0;

  /* maybe we can take some stuff off of the write queue, and put it onto
     the xmit queue. */
  if (sk->wfront != NULL && sk->packets_out < sk->cong_window)
    {
      if (after (sk->window_seq, sk->wfront->h.seq))
	{
	  tcp_write_xmit (sk);
	}
    }
  else
    {
       if (sk->send_head == NULL && sk->ack_backlog == 0 &&
	   sk->state != TCP_TIME_WAIT)
	 {
	    delete_timer((struct timer *)&sk->time_wait);
	    sk->timeout = 0;
	 }
       else
	 {
	    if (sk->state == TCP_TIME_WAIT)
	      {
		 sk->time_wait.len = TCP_TIMEWAIT_LEN;
		 sk->timeout = TIME_CLOSE;
	      }
	    reset_timer ((struct timer *)&sk->time_wait);
	 }
    }

  /* see if we are done. */
  if ( sk->state == TCP_TIME_WAIT)
    {
       if (sk->rcv_ack_seq == sk->send_seq && 	 
	   sk->acked_seq == sk->fin_seq);
       if (!sk->dead) wake_up (sk->sleep);
       sk->state = TCP_CLOSE;
    }

  if (sk->state == TCP_FIN_WAIT1)
    {
      if (sk->rcv_ack_seq == sk->send_seq)
	sk->state = TCP_FIN_WAIT2;
    }
  
  if (sk->state == TCP_LAST_ACK)
    {
      if (sk->rcv_ack_seq == sk->send_seq)
	{
	   if (sk->acked_seq != sk->fin_seq)
	     {
		tcp_time_wait(sk);
	     }
	   else
	     {
		sk->state = TCP_CLOSE;
	     }
	}
      if (!sk->dead) wake_up (sk->sleep);
    }

  return (1);
}

/* This routine handles the data.  If there is room in the buffer, it
   will be have already been moved into it.  If there is no room,
   then we will just have to discard the packet. */

static  int
tcp_data (struct sk_buff *skb, volatile struct sock *sk, 
	  unsigned long saddr, unsigned short len)
{
  struct sk_buff *skb1, *skb2;
  struct tcp_header *th;

  th = skb->h.th;
  print_th (th);
  skb->len = len - (th->doff*4);

  PRINTK("tcp_data len = %d sk = %X:\n",skb->len, sk);
  print_sk(sk);

  sk->bytes_rcv += skb->len;

  if (skb->len == 0 && !th->fin && !th->urg && !th->psh)
    {
      /* don't want to keep passing ack's back and fourth. */
      if (!th->ack)
	tcp_send_ack (sk->send_seq, sk->acked_seq,sk, th, saddr);
      free_skb(skb, FREE_READ);
      return (0);
    }

  if (sk->shutdown & RCV_SHUTDOWN)
    {
       /* just ack everything. */
       sk->acked_seq = th->seq + skb->len + th->syn + th->fin;
       tcp_send_ack (sk->send_seq, sk->acked_seq, sk, skb->h.th, saddr);
       free_skb (skb, FREE_READ);
       if (sk->state == TCP_TIME_WAIT && sk->acked_seq == sk->fin_seq)
	 {
	    if (!sk->dead) wake_up (sk->sleep);
	    sk->state = TCP_CLOSE;
	 }
       return (0);
    }

  /* now we have to walk the chain, and figure out where this one
     goes into it.  This is set up so that the last packet we received
     will be the first one we look at, that way if everything comes
     in order, there will be no performance loss, and if they come
     out of order we will be able to fit things in nicely. */
  
  if (sk->rqueue == NULL)
    {
       PRINTK ("tcp_data: skb = %X:\n",skb);
       print_skb (skb);

       sk->rqueue = skb;
       skb->next = skb;
       skb->prev = skb;
       skb1= NULL;
    }
  else
    {
      PRINTK ("tcp_data adding to chain sk = %X:\n",sk);
      print_sk (sk);

      for (skb1=sk->rqueue; ; skb1=skb1->prev)
	{
	  PRINTK ("skb1=%X\n",skb1);
	  print_skb(skb1);
	  PRINTK ("skb1->h.th->seq = %d\n", skb1->h.th->seq);
	  if (after ( th->seq+1, skb1->h.th->seq))
	    {
	      skb->prev = skb1;
	      skb->next = skb1->next;
	      skb->next->prev = skb;
	      skb1->next = skb;
	      if (skb1 == sk->rqueue)
		sk->rqueue = skb;
	      break;
	    }
	  if  ( skb1->prev == sk->rqueue)
	    {
	       skb->next= skb1;
	       skb->prev = skb1->prev;
	       skb->prev->next = skb;
	       skb1->prev = skb;
	       skb1 = NULL; /* so we know we might be able to ack stuff. */
	       break;
	    }
	}

      PRINTK ("skb = %X:\n",skb);
      print_skb (skb);
      PRINTK ("sk now equals:\n");
      print_sk (sk);

    }

  th->ack_seq = th->seq + skb->len;
  if (th->syn) th->ack_seq ++;
  if (th->fin) th->ack_seq ++;

  if (before (sk->acked_seq, sk->copied_seq))
    {
       printk ("*** tcp.c:tcp_data bug acked < copied\n");
       sk->acked_seq = sk->copied_seq;
    }

  /* now figure out if we can ack anything. */
  if (skb1 == NULL || skb1->acked || before (th->seq, sk->acked_seq+1))
    {
      if (before (th->seq, sk->acked_seq+1))
	{
	  sk->acked_seq = th->ack_seq;
	  skb->acked = 1;
	  
	  for (skb2=skb->next; skb2 != sk->rqueue->next; skb2=skb2->next)
	    {
	       if (before(skb2->h.th->seq, sk->acked_seq+1))
		 {
		    sk->acked_seq = skb2->h.th->ack_seq;
		    skb2->acked = 1;
		    /* force an immediate ack. */
		    sk->ack_backlog = sk->max_ack_backlog;
		 }
	       else
		 break;
	    }

	  /* this also takes care of updating the window. */
	  /* this if statement needs to be simplified. */

	  if (!sk->delay_acks || 
	      sk->ack_backlog >= sk->max_ack_backlog || 
	      sk->window < 2*sk->mtu + sk->bytes_rcv ||
	      sk->bytes_rcv > sk->max_unacked || 
	      th->fin)
	    {
		tcp_send_ack (sk->send_seq, sk->acked_seq,sk,th, saddr);
	    }
	  else
	    {
	       sk->ack_backlog++;
	       sk->time_wait.len = TCP_ACK_TIME;
	       sk->timeout = TIME_WRITE;
	       reset_timer ((struct timer *)&sk->time_wait);
	    }
       }
   }
  else
    {
       /* we missed a packet.  Send an ack to try to resync things. */
       tcp_send_ack (sk->send_seq, sk->acked_seq, sk, th, saddr);
    }

  /* now tell the user we may have some data. */
  if (!sk->dead)
    {
       wake_up (sk->sleep);
    }
  else
    {
       PRINTK ("data received on dead socket. \n");
    }

  if (sk->state > TCP_CLOSING && sk->acked_seq == sk->fin_seq)
    {
       sk->state = TCP_CLOSE;
    }

  return (0);
}

static  int
tcp_urg (volatile struct sock *sk, struct tcp_header *th, unsigned long saddr)
{
    extern int kill_pg (int pg, int sig, int priv);
    extern int kill_proc (int pid, int sig, int priv);
    
    if (!sk->dead)
      wake_up(sk->sleep);
    
    if (sk->urginline)
      {
	  th->urg = 0;
	  th->psh = 1;
	  return (0);
      }

    sk->urg++;

    if (!sk->urg)
      {
	  /* so if we get more urgent data, we don't 
	     signal the user again. */
	  if (sk->proc == 0) return (0);
	  if (sk->proc > 0)
	    {
		kill_proc (sk->proc, SIGURG, 1);
	    }
	  else 
	    {
		kill_pg (-sk->proc, SIGURG, 1);
	    }
      }
    return (0);
}

/* this deals with incoming fins. */
static  int
tcp_fin (volatile struct sock *sk, struct tcp_header *th, 
	 unsigned long saddr, struct device *dev)
{
  struct sk_buff *buff;
  struct tcp_header *t1;
  int tmp;
  PRINTK ("tcp_fin (sk=%X, th=%X, saddr=%X, dev=%X)\n",
	  sk, th, saddr, dev);
  
  if (!sk->dead)
    {
      wake_up (sk->sleep);
    }

  /* after sending the fin, we aren't allowed to write anymore. */
  sk->shutdown |= SEND_SHUTDOWN;

  sk->err = 0;
  switch (sk->state)
    {
    case TCP_SYN_RECV:
    case TCP_SYN_SENT:
    case TCP_ESTABLISHED:
      sk->state = TCP_LAST_ACK;
      break;

     default:
    case TCP_FIN_WAIT1:
    case TCP_TIME_WAIT:
      sk->state = TCP_LAST_ACK;
      /* start the timers. */
      sk->time_wait.len = TCP_TIMEWAIT_LEN;
      sk->timeout = TIME_CLOSE;
      reset_timer ((struct timer *)&sk->time_wait);
      return (0);

    case TCP_FIN_WAIT2:
      sk->state = TCP_CLOSE;
      return (0);
    }

  /* send an ack and our own fin. */
  buff=sk->prot->wmalloc(sk,MAX_ACK_SIZE,1);
  if (buff == NULL)
    {
       /* we will ignore the fin.  That way it will be sent again. */
       return (1);
    }

  buff->mem_addr = buff;
  buff->mem_len = MAX_ACK_SIZE;
  buff->len=sizeof (struct tcp_header);
  buff->sk = sk;

  t1 = (struct tcp_header *)(buff + 1);
  /* put in the ip_header and routing stuff. */
  tmp = sk->prot->build_header (buff, sk->saddr, sk->daddr, &dev,
				IPPROTO_TCP,  sk->opt, MAX_ACK_SIZE);
  if (tmp < 0)
    {
      sk->prot->wfree(sk, buff->mem_addr, buff->mem_len);
      return (0);
    }

  buff->len += tmp;
  t1 = (struct tcp_header *)((char *)t1 +tmp);
  
  memcpy (t1, th, sizeof (*t1));

  /* swap the send and the receive. */
  t1->dest = th->source;
  t1->source = th->dest;


  t1->seq = net32(sk->send_seq++);

  /* contains the one that needs to be acked. */
  sk->fin_seq = th->seq+1;

  buff->h.seq = sk->send_seq;
  t1->window = net16(sk->prot->rspace(sk));

  t1->res1=0;
  t1->res2=0;
  t1->rst = 0;
  t1->urg = 0;
  t1->syn = 0;
  t1->psh = 0;
  t1->ack = 1; 
  t1->fin = 1;
  t1->ack_seq = net32(sk->acked_seq);

  t1->doff = sizeof (*t1)/4;
  tcp_send_check (t1, sk->saddr, sk->daddr, sizeof (*t1), sk);

  /* can't just queue this up.  It should go at the end of
     the write queue. */
  if (sk->wback != NULL)
    {
      buff->next = NULL;
      sk->wback->next = buff;
      sk->wback = buff;
    }
  else
    {
      sk->prot->queue_xmit (sk, dev, buff,0);
    }

  return (0);
}


/* this will accept the next outstanding connection. */

static volatile struct sock *
tcp_accept (volatile struct sock *sk, int flags)
{
  volatile struct sock *newsk;
  struct sk_buff *skb;
  
  PRINTK ("tcp_accept(sk=%X, flags=%X)\n", sk, flags);
  print_sk(sk);
  /* we need to make sure that this socket is listening, and that
     it has something pending. */
  
  if (sk->state != TCP_LISTEN)
    {
      sk->err = EINVAL;
      return (NULL); 
    }
  /* avoid the race. */

  sk->inuse = 1;
  cli();
  while ( (skb = get_firstr(sk)) == NULL )
    {
      if (flags & O_NONBLOCK)
	{
	  sti();
	  release_sock (sk);
	  sk->err = EAGAIN;
	  return (NULL);
	}

      release_sock (sk);
      interruptible_sleep_on (sk->sleep);
      if (current->signal & ~current->blocked)
	{
	   sti();
	   sk->err = ERESTARTSYS;
	   return (NULL);
	}

      sk->inuse = 1;
    }
  sti();

  /* now all we need to do is return skb->sk. */
  newsk = skb->sk;
  free_skb (skb, FREE_READ);
  release_sock (sk);
  return (newsk);
}



/* this will initiate an outgoing connection. */
static int
tcp_connect (volatile struct sock *sk, struct sockaddr_in *usin, int addr_len)
{
  struct sk_buff *buff;
  struct sockaddr_in sin;
  struct device *dev=NULL;
  unsigned char *ptr;
  int tmp;
  struct tcp_header *t1;
  if (sk->state != TCP_CLOSE) return (-EISCONN);
  if (addr_len < 8) return (-EINVAL);

  verify_area (usin, addr_len);
  memcpy_fromfs (&sin,usin, min(sizeof (sin), addr_len));

  if (sin.sin_family && sin.sin_family != AF_INET) return (-EAFNOSUPPORT);

  sk->daddr = sin.sin_addr.s_addr;
  sk->send_seq = timer_seq*SEQ_TICK-seq_offset;
  sk->rcv_ack_seq = sk->send_seq -1;
  sk->err = 0;
  sk->dummy_th.dest = sin.sin_port;

  buff=sk->prot->wmalloc(sk,MAX_SYN_SIZE,0);
  if (buff == NULL) 
    {
      return (-ENOMEM);
    }
  sk->inuse = 1;
  buff->mem_addr = buff;
  buff->mem_len = MAX_SYN_SIZE;
  buff->len=24;
  buff->sk = sk;
  t1=(struct tcp_header *)(buff + 1);
  /* put in the ip_header and routing stuff. */
  /* We need to build the routing stuff fromt the things saved
     in skb. */
  tmp = sk->prot->build_header (buff, sk->saddr, sk->daddr, &dev,
				IPPROTO_TCP, NULL, MAX_SYN_SIZE);
  if (tmp < 0)
    {
      sk->prot->wfree(sk, buff->mem_addr, buff->mem_len);
      release_sock (sk);
      return (-ENETUNREACH);
    }
  buff->len += tmp;
  t1 = (struct tcp_header *)((char *)t1 +tmp);

  memcpy (t1, (void *)&(sk->dummy_th), sizeof (*t1));
  t1->seq = net32(sk->send_seq++);
  buff->h.seq = sk->send_seq;
  t1->ack = 0;
  t1->window = 2;
  t1->res1=0;
  t1->res2=0;
  t1->rst = 0;
  t1->urg = 0;
  t1->psh = 0;
  t1->syn = 1;
  t1->urg_ptr = 0;
  t1->doff =6;
  /* put in the tcp options to say mtu. */
  ptr=(unsigned char *)(t1+1);
  ptr[0]=2;
  ptr[1]=4;
  ptr[2]=(dev->mtu- HEADER_SIZE) >> 8;
  ptr[3]=(dev->mtu- HEADER_SIZE) & 0xff;
  sk->mtu = dev->mtu - HEADER_SIZE;
  tcp_send_check (t1, sk->saddr, sk->daddr,
		  sizeof (struct tcp_header) + 4, sk);
  /* this must go first otherwise a really quick response will
     get reset. */
  sk->state = TCP_SYN_SENT;

  sk->prot->queue_xmit(sk, dev, buff, 0);
  
  sk->time_wait.len = TCP_CONNECT_TIME;
  reset_timer ((struct timer *)&sk->time_wait);
  sk->retransmits = TCP_RETR1 - TCP_SYN_RETRIES;
  release_sock (sk);
  return (0);
}


/* this functions checks to see if the tcp header is actually
   acceptible. */

static  int
tcp_sequence (volatile struct sock *sk, struct tcp_header *th, short len,
	      struct options *opt, unsigned long saddr)
{
   /* this isn't quite right.  sk->acked_seq could be more recent
      than sk->window.  This is however close enough.  We will accept
      slightly more packets than we should, but it should not cause
      problems unless someone is trying to forge packets. */

  if (between(th->seq, sk->acked_seq, sk->acked_seq + sk->window)||
      between(th->seq + len-sizeof (*th), sk->acked_seq+1, 
	      sk->acked_seq + sk->window))
    {
       return (1);
    }

  /* if it's too far ahead, send an ack to let the other end
     know what we expect. */
  if (after (th->seq, sk->acked_seq + sk->window))
    {
       tcp_send_ack (sk->send_seq, sk->acked_seq, sk, th, saddr);
       return (0);
    }

  if (!th->rst)
    {
       if (len != th->doff*4 || th->fin || th->syn)
	 {
	    sk->delay_acks = 0;
	 }

       /* try to resync things. */
       tcp_send_ack (net32(th->ack_seq), sk->acked_seq, sk, th, saddr);
    }

  /* in case it's just a late ack, let it through */
  if (th->ack && len == th->doff*4 && after (th->seq, sk->acked_seq - 4096) &&
      !th->fin && !th->syn) return (1);

  return (0);
}

/* This deals with the tcp option.  It isn't very general yet. */
static void
tcp_options (volatile struct sock *sk, struct tcp_header *th)
{
  unsigned char *ptr;
  ptr = (unsigned char *)(th + 1);
  if (ptr[0] != 2 || ptr[1] != 4)
    {
       sk->mtu = min (sk->mtu, 576-HEADER_SIZE);
       return;
    }
  sk->mtu = min (sk->mtu, ptr[2]*256 + ptr[3] - HEADER_SIZE);
}

int
tcp_rcv(struct sk_buff *skb, struct device *dev, struct options *opt,
	unsigned long daddr, unsigned short len,
	unsigned long saddr, int redo, struct ip_protocol * protocol)
{
  struct tcp_header *th;
  volatile struct sock *sk;

  if (!skb)
    {
      printk ("tcp.c: tcp_rcv skb = NULL\n");
      return (0);
    }
#if 0 /* it's ok for protocol to be NULL */
  if (!protocol)
    {
      printk ("tcp.c: tcp_rcv protocol = NULL\n");
      return (0);
    }

  if (!opt) /* it's ok for opt to be NULL */
    {
      printk ("tcp.c: tcp_rcv opt = NULL\n");
    }
#endif
  if (!dev)
    {
      printk ("tcp.c: tcp_rcv dev = NULL\n");
      return (0);
    }

  th = skb->h.th;

  /* find the socket. */
  sk=get_sock(&tcp_prot, net16(th->dest), saddr, th->source, daddr);
  PRINTK("<<\n");
  PRINTK("len = %d, redo = %d, skb=%X\n", len, redo, skb);

  if (sk)
    {
      PRINTK ("sk = %X:\n",sk);
      print_sk (sk);
    }

  if (!redo)
    {
       if (th->check && tcp_check (th, len, saddr, daddr ))
	 {
	    skb->sk = NULL;
	    free_skb (skb, 0);
	    /* we don't release the socket because it was never
	       marked in use. */
	    return (0);
	 }

       /*See if we know about the socket. */
       if (sk == NULL)
	{
	  if (!th->rst)
	    tcp_reset (daddr, saddr, th, &tcp_prot, opt,dev);
	  skb->sk = NULL;
	  free_skb (skb, 0);
	  return (0);
	}

       skb->len = len;
       skb->sk = sk;
       skb->acked = 0;
       skb->used = 0;
       skb->free = 0;
       skb->urg_used = 0;
       skb->saddr = daddr;
       skb->daddr = saddr;

       th->seq = net32(th->seq);

       cli();

       /* we may need to add it to the backlog here. */
       if (sk->inuse)
	 {
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
  else
    {
      if (!sk)
	{
	  printk ("tcp.c: tcp_rcv bug sk=NULL redo = 1\n");
	  return (0);
	}
    }

  if (!sk->prot)
    {
      printk ("tcp.c: tcp_rcv sk->prot = NULL \n");
      return (0);
    }

  /* charge the memory to the socket. */
  if (sk->rmem_alloc + skb->mem_len >= SK_RMEM_MAX)
    {
       skb->sk = NULL;
       free_skb (skb, 0);
       release_sock (sk);
       return (0);
    }
       
  sk->rmem_alloc += skb->mem_len;

  PRINTK ("About to do switch. \n");

  /* now deal with it. */

  switch (sk->state)
    {
       /* this should close the system down if it's waiting for an
	  ack that is never going to be sent. */
    case TCP_LAST_ACK:
      if (th->rst)
	{
	  sk->err = ECONNRESET;
	  sk->state = TCP_CLOSE;
	  if (!sk->dead)
	    {
	      wake_up (sk->sleep);
	    }
	  free_skb (skb, FREE_READ);
	  release_sock(sk);
	  return (0);
	}

    case TCP_ESTABLISHED:
    case TCP_FIN_WAIT1:
    case TCP_FIN_WAIT2:
    case TCP_TIME_WAIT:
   
      if (!tcp_sequence (sk, th, len, opt, saddr))
	{
	   free_skb (skb, FREE_READ);
	   release_sock(sk);
	   return (0);
	}

      if (th->rst)
	{
	  sk->err = ECONNRESET;
	  sk->state = TCP_CLOSE;
	  if (!sk->dead)
	    {
	      wake_up (sk->sleep);
	    }
	  free_skb (skb, FREE_READ);
	  release_sock(sk);
	  return (0);
	}
      if (opt && (opt->security != 0 || opt->compartment != 0 || th->syn))
	{
	   sk->err = ECONNRESET;
	   sk->state = TCP_CLOSE;
	   tcp_reset (daddr, saddr,  th, sk->prot, opt,dev);
	   if (!sk->dead)
	     {
		wake_up (sk->sleep);
	     }
	   free_skb (skb, FREE_READ);
	   release_sock(sk);
	   return (0);
	}

      if (th->ack)
	{
	   if(!tcp_ack (sk, th, saddr))
	    {
	       free_skb (skb, FREE_READ);
	       release_sock(sk);
	       return (0);
	   }
	}
      if (th->urg)
	{
	  if (tcp_urg (sk, th, saddr))
	    {
	       free_skb (skb, FREE_READ);
	       release_sock(sk);
	       return (0);
	    }
	}

      if ( tcp_data (skb, sk, saddr, len))
	{
	   free_skb (skb, FREE_READ);
	   release_sock(sk);
	   return (0);
	}

      if (!th->fin)
	{
	  release_sock(sk);
	  return (0);
	}

      tcp_fin (sk, th, saddr, dev);
      release_sock(sk);
      return (0);

    case TCP_CLOSE:

      if (sk->dead || sk->daddr)
	{
	   PRINTK ("packet received for closed,dead socket\n");
	   free_skb (skb, FREE_READ);
	   release_sock (sk);
	   return (0);
	}

      if (!th->rst)
	{
	  if (!th->ack)
	    th->ack_seq=0;
	  tcp_reset (daddr, saddr, th, sk->prot, opt,dev);
	}
      free_skb (skb, FREE_READ);
      release_sock(sk);
      return (0);

    case TCP_LISTEN:
      if (th->rst)
	{
	   free_skb (skb, FREE_READ);
	   release_sock(sk);
	   return (0);
	}
      if (th->ack)
	{
	  tcp_reset (daddr, saddr, th, sk->prot, opt,dev );
	  free_skb (skb, FREE_READ);
	  release_sock(sk);
	  return (0);
	}

      if (th->syn)
	{
/*	  if (opt->security != 0 || opt->compartment != 0)
	    {
	      tcp_reset (daddr, saddr, th, prot, opt,dev);
	      release_sock(sk);
	      return (0);
	    } */

	  /* now we just put the whole thing including the header
	     and saddr, and protocol pointer into the buffer.
	     We can't respond until the user tells us to accept
	     the connection. */

	  tcp_conn_request (sk, skb, daddr, saddr, opt, dev);

	  release_sock(sk);
	  return (0);
	}

      free_skb (skb, FREE_READ);
      release_sock(sk);
      return (0);

    default:
      if (!tcp_sequence (sk, th, len, opt, saddr)) 
	{
	   free_skb (skb, FREE_READ);
	   release_sock(sk);
	   return (0);
	}

    case TCP_SYN_SENT:
      if (th->rst)
	{
	  sk->err = ECONNREFUSED;
	  sk->state = TCP_CLOSE;
	  if (!sk->dead)
	    {
	      wake_up (sk->sleep);
	    }
	  free_skb (skb, FREE_READ);
	  release_sock(sk);
	  return (0);
	}
/*      if (opt->security != 0 || opt->compartment != 0 )
	{
	  sk->err = ECONNRESET;
	  sk->state = TCP_CLOSE;
	  tcp_reset (daddr, saddr,  th, sk->prot, opt, dev);
	  if (!sk->dead)
	  {
	  wake_up (sk->sleep);
	  }
	  free_skb (skb, FREE_READ);
	  release_sock(sk);
	  return (0);
	} */

      if (!th->ack) 
	{
	  if (th->syn)
	    {
	      sk->state = TCP_SYN_RECV;
	    }

	  free_skb (skb, FREE_READ);
	  release_sock(sk);
	  return (0);
	}

      switch (sk->state)
	{
	case TCP_SYN_SENT:
	  if (!tcp_ack(sk, th, saddr))
	    {
	      tcp_reset(daddr, saddr, th, sk->prot, opt,dev);
	      free_skb (skb, FREE_READ);
	      release_sock(sk);
	      return (0);
	    }

	  /* if the syn bit is also set, switch to tcp_syn_recv,
	     and then to established. */
	      
	  if (!th->syn) 
	    {
	      free_skb (skb, FREE_READ);
	      release_sock (sk);
	      return (0);
	    }

	  /* ack the syn and fall through. */
	  sk->acked_seq = th->seq+1;
	  sk->fin_seq = th->seq;
	  tcp_send_ack (sk->send_seq, th->seq+1, sk, 
			     th, sk->daddr);
	
	case TCP_SYN_RECV:
	  if (!tcp_ack(sk, th, saddr))
	    {
	      tcp_reset(daddr, saddr, th, sk->prot, opt, dev);
	      free_skb (skb, FREE_READ);
	      release_sock(sk);
	      return (0);
	    }

	  sk->state = TCP_ESTABLISHED;
	  /* now we need to finish filling out some of the tcp
	     header. */

	  /* we need to check for mtu info. */
	  tcp_options(sk, th);
	  sk->dummy_th.dest = th->source;
	  sk->copied_seq = sk->acked_seq-1;
	  if (!sk->dead)
	    {
	      wake_up (sk->sleep);
	    }

	  /* now process the rest like we were already in the established
	     state. */
	  if (th->urg)
	    if (tcp_urg (sk, th, saddr))
	      {
		 free_skb (skb, FREE_READ);
		 release_sock(sk);
		 return (0);
	      }
	  if (tcp_data (skb, sk, saddr, len))
	    free_skb (skb, FREE_READ);
	  
	  if (th->fin)
	    tcp_fin(sk, th, saddr, dev);

	  release_sock(sk);
	  return (0);
	}

      if (th->urg)
	{
	  if (tcp_urg (sk, th, saddr))
	    {
	       free_skb (skb, FREE_READ);
	       release_sock (sk);
	       return (0);
	    }
	}

      if (tcp_data (skb, sk, saddr, len))
	{
	   free_skb (skb, FREE_READ);
	   release_sock (sk);
	   return (0);
	}

      if (!th->fin)
	{
	  release_sock(sk);
	  return (0);
	}
      tcp_fin (sk, th, saddr, dev);
      release_sock(sk);
      return (0);
    }
}


/* this routine sends a packet with an out of date sequence number. It
   assumes the other end will try to ack it. */

static  void
tcp_write_wakeup(volatile struct sock *sk)
{
  struct sk_buff *buff;
  struct tcp_header *t1;
  struct device *dev=NULL;
  int tmp;
  if (sk -> state != TCP_ESTABLISHED) return;

  buff=sk->prot->wmalloc(sk,MAX_ACK_SIZE,1);
  /* no big loss. */
  if (buff == NULL) return;

  buff->mem_addr = buff;
  buff->mem_len = MAX_ACK_SIZE;
  buff->len=sizeof (struct tcp_header);
  buff->free = 1;
  buff->sk = sk;
  PRINTK ("in tcp_write_wakeup\n");
  t1=(struct tcp_header *)(buff + 1);

  /* put in the ip_header and routing stuff. */
  tmp = sk->prot->build_header (buff, sk->saddr, sk->daddr, &dev,
				IPPROTO_TCP, sk->opt, MAX_ACK_SIZE);
  if (tmp < 0)
    {
      sk->prot->wfree(sk, buff->mem_addr, buff->mem_len);
      return;
    }

  buff->len += tmp;
  t1 = (struct tcp_header *)((char *)t1 +tmp);

  memcpy (t1,(void *) &sk->dummy_th, sizeof (*t1));

  /* use a previous sequence.  This should cause the other end
     to send an ack. */
  t1->seq = net32(sk->send_seq-1);
  t1->ack = 1; 
  t1->res1= 0;
  t1->res2= 0;
  t1->rst = 0;
  t1->urg = 0;
  t1->psh = 0;
  t1->fin = 0;
  t1->syn = 0;
  t1->ack_seq = net32(sk->acked_seq);
  t1->window = net16(sk->prot->rspace(sk));
  t1->doff = sizeof (*t1)/4;
  tcp_send_check (t1, sk->saddr, sk->daddr, sizeof (*t1), sk);
  /* send it and free it.  This will prevent the timer from 
     automatically being restarted. */
  sk->prot->queue_xmit(sk, dev, buff, 1);

}

struct proto tcp_prot =
{
  sock_wmalloc,
  sock_rmalloc,
  sock_wfree,
  sock_rfree,
  sock_rspace,
  sock_wspace,
  tcp_close,
  tcp_read,
  tcp_write,
  NULL,
  NULL,
  ip_build_header,
  tcp_connect,
  tcp_accept,
  ip_queue_xmit,
  tcp_retransmit,
  tcp_write_wakeup,
  tcp_read_wakeup,
  tcp_rcv,
  tcp_select,
  tcp_ioctl,
  NULL,
  128,
  0,
  {NULL,}
};




