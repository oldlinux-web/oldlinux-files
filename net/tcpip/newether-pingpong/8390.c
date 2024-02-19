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
    "8390.c:v0.68 3/9/93 for 0.99.6 Donald Becker (becker@super.org)\n";
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
  The National Semiconductor LAN Databook, and the 3Com 3c503 databook.
  The NE* programming info came from the Crynwr packet driver, and figuring
  out that the those boards are similar to the NatSemi evaluation board
  described in AN-729.  Thanks NS, no thanks to Novell/Eagle.
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

#define ei_reset_8390 (ei_local->reset_8390)
#define ei_block_output (ei_local->block_output)
#define ei_block_input (ei_local->block_input)

/* use 0 for production, 1 for verification, >2 for debug */
#ifdef EI_DEBUG
int ei_debug = EI_DEBUG;
#else
int ei_debug = 2;
#endif

struct device *irq2dev_map[16] = {0,0,0, /* zeroed...*/};

#ifdef PINGPONG
static int lasttx = 0;
#endif

/* Max number of packets received at one Intr. */
/*static int high_water_mark = 0;*/

/* Index to functions. */
/* Put in the device structure. */
static int ei_open(struct device *dev);
/* Dispatch from interrupts. */
void ei_interrupt(int reg_ptr);
static void ei_tx_intr(struct device *dev);
static void ei_receive(struct device *dev);
static void ei_rx_overrun(struct device *dev);

int ethdev_init(struct device *dev);
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
    struct ei_device *ei_local = dev->private;

    if ( ! ei_local) {
	printk("%s: Opening a non-existent physical device\n", dev->name);
	return 1;		/* ENXIO would be more accurate. */
    }

  irq2dev_map[dev->irq] = dev;
  NS8390_init(dev, 1);
  ei_local->tx1 = ei_local->tx2 = 0;
  /* The old local flags... */
  ei_local->txing = 0;
  /* ... are now global. */
  dev->tbusy = 0;
  dev->interrupt = 0;
  dev->start = 1;
  return 0;
}

