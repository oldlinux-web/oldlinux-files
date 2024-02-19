#ifndef VERSION_H
#define VERSION_H

#include "patchlvl.h"

/*
 *	to get around no valid argv[0] with some compilers...
 */
char	       *myname  = "nroff";
int		patchlevel = PATCHLEVEL;

#ifdef GEMDOS
char	       *version = "nroff (TOS) v1.10 07/30/90 wjr";
#endif
#ifdef MINIX_ST
char	       *version = "nroff (Minix-ST) v1.10 07/30/90 wjr";
#endif
#ifdef MINIX_PC
char	       *version = "nroff (Minix-PC) v1.10 07/30/90 wjr";
#endif
#ifdef UNIX
char	       *version = "nroff (Unix) v1.10 07/30/90 wjr";
#endif

#endif /*VERSION_H*/
