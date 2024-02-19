/*	This file is for functions dealing with execution of
	commands, command lines, buffers, files and startup files

	written 1986 by Daniel Lawrence				*/

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"

/* namedcmd:	execute a named command even if it is not bound */

PASCAL NEAR namedcmd(f, n)

int f, n;	/* command arguments [passed through to command executed] */

{
	int (PASCAL NEAR *kfunc)(); 	/* ptr to the function to execute */
	char buffer[NSTRING];		/* buffer to store function name */
	int status;

	/* if we are non-interactive.... force the command interactivly */
	if (clexec == TRUE) {
		/* grab token and advance past */
		execstr = token(execstr, buffer, NPAT);

		/* evaluate it */
		strcpy(buffer, fixnull(getval(buffer)));
		if (strcmp(buffer, errorm) == 0)
			return(FALSE);

		/* and look it up */
		if ((kfunc = fncmatch(buffer)) == NULL) {
			mlwrite(TEXT16);
/*      	                "[No such Function]" */
			return(FALSE);
		}
		
		/* and execute it  INTERACTIVE */
		clexec = FALSE;
		status = (*kfunc)(f, n);	/* call the function */
		clexec = TRUE;
		return(status);
	}

	/* prompt the user to type a named command */
	/* and get the function name to execute */
	kfunc = getname(": ");
	if (kfunc == NULL) {
		mlwrite(TEXT16);
/*                      "[No such function]" */
		return(FALSE);
	}

	/* and then execute the command */
	return((*kfunc)(f, n));
}

/*	execcmd:	Execute a command line command to be typed in
			by the user					*/

PASCAL NEAR execcmd(f, n)

int f, n;	/* default Flag and Numeric argument */

{
	register int status;		/* status return */
	char cmdstr[NSTRING];		/* string holding command to execute */

	/* get the line wanted */
	if ((status = mlreply(": ", cmdstr, NSTRING)) != TRUE)
		return(status);

	execlevel = 0;
	return(docmd(cmdstr));
}

/*	docmd:	take a passed string as a command line and translate
		it to be executed as a command. This function will be
		used by execute-command-line and by all source and
		startup files. Lastflag/thisflag is also updated.

	format of the command line is:

		{# arg} <command-name> {<argument string(s)>}

*/

PASCAL NEAR docmd(cline)

char *cline;	/* command line to execute */

{
	register int f;		/* default argument flag */
	register int n;		/* numeric repeat value */
	int (PASCAL NEAR *fnc)();/* function to execute */
	BUFFER *bp;		/* buffer to execute */
	int status;		/* return status of function */
	int oldcle;		/* old contents of clexec flag */
	char *oldestr;		/* original exec string */
	char tkn[NSTRING];	/* next token off of command line */
        char bufn[NBUFN+2];	/* name of buffer to execute */

	/* if we are scanning and not executing..go back here */
	if (execlevel)
		return(TRUE);

	oldestr = execstr;	/* save last ptr to string to execute */
	execstr = cline;	/* and set this one as current */

	/* first set up the default command values */
	f = FALSE;
	n = 1;
	lastflag = thisflag;
	thisflag = 0;

	if ((status = macarg(tkn)) != TRUE) {	/* and grab the first token */
		execstr = oldestr;
		return(status);
	}

	/* process leadin argument */
	if (gettyp(tkn) != TKCMD) {
		f = TRUE;
		strcpy(tkn, fixnull(getval(tkn)));
		n = asc_int(tkn);

		/* and now get the command to execute */
		if ((status = macarg(tkn)) != TRUE) {
			execstr = oldestr;
			return(status);
		}
	}

	/* and match the token to see if it exists */
	if ((fnc = fncmatch(tkn)) == NULL) {

		/* construct the buffer name */
		strcpy(bufn, "[");
		strcat(bufn, tkn);
		strcat(bufn, "]");

		/* find the pointer to that buffer */
        	if ((bp=bfind(bufn, FALSE, 0)) == NULL) {
			mlwrite(TEXT16);
/*	                      "[No such Function]" */
			execstr = oldestr;
			return(FALSE);
		}

		/* execute the buffer */
		oldcle = clexec;	/* save old clexec flag */
		clexec = TRUE;		/* in cline execution */
		while (n-- > 0)
			if ((status = dobuf(bp)) != TRUE)
				break;
		cmdstatus = status;	/* save the status */
		clexec = oldcle;	/* restore clexec flag */
		execstr = oldestr;
		return(status);
	}
	
	/* save the arguments and go execute the command */
	oldcle = clexec;		/* save old clexec flag */
	clexec = TRUE;			/* in cline execution */
	status = (*fnc)(f, n);		/* call the function */
	cmdstatus = status;		/* save the status */
	clexec = oldcle;		/* restore clexec flag */
	execstr = oldestr;
	return(status);
}

