/* SLIP definitions */
#define	SLIP_ALLOC	40	/* Receiver allocation increment */

#define	FR_END		0300	/* Frame End */
#define	FR_ESC		0333	/* Frame Escape */
#define	T_FR_END	0334	/* Transposed frame end */
#define	T_FR_ESC	0335	/* Transposed frame escape */


/* Slip protocol control structure */
struct slip {
	struct mbuf *sndq;	/* Encapsulated packets awaiting transmission */
	int16 sndcnt;		/* Number of datagrams on queue */
	char escaped;		/* Receiver State control flag */
	char vjcomp;		/* compression on */
	struct mbuf *rbp;	/* Head of mbuf chain being filled */
	struct mbuf *rbp1;	/* Pointer to mbuf currently being written */
	char *rcp;		/* Write pointer */
	int16 rcnt;		/* Length of mbuf chain */
	struct mbuf *tbp;	/* Transmit mbuf being sent */
	char *slcomp;	/* TCP header compression table */
	int16 errors;		/* Receiver input errors */
	void (*recv)();		/* Function to call with an incoming buffer */
};
extern struct slip slip[];

