/* 3c503.c: A general NS8390 ethernet driver for linux. */
/*
    Copyright (C) 1992  Donald Becker

    This is alpha test code.  No general redistribution is permitted.
    This driver should work with the 3Com Etherlink II 3c503 and
    (hardwiring the ne2000 variable) the NE1000, NE2000, and NE2000 clones.

    The Author may be reached as becker@super.org or
    C/O Supercomputing Research Ctr., 17100 Science Dr., Bowie MD 20715
*/
#include <linux/config.h>
#ifdef EI8390
/*
  Braindamage remaining:

  I mostly do full dispatching on cards now, but you might still have to edit
  file until all of the probe code is working.  Ideally you could have a
  subset of the possible cards configured into the kernel and probed for
  at boot time.

  We should be able to support multiple, diverse boards
  simultaneousely.  Pass us 'kmem' at boot time like the drivers in
  chr_drv and I'll finish doing the minor bits to put all of the statics
  into a structure allocated per-board.  Mostly that means putting a few
  global statics into an allocated struct, and doing a gensym to get a
  new name "ei8390<0-9>".

  When everything is working right, I should make it possible to have a
  compile-time configuration of a single ethercard type, and not include
  the general code.

Sources:
  The National Semiconductor LAN Databook, and 3Com databooks, both companies
  provided information readily.  The NE* info came from the Clarkson packet
  driver, and figuring out that the those boards are similar to the NatSemi
  evaluation board described in AN-729.  Thanks NS, no thanks to Novell; buy
  a NE2000 _clone_ from someone beside Novell.
  */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/tty.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/string.h>
#include <asm/system.h>
#include <asm/segment.h>
/* If you are having flakey problems put this back in _before_ <io.h>. */
/* #define REALLY_SLOW_IO*/
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

#include "3c503reg.h"

/* These should be in <asm/io.h> someday, borrowed from blk_drv/hd.c. */

#define port_read(port,buf,nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr):"cx","di")
#define port_write(port,buf,nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr):"cx","si")

static unsigned short inline inw( unsigned short port )
{   unsigned short _v;
    __asm__ volatile ("inw %1,%0"
		      :"=a" (_v):"d" ((unsigned short) port));
   return _v;
}
static void inline outw( unsigned short value, unsigned short port )
{   __asm__ volatile ("outw %0,%1" ::"a" ((unsigned short) value),
		      "d" ((unsigned short) port));
}

#define EI_NAME "eth_if"
#define SM_TSTART_PG (ei_status.tx_start_page)
#define SM_RSTART_PG (ei_status.rx_start_page)
#define SM_RSTOP_PG (ei_status.stop_page)
#define ei_reset_8390 (ei_status.reset_8390)
#define ei_block_output (ei_status.block_output)
#define ei_block_input (ei_status.block_input)

#define NEEI_NAME "NE2000"		/* Unused for now. */
/* These are for the ne1000 and are untested */
#define NE1SM_TSTART_PG	0x20	/* First page of TX buffer */
#define NE1SM_RSTART_PG	0x26	/* Starting page of RX ring */
#define NE1SM_RSTOP_PG 	0x40	/* Last page +1 of RX ring */
#define NESM_TSTART_PG	0x40	/* First page of TX buffer */
#define NESM_RSTART_PG	0x46	/* Starting page of RX ring */
#define NESM_RSTOP_PG	0x80	/* Last page +1 of RX ring */

#define EL2EI_NAME "3c503"		/* Unused for now. */

#define EN_CMD (e8390_base)
#define E8390_BASE (e8390_base)

/* use 0 for production, 1 for devel., >2 for debug */
#ifdef EI_DEBUG
static int ei_debug = EI_DEBUG;
#else
static int ei_debug = 2;
#endif

static int e8390_base;

static struct device *eifdev;	/* Only for consistency checking.  */
static int etherlink2 = 0, ne2000 = 0, ne1000 = 0;
 
static struct {			/* These should be stored per-board */
  char *name;
  int exists:1;		/* perhaps in dev->private. */
  int open:1;
  int txing:1;		/* Transmit Active, don't confuse the 8390  */
  int word16:1;	/* We have the 16-bit (vs 8-bit) version of the card. */
  int in_interrupt;
  int overruns;		/* Rx overruns. */
  void (*reset_8390)(struct device *);
  void (*block_output)(struct device *, int, const unsigned char *);
  int  (*block_input)(struct device *, int, char *, int);
  int tx_start_page, rx_start_page, stop_page;
  int current_page;		/* Read pointer in buffer  */
  int thin_bit;		/* Value to write to the 3c503 E33G_CNTRL */
} ei_status = { "eth_if", 0, 0, 0, 0, 0, 0,
		  NULL, NULL, NULL, 0, 0, 0, 0, ECNTRL_THIN};
/* The statistics, perhaps these should be in the above structure. */
static int tx_packets = 0;
static int rx_packets = 0;
static int tx_errors = 0;
static int soft_rx_errors = 0;
static int soft_rx_err_bits = 0;

/* Index to functions. */
/* Put in the device structure. */
static int ei_open(struct device *dev);
static void ei_send_packet(struct sk_buff *skb, struct device *dev);
/* Dispatch from interrupts. */
static void ei_interrupt(int reg_ptr);
static void ei_tx_intr(struct device *dev);
static void ei_receive(struct device *dev);
static void ei_rx_overrun(struct device *dev);

/* Routines generic to NS8390-based boards. */
static void NS8390_init(struct device *dev);
static void NS8390_start();
static void NS8390_trigger_send(unsigned int length);

