/* whatsnew - mark changes in a patched file	Author: Andy Tanenbaum */

/* MINIX programs are constantly being updated.  When a cdiff patch comes in,
 * it would be nice to be able to apply the patch, and then list the new file,
 * which the changed lines marked somehow.  This program allows that.  Suppose
 * prog.c is the newly patched file and prog.cdif is the cdiff listing that was
 * used to produce it.  Then the command
 *
 * 	whatsnew prog.c prog.cdif
 *
 * will copy prog.c to standard output, marking all added or changed lines
 * with + or ! at the end, respectively.  Output lines are forced to a standard
 * length to make the + and ! symbols easy to find.  The default length is 80,
 * but it can be changed using a flag.  For example,
 *
 *	whatsnew -75 prog.c prog.cdif
 *
 * which pads or truncates the output lines to 75 characters before appending
 * ! or +.  Tabs are converted to spaces in the process.
 */

#include <stdio.h>

#define MAX_WIDTH       256	/* max line length */
#define HEAP_LINES    10000	/* max length of the diff listing */
#define HEAP_BYTES    30000	/* total space for saved lines */
#define HASH_SIZE        10	/* how much of a line is the hash key */
#define TAB_WIDTH         8	/* how far apart are the tab stops */
#define DEF_WIDTH        80	/* default width */

int cdif_found = 0;		/* set to 1 if cdif entry found */
char *index[HEAP_LINES];	/* pointers into the heap */
char heap[HEAP_BYTES];		/* the + and ! lines are saved here. */
int lines_used;			/* number of index slots used */
char *hp = heap;		/* pointer to end of the heap */
int width = DEF_WIDTH;		/* how wide to print the listing */

main(argc, argv)
int argc;
char *argv[];
{

  int k;
  char *p;
  FILE *newf, *cdif;

  /* Process the command line. */
  if (argc < 2) usage();

  k = 1;
  p = argv[1];
  if (*p == '-') {
	width = atoi(p + 1);
	if (width < 0 || width > MAX_WIDTH) {
		fprintf(stderr, "%s: max line size is %d\n",argv[0],MAX_WIDTH);
		exit(1);
	}
	k++;
	if (argc != 4) usage();
  } else {
	if (argc != 3) usage();
  }

  newf = fopen(argv[k], "r");
  if (newf == NULL) {
	fprintf(stderr, "%s: cannot open %s\n", argv[0], argv[k]);
	exit(1);
  }
  cdif = fopen(argv[k + 1], "r");
  if (cdif == NULL) {
	fprintf(stderr, "%s: cannot open %s\n", argv[0], argv[k + 1]);
	exit(1);
  }

  /* Read the cdif file and hash all the relevant lines. */
  eat_cdif(cdif, argv[k]);

  /* Read and print the new file. */
  read_and_print(newf, width);

  exit(0);
}

eat_cdif(cdif, name)
FILE *cdif;
char *name;
{
  int n;
  char line[MAX_WIDTH], *p;

  /* First find the start of the relevant cdif stuff. */
  while (fgets(line, MAX_WIDTH, cdif) != NULL) {
	if (strncmp(line, "*** ", 4) != 0) continue;
	if (present(line, name) && present(line, ":")) {
		cdif_found = 1;
		break;
	}
  }
  if (cdif_found == 0) {
	fprintf(stderr, "no cdif found for %s\n", name);
	exit(2);
  }

  /* We found it. Save all the ! and + lines in the hash table. */
  while (fgets(line, MAX_WIDTH, cdif) != NULL) {
	if (strcmp(line, "*** ", 4) == 0 && present(line, ":")) break;
	if (line[0] != '+' && line[0] != '!') continue;
	if (line[1] != ' ') continue;
	enter_line(line);
  }
}


read_and_print(newf)
FILE *newf;
{
  int h, hit;
  char line[MAX_WIDTH], *p, c;

  /* Examine each line in the patched file. */
  while (fgets(line, MAX_WIDTH, newf) != NULL) {
	/* Hash the line and see if it is in the table. */
	hit = -1;
	h = hash(line);
	while (index[h] != 0) {
		if (strcmp(line, index[h] + 2) == 0) {
			hit = h;
			break;
		}
		h = (h + 1) % HEAP_LINES;
	}
	c = (hit >= 0 ? *(index[h]) : ' ');
	print_line(line, c);
  }
}


int present(line, name)
char *line, *name;
{
/* Scan line to see if the string name is present in it anywhere.  If so,
 * return 1, else return 0.
 */

  register int n;
  register char *p;

  p = line;
  n = strlen(name);
  while (*p != 0) {
	if (strncmp(p, name, n) == 0) return(1);
	p++;
  }
  return(0);
}


enter_line(line)
char *line;
{
/* Enter a line in the hash table.  Note that the first two characters
 * do not count, as they have been added by cdiff.
 */

  int n, h;

  /* Certain lines should not be entered as they occur often. */
  n = strlen(line) - 2;
  if (n == 1) return;		/* don't enter null lines */
  if (strncmp(line, "+ /*==", 6) == 0) return;
  if (strncmp(line, "+  *==", 6) == 0) return;
  if (strncmp(line, "+   {", 5) == 0) return;
  if (strncmp(line, "+   }", 5) == 0) return;
  if (strncmp(line, "+ {", 3) == 0) return;
  if (strncmp(line, "+ }", 3) == 0) return;
  if (strncmp(line, "+ \t{", 4) == 0) return;
  if (strncmp(line, "+ \t}", 4) == 0) return;
  if (strncmp(line, "+ else", 6) == 0) return;
  if (strncmp(line, "+ #end", 6) == 0) return;

  if (lines_used == HEAP_LINES) {
	fprintf(stderr, "cdif listing has too many lines\n");
	exit(1);
  }
  if (hp + n >= &heap[HEAP_BYTES]) {
	fprintf(stderr, "cdif listing has too many bytes\n");
	exit(1);
  }

  /* Make the heap entry. */
  h = hash(line + 2);
  while (index[h] != 0) h = (h + 1) % HEAP_LINES;
  index[h] = hp;		/* index table points to the lines */
  strcpy(hp, line);		/* first two chars also copied */
  lines_used++;			/* count # table slots used */
  hp += n + 3;			/* 0 byte counts also */
}


int hash(p)
char *p;
{
/* Compute and return the hash code of p. */

  int i, n;
  unsigned h;

  n = strlen(p);
  if (n > HASH_SIZE) n = HASH_SIZE;
  h = 0;
  for (i = 0; i < n; i++) {
	h += 23 * i * (int) *p;
	p++;
  }
  h = h % HEAP_LINES;
  return((int) h);
}


print_line(line, c)
char *line, c;
{
/* Print the line. */

  int col, spaces;
  char out_buf[3 * MAX_WIDTH], *p, *q;

  p = line;
  q = out_buf;
  col = 0;
  while (*p != '\n') {
	/* This loop executed once for each character in the line. */
	if (*p != '\t') {
		/* Not a tab. */
		*q++ = *p++;
		col++;
	} else {
		/* Tab. */
		spaces = TAB_WIDTH - (col % TAB_WIDTH);
		col += spaces;
		while (spaces--) *q++ = ' ';
		p++;
	}
  }
  while (q < &out_buf[width]) *q++ = ' ';
  q = &out_buf[width];
  *q++ = ' ';
  *q++ = c;
  *q++ = '\n';
  *q = 0;
  fputs(out_buf, stdout);
}


usage()
{
  fprintf(stderr, "Usage: whatsnew [-<width>] file file.cdif\n");
  exit(1);
}
