/*
 * This file contains the command processing functions for a number of random
 * commands. There is no functional grouping here, for sure.
 */

#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include        "edef.h"
#include	"elang.h"

/*
 * Set fill column to n.
 */
PASCAL NEAR setfillcol(f, n)

int f,n;	/* prefix flag and argument */

{
        fillcol = n;
	mlwrite(TEXT59,n);
/*              "[Fill column is %d]" */
        return(TRUE);
}

/*
 * Display the current position of the cursor, in origin 1 X-Y coordinates,
 * the character that is under the cursor (in hex), and the fraction of the
 * text that is before the cursor. The displayed column is not the current
 * column, but the column that would be used on an infinite width display.
 * Normally this is bound to "C-X =".
 */
PASCAL NEAR showcpos(f, n)

int f,n;	/* prefix flag and argument */

{
        register LINE   *lp;		/* current line */
        register long   numchars;	/* # of chars in file */
        register int	numlines;	/* # of lines in file */
        register long   predchars;	/* # chars preceding point */
        register int	predlines;	/* # lines preceding point */
        register int    curchar;	/* character under cursor */
        int ratio;
        int col;
	int savepos;			/* temp save for current offset */
	int ecol;			/* column pos/end of current line */

	/* starting at the beginning of the buffer */
        lp = lforw(curbp->b_linep);
	curchar = '\r';

	/* start counting chars and lines */
        numchars = 0;
        numlines = 0;
        while (lp != curbp->b_linep) {
		/* if we are on the current line, record it */
		if (lp == curwp->w_dotp) {
			predlines = numlines;
			predchars = numchars + curwp->w_doto;
			if ((curwp->w_doto) == llength(lp))
				curchar = '\r';
			else
				curchar = lgetc(lp, curwp->w_doto);
		}
		/* on to the next line */
		++numlines;
		numchars += llength(lp) + 1;
		lp = lforw(lp);
        }

	/* if at end of file, record it */
	if (curwp->w_dotp == curbp->b_linep) {
		predlines = numlines;
		predchars = numchars;
	}

	/* Get real column and end-of-line column. */
	col = getccol(FALSE);
	savepos = curwp->w_doto;
	curwp->w_doto = llength(curwp->w_dotp);
	ecol = getccol(FALSE);
	curwp->w_doto = savepos;

        ratio = 0;              /* Ratio before dot. */
        if (numchars != 0)
                ratio = (100L*predchars) / numchars;

	/* summarize and report the info */
#if	DBCS
	if (is2byte(curwp->w_dotp->l_text,
	    curwp->w_dotp->l_text + curwp->w_doto)) {
		mlwrite(TEXT220,
/*              "Line %d/%d Col %d/%d Char %D/%D (%d%%) char = 0x%x%x" */
			predlines+1, numlines+1, col, ecol,
			predchars, numchars, ratio, (unsigned char)curchar,
			(unsigned char)(curwp->w_dotp->l_text[curwp->w_doto+1]));
		return(TRUE);
	}
#endif
	mlwrite(TEXT60,
/*              "Line %d/%d Col %d/%d Char %D/%D (%d%%) char = 0x%x" */
		predlines+1, numlines+1, col, ecol,
		predchars, numchars, ratio, curchar);
        return(TRUE);
}

PASCAL NEAR getlinenum(bp, sline)	/* get the a line number */

BUFFER *bp;	/* buffer to get current line from */
LINE *sline;	/* line to search for */

{
        register LINE   *lp;		/* current line */
        register int	numlines;	/* # of lines before point */

	/* starting at the beginning of the buffer */
        lp = lforw(bp->b_linep);

	/* start counting lines */
        numlines = 0;
        while (lp != bp->b_linep) {
		/* if we are on the current line, record it */
		if (lp == sline)
			break;
		++numlines;
		lp = lforw(lp);
        }

	/* and return the resulting count */
	return(numlines + 1);
}

/*
 * Return current column.  Stop at first non-blank given TRUE argument.
 */
