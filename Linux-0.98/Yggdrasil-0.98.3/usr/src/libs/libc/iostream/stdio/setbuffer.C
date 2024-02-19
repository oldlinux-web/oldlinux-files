#include <stdioprivate.h>
#include <errno.h>

void setbuffer(FILE* stream, char* buf, int size)
{
    if (!__validfp(stream)) {
	errno = EBADF;
	return;
    }
    if (buf) 
	(void)((streambuf*)stream)->setbuf(buf, size);
    else
	(void)((streambuf*)stream)->setbuf(NULL, 0);
}
