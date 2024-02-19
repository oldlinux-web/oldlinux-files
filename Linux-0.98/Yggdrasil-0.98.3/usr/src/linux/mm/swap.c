/*
 *  linux/mm/swap.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * This file should contain most things doing the swapping from/to disk.
 * Started 18.12.91
 */

#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <asm/system.h> /* for cli()/sti() */

#define MAX_SWAPFILES 8

#define SWP_USED	1
#define SWP_WRITEOK	3

static int nr_swapfiles = 0;
static struct wait_queue * lock_queue = NULL;

static struct swap_info_struct {
	unsigned long flags;
	struct inode * swap_file;
	unsigned int swap_device;
	unsigned char * swap_map;
	char * swap_lockmap;
	int lowest_bit;
	int highest_bit;
} swap_info[MAX_SWAPFILES];

extern unsigned long free_page_list;

/*
 * The following are used to make sure we don't thrash too much...
 * NOTE!! NR_LAST_FREE_PAGES must be a power of 2...
 */
#define NR_LAST_FREE_PAGES 32
static unsigned long last_free_pages[NR_LAST_FREE_PAGES] = {0,};

#define SWAP_BITS 4096

#define bitop(name,op) \
static inline int name(char * addr,unsigned int nr) \
{ \
int __res; \
__asm__ __volatile__("bt" op " %1,%2; adcl $0,%0" \
:"=g" (__res) \
:"r" (nr),"m" (*(addr)),"0" (0)); \
return __res; \
}

bitop(bit,"")
bitop(setbit,"s")
bitop(clrbit,"r")

void rw_swap_page(int rw, unsigned int nr, char * buf)
{
	struct swap_info_struct * p;

	if ((nr >> 24) >= nr_swapfiles) {
		printk("Internal error: bad swap-device\n");
		return;
	}
	p = swap_info + (nr >> 24);
	nr &= 0x00ffffff;
	if (nr >= SWAP_BITS) {
		printk("rw_swap_page: weirdness\n");
		return;
	}
	if (!(p->flags & SWP_USED)) {
		printk("Trying to swap to unused swap-device\n");
		return;
	}
	while (setbit(p->swap_lockmap,nr))
		sleep_on(&lock_queue);
	if (p->swap_device) {
		ll_rw_page(rw,p->swap_device,nr,buf);
	} else if (p->swap_file) {
		unsigned int zones[4];
		unsigned int block = nr << 2;
		int i;

		for (i = 0; i < 4; i++)
			if (!(zones[i] = bmap(p->swap_file,block++))) {
				printk("rw_swap_page: bad swap file\n");
				return;
			}
		ll_rw_swap_file(rw,p->swap_file->i_dev, zones,4,buf);
	} else
		printk("re_swap_page: no swap file or device\n");
	if (!clrbit(p->swap_lockmap,nr))
		printk("rw_swap_page: lock already cleared\n");
	wake_up(&lock_queue);
}

static unsigned int get_swap_page(void)
{
	struct swap_info_struct * p;
	unsigned int block_nr, swap_nr;

	p = swap_info;
	for (swap_nr = 0 ; swap_nr < nr_swapfiles ; swap_nr++,p++) {
		if ((p->flags & SWP_WRITEOK) != SWP_WRITEOK)
			continue;
		for (block_nr = p->lowest_bit; block_nr <= p->highest_bit ; block_nr++) {
			if (p->swap_map[block_nr])
				continue;
			p->swap_map[block_nr] = 1;
			if (block_nr == p->highest_bit)
				p->highest_bit--;
			p->lowest_bit = block_nr;
			return block_nr + (swap_nr << 24);
		}
	}
	return 0;
}

void swap_duplicate(unsigned int nr)
{
	struct swap_info_struct * p;

	if (!nr)
		return;
	if ((nr >> 24) >= nr_swapfiles) {
		printk("Trying to free nonexistent swap-page\n");
		return;
	}
	p = (nr >> 24) + swap_info;
	nr &= 0x00ffffff;
	if (nr >= SWAP_BITS) {
		printk("swap_free: weirness\n");
		return;
	}
	if (!p->swap_map[nr]) {
		printk("swap_duplicate: trying to duplicate unused page\n");
		return;
	}
	p->swap_map[nr]++;
}

