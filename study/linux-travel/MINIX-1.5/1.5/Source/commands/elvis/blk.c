/* blk.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains the functions that get/put blocks from the temp file.
 * It also contains the "do" and "undo" functions.
 */

#include "vi.h"

#define NBUFS	5		/* must be at least 3 -- more is better */

extern long lseek();

/*------------------------------------------------------------------------*/

BLK		hdr;		/* buffer for the header block */

static int	b4cnt;		/* used to count context of beforedo/afterdo */
static struct _blkbuf
{
	BLK	buf;		/* contents of a text block */
	ushort	logical;	/* logical block number */
	int	dirty;		/* must the buffer be rewritten? */
}
		blk[NBUFS],	/* buffers for text[?] blocks */
		*toonew,	/* buffer which shouldn't be recycled yet */
		*newtoo,	/* another buffer which should be recycled */
		*recycle = blk;	/* next block to be recycled */





/* This function wipes out all buffers */
blkinit()
{
	int	i;

	for (i = 0; i < NBUFS; i++)
	{
		blk[i].logical = 0;
		blk[i].dirty = FALSE;
	}
	for (i = 0; i < MAXBLKS; i++)
	{
		hdr.n[i] = 0;
	}
}

/* This function allocates a buffer and fills it with a given block's text */
BLK *blkget(logical)
	int	logical;	/* logical block number to fetch */
{
	register struct _blkbuf	*this;	/* used to step through blk[] */
	register int		i;

	/* if logical is 0, just return the hdr buffer */
	if (logical == 0)
	{
		return &hdr;
	}

	/* see if we have that block in mem already */
	for (this = blk; this < &blk[NBUFS]; this++)
	{
		if (this->logical == logical)
		{
			newtoo = toonew;
			toonew = this;
			return &this->buf;
		}
	}

	/* choose a block to be recycled */
	do
	{
		this = recycle++;
		if (recycle == &blk[NBUFS])
		{
			recycle = blk;
		}
	} while (this == toonew || this == newtoo);

	/* if it contains a block, flush that block */
	blkflush(this);

	/* fill this buffer with the desired block */
	this->logical = logical;
	if (hdr.n[logical])
	{
		/* it has been used before - fill it from tmp file */
		lseek(tmpfd, (long)hdr.n[logical] * (long)BLKSIZE, 0);
		if (read(tmpfd, this->buf.c, BLKSIZE) != BLKSIZE)
		{
			msg("Error reading back from tmp file!");
		}
	}
	else
	{
		/* it is new - zero it */
		for (i = 0; i < BLKSIZE; i++)
		{
			this->buf.c[i] = 0;
		}
	}

	/* This isn't really a change, but it does potentially invalidate
	 * the kinds of shortcuts that the "changes" variable is supposed
	 * to protect us from... so count it as a change.
	 */
	changes++;

	/* mark it as being "not dirty" */
	this->dirty = 0;

	/* return it */
	newtoo = toonew;
	toonew = this;
	return &this->buf;
}



/* This function writes a block out to the temporary file */
blkflush(this)
	register struct _blkbuf	*this;	/* the buffer to flush */
{
	long	seekpos;	/* seek position of the new block */
	ushort	physical;	/* physical block number */

	/* if its empty (an orphan blkadd() maybe?) then make it dirty */
	if (this->logical && !*this->buf.c)
	{
		blkdirty(&this->buf);
	}

	/* if it's an empty buffer or a clean version is on disk, quit */
	if (!this->logical || hdr.n[this->logical] && !this->dirty)
	{
		return;
	}

	/* find a free place in the file */
#ifndef NO_RECYCLE
	seekpos = allocate();
	lseek(tmpfd, seekpos, 0);
#else
	seekpos = lseek(tmpfd, 0L, 2);
#endif
	physical = seekpos / BLKSIZE;

	/* put the block there */
	if (write(tmpfd, this->buf.c, BLKSIZE) != BLKSIZE)
	{
		msg("Trouble writing to tmp file");
	}
	this->dirty = FALSE;

	/* update the header so it knows we put it there */
	hdr.n[this->logical] = physical;
}


