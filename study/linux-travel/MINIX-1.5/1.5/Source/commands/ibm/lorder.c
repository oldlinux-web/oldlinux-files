/* lorder - order a library		Author: Monty Walls */

/* lorder: find ordering relations for object library
 *
 * author:	Monty Walls
 * written:	1/29/88
 * Copyright:	Copyright (c) 1988 by Monty Walls.
 *		Not derived from licensed software.
 *
 *		Permission to copy and/or distribute granted under the
 *		following conditions:
 *
 *		1). This notice must remain intact.
 *		2). The author is not responsible for the consequences of use
 *			this software, no matter how awful, even if they
 *			arise from defects in it.
 *		3). Altered version must not be represented as being the
 *			original software.
 *
 * change log:
 *		corrected & rewrote scanner to avoid lex. - 2/22/88 - mrw
 *		oops reversed output filename order. 3/14/88 - mrw
 *		progname = argv[0] - should be first. 5/25/88 - mbeck
 */

#include <ctype.h>
#include <signal.h>
#include <ar.h>
#include <stdio.h>

#define MAXLINE		256

FILE *lexin;
char *yyfile;
char *tempfile;
char *progname;
char template[] = "lorder.XXXXXX";

struct filelist {
  char *name;
  struct filelist *next;
};

struct node {
  char *name;
  char *file;
  struct filelist *list;
  struct node *left, *right;
};

struct filelist *list;
struct node *tree, *lastnode;
extern char *malloc(), *mktemp();
extern FILE *popen(), *fopen();
extern char *addfile();
extern void user_abort();

main(argc, argv)
int argc;
char **argv;
{
  int i;
  char cmdstr[MAXLINE];

  progname = argv[0];
  if (argc > 1) {
	signal(SIGINT, user_abort);
	for (i = 1; argv[i] && *argv[i]; ++i) {
		/* The following code is caused by not enough memory
		 * on floppy systems.
		 * 
		 * so instead of ar | libupack ->to us. we use ar
		 * >tmpfle; libupack <tempfile ->to us */
		if (is_liba(argv[i])) {
			tempfile = mktemp(template);
			sprintf(cmdstr, "ar pv %s >%s", argv[i], tempfile);
			system(cmdstr);
			sprintf(cmdstr, "libupack <%s", tempfile);
		} else {
			yyfile = addfile(argv[i]);
			sprintf(cmdstr, "libupack <%s", argv[i]);
		}
		if ((lexin = popen(cmdstr, "r")) != (FILE *) NULL) {
			while (yylex() != EOF);
			pclose(lexin);
			if (tempfile) unlink(tempfile);
		} else {
			fprintf(stderr, "Error: %s could not open %s\n", progname, argv[i]);
			exit(1);
		}
	}
	printtree(tree);
	/* Then print list of files for ar also */
	for (; list; list = list->next)
		fprintf(stdout, "%s %s\n", list->name, list->name);
  } else {
	fprintf(stderr, "Usage: %s file ....\n", progname);
	exit(1);
  }
}

void user_abort()
{
  unlink(tempfile);
  exit(1);
}

char *xalloc(n)
int n;
{
  char *p;

  if ((p = malloc(n)) == (char *) NULL) {
	fprintf(stderr, "Error %s - out of memory\n", progname);
	exit(1);
  }
  return(p);
}

int is_liba(s)			/* error handling done later */
char *s;
{
  unsigned short key;
  FILE *fp;
  int ret = 0;

  if ((fp = fopen(s, "r")) != (FILE *) NULL) {
	fread(&key, sizeof(key), 1, fp);
	if (key == ARMAG) ret = 1;
	fclose(fp);
  }
  return(ret);
}

char *strsave(s)
char *s;
{
  char *p;

  p = xalloc(strlen(s) + 1);
  strcpy(p, s);
  return(p);
}

char *addfile(s)
char *s;
{
  struct filelist *p;

  p = (struct filelist *) xalloc(sizeof(struct filelist));
  p->name = strsave(s);
  if (list)
	p->next = list;
  else
	p->next = NULL;
  list = p;
  return(p->name);
}

printtree(t)
struct node *t;
{
  struct filelist *fp;

  if (t) {
	if (t->file) {
		for (fp = t->list; fp && fp->name; fp = fp->next)
			if (t->file != fp->name)
				fprintf(stdout, "%s %s\n", fp->name, t->file);
	}
	printtree(t->right);
	printtree(t->left);
  }
}


struct node *finddef(s)
char *s;
{
  struct node *n;
  int cmp;

  if (tree) {
	lastnode = n = tree;
	while (n && n->name) {
		lastnode = n;
		if (!(cmp = strcmp(s, n->name)))
			return(n);
		else if (cmp > 0)
			n = n->left;
		else
			n = n->right;
	}
  }
  return((struct node *) NULL);
}

