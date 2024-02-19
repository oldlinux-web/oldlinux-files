#define __LIBRARY__
#include <unistd.h>

_syscall5(int,mount,const char*,special,const char*,dir,
	  const char*,type,unsigned long,rw_flag, const void*,data)
