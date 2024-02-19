/* dev.h */
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
#ifndef _TCP_DEV_H
#define _TCP_DEV_H
/* for future expansion when we will have different priorities. */
#define DEV_NUMBUFFS 3
#define MAX_ADDR_LEN 6
#define MAX_HEADER 14
#define MAX_ROUTE 16

struct device
{
  char *name;
  unsigned long rmem_end;
  unsigned long rmem_start;
  unsigned long mem_end;
  unsigned long mem_start;
  unsigned short base_addr;
  unsigned char irq;
  unsigned char start:1,
                tbusy:1,
                loopback:1,
                interrupt:1,
                up:1;
  struct device *next;
  void (*init)(struct device *dev);
  unsigned long trans_start;
  struct sk_buff *buffs[DEV_NUMBUFFS];
  struct sk_buff *backlog;
  int  (*open)(struct device *dev);
  int  (*stop)(struct device *dev);
  int (*hard_start_xmit) (struct sk_buff *skb, struct device *dev);
  int (*hard_header) (unsigned char *buff, struct device *dev,
		      unsigned short type, unsigned long daddr,
		      unsigned long saddr, unsigned len);
  void (*add_arp) (unsigned long addr, struct sk_buff *skb,
		   struct device *dev);
  void (*queue_xmit)(struct sk_buff *skb, struct device *dev, int pri);
  int (*rebuild_header)(void *eth, struct device *dev);
  unsigned short (*type_trans) (struct sk_buff *skb, struct device *dev);
  void (*send_packet)(struct sk_buff *skb, struct device *dev);
  void *private;

  unsigned short type;
  unsigned short hard_header_len;
  unsigned short mtu;
  unsigned char broadcast[MAX_ADDR_LEN];
  unsigned char dev_addr[MAX_ADDR_LEN];
  unsigned char addr_len;
};

extern struct device *dev_base;

struct packet_type
{
   unsigned short type; /* This is really NET16(ether_type) other devices
			   will have to translate appropriately. */
   unsigned short copy:1;
   int (*func) (struct sk_buff *, struct device *, struct packet_type *);
   void *data;
   struct packet_type *next;
};

/* used by dev_rint */
#define IN_SKBUFF 1

extern struct packet_type *ptype_base;
void dev_queue_xmit (struct sk_buff *skb, struct device *dev, int pri);
int dev_rint (unsigned char *buff, unsigned long len, int flags,
	      struct device *dev);
unsigned long dev_tint (unsigned char *buff, struct device *dev);
void dev_add_pack (struct packet_type *pt);
void dev_remove_pack (struct packet_type *pt);
struct device *get_dev (char *name);


#endif
