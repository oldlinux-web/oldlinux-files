/* nr4.h:  defines for netrom layer 4 (transport) support.
 * Copyright 1989 by Daniel M. Frank, W9NK.  Permission granted for
 * non-commercial distribution only.
 */
 
/* compile-time limitations */

#define	NR4MAXCIRC	20		/* maximum number of open circuits */
#define NR4MAXWIN	127		/* maximum window size, send and receive */
							
/* protocol limitation: */

#define	NR4MAXINFO	236		/* maximum data in an info packet */

/* sequence number wraparound mask */

#define NR4SEQMASK	0xff	/* eight-bit sequence numbers */

/* flags in high nybble of opcode byte */

#define	NR4CHOKE	0x80
#define	NR4NAK		0x40
#define	NR4MORE		0x20	/* The "more follows" flag for */
							/* pointless packet reassembly */

/* mask for opcode nybble */

#define	NR4OPCODE	0x0f

/* opcodes */

#define NR4OPPID	0		/* protocol ID extension to network layer */
#define	NR4OPCONRQ	1		/* connect request */
#define	NR4OPCONAK	2		/* connect acknowledge */
#define	NR4OPDISRQ	3		/* disconnect request */
#define	NR4OPDISAK	4		/* disconnect acknowledge */
#define	NR4OPINFO	5		/* information packet */
#define	NR4OPACK	6		/* information ACK */
#define NR4NUMOPS	7		/* number of transport opcodes */

/* minimum length of NET/ROM transport header */

#define	NR4MINHDR	5

/* host format net/rom transport header */

struct nr4hdr {
	unsigned char opcode ;		/* opcode and flags */
	unsigned char yourindex ;	/* receipient's circuit index */
	unsigned char yourid ;		/* receipient's circuit ID */
	
	union {

		struct {				/* network extension */
			unsigned char family ;	/* protocol family */
			unsigned char proto ;	/* protocol within family */
		} pid ;

		struct {				/* connect request */
			unsigned char myindex ;	/* sender's circuit index */
			unsigned char myid ;	/* sender's circuit ID */
			unsigned char window ;	/* sender's proposed window size */
			struct ax25_addr user ;	/* callsign of originating user */
			struct ax25_addr node ;	/* callsign of originating node */
		} conreq ;

		struct {				/* connect acknowledge */
			unsigned char myindex ;	/* sender's circuit index */
			unsigned char myid ;	/* sender's circuit ID */
			unsigned char window ; 	/* accepted window size */
		} conack ;

		struct {				/* information */
			unsigned char txseq ;	/* sender's tx sequence number */
			unsigned char rxseq ;	/* sender's rx sequence number */
		} info ;

		struct {				/* information acknowledge */
			unsigned char rxseq ;	/* sender's rx sequence number */
		} ack ;

	} u ;	/* End of union */

} ;

/* The netrom circuit pointer structure */

struct nr4circp {
	unsigned char cid ;			/* circuit ID; incremented each time*/
								/* this circuit is used */
	struct nr4cb *ccb ;			/* pointer to circuit control block, */
								/*  NULLNR4CB if not in use */
} ;

/* The circuit table: */

extern struct nr4circp Nr4circuits[NR4MAXCIRC] ;

/* A netrom send buffer structure */

struct nr4txbuf {
	struct timer tretry ;		/* retry timer */
	unsigned retries ;			/* number of retries */
	struct mbuf *data ;			/* data sent but not acknowledged */
} ;

/* A netrom receive buffer structure */

struct nr4rxbuf {
	unsigned char occupied ;	/* flag: buffer in use */
	struct mbuf *data ; 		/* data received out of sequence */
} ;

/* The netrom circuit control block */

struct nr4cb {
	unsigned mynum ;			/* my circuit number */
	unsigned myid ;				/* my circuit ID */
	unsigned yournum ;			/* remote circuit number */
	unsigned yourid ;			/* remote circuit ID */
	struct ax25_addr user ;		/* callsign of originating user (if any) */
	struct ax25_addr node ;		/* callsign of remote node */
	struct ax25_addr luser ;	/* callsign of local "user", for connections */
								/* that we initiate */
	unsigned window ;			/* negotiated window size */

	/* Data for round trip timer calculation and setting */
	
	long srtt ;					/* Smoothed round trip time */
	long mdev ;					/* Mean deviation in round trip time */
	unsigned blevel ;			/* Backoff level */
	unsigned txmax ;			/* The maximum number of retries among */
								/* the frames in the window.  This is 0 */
								/* if there are no frames in the window. */
								/* It is used as a baseline to determine */
								/* when to increment the backoff level. */
								
	/* flags */
	
	char choked ;				/* choke received from remote */
	char qfull ;				/* receive queue is full, and we have */
								/* choked the other end */
	char naksent ;				/* a NAK has already been sent */

	/* transmit buffers and window variables */

	struct nr4txbuf *txbufs ;	/* pointer to array[windowsize] of bufs */
	unsigned char nextosend ;	/* sequence # of next frame to send */
	unsigned char ackxpected ;	/* sequence number of next expected ACK */
	unsigned nbuffered ;		/* number of buffered TX frames */
	struct mbuf *txq ;			/* queue of unsent data */
	
	/* receive buffers and window variables */

	struct nr4rxbuf *rxbufs ;	/* pointer to array[windowsize] of bufs */
	unsigned char rxpected ;	/* # of next receive frame expected */
	unsigned char rxpastwin ;	/* top of RX window + 1 */ 
	struct mbuf *rxq ;			/* "fully" received data queue */