void swap_free(unsigned int nr)
{
	struct swap_info_struct * p;

	if (!nr)
		return;
	if ((nr >> 24) >= nr_swapfiles) {
		printk("Trying to free nonexistent swap-page\n");
		return;
	}
	p = (nr >> 24) + swap_info;
	nr &= 0x00ffffff;
	if (nr >= SWAP_BITS) {
		printk("swap_free: weirness\n");
		return;
	}
	if (!(p->flags & SWP_USED)) {
		printk("Trying to free swap from unused swap-device\n");
		return;
	}
	while (setbit(p->swap_lockmap,nr))
		sleep_on(&lock_queue);
	if (nr < p->lowest_bit)
		p->lowest_bit = nr;
	if (nr > p->highest_bit)
		p->highest_bit = nr;
	if (!p->swap_map[nr])
		printk("swap_free: swap-space map bad (page %d)\n",nr);
	else
		p->swap_map[nr]--;
	if (!clrbit(p->swap_lockmap,nr))
		printk("swap_free: lock already cleared\n");
	wake_up(&lock_queue);
}

void swap_in(unsigned long *table_ptr)
{
	unsigned long swap_nr;
	unsigned long page;

	swap_nr = *table_ptr;
	if (1 & swap_nr) {
		printk("trying to swap in present page\n\r");
		return;
	}
	if (!swap_nr) {
		printk("No swap page in swap_in\n\r");
		return;
	}
	page = get_free_page(GFP_KERNEL);
	if (!page) {
		oom(current);
		page = BAD_PAGE;
	} else	
		read_swap_page(swap_nr>>1, (char *) page);
	if (*table_ptr != swap_nr) {
		free_page(page);
		return;
	}
	*table_ptr = page | (PAGE_DIRTY | 7);
	swap_free(swap_nr>>1);
}

static int try_to_swap_out(unsigned long * table_ptr)
{
	int i;
	unsigned long page;
	unsigned long swap_nr;

	page = *table_ptr;
	if (!(PAGE_PRESENT & page))
		return 0;
	if (page >= high_memory) {
		printk("try_to_swap_out: bad page (%08x)\n",page);
		*table_ptr = 0;
		return 0;
	}
	if (mem_map[MAP_NR(page)] & MAP_PAGE_RESERVED)
		return 0;
	if (PAGE_ACCESSED & page) {
		*table_ptr &= ~PAGE_ACCESSED;
		return 0;
	}
	for (i = 0; i < NR_LAST_FREE_PAGES; i++)
		if (last_free_pages[i] == (page & 0xfffff000))
			return 0;
	if (PAGE_DIRTY & page) {
		page &= 0xfffff000;
		if (mem_map[MAP_NR(page)] != 1)
			return 0;
		if (!(swap_nr = get_swap_page()))
			return 0;
		*table_ptr = swap_nr<<1;
		invalidate();
		write_swap_page(swap_nr, (char *) page);
		free_page(page);
		return 1;
	}
	page &= 0xfffff000;
	*table_ptr = 0;
	invalidate();
	free_page(page);
	return 1 + mem_map[MAP_NR(page)];
}

/*
 * sys_idle() does nothing much: it just searches for likely candidates for
 * swapping out or forgetting about. This speeds up the search when we
 * actually have to swap.
 */
int sys_idle(void)
{
	need_resched = 1;
	return 0;
}

/*
 * Go through the page tables, searching for a user page that
 * we can swap out.
 * 
 * We now check that the process is swappable (normally only 'init'
 * is un-swappable), allowing high-priority processes which cannot be
 * swapped out (things like user-level device drivers (Not implemented)).
 */
