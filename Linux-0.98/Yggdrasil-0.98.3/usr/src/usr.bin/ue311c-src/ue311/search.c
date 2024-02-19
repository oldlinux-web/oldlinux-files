/*
 * The functions in this file implement commands that search in the forward
 * and backward directions.
 *
 * (History comments formerly here have been moved to history.c)
 *
 */

#include <stdio.h>
#include "estruct.h"
#include "eproto.h"
#include "edef.h"
#include "elang.h"

static int	patlenadd;
static int	lastchfjump, lastchbjump;
static int	deltaf[HICHAR], deltab[HICHAR];
static int	replen;		/* length of replacement string */

#if	MAGIC
static int		group_count;
static int	group_len[MAXGROUPS];
static REGION	group_reg[MAXGROUPS];
#endif

/*
 * forwsearch -- Search forward.  Get a search string from the user, and
 *	search for the string.  If found, reset the "." to be just after
 *	the match string, and (perhaps) repaint the display.
 */
int PASCAL NEAR forwsearch(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int	status;

	/* If n is negative, search backwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return (backsearch(f, -n));

	/* Ask the user for the text of a pattern.  If the response is
	 * TRUE (responses other than FALSE are possible), search for
	 * pattern for up to n times, as long as the pattern is there
	 * to be found.
	 */
	if ((status = readpattern(TEXT78, &pat[0], TRUE)) == TRUE)
		status = forwhunt(f, n);
/*				"Search" */

	return (status);
}

/*
 * forwhunt -- Search forward for a previously acquired search string.
 *	If found, reset the "." to be just after the match string,
 *	and (perhaps) repaint the display.
 */
int PASCAL NEAR forwhunt(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int	spoint = PTEND;
	register int	status;

	if (n < 0)		/* search backwards */
		return (backhunt(f, -n));

	/* Make sure a pattern exists, or that we didn't switch
	 * into MAGIC mode after we entered the pattern.
	 */
	if (pat[0] == '\0') {
		mlwrite(TEXT80);
/*			"No pattern set" */
		return FALSE;
	}

#if	MAGIC
	if ((curwp->w_bufp->b_mode & MDMAGIC) && mcpat[0].mc_type == MCNIL) {
		if (!mcstr())
			return FALSE;
	}
#endif

	/*
	 * Do one extra search to get us past our current
	 * match, if the search type has us at the start
	 * of a match, instead of after a match.
	 */
	if (searchtype == SRBEGIN) {
		spoint = PTBEG;
		if (lastflag & CFSRCH)
			n = (n > 2)? (n + 1): 2;
	}

#if	MAGIC
	if (magical && (curwp->w_bufp->b_mode & MDMAGIC))
		status = mcscanner(FORWARD, spoint, n);
	else
#endif
		status = scanner(FORWARD, spoint, n);

	/* Complain if not there.
	 */
	if (status == FALSE)
		mlwrite(TEXT79);
/*			"Not found" */

	thisflag |= CFSRCH;
	return (status);
}

/*
 * backsearch -- Reverse search.  Get a search string from the user, and
 *	search, starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
int PASCAL NEAR backsearch(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int	status;

	/* If n is negative, search forwards.
	 * Otherwise proceed by asking for the search string.
	 */
	if (n < 0)
		return (forwsearch(f, -n));

	/* Ask the user for the text of a pattern.  If the response is
	 * TRUE (responses other than FALSE are possible), search for
	 * pattern for up to n times, as long as the pattern is there
	 * to be found.
	 */
	if ((status = readpattern(TEXT81, &pat[0], TRUE)) == TRUE)
		status = backhunt(f, n);
/*				"Reverse search" */

	return (status);
}

/*
 * backhunt -- Reverse search for a previously acquired search string,
 *	starting at "." and proceeding toward the front of the buffer.
 *	If found "." is left pointing at the first character of the pattern
 *	(the last character that was matched).
 */
int PASCAL NEAR backhunt(f, n)
int f, n;	/* default flag / numeric argument */
{
	register int	spoint = PTBEG;
	register int	status;

	if (n < 0)
		return (forwhunt(f, -n));

	/* Make sure a pattern exists, or that we didn't switch
	 * into MAGIC mode after we entered the pattern.
	 */
	if (tap[0] == '\0') {
		mlwrite(TEXT80);
/*			"No pattern set" */
		return FALSE;
	}
#if	MAGIC
	if ((curwp->w_bufp->b_mode & MDMAGIC) && tapcm[0].mc_type == MCNIL) {
		if (!mcstr())
			return FALSE;
	}
#endif

	/*
	 * Do one extra search to get us past our current
	 * match, if the search type has us at the start
	 * of a match, instead of after a match.
	 */
	if (searchtype == SREND) {
		spoint = PTEND;
		if (lastflag & CFSRCH)
			n = (n > 2)? (n + 1): 2;
	}

#if	MAGIC
	if (magical && (curwp->w_bufp->b_mode & MDMAGIC))
		status = mcscanner(REVERSE, spoint, n);
	else
#endif
		status = scanner(REVERSE, spoint, n);

	/* Complain if not there.
	 */
	if (status == FALSE)
		mlwrite(TEXT79);
/*			"Not found" */

	thisflag |= CFSRCH;
	return (status);
}

#if	MAGIC
/*
 * mcscanner -- Search for a meta-pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 */
