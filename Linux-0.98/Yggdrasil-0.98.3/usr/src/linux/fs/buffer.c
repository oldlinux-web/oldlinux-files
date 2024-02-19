/*
 *  linux/fs/buffer.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 *  'buffer.c' implements the buffer-cache functions. Race-conditions have
 * been avoided by NEVER letting a interrupt change a buffer (except for the
 * data, of course), but instead letting the caller do it.
 */

/*
 * NOTE! There is one discordant note here: checking floppies for
 * disk change. This is where it fits best, I think, as it should
 * invalidate changed floppy-disk-caches.
 */

#include <stdarg.h>
 
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/locks.h>

#include <asm/system.h>
#include <asm/io.h>

#ifdef CONFIG_SCSI
#ifdef CONFIG_BLK_DEV_SR
extern int check_cdrom_media_change(int, int);
#endif
#ifdef CONFIG_BLK_DEV_SD
extern int check_scsidisk_media_change(int, int);
extern int revalidate_scsidisk(int, int);
#endif
#endif

static struct buffer_head * hash_table[NR_HASH];
static struct buffer_head * free_list = NULL;
static struct buffer_head * unused_list = NULL;
static struct wait_queue * buffer_wait = NULL;

int nr_buffers = 0;
int nr_buffer_heads = 0;

/*
 * Rewrote the wait-routines to use the "new" wait-queue functionality,
 * and getting rid of the cli-sti pairs. The wait-queue routines still
 * need cli-sti, but now it's just a couple of 386 instructions or so.
 *
 * Note that the real wait_on_buffer() is an inline function that checks
 * if 'b_wait' is set before calling this, so that the queues aren't set
 * up unnecessarily.
 */
void __wait_on_buffer(struct buffer_head * bh)
{
	add_wait_queue(&bh->b_wait,&current->wait);
repeat:
	current->state = TASK_UNINTERRUPTIBLE;
	if (bh->b_lock) {
		schedule();
		goto repeat;
	}
	remove_wait_queue(&bh->b_wait,&current->wait);
	current->state = TASK_RUNNING;
}

static void sync_buffers(dev_t dev)
{
	int i;
	struct buffer_head * bh;

	bh = free_list;
	for (i = nr_buffers*2 ; i-- > 0 ; bh = bh->b_next_free) {
		if (bh->b_lock)
			continue;
		if (!bh->b_dirt)
			continue;
		ll_rw_block(WRITE,bh);
	}
}

void sync_dev(dev_t dev)
{
	sync_buffers(dev);
	sync_supers(dev);
	sync_inodes(dev);
	sync_buffers(dev);
}

int sys_sync(void)
{
	sync_dev(0);
	return 0;
}

void invalidate_buffers(dev_t dev)
{
	int i;
	struct buffer_head * bh;

	bh = free_list;
	for (i = nr_buffers*2 ; --i > 0 ; bh = bh->b_next_free) {
		if (bh->b_dev != dev)
			continue;
		wait_on_buffer(bh);
		if (bh->b_dev == dev)
			bh->b_uptodate = bh->b_dirt = 0;
	}
}

/*
 * This routine checks whether a floppy has been changed, and
 * invalidates all buffer-cache-entries in that case. This
 * is a relatively slow routine, so we have to try to minimize using
 * it. Thus it is called only upon a 'mount' or 'open'. This
 * is the best way of combining speed and utility, I think.
 * People changing diskettes in the middle of an operation deserve
 * to loose :-)
 *
 * NOTE! Although currently this is only for floppies, the idea is
 * that any additional removable block-device will use this routine,
 * and that mount/open needn't know that floppies/whatever are
 * special.
 */
void check_disk_change(dev_t dev)
{
	int i;
	struct buffer_head * bh;

	switch(MAJOR(dev)){
	case 2: /* floppy disc */
		if (!(bh = getblk(dev,0,1024)))
			return;
		i = floppy_change(bh);
		brelse(bh);
		break;

#if defined(CONFIG_BLK_DEV_SD) && defined(CONFIG_SCSI)
         case 8: /* Removable scsi disk */
		i = check_scsidisk_media_change(dev, 0);
		if (i) printk("Flushing buffers and inodes for SCSI disk\n");
		break;
#endif

#if defined(CONFIG_BLK_DEV_SR) && defined(CONFIG_SCSI)
         case 11: /* CDROM */
		i = check_cdrom_media_change(dev, 0);
		if (i) printk("Flushing buffers and inodes for CDROM\n");
		break;
#endif

         default:
		return;
	};

	if (!i)	return;

	for (i=0 ; i<NR_SUPER ; i++)
		if (super_block[i].s_dev == dev)
			put_super(super_block[i].s_dev);
	invalidate_inodes(dev);
	invalidate_buffers(dev);

#if defined(CONFIG_BLK_DEV_SD) && defined(CONFIG_SCSI)
/* This is trickier for a removable hardisk, because we have to invalidate
   all of the partitions that lie on the disk. */
	if (MAJOR(dev) == 8)
		revalidate_scsidisk(dev, 0);
#endif
}

