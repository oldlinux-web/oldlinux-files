/*	SCREEN.C:	Screen manipulation commands
			for MicroEMACS 3.11
			written by Daniel Lawrence
*/

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include        "edef.h"
#include	"elang.h"

#if	0
dumpscreens(msg)

char *msg;

{
	SCREEN *sp;

	printf("<%s>\n", msg);
	sp = first_screen;
	while (sp != (SCREEN *)NULL) {
		printf("%lu - [%s] %d/%d to %d/%d \n", sp, sp->s_screen_name,
		sp->s_roworg, sp->s_colorg, sp->s_nrow, sp->s_ncol);
		sp = sp->s_next_screen;
	}
	printf("   0     -   [EOL]\n");
	tgetc();
}
#endif

/*	This command takes the last window in the linked window list,
	which is visibly rearmost, and brings it to front. It is bound
	to A-N on machines with an ALT key
*/

PASCAL NEAR cycle_screens(f, n)

int f,n;	/* prefix flag and argument */

{
	SCREEN *sp;		/* ptr to screen to switch to */

	/* find the last screen */
	sp = first_screen;
	while (sp->s_next_screen != (SCREEN *)NULL)
		sp = sp->s_next_screen;

	/* and make this screen current */
	return(select_screen(sp, TRUE));
}

PASCAL NEAR find_screen(f, n)

int f,n;	/* prefix flag and argument */

{
	char scr_name[NSTRING];	/* buffer to hold screen name */
	SCREEN *sp;		/* ptr to screen to switch to */

	/* get the name of the screen to switch to */
	mlreply("Find Screen: ", scr_name, NSTRING);
	sp = lookup_screen(scr_name);

	if (sp == (SCREEN *)NULL) {

		/* save the current dot position in the buffer info
		   so the new screen will start there! */
		curbp->b_dotp = curwp->w_dotp;
		curbp->b_doto = curwp->w_doto;

		/* screen does not exist, create it */
		sp = init_screen(scr_name, curbp);
	}

	/* and make this screen current */
	return(select_screen(sp, TRUE));
}

PASCAL NEAR free_screen(sp)	/* free all resouces associated with a screen */

SCREEN *sp;	/* screen to dump */

{
	register int cmark;	/* mark ordinal index */
	register WINDOW *wp;	/* ptr to window to free */
	register WINDOW *tp;	/* temp window pointer */

	/* first, free the screen's windows */
	wp = sp->s_first_window;
	while (wp) {
		if (--wp->w_bufp->b_nwnd == 0) {
			wp->w_bufp->b_dotp  = wp->w_dotp;
			wp->w_bufp->b_doto  = wp->w_doto;
			for (cmark = 0; cmark < NMARKS; cmark++) {
				wp->w_bufp->b_markp[cmark] = wp->w_markp[cmark];
				wp->w_bufp->b_marko[cmark] = wp->w_marko[cmark];
			}
			wp->w_bufp->b_fcol  = wp->w_fcol;
		}

		/* on to the next window, free this one */
		tp = wp->w_wndp;
		free((char *) wp);
		wp = tp;
	}

	/* and now, free the screen struct itself */
	free(sp->s_screen_name);
	free((char *) sp);
}

PASCAL NEAR delete_screen(f, n)

int f,n;	/* prefix flag and argument */

{
	char scr_name[NSTRING];	/* buffer to hold screen name */
	SCREEN *sp;		/* ptr to screen to switch to */
	SCREEN *last_scr;	/* screen previous to one to delete */

	/* get the name of the screen to delete */
	mlreply("Delete Screen: ", scr_name, NSTRING);
	sp = lookup_screen(scr_name);

	/* make sure it exists... */
	if (sp == (SCREEN *)NULL) {
		mlwrite("[No such screen]");
		return(FALSE);
	}

	/* it can't be current... */
	if (sp == first_screen) {
		mlwrite("%%Can't delete current screen");
		return(FALSE);
	}

	/* find the place to delete */
	last_scr = first_screen;
	while (last_scr) {
		if (last_scr->s_next_screen == sp)
			break;
		last_scr = last_scr->s_next_screen;
	}
	last_scr->s_next_screen = sp->s_next_screen;
	free_screen(sp);
#if	WINDOW_TEXT
			refresh_screen(first_screen);
#endif
	return(TRUE);
}

