/* OS- and machine-dependent stuff for the 8250 asynch chip on a IBM-PC */
/* hacked to support BIOS interaction for NET PC9801 ala JK1NNT by N3EUA */

#include "config.h"
#if !defined(PLUS)

#include <stdio.h>
#include "global.h"
#include "asy.h"
#include "8250.h"
#include "iface.h"

struct asy asy[ASY_MAX];
unsigned nasy;

unsigned h2ivec[ASYHANDLE_MAX] ;
unsigned nhandlers ;

struct ivec ivec[NIVECS] ;

#ifdef PC9801
int work, f_handle, jkintdos();
#define mask 0x00ff
#endif

/* ASY interrupt handlers */
#ifndef PC9801
extern void asy0vec(),asy1vec(),asy2vec(),asy3vec(),asy4vec();
void (*handle[])() = {asy0vec,asy1vec,asy2vec,asy3vec,asy4vec};
#endif

/* Initialize asynch port "dev" */
int
asy_init(dev,arg1,arg2,bufsize)
int16 dev;
char *arg1,*arg2;	/* Attach args for address and vector */
unsigned bufsize;
{
	register unsigned base;
	register struct fifo *fp;
	register struct asy *ap;
	unsigned vec ;
	void (*getirq())();
	char i_state;

	ap = &asy[dev];
	ap->addr = htoi(arg1);
	ap->vec = htoi(arg2);
	/* Set up receiver FIFO */
	fp = &ap->fifo;
	if((fp->buf = malloc(bufsize)) == NULLCHAR){
		printf("asy%d: No space for rx buffer\r\n",dev);
		fflush(stdout);
		return -1;
	}
	fp->bufsize = bufsize;
	fp->wp = fp->rp = fp->buf;
	fp->cnt = 0;

	base = ap->addr;
	ap->urgent = NULLCHAR;	/* For SLFP urgent data */

#ifdef PC9801
	f_handle = auxopen();
#else
	/* Purge the receive data buffer */
	(void)inportb(base+RBR);

	i_state = disable();

	/* Set up interrupt vector structure if necessary */
	vec = ap->vec ;
	if (ivec[vec].ichain == NULLASY) {
		if (nhandlers == ASYHANDLE_MAX) {
			printf("asy%d: No more interrupt handlers\n",dev) ;
			return ;
		}

		/* Save original interrupt vector and mask */
		ivec[vec].oldvec = getirq(ap->vec) ;
		ivec[vec].oldmask = getmask(ap->vec) ;

		/* Set interrupt vector to SIO handler */
		setirq(ap->vec,handle[nhandlers]);

		/* Set up correspondence between handler and interrupt */
		h2ivec[nhandlers] = vec ;
		
		nhandlers++ ;		/* step to next handler */

		/* Start chain */
		ivec[vec].ichain = &asy[dev] ;
		ap->ichain = NULLASY ;		/* terminate the chain */
	}
	else {		/* Already a handler for this; just put in chain */
		ap->ichain = ivec[vec].ichain ;	/* Put at head */
		ivec[vec].ichain = &asy[dev] ;
	}
	
	/* Update vector reference count */
	ivec[vec].refcnt++ ;

	/* Save original control bits */
	ap->save.lcr = inportb(base+LCR);
	ap->save.ier = inportb(base+IER);
	ap->save.mcr = inportb(base+MCR);

	/* save speed bytes */
	setbit(base+LCR,LCR_DLAB);
	ap->save.divl = inportb(base+DLL);
	ap->save.divh = inportb(base+DLM);
	clrbit(base+LCR,LCR_DLAB);

	/* Set line control register: 8 bits, no parity */
	outportb(base+LCR,(char)LCR_8BITS);

	/* Turn on receive interrupt enable in 8250, leave transmit
	 * and modem status interrupts turned off for now
	 */
	outportb(base+IER,(char)IER_DAV);

	/* Set modem control register: assert DTR, RTS, turn on 8250
	 * master interrupt enable (connected to OUT2)
	 */
	outportb(base+MCR,(char)(MCR_DTR|MCR_RTS|MCR_OUT2));

	/* Enable interrupt */
	maskon(ap->vec);
	restore(i_state);
#endif /* PC9801 */
}
int
asy_stop(iface)
struct interface *iface;
{
	register unsigned base;
	register struct asy *ap;
	unsigned vec ;
	char i_state;

	ap = &asy[iface->dev];
	base = ap->addr;

#ifndef PC9801
	/* Purge the receive data buffer */
	(void)inportb(base+RBR);

