#ifndef CONFIG_H
#define CONFIG_H

/*
 *	for diffent os, define tos, unix, or minix. for gemdos, pick
 *	a compiler (alcyon, mwc, etc). see makefile for VERSFLAGS.
 *
 *	for atari TOS, do:	cc -Dtos -Dalcyon ...
 *
 *	for minix, do:		cc -D_MINIX -D_ST ...		(ST minix)
 *				cc -D_MINIX ...			(PC minix)
 *
 *	for unix, do:		cc -Dunix ...			(generic)
 *				cc -Dunix -DBSD...		(BSD)
 *
 *	note: so far there is no specific ST minix version. -D_ST is ignored.
 *
 *	nroff uses index/rindex. you may need -Dindex=strchr -Drindex=strrchr
 *	as well. this file is included in "nroff.h" which gets included in all
 *	sources so any definitions you need should be added here.
 *
 *	all os-dependent code is #ifdef'ed with GEMDOS, MINIX_ST, MINIX_PC,
 *	MINIX, or UNIX. most of the differences deal with i/o only.
 */
#ifdef ALCYON
# ifndef tos
#  define tos
# endif
# ifndef alcyon
#  define alcyon
# endif
#endif

#ifdef tos
# define GEMDOS
# undef minix
# undef unix
# undef MINIX
# undef MINIX_ST
# undef MINIX_PC
# undef UNIX
/*#define register*/
#endif

#ifdef alcyon
# ifndef ALCYON
#  define ALCYON			/* for gemdos version, alcyon C */
# endif
# ifndef GEMDOS
#  define GEMDOS
# endif
#endif

#ifdef minix
# ifndef _MINIX
#  define _MINIX
# endif
#endif

#ifdef _MINIX
# define MINIX
# undef tos
# undef unix
# undef GEMDOS
# undef MINIX_ST
# undef MINIX_PC
# undef UNIX
# ifdef _ST
#  ifndef atariST
#   define atariST
#  endif
# endif
# ifdef atariST
#  define MINIX_ST
# else
#  define MINIX_PC
# endif
#endif

#ifdef unix
/* just in case <ctype.h> does not define it (slightly different anyway) */
# undef tos
# undef minix
# undef _MINIX
# undef GEMDOS
# undef _ST
# undef MINIX_ST
# undef MINIX_PC
# ifndef UNIX
#  define UNIX
# endif
#endif

#endif /*CONFIG_H*/

