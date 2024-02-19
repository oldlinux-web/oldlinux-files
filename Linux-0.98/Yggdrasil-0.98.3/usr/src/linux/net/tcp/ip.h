/* ip.h */
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
#ifndef _TCP_IP_H
#define _TCP_IP_H

#include "dev.h"
#include <linux/sock_ioctl.h>
/* #include <netinet/protocols.h>*/
#include <netinet/in.h>

struct rtable
{
  unsigned long net;
  unsigned long router;
  struct device *dev;
  struct rtable *next;
};

struct route
{
  char route_size;
  char pointer;
  unsigned long route[MAX_ROUTE];
};

struct timestamp
{
  unsigned char len;
  unsigned char ptr;
  union
    {
      unsigned char flags:4, overflow:4;
      unsigned char full_char;
    } x;
  unsigned long data[9];
};

struct options
{
  struct route record_route;
  struct route loose_route;
  struct route strict_route;
  struct timestamp tstamp;
  unsigned short security;
  unsigned short compartment;
  unsigned short handling;
  unsigned short stream;
  unsigned tcc;
};



struct ip_header
{
  unsigned char ihl:4, version:4;
  unsigned char tos;
  unsigned short tot_len;
  unsigned short id;
  unsigned short frag_off;
  unsigned char ttl;
  unsigned char protocol;
  unsigned short check;
  unsigned long saddr;
  unsigned long daddr;
  /*The options start here. */
};


#define IPOPT_END 0
#define IPOPT_NOOP 1
#define IPOPT_SEC 130
#define IPOPT_LSRR 131
#define IPOPT_SSRR 137
#define IPOPT_RR 7
#define IPOPT_SID 136
#define IPOPT_TIMESTAMP 68
#define IP_LOOPBACK_ADDR 0x0100007f
 
static inline unsigned short
net16(unsigned short x)
{
  __asm__("xchgb %%cl,%%ch": "=c" (x) : "0" (x) : "cx");
  return (x);
}

static inline unsigned long
net32(unsigned long x)
{
  __asm__("xchgb %%cl,%%ch\n"
	  "\t roll $16,%%ecx\n"
	  "\t xchgb %%cl,%%ch":"=c" (x):"0"(x):"cx");
  return (x);
}

/* change the name of this. */
#define MAX_IP_PROTOS 32 /* Must be a power of 2 */

/* This is used to register protocols. */
struct ip_protocol 
{
   int (*handler) (struct sk_buff *skb, struct device *dev,
		   struct options *opt, unsigned long daddr,
		   unsigned short len, unsigned long saddr,
		   int redo, struct ip_protocol *protocol);
   void (*err_handler) (int err, unsigned char *buff, unsigned long daddr,
			unsigned long saddr, struct ip_protocol *ipprot);
   struct ip_protocol *next;
   unsigned char protocol;
   unsigned char copy:1;
   void *data;
};

extern struct ip_protocol *ip_protocol_base;
extern struct ip_protocol *ip_protos[MAX_IP_PROTOS];

#define MAX_IP_ADDRES 5
extern unsigned long ip_addr[MAX_IP_ADDRES];
#define MY_IP_ADDR ip_addr[0];
int my_ip_addr(unsigned long);

#include "eth.h"

void
print_iph (struct ip_header *);

void
print_eth (struct enet_header *);

int ip_set_dev (struct ip_config *);

int ip_build_header(struct sk_buff *skb, unsigned long saddr,
		    unsigned long daddr, struct device **dev, int type,
		    struct options *opt, int len);
void ip_queue_xmit (volatile struct sock *sk, struct device *dev, 
		    struct sk_buff *skb, int free);
void ip_retransmit(volatile struct sock *sk, int all);
int ip_rcv(struct sk_buff *buff, struct device *dev, struct packet_type *);
void add_ip_protocol (struct ip_protocol *);
int delete_ip_protocol (struct ip_protocol *);
int ip_handoff (volatile struct sock *sk);
unsigned short ip_compute_csum (unsigned char *buff, int len);
int ip_addr_match (unsigned long, unsigned long);
void add_route (struct rtable *rt);
void ip_route_check (unsigned long daddr);

#endif






