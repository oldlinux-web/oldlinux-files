/* Interface driver for the PACCOMM PC-100 board for the IBM PC */
/* UNFINISHED, DOESN'T WORK YET - work in progress by Bdale */
/* currently only attempting to use the AMD7910 on Channel A */

#include <stdio.h>
#include "global.h"
#include "mbuf.h"
#include "iface.h"
#include "pc100.h"
#include "8530.h"
#include "ax25.h"
#include "trace.h"

struct pc100 pc100[NPC];
void pc0vec(),write_scc(),rts();
void (*pchandle[])() = { pc0vec };
struct hdlc hdlc[2*NPC];
int16 npc;

/* Branch table for interrupt handler */
void htxint(), hexint(), hrxint(), hspint();
static void (*svec[])() = {
	htxint, hexint, hrxint, hspint
};

/* Master interrupt handler for the PC-100 card. All interrupts come
 * here first, then are switched out to the appropriate routine.
 */
void
pcint(dev)
int16 dev;
{
	register char iv;
	register int16 pcbase;
	struct hdlc *hp;

	pc100[dev].ints++;
	pcbase = pc100[dev].addr;

	/* Read interrupt vector, including status, from channel B */
	iv = read_scc(CTL+pcbase+CHANB,R2);

	hp = &hdlc[2 * dev + ((iv & 0x80)? 0 : 1)];

	/* Now switch to appropriate routine */
	(*svec[(iv>>1) & 0x3])(hp);

	/* Reset interrupt pending state (register A only) */
	write_scc(CTL+pcbase+CHANA,R0,RES_H_IUS);

	/* Wang the 8530 hardware interrupt acknowledge line - Bdale */
	inportb(pcbase+INTACK);
}
/* HDLC Special Receive Condition interrupt
 * The most common event that triggers this interrupt is the
 * end of a frame; it can also be caused by a receiver overflow.
 */
static void
hspint(hp)
register struct hdlc *hp;
{
	register char c;

	hp->spints++;
	c = read_scc(CTL+hp->base,R1);	/* Fetch latched bits */

	if((c & (END_FR|CRC_ERR)) == END_FR && hp->rcvbuf != NULLBUF
		&& hp->rcvbuf->cnt > 1){
		/* End of valid frame */
		hp->rcvbuf->cnt--;	/* Toss 1st crc byte */
		enqueue(&hp->rcvq,hp->rcvbuf);
		hp->rcvbuf = NULLBUF;
		hp->rcvcnt++;
	} else {
		/* An overflow or CRC error occurred; restart receiver */
		hp->crcerr++;
		if(hp->rcvbuf != NULLBUF){
			hp->rcp = hp->rcvbuf->data;
			hp->rcvbuf->cnt = 0;
		}
	}
	write_scc(CTL+hp->base,R0,ERR_RES);
}
/* HDLC SIO External/Status interrupts
 * The only one of direct interest is a receiver abort; the other
 * usual cause is a change in the modem control leads, so kick the
 * transmit interrupt routine.
 */
static void
hexint(hp)
register struct hdlc *hp;
{
	hp->exints++;
	hp->status = read_scc(CTL+hp->base,R0);	/* Fetch status */
	if((hp->status & BRK_ABRT) && hp->rcvbuf != NULLBUF){
		hp->aborts++;
		/* Restart receiver */
		hp->rcp = hp->rcvbuf->data;
		hp->rcvbuf->cnt = 0;
	}
	write_scc(CTL+hp->base,R0,RES_EXT_INT);
	write_scc(CTL+hp->base,R0,RES_H_IUS);
	/* Kick the transmit interrupt routine for a possible modem change */
	htxint(hp);
}
/* HDLC receiver interrupt handler. Allocates buffers off the freelist,
 * fills them with receive data, and puts them on the receive queue.
 */
