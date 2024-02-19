#include <sys/types.h>
#include <stdio.h>

int main()

{
  printf("unsigned ");
  if (sizeof(size_t) <= sizeof(unsigned short) &&
      sizeof(unsigned short) < sizeof(int))
    printf("short");
  else if (sizeof(size_t) <= sizeof(int))
    printf("int");
  else
    printf("long");
  putchar('\n');
  return 0;
}
