/* $Id: 7000fasst.c,v 1.1 1992/07/24 06:27:38 root Exp root $
 *  linux/kernel/7000fasst.c
 *
 *  Copyright (C) 1992  Thomas Wuensche
 *	closely related to the aha1542 driver from Tommy Thorn
 *	( as close as different hardware allows on a lowlevel-driver :-) )
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

struct mailbox{
	unchar status;
	unchar scbptr[3];
};

/* #define DEBUG */

#include "7000fasst.h"
#ifdef DEBUG
#define DEB(x) x
#else
#define DEB(x)
#endif

/*static const char RCSid[] = "$Header: /usr/src/linux/kernel/blk_drv/scsi/RCS/7000fasst.c,v 1.1 1992/07/24 06:27:38 root Exp root $";*/

static struct scb scbs[OGMB_CNT];

long wd7000fasst_WAITnexttimeout = 3000000;

void (*wd7000fasst_do_done)() = NULL;
extern void wd7000fasst_interrupt();
void wd7000fasst_call_buh();

static unchar controlstat = 0;
static unchar wd7000fasst_hostno;

#define wd7000fasst_intr_reset()  outb(0,INTR_ACK)
#define PC_IMR	0x21
#define AT_IMR	0xa1

#define wd7000fasst_enable_intr(){\
	controlstat |= INT_EN;\
	outb(controlstat,CONTROL);\
	outb((inb((intr_chan<=7)?PC_IMR:AT_IMR))& ~0xff,(intr_chan<=7)?PC_IMR:AT_IMR);}

#define wd7000fasst_disable_intr() outb(controlstat |= INT_EN, CONTROL)
#define wd7000fasst_enable_dma() {\
	controlstat |= DMA_EN;\
	outb(controlstat,CONTROL);\
	outb((DMA_CH|CASCADE),DMA_MODE_REG);\
	outb(DMA_CH,DMA_MASK_REG);}

#define wd7000fasst_disable_dma() {\
	outb(DMA_CH|S_DMA_MASK,DMA_MASK_REG);\
	controlstat &= ~DMA_EN;\
	outb(controlstat,CONTROL);}

#define WAIT(port, mask, allof, noneof)					\
 { register WAITbits;							\
   register WAITtimeout = wd7000fasst_WAITnexttimeout;				\
   while (1) {								\
     WAITbits = inb(port) & (mask);					\
     if ((WAITbits & (allof)) == (allof) && ((WAITbits & (noneof)) == 0)) \
       break;                                                         	\
     if (--WAITtimeout == 0) goto fail;					\
   }									\
 }

static void wd7000fasst_stat(void)
{
/*    int s = inb(ASC_STAT), i = inb(INTR_STAT);*/
/*    printk("status = %x, intrflags = %x served %d last %x\n", s, i, intr_flag, intr_last); 
    printk("status=%x intrflags=%x\n", s, i);
*/}

static int wd7000fasst_out(unchar *cmdp, int len)
{
    while (len--)
      {
	  WAIT(ASC_STAT, STATMASK, CMD_RDY, 0);
	  outb(*cmdp++, COMMAND);
      }
    return 0;
  fail:
    printk("wd7000fasst_out failed(%d): ", len+1); wd7000fasst_stat();
    return 1;
}

int wd7000fasst_make_error_code(unsigned hosterr, unsigned scsierr)
{   
#ifdef DEBUG
    int in_error=hosterr;
#endif
    switch ((hosterr&0xff00)>>8){
	case 0:	/* It is reserved, should never happen */
		hosterr=DID_ERROR;
		break;
	case 1:	hosterr=DID_OK;
		break;
	case 2: /* Command complete with logged error */
		/* My actual copies of the manual pages are unreadable
		 * For now we simply tell there is an error */
		DEB(printk("Hosterror: VUE = %x\n",hosterr&0xff);)
		switch (hosterr&0xff) {
		    default:	DEB(printk("wd7000fasst_make_error_code: unknown hoststatus %x\n", hosterr);)
				hosterr=DID_ERROR;
		    		break;
	    	}
	    	break;
	case 4:	hosterr=DID_BAD_TARGET;	/* Command failed to complete without SCSI status */
		break;
	case 5: hosterr=DID_RESET;	/* Cmd terminated; Bus reset by external device */
		break;
	case 6:	hosterr=DID_ERROR;/* Hardware Failure, requires host reset */
		break;
	case 7: hosterr=DID_RESET;
		break;
	case 8:	hosterr=DID_OK;
		printk("wd7000fasst: Linked command not implemented\n");
		break;
	}
#ifdef DEBUG
    if (scsierr||hosterr) printk("SCSI-Command error: SCSI %x HOST %x RETURN %x\n",scsierr,in_error,hosterr);
#endif
    return scsierr|(hosterr << 16);
}

