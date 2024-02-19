#include <stdio.h>

char *
xmalloc (size)
     int size;
{
  register int value = malloc (size);
  if (value == 0) {
    fprintf( stderr, "virtual memory exhausted" );
    exit(1);
  }
  return value;
}
