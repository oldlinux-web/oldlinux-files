/* $Id: aha1542.c,v 1.1 1992/07/24 06:27:38 root Exp root $
 *  linux/kernel/aha1542.c
 *
 *  Copyright (C) 1992  Tommy Thorn
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/types.h>
#include <linux/string.h>

#include <asm/system.h>
#include <asm/io.h>
#include "scsi.h"
#include "hosts.h"

#include "aha1542.h"
#ifdef DEBUG
#define DEB(x) x
#else
#define DEB(x)
#endif

/*
static const char RCSid[] = "$Header: /usr/src/linux/kernel/blk_drv/scsi/RCS/aha1542.c,v 1.1 1992/07/24 06:27:38 root Exp root $";
*/

#define base 0x330
#define intr_chan 11

static struct mailbox mb[2];
static struct ccb ccb;

extern int slow_scsi_io;

long WAITtimeout, WAITnexttimeout = 3000000;

void (*do_done)(int, int) = NULL;
int aha1542_host = 0;
extern void aha1542_interrupt();

#define aha1542_intr_reset()  outb(IRST, CONTROL)
#define aha1542_enable_intr() outb(inb_p(0xA1) & ~8, 0xA1)
#define aha1542_disable_intr() outb(inb_p(0xA1) | 8, 0xA1)

#define WAIT(port, mask, allof, noneof)					\
 { register WAITbits;							\
   register WAITtimeout = WAITnexttimeout;				\
   while (1) {								\
     WAITbits = inb(port) & (mask);					\
     if ((WAITbits & (allof)) == (allof) && ((WAITbits & (noneof)) == 0)) \
       break;                                                         	\
     if (--WAITtimeout == 0) goto fail;					\
   }									\
 }

static void aha1542_stat(void)
{
/*    int s = inb(STATUS), i = inb(INTRFLAGS);
  printk("status=%x intrflags=%x\n", s, i, WAITnexttimeout-WAITtimeout); */
}

static int aha1542_out(unchar *cmdp, int len)
{
    while (len--)
      {
	  WAIT(STATUS, CDF, 0, CDF);
	  outb(*cmdp++, DATA);
      }
    return 0;
  fail:
    printk("aha1542_out failed(%d): ", len+1); aha1542_stat();
    return 1;
}

static int aha1542_in(unchar *cmdp, int len)
{
    while (len--)
      {
	  WAIT(STATUS, DF, DF, 0);
	  *cmdp++ = inb(DATA);
      }
    return 0;
  fail:
    printk("aha1542_in failed(%d): ", len+1); aha1542_stat();
    return 1;
}
int makecode(unsigned hosterr, unsigned scsierr)
{
    switch (hosterr) {
      case 0x0:
      case 0xa: /* Linked command complete without error and linked normally */
      case 0xb: /* Linked command complete without error, interrupt generated */
	hosterr = 0;
	break;

      case 0x11: /* Selection time out-The initiator selection or target
		    reselection was not complete within the SCSI Time out period */
	hosterr = DID_TIME_OUT;
	break;

      case 0x12: /* Data overrun/underrun-The target attempted to transfer more data
		    thean was allocated by the Data Length field or the sum of the
		    Scatter / Gather Data Length fields. */

      case 0x13: /* Unexpected bus free-The target dropped the SCSI BSY at an unexpected time. */

      case 0x15: /* MBO command was not 00, 01 or 02-The first byte of the CB was
		    invalid. This usually indicates a software failure. */

      case 0x16: /* Invalid CCB Operation Code-The first byte of the CCB was invalid.
		    This usually indicates a software failure. */

      case 0x17: /* Linked CCB does not have the same LUN-A subsequent CCB of a set
		    of linked CCB's does not specify the same logical unit number as
		    the first. */
      case 0x18: /* Invalid Target Direction received from Host-The direction of a
		    Target Mode CCB was invalid. */

      case 0x19: /* Duplicate CCB Received in Target Mode-More than once CCB was
		    received to service data transfer between the same target LUN
		    and initiator SCSI ID in the same direction. */

      case 0x1a: /* Invalid CCB or Segment List Parameter-A segment list with a zero
		    length segment or invalid segment list boundaries was received.
		    A CCB parameter was invalid. */
	hosterr = DID_ERROR; /* Couldn't find any better */
	break;

      case 0x14: /* Target bus phase sequence failure-An invalid bus phase or bus
		    phase sequence was requested by the target. The host adapter
		    will generate a SCSI Reset Condition, notifying the host with
		    a SCRD interrupt */
	hosterr = DID_RESET;
	break;
      default:
	printk("makecode: unknown hoststatus %x\n", hosterr);
	break;
    }
    return scsierr|(hosterr << 16);
}

