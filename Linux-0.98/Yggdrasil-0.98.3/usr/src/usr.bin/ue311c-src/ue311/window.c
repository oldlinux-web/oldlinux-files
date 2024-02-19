/*
 * Window management. Some of the functions are internal, and some are
 * attached to keys that the user actually types.
 */

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

/*
 * Reposition dot in the current window to line "n". If the argument is
 * positive, it is that line. If it is negative it is that line from the
 * bottom. If it is 0 the window is centered (this is what the standard
 * redisplay code does). With no argument it defaults to 0. Bound to M-!.
 */
PASCAL NEAR reposition(f, n)

int f, n;	/* prefix flag and argument */

{
    if (f == FALSE)	/* default to 0 to center screen */
	n = 0;
    curwp->w_force = n;
    curwp->w_flag |= WFFORCE;
    return (TRUE);
    }

/*
 * Refresh the screen. With no argument, it just does the refresh. With an
 * argument it recenters "." in the current window. Bound to "C-L".
 */
PASCAL NEAR refresh(f, n)

int f, n;	/* prefix flag and argument */

{
    if (f == FALSE)
	sgarbf = TRUE;
    else
	{
	curwp->w_force = 0;		/* Center dot. */
	curwp->w_flag |= WFFORCE;
	}

    return (TRUE);
    }

/*
 * The command make the next window (next => down the screen) the current
 * window. There are no real errors, although the command does nothing if
 * there is only 1 window on the screen. Bound to "C-X C-N".
 *
 * with an argument this command finds the <n>th window from the top
 *
 */
PASCAL NEAR nextwind(f, n)

int f, n;	/* default flag and numeric argument */

{
	register WINDOW *wp;
	register int nwindows;		/* total number of windows */

	if (f) {

		/* first count the # of windows */
		wp = wheadp;
		nwindows = 1;
		while (wp->w_wndp != NULL) {
			nwindows++;
			wp = wp->w_wndp;
		}

		/* if the argument is negative, it is the nth window
		   from the bottom of the screen			*/
		if (n < 0)
			n = nwindows + n + 1;

		/* if an argument, give them that window from the top */
		if (n > 0 && n <= nwindows) {
			wp = wheadp;
			while (--n)
				wp = wp->w_wndp;
		} else {
			mlwrite(TEXT203);
/*				"Window number out of range" */
			return(FALSE);
		}
	} else
		if ((wp = curwp->w_wndp) == NULL)
			wp = wheadp;
	curwp = wp;
	curbp = wp->w_bufp;
	upmode();
	return (TRUE);
}

/*
 * This command makes the previous window (previous => up the screen) the
 * current window. There arn't any errors, although the command does not do a
 * lot if there is 1 window.
 */
PASCAL NEAR prevwind(f, n)

int f,n;	/* prefix flag and argument */

{
	register WINDOW *wp1;
	register WINDOW *wp2;

	/* if we have an argument, we mean the nth window from the bottom */
	if (f)
		return(nextwind(f, -n));

	wp1 = wheadp;
	wp2 = curwp;

	if (wp1 == wp2)
		wp2 = NULL;

	while (wp1->w_wndp != wp2)
		wp1 = wp1->w_wndp;

	curwp = wp1;
	curbp = wp1->w_bufp;
	upmode();
	return (TRUE);
}

/*
 * This command moves the current window down by "arg" lines. Recompute the
 * top line in the window. The move up and move down code is almost completely
 * the same; most of the work has to do with reframing the window, and picking
 * a new dot. We share the code by having "move down" just be an interface to
 * "move up". Magic. Bound to "C-X C-N".
 */
PASCAL NEAR mvdnwind(f, n)

int f, n;	/* prefix flag and argument */

{
	return (mvupwind(f, -n));
}

/*
 * Move the current window up by "arg" lines. Recompute the new top line of
 * the window. Look to see if "." is still on the screen. If it is, you win.
 * If it isn't, then move "." to center it in the new framing of the window
 * (this command does not really move "."; it moves the frame). Bound to
 * "C-X C-P".
 */
PASCAL NEAR mvupwind(f, n)

int f, n;	/* prefix flag and argument */