PASCAL NEAR getccol(bflg)
int bflg;
{
        register int c, i, col;
        col = 0;
        for (i=0; i<curwp->w_doto; ++i) {
                c = lgetc(curwp->w_dotp, i);
                if (c!=' ' && c!='\t' && bflg)
                        break;
                if (c == '\t')
                        col += -(col % tabsize) + (tabsize - 1);
                else if (c<0x20 || c==0x7F)
                        ++col;
                ++col;
        }
        return(col);
}

/*
 * Set current column.
 */
PASCAL NEAR setccol(pos)

int pos;	/* position to set cursor */

{
        register int c;		/* character being scanned */
	register int i;		/* index into current line */
	register int col;	/* current cursor column   */
	register int llen;	/* length of line in bytes */

	col = 0;
	llen = llength(curwp->w_dotp);

	/* scan the line until we are at or past the target column */
	for (i = 0; i < llen; ++i) {
		/* upon reaching the target, drop out */
		if (col >= pos)
			break;

		/* advance one character */
                c = lgetc(curwp->w_dotp, i);
                if (c == '\t')
                        col += -(col % tabsize) + (tabsize - 1);
                else if (c<0x20 || c==0x7F)
                        ++col;
                ++col;
        }

	/* set us at the new position */
	curwp->w_doto = i;

	/* and tell weather we made it */
	return(col >= pos);
}

/*
 * Twiddle the two characters on either side of dot. If dot is at the end of
 * the line twiddle the two characters before it. Return with an error if dot
 * is at the beginning of line; it seems to be a bit pointless to make this
 * work. This fixes up a very common typo with a single stroke. Normally bound
 * to "C-T". This always works within a line, so "WFEDIT" is good enough.
 */
PASCAL NEAR twiddle(f, n)

int f,n;	/* prefix flag and argument */

{
        register LINE   *dotp;
        register int    doto;
        register int    cl;
        register int    cr;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        dotp = curwp->w_dotp;
        doto = curwp->w_doto;
        if (doto==llength(dotp) && --doto<0)
                return(FALSE);
        cr = lgetc(dotp, doto);
        if (--doto < 0)
                return(FALSE);
        cl = lgetc(dotp, doto);
        lputc(dotp, doto+0, cr);
        lputc(dotp, doto+1, cl);
        lchange(WFEDIT);
        return(TRUE);
}

/*
 * Quote the next character, and insert it into the buffer. All the characters
 * are taken literally, including the newline, which does not then have
 * its line splitting meaning. The character is always read, even if it is
 * inserted 0 times, for regularity. Bound to "C-Q"
 */

PASCAL NEAR quote(f, n)

int f,n;	/* prefix flag and argument */

{
        register int c;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        c = tgetc();
        if (n < 0)
                return(FALSE);
        if (n == 0)
                return(TRUE);
        return(linsert(n, c));
}

/*
 * Set tab size if given non-default argument (n <> 1).  Otherwise, insert a
 * tab into file.  If given argument, n, of zero, change to hard tabs.
 * If n > 1, simulate tab stop every n-characters using spaces. This has to be
 * done in this slightly funny way because the tab (in ASCII) has been turned
 * into "C-I" (in 10 bit code) already. Bound to "C-I".
 */
PASCAL NEAR tab(f, n)

int f,n;	/* prefix flag and argument */

{
        if (n < 0)
                return(FALSE);
        if (n == 0 || n > 1) {
                stabsize = n;
                return(TRUE);
        }
        if (!stabsize)
                return(linsert(1, '\t'));
        return(linsert(stabsize - (getccol(FALSE) % stabsize), ' '));
}

PASCAL NEAR detab(f, n)	/* change tabs to spaces */

int f,n;	/* default flag and numeric repeat count */

{
	register int inc;	/* increment to next line [sgn(n)] */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	if (f == FALSE)
		n = reglines();

	/* loop thru detabbing n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		curwp->w_doto = 0;	/* start at the beginning */

		/* detab the entire current line */
		while (curwp->w_doto < llength(curwp->w_dotp)) {
			/* if we have a tab */
			if (lgetc(curwp->w_dotp, curwp->w_doto) == '\t') {
				ldelete(1L, FALSE);
/*				insspace(TRUE, 8 - (curwp->w_doto & 7));*/
				insspace(TRUE, tabsize - (curwp->w_doto % tabsize));
			}
			forwchar(FALSE, 1);
		}

		/* advance/or back to the next line */
		forwline(TRUE, inc);
		n -= inc;
	}
	curwp->w_doto = 0;	/* to the begining of the line */
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	lchange(WFEDIT);	/* yes, we have made at least an edit */
	return(TRUE);
}