int PASCAL NEAR mcscanner(direct, beg_or_end, repeats)
int	direct;		/* which way to go.*/
int	beg_or_end;	/* put point at beginning or end of pattern.*/
int	repeats;	/* search repetitions.*/
{
	MC	*mcpatrn;	/* pointer into pattern */
	LINE	*curline;	/* current line during scan */
	int	curoff;		/* position within current line */

	/* If we are going in reverse, then the 'end' is actually
	 * the beginning of the pattern.  Toggle it.
	 */
	beg_or_end ^= direct;

	/* Another directional problem: if we are searching forwards,
	 * use the pattern in mcpat[].  Otherwise, use the reversed
	 * pattern in tapcm[].
	 */
	mcpatrn = (direct == FORWARD)? &mcpat[0]: &tapcm[0];

	/* Setup local scan pointers to global ".".
	 */
	curline = curwp->w_dotp;
	curoff  = curwp->w_doto;

	/* Scan each character until we hit the head link record.
	 */
	while (!boundry(curline, curoff, direct)) {
		/* Save the current position in case we need to
		 * restore it on a match, and initialize matchlen to
		 * zero in case we are doing a search for replacement.
		 */
		matchline = curline;
		matchoff = curoff;
		matchlen = 0;

		if (amatch(mcpatrn, direct, &curline, &curoff)) {
			/* A SUCCESSFULL MATCH!!!
			 * Flag that we have moved,
			 * reset the global "." pointers.
			 */
			curwp->w_flag |= WFMOVE;
			if (beg_or_end == PTEND)	/* at end of string */
			{
				curwp->w_dotp = curline;
				curwp->w_doto = curoff;
			}
			else		/* at beginning of string */
			{
				curwp->w_dotp = matchline;
				curwp->w_doto = matchoff;
			}

			/* If we're heading in reverse, set the "match"
			 * pointers to the start of the string, for savematch().
			 */
			if (direct == REVERSE) {
				matchline = curline;
				matchoff = curoff;
			}

			if (savematch() == ABORT)
				return ABORT;

			/* Continue scanning if we haven't found
			 * the nth match.
			 */
			if (--repeats <= 0)
				return TRUE;
		}

		/* Advance the cursor.
		 */
		nextch(&curline, &curoff, direct);
	}

	return FALSE;	/* We could not find a match.*/
}

/*
 * amatch -- Search for a meta-pattern in either direction.  Based on the
 *	recursive routine amatch() (for "anchored match") in
 *	Kernighan & Plauger's "Software Tools".
 */
int PASCAL NEAR	amatch(mcptr, direct, pcwline, pcwoff)
register MC	*mcptr;		/* pattern to scan for */
int		direct;		/* which way to go.*/
LINE		**pcwline;	/* current line during scan */
int		*pcwoff;	/* position within current line */
{
	register int	c;	/* character at current position */
	LINE	*curline;	/* current line during scan */
	int	curoff;		/* position within current line */
	int	n_chars;	/* number of chars matched in a closure.*/
	int	cl_min;		/* minimum number of chars matched in closure.*/
	int	cl_type;	/* Which closure type?.*/

	/* Set up local scan pointers to ".".
	 */
	curline = *pcwline;
	curoff = *pcwoff;

	/*
	 * Loop through the meta-pattern, laughing all the way.
	 */
	while (mcptr->mc_type != MCNIL) {
		/* Is the current meta-character modified
		 * by a closure?
		 */
		if (cl_type = (mcptr->mc_type & ALLCLOS)) {
			c = nextch(&curline, &curoff, direct);
			n_chars = 0;

			if (cl_type == ZEROONE) {
				cl_min = 0;

				if (mceq(c, mcptr)) {
					c = nextch(&curline, &curoff, direct);
					n_chars++;
				}
			}
			else
			{
				/* Minimum number of characters that may
				 * match is 0 or 1.
				 */
				cl_min = (cl_type == CLOSURE_1);

				/* Match as many characters as possible
				 * against the current meta-character.
				 */
				while (mceq(c, mcptr)) {
					c = nextch(&curline, &curoff, direct);
					n_chars++;
				}
			}

			/* We are now at the character that made us
			 * fail.  Try to match the rest of the pattern.
			 * Shrink the closure by one for each failure.
			 */
			mcptr++;
			matchlen += n_chars;
			for (;;) {
				if (n_chars < cl_min) {
					matchlen -= n_chars;
					return FALSE;
				}

				nextch(&curline, &curoff, direct ^ REVERSE);

				if (amatch(mcptr, direct, &curline, &curoff))
					goto success;
				matchlen--;
				n_chars--;
			}
		}
		else if (mcptr->mc_type == GRPBEG) {
			group_reg[mcptr->u.group_no].r_offset = curoff;
			group_reg[mcptr->u.group_no].r_linep = curline;
			group_reg[mcptr->u.group_no].r_size = (direct == FORWARD)? -matchlen: matchlen;
		}
		else if (mcptr->mc_type == GRPEND) {
			group_len[mcptr->u.group_no] = (direct == FORWARD)? matchlen: -matchlen;
		}
		else if (mcptr->mc_type == BOL) {
			if (curoff != 0)
				return FALSE;
		}
		else if (mcptr->mc_type == EOL) {
			if (curoff != llength(curline))
				return FALSE;
		}
		else
		{
			/* A character to compare against
			 * the meta-character equal function.
			 * If we still match, increment the length.
			 */
			if (!mceq(nextch(&curline, &curoff, direct), mcptr))
				return FALSE;

			matchlen++;
		}

		mcptr++;
	}			/* End of mcptr loop.*/

	/* A SUCCESSFULL MATCH!!!
	 * Reset the "." pointers.
	 */
success:
	*pcwline = curline;
	*pcwoff  = curoff;

	return (TRUE);
}
#endif

/*
 * scanner -- Search for a pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 *	Fast version using simplified version of Boyer and Moore
 *	Software-Practice and Experience, vol 10, 501-506 (1980)
 */
