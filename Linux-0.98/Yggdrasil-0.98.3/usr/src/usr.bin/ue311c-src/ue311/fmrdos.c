/*	FMRDOS.C:	Operating specific I/O and Spawning functions
			under the MSDOS/FMR operating system
			for MicroEMACS 3.10
			(C)Copyright 1990 by Daniel M. Lawrence
*/

#include        <stdio.h>
#include	"estruct.h"
#include	"eproto.h"

#ifdef	MSDOS
#include        "edef.h"
#include	"elang.h"

/* The Mouse driver only works with typeahead defined */
#if	MOUSE
#undef	TYPEAH
#define	TYPEAH	1
#endif

#if  TURBO
#include <conio.h>
#include <dir.h>
#include <dos.h>
#include <bios.h>

struct ffblk fileblock;	/* structure for directory searches */
#endif
#if	MSC
#include <dos.h>

struct find_t fileblock;	/* structure for directory searches */
#endif

#if     LATTICE | MSC | DTL | TURBO | AZTEC | MWC
union REGS rg;		/* cpu register for use of DOS calls */
struct SREGS segreg;	/* cpu segment registers	     */
int nxtchar = -1;	/* character held from type ahead    */
#endif

#if	MSC | TURBO
#include	<process.h>
#endif

/*	Some global variable	*/
#define INBUFSIZ	40
static int mexist;	/* is the mouse driver installed? */
static int nbuttons;	/* number of buttons on the mouse */
static int oldbut;	/* Previous state of mouse buttons */

PASCAL NEAR execprog(char *cmd);

/*	input buffers and pointers	*/

#define	IBUFSIZE	64	/* this must be a power of 2 */

unsigned char in_buf[IBUFSIZE];	/* input character buffer */
int in_next = 0;		/* pos to retrieve next input character */
int in_last = 0;		/* pos to place most recent input character */

in_init()	/* initialize the input buffer */

{
	in_next = in_last = 0;
}

in_check()	/* is the input buffer non-empty? */

{
	if (in_next == in_last)
		return(FALSE);
	else
		return(TRUE);
}

in_put(event)

int event;	/* event to enter into the input buffer */

{
	in_buf[in_last++] = event;
	in_last &= (IBUFSIZE - 1);
}

int in_get()	/* get an event from the input buffer */

{
	register int event;	/* event to return */

	event = in_buf[in_next++];
	in_next &= (IBUFSIZE - 1);
	return(event);
}

/*
 * This function is called once to set up the terminal device streams.
 */

PASCAL NEAR ttopen()

