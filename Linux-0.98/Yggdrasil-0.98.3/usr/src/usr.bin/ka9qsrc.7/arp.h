/* Size of ARP hash table */
#define	ARPSIZE	17

/* Lifetime of a valid ARP entry */
#define	ARPLIFE		900	/* 15 minutes */
/* Lifetime of a pending ARP entry */
#define	PENDTIME	30	/* 30 seconds */

/* ARP definitions (see RFC 826) */

/* Address size definitions */
#define	IPALEN	4		/* Length in bytes of an IP address */
#define	MAXHWALEN	255	/* Maximum length of a hardware address */

/* ARP opcodes */
#define	ARP_REQUEST	1
#define	ARP_REPLY	2

/* Hardware types */
#define	ARP_NETROM	0	/* Fake for NET/ROM (never actually sent) */
#define	ARP_ETHER	1	/* Assigned to 10 megabit Ethernet */
#define	ARP_EETHER	2	/* Assigned to experimental Ethernet */
#define	ARP_AX25	3	/* Assigned to AX.25 Level 2 */
#define	ARP_PRONET	4	/* Assigned to PROnet token ring */
#define	ARP_CHAOS	5	/* Assigned to Chaosnet */
#define	ARP_ARCNET	7
#define	ARP_APPLETALK	8
extern char *arptypes[];	/* Type fields in ASCII, defined in arpcmd */
#define	NHWTYPES 9

/* Table of hardware types known to ARP */
struct arp_type {
	int16 hwalen;		/* Hardware length */
	int16 iptype;		/* Hardware type field for IP */
	int16 arptype;		/* Hardware type field for ARP */
	char *bdcst;		/* Hardware broadcast address */
	int (*format)();	/* Function that formats addresses */
	int (*scan)();		/* Reverse of format */
};
extern struct arp_type arp_type[];
#define	NULLATYPE	(struct arp_type *)0

/* Format of an ARP request or reply packet. From p. 3 */
struct arp {
	int16 hardware;			/* Hardware type */
	int16 protocol;			/* Protocol type */
	char hwalen;			/* Hardware address length, bytes */
	char pralen;			/* Length of protocol address */
	int16 opcode;			/* ARP opcode (request/reply) */
	char shwaddr[MAXHWALEN];	/* Sender hardware address field */
	int32 sprotaddr;		/* Sender Protocol address field */
	char thwaddr[MAXHWALEN];	/* Target hardware address field */
	int32 tprotaddr;		/* Target protocol address field */
};
		
/* Format of ARP table */
struct arp_tab {
	struct arp_tab *next;	/* Doubly-linked list pointers */
	struct arp_tab *prev;	
	int32 ip_addr;		/* IP Address, host order */
	int16 hardware;		/* Hardware type */
	char *hw_addr;		/* Hardware address */
	char state;		/* (In)complete */
#define	ARP_PENDING	0
#define	ARP_VALID	1
	char pub;		/* Respond to requests for this entry? */
	struct timer timer;	/* Time until aging this entry */
	struct mbuf *pending;	/* Queue of datagrams awaiting resolution */
};
struct arp_tab *arp_lookup(),*arp_add();
#define	NULLARP	(struct arp_tab *)0
extern struct arp_tab *arp_tab[];

struct arp_stat {
	unsigned recv;		/* Total number of ARP packets received */
	unsigned badtype;	/* Incoming requests for unsupported hardware */
	unsigned badlen;	/* Incoming length field(s) didn't match types */
	unsigned badaddr;	/* Bogus incoming addresses */
	unsigned inreq;		/* Incoming requests for us */
	unsigned replies;	/* Replies sent */
	unsigned outreq;	/* Outoging requests sent */
};
