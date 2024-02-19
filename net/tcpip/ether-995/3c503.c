/* 3c503.c: A shared-memory NS8390 ethernet driver for linux. */
/*
    Written 1992,1993 by Donald Becker. This is alpha test code.
    This is a extension to the Linux operating system, and is covered by
    same Gnu Public License that covers that work.

    This driver should work with the 3c503 and 3c503/16.  It must be used
    in shared memory mode.

    The Author may be reached as becker@super.org or
    C/O Supercomputing Research Ctr., 17100 Science Dr., Bowie MD 20715
*/

static char *version = "3c503.c:v0.30 1/30/93 Donald Becker (becker@super.org)\n";

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/system.h>

#include "dev.h"

#include "8390.h"
#include "3c503reg.h"

extern void NS8390_init(struct device *dev, int startp);
extern int ei_debug;
extern struct sigaction ei_sigaction;
extern struct ei_device ei_status;

int etherlink2 = 0;

int el2autoprobe(int ioaddr, struct device *dev);
int el2probe(int ioaddr, struct device *dev);

static void el2_reset_8390(struct device *dev);
static void el2_init_card(struct device *dev);
static void el2_block_output(struct device *dev, int count,
			     const unsigned char *buf, const start_page);
static int el2_block_input(struct device *dev, int count, char *buf,
			   int ring_offset);


int
el2autoprobe(int ioaddr, struct device *dev)
{
    int *addr, addrs[] = { 0xddfff, 0xd9fff, 0xcdfff, 0xc9fff, 0};
    int ports[] = {0, 0x300, 0x310, 0x330, 0x350, 0x250, 0x280, 0x2a0, 0x2e0, 0};

    /* Non-autoprobe case first: */
    if (ioaddr > 0)
	return el2probe(ioaddr, dev);

    /* We check for a memory-mapped 3c503 board by looking at the
       end of boot PROM space (works even if a PROM isn't there). */
    for (addr = addrs; *addr; addr++) {
	unsigned int base_bits = *(unsigned char *)*addr, i;
	/* Find first set bit. */
	for(i = 8; i; i--, base_bits >>= 1)
	    if (base_bits & 0x1)
		break;
	if (base_bits == 1  &&  el2probe(ports[i], dev))
	    return dev->base_addr;
    }
#ifdef notdef
    /* If it's not memory mapped, we don't care to find it.  I haven't
       tested the non-memory-mapped code.  */
    /* It's not memory mapped -- try all of the locations that aren't
       obviously empty. */
    {	int *port;
	for (port = &ports[1]; *port; port++)
	    if (inb_p(*port) != 0xff && el2probe(*port, dev))
		return dev->base_addr = *port;
    }
#endif
    return 0;
}
    
/* Probe for the Etherlink II card at I/O port base IOADDR,
   returning non-zero on sucess.  If found, set the station
   address and memory parameters in DEVICE. */