/* The following is space for the Mailboxes */
struct{ struct mailbox ombox[OGMB_CNT]; 
	struct mailbox imbox[ICMB_CNT]; } mbstruct;

int wd7000fasst_init(void)
{   int i;
    volatile int debug = 0;
    /* Page 47 */
    unchar init_block[]={ 1, 7, 0x18, 0x18, 0, 0, 0, 0, OGMB_CNT, ICMB_CNT };
    /* Reset the adapter. I ought to make a hard reset, but it's not really nessesary */
    
    DEB(printk("wd7000fasst_init called \n")); 
    
    outb(SCSI_RES|ASC_RES, CONTROL);
    /* Wait at least 25 us */
    for (i=0; i< 1000; i++) inb(ASC_STAT);
    /* Now reset the reset */
    outb(0,CONTROL);
    debug = 1;
    /* Expect Command Port Ready */
    WAIT(ASC_STAT, STATMASK, CMD_RDY, 0);
    DEB(printk("wd7000fasst_init: Power on Diagnostics finished\n"));
    if ((i=inb(INTR_STAT))!=1) 
	printk("Power on Diagnostics error %x\n",i); 
    
    debug = 2;
    /* Clear mbstruct */
    memset(&mbstruct,0,sizeof (mbstruct));
    /* Set up init block */
    any2scsi(init_block+5,&mbstruct);
    /* Execute init command */
    wd7000fasst_out(init_block,sizeof(init_block));
    DEB(printk("Init-Block :");
    for (i=0;i<sizeof(init_block);i++) printk(" %x",init_block[i]);
    printk("\n");)
    /* Wait until init finished */
    WAIT(ASC_STAT, STATMASK, CMD_RDY | ASC_INI, 0);
    outb(2,COMMAND);
    WAIT(ASC_STAT, STATMASK, CMD_RDY | ASC_INI, 0);
    /* Enable Interrupt and DMA */
    wd7000fasst_enable_dma();
    wd7000fasst_call_buh();
    DEB(printk("wd7000fasst_detect: enable interrupt channel %d\n", intr_chan));
    wd7000fasst_enable_intr(); 
    printk("wd7000fasst_init: Controller initialized\n");
    return 1;
  fail:
    return 0;					/* 0 = not ok */
}

/* What's this little function for? */
char *wd7000fasst_info(void)
{
    static char buffer[] = "Western Digital 7000-FASST";
    return buffer;
}

