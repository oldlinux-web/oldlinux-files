/* gres - grep and substitute		Author: Martin C. Atkins */

/*
 *	globally search, and replace
 *
 *<-xtx-*>cc -o gres gres.c -lregexp
 */

/*
 *	This program was written by:
 *		Martin C. Atkins,
 *		University of York,
 *		Heslington,
 *		York. Y01 5DD
 *		England
 *	and is released into the public domain, on the condition
 *	that this comment is always included without alteration.
 */

#include <regexp.h>
#include <string.h>
#include <stdio.h>

#define MAXLINE (1024)

int status = 1;
char *usagemsg = "Usage: gres [-g] search replace [file ...]\n";
char *progname;
int gflag = 0;			/* != 0 => only do first substitution on line */

main(argc, argv)
int argc;
char *argv[];
{
  regexp *exp;
  char *repstr;
  char **argp = &argv[1];

  progname = argv[0];
  if (*argp != 0 && argp[0][0] == '-' && argp[0][1] == 'g') {
	gflag = 1;
	argp++, argc--;
  }
  if (argc < 3) {
	std_err(usagemsg);
	exit(2);
  }
  if (argp[0][0] == '\0') {
	std_err("gres: null match string is silly\n");
	exit(2);
  }
  if ((exp = regcomp(*argp++)) == (regexp *) NULL) {
	std_err("gres: regcomp failed\n");
	exit(2);
  }
  repstr = *argp++;
  if (*argp == 0)
	process(stdin, exp, repstr);
  else
	while (*argp) {
		FILE *inf;

		if (strcmp(*argp, "-") == 0)
			process(stdin, exp, repstr);
		else {
			if ((inf = fopen(*argp, "r")) == (FILE *) NULL) {
				std_err("gres: Can't open ");
				std_err(*argp);
				std_err("\n");
				status = 2;
			} else {
				process(inf, exp, repstr);
				fclose(inf);
			}
		}
		argp++;
	}
  exit(status);
}

/* This routine does the processing. */
process(inf, exp, repstr)
FILE *inf;
regexp *exp;
char *repstr;
{
  char ibuf[MAXLINE];

  while (fgets(ibuf, MAXLINE, inf) != (char *) NULL) {
	char *cr = strchr(ibuf, '\n');
	if (cr == 0)
		std_err("gres: Line broken\n");
	else
		*cr = '\0';
	if (regexec(exp, ibuf, 1)) {
		pline(exp, ibuf, repstr);
		if (status != 2) status = 0;
	} else
		puts(ibuf);
  }
}

void regerror(s)
char *s;
{
  std_err("gres: ");
  std_err(s);
  std_err("\n");
  exit(2);
}

char *getbuf(exp, repstr)
regexp *exp;
char *repstr;
{
  char *malloc();
  void free();
  static bufsize = 0;
  static char *buf = 0;
  int guess = 10;
  int ch;

  while (*repstr) {
	switch (*repstr) {
	    case '&':
		guess += exp->endp[0] - exp->startp[0];
		break;
	    case '\\':
		if ((ch = *++repstr) < '0' || ch > '9')
			guess += 2;
		else {
			ch -= '0';
			guess += exp->endp[ch] - exp->startp[ch];
		}
		break;
	    default:	guess++;
	}
	repstr++;
  }
  if (bufsize < guess) {
	if (buf != 0) free((char *) buf);
	buf = malloc(guess);
  }
  return buf;
}

pline(exp, ibuf, repstr)
regexp *exp;
char ibuf[];
char *repstr;
{
  do {
	dosub(exp, ibuf, repstr);
	ibuf = exp->endp[0];
	if (*ibuf == '\0') break;
	if (ibuf == exp->startp[0]) putchar(*ibuf++);
  } while (!gflag && regexec(exp, ibuf, 0));
  puts(ibuf);
}

/* Print one subsitution. */
dosub(exp, ibuf, repstr)
regexp *exp;
char ibuf[];
char *repstr;
{
  char *buf = getbuf(exp, repstr);
  char *end = exp->startp[0];
  int ch = *end;

  *end = '\0';
  fputs(ibuf, stdout);		/* output the initial part of line */
  *end = ch;
  regsub(exp, repstr, buf);
  fputs(buf, stdout);
}