struct node *makedef(s)
char *s;
{
  struct node *n;
  int cmp;

  n = (struct node *) xalloc(sizeof(struct node));
  n->name = strsave(s);
  n->left = (struct node *) NULL;
  n->right = (struct node *) NULL;
  if (tree) {
	cmp = strcmp(s, lastnode->name);
	if (cmp > 0)
		lastnode->left = n;
	else
		lastnode->right = n;
  } else
	tree = n;

  return(n);
}

void dodef(s)
char *s;
{
  struct node *n;

  if (n = finddef(s)) {
	if (n->file != NULL) fprintf(stderr,
			"Error %s - %s defined twice in %s and %s\n",
			progname, s, n->file, yyfile);
	else
		n->file = yyfile;
  } else {
	n = makedef(s);
	n->file = yyfile;
	n->list = (struct filelist *) NULL;
  }
}

void usedef(s)
char *s;
{
  struct node *n;
  struct filelist *fp, *lastfp;

  if (n = finddef(s)) {
	/* Scan file list for match */
	if (n->list) {
		for (fp = n->list; fp; fp = fp->next) {
			if (fp->name == yyfile) {
				return;
			}
			lastfp = fp;
		}

		/* Reached here with no match */
		lastfp->next = (struct filelist *) xalloc(sizeof(struct filelist));
		lastfp->next->name = yyfile;
		lastfp->next->next = (struct filelist *) NULL;
	} else {
		/* Empty list so far */
		n->list = (struct filelist *) xalloc(sizeof(struct filelist));
		n->list->name = yyfile;
		n->list->next = (struct filelist *) NULL;
	}
  } else {
	n = makedef(s);
	n->file = (char *) NULL;
	n->list = (struct filelist *) xalloc(sizeof(struct filelist));
	n->list->name = yyfile;
	n->list->next = (struct filelist *) NULL;
  }
}

/* Yylex - scanner for lorder
 *
 */
#define MAXNAME 33
#define is_first_char(c)	((c) == '.' || (c) == '_')
#define is_second_char(c)	((c) == '_' || isalpha((c)))
#define is_other_char(c)	((c) == '_' || isalnum((c)))
#define alt_space(c)		((c) == ',' || (c) == '#')
int yylex()
{
  int col = 0;
  int i = 0;
  int is_member = 0;
  int in_define = 0;
  int lastch = 0;
  int was_space = 1;
  char s[MAXNAME];


  while ((lastch = fgetc(lexin)) != EOF) {
	col++;			/* increment col */
	if (isspace(lastch)) {
  EOS:				/* eos comes here */
		if (i) {	/* we have a string */
			s[i] = '\0';	/* set eos */
			i = 0;
			/* If we are in a define use dodef to add
			 * location of defining member and global to
			 * symbol table. */
			if (in_define) dodef(s);
			/* If we are on a 'p -' line for an ar lib
			 * define this member as the file we are
			 * using. */
			else if (is_member > 0) {
				is_member = 0;
				yyfile = addfile(s);
			}

			/* If we have a '.define' mark this line as
			 * in_define. */
			else if (strcmp(s, ".define") == 0)
				in_define = 1;
			/* Just a reference in the code to a var, so
			 * add this reference to our symbol table. */
			else
				usedef(s);
		}

		/* We are at the eol: reset our counters and switches */
		if (lastch == '\n') {
			col = 0;
			is_member = 0;
			in_define = 0;
		}

		/* Lets do another character */
		was_space = 1;
		continue;
	}

	/* Not a space and i == 0 */
	if (i == 0) {
		/* Are we seeing 'p' in col 1 */
		if (lastch == 'p' && col == 1) {
			is_member = -1;
		}

		/* Are we seeing '-' that follows 'p' in col 1 */
		else if (lastch == '-' && is_member < 0 && col == 3) {
			is_member = 1;
		}

		/* If we have seen 'p -' now we are reading the name
		 * or the first character of a global symbol */
		else if (is_member > 0 || (is_first_char(lastch) && was_space)) {
			s[i++] = lastch;
			if (is_member < 0) is_member = 0;
		}
		was_space = alt_space(lastch);
	}

	/* Do the second char of a name */
	else if (i == 1) {
		if (is_member > 0 || is_second_char(lastch)) {
			s[i++] = lastch;
		} else
			is_member = 0;
			was_space = alt_space(lastch);
	}

	/* Do the rest of a symbol or member name */
	else if (is_member > 0 || is_other_char(lastch)) {
		s[i++] = lastch;
		was_space = alt_space(lastch);
	} else {
		was_space = alt_space(lastch);
		goto EOS;
	}
  }

  /* Returns EOF on end of file */
  return(lastch);
}