int PASCAL NEAR scanner(direct, beg_or_end, repeats)
int	direct;		/* which way to go.*/
int	beg_or_end;	/* put point at beginning or end of pattern.*/
int	repeats;	/* search repetitions.*/
{
	register int	c;		/* character at current position */
	register char	*patptr;	/* pointer into pattern */
	char	*patrn;			/* string to scan for */
	LINE	*curline;		/* current line during scan */
	int	curoff;			/* position within current line */
	LINE	*scanline;		/* current line during scanning */
	int	scanoff;		/* position in scanned line */
	int	jump;			/* next offset */

	/* If we are going in reverse, then the 'end' is actually
	 * the beginning of the pattern.  Toggle it.
	 */
	beg_or_end ^= direct;

	/* Another directional problem: if we are searching
	 * forwards, use the pattern in pat[], and the jump
	 * size in lastchfjump.  Otherwise, use the reversed
	 * pattern in tap[], and the jump size of lastchbjump.
	 */
	if (direct == FORWARD) {
		patrn = &pat[0];
		jump = lastchfjump;
	}
	else
	{
		patrn = &tap[0];
		jump = lastchbjump;
	}

	/* Set up local pointers to global ".".
	 */
	curline = curwp->w_dotp;
	curoff = curwp->w_doto;

	/* Scan each character until we hit the head link record.
	 * Get the character resolving newlines, offset
	 * by the pattern length, i.e. the last character of the
	 * potential match.
	 */
	if (!fbound(patlenadd, &curline, &curoff, direct)) {
		do
		{
			/* Set up the scanning pointers, and save
			 * the current position in case we match
			 * the search string at this point.
			 */
			scanline = matchline = curline;
			scanoff  = matchoff  = curoff;

			patptr = patrn;

			/* Scan through the pattern for a match.
			 */
			while ((c = *patptr++) != '\0')
				if (!eq(c, nextch(&scanline, &scanoff, direct)))
					goto fail;

			/* A SUCCESSFULL MATCH!!!
			 * Flag that we have moved and reset the
			 * global "." pointers.
			 */
			curwp->w_flag |= WFMOVE;
			if (beg_or_end == PTEND)	/* at end of string */
			{
				curwp->w_dotp = scanline;
				curwp->w_doto = scanoff;
			}
			else		/* at beginning of string */
			{
				curwp->w_dotp = matchline;
				curwp->w_doto = matchoff;
			}

			/* If we're heading in reverse, set the "match"
			 * pointers to the start of the string, for savematch().
			 */
			if (direct == REVERSE) {
				matchline = scanline;
				matchoff = scanoff;
			}

			if (savematch() == ABORT)
				return ABORT;

			/* Continue scanning if we haven't found
			 * the nth match.
			 */
			if (--repeats <= 0)
				return (TRUE);
			else
			{
				curline = scanline;
				curoff = scanoff;
			}

fail:;			/* continue to search */
		} while (!fbound(jump, &curline, &curoff, direct));
	}

	return FALSE;	/* We could not find a match */
}

/*
 * fbound -- Return information depending on whether we have hit a boundry
 *	(and may therefore search no further) or if a trailing character
 *	of the search string has been found.  See boundry() for search
 *	restrictions.
 */
int PASCAL NEAR	fbound(jump, pcurline, pcuroff, dir)
LINE	**pcurline;
int	*pcuroff, dir, jump;
{
	register int	spare, curoff;
	register LINE	*curline;

	curline = *pcurline;
	curoff = *pcuroff;

	if (dir == FORWARD)
	{
		while (jump != 0) {
			curoff += jump;
			spare = curoff - llength(curline);

			if (curline == curbp->b_linep)
				return (TRUE);	/* hit end of buffer */

			while (spare > 0) {
				curline = lforw(curline);/* skip to next line */
				curoff = spare - 1;
				spare = curoff - llength(curline);
				if (curline == curbp->b_linep)
					return (TRUE);	/* hit end of buffer */
			}

			if (spare == 0)
				jump = deltaf[(int) '\r'];
			else
				jump = deltaf[(int) lgetc(curline, curoff)];
		}

		/* The last character matches, so back up to start
		 * of possible match.
		 */
		curoff -= patlenadd;

		while (curoff < 0) {
			curline = lback(curline);	/* skip back a line */
			curoff += llength(curline) + 1;
		}

	}
	else			/* Reverse.*/
	{
		jump++;		/* allow for offset in reverse */
		while (jump != 0) {
			curoff -= jump;
			while (curoff < 0) {
				curline = lback(curline);	/* skip back a line */
				curoff += llength(curline) + 1;
				if (curline == curbp->b_linep)
					return (TRUE);	/* hit end of buffer */
			}

			if (curoff == llength(curline))
				jump = deltab[(int) '\r'];
			else
				jump = deltab[(int) lgetc(curline, curoff)];
		}

		/* The last character matches, so back up to start
		 * of possible match.
		 */
		curoff += patlenadd + 1;
		spare = curoff - llength(curline);
		while (spare > 0) {
			curline = lforw(curline);	/* skip back a line */
			curoff = spare - 1;
			spare = curoff - llength(curline);
		}
	}

	*pcurline = curline;
	*pcuroff = curoff;
	return FALSE;
}

/*
 * setjtable -- Settting up search delta forward and delta backward
 *	tables.  The reverse search string and string lengths are
 *	set here, for table initialization and for substitution
 *	purposes.  The default for any character to jump is the
 *	pattern length.
 */
VOID PASCAL NEAR setjtable()
{
	int	i;

	rvstrcpy(tap, pat);
	patlenadd = (matchlen = strlen(pat)) - 1;

	for (i = 0; i < HICHAR; i++) {
		deltaf[i] = matchlen;
		deltab[i] = matchlen;
	}

	/* Now put in the characters contained
	 * in the pattern, duplicating the CASE.
	 */
	for (i = 0; i < patlenadd; i++) {
		if (isletter(pat[i]))
			deltaf[(unsigned int) chcase((unsigned int) pat[i])]
			 = patlenadd - i;
		deltaf[(unsigned int) pat[i]] = patlenadd - i;

		if (isletter(tap[i]))
			deltab[(unsigned int) chcase((unsigned int) tap[i])]
			 = patlenadd - i;
		deltab[(unsigned int) tap[i]] = patlenadd - i;
	}

	/* The last character will have the pattern length
	 * unless there are duplicates of it.  Get the number to
	 * jump from the arrays delta, and overwrite with zeroes
	 * in delta duplicating the CASE.
	 *
	 * The last character is, of course, the first character
	 * of the reversed string.
	 */
	lastchfjump = patlenadd + deltaf[(unsigned int) tap[0]];
	lastchbjump = patlenadd + deltab[(unsigned int) pat[0]];

	if (isletter(tap[0]))
		deltaf[(unsigned int) chcase(tap[0])] = 0;
	deltaf[(unsigned int) tap[0]] = 0;

	if (isletter(pat[0]))
		deltab[(unsigned int) chcase(pat[0])] = 0;
	deltab[(unsigned int) pat[0]] = 0;
}

