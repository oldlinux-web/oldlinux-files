/*
 * The functions in this file are a general set of line management utilities.
 * They are the only routines that touch the text. They also touch the buffer
 * and window structures, to make sure that the necessary updating gets done.
 * There are routines in this file that handle the kill buffer too. It isn't
 * here for any good reason.
 *
 * Note that this code only updates the dot and mark values in the window list.
 * Since all the code acts on the current window, the buffer that we are
 * editing must be being displayed, which means that "b_nwnd" is non zero,
 * which means that the dot and mark values in the buffer headers are nonsense.
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

#define	BSIZE(a)	(a + NBLOCK - 1) & (~(NBLOCK - 1))

static long last_size = -1L;	/* last # of bytes yanked */

/*
 * This routine allocates a block of memory large enough to hold a LINE
 * containing "used" characters. Return a pointer to the new block, or
 * NULL if there isn't any memory left. Print a message in the message
 * line if no space.
 */

LINE *PASCAL NEAR lalloc(used)

register int used;

{
	register LINE	*lp;

	if ((lp = (LINE *)malloc(sizeof(LINE)+used)) == NULL) {
		mlwrite(TEXT94);
/*                      "%%Out of memory" */
		return(NULL);
	}
	lp->l_size = used;
	lp->l_used = used;
	return(lp);
}

/*
 * Delete line "lp". Fix all of the links that might point at it (they are
 * moved to offset 0 of the next line. Unlink the line from whatever buffer it
 * might be in. Release the memory. The buffers are updated too; the magic
 * conditions described in the above comments don't hold here.
 */
