/* 8390.c: A general NS8390 ethernet driver core for linux. */
/*
    Written 1992,1993 by Donald Becker. This is alpha test code.
    This is a extension to the Linux operating system, and is covered by
    same Gnu Public License that covers that work.
    
    This driver should work with many 8390-based ethernet adaptors.

    The Author may be reached as becker@super.org or
    C/O Supercomputing Research Ctr., 17100 Science Dr., Bowie MD 20715
*/

static char *version =
    "8390.c:v0.43 2/12/93 for 0.99.5+ Donald Becker (becker@super.org)\n";
#include <linux/config.h>
#if !defined(EL2) && !defined(NE2000) && !defined(WD80x3) && !defined(HPLAN)
/* They don't know what they want -- give it all to them! */
#define EL2
#define NE2000
#define WD80x3
#define HPLAN
#endif

/*
  Braindamage remaining:

  Ethernet devices should use a chr_drv device interface, with ioctl()s to
  configure the card, bring the interface up or down, allow access to
  statistics, and maybe read() and write() access to raw packets.
  This won't be done until after Linux 1.00.

  This driver should support multiple, diverse boards simultaneousely.
  This won't be done until after Linux 1.00.

Sources:
  The National Semiconductor LAN Databook, and 3Com databooks, both companies
  provided information readily.  The NE* info came from the Crynwr packet
  driver, and figuring out that the those boards are similar to the NatSemi
  evaluation board described in AN-729.  Thanks NS, no thanks to Novell/Eagle.
  Cabletron provided only info I had already gotten from other sources -- hiss.
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
#include <asm/io.h>
#include <errno.h>
#include <linux/fcntl.h>
#include <netinet/in.h>
#include <linux/interrupt.h>

#include "dev.h"
#include "eth.h"
#include "timer.h"
#include "ip.h"
#include "tcp.h"
#include "sock.h"
#include "arp.h"

#include "8390.h"

#define ei_reset_8390 (ei_status.reset_8390)
#define ei_block_output (ei_status.block_output)
#define ei_block_input (ei_status.block_input)

#define EN_CMD (e8390_base)
#define E8390_BASE (e8390_base)

/* use 0 for production, 1 for verification, >2 for debug */
#ifdef EI_DEBUG
int ei_debug = EI_DEBUG;
#else
int ei_debug = 2;
#endif

static int e8390_base;

static struct device *eifdev;	/* For single-board consistency checking.  */
extern int etherlink2;