static int el2probe(int ioaddr, unsigned char station_addr[ETHER_ADDR_LEN]);
static void el2_reset_8390(struct device *dev);
static void el2_init_card(struct device *dev);
static void el2_block_output(struct device *dev, int count,
			     const unsigned char *buf);
static int el2_block_input(struct device *dev, int count, char *buf,
			   int ring_offset);

static int neprobe(int ioaddr, unsigned char station_addr[ETHER_ADDR_LEN]);
static void ne_reset_8390(struct device *dev);
static int ne_block_input(struct device *dev, int count,
			  char *buf, int ring_offset);
static void ne_block_output(struct device *dev, int count,
			    const unsigned char *buf);
static void ne_init_card(struct device *dev);

static struct sigaction ei_sigaction = { ei_interrupt, 0, 0, NULL, };

/* Open/initialize the board.  This routine goes all-out, setting everything
   up anew at each open, even though many of these registers should only
   need to be set once at boot.
   */
static int
ei_open(struct device *dev)
{
  if ( ! ei_status.exists) {
      printk(EI_NAME ": Opening a non-existent physical device\n");
      return 1;		/* We should have a better error return. */
  }
  if (etherlink2) {
      outb_p(ei_status.thin_bit, E33G_CNTRL);
  } else {
      ei_reset_8390(dev);
  }

  NS8390_init(dev);

  if (etherlink2) {
    /* Set ASIC copy of rx's first and last+1 buffer pages */
    /* This must be the same as in the 8390. */
    outb_p(EL2SM_RSTART_PG+1, E33G_STARTPG);
    outb_p(EL2SM_RSTOP_PG, E33G_STOPPG);
  }

  NS8390_start();

  ei_status.txing = 0;
  ei_status.in_interrupt = 0;
  ei_status.open = 1;
  if (etherlink2)
      outb(EGACFR_NORM, E33G_GACFR);   /* Enable RAM (redundent?) */
  return (0);
}

int
ei_start_xmit(struct sk_buff *skb, struct device *dev)
{
    if ( ! ei_status.exists)
	return 0;		/* We should have a better error return. */

    cli();
    if (ei_status.txing) {	/* Follow the 8003 driver and do timeouts. */
	if (jiffies - dev->trans_start < 30) {
	    sti();
	    return 1;
	}
	printk (EI_NAME ": transmit timed out with tx status %#2x.\n",
		inb(E8390_BASE+EN0_TSR));
	ei_reset_8390(dev);
    }
    sti();

    /* This is new: it means some higher layer thinks we've missed an
       tx-done interrupt. */
    if (skb == NULL) {
	/* Alternative is ei_tx_intr(dev); */
	ei_status.txing = 1;
	if (dev_tint(NULL, dev) == 0)
	    ei_status.txing = 0;
	return 0;
    }
    /* Fill in the ethernet header. */
    if (!skb->arp  &&  dev->rebuild_header(skb+1, dev)) {
	skb->dev = dev;
	arp_queue (skb);
	return 0;
    }

    dev->trans_start = jiffies;
    ei_status.txing = 1;
    ei_send_packet(skb, dev);
    if (skb->free)
#ifdef notdef			/* 0.98.5 and beyond use kfree_skb(). */
	free_skb (skb, FREE_WRITE);
#else
	kfree_skb (skb, FREE_WRITE);
#endif
    return 0;
}

/* The typical workload of the driver:
   Handle the ether interface interrupts. */
static void
ei_interrupt(int reg_ptr)
{
    int irq = -(((struct pt_regs *)reg_ptr)->orig_eax+2);
    struct device *dev;
    int interrupts, boguscount = 0;

    /* We do the same thing as the 8013 driver, but this is mostly bogus. */
    for (dev = dev_base; dev != NULL; dev = dev->next) {
	if (dev->irq == irq) break;
    }
    ei_status.in_interrupt++;
    sti(); /* Allow other interrupts. */

    /* If we a getting a reset-complete interrupt the 8390 might not be
       mapped in for the 3c503. */
    if (etherlink2)
	outb_p(ei_status.thin_bit, E33G_CNTRL), outb_p(0x00, E33G_STATUS);

    /* Change to page 0 and read the intr status reg. */
    outb_p(E8390_NODMA+E8390_PAGE0, EN_CMD);

    if (ei_status.in_interrupt > 1)
	printk(EI_NAME ": Reentering the interrupt driver!\n");
    if (dev == NULL) {
	printk (EI_NAME ": irq %d for unknown device\n", irq);
	ei_status.in_interrupt--;
	return;
    }
    /* !!Assumption!! -- we stay in page 0.  Don't break this. */
    while ((interrupts = inb_p(E8390_BASE + EN0_ISR)) != 0
	   && ++boguscount < 20) {
      /* The reset interrupt is the most important... */
      if (interrupts & ENISR_RDC) {
	outb_p(ENISR_RDC, E8390_BASE + EN0_ISR); /* Ack intr. */
      }
      if (interrupts & ENISR_OVER) {
	ei_status.overruns++;
	ei_rx_overrun(dev);
      } else if (interrupts & (ENISR_RX+ENISR_RX_ERR)) {
	/* Got a good (?) packet. */
	ei_receive(dev);
      }
      /* Push the next to-transmit packet through. */
      if (interrupts & ENISR_TX) {
	ei_tx_intr(dev);
      } else if (interrupts & ENISR_COUNTERS) {
	/* Gotta read the counter to clear the irq, even if we
	   don't care about their values. */
	inb_p(E8390_BASE + EN0_COUNTER0);
	inb_p(E8390_BASE + EN0_COUNTER1);
	inb_p(E8390_BASE + EN0_COUNTER2);
	outb_p(ENISR_COUNTERS, E8390_BASE + EN0_ISR); /* Ack intr. */
	outb_p(E8390_NODMA + E8390_PAGE0 + E8390_START, EN_CMD);
      }

      /* Ignore the transmit errs and reset intr for now. */
      if (interrupts & ENISR_TX_ERR) {
	outb_p(ENISR_TX_ERR, E8390_BASE + EN0_ISR); /* Ack intr. */
      }
      outb_p(E8390_NODMA + E8390_PAGE0 + E8390_START, EN_CMD);
    }

    if (interrupts && ei_debug) {
      printk(EI_NAME ": unknown interrupt %#2x\n", interrupts);
      outb_p(E8390_NODMA+E8390_PAGE0+E8390_START, EN_CMD);
      outb_p(0xff, E8390_BASE + EN0_ISR); /* Ack. all intrs. */
    }
    ei_status.in_interrupt--;
    return;
}

