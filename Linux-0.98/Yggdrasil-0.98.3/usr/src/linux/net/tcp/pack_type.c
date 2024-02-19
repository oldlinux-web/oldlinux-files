/* pack_type.c - implements raw packet sockets. */
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

#include <linux/stddef.h>
#include "dev.h"
#include "eth.h"

extern int arp_rcv (struct sk_buff *skb, struct device *dev,
		    struct packet_type *pt);

static struct packet_type arp_packet_type=
{
   NET16(ETHERTYPE_ARP),
   0, /* copy */
   arp_rcv,
   NULL,
   NULL /* next */
};

extern int ip_rcv (struct sk_buff *skb, struct device *dev,
		   struct packet_type *pt);

static struct packet_type ip_packet_type=
{
   NET16(ETHERTYPE_IP),
   0, /* copy */
   ip_rcv,
   NULL,
   &arp_packet_type
};
   
struct packet_type *ptype_base = &ip_packet_type;





