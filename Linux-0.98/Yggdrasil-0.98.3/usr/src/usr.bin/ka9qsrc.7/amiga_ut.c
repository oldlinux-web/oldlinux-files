/*  C K I U T L --  Utility functions for C-Kermit on the Amiga */
 
/*
 Author: Jack Rouse
 Contributed to Columbia University for inclusion in C-Kermit.
 Copyright (C) 1986, Jack J. Rouse, 106 Rubin Ct. Apt. A-4, Cary NC 27511
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained. 

 The file status routines assume all file protection modes are real, instead
 of just delete protection on files and write protection on disks.
*/

#include <stdio.h>
#include "exec/types.h"
#include "exec/exec.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"

LONG Execute();
LONG IoErr();
BPTR Lock();
VOID UnLock();   

/* portable library */
char *malloc();

/* Amiga Kermit externals (defined in ckitio.c) */

extern struct Process *MyProc;
extern struct CommandLineInterface *MyCLI;

/*
 * getwd -- get current working directory text
 */
char *getwd(buf, len)
register char *buf;
int len;
{
	register UBYTE *dirname;

	if (MyCLI == NULL) return(NULL);
	dirname = (UBYTE *)BADDR(MyCLI->cli_SetName);
	if (len < *dirname + 1) return(NULL);
	strncpy(buf, dirname + 1, *dirname);
	buf[*dirname] = 0;
	return(buf);
}

/*
 * system(cmd) -- execute a command
 *    provides no sensible return value
 */
system(cmd)
char *cmd;
{
	BPTR fh;

	fflush(stdout);
	Execute(cmd, (BPTR)NULL, (BPTR)NULL);
}
/*
 * update the current directory name in the CLI process structure
 *
 * This version generates the new name from the current name and
 * the specified path, something like the CD command does.
 * A much better version could be written using Parent(), Examine(),
 * and VolInfo to reconstitute a name from a lock, but Parent() doesn't
 * work with RAM: in V1.1.  The current implementation, like the CD
 * command, has difficulty with backing up the directory tree.  For example
 * { chdir("c:"); chdir("/"); } results in a dir name of "c:/".  However,
 * this version shouldn't get as far out of sync as the CD command.  (The
 * same sequence with the CD command puts you in the parent dir of "c:" but
 * with a directory name of "c:".)
 */
static void update_dirname(name)
register char *name;
{
	register UBYTE *dirname;		/* DOS directory name BSTR */
	char buf[100];				/* about same size as DOS */
	register char *tail;

	/* locate the DOS copy of the directory name */
	if (MyCLI == NULL) return;
	dirname = (UBYTE *)BADDR(MyCLI->cli_SetName);

	/* if the name is anchored (like "DF1:") simply replace the name */
	if (rindex(name, ':') != NULL)
	{
		*dirname = strlen(name);
		strncpy(&dirname[1], name, *dirname);
		return;
	}

	/* name is relative to current directory, copy name to work with */
	strncpy(buf, &dirname[1], *dirname);
	tail = &buf[*dirname];

	/* traverse the path in the name */
	while (*name)
	{
		/* go to parent dir? */
		if (*name == '/')
		{	/* remove a component from the directory path */

			/* advance past parent slash */
			++name;

			/* if at colon, can't back up */
			if (tail[-1] == ':')
			{
				*tail++ = '/';
				continue;
			}

			/* if at slash, see if name given */
			if (tail[-1] == '/')
			{
				/* if no name, can't back up */
				if (tail[-2] == '/' || tail[-2] == ':')
				{
					*tail++ = '/';
					continue;
				}

				/* remove trailing slash */
				--tail;
			}

			/* remove remainder of component */
			while (tail[-1] != '/' && tail[-1] != ':')
				--tail;
		}
		else
		{	/* add component to directory path */
			/* add slash if necessary to separate name */
			if (tail[-1] != ':' && tail[-1] != '/')
				*tail++ = '/';

			/* add component name */
			while (*name && *name != '/')
				*tail++ = *name++;

			/* add trailing slash if specified */
			if (*name == '/')
				*tail++ = *name++;
		}
	}

	/* set BSTR to derived name */
	*dirname = tail - buf;
	strncpy(&dirname[1], buf, *dirname);
}

/*
 * change current directory
 */
int chdir(name)
char *name;
{
	BPTR lock;
	BPTR oldlock;
	struct FileInfoBlock *fib;

	/* ignore chdir("") */
	if (*name == '\0') return(0);

	/* try to look at the object */ 
	lock = Lock(name, (LONG)ACCESS_READ);
	if (lock == NULL) return(-1);

	/* make sure the file is a directory */
	fib = (struct FileInfoBlock *)malloc(sizeof(*fib));
	if (fib == NULL || !Examine(lock, fib) || fib->fib_DirEntryType <= 0)
	{
		if (fib) free(fib);
		if ( lock >= 0 )
			UnLock(lock);
		return(-1);
	}

	/* don't need file info any more */
	free(fib);

	/* change the current directory */
	oldlock = CurrentDir(lock);

	/* update the DOS copy of the directory name */
	update_dirname(name);

	/* unlock the previous current directory */
	if (oldlock)
	{
		if ( oldlock >= 0)
			UnLock(oldlock);
	}

	return(0);
}