/* token:	chop a token off a string
		return a pointer past the token
*/

char *PASCAL NEAR token(src, tok, size)

char *src, *tok;	/* source string, destination token string */
int size;		/* maximum size of token */

{
	register int quotef;	/* is the current string quoted? */
	register char c;	/* temporary character */

	/* first scan past any whitespace in the source string */
	while (*src == ' ' || *src == '\t')
		++src;

	/* scan through the source string */
	quotef = FALSE;
	while (*src) {
		/* process special characters */
		if (*src == '~') {
			++src;
			if (*src == 0)
				break;
			switch (*src++) {
				case 'r':	c = 13; break;
				case 'n':	c = 13; break;
				case 'l':	c = 10; break;
				case 't':	c = 9;  break;
				case 'b':	c = 8;  break;
				case 'f':	c = 12; break;
				default:	c = *(src-1);
			}
			if (--size > 0) {
				*tok++ = c;
			}
		} else {
			/* check for the end of the token */
			if (quotef) {
				if (*src == '"')
					break;
			} else {
				if (*src == ' ' || *src == '\t')
					break;
			}

			/* set quote mode if quote found */
			if (*src == '"')
				quotef = TRUE;

			/* record the character */
			c = *src++;
			if (--size > 0)
				*tok++ = c;
		}
	}

	/* terminate the token and exit */
	if (*src)
		++src;
	*tok = 0;
	return(src);
}

PASCAL NEAR macarg(tok)	/* get a macro line argument */

char *tok;	/* buffer to place argument */

{
	int savcle;	/* buffer to store original clexec */
	int status;

	savcle = clexec;	/* save execution mode */
	clexec = TRUE;		/* get the argument */
	status = nextarg("", tok, NSTRING, ctoec('\r'));
	clexec = savcle;	/* restore execution mode */
	return(status);
}

/*	nextarg:	get the next argument	*/

PASCAL NEAR nextarg(prompt, buffer, size, terminator)

char *prompt;		/* prompt to use if we must be interactive */
char *buffer;		/* buffer to put token into */
int size;		/* size of the buffer */
int terminator;		/* terminating char to be used on interactive fetch */

{
	register char *sp;	/* return pointer from getval() */

	/* if we are interactive, go get it! */
	if (clexec == FALSE)
		return(getstring(prompt, buffer, size, terminator));

	/* grab token and advance past */
	execstr = token(execstr, buffer, size);

	/* evaluate it */
	if ((sp = getval(buffer)) == NULL)
		return(FALSE);
	strcpy(buffer, sp);
	return(TRUE);
}

/*	storemac:	Set up a macro buffer and flag to store all
			executed command lines there			*/

PASCAL NEAR storemac(f, n)

int f;		/* default flag */
int n;		/* macro number to use */

{
	register struct BUFFER *bp;	/* pointer to macro buffer */
	char bname[NBUFN];		/* name of buffer to use */

	/* must have a numeric argument to this function */
	if (f == FALSE) {
		mlwrite(TEXT111);
/*                      "No macro specified" */
		return(FALSE);
	}

	/* range check the macro number */
	if (n < 1 || n > 40) {
		mlwrite(TEXT112);
/*                      "Macro number out of range" */
		return(FALSE);
	}

	/* construct the macro buffer name */
	strcpy(bname, "[Macro xx]");
	bname[7] = '0' + (n / 10);
	bname[8] = '0' + (n % 10);

	/* set up the new macro buffer */
	if ((bp = bfind(bname, TRUE, BFINVS)) == NULL) {
		mlwrite(TEXT113);
/*                      "Can not create macro" */
		return(FALSE);
	}

	/* and make sure it is empty */
	bclear(bp);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return(TRUE);
}

