/*
**	$Id: strdup.c,v 2.0.1.1 90/10/09 02:35:49 paul Exp $
**
**	Implements strdup(3c) [strdup(S) for you Xenix-types].
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
**	$Log:	strdup.c,v $
**	Revision 2.0.1.1  90/10/09  02:35:49  paul
**	patch1: Added missing #include for getty.h.
**	
**	Revision 2.0  90/09/19  20:18:12  paul
**	Initial 2.0 release
**	
*/

#include "getty.h"

#if defined(RCSID) && !defined(lint)
static char *RcsId =
"@(#)$Id: strdup.c,v 2.0.1.1 90/10/09 02:35:49 paul Exp $";
#endif

/*
**	strdup() - duplicates string s in memory.
**
**	Returns a pointer to the new string, or NULL if an error occurrs.
*/

char *
strdup(s)
register char *s;
{
	register char *p = (char *) NULL;

	if (s != (char *) NULL)
		if ((p = malloc((unsigned) (strlen(s)+1))) != (char *) NULL)
			(void) strcpy(p, s);

	return(p);
}


/* end of strdup.c */
