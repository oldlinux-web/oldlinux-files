/* Packet driver interface classes */
#define	ETHERNET	1
#define	PRONET-10	2
#define	IEEE8025	3
#define	OMNINET		4
#define	APPLETALK	5
#define	SERIAL_LINE	6
#define	STARLAN		7
#define	ARCNET		8

/* Packet driver interface types (not a complete list) */
#define	TC500		1
#define	PC2000		10
#define	WD8003		14
#define	ANYTYPE		0xffff

/* Packet driver function call numbers. From Appendix B. */
#define	DRIVER_INFO		1
#define	ACCESS_TYPE		2
#define	RELEASE_TYPE		3
#define	SEND_PKT		4
#define	TERMINATE		5
#define	GET_ADDRESS		6
#define	RESET_INTERFACE		7
#define	SET_RCV_MODE		20
#define	GET_RCV_MODE		21
#define	SET_MULTICAST_LIST	22
#define	GET_MULTICAST_LIST	23
#define	GET_STATISTICS		24

/* Packet driver error return codes. From Appendix C. */

#define	NO_ERROR	0
#define	BAD_HANDLE	1	/* invalid handle number */
#define	NO_CLASS	2	/* no interfaces of specified class found */
#define	NO_TYPE		3	/* no interfaces of specified type found */
#define	NO_NUMBER	4	/* no interfaces of specified number found */
#define	BAD_TYPE	5	/* bad packet type specified */
#define	NO_MULTICAST	6	/* this interface does not support multicast */
#define	CANT_TERMINATE	7	/* this packet driver cannot terminate */
#define	BAD_MODE	8	/* an invalid receiver mode was specified */
#define	NO_SPACE	9	/* operation failed because of insufficient space */
#define	TYPE_INUSE	10	/* the type had previously been accessed, and not released */
#define	BAD_COMMAND	11	/* the command was out of range, or not	implemented */
#define	CANT_SEND	12	/* the packet couldn't be sent (usually	hardware error) */

typedef union {
	struct {
		unsigned char lo;
		unsigned char hi;
	} byte;
	unsigned short word;
} ureg;

#define	CARRY_FLAG	0x1

struct pktdrvr {
	int class;	/* Interface class (ether/slip/etc) */
	int intno;	/* Interrupt vector */
	short handle1;	/* Driver handle(s) */
	short handle2;
	struct mbuf *buffer;	/* Currently allocated rx buffer */
	struct mbuf *rcvq;	/* Receive queue */
	unsigned rcvcnt;	/* Number of packets on queue */
	unsigned rcvmax;	/* Maximum length of rcvq */
};

extern struct pktdrvr pktdrvr[];
#define	PK_MAX	3

#define	PKT_SIG	"PKT DRVR"	/* Packet driver signature */

