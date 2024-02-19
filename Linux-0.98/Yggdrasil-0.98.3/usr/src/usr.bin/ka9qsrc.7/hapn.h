/*  HAPN-1 PC plug-in card driver.
 *  This card contains an Intel 8273 SDLC/HDLC Protocol Controller
 *  The card is hardwired to addresses 310-31f and IRQ 2
 */

#define NHAPN 1
#define INTMASK 0x21		/*  PC interrupt controller (8259) */

struct hapn {
	long rxints;            /* RX interupt count                  */
	long txints;            /* TX interrupt count                 */
				/* Error counters                     */
	int badint;             /* Bad interrupt type                 */
	int crcerr;             /* CRC errors                         */
	int aborts;             /* RX frame aborts                    */
	int dmaorun;            /* DMA overun                         */
	int toobig;             /* RX frame too large                 */
	int rframes;            /* # of RX frames                     */
	int cdloss;             /* Loss of DCD during receive         */
	int rxorun;             /* Receive interrupt overun           */
	int nomem;              /* insufficient memory                */
	int t_urun;             /* TX underruns                       */
	int ctsloss;            /* Loss of CTS (dead-man timeout)     */
	int taborts;            /* TX aborts                          */
	int tframes;            /* # of TX frames                     */

	struct mbuf *rcvq;      /* Received frame queue               */
	int16 rcvcnt;           /* Count of frames on rcvq            */
	struct mbuf *rcvbuf;    /* Current receive buffer             */
	int16 bufsiz;           /* Maximum RX frame size              */
	char *rcp;		/* RX data pointer                    */

	struct mbuf *sndq;      /* Transmit frames queue              */
	int16 sndcnt;           /* Count of frames on sndq            */
	struct mbuf *sndbuf;    /* Current TX frame buffer            */
	int tstate;             /* Transmitter state                  */
#define IDLE 0
#define DEFER 1
#define ACTIVE 2
	int mode;               /* Channel-access mode                */
#define CSMA 0
#define FULLDUP 1
	int16 base;             /* Base I/O address of board          */
	unsigned vec;           /* Interrupt level                    */
	void (*oldvec)();	/* Previous interrupt service vector */
};

extern struct hapn hapn[];

/*  Interrupt vector handler  */

void haint();

/*  8273 register addresses  */

#define CMD 0
#define STA 0
#define PAR 1
#define RES 1
#define RST 1
#define TXI 2
#define RXI 3
#define TXD 4
#define RXD 8

/*  8273 commands  */

#define	SET_ONE		0xa4
#define	RST_ONE		0x64
#define	SET_XFER	0x97
#define	RST_XFER	0x57
#define	SET_MODE	0x91
#define	RST_MODE	0x51
#define	HDLC		0x20
#define	EOP		0x10
#define	EARLY		0x8
#define	BUFFERD		4
#define	PREFRM		2
#define	FLG_STM		1
#define	SET_SERIAL	0xa0
#define	RST_SERIAL	0x60
#define	LOOP		4
#define	TXC_RXC		2
#define	NRZI		1
#define GENERAL_RX	0xc0
#define SELECT_RX	0xc1
#define SELECT_LRX	0xc2
#define RX_DISABLE	0xc5
#define TX_FRAME	0xc8
#define LOOP_TX		0xca
#define TX_TRANS	0xc9
#define ABORT_TXF	0xcc
#define ABORT_LTX	0xce
#define ABORT_TXT	0xcd
#define READ_A		0x22
#define	CD		2
#define	CTS		1
#define READ_B		0x23
#define SET_B		0xa3
#define RST_B		0x63
#define FLAG_D		0x20
#define IRQ_ENB		8
#define RTS		1

/*  Status register bits  */

#define CBSY	0x80
#define CBF	0x40
#define CPBF	0x20
#define CRBF	0x10
#define RXINT	8
#define TXINT	4
#define RXIRA	2
#define TXIRA	1

/*  Transmit result codes  */

#define EARLY_TXI	0xc
#define TX_CMPLT	0xd
#define DMA_URUN	0xe
#define CTS_LOSS	0xf
#define ABORT_CMPLT	0x10

/*  Receive result codes  */

#define A1_MATCH	0
#define A2_MATCH	1
#define CRCERR		3
#define ABORT_DET	4
#define IDLE_DET	5
#define EOP_DET		6
#define SHORT_FRM	7
#define DMA_OVRN	8
#define MEM_OVFL	9
#define CD_LOSS		0xa
#define RX_ORUN		0xb