{
    register LINE *lp;
    register int i;

    lp = curwp->w_linep;

    if (n < 0)
	{
	while (n++ && lp!=curbp->b_linep)
	    lp = lforw(lp);
	}
    else
	{
	while (n-- && lback(lp)!=curbp->b_linep)
	    lp = lback(lp);
	}

    curwp->w_linep = lp;
    curwp->w_flag |= WFHARD;		/* Mode line is OK. */

    for (i = 0; i < curwp->w_ntrows; ++i)
	{
	if (lp == curwp->w_dotp)
	    return (TRUE);
	if (lp == curbp->b_linep)
	    break;
	lp = lforw(lp);
	}

    lp = curwp->w_linep;
    i  = curwp->w_ntrows/2;

    while (i-- && lp != curbp->b_linep)
	lp = lforw(lp);

    curwp->w_dotp  = lp;
    curwp->w_doto  = 0;
    return (TRUE);
    }

/*
 * This command makes the current window the only window on the screen. Bound
 * to "C-X 1". Try to set the framing so that "." does not have to move on the
 * display. Some care has to be taken to keep the values of dot and mark in
 * the buffer structures right if the distruction of a window makes a buffer
 * become undisplayed.
 */
PASCAL NEAR onlywind(f, n)

int f,n;	/* prefix flag and argument */

{
	register WINDOW *wp;
	register LINE	*lp;
	register int	i;
	int cmark;		/* current mark */

	while (wheadp != curwp) {
		wp = wheadp;
		first_screen->s_first_window = wheadp = wp->w_wndp;
		if (--wp->w_bufp->b_nwnd == 0) {
			wp->w_bufp->b_dotp  = wp->w_dotp;
			wp->w_bufp->b_doto  = wp->w_doto;
			for (cmark = 0; cmark < NMARKS; cmark++) {
				wp->w_bufp->b_markp[cmark] = wp->w_markp[cmark];
				wp->w_bufp->b_marko[cmark] = wp->w_marko[cmark];
			}
			wp->w_bufp->b_fcol  = wp->w_fcol;
		}
		free((char *) wp);
	}
	while (curwp->w_wndp != NULL) {
		wp = curwp->w_wndp;
		curwp->w_wndp = wp->w_wndp;
		if (--wp->w_bufp->b_nwnd == 0) {
			wp->w_bufp->b_dotp  = wp->w_dotp;
			wp->w_bufp->b_doto  = wp->w_doto;
			for (cmark = 0; cmark < NMARKS; cmark++) {
				wp->w_bufp->b_markp[cmark] = wp->w_markp[cmark];
				wp->w_bufp->b_marko[cmark] = wp->w_marko[cmark];
			}
			wp->w_bufp->b_fcol  = wp->w_fcol;
		}
		free((char *) wp);
	}
	lp = curwp->w_linep;
	i  = curwp->w_toprow;
	while (i!=0 && lback(lp)!=curbp->b_linep) {
		--i;
		lp = lback(lp);
	}
	curwp->w_toprow = 0;
	curwp->w_ntrows = term.t_nrow-1;
	curwp->w_linep	= lp;
	curwp->w_flag  |= WFMODE|WFHARD;
	return (TRUE);
}

/*
 * Delete the current window, placing its space in the window above,
 * or, if it is the top window, the window below. Bound to C-X 0.
 */

PASCAL NEAR delwind(f,n)

int f, n;	/* arguments are ignored for this command */

{
	register WINDOW *wp;	/* window to recieve deleted space */
	register WINDOW *lwp;	/* ptr window before curwp */
	register int target;	/* target line to search for */
	int cmark;		/* current mark */

	/* if there is only one window, don't delete it */
	if (wheadp->w_wndp == NULL) {
		mlwrite(TEXT204);
/*			"Can not delete this window" */
		return(FALSE);
	}

	/* find window before curwp in linked list */
	wp = wheadp;
	lwp = NULL;
	while (wp != NULL) {
		if (wp == curwp)
			break;
		lwp = wp;
		wp = wp->w_wndp;
	}

	/* find recieving window and give up our space */
	wp = wheadp;
	if (curwp->w_toprow == 0) {
		/* find the next window down */
		target = curwp->w_ntrows + 1;
		while (wp != NULL) {
			if (wp->w_toprow == target)
				break;
			wp = wp->w_wndp;
		}
		if (wp == NULL)
			return(FALSE);
		wp->w_toprow = 0;
		wp->w_ntrows += target;
	} else {
		/* find the next window up */
		target = curwp->w_toprow - 1;
		while (wp != NULL) {
			if ((wp->w_toprow + wp->w_ntrows) == target)
				break;
			wp = wp->w_wndp;
		}
		if (wp == NULL)
			return(FALSE);
		wp->w_ntrows += 1 + curwp->w_ntrows;
	}

	/* get rid of the current window */
	if (--curwp->w_bufp->b_nwnd == 0) {
		curwp->w_bufp->b_dotp  = curwp->w_dotp;
		curwp->w_bufp->b_doto  = curwp->w_doto;
		for (cmark = 0; cmark < NMARKS; cmark++) {
			curwp->w_bufp->b_markp[cmark] = curwp->w_markp[cmark];
			curwp->w_bufp->b_marko[cmark] = curwp->w_marko[cmark];
		}
		curwp->w_bufp->b_fcol  = curwp->w_fcol;
	}
	if (lwp == NULL)
		first_screen->s_first_window = wheadp = curwp->w_wndp;
	else
		lwp->w_wndp = curwp->w_wndp;
	free((char *)curwp);
	curwp = wp;
	wp->w_flag |= WFHARD;
	curbp = wp->w_bufp;
	upmode();
	return(TRUE);
}

