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
      printf("123456789012345\n");

  return 0;
}
