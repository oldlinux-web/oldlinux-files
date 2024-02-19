/* @(#)getopt.c */

#include <stdio.h>
#include <string.h>
#include "getopt.h"

/*
 * get option letter from argument vector
 */
int
	optind = 1,		/* index into parent argv vector */
	optopt;			/* character checked for validity */
char	*optarg;		/* argument associated with option */

int
getopt(nargc, nargv, ostr)
int nargc;
char **nargv, *ostr;
{
	register char	*oli;		/* option letter list index */
	static char	*place = EMSG;	/* option letter processing */

	if(!*place) {			/* update scanning pointer */
		if(optind >= nargc || *(place = nargv[optind]) != '-' || !*++place) return(EOF);
		if (*place == '-') {	/* found "--" */
			++optind;
			return EOF;
		}
	}				/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' || !(oli = index(ostr,optopt))) {
		if(!*place) ++optind;
		tell(": illegal option -- ");
	}
	if (*++oli != ':') {		/* don't need argument */
		optarg = NULL;
		if (!*place)
			++optind;
	} else {				/* need an argument */
		if (*place) {			/* no white space */
			optarg = place;
		} else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			tell(": option requires an argument -- ");
		} else {
			optarg = nargv[optind];	/* white space */
		}
		place = EMSG;
		++optind;
	}
	return optopt;			/* dump back option letter */
}

int
getarg(nargc, nargv)
int nargc;
char **nargv;
{
	if (nargc <= optind) {
		optarg = (char *) 0;
		return EOF;
	} else {
		optarg = nargv[optind++];
		return 0;
	}
}
