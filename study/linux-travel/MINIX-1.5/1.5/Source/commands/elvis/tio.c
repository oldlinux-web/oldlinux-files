/* tio.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains terminal I/O functions */

#include <sys/types.h>
#include <signal.h>
#include "vi.h"


/* This function reads in a line from the terminal. */
int vgets(prompt, buf, bsize)
	char	prompt;	/* the prompt character, or '\0' for none */
	char	*buf;	/* buffer into which the string is read */
	int	bsize;	/* size of the buffer */
{
	int	len;	/* how much we've read so far */
	int	ch;	/* a character from the user */
	int	quoted;	/* is the next char quoted? */
	int	tab;	/* column position of cursor */
	char	widths[132];	/* widths of characters */

	/* show the prompt */
	move(LINES - 1, 0);
	tab = 0;
	if (prompt)
	{
		addch(prompt);
		tab = 1;
	}
	clrtoeol();
	refresh();

	/* read in the line */
	quoted = len = 0;
	for (;;)
	{
		ch = getkey(quoted ? 0 : WHEN_EX);

		/* some special conversions */
		if (ch == ctrl('D') && len == 0)
			ch = ctrl('[');

		/* inhibit detection of special chars (except ^J) after a ^V */
		if (quoted && ch != '\n')
		{
			ch |= 256;
		}

		/* process the character */
		switch(ch)
		{
		  case ctrl('V'):
			qaddch('^');
			qaddch('\b');
			quoted = TRUE;
			break;

		  case ctrl('['):
			return -1;

		  case '\n':
		  case '\r':
			clrtoeol();
			goto BreakBreak;

		  case '\b':
			if (len > 0)
			{
				len--;
				addstr("\b\b\b\b\b\b\b\b" + 8 - widths[len]);
				if (mode == MODE_EX)
				{
					clrtoeol();
				}
				tab -= widths[len];
			}
			else
			{
				return -1;
			}
			break;

		  default:
			/* strip off quotation bit */
			if (ch & 256)
			{
				ch &= ~256;
				quoted = FALSE;
				qaddch(' ');
				qaddch('\b');
			}
			/* add & echo the char */
			if (len < bsize - 1)
			{
				if (ch == '\t')
				{
					widths[len] = *o_tabstop - (tab % *o_tabstop);
					addstr("        " + 8 - widths[len]);
					tab += widths[len];
				}
				else if (ch < ' ')
				{
					addch('^');
					addch(ch + '@');
					widths[len] = 2;
					tab += 2;
				}
				else if (ch == '\177')
				{
					addch('^');
					addch('?');
					widths[len] = 2;
					tab += 2;
				}
				else
				{
					addch(ch);
					widths[len] = 1;
					tab++;
				}
				buf[len++] = ch;
			}
			else
			{
				beep();
			}
		}
	}
BreakBreak:
	refresh();
	buf[len] = '\0';
	return len;
}


/* ring the terminal's bell */
beep()
{
	if (*o_vbell)
	{
		tputs(VB, 1, faddch);
		refresh();
	}
	else
	{
		write(1, "\007", 1);
	}
}

static manymsgs; /* This variable keeps msgs from overwriting each other */

/* Write a message in an appropriate way.  This should really be a varargs
 * function, but there is no such thing as vwprintw.  Hack!!!  Also uses a
 * little sleaze in the way it saves messages for repetition later.
 *
 * msg((char *)0)	- repeats the previous message
 * msg("")		- clears the message line
 * msg("%s %d", ...)	- does a printf onto the message line
 */
