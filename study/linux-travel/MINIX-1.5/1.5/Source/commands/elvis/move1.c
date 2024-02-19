/* move1.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains most movement functions */

#include <ctype.h>
#include "vi.h"

#ifndef isascii
# define isascii(c)	!((c) & ~0x7f)
#endif

MARK	moveup(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	DEFAULT(1);

	/* if at top already, don't move */
	if (markline(m) - cnt < 1)
	{
		return MARK_UNSET;
	}

	/* else move up one line */
	m -= MARK_AT_LINE(cnt);

	return m;
}

MARK	movedown(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	DEFAULT(1);

	/* if at bottom already, don't move */
	if (markline(m) + cnt > nlines)
	{
		return MARK_UNSET;
	}

	/* else move down one line */
	m += MARK_AT_LINE(cnt);

	/* adjust column number */
	if (markidx(m) >= plen)
	{
		m = (m & ~(BLKSIZE - 1));
		if (plen > 0)
		{
			m += plen - 1;
		}
	}

	return m;
}

MARK	moveright(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	int		idx;	/* index of the new cursor position */

	DEFAULT(1);

	/* move to right, if that's OK */
	pfetch(markline(m));
	idx = markidx(m) + cnt;
	if (idx < plen)
	{
		m += cnt;
	}
	else
	{
		return MARK_UNSET;
	}

	return m;
}

MARK	moveleft(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	DEFAULT(1);

	/* move to the left, if that's OK */
	if (markidx(m) >= cnt)
	{
		m -= cnt;
	}
	else
	{
		return MARK_UNSET;
	}

	return m;
}

