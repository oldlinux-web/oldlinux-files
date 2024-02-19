/* protocols.c */
/* these headers are overkill, but until I clean up the socket header
   files, this is the best way. */

#include <asm/segment.h>
#include <asm/system.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <netinet/in.h>
#include "timer.h"
#include "ip.h"
#include "tcp.h"
#include "sock.h"
#include "icmp.h"

int udp_rcv(struct sk_buff *skb, struct device *dev, struct options *opt,
	    unsigned long daddr, unsigned short len,
	    unsigned long saddr, int redo, struct ip_protocol *protocol);

void udp_err  (int err, unsigned char *header, unsigned long daddr,
	       unsigned long saddr, struct ip_protocol *protocol);


int tcp_rcv(struct sk_buff *skb, struct device *dev, struct options *opt,
	    unsigned long daddr, unsigned short len,
	    unsigned long saddr, int redo, struct ip_protocol *protocol);

void tcp_err  (int err, unsigned char *header, unsigned long daddr,
	       unsigned long saddr, struct ip_protocol *protocol);

int icmp_rcv(struct sk_buff *skb, struct device *dev, struct options *opt,
	     unsigned long daddr, unsigned short len,
	     unsigned long saddr, int redo, struct ip_protocol *protocol);


static struct ip_protocol tcp_protocol =
{
   tcp_rcv,
   tcp_err,
   NULL,
   IPPROTO_TCP,
   0, /* copy */
   NULL
};

static struct ip_protocol udp_protocol =
{
   udp_rcv,
   udp_err,
   &tcp_protocol,
   IPPROTO_UDP,
   0, /* copy */
   NULL
};

static struct ip_protocol icmp_protocol =
{
   icmp_rcv,
   NULL,
   &udp_protocol,
   IPPROTO_ICMP,
   0, /* copy */
   NULL
};

struct ip_protocol *ip_protocol_base = &icmp_protocol;
