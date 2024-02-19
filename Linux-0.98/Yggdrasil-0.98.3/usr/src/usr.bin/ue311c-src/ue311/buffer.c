/*  BUFFER.C:	buffer mgmt. routines
		MicroEMACS 3.10

 * Buffer management.
 * Some of the functions are internal,
 * and some are actually attached to user
 * keys. Like everyone else, they set hints
 * for the display system.
 */
#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
PASCAL NEAR usebuffer(f, n)

int f,n;	/* prefix flag and argument */

{
	register BUFFER *bp;	/* temporary buffer pointer */

	/* get the buffer name to switch to */
	bp = getdefb();
	bp = getcbuf(TEXT24, bp ? bp->b_bname : mainbuf, TRUE);
/*			    "Use buffer" */
	if (!bp)
		return(ABORT);

	/* make it invisible if there is an argument */
	if (f == TRUE)
		bp->b_flag |= BFINVS;

	/* switch to it in any case */
	return(swbuffer(bp));
}

PASCAL NEAR nextbuffer(f, n)	/* switch to the next buffer in the buffer list */

int f, n;	/* default flag, numeric argument */
{
	register BUFFER *bp;	/* current eligable buffer */
	register int status;

	/* make sure the arg is legit */
	if (f == FALSE)
		n = 1;
	if (n < 1)
		return(FALSE);

	/* cycle thru buffers until n runs out */
	while (n-- > 0) {
		bp = getdefb();
		if (bp == NULL)
			return(FALSE);
		status = swbuffer(bp);
		if (status != TRUE)
			return(status);
	}
	return(status);
}

PASCAL NEAR swbuffer(bp)	/* make buffer BP current */

BUFFER *bp;

