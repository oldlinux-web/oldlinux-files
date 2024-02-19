#include "stdioprivate.h"

#undef feof

int feof(FILE* fp)
{
    return (!__validfp(fp) || (fp->_flags & _S_EOF_SEEN))
	? EOF : 0;
}
