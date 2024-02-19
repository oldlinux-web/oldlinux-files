/* co - check out			Author: Peter S. Housel 12/24/87 */

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <stdio.h>

#define SUFFIX		",S"	/* svc indicator */
#define SVCDIR		"SVC"	/* svc postfix indicator */

#define LINELEN		256	/* maximum line length */

#ifdef MAXPATHLEN
#define PATHLEN MAXPATHLEN
#else
#define PATHLEN 128		/* buffer length for filenames */
#endif

char file[PATHLEN];		/* file to be checked in */
char svc[PATHLEN];		/* filename for svc file */
char newsvc[PATHLEN];		/* new copy of SVC file */
char line[LINELEN];		/* temporary line buffer */
char *p;			/* scratch character pointer */

FILE *svcfp;			/* svc file */
int rev;			/* old revision number */
int lastrev, lockrev;		/* latest file revision, lock into */
int status;			/* wait() buffer */
int lock;			/* lock the SVC file */
struct stat stb;		/* stat() buffer */
char *base;			/* basename of file */

char difftemp[PATHLEN];		/* extract() fix/patch input */

extern char *mktemp();
extern struct passwd *getpwuid();

char *whoami(), *basename();

main(argc, argv)
int argc;
char **argv;
{
#ifdef perprintf
  char errbuf[BUFSIZ];
  setbuf(stderr, errbuf);
  perprintf(stderr);
#endif

  while (++argv, --argc) {
	if ('-' == (*argv)[0]) {
		if ('r' == (*argv)[1]) {
			--argc;
			rev = atoi(*++argv);
			if (rev < 1) {
				fprintf(stderr, "Illegal revision number\n");
				exit(1);
			}
		} else if ('l' == (*argv)[1])
			++lock;
		else {
			fprintf(stderr, "co: illegal option -%c\n", (*argv)[1]);
			exit(1);
		}
	} else
		break;
  }

  if (1 != argc) {
	fprintf(stderr, "co: bad number of files arguments\n");
	exit(1);
  }
  fname(*argv, file);
  svcname(file, svc);

  fprintf(stderr, "%s -> %s\n", svc, base = basename(file));

  if ((FILE *) NULL == (svcfp = fopen(svc, "r"))) {
	perror("co: can't read SVC file");
	exit(1);
  }
  if (1 != fscanf(svcfp, "# %d", &lastrev) || lastrev < 1) {
	fprintf(stderr, "co: illegal SVC file format\n");
	exit(1);
  }
  fclose(svcfp);

  if (stat(base, &stb) >= 0 && (stb.st_mode & 0222)) {
	fprintf(stderr, "Writable %s exists - overwrite (n/y)? ", base);
	if (!getyn()) {
		fprintf(stderr, "Checkout aborted\n");
		exit(1);
	}
  }
  if (strlen(base)) unlink(base);
  if (0 == rev) rev = lastrev;
  fprintf(stderr, "Checking out revision %d", rev);
  extract(svc, base, rev);

  if (lock) {
	lockrev = lastrev + 1;
	fprintf(stderr, "; Locking into revision %d\n", lockrev);
	if (stat(svc, &stb) < 0 || chmod(svc, stb.st_mode | 0200) < 0)
		perror("co: can't chmod SVC file");

	if (stat(base, &stb) < 0 || chmod(base, stb.st_mode | 0200) < 0)
		perror("co: can't chmod source file");

	if ((FILE *) NULL == (svcfp = fopen(svc, "a"))
	    || (fprintf(svcfp, "#***SVCLOCK*** %s %d\n", whoami(), lockrev), ferror(svcfp))) {
		fprintf(stderr, "co: can't lock %s\n", svc);
		exit(1);
	}
	if (stat(svc, &stb) < 0 || chmod(svc, stb.st_mode & 0555))
		perror("co: can't chmod SVC file");
  } else {
	putchar('\n');
	if (stat(base, &stb) < 0 || chmod(base, stb.st_mode & 0555))
		perror("co: can't chmod source file");
  }

  exit(0);
}


fname(src, dst)
char *src, *dst;
{
  char *p;
  strcpy(dst, src);
  p = &dst[strlen(src) - strlen(SUFFIX)];
  if (!strcmp(p, SUFFIX)) *p = '\0';
}

svcname(src, dst)
char *src, *dst;
{
  char *p;

  strcpy(dst, src);
  strcat(dst, SUFFIX);

  if (0 != access(dst, 4)) {
	char dirname[PATHLEN];
	if ( (char *) NULL != (p = strrchr(src, '/')))
		strncpy(dirname, src, (int)(p - src) + 1);
	else
		dirname[0] = '\0';
	strcat(dirname, SVCDIR);

	if (0 == access(dirname, 1)) {
		strcpy(dst, dirname);
		if ((char *) NULL == p) {
			strcat(dst, "/");
			strcat(dst, src);
		} else
			strcat(dst, p);
		strcat(dst, SUFFIX);
	}
  }
}

extract(script, out, rev)
char *script, *out;
int rev;
{
  FILE *outfp;
  int testrev;
  char buf[80];

  sprintf(difftemp, "Fix.%s", out);

  svcfp = fopen(script, "r");
  fgets(line, LINELEN, svcfp);	/* skip '# rev' line */
  fgets(line, LINELEN, svcfp);	/* skip 'cat <***MAIN-eof***' line */

  if ((FILE *) NULL == (outfp = fopen(out, "w"))) {
	perror("co: can't create output file");
	return;
  }
  while ((char *) NULL != fgets(line, LINELEN, svcfp) &&
	  strcmp(line, "***MAIN-eof***\n"))
	fputs(line, outfp);

  fclose(outfp);

  while ((char *) NULL != fgets(line, LINELEN, svcfp)) {
	if (!strncmp(line, "if ", 3)) {
		sscanf(line, "if test $2 -ge %d", &testrev);
		if (rev >= testrev) {
			unlink(difftemp);
			return;
		}
		if ((FILE *) NULL == (outfp = fopen(difftemp, "w"))) {
			perror("co: can't create output file");
			return;
		}
		sprintf(buf, "***%d-eof***\n", testrev);
		while ((char *) NULL != fgets(line, LINELEN, svcfp) &&
							strcmp(line, buf))
			fputs(line, outfp);
		fclose(outfp);
	} else if (!strncmp(line, "mv ", 3)) {
		sprintf(buf, "mv Fix.%s %s", out, out);
		system(buf);
	} else if (!strncmp(line, "fix ", 4)) {
		sprintf(buf, "fix %s Fix.%s > New.%s; mv New.%s %s", out, out, out, out, out);
		system(buf);
	} else if (!strncmp(line, "patch ", 6)) {
		sprintf(buf, "patch -n -s %s < Fix.%s; rm -f %s.orig", out, out, out);
		system(buf);
	} else {		/* ignore */
	}
  }

  unlink(difftemp);
  return;
}

char *basename(name)
char *name;
{
  char *p;

  if ((char *) NULL == (p = strrchr(name, '/')))
	return name;
  else
	return p + 1;
}

char *whoami()
{
  struct passwd *pw;

  if ((struct passwd *) NULL != (pw = getpwuid(getuid())))
	return pw->pw_name;
  else
	return "nobody";
}

int getyn()
{
  char ans[10];

  return((char *) NULL != fgets(ans, 10, stdin)) && ('y' == ans[0] || 'Y' == ans[0]);
}
