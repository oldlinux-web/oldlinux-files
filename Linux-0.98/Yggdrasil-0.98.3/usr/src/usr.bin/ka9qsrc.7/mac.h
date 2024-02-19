/* Asynch controller control block */
#define MAX_STORE 1024

struct Store_input {
	char *head;
	char store[MAX_STORE];
	char *tail;
	int   amt;
};

struct asy {
	unsigned int speed;		/* Line speed */
	int	portIn;
	int portOut;
	int	devopen;
	char *tty;
	char recv_buf[MAX_STORE];
	char snd_buf[MAX_STORE];
};
extern struct asy asy[];
extern unsigned nasy;		/* Actual number of asynch lines */
#define	ASY_MAX	2		/* Two asynch ports on the MAC */