static int
ei_start_xmit(struct sk_buff *skb, struct device *dev)
{
    int e8390_base = dev->base_addr;
    struct ei_device *ei_local = dev->private;

    if (dev->tbusy) {	/* Do timeouts, just like the 8003 driver. */
	int txsr = inb(e8390_base+EN0_TSR), isr;
	int tickssofar = jiffies - dev->trans_start;
	if (tickssofar < 5  ||  (tickssofar < 15 && ! (txsr & ENTSR_PTX))) {
	    return 1;
	}
	isr = inb(e8390_base+EN0_ISR);
	printk("%s: transmit timed out, TX status %#2x, ISR %#2x.\n",
		dev->name, txsr, isr);
	/* It's possible to check for an IRQ conflict here.
	   I may have to do that someday. */
	if (isr)
	    printk("%s: Possible IRQ conflict on IRQ%d?", dev->name, dev->irq);
	else
	    printk("%s: Possible network cable problem?\n", dev->name);
	/* It futile, but try to restart it anyway. */
	ei_reset_8390(dev);
	NS8390_init(dev, 1);
	printk("\n");
    }

    /* This is new: it means some higher layer thinks we've missed an
       tx-done interrupt. Caution: dev_tint() handles the cli()/sti()
       itself. */
    if (skb == NULL) {
	dev_tint(dev);
	return 0;
    }
    /* Fill in the ethernet header. */
    if (!skb->arp  &&  dev->rebuild_header(skb+1, dev)) {
	skb->dev = dev;
	arp_queue (skb);
	return 0;
    }

#ifdef PINGPONG
    {
	int length = skb->len;
	int output_page;
	int send_length = ETHER_MIN_LEN < length ? length : ETHER_MIN_LEN;
	if (length <= 0)
	    return 0;
	/* Turn off interrupts so that we can put the packet out safely. */
	outb(0x00,  e8390_base + EN0_IMR);
	if (ei_local->tx1 == 0) {
	    output_page = ei_local->tx_start_page;
	    ei_local->tx1 = send_length;
	    if (ei_debug  &&  ei_local->tx2 > 0)
		printk("%s: idle transmitter tx2=%d, lasttx=%d, txing=%d.\n",
		       dev->name, ei_local->tx2, lasttx, ei_local->txing);
	} else if (ei_local->tx2 == 0) {
	    output_page = ei_local->tx_start_page + 6;
	    ei_local->tx2 = send_length;
	    if (ei_debug  &&  ei_local->tx1 > 0)
		printk("%s: idle transmitter, tx1=%d, lasttx=%d, txing=%d.\n",
		       dev->name, ei_local->tx1, lasttx, ei_local->txing);
	} else {
	    /* We can get to here if we get an rx interrupt and queued
	       a tx packet just before masking 8390 irqs above. */
	    if (ei_debug > 2)
		printk("%s: No packet buffer space for ping-pong use.\n",
		       dev->name);
	    outb_p(ENISR_ALL,  e8390_base + EN0_IMR);
	    return 1;
	}
	dev->trans_start = jiffies;
	ei_block_output(dev, length, (void*)(skb+1), output_page);
	if (! ei_local->txing) {
	    NS8390_trigger_send(dev, send_length, output_page);
	    if (output_page == ei_local->tx_start_page)
		ei_local->tx1 = -1, lasttx = -1;
	    else
		ei_local->tx2 = -1, lasttx = -2;
	    ei_local->txing = 1;
	} else
	    ei_local->txqueue++;
	if (ei_local->tx1  &&  ei_local->tx2)
	    dev->tbusy = 1;
	/* Turn 8390 interrupts back on. */
	outb_p(ENISR_ALL,  e8390_base + EN0_IMR);
    }
#else
    {
	int length = skb->len;
	int send_length = ETHER_MIN_LEN < length ? length : ETHER_MIN_LEN;
	if (length <= 0)
	    return 0;
	dev->trans_start = jiffies;
	/* Turn off interrupts so that we can put the packet out safely. */
	outb(0x00,  e8390_base + EN0_IMR);
	ei_block_output(dev, length, (void*)(skb+1), ei_local->tx_start_page);
	NS8390_trigger_send(dev, send_length, ei_local->tx_start_page);
	dev->tbusy = 1;
	/* Turn 8390 interrupts back on. */
	outb_p(ENISR_ALL,  e8390_base + EN0_IMR);
    }
#endif  /* PINGPONG */
    if (skb->free)
	kfree_skb (skb, FREE_WRITE);
    return 0;
}

/* The typical workload of the driver:
   Handle the ether interface interrupts. */
void
ei_interrupt(int reg_ptr)
{
    int irq = -(((struct pt_regs *)reg_ptr)->orig_eax+2);
    struct device *dev = irq2dev_map[irq];
    int e8390_base;
    int interrupts, boguscount = 0;

    if (dev == NULL) {
	printk ("net_interrupt(): irq %d for unknown device.\n", irq);
	return;
    }
    dev->interrupt = 1;
    sti(); /* Allow other interrupts. */
    
    e8390_base = dev->base_addr;

    /* Change to page 0 and read the intr status reg. */
    outb_p(E8390_NODMA+E8390_PAGE0, e8390_base + E8390_CMD);
    if (ei_debug > 3)
	printk("%s: interrupt(isr=%#2.2x).\n", dev->name,
	       inb_p(e8390_base + EN0_ISR));

    /* !!Assumption!! -- we stay in page 0.  Don't break this. */
    while ((interrupts = inb_p(e8390_base + EN0_ISR)) != 0
	   && ++boguscount < 20) {
	if (interrupts & ENISR_RDC) {
	    /* Ack meaningless DMA complete. */
	    outb_p(ENISR_RDC, e8390_base + EN0_ISR);
	}
	if (interrupts & ENISR_OVER) {
	    ei_rx_overrun(dev);
	} else if (interrupts & (ENISR_RX+ENISR_RX_ERR)) {
	    /* Got a good (?) packet. */
	    ei_receive(dev);
	}
	/* Push the next to-transmit packet through. */
	if (interrupts & ENISR_TX) {
	    ei_tx_intr(dev);
	} else if (interrupts & ENISR_COUNTERS) {
	    struct ei_device *ei_local = dev->private;
	    ei_local->soft_rx_errors += inb_p(e8390_base + EN0_COUNTER0);
	    ei_local->soft_rx_errors += inb_p(e8390_base + EN0_COUNTER1);
	    ei_local->missed_packets += inb_p(e8390_base + EN0_COUNTER2);
	    outb_p(ENISR_COUNTERS, e8390_base + EN0_ISR); /* Ack intr. */
	}

	/* Ignore the transmit errs and reset intr for now. */
	if (interrupts & ENISR_TX_ERR) {
	    outb_p(ENISR_TX_ERR, e8390_base + EN0_ISR); /* Ack intr. */
	}
	outb_p(E8390_NODMA+E8390_PAGE0+E8390_START, e8390_base + E8390_CMD);
    }

    if (interrupts && ei_debug) {
      printk("%s: unknown interrupt %#2x\n", dev->name, interrupts);
      outb_p(E8390_NODMA+E8390_PAGE0+E8390_START, e8390_base + E8390_CMD);
      outb_p(0xff, e8390_base + EN0_ISR); /* Ack. all intrs. */
    }
    dev->interrupt = 0;
    return;
}

