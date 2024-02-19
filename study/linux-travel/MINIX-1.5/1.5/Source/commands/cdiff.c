/* cdiff - context diff			Author: Larry Wall */

/* Cdiff - turns a regular diff into a new-style context diff
 *
 * Usage: cdiff file1 file2
 */

#define PATCHLEVEL 2

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>

char buff[512];

FILE *inputfp, *oldfp, *newfp;
char *ctime();

int oldmin, oldmax, newmin, newmax;
int oldbeg, oldend, newbeg, newend;
int preoldmax, prenewmax;
int preoldbeg, preoldend, prenewbeg, prenewend;
int oldwanted, newwanted;

char *oldhunk, *newhunk;
unsigned oldsize, oldalloc, newsize, newalloc;

void dumphunk();
char *getold();
char *getnew();
char *malloc();
char *realloc();
char *fgets();
FILE *popen();

#define Nullfp (FILE*)0
#define Nullch (char*)0

main(argc, argv)
int argc;
char **argv;
{
  char *old, *new;
  int context = 3;
  struct stat statbuf;
  register char *s;
  char op;
  char *newmark, *oldmark;
  int len;
  char *line;
  int i;

  oldalloc = 512;
  oldhunk = malloc(oldalloc);
  newalloc = 512;
  newhunk = malloc(newalloc);

  for (argc--, argv++; argc; argc--, argv++) {
	if (argv[0][0] != '-') break;

	if (argv[0][1] == 'c') context = atoi(argv[0] + 2);
  }

  if (argc != 2) {
	fprintf(stderr, "Usage: cdiff old new\n");
	exit(1);
  }
  old = argv[0];
  new = argv[1];

  sprintf(buff, "diff %s %s", old, new);
  inputfp = popen(buff, "r");
  if (!inputfp) {
	fprintf(stderr, "Can't execute diff %s %s\n", old, new);
	exit(1);
  }
  oldfp = fopen(old, "r");
  if (!oldfp) {
	fprintf(stderr, "Can't open %s\n", old);
	exit(1);
  }
  newfp = fopen(new, "r");
  if (!newfp) {
	fprintf(stderr, "Can't open %s\n", new);
	exit(1);
  }
  fstat(fileno(oldfp), &statbuf);
  printf("*** %s\t%s", old, ctime(&statbuf.st_mtime));
  fstat(fileno(newfp), &statbuf);
  printf("--- %s\t%s", new, ctime(&statbuf.st_mtime));

  preoldend = -1000;

  while (fgets(buff, sizeof buff, inputfp) != Nullch) {
	if (isdigit(*buff)) {
		oldmin = atoi(buff);
		for (s = buff; isdigit(*s); s++);
		if (*s == ',') {
			s++;
			oldmax = atoi(s);
			for (; isdigit(*s); s++);
		} else {
			oldmax = oldmin;
		}
		if (*s != 'a' && *s != 'd' && *s != 'c') {
			fprintf(stderr, "Unparseable input: %s", s);
			exit(1);
		}
		op = *s;
		s++;
		newmin = atoi(s);
		for (; isdigit(*s); s++);
		if (*s == ',') {
			s++;
			newmax = atoi(s);
			for (; isdigit(*s); s++);
		} else {
			newmax = newmin;
		}
		if (*s != '\n' && *s != ' ') {
			fprintf(stderr, "Unparseable input: %s", s);
			exit(1);
		}
		newmark = oldmark = "! ";
		if (op == 'a') {
			oldmin++;
			newmark = "+ ";
		}
		if (op == 'd') {
			newmin++;
			oldmark = "- ";
		}
		oldbeg = oldmin - context;
		oldend = oldmax + context;
		if (oldbeg < 1) oldbeg = 1;
		newbeg = newmin - context;
		newend = newmax + context;
		if (newbeg < 1) newbeg = 1;

		if (preoldend < oldbeg - 1) {
			if (preoldend >= 0) {
				dumphunk();
			}
			preoldbeg = oldbeg;
			prenewbeg = newbeg;
			oldwanted = newwanted = 0;
			oldsize = newsize = 0;
		} else {	/* we want to append to previous hunk */
			oldbeg = preoldmax + 1;
			newbeg = prenewmax + 1;
		}

		for (i = oldbeg; i <= oldmax; i++) {
			line = getold(i);
			if (!*line) {
				oldend = oldmax = i - 1;
				break;
			}
			len = strlen(line) + 2;
			if (oldsize + len + 1 >= oldalloc) {
				oldalloc *= 2;
				oldhunk = realloc(oldhunk, oldalloc);
			}
			if (i >= oldmin) {
				strcpy(oldhunk + oldsize, oldmark);
				oldwanted++;
			} else {
				strcpy(oldhunk + oldsize, "  ");
			}
			strcpy(oldhunk + oldsize + 2, line);
			oldsize += len;
		}
		preoldmax = oldmax;
		preoldend = oldend;

		for (i = newbeg; i <= newmax; i++) {
			line = getnew(i);
			if (!*line) {
				newend = newmax = i - 1;
				break;
			}
			len = strlen(line) + 2;
			if (newsize + len + 1 >= newalloc) {
				newalloc *= 2;
				newhunk = realloc(newhunk, newalloc);
			}
			if (i >= newmin) {
				strcpy(newhunk + newsize, newmark);
				newwanted++;
			} else {
				strcpy(newhunk + newsize, "  ");
			}
			strcpy(newhunk + newsize + 2, line);
			newsize += len;
		}
		prenewmax = newmax;
		prenewend = newend;
	}
  }

  if (preoldend >= 0) {
	dumphunk();
  }
}