int aha1542_test_port(void)
{
    volatile int debug = 0;
    
    /* Reset the adapter. I ought to make a hard reset, but it's not really nessesary */
    
    /*  DEB(printk("aha1542_test_port called \n")); */
    
    outb(SRST|IRST/*|SCRST*/, CONTROL);
    
    debug = 1;
    /* Expect INIT and IDLE, any of the others are bad */
    WAIT(STATUS, STATMASK, INIT|IDLE, STST|DIAGF|INVDCMD|DF|CDF);
    
    debug = 2;
    /* Shouldn't have generated any interrupts during reset */
    if (inb(INTRFLAGS)&INTRMASK) goto fail;
    
    debug = 3;
    /* Test the basic ECHO command */
    outb(CMD_ECHO, DATA);
    
    debug = 4;
    /* Wait for CDF=0. If any of the others are set, it's bad */
    WAIT(STATUS, STATMASK, 0, STST|DIAGF|INVDCMD|DF|CDF);
    
    debug = 5;
    /* The meaning of life */
    outb(42, DATA);
    
    debug = 6;
    /* Expect only DF, that is, data ready */
    WAIT(STATUS, STATMASK, DF, STST|DIAGF|CDF|INVDCMD);
    
    debug = 7;
    /* Is the answer correct? */
    if (inb(DATA) != 42) goto fail;
    
    debug = 8;
    /* Reading port should reset DF */
    if (inb(STATUS) & DF) goto fail;
    
    debug = 9;
    /* When HACC, command is completed, and we're though testing */
    WAIT(INTRFLAGS, HACC, HACC, 0);
    /* now initialize adapter */
    
    debug = 10;
    /* Clear interrupts */
    outb(IRST, CONTROL);
    
    debug = 11;
    
    return debug;				/* 1 = ok */
  fail:
    return 0;					/* 0 = not ok */
}

/* What's this little function for? */
char *aha1542_info(void)
{
    static char buffer[] = "";			/* looks nicer without anything here */
    return buffer;
}

/* A "high" level interrupt handler */
void aha1542_intr_handle(void)
{
    void (*my_done)(int, int) = do_done;
    int errstatus;

    do_done = NULL;
#ifdef DEBUG
    {
    int flag = inb(INTRFLAGS);
    printk("aha1542_intr_handle: ");
    if (!(flag&ANYINTR)) printk("no interrupt?");
    if (flag&MBIF) printk("MBIF ");
    if (flag&MBOA) printk("MBOF ");
    if (flag&HACC) printk("HACC ");
    if (flag&SCRD) printk("SCRD ");
    printk("status %02x\n", inb(STATUS));
    if (ccb.tarstat|ccb.hastat)
      printk("aha1542_command: returning %x (status %d)\n", ccb.tarstat + ((int) ccb.hastat << 16), mb[1].status);
  };
#endif
    aha1542_intr_reset();
    if (!my_done) {
	printk("aha1542_intr_handle: Unexpected interrupt\n");
	return;
    }

    /* is there mail :-) */
	
    if (!mb[1].status) {
	DEB(printk("aha1542_intr_handle: strange: mbif but no mail!\n"));
	my_done(aha1542_host, DID_TIME_OUT << 16);
	return;
    }

    /* more error checking left out here */
    if (mb[1].status != 1)
      /* This is surely wrong, but I don't know what's right */
      errstatus = makecode(ccb.hastat, ccb.tarstat);
    else
      errstatus = 0;

    mb[1].status = 0;

    if (ccb.tarstat == 2) {
#ifdef DEBUG
	int i;
#endif
	DEB(printk("aha1542_intr_handle: sense:"));
#ifdef DEBUG
	for (i = 0; i < 12; i++)
	  printk("%02x ", ccb.cdb[ccb.cdblen+i]);
	printk("\n");
#endif
/*
	DEB(printk("aha1542_intr_handle: buf:"));
	for (i = 0; i < bufflen; i++)
	  printk("%02x ", ((unchar *)buff)[i]);
	printk("\n");
*/
    }
    DEB(if (errstatus) printk("aha1542_intr_handle: returning %6x\n", errstatus));
    my_done(aha1542_host, errstatus);
    return;
}

