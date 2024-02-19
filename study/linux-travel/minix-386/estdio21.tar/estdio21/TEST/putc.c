#include <stdio.h>

int main(argc, argv)

int argc;
char **argv;

{
  int i, j;
  int atoi();

  i = atoi(argv[1]);
  while (i--)
    for (j = 512; j; j--) {
      putchar('a');
      putchar('\n');
    }
  return 0;
}
