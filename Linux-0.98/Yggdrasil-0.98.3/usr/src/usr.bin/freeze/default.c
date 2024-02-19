#include "freeze.h"

		/*-------------------------------*/
		/*     DEFAULTS FILE HANDLING    */
		/*-------------------------------*/

#define OK      0
#define FAIL    NULL
#define NOFILE  ((FILE *) 0)
#define MAXLINE 128

extern int      errno;
char            *strchr();
static FILE     *defd = NOFILE;  /* defaults file stream */

int     defopen(fname)          /* open | reopen | close defaults file */
	char    *fname;
{
	register FILE   *fd;

	if (!fname) {
		if (defd)
			(void) fclose(defd);
		defd = NOFILE;
		return OK;
	}

	if (!(fd = fopen(fname, "r")))
		return errno;                   /* problems opening file */

	defd = fd;
	return OK;
}

static char     defline[MAXLINE + 1];

char    *defread(pattern)
	register char   *pattern;
{
	register        sz_patt;
	register char   *cp;

	if (!defd)
		return FAIL;            /* defaults file not opened */

	rewind(defd);
	sz_patt = strlen(pattern);

	while (fgets(defline, MAXLINE, defd)) {
		if (!(cp = strchr(defline, '\n')))
			return FAIL;     /* line too long  */
		if (cp - defline < sz_patt)
			continue;       /* line too short */
		*cp = '\0';
		if (!strncmp(pattern, defline, sz_patt))
			return defline + sz_patt;       /* match found */
	}

	return FAIL;                    /* no matching lines */
}
