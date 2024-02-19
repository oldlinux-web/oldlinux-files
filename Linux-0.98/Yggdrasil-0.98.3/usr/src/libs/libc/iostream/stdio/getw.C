#include "stdioprivate.h"
#include "errno.h"

int
getw(FILE *fp)
{
  int w;
  void *buf = (void *) &w;
  streambuf* sb = FILE_to_streambuf(fp);
  if (!sb) {
    errno = EBADF;
    return 0;
  }
  size_t bytes_requested = sizeof(w);
  size_t bytes_read = sb->sgetn(buf, bytes_requested);
  return (bytes_requested == bytes_read ? w : EOF);
}
