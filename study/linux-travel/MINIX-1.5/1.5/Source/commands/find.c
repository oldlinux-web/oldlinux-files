/* find - look for files satisfying a predicat	Author: Erik Baalbergen */

/*
   *** Check the switches in the SWITCHES section below. ***

   Differences from UNIX version 7 find(1):
  * -name: no name allowed; only uid
	e.g. find all core files: "find . -name core -a -print"
  * -xdev: do not cross file system boundaries

   The "-atime" may not work well on Minix.
   Please report bugs and suggestions to erikb@cs.vu.nl
*/


#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>

#define SHELL "/usr/bin/sh"

#define PLEN	256		/* maximum path length; overflows are not
			 * detected */
#define DIRSIZ	16		/* size of a directory entry */
#define MAXARG	256		/* maximum length for an argv */
#define NPATHS	256		/* maximum number of paths in path-list */
#define BSIZE  1024		/* bytes per block */

/*######################## DEFINITIONS ##############################*/
#define SECS_PER_DAY	(24L * 60L * 60L)	/* check your planet */

struct exec {
  int e_cnt;
  char *e_vec[MAXARG];
};

#define OP_NAME		1
#define OP_PERM		2
#define OP_TYPE		3
#define OP_LINKS	4
#define OP_USER		5
#define OP_GROUP	6
#define OP_SIZE		7
#define OP_INUM		8
#define OP_ATIME	9
#define OP_MTIME	10
#define OP_EXEC		11
#define OP_OK		12
#define OP_PRINT	13
#define OP_NEWER	14
#define OP_AND		15
#define OP_OR		16
#define OP_XDEV		17	/* do not cross file-system boundaries */

struct oper {
  char *op_str;
  int op_val;
} ops[] = {
  {	"name",		OP_NAME  },
  {	"perm",		OP_PERM  },
  {	"type",		OP_TYPE  },
  {	"links",	OP_LINKS  },
  {	"user",		OP_USER  },
  {	"group",	OP_GROUP  },
  {	"size",		OP_SIZE  },
  {	"inum",		OP_INUM  },
  {	"atime",	OP_ATIME  },
  {	"mtime",	OP_MTIME  },
  {	"exec",		OP_EXEC  },
  {	"ok",		OP_OK  },
  {	"print",	OP_PRINT  },
  {	"newer",	OP_NEWER  },
  {	"a",		OP_AND  },
  {	"o",		OP_OR  },
  {	"xdev",		OP_XDEV  },
  {	0, 0  }
};

#define EOI	-1
#define NONE	0
#define LPAR	20
#define RPAR	21
#define NOT	22

char *prog, *strcpy(), *Malloc(), *find_bin();

struct node {
  int n_type;			/* any OP_ or NOT */
  union {
	char *n_str;
	struct {
		long n_val;
		int n_sign;
	} n_int;
	struct exec *n_exec;
	struct {
		struct node *n_left, *n_right;
	} n_opnd;
  } n_info;
};
struct node *expr();

char **ipp;
int tty;			/* fd for /dev/tty when using -ok */
long current_time;
int xdev_flag = 0;
int devnr;

char *
 Malloc(n)
{
  char *malloc(), *m;

  if ((m = malloc(n)) == 0) fatal("out of memory", "");
  return m;
}

char *
 Salloc(s)
char *s;
{
  return strcpy(Malloc(strlen(s) + 1), s);
}

main(argc, argv)
char *argv[];
{
  char *pathlist[NPATHS];
  int pathcnt = 0;
  register i;
  struct node *pred;

  prog = *argv++;
  while (--argc > 0 && lex(*argv) == NONE) pathlist[pathcnt++] = *argv++;
  if (pathcnt == 0 || argc == 0)
	fatal("Usage: path-list predicate-list", "");
  ipp = argv;
  time(&current_time);
  pred = expr(lex(*ipp));
  if (lex(*++ipp) != EOI)
	fatal("syntax error: garbage at end of predicate", "");
  for (i = 0; i < pathcnt; i++) {
	if (xdev_flag) xdev_flag = 2;
	find(pathlist[i], pred, "");
  }
  exit(0);
}

find(path, pred, last)
char *path, *last;
struct node *pred;
{
  char spath[PLEN], ent[DIRSIZ + 1];
  struct stat st;
  register char *send = spath;
  FILE *fp, *fopen();

