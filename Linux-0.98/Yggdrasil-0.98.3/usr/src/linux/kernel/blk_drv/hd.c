/*
 *  linux/kernel/hd.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * This is the low-level hd interrupt support. It traverses the
 * request-list, using interrupts to jump between functions. As
 * all the functions are called within interrupts, we may not
 * sleep. Special care is recommended.
 * 
 *  modified by Drew Eckhardt to check nr of hd's from the CMOS.
 *
 *  Thanks to Branko Lankester, lankeste@fwi.uva.nl, who found a bug
 *  in the early extended-partition checks and added DM partitions
 */

#include <linux/config.h>
#ifdef CONFIG_BLK_DEV_HD

#define HD_IRQ 14

#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/hdreg.h>
#include <linux/genhd.h>

#define REALLY_SLOW_IO
#include <asm/system.h>
#include <asm/io.h>
#include <asm/segment.h>

#define MAJOR_NR 3
#include "blk.h"

extern void resetup_one_dev(struct gendisk *, unsigned int);
static int revalidate_hddisk(int, int);

static inline unsigned char CMOS_READ(unsigned char addr)
{
	outb_p(0x80|addr,0x70);
	return inb_p(0x71);
}

#define	HD_DELAY	0

/* Max read/write errors/sector */
#define MAX_ERRORS	7
#define MAX_HD		2

static void recal_intr(void);
static void bad_rw_intr(void);

static char recalibrate[ MAX_HD ] = { 0, };
static int access_count[MAX_HD] = {0, };
static char busy[MAX_HD] = {0, };
static struct wait_queue * busy_wait = NULL;

static int reset = 0;

#if (HD_DELAY > 0)
unsigned long last_req, read_timer();
#endif

/*
 *  This struct defines the HD's and their types.
 */
struct hd_i_struct {
	unsigned int head,sect,cyl,wpcom,lzone,ctl;
	};
#ifdef HD_TYPE
struct hd_i_struct hd_info[] = { HD_TYPE };
#define NR_HD ((sizeof (hd_info))/(sizeof (struct hd_i_struct)))
#else
struct hd_i_struct hd_info[] = { {0,0,0,0,0,0},{0,0,0,0,0,0} };
static int NR_HD = 0;
#endif

static struct hd_struct hd[MAX_HD<<6]={{0,0},};
static int hd_sizes[MAX_HD<<6] = {0, };

#define port_read(port,buf,nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr):"cx","di")

#define port_write(port,buf,nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr):"cx","si")

#if (HD_DELAY > 0)
unsigned long read_timer(void)
{
	unsigned long t;
	int i;

	cli();
    	outb_p(0xc2, 0x43);
	t = jiffies * 11931 + (inb_p(0x40) & 0x80 ? 5966 : 11932);
	i = inb_p(0x40);
	i |= inb(0x40) << 8;
	sti();
	return(t - i / 2);
}
#endif

static int controller_ready(void)
{
	int retries = 100000;

	while (--retries && (inb_p(HD_STATUS)&0x80))
		/* nothing */;
	if (!retries)
		printk("controller_ready: status = %02x\n\r",
			(unsigned char) inb_p(HD_STATUS));
	return (retries);
}

static int win_result(void)
{
	int i=inb_p(HD_STATUS);

	if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT))
		== (READY_STAT | SEEK_STAT))
		return 0; /* ok */
	printk("HD: win_result: status = 0x%02x\n",i);
	if (i&1) {
		i=inb(HD_ERROR);
		printk("HD: win_result: error = 0x%02x\n",i);
	}	
	return 1;
}

static void hd_out(unsigned int drive,unsigned int nsect,unsigned int sect,
		unsigned int head,unsigned int cyl,unsigned int cmd,
		void (*intr_addr)(void))
{
	unsigned short port;

	if (drive>1 || head>15)
		panic("Trying to write bad sector");
#if (HD_DELAY > 0)
	while (read_timer() - last_req < HD_DELAY)
		/* nothing */;
#endif
	if (reset || !controller_ready()) {
		reset = 1;
		return;
	}
	SET_INTR(intr_addr);
	outb_p(hd_info[drive].ctl,HD_CMD);
	port=HD_DATA;
	outb_p(hd_info[drive].wpcom>>2,++port);
	outb_p(nsect,++port);
	outb_p(sect,++port);
	outb_p(cyl,++port);
	outb_p(cyl>>8,++port);
	outb_p(0xA0|(drive<<4)|head,++port);
	outb_p(cmd,++port);
}

