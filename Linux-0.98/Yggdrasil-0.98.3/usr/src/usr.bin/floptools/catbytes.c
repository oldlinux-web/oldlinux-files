#include <stdio.h>
#include <ctype.h>

/*
 * catbytes n
 *
 * Copies ``n'' bytes from stdin to stdout. Unlike ``dd'' it doesn't
 * care how stdin comes in buffered; unlike everything else I can find
 * it doesn't gobble any excess bytes from stdin. Why does this have
 * to be so hard?
 */

#define BS 65536

static char msg1[] = "syntax: catbytes count\n";
static char msg2[] = "short input\n";
static char msg3[] = "short write\n";

int main(int argc, char **argv) {
  static char buffer[BS];
  int i, n, total;
  extern int read(int, char *, int);
  extern int write(int, char *, int);
  extern int atoi(char *);

  if (argc != 2 || !isdigit(argv[1][0])) {
    (void) write(2, msg1, sizeof(msg1));
    exit(1);
  }
  total = atoi(argv[1]);

  while (total > 0) {
    i = (total > BS) ? BS : total;
    if ((n = read(0, buffer, i)) <= 0) {
      (void) write(2, msg2, sizeof(msg2));
      exit(1);
    }
    if (write(1, buffer, n) != n) {
      (void) write(2, msg3, sizeof(msg3));
      exit(1);
    }
    total -= n;
  }
  return(0);
}
