#include <stdioprivate.h>
#include <errno.h>

void setbuf(FILE* stream, char* buf)
{
    if (!__validfp(stream)) {
	errno = EBADF;
	return;
    }
    if (buf) 
	(void)((streambuf*)stream)->setbuf(buf, BUFSIZ);
    else
	(void)((streambuf*)stream)->setbuf(NULL, 0);
}
