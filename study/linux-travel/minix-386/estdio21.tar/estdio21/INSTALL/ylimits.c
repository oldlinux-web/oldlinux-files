#include <stdio.h>

int main(argc, argv)

int argc;
char *argv[];

{
  char c;
  unsigned int ui = ~0;
  unsigned long ul = ~0;
  int char_bit;
  int j;
  void exit();
  int strcmp();

  if (argc > 2)
    exit(1);

  for (char_bit = 0, j = 1; (c = j), c != 0; char_bit++, j <<= 1)
    ;

  if (argc < 2 || strcmp(argv[1], "CHAR_BIT") == 0)
    printf("#define CHAR_BIT %d\n", char_bit);
  if (argc < 2 || strcmp(argv[1], "UINT_MAX") == 0)
    printf("#define UINT_MAX %u\n", ui);
  if (argc < 2 || strcmp(argv[1], "ULONG_MAX") == 0)
    printf("#define ULONG_MAX %lu\n", ul);
  if (argc < 2 || strcmp(argv[1], "INT_MAX") == 0)
    printf("#define INT_MAX %d\n", (int) (ui >> 1));
  if (argc < 2 || strcmp(argv[1], "LONG_MAX") == 0)
    printf("#define LONG_MAX %ld\n", (int) (ul >> 1));

  return 0;
}
