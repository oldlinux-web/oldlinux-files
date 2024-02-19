#include <stdioprivate.h>
#include <errno.h>

int setvbuf(FILE* stream, char* buf, int mode, size_t size)
{
    if (!__validfp(stream)) {
	errno = EBADF;
	return EOF;
    }
    switch (mode) {
      case _IOFBF:
	((streambuf*)stream)->linebuffered(0);
	return ((streambuf*)stream)->setbuf(buf, size) == NULL ? EOF : 0;
      case _IOLBF:
	((streambuf*)stream)->linebuffered(1);
	return ((streambuf*)stream)->setbuf(buf, size) == NULL ? EOF : 0;
      case _IONBF:
	return ((streambuf*)stream)->setbuf(NULL, 0) == NULL ? EOF : 0;
      default:
	return EOF;
    }
}
