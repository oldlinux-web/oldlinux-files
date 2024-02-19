#include "stdioprivate.h"
#include "errno.h"

char* fgets(char* s, int n, FILE* stream)
{
    if (!__validfp(stream)) {
	errno = EBADF;
	return NULL;
    }
    if (((streambuf*)stream)->sgetline(s, n, '\n', 1) <= 0)
	return NULL;
    return s;
}
