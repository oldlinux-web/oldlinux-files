/* ne.c: A general non-shared-memory NS8390 ethernet driver for linux. */
/*
    Written 1992,1993 by Donald Becker. This is alpha test code.
    This is a extension to the Linux operating system, and is covered by
    same Gnu Public License that covers that work.
    
    This driver should work with many 8390-based ethernet boards.  Currently
    it support the NE1000, NE2000 (and clones), and some Cabletron products.
    8-bit ethercard support is enabled with  #define EI_8BIT

    The Author may be reached as becker@super.org or
    C/O Supercomputing Research Ctr., 17100 Science Dr., Bowie MD 20715
*/

/* Routines for the NatSemi-based designs (NE[12]000). */

static char *version =
    "ne.c:v0.50 2/19/93 Donald Becker (becker@super.org)\n";

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/system.h>

#include "dev.h"
#include "8390.h"

/* These should be in <asm/io.h> someday, borrowed from blk_drv/hd.c. */
#define port_read(port,buf,nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr):"cx","di")
#define port_write(port,buf,nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr):"cx","si")

#define port_read_b(port,buf,nr) \
__asm__("cld;rep;insb"::"d" (port),"D" (buf),"c" (nr):"cx","di")
#define port_write_b(port,buf,nr) \
__asm__("cld;rep;outsb"::"d" (port),"S" (buf),"c" (nr):"cx","si")

#define EN_CMD	 (dev->base_addr)
#define NE_BASE	 (dev->base_addr)
#define NE_DATAPORT	0x10	/* NatSemi-defined port window offset. */
#define NE_RESET	0x1f	/* Issue a read to reset, a write to clear. */

#define NE1SM_START_PG	0x20	/* First page of TX buffer */
#define NE1SM_STOP_PG 	0x40	/* Last page +1 of RX ring */
#define NESM_START_PG	0x40	/* First page of TX buffer */
#define NESM_STOP_PG	0x80	/* Last page +1 of RX ring */

extern void NS8390_init(struct device *dev, int startp);
extern int ei_debug;
extern struct sigaction ei_sigaction;
extern struct ei_device ei_status;

int neprobe(int ioaddr, struct device *dev);
static int neprobe1(int ioaddr, struct device *dev);

static void ne_reset_8390(struct device *dev);
static int ne_block_input(struct device *dev, int count,
			  char *buf, int ring_offset);
static void ne_block_output(struct device *dev, const int count,
		const unsigned char *buf, const int start_page);
static void ne_init_card(struct device *dev);


/*  Probe for the NE1000 and NE2000.  NEx000-like boards have 0x57,0x57 in
    bytes 0x0e,0x0f of the SAPROM, but if we read by 16 bit words the NE1000
    appears to have 0x00, 0x42. */
/* Also initialize the card and fill in STATION_ADDR with the station
   address.  The station address (and other data) is stored in the
   packet buffer memory space, 32 bytes starting at remote DMA address 0. */

int neprobe(int ioaddr,  struct device *dev)
{
    int *port, ports[] = {0x300, 0x320, 0x340, 0x360, 0};

    if (ioaddr > 0x100)
	return neprobe1(ioaddr, dev);

    for (port = &ports[0]; *port; port++)
	if (inb_p(*port) != 0xff && neprobe1(*port, dev))
	    return dev->base_addr = *port;
    return 0;
}