static int drive_busy(void)
{
	unsigned int i;
	unsigned char c;

	for (i = 0; i < 500000 ; i++) {
		c = inb_p(HD_STATUS);
		c &= (BUSY_STAT | READY_STAT | SEEK_STAT);
		if (c == (READY_STAT | SEEK_STAT))
			return 0;
	}
	printk("HD controller times out, status = 0x%02x\n\r",c);
	return(1);
}

static void reset_controller(void)
{
	int	i;

	printk("HD-controller reset\r\n");
	outb(4,HD_CMD);
	for(i = 0; i < 1000; i++) nop();
	outb(hd_info[0].ctl & 0x0f ,HD_CMD);
	if (drive_busy())
		printk("HD-controller still busy\n\r");
	if ((i = inb(HD_ERROR)) != 1)
		printk("HD-controller reset failed: %02x\n\r",i);
}

static void reset_hd(void)
{
	static int i;

repeat:
	if (reset) {
		reset = 0;
		i = -1;
		reset_controller();
	} else if (win_result()) {
		bad_rw_intr();
		if (reset)
			goto repeat;
	}
	i++;
	if (i < NR_HD) {
		hd_out(i,hd_info[i].sect,hd_info[i].sect,hd_info[i].head-1,
			hd_info[i].cyl,WIN_SPECIFY,&reset_hd);
		if (reset)
			goto repeat;
	} else
		do_hd_request();
}

/*
 * Ok, don't know what to do with the unexpected interrupts: on some machines
 * doing a reset and a retry seems to result in an eternal loop. Right now I
 * ignore it, and just set the timeout.
 */
void unexpected_hd_interrupt(void)
{
	sti();
	printk("Unexpected HD interrupt\n\r");
	SET_TIMER;
}

static void bad_rw_intr(void)
{
	int i;

	if (!CURRENT)
		return;
	if (++CURRENT->errors >= MAX_ERRORS)
		end_request(0);
	else if (CURRENT->errors > MAX_ERRORS/2)
		reset = 1;
	else
		for (i=0; i < NR_HD; i++)
			recalibrate[i] = 1;
}

static inline int wait_DRQ(void)
{
	int retries = 100000;

	while (--retries > 0)
		if (inb_p(HD_STATUS) & DRQ_STAT)
			return 0;
	return -1;
}

#define STAT_MASK (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT)
#define STAT_OK (READY_STAT | SEEK_STAT)

static void read_intr(void)
{
	int i;
	int retries = 100000;

	do {
		i = (unsigned) inb_p(HD_STATUS);
		if ((i & STAT_MASK) != STAT_OK)
			break;
		if (i & DRQ_STAT)
			goto ok_to_read;
	} while (--retries > 0);
	sti();
	printk("HD: read_intr: status = 0x%02x\n",i);
	if (i & ERR_STAT) {
		i = (unsigned) inb(HD_ERROR);
		printk("HD: read_intr: error = 0x%02x\n",i);
	}
	bad_rw_intr();
	cli();
	do_hd_request();
	return;
ok_to_read:
	port_read(HD_DATA,CURRENT->buffer,256);
	CURRENT->errors = 0;
	CURRENT->buffer += 512;
	CURRENT->sector++;
	i = --CURRENT->nr_sectors;
	--CURRENT->current_nr_sectors;
#ifdef DEBUG
	printk("hd%d : sector = %d, %d remaining to buffer = %08x\n",
		MINOR(CURRENT->dev), CURRENT->sector, i, CURRENT-> 
		buffer);
#endif
	if (!i || (CURRENT->bh && !SUBSECTOR(i)))
		end_request(1);
	if (i > 0) {
		SET_INTR(&read_intr);
		sti();
		return;
	}
	(void) inb_p(HD_STATUS);
#if (HD_DELAY > 0)
	last_req = read_timer();
#endif
	do_hd_request();
	return;
}

