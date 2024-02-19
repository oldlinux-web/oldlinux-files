/* nalloc.c: a simple single-arena allocator for command-line-lifetime allocation */

#include "rc.h"
#include "utils.h"
#include "nalloc.h"

static struct Block {
	SIZE_T used;
	SIZE_T size;
	char *mem;
	Block *n;
} *fl, *ul;

/* alignto() works only with power of 2 blocks and assumes 2's complement arithmetic */
#define alignto(m, n)   ((m + n - 1) & ~(n - 1))
#define BLOCKSIZE ((SIZE_T) 4096)

/* gets a block from malloc space and places it at the head of the used-list */

static void getblock(SIZE_T n) {
	Block *r, *p;

	for (r = fl, p = NULL; r != NULL; p = r, r = r->n)
		if (n <= r->size)
			break;

	if (r != NULL) {
		if (p != NULL)
			p->n = r->n;
		else
			fl = r->n;
	} else {
		r = enew(Block);
		r->mem = ealloc(alignto(n, BLOCKSIZE));
		r->size = alignto(n, BLOCKSIZE);
	}

	r->used = 0;
	r->n = ul;
	ul = r;
}

/*
   A fast single-arena allocator. Looks at the current block, and if there is not enough room,
   it goes to getblock() for more. "ul" stands for "used list", and the head of the list is the
   current block.
*/

void *nalloc(SIZE_T n) {
	char *ret;

        n = alignto(n, sizeof (ALIGN_T));

	if (ul == NULL || n + ul->used >= ul->size)
		getblock(n);

	ret = ul->mem + ul->used;
	ul->used += n;
	return ret;
}

/*
   Frees memory from nalloc space by putting it on the freelist. Returns free blocks to the
   system, retaining at least MAXMEM bytes worth of blocks for nalloc.
*/

#define MAXMEM 500000

void nfree() {
	Block *r;
	SIZE_T count;

	if (ul == NULL)
		return;

	for (r = ul; r->n != NULL; r = r->n)
		;

	r->n = fl;
	fl = ul;
	ul = NULL;

	for (r = fl, count = r->size; r->n != NULL; r = r->n, count += r->size) {
		if (count >= MAXMEM) {
			Block *tmp = r;

			r = r->n;
			tmp->n = NULL;		/* terminate the freelist */

			while (r != NULL) {	/* free memory off the tail of the freelist */
				tmp = r->n;
				efree(r->mem);
				efree(r);
				r = tmp;
			}
		return;
		}
	}
}

/*
   "allocates" a new arena by zeroing out the old one. Up to the calling routine to keep
   the old value of the block around.
*/

Block *newblock() {
	Block *ret = ul;

	ul = NULL;
	return ret;
}

/* "restores" an arena to its saved value. */

void restoreblock(Block *b) {
	nfree();
	ul = b;
}
