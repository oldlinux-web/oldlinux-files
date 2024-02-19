/* Various I/O definitions specific to the Atari ST */

/* Iorec structure as defined by Atari BIOS */
struct iorec {
	char *ibuf;		/* pointer to buffer */
	int16 ibufsiz;		/* size of buffer */
	int16 ibufhd;		/* head index */
	int16 ibuftl;		/* tail index */
	int16 ibuflow;		/* low-water mark */
	int16 ibufhi;		/* high-water mark */
};

/* Asynch controller control block */
struct asy {
	struct iorec *in,*out;	/* ptr to current iorecs */
	struct iorec oldin,oldout; /* saved iorecs */
	unsigned addr;		/* Base I/O address, 1 or 3 on the Atari */
	unsigned vec;		/* Interrupt vector, used as route thru flag */
	unsigned speed; 	/* Line speed */
};

extern struct asy asy[];

#define RS232	1		/* address of RS232 */
#define MIDI	3		/* address of MIDI */

#define FLOW	1		/* flow control just like FLOW ON on a tnc-2 */
