#define __LIBRARY__
#define close	libc_close
#define lseek	libc_lseek
#define stat(filename,stat_buf) libc_stat()
#include <sys/dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#undef close
#undef lseek
#undef stat(filename,stat_buf)

static inline
_syscall1(int,close,int,fd)

static inline
 _syscall3(off_t,lseek,int,fildes,off_t,offset,int,origin)

static inline
_syscall2(int,stat,const char *,filename,struct stat *,stat_buf)

/*
 * opendir just makes an open() call - it return NULL if it fails
 * (open sets errno), otherwise it returns a DIR * pointer.
 */
DIR * opendir(const char * name)
{
  int fd;
  struct stat statbuf;
  struct dirent *buf;
  DIR *ptr;

  if (stat(name,&statbuf)) return NULL;
  if (!S_ISDIR(statbuf.st_mode)) {
    errno = ENOTDIR;
    return NULL;
  }
  if ((fd = open(name,O_RDONLY)) < 0)
    return NULL;
  if (!(ptr=malloc(sizeof(*ptr)))) {
    close(fd);
    errno = ENOMEM;
    return NULL;
  }
  if (!(buf=malloc(NUMENT*sizeof (struct dirent)))) {
    close(fd);
    free(ptr);
    errno = ENOMEM;
    return NULL;
  }
  ptr->dd_fd = fd;
  ptr->dd_loc = ptr->dd_size = 0;
  ptr->dd_buf = buf;
  return ptr;
}

int closedir(DIR * dir)
{
  int fd;

  if (!dir) {
    errno = EBADF;
    return -1;
  }
  fd = dir->dd_fd;
  free(dir->dd_buf);
  free(dir);
  return close(fd);
}

/*
 * rewinddir() just does an lseek(fd,0,0) - see close for comments
 */
void
rewinddir(DIR * dir)
{
  if (!dir) {
    errno = EBADF;
    return;
  }
  lseek(dir->dd_fd,0,SEEK_SET);
}

/*
 * readdir fills up the buffer with the readdir system call. it also
 * gives a third parameter (currently ignored, but should be 1) that
 * can with a future kernel be enhanced to be the number of entries
 * to be gotten.
 *
 * Right now the readdir system call return the number of characters
 * in the name - in the future it will probably return the number of
 * entries gotten. No matter - right now we just check for positive:
 * that will always work (as we know that it cannot be bigger than 1
 * in the future: we just asked for one entry).
 */
struct dirent *readdir(DIR * dir)
{
  int result;
  int count = NUMENT;

  if (!dir) {
    errno = EBADF;
    return NULL; 
  }

  if (dir->dd_size <= dir->dd_loc) {
    /* read count of directory entries. For now it should be one. */
    __asm__("int $0x80"
	:"=a" (result)
	:"0" (__NR_readdir),"b" (dir->dd_fd),
	"c" ((long) dir->dd_buf),"d" (count));
    if (result <= 0) {
      errno = -result;
      return NULL;
    }

    /*
     * Right now the readdir system call return the number of
     * characters in the name - in the future it will probably return
     * the number of entries gotten. No matter - right now we just
     * check for positive:
     */
#if 0
    dir->dd_size = result;
#else
    dir->dd_size = 1;
#endif

    dir->dd_loc = 0;
  } 

  return &(dir->dd_buf [(dir->dd_loc)++]);
}

off_t telldir(DIR * dir)
{
  if (!dir) {
    errno = EBADF;
    return -1;
  }
  return lseek(dir->dd_fd,0,SEEK_CUR);
}

void seekdir(DIR * dir, off_t offset)
{
  if (!dir) {
    errno = EBADF;
    return;
  }
  lseek(dir->dd_fd,offset,SEEK_SET);
}

/*
--------------------
NOTES:

(1) Here are all the functions needed: as you can see, the other
functions are pretty simple as well with this kernel call.  Just
wrappers for open(), close() and lseek().  The reason we cannot have a

	struct dirent * readdir(DIR * dirp);
  
directly in the kernel is simple: the kernel doesn't know how to
allocate space in user-space, so it cannot return a pointer to that
space.  With the above wrapper, we give the kernel the address to fill
in. 

The whole code results in a couple of machine-code instructions.

(2) We give (a currently ignored) third parameter to the system-call:
this way we won't break future kernels which will fill in several
directory-entris at once if asked. The third parameter is the maximum
number of entries we want: 1 - this way we know if we failed if the
return-number was zero or negative. With some additional kernel
patches, we can easily make the readdir function fill up several
dirents at once like your getdents call.

Linus

*/

/*

I hacked the code such that opendir () will allocate a buffer for
readdir (), which will be freed by closedir ().

DIR is changed. See dirent.h.

H.J.

*/
