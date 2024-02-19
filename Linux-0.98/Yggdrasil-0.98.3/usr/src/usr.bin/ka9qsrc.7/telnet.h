#define	LINESIZE	256	/* Length of local editing buffer */


/* Telnet command characters */
#define	IAC		255	/* Interpret as command */
#define	WILL		251
#define	WONT		252
#define	DO		253
#define	DONT		254
#define	TN_DM		242		/* data mark--for connect. cleaning */
#define	SB		250		/* interpret as subnegotiation */
#define	SE		240		/* end sub negotiation */


/* Telnet options */
#define	TN_TRANSMIT_BINARY	0
#define	TN_ECHO			1
#define	TN_SUPPRESS_GA		3
#define	TN_STATUS		5
#define	TN_TIMING_MARK		6
#define	TN_TTYPE		24
#define TN_LFLOW		33
#define	NOPTIONS		34

/* Telnet protocol control block */
struct telnet {
	struct tcb *tcb;
	char state;

#define	TS_DATA	0	/* Normal data state */
#define	TS_IAC	1	/* Received IAC */
#define	TS_WILL	2	/* Received IAC-WILL */
#define	TS_WONT	3	/* Received IAC-WONT */
#define	TS_DO	4	/* Received IAC-DO */
#define	TS_DONT	5	/* Received IAC-DONT */
#define	TS_SB	7		/* sub-option collection */
#define	TS_SE	8		/* looking for sub-option end */

#define SUBBUFSIZE 64

#define	SB_CLEAR()	tn->subpointer = tn->subbuffer;
#define	SB_TERM()	{ tn->subend = tn->subpointer; SB_CLEAR(); }
#define	SB_ACCUM(c)	if (tn->subpointer < (tn->subbuffer+SUBBUFSIZE)) { \
				*(tn->subpointer)++ = (c); \
			}
#define	SB_GET()	((*(tn->subpointer)++)&0xff)
#define	SB_PEEK()	((*(tn->subpointer))&0xff)
#define	SB_EOF()	(tn->subpointer >= tn->subend)
#define	SB_LEN()	(tn->subend - tn->subpointer)

	char outsup;		/* nonzero if output suppressed */
	char lflow;		/* 1 if flow control on */
	char local[NOPTIONS];	/* Local option settings */
	char remote[NOPTIONS];	/* Remote option settings */
	/* buffer for sub-options */
	unsigned char	subbuffer[SUBBUFSIZE], *subpointer, *subend;
#ifdef	UNIX
	struct	mbuf	*inbuf;		/* ptr to buffer holding excess rcvd chars */
	struct	mbuf	*outbuf;	/* ptr to buffer holding excess to-send chars */
	int	fd;			/* file descriptor of pty */
#endif
	struct session *session;	/* Pointer to session structure */
};
#define	NULLTN	(struct telnet *)0
extern int refuse_echo;
struct telnet *open_telnet();
int send_tel(),tel_input();
