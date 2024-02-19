/* Global structures and constants pertaining to the interface between IP and
 * 	higher level protocols
 */

/* IP protocol field values */
#define	ICMP_PTCL	1	/* Internet Control Message Protocol */
#define	TCP_PTCL	6	/* Transmission Control Protocol */
#define	UDP_PTCL	17	/* User Datagram Protocol */

#define	MAXTTL		255	/* Maximum possible IP time-to-live value */

/* DoD-style precedences */
#define	ROUTINE		0x00
#define	PRIORITY	0x20
#define	IMMEDIATE	0x40
#define	FLASH		0x60
#define	FLASH_OVER	0x80
#define	CRITIC		0xa0
#define	INET_CTL	0xc0
#define	NET_CTL		0xe0

/* Amateur-style precedences */
#define	AM_ROUTINE	0x00
#define	AM_WELFARE	0x20
#define	AM_PRIORITY	0x40
#define	AM_EMERGENCY	0x60

/* Class-of-service bits */
#define	LOW_DELAY	0x10
#define	THROUGHPUT	0x08
#define	RELIABILITY	0x04

/* IP TOS fields */
#define	PREC(x)		((x)>>5 & 7)
#define	DELAY		0x10
#define	THRUPUT		0x8
#define	RELIABLITY	0x4

/* Pseudo-header for TCP and UDP checksumming */
struct pseudo_header {
	int32 source;		/* IP source */
	int32 dest;		/* IP destination */
	char protocol;		/* Protocol */
	int16 length;		/* Data field length */
};
#define	NULLHEADER	(struct pseudo_header *)0

void tcp_input(),udp_input(),dump_ip();
int16 cksum();
int ip_send();