/*
 * rvstrcpy -- Reverse string copy.
 */
VOID PASCAL NEAR rvstrcpy(rvstr, str)
register char	*rvstr, *str;
{
	register int i;

	str += (i = strlen(str));

	while (i-- > 0)
		*rvstr++ = *--str;

	*rvstr = '\0';
}

/*
 * eq -- Compare two characters.  The "bc" comes from the buffer, "pc"
 *	from the pattern.  If we are not in EXACT mode, fold out the case.
 */
int PASCAL NEAR eq(bc, pc)
register int	bc;
register int	pc;
{
	if ((curwp->w_bufp->b_mode & MDEXACT) == 0) {
		if (islower(bc))
			bc = chcase(bc);

		if (islower(pc))
			pc = chcase(pc);
	}

	return (bc == pc);
}

/*
 * readpattern -- Read a pattern.  Stash it in apat.  If it is the
 *	search string (which means that the global variable pat[]
 *	has been passed in), create the reverse pattern and the magic
 *	pattern, assuming we are in MAGIC mode (and #defined that way).
 *
 *	Apat is not updated if the user types in an empty line.  If
 *	the user typed an empty line, and there is no old pattern, it is
 *	an error.  Display the old pattern, in the style of Jeff Lomicka.
 *	There is some do-it-yourself control expansion.  Change to using
 *	<META> to delimit the end-of-pattern to allow <NL>s in the search
 *	string. 
 */
int PASCAL NEAR readpattern(prompt, apat, srch)
char	*prompt;
char	apat[];
int	srch;
{
	register int	status;
	char		tpat[NPAT+20];

	strcpy(tpat, prompt);	/* copy prompt to output string */
	strcat(tpat, " [");
	expandp(&apat[0], &tpat[strlen(tpat)], NPAT/2);	/* add old pattern */
	strcat(tpat, "]");

	/* display the proper current search terminator character */
	switch (sterm) {

		case CTRL | '[':	strcat(tpat, "<META>"); break;
		case CTRL | 'M':	strcat(tpat, "<NL>"); break;
		default:		strcat(tpat, "<");
					cmdstr(sterm, &(tpat[strlen(tpat)]));
					strcat(tpat, ">");
	}
	strcat(tpat, ": ");

	/* Read a pattern.  Either we get one,
	 * or we just get the META charater, and use the previous pattern.
	 * Then, if it's the search string, create the delta tables.
	 * *Then*, make the meta-pattern, if we are defined that way.
	 */
	if ((status = mltreply(tpat, tpat, NPAT, sterm)) == TRUE) {
		lastflag &= ~CFSRCH;
		strcpy(apat, tpat);

		if (srch)
			setjtable();

	}
	else if (status == FALSE && apat[0] != 0)	/* Old one */
		status = TRUE;

#if	MAGIC
	/* Only make the meta-pattern if in magic mode, since the
	 * pattern in question might have an invalid meta combination.
	 */
	if (status == TRUE)
		if ((curwp->w_bufp->b_mode & MDMAGIC) == 0) {
			mcclear();
			rmcclear();
		}
		else
			status = srch? mcstr(): rmcstr();
#endif
	return (status);
}

/*
 * savematch -- We found the pattern?  Let's save it away.
 */
int PASCAL NEAR savematch()
{
	register int	j;
	REGION		tmpreg;

	if (patmatch != NULL)
		free(patmatch);

	if ((patmatch = malloc(matchlen + 1)) == NULL) {
		mlwrite(TEXT94);
/*			"%%Out of memory" */
		return ABORT;
	}

	tmpreg.r_offset = matchoff;
	tmpreg.r_linep = matchline;
	tmpreg.r_size = matchlen;

#if	MAGIC == 0
	regtostr(patmatch, &tmpreg);
#else
	/*
	 * Save the groups.
	 */
	grpmatch[0] = regtostr(patmatch, &tmpreg);
	for (j = 1; j <= group_count; j++)
	{
		group_reg[j].r_size += group_len[j];

		if (grpmatch[j] != NULL)
			free(grpmatch[j]);

		if ((grpmatch[j] = malloc(group_reg[j].r_size + 1)) != NULL)
			regtostr(grpmatch[j], &group_reg[j]);
		else {
			mlwrite(TEXT94);
/*			"%%Out of memory" */
			return ABORT;
		}
	}
#endif
	return TRUE;
}

/*
 * sreplace -- Search and replace.
 */
int PASCAL NEAR sreplace(f, n)
int f;		/* default flag */
int n;		/* # of repetitions wanted */
{
	return (replaces(FALSE, f, n));
}

/*
 * qreplace -- search and replace with query.
 */
int PASCAL NEAR qreplace(f, n)
int f;		/* default flag */
int n;		/* # of repetitions wanted */
{
	return (replaces(TRUE, f, n));
}

/*
 * replaces -- Search for a string and replace it with another
 *	string.  Query might be enabled (according to kind).
 */
