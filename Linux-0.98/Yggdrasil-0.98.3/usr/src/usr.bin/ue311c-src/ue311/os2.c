/*	OS2.C:		Operating specific I/O and Spawning functions
			for the OS/2 operating system
			for MicroEMACS 3.9
			(C)Copyright 1988 by Daniel M. Lawrence

Note:  don't try to compile this on non OS/2 systems.... the OS/2
	includes MUST come before ours.....

Modifications needed: check that we don't construct command lines and
temporary filenames which are too large for their buffers.

*/

#define INCL_DOS
#include "os2.h"

#include        <stdio.h>

#undef	PASCAL
#undef	NEAR
#undef	HIBYTE

#include	"estruct.h"
#include	"eproto.h"

#if	OS2
#define INCL_BASE

#include "elang.h"
#include "edef.h"


/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
spawncli(f, n)
{
	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
	TTkclose();
	shell();
	TTkopen();
        sgarbf = TRUE;
        return(TRUE);
}


/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
spawn(f, n)
{
        register int s;
        char line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

        if ((s=mlreply("!", line, NLINE)) != TRUE)
                return(s);
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
	system(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
	        puts(TEXT6);
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
        register int s;
        char line[NLINE];

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

        if ((s=mlreply("$", line, NLINE)) != TRUE)
                return(s);
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        execprog(line);
	TTkopen();
	/* if we are interactive, pause here */
	if (clexec == FALSE) {
	        puts(TEXT6);
/*                     "\r\n\n[End]" */
        	tgetc();
        }
        sgarbf = TRUE;
        return (TRUE);
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 * We use a unique temporary file name so that multiple instances of
 * MicroEMACS don't try to use the same file.
 */
pipecmd(f, n)
{
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
	register char *tmp;	/* ptr to TMP DOS environment variable */
	char line[NLINE];	/* command line send to shell */
	static char bname[] = "command";
	static char filnam[NSTRING];
	char *getenv();

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

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
		/* get rid of the existing command buffer */
		if (zotbuf(bp) != TRUE)
			return(FALSE);
	}

	/* get the command to pipe in */
        if (mlreply("@", line, NLINE) != TRUE)
                return(FALSE);

	/* Call mktemp() to get a unique filename in the tmp directory. */
	if ((tmp = getenv("TMP")) == NULL)
		filnam[0] = 0;
	else {
		strcpy(filnam, tmp);
		if (filnam[strlen(filnam) - 1] != '\\')
			strcat(filnam, "\\");
        }
	strcat(filnam,"eXXXXXX");
	mktemp(filnam);

	/* redirect the command output to the output file */
	strcat(line, " >>");
	strcat(line, filnam);
	movecursor(term.t_nrow - 1, 0);

	/* execute the command */
	TTkclose();
	system(line);
	TTkopen();
        sgarbf = TRUE;

	/* did the output file get generated? */
	if (access( filnam, 0) != 0)
		return(FALSE);
		
	/* split the current window to make room for the command output */
	if (splitwind(FALSE, 1) == FALSE)
			return(FALSE);

	/* and read the stuff in */
	if (getfile(filnam, FALSE) == FALSE)
		return(FALSE);

	/* rename the buffer */
	strcpy( curwp->w_bufp->b_bname, "command");
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
 * We use unique temporary file names so that multiple instances of
 * MicroEMACS don't try to use the same file.
 */
filter(f, n)

{
	register int    s;	/* return status from CLI */
	register BUFFER *bp;	/* pointer to buffer to zot */
	char line[NLINE];	/* command line send to shell */
	char tmpnam[NFILEN];	/* place to store real file name */
	char *tmp;		/* ptr to TMP DOS environment variable */

	static char filnam1[NSTRING];
	static char filnam2[NSTRING];

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/

	/* get the filter name and its args */
        if ((s=mlreply("#", line, NLINE)) != TRUE)
                return(s);

	/* Call mktemp() to get unique filenames in the tmp directory. */
	if ((tmp = getenv("TMP")) == NULL)
		filnam1[0] = filnam2[0] = 0;
	else {
		strcpy(filnam1, tmp);
		strcpy(filnam2, tmp);
		if (filnam1[strlen(filnam1) - 1] != '\\') {
			strcat(filnam1, "\\");
			strcat(filnam2, "\\");
		}
        }
	strcat(filnam1,"eXXXXXX");
	strcat(filnam2,"eXXXXXX");
	mktemp(filnam1);
                
	/* setup the proper file names */
	bp = curbp;
	strcpy(tmpnam, bp->b_fname);	/* save the original name */
	strcpy(bp->b_fname, filnam1);	/* set it to our new one */

	/* write it out, checking for errors */
	if (writeout(filnam1, "w") != TRUE) {
		mlwrite(TEXT2);
/*                      "[Cannot write filter file]" */
		strcpy(bp->b_fname, tmpnam);
		return(FALSE);
	}

	mktemp(filnam2);
	strcat(line, " <");		/* construct the command line */
	strcat(line, filnam1);
	strcat(line, " >");
	strcat(line, filnam2);
	
	movecursor(term.t_nrow - 1, 0);
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

	/* and get rid of the temporary file */
	unlink(filnam1);
	unlink(filnam2);
	return(TRUE);
}



/*	SHELL: Bring up a shell. */

shell(void)
{
	char *shell;		/* Name of system command processor */


	/*  get name of system shell  */
	if ((shell = getenv("COMSPEC")) == NULL) {
		return(FALSE);		/*  No shell located  */
	}

	/*
	 * We are actually setting up a shell inside a shell here.
	 * Is there a better way?
	 */
	return(system(shell));
}



execprog( char *cmd)
{
	char		 args[NSTRING];		/* args passed to program */
	char		*sp;
	char		 failName[NFILEN];
	char		 prog[NSTRING];		/* name of program */
	USHORT		 i;
	PRESULTCODES	 results;
	

	/*
	 * Parse the command name from the command line and copy it
	 * into the prog and args arrays.
	 */
	i = 0;
	while (cmd[i]  &&  (cmd[i] != ' ')  &&  (cmd[i] != '\t')) {
		prog[i] = args[i] = cmd[i];
		i++;
	}
	prog[i] = args[i] = 0;		/* terminate with a null */

	/* skip whitespace and copy the args */
	while (cmd[i]  &&  ((cmd[i] == ' ')  ||  (cmd[i] == '\t')))
		i++;
	while (cmd[i]) {
		args[i] = cmd[i];
		i++;
	}
	args[i] = args[i + 1] = 0;	/* terminate with 2 nulls */
	
	
	/* look up the program on the path, trying various extentions */
	if ((sp = flook(prog, TRUE)) == NULL)
		if ((sp = flook(strcat(prog, ".exe"), TRUE)) == NULL) {
			strcpy(&prog[strlen(prog)-4], ".com");
			if ((sp = flook(prog, TRUE)) == NULL)
				return(FALSE);
		}
	strcpy(prog, sp);

	/*
	 * Execute the program synchronously.  We wait for child
	 * to return.
	 */
	return (0 == DosExecPgm( failName, NFILEN, EXEC_SYNC,
					 args, 0, results, prog));
}

	
/*	FILE Directory routines		*/

FILEFINDBUF pBuf;	/* buffer to hold file information */
HDIR hDir;		/* directory handle */
int num_found;		/* number of directory entries found/to find */

char path[NFILEN];	/* path of file to find */
char rbuf[NFILEN];	/* return file buffer */

/*	do a wild card directory search (for file name completion) */

char *PASCAL NEAR getffile(fspec)

char *fspec;	/* pattern to match */

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	char fname[NFILEN];		/* file/path for DOS call */

	/* first parse the file path off the file spec */
	strcpy(path, fspec);
	index = strlen(path) - 1;
	while (index >= 0 && (path[index] != '/' &&
				path[index] != '\\' && path[index] != ':'))
		--index;
	path[index+1] = 0;

	/* check for an extension */
	point = strlen(fspec) - 1;
	extflag = FALSE;
	while (point > index) {
		if (fspec[point] == '.') {
			extflag = TRUE;
			break;
		}
		point--;
	}

	/* construct the composite wild card spec */
	strcpy(fname, path);
	strcat(fname, &fspec[index+1]);
	strcat(fname, "*");
	if (extflag == FALSE)
		strcat(fname, ".*");

	/* and call for the first file */
	num_found = 1;
	hDir = HDIR_CREATE;
	if (DosFindFirst(fname, &hDir, FILE_NORMAL|FILE_DIRECTORY,
			  &pBuf, sizeof(pBuf), &num_found, 0L) != 0)
		return(NULL);

	/* return the first file name! */
	strcpy(rbuf, path);
	strcat(rbuf, pBuf.achName);
	mklower(rbuf);
	if (pBuf.attrFile == FILE_DIRECTORY)
		strcat(rbuf, DIRSEPSTR);
	return(rbuf);
}

char *PASCAL NEAR getnfile()

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	char fname[NFILEN];		/* file/path for DOS call */

	/* and call for the next file */
	if (DosFindNext(hDir, (void *)&pBuf, sizeof(pBuf), &num_found) != 0)
		return(NULL);

	/* return the first file name! */
	strcpy(rbuf, path);
	strcat(rbuf, pBuf.achName);
	mklower(rbuf);
	if (pBuf.attrFile == FILE_DIRECTORY)
		strcat(rbuf, DIRSEPSTR);
	return(rbuf);
}