/*	storeproc:	Set up a procedure buffer and flag to store all
			executed command lines there			*/

PASCAL NEAR storeproc(f, n)

int f;		/* default flag */
int n;		/* macro number to use */

{
	register struct BUFFER *bp;	/* pointer to macro buffer */
	register int status;		/* return status */
	char bname[NBUFN];		/* name of buffer to use */

	/* a numeric argument means its a numbered macro */
	if (f == TRUE)
		return(storemac(f, n));

	/* get the name of the procedure */
        if ((status = mlreply(TEXT114, &bname[1], NBUFN-2)) != TRUE)
/*                            "Procedure name: " */
                return(status);

	/* construct the macro buffer name */
	bname[0] = '[';
	strcat(bname, "]");

	/* set up the new macro buffer */
	if ((bp = bfind(bname, TRUE, BFINVS)) == NULL) {
		mlwrite(TEXT113);
/*                      "Can not create macro" */
		return(FALSE);
	}

	/* and make sure it is empty */
	bclear(bp);

	/* and set the macro store pointers to it */
	mstore = TRUE;
	bstore = bp;
	return(TRUE);
}

/*	execproc:	Execute a procedure				*/

PASCAL NEAR execproc(f, n)

int f, n;	/* default flag and numeric arg */

{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
        char bufn[NBUFN+2];		/* name of buffer to execute */

	/* find out what buffer the user wants to execute */
        if ((status = mlreply(TEXT115, &bufn[1], NBUFN)) != TRUE)
/*                            "Execute procedure: " */
                return(status);

	/* construct the buffer name */
	bufn[0] = '[';
	strcat(bufn, "]");

	/* find the pointer to that buffer */
        if ((bp=bfind(bufn, FALSE, 0)) == NULL) {
		mlwrite(TEXT116);
/*                      "No such procedure" */
                return(FALSE);
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return(status);
	return(TRUE);
}

/*	execbuf:	Execute the contents of a buffer of commands	*/

PASCAL NEAR execbuf(f, n)

int f, n;	/* default flag and numeric arg */

{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
        char bufn[NSTRING];		/* name of buffer to execute */

	/* find out what buffer the user wants to execute */
        if ((status = mlreply(TEXT117, bufn, NBUFN)) != TRUE)
/*                            "Execute buffer: " */
                return(status);

	/* find the pointer to that buffer */
        if ((bp=bfind(bufn, FALSE, 0)) == NULL) {
		mlwrite(TEXT118);
/*                      "No such buffer" */
                return(FALSE);
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return(status);
	return(TRUE);
}

/*	dobuf:	execute the contents of the buffer pointed to
		by the passed BP

	Directives start with a "!" and include:

	!endm		End a macro
	!if (cond)	conditional execution
	!else
	!endif
	!return		Return (terminating current macro)
	!goto <label>	Jump to a label in the current macro
	!force		Force macro to continue...even if command fails
	!while (cond)	Execute a loop if the condition is true
	!endwhile
	
	Line Labels begin with a "*" as the first nonblank char, like:

	*LBL01
*/

PASCAL NEAR dobuf(bp)

BUFFER *bp;	/* buffer to execute */

{
        register int status;	/* status return */
	register LINE *lp;	/* pointer to line to execute */
	register LINE *hlp;	/* pointer to line header */
	register LINE *glp;	/* line to goto */
	LINE *mp;		/* Macro line storage temp */
	int dirnum;		/* directive index */
	int linlen;		/* length of line to execute */
	int i;			/* index */
	int force;		/* force TRUE result? */
	WINDOW *wp;		/* ptr to windows to scan */
	WHBLOCK *whlist;	/* ptr to !WHILE list */
	WHBLOCK *scanner;	/* ptr during scan */
	WHBLOCK *whtemp;	/* temporary ptr to a WHBLOCK */
	char *einit;		/* initial value of eline */
	char *eline;		/* text of line to execute */
	char tkn[NSTRING];	/* buffer to evaluate an expresion in */
#if	LOGFLG
	FILE *fp;		/* file handle for log file */
#endif
#if	DEBUGM
	int skipflag;		/* are we skipping debugging a function? */
#endif

	/* clear IF level flags/while ptr */
	execlevel = 0;
	whlist = NULL;
	scanner = NULL;

	/* flag we are executing the buffer */
	bp->b_exec += 1;

#if	DEBUGM
	/* we are not skipping a function yet (for the debugger) */
	skipflag = FALSE;
#endif

	/* scan the buffer to execute, building WHILE header blocks */
	hlp = bp->b_linep;
	lp = hlp->l_fp;
	while (lp != hlp) {
		/* scan the current line */
		eline = lp->l_text;
		i = lp->l_used;

		/* trim leading whitespace */
		while (i-- > 0 && (*eline == ' ' || *eline == '\t'))
			++eline;

		/* if theres nothing here, don't bother */
		if (i <= 0)
			goto nxtscan;

		/* if is a while directive, make a block... */
		if (eline[0] == '!' && eline[1] == 'w' && eline[2] == 'h') {
			whtemp = (WHBLOCK *)malloc(sizeof(WHBLOCK));
			if (whtemp == NULL) {
noram:				errormesg(TEXT119, bp, lp);
/*                                        "%%Out of memory during while scan" */
failexit:			freewhile(scanner);
				freewhile(whlist);
				bp->b_exec -= 1;
				return(FALSE);
			}
			whtemp->w_begin = lp;
			whtemp->w_type = BTWHILE;
			whtemp->w_next = scanner;
			scanner = whtemp;
		}

		/* if is a BREAK directive, make a block... */
		if (eline[0] == '!' && eline[1] == 'b' && eline[2] == 'r') {
			if (scanner == NULL) {
				errormesg(TEXT120, bp, lp);
/*                                        "%%!BREAK outside of any !WHILE loop" */
				goto failexit;
			}
			whtemp = (WHBLOCK *)malloc(sizeof(WHBLOCK));
			if (whtemp == NULL)
				goto noram;
			whtemp->w_begin = lp;
			whtemp->w_type = BTBREAK;
			whtemp->w_next = scanner;
			scanner = whtemp;
		}

		/* if it is an endwhile directive, record the spot... */
		if (eline[0] == '!' && strncmp(&eline[1], "endw", 4) == 0) {
			if (scanner == NULL) {
				errormesg(TEXT121, bp, lp);
/*                                      "%%!ENDWHILE with no preceding !WHILE" */
				goto failexit;
			}
			/* move top records from the scanner list to the
			   whlist until we have moved all BREAK records
			   and one WHILE record */
			do {
				scanner->w_end = lp;
				whtemp = whlist;
				whlist = scanner;
				scanner = scanner->w_next;
				whlist->w_next = whtemp;
			} while (whlist->w_type == BTBREAK);
		}

nxtscan:	/* on to the next line */
		lp = lp->l_fp;
	}

	/* while and endwhile should match! */
	if (scanner != NULL) {
		errormesg(TEXT122, bp, lp);
/*                      "%%!WHILE with no matching !ENDWHILE" */
		goto failexit;
	}

	/* let the first command inherit the flags from the last one..*/
	thisflag = lastflag;

	/* starting at the beginning of the buffer */
	hlp = bp->b_linep;
	lp = hlp->l_fp;
	while (lp != hlp && eexitflag == FALSE) {

		/* allocate eline and copy macro line to it */
		linlen = lp->l_used;
		if ((einit = eline = malloc(linlen+1)) == NULL) {
			errormesg(TEXT123, bp, lp);
/*                              "%%Out of Memory during macro execution" */
			freewhile(whlist);
			bp->b_exec -= 1;
			return(FALSE);
		}
		bytecopy(eline, lp->l_text, linlen);
		eline[linlen] = 0;	/* make sure it ends */

		/* trim leading whitespace */
		while (*eline == ' ' || *eline == '\t')
			++eline;

		/* dump comments and blank lines */
		if (*eline == ';' || *eline == 0)
			goto onward;

#if	LOGFLG
		/* append the current command to the log file */
		fp = fopen("emacs.log", "a");
		strcpy(outline, eline);
		fprintf(fp, "%s\n", outline);
		fclose(fp);
#endif
	
#if	DEBUGM
		/* only do this if we are debugging */
		if (macbug && !mstore && (execlevel == 0))
			if (debug(bp, eline, &skipflag) == FALSE) {
				errormesg(TEXT54, bp, lp);
/*                                      "[Macro aborted]" */
				freewhile(whlist);
				bp->b_exec -= 1;
				return(FALSE);
			}
#endif

		/* Parse directives here.... */
		dirnum = -1;
		if (*eline == '!') {
			/* Find out which directive this is */
			++eline;
			for (dirnum = 0; dirnum < NUMDIRS; dirnum++)
				if (strncmp(eline, dname[dirnum],
				            strlen(dname[dirnum])) == 0)
					break;

			/* and bitch if it's illegal */
			if (dirnum == NUMDIRS) {
				errormesg(TEXT124, bp, lp);
/*                                      "%%Unknown Directive" */
				freewhile(whlist);
				bp->b_exec -= 1;
				return(FALSE);
			}

			/* service only the !ENDM macro here */
			if (dirnum == DENDM) {
				mstore = FALSE;
				bstore = NULL;
				goto onward;
			}

			/* restore the original eline....*/
			--eline;
		}

		/* if macro store is on, just salt this away */
		if (mstore) {
			/* allocate the space for the line */
			linlen = strlen(eline);
			if ((mp=lalloc(linlen)) == NULL) {
				errormesg(TEXT125, bp, lp);
/*                                      "Out of memory while storing macro" */
				bp->b_exec -= 1;
				return(FALSE);
			}
	
			/* copy the text into the new line */
			for (i=0; i<linlen; ++i)
				lputc(mp, i, eline[i]);
	
			/* attach the line to the end of the buffer */
	       		bstore->b_linep->l_bp->l_fp = mp;
			mp->l_bp = bstore->b_linep->l_bp;
			bstore->b_linep->l_bp = mp;
			mp->l_fp = bstore->b_linep;
			goto onward;
		}
	
		force = FALSE;

		/* dump comments */
		if (*eline == '*')
			goto onward;

		/* now, execute directives */
		if (dirnum != -1) {
			/* skip past the directive */
			while (*eline && *eline != ' ' && *eline != '\t')
				++eline;
			execstr = eline;

			switch (dirnum) {
			case DIF:	/* IF directive */
				/* grab the value of the logical exp */
				if (execlevel == 0) {
					if (macarg(tkn) != TRUE)
						goto eexec;
					if (stol(tkn) == FALSE)
						++execlevel;
				} else
					++execlevel;
				goto onward;

			case DWHILE:	/* WHILE directive */
				/* grab the value of the logical exp */
				if (execlevel == 0) {
					if (macarg(tkn) != TRUE)
						goto eexec;
					if (stol(tkn) == TRUE)
						goto onward;
				}
				/* drop down and act just like !BREAK */

			case DBREAK:	/* BREAK directive */
				if (dirnum == DBREAK && execlevel)
					goto onward;

				/* jump down to the endwhile */
				/* find the right while loop */
				whtemp = whlist;
				while (whtemp) {
					if (whtemp->w_begin == lp)
						break;
					whtemp = whtemp->w_next;
				}
			
				if (whtemp == NULL) {
					errormesg(TEXT126, bp, lp);
/*                                              "%%Internal While loop error" */
					freewhile(whlist);
					bp->b_exec -= 1;
					return(FALSE);
				}
			
				/* reset the line pointer back.. */
				lp = whtemp->w_end;
				goto onward;

			case DELSE:	/* ELSE directive */
				if (execlevel == 1)
					--execlevel;
				else if (execlevel == 0 )
					++execlevel;
				goto onward;

			case DENDIF:	/* ENDIF directive */
				if (execlevel)
					--execlevel;
				goto onward;

			case DGOTO:	/* GOTO directive */
				/* .....only if we are currently executing */
				if (execlevel == 0) {

					/* grab label to jump to */
					eline = token(eline, golabel, NPAT);
					linlen = strlen(golabel);
					glp = hlp->l_fp;
					while (glp != hlp) {
						if (*glp->l_text == '*' &&
						    (strncmp(&glp->l_text[1], golabel,
						            linlen) == 0)) {
							lp = glp;
							goto onward;
						}
						glp = glp->l_fp;
					}
					errormesg(TEXT127, bp, lp);
/*                                              "%%No such label" */
					freewhile(whlist);
					bp->b_exec -= 1;
					return(FALSE);
				}
				goto onward;
	
			case DRETURN:	/* RETURN directive */
				if (execlevel == 0)
					goto eexec;
				goto onward;

			case DENDWHILE:	/* ENDWHILE directive */
				if (execlevel) {
					--execlevel;
					goto onward;
				} else {
					/* find the right while loop */
					whtemp = whlist;
					while (whtemp) {
						if (whtemp->w_type == BTWHILE &&
						    whtemp->w_end == lp)
							break;
						whtemp = whtemp->w_next;
					}
		
					if (whtemp == NULL) {
						errormesg(TEXT126, bp, lp);
/*                                                      "%%Internal While loop error" */
						freewhile(whlist);
						bp->b_exec -= 1;
						return(FALSE);
					}
		
					/* reset the line pointer back.. */
					lp = whtemp->w_begin->l_bp;
					goto onward;
				}

			case DFORCE:	/* FORCE directive */
				force = TRUE;

			}
		}

		/* execute the statement */
		status = docmd(eline);
		if (force)		/* force the status */
			status = TRUE;

		/* check for a command error */
		if (status != TRUE) {

			/* look if buffer is showing */
			wp = wheadp;
			while (wp != NULL) {
				if (wp->w_bufp == bp) {
					/* and point it */
					wp->w_dotp = lp;
					wp->w_doto = 0;
					wp->w_flag |= WFHARD;
				}
				wp = wp->w_wndp;
			}

			/* in any case set the buffer . */
			bp->b_dotp = lp;
			bp->b_doto = 0;
			free(einit);
			execlevel = 0;
			freewhile(whlist);

			errormesg(TEXT219, bp, lp);
/*				  "%%Macro Failed" */
			bp->b_exec -= 1;
			return(status);
		}

onward:		/* on to the next line */
		free(einit);
		lp = lp->l_fp;
#if	DEBUGM
		if (skipflag)
			macbug = TRUE;
#endif
	}

eexec:	/* exit the current function */
	execlevel = 0;
	freewhile(whlist);
	bp->b_exec -= 1;
        return(TRUE);
}

/* errormesg:	display a macro execution error along with the buffer and
		line currently being executed */

errormesg(mesg, bp, lp)

char *mesg;	/* error message to display */
BUFFER *bp;	/* buffer error occured in */
LINE *lp;	/* line " */

{
	char buf[NSTRING];

	exec_error = TRUE;

	/* build error message line */
	strcpy(buf, "\n");
	strcat(buf, mesg);
	strcat(buf, " in <");
	strcat(buf, bp->b_bname);
	strcat(buf, "> at line ");
	strcat(buf, int_asc(getlinenum(bp, lp)));
	mlforce(buf);
}

#if	DEBUGM
/*		Interactive debugger

		if $debug == TRUE, The interactive debugger is invoked
		commands are listed out with the ? key			*/

PASCAL NEAR debug(bp, eline, skipflag)

BUFFER *bp;	/* buffer to execute */
char *eline;	/* text of line to debug */
int *skipflag;	/* are we skipping debugging? */

{
	register int oldcmd;		/* original command display flag */
	register int oldinp;		/* original connamd input flag */
	register int oldstatus;		/* status of last command */
	register int c;			/* temp character */
	register KEYTAB *key;		/* ptr to a key entry */
	static char track[NSTRING] = "";/* expression to track value of */
	char temp[NSTRING];		/* command or expression */

dbuild:	/* Build the information line to be presented to the user */

	strcpy(outline, "<<<");

	/* display the tracked expression */
	if (track[0] != 0) {
		oldstatus = cmdstatus;
		docmd(track);
		cmdstatus = oldstatus;
		strcat(outline, "[=");
		strcat(outline, gtusr("track"));
		strcat(outline, "]");
	}

	/* debug macro name */
	strcat(outline, bp->b_bname);
	strcat(outline, ":");

	/* and lastly the line */
	strcat(outline, eline);
	strcat(outline, ">>>");

	/* expand the %'s so mlwrite() won't interpret them */
	makelit(outline);

	/* write out the debug line */
dinput:	outline[term.t_ncol - 1] = 0;
	mlforce(outline);
	update(TRUE);

	/* and get the keystroke */
	c = getkey();

	/* META key turns off debugging */
	key = getbind(c);
	if (key && key->k_ptr.fp == meta)
		macbug = FALSE;

	else if (c == abortc) {
		return(FALSE);

	} else switch (c) {

		case '?': /* list commands */
			strcpy(outline, TEXT128);
/*"(e)val exp, (c/x)ommand, (t)rack exp, (^G)abort, <SP>exec, <META> stop debug"*/
			goto dinput;

		case 'c': /* execute statement */
			oldcmd = discmd;
			discmd = TRUE;
			oldinp = disinp;
			disinp = TRUE;
			execcmd(FALSE, 1);
			discmd = oldcmd;
			disinp = oldinp;
			goto dbuild;

		case 'x': /* execute extended command */
			oldcmd = discmd;
			discmd = TRUE;
			oldinp = disinp;
			disinp = TRUE;
			oldstatus = cmdstatus;
			namedcmd(FALSE, 1);
			cmdstatus = oldstatus;
			discmd = oldcmd;
			disinp = oldinp;
			goto dbuild;

		case 'e': /* evaluate expresion */
			strcpy(temp, "set %track ");
			oldcmd = discmd;
			discmd = TRUE;
			oldinp = disinp;
			disinp = TRUE;
			getstring("Exp: ", &temp[11], NSTRING, ctoec('\r'));
			discmd = oldcmd;
			disinp = oldinp;
			oldstatus = cmdstatus;
			docmd(temp);
			cmdstatus = oldstatus;
			strcpy(temp, " = [");
			strcat(temp, gtusr("track"));
			strcat(temp, "]");
			mlforce(temp);
			c = getkey();
			goto dinput;

		case 't': /* track expresion */
			oldcmd = discmd;
			discmd = TRUE;
			oldinp = disinp;
			disinp = TRUE;
			getstring("Exp: ", temp, NSTRING, ctoec('\r'));
			discmd = oldcmd;
			disinp = oldinp;
			strcpy(track, "set %track ");
			strcat(track, temp);
			goto dbuild;

		case 's': /* execute a function */
			*skipflag = TRUE;
			macbug = FALSE;
			break;

		case ' ': /* execute a statement */
			break;

		default: /* illegal command */
			TTbeep();
			goto dbuild;
	}
	return(TRUE);
}
#endif

PASCAL NEAR makelit(s)		/* expand all "%" to "%%" */

char *s;	/* string to expand */

{
	register char *sp;	/* temp for expanding string */
	register char *ep;	/* ptr to end of string to expand */

	sp = s;
	while (*sp)
	if (*sp++ == '%') {
		/* advance to the end */
		ep = --sp;
		while (*ep++)
			;
		/* null terminate the string one out */
		*(ep + 1) = 0;
		/* copy backwards */
		while(ep-- > sp)
			*(ep + 1) = *ep;

		/* and advance sp past the new % */
		sp += 2;					
	}
}

PASCAL NEAR freewhile(wp)	/* free a list of while block pointers */

WHBLOCK *wp;	/* head of structure to free */

{
	if (wp != NULL)
	{
		freewhile(wp->w_next);
		free((char *) wp);
	}
}

PASCAL NEAR execfile(f, n)	/* execute a series of commands in a file */

int f, n;	/* default flag and numeric arg to pass on to file */

{
	register int status;	/* return status of name query */
	char fname[NSTRING];	/* name of file to execute */
	char *fspec;		/* full file spec */

	if ((status = mlreply(TEXT129, fname, NSTRING -1)) != TRUE)
/*                            "File to execute: " */
		return(status);

	/* look up the path for the file */
	fspec = flook(fname, TRUE);

	/* if it isn't around */
	if (fspec == NULL) {

		/* try to default the extension */
		if (sindex(fname, ".") == 0) {
			strcat(fname, ".cmd");
			fspec = flook(fname, TRUE);
			if (fspec != NULL)
				goto exec1;
		}

		/* complain if we are interactive */
		if (clexec == FALSE)
			mlwrite(TEXT214, fname);
/*			        "%%No such file as %s" */
		return(FALSE);
	}

exec1:	/* otherwise, execute it */
	while (n-- > 0)
		if ((status=dofile(fspec)) != TRUE)
			return(status);

	return(TRUE);
}

/*	dofile:	yank a file into a buffer and execute it
		if there are no errors, delete the buffer on exit */

PASCAL NEAR dofile(fname)

char *fname;	/* file name to execute */

{
	register BUFFER *bp;	/* buffer to place file to exeute */
	register BUFFER *cb;	/* temp to hold current buf while we read */
	register int status;	/* results of various calls */
	char bname[NBUFN];	/* name of buffer */

	makename(bname, fname);		/* derive the name of the buffer */
	unqname(bname);			/* make sure we don't stomp things */
	if ((bp = bfind(bname, TRUE, 0)) == NULL) /* get the needed buffer */
		return(FALSE);

	bp->b_mode = MDVIEW;	/* mark the buffer as read only */
	cb = curbp;		/* save the old buffer */
	curbp = bp;		/* make this one current */
	/* and try to read in the file to execute */
	if ((status = readin(fname, FALSE)) != TRUE) {
		curbp = cb;	/* restore the current buffer */
		return(status);
	}

	/* go execute it! */
	curbp = cb;		/* restore the current buffer */
	if ((status = dobuf(bp)) != TRUE)
		return(status);

	/* if not displayed, remove the now unneeded buffer and exit */
	if (bp->b_nwnd == 0)
		zotbuf(bp);
	return(TRUE);
}

/*	cbuf:	Execute the contents of a numbered buffer	*/

PASCAL NEAR cbuf(f, n, bufnum)

int f, n;	/* default flag and numeric arg */
int bufnum;	/* number of buffer to execute */

{
        register BUFFER *bp;		/* ptr to buffer to execute */
        register int status;		/* status return */
	static char bufname[] = "[Macro xx]";

	/* make the buffer name */
	bufname[7] = '0' + (bufnum / 10);
	bufname[8] = '0' + (bufnum % 10);

	/* find the pointer to that buffer */
        if ((bp=bfind(bufname, FALSE, 0)) == NULL) {
        	mlwrite(TEXT130);
/*                      "Macro not defined" */
                return(FALSE);
        }

	/* and now execute it as asked */
	while (n-- > 0)
		if ((status = dobuf(bp)) != TRUE)
			return(status);
	return(TRUE);
}

PASCAL NEAR cbuf1(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 1));
}

