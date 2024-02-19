/*
 * Routines for implementing RAM-based virtual files.
 * Written for Mark Williams stdio library.
 *   -- Howard Chu  03:15, 12-07-88
 */

#include <stdio.h>

#define	MEMSIZ	(16*1024)
#define	_FMEM	0x40		/* For stdio flags... */

typedef	struct	vfile {
	char buf[MEMSIZ];
	short cnt;
	struct vfile *next;
}	vfile;

int	vgetc(),vputc();
static void vfree();

FILE *tmpfile()
{
	register FILE *newbuf;
	vfile *ptr;

	newbuf=(FILE *)malloc(sizeof(struct FILE)+sizeof(struct vfile));

	if(newbuf != NULL) {
		ptr = (vfile *)((long)newbuf + sizeof(struct FILE));
		ptr->next = NULL;
	/*
	 * We'll use _cp as current pointer, _bp as buffer end, and
	 * _dp as buffer beginning... _cc is number of bytes in this
	 * block.
	 */
		newbuf->_cp=newbuf->_dp=newbuf->_bp=(char *)ptr;
		newbuf->_cc=0;
		newbuf->_gt=vgetc;
		newbuf->_pt=vputc;
		newbuf->_ff=(_FMEM | _FINUSE | _FSTBUF);
					/* Mem, in use, user buffer */
		newbuf->_uc=0;
		newbuf->_fd=0xff;
	}
	return newbuf;
}

int
vseek(fp, where, how)
register FILE *fp;
long where;
int how;
{
	vfile *ptr;

	if(fp->_ff & _FMEM) {

		ptr=(vfile *)(fp->_dp);
		ptr->cnt = fp->_cc;

		switch(how) {
		case 0: ptr=(vfile *)((long)fp+sizeof(struct FILE));
			while(where > MEMSIZ) {
				ptr = ptr->next;
				where -= MEMSIZ;
			}
			fp->_cp = fp->_dp = fp->_bp = (char *)ptr;
			fp->_cp += where;
			if(ptr)
				fp->_bp += ptr->cnt;
			break;
		default:	return (-1);		/* Too lazy... */
		}
		return 0;
	} else return fseek(fp, where, how);
}
	
int
vclose(fp)
register FILE *fp;
{
	vfile *ptr;

	if(fp->_ff & _FMEM) {
		ptr=(vfile *)(fp->_dp);
		vfree(ptr->next);
		free(fp);
		return 0;
	} else	return(fclose(fp));
}

static void
vfree(ptr)
register vfile *ptr;
{
	if(ptr) {
		vfree(ptr->next);
		free(ptr);
	}
}

int
vgetc(fp)
register FILE *fp;
{
	register vfile *ptr;

	if(fp->_cp < fp->_bp)
		return *(fp->_cp++);

	if(fp->_bp == fp->_dp)		/* No data yet. */
		return EOF;

	ptr=(vfile *)(fp->_dp);
	ptr = ptr->next;
	if(!ptr)			/* No more blocks in chain */
		return EOF;

	fp->_cp = fp->_dp = (char *)(ptr);	/* Get next block */
	fp->_bp = fp->_dp + ptr->cnt;
	return *(fp->_cp++);
}

int
vputc(c, fp)
char c;
register FILE *fp;
{
	vfile *ptr;

	if(fp->_cc < MEMSIZ) {
		*(fp->_cp++) = c;
		fp->_cc++;
		return c;
	}

	ptr=(vfile *)fp->_dp;
	if(!(ptr->next = (vfile *)malloc(sizeof(vfile))))
		return EOF;		/* End of the line...! */

	ptr->cnt = fp->_cc;
	ptr->next->next = (vfile *)0;
	fp->_cc = 1;
	fp->_cp = fp->_bp = fp->_dp = (char *)(ptr->next);

	*(fp->_cp++) = c;
	return c;
}
