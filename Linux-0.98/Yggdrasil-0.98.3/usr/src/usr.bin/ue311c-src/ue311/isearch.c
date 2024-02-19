/*
 * The functions in this file implement commands that perform incremental
 * searches in the forward and backward directions.  This "ISearch" command
 * is intended to emulate the same command from the original EMACS
 * implementation (ITS).  Contains references to routines internal to
 * SEARCH.C.
 *
 * REVISION HISTORY:
 *
 *	D. R. Banks 9-May-86
 *	- added ITS EMACSlike ISearch
 *
 *	John M. Gamble 5-Oct-86
 *	- Made iterative search use search.c's scanner() routine.
 *	  This allowed the elimination of bakscan().
 *	- Put isearch constants into estruct.h
 *	- Eliminated the passing of 'status' to scanmore() and
 *	  checknext(), since there were no circumstances where
 *	  it ever equalled FALSE.
 *	Dan Corkill 6-Oct-87
 *      - Changed character loop to terminate with extended characters
 *        (thus arrow keys, and most commands behave intuitively).
 *      - Changed META to be reread rather than simply aborting.
 *      - Conditionalized VMS alternates for ^S and ^Q to only apply
 *        to VMS ports.  (Allowing ^X as a synonym for ^S defeats some
 *        of the benefits of the first change above.)
 */

#include <stdio.h>
#include "estruct.h"
#include "eproto.h"
#include "edef.h"
#include "elang.h"

#if	ISRCH

#define CMDBUFLEN	256	/* Length of our command buffer */

/* A couple of "own" variables for re-eat */
/* Hey, BLISS user, these were "GLOBAL", I made them "OWN". */
static int	(PASCAL NEAR *saved_get_char)();/* Get character routine */
static int	eaten_char = -1;		/* Re-eaten char */

/* A couple more "own" variables for the command string */

static int cmd_buff[CMDBUFLEN];		/* Save the command args here */
static int cmd_offset;			/* Current offset into command buff */
static int cmd_reexecute = -1;		/* > 0 if re-executing command */

/*
 * Subroutine to do incremental reverse search.  It actually uses the
 * same code as the normal incremental search, as both can go both ways.
 */
int PASCAL NEAR risearch(f, n)

int f,n;	/* prefix flag and argument */

{
    register int	status;

    /* Make sure the search doesn't match where we already are:		      */

    backchar(TRUE, 1);			/* Back up a character		      */

    if (status = isearch(REVERSE))
	mlerase();			/* If happy, just erase the cmd line  */
    else
	mlwrite(TEXT164);
/*               "[search failed]" */
    return (status);
}

/* Again, but for the forward direction */

int PASCAL NEAR fisearch(f, n)

int f,n;	/* prefix flag and argument */

{
    register int	 status;

    if (status = isearch(FORWARD))
	mlerase();			/* If happy, just erase the cmd line  */
    else
	mlwrite(TEXT164);
/*               "[search failed]" */
    return (status);
}

/*
 * Subroutine to do an incremental search.  In general, this works similarly
 * to the older micro-emacs search function, except that the search happens
 * as each character is typed, with the screen and cursor updated with each
 * new search character.
 *
 * While searching forward, each successive character will leave the cursor
 * at the end of the entire matched string.  Typing a Control-S
 * will cause the next occurrence of the string to be searched for (where the
 * next occurrence does NOT overlap the current occurrence).  A Control-R will
 * change to a backwards search, META will terminate the search and Control-G
 * will abort the search.  Rubout will back up to the previous match of the
 * string, or if the starting point is reached first, it will delete the
 * last character from the search string.
 *
 * While searching backward, each successive character will leave the cursor
 * at the beginning of the matched string.  Typing a Control-R will search
 * backward for the next occurrence of the string.  Control-S
 * will revert the search to the forward direction.  In general, the reverse
 * incremental search is just like the forward incremental search inverted.
 *
 * In all cases, if the search fails, the user will be feeped, and the search
 * will stall until the pattern string is edited back into something that
 * exists (or until the search is aborted).
 */
