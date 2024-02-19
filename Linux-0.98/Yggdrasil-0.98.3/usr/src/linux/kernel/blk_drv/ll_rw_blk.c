/*
 *  linux/kernel/blk_dev/ll_rw.c
 *
 * Copyright (C) 1991, 1992 Linus Torvalds
 */

/*
 * This handles all read/write requests to block devices
 */
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/config.h>
#include <linux/locks.h>

#include <asm/system.h>

#include "blk.h"

extern long rd_init(long mem_start, int length);

/*
 * The request-struct contains all necessary data
 * to load a nr of sectors into memory
 */
struct request request[NR_REQUEST];

/*
 * used to wait on when there are no free requests
 */
struct wait_queue * wait_for_request = NULL;

/* blk_dev_struct is:
 *	do_request-address
 *	next-request
 */
struct blk_dev_struct blk_dev[NR_BLK_DEV] = {
	{ NULL, NULL },		/* no_dev */
	{ NULL, NULL },		/* dev mem */
	{ NULL, NULL },		/* dev fd */
	{ NULL, NULL },		/* dev hd */
	{ NULL, NULL },		/* dev ttyx */
	{ NULL, NULL },		/* dev tty */
	{ NULL, NULL },		/* dev lp */
	{ NULL, NULL },		/* dev pipes */
	{ NULL, NULL },		/* dev sd */
	{ NULL, NULL }		/* dev st */
};

/*
 * blk_size contains the size of all block-devices:
 *
 * blk_size[MAJOR][MINOR]
 *
 * if (!blk_size[MAJOR]) then no minor size checking is done.
 */
int * blk_size[NR_BLK_DEV] = { NULL, NULL, };

/* RO fail safe mechanism */

static long ro_bits[NR_BLK_DEV][8];

int is_read_only(int dev)
{
	int minor,major;

	major = MAJOR(dev);
	minor = MINOR(dev);
	if (major < 0 || major >= NR_BLK_DEV) return 0;
	return ro_bits[major][minor >> 5] & (1 << (minor & 31));
}

void set_device_ro(int dev,int flag)
{
	int minor,major;

	major = MAJOR(dev);
	minor = MINOR(dev);
	if (major < 0 || major >= NR_BLK_DEV) return;
	if (flag) ro_bits[major][minor >> 5] |= 1 << (minor & 31);
	else ro_bits[major][minor >> 5] &= ~(1 << (minor & 31));
}

/*
 * add-request adds a request to the linked list.
 * It disables interrupts so that it can muck with the
 * request-lists in peace.
 */
static void add_request(struct blk_dev_struct * dev, struct request * req)
{
	struct request * tmp;

	req->next = NULL;
	cli();
	if (req->bh)
		req->bh->b_dirt = 0;
	if (!(tmp = dev->current_request)) {
		dev->current_request = req;
		(dev->request_fn)();
		sti();
		return;
	}
	for ( ; tmp->next ; tmp = tmp->next) {
		if ((IN_ORDER(tmp,req) ||
		    !IN_ORDER(tmp,tmp->next)) &&
		    IN_ORDER(req,tmp->next))
			break;
	}
	req->next = tmp->next;
	tmp->next = req;
	sti();
}

static void make_request(int major,int rw, struct buffer_head * bh)
{
	unsigned int sector, count;
	struct request * req;
	int rw_ahead;

/* WRITEA/READA is special case - it is not really needed, so if the */
/* buffer is locked, we just forget about it, else it's a normal read */
	if (rw_ahead = (rw == READA || rw == WRITEA)) {
		if (bh->b_lock)
			return;
		if (rw == READA)
			rw = READ;
		else
			rw = WRITE;
	}
	if (rw!=READ && rw!=WRITE) {
		printk("Bad block dev command, must be R/W/RA/WA\n");
		return;
	}
	count = bh->b_size >> 9;
	sector = bh->b_blocknr * count;
	if (blk_size[major])
		if (blk_size[major][MINOR(bh->b_dev)] < (sector + count)>>1) {
			bh->b_dirt = bh->b_uptodate = 0;
			return;
		}
	lock_buffer(bh);
	if ((rw == WRITE && !bh->b_dirt) || (rw == READ && bh->b_uptodate)) {
		unlock_buffer(bh);
		return;
	}
repeat:
	cli();
	if ((major == 3 ||  major == 8 || major == 11)&& (req = blk_dev[major].current_request)) {
		while (req = req->next) {
			if (req->dev == bh->b_dev &&
			    !req->waiting &&
			    req->cmd == rw &&
			    req->sector + req->nr_sectors == sector &&
			    req->nr_sectors < 254) {
				req->bhtail->b_reqnext = bh;
				req->bhtail = bh;
				req->nr_sectors += count;
				bh->b_dirt = 0;
				sti();
				return;
			}
		}
	}
/* we don't allow the write-requests to fill up the queue completely:
 * we want some room for reads: they take precedence. The last third
 * of the requests are only for reads.
 */
	if (rw == READ)
		req = request+NR_REQUEST;
	else
		req = request+(NR_REQUEST/2);
/* find an empty request */
	while (--req >= request)
		if (req->dev < 0)
			goto found;
/* if none found, sleep on new requests: check for rw_ahead */
	if (rw_ahead) {
		sti();
		unlock_buffer(bh);
		return;
	}
	sleep_on(&wait_for_request);
	sti();
	goto repeat;

found:
/* fill up the request-info, and add it to the queue */
	req->dev = bh->b_dev;
	sti();
	req->cmd = rw;
	req->errors = 0;
	req->sector = sector;
	req->nr_sectors = count;
	req->current_nr_sectors = count;
	req->buffer = bh->b_data;
	req->waiting = NULL;
	req->bh = bh;
	req->bhtail = bh;
	req->next = NULL;
	add_request(major+blk_dev,req);
}