static int neprobe1(int ioaddr, struct device *dev)
{
  int i;
  unsigned char *station_addr = dev->dev_addr;
  unsigned char SA_prom[32];
  int cmdreg;
  int ne2000 = 0, ne1000 = 0, ctron = 0, dlink = 0;

  printk("8390 ethercard probe at %#3x:", ioaddr);

  cmdreg = inb_p(ioaddr);
  if (cmdreg == 0xFF) {
    printk(" not found (%#2.2x).\n", cmdreg);
    return 0;
  }
  /* Ooops, we must first initialize registers -- we can't just read the PROM
     address right away.  (Learned the hard way.) */
  /* NS8390_init(eifdev, 0);*/
  outb_p(E8390_NODMA+E8390_PAGE0+E8390_STOP, ioaddr);
#ifdef EI_8BIT
  outb_p(0x48, ioaddr + EN0_DCFG); 		/* Set byte-wide for probe. */
#else
  outb_p(0x49, ioaddr + EN0_DCFG); 		/* Set word-wide for probe. */
#endif
  /* Even though we'll set them soon, we must clear them! */
  outb_p(0x00, ioaddr + EN0_RCNTLO);
  outb_p(0x00, ioaddr + EN0_RCNTHI);

  outb_p(0x00, ioaddr + EN0_IMR); 		/* Mask completion irq. */
  outb_p(0xFF, ioaddr + EN0_ISR);

  /* Set to monitor and loopback mode. */
  outb_p(E8390_RXOFF, ioaddr + EN0_RXCR); /* 0x20 */
  outb_p(E8390_TXOFF, ioaddr + EN0_TXCR); /* 0x02 */


#ifdef EI_8BIT
  outb_p(sizeof(SA_prom), ioaddr + EN0_RCNTLO);
#else
  /* Double count 0x20 words, the SA PROM is only byte wide. */
  outb_p(2*sizeof(SA_prom), ioaddr + EN0_RCNTLO);
#endif
  outb_p(0x00, ioaddr + EN0_RCNTHI);
  outb_p(0x00, ioaddr + EN0_RSARLO);	/* DMA starting at 0x0000. */
  outb_p(0x00, ioaddr + EN0_RSARHI);
  outb_p(E8390_RREAD+E8390_START, ioaddr);
  for(i = 0; i < sizeof(SA_prom); i++) {
    SA_prom[i] = inb_p(ioaddr + NE_DATAPORT);
    if (i < ETHER_ADDR_LEN  &&  station_addr) {
      printk(" %2.2x", SA_prom[i]);
      station_addr[i] = SA_prom[i];
    }
  }
#ifdef EI_8BIT
  ne1000 = (SA_prom[14] == 0x57  &&  SA_prom[15] == 0x57);
  ctron =  (SA_prom[0] == 0x00 && SA_prom[1] == 0x00 && SA_prom[2] == 0x1d);
  dlink =  (SA_prom[0] == 0x00 && SA_prom[1] == 0xDE && SA_prom[2] == 0x01);
#else
  ne2000 = (SA_prom[14] == 0x57  &&  SA_prom[15] == 0x57);
  ne1000 = (SA_prom[14] == 0x00  &&  SA_prom[15] == 0x42);
  ctron =  (SA_prom[0] == 0x00 && SA_prom[1] == 0x00 && SA_prom[2] == 0x1d);
  dlink =  (SA_prom[0] == 0x00 && SA_prom[1] == 0xDE && SA_prom[2] == 0x01);
#endif

  /* Set up the rest of the parameters. */
  if (ne1000 || dlink) {
      ei_status.name = ne1000 ? "NE1000" : "D-Link";
      ei_status.word16 = 0;
      ei_status.tx_start_page = NE1SM_START_PG;
      ei_status.rx_start_page = NE1SM_START_PG + TX_PAGES;
      ei_status.stop_page = NE1SM_STOP_PG;
  } else if (ne2000) {
      ei_status.name = "NE2000";
      ei_status.word16 = 1;
      ei_status.tx_start_page = NESM_START_PG;
      ei_status.rx_start_page = NESM_START_PG + TX_PAGES;
      ei_status.stop_page = NESM_STOP_PG;
  } else if (ctron) {
      /* You'll have to set these yourself, but this info might be useful.
	 Cabletron packet buffer locations:
	 E1010   starts at 0x100 and ends at 0x2000.
	 E1010-x starts at 0x100 and ends at 0x8000. ("-x" means "more memory")
	 E2010	 starts at 0x100 and ends at 0x4000.
	 E2010-x starts at 0x100 and ends at 0xffff.  */
      ei_status.name = "Cabletron";
#ifdef EI_8BIT
      ei_status.word16 = 0;
#else
      ei_status.word16 = 1;
#endif
      ei_status.tx_start_page = 0x01;
      ei_status.rx_start_page = 0x01 + TX_PAGES;
#ifndef CTRON_MEMSIZE
#define CTRON_MEMSIZE 0x20	/* Extra safe... */
#endif
      ei_status.stop_page = CTRON_MEMSIZE;
  } else {
      printk(" not found.\n");
      return 0;
  }

  dev->base_addr = ioaddr;

  if (dev->irq < 2) {
      int nic_base = dev->base_addr;
      autoirq_setup(0);
      outb_p(0x50, nic_base + EN0_IMR);	/* Enable one interrupt. */
      outb_p(0x00, nic_base + EN0_RCNTLO);
      outb_p(0x00, nic_base + EN0_RCNTHI);
      outb_p(E8390_RREAD+E8390_START, nic_base); /* Trigger it... */
      outb_p(0x00, nic_base + EN0_IMR); 		/* Mask it again. */
      dev->irq = autoirq_report(0);
      if (ei_debug > 2)
	  printk(" autoirq is %d", dev->irq);
  } else if (dev->irq == 2)
      /* Fixup for users that don't know that IRQ 2 is really IRQ 9,
	 or don't know which one to set. */
      dev->irq = 9;

  /* Snarf the interrupt now.  There's no point in waiting since we cannot
     share and the board will usually be enabled. */
  { int irqval = irqaction (dev->irq, &ei_sigaction);
    if (irqval) {
	printk (" unable to get IRQ %d (irqval=%d).\n", dev->irq, irqval);
	return 0;
    }
  }

  printk(" %s found, using IRQ %d.\n", ei_status.name, dev->irq);
  if (ei_debug > 1)
      printk(version);
  ei_status.reset_8390 = &ne_reset_8390;
  ei_status.block_input = &ne_block_input;
  ei_status.block_output = &ne_block_output;
  ne_init_card(dev);
  return dev->base_addr;
}

