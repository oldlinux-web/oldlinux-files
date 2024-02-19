#include <stdio.h>
#include "alloc.h"
/* memory allocation routines
 *
 * Adapted from alloc routine in K&R; memory statistics and interrupt
 * protection added for use with net package.
 */

static HEADER base, *allocp = NULLHDR;
static unsigned memfail;

/* Define ALLOCDEBUG for allocation recording. Note: works only with
 * small memory model
 */
#ifdef	ALLOCDEBUG
#define	NALLOC	150
/* Map of allocated memory chunks */
struct alloc {
	char *addr;
	unsigned size;
	long pc;
};
struct alloc alloc[NALLOC];
#endif

/* Allocate block of 'nb' bytes */
char *
malloc(nb)
unsigned nb;
{
	HEADER *morecore();
	register HEADER *p, *q;
	register unsigned nu;
	char i_state;

#ifdef	ALLOCDEBUG
	struct alloc *ap;
	unsigned short *ptr;
#endif

	i_state = disable();
	nu = ((nb + 3) >> 2) + 1;	/* Round up to full block, incl hdr */
	if ((q = allocp) == NULLHDR){
		base.s.ptr = allocp = q = &base;
		base.s.size = 1;
	}
	for (p = q->s.ptr; ; q = p, p = p->s.ptr){
		if (p->s.size >= nu){
			/* This chunk is at least as large as we need */
			if (p->s.size == nu){
				/* Perfect fit; remove from list */
				q->s.ptr = p->s.ptr;
			} else {
				/* Carve out piece from end of entry */
				p->s.size -= nu;
				p += p->s.size;
				p->s.size = nu;
			}
			allocp = q;
			p->s.ptr = p;	/* for auditing */
#ifdef	ALLOCDEBUG
			for(ap = alloc;ap < &alloc[NALLOC];ap++){
				if(ap->addr == NULLCHAR){
					ap->addr = (char *)(p+1);
					ap->size = nb;
					ptr = &nb;
#ifdef	LARGECODE
					ap->pc = (long)ptr[-1] << 16 +
						(long)ptr[-2];
#else
					ap->pc = (long)ptr[-1];
#endif
					break;
				}
			}
#endif
			restore(i_state);
			return (char *)(p + 1);
		}
		if (p == allocp && (p = morecore(nu)) == NULLHDR){
			memfail++;
			restore(i_state);
			return NULLCHAR;
		}
	}
}
/* Get more memory from the system and put it on the heap */
HEADER *
morecore(nu)
unsigned nu;
{
	char *sbrk();
	register char *cp;
	register HEADER *up;

	if ((int)(cp = sbrk(nu << 2)) == -1)
		return NULLHDR;
	up = (HEADER *)cp;
	up->s.size = nu;
	up->s.ptr = up;	/* satisfy audit */
	free((char *)(up + 1));
	return allocp;
}
/* Grab as much memory as possible from the system and put it on the heap */
unsigned
grabcore()
{
	char *sbrk(),*cp;
	register HEADER *up;
	register unsigned int size;

	/* Initialize the heap pointers */
	if (allocp == NULLHDR){
		base.s.ptr = allocp = &base;
		base.s.size = 1;
	}
	/* Find out where the break is */
	cp = sbrk(0);
	/* Now try to push it as high as possible */
	for(size=256;;size += 256){
		if(brk(cp + size) == -1)
			break;
	}

	up = (HEADER *)cp;
	up->s.size = size / sizeof(HEADER);
	up->s.ptr = up;	/* satisfy audit */
	free((char *)(up + 1));
	return size;
}

