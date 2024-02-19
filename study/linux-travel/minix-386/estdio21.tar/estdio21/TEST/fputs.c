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
    for (j = 1024/16; j; j--)
      fputs("123456789012345\n", stdout);

  return 0;
}
