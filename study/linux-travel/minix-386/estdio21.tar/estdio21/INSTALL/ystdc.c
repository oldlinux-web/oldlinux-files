#include <stdarg.h>
#include <limits.h>
#include <float.h>
#ifndef	__STDC__
  This is not ANSI C
#endif

int main(argc, argv)

int argc;
char **argv;

{
  int i;

  for (i = 1; i < argc; i++)
    argv[i] = 0;

  return 0;
}
