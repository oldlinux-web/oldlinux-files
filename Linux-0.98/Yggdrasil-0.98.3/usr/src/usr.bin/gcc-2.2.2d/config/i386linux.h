/* Definitions for Intel 386 running Linux
 * Copyright (C) 1992 Free Software Foundation, Inc.
 *
 * Written by H.J. Lu (hlu@eecs.wsu.edu)
 *
 * Linux is a POSIX.1 compatible UNIX clone for i386, which uses GNU
 * stuffs as the native stuffs.
 */

#if 0	/* The FSF has fixed the known bugs. But ....... */

/* Linux has a hacked gas 1.38.1, which can handle repz, repnz
 * and fildll.
 */

#define GOOD_GAS

#endif

#include "i386gas.h"

/* Specify predefined symbols in preprocessor.  */

#undef CPP_PREDEFINES
#define CPP_PREDEFINES "-Dunix -Di386 -Dlinux"

#undef SIZE_TYPE
#define SIZE_TYPE "unsigned int"

#undef PTRDIFF_TYPE
#define PTRDIFF_TYPE "int"

#undef WCHAR_TYPE
#define WCHAR_TYPE "long int"

#undef WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE BITS_PER_WORD

#undef HAVE_ATEXIT
#define HAVE_ATEXIT

/* Linux uses ctype from glibc.a. I am not sure how complete it is.
 * For now, we play safe. It may change later.
 */
#if 0
#undef MULTIBYTE_CHARS
#define MULTIBYTE_CHARS	1
#endif

#undef LIB_SPEC
#define LIB_SPEC "%{g*:-lg} %{!g*:%{!p:%{!pg:-lc}}%{p:-lgmon -lc_p}%{pg:-lgmon -lc_p}}"

#undef STARTFILE_SPEC
#ifdef NO_JUMPTABLE
#define STARTFILE_SPEC  \
  "%{pg:gcrt0.o%s}%{!pg:%{p:gcrt0.o%s}%{!p:crt0.o%s %{!g:%{!static:-L"STANDARD_STARTFILE_PREFIX"shared}}}}"
#else
#define STARTFILE_SPEC  \
  "%{g*:crt0.o%s} %{!g*:%{pg:gcrt0.o%s} %{!pg:%{p:gcrt0.o%s} %{!p:crt0.o%s %{!static:%{jump:-L"STANDARD_STARTFILE_PREFIX"jump} %{!jump:-L"STANDARD_STARTFILE_PREFIX"shared}}}}}"
#endif
