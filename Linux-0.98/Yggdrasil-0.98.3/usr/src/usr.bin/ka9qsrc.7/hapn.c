/*  Driver for HAPN-1 8273 card
 *  Jon Bloom, KE3Z; adapted from KA9Q's PC-100 driver
 *  Modified Rx interrupt routine to prevent lockup
 *  John Tanner VK2ZXQ 6th Feb 1988
 *  Adapted back into 871225.9 by KA9Q 15 Feb 1988
 */
#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "hapn.h"
#include "ax25.h"
#include "trace.h"

struct hapn hapn[NHAPN];
void ha0vec();
void (*h_handle[])() = { ha0vec };
int16 nhapn;

/*  send command to the 8273
 *  "base" = base port of 8273
 *  "cmd"  = command byte
 *  "np"   = number of parameter bytes
 *  "p1"   = first parameter (parameters are int)
 */
/*VARARGS3*/
static
cmd_8273(base, cmd, np, p1)
int16 base;
int cmd, np, p1;
{
	int *p;

	while(inportb(base+STA) & CBSY)
		;
	outportb(base+CMD, cmd);
	p = &p1;
	while(np--){
		while(inportb(base+STA) & CPBF)
			;
		outportb(base+PAR, *p++);
	}
}

/*  Start receiver of 8273 */
static
hrxgo(hp)
register struct hapn *hp;
{
	cmd_8273(hp->base, GENERAL_RX, 2, hp->bufsiz & 0xff, hp->bufsiz >> 8);
}

/*  Interrupt service function.  Entered with hapn index
 *  The "flag" variable is used in this routine to indicate a
 *  valid TX or RX interrupt. If an invalid interrupt is detected
 *  the 8273 is reset.
 */
void
haint(dev)
int dev;
{
	register struct hapn *hp;
	register int16 base;
	char flag = 0;
	void htxint(),hrxint();

	hp = &hapn[dev];
	base = hp->base;

	/*  Check for TX interrupt  */
	if(inportb(base+STA) & TXINT){
		flag = 1;	/* Valid interrupt, set flag */
		htxint(hp);
	}
	/*  Check for RX interrupt  */
	if(inportb(base+STA) & RXINT){
		flag = 1;	/* Valid interrupt, set flag */
		hrxint(hp);
	}
	/* Check for unknown interrupt  */
	if(!flag){
		hp->badint++;	/* Increment error counter */
		hapn_init(hp);	/* Reinitialise the 8273 */
	}
}
/*  RX interrupt service
 *  if status register bit "RXIRA" is set, interrupt is final,
 *  otherwise, interrupt is data request
 */
static void
hrxint(hp)
register struct hapn *hp;
{
	register struct mbuf *bp;
	register int16 base;
	unsigned char results[10];

	hp->rxints++;
	base = hp->base;

	if(inportb(base+STA) & RXIRA){
		/* RX result interrupt
		 * If the result is a good frame 3 bytes need to be read
		 * If an error has occurred only one byte need to be read
		 */

		/* Read first result byte and test for good data */
		if((results[0]=(inportb(base + RXI))) == 0xe0){
			/* Good result; read two more result bytes */
			while((inportb(base + STA) & RXIRA) == 0)
				;
			/* Read second result byte */
			results[1] = inportb(base + RXI);
			/* Wait for third result byte  */
			while((inportb(base + STA) & RXIRA) == 0)
				;  
			results[2] = inportb(base + RXI);/* Read it */

			/* Since this frame is ok put it on the queue */
			enqueue(&hp->rcvq, hp->rcvbuf);
			hp->rcvbuf = NULLBUF;
			hp->rcvcnt++;
			hp->rframes++;
		} else {
			/* Error termination
			 * Parse RIC and act accordingly
			 * Only one result byte returned on error
			 */
			switch(results[0]){
			case CRCERR:
				hp->crcerr++;
				break;
			case ABORT_DET:
				hp->aborts++;
				break;
			case DMA_OVRN:
				hp->dmaorun++;
				break;
			case MEM_OVFL:
				hp->toobig++;
				break;
			case CD_LOSS:
				hp->cdloss++;
				hapn_init(hp);	/* 8273 reset on cd error */
				break;
			case RX_ORUN:
				hp->rxorun++;
				break;
			}
			/* Throw rx buffer contents away to start over */
			hp->rcp = hp->rcvbuf->data;
			hp->rcvbuf->cnt = 0;
		}
		/* Restart the receiver */
		cmd_8273(base,RX_DISABLE,0);
		hrxgo(hp);
	} else {
		/* RX data interrupt; allocate new rx buffer if none present */
		if((bp = hp->rcvbuf) == NULLBUF){
			bp = hp->rcvbuf = alloc_mbuf(hp->bufsiz);
			if(bp == NULLBUF){
				/* No memory available */
				hp->nomem++;
				cmd_8273(base, RX_DISABLE, 0);
				hrxgo(hp);
				return;
			}
			/* Init buffer pointer */
			hp->rcp = hp->rcvbuf->data;
		}
		/*  Barf if rx data is more than buffer can hold (should never
		 *  happen since 8273 is also counting bytes).
		 */
		if(bp->cnt++ >= hp->bufsiz){
			hp->toobig++;
			cmd_8273(base, RX_DISABLE, 0);
			hrxgo(hp);
			free_p(bp);
			hp->rcvbuf = NULLBUF;
			return;
		}
		/* Store the received byte */
		*hp->rcp++ = inportb(base+RXD);
	}
}

