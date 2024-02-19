/* Stuff specific to the 3-Com 3C500 Ethernet controller board */
#define	EC_MAX	3
extern unsigned nec;	/* Actual number of Ethernet controllers */

/* Find base address of specified device */
#define	IE(dev)	(ec_dev[dev].addr)

/* The various IE command registers */
#define	EDLC_ADDR(num)	(num)		/* EDLC station address, 6 bytes */
#define	EDLC_RCV(num)	((num)+0x6)	/* EDLC receive csr */
#define	EDLC_XMT(num)	((num)+0x7)	/* EDLC transmit csr */
#define	IE_GP(num)	((num)+0x8)	/* GP pointer */
#define	IE_RP(num)	((num)+0xa)	/* Receive buffer pointer */
#define	IE_SAPROM(num)	((num)+0xc)	/* window on station addr prom */
#define	IE_CSR(num)	((num)+0xe)	/* IE command/status */
#define	IE_BFR(num)	((num)+0xf)	/* window on packet buffer */

/* Bits in EDLC_RCV, interrupt enable on write, status when read */
#define	EDLC_NONE	0x00	/* match mode in bits 5-6, write only */
#define	EDLC_ALL	0x40	/* promiscuous receive, write only */
#define	EDLC_BROAD	0x80	/* station address plus broadcast */
#define	EDLC_MULTI	0xc0	/* station address plus multicast */

#define	EDLC_STALE	0x80	/* receive CSR status previously read */
#define	EDLC_GOOD	0x20	/* well formed packets only */
#define	EDLC_ANY	0x10	/* any packet, even those with errors */
#define	EDLC_SHORT	0x08	/* short frame */
#define	EDLC_DRIBBLE	0x04	/* dribble error */
#define	EDLC_FCS	0x02	/* CRC error */
#define	EDLC_OVER	0x01	/* data overflow */

#define	EDLC_RERROR	(EDLC_SHORT|EDLC_DRIBBLE|EDLC_FCS|EDLC_OVER)
#define	EDLC_RMASK	(EDLC_GOOD|EDLC_ANY|EDLC_RERROR)

/* bits in EDLC_XMT, interrupt enable on write, status when read */
#define	EDLC_IDLE	0x08	/* transmit idle */
#define	EDLC_16		0x04	/* packet experienced 16 collisions */
#define	EDLC_JAM	0x02	/* packet experienced a collision */
#define	EDLC_UNDER	0x01	/* data underflow */

/* bits in IE_CSR */
#define	IE_RESET	0x80	/* reset the controller (wo) */
#define	IE_XMTBSY	0x80	/* Transmitter busy (ro) */
#define	IE_RIDE		0x40	/* request interrupt/DMA enable (rw) */
#define	IE_DMA		0x20	/* DMA request (rw) */
#define	IE_EDMA		0x10	/* DMA done (ro) */

#define	IE_BUFCTL	0x0c	/* mask for buffer control field (rw) */
#define	IE_LOOP		0x0c	/* 2 bit field in bits 2,3, loopback */
#define	IE_RCVEDLC	0x08	/* gives buffer to receiver */
#define	IE_XMTEDLC	0x04	/* gives buffer to transmit */
#define	IE_SYSBFR	0x00	/* gives buffer to processor */

#define	IE_CRC		0x01	/* causes CRC error on transmit (wo) */
#define	IE_RCVBSY	0x01	/* receive in progress (ro) */

/* miscellaneous sizes */
#define	BFRSIZ		2048	/* number of bytes in a buffer */

struct estats {
	long recv;		/* Good packets received */
	long bad;		/* Bad receive packets */
	long over;		/* Overflow errors */
	long drop;		/* Dropped because RX queue too long */
	long nomem;		/* Dropped because buffer malloc failed */
	long intrpt;		/* Interrupts */
	long xmit;		/* Total output packets */
	long timeout;		/* Transmitter timeouts */
	long jam;		/* Collisions */
	long jam16;		/* 16 successive collisions */
};
struct ec {
	unsigned base;		/* Base I/O address */
	unsigned vec;		/* Interrupt vector */
	struct mbuf *rcvq;	/* Queue of incoming packets */
	unsigned rcvcnt;	/* Number of packets on queue */
	unsigned rcvmax;	/* Maximum length of rcvq */
	struct estats estats;	/* Controller statistics */
	short size;		/* Size of current transmit packet */
	struct interface *iface;
};
extern struct ec ec[];

	