/* this function initializes a new screen.... */

SCREEN *PASCAL NEAR init_screen(scr_name, scr_buf)

char *scr_name;		/* screen name */
BUFFER *scr_buf;	/* buffer to place in first window of screen */

{
	int cmark;		/* current mark to initialize */
	SCREEN *sp;		/* pointer to allocated screen */
	SCREEN *last_sp;	/* pointer to last screen */
	WINDOW *wp;		/* ptr to first window of new screen */

	/* allocate memory for this screen */
	sp = (SCREEN *)malloc(sizeof(SCREEN));
	if (sp == (SCREEN *)NULL)
		return(sp);

	/* set up this new screens fields! */
	sp->s_next_screen = (SCREEN *)NULL;
	sp->s_screen_name = copystr(scr_name);
	sp->s_roworg = term.t_roworg;
	sp->s_colorg = term.t_colorg;
	sp->s_nrow = term.t_nrow;
	sp->s_ncol = term.t_ncol;

	/* allocate its first window */
	wp = (WINDOW *)malloc(sizeof(WINDOW));
	if (wp == (WINDOW *)NULL) {
		free((char *)sp);
		return((SCREEN *)NULL);
	}
	sp->s_first_window = sp->s_cur_window = wp;

	/* and setup the windows info */
	wp->w_wndp = NULL;
	wp->w_bufp = scr_buf;
	scr_buf->b_nwnd += 1;	
	wp->w_linep = scr_buf->b_linep;

	/* position us at the buffers dot */
	wp->w_dotp  = scr_buf->b_dotp;
	wp->w_doto  = scr_buf->b_doto;

	/* set all the marks to UNSET */
	for (cmark = 0; cmark < NMARKS; cmark++) {
		wp->w_markp[cmark] = NULL;
		wp->w_marko[cmark] = 0;
	}
	wp->w_toprow = 0;
#if	COLOR
	/* initalize colors to global defaults */
	wp->w_fcolor = gfcolor;
	wp->w_bcolor = gbcolor;
#endif
	wp->w_fcol = 0;
	wp->w_ntrows = term.t_nrow-1;		/* "-1" for mode line.	*/
	wp->w_force = 0;
	wp->w_flag  = WFMODE|WFHARD;		/* Full.		*/

	/* first screen? */
	if (first_screen == (SCREEN *)NULL) {
		first_screen = sp;
		return(sp);
	}

	/* insert it at the tail of the screen list */
	last_sp = first_screen;
	while (last_sp->s_next_screen != (SCREEN *)NULL)
		last_sp = last_sp->s_next_screen;
	last_sp->s_next_screen = sp;

	/* and return the new screen pointer */
	return(sp);
}

SCREEN *PASCAL NEAR lookup_screen(scr_name)

char *scr_name;		/* named screen to find */

{
	SCREEN *result;

	/* scan the screen list */
	result = first_screen;
	while (result) {

		/* if this is it, return its handle! */
		if (strcmp(scr_name, result->s_screen_name) == 0)
			return(result);

		/* on to the next screen */
		result = result->s_next_screen;
	}

	/* we didn't find it..... */
	return((SCREEN *)NULL);
}

int PASCAL NEAR select_screen(sp, announce)

SCREEN *sp;	/* ptr to screen to switch to */
int announce;	/* announce the selection? */

