#include <stdioprivate.h>
#include <stdarg.h>
#include <errno.h>

int vprintf(const char* format, _G_va_list args)
{
    streambuf* sb = FILE_to_streambuf(stdout);
    if (!sb) {
	errno = EBADF;
	return EOF;
    }
    return sb->vform(format, args);
}
