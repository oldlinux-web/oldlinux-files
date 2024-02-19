/* 3c501.c: A 3Com 3c501 ethernet driver for linux. */
#include <linux/config.h>
#ifdef ETHERLINK1
/*
    Copyright (C) 1992  Donald Becker

    This is alpha test code.  No general redistribution is permitted.
    This is a device driver for the 3Com Etherlink 3c501.
    Do not purchase this card, even as a joke.  It's performance is horrible,
    and it breaks in many ways.  

    The Author may be reached as becker@super.org or
    C/O Supercomputing Research Ctr., 17100 Science Dr., Bowie MD 20715
*/

/*
  Braindamage remaining:
  The 3c501 board.
  */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/tty.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/string.h>
#include <asm/system.h>
#include <asm/segment.h>
/* This should be checked for necessity after testing. */
#define REALLY_SLOW_IO
#include <asm/io.h>
#include <errno.h>
#include <linux/fcntl.h>
#include <netinet/in.h>

#include "dev.h"
#include "eth.h"
#include "timer.h"
#include "ip.h"
#include "tcp.h"
#include "sock.h"
#include "arp.h"

/* These should be in <asm/io.h> someday, borrowed from blk_drv/hd.c. */

#define port_read_b(port,buf,nr) \
__asm__("cld;rep;insb"::"d" (port),"D" (buf),"c" (nr):"cx","di")
#define port_write_b(port,buf,nr) \
__asm__("cld;rep;outsb"::"d" (port),"S" (buf),"c" (nr):"cx","si")

#define DEBUG  10	/* use 0 for production, 1 for devel., >2 for debug */


/* Index to functions. */
static void el_interrupt(int reg_ptr);
static void el_send_packet(struct sk_buff *skb, struct device *dev);
static void el_receive(struct device *dev);

#define EL_NAME "EtherLink 3c501"

static int el_debug = DEBUG;	/* Anything above 5 is wordy death! */
static int el_base;
static struct device *eldev;	/* Only for consistency checking.  */
 
/* We could put everything in a struct to be malloc()ed per-board, but
   who would want more than one 3c501?. */
static struct {			/* This should be stored per-board */
  char *name;
  int exists:1;		/* perhaps in dev->private. */
  int open:1;
  int txing:1;		/* Transmit Active, don't confuse the 8390  */
  int in_interrupt:4;
  int overruns, rx_errors, tx_errors;
} el_status = { "Etherlink I", 0, 0, 0, 0, 0, 0, 0 };

static int collisions;		/* Tx collisions this packet */
static int tx_pkt_length;	/* To reset GP after each collision. */
static int runt_packets = 0;	/* # of runt packets picked up so far. */
/* static int rx_missed = 0;	 * # of packets we missed. */

static struct sigaction el_sigaction = { el_interrupt, 0, 0, NULL, };

#define RX_STATUS (el_base + 0x06)
#define TX_STATUS (el_base + 0x07)
#define GP_LOW 	  (el_base + 0x08)
#define GP_HIGH   (el_base + 0x09)
#define RX_LOW	  (el_base + 0x0A)
#define RX_HIGH   (el_base + 0x0B)
#define SAPROM	  (el_base + 0x0C)
#define AX_STATUS (el_base + 0x0E)
#define DATAPORT  (el_base + 0x0F)
#define TX_RDY 0x08		/* In TX_STATUS */

/* Writes to the ax command register. */
#define AX_OFF	0x40			/* Irq off, buffer access on */
#define AX_SYS  0x41			/* Load the buffer */
#define AX_XMIT 0x45			/* Transmit a packet */
#define AX_RX	0x49			/* Receive a packet */
#define AX_LOOP	0x4D			/* Loopback */

/* Normal receive mode written to RX_STATUS.  We must intr on short packets
   to avoid bogus rx lockups. */
#define RX_NORM 0xA8
/* TX_STATUS register. */
#define TX_COLLISION 0x02

/* Open/initialize the board. */
static int
el_open(struct device *dev)
{
  if ( ! el_status.exists) {
      printk(EL_NAME ": Opening a non-existent physical device\n");
      return 1;		/* We should have a better error return. */
  }
  if (el_debug > 2)
      printk(EL_NAME ": Doing el_open(%s)...",
	     dev == eldev ? dev->name : " on unknown dev");
  el_status.txing = 0;
  el_status.in_interrupt = 0;

  
  outb_p(AX_LOOP, AX_STATUS);	/* Aux control, irq and loopback enabled */
  outb_p(0x00, RX_LOW);		/* Set rx packet area to 0. */
  outb_p(RX_NORM, RX_STATUS);	/* Set Rx commands. */
  outb_p(AX_RX, AX_STATUS);	/* Aux control, irq and receive enabled */

  el_status.open = 1;
  if (el_debug > 2)
     printk("finished el_open().\n");
  return (0);
}

