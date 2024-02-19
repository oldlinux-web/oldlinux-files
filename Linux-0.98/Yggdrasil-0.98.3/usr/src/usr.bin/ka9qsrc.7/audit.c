#include <stdio.h>
#include "global.h"
#include "mbuf.h"

union header {
	struct {
		union header *ptr;
		unsigned size;
	} s;
	long l;
};
/* Perform sanity checks on mbuf. Print any errors, return 0 if none,
 * nonzero otherwise
 */
audit(bp,file,line)
struct mbuf *bp;
char *file;
int line;
{
	register struct mbuf *bp1;

	for(bp1 = bp;bp1 != NULLBUF; bp1 = bp1->next)
		audit_mbuf(bp1,file,line);
}
audit_mbuf(bp,file,line)
register struct mbuf *bp;
char *file;
int line;
{
	union header *blk;
	char *bufstart,*bufend;
	int16 overhead = sizeof(union header) + sizeof(struct mbuf);
	int16 datasize;
	int errors = 0;
	char *heapbot,*heaptop;
#ifndef	ATARI_ST		/* Atari/MWC uses an arena, not a heap -- hyc */
	extern char _Uend;
	extern int _STKRED;
#endif
	if(bp == NULLBUF)
		return;

#ifndef	ATARI_ST
	heapbot = &_Uend;
	heaptop = (char *) -_STKRED;
#endif
	/* Does buffer appear to be a valid malloc'ed block? */
	blk = ((union header *)bp) - 1;
	if(blk->s.ptr != blk){
		printf("Garbage bp %lx\n",(long)bp);
		errors++;
	}
	if((datasize = blk->s.size*sizeof(union header) - overhead) != 0){
		/* mbuf has data area associated with it, verify that
		 * pointers are within it
		 */
		bufstart = (char *)(bp + 1);
		bufend = (char *)bufstart + datasize;
		if(bp->data < bufstart){
			printf("Data pointer before buffer\n");
			errors++;
		}
		if(bp->data + bp->cnt > bufend){
			printf("Data pointer + count past bounds\n");
			errors++;
		}
	} else {
		/* Dup'ed mbuf, at least check that pointers are within
		 * heap area
		*/

		if(bp->data < heapbot
		 || bp->data + bp->cnt > heaptop){
			printf("Data outside heap\n");
			errors++;
		}
	}
	/* Now check link list pointers */
	if(bp->next != NULLBUF && ((bp->next < (struct mbuf *)heapbot)
		 || bp->next > (struct mbuf *)heaptop)){
			printf("next pointer out of limits\n");
			errors++;
	}
	if(bp->anext != NULLBUF && ((bp->anext < (struct mbuf *)heapbot)
		 || bp->anext > (struct mbuf *)heaptop)){
			printf("anext pointer out of limits\n");
			errors++;
	}
	if(errors != 0){
		dumpbuf(bp);
		printf("PANIC: buffer audit failure in %s line %d\n",file,line);
		fflush(stdout);
		for(;;)
			;
	}
	return;
}
dumpbuf(bp)
struct mbuf *bp;
{
	union header *blk;
	if(bp == NULLBUF){
		printf("NULL BUFFER\n");
		return;
	}
	blk = ((union header *)bp) - 1;
	printf("bp %lx tot siz %u data %lx cnt %u next %lx anext %lx\n",
		(long)bp,blk->s.size * sizeof(union header),
		(long)bp->data,bp->cnt,
		(long)bp->next,(long)bp->anext);
}
