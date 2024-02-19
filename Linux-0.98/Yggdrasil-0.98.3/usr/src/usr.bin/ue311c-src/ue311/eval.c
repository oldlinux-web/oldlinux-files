/*	EVAL.C: Expresion evaluation functions for
		MicroEMACS

	written 1986 by Daniel Lawrence 			*/

#include	<stdio.h>
#include	"estruct.h"
#include	"eproto.h"
#include	"edef.h"
#include	"elang.h"
#include	"evar.h"

PASCAL NEAR varinit()	/* initialize the user variable list */

{
	register int i;

	for (i=0; i < MAXVARS; i++)
		uv[i].u_name[0] = 0;
}

PASCAL NEAR varclean()	/* initialize the user variable list */

{
	register int i;

	for (i=0; i < MAXVARS; i++)
		if (uv[i].u_name[0] != 0)
			free(uv[i].u_value);
}

char *PASCAL NEAR gtfun(fname)	/* evaluate a function */

char *fname;		/* name of function to evaluate */

{
	register int fnum;		/* index to function to eval */
	register int arg;		/* value of some arguments */
	char arg1[NSTRING];		/* value of first argument */
	char arg2[NSTRING];		/* value of second argument */
	char arg3[NSTRING];		/* value of third argument */
	static char result[2 * NSTRING];	/* string result */
#if	ENVFUNC
	char *getenv(); 		/* get environment string */
#endif

	/* look the function up in the function table */
	fname[3] = 0;	/* only first 3 chars significant */
	mklower(fname); /* and let it be upper or lower case */
	fnum = binary(fname, funval, NFUNCS);

	/* return errorm on a bad reference */
	if (fnum == -1)
		return(errorm);

	/* if needed, retrieve the first argument */
	if (funcs[fnum].f_type >= MONAMIC) {
		if (macarg(arg1) != TRUE)
			return(errorm);

		/* if needed, retrieve the second argument */
		if (funcs[fnum].f_type >= DYNAMIC) {
			if (macarg(arg2) != TRUE)
				return(errorm);

			/* if needed, retrieve the third argument */
			if (funcs[fnum].f_type >= TRINAMIC)
				if (macarg(arg3) != TRUE)
					return(errorm);
		}
	}


	/* and now evaluate it! */
	switch (fnum) {
		case UFADD:	return(int_asc(asc_int(arg1) + asc_int(arg2)));
		case UFSUB:	return(int_asc(asc_int(arg1) - asc_int(arg2)));
		case UFTIMES:	return(int_asc(asc_int(arg1) * asc_int(arg2)));
		case UFDIV:	return(int_asc(asc_int(arg1) / asc_int(arg2)));
		case UFMOD:	return(int_asc(asc_int(arg1) % asc_int(arg2)));
		case UFNEG:	return(int_asc(-asc_int(arg1)));
		case UFCAT:	strcpy(result, arg1);
				return(strcat(result, arg2));
		case UFLEFT:	return(bytecopy(result, arg1, asc_int(arg2)));
		case UFRIGHT:	arg = asc_int(arg2);
				if (arg > strlen(arg1))
					arg = strlen(arg1);
				return(strcpy(result,
					&arg1[strlen(arg1) - arg]));
		case UFMID:	arg = asc_int(arg2);
				if (arg > strlen(arg1))
					arg = strlen(arg1);
				return(bytecopy(result, &arg1[arg-1],
					asc_int(arg3)));
		case UFNOT:	return(ltos(stol(arg1) == FALSE));
		case UFEQUAL:	return(ltos(asc_int(arg1) == asc_int(arg2)));
		case UFLESS:	return(ltos(asc_int(arg1) < asc_int(arg2)));
		case UFGREATER: return(ltos(asc_int(arg1) > asc_int(arg2)));
		case UFGROUP:
				if ((arg = asc_int(arg1)) < 0 || arg >= MAXGROUPS)
					return(bytecopy(result, errorm, NSTRING * 2));
				    
#if	MAGIC
				return(bytecopy(result, fixnull(grpmatch[arg]),
					 NSTRING * 2));
#else
				if (arg == 0)
					bytecopy(result, patmatch, NSTRING * 2);
				else
					result[0] = '\0';
				return(result);
#endif
		case UFSEQUAL:	return(ltos(strcmp(arg1, arg2) == 0));
		case UFSLESS:	return(ltos(strcmp(arg1, arg2) < 0));
		case UFSGREAT:	return(ltos(strcmp(arg1, arg2) > 0));
		case UFIND:	return(strcpy(result, fixnull(getval(arg1))));
		case UFAND:	return(ltos(stol(arg1) && stol(arg2)));
		case UFOR:	return(ltos(stol(arg1) || stol(arg2)));
		case UFLENGTH:	return(int_asc(strlen(arg1)));
		case UFUPPER:	return(mkupper(arg1));
		case UFLOWER:	return(mklower(arg1));
		case UFTRUTH:	return(ltos(asc_int(arg1) == 42));
		case UFASCII:	return(int_asc((int)arg1[0]));
		case UFCHR:	result[0] = asc_int(arg1);
				result[1] = 0;
				return(result);
		case UFGTCMD:	cmdstr(getcmd(), result);
				return(result);
		case UFGTKEY:	result[0] = tgetc();
				result[1] = 0;
				return(result);
		case UFRND:	return(int_asc((ernd() % absv(asc_int(arg1))) + 1));
		case UFABS:	return(int_asc(absv(asc_int(arg1))));
		case UFSINDEX:	return(int_asc(sindex(arg1, arg2)));
		case UFENV:
#if	ENVFUNC
				return(fixnull(getenv(arg1)));
#else
				return("");
#endif
		case UFBIND:	return(transbind(arg1));
		case UFEXIST:	return(ltos(fexist(arg1)));
		case UFFIND:
				return(fixnull(flook(arg1, TRUE)));
		case UFBAND:	return(int_asc(asc_int(arg1) & asc_int(arg2)));
		case UFBOR:	return(int_asc(asc_int(arg1) | asc_int(arg2)));
		case UFBXOR:	return(int_asc(asc_int(arg1) ^ asc_int(arg2)));
		case UFBNOT:	return(int_asc(~asc_int(arg1)));
		case UFXLATE:	return(xlat(arg1, arg2, arg3));
		case UFTRIM:	return(trimstr(arg1));
		case UFSLOWER:	return(setlower(arg1, arg2), "");
		case UFSUPPER:	return(setupper(arg1, arg2), "");
 		case UFISNUM:	return(ltos(is_num(arg1)));
	}

	meexit(-11);	/* never should get here */
}

