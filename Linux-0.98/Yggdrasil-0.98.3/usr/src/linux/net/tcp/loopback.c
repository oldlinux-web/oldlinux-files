/* loopback.c contains the loopback device functions. */
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
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/tty.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/string.h>
#include <asm/system.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/fcntl.h>
#include <netinet/in.h>

#include "dev.h"
#include "eth.h"
#include "timer.h"
#include "ip.h"
#include "tcp.h"
#include "sock.h"
#include "arp.h"

#include "../kern_sock.h" /* for PRINTK */



static int
loopback_xmit(struct sk_buff *skb, struct device *dev)
{
  static int inuse=0;
  struct enet_header *eth;
  int i;
  int done;
  static unsigned char buff[2048];
  unsigned char *tmp;

  PRINTK ("loopback_xmit (dev = %X)\n", dev);
  cli();
  if (inuse)
    {
       sti();
       return (1);
    }
  inuse = 1;
  sti();
  tmp = NULL;
  done = dev_rint ((unsigned char *)(skb+1), skb->len, 0, dev);

  if (skb->free)
    free_skb (skb, FREE_WRITE);

  while (done != 1)
	 {
	    if (done != -1 && (i = dev_tint (buff,dev)) != 0)
	      {
		 /* print out the buffer. */
		 PRINTK ("ethernet xmit: \n");
		 eth = (struct enet_header *)buff;
		 print_eth (eth);
		 tmp = buff;
		 done = dev_rint (buff, i, 0, dev);
		 if (done != -1) tmp = NULL;
	      }
	    else
	      {
		 done = dev_rint (tmp, 0, 0, dev);
	      }
	    
	 }
  inuse = 0;
  return (0);
}

void
loopback_init(struct device *dev)
{
   printk ("Loopback device init\n");
  /* initialize the rest of the device structure. */
  dev->mtu = 2000; /* mtu */
  dev->hard_start_xmit = loopback_xmit;
  dev->open = NULL;
  dev->hard_header = eth_hard_header;
  dev->add_arp = NULL;
  dev->hard_header_len = sizeof (struct enet_header);
  dev->addr_len = ETHER_ADDR_LEN;
  dev->type = ETHER_TYPE;
  dev->queue_xmit = dev_queue_xmit;
  dev->rebuild_header = eth_rebuild_header;
  dev->type_trans = eth_type_trans;
  dev->loopback = 1;
}
