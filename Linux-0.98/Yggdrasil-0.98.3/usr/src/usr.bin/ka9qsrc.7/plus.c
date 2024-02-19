/* OS- and machine-dependent stuff for HP 110+ running MS-DOS */
#if defined(__TURBOC__)
#pragma inline
#include <stdio.h>
#include "config.h"
#if defined(PLUS)
#include "global.h"
#include "8250.h"
#include "asy.h"
#include "plus.h"
#include "iface.h"

struct asy asy[ASY_MAX];
unsigned nasy;
byte model;

void	_Cdecl	__int__		(int interruptnum);
void	interrupt 	(* _Cdecl getvect(int interruptno)) ();
#define geninterrupt(i) __int__(i)

extern unsigned char intstk[];
extern unsigned int sssave, spsave;
/*
 =================================================================
 */
int16 modemdev, serialdev;
struct plusasy plusasy[ASY_MAX];
/* ASY interrupt handlers */
byte modemconfig, serialconfig;
extern void interrupt plustxsvec();
extern void interrupt plusrxsvec();
extern void interrupt plustxmvec();
extern void interrupt plusrxmvec();
byte *com1_name="COM1";         /* file to open  for serial		    */
byte *com3_name="COM3";         /* file to open for modem		    */
int com1handle;			/* file handle saved here		    */
int com3handle;			/* file handle saved here		    */
byte *ioctlon1="M1;";           /* IOCTL to turn on  serial port            */
byte *ioctloff1="M3;";          /* IOCTL to turn off serial port            */
byte *ioctlon3="M4;";           /* IOCTL to turn on  modem port            */
byte *ioctloff3="M5;";          /* IOCTL to turn off modem port            */

/* Initialize asynch port "dev" */
int
asy_init(dev,arg1,arg2,bufsize)
int16 dev;
char *arg1,*arg2;
unsigned bufsize;
{
    register unsigned base;
    register struct fifo *fp;
    register struct asy *ap;
  
    ap = &asy[dev];
    ap->addr = htoi(arg1);
    ap->vec = htoi(arg2);
    /* Set up receiver FIFO */
    fp = &ap->fifo;
    if((fp->buf = malloc(bufsize)) == NULLCHAR){
        printf("asy%d: No space for rx buffer\r\n");
        fflush(stdout);
        return;
    }
    fp->bufsize = bufsize;
    fp->wp = fp->rp = fp->buf;
    fp->cnt = 0;
  
    base = ap->addr;
    /* Purge the receive data buffer
     * Set line control register: 8 bits, no parity
     *
     * Turn on receive interrupt enable, leave transmit
     * interrupts turned off for now
     *
     * Set modem control register: assert DTR, RTS, turn on serial
     * port
     *
     * Enable rx interrupt
     */
    (void)init_com(dev,'N',1);
    /*
     * set calibration in delay
     */
     /*
      * turn off lousy beeper on ring interrupts.
      */
     _AL=2;
     __int__(0x59);
    /* Set interrupt vector to SIO handler
     *  DON'T bump them by 8 as we use the actual
     *  interrupt numbers, not the number -8 as in
     *  the pc asy entry.
     *
     *  If the computer is a portable only the serial interrupts are used
     *  If the computer is a P+ both serial and modem interrupts are used
     *
     *                     110    110p serial   110p modem
     *          rx vector  0x4f       0x4f	   0x4e
     *          tx vector  0x4a       0x4a	   0x40
     *          base addr  0xa4       0x44         0xa4   
     *
     */
    if(base==0x44 || model==1) {
	plusasy[dev].oldrx=getvect(0x4f);
	plusasy[dev].oldtx=getvect(0x4a);
	if(model==1) {
		asy[dev].addr=0xa4;
    		setvect(0x4a,plustxmvec);
    		setvect(0x4f,plusrxmvec);
		modemdev=dev;
	}
	else {
		setvect(0x4a,plustxsvec);
		setvect(0x4f,plusrxsvec);
		serialdev=dev;
	}
    }
    else { /* address not 0x44 and not a portable, must be plus modem */
	plusasy[dev].oldrx=getvect(0x4e);
	plusasy[dev].oldtx=getvect(0x40);
	setvect(0x40,plustxmvec);
	setvect(0x4e,plusrxmvec);
	modemdev=dev;
    }
} /* asy_init */

int
asy_stop(iface)
struct interface *iface;
{
    register unsigned base;
    register struct asy *ap;
  
    ap = &asy[iface->dev];
    base = ap->addr;
  
/*
 * hardware ints off
 */
    outportb(base+INTRCONTROL,(unsigned char)0xf0);
/*
 * turn off power to the serial port and
 * Restore original interrupt vectors
 */
    if(base==0x44 || model==1) {
    	serial_power(0);
    	setvect(0x4f,plusasy[iface->dev].oldrx);
    	setvect(0x4a,plusasy[iface->dev].oldtx);
    }
    else {
    	modem_power(0);
    	setvect(0x4e,plusasy[iface->dev].oldrx);
    	setvect(0x40,plusasy[iface->dev].oldtx);
    }
    return(0);
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
    register unsigned base;
    register int divisor;
    char i_state;
  
