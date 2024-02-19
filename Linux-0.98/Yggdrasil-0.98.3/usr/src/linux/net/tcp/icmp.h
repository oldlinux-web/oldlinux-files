/* Internet Control Message Protocol (ICMP) header file */


#define ICMP_ECHOREPLY		0
#define ICMP_DEST_UNREACH	3
#define ICMP_SOURCE_QUENCH	4
#define ICMP_REDIRECT		5
#define ICMP_ECHO		8
#define ICMP_TIME_EXCEEDED     11
#define ICMP_PARAMETERPROB     12
#define ICMP_TIMESTAMP	       13
#define ICMP_TIMESTAMPREPLY    14
#define ICMP_INFO_REQUEST      15
#define ICMP_INFO_REPLY	       16


/* used by unreachable. */

#define ICMP_NET_UNREACH	0
#define ICMP_HOST_UNREACH	1
#define ICMP_PROT_UNREACH	2
#define ICMP_PORT_UNREACH	3 /* lots of room for confusion. */
#define ICMP_FRAG_NNEDED	4
#define ICMP_SR_FAILED		5
#define ICMP_NET_UNKNOWN	6
#define ICMP_HOST_UNKNOWN	7
#define ICMP_HOST_ISOLATED	8
#define ICMP_NET_ANO		9
#define ICMP_HOST_ANO	       10
#define ICMP_NET_UNR_TOS       11
#define ICMP_HOST_UNR_TOS      12


struct icmp_header
{
	unsigned char type;
	unsigned char code;
	unsigned short checksum;
	union 
	  {
	     struct
	       {
		  unsigned short id;
		  unsigned short sequence;
	       } echo;
	     unsigned long gateway;
	  } un;
};

struct icmp_err 
{
   int errno;
   unsigned fatal:1;
};

extern struct icmp_err icmp_err_convert[];

int
icmp_rcv(struct sk_buff *skb, struct device *dev, struct options *opt,
	unsigned long daddr, unsigned short len,
	unsigned long saddr, int redo, struct ip_protocol *protocol);

void
icmp_reply (struct sk_buff *skb_in,  int type, int code, struct device *dev);