PASCAL NEAR entab(f, n)	/* change spaces to tabs where posible */

int f,n;	/* default flag and numeric repeat count */

{
	register int inc;	/* increment to next line [sgn(n)] */
	register int fspace;	/* pointer to first space if in a run */
	register int ccol;	/* current cursor column */
	register char cchar;	/* current character */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	if (f == FALSE)
		n = reglines();

	/* loop thru entabbing n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		/* entab the entire current line */

		ccol = curwp->w_doto = 0;	/* start at the beginning */
		fspace = -1;

		while (curwp->w_doto < llength(curwp->w_dotp)) {
			/* see if it is time to compress */
			if ((fspace >= 0) && (nextab(fspace) <= ccol))
				if (ccol - fspace < 2)
					fspace = -1;
				else {
					backchar(TRUE, ccol - fspace);
					ldelete((long)(ccol - fspace), FALSE);
					linsert(1, '\t');	
					fspace = -1;
				}

			/* get the current character */
			cchar = lgetc(curwp->w_dotp, curwp->w_doto);

			switch (cchar) {
				case '\t': /* a tab...count em up (no break here)  */
					ldelete(1L, FALSE);
					insspace(TRUE, tabsize - (ccol % tabsize));

				case ' ':  /* a space...compress? */
					if (fspace == -1)
						fspace = ccol;
					break;

				default:   /* any other char...just count */
					fspace = -1;
					break;
			}
			ccol++;
			forwchar(FALSE, 1);
		}

		/* advance/or back to the next line */
		forwline(TRUE, inc);
		n -= inc;
		curwp->w_doto = 0;	/* start at the beginning */
	}
	curwp->w_doto = 0;	/* to the begining of the line */
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	lchange(WFEDIT);	/* yes, we have made at least an edit */
	return(TRUE);
}

/* trim:	trim trailing whitespace from the point to eol
		with no arguments, it trims the current region
*/

PASCAL NEAR trim(f, n)

int f,n;	/* default flag and numeric repeat count */

{
	register LINE *lp;	/* current line pointer */
	register int offset;	/* original line offset position */
	register int length;	/* current length */
	register int inc;	/* increment to next line [sgn(n)] */

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	if (f == FALSE)
		n = reglines();

	/* loop thru trimming n lines */
	inc = ((n > 0) ? 1 : -1);
	while (n) {
		lp = curwp->w_dotp;		/* find current line text */
		offset = curwp->w_doto;		/* save original offset */
		length = lp->l_used;		/* find current length */

		/* trim the current line */
		while (length > offset) {
			if (lgetc(lp, length-1) != ' ' &&
			    lgetc(lp, length-1) != '\t')
			    	break;
			length--;
		}
		lp->l_used = length;

		/* advance/or back to the next line */
		forwline(TRUE, inc);
		n -= inc;
	}
	lchange(WFEDIT);
	thisflag &= ~CFCPCN;	/* flag that this resets the goal column */
	return(TRUE);
}

/*
 * Open up some blank space. The basic plan is to insert a bunch of newlines,
 * and then back up over them. Everything is done by the subcommand
 * procerssors. They even handle the looping. Normally this is bound to "C-O".
 */
PASCAL NEAR openline(f, n)

int f,n;	/* prefix flag and argument */

{
        register int    i;
        register int    s;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return(FALSE);
        if (n == 0)
                return(TRUE);
        i = n;                                  /* Insert newlines.     */
        do {
                s = lnewline();
        } while (s==TRUE && --i);
        if (s == TRUE)                          /* Then back up overtop */
                s = backchar(f, n);             /* of them all.         */
        return(s);
}

/*
 * Insert a newline. Bound to "C-M". If we are in CMODE, do automatic
 * indentation as specified.
 */
PASCAL NEAR newline(f, n)

int f,n;	/* prefix flag and argument */

