/* vcmd.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains the functions that handle VI commands */


#include "vi.h"


/* This function puts the editor in EX mode */
MARK v_quit()
{
	mode = MODE_EX;
	return cursor;
}

/* This function causes the screen to be redrawn */
MARK v_redraw()
{
	redraw(MARK_UNSET, FALSE);
	return cursor;
}

/* This function executes a single EX command, and waits for a user keystroke
 * before returning to the VI screen.  If that keystroke is another ':', then
 * another EX command is read and executed.
 */
MARK v_1ex(m, text)
	MARK	m;	/* the current line */
	char	*text;	/* the first command to execute */
{
	/* scroll up, so we don't overwrite the command */
	addch('\n');
	refresh();

	/* run the command.  be careful about modes & output */
	exwrote = FALSE;
	mode = MODE_COLON;
	doexcmd(text);
	exrefresh();
	if (mode == MODE_COLON)
		mode = MODE_VI;

	/* if mode is no longer MODE_VI, then we should quit right away! */
	if (mode != MODE_VI)
		return cursor;

	/* The command did some output.  Wait for a keystoke. */
	if (exwrote)
	{
		msg("[Hit any key to continue]");
		if (getkey(0) == ':')
		{
			ungetkey(':');
		}
	}

	redraw(MARK_UNSET, FALSE);
	return cursor;
}

/* This function undoes the last change */
MARK v_undo(m)
	MARK	m;	/* (ignored) */
{
	undo();
	redraw(MARK_UNSET, FALSE);
	return cursor;
}

/* This function deletes the character(s) that the cursor is on */
MARK v_xchar(m, cnt)
	MARK	m;	/* where to start deletions */
	long	cnt;	/* number of chars to delete */
{
	DEFAULT(1);

	pfetch(markline(m));
	if (markidx(m + cnt) > plen)
	{
		cnt = plen - markidx(m);
	}
	if (cnt == 0L)
	{
		return MARK_UNSET;
	}
	ChangeText
	{
		cut(m, m + cnt);
		delete(m, m + cnt);
	}
	return m;
}

/* This function deletes character to the left of the cursor */
MARK v_Xchar(m, cnt)
	MARK	m;	/* where deletions end */
	long	cnt;	/* number of chars to delete */
{
	DEFAULT(1);

	/* if we're at the first char of the line, error! */
	if (markidx(m) == 0)
	{
		return MARK_UNSET;
	}

	/* make sure we don't try to delete more chars than there are */
	if (cnt > markidx(m))
	{
		cnt = markidx(m);
	}

	/* delete 'em */
	ChangeText
	{
		cut(m - cnt, m);
		delete(m - cnt, m);
	}

	return m - cnt;
}

/* This function defines a mark */
MARK v_mark(m, count, key)
	MARK	m;	/* where the mark will be */
	long	count;	/* (ignored) */
	int	key;	/* the ASCII label of the mark */
{
	if (key < 'a' || key > 'z')
	{
		msg("Marks must be from a to z");
	}
	else
	{
		mark[key - 'a'] = m;
	}
	return m;
}

/* This function toggles upper & lower case letters */
MARK v_ulcase(m)
	MARK	m;	/* where to make the change */
{
	char	new[2];

	/* extract the char that's there now */
	pfetch(markline(m));
	new[0] = ptext[markidx(m)];
	new[1] = '\0';

	/* change it if necessary */
	if (new[0] >= 'a' && new[0] <= 'z' || new[0] >= 'A' && new[0] <= 'Z')
	{
		new[0] ^= ('A' ^ 'a');
		ChangeText
		{
			change(m, m + 1, new);
		}
	}
	if (new[0] && ptext[markidx(m) + 1])
	{
		m++;
	}
	return m;
}


MARK v_replace(m, cnt, key)
	MARK	m;	/* first char to be replaced */
	long	cnt;	/* number of chars to replace */
	int	key;	/* what to replace them with */
{
	register char	*text;
	register int	i;
	static int	samekey;

	DEFAULT(1);

	/* map ^M to '\n' */
	if (key == '\r')
	{
		key = '\n';
	}
	else if (key == ctrl('V'))
	{
		if (doingdot)
			key = samekey;
		else
			key = samekey = getkey(0);
		if (key == 0)
			return MARK_UNSET;
	}
	else if (!doingdot && key == ctrl('['))
	{
		samekey = 0;
		return MARK_UNSET;
	}

	/* make sure the resulting line isn't too long */
	if (cnt > BLKSIZE - 2 - markidx(m))
	{
		cnt = BLKSIZE - 2 - markidx(m);
	}

	/* build a string of the desired character with the desired length */
	for (text = tmpblk.c, i = cnt; i > 0; i--)
	{
		*text++ = key;
	}
	*text = '\0';

	/* make sure cnt doesn't extend past EOL */
	pfetch(markline(m));
	key = markidx(m);
	if (key + cnt > plen)
	{
		cnt = plen - key;
	}

	/* do the replacement */
	ChangeText
	{
		change(m, m + cnt, tmpblk.c);
	}

	if (*tmpblk.c == '\n')
	{
		return (m & ~(BLKSIZE - 1)) + cnt * BLKSIZE;
	}
	else
	{
		return m + cnt - 1;
	}
}

