#include <stdioprivate.h>
#include <stdarg.h>
#include <errno.h>

int printf(const char* format, ...)
{
    streambuf* sb = FILE_to_streambuf(stdout);
    if (!sb) {
	errno = EBADF;
	return EOF;
    }
    va_list args;
    va_start(args, format);
    int ret = sb->vform(format, args);
    va_end(args);
    return ret;
}