/* This is stuffed into the dev struct to be called by dev.c:dev_tint(). */
static void
ei_send_packet(struct sk_buff *skb, struct device *dev)
{
    int length = skb->len;

    if (length) {
	if (etherlink2)
	    outb(EGACFR_NORM, E33G_GACFR);   /* Enable RAM (redundent?) */
	ei_block_output(dev, length, (void*)(skb+1));
	NS8390_trigger_send(ETHER_MIN_LEN < length ? length : ETHER_MIN_LEN);
	tx_packets++;
    }
}

/* We have finished a transmit: check for errors and then put the next
   packet in the tx buffer. */
static void
ei_tx_intr(struct device *dev)
{
    int status = inb(E8390_BASE + EN0_TSR);
    outb_p(ENISR_TX, E8390_BASE + EN0_ISR); /* Ack intr. */
    if (status & ENTSR_PTX) {
	if (dev_tint(NULL, dev) == 0)
	    ei_status.txing = 0;
    } else {			/* Bogus!  Transmit error */
      tx_errors++;
    }
}

/* We have a good packet(s), get it/them out of the buffers. */

static void
ei_receive(struct device *dev)
{
    int rxing_page, this_frame, next_frame, current_offset;
    int boguscount = 0;
    struct e8390_pkt_hdr rx_frame;
    int state = 2;
    int num_rx_pages = ei_status.stop_page-ei_status.rx_start_page;

    while (state != 1 && ++boguscount < 10) {
      int size;

      cli();
      outb_p(E8390_NODMA+E8390_PAGE1, EN_CMD); /* Get the rec. page. */
      rxing_page = inb_p(E8390_BASE+EN1_CURPAG);/* (Incoming packet pointer).*/
      outb_p(E8390_NODMA+E8390_PAGE0, EN_CMD);
      sti();

      /* Remove one frame from the ring.  Boundary is alway a page behind. */
      this_frame = inb_p(E8390_BASE + EN0_BOUNDARY) + 1;
      if (this_frame >= ei_status.stop_page)
	this_frame = ei_status.rx_start_page;

      /* Someday we'll omit the previous step, iff we never get this message.*/
      if (ei_debug > 0  &&  this_frame != ei_status.current_page)
	printk(EI_NAME": mismatched read page pointers %2x vs %2x.\n",
	       this_frame, ei_status.current_page);

      if (this_frame == rxing_page) 	/* Read all the frames? */
	break;				/* Done for now */

      current_offset = this_frame << 8;
      ei_block_input(dev, sizeof(rx_frame), (void *)&rx_frame,
		     current_offset);

      size = rx_frame.count - sizeof(rx_frame);

      next_frame = this_frame + 1 + ((size+4)>>8);

      /* Check for bogosity warned by 3c503 book: the status byte is never
	 written.  This happened a lot during testing! This code should be
	 cleaned up someday, and the printk()s should be PRINTK()s. */
      if (   rx_frame.next != next_frame
	  && rx_frame.next != next_frame + 1
	  && rx_frame.next != next_frame - num_rx_pages
	  && rx_frame.next != next_frame + 1 - num_rx_pages) {

	static int last_rx_bogosity = -1;

	printk(EI_NAME": bogus packet header, status=%#2x nxpg=%#2x sz=%#x (at %#2x)\n",
	       rx_frame.status, rx_frame.next, rx_frame.count, this_frame);
	
	if (rx_packets != last_rx_bogosity
	    && ei_status.rx_start_page <= rx_frame.status
	    && rx_frame.status < ei_status.stop_page) {
	  /* Maybe we can continue without stopping... */
	  ei_status.current_page = rx_frame.status;
	  printk(EI_NAME":   setting next frame to %#2x (nxt=%#2x, rx_frm.nx=%#2x rx_frm.stat=%#2x).\n",
		 ei_status.current_page, next_frame,
		 rx_frame.next, rx_frame.status);
	  last_rx_bogosity = rx_packets;
	  outb(ei_status.current_page-1, E8390_BASE+EN0_BOUNDARY);
	  state = 0;
	  continue;
	} else {
	  /* Oh no Mr Bill! Last ditch error recovery. */
	  printk(EI_NAME": multiple sequential lossage, resetting at #%d..",
		 rx_packets);
	  ei_reset_8390(dev);
	  NS8390_init(dev);
	  NS8390_start(dev);
	  printk("restarting.\n");
	  return;
	}
      }

      if ((size < 32  ||  size > 1535) && ei_debug)
	printk(EI_NAME": bogus big packet, status=%#2x nxpg=%#2x size=%#x\n",
	       rx_frame.status, rx_frame.next, rx_frame.count);
      if ((rx_frame.status & 0x0F) == ENRSR_RXOK) {
	int sksize = sizeof(struct sk_buff) + size;
	struct sk_buff *skb;
	skb = kmalloc(sksize, GFP_ATOMIC);
	if (skb != NULL) {
	  skb->mem_len = sksize;
	  skb->mem_addr = skb;
	  /* 'skb+1' points to the start of sk_buff data area. */
	  ei_block_input(dev, size, (void *)(skb+1),
			 current_offset + sizeof(rx_frame));
	  state = dev_rint((void *)skb, size, IN_SKBUFF, dev);
	} else if (ei_debug) {
	  printk("Couldn't allocate a sk_buff of size %d.\n", sksize);
	  state = 0;
	  break;
	}
	if (state < 0) continue; /* Redo packet, dev_rint() screwed up */
	rx_packets++;
      } else {
	if (ei_debug)
	  printk(EI_NAME": bogus packet, status=%#2x nxpg=%#2x size=%d\n",
		 rx_frame.status, rx_frame.next, rx_frame.count);
	soft_rx_err_bits |= rx_frame.status,
	soft_rx_errors++;
	state = 0;
      }
      next_frame = rx_frame.next;

	 /* This should never happen, it's here for debugging. */
      if (next_frame >= ei_status.stop_page) {
	printk(EI_NAME": next frame inconsistency, %#2x..", next_frame);
	next_frame = ei_status.rx_start_page;
      }
      ei_status.current_page += 1 + ((size+4)>>8);
#ifdef notdef
      if (ei_status.current_page > ei_status.stop_page)
	ei_status.current_page -= ei_status.stop_page-ei_status.rx_start_page;
      if (ei_status.current_page != next_frame) {
	printk(EI_NAME": inconsistency in next_frame %#2x!=%#2x.\n",
	       this_frame, next_frame);
	/* Assume this packet frame is scrogged by the NIC, use magic to
	   skip to the next frame.  Actually we should stop and restart.*/
	next_frame = size > 1535 ? rx_frame.status : rx_frame.next;
	ei_status.current_page = next_frame;
	break;
      }
#endif

      ei_status.current_page = next_frame;
      outb(next_frame-1, E8390_BASE+EN0_BOUNDARY);
    }

    /* Tell the upper levels we're done. */
    while (state != 1 && ++boguscount < 20)
      state = dev_rint(NULL, 0, 0, dev);
    outb_p(ENISR_RX+ENISR_RX_ERR, E8390_BASE+EN0_ISR); /* Ack intr. */
    return;
}