int
el_start_xmit(struct sk_buff *skb, struct device *dev)
{
    int axsr;

    if ( ! el_status.exists)
	return 0;		/* We should have a better error return. */
    if (el_debug > 2)
	printk(EL_NAME": Doing el_start_xmit(<sk_buff%x>,%s).\n", skb,
	       dev == eldev ? "EtherLink" : "unknown dev");
    cli();
    if (el_debug > 4) printk(EL_NAME": interrupts suspended...");
    axsr = inb_p(AX_STATUS);
    if (el_status.txing || axsr & 0x01) {
	if (jiffies - dev->trans_start < 30) {
	    sti();
	    if (el_debug > 2)
		printk(EL_NAME": transmit deferred, no timeout.\n");
	    return 1;
	}
	printk (EL_NAME ": transmit timed out with tx status %#2x.\n",
		inb(TX_STATUS));
    }
    if (el_debug > 4) printk("doing sti()...");
    sti();
    if (el_debug > 4) printk("filling in hdr...");

    /* This is new: it means some higher layer thinks we've missed an
       tx-done interrupt. */
    if (skb == NULL) {
	/* Alternative is ei_tx_intr(dev); */
	el_status.txing = 1;
	if (dev_tint(NULL, dev) == 0)
	    el_status.txing = 0;
	return 0;
    }
    /* Fill in the ethernet header. */
    if (!skb->arp  &&  dev->rebuild_header(skb+1, dev)) {
	skb->dev = dev;
	arp_queue (skb);
	if (el_debug > 1)
	  printk("    Exiting from xmit_start() via rebuild header?\n");
	return 0;
    }
    dev->trans_start = jiffies;
    el_status.txing = 1;
    outb_p(0x0A, TX_STATUS);		/* tx irq on done, collision */
    el_send_packet(skb, dev);
    if (skb->free)
	kfree_skb (skb, FREE_WRITE);
    if (el_debug > 3)
	printk(EL_NAME": Returning from el_start_xmit().\n");
    return 0;
}

/* The typical workload of the driver:
   Handle the ether interface interrupts. */
static void
el_interrupt(int reg_ptr)
{
    int irq = -(((struct pt_regs *)reg_ptr)->orig_eax+2);
    int axsr;			/* Aux. status reg. */
    int txsr;			/* Tx. status reg. */
    int rxsr;			/* Rx. status reg. */

    if (eldev->irq != irq) {
	printk (EL_NAME ": irq %d for unknown device\n", irq);
	return;
    }

    el_status.in_interrupt++;
    sti(); /* Turn interrupts back on. */
    axsr = inb_p(AX_STATUS);
    txsr = inb_p(TX_STATUS);
    rxsr = inb_p(RX_STATUS);
    if (el_debug > 1)
      printk(EL_NAME": in el_interrupt(), axsr=%#2x, txsr=%#2x, rxsr=%#2x.\n",
	     axsr, txsr, rxsr);
    if (el_status.in_interrupt > 1) {
	/* We should probably return here -- the 3c501 glitches the
	   interrupt line when you write to the rx or tx command register. */
	printk(EL_NAME ": Reentering the interrupt driver!\n");
    }
    if (rxsr & 0x08)
	runt_packets++;			/* Just reading rxstatus fixes this. */
    else if (rxsr & 0x20)
	el_receive(eldev);
    else if (txsr & TX_COLLISION) {
	if (++collisions > 16) {
	    printk(EL_NAME": Transmit failed 16 times, ethernet jammed?\n");
	    /* Turn receiving back on. */
	    el_status.txing = 0;
	    outb_p(0x00, RX_LOW);
	    outb_p(AX_RX, AX_STATUS);
	} else {			/* Retrigger xmit. */
	    int gp_start = 0x800 - tx_pkt_length;
	    outb_p(gp_start, GP_LOW);
	    outb_p(gp_start>>8, GP_HIGH);
	    outb_p(AX_XMIT, AX_STATUS);
	}
    } else if (txsr & TX_RDY) {
	if (dev_tint(NULL, eldev) == 0)
	    el_status.txing = 0;	/* We could turn off the tx... */
    }
    el_status.in_interrupt--;
    return;
}

