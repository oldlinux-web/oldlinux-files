/* machines.h --
   Included file in the makefile that gets run through Cpp.  This file
   tells which machines have what features based on the unique machine
   identifier present in Cpp. */

/* **************************************************************** */
/*								    */
/*			Sun Miscrosystems Machines      	    */
/*								    */
/* **************************************************************** */

#if defined (sun)
#  if defined (mc68010)
#    define sun2
#  endif
#  if defined (mc68020)
#    define sun3
#  endif
#  if defined (sparc)
#    define sun4
#  endif
#  if defined (i386)
#    define Sun386i
#    undef i386
#  endif
#if defined (HAVE_SHARED_LIBS)
#  define M_OS SunOS4
#else
#  define M_OS SunOS3
#endif
#endif /* sun */

/* ************************ */
/*			    */
/*	    Sun2	    */
/*			    */
/* ************************ */
#if defined (sun2)
#define M_MACHINE "sun2"
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#define HAVE_SETLINEBUF
#define HAVE_VPRINTF
#ifndef HAVE_GCC
#define HAVE_ALLOCA
#endif
#endif /* sun2 */

/* ************************ */
/*			    */
/*	    Sun3	    */
/*			    */
/* ************************ */
#if defined (sun3)
#define M_MACHINE "sun3"
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#define HAVE_SETLINEBUF
#define HAVE_VPRINTF
#ifndef HAVE_GCC
#define HAVE_ALLOCA
#endif
#endif /* sun3 */

/* ************************ */
/*			    */
/*	    Sparc	    */
/*			    */
/* ************************ */
#if defined (sun4)
#define M_MACHINE "sparc"
#define HAVE_SIGLIST
#define HAVE_SETLINEBUF
#define HAVE_VPRINTF
#define USE_GNU_MALLOC
#ifndef HAVE_GCC
#define HAVE_ALLOCA
#endif
#endif /* sparc */

/* ************************ */
/*			    */
/*	    Sun386i	    */
/*			    */
/* ************************ */
#if defined (Sun386i)
#define M_MACHINE "Sun386i"
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#define HAVE_SETLINEBUF
#define HAVE_VPRINTF
#ifndef HAVE_GCC
#define HAVE_ALLOCA
#endif
#endif /* Sun386i */

/* **************************************************************** */
/*								    */
/*			DEC Machines (vax, decstations)   	    */
/*								    */
/* **************************************************************** */

/* ************************ */
/*			    */
/*	    Vax		    */
/*			    */
/* ************************ */
#if defined (vax)
#define M_MACHINE "vax"
#if defined (ultrix)
#  define HAVE_VPRINTF
#  define M_OS Ultrix
#else
#  define M_OS Bsd
#endif /* ultrix */
#define HAVE_SETLINEBUF
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#endif /* vax */

/* ************************ */
/*			    */
/*	    MIPSEL	    */
/*			    */
/* ************************ */
#if defined (MIPSEL)
#define M_MACHINE "MIPSEL"
#if defined (ultrix)
#  define M_OS Ultrix
#else
#  define M_OS Bsd
#endif
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#define HAVE_SETLINEBUF
#define HAVE_VPRINTF
#ifndef HAVE_GCC
#define HAVE_ALLOCA
#endif
#endif /* MIPSEL */

/* ************************ */
/*			    */
/*	    MIPSEB	    */
/*			    */
/* ************************ */
#if defined (MIPSEB)
#define M_MACHINE "MIPSEB"
#define M_OS Bsd
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#define HAVE_SETLINEBUF
#define HAVE_VPRINTF
#ifndef HAVE_GCC
#define HAVE_ALLOCA
#endif
#define SYSDEP_CFLAGS -systype bsd43
#endif /* MIPSEB */

/* ************************ */
/*                          */
/*          Pyramid         */
/*                          */
/* ************************ */
#if defined (pyr)
#define M_MACHINE "Pyramid"
#define M_OS Bsd
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#define HAVE_SETLINEBUF
/* We do have vfprinf and varargs, but I get an syntax error at line
   486 of make_cmd if I use them ?! */
/* #define HAVE_VPRINTF */
#ifndef HAVE_GCC
#define HAVE_ALLOCA
#endif
#endif /* pyr */

/* ************************ */
/*			    */
/*	    IBMRT	    */
/*			    */
/* ************************ */
#if defined (ibm032)
#define M_MACHINE "IBMRT"
#define M_OS Bsd
#define HAVE_SIGLIST
#define HAVE_SETLINEBUF
#define USE_GNU_MALLOC
#define SYSDEP_CFLAGS -ma
#endif /* ibm032 */

/* ************************ */
/*			    */
/*	    i386	    */
/*			    */
/* ************************ */
#if defined (i386)
#undef i386
#define M_MACHINE "i386"
#define M_OS SYSV
#define SYSDEP_CFLAGS -DNO_WAIT_H
#if !defined (HAVE_GCC)
#  define HAVE_ALLOCA
#  define REQUIRED_LIBRARIES -lPW
#endif /* !HAVE_GCC */
#define USE_GNU_MALLOC
#endif /* i386 */

/* ************************ */
/*			    */
/*	    NeXT	    */
/*			    */
/* ************************ */
#if defined (NeXT)
#define M_MACHINE "NeXT"
#define M_OS Bsd
#define HAVE_ALLOCA
#define HAVE_VPRINTF
#define HAVE_SIGLIST
#endif

