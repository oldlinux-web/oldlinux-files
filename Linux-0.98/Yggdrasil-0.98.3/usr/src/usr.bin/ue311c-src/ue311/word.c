/*
 * The routines in this file implement commands that work word or a
 * paragraph at a time.  There are all sorts of word mode commands.  If I
 * do any sentence mode commands, they are likely to be put in this file. 
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

/* Word wrap on n-spaces. Back-over whatever precedes the point on the current
 * line and stop on the first word-break or the beginning of the line. If we
 * reach the beginning of the line, jump back to the end of the word and start
 * a new line.	Otherwise, break the line at the word-break, eat it, and jump
 * back to the end of the word. Make sure we force the display back to the
 * left edge of the current window
 * Returns TRUE on success, FALSE on errors.
 */
PASCAL NEAR wrapword(f, n)

int f;		/* default flag */
int n;		/* numeric argument */
 
{
	register int cnt;	/* size of word wrapped to next line */
	register int c;		/* charector temporary */

	/* backup from the <NL> 1 char */
	if (!backchar(FALSE, 1))
		return(FALSE);

	/* back up until we aren't in a word,
	   make sure there is a break in the line */
	cnt = 0;
	while (((c = lgetc(curwp->w_dotp, curwp->w_doto)) != ' ')
				&& (c != '\t')) {
		cnt++;
		if (!backchar(FALSE, 1))
			return(FALSE);
		/* if we make it to the beginning, start a new line */
		if (curwp->w_doto == 0) {
			gotoeol(FALSE, 0);
			return(lnewline());
		}
	}

	/* delete the forward white space */
	if (!forwdel(0, 1))
		return(FALSE);

	/* put in a end of line */
	if (!lnewline())
		return(FALSE);

	/* and past the first word */
	while (cnt-- > 0) {
		if (forwchar(FALSE, 1) == FALSE)
			return(FALSE);
	}

	/* make sure the display is not horizontally scrolled */
	if (curwp->w_fcol != 0) {
		curwp->w_fcol = 0;
		curwp->w_flag |= WFHARD | WFMOVE | WFMODE;
	}

	return(TRUE);
}

/*
 * Move the cursor backward by "n" words. All of the details of motion are
 * performed by the "backchar" and "forwchar" routines. Error if you try to
 * move beyond the buffers.
 */
PASCAL NEAR backword(f, n)

int f,n;	/* prefix flag and argument */

{
	if (n < 0)
		return(forwword(f, -n));
	if (backchar(FALSE, 1) == FALSE)
		return(FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return(FALSE);
		}
		while (inword() != FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return(FALSE);
		}
	}
	return(forwchar(FALSE, 1));
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwchar". Error if you try and move beyond the buffer's end.
 */
PASCAL NEAR forwword(f, n)

int f,n;	/* prefix flag and argument */

{
	if (n < 0)
		return(backword(f, -n));
	while (n--) {
		/* scan through the current word */
		while (inword() == TRUE) {
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
		}

		/* scan through the intervening white space */
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
		}
	}
	return(TRUE);
}

/*
 * Move forward to the end of the nth next word. Error if you move past
 * the end of the buffer.
 */
PASCAL NEAR endword(f, n)

int f,n;	/* prefix flag and argument */