/*

Split the current window.  A window smaller than 3 lines cannot be
split.	(Two line windows can be split when mode lines are disabled) An
argument of 1 forces the cursor into the upper window, an argument of
two forces the cursor to the lower window.  The only other error that
is possible is a "malloc" failure allocating the structure for the new
window.  Bound to "C-X 2". 

*/

PASCAL NEAR splitwind(f, n)

int f, n;	/* default flag and numeric argument */

{
	register WINDOW *wp;
	register LINE	*lp;
	register int	ntru;
	register int	ntrl;
	register int	ntrd;
	register WINDOW *wp1;
	register WINDOW *wp2;
	int cmark;		/* current mark */

	/* make sure we have enough space */
	if (curwp->w_ntrows < (modeflag ? 3 : 2)) {
		mlwrite(TEXT205, curwp->w_ntrows);
/*			"Cannot split a %d line window" */
		return (FALSE);
	}
	if ((wp = (WINDOW *) malloc(sizeof(WINDOW))) == NULL) {
		mlwrite(TEXT94);
/*			"%%Out of memory" */
		return (FALSE);
	}
	++curbp->b_nwnd;			/* Displayed twice.	*/
	wp->w_bufp  = curbp;
	wp->w_dotp  = curwp->w_dotp;
	wp->w_doto  = curwp->w_doto;
	for (cmark = 0; cmark < NMARKS; cmark++) {
		wp->w_markp[cmark] = curwp->w_markp[cmark];
		wp->w_marko[cmark] = curwp->w_marko[cmark];
	}
	wp->w_fcol  = curwp->w_fcol;
	wp->w_flag  = 0;
	wp->w_force = 0;
#if	COLOR
	/* set the colors of the new window */
	wp->w_fcolor = gfcolor;
	wp->w_bcolor = gbcolor;
#endif
	ntru = (curwp->w_ntrows-1) / 2; 	/* Upper size		*/
	ntrl = (curwp->w_ntrows-1) - ntru;	/* Lower size		*/
	lp = curwp->w_linep;
	ntrd = 0;
	while (lp != curwp->w_dotp) {
		++ntrd;
		lp = lforw(lp);
	}
	lp = curwp->w_linep;
	if (((f == FALSE) && (ntrd <= ntru)) || ((f == TRUE) && (n == 1))) {
		/* Old is upper window. */
		if (ntrd == ntru)		/* Hit mode line.	*/
			lp = lforw(lp);
		curwp->w_ntrows = ntru;
		wp->w_wndp = curwp->w_wndp;
		curwp->w_wndp = wp;
		wp->w_toprow = curwp->w_toprow+ntru+1;
		wp->w_ntrows = ntrl;
	} else {				/* Old is lower window	*/
		wp1 = NULL;
		wp2 = wheadp;
		while (wp2 != curwp) {
			wp1 = wp2;
			wp2 = wp2->w_wndp;
		}
		if (wp1 == NULL)
			first_screen->s_first_window = wheadp = wp;
		else
			wp1->w_wndp = wp;
		wp->w_wndp   = curwp;
		wp->w_toprow = curwp->w_toprow;
		wp->w_ntrows = ntru;
		++ntru; 			/* Mode line.		*/
		curwp->w_toprow += ntru;
		curwp->w_ntrows  = ntrl;
		while (ntru--)
			lp = lforw(lp);
	}
	curwp->w_linep = lp;			/* Adjust the top lines */
	wp->w_linep = lp;			/* if necessary.	*/
	curwp->w_flag |= WFMODE|WFHARD;
	wp->w_flag |= WFMODE|WFHARD;
	return (TRUE);
}

