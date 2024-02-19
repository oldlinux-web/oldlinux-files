/* eth.h */
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


#ifndef _TCP_ETH_H
#define _TCP_ETH_H

#define ETHER_MIN_LEN 64
#define ETHER_ADDR_LEN 6

#define ETHERTYPE_ARP 0x806
#define ETHERTYPE_IP  0x800
#define ETHER_TYPE 1

/* Reciever modes */
#define ETH_MODE_MONITOR		1	/* Monitor mode - no receive */
#define ETH_MODE_PHYSICAL		2	/* Physical address receive only */
#define ETH_MODE_BROADCAST		3	/* Broadcast receive + mode 2 */
#define ETH_MODE_MULTICAST		4	/* Multicast receive + mode 3 */
#define ETH_MODE_PROMISCUOUS		5	/* Promiscuous mode - receive all */

#define WD_RX_SAVE_ERRORS	1	/* save error packets */
#define WD_RX_RUNT		2	/* accept runt packets */
#define WD_RX_BROAD		4	/* accept broadcast packets */
#define WD_RX_MULTI		8	/* accept multicast packets */
#define WD_RX_PROM		0x10	/* accept all packets */
#define WD_RX_MON		0x20	/* monitor mode (just count packets) */

#define NET16(x) (((x&0xff)<<8)|((x>>8)&0xff))

struct enet_header
{
  unsigned char daddr[ETHER_ADDR_LEN];
  unsigned char saddr[ETHER_ADDR_LEN];
  unsigned short type;
};

#define ETHER_HEADER sizeof(struct enet_header)

struct enet_statistics{
  int rx_packets;	/* total packets received */
  int tx_packets;	/* total packets transmitted */
  int rx_errors;	/* bad packets received */
  int tx_errors;	/* packet transmit problems */
  int rx_dropped;	/* no space in linux buffers */
  int tx_dropped;	/* no space available in linux */
  int collisions;	/* total number of collisions */
  int multicast;	/* multicast packets received */
			/* detailed rx_errors: */
  int rx_length_errors;
  int rx_over_errors;	/* receiver overwrote ring buffer in card */
  int rx_crc_errors; 	/* received packet with crc error */
  int rx_frame_errors; 	/* received frame alignment error */
  int rx_fifo_errors;	/* receiver fifo overrun */
  int rx_missed_errors; /* receiver missed packet */
			/* detailed tx_errors */
  int tx_aborted_errors;
  int tx_carrier_errors;
  int tx_fifo_errors;
  int tx_heartbeat_errors;
  int tx_window_errors;
};

void print_eth(struct enet_header *eth);
int eth_hard_header (unsigned char *buff, struct device *dev,
		     unsigned short type, unsigned long daddr,
		     unsigned long saddr, unsigned len);

int eth_rebuild_header(void *eth, struct device *dev);
void eth_add_arp (unsigned long addr, struct sk_buff *skb,
		  struct device *dev);
unsigned short eth_type_trans (struct sk_buff *skb, struct device *dev);

#endif
