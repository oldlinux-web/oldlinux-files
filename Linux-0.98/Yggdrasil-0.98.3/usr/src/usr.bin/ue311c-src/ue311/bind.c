/*	This file is for functions having to do with key bindings,
	descriptions, help commands and startup file.

	written 11-feb-86 by Daniel Lawrence
								*/

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"
#include	"epath.h"

PASCAL NEAR help(f, n)	/* give me some help!!!!
		   bring up a fake buffer and read the help file
		   into it with view mode			*/

int f,n;	/* prefix flag and argument */

{
	register BUFFER *bp;	/* buffer pointer to help */
	char *fname;		/* file name of help file */

	/* first check if we are already here */
	bp = bfind("emacs.hlp", FALSE, BFINVS);

	if (bp == NULL) {
#if SHARED
		strcpy(tname, pathname[1]);
		fname = flook(tname, FALSE);
#else	    
		fname = flook(pathname[1], FALSE);
#endif
		if (fname == NULL) {
			mlwrite(TEXT12);
/*				"[Help file is not online]" */
			return(FALSE);
		}
	}

	/* split the current window to make room for the help stuff */
	if (splitwind(FALSE, 1) == FALSE)
			return(FALSE);

	if (bp == NULL) {
		/* and read the stuff in */
		if (getfile(fname, FALSE) == FALSE)
			return(FALSE);
	} else
		swbuffer(bp);

	/* make this window in VIEW mode, update all mode lines */
	curwp->w_bufp->b_mode |= MDVIEW;
	curwp->w_bufp->b_flag |= BFINVS;
	upmode();
	return(TRUE);
}

PASCAL NEAR deskey(f, n)	/* describe the command for a certain key */

int f,n;	/* prefix flag and argument */

{
	register int c; 	/* key to describe */
	register char *ptr;	/* string pointer to scan output strings */
	char outseq[NSTRING];	/* output buffer for command sequence */

	/* prompt the user to type us a key to describe */
	mlwrite(TEXT13);
/*		": describe-key " */

	/* get the command sequence to describe
	   change it to something we can print as well */
	cmdstr(c = getckey(FALSE), &outseq[0]);

	/* and dump it out */
	ostring(outseq);
	ostring(" ");

	/* find the right ->function */
	if ((ptr = getfname(getbind(c))) == NULL)
		ptr = "Not Bound";

	/* output the command sequence */
	ostring(ptr);
	return(TRUE);
}

/* bindtokey:	add a new key to the key binding table		*/

PASCAL NEAR bindtokey(f, n)

int f, n;	/* command arguments [IGNORED] */