/* This is stuffed into the dev struct to be called by dev.c:dev_tint(). */
static void
el_send_packet(struct sk_buff *skb, struct device *dev)
{
    tx_pkt_length = skb->len;
    collisions = 0;
    if (el_debug > 3) printk(" el_send_packet(%d)...", tx_pkt_length);
    /* Should we check for tiny (or huge) lengths here? */
    if (tx_pkt_length) {
	int gp_start = 0x800 - tx_pkt_length;
	unsigned char *buf = (void *)(skb+1);
	outb_p(AX_OFF, AX_STATUS); 		/* irq disabled, rx off  */
	outb_p(gp_start, GP_LOW);
	outb_p(gp_start>>8, GP_HIGH);
	/* After testing use port_write(), defined above. */
	for (; gp_start < 0x800; gp_start++)
	    outb_p(*buf++, DATAPORT);
	outb_p(AX_XMIT, AX_STATUS);		/* Trigger xmit.  */
    }
}

/* We have a good packet; well, not really "good", just mostly not broken.
   We must check everything to see if it is good. */
static void
el_receive(struct device *dev)
{
    int state = 0, sksize, length;
    struct sk_buff *skb;

    if (el_debug > 2)
      printk("in el_receive...");
    /* Painfully read it out of the local memory. */
    outb_p(AX_SYS, AX_STATUS);
    length = inb_p(RX_LOW) + (inb_p(RX_HIGH)<<8);
    if ((length < 60  ||  length > 1535)) {
	if (el_debug)
	  printk(EL_NAME": bogus packet, length=%d\n", length);
	/* We should reset to receive... */
	return;
    }
    sksize = sizeof(struct sk_buff) + length;
    skb = kmalloc(sksize, GFP_ATOMIC);
    outb_p(0x00, GP_LOW);
    outb_p(0x00, GP_HIGH);
    if (skb != NULL) {
	unsigned char *buf = (void *)(skb+1);
	skb->mem_len = sksize;
	skb->mem_addr = skb;
	/* After testing use port_read(), defined above. */
	while (length-- > 0)
	    *buf++ = inb_p(DATAPORT);
	state = dev_rint((void *)skb, length, IN_SKBUFF, dev);
    } else if (el_debug) {
	printk("Couldn't allocate a sk_buff of size %d.\n", sksize);
    }
    dev_rint(NULL, 0,0, dev);		/* Inform upper level */
    if (el_debug > 2)
	printk("done.\n");
    return;
}

void
etherlink_init(struct device *dev)
{
    int i;

    eldev = dev;		/* Store for debugging. */
    el_base = dev->base_addr;

    printk("3c501 probe at %#3.3x: ", el_base);
    outb(0x00, GP_HIGH);
    for (i = 0; i < 6; i++) {
	outb(i, GP_LOW);	/* Set station address prom addr */
	dev->dev_addr[i] = inb(SAPROM);	/* Read Station address prom */
	printk(" %2.2x", dev->dev_addr[i]);
    }
    /* Check the first three octets of the S.A. for 3Com's code. */ 
    if (dev->dev_addr[0] != 0x02  ||  dev->dev_addr[1] != 0x60
	|| dev->dev_addr[2] != 0x8c) {
	printk(" Etherlink not found.\n", dev->base_addr);
	el_status.exists = 0;
	return;
    }
    el_status.exists = 1;
    printk(" Etherlink found.\n");

    /* Initialize the rest of the device structure. Most of these should
       be in Space.c. */
    for (i = 0; i < DEV_NUMBUFFS; i++)
	dev->buffs[i] = NULL;

    dev->hard_header = eth_hard_header;
    dev->add_arp = eth_add_arp;
    dev->queue_xmit = dev_queue_xmit;
    dev->rebuild_header = eth_rebuild_header;
    dev->type_trans = eth_type_trans;

    dev->send_packet = &el_send_packet;
    dev->open = &el_open;
    dev->hard_start_xmit = &el_start_xmit;

    dev->type = ETHER_TYPE;
    dev->hard_header_len = sizeof (struct enet_header);
    dev->mtu = 1500; /* eth_mtu */
    dev->addr_len = ETHER_ADDR_LEN;
    for (i = 0; i < dev->addr_len; i++) {
	dev->broadcast[i]=0xff;
    }
    /* Turn off interrupts. */

    /* Snarf the assigned interrupt. */
    {   int irqval = irqaction (dev->irq, &el_sigaction);
	if (irqval) {
	    printk (" unable to get IRQ%d, error=%d.\n", dev->irq, irqval);
	    return;			/* Return failure someday */
	}
    }
    outb(0x8C, 0x0e);				/* setup Aux. control reg. */
    return;
}
#endif  /* ETHERLINK1 */

/*
 * Local variables:
 *  compile-command: "gcc -DKERNEL -Wall -O6 -fomit-frame-pointer -DMAX_16M -I/usr/linux-master/net/tcp -c -o 3c501.o 3c501.c"
 * End:
 */