struct ei_device ei_status = { EI_NAME, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The statistics, perhaps these should be in the above structure. */
static int tx_packets = 0;
static int rx_packets = 0;
static int tx_errors = 0;
static int soft_rx_errors = 0;
static int soft_rx_err_bits = 0;
static int missed_packets = 0;
static int rx_overrun_packets = 0;
/* Max number of packets received at one Intr. */
/*static int high_water_mark = 0;*/

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
void NS8390_init(struct device *dev, int startp);
static void NS8390_trigger_send(struct device *dev, unsigned int length,
				int start_page);

extern int el2autoprobe(int ioaddr, struct device *dev);
extern int el2probe(int ioaddr, struct device *dev);
extern int neprobe(int ioaddr, struct device *dev);
extern int wdprobe(int ioaddr, struct device *dev);
extern int hpprobe(int ioaddr, struct device *dev);

struct sigaction ei_sigaction = { ei_interrupt, 0, 0, NULL, };

/* Open/initialize the board.  This routine goes all-out, setting everything
   up anew at each open, even though many of these registers should only
   need to be set once at boot.
   */
static int
ei_open(struct device *dev)
{
  if ( ! ei_status.exists) {
      printk(EI_NAME ": Opening a non-existent physical device\n");
      return 1;		/* ENXIO would be more accurate. */
  }

  NS8390_init(dev, 1);

  /* The old local flags... */
  ei_status.txing = 0;
  ei_status.in_interrupt = 0;
  ei_status.open = 1;
  /* ... are now global. */
  dev->tbusy = 0;
  dev->interrupt = 0;
  dev->start = 1;
  return 0;
}

static int
ei_start_xmit(struct sk_buff *skb, struct device *dev)
{
    if ( ! ei_status.exists)
	return 0;		/* We should be able to do ENODEV, but nooo. */

    if (ei_status.txing) {	/* Do timeouts, just like the 8003 driver. */
	int txsr = inb(E8390_BASE+EN0_TSR);
	int tickssofar = jiffies - dev->trans_start;
	if (tickssofar < 5  ||  (tickssofar < 15 && ! (txsr & ENTSR_PTX))) {
	    return 1;
	}
	printk(EI_NAME": transmit timed out, TX status %#2x, ISR %#2x.\n",
		txsr, inb(E8390_BASE+EN0_ISR));
	/* It's possible to check for an IRQ conflict here.
	   I may have to do that someday. */
	if ((txsr & ~0x02) == ENTSR_PTX) 	/* Strip an undefined bit. */
	    printk(EI_NAME": Possible IRQ conflict?\n");
	else
	    printk(EI_NAME": Possible network cable problem?\n");
	/* It futile, but try to restart it anyway. */
	ei_reset_8390(dev);
	NS8390_init(dev, 1);
    }

    /* This is new: it means some higher layer thinks we've missed an
       tx-done interrupt. Caution: dev_tint() handles the cli()/sti()
       itself. */
    if (skb == NULL) {
#ifdef pre_995
	/* Alternative is ei_tx_intr(dev); */
	ei_status.txing = 1;
	if (dev_tint(NULL, dev) == 0)
	    ei_status.txing = 0;
#else
	dev_tint(dev);
#endif
	return 0;
    }
    /* Fill in the ethernet header. */
    if (!skb->arp  &&  dev->rebuild_header(skb+1, dev)) {
	skb->dev = dev;
	arp_queue (skb);
	return 0;
    }

    dev->trans_start = jiffies;
    cli();
#ifdef PINGPONG
    ei_tx_intr(dev);
#endif
    ei_send_packet(skb, dev);
    sti();
    if (skb->free)
	kfree_skb (skb, FREE_WRITE);
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
    dev->interrupt = 1;
    ei_status.in_interrupt++;
    sti(); /* Allow other interrupts. */

#ifdef notneeded
    /* If we a getting a reset-complete interrupt the 8390 might not be
       mapped in for the 3c503. */
    if (etherlink2)
	outb_p(ei_status.thin_bit, E33G_CNTRL),
	outb_p(0x00, E33G_STATUS);
#endif

    /* Change to page 0 and read the intr status reg. */
    outb_p(E8390_NODMA+E8390_PAGE0, EN_CMD);
    if (ei_debug > 3)
	printk(EI_NAME": interrupt(isr=%#2.2x).\n",
	       inb_p(E8390_BASE + EN0_ISR));

    if (ei_status.in_interrupt > 1)
	printk(EI_NAME ": Reentering the interrupt driver!\n");
    if (dev == NULL) {
	printk (EI_NAME ": irq %d for unknown device\n", irq);
	ei_status.in_interrupt--;
	return;
    } else if (ei_debug > 0  &&  eifdev != dev) {
	printk (EI_NAME": device mismatch on irq %d.\n", irq);
	dev = eifdev;
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
	missed_packets += inb_p(E8390_BASE + EN0_COUNTER2);
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

#ifdef PINGPONG
static int lasttx = 0;
#endif

/* This is stuffed into the dev struct to be called by dev.c:dev_tint().
   Evenually this should be replaced by the block_output() routines. */
static void
ei_send_packet(struct sk_buff *skb, struct device *dev)
{
    int length = skb->len;
    int send_length = ETHER_MIN_LEN < length ? length : ETHER_MIN_LEN;

    if (length <= 0)
	return;

#ifdef PINGPONG
    if (ei_status.tx1 == 0) {	/* First buffer empty */
	ei_block_output(dev, length, (void*)(skb+1),
			ei_status.tx_start_page);
	ei_status.tx1 = send_length;
    } else if (ei_status.tx2 == 0) { /* Second buffer empty */
	ei_block_output(dev, length, (void*)(skb+1),
			ei_status.tx_start_page+6);
	ei_status.tx2 = send_length;
    } else {
	printk("%s: Internal error, no transmit buffer space tx1=%d tx2=%d lasttx=%d.\n",
	       ei_status.name, ei_status.tx1, ei_status.tx2, lasttx);
    }
    ei_status.txqueue++;
    /* The following should be merged with ei_tx_intr(). */
    if (lasttx = 0) {
	if (ei_status.tx1 > 0) {
	    NS8390_trigger_send(dev, ei_status.tx1,
				ei_status.tx_start_page + 6),
	    ei_status.tx1 = -1,
	    lasttx = 1;
	} else if (ei_status.tx2 > 0) {
	    NS8390_trigger_send(dev, ei_status.tx2,
				ei_status.tx_start_page + 6),
	    ei_status.tx2 = -1,
	    lasttx = 2;
	}
    }
#else
    ei_block_output(dev, length, (void*)(skb+1), ei_status.tx_start_page);
    NS8390_trigger_send(dev, send_length, ei_status.tx_start_page);
#endif
    return;
}

/* We have finished a transmit: check for errors and then trigger the next
   packet to be sent. */
static void
ei_tx_intr(struct device *dev)
{
    int status = inb(E8390_BASE + EN0_TSR);
    outb_p(ENISR_TX, E8390_BASE + EN0_ISR); /* Ack intr. */
    if ((status & ENTSR_PTX) == 0)
	tx_errors++;
    else
	tx_packets++;

#ifdef PINGPONG
    ei_status.txqueue--;
    if (ei_status.tx1 < 0) {
	uif (lasttx != 1)
	    printk("%s: bogus last_tx_buffer %d, tx1=%d.\n",
		   ei_status.name, lasttx, ei_status.tx1);
	ei_status.tx1 = 0;
	lasttx = 0;
	if (ei_status.tx2 > 0) {
	    NS8390_trigger_send(dev, ei_status.tx2,
				ei_status.tx_start_page + 6),
	    ei_status.tx2 = -1,
	    lasttx = 2;
	}
    } else if (ei_status.tx2 < 0) {
	if (lasttx != 2)
	    printk("%s: bogus last_tx_buffer %d, tx2=%d.\n",
		   ei_status.name, lasttx, ei_status.tx2);
	ei_status.tx2 = 0;
	lasttx = 0;
	if (ei_status.tx1 > 0) {
	    NS8390_trigger_send(dev, ei_status.tx1,
				ei_status.tx_start_page),
	    ei_status.tx1 = -1;
	    lasttx = 1;
	}
    } /*else
	printk(EI_NAME": unexpected TX interrupt.\n");*/
    while ((ei_status.tx1 == 0) || (ei_status.tx2 == 0)) {
	dev->tbusy = 0;
	dev_tint(dev);
	if (dev->tbusy)
	    return;
	else if (lasttx == 0) {
	    if (ei_status.tx1 == 0 || ei_status.tx2 != 0)
		printk(EI_NAME": Unexpected tx buffer busy tx1=%d tx2=%d.\n",
		       ei_status.tx1, ei_status.tx2);
	    NS8390_trigger_send(dev, ei_status.tx1,
				ei_status.tx_start_page),
	    ei_status.tx1 = -1;
	    lasttx = 1;
	}
    }
#else
    ei_status.txing = 0;
    dev->tbusy = 0;
#ifdef pre_995
    dev_tint(NULL, dev)
#else
    mark_bh (INET_BH);
#endif
#endif
}

/* We have a good packet(s), get it/them out of the buffers. */

static void
ei_receive(struct device *dev)
{
    int rxing_page, this_frame, next_frame, current_offset;
    int boguscount = 0;
    struct e8390_pkt_hdr rx_frame;
    int num_rx_pages = ei_status.stop_page-ei_status.rx_start_page;

    while (++boguscount < 10) {
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
#ifndef EI_DEBUG
	  ei_status.current_page = rxing_page;
	  outb(ei_status.current_page-1, E8390_BASE+EN0_BOUNDARY);
	  continue;
#else
	static int last_rx_bogosity = -1;
	printk(EI_NAME": bogus packet header, status=%#2x nxpg=%#2x sz=%#x (at %#4x)\n",
	       rx_frame.status, rx_frame.next, rx_frame.count, current_offset);

	if (rx_packets != last_rx_bogosity) {
	  /* Maybe we can avoid resetting the chip... empty the packet ring. */
	  ei_status.current_page = rxing_page;
	  printk(EI_NAME":   setting next frame to %#2x (nxt=%#2x, rx_frm.nx=%#2x rx_frm.stat=%#2x).\n",
		 ei_status.current_page, next_frame,
		 rx_frame.next, rx_frame.status);
	  last_rx_bogosity = rx_packets;
	  outb(ei_status.current_page-1, E8390_BASE+EN0_BOUNDARY);
	  continue;
	} else {
	  /* Oh no Mr Bill! Last ditch error recovery. */
	  printk(EI_NAME": multiple sequential lossage, resetting at packet #%d.",
		 rx_packets);
	  sti();
	  ei_reset_8390(dev);
	  NS8390_init(dev, 1);
	  printk("restarting.\n");
	  return;
	}
#endif  /* EI8390_NOCHECK */
      }

      if ((size < 32  ||  size > 1535) && ei_debug)
	printk(EI_NAME": bogus packet size, status=%#2x nxpg=%#2x size=%#x\n",
	       rx_frame.status, rx_frame.next, rx_frame.count);
      if ((rx_frame.status & 0x0F) == ENRSR_RXOK) {
	int sksize = sizeof(struct sk_buff) + size;
	struct sk_buff *skb;
	skb = kmalloc(sksize, GFP_ATOMIC);
	if (skb != NULL) {
	  skb->lock = 0;
	  skb->mem_len = sksize;
	  skb->mem_addr = skb;
	  /* 'skb+1' points to the start of sk_buff data area. */
	  ei_block_input(dev, size, (void *)(skb+1),
			 current_offset + sizeof(rx_frame));
	  if(dev_rint((void *)skb, size, IN_SKBUFF, dev)) {
	      printk(EI_NAME": receive buffers full.\n");
	      break;
	  }
	} else if (ei_debug) {
	  printk(EI_NAME": Couldn't allocate a sk_buff of size %d.\n", sksize);
	  break;
	}
	rx_packets++;
      } else {
	if (ei_debug)
	  printk(EI_NAME": bogus packet, status=%#2x nxpg=%#2x size=%d\n",
		 rx_frame.status, rx_frame.next, rx_frame.count);
	soft_rx_err_bits |= rx_frame.status,
	soft_rx_errors++;
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
    while (dev_rint(NULL, 0, 0, dev) == 1
	   && ++boguscount < 20)
	;
    /* Bug alert!  Reset ENISR_OVER to avoid spurious overruns! */
    outb_p(ENISR_RX+ENISR_RX_ERR+ENISR_OVER, E8390_BASE+EN0_ISR); /* Ack intr. */
    return;
}

/* We have a receiver overrun: we have to kick the 8390 to get it started
   again.  Overruns are detected on a per-256byte-page basis. */
static void
ei_rx_overrun(struct device *dev)
{
    int reset_start_time = jiffies;
    
    /* We should already be stopped and in page0.  Remove after testing. */
    outb_p(E8390_NODMA+E8390_PAGE0+E8390_STOP, EN_CMD);

    if (ei_debug)
	printk(EI_NAME ": Receiver overrun.\n");

    /* The we.c driver does dummy = inb_p( RBCR[01] ); at this point.
       It might mean something -- magic to speed up a reset?  A 8390 bug?*/

    /* Wait for reset in case the NIC is doing a tx or rx.  This could take up to
       1.5msec, but we have no way of timing something in that range.  The 'jiffies'
       are just a sanity check. */
    while ((inb_p(E8390_BASE+EN0_ISR) & ENISR_RESET) == 0)
	if (jiffies - reset_start_time > 1) {
	    printk(EI_NAME": reset did not complete at ei_rx_overrun.\n");
	    NS8390_init(dev, 1);
	    return;
	};

    {
	int old_rx_packets = rx_packets;
	/* Remove packets right away. */
	ei_receive(dev);
	rx_overrun_packets += (rx_packets - old_rx_packets);
    }
    outb_p(0xff, E8390_BASE+EN0_ISR);
    /* Generic 8390 insns to start up again, same as in open_8390(). */
    outb_p(E8390_NODMA + E8390_PAGE0 + E8390_START, EN_CMD);
    outb_p(E8390_TXCONFIG, E8390_BASE + EN0_TXCR); /* xmit on. */
#ifdef notneeded
    outb_p(E8390_RXCONFIG, E8390_BASE + EN0_RXCR); /* rx on,  */
#endif    
}

int
ethif_init(struct device *dev)
{
    int i;

    eifdev = dev;		/* Store for debugging. */

    if (ei_debug > 3)
	printk(version);
    if (1
#ifdef WD80x3
	&& ! wdprobe(dev->base_addr, dev)
#endif		     
#ifdef EL2
	&& ! el2autoprobe(dev->base_addr, dev)
#endif
#ifdef NE2000
	&& ! neprobe(dev->base_addr, dev)
#endif		     
#ifdef HPLAN
	&& ! hpprobe(dev->base_addr, dev)
#endif		     
	&& 1 ) {
	dev->open = &ei_open;
	printk("No ethernet device found.\n");
	ei_status.exists = 0;
	return 1;			/* ENODEV or EAGAIN would be more accurate. */
    }

    e8390_base = dev->base_addr;

    /* Initialize the rest of the device structure. Many of these could
       be in Space.c. */
    for (i = 0; i < DEV_NUMBUFFS; i++)
	dev->buffs[i] = NULL;

    ei_status.exists = 1;
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
    return 0;
}


/* This page of functions should be 8390 generic */
/* Follow National Semi's recommendations for initializing the "NIC". */
void NS8390_init(struct device *dev, int startp)
{
  int i;
  int endcfg = ei_status.word16 ? (0x48 | ENDCFG_WTS) : 0x48;
  int e8390_base = dev->base_addr;

  /* Follow National Semi's recommendations for initing the DP83902. */
  outb_p(E8390_NODMA+E8390_PAGE0+E8390_STOP, e8390_base); /* 0x21 */
  outb_p(endcfg, e8390_base + EN0_DCFG); 	/* 0x48 or 0x49 */
  /* Clear the remote byte count registers. */
  outb_p(0x00,  e8390_base + EN0_RCNTLO);
  outb_p(0x00,  e8390_base + EN0_RCNTHI);
  /* Set to monitor and loopback mode -- this is vital!. */
  outb_p(E8390_RXOFF, e8390_base + EN0_RXCR); /* 0x20 */
  outb_p(E8390_TXOFF, e8390_base + EN0_TXCR); /* 0x02 */
  /* Set the transmit page and receive ring. */
  outb_p(ei_status.tx_start_page,  e8390_base + EN0_TPSR);
  ei_status.tx1 = ei_status.tx2 = 0;
  outb_p(ei_status.rx_start_page,  e8390_base + EN0_STARTPG);
  outb_p(ei_status.stop_page-1, e8390_base + EN0_BOUNDARY); /* 3c503 says 0x3f,NS0x26*/
  ei_status.current_page = ei_status.rx_start_page; 	    /* assert boundary+1 */
  outb_p(ei_status.stop_page,   e8390_base + EN0_STOPPG);
  /* Clear the pending interrupts and mask. */
  outb_p(0xFF, e8390_base + EN0_ISR);
  outb_p(0x00,  e8390_base + EN0_IMR);

  /* Copy the station address into the DS8390 registers,
     and set the multicast hash bitmap to receive all multicasts. */
  cli();
  outb_p(E8390_NODMA + E8390_PAGE1 + E8390_STOP, e8390_base); /* 0x61 */
  for(i = 0; i < 6; i++) {
    outb_p(dev->dev_addr[i], e8390_base + EN1_PHYS + i);
  }
  for(i = 0; i < 8; i++)
    outb_p(0xff, e8390_base + EN1_MULT + i);

  outb_p(ei_status.rx_start_page,  e8390_base + EN1_CURPAG);
  outb_p(E8390_NODMA+E8390_PAGE0+E8390_STOP, e8390_base);
  sti();
  if (startp) {
      outb_p(0xff,  e8390_base + EN0_ISR);
      outb_p(ENISR_ALL,  e8390_base + EN0_IMR);
      outb_p(E8390_NODMA+E8390_PAGE0+E8390_START, e8390_base);
      outb_p(E8390_TXCONFIG, e8390_base + EN0_TXCR); /* xmit on. */
      /* 3c503 TechMan says rxconfig only after the NIC is started. */
      outb_p(E8390_RXCONFIG,  e8390_base + EN0_RXCR); /* rx on,  */
  }
  return;
}

/* Trigger a transmit start, assuming the length is valid. */
static void NS8390_trigger_send(struct device *dev, unsigned int length,
				int start_page)
{
    int e8390_base = dev->base_addr;

    ei_status.txing = 1;
    dev->tbusy = 1;
    outb_p(E8390_NODMA+E8390_PAGE0, e8390_base);

    if (inb_p(EN_CMD) & E8390_TRANS) {
      printk(EI_NAME": trigger_send() called with the transmitter busy.\n");
      return;
    }
    outb_p(length & 0xff, e8390_base + EN0_TCNTLO);
    outb_p(length >> 8, e8390_base + EN0_TCNTHI);
    outb_p(start_page, e8390_base + EN0_TPSR);
    outb_p(E8390_NODMA+E8390_TRANS+E8390_START, e8390_base);
    outb_p(ENISR_RDC, e8390_base + EN0_ISR);
    return;
}


/*
 * Local variables:
 *  compile-command: "gcc -DKERNEL -Wall -O6 -fomit-frame-pointer -DPINGPONG -I/usr/src/linux/net/tcp -c 8390.c"
 *  version-control: t
 *  kept-new-versions: 5
 * End:
 */