MARK	movetoline(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric line number */
{
	/* if no number specified, assume last line */
	DEFAULT(nlines);

	/* if invalid line number, don't move */
	if (cnt > nlines)
	{
		msg("Line numbers range from 1 to %ld", nlines);
		return MARK_UNSET;
	}

	/* move to first character of the selected line */
	m = MARK_AT_LINE(cnt);
	return m;
}

MARK	movetocol(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	char	*text;	/* text of the line */
	int	col;	/* column number */
	int	idx;	/* index into the line */

	DEFAULT(1);

	/* internally, columns are numbered 0..COLS-1, not 1..COLS */
	cnt--;

	/* if 0, that's easy */
	if (cnt == 0)
	{
		m &= ~(BLKSIZE - 1);
		return m;
	}

	/* find that column within the line */
	pfetch(markline(m));
	text = ptext;
	for (col = idx = 0; col < cnt && *text; text++, idx++)
	{
		if (*text == '\t')
		{
			col += *o_tabstop;
			col -= col % *o_tabstop;
		}
		else if (*text >= '\0' && *text < ' ' || *text == '\177')
		{
			col += 2;
		}
#ifndef SET_NOCHARATTR
		else if (text[0] == '\\' && text[1] == 'f' && text[2] && *o_charattr)
		{
			text += 2; /* plus one more as part of for loop */
		}
#endif
		else
		{
			col++;
		}
	}
	if (!*text)
	{
		return MARK_UNSET;
	}
	else
	{
		m = (m & ~(BLKSIZE - 1)) + idx;
	}
	return m;
}

MARK	movefront(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument (ignored) */
{
	char	*scan;

	/* move to the first non-whitespace character */
	pfetch(markline(m));
	scan = ptext;
	m &= ~(BLKSIZE - 1);
	while (*scan == ' ' || *scan == '\t')
	{
		scan++;
		m++;
	}

	return m;
}

MARK	moverear(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument (ignored) */
{
	/* Try to move *EXTREMELY* far to the right.  It is fervently hoped
	 * that other code will convert this to a more reasonable MARK before
	 * anything tries to actually use it.  (See adjmove() in vi.c)
	 */
	return m | (BLKSIZE - 1);
}

MARK	movefword(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	register long	l;
	register char	*text;
	register int	i;

	DEFAULT(1);

	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);
	while (cnt-- > 0) /* yes, ASSIGNMENT! */
	{
		i = *text++;
		/* if we hit the end of the line, continue with next line */
		if (!isascii(i) || isalnum(i) || i == '_')
		{
			/* include an alphanumeric word */
			while (i && (!isascii(i) || isalnum(i) || i == '_'))
			{
				i = *text++;
			}
		}
		else
		{
			/* include contiguous punctuation */
			while (i && isascii(i) && !isalnum(i) && !isspace(i))
			{
				i = *text++;
			}
		}

		/* include trailing whitespace */
		while (!i || isascii(i) && isspace(i))
		{
			/* did we hit the end of this line? */
			if (!i)
			{
				/* move to next line, if there is one */
				l++;
				if (l > nlines)
				{
					return MARK_UNSET;
				}
				pfetch(l);
				text = ptext;
			}

			i = *text++;
		}
		text--;
	}

	/* construct a MARK for this place */
	m = buildmark(text);
	return m;
}


MARK	movebword(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	register long	l;
	register char	*text;

	DEFAULT(1);

	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);
	while (cnt-- > 0) /* yes, ASSIGNMENT! */
	{
		text--;

		/* include preceding whitespace */
		while (text < ptext || isascii(*text) && isspace(*text))
		{
			/* did we hit the end of this line? */
			if (text < ptext)
			{
				/* move to preceding line, if there is one */
				l--;
				if (l <= 0)
				{
					return MARK_UNSET;
				}
				pfetch(l);
				text = ptext + plen - 1;
			}
			else
			{
				text--;
			}
		}

		if (!isascii(*text) || isalnum(*text) || *text == '_')
		{
			/* include an alphanumeric word */
			while (text >= ptext && (!isascii(*text) || isalnum(*text) || *text == '_'))
			{
				text--;
			}
		}
		else
		{
			/* include contiguous punctuation */
			while (text >= ptext && isascii(*text) && !isalnum(*text) && !isspace(*text))
			{
				text--;
			}
		}
		text++;
	}

	/* construct a MARK for this place */
	m = buildmark(text);
	return m;
}

MARK	moveeword(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	register long	l;
	register char	*text;
	register int	i;

	DEFAULT(1);

	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);
	while (cnt-- > 0) /* yes, ASSIGNMENT! */
	{
		text++;
		i = *text++;

		/* include preceding whitespace */
		while (!i || isascii(i) && isspace(i))
		{
			/* did we hit the end of this line? */
			if (!i)
			{
				/* move to next line, if there is one */
				l++;
				if (l > nlines)
				{
					return MARK_UNSET;
				}
				pfetch(l);
				text = ptext;
			}

			i = *text++;
		}

		if (!isascii(i) || isalnum(i) || i == '_')
		{
			/* include an alphanumeric word */
			while (i && (!isascii(i) || isalnum(i) || i == '_'))
			{
				i = *text++;
			}
		}
		else
		{
			/* include contiguous punctuation */
			while (i && isascii(i) && !isalnum(i) && !isspace(i))
			{
				i = *text++;
			}
		}
		text -= 2;
	}

	/* construct a MARK for this place */
	m = buildmark(text);
	return m;
}

MARK	movefWord(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	register long	l;
	register char	*text;
	register int	i;

	DEFAULT(1);

	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);
	while (cnt-- > 0) /* yes, ASSIGNMENT! */
	{
		i = *text++;
		/* if we hit the end of the line, continue with next line */
		/* include contiguous non-space characters */
		while (i && !isspace(i))
		{
			i = *text++;
		}

		/* include trailing whitespace */
		while (!i || isascii(i) && isspace(i))
		{
			/* did we hit the end of this line? */
			if (!i)
			{
				/* move to next line, if there is one */
				l++;
				if (l > nlines)
				{
					return MARK_UNSET;
				}
				pfetch(l);
				text = ptext;
			}

			i = *text++;
		}
		text--;
	}

	/* construct a MARK for this place */
	m = buildmark(text);
	return m;
}


MARK	movebWord(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	register long	l;
	register char	*text;

	DEFAULT(1);

	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);
	while (cnt-- > 0) /* yes, ASSIGNMENT! */
	{
		text--;

		/* include trailing whitespace */
		while (text < ptext || isascii(*text) && isspace(*text))
		{
			/* did we hit the end of this line? */
			if (text < ptext)
			{
				/* move to next line, if there is one */
				l--;
				if (l <= 0)
				{
					return MARK_UNSET;
				}
				pfetch(l);
				text = ptext + plen - 1;
			}
			else
			{
				text--;
			}
		}

		/* include contiguous non-whitespace */
		while (text >= ptext && (!isascii(*text) || !isspace(*text)))
		{
			text--;
		}
		text++;
	}

	/* construct a MARK for this place */
	m = buildmark(text);
	return m;
}