static int swap_out(unsigned int priority)
{
	static int swap_task = 1;
	static int swap_table = 0;
	static int swap_page = 0;
	int counter = NR_TASKS*8;
	int pg_table;
	struct task_struct * p;

	counter >>= priority;
check_task:
	if (counter-- < 0)
		return 0;
	if (swap_task >= NR_TASKS) {
		swap_task = 1;
		goto check_task;
	}
	p = task[swap_task];
	if (!p || !p->swappable) {
		swap_task++;
		goto check_task;
	}
check_dir:
	if (swap_table >= 1024) {
		swap_table = 0;
		swap_task++;
		goto check_task;
	}
	pg_table = ((unsigned long *) p->tss.cr3)[swap_table];
	if (pg_table >= high_memory || (mem_map[MAP_NR(pg_table)] & MAP_PAGE_RESERVED)) {
		swap_table++;
		goto check_dir;
	}
	if (!(1 & pg_table)) {
		printk("bad page-table at pg_dir[%d]: %08x\n\r",
			swap_table,pg_table);
		((unsigned long *) p->tss.cr3)[swap_table] = 0;
		swap_table++;
		goto check_dir;
	}
	pg_table &= 0xfffff000;
check_table:
	if (swap_page >= 1024) {
		swap_page = 0;
		swap_table++;
		goto check_dir;
	}
	switch (try_to_swap_out(swap_page + (unsigned long *) pg_table)) {
		case 0: break;
		case 1: p->rss--; return 1;
		default: p->rss--;
	}
	swap_page++;
	goto check_table;
}

static int try_to_free_page(void)
{
	int i=6;

	while (i--) {
		if (shrink_buffers(i))
			return 1;
		if (swap_out(i))
			return 1;
	}
	return 0;
}

/*
 * Note that this must be atomic, or bad things will happen when
 * pages are requested in interrupts (as malloc can do). Thus the
 * cli/sti's.
 */
static inline void add_mem_queue(unsigned long addr, unsigned long * queue)
{
	addr &= 0xfffff000;
	*(unsigned long *) addr = *queue;
	*queue = addr;
}

/*
 * Free_page() adds the page to the free lists. This is optimized for
 * fast normal cases (no error jumps taken normally).
 *
 * The way to optimize jumps for gcc-2.2.2 is to:
 *  - select the "normal" case and put it inside the if () { XXX }
 *  - no else-statements if you can avoid them
 *
 * With the above two rules, you get a straight-line execution path
 * for the normal case, giving better asm-code.
 */
void free_page(unsigned long addr)
{
	if (addr < high_memory) {
		unsigned short * map = mem_map + MAP_NR(addr);

		if (*map) {
			if (!(*map & MAP_PAGE_RESERVED)) {
				unsigned long flag;

				save_flags(flag);
				cli();
				if (!--*map) {
					if (nr_secondary_pages < MAX_SECONDARY_PAGES) {
						add_mem_queue(addr,&secondary_page_list);
						nr_secondary_pages++;
						restore_flags(flag);
						return;
					}
					add_mem_queue(addr,&free_page_list);
					nr_free_pages++;
				}
				restore_flags(flag);
			}
			return;
		}
		printk("Trying to free free memory (%08x): memory probabably corrupted\n",addr);
		return;
	}
	printk("Trying to free nonexistent page %08x\n",addr);
	return;
}

/*
 * This is one ugly macro, but it simplifies checking, and makes
 * this speed-critical place reasonably fast, especially as we have
 * to do things with the interrupt flag etc.
 *
 * Note that this #define is heavily optimized to give fast code
 * for the normal case - the if-statements are ordered so that gcc-2.2.2
 * will make *no* jumps for the normal code. Don't touch unless you
 * know what you are doing.
 */
