/* timer.c */
/*
  Copyright (C) 1992  Ross Biro
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
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
#include <linux/errno.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <asm/system.h>
#include "timer.h"
#include "ip.h"
#include "tcp.h"
#include "sock.h"
#include "arp.h"
#include "../kern_sock.h"

static struct timer *timer_base=NULL;
unsigned long seq_offset;

void
delete_timer (struct timer *t)
{
   struct timer *tm;
   PRINTK ("delete_timer (t=%X)\n",t);
   if (timer_base == NULL) return;
   cli();
   if (t == timer_base) 
     {
	timer_base = t->next;
	if (timer_base != NULL)
	  {
	     timer_table[NET_TIMER].expires = timer_base->when;
	     timer_active |= 1 << NET_TIMER;
	  }
	else
	  {
	     timer_active &= ~(1 << NET_TIMER);
	  }
	sti();
	return;
     }
   for (tm = timer_base;tm->next != NULL ;tm=tm->next)
     {
	if (tm->next == t)
	  {
	     tm->next = t->next;
	     sti();
	     return;
	  }
     }
   sti();
}


void
reset_timer (struct timer *t)
{
   struct timer *tm;

   delete_timer (t);
   t->when = timer_seq + t->len;
   PRINTK ("reset_timer (t=%X) when = %d jiffies = %d\n",t, t->when, jiffies);
   if (t == NULL)
     {
	printk ("*** reset timer NULL timer\n");
	__asm__ ("\t int $3\n"::);
     }
   /* first see if it goes at the beginning. */
   cli();
   if (timer_base == NULL) 
     {
	t->next = NULL;
	timer_base = t;
	timer_table[NET_TIMER].expires = t->when;
	timer_active |= 1 << NET_TIMER;
	sti();
	return;
     }
   if (before (t->when, timer_base->when))
     {
	t->next = timer_base;
	timer_base = t;
	timer_table[NET_TIMER].expires = t->when;
	timer_active |= 1 << NET_TIMER;
	sti();
	return;
     }
   for (tm = timer_base; ; tm=tm->next)
     {
	if (tm->next == NULL || before (t->when,tm->next->when))
	  {
	     t->next = tm->next;
	     tm->next = t;
	     sti();
	     return;
	  }
     }
}

void
net_timer (void)
{
   volatile struct sock *sk;

   /* now we will only be called whenever we need to do something, but
      we must be sure to process all of the sockets that need it. */

   while (timer_base != NULL && after (timer_seq+1 ,timer_base->when))
     {
	int why;
	sk = timer_base->sk;
	cli();
	if (sk->inuse)
	  {
	     sti();
	     break;
	  }
	sk->inuse = 1;
	sti();
	PRINTK ("net_timer: found sk=%X\n",sk);
	why = sk->timeout;

	if (sk->keepopen)
	  {
	     sk->time_wait.len = TCP_TIMEOUT_LEN;
	     sk->timeout = TIME_KEEPOPEN;
	     reset_timer (timer_base);
	  }
	else
	  {
	     sk->timeout = 0;
	     delete_timer(timer_base);
	  }
	
	/* always see if we need to send an ack. */
	if (sk->ack_backlog)
	  {
	     sk->prot->read_wakeup(sk);
	     if (!sk->dead) wake_up (sk->sleep);
	  }
	
	/* now we need to figure out why the socket was on the timer. */
	switch (why)
	  {

	    case TIME_DONE:
	     if (!sk->dead || sk->state != TCP_CLOSE)
	       {
		  printk ("non dead socket in time_done\n");
		  release_sock (sk);
		  break;
	       }
	     destroy_sock (sk);
	     break;

	    case TIME_DESTROY: /* we've waited for a while for all
				  the memory assosiated with the
				  socket to be freed.  We need to
				  print an error message. */
	     PRINTK ("possible memory leak.  sk = %X\n", sk);
	     print_sk (sk);
	     reset_timer ((struct timer *)&sk->time_wait);
	     sk->inuse = 0;
	     break;
	     
	    case TIME_CLOSE: /* we've waited long enough, close the
				socket. */
	     
	     sk->state = TCP_CLOSE;
	     delete_timer ((struct timer *)&sk->time_wait);
	     /* kill the arp entry 
		in case the hardware has changed. */
	     arp_destroy (sk->daddr);
	     if (!sk->dead)
	       wake_up (sk->sleep);
	     release_sock(sk);
	     break;
	     
	    case TIME_WRITE: /* try to retransmit. */
	     if (sk->send_head != NULL)
	       {
		  sk->retransmits ++;
		  if (sk->retransmits > TCP_RETR1)
		    {
		       arp_destroy (sk->daddr);
		       ip_route_check (sk->daddr);
		    }

		  if (sk->retransmits > TCP_RETR2)
		    {
		       sk->err = ETIMEDOUT;
		       arp_destroy (sk->daddr);
		       if (sk->state == TCP_FIN_WAIT1 ||
			   sk->state == TCP_FIN_WAIT2 ||
			   sk->state == TCP_LAST_ACK)
			 {
			    sk->state = TCP_TIME_WAIT;
			    sk->timeout = TIME_CLOSE;
			    sk->time_wait.len = TCP_TIMEWAIT_LEN;
			    reset_timer ((struct timer *)&sk->time_wait);
			    release_sock(sk);
			    break;
			 }
		       else /* sk->state == ... */
			 {
			    sk->prot->close (sk,1);
			    break;
			 }
		    }
		  else /* sk->retransmites .. */
		    {
		       sk->prot->retransmit (sk, 1);
		       release_sock (sk);
		    }
		  break;
	       }
	     /* if we have stuff which hasn't been written because the
		window is too small, fall throught to TIME_KEEPOPEN */
	     if (sk->wfront == NULL)
	       {
		  release_sock (sk);
		  break;
	       }

	     /* this basically assumes tcp here. */
	     /* exponential fall back. */
	     sk->rtt *= 2;
	     sk->time_wait.len = sk->rtt*2;
	     sk->timeout = TIME_WRITE;
	     reset_timer ((struct timer *)&sk->time_wait);

	    case TIME_KEEPOPEN: /* send something to keep the
				   connection open. */
	     sk->retransmits ++;
	     if (sk->retransmits > TCP_RETR1)
	       {
		  arp_destroy (sk->daddr);
		  ip_route_check (sk->daddr);
		  
	       }
	     if (sk->retransmits > TCP_RETR2)
	       {
		  arp_destroy (sk->daddr);
		  sk->err = ETIMEDOUT;
		  if (sk->state == TCP_FIN_WAIT1 ||
		      sk->state == TCP_FIN_WAIT2)
		    {
		       sk->state = TCP_TIME_WAIT;
		       if (!sk->dead)
			 wake_up (sk->sleep);
		       release_sock(sk);
		    }
		  else /* sk->state == */
		    {
		       sk->prot->close (sk, 1);
		    }
		  break;
	       }
	     else /* sk->retransmits. */
	       {
		  if (sk->prot->write_wakeup != NULL)
		    sk->prot->write_wakeup(sk);
		  release_sock (sk);
		  break;
	       }
	     
	    default:
	     release_sock(sk);
	     break;
	  } /* switch */
     } /* while (timer_base != ...  */

   /* Now we need to reset the timer. */
   if (timer_base != NULL)
     {
	timer_table[NET_TIMER].expires = timer_base->when;
	timer_active |= 1 << NET_TIMER;
     }
}


