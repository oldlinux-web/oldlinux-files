/* glob.c: rc's (ugly) globber. This code is not elegant, but it works */

#include <sys/types.h>
#include <sys/stat.h>
#include "rc.h"
#include "glob.h"
#include "glom.h"
#include "nalloc.h"
#include "utils.h"
#include "match.h"
#include "footobar.h"
#include "list.h"
#ifdef NODIRENT
#include <sys/dir.h>
#define dirent direct /* need to get the struct declaraction right */
#else
#include <dirent.h>
#endif

static List *dmatch(char *, char *, char *);
static List *doglob(char *, char *);
static List *lglob(List *, char *, char *, SIZE_T);
static List *sort(List *);

/*
   matches a list of words s against a list of patterns p. Returns true iff
   a pattern in p matches a word in s. null matches null, but otherwise null
   patterns match nothing.
*/

boolean lmatch(List *s, List *p) {
	List *q;
	int i;
	boolean okay;

	if (s == NULL) {
		if (p == NULL) /* null matches null */
			return TRUE;
		for (; p != NULL; p = p->n) { /* one or more stars match null */
			if (*p->w != '\0') { /* the null string is a special case; it does *not* match () */
				okay = TRUE;
				for (i = 0; p->w[i] != '\0'; i++)
					if (p->w[i] != '*' || p->m[i] != 1) {
						okay = FALSE;
						break;
					}
				if (okay)
					return TRUE;
			}
		}
		return FALSE;
	}

	for (; s != NULL; s = s->n)
		for (q = p; q != NULL; q = q->n)
			if (match(q->w, q->m, s->w))
				return TRUE;
	return FALSE;
}

/* Matches a pattern p against the contents of directory d */

static List *dmatch(char *d, char *p, char *m) {
	boolean matched = FALSE;
	List *top, *r;
	DIR *dirp;
	struct dirent *dp;
	struct stat s;
	/* prototypes for XXXdir functions. comment out if necessary */
	extern DIR *opendir(const char *);
	extern struct dirent *readdir(DIR *);
	extern int closedir(DIR *);

	/* opendir succeeds on regular files on some systems, so the stat() call is necessary (sigh) */
	if (stat(d, &s) < 0 || (s.st_mode & S_IFMT) != S_IFDIR)
		return NULL;
	if ((dirp = opendir(d)) == NULL)
		return NULL;

	top = r = NULL;

	while ((dp = readdir(dirp)) != NULL)
		if ((*dp->d_name != '.' || *p == '.') && match(p, m, dp->d_name)) { /* match ^. explicitly */
			matched = TRUE;
			if (top == NULL)
				top = r = nnew(List);
			else
				r = r->n = nnew(List);
			r->w = ncpy(dp->d_name);
			r->m = NULL;
		}

	closedir(dirp);

	if (!matched)
		return NULL;

	r->n = NULL;
	return top;
}

/*
   lglob() globs a pattern agains a list of directory roots. e.g., (/tmp /usr /bin) "*"
   will return a list with all the files in /tmp, /usr, and /bin. NULL on no match.
   slashcount indicates the number of slashes to stick between the directory and the
   matched name. e.g., for matching ////tmp/////foo*
*/

static List *lglob(List *s, char *p, char *m, SIZE_T slashcount) {
	List *q, *r, *top, foo;
	static List slash;
	static SIZE_T slashsize = 0;

	if (slashcount + 1 > slashsize) {
		slash.w = ealloc(slashcount + 1);
		slashsize = slashcount;
	}
	slash.w[slashcount] = '\0';
	while (slashcount > 0)
		slash.w[--slashcount] = '/';

	for (top = r = NULL; s != NULL; s = s->n) {
		q = dmatch(s->w, p, m);
		if (q != NULL) {
			foo.w = s->w;
			foo.m = NULL;
			foo.n = NULL;
			if (!(s->w[0] == '/' && s->w[1] == '\0')) /* need to separate */
				q = concat(&slash, q);		  /* dir/name with slash */
			q = concat(&foo, q);
			if (r == NULL)
				top = r = q;
			else
				r->n = q;
			while (r->n != NULL)
				r = r->n;
		}
	}
	return top;
}