#define _hashfn(dev,block) (((unsigned)(dev^block))%NR_HASH)
#define hash(dev,block) hash_table[_hashfn(dev,block)]

static inline void remove_from_hash_queue(struct buffer_head * bh)
{
	if (bh->b_next)
		bh->b_next->b_prev = bh->b_prev;
	if (bh->b_prev)
		bh->b_prev->b_next = bh->b_next;
	if (hash(bh->b_dev,bh->b_blocknr) == bh)
		hash(bh->b_dev,bh->b_blocknr) = bh->b_next;
	bh->b_next = bh->b_prev = NULL;
}

static inline void remove_from_free_list(struct buffer_head * bh)
{
	if (!(bh->b_prev_free) || !(bh->b_next_free))
		panic("Free block list corrupted");
	bh->b_prev_free->b_next_free = bh->b_next_free;
	bh->b_next_free->b_prev_free = bh->b_prev_free;
	if (free_list == bh)
		free_list = bh->b_next_free;
	bh->b_next_free = bh->b_prev_free = NULL;
}

static inline void remove_from_queues(struct buffer_head * bh)
{
	remove_from_hash_queue(bh);
	remove_from_free_list(bh);
}

static inline void put_first_free(struct buffer_head * bh)
{
	if (!bh || (bh == free_list))
		return;
	remove_from_free_list(bh);
/* add to front of free list */
	bh->b_next_free = free_list;
	bh->b_prev_free = free_list->b_prev_free;
	free_list->b_prev_free->b_next_free = bh;
	free_list->b_prev_free = bh;
	free_list = bh;
}

static inline void put_last_free(struct buffer_head * bh)
{
	if (!bh)
		return;
	if (bh == free_list) {
		free_list = bh->b_next_free;
		return;
	}
	remove_from_free_list(bh);
/* add to back of free list */
	bh->b_next_free = free_list;
	bh->b_prev_free = free_list->b_prev_free;
	free_list->b_prev_free->b_next_free = bh;
	free_list->b_prev_free = bh;
}

static inline void insert_into_queues(struct buffer_head * bh)
{
/* put at end of free list */
	bh->b_next_free = free_list;
	bh->b_prev_free = free_list->b_prev_free;
	free_list->b_prev_free->b_next_free = bh;
	free_list->b_prev_free = bh;
/* put the buffer in new hash-queue if it has a device */
	bh->b_prev = NULL;
	bh->b_next = NULL;
	if (!bh->b_dev)
		return;
	bh->b_next = hash(bh->b_dev,bh->b_blocknr);
	hash(bh->b_dev,bh->b_blocknr) = bh;
	if (bh->b_next)
		bh->b_next->b_prev = bh;
}

static struct buffer_head * find_buffer(dev_t dev, int block, int size)
{		
	struct buffer_head * tmp;

	for (tmp = hash(dev,block) ; tmp != NULL ; tmp = tmp->b_next)
		if (tmp->b_dev==dev && tmp->b_blocknr==block)
			if (tmp->b_size == size)
				return tmp;
			else {
				printk("wrong block-size on device %04x\n",dev);
				return NULL;
			}
	return NULL;
}

/*
 * Why like this, I hear you say... The reason is race-conditions.
 * As we don't lock buffers (unless we are readint them, that is),
 * something might happen to it while we sleep (ie a read-error
 * will force it bad). This shouldn't really happen currently, but
 * the code is ready.
 */
struct buffer_head * get_hash_table(dev_t dev, int block, int size)
{
	struct buffer_head * bh;

	for (;;) {
		if (!(bh=find_buffer(dev,block,size)))
			return NULL;
		bh->b_count++;
		wait_on_buffer(bh);
		if (bh->b_dev == dev && bh->b_blocknr == block && bh->b_size == size) {
			put_last_free(bh);
			return bh;
		}
		bh->b_count--;
	}
}

/*
 * Ok, this is getblk, and it isn't very clear, again to hinder
 * race-conditions. Most of the code is seldom used, (ie repeating),
 * so it should be much more efficient than it looks.
 *
 * The algoritm is changed: hopefully better, and an elusive bug removed.
 *
 * 14.02.92: changed it to sync dirty buffers a bit: better performance
 * when the filesystem starts to get full of dirty blocks (I hope).
 */