int PASCAL NEAR	replaces(kind, f, n)
int	kind;	/* Query enabled flag */
int	f;	/* default flag */
int	n;	/* # of repetitions wanted */
{
	register int status;	/* success flag on pattern inputs */
	register int nummatch;	/* number of found matches */
	int numsub;		/* number of substitutions */
	int nlflag;		/* last char of search string a <NL>? */
	int nlrepl;		/* was a replace done on the last line? */
	char c;			/* input char for query */
	char tpat[NPAT];	/* temporary to hold search pattern */
	LINE *origline;		/* original "." position */
	int origoff;		/* and offset (for . query option) */
	LINE *lastline;		/* position of last replace and */
	int lastoff;		/* offset (for 'u' query option) */
	int oldmatchlen;	/* Closure may alter the match length.*/
	static char *oldpatmatch = NULL;	/* allocated memory for un-do.*/

	if (curbp->b_mode & MDVIEW)	/* don't allow this command if	*/
		return (rdonly());	/* we are in read only mode	*/

	/* Check for negative repetitions.
	 */
	if (f && n < 0)
		return FALSE;

	/* Ask the user for the text of a pattern.
	 */
	if ((status = readpattern(
	    (kind == FALSE ? TEXT84 : TEXT85), &pat[0], TRUE)) != TRUE)
/*				"Replace" */
/*					"Query replace" */
		return (status);

	/* Ask for the replacement string, and get its length.
	 */
	if ((status = readpattern(TEXT86, &rpat[0], FALSE)) == ABORT)
/*				"with" */
		return (status);

	/* Set up flags so we can make sure not to do a recursive
	 * replace on the last line.
	 */
	nlflag = (pat[matchlen - 1] == '\r');
	nlrepl = FALSE;

	if (kind) {
		/* Build query replace question string.
		 */
		strcpy(tpat, TEXT87);
/*				"Replace '" */
		expandp(&pat[0], &tpat[strlen(tpat)], NPAT/3);
		strcat(tpat, TEXT88);
/*				"' with '" */
		expandp(&rpat[0], &tpat[strlen(tpat)], NPAT/3);
		strcat(tpat, "'? ");

		/* Initialize last replaced pointers.
		 */
		lastline = NULL;
		lastoff = 0;
	}

	/* Save original . position, reset the number of matches and
	 * substitutions, and scan through the file.
	 */
	origline = curwp->w_dotp;
	origoff = curwp->w_doto;
	numsub = 0;
	nummatch = 0;

	while ( (f == FALSE || n > nummatch) &&
		(nlflag == FALSE || nlrepl == FALSE) ) {
		/* Search for the pattern.
		 * If we search with a regular expression,
		 * matchlen is reset to the true length of
		 * the matched string.
		 */
#if	MAGIC
		if (magical && (curwp->w_bufp->b_mode & MDMAGIC)) {
			if (!mcscanner(FORWARD, PTBEG, 1))
				break;
		}
		else
#endif
			if (!scanner(FORWARD, PTBEG, 1))
				break;		/* all done */

		++nummatch;	/* Increment # of matches */

		/* Check if we are on the last line.
		 */
		nlrepl = (lforw(curwp->w_dotp) == curwp->w_bufp->b_linep);

		/* Check for query.
		 */
		if (kind) {
			/* Get the query.
			 */
pprompt:		mlwrite(&tpat[0]);
qprompt:
			update(TRUE);  /* show the proposed place to change */
			c = tgetc();			/* and input */
			mlerase();			/* and clear it */

			/* And respond appropriately.
			 */
			switch (c) {
#if	FRENCH
				case 'o':	/* oui, substitute */
				case 'O':
#endif
				case 'y':	/* yes, substitute */
				case 'Y':
				case ' ':
					break;

				case 'n':	/* no, onward */
				case 'N':
					forwchar(FALSE, 1);
					continue;

				case '!':	/* yes/stop asking */
					kind = FALSE;
					break;

				case 'u':	/* undo last and re-prompt */
				case 'U':
					/* Restore old position.
					 */
					if (lastline == NULL) {
						/* There is nothing to undo.
						 */
						TTbeep();
						goto pprompt;
					}
					curwp->w_dotp = lastline;
					curwp->w_doto = lastoff;
					lastline = NULL;
					lastoff = 0;

					/* Delete the new string,
					 * restore the old match.
					 */
					backchar(FALSE, replen);
					status = delins(replen, oldpatmatch, FALSE);
					if (status != TRUE)
						return (status);

					/* Record one less substitution,
					 * backup, save our place, and
					 * reprompt.
					 */
					--numsub;
					backchar(FALSE, oldmatchlen);
					matchlen = oldmatchlen;
					matchline = curwp->w_dotp;
					matchoff  = curwp->w_doto;
					continue;

				case '.':	/* abort! and return */
					/* restore old position */
					curwp->w_dotp = origline;
					curwp->w_doto = origoff;
					curwp->w_flag |= WFMOVE;

				case BELL:	/* abort! and stay */
					mlwrite(TEXT89);
/*						"Aborted!" */
					return FALSE;

				default:	/* bitch and beep */
					TTbeep();

				case '?':	/* help me */
					mlwrite(TEXT90);
/*"(Y)es, (N)o, (!)Do rest, (U)ndo last, (^G)Abort, (.)Abort back, (?)Help: "*/
					goto qprompt;

			}	/* end of switch */
		}	/* end of "if kind" */

		/* if this is the point origin, flag so we a can reset it */
		if (curwp->w_dotp == origline) {
			origline = NULL;
			lastline = lback(curwp->w_dotp);
		}

		/* Delete the sucker, and insert its
		 * replacement.
		 */
#if	MAGIC
		status = delins(matchlen, &rpat[0], rmagical);
#else
		status = delins(matchlen, &rpat[0], FALSE);
#endif
		if (origline == NULL) {
			origline = lforw(lastline);
			origoff = 0;
		}

		if (status != TRUE)
			return (status);

		numsub++;	/* increment # of substitutions */

		/* Save our position, the match length, and the string
		 * matched if we are query-replacing, as we may undo
		 * the replacement. If we are not querying, check to
		 * make sure that we didn't replace an empty string
		 * (possible in MAGIC mode), because we'll infinite loop.
		 */
		if (kind) {
			lastline = curwp->w_dotp;
			lastoff = curwp->w_doto;
			oldmatchlen = matchlen;	/* Save the length for un-do.*/

			if (oldpatmatch != NULL)
				free(oldpatmatch);

			if ((oldpatmatch = malloc(matchlen + 1)) == NULL) {
				mlwrite(TEXT94);
/*					"%%Out of memory" */
				return ABORT;
			}
			strcpy(oldpatmatch, patmatch);
		}
		else if (matchlen == 0) {
			mlwrite(TEXT91);
/*				"Empty string replaced, stopping." */
			return FALSE;
		}
	}

	/* And report the results.
	 */
	mlwrite(TEXT92, numsub);
/*		"%d substitutions" */
	return (TRUE);
}

/*
 * delins -- Delete a specified length from the current point
 *	then either insert the string directly, or make use of
 *	replacement meta-array.
 */
