/* Hardware-dependent routines for the EAGLE card for the PC
 * This card contains a Zilog 8530 only - no modem!
 */
#define EGMAX	1		/* One card max */
#define AX_MTU	512
#define INTMASK 0x21		/* Intel 8259 interrupt controller mask */

struct EGTAB {
	void (*oldvec)();	/* Original interrupt vector contents */
	int16 addr;		/* Base I/O address */
	unsigned vec;		/* Vector */
	long ints;		/* Interrupt count */
};
extern struct EGTAB eagle[];

/* Register offset info, specific to the EAGLE
 * E.g., to read the data port on channel A, use
 *	inportb(egchan[dev].base + CHANA + DATA)
 */
#define CHANB		0	/* Base of channel B regs */
#define CHANA		2	/* Base of channel A regs */

/* 8530 ports on each channel */
#define CTL	0
#define DATA	1
#define DMACTRL 	4	/* Base of channel + 4 */

/* EAGLE DMA/INTERRUPT CONTROL REGISTER */
#define DMAPORT 	0	/* 0 = Data Port */
#define INTPORT 	1	/* 1 = Interrupt Port */
#define DMACHANA	0	/* 0 = DMA on CHANNEL A */
#define DMACHANB	2	/* 1 = DMA on Channel B */
#define DMADISABLE	0	/* 0 = DMA disabled */
#define DMAENABLE	4	/* 1 = DMA enabled */
#define INTDISABLE	0	/* 0 = Interrupts disabled */
#define INTENABLE	8	/* 1 = Interrupts enabled */
#define INTACKTOG	10	/* 1 = INT ACK TOGGLE */


struct egchan {
	long rxints;		/* Receiver interrupts */
	long txints;		/* Transmitter interrupts */
	long exints;		/* External/status interrupts */
	long spints;		/* Special receiver interrupts */

	int enqueued;		/* Packets actually forwarded */
	int rxframes;		/* Number of Frames Actally Received */
	int toobig;		/* Giant receiver packets */
	int crcerr;		/* CRC Errors */
	int aborts;		/* Receiver aborts */
	int rovers;		/* Receiver Overruns */

	char status;		/* Copy of R0 at last external interrupt */
	struct mbuf *rcvq;	/* Receive queue */
	int16 rcvcnt;		/* Number of packets on rcvq */
	struct mbuf *rcvbuf;	/* Buffer for current rx packet */
	int16 bufsiz;		/* Size of rcvbuf */
	char *rcp;		/* Pointer into rcvbuf */

	struct mbuf *sndq;	/* Packets awaiting transmission */
	int16 sndcnt;		/* Number of packets on sndq */
	struct mbuf *sndbuf;	/* Current buffer being transmitted */
	char tstate;		/* Tranmsitter state */
#define IDLE	0		/* Transmitter off, no data pending */
#define ACTIVE	1		/* Transmitter on, sending data */
#define UNDERRUN 2		/* Transmitter on, flushing CRC */
#define FLAGOUT 3		/* CRC sent - attempt to start next frame */
#define DEFER 4 		/* Receive Active - DEFER Transmit */
	char rstate;		/* Set when !DCD goes to 0 (TRUE) */
/* Normal state is ACTIVE if Receive enabled */
#define RXERROR 2		/* Error -- Aborting current Frame */
#define RXABORT 3		/* ABORT sequence detected */
#define TOOBIG 4		/* too large a frame to store */
	int16 dev;		/* Device number */
	int16 base;		/* Base of I/O registers */
	int16 stata;		/* address of Channel A status regs */
	int16 statb;		/* address of Channel B status regs */
	int16 dmactrl;		/* address of DMA/INTERRUPT reg on card */
	int16 speed;		/* Line speed, bps */
#define TXDELAY 0		/* Transmit Delay 10 ms/cnt */
#define PERSIST 1		/* Persistence (0-255) as a % */
#define SLOTIME 2		/* Delay to wait on persistence hit */
#define SQUELDELAY 3		/* Delay after XMTR OFF for seuelch tail */
	char params[4]; 	/* Channel control parameters */
};
extern struct egchan egchan[];

/* Interrupt vector handlers (assembler) */
int pcint();

/* Other utility routines */
char read_sio();

#define OFF	0
#define ON	1
#define INIT	2

/* 8530 clock speed */

#define XTAL	((long) 3686400/2)	 /* 32X clock constant */

/*************************************************************/
/* TEMP FOR DEBUG ONLY - eliminates Divide by zero interrupt */
/*		       - preset for 1200 BAUD !!!!!!!!!!!!!! */
/*************************************************************/
#define TXCONST 1534			 /* (XTAL/1200L)-2 */
#define RXCONST 46			 /* ((XTAL/32)/1200L)-2 */

/* Baud rate generator definitions */
struct baudrate {
	int16 speed;
	char val;
};
