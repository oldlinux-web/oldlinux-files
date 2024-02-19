#include <ansidecl.h>
#include <signal.h>
#include <gnu-stabs.h>

#undef  sigsetmask

function_alias(sigsetmask, ___ssetmask, int, (mask),
		DEFUN(sigsetmask, (mask), int mask))
