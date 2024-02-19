/* tail - print the end of a file */

#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define BLANK ' '
#define TAB '\t'
#define NEWL '\n'

int lines, chars;
char buff[BUFSIZ];

main(argc, argv)
int argc;
char *argv[];
{
  char *s;
  FILE *input, *fopen();
  int count;

  setbuf(stdout, buff);
  argc--;
  argv++;
  lines = TRUE;
  chars = FALSE;
  count = -10;

  if (argc == 0) {
	tail(stdin, count);
	exit(0);
  }
  s = *argv;
  if (*s == '-' || *s == '+') {
	s++;
	if (*s >= '0' && *s <= '9') {
		count = stoi(*argv);
		s++;
		while (*s >= '0' && *s <= '9') s++;
	}
	if (*s == 'c') {
		chars = TRUE;
		lines = FALSE;
	} else if (*s != 'l' && *s != '\0') {
		fprintf(stderr, "tail: unknown option %c\n", *s);
		argc = 0;
	}
	argc--;
	argv++;
  }
  if (argc < 0) {
	fprintf(stderr, "Usage: tail [+/-[number][lc]] [files]\n");
	exit(1);
  }
  if (argc == 0) tail(stdin, count);

  else if ((input = fopen(*argv, "r")) == NULL) {
	fprintf(stderr, "tail: can't open %s\n", *argv);
	exit(1);
  } else {
	tail(input, count);
	fclose(input);
  }

  exit(0);
}

/* Stoi - convert string to integer */
stoi(s)
char *s;
{
  int n, sign;

  while (*s == BLANK || *s == NEWL || *s == TAB) s++;

  sign = 1;
  if (*s == '+')
	s++;
  else if (*s == '-') {
	sign = -1;
	s++;
  }
  for (n = 0; *s >= '0' && *s <= '9'; s++) n = 10 * n + *s - '0';
  return(sign * n);
}

/* Tail - print 'count' lines/chars */

#define INCR(p)  if (p >= end) p=cbuf ; else p++
#define BUF_SIZE 4098

char cbuf[BUF_SIZE];

tail(in, goal)
FILE *in;
int goal;
{
  int c, count;
  char *start, *finish, *end;

  count = 0;

  if (goal > 0) {		/* skip */
	count++;		/* start counting at 1 */
	if (lines)		/* lines */
		while ((c = getc(in)) != EOF) {
			if (c == NEWL) count++;
			if (count >= goal) break;
		}
	else			/* chars */
		while (getc(in) != EOF) {
			count++;
			if (count >= goal) break;
		}
	if (count >= goal) while ((c = getc(in)) != EOF)
			putc(c, stdout);
  } else {			/* tail */

	goal = -goal;
	start = finish = cbuf;
	end = &cbuf[BUF_SIZE - 1];

	while ((c = getc(in)) != EOF) {
		*finish = c;
		INCR(finish);

		if (start == finish) INCR(start);
		if (!lines || c == NEWL) count++;

		if (count > goal) {
			count = goal;
			if (lines) while (*start != NEWL)
					INCR(start);
			INCR(start);
		}
	}			/* end while */

	while (start != finish) {
		putc(*start, stdout);
		INCR(start);
	}

  }				/* end else */

}				/* end tail */