void ll_rw_page(int rw, int dev, int page, char * buffer)
{
	struct request * req;
	unsigned int major = MAJOR(dev);

	if (major >= NR_BLK_DEV || !(blk_dev[major].request_fn)) {
		printk("Trying to read nonexistent block-device %04x (%d)\n",dev,page*8);
		return;
	}
	if (rw!=READ && rw!=WRITE)
		panic("Bad block dev command, must be R/W");
	if (rw == WRITE && is_read_only(dev)) {
		printk("Can't page to read-only device 0x%X\n\r",dev);
		return;
	}
	cli();
repeat:
	req = request+NR_REQUEST;
	while (--req >= request)
		if (req->dev<0)
			break;
	if (req < request) {
		sleep_on(&wait_for_request);
		goto repeat;
	}
	sti();
/* fill up the request-info, and add it to the queue */
	req->dev = dev;
	req->cmd = rw;
	req->errors = 0;
	req->sector = page<<3;
	req->nr_sectors = 8;
	req->current_nr_sectors = 8;
	req->buffer = buffer;
	req->waiting = current;
	req->bh = NULL;
	req->next = NULL;
	current->state = TASK_UNINTERRUPTIBLE;
	add_request(major+blk_dev,req);
	schedule();
}

void ll_rw_block(int rw, struct buffer_head * bh)
{
	unsigned int major;

	if (!bh)
		return;
	if (bh->b_size != 1024) {
		printk("ll_rw_block: only 1024-char blocks implemented (%d)\n",bh->b_size);
		bh->b_dirt = bh->b_uptodate = 0;
		return;
	}
	if ((major=MAJOR(bh->b_dev)) >= NR_BLK_DEV ||
	!(blk_dev[major].request_fn)) {
		printk("ll_rw_block: Trying to read nonexistent block-device %04x (%d)\n",bh->b_dev,bh->b_blocknr);
		bh->b_dirt = bh->b_uptodate = 0;
		return;
	}
	if ((rw == WRITE || rw == WRITEA) && is_read_only(bh->b_dev)) {
		printk("Can't write to read-only device 0x%X\n\r",bh->b_dev);
		bh->b_dirt = bh->b_uptodate = 0;
		return;
	}
	make_request(major,rw,bh);
}

void ll_rw_swap_file(int rw, int dev, unsigned int *b, int nb, char *buf)
{
	int i;
	struct request * req;
	unsigned int major = MAJOR(dev);

	if (major >= NR_BLK_DEV || !(blk_dev[major].request_fn)) {
		printk("ll_rw_swap_file: trying to swap nonexistent block-device\n\r");
		return;
	}

	if (rw!=READ && rw!=WRITE) {
		printk("ll_rw_swap: bad block dev command, must be R/W");
		return;
	}
	if (rw == WRITE && is_read_only(dev)) {
		printk("Can't swap to read-only device 0x%X\n\r",dev);
		return;
	}
	
	for (i=0; i<nb; i++, buf += BLOCK_SIZE)
	{
repeat:
		req = request+NR_REQUEST;
		while (--req >= request)
			if (req->dev<0)
				break;
		if (req < request) {
			sleep_on(&wait_for_request);
			goto repeat;
		}

		req->dev = dev;
		req->cmd = rw;
		req->errors = 0;
		req->sector = b[i] << 1;
		req->nr_sectors = 2;
		req->current_nr_sectors = 2;
		req->buffer = buf;
		req->waiting = current;
		req->bh = NULL;
		req->next = NULL;
		current->state = TASK_UNINTERRUPTIBLE;
		add_request(major+blk_dev,req);
		schedule();
	}
}

long blk_dev_init(long mem_start, long mem_end)
{
	int i;

	for (i=0 ; i<NR_REQUEST ; i++) {
		request[i].dev = -1;
		request[i].next = NULL;
	}
	memset(ro_bits,0,sizeof(ro_bits));
#ifdef CONFIG_BLK_DEV_HD
	mem_start = hd_init(mem_start,mem_end);
#endif
#ifdef RAMDISK
	mem_start += rd_init(mem_start, RAMDISK*1024);
#endif
	return mem_start;
}