MARK v_overtype(m)
	MARK		m;	/* where to start overtyping */
{
	MARK		end;	/* end of a substitution */
	static long	width;	/* width of a single-line replace */

	/* the "doingdot" version of replace is really a substitution */
	if (doingdot)
	{
		/* was the last one really repeatable? */
		if (width < 0)
		{
			msg("Can't repeat a multi-line overtype command");
			return MARK_UNSET;
		}

		/* replacing nothing by nothing?  Don't bother */
		if (width == 0)
		{
			return m;
		}

		/* replace some chars by repeated text */
		return v_subst(m, width);
	}

	/* Normally, we input starting here, in replace mode */
	ChangeText
	{
		end = input(m, m, WHEN_VIREP);
	}

	/* if we ended on the same line we started on, then this
	 * overtype is repeatable via the dot key.
	 */
	if (markline(end) == markline(m) && end >= m - 1L)
	{
		width = end - m + 1L;
	}
	else /* it isn't repeatable */
	{
		width = -1L;
	}

	return end;
}


/* This function selects which cut buffer to use */
MARK v_selcut(m, cnt, key)
	MARK	m;
	long	cnt;
	int	key;
{
	cutname(key);
	return m;
}

/* This function pastes text from a cut buffer */
MARK v_paste(m, cnt, cmd)
	MARK	m;	/* where to paste the text */
	long	cnt;	/* (ignored) */
	int	cmd;	/* either 'p' or 'P' */
{
	ChangeText
	{
		m = paste(m, cmd == 'p', FALSE);
	}
	return m;
}

/* This function yanks text into a cut buffer */
MARK v_yank(m, n)
	MARK	m, n;	/* range of text to yank */
{
	cut(m, n);
	return m;
}

/* This function deletes a range of text */
MARK v_delete(m, n)
	MARK	m, n;	/* range of text to delete */
{
	ChangeText
	{
		cut(m, n);
		delete(m, n);
	}
	return m;
}


/* This starts input mode without deleting anything */
MARK v_insert(m, cnt, key)
	MARK	m;	/* where to start (sort of) */
	long	cnt;	/* repeat how many times? */
	int	key;	/* what command is this for? {a,A,i,I,o,O} */
{
	int	wasdot;
	long	reps;

	DEFAULT(1);

	ChangeText
	{
		/* tweak the insertion point, based on command key */
		switch (key)
		{
		  case 'i':
			break;

		  case 'a':
			pfetch(markline(m));
			if (plen > 0)
			{
				m++;
			}
			break;

		  case 'I':
			m = movefront(m, 1L);
			break;

		  case 'A':
			pfetch(markline(m));
			m = (m & ~(BLKSIZE - 1)) + plen;
			break;

		  case 'O':
			m &= ~(BLKSIZE - 1);
			add(m, "\n");
			break;

		  case 'o':
			m = (m & ~(BLKSIZE - 1)) + BLKSIZE;
			add(m, "\n");
			break;
		}

		/* insert the same text once or more */
		for (reps = cnt, wasdot = doingdot; reps > 0; reps--, doingdot = TRUE)
		{
			m = input(m, m, WHEN_VIINP);
		}

		/* compensate for inaccurate redraw clues from input() */
		if (key == 'O' | key == 'o' && wasdot)
		{
			redrawpost++;
		}

		doingdot = FALSE;
	}

	return m;
}

/* This starts input mode with some text deleted */
MARK v_change(m, n)
	MARK	m, n;	/* the range of text to change */
{
	int	lnmode;	/* is this a line-mode change? */

	/* swap them if they're in reverse order */
	if (m > n)
	{
		MARK	tmp;
		tmp = m;
		m = n;
		n = tmp;
	}

	/* for line mode, retain the last newline char */
	lnmode = (markidx(m) == 0 && markidx(n) == 0 && m != n);
	if (lnmode)
	{
		n -= BLKSIZE;
		pfetch(markline(n));
		n = (n & ~(BLKSIZE - 1)) + plen;
	}

	ChangeText
	{
		cut(m, n);
		m = input(m, n, WHEN_VIINP);
	}

	/* compensate for inaccurate redraw clues from paste() */
	if (doingdot)
	{
		redrawpre = markline(n);
		if (lnmode)
		{
			redrawpre++;
			redrawpost++;
		}
	}

	return m;
}