{
	if (n < 0)
		return(backword(f, -n));
	while (n--) {
		/* scan through the intervening white space */
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
		}

		/* scan through the current word */
		while (inword() == TRUE) {
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
		}
	}
	return(TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move,
 * convert any characters to upper case. Error if you try and move beyond the
 * end of the buffer. Bound to "M-U".
 */
PASCAL NEAR upperword(f, n)

int f,n;	/* prefix flag and argument */

{
	int c;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (n < 0)
		return(FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (islower(c)) {
				c = upperc(c);
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
		}
	}
	return(TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert characters to lower case. Error if you try and move over the end of
 * the buffer. Bound to "M-L".
 */
PASCAL NEAR lowerword(f, n)

int f,n;	/* prefix flag and argument */

{
	int c;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (n < 0)
		return(FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (isupper(c)) {
				c = lowerc(c);
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
		}
	}
	return(TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert the first character of the word to upper case, and subsequent
 * characters to lower case. Error if you try and move past the end of the
 * buffer. Bound to "M-C".
 */
PASCAL NEAR capword(f, n)

int f,n;	/* prefix flag and argument */

{
	int c;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (n < 0)
		return(FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
		}
		if (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (islower(c)) {
				c = upperc(c);
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
			while (inword() != FALSE) {
				c = lgetc(curwp->w_dotp, curwp->w_doto);
				if (isupper(c)) {
					c = lowerc(c);
					lputc(curwp->w_dotp, curwp->w_doto, c);
					lchange(WFHARD);
				}
				if (forwchar(FALSE, 1) == FALSE)
					return(FALSE);
			}
		}
	}
	return(TRUE);
}

/*
 * Kill forward by "n" words. Remember the location of dot. Move forward by
 * the right number of words. Put dot back where it was and issue the kill
 * command for the right number of characters. With a zero argument, just
 * kill one word and no whitespace. Bound to "M-D".
 */
PASCAL NEAR delfword(f, n)

int f,n;	/* prefix flag and argument */

{
	register LINE	*dotp;	/* original cursor line */
	register int	doto;	/*	and row */
	register int c;		/* temp char */
	long size;		/* # of chars to delete */

	/* don't allow this command if we are in read only mode */
	if (curbp->b_mode&MDVIEW)
		return(rdonly());

	/* ignore the command if there is a negative argument */
	if (n < 0)
		return(FALSE);

	/* Clear the kill buffer if last command wasn't a kill */
	if ((lastflag&CFKILL) == 0)
		next_kill();
	thisflag |= CFKILL;	/* this command is a kill */

	/* save the current cursor position */
	dotp = curwp->w_dotp;
	doto = curwp->w_doto;

	/* figure out how many characters to give the axe */
	size = 0;

	/* get us into a word.... */
	while (inword() == FALSE) {
		if (forwchar(FALSE, 1) == FALSE)
			return(FALSE);
		++size;
	}

	if (n == 0) {
		/* skip one word, no whitespace! */
		while (inword() == TRUE) {
			if (forwchar(FALSE, 1) == FALSE)
				return(FALSE);
			++size;
		}
	} else {
		/* skip n words.... */
		while (n--) {
	
			/* if we are at EOL; skip to the beginning of the next */
			while (curwp->w_doto == llength(curwp->w_dotp)) {
				if (forwchar(FALSE, 1) == FALSE)
					return(FALSE);
				++size;
			}
	
			/* move forward till we are at the end of the word */
			while (inword() == TRUE) {
				if (forwchar(FALSE, 1) == FALSE)
					return(FALSE);
				++size;
			}
	
			/* if there are more words, skip the interword stuff */
			if (n != 0)
				while (inword() == FALSE) {
					if (forwchar(FALSE, 1) == FALSE)
						return(FALSE);
					++size;
				}
		}
	
		/* skip whitespace and newlines */
		while ((curwp->w_doto == llength(curwp->w_dotp)) ||
			((c = lgetc(curwp->w_dotp, curwp->w_doto)) == ' ') ||
			(c == '\t')) {
				if (forwchar(FALSE, 1) == FALSE)
					break;
				++size;
		}
	}

	/* restore the original position and delete the words */
	curwp->w_dotp = dotp;
	curwp->w_doto = doto;
	return(ldelete(size, TRUE));
}

/*
 * Kill backwards by "n" words. Move backwards by the desired number of words,
 * counting the characters. When dot is finally moved to its resting place,
 * fire off the kill command. Bound to "M-Rubout" and to "M-Backspace".
 */
PASCAL NEAR delbword(f, n)

int f,n;	/* prefix flag and argument */

{
	long size;

	/* don't allow this command if we are in read only mode */
	if (curbp->b_mode&MDVIEW)
		return(rdonly());

	/* ignore the command if there is a nonpositive argument */
	if (n <= 0)
		return(FALSE);

	/* Clear the kill buffer if last command wasn't a kill */
	if ((lastflag&CFKILL) == 0)
		next_kill();
	thisflag |= CFKILL;	/* this command is a kill */

	if (backchar(FALSE, 1) == FALSE)
		return(FALSE);
	size = 0;
	while (n--) {
		while (inword() == FALSE) {
			if (backchar(FALSE, 1) == FALSE)
				return(FALSE);
			++size;
		}
		while (inword() != FALSE) {
			++size;
			if (backchar(FALSE, 1) == FALSE)
				goto bckdel;
		}
	}
	if (forwchar(FALSE, 1) == FALSE)
		return(FALSE);
bckdel:	if (forwchar(FALSE, size) == FALSE)
		return(FALSE);
	return(ldelete(-size, TRUE));
}

/*
 * Return TRUE if the character at dot is a character that is considered to be
 * part of a word. The default word character list is hard coded. If $wchars
 * has been set by the user, use that instead
 */

PASCAL NEAR inword()

{
	register int c;

	/* the end of a line is never in a word */
	if (curwp->w_doto == llength(curwp->w_dotp))
		return(FALSE);

	/* grab the word to check */
	c = lgetc(curwp->w_dotp, curwp->w_doto);

	/* if we are using the table.... */
	if (wlflag)
		return(wordlist[c]);

	/* else use the default hard coded check */
	if (isletter(c))
		return(TRUE);
	if (c>='0' && c<='9')
		return(TRUE);
	if (c == '_')
		return(TRUE);
	return(FALSE);
}

PASCAL NEAR fillpara(f, n)	/* Fill the current paragraph according to the
			   current fill column */

int f, n;	/* Default flag and Numeric argument */

{
	register char *pp;	/* ptr into paragraph being reformed */
	register char *para;	/* malloced buffer for paragraph */
	register LINE *lp;	/* ptr to current line */
	register int lsize;	/* bytes in current line */
	register char *txtptr;	/* ptr into current line */
	LINE *ptline;		/* line the point started on */
	int ptoff;		/* offset of original point */
	int back;		/* # of characters from origin point to eop */
	int status;		/* return status from linstr() */
	int psize;		/* byte size of paragraph */
	LINE *bop;		/* ptr to beg of paragraph */
	LINE *eop;		/* pointer to line just past EOP */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (fillcol == 0) {	/* no fill column set */
		mlwrite(TEXT98);
/*                      "No fill column set" */
		return(FALSE);
	}

	/* save the original point */
	ptline = curwp->w_dotp;
	ptoff = curwp->w_doto;

	/* record the pointer to the line just past the EOP */
	gotoeop(FALSE, 1);
	eop = lforw(curwp->w_dotp);

	/* and back top the beginning of the paragraph */
	gotobop(FALSE, 1);
	bop = lp = curwp->w_dotp;

	/* ok, how big is this paragraph? */
	psize = 0;
	while (lp != eop) {
		psize += lp->l_used + 1;
		lp = lp->l_fp;
	}

	/* must have size! */
	if (psize == 0)
		return(TRUE);

	/* create a buffer to hold this stuff */
	para = malloc(psize + 10);	/* this prabably could be + 1 */
	if (para == NULL) {
		mlwrite(TEXT94);
/*                      "%%Out of memory" */
		return(FALSE);
	}

	/* now, grab all the text into a string */
	back = 0;	/* counting the distance to backup when done */
	lp = bop;
	pp = para;
	while (lp != eop) {
		lsize = lp->l_used;
		if (back == 0) {
			if (lp == ptline)
				back = lsize - ptoff + 1;
		} else
			back += lsize + 1;
		txtptr = lp->l_text;
		while (lsize--)			/* copy a line */
			*pp++ = *txtptr++;
		*pp++ = ' ';			/* turn the NL to a space */
		lp = lp->l_fp;
		lfree(lp->l_bp);		/* free the old line */
	}
	*(--pp) = 0;	/* truncate the last space */

	/* reformat the paragraph in the buffer */
	reform(para);

	/* insert the reformatted paragraph back into the current buffer */
	status = linstr(para);
	lnewline();		/* add the last newline to our paragraph */
	if (status == TRUE)	/* reposition us to the same place */
		status = backchar(FALSE, back);

	/* make sure the display is not horizontally scrolled */
	if (curwp->w_fcol != 0) {
		curwp->w_fcol = 0;
		curwp->w_flag |= WFHARD | WFMOVE | WFMODE;
	}

	/* free the buffer and return */
	free(para);
	return(status);
}

PASCAL NEAR reform(para)	/* reformat a paragraph as stored in a string */

char *para;	/* string buffer containing paragraph */

{
	register char *sp;		/* string scan pointer */
	register int col;		/* current colomn position */
	register char *lastword;	/* ptr to end of last word */

	/* scan string, replacing some whitespace with newlines */
	sp = para;
	lastword = para;
	col = 0;
	while (*sp) {
		/* if we are at white space.... */
		if ((*sp == ' ') || (*sp == '\t')) {
			if (*sp == '\t')
				col = (col + 8) & (~7);
			else
				col++;

			/* break on whitespace? */
			if (col > fillcol) {
				*sp = '\r';
				col = 0;
			}

			/* onward, resetting the most recent begin of word */
			++sp;
			lastword = sp;

		} else {	/* a non-blank to process */

			++sp;
			++col;
			if (col > fillcol) {
				/* line break here! */
				if ((lastword > para) &&
				   (*(lastword - 1) != '\r')) {
				   	*(lastword - 1) = '\r';
				   	sp = lastword;
				   	col = 0;
				}
			}
		}
	}
}

PASCAL NEAR killpara(f, n)	/* delete n paragraphs starting with the current one */

int f;	/* default flag */
int n;	/* # of paras to delete */

{
	register int status;	/* returned status of functions */

	while (n--) {		/* for each paragraph to delete */

		/* mark out the end and beginning of the para to delete */
		gotoeop(FALSE, 1);

		/* set the mark here */
		curwp->w_markp[0] = curwp->w_dotp;
		curwp->w_marko[0] = curwp->w_doto;

		/* go to the beginning of the paragraph */
		gotobop(FALSE, 1);
		curwp->w_doto = 0;	/* force us to the beginning of line */
	
		/* and delete it */
		if ((status = killregion(FALSE, 1)) != TRUE)
			return(status);

		/* and clean up the 2 extra lines */
		ldelete(2L, TRUE);
	}
	return(TRUE);
}

/*	wordcount:	count the # of words in the marked region,
			along with average word sizes, # of chars, etc,
			and report on them.			*/

PASCAL NEAR wordcount(f, n)

int f, n;	/* ignored numeric arguments */

{
	register LINE *lp;	/* current line to scan */
	register int offset;	/* current char to scan */
	long size;		/* size of region left to count */
	register int ch;	/* current character to scan */
	register int wordflag;	/* are we in a word now? */
	register int lastword;	/* were we just in a word? */
	long nwords;		/* total # of words */
	long nchars;		/* total number of chars */
	int nlines;		/* total number of lines in region */
	int avgch;		/* average number of chars/word */
	int status;		/* status return code */
	REGION region;		/* region to look at */

	/* make sure we have a region to count */
	if ((status = getregion(&region)) != TRUE)
		return(status);
	lp = region.r_linep;
	offset = region.r_offset;
	size = region.r_size;

	/* count up things */
	lastword = FALSE;
	nchars = 0L;
	nwords = 0L;
	nlines = 0;
	while (size--) {

		/* get the current character */
		if (offset == llength(lp)) {	/* end of line */
			ch = '\r';
			lp = lforw(lp);
			offset = 0;
			++nlines;
		} else {
			ch = lgetc(lp, offset);
			++offset;
		}

		/* and tabulate it */
		wordflag = ((ch >= 'a' && ch <= 'z') ||
			    (ch >= 'A' && ch <= 'Z') ||
			    (ch >= '0' && ch <= '9'));
		if (wordflag == TRUE && lastword == FALSE)
			++nwords;
		lastword = wordflag;
		++nchars;
	}

	/* and report on the info */
	if (nwords > 0L)
		avgch = (int)((100L * nchars) / nwords);
	else
		avgch = 0;

	mlwrite(TEXT100,
/*              "Words %D Chars %D Lines %d Avg chars/word %f" */
		nwords, nchars, nlines + 1, avgch);
	return(TRUE);
}