/* A "high" level interrupt handler */
void wd7000fasst_intr_handle(void)
{   struct scb * scbptr;
    DEB(int len=sizeof (struct scb);)
    DEB(int k;)
    unsigned host_error,scsi_error;
    int flag = inb(INTR_STAT);
    void (*my_done)() = wd7000fasst_do_done;
    int errstatus;
    DEB(printk("WD Interrupt aufgetreten\n"));
    if (!(inb(ASC_STAT)&0x80)){ 
	printk("Interrupt without Interrupt\n"); 
	wd7000fasst_intr_reset();
	return; 
    }
    wd7000fasst_do_done = NULL;
    if (!my_done) {
	printk("wd7000fasst_intr_handle: Unexpected interrupt\n");
	wd7000fasst_intr_reset();
	return;
    }

    /* is there mail for me :-) */
    if ((flag&0xc0)==0xc0){
	/* Ok, the interrupt is for an incoming mailbox */
	/* We make the content available for the starter  of the command */
	DEB(if ((flag&0xc0)==0xc0) printk("INTR_STAT: %x mbstat: %x\n",flag,mbstruct.imbox[flag&0x3f].status));
	if (mbstruct.imbox[flag&0x3f].status==0){
	    /* Something strange happened */
	    wd7000fasst_intr_reset();
	    return;
	    ;
	}
	scbptr=(struct scb *)scsi2int(mbstruct.imbox[flag&0x3f].scbptr);
	DEB(printk("Datenbereiche aus %x ein %x \n",scbptr,&(scbs[flag&0x3f]));
	printk("SCB after return:\n");
	k=0;
	while (len-- >0){
	    printk("%x ",*((unchar *)scbptr));
	    ((unchar *)scbptr)++;
	    if (++k==16){ printk("\n"); k=0; }
	});
    }
    else { printk("Error in interrupt\n"); return; }
    /* more error checking left out here */

    scbptr=(struct scb *)scsi2int(mbstruct.imbox[flag&0x3f].scbptr);
    host_error=scbptr->vue |  mbstruct.imbox[flag&0x3f].status<<8;
    scsi_error=scbptr->sretstat;
    errstatus=wd7000fasst_make_error_code(host_error,scsi_error);    
    DEB(if (errstatus) printk("Target was %x\n",scbptr->idlun>>5);)
    DEB(if (errstatus) printk("wd7000fasst_intr_handle: returning %6x\n", errstatus));
    DEB(printk("wd7000fasst_intr_handle: Status of the finished command: %x\n",mbstruct.imbox[flag&0x3f].status));
    /* I make a SCSI reset */
    /* Left out */
    my_done(wd7000fasst_hostno,errstatus);
    wd7000fasst_intr_reset();
    return;
}

volatile static int internal_done_flag = 0;
volatile static int internal_done_errcode = 0;

/* The following code queues a SCSI command */
int wd7000fasst_queuecommand(unchar target, const void *cmnd, void *buff, int bufflen, 
		void (*done)(int, int))
{
    int i;
#ifdef DEBUG
    int j;
#endif
    unchar *cmd = (unchar *) cmnd;
/* We first look for a free outgoing mailbox */
    for (i=0;i<OGMB_CNT;i++){
	if (mbstruct.ombox[i].status==0){
	    /* We found one, now set up the scb */
	    DEB(printk("Found outgoing mbox %x\n",i));
	    memset(&scbs[i], 0, sizeof(struct scb));
	    /* scbs[i].cdblen = (*cmd<=0x1f)?6:10; */	/* SCSI Command Descriptor Block Length */
	    memcpy(scbs[i].scbdata, cmd, (*cmd<=0x1f)?6:10);
	    scbs[i].op = 0;				/* SCSI Initiator Command */
	    scbs[i].idlun = (target<<5)&0xe0;		/* SCSI Target Id Bit 7-5 Target Id*/
	    any2scsi(scbs[i].dataptr,buff);
	    any2scsi(scbs[i].maxdata,bufflen);
	    scbs[i].direc=0x40;		/* Disable direction check */
	    DEB(printk("Kommando fuer target %x ist: ",target);
	    	for (j=0;j<12;j++) printk(" %x",scbs[i].scbdata[j]);
	    	printk("\n"));
	    /* Now we set up the pointer to scb, then the status of the mbox */
	    any2scsi((mbstruct.ombox[i].scbptr),&(scbs[i]));
	    mbstruct.ombox[i].status=1;
	    /* Everything set up, start the command */
	    break;
	}
    }
    if (i==OGMB_CNT){
	/* No free mbox, send command "Interrupt on free OGMB" */
	DEB(printk("No free Mailbox\n"));
	return 0;
    }
    {	int len,k;
	struct scb * scbptr;
	    DEB(printk("Found outgoing mbox %x\n",i));
	scbptr=&(scbs[i]);
	len=sizeof(struct scb);
	k=0;
	DEB(printk("SCB before execute:\n");
	    while (len-- >0){
	    	printk("%x ",*((unchar *)scbptr));
	    	((unchar *)scbptr)++;
	    	if (++k==16){ printk("\n"); k=0; }
	    };)
    }
    /* Set up the "done" response function */
    if (done) {
	DEB(printk("wd7000fasst_queuecommand: now waiting for interrupt "); 
	    wd7000fasst_stat());
	if (wd7000fasst_do_done)
	    printk("wd7000fasst_queuecommand: Two concurrent queuecommand?\n");
	else
	    wd7000fasst_do_done = done;
	DEB(wd7000fasst_stat());
	wd7000fasst_enable_intr(); 
    }
    else{
	printk("wd7000fasst_queuecommand: done can't be NULL\n");
	return 0;
    }
    /* Now we initialize execution */
retry:	WAIT(ASC_STAT,STATMASK,CMD_RDY,0);
	outb(0x80+i,COMMAND);
	WAIT(ASC_STAT,STATMASK,CMD_RDY,0);
	if (inb(ASC_STAT)&CMD_REJ) goto retry;
	return 1;
    /* Wait until done */

fail:
    return 0;
}

