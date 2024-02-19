#include <stdio.h>

int main()

{
  char c;
  int i, j;

  for (i = 0, j = 1; (c = j), c != 0; i++, j <<= 1)
    ;
  printf("0x%x\n", j-1);

  return 0;
}
