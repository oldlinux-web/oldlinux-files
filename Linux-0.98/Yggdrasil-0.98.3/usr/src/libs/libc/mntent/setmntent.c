#include <ansidecl.h>
#include <stdio.h>
#include <gnu-stabs.h>
#include <mntent.h>

function_alias(setmntent, fopen, FILE *, (filename, mode),
	DEFUN(fopen, (filename, mode), CONST char *filename AND CONST char *mode))