MARK	moveeWord(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	register long	l;
	register char	*text;
	register int	i;

	DEFAULT(1);

	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);
	while (cnt-- > 0) /* yes, ASSIGNMENT! */
	{
		text++;
		i = *text++;

		/* include preceding whitespace */
		while (!i || isascii(i) && isspace(i))
		{
			/* did we hit the end of this line? */
			if (!i)
			{
				/* move to next line, if there is one */
				l++;
				if (l > nlines)
				{
					return MARK_UNSET;
				}
				pfetch(l);
				text = ptext;
			}

			i = *text++;
		}

		/* include contiguous non-whitespace */
		while (i && (!isascii(i) || !isspace(i)))
		{
			i = *text++;
		}
		text -= 2;
	}

	/* construct a MARK for this place */
	m = buildmark(text);
	return m;
}

MARK	movefsentence(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	register char	*text;
	register long	l;

	DEFAULT(1);

	/* get the current line */
	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);

	/* for each requested sentence... */
	while (cnt-- > 0)
	{
		/* search forward for one of [.?!] followed by spaces or EOL */
		do
		{
			/* wrap at end of line */
			if (!text[0])
			{
				if (l >= nlines)
				{
					return MARK_UNSET;
				}
				l++;
				pfetch(l);
				text = ptext;
			}
			else
			{
				text++;
			}
		} while (text[0] != '.' && text[0] != '?' && text[0] != '!'
			|| text[1] && (text[1] != ' ' || text[2] && text[2] != ' '));
	}

	/* construct a mark for this location */
	m = buildmark(text);

	/* move forward to the first word of the next sentence */
	m = movefword(m, 1L);

	return m;
}

MARK	movebsentence(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	register char	*text;	/* used to scan thru text */
	register long	l;	/* current line number */
	int		flag;	/* have we passed at least one word? */

	DEFAULT(1);

	/* get the current line */
	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);

	/* for each requested sentence... */
	flag = TRUE;
	while (cnt-- > 0)
	{
		/* search backward for one of [.?!] followed by spaces or EOL */
		do
		{
			/* wrap at beginning of line */
			if (text == ptext)
			{
				do
				{
					if (l <= 1)
					{
						return MARK_UNSET;
					}
					l--;
					pfetch(l);
				} while (!*ptext);
				text = ptext + plen - 1;
			}
			else
			{
				text--;
			}

			/* are we moving past a "word"? */
			if (text[0] >= '0')
			{
				flag = FALSE;
			}
		} while (flag || text[0] != '.' && text[0] != '?' && text[0] != '!'
			|| text[1] && (text[1] != ' ' || text[2] && text[2] != ' '));
	}

	/* construct a mark for this location */
	m = buildmark(text);

	/* move to the front of the following sentence */
	m = movefword(m, 1L);

	return m;
}

MARK	movefparagraph(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	char	*text;
	char	*pscn;	/* used to scan thru value of "paragraphs" option */
	long	l;

	DEFAULT(1);

	for (l = markline(m); cnt > 0 && l++ < nlines; )
	{
		text = fetchline(l);
		if (!*text)
		{
			cnt--;
		}
		else if (*text == '.')
		{
			for (pscn = o_paragraphs; pscn[0] && pscn[1]; pscn += 2)
			{
				if (pscn[0] == text[1] && pscn[1] == text[2])
				{
					cnt--;
					break;
				}
			}
		}
	}
	if (l <= nlines)
	{
		m = MARK_AT_LINE(l);
	}
	else
	{
		m = MARK_LAST;
	}
	return m;
}

MARK	movebparagraph(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	char	*text;
	char	*pscn;	/* used to scan thru value of "paragraph" option */
	long	l;

	DEFAULT(1);

	for (l = markline(m); cnt > 0 && l-- > 1; )
	{
		text = fetchline(l);
		if (!*text)
		{
			cnt--;
		}
		else if (*text == '.')
		{
			for (pscn = o_paragraphs; pscn[0] && pscn[1]; pscn += 2)
			{
				if (pscn[0] == text[1] && pscn[1] == text[2])
				{
					cnt--;
					break;
				}
			}
		}
	}
	if (l >= 1)
	{
		m = MARK_AT_LINE(l);
	}
	else
	{
		m = MARK_FIRST;
	}
	return m;
}