char *PASCAL NEAR gtusr(vname)	/* look up a user var's value */

char *vname;		/* name of user variable to fetch */

{
	register int vnum;	/* ordinal number of user var */
	register char *vptr;	/* temp pointer to function value */

	/* limit comparisons to significant length */
	if (strlen(vname) >= NVSIZE)	/* "%" counts, but is not passed */
		vname[NVSIZE-1] = '\0';

	/* scan the list looking for the user var name */
	for (vnum = 0; vnum < MAXVARS; vnum++) {
		if (uv[vnum].u_name[0] == 0)
			return(errorm);
		if (strcmp(vname, uv[vnum].u_name) == 0) {
			vptr = uv[vnum].u_value;
			if (vptr)
				return(vptr);
			else
				return(errorm);
		}
	}

	/* return errorm if we run off the end */
	return(errorm);
}

char *PASCAL NEAR funval(i)

int i;

{
	return(funcs[i].f_name);
}

char *PASCAL NEAR envval(i)

int i;

{
	return(envars[i]);
}

PASCAL NEAR binary(key, tval, tlength)

char *key;		/* key string to look for */
char *(PASCAL NEAR *tval)();	/* ptr to function to fetch table value with */
int tlength;		/* length of table to search */

{
	int l, u;	/* lower and upper limits of binary search */
	int i;		/* current search index */
	int cresult;	/* result of comparison */

	/* set current search limit as entire list */
	l = 0;
	u = tlength - 1;

	/* get the midpoint! */
	while (u >= l) {
		i = (l + u) >> 1;

		/* do the comparison */
		cresult = strcmp(key, (*tval)(i));
		if (cresult == 0)
			return(i);
		if (cresult < 0)
			u = i - 1;
		else
			l = i + 1;
	}
	return(-1);
}

char *PASCAL NEAR gtenv(vname)

char *vname;		/* name of environment variable to retrieve */

