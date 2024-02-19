#define __LIBRARY__
#include <unistd.h>

_syscall1(int,uname,struct utsname *,uname_buf)
