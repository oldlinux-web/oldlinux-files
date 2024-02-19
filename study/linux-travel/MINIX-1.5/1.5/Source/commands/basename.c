/* basename - print the last part of a path:	Author: Blaine Garfolo */

#define NULL 0

main(argc, argv)
int argc;
char *argv[];
{
  int j, suflen;
  char *c;
  char *d;
  extern char *rindex();

  if (argc < 2) {
	std_err("Usage: basename string [suffix]  \n");
	exit(1);
  }
  c = argv[1];
  d = rindex(argv[1], '/');
  if (d == NULL)
	d = argv[1];
  else
	d++;

  if (argc == 2) {		/* if no suffix */
	prints("%s\n", d);
	exit(0);
  } else {			/* if suffix is present */
	c = d;
	suflen = strlen(argv[2]);
	j = strlen(c) - suflen;
	if (strcmp(c + j, argv[2]) == 0) *(c + j) = 0;
  }
  prints("%s\n", c);
}