/* return a system dependant string with the current time */

char *timeset()

{
	register char *sp;	/* temp string pointer */
	char buf[16];		/* time data buffer */
	extern char *ctime();

	time(buf);
	sp = ctime(buf);
	sp[strlen(sp)-1] = 0;
	return(sp);
}

#if	OBSOLETE
/*	extcode:	resolve MSDOS extended character codes
			encoding the proper sequences into emacs
			printable character specifications
*/

int extcode(
	unsigned c	/* byte following a zero extended char byte */
)

{
	/* function keys 1 through 9 */
	if (c >= 59 && c < 68)
		return(SPEC | c - 58 + '0');

	/* function key 10 */
	if (c == 68)
		return(SPEC | '0');

	/* shifted function keys */
	if (c >= 84 && c < 93)
		return(SPEC | SHFT | c - 83 + '0');
	if (c == 93)
		return(SPEC | SHFT | '0');

	/* control function keys */
	if (c >= 94 && c < 103)
		return(SPEC | CTRL | c - 93 + '0');
	if (c == 103)
		return(SPEC | CTRL | '0');

	/* ALTed function keys */
	if (c >= 104 && c < 113)
		return(SPEC | ALTD | c - 103 + '0');
	if (c == 113)
		return(SPEC | ALTD | '0');

	/* ALTed number keys */
	if (c >= 120 && c < 129)
		return(ALTD | c - 119 + '0');
	if (c == 130)
		return(ALTD | '0');

	/* some others as well */
	switch (c) {
		case 3:		return(0);		/* null */
		case 15:	return(SHFT | CTRL | 'I');	/* backtab */

		case 16:	return(ALTD | 'Q');
		case 17:	return(ALTD | 'W');
		case 18:	return(ALTD | 'E');
		case 19:	return(ALTD | 'R');
		case 20:	return(ALTD | 'T');
		case 21:	return(ALTD | 'Y');
		case 22:	return(ALTD | 'U');
		case 23:	return(ALTD | 'I');
		case 24:	return(ALTD | 'O');
		case 25:	return(ALTD | 'P');

		case 30:	return(ALTD | 'A');
		case 31:	return(ALTD | 'S');
		case 32:	return(ALTD | 'D');
		case 33:	return(ALTD | 'F');
		case 34:	return(ALTD | 'G');
		case 35:	return(ALTD | 'H');
		case 36:	return(ALTD | 'J');
		case 37:	return(ALTD | 'K');
		case 38:	return(ALTD | 'L');

		case 44:	return(ALTD | 'Z');
		case 45:	return(ALTD | 'X');
		case 46:	return(ALTD | 'C');
		case 47:	return(ALTD | 'V');
		case 48:	return(ALTD | 'B');
		case 49:	return(ALTD | 'N');
		case 50:	return(ALTD | 'M');

		case 71:	return(SPEC | '<');	/* HOME */
		case 72:	return(SPEC | 'P');	/* cursor up */
		case 73:	return(SPEC | 'Z');	/* page up */
		case 75:	return(SPEC | 'B');	/* cursor left */
		case 77:	return(SPEC | 'F');	/* cursor right */
		case 79:	return(SPEC | '>');	/* end */
		case 80:	return(SPEC | 'N');	/* cursor down */
		case 81:	return(SPEC | 'V');	/* page down */
		case 82:	return(SPEC | 'C');	/* insert */
		case 83:	return(SPEC | 'D');	/* delete */
		case 115:	return(SPEC | CTRL | 'B');	/* control left */
		case 116:	return(SPEC | CTRL | 'F');	/* control right */
		case 117:	return(SPEC | CTRL | '>');	/* control END */
		case 118:	return(SPEC | CTRL | 'V');	/* control page down */
		case 119:	return(SPEC | CTRL | '<');	/* control HOME */
		case 132:	return(SPEC | CTRL | 'Z');	/* control page up */
	}

	return(ALTD | c);
}
#endif	/* obsolete */
#endif