  if (path[1] == '\0' && *path == '/') {
	*send++ = '/';
	*send = '\0';
  } else
	while (*send++ = *path++) {
	}

  if (stat(spath, &st) == -1)
	nonfatal("can't get status of ", spath);
  else {
	switch (xdev_flag) {
	    case 0:
		break;
	    case 1:
		if (st.st_dev != devnr) return;
		break;
	    case 2:		/* set current device number */
		xdev_flag = 1;
		devnr = st.st_dev;
		break;
	}

	(void) check(spath, &st, pred, last);
	if ((st.st_mode & S_IFMT) == S_IFDIR) {
		if ((fp = fopen(spath, "r")) == NULL) {
			nonfatal("can't read directory ", spath);
			return;
		}
		send[-1] = '/';
		ent[DIRSIZ] = '\0';
		while (fread(ent, DIRSIZ, 1, fp) == 1) {
			if (!((*ent == '\0' && ent[1] == '\0')
			      || (ent[2] == '.') &&
			      (ent[3] == '\0'
			       || (ent[3] == '.' && ent[4] == '\0'))
			      )) {
				strcpy(send, ent + 2);
				find(spath, pred, ent + 2);
			}
		}
		fclose(fp);
	}
  }
}

check(path, st, n, last)
char *path, *last;
register struct stat *st;
register struct node *n;
{
  switch (n->n_type) {
      case OP_AND:
	return check(path, st, n->n_info.n_opnd.n_left, last) &&
		check(path, st, n->n_info.n_opnd.n_right, last);
      case OP_OR:
	return check(path, st, n->n_info.n_opnd.n_left, last) ||
		check(path, st, n->n_info.n_opnd.n_right, last);
      case NOT:
	return !check(path, st, n->n_info.n_opnd.n_left, last);
      case OP_NAME:
	return smatch(last, n->n_info.n_str);
      case OP_PERM:
	if (n->n_info.n_int.n_sign < 0)
		return st->st_mode == (int) n->n_info.n_int.n_val;
	return(st->st_mode & 0777) == (int) n->n_info.n_int.n_val;
      case OP_NEWER:
	return st->st_mtime > n->n_info.n_int.n_val;
      case OP_TYPE:
	return(st->st_mode & S_IFMT) == n->n_info.n_int.n_val;
      case OP_LINKS:
	return ichk((long) (st->st_nlink), n);
      case OP_USER:
	return st->st_uid == n->n_info.n_int.n_val;
      case OP_GROUP:
	return st->st_gid == n->n_info.n_int.n_val;
      case OP_SIZE:
	return ichk((st->st_size == 0) ? 0L :
		    ((st->st_size - 1) / BSIZE + 1), n);
      case OP_INUM:
	return ichk((long) (st->st_ino), n);
      case OP_ATIME:
	return ichk(st->st_atime, n);
      case OP_MTIME:
	return ichk(st->st_mtime, n);
      case OP_EXEC:
      case OP_OK:
	return execute(n->n_type, n->n_info.n_exec, path);
      case OP_PRINT:
	prints("%s\n", path);
	return 1;
      case OP_XDEV:	return 1;
}
  fatal("ILLEGAL NODE", "");
}

ichk(val, n)
long val;
struct node *n;
{
  switch (n->n_info.n_int.n_sign) {
      case 0:
	return val == n->n_info.n_int.n_val;
      case 1:
	return val > n->n_info.n_int.n_val;
      case -1:	return val < n->n_info.n_int.n_val;
}
  fatal("internal: bad n_sign", "");
}

lex(str)
char *str;
{
  if (str == 0) return EOI;
  if (*str == '-') {
	register struct oper *op;

	str++;
	for (op = ops; op->op_str; op++)
		if (strcmp(str, op->op_str) == 0) break;
	return op->op_val;
  }
  if (str[1] == 0) {
	switch (*str) {
	    case '(':
		return LPAR;
	    case ')':
		return RPAR;
	    case '!':	return NOT;
	}
  }
  return NONE;
}

struct node *
 newnode(t)
{
  struct node *n = (struct node *) Malloc(sizeof(struct node));

  n->n_type = t;
  return n;
}

/*########################### PARSER ###################################*/
/* Grammar:
  expr : primary | primary OR expr;
  primary : secondary | secondary AND primary | secondary primary;
  secondary : NOT secondary | LPAR expr RPAR | simple;
  simple : -OP args...
*/
struct node *expr(), *primary(), *secondary(), *simple();

