/* dev.c */
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

#include <asm/segment.h>
#include <asm/system.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include "dev.h"
#include "eth.h"
#include "timer.h"
#include "ip.h"
#include "tcp.h"
#include "sock.h"
#include <linux/errno.h>
#include "arp.h"

#undef DEV_DEBUG
#ifdef DEV_DEBUG
#define PRINTK printk
#else
#define PRINTK dummy_routine
#endif


static  unsigned long
min(unsigned long a, unsigned long b)
{
   if (a < b) return (a);
   return (b);
}

void
dev_add_pack (struct packet_type *pt)
{
   struct packet_type *p1;
   pt->next = ptype_base;

   /* see if we need to copy it. */
   for (p1 = ptype_base; p1 != NULL; p1 = p1->next)
     {
	if (p1->type == pt->type)
	  {
	     pt->copy = 1;
	     break;
	  }
     }

   ptype_base = pt;
   
}

void
dev_remove_pack (struct packet_type *pt)
{
   struct packet_type *lpt, *pt1;
   if (pt == ptype_base)
     {
	ptype_base = pt->next;
	return;
     }

   lpt = NULL;

   for (pt1 = ptype_base; pt1->next != NULL; pt1=pt1->next)
     {
	if (pt1->next == pt )
	  {
	     cli();
	     if (!pt->copy && lpt) 
	       lpt->copy = 0;
	     pt1->next = pt->next;
	     sti();
	     return;
	  }

	if (pt1->next -> type == pt ->type)
	  {
	     lpt = pt1->next;
	  }
     }
}

struct device *
get_dev (char *name)
{
   struct device *dev;
   for (dev = dev_base; dev != NULL; dev=dev->next)
     {
	if (strcmp (dev->name, name) == 0) return (dev);
     }
   return (NULL);
}

void
dev_queue_xmit (struct sk_buff *skb, struct device *dev, int pri)
{
  struct sk_buff *skb2;
  PRINTK ("eth_queue_xmit (skb=%X, dev=%X, pri = %d)\n", skb, dev, pri);
  skb->dev = dev;
  if (pri < 0 || pri >= DEV_NUMBUFFS)
    {
       printk ("bad priority in dev_queue_xmit.\n");
       pri = 1;
    }

  if (dev->hard_start_xmit(skb, dev) == 0)
    {
       return;
    }

  if (skb->next != NULL)
    {
       printk ("retransmitted packet still on queue. \n");
       return;
    }

  /* used to say it is not currently on a send list. */
  skb->next = NULL;


  /* put skb into a bidirectional circular linked list. */
  PRINTK ("eth_queue dev->buffs[%d]=%X\n",pri, dev->buffs[pri]);
  /* interrupts should already be cleared by hard_start_xmit. */
  cli();
  if (dev->buffs[pri] == NULL)
    {
      dev->buffs[pri]=skb;
      skb->next = skb;
      skb->prev = skb;
    }
  else
    {
      skb2=dev->buffs[pri];
      skb->next = skb2;
      skb->prev = skb2->prev;
      skb->next->prev = skb;
      skb->prev->next = skb;
    }
  sti();

}


/* this routine now just gets the data out of the card and returns.
   it's return values now mean.

   1 <- exit even if you have more packets.
   0 <- call me again no matter what.
  -1 <- last packet not processed, try again. */

int
dev_rint(unsigned char *buff, unsigned long len, int flags,
	 struct device * dev)
{
   struct sk_buff *skb=NULL;
   struct packet_type *ptype;
   unsigned short type;
   unsigned char flag =0;
   unsigned char *to;
   int amount;

   /* try to grab some memory. */
   if (len > 0 && buff != NULL)
     {
	skb = malloc (sizeof (*skb) + len);
	if (skb != NULL)
	  {
	    skb->mem_len = sizeof (*skb) + len;
	    skb->mem_addr = skb;
	  }
     }