{
	register int vnum;	/* ordinal number of var refrenced */
	static char result[2 * NSTRING];	/* string result */

	/* scan the list, looking for the referenced name */
	vnum = binary(vname, envval, NEVARS);

	/* return errorm on a bad reference */
	if (vnum == -1)
		return(errorm);

	/* otherwise, fetch the appropriate value */
	switch (vnum) {
		case EVFILLCOL: return(int_asc(fillcol));
		case EVPAGELEN: return(int_asc(term.t_nrow + 1));
		case EVCURCOL:	return(int_asc(getccol(FALSE)));
		case EVCURLINE: return(int_asc(getlinenum(curbp, curwp->w_dotp)));
		case EVRAM:	return(int_asc((int)(envram / 1024l)));
		case EVFLICKER: return(ltos(flickcode));
		case EVCURWIDTH:return(int_asc(term.t_ncol));
		case EVCBFLAGS: return(int_asc(curbp->b_flag));
		case EVCBUFNAME:return(curbp->b_bname);
		case EVCFNAME:	return(curbp->b_fname);
		case EVSRES:	return(sres);
		case EVDEBUG:	return(ltos(macbug));
		case EVSTATUS:	return(ltos(cmdstatus));
		case EVPALETTE: return(palstr);
		case EVASAVE:	return(int_asc(gasave));
		case EVACOUNT:	return(int_asc(gacount));
		case EVLASTKEY: return(int_asc(lastkey));
		case EVCURCHAR:
			return(curwp->w_dotp->l_used ==
					curwp->w_doto ? int_asc('\r') :
				int_asc(lgetc(curwp->w_dotp, curwp->w_doto)));
		case EVDISCMD:	return(ltos(discmd));
		case EVVERSION: return(VERSION);
		case EVPROGNAME:return(PROGNAME);
		case EVLANG:	return(LANGUAGE);
		case EVSEED:	return(int_asc(seed));
		case EVDISINP:	return(ltos(disinp));
		case EVWLINE:	return(int_asc(curwp->w_ntrows));
		case EVCWLINE:	return(int_asc(getwpos()));
		case EVTARGET:	saveflag = lastflag;
				return(int_asc(curgoal));
		case EVSEARCH:	return(pat);
		case EVTIME:	return(timeset());
		case EVREPLACE: return(rpat);
		case EVMATCH:	return(fixnull(patmatch));
		case EVKILL:	return(getkill());
		case EVREGION:	return(getreg(result));
		case EVCMODE:	return(int_asc(curbp->b_mode));
		case EVGMODE:	return(int_asc(gmode));
		case EVTPAUSE:	return(int_asc(term.t_pause));
		case EVPENDING:
#if	TYPEAH
				return(ltos(typahead()));
#else
				return(falsem);
#endif
		case EVLWIDTH:	return(int_asc(llength(curwp->w_dotp)));
		case EVLINE:	return(getctext());
		case EVGFLAGS:	return(int_asc(gflags));
		case EVRVAL:	return(int_asc(rval));
		case EVREADHK:	return(fixnull(getfname(&readhook)));
		case EVWRAPHK:	return(fixnull(getfname(&wraphook)));
		case EVCMDHK:	return(fixnull(getfname(&cmdhook)));
		case EVXPOS:	return(int_asc(xpos));
		case EVYPOS:	return(int_asc(ypos));
		case EVSTERM:	cmdstr(sterm, result);
				return(result);
		case EVMODEFLAG:return(ltos(modeflag));
		case EVSSCROLL: return(ltos(sscroll));
		case EVLASTMESG:return(lastmesg);
		case EVHARDTAB: return(int_asc(tabsize));
		case EVSOFTTAB: return(int_asc(stabsize));
		case EVSSAVE:	return(ltos(ssave));
		case EVFCOL:	return(int_asc(curwp->w_fcol));
		case EVHSCROLL: return(ltos(hscroll));
		case EVHJUMP:	return(int_asc(hjump));
		case EVBUFHOOK: return(fixnull(getfname(&bufhook)));
		case EVEXBHOOK: return(fixnull(getfname(&exbhook)));
		case EVWRITEHK: return(fixnull(getfname(&writehook)));
		case EVDIAGFLAG:return(ltos(diagflag));
		case EVMSFLAG:	return(ltos(mouseflag));
		case EVOCRYPT:	return(ltos(oldcrypt));
		case EVSEARCHPNT:	return(int_asc(searchtype));
		case EVDISPHIGH:return(ltos(disphigh));
		case EVLTERM:	return(lterm);
		case EVPARALEAD:return(paralead);
		case EVFMTLEAD:	return(fmtlead);
		case EVWCHARS:	return(getwlist(result));
		case EVPOPFLAG: return(ltos(popflag));
		case EVYANKFLAG:	return(ltos(yankflag));
		case EVPOSFLAG: return(ltos(posflag));
		case EVSCRNAME:	return(first_screen->s_screen_name);
		case EVCURWIND: return(int_asc(getcwnum()));
		case EVNUMWIND: return(int_asc(gettwnum()));
		case EVORGCOL:	return(int_asc(term.t_colorg));
		case EVORGROW:	return(int_asc(term.t_roworg));
		case EVDESKCLR:	return(cname[deskcolor]);
		case EVTIMEFLAG: return(ltos(timeflag));
	}
	meexit(-12);	/* again, we should never get here */
}

char *PASCAL NEAR fixnull(s)	/* Don't return NULL pointers! */

char *s;

{
	if (s == NULL)
		return("");
	else
		return(s);
}

/* return some of the contents of the kill buffer */

char *PASCAL NEAR getkill()

{
	register int size;	/* max number of chars left to return */
	register char *sp;	/* ptr into KILL block data chunk */
	register char *vp;	/* ptr into return value */
	KILL *kptr;		/* ptr to the current KILL block */
	int counter;		/* index into data chunk */
	static char value[NSTRING];	/* temp buffer for value */

	/* no kill buffer....just a null string */
	if (kbufh[kill_index] == (KILL *)NULL) {
		value[0] = 0;
		return(value);
	}

	/* set up the output buffer */
	vp = value;
	size = NSTRING - 1;

	/* backed up characters? */
	if (kskip[kill_index] > 0) {
		kptr = kbufh[kill_index];
		sp = &(kptr->d_chunk[kskip[kill_index]]);
		counter = kskip[kill_index];
		while (counter++ < KBLOCK) {
			*vp++ = *sp++;
			if (--size == 0) {
				*vp = 0;
				return(value);
			}
		}
		kptr = kptr->d_next;
	} else {
		kptr = kbufh[kill_index];
	}

	if (kptr != (KILL *)NULL) {
		while (kptr != kbufp[kill_index]) {
			sp = kptr->d_chunk;
			for (counter = 0; counter < KBLOCK; counter++) {
				*vp++ = *sp++;
				if (--size == 0) {
					*vp = 0;
					return(value);
				}
			}
			kptr = kptr->d_next;
		}
		counter = kused[kill_index];
		sp = kptr->d_chunk;
		while (counter--) {
			*vp++ = *sp++;
			if (--size == 0) {
				*vp = 0;
				return(value);
			}
		}
	}
	
	/* and return the constructed value */
	*vp = 0;
	return(value);
}