int PASCAL NEAR isearch(dir)
int	dir;
{
    int			status;		/* Search status */
    int			col;		/* prompt column */
    register int	cpos;		/* character number in search string  */
    register int	c;		/* current input character */
    register int	expc;		/* function expanded input char	      */
    char		pat_save[NPAT];	/* Saved copy of the old pattern str  */
    LINE		*curline;	/* Current line on entry	      */
    int			curoff;		/* Current offset on entry	      */
    int			init_direction;	/* The initial search direction	      */
    KEYTAB		*ktp;		/* The command bound to the key	      */
    register int (PASCAL NEAR *kfunc)();/* ptr to the requested function to bind to */

    /* Initialize starting conditions */

    cmd_reexecute = -1;			/* We're not re-executing (yet?)      */
    cmd_offset = 0;			/* Start at the beginning of cmd_buff */
    cmd_buff[0] = '\0';			/* Reset the command buffer	      */
    bytecopy(pat_save, pat, NPAT);	/* Save the old pattern string	      */
    curline = curwp->w_dotp;		/* Save the current line pointer      */
    curoff  = curwp->w_doto;		/* Save the current offset	      */
    init_direction = dir;		/* Save the initial search direction  */


start_over:    /* This is a good place to start a re-execution: */

    /*
     * Ask the user for the text of a pattern,
     * and remember the col.
     */
    col = promptpattern(TEXT165);
/*                      "ISearch: " */

    cpos = 0;					/* Start afresh		      */
    status = TRUE;				/* Assume everything's cool   */

    for (;;)					/* ISearch per character loop */
    {
	/* Check for special characters first.
         * That is, a control or ^X or FN or mouse function.
	 * Most cases here change the search.
	 */
	c = ectoc(expc = get_char());

	if (expc == sterm) {			/* Want to quit searching?    */
	    setjtable();			/* Update the jump tables     */
	    return(TRUE);			/* Quit searching now	      */
	}

	if (expc == abortc)			/* If abort search request    */
	    break;				/* Quit searching	      */

	if (expc == quotec)			/* Quote character?	      */
	{
	    c = ectoc(expc = get_char());	/* Get the next char	      */
	}
	else if ((expc > 255 || expc == 0) && (c != '\t' && c != '\r'))
	{
	    if (ktp = getbind(expc))
	    	kfunc = ktp->k_ptr.fp;
	    else
	        kfunc = NULL;

	    if (kfunc == forwsearch || kfunc == forwhunt || kfunc == fisearch || 
		kfunc == backsearch || kfunc == backhunt || kfunc == risearch)
            {
            	dir = (kfunc == backsearch || kfunc == backhunt || kfunc == risearch)? REVERSE: FORWARD;

		/*
		 * if cpos == 0 then we are either just starting
		 * or starting over.  Use the original pattern
		 * in pat, which has either not been changed or
		 * has just been restored.  Find the length and
		 * re-echo the string.
		 */
	        if (cpos == 0)
	            while (pat[cpos] != 0)
	                col = echochar((int)pat[cpos++],col);

	        status = scanmore(dir);
	        continue;
            }
            else if (kfunc == backdel)
            {
	        if (cmd_offset <= 1)		/* Anything to delete?	      */
	            return(TRUE);		/* No, just exit	      */

	        cmd_offset -= 2;		/* Back up over the Rubout    */
	        cmd_buff[cmd_offset] = '\0';	/* Yes, delete last char   */
	        curwp->w_dotp = curline;	/* Reset the line pointer     */
	        curwp->w_doto = curoff;		/*  and the offset	      */
		dir = init_direction;		/* Reset the search direction */
		bytecopy(pat, pat_save, NPAT);	/* Restore the old search str */
		setjtable();			/* and its jump tables.       */
		cmd_reexecute = 0;		/* Start the whole mess over  */
		goto start_over;		/* Let it take care of itself */
            }

            /* Presumably the key was uninteresting...*/

            reeat(expc);		/* Re-eat the char	      */
            return(TRUE);		/* And return the last status */
	}

	/* I guess we got something to search for, so search for it	      */

	pat[cpos++] = c;		/* put the char in the buffer */

	if (cpos >= NPAT)			/* too many chars in string?  */
	{					/* Yup.  Complain about it    */
	    mlwrite(TEXT166);
/*                  "? Search string too long" */
	    bytecopy(pat, pat_save, NPAT);	/* Restore the old search str */
	    setjtable();			/* and its jump tables.       */
	    return(FALSE);			/* Return an error, but stay. */
	}

	pat[cpos] = 0;				/* null terminate the buffer  */
	col = echochar(c,col);			/* Echo the character	      */
	if (!status)				/* If we lost last time	      */
	    TTbeep();				/* Feep again		*/
	else					/* Otherwise, we must have won*/
	    status = checknext(c, dir);	/* See if still matches or find next */

    }	/* for {;;} */

    curwp->w_dotp = curline;		/* Reset the line pointer	      */
    curwp->w_doto = curoff;		/*  and the offset to original value  */
    curwp->w_flag |= WFMOVE;		/* Say we've moved		      */
    update(FALSE);			/* And force an update		      */
    return (FALSE);
}