int aha1542_queuecommand(unchar target, const void *cmnd, void *buff, int bufflen, void (*done)(int, int))
{
    unchar ahacmd = CMD_START_SCSI;
    unchar direction;
    unchar *cmd = (unchar *) cmnd;
    DEB(int i);

    DEB(if (target > 1) {done(aha1542_host, DID_TIME_OUT << 16); return 0;});
    
    if(*cmd == REQUEST_SENSE){
      memcpy(buff, &ccb.cdb[ccb.cdblen], bufflen);
      done(aha1542_host, 0); 
      return 0;
    };

#ifdef DEBUG
    if (*cmd == READ_10 || *cmd == WRITE_10)
      i = xscsi2int(cmd+2);
    else if (*cmd == READ_6 || *cmd == WRITE_6)
      i = scsi2int(cmd+2);
    else
      i = -1;
    if (done)
      printk("aha1542_queuecommand: dev %d cmd %02x pos %d len %d ", target, *cmd, i, bufflen);
    else
      printk("aha1542_command: dev %d cmd %02x pos %d len %d ", target, *cmd, i, bufflen);
    aha1542_stat();
    printk("aha1542_queuecommand: dumping scsi cmd:");
    for (i = 0; i < (*cmd<=0x1f?6:10); i++) printk("%02x ", cmd[i]);
    printk("\n");
    if (*cmd == WRITE_10 || *cmd == WRITE_6)
      return 0; /* we are still testing, so *don't* write */
#endif
    memset(&ccb, 0, sizeof ccb);

    ccb.cdblen = (*cmd<=0x1f)?6:10;	/* SCSI Command Descriptor Block Length */

    direction = 0;
    if (*cmd == READ_10 || *cmd == READ_6)
	direction = 8;
    else if (*cmd == WRITE_10 || *cmd == WRITE_6)
	direction = 16;

    memcpy(ccb.cdb, cmd, ccb.cdblen);
    ccb.op = 0;				/* SCSI Initiator Command */
    ccb.idlun = (target&7)<<5 | direction;  /* SCSI Target Id */
    ccb.rsalen = 12;
    any2scsi(ccb.datalen, bufflen);
    any2scsi(ccb.dataptr, buff);
    ccb.linkptr[0] = ccb.linkptr[1] = ccb.linkptr[2] = 0;
    ccb.commlinkid = 0;
    
    mb[0].status = 1;
    mb[1].status = 0;
    
#ifdef DEBUGd
    printk("aha1542_command: sending.. ");
    for (i = 0; i < sizeof(ccb)-10; i++)
      printk("%02x ", ((unchar *)&ccb)[i]);
#endif
    
    if (done) {
	DEB(printk("aha1542_queuecommand: now waiting for interrupt "); aha1542_stat());
	if (do_done)
	  printk("aha1542_queuecommand: Two concurrent queuecommand?\n");
	else
	  do_done = done;
	aha1542_out(&ahacmd, 1);		/* start scsi command */
	DEB(aha1542_stat());
	aha1542_enable_intr();
    }
    else
      printk("aha1542_queuecommand: done can't be NULL\n");
    
    return 0;
}

volatile static int internal_done_flag = 0;
volatile static int internal_done_errcode = 0;
static void internal_done(int host, int errcode)
{
    internal_done_errcode = errcode;
    ++internal_done_flag;
}

int aha1542_command(unchar target, const void *cmnd, void *buff, int bufflen)
{
    DEB(printk("aha1542_command: ..calling aha1542_queuecommand\n"));
    aha1542_queuecommand(target, cmnd, buff, bufflen, internal_done);

    while (!internal_done_flag);
    internal_done_flag = 0;
    return internal_done_errcode;
}

/* Initialize mailboxes */
static void setup_mailboxes()
{
    static unchar cmd[5] = {CMD_MBINIT, 1};
    
    mb[0].status = mb[1].status = 0;
    aha1542_intr_reset();		/* reset interrupts, so they don't block */	
    any2scsi((cmd+2), mb);
    any2scsi(mb[0].ccbptr, &ccb);
    aha1542_out(cmd, 5);
    WAIT(INTRFLAGS, INTRMASK, HACC, 0);
    while (0) {
      fail:
	printk("aha1542_detect: failed setting up mailboxes\n");
    }
    aha1542_intr_reset();
}

/* a hack to avoid a strange compilation error */