PASCAL NEAR lfree(lp)
register LINE	*lp;
{
	register BUFFER *bp;
	SCREEN *scrp;		/* screen to fix pointers in */
	register WINDOW *wp;
	register int cmark;		/* current mark */

	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {
		wp = scrp->s_first_window;
		while (wp != NULL) {
			if (wp->w_linep == lp)
				wp->w_linep = lp->l_fp;
			if (wp->w_dotp	== lp) {
				wp->w_dotp  = lp->l_fp;
				wp->w_doto  = 0;
			}
			for (cmark = 0; cmark < NMARKS; cmark++) {
				if (wp->w_markp[cmark] == lp) {
					wp->w_markp[cmark] = lp->l_fp;
					wp->w_marko[cmark] = 0;
				}
			}
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}

	bp = bheadp;
	while (bp != NULL) {
		if (bp->b_nwnd == 0) {
			if (bp->b_dotp	== lp) {
				bp->b_dotp = lp->l_fp;
				bp->b_doto = 0;
			}
			for (cmark = 0; cmark < NMARKS; cmark++) {
				if (bp->b_markp[cmark] == lp) {
					bp->b_markp[cmark] = lp->l_fp;
					bp->b_marko[cmark] = 0;
				}
			}
		}
		bp = bp->b_bufp;
	}
	lp->l_bp->l_fp = lp->l_fp;
	lp->l_fp->l_bp = lp->l_bp;
	free((char *) lp);
}

/*
 * This routine gets called when a character is changed in place in the current
 * buffer. It updates all of the required flags in the buffer and window
 * system. The flag used is passed as an argument; if the buffer is being
 * displayed in more than 1 window we change EDIT t HARD. Set MODE if the
 * mode line needs to be updated (the "*" has to be set).
 */
PASCAL NEAR lchange(flag)
register int	flag;
{
	register WINDOW *wp;
	SCREEN *scrp;		/* screen to fix pointers in */

	if (curbp->b_nwnd != 1) 		/* Ensure hard. 	*/
		flag = WFHARD;
	if ((curbp->b_flag&BFCHG) == 0) {	/* First change, so	*/
		flag |= WFMODE; 		/* update mode lines.	*/
		curbp->b_flag |= BFCHG;
	}

	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {
		/* make sure all the needed windows get this flag */
		wp = scrp->s_first_window;
		while (wp != NULL) {
			if (wp->w_bufp == curbp)
				wp->w_flag |= flag;
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}
}

PASCAL NEAR insspace(f, n)	/* insert spaces forward into text */

int f, n;	/* default flag and numeric argument */

{
	linsert(n, ' ');
	backchar(f, n);
}

/*
 * linstr -- Insert a string at the current point
 */

int PASCAL NEAR linstr(instr)
char	*instr;
{
	register int status;

	status = TRUE;
	if (instr != NULL)
		while (*instr) {
			status = ((*instr == '\r') ? lnewline(): linsert(1, *instr));

			/* Insertion error? */
			if (status != TRUE) {
				mlwrite(TEXT168);
/*                                      "%%Can not insert string" */
				break;
			}
			instr++;
		}
	return(status);
}

/*
 * Insert "n" copies of the character "c" at the current location of dot. In
 * the easy case all that happens is the text is stored in the line. In the
 * hard case, the line has to be reallocated. When the window list is updated,
 * take special care; I screwed it up once. You always update dot in the
 * current window. You update mark, and a dot in another window, if it is
 * greater than the place where you did the insert. Return TRUE if all is
 * well, and FALSE on errors.
 */

#if	PROTO
PASCAL NEAR linsert(int n, char c)
#else
PASCAL NEAR linsert(n, c)

int	n;
char	c;
#endif

{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register int	doto;
	register int	i;
	register WINDOW *wp;
	SCREEN *scrp;		/* screen to fix pointers in */
	int cmark;		/* current mark */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* a zero insert means do nothing! */
	if (n == 0)
		return(TRUE);

	/* Negative numbers of inserted characters are right out! */
	if (n < 1)
		return(FALSE);

	/* mark the current window's buffer as changed */
	lchange(WFEDIT);

	lp1 = curwp->w_dotp;			/* Current line 	*/
	if (lp1 == curbp->b_linep) {		/* At the end: special	*/
		if (curwp->w_doto != 0) {
			mlwrite(TEXT170);
/*                              "bug: linsert" */
			return(FALSE);
		}
		if ((lp2=lalloc(BSIZE(n))) == NULL)	/* Allocate new line	*/
			return(FALSE);
		lp2->l_used = n;
		lp3 = lp1->l_bp;		/* Previous line	*/
		lp3->l_fp = lp2;		/* Link in		*/
		lp2->l_fp = lp1;
		lp1->l_bp = lp2;
		lp2->l_bp = lp3;
		for (i=0; i<n; ++i)
			lp2->l_text[i] = c;
		curwp->w_dotp = lp2;
		curwp->w_doto = n;
		return(TRUE);
	}
	doto = curwp->w_doto;			/* Save for later.	*/
	if (lp1->l_used+n > lp1->l_size) {	/* Hard: reallocate	*/
		if ((lp2=lalloc(BSIZE(lp1->l_used+n))) == NULL)
			return(FALSE);
		lp2->l_used = lp1->l_used+n;
		cp1 = &lp1->l_text[0];
		cp2 = &lp2->l_text[0];
		while (cp1 != &lp1->l_text[doto])
			*cp2++ = *cp1++;
		cp2 += n;
		while (cp1 != &lp1->l_text[lp1->l_used])
			*cp2++ = *cp1++;
		lp1->l_bp->l_fp = lp2;
		lp2->l_fp = lp1->l_fp;
		lp1->l_fp->l_bp = lp2;
		lp2->l_bp = lp1->l_bp;
		free((char *) lp1);
	} else {				/* Easy: in place	*/
		lp2 = lp1;			/* Pretend new line	*/
		lp2->l_used += n;
		cp2 = &lp1->l_text[lp1->l_used];
		cp1 = cp2-n;
		while (cp1 != &lp1->l_text[doto])
			*--cp2 = *--cp1;
	}
	for (i=0; i<n; ++i)			/* Add the characters	*/
		lp2->l_text[doto+i] = c;
	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {

		wp = scrp->s_first_window;
		while (wp != NULL) {
			if (wp->w_linep == lp1)
				wp->w_linep = lp2;
			if (wp->w_dotp == lp1) {
				wp->w_dotp = lp2;
				if (wp==curwp || wp->w_doto>doto)
					wp->w_doto += n;
			}
			for (cmark = 0; cmark < NMARKS; cmark++) {
				if (wp->w_markp[cmark] == lp1) {
					wp->w_markp[cmark] = lp2;
					if (wp->w_marko[cmark] > doto)
						wp->w_marko[cmark] += n;
				}
			}
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}
	return(TRUE);
}

/*
 * Overwrite a character into the current line at the current position
 *
 */

#if	PROTO
PASCAL NEAR lowrite(char c)
#else
PASCAL NEAR lowrite(c)

char c;		/* character to overwrite on current position */
#endif

{
	if (curwp->w_doto < curwp->w_dotp->l_used &&
		(lgetc(curwp->w_dotp, curwp->w_doto) != '\t' ||
		 (curwp->w_doto) % 8 == 7))
			ldelete(1L, FALSE);
	return(linsert(1, c));
}

/*
 * lover -- Overwrite a string at the current point
 */

int PASCAL NEAR lover(ostr)

char	*ostr;

{
	register int status = TRUE;

	if (ostr != NULL)
		while (*ostr && status == TRUE) {
			status = ((*ostr == '\r') ? lnewline(): lowrite(*ostr));

			/* Insertion error? */
			if (status != TRUE) {
				mlwrite(TEXT172);
/*                                      "%%Out of memory while overwriting" */
				break;
			}
			ostr++;
		}
	return(status);
}

/*
 * Insert a newline into the buffer at the current location of dot in the
 * current window. The funny ass-backwards way it does things is not a botch;
 * it just makes the last line in the file not a special case. Return TRUE if
 * everything works out and FALSE on error (memory allocation failure). The
 * update of dot and mark is a bit easier then in the above case, because the
 * split forces more updating.
 */
int PASCAL NEAR lnewline()
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register int	doto;
	register WINDOW *wp;
	SCREEN *scrp;		/* screen to fix pointers in */
	int cmark;		/* current mark */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	lchange(WFHARD);
	lp1  = curwp->w_dotp;			/* Get the address and	*/
	doto = curwp->w_doto;			/* offset of "."	*/
	if ((lp2=lalloc(doto)) == NULL) 	/* New first half line	*/
		return(FALSE);
	cp1 = &lp1->l_text[0];			/* Shuffle text around	*/
	cp2 = &lp2->l_text[0];
	while (cp1 != &lp1->l_text[doto])
		*cp2++ = *cp1++;
	cp2 = &lp1->l_text[0];
	while (cp1 != &lp1->l_text[lp1->l_used])
		*cp2++ = *cp1++;
	lp1->l_used -= doto;
	lp2->l_bp = lp1->l_bp;
	lp1->l_bp = lp2;
	lp2->l_bp->l_fp = lp2;
	lp2->l_fp = lp1;

	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {

		wp = scrp->s_first_window;
		while (wp != NULL) {
			if (wp->w_linep == lp1)
				wp->w_linep = lp2;
			if (wp->w_dotp == lp1) {
				if (wp->w_doto < doto)
					wp->w_dotp = lp2;
				else
					wp->w_doto -= doto;
			}
			for (cmark = 0; cmark < NMARKS; cmark++) {
				if (wp->w_markp[cmark] == lp1) {
					if (wp->w_marko[cmark] < doto)
						wp->w_markp[cmark] = lp2;
					else
						wp->w_marko[cmark] -= doto;
				}
			}
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}
	return(TRUE);
}

/*

LDELETE:

	This function deletes "n" bytes, starting at dot. Positive n
deletes forward, negative n deletes backwords. It understands how to
deal with end of lines, and with two byte characters. It returns TRUE
if all of the characters were deleted, and FALSE if they were not
(because dot ran into the buffer end). The "kflag" is TRUE if the text
should be put in the kill buffer.

*/

PASCAL NEAR ldelete(n, kflag)

long n; 	/* # of chars to delete */
int kflag;	/* put killed text in kill buffer flag */

{
	register char	*cp1;
	register char	*cp2;
	register LINE	*dotp;
	register int	doto;
	register int	chunk;
	register WINDOW *wp;
	int cmark;		/* current mark */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* going Forward? */
	if (n >= 0) {

		while (n > 0) {
#if	DBCS
			/* never start forward on a 2 byte char */
			if (curwp->w_doto > 0 && is2byte(curwp->w_dotp->l_text,
			    &curwp->w_dotp->l_text[curwp->w_doto - 1])) {
				curwp->w_doto--;
				n++;
			}
#endif
			/* record the current point */
			dotp = curwp->w_dotp;
			doto = curwp->w_doto;
	
			/* can't delete past the end of the buffer */
			if (dotp == curbp->b_linep)
				return(FALSE);
	
			/* find out how many chars to delete on this line */
			chunk = dotp->l_used-doto;	/* Size of chunk.	*/
			if (chunk > n)
				chunk = n;
	
			/* if at the end of a line, merge with the next */
			if (chunk == 0) {
	
				/* flag that we are making a hard change */
				lchange(WFHARD);
				if (ldelnewline() == FALSE ||
				    (kflag != FALSE &&
				     kinsert(FORWARD, '\r')==FALSE))
					return(FALSE);
				--n;
				continue;
			}
	
			/* flag the fact we are changing the current line */
			lchange(WFEDIT);
	
			/* find the limits of the kill */
			cp1 = &dotp->l_text[doto];
			cp2 = cp1 + chunk;
#if	DBCS
			/* never leave half a character */
			if (is2byte(dotp->l_text, cp2 - 1)) {
				++chunk;
				++cp2;
				++n;
			}
#endif

			/* save the text to the kill buffer */
			if (kflag != FALSE) {
				while (cp1 != cp2) {
					if (kinsert(FORWARD, *cp1) == FALSE)
						return(FALSE);
					++cp1;
				}
				cp1 = &dotp->l_text[doto];
			}
	
			/* copy what is left of the line upward */
			while (cp2 != &dotp->l_text[dotp->l_used])
				*cp1++ = *cp2++;
			dotp->l_used -= chunk;
	
			/* fix any other windows with the same text displayed */
			wp = wheadp;
			while (wp != NULL) {
	
				/* reset the dot if needed */
				if (wp->w_dotp==dotp && wp->w_doto>=doto) {
					wp->w_doto -= chunk;
					if (wp->w_doto < doto)
						wp->w_doto = doto;
				}
	
				/* reset any marks if needed */
				for (cmark = 0; cmark < NMARKS; cmark++) {
					if (wp->w_markp[cmark]==dotp && wp->w_marko[cmark]>=doto) {
						wp->w_marko[cmark] -= chunk;
						if (wp->w_marko[cmark] < doto)
							wp->w_marko[cmark] = doto;
					}
				}
	
				/* onward to the next window */
				wp = wp->w_wndp;
			}
	
			/* indicate we have deleted chunk characters */
			n -= chunk;
		}
	} else {
		while (n < 0) {
#if	DBCS
			/* never start backwards on the
			   1st of a 2 byte character */
			if (curwp->w_doto > 1 && is2byte(curwp->w_dotp->l_text,
			    &curwp->w_dotp->l_text[curwp->w_doto-1])) {
				curwp->w_doto++;
				n--;
			}
#endif
			/* record the current point */
			dotp = curwp->w_dotp;
			doto = curwp->w_doto;
	
			/* can't delete past the beginning of the buffer */
			if (dotp == lforw(curbp->b_linep) && (doto == 0))
				return(FALSE);
	
			/* find out how many chars to delete on this line */
			chunk = doto;		/* Size of chunk.	*/
			if (chunk > -n)
				chunk = -n;
	
			/* if at the beginning of a line, merge with the last */
			if (chunk == 0) {
	
				/* flag that we are making a hard change */
				lchange(WFHARD);
				backchar(TRUE, 1);
				if (ldelnewline() == FALSE ||
				    (kflag != FALSE &&
				     kinsert(BACKWARD, '\r')==FALSE))
					return(FALSE);
				++n;
				continue;
			}
	
			/* flag the fact we are changing the current line */
			lchange(WFEDIT);
	
			/* find the limits of the kill */
			cp1 = &dotp->l_text[doto];
			cp2 = cp1 - chunk;
#if	DBCS
			if (is2byte(dotp->l_text, cp2 - 1)) {
				++chunk;
				--cp2;
				++n;
			}
#endif
	
			/* save the text to the kill buffer */
			if (kflag != FALSE) {
				while (cp1 > cp2) {
					if (kinsert(BACKWARD, *(--cp1)) == FALSE)
						return(FALSE);
				}
				cp1 = &dotp->l_text[doto];
			}
	
			/* copy what is left of the line downward */
			while (cp1 != &dotp->l_text[dotp->l_used])
				*cp2++ = *cp1++;
			dotp->l_used -= chunk;
			curwp->w_doto -= chunk;
	
			/* fix any other windows with the same text displayed */
			wp = wheadp;
			while (wp != NULL) {
	
				/* reset the dot if needed */
				if (wp->w_dotp==dotp && wp->w_doto>=doto) {
					wp->w_doto -= chunk;
					if (wp->w_doto < doto)
						wp->w_doto = doto;
				}
	
				/* reset any marks if needed */
				for (cmark = 0; cmark < NMARKS; cmark++) {
					if (wp->w_markp[cmark]==dotp && wp->w_marko[cmark]>=doto) {
						wp->w_marko[cmark] -= chunk;
						if (wp->w_marko[cmark] < doto)
							wp->w_marko[cmark] = doto;
					}
				}
	
				/* onward to the next window */
				wp = wp->w_wndp;
			}
	
			/* indicate we have deleted chunk characters */
			n += chunk;
		}
	}
	return(TRUE);
}

/* getctext:	grab and return a string with the text of
		the current line
*/

char *PASCAL NEAR getctext()

{
	register LINE *lp;	/* line to copy */
	register int size;	/* length of line to return */
	register char *sp;	/* string pointer into line */
	register char *dp;	/* string pointer into returned line */
	char rline[NSTRING];	/* line to return */

	/* find the contents of the current line and its length */
	lp = curwp->w_dotp;
	sp = lp->l_text;
	size = lp->l_used;
	if (size >= NSTRING)
		size = NSTRING - 1;

	/* copy it across */
	dp = rline;
	while (size--)
		*dp++ = *sp++;
	*dp = 0;
	return(rline);
}

/* putctext:	replace the current line with the passed in text	*/

PASCAL NEAR putctext(iline)

char *iline;	/* contents of new line */

{
	register int status;

	/* delete the current line */
	curwp->w_doto = 0;	/* starting at the beginning of the line */
	if ((status = killtext(TRUE, 1)) != TRUE)
		return(status);

	/* insert the new line */
	if ((status = linstr(iline)) != TRUE)
		return(status);
	status = lnewline();
	backline(TRUE, 1);
	return(status);
}

/*
 * Delete a newline. Join the current line with the next line. If the next line
 * is the magic header line always return TRUE; merging the last line with the
 * header line can be thought of as always being a successful operation, even
 * if nothing is done, and this makes the kill buffer work "right". Easy cases
 * can be done by shuffling data around. Hard cases require that lines be moved
 * about in memory. Return FALSE on error and TRUE if all looks ok. Called by
 * "ldelete" only.
 */
int PASCAL NEAR ldelnewline()
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register WINDOW *wp;
	SCREEN *scrp;		/* screen to fix pointers in */
	int cmark;		/* current mark */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	lp1 = curwp->w_dotp;
	lp2 = lp1->l_fp;
	if (lp2 == curbp->b_linep) {		/* At the buffer end.	*/
		if (lp1->l_used == 0)		/* Blank line.		*/
			lfree(lp1);
		return(TRUE);
	}
	if (lp2->l_used <= lp1->l_size-lp1->l_used) {
		cp1 = &lp1->l_text[lp1->l_used];
		cp2 = &lp2->l_text[0];
		while (cp2 != &lp2->l_text[lp2->l_used])
		*cp1++ = *cp2++;

		/* in all screens.... */
		scrp = first_screen;
		while (scrp) {

			wp = scrp->s_first_window;
			while (wp != NULL) {
				if (wp->w_linep == lp2)
					wp->w_linep = lp1;
				if (wp->w_dotp == lp2) {
					wp->w_dotp  = lp1;
					wp->w_doto += lp1->l_used;
				}
				for (cmark = 0; cmark < NMARKS; cmark++) {
					if (wp->w_markp[cmark] == lp2) {
						wp->w_markp[cmark]  = lp1;
						wp->w_marko[cmark] += lp1->l_used;
					}
				}
				wp = wp->w_wndp;
			}

			/* next screen! */
			scrp = scrp->s_next_screen;
		}

		lp1->l_used += lp2->l_used;
		lp1->l_fp = lp2->l_fp;
		lp2->l_fp->l_bp = lp1;
		free((char *) lp2);
		return(TRUE);
	}
	if ((lp3=lalloc(lp1->l_used+lp2->l_used)) == NULL)
		return(FALSE);
	cp1 = &lp1->l_text[0];
	cp2 = &lp3->l_text[0];
	while (cp1 != &lp1->l_text[lp1->l_used])
		*cp2++ = *cp1++;
	cp1 = &lp2->l_text[0];
	while (cp1 != &lp2->l_text[lp2->l_used])
		*cp2++ = *cp1++;
	lp1->l_bp->l_fp = lp3;
	lp3->l_fp = lp2->l_fp;
	lp2->l_fp->l_bp = lp3;
	lp3->l_bp = lp1->l_bp;

	/* in all screens.... */
	scrp = first_screen;
	while (scrp) {

		wp = scrp->s_first_window;
		while (wp != NULL) {
			if (wp->w_linep==lp1 || wp->w_linep==lp2)
				wp->w_linep = lp3;
			if (wp->w_dotp == lp1)
				wp->w_dotp  = lp3;
			else if (wp->w_dotp == lp2) {
				wp->w_dotp  = lp3;
				wp->w_doto += lp1->l_used;
			}
			for (cmark = 0; cmark < NMARKS; cmark++) {
				if (wp->w_markp[cmark] == lp1)
					wp->w_markp[cmark]  = lp3;
				else if (wp->w_markp[cmark] == lp2) {
					wp->w_markp[cmark]  = lp3;
					wp->w_marko[cmark] += lp1->l_used;
				}
			}
			wp = wp->w_wndp;
		}

		/* next screen! */
		scrp = scrp->s_next_screen;
	}

	free((char *) lp1);
	free((char *) lp2);
	return(TRUE);
}

/*	Add a new line to the end of the indicated buffer.
	return FALSE if we run out of memory
	note that this works on non-displayed buffers as well!
*/

#if	PROTO
int PASCAL NEAR addline(BUFFER *bp, char *text)
#else
int PASCAL NEAR addline(bp, text)

BUFFER *bp;	/* buffer to add text to */
char *text;	/* line to add */
#endif
{
	register LINE	*lp;
	register int	i;
	register int	ntext;

	/* allocate the memory to hold the line */
	ntext = strlen(text);
	if ((lp=lalloc(ntext)) == NULL)
		return(FALSE);

	/* copy the text into the new line */
	for (i=0; i<ntext; ++i)
		lputc(lp, i, text[i]);

	/* add the new line to the end of the buffer */
	bp->b_linep->l_bp->l_fp = lp;
	lp->l_bp = bp->b_linep->l_bp;
	bp->b_linep->l_bp = lp;
	lp->l_fp = bp->b_linep;

	/* if the point was at the end of the buffer,
	   move it to the beginning of the new line */
	if (bp->b_dotp == bp->b_linep)
		bp->b_dotp = lp;
	return(TRUE);
}

/*
 * Delete all of the text saved in the kill buffer. Called by commands when a
 * new kill context is being created. The kill buffer array is released, just
 * in case the buffer has grown to immense size. No errors.
 */

void kdelete()

{
	KILL *kp;	/* ptr to scan kill buffer chunk list */

	if (kbufh[kill_index] != NULL) {

		/* first, delete all the chunks */
		kbufp[kill_index] = kbufh[kill_index];
		while (kbufp[kill_index] != NULL) {
			kp = kbufp[kill_index]->d_next;
			free((char *)kbufp[kill_index]);
			kbufp[kill_index] = kp;
		}

		/* and reset all the kill buffer pointers */
		kbufh[kill_index] = kbufp[kill_index] = NULL;
		kskip[kill_index] = 0;
		kused[kill_index] = KBLOCK; 	        
	}
}

/*	next_kill:	advance to the next position in the kill ring,
			pushing the current kill buffer and clearing
			what will be the new kill buffer
*/

PASCAL NEAR next_kill()

{
	/* advance to the next kill ring entry */
	kill_index++;
	if (kill_index == NRING)
		kill_index = 0;

	/* and clear it, so it is ready for use */
	kdelete();
}

/*
 * Insert a character to the kill buffer, allocating new chunks as needed.
 * Return TRUE if all is well, and FALSE on errors.
 */

#if	PROTO
PASCAL NEAR kinsert(int direct, char c)
#else
PASCAL NEAR kinsert(direct, c)

int direct;	/* direction (FORWARD/BACKWARD) to insert characters */
char c;		/* character to insert in the kill buffer */
#endif

{
	KILL *nchunk;	/* ptr to newly malloced chunk */

	if (direct == FORWARD) {

		/* check to see if we need a new chunk */
		if (kused[kill_index] >= KBLOCK) {
			if ((nchunk = (KILL *)malloc(sizeof(KILL))) == NULL)
				return(FALSE);
			if (kbufh[kill_index] == NULL)	/* set head ptr if first time */
				kbufh[kill_index] = nchunk;
			if (kbufp[kill_index] != NULL)	/* point the current to this new one */
				kbufp[kill_index]->d_next = nchunk;
			kbufp[kill_index] = nchunk;
			kbufp[kill_index]->d_next = NULL;
			kused[kill_index] = 0;
		}
	
		/* and now insert the character */
		kbufp[kill_index]->d_chunk[kused[kill_index]++] = c;
	} else {
		/* BACKWARDS */
		/* check to see if we need a new chunk */
		if (kskip[kill_index] == 0) {
			if ((nchunk = (KILL *)malloc(sizeof(KILL))) == NULL)
				return(FALSE);
			if (kbufh[kill_index] == NULL) {	/* set head ptr if first time */
				kbufh[kill_index] = nchunk;
				kbufp[kill_index] = nchunk;
				kskip[kill_index] = KBLOCK;
				kused[kill_index] = KBLOCK;
				nchunk->d_next = (KILL *)NULL;
			} else {
				nchunk->d_next = kbufh[kill_index];
				kbufh[kill_index] = nchunk;
				kskip[kill_index] = KBLOCK;
			}
		}
	
		/* and now insert the character */
		kbufh[kill_index]->d_chunk[--kskip[kill_index]] = c;
	}
	return(TRUE);
}

/*
 * Yank text back from the kill buffer. This is really easy. All of the work
 * is done by the standard insert routines. All you do is run the loop, and
 * check for errors. Bound to "C-Y".
 */

#define	Char_insert(a)	(a == '\r' ? lnewline() : linsert(1, a))

PASCAL NEAR yank(f, n)

int f,n;	/* prefix flag and argument */

{
	register int counter;	/* counter into kill buffer data */
	register char *sp;	/* pointer into string to insert */
	short int curoff;	/* storage for line before yanking */
	LINE *curline;
	KILL *kptr;		/* pointer into kill buffer */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (n < 0)
		return(FALSE);

	/* make sure there is something to yank */
	if (kbufh[kill_index] == NULL) {
		last_size = 0L;
		return(TRUE);		/* not an error, just nothing */
	}

	/*
	 * Save the local pointers to hold global ".".
	 */
	if (yankflag) {
		/* Find the *previous* line, since the line we are on
		 * may disappear due to re-allocation.  This works even
		 * if we are on the first line of the file.
		 */
		curline = lback(curwp->w_dotp);
		curoff = curwp->w_doto;
	}

	/* for each time.... */
	while (n--) {
		last_size = 0L;
		if (kskip[kill_index] > 0) {
			kptr = kbufh[kill_index];
			sp = &(kptr->d_chunk[kskip[kill_index]]);
			counter = kskip[kill_index];
			while (counter++ < KBLOCK) {
				Char_insert(*sp);
				last_size++;
				++sp;
			}
			kptr = kptr->d_next;
		} else {
			kptr = kbufh[kill_index];
		}
	
		if (kptr != (KILL *)NULL) {
			while (kptr != kbufp[kill_index]) {
				sp = kptr->d_chunk;
				for(counter = 0; counter < KBLOCK; counter++) {
					Char_insert(*sp);
					last_size++;
					++sp;
				}
				kptr = kptr->d_next;
			}
			counter = kused[kill_index];
			sp = kptr->d_chunk;
			while (counter--) {
				Char_insert(*sp);
				last_size++;
				++sp;
			}
		}
	}

	/* If requested, set global "." back to the
	 * beginning of the yanked text.
	 */
	if (yankflag) {
		curwp->w_dotp = lforw(curline);
		curwp->w_doto = curoff;
	}
	thisflag |= CFYANK;
	return(TRUE);
}

PASCAL NEAR cycle_ring(f, n)

int f,n;	/* prefix flag and argument */

{
	register int orig_index;	/* original kill_index */

	/* if there is an argument, cycle the kill index */
	if (f) {
		while (n) {
			orig_index = kill_index;
			do {
				kill_index--;
				if (kill_index < 0)
					kill_index = NRING - 1;
			} while ((orig_index != kill_index) &&
				(kbufh[kill_index] == (KILL *)NULL));
			n--;
		}
	}
}

PASCAL NEAR yank_pop(f, n)

int f,n;	/* prefix flag and argument */

{
	/* defaulted non first call will cycle by 1 */
	if ((lastflag & CFYANK) && (f == FALSE)) {
		f = TRUE;
		n = 1;
	}

	/* cycle the kill ring appropriately */
	cycle_ring(f, n);

	/* if not the first consectutive time, delete the last yank */
	if ((lastflag & CFYANK))
		ldelete(-last_size, FALSE);

	/* and insert the current kill buffer */
	return(yank(FALSE, 1));
}

PASCAL NEAR clear_ring(f, n)

int f,n;	/* prefix flag and argument */

{
	register int index;

	for (index = 0; index < NRING; index++)
		next_kill();
	mlwrite(TEXT228);
/*		"[Kill ring cleared]" */
	return(TRUE);
}

#if	0
dispkill()

{
	KILL *kptr;
	int index;
	char *sp;
	int counter;

	if (kbufh[kill_index] == (KILL *)NULL) {
		printf("<EMPTY>\n");
		return;
	}

	index = 1;
	if (kskip[kill_index] > 0) {
		kptr = kbufh[kill_index];
		printf("kskip[kill_index] = %d\nBLOCK %d <", kskip[kill_index], index++);
		sp = &(kptr->d_chunk[kskip[kill_index]]);
		counter = kskip[kill_index];
		while (counter++ < KBLOCK) {
			putchar(*sp++);
		}
		printf(">\n");
		kptr = kptr->d_next;
	} else {
		kptr = kbufh[kill_index];
	}

	if (kptr != (KILL *)NULL) {
		while (kptr != kbufp[kill_index]) {
			printf("BLOCK %d <%255s>\n", index++, kptr->d_chunk);
			kptr = kptr->d_next;
		}
		printf("BLOCK %d <", index++);
		counter = kused[kill_index];
		sp = kptr->d_chunk;
		while (counter--) {
			putchar(*sp++);
		}
		printf(">\nkused[kill_index] = %d\n", kused[kill_index]);
	}

}
#endif
