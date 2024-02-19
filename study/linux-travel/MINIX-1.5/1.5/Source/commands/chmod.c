/* chmod - change mode		Author: James da Silva */


/* Author James da Silva (ihnp4!killer!jaime)
 *
 *  a (hopefully) 7th Edition Unix compatible chmod for Minix.
 */

#include <sys/types.h>
#include <sys/stat.h>

#define isop(c)         ((c=='+')||(c=='-')||(c=='='))
#define isperm(c)       ((c=='r')||(c=='w')||(c=='x')||(c=='s')||(c=='t')||\
                         (c=='u')||(c=='g')||(c=='o'))

/* The bits associated with user, group, other */
#define U_MSK   (0700 | S_ISUID)
#define G_MSK   (0070 | S_ISGID)
#define O_MSK    0007

typedef unsigned short bitset;	/* type used for modes */

struct stat st;			/* structure returned by stat() */
char *pname, *arg;
bitset newmode, absolute(), symbolic();
int isabsolute;

main(argc, argv)
int argc;
char **argv;
{
  int i;

  pname = *(argv++);
  if (argc < 3) usage();
  arg = *argv;			/* save pointer to mode arg */

  /* Check for octal mode */
  if (isabsolute = ((*arg >= '0') && (*arg <= '7'))) newmode = absolute();

  /* Apply the mode to all files listed */
  for (i = 2; i < argc; i++) {
	argv++;
	if (stat(*argv, &st)) {	/* get current file mode */
		printf("%s: cannot find `%s'\n", pname, *argv);
		exit(1);
	}

	/* Calculate new mode for this file */
	if (!isabsolute) newmode = symbolic(st.st_mode);

	if (chmod(*argv, newmode)) {	/* change the mode */
		printf("%s: cannot chmod `%s'\n", pname, *argv);
		exit(1);
	}
  }
  exit(0);
}


/* Absolute interprets an octal mode.
 * The file modes will be set to this value.
 */
bitset absolute()
{
  bitset m;
  char *s;

  m = 0;
  s = arg;

  /* Convert octal string to integer */
  while ((*s >= '0') && (*s <= '7')) m = m * 8 + (*(s++) - '0');

  /* If something else is there, choke */
  if (*s) badmode(s);

  return m;
}


/* Symbolic
 *
 * Processes symbolic mode of the form (in EBNF):
 *      <symbolic> ::= <pgroup> { ',' <pgroup> }.
 *      <pgroup> ::= [ <who> ] <op> <permissions> { <op> <permissions> }.
 *
 *      <who> ::= <whoch> { <whoch> }.
 *      <whoch> ::= 'a' | 'u' | 'g' | 'o'.
 *
 *      <op> ::= '+' | '-' | '='.
 *
 *      <permissions> ::= <permch> { <permch> }.
 *      <permch> ::= 'r' | 'w' | 'x' | 's' | 't' | 'u' | 'g' | 'o'.
 *
 * If <who> is omitted, 'a' is assumed, BUT umask()ed bits are uneffected.
 * If <op> is '=', all unspecified permissions are turned off for this <who>.
 * For permissions 'u', 'g', and 'o', the permissions are taken from the
 * specified set.  i.e.  o=g sets the permissions for other the same as for
 * group.
 *
 * Pain in the duff, isn't it?
 */
bitset symbolic(mode)
bitset mode;
{
  int g, o, u, haswho, haspcopy;
  bitset u_mask, emask, partial, other, applyop();
  char *s, c, op;

  s = arg;
  u_mask = umask(0);		/* get the umasked bits */

  do {				/* pgroup */
	haswho = u = g = o = 0;

	while (!isop(*s)) {
		/* We must have a 'who' then */
		haswho = 1;
		switch (*s) {
		    case 'a':	u = g = o = 1;	break;
		    case 'u':	u = 1;	break;
		    case 'g':	g = 1;	break;
		    case 'o':	o = 1;	break;

		    default:	badmode(s);
		}
		s++;
	}

	if (!haswho) {
		u = g = o = 1;	/* assume all */
		emask = ~u_mask;/* effective umask */
	} else
		emask = ~0;


	/* Process each given operator */
	while (isop(*s)) {
		op = *(s++);
		other = partial = haspcopy = 0;

		/* Collect the specified permissions */

		while (isperm(*s)) {

			/* Berkeley only allows one of 'u' 'g' or 'o'
			 * as permissions */

			if ((*s == 'u') || (*s == 'g') || (*s == 'o'))
				if (haspcopy)
					badmode(s);
				else
					haspcopy = 1;

			switch (*s) {
			    case 'r':
				partial |= 4;
				break;
			    case 'w':
				partial |= 2;
				break;
			    case 'x':
				partial |= 1;
				break;

			    case 'u':
				partial |= (mode & U_MSK & ~S_ISUID) >> 6;
				other |= mode & S_ISUID;
				break;
			    case 'g':
				partial |= (mode & G_MSK & ~S_ISGID) >> 3;
				other |= mode & S_ISGID;
				break;
			    case 'o':
				partial |= (mode & O_MSK);
				break;

#ifdef S_ISVTX
			    case 't':
				other |= S_ISVTX;
				break;
#endif

			    case 's':
				if (u) other |= S_ISUID;
				if (g) other |= S_ISGID;
				break;

			    default:	badmode(s);
			}
			s++;
		}

		/* Apply the op using the affected bits and masks */
		if (u)
			mode = applyop(mode, op, (other | (partial << 6)), emask, U_MSK);
		if (g)
			mode = applyop(mode, op, (other | (partial << 3)), emask, G_MSK);
		if (o)
			mode = applyop(mode, op, (other | partial), emask, O_MSK);
	}

  } while (*(s++) == ',');

  /* Not at end - choke */

  if (*(--s)) badmode(s);

  return mode;
}


/* Applyop
 *
 * applies the operator to the current mode using the specified bitset
 * and mask.  'bits' will contain 1's in every bit affected by the
 * operator '+', '-', or '='.  In the case of '=', msk is used to
 * determine which bits will be forced off. 'emask' is the effective
 * umask.
 */
bitset applyop(mode, op, bits, emask, msk)
char op;
bitset mode, bits, emask, msk;
{
  switch (op) {
      case '+':
	mode |= bits & emask;	/* turn these bits on */
	break;
      case '-':
	mode &= ~(bits & emask);/* turn these off */
	break;
      case '=':
	mode |= bits & emask;	/* turn these bits on */
	mode &= ~(~bits & msk & emask);	/* others off */
	break;
      default:			/* should never get here (famous last words) */
	printf("%s: panic: bad op `%c' passed\n", pname, op);
  }
  return mode;
}


/* Usage
 *
 * Prints a terse usage message and exits.
 */
usage()
{
  printf("Usage: %s [absolute-mode | symbolic-mode] files\n", pname);
  exit(1);
}


/* Badmode
 *
 * Called when the parser chokes on the given mode.
 * Prints a message showing the offending character and exits.
 */
badmode(s)
char *s;
{
  int i, sp;
  char buffer[80], *bp;

  sp = s - arg + strlen(pname) + 21;
  sp = sp > 79 ? 79 : sp;	/* check for buffer overflow */

  for (i = 0, bp = buffer; i < sp; i++, bp++) *bp = ' ';
  *bp = '\0';

  printf("%s: badly formed mode `%s'\n", pname, arg);
  printf("%s^\n", buffer);
  exit(1);
}