/*
 * Enlarge the current window. Find the window that loses space. Make sure it
 * is big enough. If so, hack the window descriptions, and ask redisplay to do
 * all the hard work. You don't just set "force reframe" because dot would
 * move. Bound to "C-X Z".
 */
PASCAL NEAR enlargewind(f, n)

int f,n;	/* prefix flag and argument */

{
	register WINDOW *adjwp;
	register LINE	*lp;
	register int	i;

	if (n < 0)
		return (shrinkwind(f, -n));
	if (wheadp->w_wndp == NULL) {
		mlwrite(TEXT206);
/*			"Only one window" */
		return (FALSE);
	}
	if ((adjwp=curwp->w_wndp) == NULL) {
		adjwp = wheadp;
		while (adjwp->w_wndp != curwp)
			adjwp = adjwp->w_wndp;
	}
	if ((adjwp->w_ntrows + (modeflag ? 0 : 1)) <= n) {
		mlwrite(TEXT207);
/*			"Impossible change" */
		return (FALSE);
	}
	if (curwp->w_wndp == adjwp) {		/* Shrink below.	*/
		lp = adjwp->w_linep;
		for (i=0; i<n && lp!=adjwp->w_bufp->b_linep; ++i)
			lp = lforw(lp);
		adjwp->w_linep	= lp;
		adjwp->w_toprow += n;
	} else {				/* Shrink above.	*/
		lp = curwp->w_linep;
		for (i=0; i<n && lback(lp)!=curbp->b_linep; ++i)
			lp = lback(lp);
		curwp->w_linep	= lp;
		curwp->w_toprow -= n;
	}
	curwp->w_ntrows += n;
	adjwp->w_ntrows -= n;
	curwp->w_flag |= WFMODE|WFHARD;
	adjwp->w_flag |= WFMODE|WFHARD;
	return (TRUE);
}

/*
 * Shrink the current window. Find the window that gains space. Hack at the
 * window descriptions. Ask the redisplay to do all the hard work. Bound to
 * "C-X C-Z".
 */
PASCAL NEAR shrinkwind(f, n)

int f,n;	/* prefix flag and argument */

{
	register WINDOW *adjwp;
	register LINE	*lp;
	register int	i;

	if (n < 0)
		return (enlargewind(f, -n));
	if (wheadp->w_wndp == NULL) {
		mlwrite(TEXT206);
/*			"Only one window" */
		return (FALSE);
	}
	if ((adjwp=curwp->w_wndp) == NULL) {
		adjwp = wheadp;
		while (adjwp->w_wndp != curwp)
			adjwp = adjwp->w_wndp;
	}
	if ((curwp->w_ntrows + (modeflag ? 0 : 1)) <= n) {
		mlwrite(TEXT207);
/*			"Impossible change" */
		return (FALSE);
	}
	if (curwp->w_wndp == adjwp) {		/* Grow below.		*/
		lp = adjwp->w_linep;
		for (i=0; i<n && lback(lp)!=adjwp->w_bufp->b_linep; ++i)
			lp = lback(lp);
		adjwp->w_linep	= lp;
		adjwp->w_toprow -= n;
	} else {				/* Grow above.		*/
		lp = curwp->w_linep;
		for (i=0; i<n && lp!=curbp->b_linep; ++i)
			lp = lforw(lp);
		curwp->w_linep	= lp;
		curwp->w_toprow += n;
	}
	curwp->w_ntrows -= n;
	adjwp->w_ntrows += n;
	curwp->w_flag |= WFMODE|WFHARD;
	adjwp->w_flag |= WFMODE|WFHARD;
	return (TRUE);
}

/*	Resize the current window to the requested size */

PASCAL NEAR resize(f, n)

int f, n;	/* default flag and numeric argument */

{
	int clines;	/* current # of lines in window */
        
	/* must have a non-default argument, else ignore call */
	if (f == FALSE)
		return(TRUE);

	/* find out what to do */
	clines = curwp->w_ntrows;

	/* already the right size? */
	if (clines == n)
		return(TRUE);

	return(enlargewind(TRUE, n - clines));
}

/*	pop up the indicated buffer
*/

#if	PROTO
int PASCAL NEAR wpopup(BUFFER *popbuf)
#else
int PASCAL NEAR wpopup(popbuf)

