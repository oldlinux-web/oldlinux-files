#define __LIBRARY__
#include <linux/unistd.h>

#define	PZERO	15

int getpriority(int which, int who)
{
        long res;

        __asm__ volatile ("int $0x80"
                :"=a" (res)
                :"0" (__NR_getpriority),"b" (which), "c" (who)
                : "bx","cx");
        if (res >= 0) {
		errno = 0;
                return (int) PZERO - res;
        }
        errno = -res;
        return -1;
}

_syscall3(int, setpriority, int, which, int, who, int, niceval);
