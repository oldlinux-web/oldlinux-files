/*
**	$Id: putenv.c,v 2.0 90/09/19 20:16:07 paul Rel $
**
**	Implements putenv(3c).
*/

/*
**	Copyright 1989,1990 by Paul Sutcliffe Jr.
**
**	Permission is hereby granted to copy, reproduce, redistribute,
**	or otherwise use this software as long as: there is no monetary
**	profit gained specifically from the use or reproduction or this
**	software, it is not sold, rented, traded or otherwise marketed,
**	and this copyright notice is included prominently in any copy
**	made.
**
**	The author make no claims as to the fitness or correctness of
**	this software for any use whatsoever, and it is provided as is. 
**	Any use of this software is at the user's own risk.
*/

/*
**	$Log:	putenv.c,v $
**	Revision 2.0  90/09/19  20:16:07  paul
**	Initial 2.0 release
**	
*/


#include "getty.h"

#if defined(RCSID) && !defined(lint)
static char *RcsId =
"@(#)$Id: putenv.c,v 2.0 90/09/19 20:16:07 paul Rel $";
#endif

#ifndef	MAXENV
#define	MAXENV	64	/* max # lines in envorinment */
#endif	/* MAXENV */


/*
**	putenv() - change or add value to environment
**
**	Returns non-zero if an error occurrs, zero otherwise.
*/

extern char **environ;

int
putenv(s)
char *s;
{
	register int i;
	register char *p;
	char *q, **envp, *env[MAXENV];
	boolean match = FALSE;

	if (s == (char *) NULL)
		return(FAIL);	/* can't add NULL to the environment */

	if ((p = malloc((unsigned) strlen(s)+1)) == (char *) NULL)
		return(FAIL);	/* malloc failed */

	(void) strcpy(p, s);
	if ((q = index(p, '=')) == (char *) NULL) {
		free(p);
		return(FAIL);	/* not in the form ``name=value'' */
	}

	*q = '\0';		/* split into two fields, name & value */

	/* copy the environ list, replacing `s' if a match is found
	 */
	for (i=0, envp=environ; *envp != (char *) NULL; i++, envp++) {
		if (strnequal(*envp, p, strlen(p))) {
			match = TRUE;
			env[i] = s;
		} else
			env[i] = *envp;
	}

	if (!match) {
		*q = '=';	/* put back the equal sign */
		env[i++] = p;	/* add p to env list */
	} else
		free(p);	/* not needed, s replaced old value */

	env[i++] = (char *) NULL;

	/* now dup env to make new environment
	 */
	if ((envp = (char **) malloc((unsigned) (i*sizeof(char *)))) ==
	    (char **) NULL) {
		return(FAIL);
	}
	environ = envp;		/* point to new area */
	for (i=0; env[i] != (char *) NULL; i++)
		*envp++ = env[i];
	*envp = (char *) NULL;

	return(SUCCESS);
}


/* end of putenv.c */