/* This function sets a block's "dirty" flag or deletes empty blocks */
blkdirty(bp)
	BLK	*bp;	/* buffer returned by blkget() */
{
	register int 	i, j;
	register char	*scan;
	register int	k;

	/* find the buffer */
	for (i = 0; i < NBUFS && bp != &blk[i].buf; i++)
	{
	}
#ifdef DEBUG
	if (i >= NBUFS)
	{
		msg("blkdirty() called with unknown buffer at 0x%lx", bp);
		return;
	}
	if (blk[i].logical == 0)
	{
		msg("blkdirty called with freed buffer");
		return;
	}
#endif

	/* if this block ends with line# INFINITY, then it must have been
	 * allocated unnecessarily during tmpstart().  Forget it.
	 */
	if (lnum[blk[i].logical] == INFINITY)
	{
#ifdef DEBUG
		if (blk[i].buf.c[0])
		{
			msg("bkldirty called with non-empty extra BLK");
		}
#endif
		blk[i].logical = 0;
		blk[i].dirty = FALSE;
		return;
	}

	/* count lines in this block */
	for (j = 0, scan = bp->c; *scan && scan < bp->c + BLKSIZE; scan++)
	{
		if (*scan == '\n')
		{
			j++;
		}
	}

	/* adjust lnum, if necessary */
	k = blk[i].logical;
	j += (lnum[k - 1] - lnum[k]);
	if (j != 0)
	{
		nlines += j;
		while (k < MAXBLKS && lnum[k] != INFINITY)
		{
			lnum[k++] += j;
		}
	}

	/* if it still has text, mark it as dirty */
	if (*bp->c)
	{
		blk[i].dirty = TRUE;
	}
	else /* empty block, so delete it */
	{
		/* adjust the cache */
		k = blk[i].logical;
		for (j = 0; j < NBUFS; j++)
		{
			if (blk[j].logical >= k)
			{
				blk[j].logical--;
			}
		}

		/* delete it from hdr.n[] and lnum[] */
		blk[i].logical = 0;
		blk[i].dirty = FALSE;
		while (k < MAXBLKS - 1)
		{
			hdr.n[k] = hdr.n[k + 1];
			lnum[k] = lnum[k + 1];
			k++;
		}
		hdr.n[MAXBLKS - 1] = 0;
		lnum[MAXBLKS - 1] = INFINITY;
	}
}


/* insert a new block into hdr, and adjust the cache */
BLK *blkadd(logical)
	int		logical;	/* where to insert the new block */
{
	register int	i;

	/* adjust hdr and lnum[] */
	for (i = MAXBLKS - 1; i > logical; i--)
	{
		hdr.n[i] = hdr.n[i - 1];
		lnum[i] = lnum[i - 1];
	}
	hdr.n[logical] = 0;
	lnum[logical] = lnum[logical - 1];

	/* adjust the cache */
	for (i = 0; i < NBUFS; i++)
	{
		if (blk[i].logical >= logical)
		{
			blk[i].logical++;
		}
	}

	/* return the new block, via blkget() */
	return blkget(logical);
}


/* This function forces all dirty blocks out to disk */
blksync()
{
	int	i;

	for (i = 0; i < NBUFS; i++)
	{
		/* blk[i].dirty = TRUE; */
		blkflush(&blk[i]);
	}
	if (*o_sync)
	{
		sync();
	}
}

/*------------------------------------------------------------------------*/

MARK	undocurs;	/* where the cursor should go if undone */


/* This function should be called before each command that changes the text.
 * It defines the state that undo() will reset the file to.
 */
beforedo(undo)
	int		undo;	/* boolean: is this for an undo? */
{
	static long	oldnlines;
	static long	oldlnum[MAXBLKS];
	register int	i;
	register long	l;

	/* if this is a nested call to beforedo, quit! Use larger context */
	if (b4cnt++ > 0)
	{
		return;
	}

	/* force all block buffers to disk */
	blksync();

#ifndef NO_RECYCLE
	/* perform garbage collection on blocks from tmp file */
	garbage();
#endif

	/* force the header out to disk */
	lseek(tmpfd, 0L, 0);
	if (write(tmpfd, hdr.c, BLKSIZE) != BLKSIZE)
	{
		msg("Trouble writing header to tmp file ");
	}

	/* set the file's "dirty" flag */
	setflag(file, MODIFIED);

	/* copy or swap oldnlines <--> nlines, oldlnum <--> lnum */
	if (undo)
	{
		for (i = 0; i < MAXBLKS; i++)
		{
			l = lnum[i];
			lnum[i] = oldlnum[i];
			oldlnum[i] = l;
		}
		l = nlines;
		nlines = oldnlines;
		oldnlines = l;
	}
	else
	{
		for (i = 0; i < MAXBLKS; i++)
		{
			oldlnum[i] = lnum[i];
		}
		oldnlines = nlines;
	}

	/* save the cursor position */
	undocurs = cursor;

	/* upon return, the calling function continues and makes changes... */
}

/* This function marks the end of a (nested?) change to the file */
afterdo()
{
	if (--b4cnt)
	{
		return;
	}

	/* make sure the cursor wasn't left stranded in deleted text */
	if (markline(cursor) > nlines)
	{
		cursor = MARK_LAST;
	}
	/* NOTE: it is still possible that markidx(cursor) is after the
	 * end of a line, so the Vi mode will have to take care of that
	 * itself */
}

/* This function discards all changes made since the last call to beforedo() */
undo()
{
	BLK		oldhdr;

	/* if beforedo() has never been run, fail */
	if (!tstflag(file, MODIFIED))
	{
		msg("You haven't modified this file yet.");
		return;
	}

	/* read the old header form the tmp file */
	lseek(tmpfd, 0L, 0);
	if (read(tmpfd, oldhdr.c, BLKSIZE) != BLKSIZE)
	{
		msg("Trouble rereading the old header from tmp file");
	}

	/* "do" the changed version, so we can undo the "undo" */
	cursor = undocurs;
	beforedo(TRUE);
	afterdo();

	/* wipe out the block buffers - we can't assume they're correct */
	blkinit();

	/* use the old header -- and therefore the old text blocks */
	hdr = oldhdr;

	/* This is a change */
	changes++;
}
