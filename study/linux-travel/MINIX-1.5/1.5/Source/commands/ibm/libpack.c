/* libpack - pack ASCII assembly code */

/* Packs common strings found in PC Minix assembly code into single	*/
/* byte abbreviations.  The abbreviations all have their high bit set.	*/
/* Do not alter the order of the entries in "table"; asld depends on it.*/

/* External interfaces */
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

/* Constants */
#define MAXLINE	256			/* maximum input line length */
#define ABBREVS	128			/* number of abbreviations */
#define COMMENT '|'			/* comment character */

/* Structures and types */
struct node {
  char *string;				/* pointer to original string */
  size_t length;			/* length of original string */
  struct node *next;			/* next node with same hash value */
};
typedef unsigned short TWOBYTES;	/* CHEAT - used to return two bytes */

/* Local interfaces */
static void error_exit(/* int rc, char *msg */);
static void hash_init(/* void */);		/* initializes hash table */
static void pack_line(/* char *line */);	/* packs an input line */
static TWOBYTES abbreviate_string(/* char *s */); /* abbreviates a string */

/* Macros */
#define HASH(word) ((*(word) + *((word) + 1)) & 0x7f)
#define HIBYTE(n)  ((n) >> 8)
#define LOBYTE(n)  ((n) & 0xff)
#define MAKETWOBYTES(hi, lo)  ((((hi) & 0xff) << 8) | ((lo) & 0xff))

/* Static storage */
static char *table[] = {
"push ax",
"ret",
"mov bp,sp",
"push bp",
"pop bp",
"mov sp,bp",
".text",
"xor ax,ax",
"push 4(bp)",
"pop bx",
"pop si",
"cbw",
"movb al,(bx)",
"pop ax",
"xorb ah,ah",
"mov ax,#1",
"call _callm1",
"add sp,#16",
"mov bx,4(bp)",
"push 6(bp)",
"mov -2(bp),ax",
"I0013:",
"call .cuu",
"mov ax,-2(bp)",
"add 4(bp),#1",
"or ax,ax",
"jmp I0011",
"mov bx,8(bp)",
"push dx",
"mov cx,#2",
"mov bx,#2",
"I0011:",
"I0012:",
"push -2(bp)",
"mov ax,4(bp)",
"mov ax,-4(bp)",
"add sp,#6",
"and ax,#255",
"push bx",
"mov bx,-2(bp)",
"loop 2b",
"jcxz 1f",
".word 4112",
"mov ax,(bx)",
"mov -4(bp),ax",
"jmp I0013",
".data",
"mov bx,6(bp)",
"mov (bx),ax",
"je I0012",
".word 8224",
".bss",
"mov ax,#2",
"call _len",
"call _callx",
".word 28494",
".word 0",
"push -4(bp)",
"movb (bx),al",
"mov bx,ax",
"mov -2(bp),#0",
"I0016:",
".word 514",
".word 257",
"mov ",
"push ",
".word ",
"pop ",
"add ",
"4(bp)",
"-2(bp)",
"(bx)",
".define ",
".globl ",
"movb ",
"xor ",
"jmp ",
"cmp ",
"6(bp)",
"-4(bp)",
"-6(bp)",
"#16",
"_callm1",
"call ",
"8(bp)",
"xorb ",
"and ",
"sub ",
"-8(bp)",
"jne ",
".cuu",
"lea ",
"inc ",
"_M+10",
"#255",
"loop",
"jcxz",
"ax,#",
"bx,#",
"cx,#",
"ax,",
"bx,",
"cx,",
"dx,",
"si,",
"di,",
"bp,",
"ax",
"bx",
"cx",
"dx",
"si",
"di",
"bp",
"sp",
"dec ",
"neg ",
"_execve",
",#0",
NULL
};

/* This table is used to look up strings.  */

struct node node[ABBREVS];

struct node *hash[ABBREVS];		/* hash table */

/* Code */
int main(argc, argv)
int argc;
char *argv[];
{
  char line[MAXLINE];

  switch (argc) {
  case 1:
	break;
  case 2:
	if (freopen(argv[1], "r", stdin) == NULL)
		error_exit(ENOENT, "libpack: could not open file\n");
	break;
  default:
	error_exit(EINVAL, "Usage: libpack [file]\n");
  }

  hash_init();
  while (fgets(line, MAXLINE, stdin) != NULL) {
	pack_line(line);
	fputs(line, stdout);
  }

  exit(0);
}

static void error_exit(rc, msg)
int rc;
char *msg;
{
  fputs(msg, stderr);
  exit(rc);
}

static void hash_init()
{
  int i;
  register char **p;
  register struct node *np;

  for (i = 0, p = table; i < ABBREVS && *p != NULL; i++, p++) {
  	if (hash[HASH(*p)] == NULL) {
		hash[HASH(*p)] = &node[i];
	} else {
		for (np = hash[HASH(*p)]; np->next != NULL; np = np->next)
			;
		np->next = &node[i];
	}
	np = &node[i];
  	np->string = *p;
	np->length = strlen(*p);
  	np->next = NULL;
  }
}

static void pack_line(line)
char *line;
{
  register char *ip;
  register char *op;
  int in_white;
  TWOBYTES n;

  for (ip = line; *ip != '\0'; ++ip)		/* translate tabs */
	if (*ip == '\t') *ip = ' ';

  op = line;
  in_white = 1;
  for (ip = line; *ip != '\0'; ++ip) {		/* pack strings in line */
	if (*ip == COMMENT)
		break;
	if (isspace(*ip)) {
		if (!in_white) {
			*op++ = ' ';
			in_white = 1;
		}
		continue;
	}
	in_white = 0;
	n = abbreviate_string(ip);
	*op++ = LOBYTE(n);
	ip += HIBYTE(n);
  }

  if (op != line) {				/* finish nicely */
	if (op[-1] == ' ')
		--op;
	*op++ = '\n';
  }
  *op = '\0';
}

static TWOBYTES abbreviate_string(ip)
register char *ip;
{
  register struct node *np;

  for (np = hash[HASH(ip)]; np != NULL; np = np->next)
	if (strncmp(ip, np->string, np->length) == 0)
		return MAKETWOBYTES(np->length - 1, 128 + (np - node));

  return MAKETWOBYTES(0, *ip);
}
