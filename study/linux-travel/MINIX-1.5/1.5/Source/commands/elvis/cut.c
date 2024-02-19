/* cut.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains function which manipulate the cut buffers. */

#include "vi.h"
extern char	*malloc();

# define NANNONS	9	/* number of annonymous buffers */

static struct cutbuf
{
	short	*phys;	/* pointer to an array of #s of BLKs containing text */
	int	nblks;	/* number of blocks in phys[] array */
	int	start;	/* offset into first block of start of cut */
	int	end;	/* offset into last block of end of cut */
	int	fd;	/* fd of tmp file, or -1 to use tmpfd */
	char	lnmode;	/* boolean: line-mode cut? (as opposed to char-mode) */
}
	named[27],	/* cut buffers "a through "z and ". */
	annon[NANNONS];	/* annonymous cut buffers */

static char	cbname;	/* name chosen for next cut/paste operation */


#ifndef NO_RECYCLE
/* This function builds a list of all blocks needed in the current tmp file
 * for the contents of cut buffers.
 * !!! WARNING: if you have more than ~450000 bytes of text in all of the
 * cut buffers, then this will fail disastrously, because buffer overflow
 * is *not* allowed for.
 */
int cutneeds(need)
	BLK		*need;	/* this is where we deposit the list */
{
	struct cutbuf	*cb;	/* used to count through cut buffers */
	int		i;	/* used to count through blocks of a cut buffer */
	int		n;	/* total number of blocks in list */

	n = 0;

	/* first the named buffers... */
	for (cb = named; cb < &named[27]; cb++)
	{
		if (cb->fd > 0)
			continue;

		for (i = cb->nblks; i-- > 0; )
		{
			need->n[n++] = cb->phys[i];
		}
	}

	/* then the anonymous buffers */
	for (cb = annon; cb < &annon[NANNONS]; cb++)
	{
		if (cb->fd > 0)
			continue;

		for (i = cb->nblks; i-- > 0; )
		{
			need->n[n++] = cb->phys[i];
		}
	}

	return n;
}
#endif

/* This function is called when we are about to abort a tmp file.  If any
 * cut buffers still need the file, then a copy of the file should be
 * created for use by the cut buffers.
 *
 * To minimize the number of extra files lying around, only named cut buffers
 * are preserved in a file switch; the annonymous buffers just go away.
 */
cutswitch(tmpname)
	char	*tmpname; /* name of the tmp file */
{
	char	cutname[50];	/* used to build a new name for the tmp file */
	int	fd;		/* a new fd for the current tmp file */
	int	i;

	/* discard all annonymous cut buffers */
	for (i = 0; i < NANNONS; i++)
	{
		cutfree(&annon[i]);
	}

	/* find the first named buffer that uses this tmp file */
	for (i = 0; i < 27; i++)
	{
		if (named[i].nblks > 0 && named[i].fd < 0)
		{
			break;
		}
	}

	/* if none of them use this tmp file, then we're done */
	if (i == 27)
	{
		return;
	}

	/* else we'll need this file and an fd a little longer */
		/* !!! we could use some error checking here */
	fd = dup(tmpfd);
	sprintf(cutname, CUTNAME, getpid(), fd);
	link(tmpname, cutname);

	/* have all cut buffers use the new fd instead */
	for (; i < 27; i++)
	{
		if (named[i].nblks > 0 && named[i].fd < 0)
		{
			named[i].fd = fd;
		}
	}
}

/* This function frees a cut buffer */
static cutfree(buf)
	struct cutbuf	*buf;
{
	char	cutname[50];
	int	i;

	/* return immediately if the buffer is already empty */
	if (buf->nblks <= 0)
	{
		return;
	}

	/* else free up stuff */
	buf->nblks = 0;
	free(buf->phys);

	/* see if anybody else needs this tmp file */
	if (buf->fd >= 0)
	{
		for (i = 0; i < 27; i++)
		{
			if (named[i].nblks > 0 && named[i].fd >= 0)
			{
				break;
			}
		}
	}

	/* if nobody else needs it, then discard the tmp file */
	if (buf->fd >= 0 && i == 27)
	{
		sprintf(cutname, CUTNAME, getpid(), buf->fd);
		unlink(cutname);
		close(buf->fd);
	}
}

/* This function should be called just before termination of vi */
cutend()
{
	int	i;

	/* free all named cut buffers, since they might be forcing an older
	 * tmp file to be retained.
	 */
	for (i = 0; i < 27; i++)
	{
		cutfree(&named[i]);
	}
}


/* This function is used to select the cut buffer to be used next */
cutname(name)
	int	name;	/* a single character */
{
	cbname = name;
}