/* We have a receiver overrun: we have to kick the 8390 to get it started
   again.  Overruns are detected on a per-256byte-page basis. */
static void
ei_rx_overrun(struct device *dev)
{
    int boguscount = 0;
    printk (EI_NAME ": receiver overrun\n");
    /* We should already be stopped and in page0.  Remove after testing. */
    outb_p(E8390_NODMA+E8390_PAGE0+E8390_STOP, EN_CMD);

#ifdef notdef
    /* The we.c driver does this.  Is it to speed up a reset? */
    {    unsigned char dummy;
	 dummy = inb_p( RBCR0 );
	 dummy = inb_p( RBCR1 );
     }
#endif
	
    /* Acknowledge the RX Interrupt.  Maybe we should also do RXOVERRUN? */
    if (inb_p(E8390_BASE+EN0_ISR) & ENISR_RX )
	outb_p(ENISR_RX, E8390_BASE+EN0_ISR);

    /* Wait for reset in case the NIX is doing a tx or rx. */
    while ((inb_p(E8390_BASE+EN0_ISR) & ENISR_RESET) == 0 )
	if (++boguscount > 150000) {
	    printk(EI_NAME": reset did not complete at ei_rx_overrun.\n");
	    NS8390_init(dev);
	    return;
	};
    /* We should clear out at least one received packet here.  */

    outb_p(ENISR_RESET, E8390_BASE+EN0_ISR); /* Ack Reset Intr. */
    /* Generic 8390 insns to start up again, same as in open_8390(). */
    outb_p(E8390_NODMA + E8390_PAGE0 + E8390_START, EN_CMD);
    outb_p(E8390_TXCONFIG, E8390_BASE + EN0_TXCR); /* xmit on. */
    outb_p(E8390_RXCONFIG, E8390_BASE + EN0_RXCR); /* rx on,  */
}

