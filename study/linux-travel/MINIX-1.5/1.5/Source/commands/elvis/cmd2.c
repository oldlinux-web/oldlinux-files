/* cmd2.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains some of the commands - mostly ones that change text */

#include "vi.h"
#include "regexp.h"


void cmd_substitute(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;	/* rest of the command line */
{
	char	*line;	/* a line from the file */
	regexp	*re;	/* the compiled search expression */
	char	*subst;	/* the substitution string */
	char	*opt;	/* substitution options */
	int	optp;	/* boolean option: print when done? */
	int	optg;	/* boolean option: substitute globally in line? */
	long	l;	/* a line number */
	char	*s, *d;	/* used during subtitutions */
	long	chline;	/* # of lines changed */
	long	chsub;	/* # of substitutions made */


	/* make sure we got a search pattern */
	if (*extra != '/' && *extra != '?')
	{
		msg("Usage: s/regular expression/new text/");
		return;
	}

	/* parse & compile the search pattern */
	subst = parseptrn(extra);
	re = regcomp(extra + 1);
	if (!re)
	{
		return;
	}

	/* parse the substitution string & find the option string */
	for (opt = subst; *opt && (*opt != *extra || opt[-1] == '\\'); opt++)
	{
	}
	if (*opt)
	{
		*opt++ = '\0';
	}

	/* analyse the option string */
	optp = optg = 0;
	while (*opt)
	{
		switch (*opt++)
		{
		  case 'p':	optp = 1;	break;
		  case 'g':	optg = 1;	break;
		  case ' ':
		  case '\t':			break;
		  default:
			msg("Subst options are p and g -- not %c", opt[-1]);
			return;
		}
	}

	ChangeText
	{
		/* reset the change counters */
		chline = chsub = 0L;

		/* for each selected line */
		for (l = markline(frommark); l <= markline(tomark); l++)
		{
			/* fetch the line */
			line = fetchline(l);

			/* if it contains the search pattern... */
			if (regexec(re, line, TRUE))
			{
				/* increment the line change counter */
				chline++;

				/* initialize the pointers */
				s = line;
				d = tmpblk.c;

				/* do once or globally ... */
				do
				{
					/* increment the substitution change counter */
					chsub++;

					/* this may be the first line to redraw */
					redrawrange(l, l + 1L, l + 1L);

					/* copy stuff from before the match */
					while (s < re->startp[0])
					{
						*d++ = *s++;
					}
	
					/* subtitute for the matched part */
					regsub(re, subst, d);
					s = re->endp[0];
					d += strlen(d);

				} while (optg && regexec(re, s, FALSE));

				/* copy stuff from after the match */
				while (*d++ = *s++)	/* yes, ASSIGNMENT! */
				{
				}

				/* replace the old version of the line with the new */
				changeline(l, tmpblk.c);

				/* if supposed to print it, do so */
				if (optp)
				{
					addstr(tmpblk.c);
					addch('\n');
					exrefresh();
				}
			}
		}
	}

	/* report what happened */
	if (chsub == 0)
	{
		msg("Substitution failed");
	}

	/* tweak for redrawing */
	if (chline > 1 || redrawafter && redrawafter != markline(cursor))
	{
		mustredraw = TRUE;
	}

	/* free the regexp */
	free(re);

	/* Reporting */
	if (chline >= *o_report)
	{
		msg("%ld substitutions on %ld lines", chsub, chline);
	}
	rptlines = 0;
}




void cmd_delete(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	MARK	curs2;	/* al altered form of the cursor */

	/* choose your cut buffer */
	if (*extra == '"')
	{
		extra++;
	}
	if (*extra)
	{
		cutname(*extra);
	}

	/* make sure we're talking about whole lines here */
	frommark = frommark & ~(BLKSIZE - 1);
	tomark = (tomark & ~(BLKSIZE - 1)) + BLKSIZE;

	/* yank the lines */
	cut(frommark, tomark);

	/* if CMD_DELETE then delete the lines */
	if (cmd != CMD_YANK)
	{
		curs2 = cursor;
		ChangeText
		{
			/* delete the lines */
			delete(frommark, tomark);
		}
		if (curs2 > tomark)
		{
			cursor = curs2 - tomark + frommark;
		}
		else if (curs2 > frommark)
		{
			cursor = frommark;
		}
	}
}


void cmd_append(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	long	l;	/* line counter */

	ChangeText
	{
		/* if we're doing a change, delete the old version */
		if (cmd == CMD_CHANGE)
		{
			/* delete 'em */
			cmd_delete(frommark, tomark, cmd, bang, extra);
		}

		/* new lines start at the frommark line, or after it */
		l = markline(frommark);
		if (cmd == CMD_APPEND)
		{
 			l++;
		}

		/* get lines until no more lines, or "." line, and insert them */
		while (vgets('\0', tmpblk.c, BLKSIZE) >= 0)
		{
			addch('\n');
			if (!strcmp(tmpblk.c, "."))
			{
				break;
			}

			addline(l, tmpblk.c);
			l++;
		}
	}

	/* on the odd chance that we're calling this from vi mode ... */
	redraw(MARK_UNSET, FALSE);
}


void cmd_put(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	/* choose your cut buffer */
	if (*extra == '"')
	{
		extra++;
	}
	if (*extra)
	{
		cutname(*extra);
	}

	/* paste it */
	ChangeText
	{
		cursor = paste(frommark, !bang, FALSE);
	}
}


void cmd_join(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	long	l;
	char	*scan;
	int	len;	/* length of the new line */

	/* if only one line is specified, assume the following one joins too */
	if (markline(frommark) == nlines)
	{
		msg("Nothing to join with this line");
		return;
	}
	if (markline(frommark) == markline(tomark))
	{
		tomark = movedown(tomark, 1L);
	}

	/* get the first line */
	l = markline(frommark);
	strcpy(tmpblk.c, fetchline(l++));
	len = strlen(tmpblk.c);

	/* build the longer line */
	while (l <= markline(tomark))
	{
		/* get the next line */
		scan = fetchline(l++);

		/* remove any leading whitespace */
		while (*scan == '\t' || *scan == ' ')
		{
			scan++;
		}

		/* see if the line will fit */
		if (strlen(scan) + len + 1 > BLKSIZE)
		{
			msg("Can't join -- the resulting line would be too long");
			return;
		}

		/* catenate it, with a space in between */
		tmpblk.c[len++] = ' ';
		strcpy(tmpblk.c + len, scan);
		len += strlen(scan);
	}

	/* make the change */
	ChangeText
	{
		frommark &= ~(BLKSIZE - 1);
		tomark &= ~(BLKSIZE - 1);
		tomark += BLKSIZE;
		delete(frommark, tomark);
		addline(markline(frommark), tmpblk.c);
	}
}



void cmd_shift(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	long	l;	/* line number counter */
	int	oldidx;	/* number of chars previously used for indent */
	int	newidx;	/* number of chars in the new indent string */
	int	oldcol;	/* previous indent amount */
	int	newcol;	/* new indent amount */
	char	*text;	/* pointer to the old line's text */

	/* figure out how much of the screen we must redraw (for vi mode) */
	if (markline(frommark) != markline(tomark))
	{
		mustredraw = TRUE;
		redrawrange(markline(frommark), markline(tomark) + 1L, markline(tomark) + 1L);
	}

	ChangeText
	{
		/* for each line to shift... */
		for (l = markline(frommark); l <= markline(tomark); l++)
		{
			/* get the line - ignore empty lines */
			text = fetchline(l);
			if (!*text)
				continue;

			/* calc oldidx and oldcol */
			for (oldidx = 0, oldcol = 0;
			     text[oldidx] == ' ' || text[oldidx] == '\t';
			     oldidx++)
			{
				if (text[oldidx] == ' ')
				{
					oldcol += 1;
				}
				else
				{
					oldcol += *o_tabstop - (oldcol % *o_tabstop);
				}
			}
	
			/* calc newcol */
			if (cmd == CMD_SHIFTR)
			{
				newcol = oldcol + (*o_shiftwidth & 0xff);
			}
			else
			{
				newcol = oldcol - (*o_shiftwidth & 0xff);
				if (newcol < 0)
					newcol = 0;
			}

			/* if no change, then skip to next line */
			if (oldcol == newcol)
				continue;

			/* build a new indent string */
			newidx = 0;
			while (newcol >= *o_tabstop)
			{
				tmpblk.c[newidx++] = '\t';
				newcol -= *o_tabstop;
			}
			while (newcol > 0)
			{
				tmpblk.c[newidx++] = ' ';
				newcol--;
			}
			tmpblk.c[newidx] = '\0';
			
			/* change the old indent string into the new */
			change(MARK_AT_LINE(l), MARK_AT_LINE(l) + oldidx, tmpblk.c);
		}
	}

	/* Reporting... */
	rptlines = markline(tomark) - markline(frommark) + 1L;
	if (cmd == CMD_SHIFTR)
	{
		rptlabel = ">ed";
	}
	else
	{
		rptlabel = "<ed";
	}
}


void cmd_read(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	long	l;	/* line number counter - where new lines go */
	int	fd, rc;	/* used while reading from the file */
	char	*scan;	/* used for finding newlines */
	char	*line;	/* points to the start of a line */
	int	prevrc;	/* used to detect abnormal EOF */

	/* first line goes after the selected line */
	l = markline(frommark) + 1;

	/* open the file */
	fd = open(extra, O_RDONLY);
	if (fd < 0)
	{
		msg("Can't open \"%s\"", extra);
		return;
	}

	/* get blocks from the file, and add each line in the block */
	ChangeText
	{
		/* NOTE!  lint worried needlessly about the order of evaluation
		 * of the 'rc' expressions in the test clause of this for(;;){}
		 */
		for (prevrc = rc = 0;
		     (rc += read(fd, tmpblk.c + rc, BLKSIZE - rc)) > 0;
		     prevrc = rc)
		{
			/* if we couldn't read anything, we damn well better have \n */
			if (prevrc == rc)
			{
				if (rc == BLKSIZE)
				{
					rc--;
				}
				if (tmpblk.c[rc - 1] != '\n' || rc <= 0)
				{
					tmpblk.c[rc++] = '\n';
				}
			}

			/* for each complete line in this block, add it */
			for (line = scan = tmpblk.c; rc > 0; rc--, scan++)
			{
				if (*scan == '\n')
				{
					*scan = '\0';
					addline(l, line);
					l++;
					line = scan + 1;
				}
				else if (!*scan)
				{
					/* protect against NUL chars in file */
					*scan = 0x80;
				}
			}

			/* any extra chars are shifted to the start of the buffer */
			rc = scan - line;
			for (scan = tmpblk.c; scan < tmpblk.c + rc; )
			{
				*scan++ = *line++;
			}
		}
	}

	/* close the file */
	close(fd);

	/* Reporting... */
	rptlines = l - markline(frommark) - 1L;
	rptlabel = "read";
}


void cmd_list(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	long		l;	/* line number counter */
	register char	*scan;	/* used for moving through the line */

	for (l = markline(frommark); l <= markline(tomark); l++)
	{
		/* list the line */
		scan = fetchline(l);

		while (*scan)
		{
			/* if the char is non-printable, write it as \000 */
			if (*scan < ' ' || *scan > '~')
			{
				/* build the \000 form & write it */
				addch('\\');
				addch('0' + ((*scan >> 6) & 3));
				addch('0' + ((*scan >> 3) & 7));
				addch('0' + (*scan & 7));
			}
			else
			{
				addch(*scan);
			}
			scan++;
		}

		/* write a $ and a \n */
		addstr("$\n");
		exrefresh();
	}
}


void cmd_undo(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	undo();
}


/* print the selected lines */
void cmd_print(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	register char	*scan;
	register long	l;

	for (l = markline(frommark); l <= markline(tomark); l++)
	{
		/* get the next line */
		scan = fetchline(l);
		addstr(scan);
		addch('\n');
		exrefresh();
	}
}


/* move or copy selected lines */
void cmd_move(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	MARK	destmark;

	/* parse the destination linespec.  No defaults.  Line 0 is okay */
	destmark = cursor;
	if (!strcmp(extra, "0"))
	{
		destmark = 0L;
	}
	else if (linespec(extra, &destmark) == extra || !destmark)
	{
		msg("invalid destination address");
		return;
	}

	/* flesh the marks out to encompass whole lines */
	frommark &= ~(BLKSIZE - 1);
	tomark = (tomark & ~(BLKSIZE - 1)) + BLKSIZE;
	destmark = (destmark & ~(BLKSIZE - 1)) + BLKSIZE;

	/* make sure the destination is valid */
	if (cmd == CMD_MOVE && destmark >= frommark && destmark < tomark)
	{
		msg("invalid destination address");
	}

	/* Do it */
	ChangeText
	{
		/* save the text to a cut buffer */
		cutname('\0');
		cut(frommark, tomark);

		/* if we're not copying, delete the old text & adjust destmark */
		if (cmd != CMD_COPY)
		{
			delete(frommark, tomark);
			if (destmark >= frommark)
			{
				destmark -= (tomark - frommark);
			}
		}

		/* add the new text */
		paste(destmark, FALSE, FALSE);
	}

	/* move the cursor to the last line of the moved text */
	cursor = destmark + (tomark - frommark);

	/* Reporting... */
	rptlabel = ( (cmd == CMD_COPY) ? "copied" : "moved" );
}



/* execute EX commands from a file */
void cmd_source(frommark, tomark, cmd, bang, extra)
	MARK	frommark;
	MARK	tomark;
	CMD	cmd;
	int	bang;
	char	*extra;
{
	/* must have a filename */
	if (!*extra)
	{
		msg("\"source\" requires a filename");
		return;
	}

	doexrc(extra);
}