int PASCAL NEAR delins(dlength, instr, use_rmc)
int	dlength;
char	*instr;
int	use_rmc;
{
	register int	status;
	register char	*rstr;
#if	MAGIC
	register RMC	*rmcptr;
#endif

	replen = 0;

	/* Zap what we gotta,
	 * and insert its replacement.
	 */
	if ((status = ldelete((long) dlength, FALSE)) != TRUE)
		mlwrite(TEXT93);
/*			"%%ERROR while deleting" */
	else
#if	MAGIC
		if (use_rmc && (curwp->w_bufp->b_mode & MDMAGIC)) {
			rmcptr = &rmcpat[0];
			while (rmcptr->mc_type != MCNIL && status == TRUE) {
				if (rmcptr->mc_type == LITCHAR)
					status = linstr(rstr = rmcptr->u.rstr);
				else if (rmcptr->mc_type == DITTO)
					status = linstr(rstr = patmatch);
				else
					status = linstr(rstr = fixnull(grpmatch[rmcptr->u.group_no]));
				replen += strlen(rstr);
				rmcptr++;
			}
		}
		else
#endif
		{
			status = linstr(instr);
			replen = strlen(instr);
		}

	return (status);
}

/*
 * expandp -- Expand control key sequences for output.
 */
int PASCAL NEAR expandp(srcstr, deststr, maxlength)
char	*srcstr;	/* string to expand */
char	*deststr;	/* destination of expanded string */
int	maxlength;	/* maximum chars in destination */
{
	unsigned char c;	/* current char to translate */

	/* Scan through the string.
	 */
	while ((c = *srcstr++) != 0) {
		if (c == '\r')		/* it's a newline */
		{
			*deststr++ = '<';
			*deststr++ = 'N';
			*deststr++ = 'L';
			*deststr++ = '>';
			maxlength -= 4;
		}
		else if (c < 0x20 || c == 0x7f)	/* control character */
		{
			*deststr++ = '^';
			*deststr++ = c ^ 0x40;
			maxlength -= 2;
		}
		else if (c == '%') {
			*deststr++ = '%';
			*deststr++ = '%';
			maxlength -= 2;
		}
		else			/* any other character */
		{
			*deststr++ = c;
			maxlength--;
		}

		/* check for maxlength */
		if (maxlength < 4) {
			*deststr++ = '$';
			*deststr = '\0';
			return FALSE;
		}
	}
	*deststr = '\0';
	return (TRUE);
}

/*
 * boundry -- Return information depending on whether we may search no
 *	further.  Beginning of file and end of file are the obvious
 *	cases, but we may want to add further optional boundry restrictions
 *	in future, a' la VMS EDT.  At the moment, just return (TRUE) or
 *	FALSE depending on if a boundry is hit (ouch).
 */
int PASCAL NEAR boundry(curline, curoff, dir)
LINE	*curline;
int	curoff, dir;
{
	register int	border;

	if (dir == FORWARD) {
		border = (curoff == llength(curline)) &&
			 (lforw(curline) == curbp->b_linep);
	}
	else
	{
		border = (curoff == 0) &&
			 (lback(curline) == curbp->b_linep);
	}
	return (border);
}

/*
 * nextch -- retrieve the next/previous character in the buffer,
 *	and advance/retreat the point.
 *	The order in which this is done is significant, and depends
 *	upon the direction of the search.  Forward searches look at
 *	the current character and move, reverse searches move and
 *	look at the character.
 */
int PASCAL NEAR nextch(pcurline, pcuroff, dir)
LINE	**pcurline;
int	*pcuroff;
int	dir;
{
	register LINE	*curline;
	register int	curoff;
	register int	c;

	curline = *pcurline;
	curoff = *pcuroff;

	if (dir == FORWARD) {
		if (curoff == llength(curline))		/* if at EOL */
		{
			curline = lforw(curline);	/* skip to next line */
			curoff = 0;
			c = '\r';			/* and return a <NL> */
		}
		else
			c = lgetc(curline, curoff++);	/* get the char */
	}
	else			/* Reverse.*/
	{
		if (curoff == 0) {
			curline = lback(curline);
			curoff = llength(curline);
			c = '\r';
		}
		else
			c = lgetc(curline, --curoff);

	}
	*pcurline = curline;
	*pcuroff = curoff;

	return (c);
}

#if	MAGIC
/*
 * mcstr -- Set up the 'magic' array.  The closure symbol is taken as
 *	a literal character when (1) it is the first character in the
 *	pattern, and (2) when preceded by a symbol that does not allow
 *	closure, such as beginning or end of line symbol, or another
 *	closure symbol.
 *
 *	Coding comment (jmg):  yes, i know i have gotos that are, strictly
 *	speaking, unnecessary.  But right now we are so cramped for
 *	code space that i will grab what i can in order to remain
 *	within the 64K limit.  C compilers actually do very little
 *	in the way of optimizing - they expect you to do that.
 */