char *PASCAL NEAR trimstr(s)	/* trim whitespace off the end of a string */

char *s;	/* string to trim */

{
	char *sp;	/* backward index */

	sp = s + strlen(s) - 1;
	while ((sp >= s) && (*sp == ' ' || *sp == '\t'))
		--sp;
	*(sp+1) = 0;
	return(s);
}

int PASCAL NEAR setvar(f, n)		/* set a variable */

int f;		/* default flag */
int n;		/* numeric arg (can overide prompted value) */

{
	register int status;	/* status return */
	VDESC vd;		/* variable num/type */
	char var[NVSIZE+1];	/* name of variable to fetch */
	char value[NSTRING];	/* value to set variable to */

	/* first get the variable to set.. */
	if (clexec == FALSE) {
		status = mlreply(TEXT51, &var[0], NVSIZE+1);
/*				 "Variable to set: " */
		if (status != TRUE)
			return(status);
	} else {	/* macro line argument */
		/* grab token and skip it */
		execstr = token(execstr, var, NVSIZE + 1);
	}

	/* check the legality and find the var */
	findvar(var, &vd, NVSIZE + 1);
        
	/* if its not legal....bitch */
	if (vd.v_type == -1) {
		mlwrite(TEXT52, var);
/*			"%%No such variable as '%s'" */
		return(FALSE);
	}

	/* get the value for that variable */
	if (f == TRUE)
		strcpy(value, int_asc(n));
	else {
		status = mlreply(TEXT53, &value[0], NSTRING);
/*				 "Value: " */
		if (status == ABORT)
			return(status);
	}

	/* and set the appropriate value */
	status = svar(&vd, value);

#if	DEBUGM
	/* if $debug == TRUE, every assignment will echo a statment to
	   that effect here. */
        
	if (macbug && (strcmp(var, "%track") != 0)) {
		strcpy(outline, "(((");

		strcat(outline, var);
		strcat(outline, " <- ");

		/* and lastly the value we tried to assign */
		strcat(outline, value);
		strcat(outline, ")))");

		/* expand '%' to "%%" so mlwrite wont bitch */
		makelit(outline);

		/* write out the debug line */
		mlforce(outline);
		update(TRUE);

		/* and get the keystroke to hold the output */
		if (getkey() == abortc) {
			mlforce(TEXT54);
/*				"[Macro aborted]" */
			status = FALSE;
		}
	}
#endif

	/* and return it */
	return(status);
}

PASCAL NEAR findvar(var, vd, size)	/* find a variables type and name */

char *var;	/* name of var to get */
VDESC *vd;	/* structure to hold type and ptr */
int size;	/* size of var array */

{
	register int vnum;	/* subscript in varable arrays */
	register int vtype;	/* type to return */

fvar:	vtype = -1;
	switch (var[0]) {

		case '$': /* check for legal enviromnent var */
			for (vnum = 0; vnum < NEVARS; vnum++)
				if (strcmp(&var[1], envars[vnum]) == 0) {
					vtype = TKENV;
					break;
				}
			break;

		case '%': /* check for existing legal user variable */
			for (vnum = 0; vnum < MAXVARS; vnum++)
				if (strcmp(&var[1], uv[vnum].u_name) == 0) {
					vtype = TKVAR;
					break;
				}
			if (vnum < MAXVARS)
				break;

			/* create a new one??? */
			for (vnum = 0; vnum < MAXVARS; vnum++)
				if (uv[vnum].u_name[0] == 0) {
					vtype = TKVAR;
					strcpy(uv[vnum].u_name, &var[1]);
					uv[vnum].u_value = NULL;
					break;
				}
			break;

		case '&':	/* indirect operator? */
			var[4] = 0;
			if (strcmp(&var[1], "ind") == 0) {
				/* grab token, and eval it */
				execstr = token(execstr, var, size);
				strcpy(var, fixnull(getval(var)));
				goto fvar;
			}
	}

	/* return the results */
	vd->v_num = vnum;
	vd->v_type = vtype;
	return;
}

int PASCAL NEAR svar(var, value)	/* set a variable */

VDESC *var;	/* variable to set */
char *value;	/* value to set to */