msg(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
	char	*fmt;
	long	arg1, arg2, arg3, arg4, arg5, arg6, arg7;
{
	static char	pmsg[80];	/* previous message */
	char		*start;		/* start of current message */

	if (mode != MODE_VI)
	{
		wprintw(stdscr, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
		addch('\n');
		exrefresh();
	}
	else
	{
		/* redrawing previous message? */
		if (!fmt)
		{
			move(LINES - 1, 0);
			standout();
			qaddch(' ');
			addstr(pmsg);
			qaddch(' ');
			standend();
			clrtoeol();
			return;
		}

		/* just blanking out message line? */
		if (!*fmt)
		{
			if (!*pmsg) return;
			*pmsg = '\0';
			move(LINES - 1, 0);
			clrtoeol();
			return;
		}

		/* wait for keypress between consecutive msgs */
		if (manymsgs)
		{
			qaddstr("[More...]");
			wqrefresh(stdscr);
			getkey(0);
		}

		/* real message */
		move(LINES - 1, 0);
		standout();
		qaddch(' ');
		start = stdscr;
		wprintw(stdscr, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
		strcpy(pmsg, start);
		qaddch(' ');
		standend();
		clrtoeol();
		refresh();
	}
	manymsgs = TRUE;
}


/* This function calls refresh() if the option exrefresh is set */
exrefresh()
{
	/* If this ex command wrote ANYTHING set exwrote so vi's  :  command
	 * can tell that it must wait for a user keystroke before redrawing.
	 */
	if (stdscr > kbuf)
	{
		exwrote = TRUE;
	}

	/* now we do the refresh thing */
	if (*o_exrefresh)
	{
		refresh();
	}
	else
	{
		wqrefresh(stdscr);
	}
	manymsgs = FALSE;
}


/* This variable holds a single ungotten key, or 0 for no key */
static int ungotten;
ungetkey(key)
	int	key;
{
	ungotten = key;
}

/* This array describes mapped key sequences */
static struct _keymap
{
	char	*name;		/* name of the key, or NULL */
	char	rawin[LONGKEY];	/* the unmapped version of input */
	char	cooked[80];	/* the mapped version of input */
	int	len;		/* length of the unmapped version */
	int	when;		/* when is this key mapped? */
}
	mapped[MAXMAPS];

/* This function reads in a keystroke for VI mode.  It automatically handles
 * key mapping.
 */
static int dummy(){} /* for timeout */
int getkey(when)
	int		when;		/* which bits must be ON? */
{
	static char	keybuf[100];	/* array of already-read keys */
	static int	nkeys;		/* total number of keys in keybuf */
	static int	next;		/* index of next key to return */
	static char	*cooked;	/* rawin, or pointer to converted key */ 
	register char	*kptr;		/* &keybuf[next] */
	register struct _keymap *km;	/* used to count through keymap */
	register int	i, j, k;

	/* if this key is needed for delay between multiple error messages,
	 * then reset the manymsgs flag and abort any mapped key sequence.
	 */
	if (manymsgs)
	{
		manymsgs = FALSE;
		cooked = (char *)0;
		ungotten = 0;
	}

	/* if we have an ungotten key, use it */
	if (ungotten != 0)
	{
		k = ungotten;
		ungotten = 0;
		return k;
	}

	/* if we're doing a mapped key, get the next char */
	if (cooked && *cooked)
	{
		return *cooked++;
	}

	/* if keybuf is empty, fill it */
	if (next == nkeys)
	{
		/* redraw if getting a VI command, refresh always */
		if (when & WHEN_VICMD)
		{
			redraw(cursor, FALSE);
		}
		refresh();

		/* read the rawin keystrokes */
		while ((nkeys = read(0, keybuf, sizeof keybuf)) < 0)
		{
			/* terminal was probably resized */
			*o_lines = LINES;
			*o_columns = COLS;
			if (when & (WHEN_VICMD|WHEN_VIINP|WHEN_VIREP))
			{
				redraw(MARK_UNSET, FALSE);
				redraw(cursor, (when & WHEN_VICMD) == 0);
				refresh();
			}
		}
		next = 0;
	}

	/* see how many mapped keys this might be */
	kptr = &keybuf[next];
	for (i = j = 0, k = -1, km = mapped; i < MAXMAPS; i++, km++)
	{
		if ((km->when & when) && km->len > 0 && *km->rawin == *kptr)
		{
			if (km->len > nkeys - next
			 && !strncmp(km->rawin, kptr, nkeys - next))
			{
				j++;
			}
			else if (km->len <= nkeys - next
			 && !strncmp(km->rawin, kptr, km->len))
			{
				j++;
				k = i;
			}
		}
	}

	/* if more than one, try to read some more */
	if (j > 1 && *o_keytime > 0)
	{
		signal(SIGALRM, dummy);
		alarm((unsigned)*o_keytime);
		k = read(0, keybuf + nkeys, sizeof keybuf - nkeys);
		alarm(0);

		/* if we couldn't read any more, pretend 0 mapped keys */
		if (k < 1)
		{
			j = 0;
		}
		else /* else we got some more - try again */
		{
			nkeys += k;
			for (i = j = 0, km = mapped; i < MAXMAPS; i++, km++)
			{
				if ((km->when & when)
				 && km->len <= nkeys - next
				 && *km->rawin == *kptr
				 && !strncmp(km->rawin, kptr, km->len))
				{
					j++;
					k = i;
				}
			}
		}
	}

	/* if unambiguously mapped key, use it! */
	if (j == 1 && k >= 0)
	{
		next += mapped[k].len;
		cooked = mapped[k].cooked;
		return *cooked++;
	}
	else
	/* assume key is unmapped, but still translate weird erase key to '\b' */
	if (keybuf[next] == ERASEKEY && when != 0)
	{
		next++;
		return '\b';
	}
	else
	{
		return keybuf[next++];
	}
}


mapkey(rawin, cooked, when, name)
	char	*rawin;	/* the input key sequence, before mapping */
	char	*cooked;/* after mapping */
	short	when;	/* bitmap of when mapping should happen */
	char	*name;	/* name of the key, if any */
{
	int	i, j;

	/* see if the key sequence was mapped before */
	j = strlen(rawin);
	for (i = 0; i < MAXMAPS; i++)
	{
		if (mapped[i].len == j && !strncmp(mapped[i].rawin, rawin, j))
		{
			break;
		}
	}

	/* if not, then try to find a new slot to use */
	if (i == MAXMAPS)
	{
		for (i = 0; i < MAXMAPS && mapped[i].len > 0; i++)
		{
		}
	}

	/* no room for the new key? */
	if (i == MAXMAPS)
	{
		msg("No room left in the key map table");
		return;
	}

	if (cooked && *cooked)
	{
		/* Map the key */
		mapped[i].len = j;
		strncpy(mapped[i].rawin, rawin, j);
		strcpy(mapped[i].cooked, cooked);
		mapped[i].when = when;
		mapped[i].name = name;
	}
	else
	{
		mapped[i].len = 0;
	}
}


dumpkey()
{
	int	i;
	char	*scan;

	for (i = 0; i < MAXMAPS; i++)
	{
		/* skip unused entries */
		if (mapped[i].len <= 0)
		{
			continue;
		}

		/* dump the key label, if any */
		if (mapped[i].name)
		{
			qaddstr(mapped[i].name);
		}
		qaddch('\t');

		/* write a '!' if defined with map! */
		if (mapped[i].when & (WHEN_EX|WHEN_VIINP))
		{
			qaddch('!');
		}
		else
		{
			qaddch(' ');
		}
		qaddch(' ');

		/* dump the raw version */
		for (scan = mapped[i].rawin; scan < mapped[i].rawin + mapped[i].len; scan++)
		{
			if (*scan >= 0 && *scan < ' ' || *scan == '\177')
			{
				qaddch('^');
				qaddch(*scan ^ '@');
			}
			else
			{
				qaddch(*scan);
			}
		}

		qaddch('\t');

		/* dump the mapped version */
		for (scan = mapped[i].cooked; *scan; scan++)
		{
			if (*scan >= 0 && *scan < ' ' || *scan == '\177')
			{
				qaddch('^');
				qaddch(*scan ^ '@');
			}
			else
			{
				qaddch(*scan);
			}
		}

		addch('\n');
	}
	exrefresh();
}



/* This function saves the current configuration of mapped keys to a file */
savekeys(fd)
	int	fd;	/* file descriptor to save them to */
{
	int	i;

	/* HACK! refresh the screen now, so the output buffer is empty. */
	refresh();

	/* now write a map command for each key other thna the arrows */
	for (i = 0; i < MAXMAPS; i++)
	{
		/* ignore keys mapped from termcap */
		if (mapped[i].name)
		{
			continue;
		}

		/* If this isn't used, ignore it */
		if (mapped[i].len <= 0)
		{
			continue;
		}

		/* write the map command */
		wprintw(stdscr, "map%c %.*s %s",
			(mapped[i].when & (WHEN_EX|WHEN_VIINP)) ? '!' : ' ',
			mapped[i].len, mapped[i].rawin,
			mapped[i].cooked);
		qaddch('\n');
	}

	/* now write the buffer to the file */
	if (stdscr != kbuf)
	{
		write(fd, kbuf, (stdscr - kbuf));
		stdscr = kbuf;
	}
}