static void
ne_reset_8390(struct device *dev)
{
    int tmp = inb_p(NE_BASE + NE_RESET);
    int reset_start_time = jiffies;

    if (ei_debug > 1) printk("resetting the 8390 t=%d...", jiffies);
    ei_status.txing = 0;

    sti();
    /* We shouldn't use the boguscount for timing, but this hasn't been
       checked yet, and you could hang your machine if jiffies break... */
    {
	int boguscount = 150000;
	while(jiffies - reset_start_time < 2)
	    if (boguscount-- < 0) {
		printk("jiffy failure (t=%d)...", jiffies);
		outb_p(tmp, NE_BASE + NE_RESET);
		return;
	    }
    }

    outb_p(tmp, NE_BASE + NE_RESET);
    while ((inb_p(NE_BASE+EN0_ISR) & ENISR_RESET) == 0)
	if (jiffies - reset_start_time > 2) {
	    printk(EI_NAME": ne_reset_8390() did not complete.\n");
	    break;
	}
}

/* Block input and output, similar to the Crynwr packet driver.  If you
   porting to a new ethercard look at the packet driver source for hints.
   The NEx000 doesn't share it on-board packet memory -- you have to put
   the packet out through the "remote DMA" dataport using outb. */

static int
ne_block_input(struct device *dev, int count, char *buf, int ring_offset)
{
    int xfer_count = count;
    int nic_base = NE_BASE;

    outb_p(E8390_NODMA+E8390_PAGE0+E8390_START, EN_CMD);
    outb_p(count & 0xff, nic_base + EN0_RCNTLO);
    outb_p(count >> 8, nic_base + EN0_RCNTHI);
    outb_p(ring_offset & 0xff, nic_base + EN0_RSARLO);
    outb_p(ring_offset >> 8, nic_base + EN0_RSARHI);
    outb_p(E8390_RREAD+E8390_START, EN_CMD);
    if (ei_status.word16) {
      port_read(NE_BASE + NE_DATAPORT,buf,count>>1);
      if (count & 0x01)
	buf[count-1] = inb(NE_BASE + NE_DATAPORT), xfer_count++;
    } else {
	port_read_b(NE_BASE + NE_DATAPORT, buf, count);
    }

    /* This was for the ALPHA version only, but enough people have
       encountering problems that it is still here. */
    if (ei_debug > 0) {		/* DMA termination address check... */
	int addr, tries = 10;
	do {
	    /* DON'T check for 'inb_p(EN0_ISR) & ENISR_RDC' here
	       -- it's broken! Check the "DMA" address instead. */
	    int high = inb_p(nic_base + EN0_RSARHI);
	    int low = inb_p(nic_base + EN0_RSARLO);
	    addr = (high << 8) + low;
	    if (((ring_offset + xfer_count) & 0xff) == low)
		return ring_offset + count;
	} while (--tries > 0);
	printk(EI_NAME": RX transfer address mismatch, %#4.4x (should be) vs. %#4.4x (actual).\n",
	       ring_offset + xfer_count, addr);
    }
    return ring_offset + count;
}