{
	register int vnum;	/* ordinal number of var refrenced */
	register int vtype;	/* type of variable to set */
	register int status;	/* status return */
	register int c; 	/* translated character */
	register char *sp;	/* scratch string pointer */

	/* simplify the vd structure (we are gonna look at it a lot) */
	vnum = var->v_num;
	vtype = var->v_type;

	/* and set the appropriate value */
	status = TRUE;
	switch (vtype) {
	case TKVAR: /* set a user variable */
		if (uv[vnum].u_value != NULL)
			free(uv[vnum].u_value);
		sp = malloc(strlen(value) + 1);
		if (sp == NULL)
			return(FALSE);
		strcpy(sp, value);
		uv[vnum].u_value = sp;
		break;

	case TKENV: /* set an environment variable */
		status = TRUE;	/* by default */
		switch (vnum) {
		case EVFILLCOL: fillcol = asc_int(value);
				break;
		case EVPAGELEN: status = newsize(TRUE, asc_int(value));
				break;
		case EVCURCOL:	status = setccol(asc_int(value));
				break;
		case EVCURLINE: status = gotoline(TRUE, asc_int(value));
				break;
		case EVRAM:	break;
		case EVFLICKER: flickcode = stol(value);
				break;
		case EVCURWIDTH:status = newwidth(TRUE, asc_int(value));
				break;
		case EVCBFLAGS: curbp->b_flag = (curbp->b_flag & ~(BFCHG|BFINVS))
					| (asc_int(value) & (BFCHG|BFINVS));
				lchange(WFMODE);
				break;
		case EVCBUFNAME:strcpy(curbp->b_bname, value);
				curwp->w_flag |= WFMODE;
				break;
		case EVCFNAME:	strcpy(curbp->b_fname, value);
				curwp->w_flag |= WFMODE;
				break;
		case EVSRES:	status = TTrez(value);
				break;
		case EVDEBUG:	macbug = stol(value);
				break;
		case EVSTATUS:	cmdstatus = stol(value);
				break;
		case EVPALETTE: bytecopy(palstr, value, 48);
				spal(palstr);
				break;
		case EVASAVE:	gasave = asc_int(value);
				break;
		case EVACOUNT:	gacount = asc_int(value);
				break;
		case EVLASTKEY: lastkey = asc_int(value);
				break;
		case EVCURCHAR: ldelete(1L, FALSE);	/* delete 1 char */
				c = asc_int(value);
				if (c == '\r')
					lnewline();
				else
					linsert(1, (char)c);
				backchar(FALSE, 1);
				break;
		case EVDISCMD:	discmd = stol(value);
				break;
		case EVVERSION: break;
		case EVPROGNAME:break;
		case EVLANG:	break;
		case EVSEED:	seed = asc_int(value);
				break;
		case EVDISINP:	disinp = stol(value);
				break;
		case EVWLINE:	status = resize(TRUE, asc_int(value));
				break;
		case EVCWLINE:	status = forwline(TRUE,
						asc_int(value) - getwpos());
				break;
		case EVTARGET:	curgoal = asc_int(value);
				thisflag = saveflag;
				break;
		case EVSEARCH:	strcpy(pat, value);
				setjtable(); /* Set up fast search arrays  */
#if	MAGIC
				mcclear();
#endif
				break;
		case EVTIME:	break;
		case EVREPLACE: strcpy(rpat, value);
#if	MAGIC
				rmcclear();
#endif 
				break;
		case EVMATCH:	break;
		case EVKILL:	break;
		case EVREGION:	break;
		case EVCMODE:	curbp->b_mode = asc_int(value);
				curwp->w_flag |= WFMODE;
				break;
		case EVGMODE:	gmode = asc_int(value);
				break;
		case EVTPAUSE:	term.t_pause = asc_int(value);
				break;
		case EVPENDING: break;
		case EVLWIDTH:	break;
		case EVLINE:	putctext(value);
				break;
		case EVGFLAGS:	gflags = asc_int(value);
				break;
		case EVRVAL:	break;
		case EVREADHK:	setkey(&readhook, value);
				break;
		case EVWRAPHK:	setkey(&wraphook, value);
				break;
		case EVCMDHK:	setkey(&cmdhook, value);
				break;
		case EVXPOS:	xpos = asc_int(value);
				break;
		case EVYPOS:	ypos = asc_int(value);
				break;
		case EVSTERM:	sterm = stock(value);
				break;
		case EVMODEFLAG:modeflag = stol(value);
				upwind();
				break;
		case EVSSCROLL: sscroll = stol(value);
				break;
		case EVLASTMESG:strcpy(lastmesg, value);
				break;
		case EVHARDTAB: tabsize = asc_int(value);
				upwind();
				break;
		case EVSOFTTAB: stabsize = asc_int(value);
				upwind();
				break;
		case EVSSAVE:	ssave = stol(value);
				break;
		case EVFCOL:	curwp->w_fcol = asc_int(value);
				if (curwp->w_fcol < 0)
					curwp->w_fcol = 0;
				curwp->w_flag |= WFHARD | WFMODE;
				break;
		case EVHSCROLL: hscroll = stol(value);
				lbound = 0;
				break;
		case EVHJUMP:	hjump = asc_int(value);
				if (hjump < 1)
					hjump = 1;
				if (hjump > term.t_ncol - 1)
					hjump = term.t_ncol - 1;
				break;
		case EVBUFHOOK: setkey(&bufhook, value);
				break;
		case EVEXBHOOK: setkey(&exbhook, value);
				break;
		case EVWRITEHK: setkey(&writehook, value);
				break;
		case EVDIAGFLAG:diagflag = stol(value);
				break;
		case EVMSFLAG:	mouseflag = stol(value);
				break;
		case EVOCRYPT:	oldcrypt = stol(value);
				break;
		case EVSEARCHPNT:	searchtype = asc_int(value);
				if (searchtype < SRNORM  || searchtype > SREND)
					searchtype = SRNORM;
				break;
		case EVDISPHIGH:
				c = disphigh;
				disphigh = stol(value);
				if (c != disphigh)
					upwind();
				break;
		case EVLTERM:	bytecopy(lterm, value, NSTRING);
				break;
		case EVPARALEAD:bytecopy(paralead, value, NSTRING);
				break;
		case EVFMTLEAD:	bytecopy(fmtlead, value, NSTRING);
				break;
		case EVWCHARS:	setwlist(value);
				break;
		case EVPOPFLAG: popflag = stol(value);
				break;
		case EVYANKFLAG:	yankflag = stol(value);
		case EVPOSFLAG: posflag = stol(value);
				upmode();
				break;
				break;
		case EVSCRNAME:	select_screen(lookup_screen(value), TRUE);
				break;
		case EVCURWIND:	nextwind(TRUE, asc_int(value));
				break;
		case EVNUMWIND:	break;
		case EVORGCOL:	status = new_col_org(TRUE, asc_int(value));
				break;
		case EVORGROW:	status = new_row_org(TRUE, asc_int(value));
				break;
		case EVDESKCLR:	c = lookup_color(mkupper(value));
				if (c != -1) {
					deskcolor = c;
#if	WINDOW_TEXT
					refresh_screen(first_screen);
#endif
				}
				break;
		}
		case EVTIMEFLAG: timeflag = stol(value);
				upmode();
				break;
		break;
	}
	return(status);
}