/* We have finished a transmit: check for errors and then trigger the next
   packet to be sent. */
static void
ei_tx_intr(struct device *dev)
{
    int e8390_base = dev->base_addr;
    int status = inb(e8390_base + EN0_TSR);
    struct ei_device *ei_local = dev->private;

    outb_p(ENISR_TX, e8390_base + EN0_ISR); /* Ack intr. */
    if ((status & ENTSR_PTX) == 0)
	ei_local->tx_errors++;
    else
	ei_local->tx_packets++;

#ifdef PINGPONG
    ei_local->txqueue--;
    if (ei_local->tx1 < 0) {
	if (lasttx != 1 && lasttx != -1)
	    printk("%s: bogus last_tx_buffer %d, tx1=%d.\n",
		   ei_local->name, lasttx, ei_local->tx1);
	ei_local->tx1 = 0;
	dev->tbusy = 0;
	if (ei_local->tx2 > 0) {
	    NS8390_trigger_send(dev, ei_local->tx2, ei_local->tx_start_page + 6);
	    ei_local->txing = 1;
	    ei_local->tx2 = -1,
	    lasttx = 2;
	} else
	    lasttx = 20, ei_local->txing = 0;
    } else if (ei_local->tx2 < 0) {
	if (lasttx != 2  &&  lasttx != -2)
	    printk("%s: bogus last_tx_buffer %d, tx2=%d.\n",
		   ei_local->name, lasttx, ei_local->tx2);
	ei_local->tx2 = 0;
	dev->tbusy = 0;
	if (ei_local->tx1 > 0) {
	    NS8390_trigger_send(dev, ei_local->tx1, ei_local->tx_start_page);
	    ei_local->txing = 1;
	    ei_local->tx1 = -1;
	    lasttx = 1;
	} else
	    lasttx = 10, ei_local->txing = 0;
    } else
	printk("%s: unexpected TX-done interrupt, lasttx=%d.\n",
	       dev->name, lasttx);
#else
    ei_local->txing = 0;
    dev->tbusy = 0;
#endif
    mark_bh (INET_BH);
}

/* We have a good packet(s), get it/them out of the buffers. */

