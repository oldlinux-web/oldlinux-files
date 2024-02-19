/* tmpfile.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains functions which create & readback a TMPFILE */


#include <sys/types.h>
#include <sys/stat.h>
#include "vi.h"

/* The FAIL() macro prints an error message and then exits. */
#define FAIL(why,arg)	mode = MODE_EX; msg(why, arg); endwin(); exit(9)

/* This is the name of the temp file */
static char	tmpname[80];

/* This function creates the temp file and copies the original file into it.
 * Returns if successful, or stops execution if it fails.
 */
int tmpstart(filename)
	char		*filename; /* name of the original file */
{
	int		origfd;	/* fd used for reading the original file */
	struct stat	statb;	/* stat buffer, used to examine inode */
	register BLK	*this;	/* pointer to the current block buffer */
	register BLK	*next;	/* pointer to the next block buffer */
	int		inbuf;	/* number of characters in a buffer */
	int		nread;	/* number of bytes read */
	register int	j, k;
	int		i;

	/* switching to a different file certainly counts as a change */
	changes++;
	redraw(MARK_UNSET, FALSE);

	/* open the original file for reading */
	*origname = '\0';
	if (filename)
	{
		strcpy(origname, filename);
		origfd = open(origname, O_RDONLY);
		if (origfd < 0 && errno != ENOENT)
		{
			FAIL("Can't open \"%s\"", origname);
		}
		if (origfd >= 0)
		{
			if (fstat(origfd, &statb) < 0)
			{
				FAIL("Can't stat \"%s\"", origname);
			}
			if (origfd >= 0 && (statb.st_mode & S_IFMT) != S_IFREG)
			{
				FAIL("\"%s\" is not a regular file", origname);
			}
		}
		else
		{
			stat(".", &statb);
		}
		if (origfd >= 0)
		{
			origtime = statb.st_mtime;
			if (*o_readonly || !(statb.st_mode &
				  (statb.st_uid != geteuid() ? 0022 : 0200)))
			{
				setflag(file, READONLY);
			}
		}
		else
		{
			origtime = 0L;
		}
	}
	else
	{
		setflag(file, NOFILE);
		origfd = -1;
		origtime = 0L;
		stat(".", &statb);
	}

	/* make a name for the tmp file */
	sprintf(tmpname, TMPNAME, statb.st_ino, statb.st_dev);

	/* make sure nobody else is editing the same file */
	if (access(tmpname, 0) == 0)
	{
		FAIL("\"%s\" is busy", filename);
	}

	/* create the temp file */
	close(creat(tmpname, 0600));
	tmpfd = open(tmpname, O_RDWR);
	if (tmpfd < 0)
	{
		FAIL("Can't create temporary file, errno=%d", errno);
	}

	/* allocate space for the header in the file */
	write(tmpfd, hdr.c, BLKSIZE);

	/* initialize lnum[] */
	for (i = 1; i < MAXBLKS; i++)
	{
		lnum[i] = INFINITY;
	}
	lnum[0] = 0;

	/* if there is no original file, then create a 1-line file */
	if (origfd < 0)
	{
		hdr.n[0] = 0;	/* invalid inode# denotes new file */

		this = blkget(1); 	/* get the new text block */
		strcpy(this->c, "\n");	/* put a line in it */

		lnum[1] = 1;	/* block 1 ends with line 1 */
		nlines = 1;	/* there is 1 line in the file */

		if (*origname)
		{
			msg("\"%s\" [NEW FILE]  1 line", origname);
		}
		else
		{
			msg("\"[NO FILE]\"  1 line");
		}
	}
	else /* there is an original file -- read it in */
	{
		hdr.n[0] = statb.st_ino;
		nlines = 0;

		/* preallocate 1 "next" buffer */
		i = 1;
		next = blkget(i);
		inbuf = 0;

		/* loop, moving blocks from orig to tmp */
		for (;;)
		{
			/* "next" buffer becomes "this" buffer */
			this = next;

			/* read [more] text into this block */
			nread = read(origfd, this->c + inbuf, BLKSIZE - 1 - inbuf);
			if (nread < 0)
			{
				close(origfd);
				close(tmpfd);
				tmpfd = -1;
				unlink(tmpname);
				FAIL("Error reading \"%s\"", origname);
			}

			/* convert NUL characters to something else */
			for (k = inbuf; k < inbuf + nread; k++)
			{
				if (!this->c[k])
				{
					setflag(file, HADNUL);
					this->c[k] = 0x80;
				}
			}
			inbuf += nread;

			/* if the buffer is empty, quit */
			if (inbuf == 0)
			{
				break;
			}

			/* search backward for last newline */
			for (k = inbuf; --k >= 0 && this->c[k] != '\n';)
			{
			}
			if (k++ < 0)
			{
				if (inbuf >= BLKSIZE - 1)
				{
					k = 80;
				}
				else
				{
					k = inbuf;
				}
			}

			/* allocate next buffer */
			next = blkget(++i);

			/* move fragmentary last line to next buffer */
			inbuf -= k;
			for (j = 0; k < BLKSIZE; j++, k++)
			{
				next->c[j] = this->c[k];
				this->c[k] = 0;
			}

			/* if necessary, add a newline to this buf */
			for (k = BLKSIZE - inbuf; --k >= 0 && !this->c[k]; )
			{
			}
			if (this->c[k] != '\n')
			{
				setflag(file, ADDEDNL);
				this->c[k + 1] = '\n';
			}

			/* count the lines in this block */
			for (k = 0; k < BLKSIZE && this->c[k]; k++)
			{
				if (this->c[k] == '\n')
				{
					nlines++;
				}
			}
			lnum[i - 1] = nlines;
		}

		/* if this is a zero-length file, add 1 line */
		if (nlines == 0)
		{
			this = blkget(1); 	/* get the new text block */
			strcpy(this->c, "\n");	/* put a line in it */

			lnum[1] = 1;	/* block 1 ends with line 1 */
			nlines = 1;	/* there is 1 line in the file */
		}

		/* report the number of lines in the file */
		msg("\"%s\"  %ld line%s", origname, nlines, nlines == 1 ? "" : "s");
	}

	/* initialize the cursor to start of line 1 */
	cursor = MARK_FIRST;

	/* close the original file */
	close(origfd);

#ifndef NO_RECYCLE
	/* initialize the block allocator */
	garbage();
#endif
	return 0;
}