   /* firs we copy the packet into a buffer, and save it for later. */
   if (buff != NULL && skb != NULL)
     {
	if ( !(flags & IN_SKBUFF))
	  {
	     to = (unsigned char *)(skb+1);
	     while (len > 0)
	       {
		  amount = min (len, (unsigned long) dev->rmem_end -
			        (unsigned long) buff);
		  memcpy (to, buff, amount);
		  len -= amount;
		  buff += amount;
		  to += amount;
		  if ((unsigned long)buff == dev->rmem_end)
		    buff = (unsigned char *)dev->rmem_start;
	       }
	  }
	else
	  {
	     free_s (skb->mem_addr, skb->mem_len);
	     skb = (struct sk_buff *)buff;
	  }

	skb->len = len;
	skb->dev = dev;
	skb->sk = NULL;

	/* now add it to the dev backlog. */
	cli();
	if (dev-> backlog == NULL)
	  {
	     skb->prev = skb;
	     skb->next = skb;
	     dev->backlog = skb;
	  }
	else
	  {
	     skb ->prev = dev->backlog->prev;
	     skb->next = dev->backlog;
	     skb->next->prev = skb;
	     skb->prev->next = skb;
	  }
	sti();
	return (0);
     }

   if (skb != NULL) 
     free_s (skb->mem_addr, skb->mem_len);

   /* anything left to process? */

   if (dev->backlog == NULL)
     {
	if (buff == NULL)
	  {
	     sti();
	     return (1);
	  }

	if (skb != NULL)
	  {
	     sti();
	     return (-1);
	  }

	sti();
	printk ("dev_rint:Dropping packets due to lack of memory\n");
	return (1);
     }

   skb= dev->backlog;
   if (skb->next == skb)
     {
	dev->backlog = NULL;
     }
   else
     {
	dev->backlog = skb->next;
	skb->next->prev = skb->prev;
	skb->prev->next = skb->next;
     }
   sti();

   /* bump the pointer to the next structure. */
   skb->h.raw = (unsigned char *)(skb+1) + dev->hard_header_len;
   skb->len -= dev->hard_header_len;

   /* convert the type to an ethernet type. */
   type = dev->type_trans (skb, dev);

   /* if there get to be a lot of types we should changes this to
      a bunch of linked lists like we do for ip protocols. */
   for (ptype = ptype_base; ptype != NULL; ptype=ptype->next)
     {
	if (ptype->type == type)
	  {
	     struct sk_buff *skb2;
	     /* copy the packet if we need to. */
	     if (ptype->copy)
	       {
		  skb2 = malloc (skb->mem_len);
		  if (skb2 == NULL) continue;
		  memcpy (skb2, skb, skb->mem_len);
		  skb2->mem_addr = skb2;
	       }
	     else
	       {
		  skb2 = skb;
		  flag = 1;
	       }
	       
	     ptype->func (skb2, dev, ptype);
	  }
     }

   if (!flag)
     {
	PRINTK ("discarding packet type = %X\n", type);
	free_skb (skb, FREE_READ);
     }

     if (buff == NULL)
       return (0);
     else
       return (-1);
}

/* This routine is called when an device interface is ready to
   transmit a packet.  Buffer points to where the packet should
   be put, and the routine returns the length of the packet.  A
   length of zero is interrpreted to mean the transmit buffers
   are empty, and the transmitter should be shut down. */

unsigned long
dev_tint(unsigned char *buff,  struct device *dev)
{
  int i;
  int tmp;
  struct sk_buff *skb;
  for (i=0; i < DEV_NUMBUFFS; i++)
    {
      while (dev->buffs[i]!=NULL)
	{
	  cli();
	  skb=dev->buffs[i];
	  if (skb->next == skb)
	    {
	      dev->buffs[i] = NULL;
	    }
	  else
	    {
	      dev->buffs[i]=skb->next;
	      skb->prev->next = skb->next;
	      skb->next->prev = skb->prev;
	    }
	  skb->next = NULL;
	  skb->prev = NULL;
	  sti();
	  tmp = skb->len;
	  if (!skb->arp)
	    {
	       if (dev->rebuild_header (skb+1, dev))
		 {
		    skb->dev = dev;
		    arp_queue (skb);
		    continue;
		 }
	    }
	     
	  if (tmp <= dev->mtu)
	    {
	       if (dev->send_packet != NULL)
		 {
		    dev->send_packet(skb, dev);
		 }
	       if (buff != NULL)
		 memcpy (buff, skb + 1, tmp);

	       PRINTK (">>\n");
	       print_eth ((struct enet_header *)(skb+1));
	    }
	  else
	    {
	       printk ("**** bug len bigger than mtu. \n");
	    }

	  if (skb->free)
	    {
		  free_skb(skb, FREE_WRITE);
	    }

	  if (tmp != 0)
	    return (tmp);
	}
    }
  PRINTK ("dev_tint returning 0 \n");
  return (0);
}

