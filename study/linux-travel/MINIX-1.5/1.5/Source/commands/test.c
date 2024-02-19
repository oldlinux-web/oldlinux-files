/* test - test conditions		Author: Erik Baalbergen */

/* Test-	version 7-like test(1).
 *
 * Grammar:	expr	::= bexpr | bexpr "-o" expr ;
 *		bexpr	::= primary | primary "-a" bexpr ;
 *		primary	::= unary-operator operand
 *			| operand binary-operator operand
 *			| operand
 *			| "(" expr ")"
 *			| "!" expr
 *			;
 *		unary-operator ::= "-r"|"-w"|"-f"|"-d"|"-s"|"-t"|"-z"|"-n";
 *		binary-operator ::= "="|"!="|"-eq"|"-ne"|"-ge"|"-gt"|"-le"|"-lt";
 *		operand ::= <any legal UNIX file name>
 *
 * Author:	Erik Baalbergen	erikb@cs.vu.nl
 *
 * History:	Fix Bert Reuling #571 (03/09/89 FVK)	bert@kyber.UUCP
 *
 * 		Add Jeroen van der Pluijm  09/25/89 jeroen@minixug.nluug.nl
 *		    Enabled linking to /usr/bin/[ so you cn use structures
 *		    like:
 *		 	   if [ -f ./test.c ]
 *		    and so on. Also added checking of argv[0] to see if it
 *		    is '[' and the last argument if it is ']'.
 *
 *		Mod Fred van Kempen 09/27/89 waltje@minixug.nluug.nl
 *		    Adapted source to MINIX Style Sheet.
 *
 *		Mod Bruce Evans 09/27/89 evans@ditsyda.oz.au
 *		    Allow whitespace in numbers.
 *		    Deleted bloat from single use of stdio (fprintf).
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sgtty.h>

#define EOI	0
#define FILRD	1
#define FILWR	2
#define FILND	3
#define FILID	4
#define FILGZ	5
#define FILTT	6
#define STZER	7
#define STNZE	8
#define STEQL	9
#define STNEQ	10
#define INTEQ	11
#define INTNE	12
#define INTGE	13
#define INTGT	14
#define INTLE	15
#define INTLT	16
#define UNEGN	17
#define BAND	18
#define BOR	19
#define LPAREN	20
#define RPAREN	21
#define OPERAND	22

#define UNOP	1
#define BINOP	2
#define BUNOP	3
#define BBINOP	4
#define PAREN	5

struct op {
  char *op_text;
  short op_num, op_type;
} ops[] = {

  {
	"-r", FILRD, UNOP
  },
  {
	"-w", FILWR, UNOP
  },
  {
	"-f", FILND, UNOP
  },
  {
	"-d", FILID, UNOP
  },
  {
	"-s", FILGZ, UNOP
  },
  {
	"-t", FILTT, UNOP
  },
  {
	"-z", STZER, UNOP
  },
  {
	"-n", STNZE, UNOP
  },
  {
	"=", STEQL, BINOP
  },
  {
	"!=", STNEQ, BINOP
  },
  {
	"-eq", INTEQ, BINOP
  },
  {
	"-ne", INTNE, BINOP
  },
  {
	"-ge", INTGE, BINOP
  },
  {
	"-gt", INTGT, BINOP
  },
  {
	"-le", INTLE, BINOP
  },
  {
	"-lt", INTLT, BINOP
  },
  {
	"!", UNEGN, BUNOP
  },
  {
	"-a", BAND, BBINOP
  },
  {
	"-o", BOR, BBINOP
  },
  {
	"(", LPAREN, PAREN
  },
  {
	")", RPAREN, PAREN
  },
  {
	0, 0, 0
  }
};


char **ip;
char *prog;
struct op *ip_op;


void syntax()
{
  write(2, prog, strlen(prog));
  write(2, ": syntax error\n", 15);
  exit(1);
}


long num(s)
register char *s;
{
  long l = 0;
  long sign = 1;

  while (*s == ' ' || *s == '\t') ++s;
  if (*s == '\0') syntax();
  if (*s == '-') {
	sign = -1;
	s++;
  }
  while (*s >= '0' && *s <= '9') l = l * 10 + *s++ - '0';
  while (*s == ' ' || *s == '\t') ++s;
  if (*s != '\0') syntax();
  return(sign * l);
}


int filstat(nm, mode)
char *nm;
int mode;
{
  struct stat s;
  struct sgttyb t;

  switch (mode) {
      case FILRD:
	return(access(nm, 4) == 0);
      case FILWR:
	return(access(nm, 2) == 0);
      case FILND:
	return(stat(nm, &s) == 0 && ((s.st_mode & S_IFMT) != S_IFDIR));
      case FILID:
	return(stat(nm, &s) == 0 && ((s.st_mode & S_IFMT) == S_IFDIR));
      case FILGZ:
	return(stat(nm, &s) == 0 && (s.st_size > 0L));
      case FILTT:
	return(ioctl((int) num(nm), TIOCGETP, &t) == 0);
  }
}


int lex(s)
register char *s;
{
  register struct op *op = ops;

  if (s == 0) return EOI;
  while (op->op_text) {
	if (strcmp(s, op->op_text) == 0) {
		ip_op = op;
		return(op->op_num);
	}
	op++;
  }
  ip_op = (struct op *) 0;
  return(OPERAND);
}


int primary(n)
int n;
{
  register char *opnd1, *opnd2;
  int res;

  if (n == EOI) syntax();
  if (n == UNEGN) return (!expr(lex(*++ip)));
  if (n == LPAREN) {
	res = expr(lex(*++ip));
	if (lex(*++ip) != RPAREN) syntax();
	return(res);
  }
  if (n == OPERAND) {
	opnd1 = *ip;
	(void) lex(*++ip);
	if (ip_op && ip_op->op_type == BINOP) {
		struct op *op = ip_op;

		if ((opnd2 = *++ip) == (char *) 0) syntax();

		switch (op->op_num) {
		    case STEQL:
			return(strcmp(opnd1, opnd2) == 0);
		    case STNEQ:
			return(strcmp(opnd1, opnd2) != 0);
		    case INTEQ:
			return(num(opnd1) == num(opnd2));
		    case INTNE:
			return(num(opnd1) != num(opnd2));
		    case INTGE:
			return(num(opnd1) >= num(opnd2));
		    case INTGT:
			return(num(opnd1) > num(opnd2));
		    case INTLE:
			return(num(opnd1) <= num(opnd2));
		    case INTLT:
			return(num(opnd1) < num(opnd2));
		}
	}
	ip--;
	return(strlen(opnd1) > 0);
  }

  /* Unary expression */
  if (ip_op->op_type != UNOP || *++ip == 0) syntax();
  if (n == STZER) return (strlen(*ip) == 0);
  if (n == STNZE) return (strlen(*ip) != 0);
  return(filstat(*ip, n));
}


int bexpr(n)
int n;
{
  int res;

  if (n == EOI) syntax();
  res = primary(n);
  if (lex(*++ip) == BAND) return(bexpr(lex(*++ip)) && res);
  ip--;
  return(res);
}


int expr(n)
int n;
{
  int res;

  if (n == EOI) syntax();
  res = bexpr(n);
  if (lex(*++ip) == BOR) return(expr(lex(*++ip)) || res);
  ip--;
  return(res);
}


main(argc, argv)
int argc;
char **argv;
{
  if (argv[0][0] == '[' && argv[argc - 1][0] != ']') {
	write(2, "test: ] missing\n", 16);
	exit(1);
  }
  if (argv[0][0] == '[') argc--;

  if (argc == 1) exit(1);
  prog = argv[0];
  ip = &argv[1];
  exit(!expr(lex(*ip)));
}