/* This function copies a selected segment of text to a cut buffer */
cut(from, to)
	MARK	from;		/* start of text to cut */
	MARK	to;		/* end of text to cut */
{
	int		first;	/* logical number of first block in cut */
	int		last;	/* logical number of last block used in cut */
	long		line;	/* a line number */
	register struct cutbuf *cb;
	register long	l;
	register int	i;
	register char	*scan;
	char		*blkc;

	/* decide which cut buffer to use */
	if (!cbname)
	{
		/* free up the last annonymous cut buffer */
		cutfree(&annon[NANNONS - 1]);

		/* shift the annonymous cut buffers */
		for (i = NANNONS - 1; i > 0; i--)
		{
			annon[i] = annon[i - 1];
		}

		/* use the first annonymous cut buffer */
		cb = annon;
		cb->nblks = 0;
	}
	else if (cbname >= 'a' && cbname <= 'z')
	{
		cb = &named[cbname - 'a'];
		cutfree(cb);
	}
	else if (cbname == '.')
	{
		cb = &named[26];
		cutfree(cb);
	}
	else
	{
		msg("Invalid cut buffer name: \"%c", cbname);
		cbname = '\0';
		return;
	}
	cbname = '\0';
	cb->fd = -1;

	/* detect whether we're doing a line mode cut */
	cb->lnmode = (markidx(from) == 0 && markidx(to) == 0);

	/* ---------- */

	/* Reporting... */	
	if (markidx(from) == 0 && markidx(to) == 0)
	{
		rptlines = markline(to) - markline(from);
		rptlabel = "yanked";
	}

	/* ---------- */
blksync();
	/* find the first block in the cut */
	line = markline(from);
	for (first = 1; line > lnum[first]; first++)
	{
	}

	/* fetch text of the block containing that line */
	blkc = scan = blkget(first)->c;

	/* find the mark in the block */
	for (l = lnum[first - 1]; ++l < line; )
	{
		while (*scan++ != '\n')
		{
		}
	}
	scan += markidx(from);

	/* remember the offset of the start */
	cb->start = scan - blkc;

	/* ---------- */

	/* find the last block in the cut */
	line = markline(to);
	for (last = first; line > lnum[last]; last++)
	{
	}

	/* fetch text of the block containing that line */
	if (last != first)
	{
		blkc = scan = blkget(last)->c;
	}
	else
	{
		scan = blkc;
	}

	/* find the mark in the block */
	for (l = lnum[last - 1]; ++l < line; )
	{
		while (*scan++ != '\n')
		{
		}
	}
	if (markline(to) <= nlines)
	{
		scan += markidx(to);
	}

	/* remember the offset of the end */
	cb->end = scan - blkc;

	/* ------- */

	/* remember the physical block numbers of all included blocks */
	cb->nblks = last - first;
	if (cb->end > 0)
	{
		cb->nblks++;
	}
	cb->phys = (short *)malloc((unsigned)(cb->nblks * sizeof(short)));
	for (i = 0; i < cb->nblks; i++)
	{
		cb->phys[i] = hdr.n[first++];
	}
}


static readcutblk(cb, blkno)
	struct cutbuf	*cb;
	int		blkno;
{
	int		fd;	/* either tmpfd or cb->fd */

	/* decide which fd to use */
	if (cb->fd >= 0)
	{
		fd = cb->fd;
	}
	else
	{
		fd = tmpfd;
	}

	/* get the block */
	lseek(fd, (long)cb->phys[blkno] * (long)BLKSIZE, 0);
	if (read(fd, tmpblk.c, BLKSIZE) != BLKSIZE)
	{
		msg("Error reading back from tmp file for pasting!");
	}
}


/* This function inserts text from a cut buffer, and returns the MARK where
 * insertion ended.  Return MARK_UNSET on errors.
 */
MARK paste(at, after, retend)
	MARK	at;	/* where to insert the text */
	int	after;	/* boolean: insert after mark? (rather than before) */
	int	retend;	/* boolean: return end marker (rather than start) */
{
	register struct cutbuf	*cb;
	register int		i;

	/* decide which cut buffer to use */
	if (cbname >= 'a' && cbname <= 'z')
	{
		cb = &named[cbname - 'a'];
	}
	else if (cbname >= '1' && cbname <= '9')
	{
		cb = &annon[cbname - '1'];
	}
	else if (cbname == '.')
	{
		cb = &named[26];
	}
	else if (!cbname)
	{
		cb = annon;
	}
	else
	{
		msg("Invalid cut buffer name: \"%c", cbname);
		return MARK_UNSET;
	}

	/* make sure it isn't empty */
	if (cb->nblks == 0)
	{
		if (cbname)
			msg("Cut buffer \"%c is empty", cbname);
		else
			msg("Cut buffer is empty");
		cbname = '\0';
		return MARK_UNSET;
	}
	cbname = '\0';

	/* adjust the insertion MARK for "after" and line-mode cuts */
	if (cb->lnmode)
	{
		at &= ~(BLKSIZE - 1);
		if (after)
		{
			at += BLKSIZE;
		}
	}
	else if (after)
	{
		/* careful! if markidx(at) == 0 we might be pasting into an
		 * empty line -- so we can't blindly increment "at".
		 */
		if (markidx(at) == 0)
		{
			pfetch(markline(at));
			if (plen != 0)
			{
				at++;
			}
		}
		else
		{
			at++;
		}
	}

	/* put a copy of the "at" mark in the mark[] array, so it stays in
	 * sync with changes made via add().
	 */
	mark[27] = at;

	/* simple one-block paste? */
	if (cb->nblks == 1)
	{
		/* get the block */
		readcutblk(cb, 0);

		/* isolate the text we need within it */
		if (cb->end)
		{
			tmpblk.c[cb->end] = '\0';
		}

		/* insert it */
		ChangeText
		{
			add(at, &tmpblk.c[cb->start]);
		}
	}
	else
	{
		/* multi-block paste */

		ChangeText
		{
			i = cb->nblks - 1;

			/* add text from the last block first */
			if (cb->end > 0)
			{
				readcutblk(cb, i);
				tmpblk.c[cb->end] = '\0';
				add(at, tmpblk.c);
				i--;
			}

			/* add intervening blocks */
			while (i > 0)
			{
				readcutblk(cb, i);
				add(at, tmpblk.c);
				i--;
			}

			/* add text from the first cut block */
			readcutblk(cb, 0);
			add(at, &tmpblk.c[cb->start]);
		}
	}

	/* Reporting... */
	rptlines = markline(mark[27]) - markline(at);
	rptlabel = "pasted";

	/* correct the redraw range */
	redrawafter = redrawpre = markline(at);
	redrawpost = markline(mark[27]);

	/* return the mark at the beginning of inserted text */
	if (retend)
	{
		return mark[27] - 1L;
	}
	return at;
}
