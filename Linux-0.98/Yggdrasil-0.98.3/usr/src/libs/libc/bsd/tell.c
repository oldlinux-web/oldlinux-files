#define __LIBRARY__
#define lseek libc_lseek
#include <unistd.h>
#undef lseek

static inline
_syscall3(off_t,lseek,int,fildes,off_t,offset,int,origin)

off_t
tell (int fildes)
{
  return lseek (fildes, 0, SEEK_CUR);
}