    if(speed == 0 || dev >= nasy)
        return;
  
    base = asy[dev].addr;
    asy[dev].speed = speed;
/*
 *  Setup baud rate
 */
    divisor=1333500L/speed - 1;
    i_state = disable();
    outportb((unsigned)(base+BAUDLO), (unsigned char)(divisor));
    outportb((unsigned)(base+BAUDHI), (unsigned char)(divisor>>8));
    restore(i_state);
}

/* Send a buffer to serial transmitter */
asy_output(dev,buf,cnt)
unsigned dev;
char *buf;
unsigned short cnt;
{
    register struct dma *dp;
    unsigned base;
    char i_state;
 
    if(cnt == 0)
	return;
    if(dev >= nasy)
        return;
    base = asy[dev].addr;
    dp = &asy[dev].dma;
    i_state = disable();
    if(dp->flags){
        restore(i_state);
        return; /* Already busy */
    }
    dp->data = buf;
    dp->cnt = cnt;
    dp->flags = 1;
    /* Enable transmitter buffer empty interrupt and simulate
     * an interrupt; this will get things rolling.
     */
    clrbit(base+INTRCONTROL,0x80);
    setbit(base+INTRCONTROL,0x40);
    asytxint(dev);
    restore(i_state);
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

/* Process 110P receiver interrupts */
static
asyrxint(dev)
unsigned dev;
{
    unsigned base;
    register struct fifo *fp;
    char c;
  
    base = asy[dev].addr;
    fp = &asy[dev].fifo;
    c = inportb(base+RXDATA);
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

/* Handle 110P transmitter interrupts */
static
asytxint(dev)
unsigned dev;
{
    register struct dma *dp;
    register unsigned base;
  
    base = asy[dev].addr;
    dp = &asy[dev].dma;
    if(!dp->flags){
        /* "Shouldn't happen", but disable transmit
         * interrupts anyway
         */
	setbit(base+INTRCONTROL,0xc0);
	return; /* Nothing to send */
    }
    while(inportb(base+STATUS) & 0x02){
        outportb(base+TXDATA,*dp->data++);
        if(--dp->cnt == 0){
            dp->flags = 0;
            /* Disable transmit interrupts */
            setbit(base+INTRCONTROL,0xc0);
            break;
	}
/*
 * turn em back on, we got more to do
 */
    	clrbit(base+INTRCONTROL,0x80);
    	setbit(base+INTRCONTROL,0x40);
    }
}

int
stxrdy(dev)
int16 dev;
{
    return(!asy[dev].dma.flags);
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

int init_com(dev,wordlen,parity)
int dev, wordlen;
char parity;
{
    int i, offset, sconfig;
    i=peek(0xffff,0x0e) & 0xff;
    if(i==0xb6) {
        model=1;            /* portable      */
	asy[dev].addr=0xa4;
        offset=i=0x60;
    }
    else
        if((i=='A') || (i=='B')) {
            model=0;            /* portable plus */
            offset=i=0;
        }
        else
            if(i>0xfb) model=2; /* pc type       */
    if(model==2) {
        return(offset=-1);
    }
/*
 * turn on port
 */
    outportb((unsigned)(asy[dev].addr+INTRCONTROL),
    	(unsigned char)0xf0); /* make sure no ints from serial */
    if(offset || asy[dev].addr==0x44) {
	serial_power(1);
    }
    else {
	modem_power(1);
    }
  
    i=wordlen-5;
    if ((i<0) || (i>3)) i=3;
    i=i<<6;
  
    sconfig=i | 2;             /* setup our local copy */
    switch(parity) {
        case 'n':
        case 'N':   i=0;
            break;
        case 'e':
        case 'E':   i=2;
            break;
        case 'o':
        case 'O':   i=3;
            break;
        default:
            i=0;
    }
    i=i<<2;
    sconfig|=i;
    if(offset || asy[dev].addr==0xa4) 
	modemconfig=sconfig;
    else
	serialconfig=sconfig;
/*
 * set up serial configuration register
 */
    outportb((unsigned)(asy[dev].addr+FORMATCONTROL),
    	(unsigned char)sconfig);
/*
 * interrupt on data avail
 */
    outportb((unsigned)(asy[dev].addr+INTRCONTROL+offset),
    	(unsigned char)0xd0);
    return 4;
} /* init_com */
  
#if 0  
/*
 * This code is if def'ed out. I left it in in case someone wanted to
 *  see how to read these lines.  WA7TAS
 */
/*
 * state of cts and dsr lines are inverted on the portable plus
 *  and not inverted on the portable. CTS is bit 6 on the portable,
 *  and bit 7 on the portable plus. DSR is bit 7 on the portable,
 *  and bit 6 on the portable plus.
 */
int cts()
{
    if(model==1) return((int)(inportb(0x2f) & 0x40));
    return(!((int)(inportb(0x2f) & 0x80)));
} /* cts */
  
int dsr()
{
    if(model==1) return((int)(inportb(0x2f) & 0x80));
    return(!((int)(inportb(0x2f) & 0x40)));
}
 /* dsr */
#endif  

void interrupt plustxsvec()
{
	asm mov ds:sssave,ss;
	asm mov ds:spsave,sp;

	_BX=intstk;
	_SS=_DS;	
	asm mov sp,bx

	asytxint(serialdev);

	asm mov sp,ds:spsave;
	asm mov ss,ds:sssave;
} /* plustxsvec */

/*
 * see turbo c user's guide page p283
 */
void interrupt plusrxsvec()
{
	register int tchar, dd;

	asm mov ds:sssave,ss;
	asm mov ds:spsave,sp;

	_BX=intstk;
	_SS=_DS;	
	asm mov sp,bx

	if((inportb(SERIAL_STATUS) & 0x80) != 0) asyrxint(serialdev);
	outportb(SERIAL_FMTCNTL,serialconfig | 2);
	tchar=inportb(SERIAL_INTRCNTL);
	tchar|=0x10;
	tchar&=(~0x20);

	asm mov sp,ds:spsave;
	asm mov ss,ds:sssave;

	outportb(SERIAL_INTRCNTL,tchar);
} /* plusrxsvec */

void interrupt plustxmvec()
{
	asm mov ds:sssave,ss;
	asm mov ds:spsave,sp;

	_BX=intstk;
	_SS=_DS;	
	asm mov sp,bx

	asytxint(modemdev);

	asm mov sp,ds:spsave;
	asm mov ss,ds:sssave;
} /* plustxmvec */
/*
 * see turbo c user's guide page p283
 */

void interrupt plusrxmvec()
{
	register int tchar, dd;
	asm mov ds:sssave,ss;
	asm mov ds:spsave,sp;

	_BX=intstk;
	_SS=_DS;	
	asm mov sp,bx

	if((inportb(MODEM_STATUS) & 0x80) != 0) asyrxint(modemdev);
	outportb(MODEM_FMTCNTL,modemconfig | 2);
	tchar=inportb(MODEM_INTRCNTL);
	tchar|=0x10;
	tchar&=(~0x20);

	asm mov sp,ds:spsave;
	asm mov ss,ds:sssave;

	outportb(MODEM_INTRCNTL,tchar);

} /* plusrmsvec */

void modem_power(on_off)
int on_off;
{
/*
 * open com3 file see 110+ tech ref page 6-18
 */
    _AX=0x3d02;
    _DX=com3_name;
    geninterrupt(0x21);
    com3handle=_AX;
/*
 * get com3 device status info
 */
    _BX=_AX;
    _AX=0x4400;
    geninterrupt(0x21);
/*
 * set raw bit
 */
    _BX=com3handle;
    _DH=0;
    _DL|=0x20;
    _AX=0x4401;
    geninterrupt(0x21);
/*
 * turn on rs232 port or off
 */
    if(on_off!=0) { /* turn it on */
	_BX=com3handle;
	_DX=ioctlon3;
	_CX=3;
	_AX=0x4403;
    }
    else {
	_DX=ioctloff3;
	_CX=3;
	_AX=0x4403;
	geninterrupt(0x21);
/*
 * close the file handle as we're done with it.
 */
	_BX=com3handle;
	_AX=0x3e00;
    }
    geninterrupt(0x21);
} /* modem_power */

void serial_power(on_off)
int on_off;
{
/*
 * open com1 file see 110+ tech ref page 6-18
 */
    _AX=0x3d02;
    _DX=com1_name;
    geninterrupt(0x21);
    com1handle=_AX;
/*
 * get com1 device status info
 */
    _BX=_AX;
    _AX=0x4400;
    geninterrupt(0x21);
/*
 * set raw bit
 */
    _BX=com1handle;
    _DH=0;
    _DL|=0x20;
    _AX=0x4401;
    geninterrupt(0x21);
/*
 * turn on rs232 port or off
 */
    if(on_off!=0) { /* turn it on */
	_BX=com1handle;
	_DX=ioctlon1;
	_CX=3;
	_AX=0x4403;
    }
    else {
	_DX=ioctloff1;
	_CX=3;
	_AX=0x4403;
	geninterrupt(0x21);
/*
 * close the file handle as we're done with it.
 */
	_BX=com1handle;
	_AX=0x3e00;
    }
    geninterrupt(0x21);
} /* serial_power */
#endif  /* if defined(PLUS) */
#endif  /* if defined(__TURBOC__) */