	/* See if this is the last asy using this interrupt, */
	/* and restore the vector and interrupt mask if it is */
	vec = ap->vec ;
	if (--ivec[vec].refcnt == 0) {
		i_state = disable();
		setirq(ap->vec,ivec[vec].oldvec);
		if(ivec[vec].oldmask)
			maskon(ap->vec);
		else
			maskoff(ap->vec);
		restore(i_state);
	}

	/* Restore original interrupt vector and 8259 mask state */
	/* Restore speed regs */
	setbit(base+LCR,LCR_DLAB);
	outportb(base+DLL,ap->save.divl);	/* Low byte */
	outportb(base+DLM,ap->save.divh);	/* Hi byte */
	clrbit(base+LCR,LCR_DLAB);

	/* Restore control regs */
	outportb(base+LCR,ap->save.lcr);
	outportb(base+IER,ap->save.ier);
	outportb(base+MCR,ap->save.mcr);
#endif /* PC9801 */
}
/* Asynchronous line I/O control */
asy_ioctl(interface,argc,argv)
struct interface *interface;
int argc;
char *argv[];
{
	if(argc < 1){
		printf("%d\r\n",asy[interface->dev].speed);
		return 0;
	}
	return asy_speed(interface->dev,atoi(argv[0]));
}
/* Set asynch line speed */
int
asy_speed(dev,speed)
int16 dev;
int speed;
{
#ifdef PC9801
  register int s_code, speed1;

  if (speed ==0 || speed >= 10000 || dev >= nasy)
    return -1;

  speed1 = 75;
  for (s_code = 0; speed >= speed1; s_code++) speed1 *= 2;
  speed1 /= 2;
  asy[dev].speed = speed1;
  set_speed(s_code);
#else
	register unsigned base;
	register int divisor;
	char i_state;

	if(speed == 0 || dev >= nasy)
		return -1;
	
	base = asy[dev].addr;
	asy[dev].speed = speed;

	divisor = BAUDCLK / (long)speed;

	i_state = disable();

	/* Purge the receive data buffer */
	(void)inportb(base+RBR);

	/* Turn on divisor latch access bit */
	setbit(base+LCR,LCR_DLAB);

	/* Load the two bytes of the register */
	outportb(base+DLL,(char)(divisor & 0xff));		/* Low byte */
	outportb(base+DLM,(char)((divisor >> 8) & 0xff));	/* Hi byte */

	/* Turn off divisor latch access bit */
	clrbit(base+LCR,LCR_DLAB);

	restore(i_state);
	return 0;
#endif /* PC9801 */
}

/* Send a buffer to serial transmitter */
asy_output(dev,buf,cnt)
unsigned dev;
char *buf;
unsigned short cnt;
{
#ifdef PC9801
  int b_cnt, b_buf;

  if (dev >= nasy) return;
  for (b_cnt = 0; b_cnt < cnt; b_cnt++) {
    b_buf = buf[b_cnt];
    aput(b_buf);
  }
#else
	register struct dma *dp;
	unsigned base;
	char i_state;

	if(dev >= nasy)
		return;
	base = asy[dev].addr;
	dp = &asy[dev].dma;
	i_state = disable();
	if(dp->flags){
		restore(i_state);
		return;	/* Already busy */
	}
	dp->data = buf;
	dp->cnt = cnt;
	dp->flags = 1;
	/* Enable transmitter buffer empty interrupt and simulate
	 * an interrupt; this will get things rolling.
	 */
	setbit(base+IER,IER_TxE);
	asytxint(dev);
	restore(i_state);
#endif /* PC9801 */
}
/* Receive characters from asynch line
 * Returns count of characters read
 */
int16
asy_recv(dev,buf,cnt)
int16 dev;
char *buf;
unsigned cnt;
{
	unsigned tot,n;
#ifdef PC9801
	int ch;
	
	for (tot = 0; tot < cnt; tot++) {
	  if (auxstat()) ch = aget();
	  else break;
	  buf[tot] = ch;
	}
	return tot;
}