void
ethif_init(struct device *dev)
{
    int i;

    eifdev = dev;		/* Store for debugging. */
    e8390_base = dev->base_addr;


    if (el2probe(dev->base_addr, dev->dev_addr)) {
      ei_status.exists = 1;
      etherlink2 = 1;
      ei_status.tx_start_page = EL2SM_TSTART_PG;
      ei_status.rx_start_page = EL2SM_RSTART_PG;
      ei_status.stop_page = EL2SM_RSTOP_PG;
      ei_status.reset_8390 = &el2_reset_8390;
      ei_status.block_input = &el2_block_input;
      ei_status.block_output = &el2_block_output;
      el2_init_card(dev);
    } else if (neprobe(dev->base_addr, dev->dev_addr)) {
      ei_status.exists = 1;
      if (ne1000) {
	ne2000 = 0;
	ei_status.word16 = 0;
	ei_status.tx_start_page = NE1SM_TSTART_PG;
	ei_status.rx_start_page = NE1SM_RSTART_PG;
	ei_status.stop_page = NE1SM_RSTOP_PG;
      } else {
	ne2000 = 1;
	ei_status.word16 = 1;
	ei_status.tx_start_page = NESM_TSTART_PG;
	ei_status.rx_start_page = NESM_RSTART_PG;
	ei_status.stop_page = NESM_RSTOP_PG;
      }
      ei_status.reset_8390 = &ne_reset_8390;
      ei_status.block_input = &ne_block_input;
      ei_status.block_output = &ne_block_output;
      ne_init_card(dev);
    } else {
      /* What should I do here?  Erase dev->open? That crashes the kernel. */
      dev->open = &ei_open;
      printk("No ethernet device found found.\n");
      return;
    }

    /* Initialize the rest of the device structure. Most of these should
       be in Space.c. */
    for (i = 0; i < DEV_NUMBUFFS; i++)
	dev->buffs[i] = NULL;

    dev->hard_header = eth_hard_header;
    dev->add_arp = eth_add_arp;
    dev->queue_xmit = dev_queue_xmit;
    dev->rebuild_header = eth_rebuild_header;
    dev->type_trans = eth_type_trans;

    dev->send_packet = &ei_send_packet;
    dev->open = &ei_open;
    dev->hard_start_xmit = &ei_start_xmit;

    dev->type = ETHER_TYPE;
    dev->hard_header_len = sizeof (struct enet_header);
    dev->mtu = 1500; /* eth_mtu */
    dev->addr_len = ETHER_ADDR_LEN;
    for (i = 0; i < dev->addr_len; i++) {
	dev->broadcast[i]=0xff;
    }
    return;
}



/* This page of functions should be 8390 generic */
/* Follow National Semi's recommendations for initializing the "NIC". */
static void NS8390_init(struct device *dev)
{
  int i;
  int endcfg = ei_status.word16 ? (0x48 | ENDCFG_WTS) : 0x48;

  /* Follow National Semi's recommendations for initing the DP83902. */
  outb_p(E8390_NODMA+E8390_PAGE0+E8390_STOP, EN_CMD); /* 0x21 */
  outb_p(endcfg, E8390_BASE + EN0_DCFG); 	/* 0x48 or 0x49 */
  /* Clear the remote byte count registers. */
  outb_p(0x00,  E8390_BASE + EN0_RCNTLO);
  outb_p(0x00,  E8390_BASE + EN0_RCNTHI);
  /* Set to monitor and loopback mode -- this is vital!. */
  outb_p(E8390_RXOFF, E8390_BASE + EN0_RXCR); /* 0x20 */
  outb_p(E8390_TXOFF, E8390_BASE + EN0_TXCR); /* 0x02 */
  /* Set the transmit page and receive ring. */
  outb_p(SM_TSTART_PG,  E8390_BASE + EN0_TPSR);
  outb_p(SM_RSTART_PG,  E8390_BASE + EN0_STARTPG);
  outb_p(SM_RSTOP_PG-1, E8390_BASE + EN0_BOUNDARY); /* 3c503 says 0x3f,NS0x26*/
  ei_status.current_page = SM_RSTART_PG; 	    /* assert boundary+1 */
  outb_p(SM_RSTOP_PG,   E8390_BASE + EN0_STOPPG);
  /* Clear the pending interrupts and mask. */
  outb_p(0xFF, E8390_BASE + EN0_ISR);
  outb_p(0x00,  E8390_BASE + EN0_IMR);

  /* Copy the station address into the DS8390 registers,
     and set the multicast hash bitmap to receive all multicasts. */
  outb_p(E8390_NODMA + E8390_PAGE1 + E8390_STOP, EN_CMD); /* 0x61 */
  for(i = 0; i < 6; i++) {
    outb_p(dev->dev_addr[i], E8390_BASE + EN1_PHYS + i);
  }
  for(i = 0; i < 8; i++)
    outb_p(0xff, E8390_BASE + EN1_MULT + i);

  outb_p(SM_RSTART_PG,  E8390_BASE + EN1_CURPAG);
  outb_p(E8390_NODMA+E8390_PAGE0+E8390_STOP, EN_CMD);
  return;
}
/* Start the NIC: 8390 insns to turn on the xmit, rx, and irqs. */
static void
NS8390_start()
{
  outb_p(E8390_NODMA+E8390_PAGE0+E8390_STOP, EN_CMD);
  outb_p(0xff,  E8390_BASE + EN0_ISR);
  outb_p(ENISR_ALL,  E8390_BASE + EN0_IMR);
  outb_p(E8390_NODMA+E8390_PAGE0+E8390_START, EN_CMD);
  outb_p(E8390_TXCONFIG, E8390_BASE + EN0_TXCR); /* xmit on. */
  /* 3c503 TechMan says this should only be done after the NIC is started. */
  outb_p(E8390_RXCONFIG,  E8390_BASE + EN0_RXCR); /* rx on,  */
  return;
}

/* Trigger a transmit start, assuming the length is valid. */
static void NS8390_trigger_send(unsigned int length)
{
    struct device *dev = eifdev;
    /* Set everything up again, just to be certain. */
    if (etherlink2)
      outb_p(0x00, E33G_CNTRL);
    outb_p(E8390_NODMA+E8390_PAGE0, EN_CMD);

    if (inb_p(EN_CMD) & E8390_TRANS) {
      printk(EI_NAME": NS8390_send_packet() called with the transmitter still busy.\n");
      /* This might be a good place to bump a counter and recover. */
      return;
    }
    cli();
    outb_p(length & 0xff, E8390_BASE + EN0_TCNTLO);
    outb_p(length >> 8, E8390_BASE + EN0_TCNTHI);
    outb_p(SM_TSTART_PG, E8390_BASE + EN0_TPSR);
    outb_p(E8390_NODMA+E8390_TRANS+E8390_START, EN_CMD);
    outb_p(ENISR_RDC, E8390_BASE + EN0_ISR);
    sti();
    return;
}

