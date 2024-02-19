#include <ansidecl.h>
#include <localeinfo.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

long int
DEFUN_VOID (__tzname_max)
{
  size_t len0 = strlen (__tzname[0]), len1 = strlen (__tzname[1]);
  return len0 > len1 ? len0 : len1;
}