{
	register unsigned int c;/* command key to bind */
	register int (PASCAL NEAR *kfunc)();/* ptr to the requested function to bind to */
	register KEYTAB *ktp;	/* pointer into the command table */
	register int found;	/* matched command flag */
	char outseq[80];	/* output buffer for keystroke sequence */

	/* prompt the user to type in a key to bind */
	/* get the function name to bind it to */
	kfunc = getname(TEXT15);
/*			": bind-to-key " */
	if (kfunc == NULL) {
		mlwrite(TEXT16);
/*			"[No such function]" */
		return(FALSE);
	}
	if (clexec == FALSE) {
		ostring(" ");
		TTflush();
	}

	/* get the command sequence to bind */
	c = getckey((kfunc == meta) || (kfunc == cex) ||
		    (kfunc == unarg) || (kfunc == ctrlg));

	if (clexec == FALSE) {

		/* change it to something we can print as well */
		cmdstr(c, &outseq[0]);

		/* and dump it out */
		ostring(outseq);
	}

	/* if the function is a unique prefix key */
	if (kfunc == unarg || kfunc == ctrlg || kfunc == quote) {

		/* search for an existing binding for the prefix key */
		ktp = &keytab[0];
		while (ktp->k_type != BINDNUL) {
			if (ktp->k_ptr.fp == kfunc)
				unbindchar(ktp->k_code);
			++ktp;
		}

		/* reset the appropriate global prefix variable */
		if (kfunc == unarg)
			reptc = c;
		if (kfunc == ctrlg)
			abortc = c;
		if (kfunc == quote)
			quotec = c;
	}

	/* search the table to see if it exists */
	ktp = &keytab[0];
	found = FALSE;
	while (ktp->k_type != BINDNUL) {
		if (ktp->k_code == c) {
			found = TRUE;
			break;
		}
		++ktp;
	}

	if (found) {	/* it exists, just change it then */
		ktp->k_ptr.fp = kfunc;
		ktp->k_type = BINDFNC;
	} else {	/* otherwise we need to add it to the end */
		/* if we run out of binding room, bitch */
		if (ktp >= &keytab[NBINDS]) {
			mlwrite(TEXT17);
/*				"Binding table FULL!" */
			return(FALSE);
		}

		ktp->k_code = c;	/* add keycode */
		ktp->k_ptr.fp = kfunc;	/* and the function pointer */
		ktp->k_type = BINDFNC;	/* and the binding type */
		++ktp;			/* and make sure the next is null */
		ktp->k_code = 0;
		ktp->k_type = BINDNUL;
		ktp->k_ptr.fp = NULL;
	}

	/* if we have rebound the meta key, make the
	   search terminator follow it			*/
	if (kfunc == meta)
		sterm = c;

	return(TRUE);
}

/* macrotokey:	Bind a key to a macro in the key binding table */

PASCAL NEAR macrotokey(f, n)

int f, n;	/* command arguments [IGNORED] */

{
	register unsigned int c;/* command key to bind */
	register BUFFER *kmacro;/* ptr to buffer of macro to bind to key */
	register KEYTAB *ktp;	/* pointer into the command table */
	register int found;	/* matched command flag */
	register int status;	/* error return */
	char outseq[80];	/* output buffer for keystroke sequence */
	char bufn[NBUFN];	/* buffer to hold macro name */

	/* get the buffer name to use */
	if ((status=mlreply(TEXT215, &bufn[1], NBUFN-2)) != TRUE)
/*		": macro-to-key " */
		return(status);

	/* build the responce string for later */
	strcpy(outseq, TEXT215);
/*		   ": macro-to-key " */
	strcat(outseq, &bufn[1]);

	/* translate it to a buffer pointer */
	bufn[0] = '[';
	strcat(bufn, "]");
	if ((kmacro=bfind(bufn, FALSE, 0)) == NULL) {
		mlwrite(TEXT130);
/*		"Macro not defined"*/
		return(FALSE);
	}

	strcat(outseq, " ");
	mlwrite(outseq);

	/* get the command sequence to bind */
	c = getckey(FALSE);

	/* change it to something we can print as well */
	cmdstr(c, &outseq[0]);

	/* and dump it out */
	ostring(outseq);

	/* search the table to see if it exists */
	ktp = &keytab[0];
	found = FALSE;
	while (ktp->k_type != BINDNUL) {
		if (ktp->k_code == c) {
			found = TRUE;
			break;
		}
		++ktp;
	}

	if (found) {	/* it exists, just change it then */
		ktp->k_ptr.buf = kmacro;
		ktp->k_type = BINDBUF;
	} else {	/* otherwise we need to add it to the end */
		/* if we run out of binding room, bitch */
		if (ktp >= &keytab[NBINDS]) {
			mlwrite(TEXT17);
/*				"Binding table FULL!" */
			return(FALSE);
		}

		ktp->k_code = c;	/* add keycode */
		ktp->k_ptr.buf = kmacro;	/* and the function pointer */
		ktp->k_type = BINDBUF;	/* and the binding type */
		++ktp;			/* and make sure the next is null */
		ktp->k_code = 0;
		ktp->k_type = BINDNUL;
		ktp->k_ptr.fp = NULL;
	}

	return(TRUE);
}