/*	asc_int:	ascii string to integer......This is too
		inconsistant to use the system's	*/

int PASCAL NEAR asc_int(st)

char *st;

{
	int result;	/* resulting number */
	int sign;	/* sign of resulting number */
	char c; 	/* current char being examined */

	result = 0;
	sign = 1;

	/* skip preceding whitespace */
	while (*st == ' ' || *st == '\t')
		++st;

	/* check for sign */
	if (*st == '-') {
		sign = -1;
		++st;
	}
	if (*st == '+')
		++st;

	/* scan digits, build value */
	while ((c = *st++))
		if (c >= '0' && c <= '9')
			result = result * 10 + c - '0';
		else
			break;

	return(result * sign);
}

/*	int_asc:	integer to ascii string.......... This is too
			inconsistant to use the system's	*/

char *PASCAL NEAR int_asc(i)

int i;	/* integer to translate to a string */

{
	register int digit;		/* current digit being used */
	register char *sp;		/* pointer into result */
	register int sign;		/* sign of resulting number */
	static char result[INTWIDTH+1]; /* resulting string */

	/* record the sign...*/
	sign = 1;
	if (i < 0) {
		sign = -1;
		i = -i;
	}

	/* and build the string (backwards!) */
	sp = result + INTWIDTH;
	*sp = 0;
	do {
		digit = i % 10;
		*(--sp) = '0' + digit;	/* and install the new digit */
		i = i / 10;
	} while (i);

	/* and fix the sign */
	if (sign == -1) {
		*(--sp) = '-';	/* and install the minus sign */
	}

	return(sp);
}

int PASCAL NEAR gettyp(token)	/* find the type of a passed token */

char *token;	/* token to analyze */

{
	register char c;	/* first char in token */

	/* grab the first char (this is all we need) */
	c = *token;

	/* no blanks!!! */
	if (c == 0)
		return(TKNUL);

	/* a numeric literal? */
	if (c >= '0' && c <= '9')
		return(TKLIT);

	switch (c) {
		case '"':	return(TKSTR);

		case '!':	return(TKDIR);
		case '@':	return(TKARG);
		case '#':	return(TKBUF);
		case '$':	return(TKENV);
		case '%':	return(TKVAR);
		case '&':	return(TKFUN);
		case '*':	return(TKLBL);

		default:	return(TKCMD);
	}
}

char *PASCAL NEAR getval(token) /* find the value of a token */

char *token;		/* token to evaluate */

{
	register int status;	/* error return */
	register BUFFER *bp;	/* temp buffer pointer */
	register int blen;	/* length of buffer argument */
	register int distmp;	/* temporary discmd flag */
	static char buf[NSTRING];/* string buffer for some returns */

	switch (gettyp(token)) {
		case TKNUL:	return("");

		case TKARG:	/* interactive argument */
				strcpy(token, fixnull(getval(&token[1])));
				distmp = discmd;	/* echo it always! */
				discmd = TRUE;
				status = getstring(token,
					   buf, NSTRING, ctoec('\r'));
				discmd = distmp;
				if (status == ABORT)
					return(NULL);
				return(buf);

		case TKBUF:	/* buffer contents fetch */

				/* grab the right buffer */
				strcpy(token, fixnull(getval(&token[1])));
				bp = bfind(token, FALSE, 0);
				if (bp == NULL)
					return(NULL);
	        
				/* if the buffer is displayed, get the window
				   vars instead of the buffer vars */
				if (bp->b_nwnd > 0) {
					curbp->b_dotp = curwp->w_dotp;
					curbp->b_doto = curwp->w_doto;
				}

				/* make sure we are not at the end */
				if (bp->b_linep == bp->b_dotp)
					return(NULL);
	        
				/* grab the line as an argument */
				blen = bp->b_dotp->l_used - bp->b_doto;
				if (blen > NSTRING)
					blen = NSTRING;
				bytecopy(buf, bp->b_dotp->l_text + bp->b_doto,
					blen);
				buf[blen] = 0;
	        
				/* and step the buffer's line ptr ahead a line */
				bp->b_dotp = bp->b_dotp->l_fp;
				bp->b_doto = 0;

				/* if displayed buffer, reset window ptr vars*/
				if (bp->b_nwnd > 0) {
					curwp->w_dotp = curbp->b_dotp;
					curwp->w_doto = 0;
					curwp->w_flag |= WFMOVE;
				}

				/* and return the spoils */
				return(buf);	        

		case TKVAR:	return(gtusr(token+1));
		case TKENV:	return(gtenv(token+1));
		case TKFUN:	return(gtfun(token+1));
		case TKDIR:	return(NULL);
		case TKLBL:	return(NULL);
		case TKLIT:	return(token);
		case TKSTR:	return(token+1);
		case TKCMD:	return(token);
	}
}

