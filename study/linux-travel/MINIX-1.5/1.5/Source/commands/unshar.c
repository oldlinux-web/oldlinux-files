/* unshar - extract files from a shell archive	Author: Warren Toomey */


/* Unshar - extract files from shell archive
 *
 * Written by Warren Toomey [wkt@csadfa.oz.au@munnari.oz@uunet.uu.net] You may
 * freely copy or give away this source as long as this notice remains
 * intact.
 *
 * Definitions used by unshar
 */


#include <stdio.h>

/* Methods of unsharing */
#define UNKNOWN	0
#define BRUTAL	1

/* Whitespace indicators */
#define WHITE	0
#define NOWHITE 1

/* Leading character indicators */
#define NOX	0
#define YESX	1

/* Emulation types available */

#define NUMTOKS    4		/* Must change NUMTOKS to equal the */
 /* Define UNKNOWN  0 *//* number of emulation types */
#define SED	   1
#define GRES 	   2
#define CAT	   3

/* The list of emulation types. */
static char *token[NUMTOKS]=
{
  "",
  "sed",
  "gres",
  "cat"
};


/* Misc. constants */
#define BUFSIZE	512		/* Size of line buffer */

/* Global variables */
int table;			/* Generate a table, or extract */
int verbose;			/* Unshar verbosely - debugging */
int numext;			/* Number of files to extract */
int binfile;			/* Binary file - err indicator */
char *exfile[100];		/* Files to extract */


#define getline(x,y)	fgetline(stdin,x,y)

int fgetline(zin, how, buf)	/* Get a line from a file */
FILE *zin;
int how;			/* Ignore leading whitespace if */
char *buf;			/* how == NOWHITE */
{
  int ch = 0;

  *buf = 0;			/* Null the buffer */
  if (how == NOWHITE) {		/* If skip any whitespace */
	while (((ch = fgetc(zin)) == ' ') || (ch == '\t'));
	if (ch == EOF) return(EOF);	/* Returning EOF or 0 */
	if (ch == '\n') return (0);
	*buf++ = ch;		/* Put char in buffer */
  }
  while ((ch = fgetc(zin)) != '\n') {	/* Now get the line */
	if (ch == EOF) {
		*buf = 0;
		return(EOF);
	}
	if (ch > 127) {
		binfile = 1;
		return(0);
	}
	*buf++ = ch;
  }

  *buf = 0;			/* Finally null-terminate the buffer */
  return(0);			/* and return */
}



char *getstring(buf)		/* Get the next string from the buffer */
char *buf;			/* ignoring any quotes */
{
  char out[BUFSIZE];
  char *temp = out;
  char inquotes = 0, ok = 1;
  while ((*buf == ' ') || (*buf == '\t'))
	buf++;			/* Skip whitespace */

  if (verbose) printf("In getstring...\n");
  *temp = 0;
  while (ok) {			/* Parse line */
	switch (*buf) {
	    case '\"':
	    case '\'':
		buf++;
		inquotes = !inquotes;	/* Toggle inquotes */
		break;
	    case 0:
	    case '\n':		/* Stop on <, >, NULL */
	    case '>':		/* \n, and sometimes */
	        case '<':	ok = 0;	break;	/* space & tab */
	    case '\t':
	    case ' ':
		if (!inquotes) ok = 0;
	    case '\\':
		if (!inquotes) {/* Ignore backquotes */
			buf++;
			break;
		}
	    default:
		*temp++ = *buf++;	/* Copy chars :-) */
	}
  }
  *temp = 0;
  if (verbose) printf("Returning *%s*\n", out);
  return(out);
}


int firstword(buf)			/* Return token value of first word */
char *buf;			/* in the buffer. Assume no leading */
{				/* whitespace in the buffer */
  int i;

  for (i = 1; i < NUMTOKS; i++)
	if (strncmp(buf, token[i], strlen(token[i])) == 0) return(i);

  return(UNKNOWN);
}


int mustget(s1)			/* Return 1 if s1 is in the list of  */
char *s1;			/* files to extract. Return 0 if not */
{
  int i;

  if (numext == 0) return(0);
  for (i = 0; i < numext; i++)
	if (!strcmp(s1, exfile[i])) return(1);
  return(0);
}