/*  test for busy channel (CD active)
 *  returns TRUE if channel busy
 */
static int
hcdchk(base)
int16 base;
{
	char isav;

	isav = disable();
	cmd_8273(base, READ_A, 0);
	while(!(inportb(base+STA) & CRBF))
		;
	restore(isav);
	return((inportb(base+RES) & CD) != 0);
}

/*  TX interrupt service
 *  if status register bit "TXIRA" is set, interrupt is final,
 *  otherwise, interrupt is data request
 */
static void
htxint(hp)
register struct hapn *hp;
{
	char isav;
	register int16 base;
	int16 len;
	char c;

	isav = disable();
	hp->txints++;
	base = hp->base;

	c = 0;
	if(inportb(base+STA) & TXIRA){		/* TX result interupt */
		hp->tstate = IDLE;
		free_p(hp->sndbuf);
		hp->sndbuf = NULLBUF;

		/*  Read result  */
		while((inportb(base+STA) & (TXINT | TXIRA)) != (TXINT | TXIRA))
			;
		c = inportb(base+TXI);

		/*  Test for tx abort  */
		switch(c & 0x1f){
		case DMA_URUN:
			hp->t_urun++;
			break;
		case CTS_LOSS:
			hp->ctsloss++;
			break;
		case ABORT_CMPLT:
			hp->taborts++;
			break;
		}
	}
	switch(hp->tstate){
	case IDLE:	/*  See if a buffer is ready to be sent  */
		if((hp->sndbuf = dequeue(&hp->sndq)) == NULLBUF)
			break;

	case DEFER:	/*  Busy-channel check  */
		if(hp->mode == CSMA && (c & 0x1f) != EARLY_TXI){
			if(hcdchk(base)){
				hp->tstate = DEFER;
				break;
			}
		}
		/*  Start transmitter  */
		len = len_mbuf(hp->sndbuf);
		cmd_8273(base, TX_FRAME, 2, len & 0xff, len >> 8);
		hp->tstate = ACTIVE;
		hp->tframes++;
		break;
	case ACTIVE:	/*  Get next byte to send  */
		if(pullup(&hp->sndbuf, &c, 1) != 1){
			cmd_8273(base, ABORT_TXF, 0);
			hp->tstate = IDLE;
		} else
			outportb(base+TXD, c);
		break;
	}
	restore(isav);
}

/*  Attach a HAPN adaptor to the system
 *  argv[0]:  hardware type, must be "hapn"
 *  argv[1]:  I/O address, e.g. "0x310"
 *  argv[2]:  vector, e.g. "2"
 *  argv[3]:  mode, must be "ax25"
 *  argv[4]:  interface name, e.g. "ha0"
 *  argv[5]:  rx packet buffer size in bytes
 *  argv[6]:  maximum transmission unit in bytes
 *  argv[7]:  channel-access mechanism, "csma" or "full"
 */
int
hapn_attach(argc, argv)
int argc;
char *argv[];
{
	register struct interface *if_h;
	extern struct interface *ifaces;
	struct hapn *hp;
	int dev, i;
	char isav;
	int hapn_init(), hapn_stop(), ax_send(), ax_output(),
		hapn_raw();
	void dohapn();
	void (*getirq())();	/* Getirq is a function returning a pointer to
				 * a function returning void */
	static struct {
		char *str;
		char type;
	} ch_access [] = { "csma", 0, "full", 1 };

	if(nhapn >= NHAPN){
		printf("Too many HAPN adaptors\n");
		return -1;
	}
	dev = nhapn++;

	/*  Initialize hardware constants */
	hapn[dev].base = htoi(argv[1]);
	hapn[dev].vec = htoi(argv[2]);

	/*  Save original interrupt vector  */
	hapn[dev].oldvec = getirq(hapn[dev].vec);

	/*  Set new interrupt vector  */
	setirq(hapn[dev].vec, h_handle[dev]);

	/*  Create new interface structure  */
	if_h = (struct interface *) calloc(1,sizeof(struct interface));

	/*  Fill interface structure  */
	if_h->name = malloc((unsigned) strlen(argv[4]) +1);
	strcpy(if_h->name, argv[4]);
	if_h->mtu = atoi(argv[6]);
	if_h->dev = dev;
	if_h->recv = dohapn;
	if_h->stop = hapn_stop;
	if_h->output = ax_output;
	if_h->raw = hapn_raw;

	if(strcmp(argv[3], "ax25")){
		printf("Mode %s unknown for interface %s\n", argv[3], argv[4]);
		free(if_h->name);
		free((char *) if_h);
		return -1;
	}
	axarp();
	if(mycall.call[0] == '\0'){
		printf("set mycall first\n");
		free(if_h->name);
		free((char *) if_h);
		return -1;
	}		
	if_h->send = ax_send;
	if(if_h->hwaddr == NULLCHAR)
		if_h->hwaddr = malloc(sizeof(mycall));
	memcpy(if_h->hwaddr,(char *)&mycall,sizeof(mycall));
	/*  Link the interface into the interface list  */
	if_h->next = ifaces;
	ifaces = if_h;

	/*  Fill the local data structure  */
	hp = &hapn[dev];
	hp->bufsiz = atoi(argv[5]);
	for(i = 0; i < (sizeof ch_access / sizeof ch_access[0]); i++)
		if(!strcmp(argv[7], ch_access[i].str))
			hp->mode = ch_access[i].type;

	/*  Initialize the hardware  */
	isav = disable();
	hapn_init(hp);

	/*  Enable the interrupt  */
	maskon(hapn[dev].vec);

	restore(isav);
	return 0;
}

