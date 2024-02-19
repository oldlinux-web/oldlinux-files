/* Various I/O definitions specific to asynch I/O on the IBM PC */

extern unsigned nasy;		/* Actual number of asynch lines */
extern unsigned nhandlers ;	/* Number of interrupt handlers used for them */

/* Output pseudo-dma control structure */
struct dma {
	char *data;	/* current output pointer */
	unsigned short cnt;	/* byte count remaining */
	char last_octet;	/* previously output octet */
	char flags;	/* transmitter active */
};

/* Read fifo control structure */
struct fifo {
	char *buf;		/* Ring buffer */
	unsigned bufsize;	/* Size of ring buffer */
	char *wp;		/* Write pointer */
	char *rp;		/* Read pointer */
	unsigned short cnt;	/* count of characters in buffer */
};

/* Asynch controller control block */
struct asy {
	struct fifo fifo;
	struct dma dma;
	unsigned addr;		/* Base I/O address */
	unsigned vec;		/* Interrupt vector */
	unsigned speed;		/* Line speed */
	struct asy *ichain ;/* Next asy in interrupt vector chain */
	unsigned (*urgent)();	/* Handler for urgent transmission */
	struct {		/* Previous configuration saved at startup */
		char divh,divl;	/* baud rate divisor */
		char lcr;	/* line control reg */
		char ier;	/* Interrupt enable register */
		char mcr;	/* modem control bits */
	} save;
};
extern struct asy asy[];

#define NULLASY	(struct asy *)0

/* Interrupt vector structure */
struct ivec {
	void (*oldvec)() ;		/* Original interrupt vector [cs:pc] */
	char oldmask;			/* Original 8259 mask */
	unsigned refcnt ;		/* How many asy's are using this int? */
	struct asy *ichain ;	/* First asy in chain for this int */
} ;

#define NIVECS	16

extern struct ivec ivec[] ;

extern unsigned h2ivec[] ;	/* map from handlers to vectors */

#define	BAUDCLK	115200L		/* 1.8432 Mhz / 16 */

/* 8250 definitions */
/* Control/status register offsets from base address */
#define	THR	0		/* Transmitter holding register */
#define	RBR	0		/* Receiver buffer register */
#define	DLL	0		/* Divisor latch LSB */
#define	DLM	1		/* Divisor latch MSB */
#define	IER	1		/* Interrupt enable register */
#define	IIR	2		/* Interrupt ident register */
#define	LCR	3		/* Line control register */
#define	MCR	4		/* Modem control register */
#define	LSR	5		/* Line status register */
#define	MSR	6		/* Modem status register */

/* 8250 Line Control Register */
#define	LCR_5BITS	0	/* 5 bit words */
#define	LCR_6BITS	1	/* 6 bit words */
#define	LCR_7BITS	2	/* 7 bit words */
#define	LCR_8BITS	3	/* 8 bit words */
#define	LCR_NSB		4	/* Number of stop bits */
#define	LCR_PEN		8	/* Parity enable */
#define	LCR_EPS		0x10	/* Even parity select */
#define	LCR_SP		0x20	/* Stick parity */
#define	LCR_SB		0x40	/* Set break */
#define	LCR_DLAB	0x80	/* Divisor Latch Access Bit */

/* 8250 Line Status Register */
#define	LSR_DR	1	/* Data ready */
#define	LSR_OE	2	/* Overrun error */
#define	LSR_PE	4	/* Parity error */
#define	LSR_FE	8	/* Framing error */
#define	LSR_BI	0x10	/* Break interrupt */
#define	LSR_THRE 0x20	/* Transmitter line holding register empty */
#define	LSR_TSRE 0x40	/* Transmitter shift register empty */

/* 8250 Interrupt Identification Register */
#define	IIR_IP		1	/* 0 if interrupt pending */
#define	IIR_ID		6	/* Mask for interrupt ID */
#define	IIR_RLS		6	/* Receiver Line Status interrupt */
#define	IIR_RDA		4	/* Receiver data available interrupt */
#define	IIR_THRE	2	/* Transmitter holding register empty int */
#define	IIR_MSTAT	0	/* Modem status interrupt */

/* 8250 interrupt enable register bits */
#define	IER_DAV	1	/* Data available interrupt */
#define	IER_TxE	2	/* Tx buffer empty interrupt */
#define	IER_RLS	4	/* Receive line status interrupt */
#define	IER_MS	8	/* Modem status interrupt */

/* 8250 Modem control register */
#define	MCR_DTR	1	/* Data Terminal Ready */
#define	MCR_RTS	2	/* Request to Send */
#define	MCR_OUT1 4	/* Out 1 (not used) */
#define	MCR_OUT2 8	/* Master interrupt enable (actually OUT 2) */
#define	MCR_LOOP 0x10	/* Loopback test mode */

/* 8250 Modem Status Register */
#define	MSR_DCTS 1	/* Delta Clear-to-Send */
#define	MSR_DDSR 2	/* Delta Data Set Ready */
#define	MSR_TERI 4	/* Trailing edge ring indicator */
#define	MSR_DRLSD 8	/* Delta Rx Line Signal Detect */
#define	MSR_CTS	0x10	/* Clear to send */
#define	MSR_DSR 0x20	/* Data set ready */
#define	MSR_RI	0x40	/* Ring indicator */
#define	MSR_RLSD 0x80	/* Received line signal detect */

