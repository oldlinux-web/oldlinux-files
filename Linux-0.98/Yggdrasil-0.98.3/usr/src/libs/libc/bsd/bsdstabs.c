#include <ansidecl.h>
#include <gnu-stabs.h>
#include <sys/param.h>

#undef  getdtablesize
#undef  getpagesize
#undef  vfork

symbol_alias (__getdtablesize, getdtablesize);

symbol_alias (__getpagesize, getpagesize);

symbol_alias (fork, vfork);