/*
   Doglob globs a pathname in pattern form against a unix path. Returns the original
   pattern (cleaned of metacharacters) on failure, or the globbed string(s).
*/

static List *doglob(char *w, char *m) {
	static char *dir = NULL, *pattern = NULL, *metadir = NULL, *metapattern = NULL;
	static SIZE_T dsize = 0;
	char *d, *p, *md, *mp;
	SIZE_T psize;
	char *s = w;
	List firstdir;
	List *matched;
	int slashcount;

	if ((psize = strlen(w) + 1) > dsize || dir == NULL) {
		efree(dir); efree(pattern); efree(metadir); efree(metapattern);
		dir = ealloc(psize);
		pattern = ealloc(psize);
		metadir = ealloc(psize);
		metapattern = ealloc(psize);
		dsize = psize;
	}

	d = dir;
	p = pattern;
	md = metadir;
	mp = metapattern;

	if (*s == '/') {
		while (*s == '/')
			*d++ = *s++, *md++ = *m++;
	} else {
		while (*s != '/' && *s != '\0')
			*d++ = *s++, *md++ = *m++; /* get first directory component */
	}
	*d = '\0';

	/*
	   Special case: no slashes in the pattern, i.e., open the current directory.
	   Remember that w cannot consist of slashes alone (the other way *s could be
	   zero) since doglob gets called iff there's a metacharacter to be matched
	*/
	if (*s == '\0') {
		matched = dmatch(".", dir, metadir);
		goto end;
	}

	if (*w == '/') {
		firstdir.w = dir;
		firstdir.m = metadir;
		firstdir.n = NULL;
		matched = &firstdir;
	} else {
		/*
		   we must glob against current directory,
		   since the first character is not a slash.
		*/
		matched = dmatch(".", dir, metadir);
	}

	do {
		for (slashcount = 0; *s == '/'; s++, m++)
			slashcount++; /* skip slashes */

		while (*s != '/' && *s != '\0')
			*p++ = *s++, *mp++ = *m++; /* get pattern */
		*p = '\0';

		matched = lglob(matched, pattern, metapattern, slashcount);

		p = pattern, mp = metapattern;
	} while (*s != '\0');

end:	if (matched == NULL) {
		matched = nnew(List);
		matched->w = w;
		matched->m = NULL;
		matched->n = NULL;
	}
	return matched;
}

/*
   Globs a list; checks to see if each element in the list has a metacharacter. If it
   does, it is globbed, and the output is sorted.
*/

List *glob(List *s) {
	List *top, *r;
	boolean meta;

	for (r = s, meta = FALSE; r != NULL; r = r->n)
		if (r->m != NULL)
			meta = TRUE;

	if (!meta)
		return s; /* don't copy lists with no metacharacters in them */

	for (top = r = NULL; s != NULL; s = s->n) {
		if (s->m == NULL) { /* no metacharacters; just tack on to the return list */
			if (top == NULL) {
				top = r = nnew(List);
			} else {
				r->n = nnew(List);
				r = r->n;
			}
			r->w = s->w;
		} else {
			if (top == NULL)
				top = r = sort(doglob(s->w, s->m));
			else
				r->n = sort(doglob(s->w, s->m));
			while (r->n != NULL)
				r = r->n;
		}
	}

	r->n = NULL;
	return top;
}

static List *sort(List *s) {
	SIZE_T nel = listnel(s);

	if (nel > 1) {
		char **a;
		List *t;

		qsort(a = list2array(s, FALSE), nel, sizeof(char *), starstrcmp);

		for (t = s; t != NULL; t = t->n)
			t->w = *a++;
	}

	return s;
}