#else
	int kbread();
	char i_state;
	struct fifo *fp;

	fp = &asy[dev].fifo;
	tot = 0;
	/* Read from serial I/O input buffer */
	i_state = disable();
	for(;;){
		n = min(cnt,fp->cnt);
		if(n == 0)
			break;
		n = min(n,&fp->buf[fp->bufsize] - fp->rp);
		memcpy(buf,fp->rp,n);
		fp->rp += n;
		if(fp->rp >= &fp->buf[fp->bufsize])
			fp->rp = fp->buf;
		fp->cnt -= n;
		buf += n;
		tot += n;
		cnt -= n;
	}
	restore(i_state);
	return tot;

}
/* Interrupt handler for 8250 asynch chip */
void
asyint(handler)
unsigned handler;
{
	register unsigned base;
	register char iir;
	register unsigned dev ;
	struct asy *ap ;
	int someint ;

	/* The following bears some explaining.  Because the PC and AT
	 * (but not the PS/2) uses edge triggered interrupts, we need
	 * to assure that the shared interrupt line makes a low-going
	 * transition before we issue an EOI.  If we don't, one of the
	 * UARTs could raise its interrupt line again after we serviced
	 * it, but before we cleared all the other UARTs' interrupts,
	 * and this would be missed by the interrupt controller in
	 * edge triggered mode.  This should still work fine on a
	 * PS/2.
	 * Many thanks to Dan Dodge of Quantum Software, Ltd., for
	 * suggesting this trick.  dmf
	 */
	
	do {					

		someint = 0 ;	/* No interrupt detected yet on this pass */
		
		ap = ivec[h2ivec[handler]].ichain ;	/* Start at head of chain */

		while (ap != NULLASY) {
			dev = ap - asy ;
			base = asy[dev].addr;
			while(((iir = inportb(base+IIR)) & IIR_IP) == 0) {
				someint = 1 ;	/* Detected an interrupt */
				switch(iir & IIR_ID){
				case IIR_RDA:	/* Receiver interrupt */
					asyrxint(dev);
					break;
				case IIR_THRE:	/* Transmit interrupt */
					asytxint(dev);
					break;
				}
			}
			ap = ap->ichain ;	/* step to next in chain */
		}
		
	} while (someint != 0) ;
}
/* Process 8250 receiver interrupts */
static
asyrxint(dev)
unsigned dev;
{
	unsigned base;
	register struct fifo *fp;
	char c;

	base = asy[dev].addr;
	fp = &asy[dev].fifo;
	while(inportb(base+LSR) & LSR_DR){
		c = inportb(base+RBR);
		/* Process incoming data;
		 * If buffer is full, we have no choice but
		 * to drop the character
		 */
		if(fp->cnt != fp->bufsize){
			*fp->wp++ = c;
			if(fp->wp == &fp->buf[fp->bufsize])
				/* Wrap around */
				fp->wp = fp->buf;
			fp->cnt++;
		}
	}
}
/* Handle 8250 transmitter interrupts */
static
asytxint(dev)
unsigned dev;
{
	register struct dma *dp;
	register unsigned base;
	unsigned urg;	/* urgent SLFP data, or == 256 */

	base = asy[dev].addr;
	dp = &asy[dev].dma;
	if(!dp->flags){
		/* "Shouldn't happen", but disable transmit
		 * interrupts anyway
		 */
		clrbit(base+IER,IER_TxE);
		return;	/* Nothing to send */
	}
	while(inportb(base+LSR) & LSR_THRE){
		/* Send any pending urgent data */
		if (asy[dev].urgent != NULLCHAR)
			if ((urg = (*asy[dev].urgent)(dev)) < 256) {
				outportb(base+THR,urg);
				continue;
			}
		dp->last_octet = *dp->data;
		outportb(base+THR,*dp->data++);
		if(--dp->cnt == 0){
			dp->flags = 0;
			/* Disable transmit interrupts */
			clrbit(base+IER,IER_TxE);
			/* Call completion interrupt here */
			break;
		}
	}
}

/* Set bit(s) in I/O port */

setbit(port,bits)
unsigned port;
char bits;
{
	outportb(port,(char)inportb(port)|bits);
}
/* Clear bit(s) in I/O port */
clrbit(port,bits)
unsigned port;
char bits;
{
	outportb(port,(char)(inportb(port) & ~bits));
}
#endif /* PC9801 */

int
stxrdy(dev)
int16 dev;
{
	return(!asy[dev].dma.flags);
}

#ifdef PC9801
auxopen()
{
  char *device;
  device = "AUX";
  work = jkintdos(0x3d02,0,device);
  return (work);
}

auxstat() 
{
  work = jkintdos(0x4406,f_handle,0) & mask;
  return (work);
}

aget()
{
  work = jkintdos(0x0300,0,0) & mask;
  return (work);
}

aput(ch)
     int ch;
{
  jkintdos(0x0400,0,ch);
}
#endif /* PC9801 */
#endif /* !defined(PLUS) */