MARK	movefsection(m, cnt, key)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* (ignored) */
	int	key;	/* second key stroke - must be ']' */
{
	char	*text;
	char	*sscn;	/* used to scan thru value of "sections" option */
	long	l;

	/* make sure second key was ']' */
	if (key != ']')
	{
		return MARK_UNSET;
	}

	for (l = markline(m); l++ < nlines; )
	{
		text = fetchline(l);
		if (*text == '{')
		{
			break;
		}
		if (*text == '.')
		{
			for (sscn = o_sections; sscn[0] && sscn[1]; sscn += 2)
			{
				if (sscn[0] == text[1] && sscn[1] == text[2])
				{
					goto BreakBreak;
				}
			}
		}
	}
BreakBreak:
	if (l <= nlines)
	{
		m = MARK_AT_LINE(l);
	}
	else
	{
		m = MARK_LAST;
	}
	return m;
}

MARK	movebsection(m, cnt, key)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* (ignored) */
	int	key;	/* second key stroke - must be '[' */
{
	char	*text;
	char	*sscn;	/* used to scan thru value of "sections" option */
	long	l;

	/* make sure second key was '[' */
	if (key != '[')
	{
		return MARK_UNSET;
	}

	for (l = markline(m); l-- > 1; )
	{
		text = fetchline(l);
		if (*text == '{')
		{
			break;
		}
		if (*text == '.')
		{
			for (sscn = o_sections; sscn[0] && sscn[1]; sscn += 2)
			{
				if (sscn[0] == text[1] && sscn[1] == text[2])
				{
					goto BreakBreak;
				}
			}
		}
	}
BreakBreak:
	if (l >= 1)
	{
		m = MARK_AT_LINE(l);
	}
	else
	{
		m = MARK_FIRST;
	}
	return m;
}


MARK	movematch(m, cnt)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* a numeric argument */
{
	long	l;
	register char	*text;
	register char	match;
	register char	nest;
	register int	count;

	/* get the current line */
	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);

	/* search forward within line for one of "[](){}" */
	for (match = '\0'; !match && *text; text++)
	{
		/* tricky way to recognize 'em in ASCII */
		nest = *text;
		if ((nest & 0xdf) == ']' || (nest & 0xdf) == '[')
		{
			match = nest ^ ('[' ^ ']');
		}
		else if ((nest & 0xfe) == '(')
		{
			match = nest ^ ('(' ^ ')');
		}
		else
		{
			match = 0;
		}
	}
	if (!match)
	{
		return MARK_UNSET;
	}
	text--;

	/* search forward or backward for match */
	if (match == '(' || match == '[' || match == '{')
	{
		/* search backward */
		for (count = 1; count > 0; )
		{
			/* wrap at beginning of line */
			if (text == ptext)
			{
				do
				{
					if (l <= 1L)
					{
						return MARK_UNSET;
					}
					l--;
					pfetch(l);
				} while (!*ptext);
				text = ptext + plen - 1;
			}
			else
			{
				text--;
			}

			/* check the char */
			if (*text == match)
				count--;
			else if (*text == nest)
				count++;
		}
	}
	else
	{
		/* search forward */
		for (count = 1; count > 0; )
		{
			/* wrap at end of line */
			if (!*text)
			{
				if (l >= nlines)
				{
					return MARK_UNSET;
				}
				l++;
				pfetch(l);
				text = ptext;
			}
			else
			{
				text++;
			}

			/* check the char */
			if (*text == match)
				count--;
			else if (*text == nest)
				count++;
		}
	}

	/* construct a mark for this place */
	m = buildmark(text);
	return m;
}

MARK	movetomark(m, cnt, key)
	MARK	m;	/* movement is relative to this mark */
	long	cnt;	/* (ignored) */
	int	key;	/* keystroke - the mark to move to */
{
	/* mark '' is a special case */
	if (key == '\'' || key == '`')
	{
		if (mark[26] == MARK_UNSET)
		{
			return MARK_FIRST;
		}
		else
		{
			return mark[26];
		}
	}

	/* if not a valid mark number, don't move */
	if (key < 'a' || key > 'z')
	{
		return MARK_UNSET;
	}

	/* return the selected mark -- may be MARK_UNSET */
	if (!mark[key - 'a'])
	{
		msg("mark '%c is unset", key);
	}
	return mark[key - 'a'];
}

