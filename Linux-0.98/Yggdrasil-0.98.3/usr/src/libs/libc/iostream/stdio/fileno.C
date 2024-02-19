#include "stdioprivate.h"

int fileno(FILE* fp)
{
  return (!__validfp(fp) || !(fp->_flags & _S_IS_FILEBUF))
	? EOF : ((filebuf*)fp)->fd();

}
