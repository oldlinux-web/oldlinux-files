#define __LIBRARY__
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#define __NR__mmap __NR_mmap
static inline
_syscall1(long,_mmap,unsigned long *,buffer);

caddr_t mmap(caddr_t addr, size_t len, int prot, int flags, int fd, off_t off)
{
	unsigned long buffer[6];
	long rval;

	buffer[0] = (unsigned long)addr;
	buffer[1] = (unsigned long)len;
	buffer[2] = (unsigned long)prot;
	buffer[3] = (unsigned long)flags;
	buffer[4] = (unsigned long)fd;
	buffer[5] = (unsigned long)off;
	rval = _mmap(buffer);
	if ((int)rval < 0)
	{
		errno = -rval;
		return (caddr_t)-1;
	}
	return (caddr_t)rval;
}

_syscall2(int,munmap, caddr_t,addr, size_t,len)
