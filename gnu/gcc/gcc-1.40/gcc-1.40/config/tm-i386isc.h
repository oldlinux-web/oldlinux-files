/* Definitions for Intel 386 running Interactive Unix System V.  */

/* Mostly it's like AT&T Unix System V. */

#include "tm-i386v.h"

/* Use crt0.o or crt1.o as a startup file and crtn.o as a closing file.  */
#undef STARTFILE_SPEC
#define STARTFILE_SPEC \
  "%{!shlib:%{posix:%{pg:mcrtp1.o%s}%{!pg:%{p:mcrtp1.o%s}%{!p:crtp0.o%s}}}\
   %{!posix:%{pg:mcrt0.o%s}%{!pg:%{p:mcrt0.o%s}%{!p:crt0.o%s}}\
   %{p:-L/lib/libp} %{pg:-L/lib/libp}}}\
   %{shlib:%{posix:crtp1.o%s}%{!posix:crt1.o%s}} "

#define ENDFILE_SPEC "crtn.o%s"

/* Library spec */
#undef LIB_SPEC
#define LIB_SPEC "%{posix:-lcposix} %{shlib:-lc_s} -lc"

/* caller has to pop the extra argument passed to functions that return
   structures. */

#define STRUCT_RETURN_CALLER_POP
