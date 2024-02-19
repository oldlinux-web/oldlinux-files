#include <stdioprivate.h>

extern "C" void _cleanup() { streambuf::flush_all(); } // For GNU libc.

// Linux uses the shared image. So we gave it up.
#if defined(HAVE_GNU_LD) && !defined(linux)
#include <gnu-stabs.h>

text_set_element(__libc_atexit, _cleanup);
#endif