/*
 * This hack will search for the next occurrence of <pat> in the buffer,
 * either forward or backward.  If we can't find any more matches, "point"
 * is left where it was before.  If we do find a match, "point" will be at
 * the end of the matched string for forward searches and at the beginning
 * of the matched string for reverse searches.
 */

int PASCAL NEAR scanmore(dir)
int	dir;			/* direction to search			*/
{
	register int	sts;		/* search status		*/

	setjtable();			/* Set up fast search arrays	*/

	sts = scanner(dir, (dir == REVERSE)? PTBEG: PTEND, 1);

	if (!sts)
		TTbeep();	/* Feep if search fails       */

	return(sts);
}

/*
 * Trivial routine to insure that the next character in the search
 * string is still true to whatever we're pointing to in the buffer.
 * This routine will not attempt to move the "point" if the match
 * fails, although it will implicitly move the "point" if we're
 * forward searching, and find a match, since that's the way forward
 * isearch works.  If we are reverse searching we compare all
 * characters in the pattern string from "point" to the new end.
 *
 * If the compare fails, we return FALSE and call scanmore or something.
 */
int PASCAL NEAR checknext(chr, dir)
int	chr;			/* Next char to look for		 */
int	dir;			/* Search direction			 */
{
    LINE *curline;			/* current line during scan	      */
    int curoff;				/* position within current line	      */
    register char *patrn;		/* The entire search string (incl chr)   */
    register int sts;			/* how well things go		      */

    /* setup the local scan pointer to current "." */

    curline = curwp->w_dotp;		/* Get the current line structure     */
    curoff  = curwp->w_doto;		/* Get the offset within that line    */

    if (dir == FORWARD)			/* If searching forward		 */
    {
	if (sts = !boundry(curline, curoff, FORWARD))
	{
	    /* Is it what we're looking for?      */
	    if (sts = eq(nextch(&curline, &curoff, FORWARD), chr))
	    {
		curwp->w_dotp = curline;	/* Yes, set the buffer's point */
		curwp->w_doto = curoff;		/*  to the matched character */
		curwp->w_flag |= WFMOVE;	/* Say that we've moved */
	    }
	}
    }
    else		/* Else, reverse search check. */
    {
    	patrn = pat;
        while (*patrn)	/* Loop for all characters in patrn   */
        {
	    if ((sts = !boundry(curline, curoff, FORWARD)) == FALSE ||
	        (sts = eq(nextch(&curline, &curoff, FORWARD), *patrn)) == FALSE)
		    break;		/* Nope, just punt it then	*/

	    patrn++;
        }
    }

    /*
     * If the 'next' character didn't fit in the pattern,
     * let's go search for it somewhere else.
     */
    if (sts == FALSE)
        sts = scanmore(dir);

    return(sts);		/* And return the status		*/
}

