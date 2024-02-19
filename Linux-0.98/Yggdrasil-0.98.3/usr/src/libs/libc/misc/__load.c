#define __LIBRARY__
#define exit	libc_exit
#define uselib	libc_uselib
#define write	libc_write
#include <unistd.h>
#undef exit
#undef uselib
#undef write

static int errno = 0;

#include <stdarg.h>
#include <sharedlib.h>
#include <errno.h>

static inline volatile void
_exit(int exit_code)
{
  register int __fooebx __asm__ ("bx") = exit_code;
  __asm__("int $0x80"::"a" (__NR_exit),"r" (__fooebx));
}

static inline
_syscall1(int,uselib,const char *,filename)

static
_syscall3(int,write,int,fd,const char *,buf,off_t,count)

static inline int
length(const char *str)
{
  const char *p;
  for (p = str; *p; p++);
  return p - str;
}

static void
__lib_print (int fd, ...)
{
  char *cp;
  va_list ap;

  va_start(ap, fd);
  for (cp = va_arg(ap, char *); cp; cp = va_arg(ap, char *))
      write (fd, cp, length(cp));
  va_end(ap);
}

static void inline
__shlib_fatal (const char *argv0, const char *lib, int err)
{
  __lib_print (2, argv0, ": can't load library '", lib, "'\n\t",
	NULL);

  switch (err) {
  case 0:
    __lib_print (2, "Incompatible version.\n", NULL);
    break;
  case EINVAL:
    __lib_print (2, "Too many libraries.\n", NULL);
    break;
  case ENOENT:
    __lib_print (2, "No such library.\n", NULL);
    break;
  case EACCES:
    __lib_print (2, "Permission denied.\n", NULL);
    break;
  case ENOEXEC:
    __lib_print (2, "Exec format error.\n", NULL);
    break;
  default:
    __lib_print (2, "Unspecified error.\n", NULL);
    break;
  }

  while (1)
    _exit (128);
}

static inline int
incompatible(int in_core, int linked)
{
  if (linked & CLASSIC_BIT) {
    return (in_core != linked);
  }
  else {
    return ((in_core & MAJOR_MASK) != (linked & MAJOR_MASK) ||
	(in_core & MINOR_MASK) < (linked & MINOR_MASK));
  }
}

extern struct libentry * __SHARED_LIBRARIES__[];

void
__load_shared_libraries (int argc, char **argv)
{
  struct libentry **ptr;

  if (argc > 0) {
    for (ptr = __SHARED_LIBRARIES__+2; *ptr; ptr++)
    {
      unsigned *vers = (unsigned *)(*ptr)->addr;
      if (uselib((*ptr)->name))
	__shlib_fatal (argv [0], (*ptr)->name, errno);
      if (incompatible(*vers, (*ptr)->vers))
	__shlib_fatal (argv [0], (*ptr)->name, 0);
    }
  }
  else {
    for (ptr = __SHARED_LIBRARIES__+2; *ptr; ptr++)
      __lib_print (1, "\t", (*ptr)->name, " (", (*ptr)->avers, ")\n", NULL);

    while (1) _exit (0);
  }
}
