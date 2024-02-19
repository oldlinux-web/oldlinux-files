#include "stdioprivate.h"
#include <errno.h>

int fclose (FILE *stream)
{
    if (!__validfp(stream)) {
	errno = EBADF;
	return EOF;
    }
    
    int status = 0;
    if (stream->_flags & _S_IS_FILEBUF) {
	status = ((filebuf*)stream)->close() == NULL ? EOF : 0;
	if (stream != stdin && stream != stdout && stream != stderr)
	    delete (filebuf*)stream;
    }
    else
	delete (streambuf*)stream;
    return status;
}