int
el2probe(int ioaddr, struct device *dev)
{
    int i, found, mem_jumpers;
    unsigned char *station_addr = dev->dev_addr;

    /* We verify that it's a 3C503 board by checking the first three octets
       of its ethernet address. */
    printk("3c503 probe at %#3x:", ioaddr);
    outb_p(ECNTRL_RESET|ECNTRL_THIN, ioaddr + 0x406); /* Reset it... */
    outb_p(ECNTRL_THIN, ioaddr + 0x406);
    /* Map the station addr PROM into the lower I/O ports. */
    outb(ECNTRL_SAPROM|ECNTRL_THIN, ioaddr + 0x406);
    for (i = 0; i < ETHER_ADDR_LEN; i++) {
	printk(" %2.2X", (station_addr[i] = inb(ioaddr + i)));
    }
    /* Map the 8390 back into the window. */
    outb(ECNTRL_THIN, ioaddr + 0x406);
    found =(   station_addr[0] == 0x02
	    && station_addr[1] == 0x60
	    && station_addr[2] == 0x8c);
    if (! found) {
	printk("  3C503 not found.\n");
	return 0;
    }
    dev->base_addr = ioaddr;
    /* Probe for, turn on and clear the board's shared memory. */
    mem_jumpers = inb(ioaddr + 0x404);	/* E33G_ROMBASE */
    if (ei_debug > 2) printk(" memory jumpers %2.2x ", mem_jumpers);
    outb(EGACFR_IRQOFF, ioaddr + 0x405);	/* Enable RAM */
    if ((mem_jumpers & 0xf0) == 0) {
	dev->mem_start = 0;
	if (ei_debug > 1) printk(" no shared memory ");
    } else {
	dev->mem_start = ((mem_jumpers & 0xc0) ? 0xD8000 : 0xC8000) +
	    ((mem_jumpers & 0xA0) ? 0x4000 : 0);

#define EL2_MEMSIZE (EL2SM_STOP_PG - EL2SM_START_PG)*256
#ifdef EL2MEMTEST
	{			/* Check the card's memory. */
	    int *mem_base = (int *)dev->mem_start;
	    int memtest_value = 0xbbadf00d;
	    mem_base[0] = 0xba5eba5e;
	    for (i = 1; i < EL2_MEMSIZE/sizeof(mem_base[0]); i++) {
		mem_base[i] = memtest_value;
		if (mem_base[0] != 0xba5eba5e
		    || mem_base[i] != memtest_value) {
		    printk(" memory failure or memory address conflict.\n");
		    dev->mem_start = 0;
		    break;
		}
		memtest_value += 0x55555555;
		mem_base[i] = 0;
	    }
	}
#endif  /* EL2MEMTEST */
	/* Divide the on-board memory into a single maximum-sized transmit
	   (double-sized for ping-pong transmit) buffer at the base, and
	   use the rest as a receive ring. */
	dev->mem_end = dev->rmem_end = dev->mem_start + EL2_MEMSIZE;
	dev->rmem_start = TX_PAGES*256 + dev->mem_start;
    }
    if (ei_debug > 2)
	printk("\n3c503: memory params start=%#5x rstart=%#5x end=%#5x rend=%#5x.\n",
	       dev->mem_start, dev->rmem_start, dev->mem_end, dev->rmem_end);

    /* Finish setting the board's parameters. */
    etherlink2 = 1;
    ei_status.tx_start_page = EL2SM_START_PG;
    ei_status.rx_start_page = EL2SM_START_PG + TX_PAGES;
    ei_status.stop_page = EL2SM_STOP_PG;
    ei_status.reset_8390 = &el2_reset_8390;
    ei_status.block_input = &el2_block_input;
    ei_status.block_output = &el2_block_output;
/* This should be probed for (or set via an ioctl()) at run-time someday. */
#if defined(EI8390_THICK) || defined(EL2_AUI)
    ei_status.thin_bit = 0;
#else
    ei_status.thin_bit = ECNTRL_THIN;
#endif

    if (dev->irq < 2) {
	int irqlist[] = {5, 2, 3, 4, 0};
	int *irqp = irqlist;
	do {
	    if (irqaction (dev->irq = *irqp, &ei_sigaction) == 0) {
		break;
	    }
	} while (*++irqp);
	if (*irqp == 0) {
	    printk("\n3c503: Unable to find an free IRQ line.\n");
	    return 0;
	}
    } else {
	if (dev->irq == 2)
	    dev->irq = 9;
	else if (dev->irq > 5 && dev->irq != 9) {
	    printk("\n3c503: configured interrupt number %d out of range.\n",
		   dev->irq);
	    return 0;
	}
	if (irqaction (dev->irq, &ei_sigaction)) {
	    printk ("\n3c503: Unable to get IRQ%d.\n", dev->irq);
	    return 0;
	}
    }

    el2_init_card(dev);

    if (dev->mem_start)
	printk("3c503 found, memory at %#6x, IRQ %d\n",
	       dev->mem_start, dev->irq);
    else
	printk(" 3c503 found, no shared memory, IRQ %d\n", dev->irq);
    if (ei_debug > 2)
	printk(version);

    return ioaddr;
}

/* This is called whenever we have a unrecoverable failure:
       transmit timeout
       Bad ring buffer packet header
 */
static void
el2_reset_8390(struct device *dev)
{
    if (ei_debug > 1) printk("3c503: Resetting the board...");
    outb_p(ECNTRL_RESET|ECNTRL_THIN, E33G_CNTRL);
    ei_status.txing = 0;
    outb_p(ei_status.thin_bit, E33G_CNTRL);
    el2_init_card(dev);
    if (ei_debug > 1) printk("done\n");
}

