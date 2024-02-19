/* Declarations for termcap library.
   Copyright (C) 1991, 1992 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef _TERMCAP_H
#define _TERMCAP_H 1

#include <sys/types.h>

#ifdef __STDC__

typedef int (*outfuntype) (int);

#ifdef __cplusplus
extern "C" {
#endif

extern int tgetent (char *__buffer, char *__termtype);

extern int tgetnum (char *__name);
extern int tgetflag (char *__name);
extern char *tgetstr (char *__name, char **__area);

extern char PC;
#ifdef linux
extern speed_t ospeed;
#else
extern short ospeed;
#endif
extern int tputs (char *__string, int __nlines, outfuntype);

extern char *tparam (char *__ctlstring, char *__buffer, int __size, ...);

extern char *UP;
extern char *BC;

extern char *tgoto (char *__cstring, int __hpos, int __vpos);

#ifdef __cplusplus
}
#endif

#else /* not ANSI C */

extern int tgetent ();

extern int tgetnum ();
extern int tgetflag ();
extern char *tgetstr ();

extern char PC;
extern short ospeed;

extern int tputs ();

extern char *tparam ();

extern char *UP;
extern char *BC;

extern char *tgoto ();

#endif /* not ANSI C */

#endif /* not _TERMCAP_H */
