/* Asynch controller control block */
#define MAX_STORE 1024
struct asy {
	int speed;		/* Line speed */
	char *tty;
	char recv_buf[MAX_STORE];
};
extern struct asy asy[];
extern unsigned nasy;		/* Actual number of asynch lines */
#define	ASY_MAX	1		/* Two asynch ports on the PC */
