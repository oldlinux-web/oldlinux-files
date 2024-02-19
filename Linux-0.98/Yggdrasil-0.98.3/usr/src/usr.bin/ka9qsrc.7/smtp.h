#define SMTPTRACE			/* enable tracing for smtp */
#define MAXSESSIONS	10		/* most connections allowed */
#define JOBNAME		13		/* max size of a job name with null */
#define	LINELEN		128
#define SLINELEN	32
#define MBOXLEN		8		/* max size of a mail box name */


/* types of address used by smtp in an address list */
#define BADADDR	0
#define LOCAL	1
#define DOMAIN	2

/* a list entry */
struct list {
	struct list *next;
	char *val;
	char type;
};


/* Per-session control block  used by smtp server */
struct mail {
	struct tcb *tcb;	/* TCP control block pointer */
	char state;
#define	COMMAND_STATE	0
#define	DATA_STATE	1
	char *system;		/* Name of remote system */
	char *from;		/* sender address */
	struct list *to;	/* Linked list of recipients */
	char buf[LINELEN];	/* Input buffer */
	char cnt;		/* Length of input buffer */
	FILE *data;		/* Temporary input file pointer */
};

/* used by smtpcli as a queue entry for a single message */
struct smtp_job {
	struct 	smtp_job *next;	/* pointer to next mail job for this system */
	char	jobname[9];	/* the prefix of the job file name */
	char	*from;		/* address of sender */
	struct list *to;	/* Linked list of recipients */
};

/* control structure used by an smtp client session */
struct smtp_cb {
	struct tcb *tcb;	/* tcp task control buffer */
	int32	ipdest;		/* address of forwarding system */
	char	 state;		/* state machine placeholder */
#define CLI_INIT_STATE	0
#define CLI_OPEN_STATE	1
#define	CLI_HELO_STATE	2
#define CLI_MAIL_STATE	3
#define CLI_RCPT_STATE	4
#define	CLI_SEND_STATE	5
#define	CLI_UNLK_STATE	6
#define CLI_QUIT_STATE	7
#define CLI_IDLE_STATE	8
	char	*wname;		/* name of workfile */
	char	*tname;		/* name of data file */
	char	buf[LINELEN];	/* Input buffer */
	char	cnt;		/* Length of input buffer */
	FILE	*tfile;
	struct	smtp_job *jobq;
	char	goodrcpt;	/* are any of the rcpt ok */
	char	cts;		/* clear to send state indication */
	int	rcpts;		/* number of unacked rcpt commands */
	struct	list 	*errlog;	
};

/* smpt server routing mode */
#define	QUEUE	1

#define	NULLLIST	(struct list *)0
#define	NULLMAIL	(struct mail *)0
#define	NULLCB		(struct smtp_cb *)0
#define NULLJOB		(struct smtp_job *)0


extern char *mailspool;
extern char *mailqdir;		/* Outgoing spool directory */
extern char *routeqdir;	/* spool directory for a router program */
extern char *mailqueue;	/* Prototype of work file */
extern char *maillock;		/* Mail system lock */
extern char hostname[];
extern char *alias;
extern int32 mailroute();
extern int mlock(),rmlock(),queuejob();
extern char *ptime();
extern void del_list();
extern long get_msgid();
extern int16 smtpmode;
extern char *fgets();
extern struct list *addlist();
