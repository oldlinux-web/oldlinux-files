/* input.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains the input() function, which implements vi's INPUT mode */

#include "vi.h"


/* This function allows the user to replace an existing (possibly zero-length)
 * chunk of text with typed-in text.  It returns the MARK of the last character
 * that the user typed in.
 */
MARK input(from, to, when)
	MARK	from;	/* where to start inserting text */
	MARK	to;	/* extent of text to delete */
	int	when;	/* either WHEN_VIINP or WHEN_VIREP */
{
	char	key[2];	/* key char followed by '\0' char */
	char	*build;	/* used in building a newline+indent string */
	char	*scan;	/* used while looking at the indent chars of a line */
	MARK	m;

#ifdef DEBUG
	/* if "from" and "to" are reversed, complain */
	if (from > to)
	{
		msg("ERROR: input(%ld:%d, %ld:%d)",
			markline(from), markidx(from),
			markline(to), markidx(to));
		return MARK_UNSET;
	}
#endif

	key[1] = 0;

	/* if we're replacing text with new text, save the old stuff */
	/* (Alas, there is no easy way to save text for replace mode) */
	if (from != to)
	{
		cut(from, to);
	}

	ChangeText
	{
		/* if doing a dot command, then reuse the previous text */
		if (doingdot)
		{
			/* delete the text thats there now */
			if (from != to)
			{
				delete(from, to);
			}

			cutname('.');
			cursor = paste(from, FALSE, TRUE) + 1L;
		}
		else
		{
			/* if doing a change within the line... */
			if (from != to && markline(from) == markline(to))
			{
				/* mark the end of the text with a "$" */
				change(to - 1, to, "$");
			}
			else
			{
				/* delete the old text right off */
				if (from != to)
				{
					delete(from, to);
				}
				to = from;
			}

			/* handle autoindent of the first line, maybe */
			cursor = from;
			if (*o_autoindent && markline(cursor) > 1L && markidx(cursor) == 0)
			{
				/* Only autoindent blank lines. */
				pfetch(markline(cursor));
				if (plen == 0)
				{
					/* Okay, we really want to autoindent */
					pfetch(markline(cursor) - 1L);
					for (scan = ptext, build = tmpblk.c;
					     *scan == ' ' || *scan == '\t';
					     )
					{
						*build++ = *scan++;
					}
					if (build > tmpblk.c)
					{
						*build = '\0';
						add(cursor, tmpblk.c);
						cursor += (build - tmpblk.c);
					}
				}
			}

			/* repeatedly add characters from the user */
			for (;;)
			{
				/* Get a character */
				redraw(cursor, TRUE);
				key[0] = getkey(when);

				/* if whitespace & wrapmargin is set & we're
				/* past the warpmargin, then change the
				/* whitespace character into a newline
				 */
				if ((*key == ' ' || *key == '\t')
				 && *o_wrapmargin != 0)
				{
					pfetch(markline(cursor));
					if (idx2col(cursor, ptext, TRUE) > COLS - (*o_wrapmargin & 0xff))
					{
						*key = '\n';
					}
				}

				/* process it */
				switch (*key)
				{
				  case ctrl('['):
					goto BreakBreak;

				  case ctrl('U'):
					if (markline(cursor) == markline(from))
					{
						cursor == from;
					}
					else
					{
						cursor &= ~(BLKSIZE - 1);
					}
					break;

				  case '\b':
				  case ctrl('D'):
					if (cursor <= from)
					{
						beep();
					}
					else if (markidx(cursor) == 0)
					{
						cursor -= BLKSIZE;
						pfetch(markline(cursor));
						cursor += plen;
					}
					else
					{
						cursor--;
					}
					break;

				  case ctrl('W'):
					m = movebword(cursor, 1L);
					if (markline(m) == markline(cursor) && m >= from)
					{
						cursor = m;
						if (from > cursor)
						{
							from = cursor;
						}
					}
					else
					{
						beep();
					}
					break;

				  case '\n':
				  case '\r':
					build = tmpblk.c;
					*build++ = '\n';
					if (*o_autoindent)
					{
						pfetch(markline(cursor));
						for (scan = ptext; *scan == ' ' || *scan == '\t'; )
						{
							*build++ = *scan++;
						}
					}
					*build = 0;
					if (cursor >= to && when != WHEN_VIREP)
					{
						add(cursor, tmpblk.c);
					}
					else
					{
						change(cursor, to, tmpblk.c);
					}
					redraw(cursor, TRUE);
					to = cursor = (cursor & ~(BLKSIZE - 1))
							+ BLKSIZE
							+ (int)(build - tmpblk.c) - 1;
					break;

				  case ctrl('A'):
					if (cursor < to)
					{
						delete(cursor, to);
					}
					cutname('.');
					to = cursor = paste(cursor, FALSE, TRUE) + 1L;
					break;

				  case ctrl('V'):
					if (cursor >= to && when != WHEN_VIREP)
					{
						add(cursor, "^");
					}
					else
					{
						change(cursor, to, "^");
					}
					redraw(cursor, TRUE);
					*key = getkey(0);
					if (*key == '\n')
					{
						/* '\n' too hard to handle */
						*key = '\r';
					}
					change(cursor, cursor + 1, key);
					cursor++;
					if (cursor > to)
					{
						to = cursor;
					}
					break;

				  case ctrl('L'):
				  case ctrl('R'):
					redraw(MARK_UNSET, FALSE);
					break;

				  case ctrl('T'):
					*key = '\t';
					/* fall through to default case... */

				  default:
					if (cursor >= to && when != WHEN_VIREP)
					{
						add(cursor, key);
						cursor++;
						to = cursor;
					}
					else
					{
						pfetch(markline(cursor));
						if (markidx(cursor) == plen)
						{
							add(cursor, key);
						}
						else
						{
							change(cursor, cursor + 1, key);
						}
						cursor++;
					}
				} /* end switch(*key) */
			} /* end for(;;) */
BreakBreak:;

			/* delete any excess characters */
			if (cursor < to)
			{
				delete(cursor, to);
			}

		} /* end if doingdot else */

	} /* end ChangeText */

	/* put the new text into a cut buffer for possible reuse */
	if (!doingdot)
	{
		blksync();
		cutname('.');
		cut(from, cursor);
	}

	/* move to last char that we inputted, unless it was newline */
	if (markidx(cursor) != 0)
	{
		cursor--;
	}

	rptlines = 0L;
	return cursor;
}
