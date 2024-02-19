/* cmd1.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains some of the EX commands - mostly ones that deal with
 * files, options, etc. -- anything except text.
 */

#include <ctype.h>
#include "vi.h"
#include "regexp.h"

#ifdef DEBUG
/* print the selected lines with info on the blocks */
void cmd_debug(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	register char	*scan;
	register long	l;
	register int	i;
	int		len;

	/* scan lnum[] to determine which block its in */
	l = markline(frommark);
	for (i = 1; l > lnum[i]; i++)
	{
	}

	do
	{
		/* fetch text of the block containing that line */
		scan = blkget(i)->c;

		/* calculate its length */
		if (scan[BLKSIZE - 1])
		{
			len = BLKSIZE;
		}
		else
		{
			len = strlen(scan);
		}

		/* print block stats */
		wprintw(stdscr, "##### hdr[%d]=%d, lnum[%d-1]=%ld, lnum[%d]=%ld (%ld lines)\n",
			i, hdr.n[i], i, lnum[i-1], i, lnum[i], lnum[i] - lnum[i - 1]);
		wprintw(stdscr, "##### len=%d, buf=0x%lx, %sdirty\n",
			len, scan, ((int *)scan)[MAXBLKS + 1] ? "" : "not ");
		if (bang)
		{
			while (--len >= 0)
			{
				addch(*scan);
				scan++;
			}
		}
		exrefresh();

		/* next block */
		i++;
	} while (i < MAXBLKS && lnum[i] && lnum[i - 1] < markline(tomark));
}


/* This function checks a lot of conditions to make sure they aren't screwy */
void cmd_validate(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	char	*scan;
	ushort	i;
	int	nlcnt;	/* used to count newlines */
	int	len;	/* counts non-NUL characters */

	/* check lnum[0] */
	if (lnum[0] != 0L)
	{
		wprintw(stdscr, "lnum[0] = %ld\n", lnum[0]);
	}

	/* check each block */
	for (i = 1; lnum[i] <= nlines; i++)
	{
		scan = blkget(i)->c;
		if (scan[BLKSIZE - 1])
		{
			wprintw(stdscr, "block %d has no NUL at the end\n", i);
		}
		else
		{
			for (nlcnt = len = 0; *scan; scan++, len++)
			{
				if (*scan == '\n')
				{
					nlcnt++;
				}
			}
			if (scan[-1] != '\n')
			{
				wprintw(stdscr, "block %d doesn't end with '\\n' (length %d)\n", i, len);
			}
			if (bang || nlcnt != lnum[i] - lnum[i - 1])
			{
				wprintw(stdscr, "block %d (line %ld?) has %d lines, but should have %ld\n",
					i, lnum[i - 1] + 1L, nlcnt, lnum[i] - lnum[i - 1]);
			}
		}
		exrefresh();
	}

	/* check lnum again */
	if (lnum[i] != INFINITY)
	{
		wprintw(stdscr, "hdr.n[%d] = %d, but lnum[%d] = %ld\n",
			i, hdr.n[i], i, lnum[i]);
	}

	wprintw(stdscr, "# = \"%s\", %% = \"%s\"\n", prevorig, origname);
}
#endif /* DEBUG */


void cmd_mark(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	/* validate the name of the mark */
	if (!extra || *extra < 'a' || *extra > 'z' || extra[1])
	{
		msg("Invalid mark name");
		return;
	}

	mark[*extra - 'a'] = tomark;
}

void cmd_write(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	int		fd;
	register long	l;
	register char	*scan;
	register int	i;

	/* if all lines are to be written, use tmpsave() */
	if (frommark == MARK_FIRST && tomark == MARK_LAST)
	{
		tmpsave(extra);
		return;
	}

	/* else do it line-by-line, like cmd_print() */
	fd = open((*extra == '>' ? extra + 2 : extra), 1);
	if (fd < 0)
	{
		fd = creat(extra, 0644);
		if (fd < 0)
		{
			msg("Can't write to \"%s\"", extra);
			return;
		}
	}
	else if (*extra == '>')
	{
		lseek(fd, 0L, 2);
	}
	for (l = markline(frommark); l <= markline(tomark); l++)
	{
		/* get the next line */
		scan = fetchline(l);
		i = strlen(scan);
		scan[i++] = '\n';

		/* print the line */
		write(fd, scan, i);
	}
	close(fd);
}	