{
	register WINDOW *wp;
	SCREEN *scrp;		/* screen to fix pointers in */
	register int cmark;		/* current mark */

	/* let a user macro get hold of things...if he wants */
	execkey(&exbhook, FALSE, 1);

	if (--curbp->b_nwnd == 0) {		/* Last use.		*/
		curbp->b_dotp  = curwp->w_dotp;
		curbp->b_doto  = curwp->w_doto;
		for (cmark = 0; cmark < NMARKS; cmark++) {
			curbp->b_markp[cmark] = curwp->w_markp[cmark];
			curbp->b_marko[cmark] = curwp->w_marko[cmark];
		}
		curbp->b_fcol  = curwp->w_fcol;
	}
	curbp = bp;				/* Switch.		*/
	if (curbp->b_active != TRUE) {		/* buffer not active yet*/
		/* read it in and activate it */
		readin(curbp->b_fname, ((curbp->b_mode&MDVIEW) == 0));
		curbp->b_dotp = lforw(curbp->b_linep);
		curbp->b_doto = 0;
		curbp->b_active = TRUE;
	}
	curwp->w_bufp  = bp;
	curwp->w_linep = bp->b_linep;		/* For macros, ignored. */
	curwp->w_flag |= WFMODE|WFFORCE|WFHARD; /* Quite nasty. 	*/
	if (bp->b_nwnd++ == 0) {		/* First use.		*/
		curwp->w_dotp  = bp->b_dotp;
		curwp->w_doto  = bp->b_doto;
		for (cmark = 0; cmark < NMARKS; cmark++) {
			curwp->w_markp[cmark] = bp->b_markp[cmark];
			curwp->w_marko[cmark] = bp->b_marko[cmark];
		}
		curwp->w_fcol  = bp->b_fcol;
	} else {
		/* in all screens.... */
		scrp = first_screen;
		while (scrp) {
			wp = scrp->s_first_window;
			while (wp != NULL) {
				if (wp!=curwp && wp->w_bufp==bp) {
					curwp->w_dotp  = wp->w_dotp;
					curwp->w_doto  = wp->w_doto;
					for (cmark = 0; cmark < NMARKS; cmark++) {
						curwp->w_markp[cmark] = wp->w_markp[cmark];
						curwp->w_marko[cmark] = wp->w_marko[cmark];
					}
					curwp->w_fcol  = wp->w_fcol;
					break;
				}
				/* next window */
				wp = wp->w_wndp;
			}

			/* next screen! */
			scrp = scrp->s_next_screen;
		}
	}

	/* let a user macro get hold of things...if he wants */
	execkey(&bufhook, FALSE, 1);

	return(TRUE);
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header. Bound to "C-X K".
 */
PASCAL NEAR killbuffer(f, n)

int f,n;	/* prefix flag and argument */

{
	register BUFFER *bp;	/* ptr to buffer to dump */

	/* get the buffer name to kill */
	bp = getdefb();
	bp = getcbuf(TEXT26, bp ? bp->b_bname : mainbuf, TRUE);
/*		     "Kill buffer" */
	if (bp == NULL)
		return(ABORT);

	return(zotbuf(bp));
}

/*	Allow the user to pop up a buffer, like we do.... */

PASCAL NEAR popbuffer(f, n)

int f, n;	/* default and numeric arguments */

{
	register BUFFER *bp;	/* ptr to buffer to dump */

	/* get the buffer name to pop */
	bp = getdefb();
	bp = getcbuf(TEXT27, bp ? bp->b_bname : mainbuf, TRUE);
/*		     "Pop buffer" */
	if (bp == NULL)
		return(ABORT);

	return(pop(bp));
	/* make it invisible if there is an argument */
	if (f == TRUE)
		bp->b_flag |= BFINVS;

}

BUFFER *PASCAL NEAR getdefb()	/* get the default buffer for a use or kill */

{
	BUFFER *bp;	/* default buffer */

	/* Find the next buffer, which will be the default */
	bp = curbp->b_bufp;

	/* cycle through the buffers to find an eligable one */
	while (bp == NULL || bp->b_flag & BFINVS) {
		if (bp == NULL)
			bp = bheadp;
		else
			bp = bp->b_bufp;

		/* don't get caught in an infinite loop! */
		if (bp == curbp) {
			bp = NULL;
			break;
		}
	}	        
	return(bp);
}

PASCAL NEAR zotbuf(bp)	/* kill the buffer pointed to by bp */

register BUFFER *bp;

{
	register BUFFER *bp1;
	register BUFFER *bp2;
	register int	s;

	/* we can not kill a displayed buffer */
	if (bp->b_nwnd != 0) {
		mlwrite(TEXT28);
/*			"Buffer is being displayed" */
		return(FALSE);
	}

	/* we can not kill an executing buffer */
	if (bp->b_exec != 0) {
		mlwrite(TEXT226);
/*			"%%Can not delete an executing buffer" */
		return(FALSE);
	}

	if ((s=bclear(bp)) != TRUE)		/* Blow text away.	*/
		return(s);
	free((char *) bp->b_linep);		/* Release header line. */
	bp1 = NULL;				/* Find the header.	*/
	bp2 = bheadp;
	while (bp2 != bp) {
		bp1 = bp2;
		bp2 = bp2->b_bufp;
	}
	bp2 = bp2->b_bufp;			/* Next one in chain.	*/
	if (bp1 == NULL)			/* Unlink it.		*/
		bheadp = bp2;
	else
		bp1->b_bufp = bp2;
	free((char *) bp);			/* Release buffer block */
	return(TRUE);
}

PASCAL NEAR namebuffer(f,n)	/*	Rename the current buffer	*/

int f, n;		/* default Flag & Numeric arg */

{
	register BUFFER *bp;	/* pointer to scan through all buffers */
	char bufn[NBUFN];	/* buffer to hold buffer name */

	/* prompt for and get the new buffer name */
ask:	if (mlreply(TEXT29, bufn, NBUFN) != TRUE)
/*		    "Change buffer name to: " */
		return(FALSE);

	/* and check for duplicates */
	bp = bheadp;
	while (bp != NULL) {
		if (bp != curbp) {
			/* if the names the same */
			if (strcmp(bufn, bp->b_bname) == 0)
				goto ask;  /* try again */
		}
		bp = bp->b_bufp;	/* onward */
	}

	strcpy(curbp->b_bname, bufn);	/* copy buffer name to structure */
	upmode();			/* make all mode lines replot */
	mlerase();
	return(TRUE);
}

/*	Build and popup a buffer containing the list of all buffers.
	Bound to "C-X C-B". A numeric argument forces it to list
	invisible buffers as well.
*/

PASCAL NEAR listbuffers(f, n)

int f,n;	/* prefix flag and argument */

{
	register int status;	/* stutus return */

	if ((status = makelist(f)) != TRUE)
		return(status);
	return(wpopup(blistp));
}

/*
 * This routine rebuilds the
 * text in the special secret buffer
 * that holds the buffer list. It is called
 * by the list buffers command. Return TRUE
 * if everything works. Return FALSE if there
 * is an error (if there is no memory). Iflag
 * indecates weather to list hidden buffers.
 */
PASCAL NEAR makelist(iflag)

int iflag;	/* list hidden buffer flag */

{
	register char	*cp1;
	register char	*cp2;
	register int	c;
	register BUFFER *bp;
	register LINE	*lp;
	register int	s;
	register int	i;
	long nbytes;		/* # of bytes in current buffer */
	char b[7+1];
	char line[128];

	blistp->b_flag &= ~BFCHG;		/* Don't complain!	*/
	if ((s=bclear(blistp)) != TRUE) 	/* Blow old text away	*/
		return(s);
	strcpy(blistp->b_fname, "");
	if (addline(blistp, TEXT30) == FALSE
/*		    "ACT   Modes      Size Buffer	   File" */
	||  addline(blistp, "--- --------- ------- --------------- ----") == FALSE)
		return(FALSE);
	bp = bheadp;				/* For all buffers	*/

	/* build line to report global mode settings */
	cp1 = &line[0];
	*cp1++ = ' ';
	*cp1++ = ' ';
	*cp1++ = ' ';
	*cp1++ = ' ';

	/* output the mode codes */
	for (i = 0; i < NUMMODES; i++)
		if (gmode & (1 << i))
			*cp1++ = modecode[i];
		else
			*cp1++ = '.';
	strcpy(cp1, TEXT31);
/*		    "	      Global Modes" */
	if (addline(blistp, line) == FALSE)
		return(FALSE);

	/* output the list of buffers */
	while (bp != NULL) {
		/* skip invisible buffers if iflag is false */
		if (((bp->b_flag&BFINVS) != 0) && (iflag != TRUE)) {
			bp = bp->b_bufp;
			continue;
		}
		cp1 = &line[0]; 		/* Start at left edge	*/

		/* output status of ACTIVE flag (has the file been read in? */
		if (bp->b_active == TRUE)    /* "@" if activated       */
			*cp1++ = '@';
		else
			*cp1++ = ' ';

		/* output status of changed flag */
		if ((bp->b_flag&BFCHG) != 0)	/* "*" if changed	*/
			*cp1++ = '*';
		else
			*cp1++ = ' ';

		/* report if the file is truncated */
		if ((bp->b_flag&BFTRUNC) != 0)
			*cp1++ = '#';
		else
			*cp1++ = ' ';

		*cp1++ = ' ';	/* space */

		/* output the mode codes */
		for (i = 0; i < NUMMODES; i++) {
			if (bp->b_mode & (1 << i))
				*cp1++ = modecode[i];
			else
				*cp1++ = '.';
		}
		*cp1++ = ' ';			/* Gap. 		*/
		nbytes = 0L;			/* Count bytes in buf.	*/
		lp = lforw(bp->b_linep);
		while (lp != bp->b_linep) {
			nbytes += (long)llength(lp)+1L;
			lp = lforw(lp);
		}
		long_asc(b, 7, nbytes); 	    /* 6 digit buffer size. */
		cp2 = &b[0];
		while (*cp2)
			*cp1++ = *cp2++;
		*cp1++ = ' ';			/* Gap. 		*/
		cp2 = &bp->b_bname[0];		/* Buffer name		*/
		while (*cp2)
			*cp1++ = *cp2++;
		*cp1++ = ' ';			/* Gap. 		*/
		cp2 = &bp->b_fname[0];		/* File name		*/
		if (*cp2 != 0) {
			while (cp1 < &line[38])
				*cp1++ = ' ';
			while (*cp2)
				*cp1++ = *cp2++;
		}
		*cp1 = 0;	      /* Add to the buffer.   */
		if (addline(blistp, line) == FALSE)
			return(FALSE);
		bp = bp->b_bufp;
	}
	return(TRUE);			       /* All done	       */
}

/* Translate a long to ascii form. Don't trust various systems
   ltoa() routines.. they aren't consistant				*/

PASCAL NEAR long_asc(buf, width, num)

char   buf[];
int    width;
long   num;

{
	buf[width] = 0; 			/* End of string.	*/
	while (num >= 10) {			/* Conditional digits.	*/
		buf[--width] = (int)(num%10L) + '0';
		num /= 10L;
	}
	buf[--width] = (int)num + '0';		/* Always 1 digit.	*/
	while (width != 0)			/* Pad with blanks.	*/
		buf[--width] = ' ';
}

/*
 * Look through the list of
 * buffers. Return TRUE if there
 * are any changed buffers. Buffers
 * that hold magic internal stuff are
 * not considered; who cares if the
 * list of buffer names is hacked.
 * Return FALSE if no buffers
 * have been changed.
 */
PASCAL NEAR anycb()
{
	register BUFFER *bp;

	bp = bheadp;
	while (bp != NULL) {
		if ((bp->b_flag&BFINVS)==0 && (bp->b_flag&BFCHG)!=0)
			return(TRUE);
		bp = bp->b_bufp;
	}
	return(FALSE);
}

/*
 * Find a buffer, by name. Return a pointer
 * to the BUFFER structure associated with it.
 * If the buffer is not found
 * and the "cflag" is TRUE, create it. The "bflag" is
 * the settings for the flags in in buffer.
 */
BUFFER *PASCAL NEAR bfind(bname, cflag, bflag)

register char	*bname; /* name of buffer to find */
int cflag;		/* create it if not found? */
int bflag;		/* bit settings for a new buffer */

{
	register BUFFER *bp;
	register BUFFER *sb;	/* buffer to insert after */
	register LINE	*lp;
	int cmark;		/* current mark */

	bp = bheadp;
	while (bp != NULL) {
		if (strcmp(bname, bp->b_bname) == 0)
			return(bp);
		bp = bp->b_bufp;
	}

	/* no such buffer exists, create it? */
	if (cflag != FALSE) {

		/* allocate the needed memory */
		if ((bp=(BUFFER *)malloc(sizeof(BUFFER))) == NULL)
			return(NULL);
		if ((lp=lalloc(0)) == NULL) {
			free((char *) bp);
			return(NULL);
		}

		/* find the place in the list to insert this buffer */
		if (bheadp == NULL || strcmp(bheadp->b_bname, bname) > 0) {
			/* insert at the beginning */
			bp->b_bufp = bheadp;
			bheadp = bp;
		} else {
			sb = bheadp;
			while (sb->b_bufp != NULL) {
				if (strcmp(sb->b_bufp->b_bname, bname) > 0)
					break;
				sb = sb->b_bufp;
			}

			/* and insert it */
			bp->b_bufp = sb->b_bufp;
			sb->b_bufp = bp;
		}

		/* and set up the other buffer fields */
		bp->b_topline = NULL;
		bp->b_botline = NULL;
		bp->b_active = TRUE;
		bp->b_dotp  = lp;
		bp->b_doto  = 0;
		for (cmark = 0; cmark < NMARKS; cmark++) {
			bp->b_markp[cmark] = NULL;
			bp->b_marko[cmark] = 0;
		}
		bp->b_fcol  = 0;
		bp->b_flag  = bflag;
		bp->b_mode  = gmode;
		bp->b_nwnd  = 0;
		bp->b_exec  = 0;
		bp->b_linep = lp;
		strcpy(bp->b_fname, "");
		strcpy(bp->b_bname, bname);
#if	CRYPT
		bp->b_key[0] = 0;
#endif
		lp->l_fp = lp;
		lp->l_bp = lp;
	}
	return(bp);
}

/*
 * This routine blows away all of the text
 * in a buffer. If the buffer is marked as changed
 * then we ask if it is ok to blow it away; this is
 * to save the user the grief of losing text. The
 * window chain is nearly always wrong if this gets
 * called; the caller must arrange for the updates
 * that are required. Return TRUE if everything
 * looks good.
 */
PASCAL NEAR bclear(bp)
register BUFFER *bp;
{
	register LINE	*lp;
	register int	s;
	int cmark;		/* current mark */

	if ((bp->b_flag&BFINVS) == 0		/* Not scratch buffer.	*/
	&& (bp->b_flag&BFCHG) != 0		/* Something changed	*/
	&& (s=mlyesno(TEXT32)) != TRUE)
/*		      "Discard changes" */
		return(s);
	bp->b_flag  &= ~BFCHG;			/* Not changed		*/
	while ((lp=lforw(bp->b_linep)) != bp->b_linep)
		lfree(lp);
	bp->b_dotp  = bp->b_linep;		/* Fix "."		*/
	bp->b_doto  = 0;
	for (cmark = 0; cmark < NMARKS; cmark++) {
		bp->b_markp[cmark] = NULL;  /* Invalidate "mark"    */
		bp->b_marko[cmark] = 0;
	}
	bp->b_fcol = 0;
	return(TRUE);
}

PASCAL NEAR unmark(f, n)	/* unmark the current buffers change flag */

int f, n;	/* unused command arguments */

{
	register WINDOW *wp;

	/* unmark the buffer */
	curbp->b_flag &= ~BFCHG;

	/* unmark all windows as well */
	upmode();

	return(TRUE);
}