number(str, base, pl, ps)
char *str;
long *pl;
int *ps;
{
  int up = '0' + base - 1;
  long val = 0;

  *ps = ((*str == '-' || *str == '+') ? ((*str++ == '-') ? -1 : 1) : 0);
  while (*str >= '0' && *str <= up) val = base * val + *str++ - '0';
  if (*str) fatal("syntax error: illegal numeric value", "");
  *pl = val;
}

struct node *
 expr(t)
{
  struct node *nd, *p, *nd2;

  nd = primary(t);
  if ((t = lex(*++ipp)) == OP_OR) {
	nd2 = expr(lex(*++ipp));
	p = newnode(OP_OR);
	p->n_info.n_opnd.n_left = nd;
	p->n_info.n_opnd.n_right = nd2;
	return p;
  }
  ipp--;
  return nd;
}

struct node *
 primary(t)
{
  struct node *nd, *p, *nd2;

  nd = secondary(t);
  if ((t = lex(*++ipp)) != OP_AND) {
	ipp--;
	if (t == EOI || t == RPAR || t == OP_OR) return nd;
  }
  nd2 = primary(lex(*++ipp));
  p = newnode(OP_AND);
  p->n_info.n_opnd.n_left = nd;
  p->n_info.n_opnd.n_right = nd2;
  return p;
}

struct node *
 secondary(t)
{
  struct node *n, *p;

  if (t == LPAR) {
	n = expr(lex(*++ipp));
	if (lex(*++ipp) != RPAR) fatal("syntax error, ) expected", "");
	return n;
  }
  if (t == NOT) {
	n = secondary(lex(*++ipp));
	p = newnode(NOT);
	p->n_info.n_opnd.n_left = n;
	return p;
  }
  return simple(t);
}

checkarg(arg)
char *arg;
{
  if (arg == 0) fatal("syntax error, argument expected", "");
}

struct node *
 simple(t)
{
  struct node *p = newnode(t);
  struct exec *e;
  struct stat est;
  long l;

  switch (t) {
      case OP_TYPE:
	checkarg(*++ipp);
	switch (**ipp) {
	    case 'b':
		p->n_info.n_int.n_val = S_IFBLK;
		break;
	    case 'c':
		p->n_info.n_int.n_val = S_IFCHR;
		break;
	    case 'd':
		p->n_info.n_int.n_val = S_IFDIR;
		break;
	    case 'f':
		p->n_info.n_int.n_val = S_IFREG;
		break;
	    default:
		fatal("-type needs b, c, d or f", "");
	}
	break;
      case OP_LINKS:
      case OP_USER:
      case OP_GROUP:
      case OP_SIZE:
      case OP_INUM:
      case OP_PERM:
	checkarg(*++ipp);
	number(*ipp, (t == OP_PERM) ? 8 : 10, &(p->n_info.n_int.n_val),
	       &(p->n_info.n_int.n_sign));
	break;
      case OP_ATIME:
      case OP_MTIME:
	checkarg(*++ipp);
	number(*ipp, 10, &l, &(p->n_info.n_int.n_sign));
	p->n_info.n_int.n_val = current_time - l * SECS_PER_DAY;
	/* More than n days old means less than the absolute time */
	p->n_info.n_int.n_sign *= -1;
	break;
      case OP_EXEC:
      case OP_OK:
	checkarg(*++ipp);
	e = (struct exec *) Malloc(sizeof(struct exec));
	e->e_cnt = 2;
	e->e_vec[0] = SHELL;
	p->n_info.n_exec = e;
	while (*ipp) {
		if (**ipp == ';' && (*ipp)[1] == '\0') {
			e->e_vec[e->e_cnt] = 0;
			break;
		}
		e->e_vec[(e->e_cnt)++] =
			(**ipp == '{' && (*ipp)[1] == '}'
		       && (*ipp)[2] == '\0') ? (char *) (-1) : *ipp;
		ipp++;
	}
	if (*ipp == 0) fatal("-exec/-ok: ; missing", "");
	if ((e->e_vec[1] = find_bin(e->e_vec[2])) == 0)
		fatal("can't find program ", e->e_vec[2]);
	if (t == OP_OK)
		if ((tty = open("/dev/tty", O_RDWR)) < 0)
			fatal("can't open /dev/tty", "");
	break;
      case OP_NEWER:
	checkarg(*++ipp);
	if (stat(*ipp, &est) == -1)
		fatal("-newer: can't get status of ", *ipp);
	p->n_info.n_int.n_val = est.st_mtime;
	break;
      case OP_NAME:
	checkarg(*++ipp);
	p->n_info.n_str = *ipp;
	break;
      case OP_XDEV:	xdev_flag = 1;	break;
      case OP_PRINT:
	break;
      default:
	fatal("syntax error, operator expected", "");
  }
  return p;
}