static void
ei_receive(struct device *dev)
{
    int e8390_base = dev->base_addr;
    struct ei_device *ei_local = dev->private;
    int rxing_page, this_frame, next_frame, current_offset;
    int boguscount = 0;
    struct e8390_pkt_hdr rx_frame;
    int num_rx_pages = ei_local->stop_page-ei_local->rx_start_page;

    while (++boguscount < 10) {
      int size;

      /* Get the rx page (incoming packet pointer). */
      outb_p(E8390_NODMA+E8390_PAGE1, e8390_base + E8390_CMD);
      rxing_page = inb_p(e8390_base + EN1_CURPAG);
      outb_p(E8390_NODMA+E8390_PAGE0, e8390_base + E8390_CMD);

      /* Remove one frame from the ring.  Boundary is alway a page behind. */
      this_frame = inb_p(e8390_base + EN0_BOUNDARY) + 1;
      if (this_frame >= ei_local->stop_page)
	this_frame = ei_local->rx_start_page;

      /* Someday we'll omit the previous step, iff we never get this message.*/
      if (ei_debug > 0  &&  this_frame != ei_local->current_page)
	printk("%s: mismatched read page pointers %2x vs %2x.\n",
	       dev->name, this_frame, ei_local->current_page);

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
	  ei_local->current_page = rxing_page;
	  outb(ei_local->current_page-1, e8390_base+EN0_BOUNDARY);
	  continue;
#else
	static int last_rx_bogosity = -1;
	printk("%s: bogus packet header, status=%#2x nxpg=%#2x sz=%#x (at %#4x)\n",
	       dev->name, rx_frame.status, rx_frame.next, rx_frame.count,
	       current_offset);

	if (rx_packets != last_rx_bogosity) {
	  /* Maybe we can avoid resetting the chip... empty the packet ring. */
	  ei_local->current_page = rxing_page;
	  printk("%s:   setting next frame to %#2x (nxt=%#2x, rx_frm.nx=%#2x rx_frm.stat=%#2x).\n",
		 dev->name, ei_local->current_page, next_frame,
		 rx_frame.next, rx_frame.status);
	  last_rx_bogosity = rx_packets;
	  outb(ei_local->current_page-1, e8390_base+EN0_BOUNDARY);
	  continue;
	} else {
	  /* Oh no Mr Bill! Last ditch error recovery. */
	  printk("%s: recovery failed, resetting at packet #%d..",
		 dev->name, rx_packets);
	  sti();
	  ei_reset_8390(dev);
	  NS8390_init(dev, 1);
	  printk("restarting.\n");
	  return;
	}
#endif  /* EI8390_NOCHECK */
      }

      if ((size < 32  ||  size > 1535) && ei_debug)
	printk("%s: bogus packet size, status=%#2x nxpg=%#2x size=%#x\n",
	       dev->name, rx_frame.status, rx_frame.next, rx_frame.count);
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
	  if (dev_rint((void *)skb, size, IN_SKBUFF, dev)) {
	      printk("%s: receive buffers full.\n", dev->name);
	      break;
	  }
	} else if (ei_debug) {
	  printk("%s: Couldn't allocate a sk_buff of size %d.\n",
		 dev->name, sksize);
	  break;
	}
	ei_local->rx_packets++;
      } else {
	if (ei_debug)
	  printk("%s: bogus packet, status=%#2x nxpg=%#2x size=%d\n",
		 dev->name, rx_frame.status, rx_frame.next, rx_frame.count);
	ei_local->soft_rx_err_bits |= rx_frame.status,
	ei_local->soft_rx_errors++;
      }
      next_frame = rx_frame.next;

	 /* This should never happen, it's here for debugging. */
      if (next_frame >= ei_local->stop_page) {
	printk("%s: next frame inconsistency, %#2x..", dev->name, next_frame);
	next_frame = ei_local->rx_start_page;
      }
      ei_local->current_page += 1 + ((size+4)>>8);
      ei_local->current_page = next_frame;
      outb(next_frame-1, e8390_base+EN0_BOUNDARY);
    }
    /* If any worth-while packets have been received, dev_rint()
       has done a mark_bh(INET_BH) for us and will work on them
       when we get to the bottom-half routine. */

    /* Bug alert!  Reset ENISR_OVER to avoid spurious overruns! */
    outb_p(ENISR_RX+ENISR_RX_ERR+ENISR_OVER, e8390_base+EN0_ISR);
    return;
}

/* We have a receiver overrun: we have to kick the 8390 to get it started
   again.*/