void cmd_shell(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	static char	prevextra[80];

	/* special case: ":sh" means ":!sh" */
	if (cmd == CMD_SHELL)
	{
		extra = o_shell;
		frommark = tomark = 0L;
	}

	/* if extra is "!", substute previous command */
	if (*extra == '!')
	{
		if (!*prevextra)
		{
			addstr("No previous shell command to substitute for '!'\n");
			return;
		}
		extra = prevextra;
	}
	else
	{
		strcpy(prevextra, extra);
	}

	/* if no lines were specified, just run the command */
	suspend_curses();
	if (frommark == 0L)
	{
		system(extra);
	}
	else /* pipe lines from the file through the command */
	{
		filter(frommark, tomark, extra);
	}

	/* resume curses quietly for MODE_EX, but noisily otherwise */
	resume_curses(mode == MODE_EX);
}


void cmd_global(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;	/* rest of the command line */
{
	char	*cmdptr;	/* the command from the command line */
	char	cmdln[100];	/* copy of the command from the command line */
	char	*line;		/* a line from the file */
	long	l;		/* used as a counter to move through lines */
	long	lqty;		/* quantity of lines to be scanned */
	regexp	*re;		/* the compiled search expression */

	/* ":g! ..." is the same as ":v ..." */
	if (bang)
	{
		cmd = CMD_VGLOBAL;
	}

	/* make sure we got a search pattern */
	if (*extra != '/' && *extra != '?')
	{
		msg("Usage: %c /regular expression/ command", cmd == CMD_GLOBAL ? 'g' : 'v');
		return;
	}

	/* parse & compile the search pattern */
	cmdptr = parseptrn(extra);
	if (!extra[1])
	{
		msg("Can't use empty regular expression with '%c' command", cmd == CMD_GLOBAL ? 'g' : 'v');
		return;
	}
	re = regcomp(extra + 1);
	if (!re)
	{
		/* regcomp found & described an error */
		return;
	}

	/* for each line in the range */
	ChangeText
	{
		/* NOTE: we have to go through the lines in a forward order,
		 * otherwise "g/re/p" would look funny.  *BUT* for "g/re/d"
		 * to work, simply adding 1 to the line# on each loop won't
		 * work.  The solution: count lines relative to the end of
		 * the file.  Think about it.
		 */
		for (l = nlines - markline(frommark), lqty = markline(tomark) - markline(frommark) + 1L;
		     lqty > 0 && nlines - l >= 0;
		     l--, lqty--)
		{
			/* fetch the line */
			line = fetchline(nlines - l);

			/* if it contains the search pattern... */
			if ((!regexec(re, line, 1)) == (cmd != CMD_GLOBAL))
			{
				/* move the cursor to that line */
				cursor = MARK_AT_LINE(nlines - l);

				/* do the ex command (without mucking up
				 * the original copy of the command line)
				 */
				strcpy(cmdln, cmdptr);
				doexcmd(cmdln);
			}
		}
	}

	/* free the regexp */
	free(re);
}


void cmd_file(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	if (frommark == tomark)
	{
		msg( "\"%s\" %s%s %ld lines, this is line %ld  [%ld%%]",
			*origname ? origname : "[NO FILE]",
			tstflag(file, MODIFIED) ? "[MODIFIED]" : "",
			tstflag(file, READONLY) ? "[READONLY]" : "",
			nlines,
			markline(frommark),
			markline(frommark) * 100 / nlines);
	}
	else
	{
		msg( "\"%s\" %s%s %ld lines, range  %ld,%ld  contains %ld lines",
			*origname ? origname : "[NO FILE]",
			tstflag(file, MODIFIED) ? "[MODIFIED]" : "",
			tstflag(file, READONLY) ? "[READONLY]" : "",
			nlines,
			markline(frommark), markline(tomark),
			markline(tomark) - markline(frommark) + 1L);
	}
}


void cmd_edit(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	long	line = 1L;	/* might be set to prevline */

	if (!strcmp(extra, prevorig))
	{
		line = prevline;
	}

	/* switch files */
	if (tmpabort(bang))
	{
		tmpstart(extra);
		if (line <= nlines && line >= 1L)
		{
			cursor = MARK_AT_LINE(line);
		}
	}
	else
	{
		addstr("Use edit! to abort changes, or w to save changes\n");

		/* so we can say ":e!" with no filename next time... */
		strcpy(prevorig, extra);
		prevline = 1L;
	}
}


void cmd_next(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	int	i, j;
	char	*scan;
	char	*build;

	/* if extra stuff given, use ":args" to define a new args list */
	if (extra && *extra)
	{
		cmd_args(frommark, tomark, cmd, bang, extra);
	}

	/* move to the next arg */
	if (cmd == CMD_NEXT)
	{
		i = argno + 1;
	}
	else
	{
		i = argno - 1;
	}
	if (i < 0 || i >= nargs)
	{
		msg("No more files to edit");
		return;
	}

	/* find & isolate the name of the file to edit */
	for (j = i, scan = args; j > 0; j--)
	{
		while(!isspace(*scan))
		{
			scan++;
		}
		while(isspace(*scan))
		{
			scan++;
		}
	}
	for (build = tmpblk.c; *scan && !isspace(*scan); )
	{
		*build++ = *scan++;
	}
	*build = '\0';

	/* switch to the next file */
	if (tmpabort(bang))
	{
		tmpstart(tmpblk.c);
		argno = i;
	}
	else
	{
		addstr("Use next! to abort changes, or w to save changes\n");
	}
}


void cmd_xit(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	static long	whenwarned;	/* when the user was last warned of extra files */

	/* if there are more files to edit, then warn user */
	if (argno + 1 < nargs && whenwarned != changes)
	{
		msg("More files to edit -- Use \":n\" to go to next file");
		whenwarned = changes;
		return;
	}

	/* else try to save this file */
	if (tmpend(bang))
	{
		mode = MODE_QUIT;
	}
	else
	{
		msg("Could not save file -- use quit! to abort changes, or w filename");
	}
}


void cmd_args(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	char	*scan;
	int	i;
	int	spc;	/* boolean: was previous char a space? */

	/* if no extra names given, or just current name, then report the args
	 * we have now.
	 */
	if (!extra || !*extra)
	{
		/* for each element of args... */
		for (scan = args, spc = TRUE, i = 0; *scan; )
		{
			/* bracket before the current arg */
			if (*scan != ' ' && *scan != '\t' && spc && i == argno)
			{
				qaddch('[');
			}

			qaddch(*scan);

			spc = (*scan == ' ' || *scan == '\t');

			scan++;

			/* bracket after current arg */
			if ((!*scan || *scan == ' ' || *scan == '\t') && !spc)
			{
				if (i == argno)
				{
					qaddch(']');
				}
				i++;
			}
		}
	
		/* write a trailing newline */
		addch('\n');
		exrefresh();
	}
	else /* new args list given */
	{
		strcpy(args, extra);
		argno = -1; /* before the first, so :next wil go to first */

		/* count the names */
		for (nargs = 0, scan = args; *scan; nargs++)
		{
			while (*scan && !isspace(*scan))
			{
				scan++;
			}
			while (isspace(*scan))
			{
				scan++;
			}
		}
		msg("%d files to edit", nargs);
		exrefresh();
	}
}


void cmd_cd(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	char	*getenv();

	/* default directory name is $HOME */
	if (!*extra)
	{
		extra = getenv("HOME");
		if (!extra)
		{
			msg("environment variable $HOME not set");
			return;
		}
	}

	/* go to the directory */
	if (chdir(extra) < 0)
	{
		perror(extra);
	}
}


void cmd_map(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	char	*mapto;

	/* "map" with no extra will dump the map table contents */
	if (!*extra)
	{
		dumpkey();
	}
	else
	{
		/* "extra" is key to map, followed my what it maps to */
		for (mapto = extra; *mapto && *mapto != ' ' && *mapto!= '\t'; mapto++)
		{
		}
		while (*mapto == ' ' || *mapto == '\t')
		{
			*mapto++ = '\0';
		}

		mapkey(extra, mapto, bang ? WHEN_VICMD|WHEN_VIINP|WHEN_EX : WHEN_VICMD, (char *)0);
	}
}


void cmd_set(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	if (!*extra)
	{
		dumpopts(FALSE);/* "FALSE" means "don't dump all" - only set */
	}
	else if (!strcmp(extra, "all"))
	{
		dumpopts(TRUE);	/* "TRUE" means "dump all" - even unset vars */
	}
	else
	{
		setopts(extra);
	}
}


void cmd_tag(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	char	*scan;	/* used to scan through the tmpblk.c */
	char	*cmp;	/* char of tag name we're comparing, or NULL */
	char	*end;	/* marks the end of chars in tmpblk.c */
	int	fd;	/* file descriptor used to read the file */
	char	wasmagic; /* preserves the original state of o_magic */

	/* open the tags file */
	fd = open(TAGS, O_RDONLY);
	if (fd < 0)
	{
		msg("No tags file");
		return;
	}

	/* Hmmm... this would have been a lot easier with <stdio.h> */

	/* find the line with our tag in it */
	for(scan = end = tmpblk.c, cmp = extra; ; scan++)
	{
		/* read a block, if necessary */
		if (scan >= end)
		{
			end = tmpblk.c + read(fd, tmpblk.c, BLKSIZE);
			scan = tmpblk.c;
			if (scan >= end)
			{
				msg("tag \"%s\" not found", extra);
				close(fd);
				return;
			}
		}

		/* if we're comparing, compare... */
		if (cmp)
		{
			/* matched??? wow! */
			if (!*cmp && *scan == '\t')
			{
				break;
			}
			if (*cmp++ != *scan)
			{
				/* failed! skip to newline */
				cmp = (char *)0;
			}
		}

		/* if we're skipping to newline, do it fast! */
		if (!cmp)
		{
			while (scan < end && *scan != '\n')
			{
				scan++;
			}
			if (scan < end)
			{
				cmp = extra;
			}
		}
	}

	/* found it! get the rest of the line into memory */
	for (cmp = tmpblk.c, scan++; scan < end && *scan != '\n'; )
	{
		*cmp++ = *scan++;
	}
	if (scan == end)
	{
		read(fd, cmp, BLKSIZE - (cmp - tmpblk.c));
	}

	/* we can close the tags file now */
	close(fd);

	/* extract the filename from the line, and edit the file */
	for (cmp = tmpblk.c; *cmp != '\t'; cmp++)
	{
	}
	*cmp++ = '\0';
	if (strcmp(origname, tmpblk.c) != 0)
	{
		if (!tmpabort(bang))
		{
			addstr("Use tag! to abort changes, or w to save changes\n");
			return;
		}
		tmpstart(tmpblk.c);
	}

	/* move to the desired line (or to line 1 if that fails) */
	wasmagic = *o_magic;
	*o_magic = FALSE;
	linespec(cmp, &cursor);
	if (cursor == MARK_UNSET)
	{
		cursor = MARK_FIRST;
	}
	*o_magic = wasmagic;
}


void cmd_visual(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	mode = MODE_VI;
}

void cmd_quit(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	if (tmpabort(bang))
	{
		mode = MODE_QUIT;
	}
	else
	{
		addstr("Use q! to abort changes, or wq to save changes\n");
	}
}


void cmd_rewind(frommark, tomark, cmd, bang, extra)
	MARK	frommark, tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	/* switch to the first file */
	if (tmpabort(bang))
	{
		argno = -1;
		cmd_next(frommark, tomark, CMD_NEXT, bang, extra);
	}
	else
	{
		addstr("Use rewind! to abort changes, or w to save changes\n");
	}
}



/* describe this version of the program */
void cmd_version(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
#ifndef DATE
	addstr(VERSION);
	addch('\n');
#else
	wprintw(stdscr, "%s  (%s)\n", VERSION, DATE);
#endif
#ifdef COPYING
	addch('\n');
	addstr(COPYING);
	addch('\n');
#endif
}


/* make a .exrc file which describes the current configuration */
void cmd_mkexrc(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	int	fd;

	/* create the .exrc file */
	fd = creat(EXRC, 0666);
	if (fd < 0)
	{
		addstr("Couldn't create .exrc file\n");
		return;
	}

	/* save stuff */
	savekeys(fd);
	saveopts(fd);

	/* close the file */
	close(fd);
	addstr("Created new .exrc file\n");
}
