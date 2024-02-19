/* SLFP definitions */
#define	SLFP_ALLOC	40	/* Receiver allocation increment */

typedef unsigned char BYTE ;
#define	SLFP_ESC	(BYTE)0362	/* Escape octet */
#define SLFP_REQ	(BYTE)0363	/* REQuest to Send octet */
#define	SLFP_ACK	(BYTE)0364	/* ACKnowledge a REQuest (clr to snd) */
#define	SLFP_END	(BYTE)0365	/* END of Packet octet */

#define ACK_LEN		1	/* Length of a SL/FP Acknowledge sequence */
#define REQ_LEN		1	/* Length of a SL/FP Request sequence */
#define HDR_LEN		4	/* Length of a SL/FP Packet Header */
#define SLFP_IP_HDR	0	/* slfp_hdr index for IP Header */
#define SLFP_ADDR_REQ	1	/* slfp_hdr index for "Addr Request" Header */

/* SL/FP protocol control structure */
struct slfp {
	struct mbuf *sndq;	/* Encapsulated packets awaiting transmission */
	struct timer ar_timer ;	/* Timer for Address Request */
	char ar_pending ;	/* =1 iff waiting for Addr Reply */
	struct timer req_timer;	/* Timer for receipt of ACK to our REQ */
	char req_pending;	/* State of wait for ACK of our REQ */
	int16 reqcnt;		/* Number of times current ACK has been REQ'd */
	int16 sndcnt;		/* Number of datagrams on queue */
	char escaped;		/* Receiver State control flag */
	char reqd;		/* =1 iff between REQ and END of rcv'g packet */
	struct mbuf *rbp;	/* Head of mbuf chain being filled */
	struct mbuf *rbp1;
	char *rcp;		/* Write pointer */
	int16 rcnt;		/* Length of mbuf chain */
	struct mbuf *tbp;	/* Transmit mbuf being sent */
	int16 errors;		/* Receiver input errors */
	int16 bad_esc;		/* Count of Bad SLFP_ESC sequences */
	int16 missed_ends;	/* Count of SLFP_REQ's rcv'd before prev END */
	int16 false_acks;	/* Count of SLFP_ACK's rcv'd sans pending xmt */
	int (*recv)();		/* Function to call with an incoming buffer */
};
extern struct slfp slfp[];
