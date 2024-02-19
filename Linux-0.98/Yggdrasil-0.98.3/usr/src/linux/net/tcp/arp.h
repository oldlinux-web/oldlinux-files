/* arp.h */
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
#ifndef _TCP_ARP_H
#define _TCP_ARP_H

struct arp
{
  unsigned short hrd;
  unsigned short pro;
  unsigned char hlen;
  unsigned char plen;
  unsigned short op;
};


struct arp_table
{
  struct arp_table *next;
  unsigned long last_used;
  unsigned long ip;
  unsigned char hlen;
  unsigned char hard[MAX_ADDR_LEN];
};



int arp_rcv(struct sk_buff *, struct device *, struct packet_type *);
void arp_snd (unsigned long, struct device *, unsigned long);
int arp_find (unsigned char *, unsigned long, struct device *dev,
	      unsigned long);
void arp_add_broad (unsigned long, struct device *dev);
void arp_destroy (unsigned long);
void arp_add (unsigned long addr, unsigned char *haddr, struct device *dev);
void arp_queue (struct sk_buff *skb);

#define ARP_TABLE_SIZE 16
#define ARP_IP_PROT ETHERTYPE_IP
#define ARP_REQUEST 1
#define ARP_REPLY 2
#define ARP_TIMEOUT 8640000 /* about 8 hours. */
#define ARP_RES_TIME 250 /* 2.5 seconds. */
#endif
