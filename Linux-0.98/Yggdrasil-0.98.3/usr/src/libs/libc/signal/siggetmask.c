#include <signal.h>
#include <gnu-stabs.h>

#undef  siggetmask

symbol_alias (___sgetmask, siggetmask);
