#include <stdio.h>

extern int errno;

int main()

{
  errno = -5;
  perror("Error 1 is ");
  errno = 65536;
  perror("Error 2 is ");
  return 0;
}
