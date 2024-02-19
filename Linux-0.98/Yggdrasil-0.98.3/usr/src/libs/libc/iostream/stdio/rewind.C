#include <stdioprivate.h>
#include <errno.h>

void rewind(FILE* fp)
{
    if (!__validfp(fp)) {
	errno = EBADF;
	return;
    }
    (void)((streambuf*)fp)->seekoff(0, ios::beg);
}