BUFFER *popbuf;
#endif
{
	register WINDOW *wp;
	register BUFFER *bp;
	register int cmark;		/* current mark */

	/* on screen already? */
	if (popbuf->b_nwnd != 0)
		goto setwin;

	/* if flaged so, do a real pop up */
	if (popflag)
		return(pop(popbuf));

	/* find the window to split */
	if (wheadp->w_wndp == NULL		/* Only 1 window	*/
	&& splitwind(FALSE, 0) == FALSE)	/* and it won't split	*/
		return(FALSE);
	wp = wheadp;				/* Find window to use	*/
	while (wp!=NULL && wp == curwp)
		wp = wp->w_wndp;

	if (popbuf->b_nwnd == 0) {		/* Not on screen yet.	*/
		bp = wp->w_bufp;
		if (--bp->b_nwnd == 0) {
			bp->b_dotp  = wp->w_dotp;
			bp->b_doto  = wp->w_doto;
			for (cmark = 0; cmark < NMARKS; cmark++) {
				bp->b_markp[cmark] = wp->w_markp[cmark];
				bp->b_marko[cmark] = wp->w_marko[cmark];
			}
			bp->b_fcol  = wp->w_fcol;
		}
		wp->w_bufp  = popbuf;
		++popbuf->b_nwnd;
	}

setwin: wp = wheadp;
	while (wp != NULL) {
		if (wp->w_bufp == popbuf) {
			wp->w_linep = lforw(popbuf->b_linep);
			wp->w_dotp  = lforw(popbuf->b_linep);
			wp->w_doto  = 0;
			for (cmark = 0; cmark < NMARKS; cmark++) {
				wp->w_markp[cmark] = NULL;
				wp->w_marko[cmark] = 0;
			}
			wp->w_flag |= WFMODE|WFHARD;
			popbuf->b_mode |= MDVIEW; /* put this buffer view mode */
			upmode();
		}
		wp = wp->w_wndp;
	}
	return(TRUE);
}

PASCAL NEAR nextup(f, n)	/* scroll the next window up (back) a page */

int f, n;	/* prefix flag and argument */

{
	nextwind(FALSE, 1);
	backpage(f, n);
	prevwind(FALSE, 1);
}

PASCAL NEAR nextdown(f, n)	/* scroll the next window down (forward) a page */

int f, n;	/* prefix flag and argument */

{
	nextwind(FALSE, 1);
	forwpage(f, n);
	prevwind(FALSE, 1);
}

PASCAL NEAR savewnd(f, n)	/* save ptr to current window */

int f, n;	/* prefix flag and argument */

{
	swindow = curwp;
	return(TRUE);
}

PASCAL NEAR restwnd(f, n)	/* restore the saved screen */

int f, n;	/* prefix flag and argument */

{
	register WINDOW *wp;

	/* find the window */
	wp = wheadp;
	while (wp != NULL) {
		if (wp == swindow) {
			curwp = wp;
			curbp = wp->w_bufp;
			upmode();
			return (TRUE);
		}
		wp = wp->w_wndp;
	}

	mlwrite(TEXT208);
/*		"[No such window exists]" */
	return(FALSE);
}

PASCAL NEAR newsize(f, n)	/* resize the screen, re-writing the screen */

int f;	/* default flag */
int n;	/* numeric argument */

