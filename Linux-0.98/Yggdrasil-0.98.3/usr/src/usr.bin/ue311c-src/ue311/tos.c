/*	TOS.C:		Operating specific I/O and Spawning functions
			for the ATARI ST operating system (GEMDOS)
			for MicroEMACS 3.10
			(C)Copyright 1988 by Daniel M. Lawrence
*/

#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#if	TOS
#include        "edef.h"
#include	"elang.h"
#include	"osbind.h"
#include	"stat.h"	/* DMABUFFER is here */
#include	"errno.h"

/****	ST Internals definitions		*****/

/*	BIOS calls */

#define	BCONSTAT	1	/* return input device status */
#define	CONIN		2	/* read character from device */
#define	BCONOUT		3	/* write character to device */

/*	XBIOS calls */

#define	INITMOUS	0	/* initialize the mouse */
#define	GETREZ		4	/* get current resolution */
#define	SETSCREEN	5	/* set screen resolution */
#define	SETPALETTE	6	/* set the color pallette */
#define	SETCOLOR	7	/* set or read a color */
#define	CURSCONF	21	/* set cursor configuration */
#define	IKBDWS		25	/* intelligent keyboard send command */
#define	KBDVBASE	34	/* get keyboard table base */

/*	GEMDOS calls */

#define	EXEC		0x4b	/* Exec off a process */

#define	CON		2	/* CON: Keyboard and screen device */

/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */
ttopen()
{
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
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
ttputc(c)

char c;

{
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
ttflush()
{
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. More complex in VMS that almost anyplace else, which figures. Very
 * simple on CPM, because the system can do exactly what you want.
 */
ttgetc()
{
}

#if	TYPEAH
typahead()

{
	int rval;	/* return value from BIOS call */

	/* get the status of the console */
	rval = bios(BCONSTAT, CON);

	/* end return the results */
	if (rval == 0)
		return(FALSE);
	else
		return(TRUE);
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
#if	ENVFUNC
	char *shell;	/* ptr to the name of the shell to execute */
	char *getenv();
#endif

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

#if	ENVFUNC
	shell = getenv("SHELL");
	if (shell == (char *)NULL)
		return(FALSE);

	/* Prepare to Jump.... */
	mlerase();
        TTflush();
	TTkclose();

	/* Execute the shell */
	system(shell);

	/* Re-enable EMACS's environment, flag junked screen */
	TTkopen();
        sgarbf = TRUE;
        return(TRUE);
#else
	return(FALSE);
#endif
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
                return(s);
	mlerase();
	TTkclose();
        system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
	        mlputs(TEXT6);
/*                     "\r\n\n[End]" */
        	tgetc();
        }
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
                return(s);
	mlerase();
	TTkclose();
        system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
	        mlputs(TEXT6);
/*                     "\r\n\n[End]" */
        	tgetc();
        }
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
	FILE *fp;

	static char filnam[NSTRING] = "command";

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	/* get the command to pipe in */
        if ((s=mlreply("@", line, NLINE)) != TRUE)
                return(s);

	/* get rid of the command output buffer if it exists */
        if ((bp=bfind(bname, FALSE, 0)) != (BUFFER *)NULL) {
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

	strcat(line," >>");
	strcat(line,filnam);
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
	system(line);
	TTkopen();
        sgarbf = TRUE;
	if ((fp = fopen(filnam, "r")) == NULL) {
		s = FALSE;
	} else {
		fclose(fp);
		s = TRUE;
	}

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
	upmode();

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

	strcat(line," <fltinp >fltout");
	movecursor(term.t_nrow, 0);
	TTkclose();
	system(line);
	TTkopen();
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
	upmode();

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return(TRUE);
}

rename(oldname, newname)	/* rename a file */

char *oldname;		/* original file name */
char *newname;		/* new file name */

{
	Frename(0, oldname, newname);
}

/* return a system dependant string with the current time */

char *PASCAL NEAR timeset()

{
	register char *sp;	/* temp string pointer */
	char buf[16];		/* time data buffer */

	time(buf);
	sp = ctime(buf);
	sp[strlen(sp)-1] = 0;
	return(sp);
}

/*	FILE Directory routines		*/

static DMABUFFER info;		/* Info about the file */
char xpath[NFILEN];		/* path of file to find */
char rbuf[NFILEN];		/* return file buffer */

/*	do a wild card directory search (for file name completion) */

char *PASCAL NEAR getffile(fspec)

char *fspec;	/* file to match */

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	char fname[NFILEN];		/* file/path for DOS call */

	/* first parse the file path off the file spec */
	strcpy(path, fspec);
	index = strlen(xpath) - 1;
	while (index >= 0 && (xpath[index] != '/' &&
				xpath[index] != '\\' && xpath[index] != ':'))
		--index;
	xpath[index+1] = 0;

	/* check for an extension */
	point = strlen(fspec) - 1;
	extflag = FALSE;
	while (point >= index) {
		if (fspec[point] == '.') {
			extflag = TRUE;
			break;
		}
		point--;
	}

	/* construct the composite wild card spec */
	strcpy(fname, xpath);
	strcat(fname, &fspec[index+1]);
	strcat(fname, "*");
	if (extflag == FALSE)
		strcat(fname, ".*");

	/* and call for the first file */
	Fsetdta(&info);		/* Initialize buffer for our search */
	if (Fsfirst(fname, 0xF7) != AE_OK)
		return(NULL);

	/* skip over the . and .. entries */
	while (strncmp(info.d_fname, "..", strlen(info.d_fname)) == 0)
		if (Fsnext != AE_OK)
			return(NULL);

	/* return the first file name! */
	strcpy(rbuf, xpath);
	strcat(rbuf, info.d_fname);
	mklower(rbuf);
	if (info.d_fattr & 0x10)
		strcat(rbuf, DIRSEPSTR);
	return(rbuf);
}

char *PASCAL NEAR getnfile()

{

	/* and call for the first file */
	if (Fsnext() != AE_OK)
		return(NULL);

	/* return the first file name! */
	strcpy(rbuf, xpath);
	strcat(rbuf, info.d_fname);
	mklower(rbuf);
	return(rbuf);
}

#if	LATTICE
system(cmd)	/* call the system to execute a new program */

char *cmd;	/* command to execute */

{
	char *pptr;			/* pointer into program name */
	char pname[NSTRING];		/* name of program to execute */
	char tail[NSTRING];		/* command tail */

	/* scan off program name.... */
	pptr = pname;
	while (*cmd && (*cmd != ' ' && *cmd != '\t'))
		*pptr++ = *cmd++;
	*pptr = 0;

	/* create program name length/string */
	tail[0] = strlen(cmd);
	strcpy(&tail[1], cmd);

	/* go do it! */
	return(gemdos(		(int)EXEC,
				(int)0,
				(char *)pname,
				(char *)tail,
				(char *)NULL));
}
#endif

#else
TOShello()
{
}
#endif