/* Initialize the 3c503 GA registers after a reset. */
static void
el2_init_card(struct device *dev)
{
    /* Unmap the station PROM and select the DIX or BNC connector. */
    outb_p(ei_status.thin_bit, E33G_CNTRL);

    /* Set ASIC copy of rx's first and last+1 buffer pages */
    /* These must be the same as in the 8390. */
    outb(ei_status.rx_start_page, E33G_STARTPG);
    outb(ei_status.stop_page,  E33G_STOPPG);

    /* Point the vector pointer registers somewhere ?harmless?. */
    outb(0xff, E33G_VP2);	/* Point at the ROM restart location 0xffff0 */
    outb(0xff, E33G_VP1);
    outb(0x00, E33G_VP0);
    /* Turn off all interrupts until we're opened. */
    outb_p(0x00,  dev->base_addr + EN0_IMR);
    outb_p(EGACFR_IRQOFF, E33G_GACFR);

    /* Set the interrupt line. */
    outb_p((0x04 << (dev->irq == 9 ? 2 : dev->irq)), E33G_IDCFR);
    outb_p(8, E33G_DRQCNT);		/* Set burst size to 8 */
    outb_p(0x20, E33G_DMAAH);	/* Put a valid addr in the GA DMA */
    outb_p(0x00, E33G_DMAAL);
    return;			/* We always succeed */
}

/* Either use the shared memory (if enabled on the board) or put the packet
   out through the ASIC FIFO.  The latter is probably much slower. */
static void
el2_block_output(struct device *dev, int count,
		 const unsigned char *buf, const start_page)
{
    int i;				/* Buffer index */
    int boguscount = 0;		/* timeout counter */
    if (dev->mem_start) {	/* Shared memory transfer */
	void *dest_addr = (void *)(dev->mem_start +
	    ((start_page - ei_status.tx_start_page) << 8));
	outb(EGACFR_NORM, E33G_GACFR);	/* Enable RAM */
	memcpy(dest_addr, buf, count);
	if (ei_debug > 2  &&  memcmp(dest_addr, buf, count))
	    printk("3c503: send_packet() bad memory copy @ %#5x.\n",
		   dest_addr);
	else if (ei_debug > 4)
	    printk("3c503: send_packet() good memory copy @ %#5x.\n",
		   dest_addr);
	return;
    }
    /* Set up then start the internal memory transfer to Tx Start Page */
    outb(0x00, E33G_DMAAL);
    outb(start_page, E33G_DMAAH);
    outb(ei_status.thin_bit | ECNTRL_OUTPUT | ECNTRL_START, E33G_CNTRL);

    /* This is the byte copy loop: it should probably be tuned for
       for speed once everything is working.  I think it is possible
       to output 8 bytes between each check of the status bit. */
    for(i = 0; i < count; i++) {
	if (count % 8 == 7)
	    while ((inb(E33G_STATUS) & ESTAT_DPRDY) == 0)
		if (++boguscount > 32) {
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
    ring_offset -= (EL2SM_START_PG<<8);

    /* Maybe enable shared memory just be to be safe... nahh.*/
    if (dev->mem_start) {	/* Use the shared memory. */
	if (dev->mem_start + ring_offset + count > end_of_ring) {
	    /* We must wrap the input move. */
	    int semi_count = end_of_ring - (dev->mem_start + ring_offset);
	    if (ei_debug > 4)
		printk("3c503: block_input() @ %#5x+%x=%5x.\n",
		       dev->mem_start, ring_offset,
		       (char *)dev->mem_start + ring_offset);
	    memcpy(buf, (char *)dev->mem_start + ring_offset, semi_count);
	    count -= semi_count;
	    memcpy(buf + semi_count, (char *)dev->rmem_start, count);
	    return dev->rmem_start + count;
	}
	if (ei_debug > 4)
	    printk("3c503: block_input() @ %#5x+%x=%5x.\n",
		   dev->mem_start, ring_offset,
		   (char *)dev->mem_start + ring_offset);
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
	    if (count % 8 == 7)
		while ((inb(E33G_STATUS) & ESTAT_DPRDY) == 0)
		    if (++boguscount > 32) {
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

/*
 * Local variables:
 *  compile-command: "gcc -DKERNEL -Wall -O6 -fomit-frame-pointer -I/usr/src/linux/net/tcp -c 3c503.c"
 *  version-control: t
 *  kept-new-versions: 5
 * End:
 */