{
	register int    s;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
	if (n < 0)
		return(FALSE);

	/* if we are in C mode and this is a default <NL> */
	if (n == 1 && (curbp->b_mode & MDCMOD) &&
	    curwp->w_dotp != curbp->b_linep)
		return(cinsert());

        /*
         * If a newline was typed, fill column is defined, the argument is non-
         * negative, wrap mode is enabled, and we are now past fill column,
	 * and we are not read-only, perform word wrap.
         */
        if ((curwp->w_bufp->b_mode & MDWRAP) && fillcol > 0 &&
	    getccol(FALSE) > fillcol &&
	    (curwp->w_bufp->b_mode & MDVIEW) == FALSE)
		execkey(&wraphook, FALSE, 1);

	/* insert some lines */
	while (n--) {
		if ((s=lnewline()) != TRUE)
			return(s);
	}
	return(TRUE);
}

PASCAL NEAR cinsert()	/* insert a newline and indentation for C */

{
	register char *cptr;	/* string pointer into text to copy */
	register int i;		/* index into line to copy indent from */
	register int llen;	/* length of line to copy indent from */
	register int bracef;	/* was there a brace at the end of line? */
	register LINE *lp;	/* current line pointer */
	register int offset;
	char ichar[NSTRING];	/* buffer to hold indent of last line */

	/* trim the whitespace before the point */
	lp = curwp->w_dotp;
	offset = curwp->w_doto;
	while (offset > 0 &&
	    (lgetc(lp, offset - 1) == ' ' ||
	    lgetc(lp, offset - 1) == '\t')) {
		backdel(FALSE, 1);
	       	offset--;
	}

	/* check for a brace */
	bracef = (offset > 0 && lgetc(lp, offset - 1) == '{');

	/* put in the newline */
	if (lnewline() == FALSE)
		return(FALSE);

	/* if the new line is not blank... don't indent it! */
	lp = curwp->w_dotp;
	if (lp->l_used != 0)
		return(TRUE);

	/* hunt for the last non-blank line to get indentation from */
	while (lp->l_used == 0 && lp != curbp->b_linep)
		lp = lp->l_bp;

	/* grab a pointer to text to copy indentation from */
	cptr = &(lp->l_text[0]);
	llen = lp->l_used;

	/* save the indent of the last non blank line */
	i = 0;
	while ((i < llen) && (cptr[i] == ' ' || cptr[i] == '\t')
		&& (i < NSTRING - 1)) {
		ichar[i] = cptr[i];
		++i;
	}
	ichar[i] = 0;		/* terminate it */

	/* insert this saved indentation */
	linstr(ichar);

	/* and one more tab for a brace */
	if (bracef)
		tab(FALSE, 1);

	return(TRUE);
}

PASCAL NEAR insbrace(n, c)	/* insert a brace into the text here...we are in CMODE */

int n;	/* repeat count */
int c;	/* brace to insert (always } for now) */

{
	register int ch;	/* last character before input */
	register int oc;	/* caractere oppose a c */
	register int i, count;
	register int target;	/* column brace should go after */
	register LINE *oldlp;
	register int  oldoff;

	/* if we aren't at the beginning of the line... */
	if (curwp->w_doto != 0)

	/* scan to see if all space before this is white space */
		for (i = curwp->w_doto - 1; i >= 0; --i) {
			ch = lgetc(curwp->w_dotp, i);
			if (ch != ' ' && ch != '\t')
				return(linsert(n, c));
		}

	/* chercher le caractere oppose correspondant */
	switch (c) {
		case '}': oc = '{'; break;
		case ']': oc = '['; break;
		case ')': oc = '('; break;
		default: return(FALSE);
	}
	
	oldlp = curwp->w_dotp;
	oldoff = curwp->w_doto;
	
	count = 1; backchar(FALSE, 1);
	
	while (count > 0) {
		if (curwp->w_doto == llength(curwp->w_dotp))
			ch = '\r';
		else
			ch = lgetc(curwp->w_dotp, curwp->w_doto);

		if (ch == c)  ++count;
		if (ch == oc) --count;
		
		backchar(FALSE, 1);
		if (boundry(curwp->w_dotp, curwp->w_doto, REVERSE))
			break;
	}
	
	if (count != 0) {	/* no match */
		curwp->w_dotp = oldlp;
		curwp->w_doto = oldoff;
		return(linsert(n, c));
	}
	
	curwp->w_doto = 0;		/* debut de ligne */
	/* aller au debut de la ligne apres la tabulation */
	while ((ch = lgetc(curwp->w_dotp, curwp->w_doto)) == ' ' || ch == '\t')
		forwchar(FALSE, 1);

	/* delete back first */
	target = getccol(FALSE);	/* c'est l'indent que l'on doit avoir */
	curwp->w_dotp = oldlp;
	curwp->w_doto = oldoff;
	
	while (target != getccol(FALSE)) {
		if (target < getccol(FALSE))	/* on doit detruire des caracteres */
			while (getccol(FALSE) > target)
				backdel(FALSE, 1);
		else {				/* on doit en inserer */
			while (target - getccol(FALSE) >= tabsize)
				linsert(1,'\t');
			linsert(target - getccol(FALSE), ' ');
		}
	}

	/* and insert the required brace(s) */
	return(linsert(n, c));
}