/* unbindkey:	delete a key from the key binding table */

PASCAL NEAR unbindkey(f, n)

int f, n;	/* command arguments [IGNORED] */

{
	register int c; 	/* command key to unbind */
	char outseq[80];	/* output buffer for keystroke sequence */

	/* prompt the user to type in a key to unbind */
	mlwrite(TEXT18);
/*		": unbind-key " */

	/* get the command sequence to unbind */
	c = getckey(FALSE);		/* get a command sequence */

	/* change it to something we can print as well */
	cmdstr(c, &outseq[0]);

	/* and dump it out */
	ostring(outseq);

	/* if it isn't bound, bitch */
	if (unbindchar(c) == FALSE) {
		mlwrite(TEXT19);
/*			"[Key not bound]" */
		return(FALSE);
	}
	return(TRUE);
}

PASCAL NEAR unbindchar(c)

int c;		/* command key to unbind */

{
	register KEYTAB *ktp;	/* pointer into the command table */
	register KEYTAB *sktp;	/* saved pointer into the command table */
	register int found;	/* matched command flag */

	/* search the table to see if the key exists */
	ktp = &keytab[0];
	found = FALSE;
	while (ktp->k_type != BINDNUL) {
		if (ktp->k_code == c) {
			found = TRUE;
			break;
		}
		++ktp;
	}

	/* if it isn't bound, bitch */
	if (!found)
		return(FALSE);

	/* save the pointer and scan to the end of the table */
	sktp = ktp;
	while (ktp->k_type != BINDNUL)
		++ktp;
	--ktp;		/* backup to the last legit entry */

	/* copy the last entry to the current one */
	sktp->k_code = ktp->k_code;
	sktp->k_type = ktp->k_type;
	sktp->k_ptr.fp	 = ktp->k_ptr.fp;

	/* null out the last one */
	ktp->k_code = 0;
	ktp->k_type = BINDNUL;
	ktp->k_ptr.fp = NULL;
	return(TRUE);
}

/* Describe bindings:

	   bring up a fake buffer and list the key bindings
	   into it with view mode
*/

PASCAL NEAR desbind(f, n)

int f,n;	/* prefix flag and argument */

{
	return(buildlist(TRUE, ""));
}

PASCAL NEAR apro(f, n)	/* Apropos (List functions that match a substring) */

int f,n;	/* prefix flag and argument */

{
	char mstring[NSTRING];	/* string to match cmd names to */
	int status;		/* status return */

	status = mlreply(TEXT20, mstring, NSTRING - 1);
/*			 "Apropos string: " */
	if (status != TRUE)
		return(status);

	return(buildlist(FALSE, mstring));
}

PASCAL NEAR buildlist(type, mstring)  /* build a binding list (limited or full) */

int type;	/* true = full list,   false = partial list */
char *mstring;	/* match string if a partial list */

