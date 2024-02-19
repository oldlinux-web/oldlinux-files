#include "stdioprivate.h"

#undef clearerr

void clearerr(FILE* fp)
{
    if (!__validfp(fp))
	return;
    fp->_flags &= ~(_S_ERR_SEEN|_S_EOF_SEEN);
}