int PASCAL NEAR stol(val)	/* convert a string to a numeric logical */

char *val;	/* value to check for stol */

{
	/* check for logical values */
	if (val[0] == 'F')
		return(FALSE);
	if (val[0] == 'T')
		return(TRUE);

	/* check for numeric truth (!= 0) */
	return((asc_int(val) != 0));
}

char *PASCAL NEAR ltos(val)	/* numeric logical to string logical */

int val;	/* value to translate */

{
	if (val)
		return(truem);
	else
		return(falsem);
}

char *PASCAL NEAR mkupper(str)	/* make a string upper case */

char *str;		/* string to upper case */

{
	char *sp;

	sp = str;
	while (*sp)
		uppercase(sp++);
	return(str);
}

char *PASCAL NEAR mklower(str)	/* make a string lower case */

char *str;		/* string to lower case */

{
	char *sp;

	sp = str;
	while (*sp)
		lowercase(sp++);
	return(str);
}

int PASCAL NEAR absv(x) /* take the absolute value of an integer */

int x;

{
	return(x < 0 ? -x : x);
}

int PASCAL NEAR ernd()	/* returns a random integer */

{
	seed = absv(seed * 1721 + 10007);
	return(seed);
}

int PASCAL NEAR sindex(source, pattern) /* find pattern within source */

char *source;	/* source string to search */
char *pattern;	/* string to look for */

{
	char *sp;	/* ptr to current position to scan */
	char *csp;	/* ptr to source string during comparison */
	char *cp;	/* ptr to place to check for equality */

	/* scanning through the source string */
	sp = source;
	while (*sp) {
		/* scan through the pattern */
		cp = pattern;
		csp = sp;
		while (*cp) {
			if (!eq(*cp, *csp))
				break;
			++cp;
			++csp;
		}

		/* was it a match? */
		if (*cp == 0)
			return((int)(sp - source) + 1);
		++sp;
	}

	/* no match at all.. */
	return(0);
}

/*	Filter a string through a translation table	*/

char *PASCAL NEAR xlat(source, lookup, trans)

char *source;	/* string to filter */
char *lookup;	/* characters to translate */
char *trans;	/* resulting translated characters */

{
	register char *sp;	/* pointer into source table */
	register char *lp;	/* pointer into lookup table */
	register char *rp;	/* pointer into result */
	static char result[NSTRING];	/* temporary result */

	/* scan source string */
	sp = source;
	rp = result;
	while (*sp) {
		/* scan lookup table for a match */
		lp = lookup;
		while (*lp) {
			if (*sp == *lp) {
				*rp++ = trans[lp - lookup];
				goto xnext;
			}
			++lp;
		}

		/* no match, copy in the source char untranslated */
		*rp++ = *sp;

xnext:		++sp;
	}

	/* terminate and return the result */
	*rp = 0;
	return(result);
}

/*	setwlist:	Set an alternative list of character to be
			considered "in a word */

PASCAL NEAR setwlist(wclist)

char *wclist;	/* list of characters to consider "in a word" */

{
	register int index;

	/* if we are turning this facility off, just flag so */
	if (wclist == NULL || *wclist == 0) {
		wlflag = FALSE;
		return;
	}

	/* first clear the table */
	for (index = 0; index < 256; index++)
		wordlist[index] = FALSE;

	/* and for each character in the new value, set that element
	   of the word character list */
	while (*wclist)
		wordlist[*wclist++] = TRUE;
	wlflag = TRUE;
	return;
}

/*	getwlist:	place in a buffer a list of characters
			considered "in a word"			*/

char *PASCAL NEAR getwlist(buf)

char *buf;	/* buffer to place list of characters */

{
	register int index;
	register char *sp;

	/* if we are defaulting to a standard word char list... */
	if (wlflag == FALSE)
		return("");

	/* build the string of characters in the return buffer */
	sp = buf;
	for (index = 0; index < 256; index++)
		if (wordlist[index])
			*sp++ = index;
	*sp = 0;
	return(buf);
}

/*	is_num:	ascii string is integer......This is too
		inconsistant to use the system's	*/
 
int PASCAL NEAR is_num(st)
 
char *st;
 
{
	int period_flag;	/* have we seen a period yet? */

	/* skip preceding whitespace */
	while (*st == ' ' || *st == '\t')
		++st;
 
	/* check for sign */
	if ((*st == '-') || (*st == '+'))
		++st;
 
	/* scan digits */
	period_flag = FALSE;
	while ((*st >= '0') && (*st <= '9') ||
	       (*st == '.' && period_flag == FALSE)) {
		if (*st == '.')
			period_flag = TRUE;
		st++;
	}
 
	/* scan rest of line for just white space */
	while (*st) {
		if ((*st != '\t') && (*st != ' '))
			return(FALSE);
		st++;
	}
	return(TRUE);
}