{
	register KEYTAB *ktp;	/* pointer into the command table */
	register NBIND *nptr;	/* pointer into the name binding table */
	register BUFFER *listbuf;/* buffer to put binding list into */
	register BUFFER *bp;	/* buffer ptr for function scan */
	int cpos;		/* current position to use in outseq */
	char outseq[80];	/* output buffer for keystroke sequence */

	/* get a buffer for the binding list */
	listbuf = bfind(TEXT21, TRUE, BFINVS);
/*		   "Binding list" */
	if (listbuf == NULL || bclear(listbuf) == FALSE) {
		mlwrite(TEXT22);
/*			"Can not display binding list" */
		return(FALSE);
	}

	/* let us know this is in progress */
	mlwrite(TEXT23);
/*		"[Building binding list]" */

	/* build the contents of this window, inserting it line by line */
	nptr = &names[0];
	while (nptr->n_func != NULL) {

		/* add in the command name */
		strcpy(outseq, nptr->n_name);
		cpos = strlen(outseq);

		/* if we are executing an apropos command..... */
		if (type == FALSE &&
		    /* and current string doesn't include the search string */
		    strinc(outseq, mstring) == FALSE)
			goto fail;

		/* search down any keys bound to this */
		ktp = &keytab[0];
		while (ktp->k_type != BINDNUL) {
			if (ktp->k_type == BINDFNC &&
			    ktp->k_ptr.fp == nptr->n_func) {
				/* padd out some spaces */
				while (cpos < 25)
					outseq[cpos++] = ' ';

				/* add in the command sequence */
				cmdstr(ktp->k_code, &outseq[cpos]);

				/* and add it as a line into the buffer */
				if (addline(listbuf, outseq) != TRUE)
					return(FALSE);

				cpos = 0;	/* and clear the line */
			}
			++ktp;
		}

		/* if no key was bound, we need to dump it anyway */
		if (cpos > 0) {
			outseq[cpos] = 0;
			if (addline(listbuf, outseq) != TRUE)
				return(FALSE);
		}

fail:		/* and on to the next name */
		++nptr;
	}

	/* add a blank line between the key and macro lists */
	if (addline(listbuf, "") != TRUE)
		return(FALSE);

	/* scan all buffers looking for macroes and their bindings */
	bp = bheadp;
	while (bp) {

		/* is this buffer a macro? */
		if (bp->b_bname[0] != '[')
			goto bfail;

		/* add in the command name */
		strcpy(outseq, bp->b_bname);
		cpos = strlen(outseq);

		/* if we are executing an apropos command..... */
		if (type == FALSE &&
		    /* and current string doesn't include the search string */
		    strinc(outseq, mstring) == FALSE)
			goto bfail;

		/* search down any keys bound to this macro */
		ktp = &keytab[0];
		while (ktp->k_type != BINDNUL) {
			if (ktp->k_type == BINDBUF &&
			    ktp->k_ptr.buf == bp) {
				/* padd out some spaces */
				while (cpos < 25)
					outseq[cpos++] = ' ';

				/* add in the command sequence */
				cmdstr(ktp->k_code, &outseq[cpos]);

				/* and add it as a line into the buffer */
				if (addline(listbuf, outseq) != TRUE)
					return(FALSE);

				cpos = 0;	/* and clear the line */
			}
			++ktp;
		}

		/* if no key was bound, we need to dump it anyway */
		if (cpos > 0) {
			outseq[cpos] = 0;
			if (addline(listbuf, outseq) != TRUE)
				return(FALSE);
		}

bfail:		/* and on to the next buffer */
		bp = bp->b_bufp;
	}

	wpopup(listbuf);
	mlerase();	/* clear the mode line */
	return(TRUE);
}

PASCAL NEAR strinc(source, sub) /* does source include sub? */

char *source;	/* string to search in */
char *sub;	/* substring to look for */

{
	char *sp;	/* ptr into source */
	char *nxtsp;	/* next ptr into source */
	char *tp;	/* ptr into substring */

	/* for each character in the source string */
	sp = source;
	while (*sp) {
		tp = sub;
		nxtsp = sp;

		/* is the substring here? */
		while (*tp) {
			if (*nxtsp++ != *tp)
				break;
			else
				tp++;
		}

		/* yes, return a success */
		if (*tp == 0)
			return(TRUE);

		/* no, onward */
		sp++;
	}
	return(FALSE);
}

/* get a command key sequence from the keyboard */

unsigned int PASCAL NEAR getckey(mflag)

int mflag;	/* going for a meta sequence? */

{
	register unsigned int c;	/* character fetched */
	char tok[NSTRING];		/* command incoming */

	/* check to see if we are executing a command line */
	if (clexec) {
		macarg(tok);	/* get the next token */
		return(stock(tok));
	}

	/* or the normal way */
	if (mflag)
		c = getkey();
	else
		c = getcmd();
	return(c);
}

