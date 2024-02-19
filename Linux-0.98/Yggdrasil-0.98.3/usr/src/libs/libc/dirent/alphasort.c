/*
**  ALPHASORT
**  Trivial sorting predicate for scandir; puts entries in alphabetical order.
*/

#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#ifdef	RCSID
static char RCS[] = "$Header: /proj/wais/wais-8-b2/lib/RCS/alphasort.c,v 1.1 91/07/17 16:37:56 jonathan Exp $";
#endif	/* RCSID */

int
alphasort(d1, d2)
    const struct dirent * const *d1;
    const struct dirent * const *d2;
{
    return strcmp(d1[0]->d_name, d2[0]->d_name);
}