PASCAL NEAR inspound()	/* insert a # into the text here...we are in CMODE */

{
	register int ch;	/* last character before input */
	register int i;

	/* if we are at the beginning of the line, no go */
	if (curwp->w_doto == 0)
		return(linsert(1,'#'));

	/* scan to see if all space before this is white space */
	for (i = curwp->w_doto - 1; i >= 0; --i) {
		ch = lgetc(curwp->w_dotp, i);
		if (ch != ' ' && ch != '\t')
			return(linsert(1, '#'));
	}

	/* delete back first */
	while (getccol(FALSE) >= 1)
		backdel(FALSE, 1);

	/* and insert the required pound */
	return(linsert(1, '#'));
}

/*
 * Delete blank lines around dot. What this command does depends if dot is
 * sitting on a blank line. If dot is sitting on a blank line, this command
 * deletes all the blank lines above and below the current line. If it is
 * sitting on a non blank line then it deletes all of the blank lines after
 * the line. Normally this command is bound to "C-X C-O". Any argument is
 * ignored.
 */
PASCAL NEAR deblank(f, n)

int f,n;	/* prefix flag and argument */

{
        register LINE   *lp1;
        register LINE   *lp2;
        long nld;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        lp1 = curwp->w_dotp;
        while (llength(lp1)==0 && (lp2=lback(lp1))!=curbp->b_linep)
                lp1 = lp2;
        lp2 = lp1;
        nld = 0;
        while ((lp2=lforw(lp2))!=curbp->b_linep && llength(lp2)==0)
                ++nld;
        if (nld == 0)
                return(TRUE);
        curwp->w_dotp = lforw(lp1);
        curwp->w_doto = 0;
        return(ldelete(nld, FALSE));
}

/*
 * Insert a newline, then enough tabs and spaces to duplicate the indentation
 * of the previous line. Tabs are every tabsize characters. Quite simple.
 * Figure out the indentation of the current line. Insert a newline by calling
 * the standard routine. Insert the indentation by inserting the right number
 * of tabs and spaces. Return TRUE if all ok. Return FALSE if one of the
 * subcomands failed. Normally bound to "C-J".
 */
PASCAL NEAR indent(f, n)

int f,n;	/* prefix flag and argument */

{
        register int    nicol;
        register int    c;
        register int    i;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return(FALSE);
        while (n--) {
                nicol = 0;
                for (i=0; i<llength(curwp->w_dotp); ++i) {
                        c = lgetc(curwp->w_dotp, i);
                        if (c!=' ' && c!='\t')
                                break;
                        if (c == '\t')
				nicol += -(nicol % tabsize) + (tabsize - 1);
                        ++nicol;
                }
                if (lnewline() == FALSE
                || ((i=nicol/tabsize)!=0 && linsert(i, '\t')==FALSE)
                || ((i=nicol%tabsize)!=0 && linsert(i,  ' ')==FALSE))
                        return(FALSE);
        }
        return(TRUE);
}

/*
 * Delete forward. This is real easy, because the basic delete routine does
 * all of the work. Watches for negative arguments, and does the right thing.
 * If any argument is present, it kills rather than deletes, to prevent loss
 * of text if typed with a big argument. Normally bound to "C-D".
 */
