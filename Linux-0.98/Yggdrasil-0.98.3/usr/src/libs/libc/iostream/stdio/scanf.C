#include <stdioprivate.h>
#include <stdarg.h>
#include <errno.h>

int scanf(const char* format, ...)
{
    streambuf* sb = FILE_to_streambuf(stdin);
    if (!sb) {
	errno = EBADF;
	return EOF;
    }
    va_list args;
    va_start(args, format);
    int ret = sb->vscan(format, args);
    va_end(args);
    return ret;
}
