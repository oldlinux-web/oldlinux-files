/* Ifdef - remove #ifdefs		Author: Warren Toomey */

/* Ifdef: Remove unwanted ifdefs from C code	      */
/* Written by Warren Toomey - 1989		      */
/* You may freely copy or give away this source as    */
/* Long as this notice remains intact.		      */
/* 9/16/89 - Added usage() before getopt() - meb      */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Definition of structures and constants used in ifdef.c  */

/* Types of symbols */
#define DEF	  1		/* Symbol is defined    */
#define UNDEF	  2		/* Symbol isn't defined */
#define IGN	  3		/* Ignore this symbol unless defined       */

/* Redef mode values */
#define MUTABLE   1		/* Symbol can change defined <-> undefined */
#define IMMUTABLE 2		/* Symbol can't change as above            */

/* Processing modes */
#define NO	0		/* Don't process */
#define YES	1		/* Process */

/* Ignore (IGN), ignore but process */
struct DEFINE {
  char *symbol;			/* SLL of defined symbols. The redef  */
  char type;			/* field indicates if this symbol can */
  char redef;			/* change from defined <-> undefined. */
  struct DEFINE *next;		/* Type is DEF or UNDEF.	      */
};

/* Global variables & structures */
FILE *zin;			/* Input file for processing  */
struct DEFINE *defptr;		/* Defined symbols SLL        */
struct DEFINE *defend;		/* Ptr to last node in defptr */
struct DEFINE *deftemp;		/* Ptr to last found node     */
int line = 1;			/* Current line number        */
int table = 0;			/* Don't normally want a table */

char fgetarg(stream, cbuf)	/* Get next arg from file into cbuf, */
FILE *stream;			/* returning the character that      */
char *cbuf;			/* terminated it. Cbuf returns NULL  */
{				/* if no arg. EOF is returned if no  */
  char ch;			/* args left in file.                */
  int i;

  i = 0;
  cbuf[i] = 0;

  while (((ch = fgetc(stream)) == ' ') || (ch == '\t') || (ch == '\n'))
	if (ch == '\n') return(ch);	/* Bypass leading */
  /* Whitespace     */
  if (feof(stream)) return(EOF);

  cbuf[i++] = ch;

  while (((ch = fgetc(stream)) != ' ') && (ch != '\t') && (ch != '\n'))
	cbuf[i++] = ch;		/* Get the argument */

  cbuf[i] = 0;
  return(ch);
}


find(sym)
char *sym;
{				/* Return DEF if defined else UNDEF */
  char found = 0;		/* IGN if symbol is ignored  */
  /* 0 if not in the list */

  deftemp = defptr;
  while (deftemp) {		/* Search for the symbol */
	if (!strcmp(deftemp->symbol, sym))
		return(deftemp->type);	/* Setting up the type */
	deftemp = deftemp->next;
  }
  return(0);
}



#define Define(x,y)	defit(x,y,DEF)
#define Undefine(x,y)	defit(x,y,UNDEF)
#define Ignore(x,y)	defit(x,y,IGN)

defit(sym, redef, type)		/* Add symbol to the define list */
char *sym;
char redef;			/* Mode: MUTABLE etc      */
char type;			/* Type: DEF, UNDEF, IGN  */
{
  struct DEFINE *temp;
  char c;

  c = find(sym);		/* First try finding the symbol */
  if (type == c) return;	/* Return if already declared */
  if (c) {			/* We have to move if from DEF <-> UNDEF */
	if (deftemp->redef == IMMUTABLE)
		return;
	else {
		deftemp->type = type;
		deftemp->redef = redef;
	}
  } else {			/* We must create a struct & add it */
	/* Malloc room for the struct */
	if ((temp = (struct DEFINE *) malloc(sizeof(struct DEFINE))) == NULL) {
		fprintf(stderr, "ifdef: could not malloc\n");
		exit(1);
	}

	/* Malloc room for symbol */
	if ((temp->symbol = (char *) malloc(strlen(sym) + 1)) == NULL) {
		fprintf(stderr, "ifdef: could not malloc\n");
		exit(1);
	}
	strcpy(temp->symbol, sym);	/* Copy symbol into struct      */
	temp->redef = redef;	/* and set its redef mode too   */
	temp->type = type;	/* as well as making it defined */


	/* Now add to the SLL */
	if (defptr == NULL)	/* If first node set  */
		defptr = temp;	/* the pointers to it */
	else
		defend->next = temp;	/* else add it to the */
	defend = temp;		/* end of the list.   */
  }
}



