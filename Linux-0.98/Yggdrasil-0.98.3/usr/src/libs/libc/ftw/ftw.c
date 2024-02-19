/*
**  FTW
**  Walk a directory hierarchy from a given point, calling a user-supplied
**  function at each thing we find.  If we go below a specified depth,
**  recycle file descriptors.
*/
#include <features.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef _POSIX_SOURCE
#include <dirent.h>
#include <limits.h>
#define direct dirent
#ifndef MAXNAMLEN
#define MAXNAMLEN	PATH_MAX
#endif
#else
#include <sys/dir.h>
#endif
#include <ftw.h>
#include <stdlib.h>
#include <string.h>

#ifdef	RCSID
static char RCS[] = "$Header: ftw.c,v 1.1 87/12/29 21:38:52 rsalz Exp $";
#endif	/* RCSID */

/* Handy shorthand. */
#define EQ(a, b)	(strcmp((a), (b)) == 0)

int
ftw(directory, funcptr, depth)
    const char	*directory;
    __ftw_fn_t	funcptr;
    int		depth;
{
    register DIR	 *Dp;
    register char	 *p;
    register int	  i;
    struct direct	 *E;
    struct stat		  Sb;
    long		  seekpoint;
    char		 *fullpath;

    /* If can't stat, tell the user so. */
    if (stat(directory, &Sb) < 0)
	return((*funcptr)(directory, &Sb, FTW_NS));

    /* If it's not a directory, call the user's function. */
    if ((Sb.st_mode & S_IFMT) != S_IFDIR)
	/* Saying "FTW_F" here is lying, what if this is a symlink? */
	return((*funcptr)(directory, &Sb, FTW_F));

    /* Open directory; and if we can't tell the user so. */
    if ((Dp = opendir(directory)) == NULL)
	return((*funcptr)(directory, &Sb, FTW_DNR));

    /* See if user wants to go further. */
    if (i = (*funcptr)(directory, &Sb, FTW_D)) {
	closedir(Dp);
	return(i);
    }

    /* Get ready to hold the full paths. */
    i = strlen(directory);
    if ((fullpath = malloc(i + 1 + MAXNAMLEN + 1)) == NULL) {
	closedir(Dp);
	return(-1);
    }
    (void)strcpy(fullpath, directory);
    p = &fullpath[i];
    if (i && p[-1] != '/')
	*p++ = '/';

    /* Read all entries in the directory.. */
    while (E = readdir(Dp))
	if (!EQ(E->d_name, ".") && !EQ(E->d_name, "..")) {
	    if (depth <= 1) {
		/* Going too deep; checkpoint and close this directory. */
		seekpoint = telldir(Dp);
		closedir(Dp);
		Dp = NULL;
	    }

	    /* Process the file. */
	    (void)strcpy(p, E->d_name);
	    if (i = ftw(fullpath, funcptr, depth - 1)) {
		/* User's finished; clean up. */
		free(fullpath);
		if (Dp)
		    closedir(Dp);
		return(i);
	    }

	    /* Reopen the directory if necessary. */
	    if (Dp == NULL) {
		if ((Dp = opendir(directory)) == NULL) {
		    /* WTF? */
		    free(fullpath);
		    return(-1);
		}
		seekdir(Dp, seekpoint);
	    }
	}

    /* Clean up. */
    free(fullpath);
    closedir(Dp);
    return(0);
}