int PASCAL NEAR mcstr()
{
	MC	*mcptr, *rtpcm;
	char	*patptr;
	int	pchr;
	int	status = TRUE;
	int	does_closure = FALSE;
	int	mj = 0;
	int	group_stack[MAXGROUPS];
	int	stacklevel = 0;

	/* If we had metacharacters in the MC array previously,
	 * free up any bitmaps that may have been allocated, and
	 * reset magical.
	 */
	if (magical)
		mcclear();

	mcptr = &mcpat[0];
	patptr = &pat[0];

	while ((pchr = *patptr) && status) {
		switch (pchr) {
			case MC_CCL:
				status = cclmake(&patptr, mcptr);
				magical = TRUE;
				does_closure = TRUE;
				break;

			case MC_BOL:
				/* If the BOL character isn't the
				 * first in the pattern, we assume
				 * it's a literal instead.
				 */
				if (mj != 0)
					goto litcase;

				mcptr->mc_type = BOL;
				magical = TRUE;
				break;

			case MC_EOL:
				/* If the EOL character isn't the
				 * last in the pattern, we assume
				 * it's a literal instead.
				 */
				if (*(patptr + 1) != '\0')
					goto litcase;

				mcptr->mc_type = EOL;
				magical = TRUE;
				break;

			case MC_ANY:
				mcptr->mc_type = ANY;
				magical = TRUE;
				does_closure = TRUE;
				break;

			case MC_CLOSURE:
			case MC_CLOSURE_1:
			case MC_ZEROONE:

				/* Does the closure symbol mean closure here?
				 * If so, back up to the previous element
				 * and indicate it is enclosed.
				 */
				if (does_closure == FALSE)
					goto litcase;
				mj--;
				mcptr--;
				if (pchr == MC_CLOSURE)
					mcptr->mc_type |= CLOSURE;
				else if (pchr == MC_CLOSURE_1)
					mcptr->mc_type |= CLOSURE_1;
				else
					mcptr->mc_type |= ZEROONE;

				magical = TRUE;
				does_closure = FALSE;
				break;

			case MC_ESC:

				/* No break between here and LITCHAR if
				 * the next character is to be taken literally.
				 */
				magical = TRUE;
				pchr = *++patptr;
				if (pchr == MC_GRPBEG) {
					/* Start of a group.  Indicate it, and
					 * set magical.
					 */
					if (++group_count < MAXGROUPS) {
						mcptr->mc_type = GRPBEG;
						mcptr->u.group_no = group_count;
						group_stack[stacklevel++] = group_count;
						does_closure = FALSE;
					}
					else
					{
						mlwrite(TEXT221);
/*							"Too many groups" */
						status = FALSE;
					}
					break;
				}
				else if (pchr == MC_GRPEND) {
					/* If we've no groups to close, assume
					 * a literal character.  Otherwise,
					 * indicate the end of a group.
					 */
					if (stacklevel > 0) {
						mcptr->u.group_no = group_stack[--stacklevel];
						mcptr->mc_type = GRPEND;
						does_closure = FALSE;
						break;
					}
				}
				else if (pchr == '\0') {
					pchr = '\\';
					--patptr;
				}
			default:
litcase:			mcptr->mc_type = LITCHAR;
				mcptr->u.lchar = pchr;
				does_closure = TRUE;
				break;
		}		/* End of switch.*/
		mcptr++;
		patptr++;
		mj++;
	}		/* End of while.*/

	/* Close off the meta-string, then set up the reverse array,
	 * if the status is good.
	 *
	 * The beginning and end of groups are reversed by flipping
	 * the bits, if they are set.
	 *
	 * If the status is not good, nil out the meta-pattern.
	 * Even if the status is bad from the cclmake() routine,
	 * the bitmap for that member is guaranteed to be freed.
	 * So we stomp a MCNIL value there, and call mcclear()
	 * to free any other bitmaps.
	 *
	 * Please note the structure assignment - your compiler may
	 * not like that.
	 */
	mcptr->mc_type = MCNIL;
	if (stacklevel) {
		status = FALSE;
		mlwrite(TEXT222);
/*			"Group not ended"	*/
	}

	if (status) {
		rtpcm = &tapcm[0];
		while (--mj >= 0) {
#if	LATTICE
			movmem(--mcptr, rtpcm, sizeof (MC));
#else
			*rtpcm = *--mcptr;
#endif
			rtpcm++;
		}
		rtpcm->mc_type = MCNIL;
	}
	else
	{
		(--mcptr)->mc_type = MCNIL;
		mcclear();
	}

	return (status);
}

/*
 * rmcstr -- Set up the replacement 'magic' array.  Note that if there
 *	are no meta-characters encountered in the replacement string,
 *	the array is never actually created - we will just use the
 *	character array rpat[] as the replacement string.
 */
int PASCAL NEAR rmcstr()
{
	RMC	*rmcptr;
	char	*patptr;
	int	pchr;
	int	status = TRUE;
	int	mj;

	patptr = &rpat[0];
	rmcptr = &rmcpat[0];
	mj = 0;
	rmagical = FALSE;

	while (*patptr && status == TRUE) {
		switch (*patptr) {
			case MC_DITTO:

				/* If there were non-magical characters in
				 * the string before reaching this character
				 * plunk it in the replacement array before
				 * processing the current meta-character.
				 */
				if (mj != 0) {
					rmcptr->mc_type = LITCHAR;
					if ((rmcptr->u.rstr = malloc(mj + 1)) == NULL) {
						mlwrite(TEXT94);
/*							"%%Out of memory" */
						status = FALSE;
						break;
					}
					bytecopy(rmcptr->u.rstr, patptr - mj, mj);
					rmcptr++;
					mj = 0;
				}
				rmcptr->mc_type = DITTO;
				rmcptr++;
				rmagical = TRUE;
				break;

			case MC_ESC:
				pchr = *(patptr + 1);	/* peek at next char.*/
				if (pchr <= '9' && pchr >= '1') {
					if (mj != 0) {
						rmcptr->mc_type = LITCHAR;
						if ((rmcptr->u.rstr = malloc(mj + 1)) == NULL) {
							mlwrite(TEXT94);
/*							"%%Out of memory" */
							status = FALSE;
							break;
						}
						bytecopy(rmcptr->u.rstr, patptr - mj, mj);
						rmcptr++;
						mj = 0;
					}
					rmcptr->mc_type = GROUP;
					rmcptr->u.group_no = pchr - '0';
					patptr++;
				}
				else
				{
					rmcptr->mc_type = LITCHAR;

					/* We malloc mj plus two here, instead
					 * of one, because we have to count the
					 * current character.
					 */
					if ((rmcptr->u.rstr = malloc(mj + 2)) == NULL) {
						mlwrite(TEXT94);
/*						"%%Out of memory" */
						status = FALSE;
						break;
					}

					bytecopy(rmcptr->u.rstr, patptr - mj, mj + 1);

					/* If MC_ESC is not the last character
					 * in the string, find out what it is
					 * escaping, and overwrite the last
					 * character with it.
					 */
					if (pchr != '\0') {
						*((rmcptr->u.rstr) + mj) = pchr;
						patptr++;
					}
					mj = 0;
				}

				rmcptr++;
				rmagical = TRUE;
				break;

			default:
				mj++;
		}
		patptr++;
	}

	if (rmagical && mj > 0) {
		rmcptr->mc_type = LITCHAR;
		if ((rmcptr->u.rstr = malloc(mj + 1)) == NULL) {
			mlwrite(TEXT94);
/*				"%%Out of memory" */
			status = FALSE;
		}
		bytecopy(rmcptr->u.rstr, patptr - mj, mj);
		rmcptr++;
	}

	rmcptr->mc_type = MCNIL;
	return (status);
}

