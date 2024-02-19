#include <stdio.h>

int main()

{
  int ch;

  while ((ch = getchar()) != EOF)
    putchar(ch);
  return ferror(stdout);
}