static void
hrxint(hp)
register struct hdlc *hp;
{
	register struct mbuf *bp;
	register int16 base;

	hp->rxints++;
	base = hp->base;
	/* Allocate a receive buffer if not already present */
	if((bp = hp->rcvbuf) == NULLBUF){
		bp = hp->rcvbuf = alloc_mbuf(hp->bufsiz);
		if(bp == NULLBUF){
			/* No memory, abort receiver */
			hp->nomem++;
			write_scc(CTL+base,R3,ENT_HM|RxENABLE|RxCRC_ENAB|Rx8);
			(void) inportb(base+DATA);
			return;
		}
		hp->rcp = hp->rcvbuf->data;
	}
	while(read_scc(CTL+base,R0) & Rx_CH_AV){
		if(bp->cnt++ >= hp->bufsiz){
			/* Too large; abort the receiver, toss buffer */
			hp->toobig++;
			write_scc(CTL+base,R3,ENT_HM|RxENABLE|RxCRC_ENAB|Rx8);
			(void) inportb(base+DATA);
			free_p(bp);
			hp->rcvbuf = NULLBUF;
			break;
		}
		/* Normal save */
		*hp->rcp++ = inportb(base+DATA);
	}
}
int ctswait;
/* HDLC transmit interrupt service routine
 *
 * The state variable tstate, along with some static pointers,
 * represents the state of the transmit "process".
 */
static void
htxint(hp)
register struct hdlc *hp;
{
	register int16 base;
	char i_state,c;

	i_state = disable();
	hp->txints++;
	base = hp->base;
	while(read_scc(CTL+base,R0) & Tx_BUF_EMP){
		switch(hp->tstate){
		/* First here for efficiency */
		case ACTIVE:		/* Sending frame */
			if(pullup(&hp->sndbuf,&c,1) == 1){
				outportb(base+DATA,c);
			} else {
				/* Do this after sending the last byte */
				write_scc(CTL+base,R0,RES_Tx_P);
				if((hp->sndbuf = dequeue(&hp->sndq)) == NULLBUF){
					switch(hp->mode){
					case CSMA:
						/* Begin transmitter shutdown */
						hp->tstate = FLUSH;
						break;
					case FULLDUP:
						hp->tstate = IDLE;
						break;
					}
				}
			}
			continue;
		case IDLE:
			/* Transmitter idle. Find a frame for transmission */
			if((hp->sndbuf = dequeue(&hp->sndq)) == NULLBUF)
				goto ret;

		case DEFER:	/* note fall-thru */
			if(hp->mode == CSMA && (hp->status & DCD)){
				hp->tstate = DEFER;
				goto ret;
			}
			rts(base,ON);	/* Transmitter on */
		case KEYUP:	/* note fall-thru */
			if((hp->status & CTS) == 0){
				ctswait++;
				hp->tstate = KEYUP;
				goto ret;
			}
			write_scc(CTL+base,R0,RES_Tx_CRC);
			pullup(&hp->sndbuf,&c,1);
			outportb(hp->base+DATA,c);
			hp->tstate = ACTIVE;
			write_scc(CTL+base,R0,RES_EOM_L);
			continue;
		case FLUSH:	/* Sending flush character */
			outportb(hp->base+DATA,(char)0);
			hp->tstate = FIN2;
			continue;
		case FIN2:
			write_scc(CTL+base,R0,SEND_ABORT);
			hp->tstate = IDLE;
			rts(base,OFF);
			write_scc(CTL+base,R0,RES_Tx_P);
			continue;
		}
	}
ret:	restore(i_state);
}