static void write_intr(void)
{
	int i;
	int retries = 100000;

	do {
		i = (unsigned) inb_p(HD_STATUS);
		if ((i & STAT_MASK) != STAT_OK)
			break;
		if ((CURRENT->nr_sectors <= 1) || (i & DRQ_STAT))
			goto ok_to_write;
	} while (--retries > 0);
	sti();
	printk("HD: write_intr: status = 0x%02x\n",i);
	if (i & ERR_STAT) {
		i = (unsigned) inb(HD_ERROR);
		printk("HD: write_intr: error = 0x%02x\n",i);
	}
	bad_rw_intr();
	cli();
	do_hd_request();
	return;
ok_to_write:
	CURRENT->sector++;
	i = --CURRENT->nr_sectors;
	--CURRENT->current_nr_sectors;
	CURRENT->buffer += 512;
	if (!i || (CURRENT->bh && !SUBSECTOR(i)))
		end_request(1);
	if (i > 0) {
		SET_INTR(&write_intr);
		port_write(HD_DATA,CURRENT->buffer,256);
		sti();
	} else {
#if (HD_DELAY > 0)
		last_req = read_timer();
#endif
		do_hd_request();
	}
	return;
}

static void recal_intr(void)
{
	if (win_result())
		bad_rw_intr();
	do_hd_request();
}

/*
 * This is another of the error-routines I don't know what to do with. The
 * best idea seems to just set reset, and start all over again.
 */
static void hd_times_out(void)
{
	DEVICE_INTR = NULL;
	sti();
	reset = 1;
	if (!CURRENT)
		return;
	printk("HD timeout\n\r");
	cli();
	if (++CURRENT->errors >= MAX_ERRORS) {
#ifdef DEBUG
		printk("hd : too many errors.\n");
#endif
		end_request(0);
	}

	do_hd_request();
}

/*
 * The driver has been modified to enable interrupts a bit more: in order to
 * do this we first (a) disable the timeout-interrupt and (b) clear the
 * device-interrupt. This way the interrupts won't mess with out code (the
 * worst that can happen is that an unexpected HD-interrupt comes in and
 * sets the "reset" variable and starts the timer)
 */
static void do_hd_request(void)
{
	unsigned int block,dev;
	unsigned int sec,head,cyl,track;
	unsigned int nsect;

	if (DEVICE_INTR)
		return;
repeat:
	timer_active &= ~(1<<HD_TIMER);
	sti();
	INIT_REQUEST;
	dev = MINOR(CURRENT->dev);
	block = CURRENT->sector;
	nsect = CURRENT->nr_sectors;
	if (dev >= (NR_HD<<6) || block >= hd[dev].nr_sects) {
#ifdef DEBUG
		printk("hd%d : attempted read for sector %d past end of device at sector %d.\n",
		   	block, hd[dev].nr_sects);
#endif
		end_request(0);
		goto repeat;
	}
	block += hd[dev].start_sect;
	dev >>= 6;
	sec = block % hd_info[dev].sect + 1;
	track = block / hd_info[dev].sect;
	head = track % hd_info[dev].head;
	cyl = track / hd_info[dev].head;
#ifdef DEBUG
	printk("hd%d : cyl = %d, head = %d, sector = %d, buffer = %08x\n",
		dev, cyl, head, sec, CURRENT->buffer);
#endif
	cli();
	if (reset) {
		int i;

		for (i=0; i < NR_HD; i++)
			recalibrate[i] = 1;
		reset_hd();
		sti();
		return;
	}
	if (recalibrate[dev]) {
		recalibrate[dev] = 0;
		hd_out(dev,hd_info[dev].sect,0,0,0,WIN_RESTORE,&recal_intr);
		if (reset)
			goto repeat;
		sti();
		return;
	}	
	if (CURRENT->cmd == WRITE) {
		hd_out(dev,nsect,sec,head,cyl,WIN_WRITE,&write_intr);
		if (reset)
			goto repeat;
		if (wait_DRQ()) {
			printk("HD: do_hd_request: no DRQ\n");
			bad_rw_intr();
			goto repeat;
		}
		port_write(HD_DATA,CURRENT->buffer,256);
		sti();
		return;
	}
	if (CURRENT->cmd == READ) {
		hd_out(dev,nsect,sec,head,cyl,WIN_READ,&read_intr);
		if (reset)
			goto repeat;
		sti();
		return;
	}
	panic("unknown hd-command");
}

