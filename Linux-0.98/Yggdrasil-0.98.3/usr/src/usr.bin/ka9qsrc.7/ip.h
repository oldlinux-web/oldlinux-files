#define	NROUTE	5	/* Number of hash chains in routing table */

extern int32 ip_addr;	/* Our IP address for ICMP and source routing */

extern char ip_ttl;	/* Default time-to-live for IP datagrams */

#define	IPVERSION	4
/* IP header, INTERNAL representation */
struct ip {
	char version;		/* IP version number */
	char tos;		/* Type of service */
	int16 length;		/* Total length */
	int16 id;		/* Identification */
	int16 fl_offs;		/* Flags + fragment offset */

#define	F_OFFSET	0x1fff	/* Offset field */
#define	DF	0x4000		/* Don't fragment flag */
#define	MF	0x2000		/* More Fragments flag */	

	char ttl;		/* Time to live */
	char protocol;		/* Protocol */
	int16 checksum;
	int32 source;		/* Source address */
	int32 dest;		/* Destination address */
	char options[44];	/* Options field */
	int16 optlen;		/* Length of options field, bytes */
};
#define	NULLIP	(struct ip *)0
#define	IPLEN	20	/* Length of standard IP header */

/* Fields in option type byte */
#define	OPT_COPIED	0x80	/* Copied-on-fragmentation flag */
#define	OPT_CLASS	0x60	/* Option class */
#define	OPT_NUMBER	0x1f	/* Option number */

/* IP option numbers */
#define	IP_EOL		0	/* End of options list */
#define	IP_NOOP		1	/* No Operation */
#define	IP_SECURITY	2	/* Security parameters */
#define	IP_LSROUTE	3	/* Loose Source Routing */
#define	IP_TIMESTAMP	4	/* Internet Timestamp */
#define	IP_RROUTE	7	/* Record Route */
#define	IP_STREAMID	8	/* Stream ID */
#define	IP_SSROUTE	9	/* Strict Source Routing */

/* Timestamp option flags */
#define	TS_ONLY		0	/* Time stamps only */
#define	TS_ADDRESS	1	/* Addresses + Time stamps */
#define	TS_PRESPEC	3	/* Prespecified addresses only */

/* IP routing table entry */
struct route {
	struct route *prev;	/* Linked list pointers */
	struct route *next;
	int32 target;		/* Target IP address */
	int32 gateway;		/* IP address of local gateway for this target */
	int metric;		/* Hop count, whatever */
	struct interface *interface;	/* Device interface structure */
};
#define	NULLROUTE	(struct route *)0
extern struct route *routes[32][NROUTE];	/* Routing table */
extern struct route r_default;			/* Default route entry */

/* Cache for the last-used routing entry, speeds up the common case where
 * we handle a burst of packets to the same destination
 */
struct rt_cache {
	int32 target;
	struct route *route;
};
extern struct rt_cache rt_cache;

/* Reassembly descriptor */
struct reasm {
	struct reasm *next;	/* Linked list pointers */
	struct reasm *prev;
	int32 source;		/* These four fields uniquely describe a datagram */
	int32 dest;
	int16 id;
	char protocol;
	int16 length;		/* Entire datagram length, if known */
	struct timer timer;	/* Reassembly timeout timer */
	struct frag *fraglist;	/* Head of data fragment chain */
};
#define	NULLREASM	(struct reasm *)0

/* Fragment descriptor in a reassembly list */
struct frag {
	struct frag *prev;	/* Previous fragment on list */
	struct frag *next;	/* Next fragment */
	struct mbuf *buf;	/* Actual fragment data */
	int16 offset;		/* Starting offset of fragment */
	int16 last;		/* Ending offset of fragment */
};
#define	NULLFRAG	(struct frag *)0

extern struct reasm *reasmq;	/* The list of reassembly descriptors */

/* IP error logging counters */
struct ip_stats {
	long total;		/* Total packets received */
	unsigned runt;		/* Smaller than minimum size */
	unsigned length;	/* IP header length field too small */
	unsigned version;	/* Wrong IP version */
	unsigned checksum;	/* IP header checksum errors */
	unsigned badproto;	/* Unsupported protocol */
};
extern struct ip_stats ip_stats;
