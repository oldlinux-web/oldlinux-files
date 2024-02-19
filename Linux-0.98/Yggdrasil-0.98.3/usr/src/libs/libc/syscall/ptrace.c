#define __LIBRARY__
#include <time.h>
#include <unistd.h>

int ptrace(int request, int pid, int addr, int data)
{
	long ret;
	long res;
#ifdef PRE_GCC_2
	register long __fooebx __asm__ ("bx") = (long) request;
#endif

	if (request > 0 && request < 4)
		(long *)data = &ret;
#ifdef PRE_GCC_2
	__asm__ volatile ("int $0x80"
		:"=a" (res)
		:"0" (__NR_ptrace),"r" (__fooebx), "c" (pid),
		 "d" (addr), "S" (data)
		: "si","bx","cx","dx");
#else
	__asm__ volatile ("int $0x80"
		:"=a" (res)
		:"0" (__NR_ptrace),"b" (request), "c" (pid),
		 "d" (addr), "S" (data)
		: "si","bx","cx","dx");
#endif
	if (res >= 0) {
		if (request > 0 && request < 4) {
			errno = 0;
			return (ret);
		}
		return (int) res;
	}
	errno = -res;
	return -1;
}
