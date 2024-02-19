#include "stdioprivate.h"
#include <errno.h>

int fflush (register FILE *fp)
{
  if (fp == NULL)
      return streambuf::flush_all();

  streambuf* sb = FILE_to_streambuf(fp);
  if (!sb)
    {
      errno = EBADF;
      return EOF;
    }

  return sb->sync() == EOF ? EOF : 0;
}
