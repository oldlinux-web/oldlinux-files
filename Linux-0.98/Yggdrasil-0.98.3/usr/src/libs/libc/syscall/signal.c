#define __LIBRARY__
#include <unistd.h>

extern void ___sig_restore();
extern void ___masksig_restore();

sigset_t ___ssetmask(sigset_t mask)
{
	long res;
#ifdef PRE_GCC_2
	register int __fooebx __asm__ ("bx") = mask;
	__asm__("int $0x80":"=a" (res)
		:"0" (__NR_ssetmask),"r" (__fooebx));
#else
	__asm__("int $0x80":"=a" (res)
		:"0" (__NR_ssetmask),"b" (mask));
#endif
	return res;
}

sigset_t ___sgetmask(void)
{
	long res;
	__asm__("int $0x80":"=a" (res) :"0" (__NR_sgetmask));
	return res;
}

#if 0
void (*signal(int sig, void (*func)(int)))(int)
#else
void (*signal(int sig, __sighandler_t func))(int)
#endif
{
	void (*res)();

#ifdef PRE_GCC_2
	register int __fooebx __asm__ ("bx") = sig;
	__asm__("int $0x80":"=a" (res):
	"0" (__NR_signal),"r" (__fooebx),"c" (func),"d" ((long)___sig_restore));
#else
	__asm__("int $0x80":"=a" (res):
	"0" (__NR_signal),"b" (sig),"c" (func),"d" ((long)___sig_restore));
#endif
	return res;
}

int sigaction(int sig,struct sigaction * sa, struct sigaction * old)
{
#ifdef PRE_GCC_2
	register int __fooebx __asm__ ("bx") = sig;
#endif

	if (sa->sa_flags & SA_NOMASK)
		sa->sa_restorer=___sig_restore;
	else
		sa->sa_restorer=___masksig_restore;

#ifdef PRE_GCC_2
	__asm__("int $0x80":"=a" (sig)
		:"0" (__NR_sigaction),"r" (__fooebx),"c" (sa),"d" (old));
#else
	__asm__("int $0x80":"=a" (sig)
		:"0" (__NR_sigaction),"b" (sig),"c" (sa),"d" (old));
#endif
	if (sig>=0)
		return 0;
	errno = -sig;
	return -1;
}
