/* move3.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains movement functions that perform character searches */

#include "vi.h"

static MARK	(*prevfwdfn)();	/* function to search in same direction */
static MARK	(*prevrevfn)();	/* function to search in opposite direction */
static char	prev_key;	/* sought cvhar from previous [fFtT] */

MARK	move_ch(m, cnt, cmd)
	MARK	m;	/* current position */
	long	cnt;
	char	cmd;	/* command: either ',' or ';' */
{
	MARK	(*tmp)();

	if (!prevfwdfn)
	{
		msg("No previous f, F, t, or T command");
		return MARK_UNSET;
	}

	if (cmd == ',')
	{
		m =  (*prevrevfn)(m, cnt, prev_key);

		/* Oops! we didn't want to change the prev*fn vars! */
		tmp = prevfwdfn;
		prevfwdfn = prevrevfn;
		prevrevfn = tmp;

		return m;
	}
	else
	{
		return (*prevfwdfn)(m, cnt, prev_key);
	}
}

/* move forward within this line to next occurrence of key */
MARK	movefch(m, cnt, key)
	MARK	m;	/* where to search from */
	long	cnt;
	char	key;	/* what to search for */
{
	register char	*text;

	DEFAULT(1);

	prevfwdfn = movefch;
	prevrevfn = moveFch;
	prev_key = key;

	pfetch(markline(m));
	text = ptext + markidx(m);
	while (cnt-- > 0)
	{
		do
		{
			m++;
			text++;
		} while (*text && *text != key);
	}
	if (!*text)
	{
		return MARK_UNSET;
	}
	return m;
}

/* move backward within this line to previous occurrence of key */
MARK	moveFch(m, cnt, key)
	MARK	m;	/* where to search from */
	long	cnt;
	char	key;	/* what to search for */
{
	register char	*text;

	DEFAULT(1);

	prevfwdfn = moveFch;
	prevrevfn = movefch;
	prev_key = key;

	pfetch(markline(m));
	text = ptext + markidx(m);
	while (cnt-- > 0)
	{
		do
		{
			m--;
			text--;
		} while (text >= ptext && *text != key);
	}
	if (text < ptext)
	{
		return MARK_UNSET;
	}
	return m;
}

/* move forward within this line almost to next occurrence of key */
MARK	movetch(m, cnt, key)
	MARK	m;	/* where to search from */
	long	cnt;
	char	key;	/* what to search for */
{
	/* skip the adjacent char */
	pfetch(markline(m));
	if (plen <= markidx(m))
	{
		return MARK_UNSET;
	}
	m++;

	m = movefch(m, cnt, key);
	if (m == MARK_UNSET)
	{
		return MARK_UNSET;
	}

	prevfwdfn = movetch;
	prevrevfn = moveTch;

	return m - 1;
}

/* move backward within this line almost to previous occurrence of key */
MARK	moveTch(m, cnt, key)
	MARK	m;	/* where to search from */
	long	cnt;
	char	key;	/* what to search for */
{
	/* skip the adjacent char */
	if (markidx(m) == 0)
	{
		return MARK_UNSET;
	}
	m--;

	m = moveFch(m, cnt, key);
	if (m == MARK_UNSET)
	{
		return MARK_UNSET;
	}

	prevfwdfn = moveTch;
	prevrevfn = movetch;

	return m + 1;
}
