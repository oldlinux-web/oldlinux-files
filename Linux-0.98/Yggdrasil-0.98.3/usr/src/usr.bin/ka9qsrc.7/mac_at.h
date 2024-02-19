#include <appletalk.h>
/* Stuff specific to the AppleTalk network */
#define	AT_MAX	1	/* Only one controller supported */
extern unsigned nat;	/* Actual number of Ethernet controllers */
extern  char appletalk_bdcst;
#define APPLEADDRLEN 4
#define APPLE_LEN 1
#define	AIP_TYPE	0xDD	/* Type field for IP */
#define	AARP_TYPE	0xDE	/* Type field for ARP */

struct appletalk {
	char source;
	char dest;
	int16	type;
};

/* miscellaneous sizes */
#define	BFRSIZ		2048	/* number of bytes in a buffer */
#define MAX_ATBUF	600-10	/* max on appletalk */
struct astats {
	long ints;		/* Interrupts */
	long any;		/* Good packets received */
	long drop;
	long badsize;
	long out;		/* Total output packets */
	long badtype;	/* Packets with unknown type fields */
	long nomem;		/* Dropped because buffer malloc failed */
};
struct at {
	ABByte ProtoType;	/* Protocol Type for AppleTalk */
	ABRecHandle rdATptr;	/* need to have a handle for the apple bus */
	ABRecHandle wrATptr;	/* need to have a handle for the apple bus */
	char *net;		/* Device name */
	char 	buffer[MAX_ATBUF];
	struct mbuf *rcvq;	/* Queue of incoming packets */
	unsigned rcvcnt;	/* Number of packets on queue */
	unsigned rcvmax;	/* Maximum length of rcvq */
	struct astats astats;	/* Controller statistics */
};
extern struct at at[];