/* Probe for the Etherlink II card at I/O port base IOADDR,
   returning non-zero on sucess. */
static int
el2probe(int ioaddr, unsigned char station_addr[ETHER_ADDR_LEN])
{
    int i, found;

    /* We check for a 3C503 board by checking the first three octets
       of its ethernet address. */
    printk("3c503 probe at %#3x:", ioaddr);
    outb_p(ECNTRL_RESET, ioaddr + 0x406); /* Reset it... */
    outb_p(0, ioaddr + 0x406);
    /* Map the station addr PROM into the lower I/O ports. */
    outb(ECNTRL_SAPROM, ioaddr + 0x406);
    for (i = 0; i < ETHER_ADDR_LEN; i++) {
	printk(" %2.2X", (station_addr[i] = inb(ioaddr + i)));
    }
    /* Map the 8390 back into the window. */
    outb(0, ioaddr + 0x406);
    found =(   station_addr[0] == 0x02
	    && station_addr[1] == 0x60
	    && station_addr[2] == 0x8c);
    printk("  3C503 %sfound.\n", found ? "":"not ");
    return found;
}

/* These functions are somewhat Clarkson packet driver-like. */
/* Right now this is only called when we have a transmit timeout. */
static void
el2_reset_8390(struct device *dev)
{
    /* Reset the board. */
    outb_p(ECNTRL_RESET | ei_status.thin_bit, E33G_CNTRL);
    outb(EGACFR_NORM, E33G_GACFR);	/* Enable RAM and irqs */
    outb_p(ei_status.thin_bit, E33G_CNTRL);
    ei_status.txing = 0;
    /* Doing a 'NS8390_init(dev)' here wouldn't hurt... */
 }

/* Initialize the card and fill in STATION_ADDR with the station address.
   The 3c503 maps the SA PROM into its lower I/O port bank. */
static void
el2_init_card(struct device *dev)
{
    int i, mem_jumpers;
    int pirq;

    /* Unmap the station PROM and turn on the thinnet connector. */
    outb(ei_status.thin_bit, E33G_CNTRL);
    /* Point the vector pointer registers somewhere ?harmless?. */
    outb(0xff, E33G_VP2);	/* Point at the ROM restart location 0xffff0 */
    outb(0xff, E33G_VP1);
    outb(0x00, E33G_VP0);
    /* Turn off all interrupts until we're opened. */
    outb_p(0x00,  E8390_BASE + EN0_IMR);
    outb_p(EGACFR_IRQOFF, E33G_GACFR);

    /* Probe for, turn on and clear the board's shared memory. */
    mem_jumpers = inb(E33G_ROMBASE);
    if (mem_jumpers & 0xf0 == 0) {
	dev->mem_start = 0;
	printk(EI_NAME": no shared memory mapped\n");
    } else {
	int *mem_base;
	dev->mem_start = ((mem_jumpers & 0xc0) ? 0xD8000 : 0xC8000) +
	    ((mem_jumpers & 0xA0) ? 0x4000 : 0);
	mem_base = (int *)dev->mem_start;
	printk(EI_NAME": shared memory at %#6X\n", mem_base);
	/* Check the card's memory. */
	for (i = 0; i < (SM_RSTOP_PG - SM_TSTART_PG)*256/sizeof(mem_base[0]);
	     i++) {
	    mem_base[i] = 0xAA425542;
	    if (mem_base[i] != 0xAA425542) {
		printk(EI_NAME": memory failure or memory address conflict.\n");
		dev->mem_start = 0;
		break;
	    }
	    mem_base[i] = 0;
	}
	/* Divide the on-board memory into a single maximum-sized transmit
	   (double-sized for ping-pong transmit) buffer at the base, and
	   use the rest as a receive ring. */
	dev->mem_end = dev->rmem_end = i * sizeof(mem_base[0]);
	dev->rmem_start = (SM_RSTART_PG - SM_TSTART_PG) * 256;
    }
    pirq = (dev->irq == 9 ? 2 : dev->irq);
    if (pirq > 5 || pirq < 2) {
	printk(EI_NAME": configured interrupt number %d out of range.\n",
	       dev->irq);
	return;			/* Return failure someday */
    } else if (irqaction (dev->irq, &ei_sigaction)) {
	printk (EI_NAME ": Unable to get IRQ%d.\n", dev->irq);
	return;			/* Return failure someday */
    } else {
      /* We could look for other free interrupts here, or something... */
    }
    outb_p((0x04 << pirq), E33G_IDCFR);	/* Set the interrupt line. */
    outb_p(8, E33G_NBURST);		/* Set burst size to 8 */
    outb_p(0x21, E33G_DMAAH);	/* Set up transmit bfr in DMA addr */
    outb_p(0x00, E33G_DMAAL);
    return;			/* Return success */
}

/* Either use the shared memory (if enabled on the board) or put the packet
   out through the ASIC FIFO.  The latter is probably much slower. */