/*######################## DIAGNOSTICS ##############################*/

nonfatal(s1, s2)
char *s1, *s2;
{
  std_err(prog);
  std_err(": ");
  std_err(s1);
  std_err(s2);
  std_err("\n");
}

fatal(s1, s2)
char *s1, *s2;
{
  nonfatal(s1, s2);
  exit(1);
}

/*################### SMATCH #########################*/
/* Don't try to understand the following one... */
smatch(s, t)			/* shell-like matching */
char *s, *t;
{
  register n;

  if (*t == '\0') return *s == '\0';
  if (*t == '*') {
	++t;
	do
		if (smatch(s, t)) return 1;
	while (*s++ != '\0');
	return 0;
  }
  if (*s == '\0') return 0;
  if (*t == '\\') return (*s == *++t) ? smatch(++s, ++t) : 0;
  if (*t == '?') return smatch(++s, ++t);
  if (*t == '[') {
	while (*++t != ']') {
		if (*t == '\\') ++t;
		if (*(t + 1) != '-')
			if (*t == *s) {
				while (*++t != ']')
					if (*t == '\\') ++t;
				return smatch(++s, ++t);
			} else
				continue;
		if (*(t + 2) == ']') return(*s == *t || *s == '-');
		n = (*(t + 2) == '\\') ? 3 : 2;
		if (*s >= *t && *s <= *(t + n)) {
			while (*++t != ']')
				if (*t == '\\') ++t;
			return smatch(++s, ++t);
		}
		t += n;
	}
	return 0;
  }
  return(*s == *t) ? smatch(++s, ++t) : 0;
}

/*####################### EXECUTE ###########################*/
/* Do -exec or -ok */

char *epath = 0;

char *
 getpath()
{
  extern char **environ;
  register char **e = environ;

  if (epath)			/* retrieve PATH only once */
	return epath;
  while (*e) {
	if (strncmp("PATH=", *e, 5) == 0) {
		return epath = *e + 5;
	}
	e++;
  }
  fatal("can't get PATH from environment", "");
}

char *
 find_bin(s)
char *s;
{
  char *f, *l, buf[PLEN];

  if (*s == '/')		/* absolute path name */
	return(access(s, 1) == 0) ? s : 0;
  l = f = getpath();
  for (;;) {
	if (*l == ':' || *l == 0) {
		if (l == f) {
			if (access(s, 1) == 0) return Salloc(s);
			f++;
		} else {
			register char *p = buf, *q = s;

			while (f != l) *p++ = *f++;
			f++;
			*p++ = '/';
			while (*p++ = *q++) {
			}
			if (access(buf, 1) == 0) return Salloc(buf);
		}
		if (*l == 0) break;
	}
	l++;
  }
  return 0;
}

execute(op, e, path)
struct exec *e;
char *path;
{
  int s, pid;
  char *argv[MAXARG];
  register char **p, **q;

  for (p = e->e_vec, q = argv; *p;)	/* replace the {}s */
	if ((*q++ = *p++) == (char *) -1) q[-1] = path;
  *q = '\0';
  if (op == OP_OK) {
	char answer[10];

	for (p = &argv[2]; *p; p++) {
		write(tty, *p, strlen(*p));
		write(tty, " ", 1);
	}
	write(tty, "? ", 2);
	if (read(tty, answer, 10) < 2 || *answer != 'y') return 0;
  }
  if ((pid = fork()) == -1) fatal("can't fork", "");
  if (pid == 0) {
	register i = 3;

	while (close(i++) == 0) {
	}			/* wow!!! */
	execv(argv[1], &argv[2]);	/* binary itself? */
	execv(argv[0], &argv[1]);	/* shell command? */
	fatal("exec failure: ", argv[1]);	/* none of them! */
	exit(127);
  }
  return wait(&s) == pid && s == 0;
}