static int hd_ioctl(struct inode * inode, struct file * file,
	unsigned int cmd, unsigned int arg)
{
	struct hd_geometry *loc = (void *) arg;
	int dev;

	if (!inode)
		return -EINVAL;
	dev = MINOR(inode->i_rdev) >> 6;
	if (dev >= NR_HD)
		return -EINVAL;
	switch (cmd) {
		case HDIO_REQ:
			if (!loc)  return -EINVAL;
			verify_area(loc, sizeof(*loc));
			put_fs_byte(hd_info[dev].head,
				(char *) &loc->heads);
			put_fs_byte(hd_info[dev].sect,
				(char *) &loc->sectors);
			put_fs_word(hd_info[dev].cyl,
				(short *) &loc->cylinders);
			put_fs_long(hd[MINOR(inode->i_rdev)].start_sect,
				(long *) &loc->start);
			return 0;
		case BLKRRPART: /* Re-read partition tables */
			return revalidate_hddisk(inode->i_rdev, 1);
		RO_IOCTLS(inode->i_rdev,arg);
		default:
			return -EINVAL;
	}
}

static int hd_open(struct inode * inode, struct file * filp)
{
	int target;
	target =  DEVICE_NR(MINOR(inode->i_rdev));

	while (busy[target])
		sleep_on(&busy_wait);
	access_count[target]++;
	return 0;
}

/*
 * Releasing a block device means we sync() it, so that it can safely
 * be forgotten about...
 */
static void hd_release(struct inode * inode, struct file * file)
{
        int target;
	sync_dev(inode->i_rdev);

	target =  DEVICE_NR(MINOR(inode->i_rdev));
	access_count[target]--;

}

static void hd_geninit();

static struct gendisk hd_gendisk = {
	MAJOR_NR,	/* Major number */	
	"hd",		/* Major name */
	6,		/* Bits to shift to get real from partition */
	1 << 6,		/* Number of partitions per real */
	MAX_HD,		/* maximum number of real */
	hd_geninit,	/* init function */
	hd,		/* hd struct */
	hd_sizes,	/* block sizes */
	0,		/* number */
	(void *) hd_info,	/* internal */
	NULL		/* next */
};
	
static void hd_interrupt(int unused)
{
	void (*handler)(void) = DEVICE_INTR;

	DEVICE_INTR = NULL;
	timer_active &= ~(1<<HD_TIMER);
	if (!handler)
		handler = unexpected_hd_interrupt;
	handler();
	sti();
}

/*
 * This is the harddisk IRQ description. The SA_INTERRUPT in sa_flags
 * means we run the IRQ-handler with interrupts disabled: this is bad for
 * interrupt latency, but anything else has led to problems on some
 * machines...
 *
 * We enable interrupts in some of the routines after making sure it's
 * safe.
 */
static struct sigaction hd_sigaction = {
	hd_interrupt,
	0,
	SA_INTERRUPT,
	NULL
};

