/* echo - echo arguments	Author: Andy Tanenbaum */

#define SIZE 1024
char buf[SIZE];
int count;
main(argc, argv)
int argc;
char *argv[];
{
  register int i, nflag;

  nflag = 0;
  if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'n') {
	nflag++;
	argc--;
	argv++;
  }
  for (i = 1; i < argc; i++) {
	collect(argv[i]);
	if (i < argc - 1) collect(" ");
  }

  if (nflag == 0) collect("\n");

  /* Print accumulated output. */
  if (count > 0) write(1, buf, count);
  exit(0);
}

collect(s)
char *s;
{
/* Collect characters.  For efficiency, write them in large chunks. */
  char c;

  while ((c = *s++) != 0) {
	if (count == SIZE) {
		write(1, buf, count);
		count = 0;
	}
	buf[count++] = c;
  }
}
