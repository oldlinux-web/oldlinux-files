/* Internet Control Message Protocol */

/* Message types */
#define	ECHO_REPLY	0	/* Echo Reply */
#define	DEST_UNREACH	3	/* Destination Unreachable */
#define	QUENCH		4	/* Source Quench */
#define	REDIRECT	5	/* Redirect */
#define	ECHO		8	/* Echo Request */
#define	TIME_EXCEED	11	/* Time-to-live Exceeded */
#define	PARAM_PROB	12	/* Parameter Problem */
#define	TIMESTAMP	13	/* Timestamp */
#define	TIME_REPLY	14	/* Timestamp Reply */
#define	INFO_RQST	15	/* Information Request */
#define	INFO_REPLY	16	/* Information Reply */

/* Internal format of an ICMP header (checksum is missing) */
struct icmp {
	char type;
	char code;
 	union icmp_args {
		int32 unused;
		unsigned char pointer;
		int32 address;
		struct {
			int16 id;
			int16 seq;
		} echo;
	} args;
};
#define	ICMPLEN		8	/* Length of ICMP header on the net */
#define	NULLICMP	(union icmp_args *)0
	
/* Destination Unreachable codes */
#define	NET_UNREACH	0	/* Net unreachable */
#define	HOST_UNREACH	1	/* Host unreachable */
#define	PROT_UNREACH	2	/* Protocol unreachable */
#define	PORT_UNREACH	3	/* Port unreachable */
#define	FRAG_NEEDED	4	/* Fragmentation needed and DF set */
#define	ROUTE_FAIL	5	/* Source route failed */

/* Time Exceeded codes */
#define	TTL_EXCEED	0	/* Time-to-live exceeded */
#define	FRAG_EXCEED	1	/* Fragment reassembly time exceeded */

/* Redirect message codes */
#define	REDR_NET	0	/* Redirect for the network */
#define	REDR_HOST	1	/* Redirect for the host */
#define	REDR_TOS	2	/* Redirect for Type of Service, or-ed with prev */

struct icmp_errors {
	unsigned checksum;		/* ICMP Checksum errors */
	unsigned nospace;		/* alloc_mbuf failed someplace */
	unsigned noloop;		/* No ICMP in response to an ICMP */
	unsigned bdcsts;		/* Ignore broadcast ICMPs */
};
#define	ICMP_TYPES	17
struct icmp_stats {
	unsigned input[ICMP_TYPES];	/* ICMP input stats by type */
	unsigned output[ICMP_TYPES];	/* ICMP output stats by type */
};

void icmp_dump();

/* ICMP messages, decoded */
extern char *icmptypes[],*unreach[],*exceed[],*redirect[];
