#include <ansidecl.h>
#include <strings.h>
#include <gnu-stabs.h>

#undef  index

function_alias(index, strchr, char *, (s, c),
		DEFUN(index, (s, c), CONST char *s AND int c))
