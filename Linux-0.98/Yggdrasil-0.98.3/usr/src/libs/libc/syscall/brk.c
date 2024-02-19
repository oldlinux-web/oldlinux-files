#define __LIBRARY__
#include <unistd.h>
#include <errno.h>

#ifdef NO_JUMP_TABLE
void * ___brk_addr = NULL;
#else
extern void * ___brk_addr;
#endif

int brk(void * end_data_seg)
{
#ifdef PRE_GCC_2
	register void * __fooebx  __asm__ ("bx") = end_data_seg;
	__asm__ volatile ("int $0x80"
		:"=a" (___brk_addr)
		:"0" (__NR_brk),"r" (__fooebx));
#else
	__asm__ volatile ("int $0x80"
		:"=a" (___brk_addr)
		:"0" (__NR_brk),"b" (end_data_seg));
#endif
	if (___brk_addr == end_data_seg)
		return 0;
	errno = ENOMEM;
	return -1;
}

void * sbrk(ptrdiff_t increment)
{
#ifdef PRE_GCC_2
	register void * tmp __asm__ ("bx") = ___brk_addr+increment;
	__asm__ volatile ("int $0x80"
		:"=a" (___brk_addr)
		:"0" (__NR_brk),"r" (tmp));
#else
	void * tmp = ___brk_addr+increment;
	__asm__ volatile ("int $0x80"
		:"=a" (___brk_addr)
		:"0" (__NR_brk),"b" (tmp));
#endif
	if (___brk_addr == tmp)
		return tmp-increment;
	errno = ENOMEM;
	return ((void *) -1);
}
