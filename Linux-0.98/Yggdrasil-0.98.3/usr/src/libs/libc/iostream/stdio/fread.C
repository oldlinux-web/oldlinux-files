#include "stdioprivate.h"
#include "errno.h"

size_t fread(void *buf, size_t size, size_t count, FILE* fp)
{
    streambuf* sb = FILE_to_streambuf(fp);
    if (!sb) {
	errno = EBADF;
	return 0;
    }
    size_t bytes_requested = size*count;
    size_t bytes_read = sb->sgetn(buf, bytes_requested);
    return bytes_requested == bytes_read ? count : bytes_read / size;
}
