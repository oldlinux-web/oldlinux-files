#include <errno.h>

extern int sys_nerr;
#ifndef LINUX
extern char *sys_errlist[];
#endif

char *strerror(errno) int errno;
{
  return (errno<sys_nerr ? sys_errlist[errno] : "unknown");
}