stop()
{				/* Stop: Tidy up at EOF */
  if (table) printtable();
  fclose(zin);
  exit(0);
}

#define Goto	{ line++; if (ch!='\n') gotoeoln(zin); }
#define Print	{ line++; if (ch!='\n')  prteoln(zin); }

gotoeoln(file)			/* Go to the end of the line */
FILE *file;
{
  int ch;
  while ((ch = fgetc(zin)) != '\n')
	if (ch == EOF) stop();
}


prteoln(file)			/* Print to the end of the line */
FILE *file;
{
  int ch;
  while ((ch = fgetc(zin)) != '\n')
	if (ch == EOF)
		stop();
	else
		putchar(ch);
  putchar('\n');
}


printtable()
{				/* Print the defines in the SLL */
  struct DEFINE *temp;

  printf("Defined\n\n");

  temp = defptr;
  while (temp) {
	if (temp->type == DEF) printf("%s\n", temp->symbol);
	temp = temp->next;
  }

  printf("\n\nUndefined\n\n");

  temp = defptr;
  while (temp) {
	if (temp->type == UNDEF) printf("%s\n", temp->symbol);
	temp = temp->next;
  }
}

getendif()
{				/* Find matching endif when ignoring */
  char word[80];		/* Buffer for symbols */
  int ch;
  int skip;			/* Number of skipped #ifdefs		 */

  skip = 1;

  while (1) {
	/* Scan through the file looking for starting lines */
	if ((ch = fgetc(zin)) == EOF)
		stop();		/* Get first char on the line */
	if (ch != '#') {	/* If not a # ignore line     */
		putchar(ch);
		Print;
		continue;
	}
	ch = fgetarg(zin, word);/* Get the word after the # */

	if (!strcmp(word, "ifdef")) skip++;	/* Keep track of ifdefs & */
	if (!strcmp(word, "endif")) skip--;	/* endifs		  */

	printf("#%s%c", word, ch);	/* Print the line out 	  */
	Print;
	if (!skip) return('\n');/* If matching endif, return */
  }
}


gettable()
{
/* Get & print a table of defines etc.  */

  char word[80];		/* Buffer for symbols */
  int ch;
  int proc;			/* Should we be processing this bit?    */
  int skip;			/* Number of skipped #ifdefs		 */

  proc = 1;
  skip = 0;

  while (1) {
	/* Scan through the file looking for starting lines */
	if ((ch = fgetc(zin)) == EOF)
		stop();		/* Get first char on the line */
	if (ch != '#') {	/* If not a # ignore line     */
		Goto;
		continue;
	}
	ch = fgetarg(zin, word);/* Get the word after the # */

	if (!strcmp(word, "define") && proc) {	/* Define: Define the */
		ch = fgetarg(zin, word);	/* symbol, and goto   */
		Define(word, MUTABLE);	/* the end of line    */
		Goto;
		continue;
	}
	if (!strcmp(word, "undef") && proc) {	/* Undef: Undefine the */
		ch = fgetarg(zin, word);	/* symbol, and goto    */
		Undefine(word, MUTABLE);	/* the end of line     */
		Goto;
		continue;
	}
	if (!strcmp(word, "ifdef")) {	/* Ifdef:            */
		if (!proc)	/* If not processing */
			skip++;	/* skip it           */
		else {
			ch = fgetarg(zin, word);	/* Get the symbol */
			if (find(word) != DEF) {
				Undefine(word, MUTABLE);	/* undefine it */
				proc = 0;	/* & stop processing */
			}
		}
		Goto;
		continue;
	}
	if (!strcmp(word, "ifndef")) {	/* Ifndef:	  */
		if (!proc)	/* If not processing */
			skip++;	/* skip the line     */
		else {
			ch = fgetarg(zin, word);	/* Get the symbol */
			if (find(word) == DEF)	/* If defined, stop */
				proc = 0;	/* processing       */
		}
		Goto;
		continue;
	}
	if (!strcmp(word, "else") && !skip) {	/* Else: Flip processing */
		proc = !proc;
		Goto;
		continue;
	}
	if (!strcmp(word, "endif")) {	/* Endif: If no skipped */
		if (!skip)	/* on, else decrement the */
			proc = 1;	/* number of skips        */
		else
			skip--;
		Goto;
		continue;
	}
	Goto;			/* else ignore the line */
  }
}



