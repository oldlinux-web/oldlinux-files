/* grep - search a file for a pattern	Author: Norbert Schlenker */

/* Norbert Schlenker (nfs@princeton.edu)  1990-02-08
 * Released into the public domain.
 *
 * Grep searches files for lines containing a pattern, as specified by
 * a regular expression, and prints those lines.  It is invoked by:
 *	grep [flags] [pattern] [file ...]
 *
 * Flags:
 *	-e pattern	useful when pattern begins with '-'
 *	-l		prints just file names, no lines (quietly overrides -n)
 *	-n		printed lines are preceded by relative line numbers
 *	-s		prints errors only (quietly overrides -l and -n)
 *	-v		prints lines which don't contain the pattern
 *
 * Semantic note:
 * 	If both -l and -v are specified, grep prints the names of those
 *	files which do not contain the pattern *anywhere*.
 *
 * Exit:
 *	Grep sets an exit status which can be tested by the caller.
 *	Note that these settings are not necessarily compatible with
 *	any other version of grep, especially when -v is specified.
 *	Possible status values are:
 *	  0	if any matches are found
 *	  1	if no matches are found
 *	  2	if syntax errors are detected or any file cannot be opened
 */


/* External interfaces */
#include <regexp.h>		/* Thanks to Henry Spencer */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern int getopt();		/* Thanks to Henry Spencer */
extern char *optarg;
extern int optind;


/* Internal constants */
#define MATCH		0	/* exit code: some match somewhere */
#define NO_MATCH	1	/* exit code: no match on any line */
#define FAILURE		2	/* exit code: syntax error or bad file name */


/* Macros */
#define SET_FLAG(c)	(flags[(c)-'a'] = 1)
#define FLAG(c)		(flags[(c)-'a'] != 0)


/* Private storage */
static char *program;		/* program name */
static char flags[26];		/* invocation flags */
static regexp *expression;	/* compiled search pattern */


/* Internal interfaces */
static int match();
static char *get_line();
static void error_exit();


int main(argc, argv)
int argc;
char *argv[];
{
  int opt;			/* option letter from getopt() */
  char *pattern;		/* search pattern */
  int exit_status = NO_MATCH;	/* exit status for our caller */
  int file_status;		/* status of search in one file */
  FILE *input;			/* input file (if not stdin) */

  program = argv[0];
  memset(flags, 0, sizeof(flags));
  pattern = (char *) NULL;

/* Process any command line flags. */
  while ((opt = getopt(argc, argv, "e:lnsv")) != EOF) {
	if (opt == '?')
		exit_status = FAILURE;
	else
	if (opt == 'e')
		pattern = optarg;
	else
		SET_FLAG(opt);
  }

/* Detect a few problems. */
  if ((exit_status == FAILURE) || (optind == argc && pattern == (char *) NULL))
	error_exit("Usage: %s [-lnsv] [-e] expression [file ...]\n");

/* Ensure we have a usable pattern. */
  if (pattern == (char *) NULL)
	pattern = argv[optind++];
  if ((expression = regcomp(pattern)) == (regexp *) NULL)
	error_exit("%s: bad regular expression\n");

/* Process the files appropriately. */
  if (optind == argc) {		/* no file names - find pattern in stdin */
	exit_status = match(stdin, (char *) NULL, "<stdin>");
  }
  else 
  if (optind + 1 == argc) {	/* one file name - find pattern in it */
	if (strcmp(argv[optind], "-") == 0) {
		exit_status = match(stdin, (char *) NULL, "-");
	} else {
		if ((input = fopen(argv[optind], "r")) == (FILE *) NULL) {
			fprintf(stderr, "%s: couldn't open %s\n",
							program, argv[optind]);
			exit_status = FAILURE;
		}
		exit_status = match(input, (char *) NULL, argv[optind]);
	}
  }
  else
  while (optind < argc) {	/* lots of file names - find pattern in all */
	if (strcmp(argv[optind], "-") == 0) {
		file_status = match(stdin, "-", "-");
	} else {
		if ((input = fopen(argv[optind], "r")) == (FILE *) NULL) {
			fprintf(stderr, "%s: couldn't open %s\n",
							program, argv[optind]);
			exit_status = FAILURE;
		} else {
			file_status = match(input, argv[optind], argv[optind]);
			fclose(input);
		}
	}
	if (exit_status != FAILURE)
		exit_status &= file_status;
	++optind;
  }
  exit(exit_status);
}


/* match - matches the lines of a file with the regular expression.
 * To improve performance when either -s or -l is specified, this
 * function handles those cases specially.
 */

static int match(input, label, filename)
FILE *input;
char *label;
char *filename;
{
  char *line;			/* pointer to input line */
  long int lineno = 0;		/* line number */
  int status = NO_MATCH;	/* summary of what was found in this file */

  if (FLAG('s') || FLAG('l')) {
	while ((line = get_line(input)) != (char *) NULL)
		if (regexec(expression, line, 1)) {
			status = MATCH;
			break;
		}
	if (FLAG('l'))
		if ((!FLAG('v') && status == MATCH) ||
		    ( FLAG('v') && status == NO_MATCH))
			puts(filename);
	return status;
  }

  while ((line = get_line(input)) != (char *) NULL) {
	++lineno;
	if (regexec(expression, line, 1)) {
		status = MATCH;
		if (!FLAG('v')) {
			if (label != (char *) NULL)
				printf("%s:", label);
			if (FLAG('n'))
				printf("%ld:", lineno);
			puts(line);
		}
	} else {
		if (FLAG('v')) {
			if (label != (char *) NULL)
				printf("%s:", label);
			if (FLAG('n'))
				printf("%ld:", lineno);
			puts(line);
		}
	}
  }
  return status;
}


/* get_line - fetch a line from the input file
 * This function reads a line from the input file into a dynamically
 * allocated buffer.  If the line is too long for the current buffer,
 * attempts will be made to increase its size to accomodate the line.
 * The trailing newline is stripped before returning to the caller.
 */

#define FIRST_BUFFER 256		/* first buffer size */

static char *buf = (char *) NULL;	/* input buffer */
static size_t buf_size = 0;		/* input buffer size */

static char *get_line(input)
FILE *input;
{
  int n;
  register char *bp;
  register int c;
  char *new_buf;
  size_t new_size;

  if (buf_size == 0) {
	if ((buf = (char *) malloc(FIRST_BUFFER)) == (char *) NULL)
		error_exit("%s: not enough memory\n");
	buf_size = FIRST_BUFFER;
  }

  bp = buf;
  n = buf_size;
  while (1) {
	while (--n > 0 && (c = getc(input)) != EOF) {
		if (c == '\n') {
			*bp = '\0';
			return buf;
		}
		*bp++ = c;
	}
	if (c == EOF)
		return (ferror(input) || bp == buf) ? (char *) NULL : buf;
	new_size = buf_size << 1;
	if ((new_buf = (char *) realloc(buf, new_size)) == (char *) NULL) {
		fprintf(stderr, "%s: line too long - truncated\n", program);
		while ((c = getc(input)) != EOF && c != '\n') ;
		*bp = '\0';
		return buf;
	} else {
		bp = new_buf + (buf_size - 1);
		n = buf_size + 1;
		buf = new_buf;
		buf_size = new_size;
	}
  }
}


/* Regular expression code calls this routine to print errors. */

void regerror(s)
char *s;
{
  fprintf(stderr, "regexp: %s\n", s);
}


/* Common exit point for outrageous errors. */

static void error_exit(msg)
char *msg;
{
  fprintf(stderr, msg, program);
  exit(FAILURE);
}
