#define __LIBRARY__
#include <unistd.h>
#include <stdarg.h>

int open(const char * filename, int flag, ...)
{
	register int res;
	va_list arg;
#ifdef PRE_GCC_2
	register const char *__fooebx __asm__ ("bx") = filename;
#endif

	va_start(arg,flag);
#ifdef PRE_GCC_2
	__asm__("int $0x80"
		:"=a" (res)
		:"0" (__NR_open),"r" (__fooebx),"c" (flag),
		"d" (va_arg(arg,int)));
#else
	__asm__("int $0x80"
		:"=a" (res)
		:"0" (__NR_open),"b" (filename),"c" (flag),
		"d" (va_arg(arg,int)));
#endif
	if (res>=0)
		return res;
	errno = -res;
	va_end(arg);
	return -1;
}
