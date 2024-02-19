/*************************************************/
/* Center for Information Technology Integration */
/*           The University of Michigan          */
/*                    Ann Arbor                  */
/*                                               */
/* Dedicated to the public domain.               */
/* Send questions to info@citi.umich.edu         */
/*                                               */
/* BOOTP is documented in RFC 951 and RFC 1048   */
/*************************************************/

#ifndef BOOTREQUEST


struct bootp {
	char	op;			/* packet opcode type */
	char	htype;			/* hardware addr type */
	char	hlen;			/* hardware addr length */
	char	hops;			/* gateway hops */
	int32	xid;			/* transaction ID */
	int16	secs;			/* seconds since boot began */
	int16	unused;
	int32	ciaddr;		/* client IP address */
	int32	yiaddr;		/* 'your' IP address */
	int32	siaddr;		/* server IP address */
	int32	giaddr;		/* gateway IP address */
	char	chaddr[16];		/* client hardware address */
	char	sname[64];		/* server host name */
	char	file[128];		/* boot file name */
	char	vend[64];		/* vendor-specific area */
};

/*
 * UDP port numbers, server and client.
 */
#define	IPPORT_BOOTPS		67
#define	IPPORT_BOOTPC		68

#define BOOTREQUEST		1
#define BOOTREPLY		2

#define BOOTP_PAD		0
#define BOOTP_SUBNET		1
#define BOOTP_GATEWAY		3
#define BOOTP_DNS		6
#define BOOTP_HOSTNAME		12
#define BOOTP_END		0xff

/*
 * "vendor" data permitted for Stanford boot clients.
 */
struct vend {
        unsigned char  v_magic[4];     /* magic number */
        unsigned long  v_flags;        /* flags/opcodes, etc. */
        unsigned char  v_unused[56];   /* currently unused */
};

#define VM_STANFORD     "STAN"  /* v_magic for Stanford */

/* v_flags values */
#define VF_PCBOOT       1       /* an IBMPC or Mac wants environment info */
#define VF_HELP         2       /* help me, I'm not registered */

extern int WantBootp;
extern char bp_ascii[];

#define __ARGS(x) x

void bootp_print_packet __ARGS((struct bootp *bp));
int bootp_validPacket __ARGS((struct ip *ip,struct mbuf **bpp));

#endif

