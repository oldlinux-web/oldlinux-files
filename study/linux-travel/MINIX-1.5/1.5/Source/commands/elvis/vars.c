/* vars.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains variables which weren't happy anyplace else */

#include "vi.h"

/*------------------------------------------------------------------------*/

/* used to remember whether the file has been modified */
struct _viflags	viflags;

/* used to access the tmp file */
long		lnum[MAXBLKS];
long		nlines;
int		tmpfd = -1;

/* used to keep track of the current file & alternate file */
long		origtime;
char		origname[256];
char		prevorig[256];
long		prevline = 1;

/* used to track various places in the text */
MARK		mark[NMARKS];	/* marks 'a through 'z, plus mark '' */
MARK		cursor;		/* the cursor position within the file */

/* which mode of the editor we're in */
enum _mode	mode;		/* vi mode? ex mode? quitting? */

/* used to manage the args list */
char		args[BLKSIZE];	/* list of filenames to edit */
int		argno;		/* index of current file in args list */
int		nargs;		/* number of filenames in args[] */

/* dummy var, never explicitly referenced */
int		bavar;		/* used only in BeforeAfter macros */

/* have we made a multi-line change? */
int		mustredraw;	/* must we redraw the whole screen? */
long		redrawafter;	/* line# of first line that must be redrawn */
long		redrawpre;	/* line# of last line changed, before change */
long		redrawpost;	/* line# of last line changed, after change */
				/* (redrawpost - redrawpre) = #lines added */

/* used to detect changes that invalidate cached text/blocks */
long		changes;	/* incremented when file is changed */

/* used to support the pfetch() macro */
int		plen;		/* length of the line */
long		pline;		/* line number that len refers to */
long		pchgs;		/* "changes" level that len refers to */
char		*ptext;		/* text of previous line, if valid */

/* misc temporary storage - mostly for strings */
BLK		tmpblk;		/* a block used to accumulate changes */

/* screen oriented stuff */
long		topline;	/* file line number of top line */
int		leftcol;	/* column number of left col */
int		physcol;	/* physical column number that cursor is on */
int		physrow;	/* physical row number that cursor is on */

/* used to help minimize that "[Hit a key to continue]" message */
int		exwrote;	/* Boolean: was the last ex command wordy? */

/* This variable affects the behaviour of certain functions -- most importantly
 * the input function.
 */
int		doingdot;	/* boolean: are we doing the "." command? */

/* These are used for reporting multi-line changes to the user */
long		rptlines;	/* number of lines affected by a command */
char		*rptlabel;	/* description of how lines were affected */

/* These store info that pertains to the shift-U command */
long		U_line;		/* line# of the undoable line, or 0L for none */
char		U_text[BLKSIZE];/* contents of the undoable line */
