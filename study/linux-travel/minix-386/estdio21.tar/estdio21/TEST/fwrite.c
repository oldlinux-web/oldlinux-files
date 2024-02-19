#include <stdio.h>

int main(argc, argv)

int argc;
char **argv;

{
  char *malloc();
  void exit();
  char *b;
  int j;
  int atoi();

  if ((b = malloc(48*1024)) == 0) {
    puts("No memory for buffer");
    exit(1);
  }

  j = atoi(argv[1]);
  while (j) {
    if (j > 48) {
      fwrite(b, 1023, 48, stdout);
      j -= 48;
    }
    else {
      fwrite(b, 1023, 1, stdout);
      j--;
    }
  }

  return 0;
}
