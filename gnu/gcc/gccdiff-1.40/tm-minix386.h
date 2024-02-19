/* Definitions for Intel 386 running system V with gnu tools
   Copyright (C) 1988 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* This is very like tm-i386gas.h, but the default is soft-float, and
   .comm and .lcomm are rounded (which seems to be relied on by the
   Gnu Emacs garbage collector). */


#include "tm-i386.h"
/* Use the bsd assembler syntax.  */
/* we need to do this because gas is really a bsd style assembler,
 * and so doesn't work well this these att-isms:
 *
 *  ASM_OUTPUT_SKIP is .set .,.+N, which isn't implemented in gas
 *  ASM_OUTPUT_LOCAL is done with .set .,.+N, but that can't be
 *   used to define bss static space
 *
 * Next is the question of whether to uses underscores.  RMS didn't
 * like this idea at first, but since it is now obvious that we
 * need this separate tm file for use with gas, at least to get
 * dbx debugging info, I think we should also switch to underscores.
 * We can keep tm-i386v for real att style output, and the few
 * people who want both form will have to compile twice.
 */

#include "tm-bsd386.h"

/* these come from tm-bsd386.h, but are specific to sequent */
#undef DBX_NO_XREFS
#undef DBX_CONTIN_LENGTH

/* By default, target has no 80387.  */

#define TARGET_DEFAULT 0

/* Specify predefined symbols in preprocessor.  */

#define CPP_PREDEFINES "-Di386"

/* Allow #sccs in preprocessor.  */

#define SCCS_DIRECTIVE

/* Output #ident as a .ident.  */

#define ASM_OUTPUT_IDENT(FILE, NAME) fprintf (FILE, "\t.ident \"%s\"\n", NAME);

/* We do not want to output SDB debugging information.  */

#undef SDB_DEBUGGING_INFO

/* We want to output DBX debugging information.  */

#define DBX_DEBUGGING_INFO

/* Implicit library calls should use memcpy, not bcopy, etc.  */

#define TARGET_MEM_FUNCTIONS

/* When using gas, .align N aligns to an N-byte boundary.  */

#undef ASM_OUTPUT_ALIGN
#define ASM_OUTPUT_ALIGN(FILE,LOG)	\
     if ((LOG)!=0) fprintf ((FILE), "\t.align %d\n", 1<<(LOG))

/* Align labels, etc. at 4-byte boundaries.  */

#define ASM_OUTPUT_ALIGN_CODE(FILE) \
     fprintf ((FILE), "\t.align 4\n");

/* Machines that use the AT&T assembler syntax
   also return floating point values in an FP register.  */
/* Define how to find the value returned by a function.
   VALTYPE is the data type of the value (as a tree).
   If the precise function being called is known, FUNC is its FUNCTION_DECL;
   otherwise, FUNC is 0.  */

#define VALUE_REGNO(MODE) \
  (((MODE)==SFmode || (MODE)==DFmode) ? FIRST_FLOAT_REG : 0)

/* 1 if N is a possible register number for a function value. */

#define FUNCTION_VALUE_REGNO_P(N) ((N) == 0 || (N)== FIRST_FLOAT_REG)

#undef ASM_FILE_START
#define ASM_FILE_START(FILE) \
  fprintf (FILE, "\t.file\t\"%s\"\n", dump_base_name);

/* These override the definitions in tm-bsd386.h */

/* This says how to output an assembler line
   to define a global common symbol.  */

#undef ASM_OUTPUT_COMMON
#define ASM_OUTPUT_COMMON(FILE, NAME, SIZE, ROUNDED)  \
( fputs (".comm ", (FILE)),			\
  assemble_name ((FILE), (NAME)),		\
  fprintf ((FILE), ",%d\n", (ROUNDED)))

/* This says how to output an assembler line
   to define a local common symbol.  */

#undef ASM_OUTPUT_LOCAL
#define ASM_OUTPUT_LOCAL(FILE, NAME, SIZE, ROUNDED)  \
( fputs (".lcomm ", (FILE)),			\
  assemble_name ((FILE), (NAME)),		\
  fprintf ((FILE), ",%d\n", (ROUNDED)))


/* Specs for start file and linker since gcc and ack libraries don't mix */
/* These are identical to the MINIX-68k version - ajm */

#define STARTFILE_SPEC  \
  "%{pg:/usr/gnu/lib/gcrt0.o%s}\
   %{!pg:\
         %{p:/usr/gnu/lib/mcrtso.o%s}\
         %{!p:/usr/gnu/lib/crtso.o%s}}"

#define LIB_SPEC \
  "%{!p:\
        %{!pg:/usr/gnu/lib/libc.a}}\
   %{p:-lgnu-p.olb}\
   %{pg:gnu-p.olb}"