#define BADNESS(bh) (((bh)->b_dirt<<1)+(bh)->b_lock)
struct buffer_head * getblk(dev_t dev, int block, int size)
{
	struct buffer_head * bh, * tmp;
	int buffers;

repeat:
	if (bh = get_hash_table(dev, block, size))
		return bh;

	if (nr_free_pages > 30)
		grow_buffers(size);

	buffers = nr_buffers;
	bh = NULL;

	for (tmp = free_list; buffers-- > 0 ; tmp = tmp->b_next_free) {
		if (tmp->b_count || tmp->b_size != size)
			continue;
		if (!bh || BADNESS(tmp)<BADNESS(bh)) {
			bh = tmp;
			if (!BADNESS(tmp))
				break;
		}
#if 0
		if (tmp->b_dirt)
			ll_rw_block(WRITEA,tmp);
#endif
	}

	if (!bh && nr_free_pages > 5) {
		grow_buffers(size);
		goto repeat;
	}
	
/* and repeat until we find something good */
	if (!bh) {
		sleep_on(&buffer_wait);
		goto repeat;
	}
	wait_on_buffer(bh);
	if (bh->b_count || bh->b_size != size)
		goto repeat;
	if (bh->b_dirt) {
		sync_buffers(bh->b_dev);
		goto repeat;
	}
/* NOTE!! While we slept waiting for this block, somebody else might */
/* already have added "this" block to the cache. check it */
	if (find_buffer(dev,block,size))
		goto repeat;
/* OK, FINALLY we know that this buffer is the only one of it's kind, */
/* and that it's unused (b_count=0), unlocked (b_lock=0), and clean */
	bh->b_count=1;
	bh->b_dirt=0;
	bh->b_uptodate=0;
	remove_from_queues(bh);
	bh->b_dev=dev;
	bh->b_blocknr=block;
	insert_into_queues(bh);
	return bh;
}

void brelse(struct buffer_head * buf)
{
	if (!buf)
		return;
	wait_on_buffer(buf);
	if (!(buf->b_count--))
		panic("Trying to free free buffer");
	wake_up(&buffer_wait);
}

/*
 * bread() reads a specified block and returns the buffer that contains
 * it. It returns NULL if the block was unreadable.
 */
struct buffer_head * bread(dev_t dev, int block, int size)
{
	struct buffer_head * bh;

	if (!(bh = getblk(dev, block, size))) {
		printk("bread: getblk returned NULL\n");
		return NULL;
	}
	if (bh->b_uptodate)
		return bh;
	ll_rw_block(READ,bh);
	wait_on_buffer(bh);
	if (bh->b_uptodate)
		return bh;
	brelse(bh);
	return NULL;
}

#define COPYBLK(from,to) \
__asm__("cld\n\t" \
	"rep\n\t" \
	"movsl\n\t" \
	::"c" (BLOCK_SIZE/4),"S" (from),"D" (to) \
	:"cx","di","si")

/*
 * bread_page reads four buffers into memory at the desired address. It's
 * a function of its own, as there is some speed to be got by reading them
 * all at the same time, not waiting for one to be read, and then another
 * etc.
 */
void bread_page(unsigned long address, dev_t dev, int b[4])
{
	struct buffer_head * bh[4];
	int i;

	for (i=0 ; i<4 ; i++)
		if (b[i]) {
			if (bh[i] = getblk(dev, b[i], 1024))
				if (!bh[i]->b_uptodate)
					ll_rw_block(READ,bh[i]);
		} else
			bh[i] = NULL;
	for (i=0 ; i<4 ; i++,address += BLOCK_SIZE)
		if (bh[i]) {
			wait_on_buffer(bh[i]);
			if (bh[i]->b_uptodate)
				COPYBLK((unsigned long) bh[i]->b_data,address);
			brelse(bh[i]);
		}
}

/*
 * Ok, breada can be used as bread, but additionally to mark other
 * blocks for reading as well. End the argument list with a negative
 * number.
 */
struct buffer_head * breada(dev_t dev,int first, ...)
{
	va_list args;
	struct buffer_head * bh, *tmp;

	va_start(args,first);
	if (!(bh = getblk(dev, first, 1024))) {
		printk("breada: getblk returned NULL\n");
		return NULL;
	}
	if (!bh->b_uptodate)
		ll_rw_block(READ,bh);
	while ((first=va_arg(args,int))>=0) {
		tmp = getblk(dev, first, 1024);
		if (tmp) {
			if (!tmp->b_uptodate)
				ll_rw_block(READA,tmp);
			tmp->b_count--;
		}
	}
	va_end(args);
	wait_on_buffer(bh);
	if (bh->b_uptodate)
		return bh;
	brelse(bh);
	return (NULL);
}

static void put_unused_buffer_head(struct buffer_head * bh)
{
	memset((void *) bh,0,sizeof(*bh));
	bh->b_next_free = unused_list;
	unused_list = bh;
}

