/* Upper sub-layer (LAPB) definitions */

/* Control field templates */
#define	I	0x00	/* Information frames */
#define	S	0x01	/* Supervisory frames */
#define	RR	0x01	/* Receiver ready */
#define	RNR	0x05	/* Receiver not ready */
#define	REJ	0x09	/* Reject */
#define	U	0x03	/* Unnumbered frames */
#define	SABM	0x2f	/* Set Asynchronous Balanced Mode */
#define	DISC	0x43	/* Disconnect */
#define	DM	0x0f	/* Disconnected mode */
#define	UA	0x63	/* Unnumbered acknowledge */
#define	FRMR	0x87	/* Frame reject */
#define	UI	0x03	/* Unnumbered information */
#define	PF	0x10	/* Poll/final bit */

#define	MMASK	7	/* Mask for modulo-8 sequence numbers */

/* FRMR reason bits */
#define	W	1	/* Invalid control field */
#define	X	2	/* Unallowed I-field */
#define	Y	4	/* Too-long I-field */
#define	Z	8	/* Invalid sequence number */

/* Per-connection link control block
 * These are created and destroyed dynamically,
 * and are indexed through a hash table.
 * One exists for each logical AX.25 Level 2 connection
 */
struct ax25_cb {
	struct ax25_cb *next;		/* Doubly linked list pointers */
	struct ax25_cb *prev;

	struct mbuf *txq;		/* Transmit queue */
	struct mbuf *rxasm;		/* Receive reassembly buffer */
	struct mbuf *rxq;		/* Receive queue */

	struct ax25 addr;		/* Address header */

	struct interface *interface;	/* Associated interface */

	char rejsent;			/* REJ frame has been sent */
	char remotebusy;		/* Remote sent RNR */
	char response;			/* Response owed to other end */

	char vs;			/* Our send state variable */
	char vr;			/* Our receive state variable */
	char unack;			/* Number of unacked frames */
	int maxframe;			/* Transmit flow control level */
	int16 paclen;			/* Maximum outbound packet size */
	int16 window;			/* Local flow control limit */
	char proto;			/* Protocol version */
#define	V1	1			/* AX.25 Version 1 */
#define	V2	2			/* AX.25 Version 2 */
	int16	pthresh;		/* Poll threshold */
	unsigned retries;		/* Retry counter */
	unsigned n2;			/* Retry limit */
	int state;			/* Link state */
#define	DISCONNECTED	0
#define	SETUP		1
#define	DISCPENDING	2
#define	CONNECTED	3
#define	RECOVERY	4
#define	FRAMEREJECT	5
	char frmrinfo[3];		/* I-field for FRMR message */
	struct timer t1;		/* Retry timer */
	struct timer t2;		/* Acknowledgement delay timer */
	struct timer t3;		/* Keep-alive poll timer */

	void (*r_upcall)();		/* Receiver upcall */
	void (*t_upcall)();		/* Transmit upcall */
	void (*s_upcall)();		/* State change upcall */
	char *user;			/* User pointer */
};
#define	NULLAX25	((struct ax25_cb *)0)
extern struct ax25_cb ax25default;
extern struct ax25_cb *ax25_cb[];
#define	NHASH	17

#define	YES	1
#define	NO	0
