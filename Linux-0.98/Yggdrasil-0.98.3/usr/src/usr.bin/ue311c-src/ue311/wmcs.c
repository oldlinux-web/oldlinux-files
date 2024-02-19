/*	WMCS.C:	Operating specific I/O and Spawning functions
		for the WICAT computer/operating system
		for MicroEMACS 3.10
		(C)Copyright 1988 by Daniel M. Lawrence
*/

#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#if	WMCS
#include        "edef.h"
#include	"elang.h"

#include	"/sysincl.sys/devtdisp.h"
#include	"/sysincl.sys/dstatdisp.h"

devicetable	dtable;
devicestatus	dstat;
ushort	xmdstat = 0x019d;
ushort	xmdsave;
ushort	xmdmask = 0xfe62;

/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */
ttopen()
{
	/* set device status */
	_giodst(1,&dtable,sizeof(dtable),&dstat);
	xmdsave = dstat.class.tty.dstyflags1;
	dstat.class.tty.dstyflags1 &= xmdmask;	/* clear the bits we need to use */
	dstat.class.tty.dstyflags1 |= xmdstat;	/* set our status bits */
	_siodst(1,&dstat);	/* set device status */

	/* on all screens we are not sure of the initial position
	   of the cursor					*/
	ttrow = 999;
	ttcol = 999;
}

/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
ttclose()
{
	/* reset device status back to saved values */
	dstat.class.tty.dstyflags1 = xmdsave;
	_siodst(1,&dstat);
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
ttputc(c)
{
        fputc(c, stdout);
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
ttflush()
{
        fflush(stdout);
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. More complex in VMS that almost anyplace else, which figures. Very
 * simple on CPM, because the system can do exactly what you want.
 */
ttgetc()
{
	char c;
	long trns;
	_read(1,-1,0,-1,&c,1,&trns);
	return((int)(c & 0x7f));
}

#if	TERMCAP
/* get a character with timeout */
mttgetc()
{
	char c;
	long trns,status;
	
	status = _read(1,-1,0,4,&c,1,&trns);
	if(status) return(-1);
	return((int)(c & 0x7f));
}
#endif

#if	TYPEAH
/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/

typahead()

{
/*  This doesnt seem to work even though it should
    So I leave it commented out...

	devicetable	xdtable;
	devicestatus	xdstat;
	
	_giodst(1,&xdtable,sizeof(xdtable),&xdstat);
	return(dstat.class.tty.dstyinputcnt);
*/
	return(FALSE);
}
#endif

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
spawncli(f, n)
{
        register char *cp;
        char *getenv();

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
        TTclose();                              /* stty to old settings */
	if ((cp = getenv("SYS$CIP")) != NULL && *cp != '\0')
                system(cp);
        else
		system("CIP");

        sgarbf = TRUE;
	sleep(2);
        TTopen();
        return(TRUE);
}

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
spawn(f, n)
{
        register int    s;
        char            line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());
        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return (s);
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
        system(line);
        TTopen();
        mlputs(TEXT188);                        /* Pause.               */
/*             "[End]" */
        TTflush();
        while ((s = tgetc()) != '\r' && s != ' ')
                ;
        sgarbf = TRUE;
        return (TRUE);
}

/*
 * Run an external program with arguments. When it returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X $".
 */

execprg(f, n)

{
        register int    s;
        char            line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return (s);
        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
        system(line);
        TTopen();
        mlputs(TEXT188);                        /* Pause.               */
/*             "[End]" */
        TTflush();
        while ((s = tgetc()) != '\r' && s != ' ')
                ;
        sgarbf = TRUE;
        return (TRUE);
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
pipecmd(f, n)
{
        register int    s;	/* return status from CLI */
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
        char	line[NLINE];	/* command line send to shell */
	static char bname[] = "command";

	static char filnam[NSTRING] = "command";

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	/* get the command to pipe in */
        if ((s=mlreply("@", line, NLINE)) != TRUE)
                return(s);

	/* get rid of the command output buffer if it exists */
        if ((bp=bfind(bname, FALSE, 0)) != FALSE) {
		/* try to make sure we are off screen */
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_bufp == bp) {
				onlywind(FALSE, 1);
				break;
			}
			wp = wp->w_wndp;
		}
		if (zotbuf(bp) != TRUE)

			return(FALSE);
	}

        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
	strcat(line,">");
	strcat(line,filnam);
        system(line);
        TTopen();
        TTflush();
        sgarbf = TRUE;
        s = TRUE;

	if (s != TRUE)
		return(s);

	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE)
			return(FALSE);

	/* and read the stuff in */
	if (getfile(filnam, FALSE) == FALSE)
		return(FALSE);

	/* make this window in VIEW mode, update all mode lines */
	curwp->w_bufp->b_mode |= MDVIEW;
	wp = wheadp;
	while (wp != NULL) {
		wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}

	/* and get rid of the temporary file */
	unlink(filnam);
	return(TRUE);
}

/*
 * filter a buffer through an external DOS program
 * Bound to ^X #
 */
filter(f, n)

{
        register int    s;	/* return status from CLI */
	register BUFFER *bp;	/* pointer to buffer to zot */
        char line[NLINE];	/* command line send to shell */
	char tmpnam[NFILEN];	/* place to store real file name */
	static char bname1[] = "fltinp";

	static char filnam1[] = "fltinp";
	static char filnam2[] = "fltout";

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* get the filter name and its args */
        if ((s=mlreply("#", line, NLINE)) != TRUE)
                return(s);

	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);	/* save the original name */
	strcpy(bp->b_fname, bname1);	/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1, "w") != TRUE) {
		mlwrite(TEXT2);
/*                      "[Cannot write filter file]" */
		strcpy(bp->b_fname, tmpnam);
		return(FALSE);
	}

        TTputc('\n');                /* Already have '\r'    */
        TTflush();
        TTclose();                              /* stty to old modes    */
	strcat(line," <fltinp >fltout");
        system(line);
        TTopen();
        TTflush();
        sgarbf = TRUE;
        s = TRUE;

	/* on failure, escape gracefully */
	if (s != TRUE || (readin(filnam2,FALSE) == FALSE)) {
		mlwrite(TEXT3);
/*                      "[Execution failed]" */
		strcpy(bp->b_fname, tmpnam);
		unlink(filnam1);
		unlink(filnam2);
		return(s);
	}

	/* reset file name */
	strcpy(bp->b_fname, tmpnam);	/* restore name */
	bp->b_flag |= BFCHG;		/* flag it as changed */

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return(TRUE);
}

/* return a system dependant string with the current time */

char *PASCAL NEAR timeset()

{
	register char *sp;	/* temp string pointer */
	char buf[16];		/* time data buffer */
	extern char *ctime();

	time(buf);
	sp = ctime(buf);
	sp[strlen(sp)-1] = 0;
	return(sp);
}
#else
wmcshello()
{
}
#endif