/* execute the startup file */

PASCAL NEAR startup(sfname)

char *sfname;	/* name of startup file (null if default) */

{
	char *fname;	/* resulting file name to execute */
	char name[NSTRING];	/* name with extention */

	/* look up the startup file */
	if (*sfname != 0) {

	 	/* default the extention */
		strcpy(name, sfname);
		if (sindex(name, ".") == 0)
			strcat(name, ".cmd");

		fname = flook(name, TRUE);
	} else
#if SHARED
	{
		strcpy(tname, pathname[0]);
		fname = flook(tname, TRUE);
	}
#else
		fname = flook(pathname[0], TRUE);
#endif

	/* if it isn't around, don't sweat it */
	if (fname == NULL)
		return(TRUE);

	/* otherwise, execute the sucker */
	return(dofile(fname));
}

/*	Look up the existance of a file along the normal or PATH
	environment variable.

	LOOKUP ORDER:

		if contains path:

			absolute

		else

			HOME environment directory
			all directories along PATH environment
			directories in table from EPATH.H
*/

char *PASCAL NEAR flook(fname, hflag)

char *fname;	/* base file name to search for */
int hflag;	/* Look in the HOME environment variable first? */

{
	register char *home;	/* path to home directory */
	register char *path;	/* environmental PATH variable */
	register char *sp;	/* pointer into path spec */
	register int i; 	/* index */
	static char fspec[NFILEN];	/* full path spec to search */
	char *getenv();

	/* if we have an absolute path.. check only there! */
	sp = fname;
	while (*sp) {
		if (*sp == ':' || *sp == '\\' || *sp == '/') {
			if (ffropen(fname) == FIOSUC) {
				ffclose();
				return(fname);
			} else
				return(NULL);
		}
		++sp;
	}

#if	ENVFUNC

	if (hflag) {
#if WMCS
		home = getenv("SYS$HOME");
#else
		home = getenv("HOME");
#endif
		if (home != NULL) {
			/* build home dir file spec */
			strcpy(fspec, home);
#if WMCS
			strcat(fspec,fname);
#else
			strcat(fspec, DIRSEPSTR);
			strcat(fspec, fname);
#endif

			/* and try it out */
			if (ffropen(fspec) == FIOSUC) {
				ffclose();
				return(fspec);
			}
		}
	}
#endif

#if	ENVFUNC
	/* get the PATH variable */
#if WMCS
	path = getenv("OPT$PATH");
#else
	path = getenv("PATH");
#endif
	if (path != NULL)
		while (*path) {

			/* build next possible file spec */
			sp = fspec;
#if	TOS
			while (*path && (*path != PATHCHR) && (*path != ','))
#else
			while (*path && (*path != PATHCHR))
#endif
				*sp++ = *path++;

			/* add a terminating dir separator if we need it */
			if (*(sp-1) != DIRSEPCHAR)
				*sp++ = DIRSEPCHAR;
			*sp = 0;
			strcat(fspec, fname);

			/* and try it out */
			if (ffropen(fspec) == FIOSUC) {
				ffclose();
				return(fspec);
			}

#if	TOS && MWC
			if ((*path == PATHCHR) || (*path == ','))
#else
			if (*path == PATHCHR)
#endif
				++path;
		}
#endif

	/* look it up via the old table method */
	for (i=2; i < NPNAMES; i++) {
		strcpy(fspec, pathname[i]);
		strcat(fspec, fname);

		/* and try it out */
		if (ffropen(fspec) == FIOSUC) {
			ffclose();
			return(fspec);
		}
	}

	return(NULL);	/* no such luck */
}

PASCAL NEAR cmdstr(c, seq) /* change a key command to a string we can print out */

int c;		/* sequence to translate */
char *seq;	/* destination string for sequence */