#if	DEBUGM
int PASCAL NEAR dispvar(f, n)		/* display a variable's value */

int f;		/* default flag */
int n;		/* numeric arg (can overide prompted value) */

{
	register int status;	/* status return */
	VDESC vd;		/* variable num/type */
	char var[NVSIZE+1];	/* name of variable to fetch */

	/* first get the variable to display.. */
	if (clexec == FALSE) {
		status = mlreply(TEXT55, &var[0], NVSIZE+1);
/*				 "Variable to display: " */
		if (status != TRUE)
			return(status);
	} else {	/* macro line argument */
		/* grab token and skip it */
		execstr = token(execstr, var, NVSIZE + 1);
	}

	/* check the legality and find the var */
	findvar(var, &vd, NVSIZE + 1);
        
	/* if its not legal....bitch */
	if (vd.v_type == -1) {
		mlwrite(TEXT52, var);
/*			"%%No such variable as '%s'" */
		return(FALSE);
	}

	/* and display the value */
	strcpy(outline, var);
	strcat(outline, " = ");

	/* and lastly the current value */
	strcat(outline, fixnull(getval(var)));

	/* expand '%' to "%%" so mlwrite wont bitch */
	makelit(outline);

	/* write out the result */
	mlforce(outline);
	update(TRUE);

	/* and return */
	return(TRUE);
}

/*	describe-variables	Bring up a fake buffer and list the contents
				of all the environment variables
*/

PASCAL NEAR desvars(f, n)

int f,n;	/* prefix flag and argument */

{
	register WINDOW *wp;	/* scanning pointer to windows */
	register BUFFER *varbuf;/* buffer to put variable list into */
	register int uindex;	/* index into uvar table */
	char outseq[256];	/* output buffer for keystroke sequence */

	/* and get a buffer for it */
	varbuf = bfind(TEXT56, TRUE, BFINVS);
/*		   "Variable list" */
	if (varbuf == NULL || bclear(varbuf) == FALSE) {
		mlwrite(TEXT57);
/*			"Can not display variable list" */
		return(FALSE);
	}

	/* let us know this is in progress */
	mlwrite(TEXT58);
/*		"[Building variable list]" */

	/* build the environment variable list */
	for (uindex = 0; uindex < NEVARS; uindex++) {

		/* add in the environment variable name */
		strcpy(outseq, "$");
		strcat(outseq, envars[uindex]);
		pad(outseq, 14);
	        
		/* add in the value */
		strcat(outseq, gtenv(envars[uindex]));

		/* and add it as a line into the buffer */
		if (addline(varbuf, outseq) != TRUE)
			return(FALSE);
	}

	if (addline(varbuf, "") != TRUE)
		return(FALSE);

	/* build the user variable list */
	for (uindex = 0; uindex < MAXVARS; uindex++) {
		if (uv[uindex].u_name[0] == 0)
			break;

		/* add in the user variable name */
		strcpy(outseq, "%");
		strcat(outseq, uv[uindex].u_name);
		pad(outseq, 14);
	        
		/* add in the value */
		strcat(outseq, uv[uindex].u_value);

		/* and add it as a line into the buffer */
		if (addline(varbuf, outseq) != TRUE)
			return(FALSE);
	}

	/* display the list */
	wpopup(varbuf);
	mlerase();	/* clear the mode line */
	return(TRUE);
}

/*	describe-functions	Bring up a fake buffer and list the
				names of all the functions
*/

PASCAL NEAR desfunc(f, n)

int f,n;	/* prefix flag and argument */

{
	register WINDOW *wp;	/* scanning pointer to windows */
	register BUFFER *fncbuf;/* buffer to put function list into */
	register int uindex;	/* index into funcs table */
	char outseq[80];	/* output buffer for keystroke sequence */

	/* get a buffer for the function list */
	fncbuf = bfind(TEXT211, TRUE, BFINVS);
/*		   "Function list" */
	if (fncbuf == NULL || bclear(fncbuf) == FALSE) {
		mlwrite(TEXT212);
/*			"Can not display function list" */
		return(FALSE);
	}

	/* let us know this is in progress */
	mlwrite(TEXT213);
/*		"[Building function list]" */

	/* build the function list */
	for (uindex = 0; uindex < NFUNCS; uindex++) {

		/* add in the environment variable name */
		strcpy(outseq, "&");
		strcat(outseq, funcs[uindex].f_name);

		/* and add it as a line into the buffer */
		if (addline(fncbuf, outseq) != TRUE)
			return(FALSE);
	}

	if (addline(fncbuf, "") != TRUE)
		return(FALSE);

	/* display the list */
	wpopup(fncbuf);
	mlerase();	/* clear the mode line */
	return(TRUE);
}

pad(s, len)	/* pad a string to indicated length */

char *s;	/* string to add spaces to */
int len;	/* wanted length of string */

{
	while (strlen(s) < len) {
                strcat(s, "          ");
		s[len] = 0;
	}
}
#endif
