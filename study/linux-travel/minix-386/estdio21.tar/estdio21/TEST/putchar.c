#include <stdio.h>

int main(argc, argv)

int argc;
char **argv;

{
  int i;
  int j;
  int atoi();
  
  i = atoi(argv[1]);
  
  while (i--)
    for (j = 1024/16; j; j--) {
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('1');
      putchar('\n');
    }

  return 0;
}