static void hd_geninit(void)
{
	int drive, i;
#ifndef HD_TYPE
	extern struct drive_info drive_info;
	void *BIOS = (void *) &drive_info;
	int cmos_disks;
	   
	for (drive=0 ; drive<2 ; drive++) {
		hd_info[drive].cyl = *(unsigned short *) BIOS;
		hd_info[drive].head = *(unsigned char *) (2+BIOS);
		hd_info[drive].wpcom = *(unsigned short *) (5+BIOS);
		hd_info[drive].ctl = *(unsigned char *) (8+BIOS);
		hd_info[drive].lzone = *(unsigned short *) (12+BIOS);
		hd_info[drive].sect = *(unsigned char *) (14+BIOS);
		BIOS += 16;
	}

	/*
		We querry CMOS about hard disks : it could be that 
		we have a SCSI/ESDI/etc controller that is BIOS
		compatable with ST-506, and thus showing up in our
		BIOS table, but not register compatable, and therefore
		not present in CMOS.

		Furthurmore, we will assume that our ST-506 drives
		<if any> are the primary drives in the system, and 
		the ones reflected as drive 1 or 2.

		The first drive is stored in the high nibble of CMOS
		byte 0x12, the second in the low nibble.  This will be
		either a 4 bit drive type or 0xf indicating use byte 0x19 
		for an 8 bit type, drive 1, 0x1a for drive 2 in CMOS.

		Needless to say, a non-zero value means we have 
		an AT controller hard disk for that drive.

		
	*/

	if ((cmos_disks = CMOS_READ(0x12)) & 0xf0)
		if (cmos_disks & 0x0f)
			NR_HD = 2;
		else
			NR_HD = 1;
	else
		NR_HD = 0;
	if (NR_HD) {
		if (irqaction(HD_IRQ,&hd_sigaction)) {
			printk("Unable to get IRQ%d for the harddisk driver\n",HD_IRQ);
			NR_HD = 0;
		}
	}
#endif
	for (i = 0 ; i < NR_HD ; i++)
		hd[i<<6].nr_sects = hd_info[i].head*
				hd_info[i].sect*hd_info[i].cyl;

	hd_gendisk.nr_real = NR_HD;
}

static struct file_operations hd_fops = {
	NULL,			/* lseek - default */
	block_read,		/* read - general block-dev read */
	block_write,		/* write - general block-dev write */
	NULL,			/* readdir - bad */
	NULL,			/* select */
	hd_ioctl,		/* ioctl */
	NULL,			/* mmap */
	hd_open,		/* open */
	hd_release		/* release */
};

unsigned long hd_init(unsigned long mem_start, unsigned long mem_end)
{
	blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;
	blkdev_fops[MAJOR_NR] = &hd_fops;
	hd_gendisk.next = gendisk_head;
	gendisk_head = &hd_gendisk;
	timer_table[HD_TIMER].fn = hd_times_out;
	return mem_start;
}

#define DEVICE_BUSY busy[target]
#define USAGE access_count[target]
#define CAPACITY (hd_info[target].head*hd_info[target].sect*hd_info[target].cyl)
/* We assume that the the bios parameters do not change, so the disk capacity
   will not change */
#undef MAYBE_REINIT
#define GENDISK_STRUCT hd_gendisk

/*
 * This routine is called to flush all partitions and partition tables
 * for a changed scsi disk, and then re-read the new partition table.
 * If we are revalidating a disk because of a media change, then we
 * enter with usage == 0.  If we are using an ioctl, we automatically have
 * usage == 1 (we need an open channel to use an ioctl :-), so this
 * is our limit.
 */
static int revalidate_hddisk(int dev, int maxusage)
{
	int target, major;
	struct gendisk * gdev;
	int max_p;
	int start;
	int i;

	target =  DEVICE_NR(MINOR(dev));
	gdev = &GENDISK_STRUCT;

	cli();
	if (DEVICE_BUSY || USAGE > maxusage) {
		sti();
		return -EBUSY;
	};
	DEVICE_BUSY = 1;
	sti();

	max_p = gdev->max_p;
	start = target << gdev->minor_shift;
	major = MAJOR_NR << 8;

	for (i=max_p - 1; i >=0 ; i--) {
		sync_dev(major | start | i);
		invalidate_inodes(major | start | i);
		invalidate_buffers(major | start | i);
		gdev->part[i].start_sect = 0;
		gdev->part[i].nr_sects = 0;
	};

#ifdef MAYBE_REINIT
	MAYBE_REINIT;
#endif

	gdev->part[start].nr_sects = CAPACITY;
	resetup_one_dev(gdev, target);

	DEVICE_BUSY = 0;
	wake_up(&busy_wait);
	return 0;
}

#endif
