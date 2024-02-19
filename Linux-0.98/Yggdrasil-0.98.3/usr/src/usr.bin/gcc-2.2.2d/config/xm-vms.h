/* Configuration for GNU C-compiler for Vax.
   Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* #defines that need visibility everywhere.  */
#define FALSE 0
#define TRUE 1

/* target machine dependencies.
   tm.h is a symbolic link to the actual target specific file.   */
#include "tm.h"

/* This describes the machine the compiler is hosted on.  */
#define HOST_BITS_PER_CHAR 8
#define HOST_BITS_PER_SHORT 16
#define HOST_BITS_PER_INT 32
#define HOST_BITS_PER_LONG 32
#define HOST_BITS_PER_LONGLONG 64

#define SUCCESS_EXIT_CODE 1
#define FATAL_EXIT_CODE (44 | 0x10000000)  /* Failure, and no DCL message.  */


/* A couple of conditionals for execution machine are controlled here.  */
#ifndef VMS
#define VMS
#endif

#ifndef __GNUC__
/* not present, at least in VAX-11 C (VMS) v2.2 */
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0

#define unlink delete
#endif

/* If compiled with GNU C, use the built-in alloca */
#ifdef __GNUC__
#define alloca __builtin_alloca
#endif

#define GCC_INCLUDE_DIR "///not used with VMS///"	/* nonsense string for now */

/* and define a local equivalent (sort of) for unlink */
#define unlink remove

/* Specify the list of include file directories.  */
#define INCLUDE_DEFAULTS \
{									\
  { "GNU_GXX_INCLUDE:", 1},						\
  { "GNU_CC_INCLUDE:", 0},	/* GNU includes */			\
  { "SYS$SYSROOT:[SYSLIB.]", 0}, /* VAX-11 "C" includes */		\
  { ".", 0},			/* Make normal VMS filespecs work.  */	\
  { 0, 0}								\
}

/* Under VMS a directory specification can be enclosed either in square
   brackets or in angle brackets.  Thus we need to check both.  This
   macro is used to help compare filenames in cp-lex.c.

   We also need to make sure that the names are all lower case, because
   we must be able to compare filenames to determine if a file implements
   a class.  */

#define FILE_NAME_NONDIRECTORY(C)				\
({								\
   char * pnt_ = (C), * pnt1_;					\
   pnt1_ = pnt_ - 1;						\
   while (*++pnt1_)						\
     if ((*pnt1_ >= 'A' && *pnt1_ <= 'Z')) *pnt1_ |= 0x20;	\
   pnt1_ = rindex (pnt_, ']'); 					\
   pnt1_ = (pnt1_ == 0 ? rindex (pnt_, '>') : pnt1_);		\
   pnt1_ = (pnt1_ == 0 ? rindex (pnt_, ':') : pnt1_);		\
   (pnt1_ == 0 ? pnt_ : pnt1_ + 1);				\
 })

/* Macro to generate the name of the cross reference file.  The standard
   one does not work, since it was written assuming that the conventions
   of a unix style filesystem will work on the host system.  */

#define XREF_FILE_NAME(BUFF, NAME)	\
  s = FILE_NAME_NONDIRECTORY (NAME);			\
  if (s == NAME) sprintf(BUFF, "%s_gxref", NAME);	\
  else {						\
    strcpy(BUFF, NAME);					\
    strcat(t, "_gxref");				\
  }

/* Macro that is used in cp-xref.c to determine whether a file name is
   absolute or not.  */

#define FILE_NAME_ABSOLUTE_P(NAME)	\
	(FILE_NAME_NONDIRECTORY (NAME) != (&NAME[1]))

/* FILE_NAME_JOINER is defined to be the characters that are inserted between 
   a directory name and a filename in order to make an absolute file
   specification.  Under VMS the directory specification contains all of the
   required characters, so we define this to be a null string.  */

#define FILE_NAME_JOINER ""

