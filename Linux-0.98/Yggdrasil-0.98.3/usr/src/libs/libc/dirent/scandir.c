/*
**  SCANDIR
**  Scan a directory, collecting all (selected) items into a an array.
*/

#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#ifdef	RCSID
static char RCS[] = "$Header: /proj/wais/wais-8-b2/lib/RCS/scandir.c,v 1.1 91/07/17 16:38:01 jonathan Exp $";
#endif	/* RCSID */

/* Initial guess at directory size. */
#define INITIAL_SIZE	20
#define USE_DIRENT

int
scandir(name, list, selector, sorter)
    const char		*name;
    struct dirent	***list;
    __dir_select_fn_t	selector;
    __dir_compar_fn_t	sorter;
{
    register struct dirent	  **names;
    register struct dirent	  *entp;
    register DIR	  *dirp;
    register int	   i;
    register int	   size;

    /* Get initial list space and open directory. */
    size = INITIAL_SIZE;
    names = (struct dirent **)malloc(size * sizeof names[0]);
    if (names == NULL)
	return -1;
    dirp = opendir(name);
    if (dirp == NULL)
	return -1;

    /* Read entries in the directory. */
    for (i = 0; entp = readdir(dirp); )
	if (selector == NULL || (*selector)(entp)) {
	    /* User wants them all, or he wants this one. */
	    if (++i >= size) {
		size <<= 1;
		names = (struct dirent **)
		    realloc(names, size * sizeof names[0]);
		if (names == NULL) {
		    closedir(dirp);
		    return -1;
		}
	    }

	    /* Copy the entry. */
#ifdef DIRSIZ
	    names[i - 1] = (struct dirent *)malloc(DIRSIZ(entp));
#else
	    names[i - 1] = (struct dirent *)malloc(sizeof(struct dirent) 
					        + strlen(entp->d_name)+1);
#endif
	    if (names[i - 1] == NULL) {
		closedir(dirp);
		return -1;
	    }
	    names[i - 1]->d_ino = entp->d_ino;
	    names[i - 1]->d_reclen = entp->d_reclen;
#ifndef USE_DIRENT
	    names[i - 1]->d_namlen = entp->d_namlen;
#endif
	    (void)strcpy(names[i - 1]->d_name, entp->d_name);
	}

    /* Close things off. */
    names[i] = NULL;
    *list = names;
    closedir(dirp);

    /* Sort? */
    if (i && sorter)
	qsort((char *)names, i, sizeof names[0], sorter);

    return i;
}
