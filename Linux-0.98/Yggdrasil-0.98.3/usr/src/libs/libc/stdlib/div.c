#include <ansidecl.h>
#include <stdlib.h>

#undef	div
#undef	ldiv

#include <gnu-stabs.h>

function_alias(div, ldiv, div_t, (num, denom),
		DEFUN(div, (num, denom), int num AND int denom))