{
	char *ptr;	/* pointer into current position in sequence */

	ptr = seq;

	/* apply ^X sequence if needed */
	if (c & CTLX) {
		*ptr++ = '^';
		*ptr++ = 'X';
	}

	/* apply ALT key sequence if needed */
	if (c & ALTD) {
		*ptr++ = 'A';
		*ptr++ = '-';
	}

	/* apply Shifted sequence if needed */
	if (c & SHFT) {
		*ptr++ = 'S';
		*ptr++ = '-';
	}

	/* apply MOUS sequence if needed */
	if (c & MOUS) {
		*ptr++ = 'M';
		*ptr++ = 'S';
	}

	/* apply meta sequence if needed */
	if (c & META) {
		*ptr++ = 'M';
		*ptr++ = '-';
	}

	/* apply SPEC sequence if needed */
	if (c & SPEC) {
		*ptr++ = 'F';
		*ptr++ = 'N';
	}

	/* apply control sequence if needed */
	if (c & CTRL) {
		*ptr++ = '^';
	}

	c = c & 255;	/* strip the prefixes */

	/* and output the final sequence */

	*ptr++ = c;
	*ptr = 0;	/* terminate the string */
}

/*	This function looks a key binding up in the binding table	*/

KEYTAB *getbind(c)

register int c;	/* key to find what is bound to it */

{
	register KEYTAB *ktp;

	/* scan through the binding table, looking for the key's entry */
	ktp = &keytab[0];
	while (ktp->k_type != BINDNUL) {
		if (ktp->k_code == c)
			return(ktp);
		++ktp;
	}

	/* no such binding */
	return((KEYTAB *)NULL);
}

/* getfname:	This function takes a ptr to KEYTAB entry and gets the name
		associated with it
*/

char *PASCAL NEAR getfname(key)

KEYTAB *key;	/* key binding to return a name of */

{
	int (PASCAL NEAR *func)(); /* ptr to the requested function */
	register NBIND *nptr;	/* pointer into the name binding table */
	register BUFFER *bp;	/* ptr to buffer to test */
	register BUFFER *kbuf;	/* ptr to requested buffer */

	/* if this isn't a valid key, it has no name */
	if (key == NULL)
		return(NULL);

	/* skim through the binding table, looking for a match */
	if (key->k_type == BINDFNC) {
		func = key->k_ptr.fp;
		nptr = &names[0];
		while (nptr->n_func != NULL) {
			if (nptr->n_func == func)
				return(nptr->n_name);
			++nptr;
		}
		return(NULL);
	}

	/* skim through the buffer list looking for a match */
	kbuf = key->k_ptr.buf;
	bp = bheadp;
	while (bp) {
		if (bp == kbuf)
			return(bp->b_bname);
		bp = bp->b_bufp;
	}
	return(NULL);
}

/* fncmatch:	match fname to a function in the names table and return
		any match or NULL if none */

#if	MSC
int (PASCAL NEAR *PASCAL NEAR fncmatch(char *fname))(void)
#else
int (PASCAL NEAR *PASCAL NEAR fncmatch(fname))()

char *fname;	/* name to attempt to match */
#endif

{
	int nval;

	if ((nval = binary(fname, namval, numfunc)) == -1)
		return(NULL);
	else
		return(names[nval].n_func);
}

char *PASCAL NEAR namval(index)

int index;	/* index of name to fetch out of the name table */

{
	return(names[index].n_name);
}

/*	stock() 	String key name TO Command Key

	A key binding consists of one or more prefix functions followed by
	a keystroke.  Allowable prefixes must be in the following order:

	^X	preceeding control-X
	A-	similtaneous ALT key (on PCs mainly)
	S-	shifted function key
	MS	mouse generated keystroke
	M-	Preceding META key
	FN	function key
	^	control key

	Meta and ^X prefix of lower case letters are converted to upper
	case.  Real control characters are automatically converted to
	the ^A form.
*/

unsigned int PASCAL NEAR stock(keyname)

char *keyname;	/* name of key to translate to Command key form */

