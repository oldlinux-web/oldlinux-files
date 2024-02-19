/* Interface control structure */
struct interface {
	struct interface *next;	/* Linked list pointer */
	char *name;		/* Ascii string with interface name */
	int16 mtu;		/* Maximum transmission unit size */
	int (*ioctl)();		/* Function to handle device control */
	int (*send)();		/* Routine to send an IP datagram */
	int (*output)();	/* Routine to send link packet */
	int (*raw)();		/* Routine to call to send raw packet */
	void (*recv)();		/* Routine to kick to process input */
	int (*stop)();		/* Routine to call before detaching */
	int16 dev;		/* Subdevice number to pass to send */
	int16 flags;		/* Configuration flags */
#define	DATAGRAM_MODE	0	/* Send datagrams in raw link frames */
#define	CONNECT_MODE	1	/* Send datagrams in connected mode */
	int16 trace;		/* Trace flags */
#define	IF_TRACE_OUT	0x01	/* Output packets */
#define	IF_TRACE_IN	0x10	/* Packets to me except broadcast */
#define	IF_TRACE_ASCII	0x100	/* Dump packets in ascii */
#define	IF_TRACE_HEX	0x200	/* Dump packets in hex/ascii */
	char *hwaddr;		/* Device hardware address, if any */
	struct interface *forw;	/* Forwarding interface for output, if rx only */
};
#define	NULLIF	(struct interface *)0
extern struct interface *ifaces;	/* Head of interface list */