/*
 * mcclear -- Free up any CCL bitmaps, and MCNIL the MC search arrays.
 */
VOID PASCAL NEAR mcclear()
{
	register MC	*mcptr;
	register int	j;

	mcptr = &mcpat[0];

	while (mcptr->mc_type != MCNIL) {
		if ((mcptr->mc_type == CCL) || (mcptr->mc_type == NCCL))
			free(mcptr->u.cclmap);
		mcptr++;
	}
	mcpat[0].mc_type = tapcm[0].mc_type = MCNIL;

	/*
	 * Remember that grpmatch[0] == patmatch.
	 */
	for (j = 0; j < MAXGROUPS; j++) {
		if (grpmatch[j] != NULL) {
			free(grpmatch[j]);
			grpmatch[j] = NULL;
		}
	}
	patmatch = NULL;
	group_count = 0;
	magical = FALSE;
}

/*
 * rmcclear -- Free up any strings, and MCNIL the RMC array.
 */
VOID PASCAL NEAR rmcclear()
{
	register RMC	*rmcptr;
	register int	j;

	rmcptr = &rmcpat[0];

	while (rmcptr->mc_type != MCNIL) {
		if (rmcptr->mc_type == LITCHAR)
			free(rmcptr->u.rstr);
		rmcptr++;
	}

	rmcpat[0].mc_type = MCNIL;
}

/*
 * mceq -- meta-character equality with a character.  In Kernighan & Plauger's
 *	Software Tools, this is the function omatch(), but i felt there were
 *	too many functions with the 'match' name already.
 */
int PASCAL NEAR	mceq(bc, mt)
int	bc;
MC	*mt;
{
	register int result;

	switch (mt->mc_type & MASKCLO) {
		case LITCHAR:
			result = eq(bc, (int) mt->u.lchar);
			break;

		case ANY:
			result = (bc != '\r');
			break;

		case CCL:
			if (!(result = biteq(bc, mt->u.cclmap))) {
				if ((curwp->w_bufp->b_mode & MDEXACT) == 0 &&
				    (isletter(bc))) {
					result = biteq(chcase(bc), mt->u.cclmap);
				}
			}
			break;

		case NCCL:
			result = !biteq(bc, mt->u.cclmap);

			if ((curwp->w_bufp->b_mode & MDEXACT) == 0 &&
			    (isletter(bc))) {
				result &= !biteq(chcase(bc), mt->u.cclmap);
			}
			break;

		default:
			mlwrite(TEXT95, mt->mc_type);
/*				"%%mceq: what is %d?" */
			result = FALSE;
			break;

	}	/* End of switch.*/

	return (result);
}

/*
 * cclmake -- create the bitmap for the character class.
 *	ppatptr is left pointing to the end-of-character-class character,
 *	so that a loop may automatically increment with safety.
 */
int PASCAL NEAR	cclmake(ppatptr, mcptr)
char	**ppatptr;
MC	*mcptr;
{
	BITMAP		bmap;
	register char	*patptr;
	register int	pchr, ochr;

#if	OS2
	if ((bmap = (BITMAP) malloc(HIBYTE_)) == NULL) {
		mlwrite(TEXT94);
/*			"%%Out of memory" */
		return FALSE;
	}

	memset(bmap, 0, HIBYTE_);
#else
	if ((bmap = (BITMAP) malloc(HIBYTE)) == NULL) {
		mlwrite(TEXT94);
/*			"%%Out of memory" */
		return FALSE;
	}

	memset(bmap, 0, HIBYTE);
#endif

	mcptr->u.cclmap = bmap;
	patptr = *ppatptr;
	ochr = MC_CCL;

	/*
	 * Test the initial character(s) in ccl for
	 * special cases - negate ccl, or an end ccl
	 * character as a first character.  Anything
	 * else gets set in the bitmap.
	 */
	if (*++patptr == MC_NCCL) {
		patptr++;
		mcptr->mc_type = NCCL;
	}
	else
		mcptr->mc_type = CCL;

	if ((pchr = *patptr) == MC_ECCL) {
		mlwrite(TEXT96);
/*			"%%No characters in character class" */
		free(bmap);
		return FALSE;
	}

	while (pchr != MC_ECCL && pchr != '\0') {
		switch (pchr) {
			/* Range character loses its meaning if it is
			 * the first or last character in the class.  We
			 * also treat it as un-ordinary if the order is
			 * wrong, e.g. "z-a".
			 */
			case MC_RCCL:
				pchr = *(patptr + 1);
				if (ochr == MC_CCL || pchr == MC_ECCL ||
				    ochr > pchr)
					setbit(MC_RCCL, bmap);
				else
				{
					do {
						setbit(++ochr, bmap);
					} while (ochr < pchr);
					patptr++;
				}
				break;

			/* Note: no break between case MC_ESC and the default.
			 */
			case MC_ESC:
				pchr = *++patptr;
			default:
				setbit(pchr, bmap);
				break;
		}
		ochr = pchr;
		pchr = *++patptr;
	}

	*ppatptr = patptr;

	if (pchr == '\0') {
		mlwrite(TEXT97);
/*			"%%Character class not ended" */
		free(bmap);
		return FALSE;
	}
	return TRUE;
}

/*
 * biteq -- is the character in the bitmap?
 */
int PASCAL NEAR	biteq(bc, cclmap)
int	bc;
BITMAP	cclmap;
{
	if (bc >= HICHAR)
		return FALSE;

	return ( (*(cclmap + (bc >> 3)) & BIT(bc & 7))? TRUE: FALSE );
}

/*
 * setbit -- Set a bit (ON only) in the bitmap.
 */
VOID PASCAL NEAR setbit(bc, cclmap)
int	bc;
BITMAP	cclmap;
{
	if (bc < HICHAR)
		*(cclmap + (bc >> 3)) |= BIT(bc & 7);
}
#endif