/* Set request-to-send on modem */
static void
rts(base,x)
int16 base;
int x;
{
	int16 cmd;

	if(x)
		cmd = TxCRC_ENAB | RTS | TxENAB | Tx8 | DTR;
	else
		cmd = TxCRC_ENAB | TxENAB | Tx8 | DTR;
	write_scc(CTL+base,R5,cmd);
}
/* (re)Initialize HDLC controller parameters */
static int
hdlcparam(hp)
register struct hdlc *hp;
{
	int16 tc;
	char i_state;
	register int16 base;

	/* Initialize 8530 channel for SDLC operation */
	base = hp->base;
	i_state = disable();

	switch(base & 2){
	case 0:
		write_scc(CTL+base,R9,CHRA);	/* Reset channel A */
		break;
	case 2:
		write_scc(CTL+base,R9,CHRB);	/* Reset channel B */
		break;
	}
	/* Wait/DMA disable, Int on all Rx chars + spec condition,
	 * parity NOT spec condition, TxINT enable, Ext Int enable
	 */
	write_scc(CTL+base,R1,INT_ALL_Rx | TxINT_ENAB | EXT_INT_ENAB);

	/* Dummy interrupt vector, will be modified by interrupt type
	 * (This probably isn't necessary)
	 */
	write_scc(CTL+base,R2,0);

	/* 8 bit RX chars, auto enables off, no hunt mode, RxCRC enable,
	 * no address search, no inhibit sync chars, enable RX
	 */
	write_scc(CTL+base,R3,Rx8|RxCRC_ENAB|RxENABLE);

	/* X1 clock, SDLC mode, Sync modes enable, parity disable
	 * (Note: the DPLL does a by-32 clock division, so it's not necessary
	 * to divide here).
	 */
	write_scc(CTL+base,R4,X1CLK | SDLC | SYNC_ENAB);

	/* DTR On, 8 bit TX chars, no break, TX enable, SDLC CRC,
	 * RTS off, TxCRC enable
	 */
	write_scc(CTL+base,R5,DTR|Tx8|TxENAB|TxCRC_ENAB);

	/* SDLC flag */
	write_scc(CTL+base,R7,FLAG);

	/* No reset, status low, master int enable, enable lower chain,
	 * no vector, vector includes status
	 */
	write_scc(CTL+base,R9,MIE|NV|VIS);
	/* CRC preset 1, NRZI encoding, no active on poll, flag idle,
	 * flag on underrun, no loop mode, 8 bit sync
	 */
	write_scc(CTL+base,R10,CRCPS|NRZI);

	/* Board no longer channel-specific for clk.  The board should be set
	 * up to run from the 4.9152Mhz onboard crystal connected to PCLK.
	 * Both channels get receive clock at 32x from PCLK via the DPLL,
	 * with TRxC as an output, via a 4040 div by 32 counter to RTxC set
	 * us as an input to provide the transmit clock.
	 */

	/*            TRxC = BR Generator Output, TRxC O/I,
	 *	      transmit clock = RTxC pin, 
	 *	      receive clock = DPLL output
	 */
	write_scc(CTL+base,R11,TRxCBR|TRxCOI|TCRTxCP|RCDPLL);

	/* Compute and load baud rate generator time constant
	 * DPLL needs x32 clock
	 * XTAL is defined in pc100.h to be the crystal clock / (2 * 32)
	 */
	tc = XTAL/(hp->speed) - 2;
	write_scc(CTL+base,R12,tc & 0xff);
	write_scc(CTL+base,R13,(tc >> 8) & 0xff);

	write_scc(CTL+base,R14,SNRZI);	/* Set NRZI mode */
	write_scc(CTL+base,R14,SSBR);	/* Set DPLL source = BR generator */
	write_scc(CTL+base,R14,SEARCH);	/* Enter search mode */
	/* Set baud rate gen source = PCLK, enable baud rate gen */
	write_scc(CTL+base,R14,BRENABL|BRSRC);

	/* Break/abort IE, TX EOM IE, CTS IE, no SYNC/HUNT IE, DCD IE,
	 * no Zero Count IE
	 */
	write_scc(CTL+base,R15,BRKIE|TxUIE|CTSIE|DCDIE);

	restore(i_state);
	if(hp->mode == FULLDUP){
		rts(base,ON);
	} else if(hp->tstate == IDLE){
		rts(base,OFF);
	}
	return 0;
}
/* Attach a PC-100 interface to the system
 * argv[0]: hardware type, must be "pc100"
 * argv[1]: I/O address, e.g., "0x380"
 * argv[2]: vector, e.g., "2"
 * argv[3]: mode, must be:
 *	    "ax25" (AX.25 UI frame format)
 * argv[4]: interface label, e.g., "pc0"
 * argv[5]: receiver packet buffer size in bytes
 * argv[6]: maximum transmission unit, bytes
 * argv[7]: interface speed, e.g, "9600"
 */