PASCAL NEAR cbuf2(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 2));
}

PASCAL NEAR cbuf3(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 3));
}

PASCAL NEAR cbuf4(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 4));
}

PASCAL NEAR cbuf5(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 5));
}

PASCAL NEAR cbuf6(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 6));
}

PASCAL NEAR cbuf7(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 7));
}

PASCAL NEAR cbuf8(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 8));
}

PASCAL NEAR cbuf9(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 9));
}

PASCAL NEAR cbuf10(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 10));
}

PASCAL NEAR cbuf11(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 11));
}

PASCAL NEAR cbuf12(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 12));
}

PASCAL NEAR cbuf13(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 13));
}

PASCAL NEAR cbuf14(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 14));
}

PASCAL NEAR cbuf15(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 15));
}

PASCAL NEAR cbuf16(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 16));
}

PASCAL NEAR cbuf17(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 17));
}

PASCAL NEAR cbuf18(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 18));
}

PASCAL NEAR cbuf19(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 19));
}

PASCAL NEAR cbuf20(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 20));
}

PASCAL NEAR cbuf21(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 21));
}

PASCAL NEAR cbuf22(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 22));
}

PASCAL NEAR cbuf23(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 23));
}

PASCAL NEAR cbuf24(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 24));
}

PASCAL NEAR cbuf25(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 25));
}

PASCAL NEAR cbuf26(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 26));
}

PASCAL NEAR cbuf27(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 27));
}

PASCAL NEAR cbuf28(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 28));
}

PASCAL NEAR cbuf29(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 29));
}

PASCAL NEAR cbuf30(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 30));
}

PASCAL NEAR cbuf31(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 31));
}

PASCAL NEAR cbuf32(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 32));
}

PASCAL NEAR cbuf33(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 33));
}

PASCAL NEAR cbuf34(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 34));
}

PASCAL NEAR cbuf35(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 35));
}

PASCAL NEAR cbuf36(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 36));
}

PASCAL NEAR cbuf37(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 37));
}

PASCAL NEAR cbuf38(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 38));
}

PASCAL NEAR cbuf39(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 39));
}

PASCAL NEAR cbuf40(f, n)

int f, n;	/* flag and numeric argument */

{
	return(cbuf(f, n, 40));
}