	/* Connection state */
	
	int state ;					/* connection state */
#define NR4STDISC	0			/* disconnected */
#define NR4STCPEND	1			/* connection pending */
#define NR4STCON	2			/* connected */
#define	NR4STDPEND	3			/* disconnect requested locally */

	int dreason ;				/* Reason for disconnect */
#define NR4RNORMAL	0			/* Normal, requested disconnect */
#define NR4RREMOTE	1			/* Remote requested */
#define	NR4RTIMEOUT	2			/* Connection timed out */
#define	NR4RRESET	3			/* Connection reset locally */
#define NR4RREFUSED	4			/* Connect request refused */

	/* Per-connection timers */
	
	struct timer tchoke ;		/* choke timeout */
	struct timer tack ;			/* ack delay timer */

	struct timer tcd ;			/* connect/disconnect timer */
	unsigned cdtries ;			/* Number of connect/disconnect tries */
	
	void (*r_upcall)() ;		/* receive upcall */
	void (*t_upcall)() ;		/* transmit upcall */
	void (*s_upcall)() ;		/* state change upcall */

	char *puser ;				/* user pointer */
} ;

#define	NULLNR4CB	(struct nr4cb *)0

/* Some globals */

extern unsigned Nr4window ;		/* The advertised window size, in frames */
extern long Nr4irtt ;			/* The initial round trip time */
extern unsigned Nr4retries ;	/* The number of times to retry */
extern long Nr4acktime ;		/* How long to wait until ACK'ing */
extern char *Nr4states[] ;		/* NET/ROM state names */
extern char *Nr4reasons[] ;		/* Disconnect reason names */
extern unsigned Nr4qlimit ;		/* max receive queue length before CHOKE */
extern long Nr4choketime ;		/* CHOKEd state timeout */

#if	UNIX || ATARI_ST
/* function definitions */

/* In nr4subr.c: */

extern int ntohnr4() ;
extern struct mbuf *htonnr4() ;
extern struct nr4cb *new_n4circ() ;
extern int init_nr4window() ;
extern void free_n4circ() ;
extern struct nr4cb *match_n4circ() ;
extern struct nr4cb *get_n4circ() ;
extern int nr4between() ;
extern void nr4defaults() ;
extern int nr4valcb() ;

/* In nr4.c: */

extern void nr4sframe() ;
extern void nr4rframe() ;
extern void nr4sbuf() ;
extern void nr4ackours() ;
extern void nr4gotnak() ;
extern int nr4output() ;
extern void nr4state() ;
extern void nr4choke() ;

/* In nr4timer.c */

extern void nr4txtimeout() ;
extern void nr4ackit() ;
extern void nr4cdtimeout() ;
extern void nr4unchoke() ;

/* In nr4user.c: */

extern struct nr4cb *open_nr4() ;
extern int send_nr4() ;
extern struct mbuf *recv_nr4() ;
extern void disc_nr4() ;
extern void reset_nr4() ;
extern int kick_nr4() ;

/* In nrcmd.c: */

extern void nr4_state() ;
extern void nr4_parse() ;
extern void nr4_incom() ;
extern void nr4_rx() ;
extern void nr4_tx() ;

#else

/* function definitions */

/* In nr4subr.c: */

extern int ntohnr4(struct nr4hdr *, struct mbuf **) ;
extern struct mbuf *htonnr4(struct nr4hdr *) ;
extern struct nr4cb *new_n4circ() ;
extern int init_nr4window(struct nr4cb *, unsigned) ;
extern void free_n4circ(struct nr4cb *) ;
extern struct nr4cb *match_n4circ(int, int,
								  struct ax25_addr *, struct ax25_addr *) ;
extern struct nr4cb *get_n4circ(int, int) ;
extern int nr4between(unsigned, unsigned, unsigned) ;
extern void nr4defaults(struct nr4cb *) ;
extern int nr4valcb(struct nr4cb *) ;

/* In nr4.c: */

extern void nr4sframe(struct ax25_addr *, struct nr4hdr *, struct mbuf *) ;
extern void nr4rframe(struct nr4cb *, unsigned, struct mbuf *) ;
extern void nr4sbuf(struct nr4cb *, unsigned) ;
extern void nr4ackours(struct nr4cb *, unsigned, int) ;
extern void nr4gotnak(struct nr4cb *, unsigned) ;
extern int nr4output(struct nr4cb *) ;
extern void nr4state(struct nr4cb *, int) ;
extern void nr4choke(struct nr4cb *) ;

/* In nr4timer.c */

extern void nr4txtimeout(char *) ;
extern void nr4ackit(char *) ;
extern void nr4cdtimeout(char *) ;
extern void nr4unchoke(char *) ;

/* In nr4user.c: */

extern struct nr4cb *open_nr4(struct ax25_addr *, struct ax25_addr *,
							  void (*)(), void (*)(), void (*)(),
							  char *) ;
extern int send_nr4(struct nr4cb *, struct mbuf *) ;
extern struct mbuf *recv_nr4(struct nr4cb *, int16) ;
extern void disc_nr4(struct nr4cb *) ;
extern void reset_nr4(struct nr4cb *) ;
extern int kick_nr4(struct nr4cb *) ;

/* In nrcmd.c: */

extern void nr4_state(struct nr4cb *, int, int) ;
extern void nr4_parse(char *, int16) ;
extern void nr4_incom(struct nr4cb *, int, int) ;
extern void nr4_rx(struct nr4cb *, int16) ;
extern void nr4_tx(struct nr4cb *, int16) ;

#endif
