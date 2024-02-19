/* eth.c */
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

#undef ETH_DEBUG
#ifdef ETH_DEBUG
#define PRINTK printk
#else
#define PRINTK dummy_routine
#endif

void
print_eth (struct enet_header *eth)
{
  int i;
  PRINTK ("ether source addr: ");
  for (i =0 ; i < ETHER_ADDR_LEN; i++)
    {
      PRINTK ("0x%2X ",eth->saddr[i]);
    }
  PRINTK ("\n");

  PRINTK ("ether dest addr: ");
  for (i =0 ; i < ETHER_ADDR_LEN; i++)
    {
      PRINTK ("0x%2X ",eth->daddr[i]);
    }
  PRINTK ("\n");
  PRINTK ("ethertype = %X\n",net16(eth->type));
}

int
eth_hard_header (unsigned char *buff, struct device *dev,
		 unsigned short type, unsigned long daddr,
		 unsigned long saddr, unsigned len)
{
  struct enet_header *eth;
  eth = (struct enet_header *)buff;
  eth->type = net16(type);
  memcpy (eth->saddr, dev->dev_addr, dev->addr_len);
  if (daddr == 0)
    {
      memset (eth->daddr, 0xff, dev->addr_len);
      return (14);
    }
  if (!arp_find (eth->daddr, daddr, dev, saddr))
    {
      return (14);
    }
  else
    {
      *(unsigned long *)eth->saddr = saddr;
       return (-14);
    }
}

int
eth_rebuild_header (void *buff, struct device *dev)
{
  struct enet_header *eth;
  eth = buff;
  if (arp_find(eth->daddr, *(unsigned long*)eth->daddr, dev, 
		   *(unsigned long *)eth->saddr))
    return (1);
  memcpy (eth->saddr, dev->dev_addr, dev->addr_len);
  return (0);
}

void
eth_add_arp (unsigned long addr, struct sk_buff *skb, struct device *dev)
{
   struct enet_header *eh;
   eh = (struct enet_header *)(skb + 1);
   arp_add (addr, eh->saddr, dev);
}

unsigned short
eth_type_trans (struct sk_buff *skb, struct device *dev)
{
   struct enet_header *eh;
   eh = (struct enet_header *)(skb + 1);
   return (eh->type);
}