void extract(how, file, end, lead)	/* Extract file, up until end word */
int how;			/* If how==YESX, then ignore lead   */
char *file;			/* character on every line */
char *end;
int lead;
{
  FILE *zout;
  char line[BUFSIZE];
  char *temp;
  int ch;

  zout = fopen(file, "w");	/* Open output file */
  if (zout == NULL) {
	perror("unshar1");
	return;
  }
  while (1) {
	binfile = 0;
	ch = getline(WHITE, line);	/* Get a line of file */
	temp = line;
	if (binfile || (ch == EOF)) {
		fprintf(zout, "%s\n", line);
		fclose(zout);
		return;
	}
	if ((how == YESX) && (*temp == lead)) temp++;	/* Skip any lead */

	if (strcmp(temp, end) == 0) {	/* If end word */
		fclose(zout);	/* close the file */
		return;
	}
	fprintf(zout, "%s\n", temp);
  }
}


void getnames(buf, file, word)	/* Get the file & end word */
char *buf, *file, *word;	/* from the buffer */
{
  char *temp;

  temp = buf;
  if (verbose) printf("Getnames: buf is %s\n", buf);

  while (*temp != 0) {		/* Scan along buffer */
	switch (*temp) {	/* Get file or end word */
	    case '>':
		strcpy(file, getstring(++temp));	/* Get the file name */
		break;
	    case '<':
		if (*(++temp) == '<') ++temp;	/* Skip 2nd < */
		strcpy(word, getstring(temp));	/* Get next word */
		break;
	    default:
		temp++;
	}
  }
}



void disembowel()
{				/* Unshar brutally! */
  char buf[BUFSIZE];		/* Line buffer */
  char file[BUFSIZE];		/* File name */
  char word[BUFSIZE];		/* Word buffer */
  int ch, x;

  if (verbose) printf("Entering disembowel\n");
  x = 'X';			/* Leading X character */
  while (1) {
	binfile = 0;
	ch = getline(NOWHITE, buf);	/* Get a line from file */
	if (ch == EOF) return;
	if (binfile) continue;

	switch (firstword(buf)) {	/* Extract, depending on first word */
	    case CAT:
		if (verbose) printf("About to do getnames\n");
		getnames(buf, file, word);
		if (table == 0) {
			if ((numext == 0) || (mustget(file))) {
				printf("unshar: Extracting  %s\n", file);
				if (verbose)
					printf("        stopping at %s\n", word);
				extract(NOX, file, word, x);
			}
		} else
			printf("  %s\n", file);
		break;
	    case GRES:
	    case SED:
		if (verbose) printf("About to do getnames\n");
		getnames(buf, file, word);
		if (table == 0) {
			if ((numext == 0) || (mustget(file))) {
				printf("unshar: Extracting  %s\n", file);
				if (verbose)
					printf("        stopping at %s\n", word);
				extract(YESX, file, word, x);
			}
		} else
			printf("  %s\n", file);
		break;
	    default:
		break;
	}
  }
}



usage()
{
  fprintf(stderr, "Usage: unshar [-t] [-b] [-v] [-xfile] [file(s)]\n");
  exit(0);
}


main(argc, argv)
int argc;
char *argv[];
{
  extern int optind;
  extern char *optarg;
  int i, c, first;

  FILE *zin;			/* Dummy file descriptor */
  int method;			/* Method of unsharing */

  method = BRUTAL;		/* Only BRUTAL currently available */
  table = 0;			/* Don't generate a table */
  verbose = 0;			/* Nor be very verbose */
  numext = 0;			/* Initially no files to extract */


  while ((c = getopt(argc, argv, "x:tbv")) != EOF) switch (c) {
	    case 't':
		table = 1;	/* Get the various options */
		break;
	        case 'b':	method = BRUTAL;	break;
	        case 'v':	verbose = 1;	break;
	    case 'x':
		exfile[numext] = (char *) malloc(strlen(optarg) + 1);
		strcpy(exfile[numext++], optarg);
		break;
	    default:
		usage();
	}

  if (argc == 1)
	first = argc;		/* Find first file argument */
  else
	for (first = 1; first < argc; first++)
		if (argv[first][0] != '-') break;

  if (first == argc) {		/* If no file argument *//* use stdin only */
	switch (method) {
	    case BRUTAL:
		disembowel();	/* Unshar brutally! */
		break;
	    default:
		fprintf(stderr, "unshar: Unknown method of unsharing\n");
		exit(1);
	}
  } else
	for (i = first; i < argc; i++) {	/* open stdio with every
						 * file */
		if (table) printf("%s:\n", argv[i]);
		fclose(stdin);
		if ((zin = fopen(argv[i], "r")) == NULL) {
			perror("unshar2");
			exit(1);
		}
		switch (method) {
		    case BRUTAL:
			disembowel();	/* Unshar brutally! */
			break;
		    default:
			fprintf(stderr, "unshar: Unknown method of unsharing\n");
			exit(1);
		}
	}
  exit(0);
}