/*  initialize the HAPN adaptor */
int
hapn_init(hp)
register struct hapn *hp;
{
	register int16 base;
	char isav;

	isav = disable();
	base = hp->base;

	/*  Reset the 8273 */
	outportb(base+RST, 1);
	outportb(base+RST, 0);
	inportb(base+TXI);		/* Clear any old IR contents */
	inportb(base+RXI);

	/*  Select the operating modes  */
	cmd_8273(base, SET_XFER, 1, 1);
	cmd_8273(base, SET_MODE, 1, HDLC | EARLY | PREFRM | FLG_STM);
	cmd_8273(base, SET_SERIAL, 1, NRZI);
	cmd_8273(base, SET_B, 1, IRQ_ENB | RTS);
	cmd_8273(base, RST_B, 1, 0xff ^ RTS);
	hrxgo(hp);
	restore(isav);
	return 0;
}

/*  shut down the HAPN adaptor */
int
hapn_stop(iface)
struct interface *iface;
{
	int16 dev;
	int16 base;
	struct hapn *hp;

	dev = iface->dev;
	hp = &hapn[dev];
	base = hp->base;

	/*  Mask off interrupt input  */
	maskoff(hp->vec);

	/*  Restore original interrupt vector  */
	setirq(hp->vec,hp->oldvec);

	/*  Reset the 8273  */
	outportb(base+RST, 1);
	outportb(base+RST, 0);
	return 0;
}

/* Display adaptor statistics */
int
dohapnstat()
{
	struct hapn *hp;
	int i;

	if(nhapn == 0){
		printf("No HAPN adaptor attached\n");
		return 1;
	}
	for(i = 0; i < nhapn; i++){
		hp = &hapn[i];
		printf("HAPN %d:   rxints: %ld   txints: %ld   badint: %-5d\r\n", i,
		 hp->rxints,hp->txints,hp->badint);
		printf(" receive  - frames:  %-5d  crcerrs: %-5d  aborts: %-5d  dmaorun: %-5d\r\n",
		 hp->rframes,hp->crcerr, hp->aborts, hp->dmaorun);
		printf("          - toobig:  %-5d  dcdloss: %-5d  rxorun: %-5d\r\n",
		 hp->toobig,hp->cdloss,hp->rxorun);
		printf(" transmit - frames:  %-5d  aborts : %-5d  uruns : %-5d  ctsloss: %-5d\r\n",
		 hp->tframes,hp->taborts, hp->t_urun, hp->ctsloss);
	}
	return 0;
}

/*  periodically kicked by mainline routine
 *  process any queued received frames
 *  kick tx if waiting on busy channel
 */
void
dohapn(iface)
struct interface *iface;
{
	struct hapn *hp;
	struct mbuf *bp;

	hp = &hapn[iface->dev];

	/*  Process any received frames  */
	while((bp = dequeue(&hp->rcvq)) != NULLBUF){
		hp->rcvcnt--;
		dump(iface,IF_TRACE_IN,TRACE_AX25,bp);
		ax_recv(iface, bp);
	}

	/*  Test for deferred transmit (CSMA)  */
	if(hp->tstate == DEFER)
		htxint(hp);
}

/* Send raw packet on HAPN interface */
int
hapn_raw(interface,bp)
struct interface *interface;
struct mbuf *bp;
{
	struct hapn *hp;

	dump(interface,IF_TRACE_OUT,TRACE_AX25,bp);
	hp = &hapn[interface->dev];
	enqueue(&hp->sndq, bp);

	/*  See if anything being transmitted  */
	if(hp->tstate == IDLE)
		htxint(hp);
	return 0;
}