static void
el2_block_output(struct device *dev, int count, const unsigned char *buf)
{
    int i;				/* Buffer index */
    int boguscount = 0;		/* timeout counter */
    if (dev->mem_start) {	/* Shared memory transfer */
	outb(EGACFR_NORM, E33G_GACFR);	/* Enable RAM */
	memcpy((char *)dev->mem_start, buf, count);
	return;
    }
    /* Set up then start the internal memory transfer to SM_TSTART_PG */
    outb(0x00, E33G_DMAAL);
    outb(SM_TSTART_PG, E33G_DMAAH);
    outb(ei_status.thin_bit | ECNTRL_OUTPUT | ECNTRL_START, E33G_CNTRL);

    /* This is the byte copy loop: it should probably be tuned for
       for speed once everything is working.  I think it is possible
       to output 8 bytes between checking the status bit. */
    for(i = 0; i < count; i++) {
	while (inb(E33G_STATUS) & ESTAT_DPRDY == 0)
	    if (++boguscount > 150000) {
		printk(EI_NAME": fifo blocked in el2_block_output.\n");
		return;
	    }
	outb(buf[i], E33G_FIFOH);
    }
    outb(ei_status.thin_bit, E33G_CNTRL);
    return;
}

/* Returns the new ring pointer. */
static int
el2_block_input(struct device *dev, int count, char *buf, int ring_offset)
{
    int boguscount = 0;
    int end_of_ring = dev->rmem_end;
    /* Maybe enable shared memory just be to be safe... nahh.*/
    if (dev->mem_start) {	/* Use the shared memory. */
	if (ring_offset + count >= end_of_ring) {
	    /* We must wrap the input move. */
	    int semi_count = end_of_ring - ring_offset; 
	    memcpy(buf, (char *)dev->mem_start + ring_offset, semi_count);
	    buf += semi_count, count -= semi_count;
	    ring_offset = dev->rmem_start;
	}
	memcpy(buf, (char *)dev->mem_start + ring_offset, count);
	return ring_offset + count;
    } else {	/* No shared memory, use the fifo. */
	int i;
	outb(ring_offset & 0xff, E33G_DMAAL);
	outb((ring_offset >> 8) & 0xff, E33G_DMAAH);
	outb(ei_status.thin_bit | ECNTRL_INPUT | ECNTRL_START, E33G_CNTRL);

	/* This is the byte copy loop: it should probably be tuned for
	   for speed once everything is working. */
	for(i = 0; i < count; i++) {
	    while (inb(E33G_STATUS) & ESTAT_DPRDY == 0)
		if (++boguscount > 150000) {
		    printk(EI_NAME": fifo blocked in el2_block_input().\n");
		    return 0;
		}
	    buf[i] = inb(E33G_FIFOH);
	}
	outb(ei_status.thin_bit, E33G_CNTRL);
	ring_offset += count;
	if (ring_offset >= end_of_ring)
	    ring_offset = dev->rmem_start + ring_offset - end_of_ring;
	return ring_offset;
    }
}

/* Routines for the NE2000. */
#define NE_BASE	 (e8390_base)
#define NE_DATAPORT	0x10	/* NE2000 Port Window. */
#define NE_RESET	0x1f	/* Issue a read for reset */

#define longpause()\
    do { int start_time = jiffies; while(jiffies-start_time < 18)  ;\
	 } while(0)

/*  Probe for the NE1000 and NE2000.  NE2000-like boards have 0x57,0x57 in
    bytes 0x0e,0x0f of the SAPROM.  I'm not certain yet what to do about
    the NE1000! */
/* Also initialize the card and fill in STATION_ADDR with the station address.
   The on-board data is stored where other cards have shared memory,
   32 bytes starting at remote DMA address 0. */

static int neprobe(int ioaddr, unsigned char station_addr[ETHER_ADDR_LEN])
{
  int i;
  unsigned char ne_board_data[32];
  int found, cmdreg;

  printk("NE2000 probing at %#3x: ", ioaddr);
  cmdreg = inb_p(ioaddr);
  if (cmdreg == 0xFF) {
    printk(" not found (%#2.2x).\n", cmdreg);
    return 0;
  }
  /* Ooops, we must first initialize registers -- we can't just read the PROM
     address right away.  (Learned the hard way.) */
  /* NS8390_init(eifdev);*/
  outb_p(E8390_NODMA+E8390_PAGE0+E8390_STOP, ioaddr);
  outb_p(0x49, ioaddr + EN0_DCFG); 		/* Set word-wide for probe. */
  /* Even though we'll set them soon, we must clear them! */
  outb_p(0x00, ioaddr + EN0_RCNTLO);
  outb_p(0x00, ioaddr + EN0_RCNTHI);

  outb_p(0x00, ioaddr + EN0_IMR); 		/* Mask completion irq. */
  outb_p(0xFF, E8390_BASE + EN0_ISR);

  /* Set to monitor and loopback mode. */
  outb_p(E8390_RXOFF, E8390_BASE + EN0_RXCR); /* 0x20 */
  outb_p(E8390_TXOFF, E8390_BASE + EN0_TXCR); /* 0x02 */


  /* Double count 0x20 words, the SA PROM is only byte wide. */
  outb_p(2*sizeof(ne_board_data), ioaddr + EN0_RCNTLO);
  outb_p(0x00, ioaddr + EN0_RCNTHI);
  outb_p(0x00, ioaddr + EN0_RSARLO);	/* DMA starting at 0x0000. */
  outb_p(0x00, ioaddr + EN0_RSARHI);
  outb_p(E8390_RREAD+E8390_START, ioaddr);
  for(i = 0; i < sizeof(ne_board_data); i++) {
    ne_board_data[i] = inb_p(ioaddr + NE_DATAPORT);
    if (i < ETHER_ADDR_LEN  &&  station_addr) {
      printk(" %#2.2x", ne_board_data[i]);
      station_addr[i] = ne_board_data[i];
    }
  }
  found = ne_board_data[14] == 0x57  &&  ne_board_data[15] == 0x57;
  printk(" %sfound.\n", found ? "" : "not ");
  return found;
}

