/*
 * malloc.c --- a general purpose kernel memory allocator for Linux.
 *
 * Written by Theodore Ts'o (tytso@mit.edu), 11/29/91
 *
 * This routine is written to be as fast as possible, so that it
 * can be called from the interrupt level.
 *
 * Limitations: maximum size of memory we can allocate using this routine
 *	is 4k, the size of a page in Linux.
 *
 * The general game plan is that each page (called a bucket) will only hold
 * objects of a given size.  When all of the object on a page are released,
 * the page can be returned to the general free pool.  When malloc() is
 * called, it looks for the smallest bucket size which will fulfill its
 * request, and allocate a piece of memory from that bucket pool.
 *
 * Each bucket has as its control block a bucket descriptor which keeps
 * track of how many objects are in use on that page, and the free list
 * for that page.  Like the buckets themselves, bucket descriptors are
 * stored on pages requested from get_free_page().  However, unlike buckets,
 * pages devoted to bucket descriptor pages are never released back to the
 * system.  Fortunately, a system should probably only need 1 or 2 bucket
 * descriptor pages, since a page can hold 256 bucket descriptors (which
 * corresponds to 1 megabyte worth of bucket pages.)  If the kernel is using
 * that much allocated memory, it's probably doing something wrong.  :-)
 *
 * Note: malloc() and free() both call get_free_page() and free_page()
 *	in sections of code where interrupts are turned off, to allow
 *	malloc() and free() to be safely called from an interrupt routine.
 *	(We will probably need this functionality when networking code,
 *	particularily things like NFS, is added to Linux.)  However, this
 *	presumes that get_free_page() and free_page() are interrupt-level
 *	safe, which they may not be once paging is added.  If this is the
 *	case, we will need to modify malloc() to keep a few unused pages
 *	"pre-allocated" so that it can safely draw upon those pages if
 * 	it is called from an interrupt routine.
 *
 * 	Another concern is that get_free_page() should not sleep; if it
 *	does, the code is carefully ordered so as to avoid any race
 *	conditions.  The catch is that if malloc() is called re-entrantly,
 *	there is a chance that unecessary pages will be grabbed from the
 *	system.  Except for the pages for the bucket descriptor page, the
 *	extra pages will eventually get released back to the system, though,
 *	so it isn't all that bad.
 */

/* I'm going to modify it to keep some free pages around.  Get free page
   can sleep, and tcp/ip needs to call malloc at interrupt time  (Or keep
   big buffers around for itself.)  I guess I'll have return from
   syscall fill up the free page descriptors. -RAB */

/* since the advent of GFP_ATOMIC, I've changed the malloc code to
   use it and return NULL if it can't get a page. -RAB */

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>

#include <asm/system.h>

struct bucket_desc {	/* 16 bytes */
	void			*page;
	struct bucket_desc	*next;
	void			*freeptr;
	unsigned short		refcnt;
	unsigned short		bucket_size;
};

struct _bucket_dir {	/* 8 bytes */
	int			size;
	struct bucket_desc	*chain;
};

/*
 * The following is the where we store a pointer to the first bucket
 * descriptor for a given size.
 *
 * If it turns out that the Linux kernel allocates a lot of objects of a
 * specific size, then we may want to add that specific size to this list,
 * since that will allow the memory to be allocated more efficiently.
 * However, since an entire page must be dedicated to each specific size
 * on this list, some amount of temperance must be exercised here.
 *
 * Note that this list *must* be kept in order.
 */
struct _bucket_dir bucket_dir[] = {
	{ 16,	(struct bucket_desc *) 0},
	{ 32,	(struct bucket_desc *) 0},
	{ 64,	(struct bucket_desc *) 0},
	{ 128,	(struct bucket_desc *) 0},
	{ 256,	(struct bucket_desc *) 0},
	{ 512,	(struct bucket_desc *) 0},
	{ 1024,	(struct bucket_desc *) 0},
	{ 2048, (struct bucket_desc *) 0},
	{ 4096, (struct bucket_desc *) 0},
	{ 0,    (struct bucket_desc *) 0}};   /* End of list marker */

/*
 * This contains a linked list of free bucket descriptor blocks
 */
static struct bucket_desc *free_bucket_desc = (struct bucket_desc *) 0;

/*
 * This routine initializes a bucket description page.
 */
