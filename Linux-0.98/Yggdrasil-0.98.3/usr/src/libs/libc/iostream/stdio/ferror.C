#include "stdioprivate.h"

#undef ferror

int ferror(FILE* fp)
{
    return (!__validfp(fp) || (fp->_flags & _S_ERR_SEEN))
	? EOF : 0;
}
