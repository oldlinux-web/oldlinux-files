#define __LIBRARY__
#include <unistd.h>

_syscall3(int, reboot, int, magic, int, magic_too, int, flag)