static void get_more_buffer_heads(void)
{
	unsigned long page;
	struct buffer_head * bh;

	if (unused_list)
		return;
	page = get_free_page(GFP_KERNEL);
	if (!page)
		return;
	bh = (struct buffer_head *) page;
	while ((unsigned long) (bh+1) <= page+4096) {
		put_unused_buffer_head(bh);
		bh++;
		nr_buffer_heads++;
	}
}

static struct buffer_head * get_unused_buffer_head(void)
{
	struct buffer_head * bh;

	get_more_buffer_heads();
	if (!unused_list)
		return NULL;
	bh = unused_list;
	unused_list = bh->b_next_free;
	bh->b_next_free = NULL;
	bh->b_data = NULL;
	bh->b_size = 0;
	return bh;
}

/*
 * Try to increase the number of buffers available: the size argument
 * is used to determine what kind of buffers we want. Currently only
 * 1024-byte buffers are supported by the rest of the system, but I
 * think this will change eventually.
 */
void grow_buffers(int size)
{
	unsigned long page;
	int i;
	struct buffer_head *bh, *tmp;

	if ((size & 511) || (size > 4096)) {
		printk("grow_buffers: size = %d\n",size);
		return;
	}
	page = get_free_page(GFP_BUFFER);
	if (!page)
		return;
	tmp = NULL;
	i = 0;
	for (i = 0 ; i+size <= 4096 ; i += size) {
		bh = get_unused_buffer_head();
		if (!bh)
			goto no_grow;
		bh->b_this_page = tmp;
		tmp = bh;
		bh->b_data = (char * ) (page+i);
		bh->b_size = size;
	}
	tmp = bh;
	while (1) {
		if (free_list) {
			tmp->b_next_free = free_list;
			tmp->b_prev_free = free_list->b_prev_free;
			free_list->b_prev_free->b_next_free = tmp;
			free_list->b_prev_free = tmp;
		} else {
			tmp->b_prev_free = tmp;
			tmp->b_next_free = tmp;
		}
		free_list = tmp;
		++nr_buffers;
		if (tmp->b_this_page)
			tmp = tmp->b_this_page;
		else
			break;
	}
	tmp->b_this_page = bh;
	return;
/*
 * In case anything failed, we just free everything we got.
 */
no_grow:
	bh = tmp;
	while (bh) {
		tmp = bh;
		bh = bh->b_this_page;
		put_unused_buffer_head(tmp);
	}	
	free_page(page);
}

/*
 * try_to_free() checks if all the buffers on this particular page
 * are unused, and free's the page if so.
 */
static int try_to_free(struct buffer_head * bh)
{
	unsigned long page;
	struct buffer_head * tmp, * p;

	tmp = bh;
	do {
		if (!tmp)
			return 0;
		if (tmp->b_count || tmp->b_dirt || tmp->b_lock)
			return 0;
		tmp = tmp->b_this_page;
	} while (tmp != bh);
	page = (unsigned long) bh->b_data;
	page &= 0xfffff000;
	tmp = bh;
	do {
		p = tmp;
		tmp = tmp->b_this_page;
		nr_buffers--;
		remove_from_queues(p);
		put_unused_buffer_head(p);
	} while (tmp != bh);
	free_page(page);
	return 1;
}

/*
 * Try to free up some pages by shrinking the buffer-cache
 *
 * Priority tells the routine how hard to try to shrink the
 * buffers: 3 means "don't bother too much", while a value
 * of 0 means "we'd better get some free pages now".
 */
int shrink_buffers(unsigned int priority)
{
	struct buffer_head *bh;
	int i;

	if (priority < 2)
		sync_buffers(0);
	bh = free_list;
	i = nr_buffers >> priority;
	for ( ; i-- > 0 ; bh = bh->b_next_free) {
		if (bh->b_count || !bh->b_this_page)
			continue;
		if (bh->b_lock)
			if (priority)
				continue;
			else
				wait_on_buffer(bh);
		if (bh->b_dirt) {
			ll_rw_block(WRITEA,bh);
			continue;
		}
		if (try_to_free(bh))
			return 1;
	}
	return 0;
}

/*
 * This initializes the initial buffer free list.  nr_buffers is set
 * to one less the actual number of buffers, as a sop to backwards
 * compatibility --- the old code did this (I think unintentionally,
 * but I'm not sure), and programs in the ps package expect it.
 * 					- TYT 8/30/92
 */
void buffer_init(void)
{
	int i;

	for (i = 0 ; i < NR_HASH ; i++)
		hash_table[i] = NULL;
	free_list = 0;
	grow_buffers(BLOCK_SIZE);
	if (!free_list)
		panic("Unable to initialize buffer free list!");
	return;
}