PASCAL NEAR forwdel(f, n)

int f,n;	/* prefix flag and argument */

{
	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return(backdel(f, -n));
        if (f != FALSE) {                       /* Really a kill.       */
                if ((lastflag&CFKILL) == 0)
                        next_kill();
                thisflag |= CFKILL;
        }
        return(ldelete((long)n, f));
}

/*
 * Delete backwards. This is quite easy too, because it's all done with other
 * functions. Just move the cursor back, and delete forwards. Like delete
 * forward, this actually does a kill if presented with an argument. Bound to
 * both "RUBOUT" and "C-H".
 */
PASCAL NEAR backdel(f, n)

int f,n;	/* prefix flag and argument */

{
        register int    s;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return(forwdel(f, -n));
        if (f != FALSE) {                       /* Really a kill.       */
                if ((lastflag&CFKILL) == 0)
                        next_kill();
                thisflag |= CFKILL;
        }
        if ((s=backchar(f, n)) == TRUE)
                s = ldelete((long)n, f);
        return(s);
}

/*
 * Kill text. If called without an argument, it kills from dot to the end of
 * the line, unless it is at the end of the line, when it kills the newline.
 * If called with an argument of 0, it kills from the start of the line to dot.
 * If called with a positive argument, it kills from dot forward over that
 * number of newlines. If called with a negative argument it kills backwards
 * that number of newlines. Normally bound to "C-K".
 */
PASCAL NEAR killtext(f, n)

int f,n;	/* prefix flag and argument */

{
        register LINE   *nextp;
        long chunk;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if ((lastflag&CFKILL) == 0)             /* Clear kill buffer if */
                next_kill();            /* last wasn't a kill.  */
        thisflag |= CFKILL;

        if (f == FALSE) {
                chunk = llength(curwp->w_dotp)-curwp->w_doto;
                if (chunk == 0)
                        chunk = 1;
	} else if (n == 0) {
                chunk = -curwp->w_doto;
        } else if (n > 0) {
                chunk = llength(curwp->w_dotp)-curwp->w_doto+1;
                nextp = lforw(curwp->w_dotp);
                while (--n) {
                        if (nextp == curbp->b_linep)
                                return(FALSE);
                        chunk += llength(nextp)+1;
                        nextp = lforw(nextp);
                }
        } else if (n < 0) {
                chunk = -curwp->w_doto;
                nextp = lback(curwp->w_dotp);
                while (n++) {
                        if (nextp == curbp->b_linep)
                                return(FALSE);
                        chunk -= llength(nextp)+1;
                        nextp = lback(nextp);
                }
        }
        return(ldelete(chunk, TRUE));
}

PASCAL NEAR setmod(f, n)	/* prompt and set an editor mode */

int f, n;	/* default and argument */

{
	return(adjustmode(TRUE, FALSE));
}

PASCAL NEAR delmode(f, n)	/* prompt and delete an editor mode */

int f, n;	/* default and argument */

{
	return(adjustmode(FALSE, FALSE));
}

PASCAL NEAR setgmode(f, n)	/* prompt and set a global editor mode */

int f, n;	/* default and argument */

{
	return(adjustmode(TRUE, TRUE));
}

PASCAL NEAR delgmode(f, n)	/* prompt and delete a global editor mode */

int f, n;	/* default and argument */

{
	return(adjustmode(FALSE, TRUE));
}

PASCAL NEAR adjustmode(kind, global)	/* change the editor mode status */

