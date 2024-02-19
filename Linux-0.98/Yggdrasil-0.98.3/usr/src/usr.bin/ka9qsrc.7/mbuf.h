/* Basic message buffer structure */
struct mbuf {
	struct mbuf *next;	/* Links mbufs belonging to single packets */
	struct mbuf *anext;	/* Links packets on queues */
	int16 size;		/* Size of associated data buffer */
	char *data;		/* Active working pointers */
	int16 cnt;
};
#define	NULLBUF	(struct mbuf *)0
#define	NULLBUFP (struct mbuf **)0
void enqueue(),hex_dump(),ascii_dump(),append();
struct mbuf *alloc_mbuf(),*free_mbuf(),*dequeue(),*copy_p(),*free_p(),*qdata(),
	*pushdown();
int16 pullup(),dup_p(),len_mbuf(),dqdata(),len_q();
int32 pull32();
int16 pull16();
char pullchar(),*put16(),*put32();
#define	AUDIT(bp)	audit(bp,__FILE__,__LINE__)