static inline int init_bucket_desc()
{
	struct bucket_desc *bdesc, *first;
	int i;
	/* this turns interrupt on, so we should be carefull. */
	first = bdesc = (struct bucket_desc *) get_free_page(GFP_ATOMIC);
	if (!bdesc)
		return 1;
	for (i = PAGE_SIZE/sizeof(struct bucket_desc); i > 1; i--) {
		bdesc->next = bdesc+1;
		bdesc++;
	}
	/*
	 * This is done last, to avoid race conditions in case
	 * get_free_page() sleeps and this routine gets called again....
	 */
	/* Get free page will not sleep because of the GFP_ATOMIC */
	bdesc->next = free_bucket_desc;
	free_bucket_desc = first;
	return (0);
}

void *malloc(unsigned int len)
{
	struct _bucket_dir	*bdir;
	struct bucket_desc	*bdesc;
	void			*retval;

	/*
	 * First we search the bucket_dir to find the right bucket change
	 * for this request.
	 */
	for (bdir = bucket_dir; bdir->size; bdir++)
		if (bdir->size >= len)
			break;

	if (!bdir->size) {
		printk("malloc called with impossibly large argument (%d)\n", len);
		return NULL;
	}
	len = bdir->size;
	/*
	 * Now we search for a bucket descriptor which has free space
	 */
	cli();	/* Avoid race conditions */
	for (bdesc = bdir->chain; bdesc; bdesc = bdesc->next)
		if (bdesc->freeptr)
			break;
	/*
	 * If we didn't find a bucket with free space, then we'll
	 * allocate a new one.
	 */
	if (!bdesc) {
		char *cp;
		int i;

		if (!free_bucket_desc)
			if (init_bucket_desc()) {
				sti();
				return NULL;
			}
		bdesc = free_bucket_desc;
		free_bucket_desc = bdesc->next;
		bdesc->refcnt = 0;
		bdesc->bucket_size = len;
		bdesc->page = bdesc->freeptr =
		  (void *) cp = get_free_page(GFP_ATOMIC);
		if (!cp) {
			sti();
			return NULL;
		}
		/* Set up the chain of free objects */
		for (i=PAGE_SIZE/len; i > 1; i--) {
			*((char **) cp) = cp + len;
			cp += len;
		}
		*((char **) cp) = 0;
		bdesc->next = bdir->chain; /* OK, link it in! */
		bdir->chain = bdesc;
	}
	retval = (void *) bdesc->freeptr;
	bdesc->freeptr = *((void **) retval);
	bdesc->refcnt++;
	sti();	/* OK, we're safe again */
	memset(retval, 0, len);
	return retval;
}

/*
 * Here is the free routine.  If you know the size of the object that you
 * are freeing, then free_s() will use that information to speed up the
 * search for the bucket descriptor.
 *
 * We will #define a macro so that "free(x)" is becomes "free_s(x, 0)"
 */
void free_s(void *obj, int size)
{
	void		*page;
	struct _bucket_dir	*bdir;
	struct bucket_desc	*bdesc, *prev;

	/* Calculate what page this object lives in */
	page = (void *)  ((unsigned long) obj & 0xfffff000);
	/* Now search the buckets looking for that page */
	for (bdir = bucket_dir; bdir->size; bdir++) {
		prev = 0;
		/* If size is zero then this conditional is always false */
		if (bdir->size < size)
			continue;
		for (bdesc = bdir->chain; bdesc; bdesc = bdesc->next) {
			if (bdesc->page == page)
				goto found;
			prev = bdesc;
		}
	}
	printk("Bad address passed to kernel free_s()");
	return;
found:
	cli(); /* To avoid race conditions */
	*((void **)obj) = bdesc->freeptr;
	bdesc->freeptr = obj;
	bdesc->refcnt--;
	if (bdesc->refcnt == 0) {
		/*
		 * We need to make sure that prev is still accurate.  It
		 * may not be, if someone rudely interrupted us....
		 */
		if ((prev && (prev->next != bdesc)) ||
		    (!prev && (bdir->chain != bdesc)))
			for (prev = bdir->chain; prev; prev = prev->next)
				if (prev->next == bdesc)
					break;
		if (prev)
			prev->next = bdesc->next;
		else {
			if (bdir->chain != bdesc)
				panic("malloc bucket chains corrupted");
			bdir->chain = bdesc->next;
		}
		free_page((unsigned long) bdesc->page);
		bdesc->next = free_bucket_desc;
		free_bucket_desc = bdesc;
	}
	sti();
	return;
}