int kind;	/* true = set,		false = delete */
int global;	/* true = global flag,	false = current buffer flag */
{
	register char *scan;		/* scanning pointer to convert prompt */
	register int i;			/* loop index */
	register int status;		/* error return on input */
#if	COLOR
	register int uflag;		/* was modename uppercase?	*/
#endif
	char prompt[50];	/* string to prompt user with */
	char cbuf[NPAT];		/* buffer to recieve mode name into */

	/* build the proper prompt string */
	if (global)
		strcpy(prompt,TEXT62);
/*                            "Global mode to " */
	else
		strcpy(prompt,TEXT63);
/*                            "Mode to " */

	if (kind == TRUE)
		strcat(prompt, TEXT64);
/*                             "add: " */
	else
		strcat(prompt, TEXT65);
/*                             "delete: " */

	/* prompt the user and get an answer */

	status = mlreply(prompt, cbuf, NPAT - 1);
	if (status != TRUE)
		return(status);

	/* make it uppercase */

	scan = cbuf;
#if	COLOR
	uflag = (*scan >= 'A' && *scan <= 'Z');
#endif
	while (*scan)
		uppercase(scan++);

	/* test it first against the colors we know */
	if ((i = lookup_color(cbuf)) != -1) {

#if	COLOR
		/* finding the match, we set the color */
		if (global) {
			if (uflag)
				gfcolor = i;
			else
				gbcolor = i;
#if	WINDOW_TEXT & 0
			refresh_screen(first_screen);
#endif
		} else
			if (uflag)
				curwp->w_fcolor = i;
			else
				curwp->w_bcolor = i;

		curwp->w_flag |= WFCOLR;
#endif
		mlerase();
		return(TRUE);
	}

	/* test it against the modes we know */

	for (i=0; i < NUMMODES; i++) {
		if (strcmp(cbuf, modename[i]) == 0) {
			/* finding a match, we process it */
			if (kind == TRUE)
				if (global) {
					gmode |= (1 << i);
					if ((1 << i) == MDOVER)
						gmode &= ~MDREPL;
					else if ((1 << i) == MDREPL)
						gmode &= ~MDOVER;
				} else {
					curbp->b_mode |= (1 << i);
					if ((1 << i) == MDOVER)
						curbp->b_mode &= ~MDREPL;
					else if ((1 << i) == MDREPL)
						curbp->b_mode &= ~MDOVER;
				}
			else
				if (global)
					gmode &= ~(1 << i);
				else
					curbp->b_mode &= ~(1 << i);
			/* display new mode line */
			if (global == 0)
				upmode();
			mlerase();	/* erase the junk */
			return(TRUE);
		}
	}

	mlwrite(TEXT66);
/*              "No such mode!" */
	return(FALSE);
}

/*	This function simply clears the message line,
		mainly for macro usage			*/

PASCAL NEAR clrmes(f, n)

int f, n;	/* arguments ignored */

{
	mlforce("");
	return(TRUE);
}

/*	This function writes a string on the message line
		mainly for macro usage			*/

PASCAL NEAR writemsg(f, n)

int f, n;	/* arguments ignored */

{
	register int status;
	char buf[NPAT];		/* buffer to recieve message into */

	if ((status = mlreply(TEXT67, buf, NPAT - 1)) != TRUE)
/*                            "Message to write: " */
		return(status);

	/* expand all '%' to "%%" so mlwrite won't expect arguments */
	makelit(buf);

	/* write the message out */
	mlforce(buf);
	return(TRUE);
}

/*	the cursor is moved to a matching fence	*/

PASCAL NEAR getfence(f, n)

int f, n;	/* not used */

{
	register LINE *oldlp;	/* original line pointer */
	register int oldoff;	/* and offset */
	register int sdir;	/* direction of search (1/-1) */
	register int count;	/* current fence level count */
	register char ch;	/* fence type to match against */
	register char ofence;	/* open fence */
	register char c;	/* current character in scan */

	/* save the original cursor position */
	oldlp = curwp->w_dotp;
	oldoff = curwp->w_doto;

	/* get the current character */
	if (oldoff == llength(oldlp))
		ch = '\r';
	else
		ch = lgetc(oldlp, oldoff);

	/* setup proper matching fence */
	switch (ch) {
		case '(': ofence = ')'; sdir = FORWARD; break;
		case '{': ofence = '}'; sdir = FORWARD; break;
		case '[': ofence = ']'; sdir = FORWARD; break;
		case ')': ofence = '('; sdir = REVERSE; break;
		case '}': ofence = '{'; sdir = REVERSE; break;
		case ']': ofence = '['; sdir = REVERSE; break;
		default: TTbeep(); return(FALSE);
	}

	/* set up for scan */
	count = 1;

	/* scan until we find it, or reach the end of file */
	while (count > 0) {
		if (sdir == FORWARD)
			forwchar(FALSE, 1);
		else
			backchar(FALSE, 1);

		if (curwp->w_doto == llength(curwp->w_dotp))
			c = '\r';
		else
			c = lgetc(curwp->w_dotp, curwp->w_doto);
		if (c == ch)
			++count;
		if (c == ofence)
			--count;
		if (boundry(curwp->w_dotp, curwp->w_doto, sdir))
			break;
	}

	/* if count is zero, we have a match, move the sucker */
	if (count == 0) {
		curwp->w_flag |= WFMOVE;
		return(TRUE);
	}

	/* restore the current position */
	curwp->w_dotp = oldlp;
	curwp->w_doto = oldoff;
	TTbeep();
	return(FALSE);
}