/*
 * Routine to prompt for I-Search string.
 */

int PASCAL NEAR promptpattern(prompt)
char *prompt;
{
    char tpat[NPAT+20];

    strcpy(tpat, prompt);		/* copy prompt to output string */
    strcat(tpat, " [");			/* build new prompt string */
    expandp(pat, &tpat[strlen(tpat)], NPAT/2);	/* add old pattern */
    strcat(tpat, "]<META>: ");

    /* check to see if we are executing a command line */
    if (!clexec) {
	mlwrite(tpat);
    }
    return(strlen(tpat));
}

/*
 * Routine to echo i-search characters
 */

int PASCAL NEAR echochar(c, col)
int c;		/* character to be echoed */
int col;	/* column to be echoed in */
{
    movecursor(term.t_nrow, col);	/* Position the cursor	      */
    if ((c < ' ') || (c == 0x7F))	/* Control character?	*/
    {
	switch (c)			/* Yes, dispatch special cases*/
	{
	  case '\r':			/* Newline	      */
	    mlout('<');
	    mlout('N');
	    mlout('L');
	    mlout('>');
	    col += 3;
	    break;

	  case '\t':			/* Tab		      */
	    mlout('<');
	    mlout('T');
	    mlout('A');
	    mlout('B');
	    mlout('>');
	    col += 4;
	    break;

	  default:		/* Vanilla control char and Rubout:   */
	    mlout('^');		/* Yes, output prefix	      */
	    mlout(c ^ 0x40);	/* Make it "^X"		      */
	    col++;		/* Count this char	      */
	}
    } else
	mlout(c);		/* Otherwise, output raw char */
    TTflush();			/* Flush the output	      */
    return(++col);		/* return the new column no   */
}

/*
 * Routine to get the next character from the input stream.  If we're reading
 * from the real terminal, force a screen update before we get the char.
 * Otherwise, we must be re-executing the command string, so just return the
 * next character.
 */

int PASCAL NEAR get_char()
{
    int	c;

    /* See if we're re-executing: */

    if (cmd_reexecute >= 0)		/* Is there an offset?		*/
	if ((c = cmd_buff[cmd_reexecute++]) != 0)
	    return(c);			/* Yes, return any character	*/

    /* We're not re-executing (or aren't any more).  Try for a real char
     */
    cmd_reexecute = -1;			/* Say we're in real mode again	*/
    update(FALSE);			/* Pretty up the screen		*/
    if (cmd_offset >= CMDBUFLEN-1)	/* If we're getting too big ...	*/
    {
	mlwrite (TEXT167);		/* Complain loudly and bitterly	*/
/*               "? command too long" */
	return(sterm);			/* And force a quit		*/
    }
    c = getkey();			/* Get the next character	*/

    cmd_buff[cmd_offset++] = c;		/* Save the char for next time	*/
    cmd_buff[cmd_offset] = '\0';	/* And terminate the buffer	*/
    return(c);				/* Return the character		*/
}

/*
 * Hacky routine to re-eat a character.  This will save the character to be
 * re-eaten by redirecting the input call to a routine here.  Hack, etc.
 */

/* Come here on the next term.t_getchar call: */

int PASCAL NEAR uneat()
{
    int c;

    term.t_getchar = saved_get_char;	/* restore the routine address	      */
    c = eaten_char;			/* Get the re-eaten char	      */
    eaten_char = -1;			/* Clear the old char		      */
    return(c);				/* and return the last char	      */
}

VOID PASCAL NEAR reeat(c)
int	c;
{
    if (eaten_char != -1)		/* If we've already been here	      */
	return/*(NULL)*/;		/* Don't do it again		      */
    eaten_char = c;			/* Else, save the char for later      */
    saved_get_char = term.t_getchar;	/* Save the char get routine	      */
    term.t_getchar = uneat;		/* Replace it with ours		      */
}
#else
int PASCAL NEAR isearch(dir)

int dir;

{
}
#endif