/* ************************ */
/*			    */
/*	    hp9000	    */
/*			    */
/* ************************ */
#if defined (hp9000) && !defined (hpux)
#define M_MACHINE "hp9000"
#define M_OS Bsd
#define HAVE_SIGLIST
#define HAVE_SETLINEBUF
#define USE_GNU_MALLOC
#endif /* hp900 && !hpux */

/* ************************ */
/*			    */
/*	    hpux	    */
/*			    */
/* ************************ */
#if defined (hpux)
#define M_MACHINE "hpux"
#define M_OS HPUX
#define REQUIRED_LIBRARIES -lBSD
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#endif /* hpux */

/* ************************ */
/*			    */
/*	    Xenix286	    */
/*			    */
/* ************************ */
#if defined (Xenix286)
#define M_MACHINE "i286"
#define M_OS "Xenix"
#define USE_GNU_MALLOC
#define REQUIRED_LIBRARIES -lx
#endif

/* ************************ */
/*			    */
/*	    Xenix286	    */
/*			    */
/* ************************ */
#if defined (Xenix286)
#define M_MACHINE "i386"
#define M_OS "Xenix"
#define ALLOCA_ASM x386-alloca.s
#define USE_GNU_MALLOC
#define REQUIRED_LIBRARIES -lx
#endif

/* ************************ */
/*			    */
/*	    convex	    */
/*			    */
/* ************************ */
#if defined (convex)
#define M_MACHINE "convex"
#define M_OS SYSV
#define HAVE_VPRINTF
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#endif /* convex */

/* ************************ */
/*			    */
/*	    sony	    */
/*			    */
/* ************************ */
#if defined (sony)
#define M_MACHINE "sony"
#define M_OS Bsd
#define HAVE_SIGLIST
#define HAVE_SETLINEBUF
#define USE_GNU_MALLOC
#endif /* sony */

/* ************************ */
/*			    */
/*	    aix		    */
/*			    */
/* ************************ */
#if defined (aix)
#define M_MACHINE "aix"
#define M_OS Bsd
#define HAVE_VPRINTF
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#endif /* aix */

/* ************************ */
/*			    */
/*	    att3b	    */
/*			    */
/* ************************ */
#if defined (att3b)
#define M_MACHINE "att3b"
#define M_OS SYSV
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#endif /* att3b */

/* ************************ */
/*			    */
/*	    att386	    */
/*			    */
/* ************************ */
#if defined (att386)
#define M_MACHINE "att386"
#define M_OS SYSV
#if !defined (HAVE_GCC)
#define HAVE_ALLOCA
#define REQUIRED_LIBRARIES -lPW
#endif /* HAVE_GCC */
#define HAVE_SIGLIST
#define USE_GNU_MALLOC
#endif /* att386 */

/* ************************ */
/*			    */
/*	    Encore	    */
/*			    */
/* ************************ */
#if defined (MULTIMAX)
#if defined (n16)
#define M_MACHINE "Multimax32k"
#else
#define M_MACHINE "Multimax"
#endif /* n16 */
#if defined (CMU)
#define M_OS Mach
#else
#define M_OS Bsd
#endif
#define HAVE_SIGLIST
#ifndef HAVE_GCC
#define HAVE_ALLOCA
#endif
#define USE_GNU_MALLOC
#define HAVE_SETLINEBUF
#endif  /* MULTIMAX */

/* ************************ */
/*			    */
/*	    clipper	    */
/*			    */
/* ************************ */
/* This is for the Orion 1/05 (A BSD 4.2 box based on a Clipper processor */
#if defined (clipper)
#define M_MACHINE "clipper"
#define M_OS Bsd
#define HAVE_ALLOCA
#define USE_GNU_MALLOC
#define HAVE_SETLINEBUF
#endif  /* clipper */

/* **************************************************************** */
/*								    */
/*			Generic Entry   			    */
/*				        			    */
/* **************************************************************** */

/* Use this entry for your machine if it isn't represented here.  It
   is based on a Vax running Bsd. */

#if !defined (M_MACHINE)
#define UNKNOWN_MACHINE
#endif

#ifdef UNKNOWN_MACHINE
#define M_MACHINE "UNKNOWN_MACHINE"
#define M_OS UNKNOWN_OS

/* Required libraries for building on this system. */
#define REQUIRED_LIBRARIES

/* Define HAVE_SIGLIST if your system has sys_siglist[]. */
#define HAVE_SIGLIST

/* Define HAVE_ALLOCA if you are not using Gcc, but your library
   or compiler has a version of alloca ().  Otherwise, we will use
   our version of alloca () in alloca.c */
#ifndef HAVE_GCC
#define HAVE_ALLOCA
#endif

/* Define USE_GNU_MALLOC if you want to use the debugging version
   of malloc ().  We like this on by default. */
#define USE_GNU_MALLOC

/* Define USE_GNU_TERMCAP if you want to use the GNU termcap library
   instead of your system termcap library. */
/* #define USE_GNU_TERMCAP */

/* Define HAVE_SETLINEBUF if your machine has the setlinebuf ()
   stream library call.  Otherwise, setvbuf () will be used.  If
   neither of them work, you can edit in your own buffer control
   based upon your machines capabilities. */
#define HAVE_SETLINEBUF

/* Define HAVE_VPRINTF if your machines has the vprintf () library
   call.  Otherwise, printf will be used.  */
#define HAVE_VPRINTF

/* Define SYSDEP_CFLAGS to be the flags to cc that make your compiler
   work.  For example, `-ma' on the RT makes alloca () work. */
#define SYSDEP_CFLAGS

#endif  /* UNKNOWN_MACHINE */