parse()
{				/* Parse & remove ifdefs from C source */
  char word[80];		/* Buffer for symbols */
  int ch;
  int proc;			/* Should we be processing this bit?    */
  int skip;			/* Number of skipped #ifdefs		 */

  proc = 1;
  skip = 0;

  while (1) {
	/* Scan through the file looking for starting lines */
	if ((ch = fgetc(zin)) == EOF)
		stop();		/* Get first char on the line */
	if (ch != '#')
		if (proc) {	/* If not # and  we're processing */
			putchar(ch);	/* then print the line */
			Print;
			continue;
		} else {
			Goto;	/* else just skip the line  */
			continue;
		}

	ch = fgetarg(zin, word);/* Get the word after the # */

	if (!strcmp(word, "define") && proc) {	/* Define: Define the */
		ch = fgetarg(zin, word);	/* symbol, and goto   */
		Define(word, MUTABLE);	/* the end of line    */
		printf("#define %s%c", word, ch);
		Print;
		continue;
	}
	if (!strcmp(word, "undef") && proc) {	/* Undef: Undefine the */
		ch = fgetarg(zin, word);	/* symbol, and goto    */
		Undefine(word, MUTABLE);	/* the end of line     */
		printf("#undef %s%c", word, ch);
		Print;
		continue;
	}
	if (!strcmp(word, "ifdef")) {	/* Ifdef: */
		if (!proc)	/* If not processing */
			skip++;	/* skip it           */
		else {
			ch = fgetarg(zin, word);	/* Get the symbol */
			switch (find(word)) {
			    case DEF:
				break;
			    case IGN:
				printf("#ifdef %s%c", word, ch);
				Print;
				ch = getendif();	/* Get matching endif */
				break;
				/* If symbol undefined */
			    default:
				Undefine(word, MUTABLE);	/* undefine it */
				proc = 0;	/* & stop processing */
			}
		}
		Goto;
		continue;
	}
	if (!strcmp(word, "ifndef")) {
		/* Ifndef: */
		if (!proc)	/* If not processing */
			skip++;	/* skip the line     */
		else {
			ch = fgetarg(zin, word);	/* Get the symbol */
			switch (find(word)) {	/* If defined, stop */
			    case DEF:
				proc = 0;	/* processing       */
				break;
			    case IGN:
				printf("#ifdef %s%c", word, ch);
				Print;
				ch = getendif();	/* Get matching endif */
				break;
			}
		}
		Goto;
		continue;
	}
	if (!strcmp(word, "else") && !skip) {	/* Else: Flip processing */
		proc = !proc;
		Goto;
		continue;
	}
	if (!strcmp(word, "endif")) {	/* Endif: If no skipped   *//* i
					 * fdefs turn processing */
		if (!skip)	/* on, else decrement the */
			proc = 1;	/* number of skips        */
		else
			skip--;
		Goto;
		continue;
	}
	if (proc) {
		/* The word fails all of the above tests, so if we're
		 * processing, print the line. */
		printf("#%s%c", word, ch);
		Print;
	} else
		Goto;
  }
}


usage()
{
  fprintf(stderr, "Usage: ifdef [-t] [-Dsymbol] [-dsymbol] [-Usymbol] [-Isymbol] <file>\n");
  exit(0);
}


main(argc, argv)
int argc;
char *argv[];
{
  extern int optind;
  extern char *optarg;

  char sym[80];			/* Temp symbol storage */
  int c;

  if (argc == 1) usage();	/* Catch the curious user	 */
  while ((c = getopt(argc, argv, "tD:d:U:I:")) != EOF) {
	switch (c) {
	    case 't':
		table = 1;	/* Get the various options */
		break;

	    case 'd':
		strcpy(sym, optarg);
		Define(sym, MUTABLE);
		break;

	    case 'D':
		strcpy(sym, optarg);
		Define(sym, IMMUTABLE);
		break;

	    case 'U':
		strcpy(sym, optarg);
		Undefine(sym, IMMUTABLE);
		break;

	    case 'I':
		strcpy(sym, optarg);
		Ignore(sym, IMMUTABLE);
		break;

	    default:	usage();
	}
  }

  zin = stdin;
  if (*argv[argc - 1] != '-') {	/* If a C file is named *//* Open
				 * stdin with it   */
	if ((zin = fopen(argv[argc - 1], "r")) == NULL) {
		perror("ifdef");
		exit(1);
	}
  }
  if (table)
	gettable();		/* Either generate a table or    */
  else
	parse();		/* parse & replace with the file */
}
