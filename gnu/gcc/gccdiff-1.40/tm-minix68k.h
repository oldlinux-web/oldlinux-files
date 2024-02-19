/* target def file for MINIX-68k for use with GNU CC */

/* The following Macros control the compilation
 *
 *	CROSS_MINIX	defined when making cross compiler for MINIX
 *	NATIVE_MINIX	defined when making compiler for MINIX
 */

#include "tm-m68k.h"

#ifndef CROSS_MINIX
#undef alloca
#endif

/* See tm-m68k.h.  0 means 68000 with no 68881.  */

#define TARGET_DEFAULT 0

/* -m68020 requires special flags to the assembler.  */

/* -m68000 (on atari) needs this flag to assembler, otherwise pc relative
   code is produced where it should not be (in places where the
   68000 only allows data ALTERABLE addressing modes) (++jrb 03/19/89) */
#define ASM_SPEC "%{m68020:-mc68020}%{!m68020:-m68000}"

/* Names to predefine in the preprocessor for this target machine.  */

#define CPP_PREDEFINES "-Dm68k"

/* default exec dir */
#ifndef STANDARD_EXEC_PREFIX
#ifdef 	CROSS_MINIX			/* Cross-compiler */
#define STANDARD_EXEC_PREFIX "/usr/local/cross-minix/lib/gcc-"
#else					/* Native compiler */
#define STANDARD_EXEC_PREFIX "/usr/gnu/lib/gcc-"
#endif
#endif

/* Alignment of field after `int : 0' in a structure.  */
/* recent gcc's have this as 16, this is left in for the benfit of */
 /* older gcc */
#undef EMPTY_FIELD_BOUNDARY
#define EMPTY_FIELD_BOUNDARY 16

/* Every structure or union's size must be a multiple of 2 bytes.  */

#undef STRUCTURE_SIZE_BOUNDARY
#define STRUCTURE_SIZE_BOUNDARY 16

/* code seems to assume this... */
#define DBX_DEBUGGING_INFO

#ifndef HAVE_VPRINTF
#define HAVE_VPRINTF 1
#endif

#ifdef abort
#undef abort
#define abort fancy_abort
#endif


/* the following stolen from tm-sun3.h, they seem to work better */
/* This is how to output an assembler line defining a `double' constant.  */

#undef ASM_OUTPUT_DOUBLE
#define ASM_OUTPUT_DOUBLE(FILE,VALUE)					\
   fprintf (FILE, "\t.double 0r%.20e\n", (VALUE))

/* This is how to output an assembler line defining a `float' constant.  */

#undef ASM_OUTPUT_FLOAT
#define ASM_OUTPUT_FLOAT(FILE,VALUE)					\
   fprintf (FILE, "\t.single 0r%.20e\n", (VALUE))

#undef ASM_OUTPUT_FLOAT_OPERAND
#define ASM_OUTPUT_FLOAT_OPERAND(FILE,VALUE)				\
   fprintf (FILE, "#0r%.9g", (VALUE))

#undef ASM_OUTPUT_DOUBLE_OPERAND
#define ASM_OUTPUT_DOUBLE_OPERAND(FILE,VALUE)				\
   fprintf (FILE, "#0r%.20g", (VALUE))

/* Specs for start file and link specs */

#ifdef CROSS_MINIX		/* Cross compiler */
#define STARTFILE_SPEC  \
  "%{pg:/usr/local/cross-minix/lib/gcrt0.o%s}\
   %{!pg:\
         %{p:/usr/local/cross-minix/lib/mcrtso.o%s}\
         %{!p:%{mshort:/usr/local/cross-minix/lib/crtso.o%s}\
              %{!mshort:/usr/local/cross-minix/lib/crtso32.o%s}}}"
#else				/* Native compiler */
#define STARTFILE_SPEC  \
  "%{pg:/usr/gnu/lib/gcrt0.o%s}\
   %{!pg:\
         %{p:/usr/gnu/lib/mcrtso.o%s}\
         %{!p:%{mshort:/usr/gnu/lib/crtso.o%s}\
              %{!mshort:/usr/gnu/lib/crtso32.o%s}}}"
#endif 

#ifdef CROSS_MINIX		/* Cross compiler */
#define LIB_SPEC \
  "%{!p:\
        %{!pg:\
              %{mshort:/usr/local/cross-minix/lib/libc.a}\
              %{!mshort:/usr/local/cross-minix/lib/libc32.a}}}\
   %{p:-lgnu-p.olb}\
   %{pg:gnu-p.olb}"
#else
#define LIB_SPEC \
  "%{!p:\
        %{!pg:\
              %{mshort:/usr/gnu/lib/libc.a}\
              %{!mshort:/usr/gnu/lib/libc32.a}}}\
   %{p:-lgnu-p.olb}\
   %{pg:gnu-p.olb}"
#endif