void call_buh()
{
    set_intr_gate(0x2b,&aha1542_interrupt);
}

/* Query the board to find out if it is a 1542 or a 1740, or whatever. */
static void aha1542_query(int hostnum)
{
  static unchar inquiry_cmd[] = {CMD_INQUIRY };
  static unchar inquiry_result[4];
  int i;
  i = inb(STATUS);
  if (i & DF) {
    i = inb(DATA);
  };
  aha1542_out(inquiry_cmd, 1);
  aha1542_in(inquiry_result, 4);
  WAIT(INTRFLAGS, INTRMASK, HACC, 0);
  while (0) {
  fail:
    printk("aha1542_detect: query card type\n");
  }
  aha1542_intr_reset();

/* For an AHA1740 series board, we select slower I/O because there is a
   hardware bug which can lead to wrong blocks being returned.  The slow
   I/O somehow prevents this.  Once we have drivers for extended mode
   on the aha1740, this will no longer be required.
*/

  if (inquiry_result[0] == 0x43) {
    slow_scsi_io = hostnum;
    printk("aha1542.c: Slow SCSI disk I/O selected for AHA 174N hardware.\n");
  };
}
/* return non-zero on detection */
int aha1542_detect(int hostnum)
{
    int i;

    DEB(printk("aha1542_detect: \n"));
    
    if (!(i = aha1542_test_port())) {
	return 0;
    }
 
    /* Set the Bus on/off-times as not to ruin floppy performens */
    {
	static unchar oncmd[] = {CMD_BUSON_TIME, 5};
	static unchar offcmd[] = {CMD_BUSOFF_TIME, 9};

	aha1542_intr_reset();
	aha1542_out(oncmd, 2);
	WAIT(INTRFLAGS, INTRMASK, HACC, 0);
	aha1542_intr_reset();
	aha1542_out(offcmd, 2);
	WAIT(INTRFLAGS, INTRMASK, HACC, 0);
	while (0) {
	  fail:
	    printk("aha1542_detect: setting bus on/off-time failed\n");
	}
	aha1542_intr_reset();
    }
    aha1542_query(hostnum);

    DEB(aha1542_stat());
    setup_mailboxes();

    DEB(aha1542_stat());

    DEB(printk("aha1542_detect: enable interrupt channel %d\n", intr_chan));
    call_buh();

    if (intr_chan >= 8)
      outb(inb_p(0x21)&0xfb,0x21);		/* open for slave ?? */

    DEB(printk("aha1542_detect: enabling interrupts\n"));
    aha1542_enable_intr();

#ifdef DEBUG
    DEB(printk(" *** READ CAPACITY ***\n"));

    {
	unchar buf[8];
	static unchar cmd[] = {	READ_CAPACITY, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i;
	
	for (i = 0; i < sizeof(buf); ++i) buf[i] = 0x87;
	for (i = 0; i < 2; ++i)
	  if (!aha1542_command(i, cmd, buf, sizeof(buf))) {
	      printk("aha_detect: LU %d sector_size %d device_size %d\n",
		     i, xscsi2int(buf+4), xscsi2int(buf));
	  }
    }

    DEB(printk(" *** NOW RUNNING MY OWN TEST *** \n"));

    for (i = 0; i < 4; ++i)
      {
	  unsigned char cmd[10];
	  static buffer[512];
	  
	  cmd[0] = READ_10;
	  cmd[1] = 0;
	  xany2scsi(cmd+2, i);
	  cmd[6] = 0;
	  cmd[7] = 0;
	  cmd[8] = 1;
	  cmd[9] = 0;
	  aha1542_command(0, cmd, buffer, 512);
      }
#endif
    aha1542_host = hostnum;
    return 1;
}

int aha1542_abort(int i)
{
    DEB(printk("aha1542_abort\n"));
    return 0;
}

int aha1542_reset(void)
{
    DEB(printk("aha1542_reset called\n"));
    return 0;
}

__asm__("
_aha1542_interrupt:
	cld
	pushl %eax
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	movl $0x17,%eax
	mov %ax,%fs
	movb $0x20,%al
	outb %al,$0xA0		# EOI to interrupt controller #1
	jmp 1f			# give port chance to breathe
1:	jmp 1f
1:	outb %al,$0x20
# Please, someone, change this to use the timer
#	andl $0xfffeffff,_timer_active
	movl $_aha1542_intr_handle,%edx
	call *%edx		# ``interesting'' way of handling intr.
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret
");