static void
ne_block_output(struct device *dev, int count,
		const unsigned char *buf, const int start_page)
{
    int retries = 0;
    int nic_base = NE_BASE;

    /* Round the count up for word writes.  Do we need to do this?
       What effect will an odd byte count have on the 8390?
       I should check someday. */
    if (ei_status.word16 && (count & 0x01))
      count++;
    /* We should already be in page 0, but to be safe... */
    outb_p(E8390_PAGE0+E8390_START+E8390_NODMA, EN_CMD);

 retry:
#if defined(rw_bugfix)
    /* Handle the read-before-write bug the same way as the
       Crynwr packet driver -- the NatSemi method doesn't work.
       Actually this doesn't aways work either, but if you have
       problems with your NEx000 this is better than nothing! */
    outb_p(0x42, nic_base + EN0_RCNTLO);
    outb_p(0x00,   nic_base + EN0_RCNTHI);
    outb_p(0x42, nic_base + EN0_RSARLO);
    outb_p(0x00, nic_base + EN0_RSARHI);
    outb_p(E8390_RREAD+E8390_START, EN_CMD);
    /* Make certain that the dummy read has occured. */
    SLOW_DOWN_IO;
    SLOW_DOWN_IO;
    SLOW_DOWN_IO;
#endif  /* rw_bugfix */

    /* Now the normal output. */
    outb_p(count & 0xff, nic_base + EN0_RCNTLO);
    outb_p(count >> 8,   nic_base + EN0_RCNTHI);
    outb_p(0x00, nic_base + EN0_RSARLO);
    outb_p(start_page, nic_base + EN0_RSARHI);

    outb_p(E8390_RWRITE+E8390_START, EN_CMD);
    if (ei_status.word16) {
	port_write(NE_BASE + NE_DATAPORT, buf, count>>1);
    } else {
	port_write_b(NE_BASE + NE_DATAPORT, buf, count);
    }

    /* This was for the ALPHA version only, but enough people have
       encountering problems that it is still here. */
    if (ei_debug > 0) {		/* DMA termination address check... */
	int addr, tries = 10;
	do {
	    /* DON'T check for 'inb_p(EN0_ISR) & ENISR_RDC' here
	       -- it's broken! Check the "DMA" address instead. */
	    int high = inb_p(nic_base + EN0_RSARHI);
	    int low = inb_p(nic_base + EN0_RSARLO);
	    addr = (high << 8) + low;
	    if ((start_page << 8) + count == addr)
		return;
	} while (--tries > 0);
	printk(EI_NAME": Packet buffer transfer address mismatch on TX, %#4.4x vs. %#4.4x.\n",
	       (start_page << 8) + count, addr);
	if (retries++ == 0)
	    goto retry;
    }
    return;
}

/* This function resets the ethercard if something screws up. */
static void
ne_init_card(struct device *dev)
{
  NS8390_init(dev, 0);
  return;
}


/*
 * Local variables:
 *  compile-command: "gcc -DKERNEL -Wall -O6 -fomit-frame-pointer -I/usr/src/linux/net/tcp -c ne.c"
 *  version-control: t
 *  kept-new-versions: 5
 * End:
 */
