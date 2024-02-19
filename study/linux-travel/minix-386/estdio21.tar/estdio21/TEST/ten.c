#include <stdio.h>

int main(argc, argv)

int argc;
char **argv;

{
  long d;
  int i, j;
  int atoi();

  i = atoi(argv[1]);
  while (i--) {
    for (j = 1024/((7 + 6 + 5) * 3 * 16); j; j--) {
      for (d = 1; d < 10000000L; d *= 10)
	printf("%15ld %15lx %15lo\n", d, d, d);
      for (d = 1; d < 0x1000000L; d <<= 4)
	printf("%15ld %15lx %15lo\n", d, d, d);
      for (d = 1; d < 0100000L; d <<= 3)
	printf("%15ld %15lx %15lo\n", d, d, d);
    }
  }
  return 0;
}