/* This function replaces a given number of characters with input */
MARK v_subst(m, cnt)
	MARK	m;	/* where substitutions start */
	long	cnt;	/* number of chars to replace */
{
	DEFAULT(1);

	/* make sure we don't try replacing past EOL */
	pfetch(markline(m));
	if (markidx(m) + cnt > plen)
	{
		cnt = plen - markidx(m);
	}

	/* Go for it! */
	ChangeText
	{
		cut(m, m + cnt);
		m = input(m, m + cnt, WHEN_VIINP);
	}
	return m;
}

/* This calls the ex "join" command to join some lines together */
MARK v_join(m, cnt)
	MARK	m;	/* the first line to be joined */
	long	cnt;	/* number of other lines to join */
{
	MARK	joint;	/* where the lines were joined */

	DEFAULT(1);

	/* figure out where the joint will be */
	pfetch(markline(m));
	joint = (m & ~(BLKSIZE - 1)) + plen;

	/* join the lines */
	cmd_join(m, m + MARK_AT_LINE(cnt), CMD_JOIN, 0, "");
	mustredraw = TRUE;

	/* the cursor should be left at the joint */
	return joint;
}

/* This calls the ex shifter command to shift some lines */
static MARK shift_help(m, n, excmd)
	MARK	m, n;	/* range of lines to shift */
	CMD	excmd;	/* which way do we shift? */
{
	/* make sure our endpoints aren't in reverse order */
	if (m > n)
	{
		MARK tmp;

		tmp = n;
		n = m;
		m = tmp;
	}

	/* linemode? adjust for inclusive endmarks in ex */
	if (markidx(m) == 0 && markidx(n) == 0)
	{
		n -= BLKSIZE;
	}

	cmd_shift(m, n, excmd, 0, "");
	return m;
}

/* This calls the ex "<" command to shift some lines left */
MARK v_shiftl(m, n)
	MARK	m, n;	/* range of lines to shift */
{
	return shift_help(m, n, CMD_SHIFTL);
}

/* This calls the ex ">" command to shift some lines right */
MARK v_shiftr(m, n)
	MARK	m, n;	/* range of lines to shift */
{
	return shift_help(m, n, CMD_SHIFTR);
}

/* This runs some lines through a filter program */
MARK v_filter(m, n)
	MARK	m, n;	/* range of lines to shift */
{
	char	cmdln[100];	/* a shell command line */

	/* linemode? adjust for inclusive endmarks in ex */
	if (markidx(m) == 0 && markidx(n) == 0)
	{
		n -= BLKSIZE;
	}

	if (vgets('!', cmdln, sizeof(cmdln)) > 0)
	{
		filter(m, n, cmdln);
	}

	redraw(MARK_UNSET, FALSE);
	return m;
}


/* This function runs the ex "file" command to show the file's status */
MARK v_status()
{
	cmd_file(cursor, cursor, CMD_FILE, 0, "");
	return cursor;
}


/* This function does a tag search on a keyword */
MARK v_tag(keyword)
	char	*keyword;
{
	cmd_tag(cursor, cursor, CMD_TAG, 0, keyword);
	redraw(MARK_UNSET, FALSE);
	return cursor;
}

/* This function looks up a keyword by calling the helpprog program */
MARK v_keyword(keyword)
	char	*keyword;
{
	int	status;

	move(LINES - 1, 0);
	addstr("---------------------------------------------------------\n");
	clrtoeol();
	refresh();
	suspend_curses();

	switch (fork())
	{
	  case -1:						/* error */
		break;

	  case 0:						/* child */
		execl(o_keywordprg, o_keywordprg, keyword, (char *)0);
		exit(2); /* if we get here, the exec failed */

	  default:						/* parent */
		wait(&status);
		if (status > 0)
		{
			write(2, "<<< failed >>>\n", 15);
		}
	}

	resume_curses(FALSE); /* "resume, but not quietly" */
	redraw(MARK_UNSET, FALSE);
	return cursor;
}



/* This function acts like the EX command "xit" */
MARK v_xit(m, cnt, key)
	MARK	m;	/* ignored */
	long	cnt;	/* ignored */
	int	key;	/* must be a second 'Z' */
{
	/* if second char wasn't 'Z', fail */
	if (key != 'Z')
	{
		return MARK_UNSET;
	}

	/* move the physical cursor to the end of the screen */
	move(LINES - 1, 0);
	clrtoeol();
	refresh();

	/* do the xit command */
	cmd_xit(m, m, CMD_XIT, FALSE, "");

	/* if we're really going to quit, then scroll the screen up 1 line */
	if (mode == MODE_QUIT)
	{
		addch('\n');
	}

	/* regardless of whether we succeeded or failed, return the cursor */
	return m;
}


/* This function undoes changes to a single line, if possible */
MARK v_undoline(m)
	MARK	m;	/* where we hope to undo the change */
{
	if (markline(m) != U_line)
	{
		return MARK_UNSET;
	}

	ChangeText
	{
		changeline(U_line, U_text);
	}
	return m & ~(BLKSIZE - 1);
}
