/* see comment at the head of ax_mbx.c - Bdale */
#ifdef XOBBS
/* Defines for the ax.25 mailbox facility */

#define true      1
#define false     0

/*message que keys*/
#define SENDKEY 200
#define RCVKEY 201
#define SCTLKEY 202
#define RCTLKEY 203

/*defines that control forwarder states */

#define NOFWD     0
#define SPAWNBBS  1
#define DOCONNECT 2
#define READXMIT  3
#define GETCON    4
#define CLEARCON  5
#define WAITING   6

#define NOCON     0    
#define YESCON    1
#define WAIT      2

struct message{
	long mtype;
	char mtext[1024];
};

/* defines for protocol type in mailbox session structure*/
#define AX25	  1
#define NTRM	  2

struct mboxsess{		/*mailbox session structure*/
	int proto;		/*is this a net/rom or ax25 session?*/
	int pid;		/*id of bbs process*/
	int bytes;		/*number of bytes for bbs to send at one time*/
	int gotbytes;		/*number of bytes we have in buffer*/
	union cbaddr {
	    struct ax25_cb *axbbscb; /*axp associated with this structure*/
	    struct nr4cb *axnrcb;   /* cb associated with the structure*/
	} cbadr;
	struct message rmsgbuf;  /*receive message buffer for this structure*/
	char call[7];		/*call of user*/
	char *mtxtptr;		/*buffer pointer for this structure*/
	struct mboxsess *next;	/*pointer to next session*/
};

#define NULLMBS  (struct mboxsess *)0
#define NULLFWD  (struct ax25_cb *)0
#define FWDTMP	"/tmp/fwdtmp"
extern int ax25mbox ;

#else /* not XOBBS */

/* Defines for the ax.25 mailbox facility */

#define NUMMBX		10		/* max number of mailbox sessions */

#define MBXLINE		128		/* max length of line */
struct mbx {
	int state ;				/* mailbox state */
#define MBX_CMD		1		/* in command mode */
#define MBX_SUBJ	2		/* waiting for a subject line */
#define MBX_DATA	3		/* collecting the message */
	int type ;				/* type of connection */
#define MBX_AX25	1		/* ax25 connection */
#define MBX_NETROM	2		/* net/rom transport connection */
	char name[10] ;			/* Name of remote station */
	union {
		struct ax25_cb	*ax25_cb ;	/* ax.25 link control block */
		struct nr4cb *nr4_cb ;		/* net/rom link control block */
	} cb ;
	char *to ;				/* To-address in form user or user@host */
	char *tofrom ;			/* Optional <from in to-address */
	char *tomsgid ;			/* Optional $msgid in to-address */
	FILE *tfile ;			/* Temporary file for message */
	char line[MBXLINE+1] ;	/* Room for null at end */
	char *lp ;				/* line pointer */
	int mbnum ;				/* which mailbox session is this? */
	int sid ;				/* Characteristics indicated by the SID */
							/* banner of the attaching station.  If */
							/* no SID was sent, this is zero.  If an */
							/* SID of any kind was received, it is */
							/* assumed that the station supports */
							/* abbreviated mail forwarding mode. */
#define	MBX_SID		0x01	/* Got any SID */
#define	MBX_SID_RLI	0x02	/* This is an RLI BBS, disconnect after F> */
							/* Space here for others, currently not of */
							/* interest to us. */
	char stype ;			/* BBS send command type (B,P,T, etc.) */
} ;
#define 	NULLMBX		(struct mbx *)0

extern struct mbx *mbox[NUMMBX] ;
extern int ax25mbox ;
#endif /* XOBBS */