/* We use this function for queueing a command from wd7000fasst_command */ 
static void internal_done(int host, int errcode)
{
    internal_done_errcode = errcode;
    ++internal_done_flag;
}

int wd7000fasst_command(unchar target, const void *cmnd, void *buff, int bufflen)
{
#ifdef DEBUG
    int k;
#endif
    wd7000fasst_queuecommand(target, cmnd, buff, bufflen, internal_done);

    while (!internal_done_flag);
    internal_done_flag = 0;
    DEB(printk("wd7000fasst_command finished: ..leaving with errcode %x\n",
	internal_done_errcode));
    DEB(for (k=0;k<5000000;k++) inb(INTR_STAT));
    return internal_done_errcode;
}

/* a hack to avoid a strange compilation error */

void wd7000fasst_call_buh()
{
    set_intr_gate((intr_chan<=7)?intr_chan+8:intr_chan+0x20,&wd7000fasst_interrupt);
}

/* return non-zero on detection */
static const char *wd_bases[] = {(char *)0xce000};
typedef struct {char * signature;
		unsigned offset;
		unsigned length;
	       }Signature;

static const Signature signatures[] =
	      {{"SSTBIOS",0xd,0x7}};

#define NUM_SIGNATURES (sizeof(signatures)/sizeof(Signature))

int wd7000fasst_detect(int hostnum) /* hostnum ignored for now */
{
    int i,j;
    char const * base_address = 0;
    /* Store our host number */
    wd7000fasst_hostno=hostnum;
    DEB(printk("wd7000fasst_detect: \n"));

    for(i=0;i<(sizeof(wd_bases)/sizeof(char *));i++){
	for(j=0;j<NUM_SIGNATURES;j++){
	    if(!memcmp((void *)(wd_bases[i] + signatures[j].offset),
		(void *) signatures[j].signature,signatures[j].length)){
		    base_address=wd_bases[i];
		    printk("WD 7000-FASST detected\n");
	    }	
	}
    }
    if (!base_address) return 0;
    wd7000fasst_init();    

    /* Set the Bus on/off-times as not to ruin floppy performens */

    wd7000fasst_stat();

    printk(" *** READ CAPACITY ***\n");

    {   unchar rstat;
	unchar buf[8];
	static unchar cmd[] = {	READ_CAPACITY, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i;
	
	for (i = 0; i < sizeof(buf); ++i) buf[i] = 0x87;
	for (i = 0; i < 3; ++i){
	  rstat=0;
	  while (rstat<2){
	    if (wd7000fasst_command(i, cmd, buf, sizeof(buf))) rstat++;
	    else break;
	  }
	  if (rstat<2)
	      printk("wd7000fasst_detect: LU %d sector_size 0x%x device_size 0x%x capacity %d\n",
		i, xscsi2int(buf+4), xscsi2int(buf), xscsi2int(buf+4)*xscsi2int(buf));
	}
    }

    return 1;
}

int wd7000fasst_abort(int i)
{
    printk("wd7000fasst_abort\n");
    return 0;
}

int wd7000fasst_reset(void)
{
    printk("wd7000fasst_reset called\n");
    return 0;
}

__asm__("
_wd7000fasst_interrupt:
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
# Please, someone, change this to use the timer
#	andl $0xfffeffff,_timer_active
	movl $_wd7000fasst_intr_handle,%edx
	call *%edx		# ``interesting'' way of handling intr.
# Free the interrupt only after resetting the host interrupt
	movb $0x20,%al
	outb %al,$0xA0		# EOI to interrupt controller #1
	jmp 1f			# give port chance to breathe
1:	jmp 1f
1:	outb %al,$0x20
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret
");
