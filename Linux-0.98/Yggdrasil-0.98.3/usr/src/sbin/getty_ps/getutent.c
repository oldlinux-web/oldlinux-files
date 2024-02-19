/*
**	$Id: getutent.c,v 2.0 90/09/19 20:00:51 paul Rel $
**
**	Implements getutent(3).
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
**	$Log:	getutent.c,v $
**	Revision 2.0  90/09/19  20:00:51  paul
**	Initial 2.0 release
**	
*/


#include "getty.h"

#if defined(RCSID) && !defined(lint)
static char *RcsId =
"@(#)$Id: getutent.c,v 2.0 90/09/19 20:00:51 paul Rel $";
#endif

typedef	struct utmp	UTMP;

static	char	*utmpfil = UTMP_FILE;	/* default utmp file */
static	FILE	*ufp = (FILE *) NULL;	/* file pointer to utmp file */
					/* NULL = no utmp file open  */
static	UTMP	ut;			/* buffer for utmp record */


/*
**	getutent() - get next valid utmp entry
**
**	Returns (UTMP*)NULL if no vaild entry found.
*/

UTMP *
getutent()
{
	if (ufp == (FILE *) NULL)
		if ((ufp = fopen(utmpfil, "r+")) == (FILE *) NULL)
			return((UTMP *) NULL);

	do {
		if (fread((char *)&ut, sizeof(ut), 1, ufp) != 1)
			return((UTMP *) NULL);

	} while (ut.ut_name[0] == '\0');	/* valid entry? */

	return(&ut);
}


/*
**	getutline() - get utmp entry that matches line.
**
**	Returns (UTMP*)NULL if no match found.
*/

UTMP *
getutline(line)
register UTMP *line;
{
	do {
		if (strequal(ut.ut_line, line->ut_line))
			return(&ut);	/* match! */

	} while (getutent() != NULL);

	return((UTMP *) NULL);
}


/*
**	setutent() - rewind utmp back to beginning
*/

void
setutent()
{
	if (ufp != (FILE *) NULL)
		rewind(ufp);
}
	

/*
**	endutent() - close utmp file
*/

void
endutent()
{
	if (ufp != (FILE *) NULL) {
		(void) fclose(ufp);
		ufp = (FILE *) NULL;
	}
}


/*
**	utmpname() - change utmp file name to "file"
*/

void
utmpname(file)
register char *file;
{
	endutent();
	utmpfil = strdup(file);
}


/* end of getutent.c */
