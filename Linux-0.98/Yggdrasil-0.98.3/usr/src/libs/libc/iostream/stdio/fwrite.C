#include <stdioprivate.h>
#include <errno.h>

size_t fwrite(const void *buf, size_t size, size_t count, FILE *fp)
{
    streambuf* sb = FILE_to_streambuf(fp);
    if (!sb) {
	errno = EBADF;
	return 0;
    }
    size_t request = size*count;
    size_t written = sb->sputn(buf, request);
    if (written == request)
	return count;
    else
	return written / size;
}
