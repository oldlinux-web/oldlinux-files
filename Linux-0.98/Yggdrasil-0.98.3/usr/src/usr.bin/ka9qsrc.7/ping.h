/* ICMP Echo ("ping") definitions */
struct ping {
	struct ping *next;	/* Linked list pointers */
	struct ping *prev;
	int32 remote;		/* Address of remote host */
	struct timer timer;	/* Ping interval timer */
	int32 ttotal;		/* Total round trip time (for averaging) */
	int32 count;		/* Number of pings sent */
	int32 echoes;		/* Number of replies received */ 
	int16 burst;		/* Number of pings/burst */
};
#define	NULLPING (struct ping *)0
#define	PMOD	7
extern struct ping *ping[];

/* ID fields for pings; indicates a oneshot or repeat ping */
#define	ONESHOT	0
#define	REPEAT	1
