#include <stdioprivate.h>
#include <errno.h>

int vfscanf(FILE *fp, const char* format, va_list args)
{
    streambuf* sb = FILE_to_streambuf(fp);
    if (!sb) {
	errno = EBADF;
	return EOF;
    }
    return sb->vscan(format, args);
}