{
	WINDOW *temp_wp;	/* backup of current window ptr (curwp) */
	SCREEN *temp_screen;	/* temp ptr into screen list */

	/* make sure there is something here to set to! */
	if (sp == (SCREEN *)NULL)
		return(FALSE);

	/* nothing to do, it is already current */
	if (sp == first_screen)
		return(TRUE);

	/* deselect the current window */
	temp_wp = curwp;
	curwp = (WINDOW *)NULL;
	modeline(temp_wp);
	updupd(TRUE);
	curwp = temp_wp;

	/* save the current screens concept of current window */
	first_screen->s_cur_window = curwp;
	first_screen->s_roworg = term.t_roworg;
	first_screen->s_colorg = term.t_colorg;
	first_screen->s_nrow = term.t_nrow;
	first_screen->s_ncol = term.t_ncol;

	/* re-order the screen list */
	temp_screen = first_screen;
	while (temp_screen->s_next_screen != (SCREEN *)NULL) {
		if (temp_screen->s_next_screen == sp) {
			temp_screen->s_next_screen = sp->s_next_screen;
			break;
		}
		temp_screen = temp_screen->s_next_screen;
	}
	sp->s_next_screen = first_screen;
	first_screen = sp;

	/* reset the current screen, window and buffer */
	wheadp = first_screen->s_first_window;
	curwp = first_screen->s_cur_window;
	curbp = curwp->w_bufp;

	/* let the display driver know we need a full screen update */
	update_size();
	upwind();
	if (announce) {
		mlwrite(TEXT225, first_screen->s_screen_name);
/*			"[Switched to screen %s]" */
	}
	return(TRUE);
}

/*	Build and popup a buffer containing the list of all screens.
	Bound to "A-B".
*/

PASCAL NEAR list_screens(f, n)

int f,n;	/* prefix flag and argument */

{
	register int status;	/* stutus return */

	if ((status = screenlist(f)) != TRUE)
		return(status);
	return(wpopup(slistp));
}


/*
 * This routine rebuilds the
 * text in the special secret buffer
 * that holds the screen list. It is called
 * by the list screens command. Return TRUE
 * if everything works. Return FALSE if there
 * is an error (if there is no memory). Iflag
 * indecates weather to list hidden screens.
 */
PASCAL NEAR screenlist(iflag)

int iflag;	/* list hidden screen flag */

{
	SCREEN *sp;		/* ptr to current screen to list */
	WINDOW *wp;		/* ptr into current screens window list */
	int status;		/* return status from functions */
	char line[NSTRING];	/* buffer to construct list lines */
	char bname[NSTRING];	/* name of next buffer */

	/* mark this buffer as unchanged so... */
	slistp->b_flag &= ~BFCHG;

	/* we can dump it's old contents without complaint */
	if ((status = bclear(slistp)) != TRUE)
		return(status);

	/* there is no file connected with this buffer */
	strcpy(slistp->b_fname, "");

	/* construct the header of this list */
	if (addline(slistp, "Screen         Buffers") == FALSE
	 || addline(slistp, "------         -------") == FALSE)
		return(FALSE);

	/* starting from the first screen */
	sp = first_screen;

	/* scan all the screens */
	while (sp) {

		/* construct the screen name */
		strcpy(line, sp->s_screen_name);
		strcat(line, "                ");
		line[15] = 0;

		/* list this screens windows's buffer names */
		wp = sp->s_first_window;
		while (wp) {

			/* grab this window's buffer name */
			strcpy(bname, wp->w_bufp->b_bname);

			/* handle full lines */
			if (strlen(line) + strlen(bname) + 1 > 78) {
				if (addline(slistp, line) == FALSE)
					return(FALSE);
				strcpy(line, "               ");
			}

			/* append this buffer name */
			if (strlen(line) > 15)
				strcat(line, " ");
			strcat(line, bname);

			/* on to the next window */
			wp = wp->w_wndp;
		}

		/* and add the line to the buffer */
		if (addline(slistp, line) == FALSE)
			return(FALSE);

		/* on to the next screen */
		sp = sp->s_next_screen;
	}

	/* all constructed! */
	return(TRUE);
}


