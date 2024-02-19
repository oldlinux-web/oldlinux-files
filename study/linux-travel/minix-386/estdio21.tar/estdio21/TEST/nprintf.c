#include <stdio.h>

int main(argc, argv)

int argc;
char **argv;

{
  int i;
  int j;
  int atoi();

  setbuf(stdout, NULL);
  i = atoi(argv[1]);
  while (i--) {
    for (j = 1024/(4*16); j; j--) {
      printf("123456789012345\n\
%15s\n\
%15d\n\
%15ld\n",
"Test Test", 0x7fff, 0x7fffffffL);
    }
  }
  return 0;
}
