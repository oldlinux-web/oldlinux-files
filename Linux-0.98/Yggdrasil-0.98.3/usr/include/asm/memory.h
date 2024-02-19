/*
 *  NOTE!!! memcpy(dest,src,n) assumes ds=es=normal data segment. This
 *  goes for all kernel functions (ds=es=kernel space, fs=local data,
 *  gs=null), as well as for all well-behaving user programs (ds=es=
 *  user data space). This is NOT a bug, as any user program that changes
 *  es deserves to die if it isn't careful.
 */
#if 0
#define memcpy(dest,src,n) ({ \
void * _res = dest; \
__asm__ __volatile__ ("cld;rep;movsb" \
	::"D" ((long)(_res)),"S" ((long)(src)),"c" ((long) (n)) \
	:"di","si","cx"); \
_res; \
})
#else

/* this is basically memcpy_tofs.  It should be faster.
   I've reorder it.  This should be a little faster. -RAB */

#define memcpy(dest, src, n) f_memcpy(dest, src, n)
extern inline void * f_memcpy(void * to, void * from, unsigned long n)
{
__asm__("cld\n\t"
	"movl %%edx, %%ecx\n\t"
	"shrl $2,%%ecx\n\t"
	"rep ; movsl\n\t"
	"testb $1,%%dl\n\t"
	"je 1f\n\t"
	"movsb\n"
	"1:\ttestb $2,%%dl\n\t"
	"je 2f\n\t"
	"movsw\n"
	"2:\n"
	::"d" (n),"D" ((long) to),"S" ((long) from)
	: "cx","di","si");
return (to);
}
#endif
