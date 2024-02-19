/* m- file for the Motorola delta running System V.3.
   tested on sys1147 (mvme147 - based system).
   Copyright (C) 1986 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


/* The following three symbols give information on
 the size of various data types.  */

#define SHORTBITS 16		/* Number of bits in a short */

#define INTBITS 32		/* Number of bits in an int */

#define LONGBITS 32		/* Number of bits in a long */

/* Define BIG_ENDIAN iff lowest-numbered byte in a word
   is the most significant byte.  */

#define BIG_ENDIAN

/* Define NO_ARG_ARRAY if you cannot take the address of the first of a
 * group of arguments and treat it as an array of the arguments.  */

/* #define NO_ARG_ARRAY */

/* Define WORD_MACHINE if addresses and such have
 * to be corrected before they can be used as byte counts.  */

/* #define WORD_MACHINE */

/* Define how to take a char and sign-extend into an int.
   On machines where char is signed, this is a no-op.  */

#define SIGN_EXTEND_CHAR(c) (c)

/* Now define a symbol for the cpu type, if your compiler
   does not define it automatically:
   vax, m68000, ns16000, pyramid, orion, tahoe and APOLLO
   are the ones defined so far.  */
#define m68000
#define NO_REMAP

#define HAVE_SYSVIPC

#define HAVE_PTYS
#define SYSV_PTYS

/* Use type int rather than a union, to represent Lisp_Object */
/* This is desirable for most machines.  */

#define NO_UNION_TYPE 
#define SWITCH_ENUM_BUG
/* Define EXPLICIT_SIGN_EXTEND if XINT must explicitly sign-extend
   the 24-bit bit field into an int.  In other words, if bit fields
   are always unsigned.

   If you use NO_UNION_TYPE, this flag does not matter.  */

#define EXPLICIT_SIGN_EXTEND

/* Data type of load average, as read out of kmem.  */

/* #define LOAD_AVE_TYPE long */

/* Convert that into an integer that is 100 for a load average of 1.0  */

/* #define LOAD_AVE_CVT(x) (int) (((double) (x)) * 100.0 / FSCALE) */

/* Define CANNOT_DUMP on machines where unexec does not work.
   Then the function dump-emacs will not be defined
   and temacs will do (load "loadup") automatically unless told otherwise.  */

/* #define CANNOT_DUMP */

/* Define VIRT_ADDR_VARIES if the virtual addresses of
   pure and impure space as loaded can vary, and even their
   relative order cannot be relied on.

   Otherwise Emacs assumes that text space precedes data space,
   numerically.  */

/* #define VIRT_ADDR_VARIES */

/* Define C_ALLOCA if this machine does not support a true alloca
   and the one written in C should be used instead.
   Define HAVE_ALLOCA to say that the system provides a properly
   working alloca function and it should be used.
   Define neither one if an assembler-language alloca
   in the file alloca.s should be used.  */

/*#define C_ALLOCA */
/*#define HAVE_ALLOCA */

#ifdef __GNUC__
/* easy. use builtin one. also be sure that no other ones are tried out. */
# define alloca __builtin_alloca
# define HAVE_ALLOCA
# undef C_ALLOCA
#else /* not __GNUC__, use `c' one. */
#define C_ALLOCA
#undef HAVE_ALLOCA
#define STACK_DIRECTION (-1)	 /* C_ALLOCA needs to know about stack. */
#endif /* __GNUC__ */

/* The standard C library is -lcieee, not -lc.
   Also use the PW library, which contains alloca.
   DO NOT USE -lPW. That version of alloca is broken, at last until version
   SVR3V6. -riku@field.fi */

#define LIB_STANDARD -lc

#define LIBS_TERMCAP -lcurses

/* SELECT and SOCKETS under testing. -no avail -riku */
/**#define HAVE_SELECT */
/**#define HAVE_SOCKETS */

#ifdef HAVE_X_WINDOWS
#lossage
/* Some bug exists when X is used, and prevents dumping.  */

/* X library implements these. */
# define BSTRING
# define HAVE_RANDOM
# define HAVE_RENAME
/* X library is in 'nonstandard' location. */
# define LD_SWITCH_MACHINE -L/usr/lib/X11/
#else
/* No sufficient justification for this.  */
/* # define C_DEBUG_SWITCH */
#endif /* HAVE_X_WINDOWS */

/* enable batdevice-dependent code to compile. */
#define BAT68K