/* This function copies the temp file back onto an original file.
 * Returns TRUE if successful, or FALSE if the file could NOT be saved.
 */
tmpsave(filename)
	char	*filename;	/* the name to save it to */
{
	int		fd;	/* fd of the file we're writing to */
	register int	len;	/* length of a text block */
	register BLK	*this;	/* a text block */
	register int	i;

	/* if no filename is given, assume the original file name */
	if (!filename || !*filename)
	{
		filename = origname;
	}

	/* if still no file name, then fail */
	if (!*filename)
	{
		msg("Don't know a name for this file -- NOT WRITTEN");
		return FALSE;
	}

	/* open the file */
	if (*filename == '>' && filename[1] == '>')
	{
		filename += 2;
		while (*filename == ' ' || *filename == '\t')
		{
			filename++;
		}
#ifdef O_APPEND
		fd = open(filename, O_WRONLY|O_APPEND);
#else
		fd = open(filename, O_WRONLY);
		lseek(fd, 0L, 2);
#endif
	}
	else
	{
		fd = creat(filename, 0666);
	}
	if (fd < 0)
	{
		msg("Can't write to \"%s\" -- NOT WRITTEN", filename);
		return FALSE;
	}

	/* write each text block to the file */
	for (i = 1; i < MAXBLKS && (this = blkget(i)) && this->c[0]; i++)
	{
		for (len = 0; len < BLKSIZE && this->c[len]; len++)
		{
		}
		write(fd, this->c, len);
	}

	/* reset the "modified" flag */
	clrflag(file, MODIFIED);
	if (strcmp(filename, SCRATCHFILE))
	{
		msg("Wrote \"%s\"  %ld lines", filename, nlines);
	}

	/* close the file */
	close(fd);

	return TRUE;
}


/* This function deletes the temporary file.  If the file has been modified
 * and "bang" is FALSE, then it returns FALSE without doing anything; else
 * it returns TRUE.
 *
 * If the "autowrite" option is set, then instead of returning FALSE when
 * the file has been modified and "bang" is false, it will call tmpend().
 */
tmpabort(bang)
	int	bang;
{
	/* if there is no file, return successfully */
	if (tmpfd < 0)
	{
		return TRUE;
	}

	/* see if we must return FALSE -- can't quit */
	if (!bang && tstflag(file, MODIFIED))
	{
		/* if "autowrite" is set, then act like tmpend() */
		if (*o_autowrite)
			return tmpend(bang);
		else
			return FALSE;
	}

	/* delete the tmp file */
	cutswitch(tmpname);
	close(tmpfd);
	tmpfd = -1;
	unlink(tmpname);
	strcpy(prevorig, origname);
	prevline = markline(cursor);
	*origname = '\0';
	origtime = 0L;
	blkinit();
	nlines = 0;
	initflags();
	return TRUE;
}

/* This function saves the file if it has been modified, and then deletes
 * the temporary file. Returns TRUE if successful, or FALSE if the file
 * needs to be saved but can't be.  When it returns FALSE, it will not have
 * deleted the tmp file, either.
 */
tmpend(bang)
	int	bang;
{
	/* save the file if it has been modified */
	if (tstflag(file, MODIFIED) && !tmpsave((char *)0) && !bang)
	{
		return FALSE;
	}

	/* delete the tmp file */
	tmpabort(TRUE);

	return TRUE;
}
