/*  fortune  -  hand out Chinese fortune cookies	Author: Bert Reuling */

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>

#define COOKIEJAR "/usr/lib/fortune.dat"

static char *Copyright = "\0fortune v1.1 Copyright (c) 1988 Bert Reuling";

long seed;

main(argc, argv)
int argc;
char *argv[];
{
  int c1, c2, c3;
  long magic();
  struct stat cookie_stat;
  FILE *cookie, *out, *fopen(), *popen();

  if ((cookie = fopen(COOKIEJAR, "r")) == NULL) {
	fprintf(stderr, "%s:\nCan't open %s\n", argv[0], COOKIEJAR);
	exit(-1);
  }

  /* Create seed from : date, time, user-id and process-id we can't get
   * the position of the moon, unfortunately.
   */
  seed = time((time_t *) 0) * (long) (getuid() + 1) * (long) getpid();

  if (stat(COOKIEJAR, &cookie_stat) != 0) {
	fprintf(stderr, "%s:\nCannot stat cookie jar\n", argv[0]);
	exit(-1);
  }
  fseek(cookie, magic((long) cookie_stat.st_size), 0);	/* move by magic... */

  c2 = c3 = '\n';
  while (((c1 = getc(cookie)) != EOF) && ((c1 != '%') || (c2 != '%') || (c3 != '\n'))) {
	c3 = c2;
	c2 = c1;
  }

  if (c1 == EOF) {
	fprintf(stderr, "%s:\n", argv[0]);
	fprintf(stderr, "The cookie jar does not have a bottom!\n");
	fprintf(stderr, "All cookies have fallen out...\n");
	exit(-1);
  }
#ifdef FORMATTER
  if ((out = popen(FORMATTER, "w")) == NULL) {
	fprintf(stderr, "%s:\nIt furthers one to see a plumber!\n", argv[0]);
	exit(-1);
  }
#else
  out = stdout;
#endif

  c2 = c3 = '\n';
  while (((c1 = getc(cookie)) != '%') || (c2 != '%') || (c3 != '\n')) {
	if (c1 == EOF) {
		rewind(cookie);
		continue;
	}
	putc(c2, out);
	c3 = c2;
	c2 = c1;
  }
  putc('\n', out);
  fclose(cookie);

#ifdef FORMATTER
  pclose(out);
#endif

  exit(0);
}

/*  magic  -  please study carefull: there is more than meets the eye */
long magic(range)
long range;
{
  int i;

  for (i = 0; i < 1234; i++)
	seed = 883L * (seed % 881L) - 2 * (seed / 883L) + 1L;
  return((long) ((int) (seed & 0x7fffL) * range / 0x7fffL));
}