/* Put memory block back on heap */
free(blk)
char *blk;
{
	register HEADER *p, *q;
	unsigned short *ptr;
	char i_state;
#ifdef	ALLOCDEBUG
	struct alloc *ap;
#endif

	i_state = disable();
#ifdef	ALLOCDEBUG
	for(ap = alloc;ap < &alloc[NALLOC];ap++){
		if(ap->addr == blk){
			ap->addr = NULLCHAR;
			break;
		}
	}
#endif
	p = (HEADER *)blk - 1;
	/* Audit check */
	if(p->s.ptr != p){
		printf("p->s.ptr: 0x%x  p: 0x%x\n",p->s.ptr,p);fflush(stdout);
		ptr = (unsigned short *)&blk;
		printf("WARNING!! freeing garbage (0x%x) pc = 0x%x %x\n",blk,
			ptr[-1],ptr[-2]);
		fflush(stdout);
		restore(i_state);
#ifdef	Z80
		printf("SP = 0x%x\n",getsp());
		for(;;) ;
#else
		return;
#endif
	}
	/* Search the free list looking for the right place to insert */
	for(q = allocp; !(p > q && p < q->s.ptr); q = q->s.ptr){
		/* Highest address on circular list? */
		if(q >= q->s.ptr && (p > q || p < q->s.ptr))
			break;
	}
	if(p + p->s.size == q->s.ptr){
		/* Combine with front of this entry */
		p->s.size += q->s.ptr->s.size;
		p->s.ptr = q->s.ptr->s.ptr;
	} else {
		/* Link to front of this entry */
		p->s.ptr = q->s.ptr;
	}
	if(q + q->s.size == p){
		/* Combine with end of this entry */
		q->s.size += p->s.size;
		q->s.ptr = p->s.ptr;
	} else {
		/* Link to end of this entry */
		q->s.ptr = p;
	}
	allocp = q;
	restore(i_state);
}

#ifdef	notdef
/* Move existing block to new area */
char *
realloc(area,size)
char *area;
unsigned size;
{
	unsigned osize;
	HEADER *hp;
	char *cp;
	char i_state;

	hp = ((HEADER *)area) - 1;
	osize = (hp->s.size -1) << 2;

	/* Make sure nobody else comes in and takes it */
	i_state = disable();
	free(area);
	if((cp = malloc(size)) != NULLCHAR && cp != area)
		memcpy((char *)cp,(char *)area,size>osize? osize : size);
	restore(i_state);
	return cp;
}
#endif
/* Allocate block of cleared memory */
char *
calloc(nelem,size)
unsigned nelem;	/* Number of elements */
unsigned size;	/* Size of each element */
{
	register unsigned i;
	register char *cp;
#ifdef	ALLOCDEBUG
	struct alloc *ap;
	short *ptr;
#endif
	i = nelem * size;
	if((cp = malloc(i)) != NULLCHAR)
		memset(cp,0,i);
#ifdef	ALLOCDEBUG
	if(cp == NULLCHAR)
		return cp;
	/* Adjust caller field to indicate real caller, not calloc() */
	ptr = (short *)&nelem;
	for(ap = alloc; ap < &alloc[NALLOC];ap++)
		if(ap->addr == cp)
			ap->pc = (long)ptr[-1];
#endif
	return cp;
}
/* Print free list map */
memstat()
{
	HEADER *p;
	unsigned total;

	printf("malloc fails %u\n",memfail);
	total = 0;
	p = &base;
	do {
		total += p->s.size * sizeof(HEADER);
		printf("0x%x %u",(unsigned)p,p->s.size * sizeof(HEADER));
		if(p == allocp)
			printf(" <-- allocp\n");
		else
			printf("\n");
		p = p->s.ptr;
	} while(p != &base);
	printf("Total: %u bytes\n",total);
#ifdef	ALLOCDEBUG
	{
		struct alloc *ap;
		printf("addr      size      caller\n");
		for(ap = alloc;ap < &alloc[NALLOC];ap++){
			if(ap->addr != NULLCHAR)
				printf("0x%-8x%-10d%-10lx\n",ap->addr,
					ap->size,ap->pc);
		}
	}
#endif
	fflush(stdout);
}