#define REMOVE_FROM_MEM_QUEUE(queue,nr) \
	cli(); \
	if (result = queue) { \
		if (!(result & 0xfff) && result < high_memory) { \
			queue = *(unsigned long *) result; \
			if (!mem_map[MAP_NR(result)]) { \
				mem_map[MAP_NR(result)] = 1; \
				nr--; \
last_free_pages[index = (index + 1) & (NR_LAST_FREE_PAGES - 1)] = result; \
				restore_flags(flag); \
				__asm__ __volatile__("cld ; rep ; stosl" \
					::"a" (0),"c" (1024),"D" (result) \
					:"di","cx"); \
				return result; \
			} \
			printk("Free page %08x has mem_map = %d\n", \
				result,mem_map[MAP_NR(result)]); \
		} else \
			printk("Result = 0x%08x - memory map destroyed\n", result); \
		queue = 0; \
		nr = 0; \
	} else if (nr) { \
		printk(#nr " is %d, but " #queue " is empty\n",nr); \
		nr = 0; \
	} \
	restore_flags(flag)

/*
 * Get physical address of first (actually last :-) free page, and mark it
 * used. If no free pages left, return 0.
 *
 * Note that this is one of the most heavily called functions in the kernel,
 * so it's a bit timing-critical (especially as we have to disable interrupts
 * in it). See the above macro which does most of the work, and which is
 * optimized for a fast normal path of execution.
 */
unsigned long get_free_page(int priority)
{
	unsigned long result, flag;
	static unsigned long index = 0;

	/* this routine can be called at interrupt time via
	   malloc.  We want to make sure that the critical
	   sections of code have interrupts disabled. -RAB
	   Is this code reentrant? */

	save_flags(flag);
repeat:
	REMOVE_FROM_MEM_QUEUE(free_page_list,nr_free_pages);
	if (priority == GFP_BUFFER)
		return 0;
	if (priority != GFP_ATOMIC)
		if (try_to_free_page())
			goto repeat;
	REMOVE_FROM_MEM_QUEUE(secondary_page_list,nr_secondary_pages);
	return 0;
}

/*
 * Trying to stop swapping from a file is fraught with races, so
 * we repeat quite a bit here when we have to pause. swapoff()
 * isn't exactly timing-critical, so who cares?
 *
 * Note the '>> 25' instead of '>> 24' when checking against
 * swap_nr: remember that the low bit in a page-address is used
 * for the PAGE_PRESENT bit, and is not part of the swap address.
 */
static int try_to_unuse(unsigned int swap_nr)
{
	int nr, pgt, pg;
	unsigned long page, *ppage;
	unsigned long tmp = 0;
	struct task_struct *p;

	nr = 0;
/*
 * When we have to sleep, we restart the whole algorithm from the same
 * task we stopped in. That at least rids us of all races.
 */
repeat:
	for (; nr < NR_TASKS ; nr++) {
		p = task[nr];
		if (!p)
			continue;
		for (pgt = 0 ; pgt < 1024 ; pgt++) {
			ppage = pgt + ((unsigned long *) p->tss.cr3);
			page = *ppage;
			if (!page)
				continue;
			if (!(page & PAGE_PRESENT) || (page >= high_memory)) {
				printk("try_to_unuse: bad page directory (%d,%d:%08x)\n",nr,pgt,page);
				*ppage = 0;
				continue;
			}
			if (mem_map[MAP_NR(page)] & MAP_PAGE_RESERVED)
				continue;
			ppage = (unsigned long *) (page & 0xfffff000);	
			for (pg = 0 ; pg < 1024 ; pg++,ppage++) {
				page = *ppage;
				if (!page)
					continue;
				if (page & PAGE_PRESENT) {
					if (page >= high_memory) {
						printk("try_to_unuse: bad page table (%d,%d,%d:%08x)\n",nr,pgt,pg,page);
						*ppage = 0;
					}
					continue;
				}
				if ((page >> 25) != swap_nr)
					continue;
				if (!tmp) {
					tmp = get_free_page(GFP_KERNEL);
					if (!tmp)
						return -ENOMEM;
					goto repeat;
				}
				read_swap_page(page>>1, (char *) tmp);
				if (*ppage == page) {
					*ppage = tmp | (PAGE_DIRTY | 7);
					++p->rss;
					swap_free(page>>1);
					tmp = 0;
				}
				goto repeat;
			}
		}
	}
	free_page(tmp);
	return 0;
}

int sys_swapoff(const char * specialfile)
{
	struct swap_info_struct * p;
	struct inode * inode;
	unsigned int swap_nr;
	int i;

	if (!suser())
		return -EPERM;
	i = namei(specialfile,&inode);
	if (i)
		return i;
	p = swap_info;
	for (swap_nr = 0 ; swap_nr < nr_swapfiles ; swap_nr++,p++) {
		if ((p->flags & SWP_WRITEOK) != SWP_WRITEOK)
			continue;
		if (p->swap_file) {
			if (p->swap_file == inode)
				break;
		} else {
			if (!S_ISBLK(inode->i_mode))
				continue;
			if (p->swap_device == inode->i_rdev)
				break;
		}
	}
	iput(inode);
	if (swap_nr >= nr_swapfiles)
		return -EINVAL;
	p->flags = SWP_USED;
	i = try_to_unuse(swap_nr);
	if (i) {
		p->flags = SWP_WRITEOK;
		return i;
	}
	iput(p->swap_file);
	p->swap_file = NULL;
	p->swap_device = 0;
	free_page((long) p->swap_map);
	p->swap_map = NULL;
	free_page((long) p->swap_lockmap);
	p->swap_lockmap = NULL;
	p->flags = 0;
	return 0;
}

/*
 * Written 01/25/92 by Simmule Turner, heavily changed by Linus.
 *
 * The swapon system call
 */
int sys_swapon(const char * specialfile)
{
	struct swap_info_struct * p;
	struct inode * swap_inode;
	unsigned int swap_nr;
	char * tmp;
	int i,j;

	if (!suser())
		return -EPERM;
	p = swap_info;
	for (swap_nr = 0 ; swap_nr < nr_swapfiles ; swap_nr++,p++)
		if (!(p->flags & SWP_USED))
			break;
	if (swap_nr >= MAX_SWAPFILES)
		return -EPERM;
	if (swap_nr >= nr_swapfiles)
		nr_swapfiles = swap_nr+1;
	p->flags = SWP_USED;
	p->swap_file = NULL;
	p->swap_device = 0;
	p->swap_map = NULL;
	p->swap_lockmap = NULL;
	p->lowest_bit = 0;
	p->highest_bit = 0;
	i = namei(specialfile,&swap_inode);
	if (i) {
		p->flags = 0;
		return i;
	}
	if (swap_inode->i_count != 1) {
		iput(swap_inode);
		p->flags = 0;
		return -EBUSY;
	}
	if (S_ISBLK(swap_inode->i_mode)) {
		p->swap_device = swap_inode->i_rdev;
		iput(swap_inode);
		if (!p->swap_device) {
			p->flags = 0;
			return -ENODEV;
		}
		for (i = 0 ; i < nr_swapfiles ; i++) {
			if (i == swap_nr)
				continue;
			if (p->swap_device == swap_info[i].swap_device) {
				p->swap_device = 0;
				p->flags = 0;
				return -EBUSY;
			}
		}
	} else if (S_ISREG(swap_inode->i_mode))
		p->swap_file = swap_inode;
	else {
		iput(swap_inode);
		p->flags = 0;
		return -EINVAL;
	}
	tmp = (char *) get_free_page(GFP_USER);
	p->swap_lockmap = (char *) get_free_page(GFP_USER);
	if (!tmp || !p->swap_lockmap) {
		printk("Unable to start swapping: out of memory :-)\n");
		free_page((long) tmp);
		free_page((long) p->swap_lockmap);
		iput(p->swap_file);
		p->swap_device = 0;
		p->swap_file = NULL;
		p->swap_map = NULL;
		p->swap_lockmap = NULL;
		p->flags = 0;
		return -ENOMEM;
	}
	read_swap_page(swap_nr << 24,tmp);
	if (strncmp("SWAP-SPACE",tmp+4086,10)) {
		printk("Unable to find swap-space signature\n\r");
		free_page((long) tmp);
		free_page((long) p->swap_lockmap);
		iput(p->swap_file);
		p->swap_device = 0;
		p->swap_file = NULL;
		p->swap_map = NULL;
		p->swap_lockmap = NULL;
		p->flags = 0;
		return -EINVAL;
	}
	memset(tmp+4086,0,10);
	j = 0;
	p->lowest_bit = 0;
	p->highest_bit = 0;
	for (i = 1 ; i < SWAP_BITS ; i++)
		if (bit(tmp,i)) {
			if (!p->lowest_bit)
				p->lowest_bit = i;
			p->highest_bit = i;
			j++;
		}
	if (!j) {
		printk("Empty swap-file\n");
		free_page((long) tmp);
		free_page((long) p->swap_lockmap);
		iput(p->swap_file);
		p->swap_device = 0;
		p->swap_file = NULL;
		p->swap_map = NULL;
		p->swap_lockmap = NULL;
		p->flags = 0;
		return -EINVAL;
	}
	i = SWAP_BITS;
	while (i--)
		if (bit(tmp,i))
			tmp[i] = 0;
		else
			tmp[i] = 128;
	tmp[0] = 128;
	p->swap_map = tmp;
	p->flags = SWP_WRITEOK;
	printk("Adding Swap: %dk swap-space\n\r",j<<2);
	return 0;
}
