/* stddef.h
	This file provides a definition for size_t and align_t that
	should work for your system. If it does not, it is up to you to
	make it the right thing. The problem is that I cannot rely upon
	<sys/params.h> to do the right thing on machines which don't
	yet have ansi header files. Note that on many RISC machines,
	align_t must be at least 32 bits wide, and sparc doubles are
	aligned on 64 bit boundaries, but of course, rc does not use
	doubles in its code, so the "typedef long ALIGN_T" is good
	enough in the sparc's case. Also for performance reasons on a
	VAX one would probably want align_t to be 32 bits wide.
*/
typedef long ALIGN_T;
typedef unsigned int SIZE_T;
typedef short int MODE_T;
typedef int PID_T;

#ifndef NULL
#define NULL 0
#endif
