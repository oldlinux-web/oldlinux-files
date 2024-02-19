#include <stdioprivate.h>
#include <errno.h>

int vscanf(const char *format, va_list args)
{
    streambuf* sb = FILE_to_streambuf(stdin);
    if (!sb) {
	errno = EBADF;
	return EOF;
    }
    return sb->vscan(format, args);
}