/*	Close fences are matched against their partners, and if
	on screen the cursor briefly lights there		*/

#if	PROTO
PASCAL NEAR fmatch(char ch)
#else
PASCAL NEAR fmatch(ch)

char ch;	/* fence type to match against */
#endif

{
	register LINE *oldlp;	/* original line pointer */
	register int oldoff;	/* and offset */
	register LINE *toplp;	/* top line in current window */
	register int count;	/* current fence level count */
	register char opench;	/* open fence */
	register char c;	/* current character in scan */
	register int i;

	/* first get the display update out there */
	update(FALSE);

	/* save the original cursor position */
	oldlp = curwp->w_dotp;
	oldoff = curwp->w_doto;

	/* setup proper open fence for passed close fence */
	if (ch == ')')
		opench = '(';
	else if (ch == '}')
		opench = '{';
	else
		opench = '[';

	/* find the top line and set up for scan */
	toplp = curwp->w_linep->l_bp;
	count = 1;
	backchar(FALSE, 1);

	/* scan back until we find it, or reach past the top of the window */
	while (count > 0 && curwp->w_dotp != toplp) {
		backchar(FALSE, 1);
		if (curwp->w_doto == llength(curwp->w_dotp))
			c = '\r';
		else
			c = lgetc(curwp->w_dotp, curwp->w_doto);
		if (c == ch)
			++count;
		if (c == opench)
			--count;
		if (curwp->w_dotp == curwp->w_bufp->b_linep->l_fp &&
		    curwp->w_doto == 0)
			break;
	}

	/* if count is zero, we have a match, display the sucker */
	/* there is a real machine dependant timing problem here we have
	   yet to solve......... */
	if (count == 0) {
		for (i = 0; i < term.t_pause; i++)
			update(FALSE);
	}

	/* restore the current position */
	curwp->w_dotp = oldlp;
	curwp->w_doto = oldoff;
	return(TRUE);
}

PASCAL NEAR istring(f, n)	/* ask for and insert a string into the current
		   buffer at the current point */

int f, n;	/* ignored arguments */

{
	register int status;	/* status return code */
	char tstring[NPAT+1];	/* string to add */

	/* ask for string to insert */
	status = mltreply(TEXT68, tstring, NPAT, sterm);
/*                        "String to insert<META>: " */
	if (status != TRUE)
		return(status);

	if (f == FALSE)
		n = 1;

	if (n < 0)
		n = - n;

	/* insert it */
	while (n-- && (status = linstr(tstring)))
		;
	return(status);
}

PASCAL NEAR ovstring(f, n) /* ask for and overwite a string into the current
		   buffer at the current point */

int f, n;	/* ignored arguments */

{
	register int status;	/* status return code */
	char tstring[NPAT+1];	/* string to add */

	/* ask for string to insert */
	status = mltreply(TEXT69, tstring, NPAT, sterm);
/*                        "String to overwrite<META>: " */
	if (status != TRUE)
		return(status);

	if (f == FALSE)
		n = 1;

	if (n < 0)
		n = - n;

	/* insert it */
	while (n-- && (status = lover(tstring)))
		;
	return(status);
}

int PASCAL NEAR lookup_color(sp)

char *sp;	/* name to look up */

{
	register int i;		/* index into color list */

	/* test it against the colors we know */
	for (i = 0; i < NCOLORS; i++) {
		if (strcmp(sp, cname[i]) == 0)
			return(i);
	}
	return(-1);
}