{
	register unsigned int c;	/* key sequence to return */

	/* parse it up */
	c = 0;

	/* Do ^X prefix */
	if(*keyname == '^' && *(keyname+1) == 'X') {
		if(*(keyname+2) != 0) { /* Key is not bare ^X */
		    c |= CTLX;
		    keyname += 2;
		}
	}

	/* and the ALT key prefix */
	if (*keyname == 'A' && *(keyname+1) == '-') {
		c |= ALTD;
		keyname += 2;
	}

	/* and the SHIFTED prefix */
	if (*keyname == 'S' && *(keyname+1) == '-') {
		c |= SHFT;
		keyname += 2;
	}

	/* and the mouse (MOUS) prefix */
	if (*keyname == 'M' && *(keyname+1) == 'S') {
		c |= MOUS;
		keyname += 2;
	}

	/* then the META prefix */
	if (*keyname == 'M' && *(keyname+1) == '-') {
		c |= META;
		keyname += 2;
	}

	/* next the function prefix */
	if (*keyname == 'F' && *(keyname+1) == 'N') {
		c |= SPEC;
		keyname += 2;
	}

	/* a control char?  (Always upper case) */
	if (*keyname == '^' && *(keyname+1) != 0) {
		c |= CTRL;
		++keyname;
		uppercase(keyname);
	}

	/* A literal control character? (Boo, hiss) */
	if (*keyname < 32) {
		c |= CTRL;
		*keyname += '@';
	}

	/* make sure we are not lower case if used with ^X or M- */
	if(!(c & (MOUS|SPEC|ALTD|SHFT)))	/* If not a special key */
	    if( c & (CTLX|META))		/* If is a prefix */
		uppercase(keyname);		/* Then make sure it's upper case */

	/* the final sequence... */
	c |= *keyname;
	return(c);
}

char *PASCAL NEAR transbind(skey)	/* string key name to binding name.... */

char *skey;	/* name of key to get binding for */

{
	char *bindname;

	bindname = getfname(getbind(stock(skey)));
	if (bindname == NULL)
		bindname = errorm;

	return(bindname);
}

int PASCAL NEAR execkey(key, f, n)	/* execute a function bound to a key */

KEYTAB *key;	/* key to execute */
int f, n;	/* agruments to C function */

{
	register int status;	/* error return */

	if (key->k_type == BINDFNC)
		return((*(key->k_ptr.fp))(f, n));
	if (key->k_type == BINDBUF) {
		while (n--) {
			status = dobuf(key->k_ptr.buf);
			if (status != TRUE)
				return(status);
		}
	}
	return(TRUE);
}

/* set a KEYTAB to the given name of the given type */

#if	PROTO
int setkey(KEYTAB *key, char *name)
#else
setkey(key, name)

KEYTAB *key;		/* ptr to key to set */
char *name;		/* name of function or buffer */
#endif
{
	int (PASCAL NEAR *ktemp)();	/* temp function pointer to assign */
	register BUFFER *kmacro;	/* ptr to buffer of macro to bind to key */
	char bufn[NBUFN];		/* buffer to hold macro name */

	/* are we unbinding it? */
	if (*name == 0) {
		key->k_type = BINDNUL;
		return(TRUE);
	}

	/* bind to a built in function? */
	if ((ktemp = fncmatch(name)) != NULL) {
		key->k_ptr.fp = ktemp;
		key->k_type = BINDFNC;
		return(TRUE);
	}

	/* is it a procedure/macro? */
	strcpy(bufn, "[");
	strcat(bufn, name);
	strcat(bufn, "]");
	if ((kmacro=bfind(bufn, FALSE, 0)) != NULL) {
		key->k_ptr.buf = kmacro;
		key->k_type = BINDBUF;
		return(TRUE);
	}

	/* not anything we can bind to */
	mlwrite(TEXT16);
/*		"[No such function]" */
	return(FALSE);
}
