/* SLIP definitions */
#define	NRS_ALLOC	40	/* Receiver allocation increment */

#define STX	0x02		/* frame start */
#define ETX 0x03		/* frame end */
#define DLE	0x10		/* data link escape */
#define NUL 0x0			/* null character */

/* packet unstuffing state machine */
#define NRS_INTER	0		/* in between packets */
#define NRS_INPACK	1		/* we've seen STX, and are in a the packet */
#define NRS_ESCAPE	2		/* we've seen a DLE while in NRS_INPACK */
#define NRS_CSUM	3		/* we've seen an ETX, and are waiting for the checksum */

/* net/rom serial protocol control structure */
struct nrs {
	struct mbuf *sndq;	/* Encapsulated packets awaiting transmission */
	int16 sndcnt;		/* Number of datagrams on queue */
	char state;			/* Receiver State control flag */
	unsigned char csum;	/* Accumulating checksum */
	struct mbuf *rbp;	/* Head of mbuf chain being filled */
	struct mbuf *rbp1;	/* Pointer to mbuf currently being written */
	char *rcp;			/* Write pointer */
	int16 rcnt;			/* Length of mbuf chain */
	struct mbuf *tbp;	/* Transmit mbuf being sent */
	long errors;		/* Checksum errors detected */
	long packets ;		/* Number of packets received successfully */
	struct interface *iface ;	/* Associated interface structure */
};

extern struct nrs nrs[];
