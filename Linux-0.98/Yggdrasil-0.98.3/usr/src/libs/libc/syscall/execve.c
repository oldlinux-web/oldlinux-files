#define __LIBRARY__
#include <unistd.h>

int execve(const char * file,char *const argv [],char *const envp [])
{
  long __res;

#ifdef PRE_GCC_2
  register long __fooebx __asm__ ("bx") = (long) file;

  __asm__ volatile ("int $0x80" :
	"=a" (__res) : "0" (__NR_execve),"r" (__fooebx),
	"c" ((long)argv), "d" ((long)envp));
#else
  __asm__ volatile ("int $0x80" :
	"=a" (__res) : "0" (__NR_execve),"b" ((long) file),
	"c" ((long)argv), "d" ((long)envp));
#endif
	
  if (__res>=0) return (int) __res;
  errno=-__res;
  return -1;
}
