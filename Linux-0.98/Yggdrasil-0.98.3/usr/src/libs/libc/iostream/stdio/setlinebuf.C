#include <stdioprivate.h>
#include <errno.h>

void setlinebuf(FILE* stream)
{
    if (!__validfp(stream)) {
	errno = EBADF;
	return;
    }
    ((streambuf*)stream)->linebuffered(1);
    // FIXME: If currently unbuffered(), set base() to NULL,
    // to force allocation later on next under/overflow.
}