static void
ei_rx_overrun(struct device *dev)
{
    int e8390_base = dev->base_addr;
    int reset_start_time = jiffies;
    struct ei_device *ei_local = dev->private;

    /* We should already be stopped and in page0.  Remove after testing. */
    outb_p(E8390_NODMA+E8390_PAGE0+E8390_STOP, e8390_base+E8390_CMD);

    if (ei_debug)
	printk("%s: Receiver overrun.\n", dev->name);
    ei_local->rx_overruns++;

    /* The we.c driver does dummy = inb_p( RBCR[01] ); at this point.
       It might mean something -- magic to speed up a reset?  A 8390 bug?*/

    /* Wait for reset in case the NIC is doing a tx or rx.  This could take up to
       1.5msec, but we have no way of timing something in that range.  The 'jiffies'
       are just a sanity check. */
    while ((inb_p(e8390_base+EN0_ISR) & ENISR_RESET) == 0)
	if (jiffies - reset_start_time > 1) {
	    printk("%s: reset did not complete at ei_rx_overrun.\n",
		   dev->name);
	    NS8390_init(dev, 1);
	    return;
	};

    {
	int old_rx_packets = ei_local->rx_packets;
	/* Remove packets right away. */
	ei_receive(dev);
	ei_local->rx_overrun_packets +=
	    (ei_local->rx_packets - old_rx_packets);
    }
    outb_p(0xff, e8390_base+EN0_ISR);
    /* Generic 8390 insns to start up again, same as in open_8390(). */
    outb_p(E8390_NODMA + E8390_PAGE0 + E8390_START, e8390_base + E8390_CMD);
    outb_p(E8390_TXCONFIG, e8390_base + EN0_TXCR); /* xmit on. */
}

int
ethif_init(struct device *dev)
{
    if (ei_debug > 1)
	printk(version);

    /* The open call may be overridden by the card-specific code. */
    dev->open = &ei_open;

    /* Make up a ei_local structure. */
    dev->private = kmalloc(sizeof(struct ei_device), GFP_KERNEL);
    memset(dev->private, 0, sizeof(struct ei_device));

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
	printk("No ethernet device found.\n");
	kfree(dev->private);
	dev->private = NULL;
	return 1;			/* ENODEV or EAGAIN would be more accurate. */
    }

    return ethdev_init(dev);
}

/* Initialize the rest of the device structure. */
int
ethdev_init(struct device *dev)
{
    int i;

    for (i = 0; i < DEV_NUMBUFFS; i++)
	dev->buffs[i] = NULL;

    dev->hard_header = eth_hard_header;
    dev->add_arp = eth_add_arp;
    dev->queue_xmit = dev_queue_xmit;
    dev->rebuild_header = eth_rebuild_header;
    dev->type_trans = eth_type_trans;

    if (dev->private == NULL) {
	dev->private = kmalloc(sizeof(struct ei_device), GFP_KERNEL);
	memset(dev->private, 0, sizeof(struct ei_device));
    }

    dev->send_packet = 0;
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
    int e8390_base = dev->base_addr;
    struct ei_device *ei_local = dev->private;
    int i;
    int endcfg = ei_local->word16 ? (0x48 | ENDCFG_WTS) : 0x48;

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
  outb_p(ei_local->tx_start_page,  e8390_base + EN0_TPSR);
  ei_local->tx1 = ei_local->tx2 = 0;
  outb_p(ei_local->rx_start_page,  e8390_base + EN0_STARTPG);
  outb_p(ei_local->stop_page-1, e8390_base + EN0_BOUNDARY); /* 3c503 says 0x3f,NS0x26*/
  ei_local->current_page = ei_local->rx_start_page; 	    /* assert boundary+1 */
  outb_p(ei_local->stop_page,   e8390_base + EN0_STOPPG);
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

  outb_p(ei_local->rx_start_page,  e8390_base + EN1_CURPAG);
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
    outb_p(E8390_NODMA+E8390_PAGE0, e8390_base);

    if (inb_p(e8390_base) & E8390_TRANS) {
      printk("%s: trigger_send() called with the transmitter busy.\n",
	     dev->name);
      return;
    }
    outb_p(length & 0xff, e8390_base + EN0_TCNTLO);
    outb_p(length >> 8, e8390_base + EN0_TCNTHI);
    outb_p(start_page, e8390_base + EN0_TPSR);
    outb_p(E8390_NODMA+E8390_TRANS+E8390_START, e8390_base);
    return;
}


/*
 * Local variables:
 *  compile-command: "gcc -DKERNEL -Wall -O6 -fomit-frame-pointer -DPINGPONG -I/usr/src/linux/net/tcp -c 8390.c"
 *  version-control: t
 *  kept-new-versions: 5
 * End:
 */
