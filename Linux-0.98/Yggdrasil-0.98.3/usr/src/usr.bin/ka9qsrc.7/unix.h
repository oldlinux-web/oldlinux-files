/* Asynch controller control block */
struct asy {
	int speed;		/* Line speed */
	char *tty;
};
extern struct asy asy[];
extern unsigned nasy;		/* Actual number of asynch lines */
