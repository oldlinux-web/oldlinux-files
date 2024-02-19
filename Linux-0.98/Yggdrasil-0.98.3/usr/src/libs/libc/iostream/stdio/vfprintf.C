#include <stdioprivate.h>
#include <errno.h>

int vfprintf(FILE *fp, const char* format, _G_va_list args)
{
    streambuf* sb = FILE_to_streambuf(fp);
    if (!sb) {
	errno = EBADF;
	return EOF;
    }
    return sb->vform(format, args);
}
