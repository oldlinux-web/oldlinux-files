#define __LIBRARY__
#include <unistd.h>
#include <stdarg.h>

int fcntl(int fildes, int cmd, ...)
{
	register int res;
	va_list arg;
#ifdef PRE_GCC_2
	register int __fooebx __asm__ ("bx") = fildes;
#endif

	va_start(arg,cmd);
#ifdef PRE_GCC_2
	__asm__("int $0x80"
		:"=a" (res)
		:"0" (__NR_fcntl),"r" (__fooebx),"c" (cmd),
		"d" (va_arg(arg,int)));
#else
	__asm__("int $0x80"
		:"=a" (res)
		:"0" (__NR_fcntl),"b" (fildes),"c" (cmd),
		"d" (va_arg(arg,int)));
#endif
	if (res>=0)
		return res;
	errno = -res;
	va_end (arg);
	return -1;
}
