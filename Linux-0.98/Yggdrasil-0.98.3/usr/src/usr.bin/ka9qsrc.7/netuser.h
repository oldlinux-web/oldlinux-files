/* Global structures and constants needed by an Internet user process */
extern char *hosts; 	/* Host table */

#define	NCONN	20		/* Maximum number of open network connections */

extern int32 ip_addr;	/* Our IP address */

extern int net_error;	/* Error return code */
#define	NONE	0		/* No error */
#define	CON_EXISTS	1	/* Connection already exists */
#define	NO_CONN	2		/* Connection does not exist */
#define	CON_CLOS	3	/* Connection closing */
#define	NO_SPACE	4	/* No memory for TCB creation */
#define	WOULDBLK	5	/* Would block */
#define	NOPROTO		6	/* Protocol or mode not supported */
#define	INVALID		7	/* Invalid arguments */

/* Codes for the tcp_open call */
#define	TCP_PASSIVE	0
#define	TCP_ACTIVE	1
#define	TCP_SERVER	2	/* Passive, clone on opening */

/* Socket structure */
struct socket {
	int32 address;		/* IP address */
	int16 port;			/* port number */
};

/* Connection structure (two sockets) */
struct connection {
	struct socket local;
	struct socket remote;
};
#define	NULLSOCK	(struct socket *)0

/* TCP port numbers */
#define	ECHO_PORT	7	/* Echo data port */
#define	DISCARD_PORT	9	/* Discard data port */
#define	FTPD_PORT	20	/* FTP Data port */
#define	FTP_PORT	21	/* FTP Control port */
#define TELNET_PORT	23	/* Telnet port */
#define SMTP_PORT	25	/* Mail port */
#define FINGER_PORT     79      /* Finger port */

int32 aton(),resolve();
char *inet_ntoa(),*psocket();
long htol();

#ifdef	STDIO_H
FILE	*myopen();
#define	fopen	myopen
#endif
