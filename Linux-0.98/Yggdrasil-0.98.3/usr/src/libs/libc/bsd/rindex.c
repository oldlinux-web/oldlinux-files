#include <ansidecl.h>
#include <strings.h>
#include <gnu-stabs.h>

#undef  rindex

function_alias(rindex, strrchr, char *, (s, c),
		DEFUN(rindex, (s, c), CONST char *s AND int c))