{
#if	MOUSE
	long miaddr;	/* mouse interupt routine address */
#endif

	/* on all screens we are not sure of the initial position
	   of the cursor					*/
	ttrow = 999;
	ttcol = 999;

#if	MOUSE
	/* check if the mouse drive exists first */
	rg.x.ax = 0x3599;	/* look at the interrupt 99 address */

#if	MSC | TURBO | DTL | LATTICE | MWC
	int86x(0x21, &rg, &rg, &segreg);
	miaddr = (((long)segreg.es) << 16) + (long)rg.x.bx;
	if (miaddr == 0 || *(char * far)miaddr == 0xcf) {
#endif
#if	AZTEC
	sysint(0x21, &rg, &rg);
	miaddr = (((long)rg.x.es) << 16) + (long)rg.x.bx;
	if (miaddr == 0 || *(char *)miaddr == 0xcf) {
#endif
		mexist = FALSE;
		return;
	}

	/* and then check for the mouse itself */
	rg.x.ax = 0;		/* mouse status flag */
	int86(0x99, &rg, &rg);	/* check for the mouse interupt */
	mexist = (rg.x.ax == 0);
	nbuttons = 3;		/* it COULD have 3 buttons?? */

	/* initialize our character input queue */
	in_init();
	if (mexist == FALSE)
		return;

	/* if the mouse exists.. get it in the upper right corner */
	rg.h.ah = 4;		/* set mouse cursor position */
	rg.x.dx = 559;		/* last col of display */
	rg.x.bx = 0;		/* top row */
	int86(0x99, &rg, &rg);
#else	/* !MOUSE */
	mexist = 0;
#endif	/* !MOUSE */
}

maxlines(lines)		/* set number of vertical rows for mouse */

int lines;	/* # of vertical lines */

{
#if	MOUSE
	if (mexist) {
		/* nothing yet! */
	}
#endif
}

/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
PASCAL NEAR ttclose()
{
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */

PASCAL NEAR ttputc(c)

int c;

{
#if     MWC
        putcnb(c);
#endif

#if	(LATTICE | AZTEC | TURBO | MSC)
	bdos(6, c, 0);
#endif
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
PASCAL NEAR ttflush()
{
}

int doschar()	/* call the dos to get a char */

{

	register unsigned int c;	/* extended character to return */ 

	rg.h.ah = 7;		/* dos Direct Console Input call */
	intdos(&rg, &rg);
	return(rg.h.al & 255);
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. Also mouse events are forced into the input stream here.
 */
PASCAL NEAR ttgetc()

{
	register int c;		/* character read */

ttc:	/* return any keystrokes waiting in the
	   type ahead buffer */
	if (in_check())
		return(in_get());

#if	TYPEAH
	if (typahead())
		return(doschar());

	/* with no mouse, this is a simple get char routine */
	if (mexist == FALSE || mouseflag == FALSE)
		return(doschar());

#if	MOUSE
	/* turn the mouse cursor on */
	rg.x.ax = 0x0200;	/* Show Cursor */
	int86(0x99, &rg, &rg);

	/* loop waiting for something to happen */
	while (TRUE) {
		if (typahead())
			break;
		if (checkmouse())
			break;
	}

	/* turn the mouse cursor back off */
	rg.x.ax = 0x0201;	/* erase Cursor */
	int86(0x99, &rg, &rg);

	goto ttc;
#endif	/* MOUSE */
#else	/* TYPEAH */
	return(doschar());
#endif	/* TYPEAH */
}

#if	MOUSE
checkmouse()

{
	register int k;		/* current bit/button of mouse */
	register int event;	/* encoded mouse event */
	int mousecol;		/* current mouse column */
	int mouserow;		/* current mouse row */
	int sstate;		/* current shift key status */
	int newbut;		/* new state of the mouse buttons */

	/* check to see if any mouse buttons are different */
	rg.h.ah = 3;	/* Get button status and mouse position */
	int86(0x99, &rg, &rg);
	newbut   = rg.h.ch & 0x07;
	mousecol = rg.x.dx /14;
	mouserow = rg.x.bx /30;

	if (oldbut == newbut)
		return(FALSE);

	/* get the shift key status as well */
	sstate = 0;
	rg.h.ah = 8;	/* return current shift status */
	int86(0x90, &rg, &rg);
	sstate = rg.h.al;

	for (k=1; k != (1 << nbuttons); k = k<<1) {

		/* For each button on the mouse */
		if ((oldbut&k) != (newbut&k)) {

			/* This button changed, generate an event */
			in_put(0);
			in_put(MOUS >> 8);
			in_put(mousecol);
			in_put(mouserow);

			event = ((newbut&k) ? 0 : 1);	/* up or down? */
			if (k == 2)			/* center button? */
				event += 4;
			if (k == 4)			/* right button? */
				event += 2;
			if (sstate & (4|32|64))		/* shifted */
				event += 'A';
			else if (sstate & 16)		/* controled? */
				event += 1;
			else
				event += 'a';		/* plain */
			in_put(event);
			oldbut = newbut;
			return(TRUE);
		}
	}

	return(FALSE);
}
#endif

#if	TYPEAH
/* typahead:	Check to see if any characters are already in the
		keyboard buffer
*/

PASCAL NEAR typahead()

{
	int flags;	/* cpu flags from dos call */

#if	TURBO  /* This conditional portion added 9/13/89 by DRK. */
	if (bioskey(1) == 0)
		return FALSE;
	else
		return TRUE;
#else
	rg.x.ax = 0x4406;	/* IOCTL input status */
	rg.x.bx = 0;		/* File handle = stdin */
#if	MSC | DTL
	int86(0x21,&rg,&rg);
	flags = rg.h.al;
#else
#if	LATTICE | AZTEC | TURBO
	flags = intdos(&rg, &rg);
#else
	intcall(&rg, &rg, 0x21);
	flags = rg.x.flags;
#endif
#endif
	if (flags & 1)		/* AL = 0xFF if ready */
		return(TRUE);
	else
		return(FALSE);
#endif
}
#endif

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C".
 */

PASCAL NEAR spawncli(f, n)

int f, n;

{
	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

        movecursor(term.t_nrow, 0);             /* Seek to last line.   */
        TTflush();
	TTkclose();
	shellprog("");
	TTkopen();
        sgarbf = TRUE;
        return(TRUE);
}

/*
 * Run a one-liner in a subjob. When the command returns, wait for a single
 * character to be typed, then mark the screen as garbage so a full repaint is
 * done. Bound to "C-X !".
 */
PASCAL NEAR spawn(f, n)

int f, n;

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
        shellprog(line);
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

PASCAL NEAR execprg(f, n)

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
PASCAL NEAR pipecmd(f, n)

int f, n;

{
	register WINDOW *wp;	/* pointer to new window */
	register BUFFER *bp;	/* pointer to buffer to zot */
	register char *tmp;	/* ptr to TMP DOS environment variable */
	FILE *fp;
        char line[NLINE];	/* command line send to shell */
	static char bname[] = "command";
	static char filnam[NSTRING] = "command";
	char *getenv();
	FILE *fopen();

	/* don't allow this command if restricted */
	if (restflag)
		return(resterr());

	if ((tmp = getenv("TMP")) == NULL)
		filnam[0] = 0;
	else {
		strcpy(filnam, tmp);
		if (filnam[strlen(filnam) - 1] != '\\')
			strcat(filnam, "\\");
        }
	strcat(filnam,"command");

	/* get the command to pipe in */
        if (mlreply("@", line, NLINE) != TRUE)
                return(FALSE);

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

	/* redirect the command output to the output file */
	strcat(line, " >>");
	strcat(line, filnam);
	movecursor(term.t_nrow - 1, 0);

	/* execute the command */
	TTkclose();
        shellprog(line);
	TTkopen();
        sgarbf = TRUE;

        /* did the output file get generated? */
	if ((fp = fopen(filnam, "r")) == NULL)
		return(FALSE);
	fclose(fp);

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
PASCAL NEAR filter(f, n)

int f, n;

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
	movecursor(term.t_nrow - 1, 0);
	TTkclose();
        shellprog(line);
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

#if	LATTICE
extern int _oserr;
#endif

#if	AZTEC | MWC
extern int errno;
#endif

#if	MSC
extern int _doserrno;
#endif

/*	SHELLPROG: Execute a command in a subshell		*/

PASCAL NEAR shellprog(cmd)

char *cmd;	/*  Incoming command line to execute  */

{
	char *shell;		/* Name of system command processor */
	char swchar;		/* switch character to use */
	union REGS regs;	/* parameters for dos call */
	char comline[NSTRING];	/* constructed command line */
	char *getenv();

	/*  detect current switch character and set us up to use it */
	regs.h.ah = 0x37;	/*  get setting data  */
	regs.h.al = 0x00;	/*  get switch character  */
	intdos(&regs, &regs);
	swchar = (char)regs.h.dl;

	/*  get name of system shell  */
	if ((shell = getenv("COMSPEC")) == NULL) {
		return(FALSE);		/*  No shell located  */
	}

	/* trim leading whitespace off the command */
	while (*cmd == ' ' || *cmd == '\t')	/*  find out if null command */
		cmd++;

	/**  If the command line is not empty, bring up the shell  **/
	/**  and execute the command.  Otherwise, bring up the     **/
	/**  shell in interactive mode.   **/

	if (*cmd) {
		strcpy(comline, shell);
		strcat(comline, " ");
		comline[strlen(comline) + 1] = 0;
		comline[strlen(comline)] = swchar;
		strcat(comline, "c ");
		strcat(comline, cmd);
		return(execprog(comline));
	} else
		return(execprog(shell));
}

/*	EXECPROG:	A function to execute a named program
			with arguments
*/

#if	LATTICE | AZTEC | MWC
#define	CFLAG	1
#endif

PASCAL NEAR execprog(cmd)

char *cmd;	/*  Incoming command line to execute  */

{
	char *sp;		/* temporary string pointer */
	char f1[38];		/* FCB1 area (not initialized */
	char f2[38];		/* FCB2 area (not initialized */
	char prog[NSTRING];	/* program filespec */
	char tail[NSTRING];	/* command tail with length byte */
	union REGS regs;	/* parameters for dos call  */
#if	MWC == 0
	struct SREGS segreg;	/* segment registers for dis call */
#endif
	struct Pblock {		/* EXEC parameter block */
		short envptr;	/* 2 byte pointer to environment string */
		char *cline;	/* 4 byte pointer to command line */
		char *fcb1;	/* 4 byte pointer to FCB at PSP+5Ch */
		char *fcb2;	/* 4 byte pointer to FCB at PSP+6Ch */
	} pblock;

	/* parse the command name from the command line */
	sp = prog;
	while (*cmd && (*cmd != ' ') && (*cmd != '\t'))
		*sp++ = *cmd++;
	*sp = 0;

	/* and parse out the command tail */
	while (*cmd && ((*cmd == ' ') || (*cmd == '\t')))
		++cmd;
	*tail = (char)(strlen(cmd)); /* record the byte length */
	strcpy(&tail[1], cmd);
	strcat(&tail[1], "\r");

	/* look up the program on the path trying various extentions */
	if ((sp = flook(prog, TRUE)) == NULL)
		if ((sp = flook(strcat(prog, ".exe"), TRUE)) == NULL) {
			strcpy(&prog[strlen(prog)-4], ".com");
			if ((sp = flook(prog, TRUE)) == NULL)
				return(FALSE);
		}
	strcpy(prog, sp);

#if	MWC == 0
	/* get a pointer to this PSPs environment segment number */
	segread(&segreg);
#endif

	/* set up the EXEC parameter block */
	pblock.envptr = 0;	/* make the child inherit the parents env */
	pblock.fcb1 = f1;		/* point to a blank FCB */
	pblock.fcb2 = f2;		/* point to a blank FCB */
        pblock.cline = tail;		/* parameter line pointer */

	/* and make the call */
	regs.h.ah = 0x4b;	/* EXEC Load or Execute a Program */
	regs.h.al = 0x00;	/* load end execute function subcode */
#if	AZTEC | MWC
	regs.x.ds = ((unsigned long)(prog) >> 16);	/* program name ptr */
	regs.x.dx = (unsigned int)(prog);
	regs.x.es = regs.x.ds;
	/*regs.x.es = ((unsigned long)(&pblock) >> 16);	* set up param block ptr */
	regs.x.bx = (unsigned int)(&pblock);
#endif
#if	LATTICE | MSC | TURBO | DTL
	segreg.ds = ((unsigned long)(prog) >> 16);	/* program name ptr */
	regs.x.dx = (unsigned int)(prog);
	segreg.es = ((unsigned long)(&pblock) >> 16);	/* set up param block ptr */
	regs.x.bx = (unsigned int)(&pblock);
#endif

#if	NOVELL
	rval = execpr(prog, &pblock);
#endif
	
#if	LATTICE && (NOVELL == 0)
	if ((intdosx(&regs, &regs, &segreg) & CFLAG) == 0) {
		regs.h.ah = 0x4d;	/* get child process return code */
		intdos(&regs, &regs);	/* go do it */
		rval = regs.x.ax;	/* save child's return code */
	} else
		rval = -_oserr;		/* failed child call */
#endif
#if	AZTEC && (NOVELL == 0)
	if ((sysint(0x21, &regs, &regs) & CFLAG) == 0) {
		regs.h.ah = 0x4d;	/* get child process return code */
		sysint(0x21, &regs, &regs);	/* go do it */
		rval = regs.x.ax;	/* save child's return code */
	} else
		rval = -errno;		/* failed child call */
#endif
#if	MWC && (NOVELL == 0)
	intcall(&regs, &regs, DOSINT);
	if ((regs.x.flags & CFLAG) == 0) {
		regs.h.ah = 0x4d;	/* get child process return code */
		intcall(&regs, &regs, DOSINT);	/* go do it */
		rval = regs.x.ax;	/* save child's return code */
	} else
		rval = -errno;		/* failed child call */
#endif
#if	(TURBO | MSC | DTL) && (NOVELL == 0)
	intdosx(&regs, &regs, &segreg);
	if (regs.x.cflag == 0) {
		regs.h.ah = 0x4d;	/* get child process return code */
		intdos(&regs, &regs);	/* go do it */
		rval = regs.x.ax;	/* save child's return code */
	} else
		rval = -_doserrno;	/* failed child call */
#endif
	return((rval < 0) ? FALSE : TRUE);
}

/* return a system dependant string with the current time */

char *PASCAL NEAR timeset()

{
#if	MWC | TURBO | MSC
	register char *sp;	/* temp string pointer */
	char buf[16];		/* time data buffer */
	extern char *ctime();

	time(buf);
	sp = ctime(buf);
	sp[strlen(sp)-1] = 0;
	return(sp);
#else
	return(errorm);
#endif
}

#if	TURBO
/*	FILE Directory routines		*/

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
	if (findfirst(fname, &fileblock, FA_DIREC) == -1)
		return(NULL);

	/* return the first file name! */
	strcpy(rbuf, path);
	strcat(rbuf, fileblock.ff_name);
	mklower(rbuf);
	if (fileblock.ff_attrib == 16)
		strcat(rbuf, DIRSEPSTR);
	return(rbuf);
}

char *PASCAL NEAR getnfile()

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	char fname[NFILEN];		/* file/path for DOS call */

	/* and call for the first file */
	if (findnext(&fileblock) == -1)
		return(NULL);

	/* return the first file name! */
	strcpy(rbuf, path);
	strcat(rbuf, fileblock.ff_name);
	mklower(rbuf);
	if (fileblock.ff_attrib == 16)
		strcat(rbuf, DIRSEPSTR);
	return(rbuf);
}
#else
#if	MSC
/*	FILE Directory routines		*/

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
	if (_dos_findfirst(fname, _A_NORMAL|_A_SUBDIR, &fileblock) != 0)
		return(NULL);

	/* return the first file name! */
	strcpy(rbuf, path);
	strcat(rbuf, fileblock.name);
	mklower(rbuf);
	if (fileblock.attrib == 16)
		strcat(rbuf, DIRSEPSTR);
	return(rbuf);
}

char *PASCAL NEAR getnfile()

{
	register int index;		/* index into various strings */
	register int point;		/* index into other strings */
	register int extflag;		/* does the file have an extention? */
	char fname[NFILEN];		/* file/path for DOS call */

	/* and call for the first file */
	if (_dos_findnext(&fileblock) != 0)
		return(NULL);

	/* return the first file name! */
	strcpy(rbuf, path);
	strcat(rbuf, fileblock.name);
	mklower(rbuf);
	if (fileblock.attrib == 16)
		strcat(rbuf, DIRSEPSTR);
	return(rbuf);
}
#else
char *PASCAL NEAR getffile(fspec)

char *fspec;	/* file to match */

{
	return(NULL);
}

char *PASCAL NEAR getnfile()

{
	return(NULL);
}
#endif
#endif
#endif
