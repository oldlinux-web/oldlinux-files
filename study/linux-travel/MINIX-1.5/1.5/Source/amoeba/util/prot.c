#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#define isblank(c)	((c) == ' ' || (c) == '\t' || (c) == '\n')

#define MAXTOKEN	16

static char *gettok(fp)
FILE *fp;
{
	static char buf[MAXTOKEN + 1];
	register char c, *p = buf;

	do		/* skip blanks */
		if ((c = getc(fp)) == EOF)
			return(0);
	while (isblank(c));
	do {
		if (c == '#') {		/* skip comment */
			while ((c = getc(fp)) != EOF)
				if (c == '\n') break;
			break;
		}
		*p++ = c;
	} while (c != ':' && (c = getc(fp)) != EOF && !isblank(c));
	*p++ = 0;
	return(buf);
}

static number(p)
char *p;
{
	int n;

	return sscanf(p, "%d", &n) == 1 ? n : -1;
}

chkprot(chkfil, chkdefault, uid, gid)
char *chkfil;
char *chkdefault;
{
	register userchk = 0, n;
	register FILE *fp;
	register char *p;
	register struct passwd *pw;
	register struct group *gr;
	extern FILE *fopen();
	extern struct passwd *getpwuid();
	extern struct group *getgrgid();

	pw = getpwuid(uid);
	endpwent();
	gr = getgrgid(gid);
	endgrent();
	if ((fp = fopen(chkfil, "r")) == NULL && (fp = fopen(chkdefault, "r")) == NULL)
		return(-1);
	while ((p = gettok(fp)) != 0)
		if (strcmp(p, "group:") == 0)
			userchk = 0;
		else if (strcmp(p, "user:") == 0)
			userchk = 1;
		else if (userchk) {
			if (pw && strcmp(p, pw->pw_name) == 0 ||
							uid == number(p)) {
				fclose(fp);
				return(1);
			}
		}
		else
			if (gr && strcmp(p, gr->gr_name) == 0 ||
							gid == number(p)) {
				fclose(fp);
				return(1);
			}
	return(0);
}
