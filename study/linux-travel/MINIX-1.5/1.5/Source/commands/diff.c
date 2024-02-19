/* diff  - print differences between 2 files	  Author: Erik Baalbergen */

/* Poor man's implementation of diff(1)
 - no options available
 - may give more output than other diffs, due to the straight-forward algorithm
 - runs out of memory if the differing chunks become too large
 - input line length should not exceed LINELEN; longer lines are truncated,
   while only the first LINELEN characters are compared

 - Bug fixes by Rick Thomas Sept. 1989

 Please report bugs and suggestions to erikb@cs.vu.nl
*/

#include <stdio.h>

#define LINELEN 128

FILE *fopen();

char *prog;
int diffs = 0;

main(argc, argv)
char **argv;
{
  FILE *fp1 = NULL, *fp2 = NULL;

  prog = *argv++;
  if (argc != 3) fatal("Usage: %s file1 file2", prog);
  if (strcmp(argv[0], "-") == 0)
	fp1 = stdin;
  else if (strcmp(argv[1], "-") == 0)
	fp2 = stdin;
  if (fp1 == NULL && (fp1 = fopen(argv[0], "r")) == NULL)
	fatal("can't read %s", argv[0]);
  if (fp2 == NULL && (fp2 = fopen(argv[1], "r")) == NULL)
	fatal("can't read %s", argv[1]);
  diff(fp1, fp2);
  exit(diffs > 0);
}

fatal(fmt, s)
char *fmt, *s;
{
  fprintf(stderr, "%s: ", prog);
  fprintf(stderr, fmt, s);
  fprintf(stderr, "\n");
  exit(2);
}

/* The line module */
char *malloc();
char *fgets();

struct line {
  struct line *l_next;
  char l_text[LINELEN + 2];
  char l_eof;
};

struct line *freelist = 0;
#define stepup(ll) ( ((ll) && ((ll)->l_eof==0)) ? (ll)->l_next : (ll) )

struct line *
 new_line()
{
  register struct line *l;

  if (l = freelist)
	freelist = freelist->l_next;
  else if ((l = (struct line *) malloc(sizeof(struct line))) == 0)
	fatal("out of memory");
  return l;
}

free_line(l)
register struct line *l;
{
  l->l_next = freelist;
  freelist = l;
}

int equal_line(l1, l2)
struct line *l1, *l2;
{
  if (l1 == 0 || l2 == 0)
	return(0);
  else if (l1->l_eof || l2->l_eof)
	return(l1->l_eof == l2->l_eof);
  else
	return(strcmp(l1->l_text, l2->l_text) == 0);
}

int equal_3(l1, l2)
struct line *l1, *l2;
{
  register int i, ansr;

  ansr = 1;
#ifdef DEBUG
  if (l1 == 0)
	fprintf(stderr, "\t(null)\n");
  else if (l1->l_eof)
	fprintf(stderr, "\t(eof)\n");
  else
	fprintf(stderr, "\t%s", l1->l_text);
  if (l2 == 0)
	fprintf(stderr, "\t(null)\n");
  else if (l2->l_eof)
	fprintf(stderr, "\t(eof)\n");
  else
	fprintf(stderr, "\t%s", l2->l_text);
#endif
  for (i = 0; i < 3; ++i) {
	if (!equal_line(l1, l2)) {
		ansr = 0;
		break;
	}
	l1 = stepup(l1);
	l2 = stepup(l2);
  }
#ifdef DEBUG
  fprintf(stderr, "\t%d\n", ansr);
#endif
  return(ansr);
}

struct line *
 read_line(fp)
FILE *fp;
{
  register struct line *l = new_line();
  register char *p;
  register int c;

  (p = &(l->l_text[LINELEN]))[1] = '\377';
  l->l_eof = 0;
  if (fgets(l->l_text, LINELEN + 2, fp) == NULL) {
	l->l_eof = 1;
	l->l_text[0] = 0;
  } else if ((p[1] & 0377) != 0377 && *p != '\n') {
	while ((c = fgetc(fp)) != '\n' && c != EOF) {
	}
	*p++ = '\n';
	*p = '\0';
  }
  l->l_next = 0;
  return l;
}

/* File window handler */
struct f {
  struct line *f_bwin, *f_ewin;
  struct line *f_aside;
  int f_linecnt;		/* line number in file of last advanced line */
  FILE *f_fp;
};

