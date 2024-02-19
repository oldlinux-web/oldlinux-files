#define __LIBRARY__
#include <unistd.h>

int sigsuspend(sigset_t *sigmask)
{
	int res;

#ifdef PRE_GCC_2
	register int __fooebx __asm__ ("bx") = 0;
	__asm__("int $0x80"
		:"=a" (res)
		:"0" (__NR_sigsuspend), "r" (__fooebx), "c" (0), "d" (*sigmask)
		:"bx","cx");
#else
	__asm__("int $0x80"
		:"=a" (res)
		:"0" (__NR_sigsuspend), "b" (0), "c" (0), "d" (*sigmask)
		:"bx","cx");
#endif
	if (res >= 0)
		return res;
	errno = -res;
	return -1;
}