{
	WINDOW *wp;	/* current window being examined */
	WINDOW *nextwp; /* next window to scan */
	WINDOW *lastwp; /* last window scanned */
	int lastline;	/* screen line of last line of current window */
	int cmark;		/* current mark */

	/* if the command defaults, assume the largest */
	if (f == FALSE)
		n = term.t_mrow + 1;

	/* make sure it's in range */
	if (n < 3 || n > term.t_mrow + 1) {
		mlwrite(TEXT209);
/*			"%%Screen size out of range" */
		return(FALSE);
	}

	if (term.t_nrow == n - 1)
		return(TRUE);
	else if (term.t_nrow < n - 1) {

		/* go to the last window */
		wp = wheadp;
		while (wp->w_wndp != NULL)
			wp = wp->w_wndp;

		/* and enlarge it as needed */
		wp->w_ntrows = n - wp->w_toprow - 2;
		wp->w_flag |= WFHARD|WFMODE;

	} else {

		/* rebuild the window structure */
		nextwp = wheadp;
		wp = NULL;
		lastwp = NULL;
		while (nextwp != NULL) {
			wp = nextwp;
			nextwp = wp->w_wndp;
        
			/* get rid of it if it is too low */
			if (wp->w_toprow > n - 2) {

				/* save the point/mark if needed */
				if (--wp->w_bufp->b_nwnd == 0) {
					wp->w_bufp->b_dotp = wp->w_dotp;
					wp->w_bufp->b_doto = wp->w_doto;
					for (cmark = 0; cmark < NMARKS; cmark++) {
						wp->w_bufp->b_markp[cmark] = wp->w_markp[cmark];
						wp->w_bufp->b_marko[cmark] = wp->w_marko[cmark];
					}
					wp->w_bufp->b_fcol = wp->w_fcol;
				}
        
				/* update curwp and lastwp if needed */
				if (wp == curwp)
					curwp = wheadp;
					curbp = curwp->w_bufp;
				if (lastwp != NULL)
					lastwp->w_wndp = NULL;

				/* free the structure */
				free((char *)wp);
				wp = NULL;

			} else {
				/* need to change this window size? */
				lastline = wp->w_toprow + wp->w_ntrows - 1;
				if (lastline >= n - 2) {
					wp->w_ntrows = n - wp->w_toprow - 2;
					wp->w_flag |= WFHARD|WFMODE;
				}
			}
        
			lastwp = wp;
		}
	}

	/* screen is garbage */
	term.t_nrow = n - 1;
	sgarbf = TRUE;
	return(TRUE);
}

PASCAL NEAR newwidth(f, n)	/* resize the screen, re-writing the screen */

int f;	/* default flag */
int n;	/* numeric argument */

{
	register WINDOW *wp;

	/* if the command defaults, assume the largest */
	if (f == FALSE)
		n = term.t_mcol;

	/* make sure it's in range */
	if (n < 10 || n > term.t_mcol) {
		mlwrite(TEXT210);
/*			"%%Screen width out of range" */
		return(FALSE);
	}

	/* otherwise, just re-width it (no big deal) */
	term.t_ncol = n;
	term.t_margin = n / 10;
	term.t_scrsiz = n - (term.t_margin * 2);

	/* force all windows to redraw */
	wp = wheadp;
	while (wp) {
		wp->w_flag |= WFHARD | WFMOVE | WFMODE;
		wp = wp->w_wndp;
	}
	sgarbf = TRUE;

	return(TRUE);
}

PASCAL NEAR new_col_org(f, n)	/* reposition the screen, re-writing the screen */

int f;	/* default flag */
int n;	/* numeric argument */

{
	register WINDOW *wp;

	/* if the command defaults, assume zero */
	if (f == FALSE)
		n = 0;

	/* make sure it's in range */
	if (n < 0 || n > term.t_mcol - term.t_ncol) {
		mlwrite(TEXT223);
/*			"%%Column origin out of range" */
		return(FALSE);
	}

	/* otherwise, just re-width it (no big deal) */
	term.t_colorg = n;
	sgarbf = TRUE;

	return(TRUE);
}

PASCAL NEAR new_row_org(f, n)	/* reposition the screen, re-writing the screen */

int f;	/* default flag */
int n;	/* numeric argument */

{
	register WINDOW *wp;

	/* if the command defaults, assume zero */
	if (f == FALSE)
		n = 0;

	/* make sure it's in range */
	if (n < 0 || n > term.t_mrow - term.t_nrow) {
		mlwrite(TEXT224);
/*			"%%Row origin out of range" */
		return(FALSE);
	}

	/* otherwise, just re-size it (no big deal) */
	term.t_roworg = n;

	sgarbf = TRUE;
	return(TRUE);
}

int PASCAL NEAR getwpos()	/* get screen offset of current line in current window */

{
	register int sline;	/* screen line from top of window */
	register LINE *lp;	/* scannile line pointer */

	/* search down the line we want */
	lp = curwp->w_linep;
	sline = 1;
	while (lp != curwp->w_dotp) {
		++sline;
		lp = lforw(lp);
	}

	/* and return the value */
	return(sline);
}

int PASCAL NEAR getcwnum()		/* get current window number */

{
	register WINDOW *wp;
	register int num;

	num = 1;
	wp = wheadp;
	while (wp != curwp) {
		wp = wp->w_wndp;
		num++;
	}
	return(num);
}


int PASCAL NEAR gettwnum()		/* get total window count */

{
	register WINDOW *wp;
	register int ctr;

	ctr = 0;
	wp = wheadp;
	while (wp) {
		ctr++;
		wp = wp->w_wndp;
	}
	return(ctr);
}