advance(f)
register struct f *f;
{
  register struct line *l;

  if (l = f->f_bwin) {
	if (f->f_ewin == l)
		f->f_bwin = f->f_ewin = 0;
	else
		f->f_bwin = l->l_next;
	free_line(l);
	(f->f_linecnt)++;
  }
}

aside(f, l)
struct f *f;
struct line *l;
{
  register struct line *ll;

  if (l == 0) return;
  if (ll = l->l_next) {
	while (ll->l_next) ll = ll->l_next;
	ll->l_next = f->f_aside;
	f->f_aside = l->l_next;
	l->l_next = 0;
	f->f_ewin = l;
  }
}

struct line *
 next(f)
register struct f *f;
{
  register struct line *l;

  if (l = f->f_aside) {
	f->f_aside = l->l_next;
	l->l_next = 0;
  } else
	l = read_line(f->f_fp);
  if (l) {
	if (f->f_bwin == 0)
		f->f_bwin = f->f_ewin = l;
	else {
		if (f->f_ewin->l_eof && l->l_eof) {
			free_line(l);
			return(f->f_ewin);
		}
		f->f_ewin->l_next = l;
		f->f_ewin = l;
	}
  }
  return l;
}

init_f(f, fp)
register struct f *f;
FILE *fp;
{
  f->f_bwin = f->f_ewin = f->f_aside = 0;
  f->f_linecnt = 0;
  f->f_fp = fp;
}

update(f, s)
register struct f *f;
char *s;
{
  while (f->f_bwin && f->f_bwin != f->f_ewin) {
	printf("%s%s", s, f->f_bwin->l_text);
	advance(f);
  }
}

/* Diff procedure */
diff(fp1, fp2)
FILE *fp1, *fp2;
{
  struct f f1, f2;
  struct line *l1, *s1, *b1, *l2, *s2, *b2;
  register struct line *ll;

  init_f(&f1, fp1);
  init_f(&f2, fp2);
  l1 = next(&f1);
  l2 = next(&f2);
  while ((l1->l_eof == 0) || (l2->l_eof == 0)) {
	if (equal_line(l1, l2)) {
  equal:
		advance(&f1);
		advance(&f2);
		l1 = next(&f1);
		l2 = next(&f2);
		continue;
	}
	s1 = b1 = l1;
	s2 = b2 = l2;
	/* Read several more lines */
	next(&f1);
	next(&f1);
	next(&f2);
	next(&f2);
	/* Start searching */
search:
	next(&f2);
	ll = s1;
	do {
		if (equal_3(ll, b2)) {
			l1 = ll;
			l2 = b2;
			aside(&f1, ll);
			aside(&f2, b2);
			differ(&f1, &f2);
			goto equal;
		}
		if (ll->l_eof) break;
		ll = stepup(ll);
	} while (ll);
	b2 = stepup(b2);

	next(&f1);
	ll = s2;
	do {
		if (equal_3(b1, ll)) {
			l1 = b1;
			l2 = ll;
			aside(&f2, ll);
			aside(&f1, b1);
			differ(&f1, &f2);
			goto equal;
		}
		if (ll->l_eof) break;
		ll = stepup(ll);
	} while (ll);
	b1 = stepup(b1);

	goto search;
  }

  /* Both of the files reached EOF */
}

differ(f1, f2)
register struct f *f1, *f2;
{
  int cnt1 = f1->f_linecnt, len1 = wlen(f1), cnt2 = f2->f_linecnt, len2 = wlen(f2);

  if ((len1 = wlen(f1)) || (len2 = wlen(f2))) {
	if (len1 == 0) {
		printf("%da", cnt1);
		range(cnt2 + 1, cnt2 + len2);
	} else if (len2 == 0) {
		range(cnt1 + 1, cnt1 + len1);
		printf("d%d", cnt2);
	} else {
		range(cnt1 + 1, cnt1 + len1);
		putchar('c');
		range(cnt2 + 1, cnt2 + len2);
	}
	putchar('\n');
	if (len1) update(f1, "< ");
	if (len1 && len2) printf("---\n");
	if (len2) update(f2, "> ");
	diffs++;
  }
}

wlen(f)
struct f *f;
{
  register cnt = 0;
  register struct line *l = f->f_bwin, *e = f->f_ewin;

  while (l && l != e) {
	cnt++;
	l = l->l_next;
  }
  return cnt;
}

range(a, b)
{
  printf(((a == b) ? "%d" : "%d,%d"), a, b);
}
