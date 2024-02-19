#include "stdioprivate.h"
#include <errno.h>

int fputs(const char *str, FILE *fp)
{
    if (!__validfp(fp)) {
	errno = EBADF;
	return EOF;
    }
    return ((streambuf*)fp)->sputn(str, strlen(str)) == EOF ? EOF : 0;
}
