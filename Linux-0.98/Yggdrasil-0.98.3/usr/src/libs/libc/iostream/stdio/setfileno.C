// Some known programs (xterm, pdksh?) non-portably change the _file
// field of s struct _iobuf.  This kludge allows the same "functionality".
// This code is an undocumented feature for iostream/stdio. Use it at
// your own risk.

#include "stdioprivate.h"

class set_filebuf : public filebuf {
  public:
    void setfd(int i) { _fb._fileno = i; }

};

extern "C" void setfileno(FILE* fp, int i)
{
    if (!__validfp(fp))
	return;
    if (!(fp->_flags & _S_IS_FILEBUF))
	return;
    ((set_filebuf*)fp)->setfd(i);
}