void dumphunk()
{
  int i;
  char *line;
  int len;

  for (i = preoldmax + 1; i <= preoldend; i++) {
	line = getold(i);
	if (!line) {
		preoldend = i - 1;
		break;
	}
	len = strlen(line) + 2;
	if (oldsize + len + 1 >= oldalloc) {
		oldalloc *= 2;
		oldhunk = realloc(oldhunk, oldalloc);
	}
	strcpy(oldhunk + oldsize, "  ");
	strcpy(oldhunk + oldsize + 2, line);
	oldsize += len;
  }
  for (i = prenewmax + 1; i <= prenewend; i++) {
	line = getnew(i);
	if (!line) {
		prenewend = i - 1;
		break;
	}
	len = strlen(line) + 2;
	if (newsize + len + 1 >= newalloc) {
		newalloc *= 2;
		newhunk = realloc(newhunk, newalloc);
	}
	strcpy(newhunk + newsize, "  ");
	strcpy(newhunk + newsize + 2, line);
	newsize += len;
  }
  fputs("***************\n", stdout);
  if (preoldbeg >= preoldend) {
	printf("*** %d ****\n", preoldend);
  } else {
	printf("*** %d,%d ****\n", preoldbeg, preoldend);
  }
  if (oldwanted) {
	fputs(oldhunk, stdout);
  }
  oldsize = 0;
  *oldhunk = '\0';
  if (prenewbeg >= prenewend) {
	printf("--- %d ----\n", prenewend);
  } else {
	printf("--- %d,%d ----\n", prenewbeg, prenewend);
  }
  if (newwanted) {
	fputs(newhunk, stdout);
  }
  newsize = 0;
  *newhunk = '\0';
}

char *
 getold(targ)
int targ;
{
  static int oldline = 0;

  while (fgets(buff, sizeof buff, oldfp) != Nullch) {
	oldline++;
	if (oldline == targ) return buff;
  }
  return Nullch;
}

char *
 getnew(targ)
int targ;
{
  static int newline = 0;

  while (fgets(buff, sizeof buff, newfp) != Nullch) {
	newline++;
	if (newline == targ) return buff;
  }
  return Nullch;
}