static void
ne_reset_8390(struct device *dev)
{
    int tmp = inb_p(NE_BASE + NE_RESET);
    ei_status.txing = 0;
    longpause();		/* We should wait a few jiffies... */
    outb_p(tmp, NE_BASE + NE_RESET);
}

/* Block input and output, based on the Clarkson packet driver.
   The NE2000 doesn't have shared memory on the board -- put the packet
   out through the ASIC FIFO.  This is probably very slow. */

static int
ne_block_input(struct device *dev, int count, char *buf, int ring_offset)
{

    outb_p(E8390_PAGE0+E8390_START, EN_CMD);
    outb_p(count & 0xff, E8390_BASE + EN0_RCNTLO);
    outb_p(count >> 8, E8390_BASE + EN0_RCNTHI);
    outb_p(ring_offset & 0xff, E8390_BASE + EN0_RSARLO);
    outb_p(ring_offset >> 8, E8390_BASE + EN0_RSARHI);
    outb_p(E8390_RREAD+E8390_START, EN_CMD);
    if (!ne1000) {
      port_read(E8390_BASE + NE_DATAPORT,buf,count>>1);
      if (count & 0x01)
	buf[count-1] = inb(E8390_BASE + NE_DATAPORT);
    } else {
      int i;
      /* Input the bytes with a slow 8-bit loop.  Tune this someday. */
      for(i = 0; i < count; i++) {
	buf[i] = inb_p(E8390_BASE + NE_DATAPORT);
      }
    }
    return ring_offset + count;
}

/* Issues to be resolved for the NE1000: we don't need to round the byte
   count up, and the dummy read only needs (count+1), not (count+2). */
static void
ne_block_output(struct device *dev, int count, const unsigned char *buf)
{
    int i;
    int boguscount = 0, tries = 0;

    /* Round the count up. This is for word writes on the NE2000, but do
       we need to do it?  What effect will an odd byte count have on the
       8390? */
    if (count & 0x01)
      count++;
  restart:
    boguscount = 0;
    /* We should already be in page 0, but to be safe... */
    outb_p(E8390_PAGE0+E8390_START, EN_CMD);

    /* This following dummy-read-first sequence is suggested by
       NatSemi to avoid a bug in the 8390. */
    outb_p((count+2) & 0xff, NE_BASE + EN0_RCNTLO);
    outb_p((count+2) >> 8, NE_BASE + EN0_RCNTHI);

    outb_p(0xfe, NE_BASE + EN0_RSARLO);
    outb_p(ei_status.tx_start_page-1, NE_BASE + EN0_RSARHI);

    outb_p(E8390_RREAD+E8390_START, EN_CMD);

    /* Make certain that the dummy read has occured -- strange bugs can
       occur otherwise. */
    while (inb_p(NE_BASE + EN0_RSARLO) != 0x00)
      if (++boguscount > 10) {
	printk(EI_NAME": Failed to write packet (addrlo=%#2x).\n",
	       inb_p(NE_BASE + EN0_RSARLO));
	if (++tries < 3)
	  goto restart;		/* 8390 burped? */
	return;			/* Splat... drop the packet..should reset? */
      }

    outb_p(E8390_RWRITE+E8390_START, EN_CMD);
    if (ne1000) {
      /* Output the bytes with a slow 8-bit loop.  This actually is almost
	 as fast as possible, but it does tie up the processor. */
      for(i = 0; i < count; i++)
	outb_p(buf[i], NE_BASE + NE_DATAPORT);
    } else			/* NE2000 */
      /* Use the 'rep' sequence for the NE2000. */
      port_write(E8390_BASE + NE_DATAPORT, buf, count>>1);

    while (inb_p(EN0_ISR) & ENISR_RDC == 0) /* DMA done? */
      if (++boguscount > 15000) {
	printk(EI_NAME": Timed out doing ne_block_output(%d).\n", count);
	return;
      }
    /* This is for the ALPHA version only, remove for later releases. */
    if (ei_debug > 0) {		/* DMA termination address check... */
      int low = inb_p(NE_BASE + EN0_RSARLO);
      int high = inb_p(NE_BASE + EN0_RSARHI);
      int addr = (high << 8) + low;
      if ((ei_status.tx_start_page << 8) + count != addr)
	printk(EI_NAME": TX Transfer address mismatch, %#4.4x vs. %#4.4x.\n",
	       (ei_status.tx_start_page << 8) + count, addr);
    }
    return;
}

/* This function resets the ethercard if something screws up. */
static void
ne_init_card(struct device *dev)
{


  NS8390_init(dev);
  /* Snarf the interrupt.  We could share or wait until open(), but... */
  { int irqval = irqaction (dev->irq, &ei_sigaction);
    if (irqval)
      printk (EI_NAME": unable to get IRQ%d, error=%d.\n", dev->irq, irqval);
  }
  return;
}

/*
 * Local variables:
 *  compile-command: "gcc -DKERNEL -Wall -O6 -fomit-frame-pointer -DEI_DEBUG=3 -I/usr/linux-master/net/tcp -c -o 3c503.o 3c503.c"
 *  version-control: t
 *  kept-new-versions: 5
 * End:
 */
#endif  /* EI8390 */
