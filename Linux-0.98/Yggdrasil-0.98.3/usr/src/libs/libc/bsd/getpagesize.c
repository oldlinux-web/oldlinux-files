#include <ansidecl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/param.h>

size_t
__getpagesize (void)
{
  return EXEC_PAGESIZE;
}


size_t
__getdtablesize (void)
{
  return OPEN_MAX;
}
