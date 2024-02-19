/*	LOCK:	File locking command routines for MicroEMACS
		written by Daniel Lawrence
								*/

#include <stdio.h>
#include "estruct.h"
#include "eproto.h"
#include "edef.h"
#include "elang.h"

#if	FILOCK

#if	BSD || WMCS || SUN || XENIX || HPUX || AVIION || USG
#include <sys/errno.h>
extern int sys_nerr;		/* number of system error messages defined */
extern char *sys_errlist[];	/* list of message texts */
#endif

extern int errno;		/* current error */

char *lname[NLOCKS];	/* names of all locked files */
int numlocks;		/* # of current locks active */

/* lockchk:	check a file for locking and add it to the list */

lockchk(fname)

char *fname;	/* file to check for a lock */

{
	register int i;		/* loop indexes */
	register int status;	/* return status */

	/* check to see if that file is already locked here */
	if (numlocks > 0)
		for (i=0; i < numlocks; ++i)
			if (strcmp(fname, lname[i]) == 0)
				return(TRUE);

	/* if we have a full locking table, bitch and leave */
	if (numlocks == NLOCKS) {
		mlwrite(TEXT173);
/*                      "LOCK ERROR: Lock table full" */
		return(ABORT);
	}

	/* next, try to lock it */
	status = lock(fname);
	if (status == ABORT)	/* file is locked, no override */
		return(ABORT);
	if (status == FALSE)	/* locked, overriden, dont add to table */
		return(TRUE);

	/* we have now locked it, add it to our table */
	lname[++numlocks - 1] = (char *)malloc(strlen(fname) + 1);
	if (lname[numlocks - 1] == NULL) {	/* malloc failure */
		undolock(fname);		/* free the lock */
		mlwrite(TEXT174);
/*                      "Cannot lock, out of memory" */
		--numlocks;
		return(ABORT);
	}

	/* everthing is cool, add it to the table */
	strcpy(lname[numlocks-1], fname);
	return(TRUE);
}

/*	lockrel:	release all the file locks so others may edit */

lockrel()

{
	register int i;		/* loop index */
	register int status;	/* status of locks */
	register int s;		/* status of one unlock */

	status = TRUE;
	while (numlocks-- > 0) {
		if ((s = unlock(lname[numlocks])) != TRUE)
			status = s;
		free(lname[numlocks]);
	}
	return(status);
}

/* lock:	Check and lock a file from access by others
		returns	TRUE = files was not locked and now is
			FALSE = file was locked and overridden
			ABORT = file was locked, abort command
*/

lock(fname)

char *fname;	/* file name to lock */

{
	register char *locker;	/* lock error message */
	register int status;	/* return status */
	char msg[NSTRING];	/* message string */

	/* attempt to lock the file */
	locker = dolock(fname);
	if (locker == NULL)	/* we win */
		return(TRUE);

	/* file failed...abort */
	if (strncmp(locker, TEXT175, 4) == 0) {
/*                          "LOCK" */
		lckerror(locker);
		return(ABORT);
	}

	/* someone else has it....override? */
	strcpy(msg, TEXT176);
/*                  "File in use by " */
	strcat(msg, locker);
	strcat(msg, TEXT177);
/*                  ", overide?" */
	status = mlyesno(msg);		/* ask them */
	if (status == TRUE)
		return(FALSE);
	else
		return(ABORT);
}

/*	unlock:	Unlock a file
		this only warns the user if it fails
							*/

unlock(fname)

char *fname;	/* file to unlock */

{
	register char *locker;	/* undolock return string */

	/* unclock and return */
	locker = undolock(fname);
	if (locker == NULL)
		return(TRUE);

	/* report the error and come back */
	lckerror(locker);
	return(FALSE);
}

lckerror(errstr)	/* report a lock error */

char *errstr;		/* lock error string to print out */

{
	char obuf[NSTRING];	/* output buffer for error message */

	strcpy(obuf, errstr);
	strcat(obuf, " - ");
#if	BSD || WMCS || SUN || XENIX || HPUX || AVIION || USG
	if (errno < sys_nerr)
		strcat(obuf, sys_errlist[errno]);
	else
		strcat(obuf, TEXT178);
/*                           "[can not get system error message]" */
#else
	strcat(obuf, "Error # ");
	strcat(obuf, int_asc(errno));
#endif
	mlwrite(obuf);
}
#else
lckhello()	/* dummy function */
{
}
#endif