int
pc_attach(argc,argv)
int argc;
char *argv[];
{
	register struct interface *if_pca,*if_pcb;
	extern struct interface *ifaces;
	struct hdlc *hp;
	int dev;
	int pc_init();
	void dopc();
	int pc_stop();
	int ax_send();
	int ax_output();
	int pc_raw();
	void (*getirq())();	/* Getirq is a function returning a pointer to
				 * a function returning void */

	if(npc >= NPC){
		printf("Too many pc100 controllers\n");
		return -1;
	}
	dev = npc++;

	/* Initialize hardware-level control structure */
	pc100[dev].addr = htoi(argv[1]);
	pc100[dev].vec = htoi(argv[2]);
	/* Initialize modems */
	outportb(pc100[dev].addr + MODEM_CTL,(char)0x22);

	/* Save original interrupt vector */
	pc100[dev].oldvec = getirq(pc100[dev].vec);
	/* Set new interrupt vector */
	if(setirq(pc100[dev].vec,pchandle[dev]) == -1){
		printf("IRQ %u out of range\n",pc100[dev].vec);
		npc--;
		return -1;
	}
	/* Create interface structures and fill in details */
	if_pca = (struct interface *)calloc(1,sizeof(struct interface));
	if_pcb = (struct interface *)calloc(1,sizeof(struct interface));

	if_pca->name = malloc((unsigned)strlen(argv[4])+1);
	strcpy(if_pca->name,argv[4]);
	if_pcb->name = malloc((unsigned)strlen(argv[4])+1);
	strcpy(if_pcb->name,argv[4]);
	if_pcb->name[strlen(argv[4]) - 1]++;	/* kludge */
	if_pcb->mtu = if_pca->mtu = atoi(argv[6]);
	if_pca->dev = 2*dev;
	if_pcb->dev = 2*dev + 1;
	if_pcb->recv = if_pca->recv = dopc;
	if_pcb->stop = if_pca->stop = pc_stop;
	if_pcb->output = if_pca->output = ax_output;
	if_pcb->raw = pc_raw;

	if(strcmp(argv[3],"ax25") == 0){

		axarp();
		if(mycall.call[0] == '\0'){
			printf("set mycall first\n");
			free((char *)if_pca);
			free((char *)if_pcb);
			return -1;
		}		
		if_pcb->send = if_pca->send = ax_send;
		if(if_pcb->hwaddr == NULLCHAR)
			if_pcb->hwaddr = malloc(sizeof(mycall));
		memcpy(if_pcb->hwaddr,(char *)&mycall,sizeof(mycall));
	} else {
		printf("Mode %s unknown for interface %s\n",
			argv[3],argv[4]);
		free((char *)if_pca);
		free((char *)if_pcb);
		return -1;
	}
	if_pca->next = if_pcb;
	if_pcb->next = ifaces;
	ifaces = if_pca;

	hp = &hdlc[2*dev+1];
	hp->speed = (int16)atoi(argv[7]);
	hp->base = pc100[dev].addr + CHANB;
	hp->bufsiz = atoi(argv[5]);
	hdlcparam(hp);

	hp = &hdlc[2*dev];
	hp->speed = (int16)atoi(argv[7]);
	hp->base = pc100[dev].addr + CHANA;
	hp->bufsiz = atoi(argv[5]);
	hdlcparam(hp);

	/* Clear mask (enable interrupt) in 8259 interrupt controller */
	clrbit(INTMASK,(char)(1<<pc100[dev].vec));

	return 0;
}
void
dopc(interface)
struct interface *interface;
{
	struct hdlc *hp;
	struct mbuf *bp;

	hp = &hdlc[interface->dev];
	while((bp = dequeue(&hp->rcvq)) != NULLBUF){
		hp->rcvcnt--;
		dump(interface,IF_TRACE_IN,TRACE_AX25,bp);
		ax_recv(interface,bp);
	}	
}
int
pc_stop(iface)
struct interface *iface;
{
	int16 dev;

	dev = iface->dev;
	if(dev & 1)
		return 0;
	dev >>= 1;	/* Convert back into PC100 number */
	/* Turn off interrupts */
	maskoff(pc100[dev].vec);

	/* Restore original interrupt vector */
	setirq(pc100[dev].vec,pc100[dev].oldvec);

	/* Force hardware reset */
	write_scc(CTL+pc100[dev].addr + CHANA,R9,FHWRES);
	return 0;
}
	
/* Send raw packet on PC-100 */
int
pc_raw(interface,bp)
struct interface *interface;
struct mbuf *bp;
{
	char kickflag;
	struct hdlc *hp;

	dump(interface,IF_TRACE_OUT,TRACE_AX25,bp);
	hp = &hdlc[interface->dev];
	kickflag = (hp->sndq == NULL);
	enqueue(&hp->sndq,bp);
	if(kickflag)
		htxint(&hdlc[interface->dev]);
	return 0;
}
