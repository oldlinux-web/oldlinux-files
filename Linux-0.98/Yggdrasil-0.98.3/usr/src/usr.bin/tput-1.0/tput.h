/* tput.h -- constants and declarations for tput
   Copyright (C) 1989 Free Software Foundation, Inc.

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

/* Capability types. */
#define BOOL 1			/* Boolean capability. */
#define NUM  2			/* Numeric capability. */
#define STR  4			/* String capability. */
#define PAD  8			/* String accepts padding. */

struct conversion
{
  char *info;			/* Terminfo name. */
  char *cap;			/* Termcap name. */
  char type;			/* Capability type. */
};

/* The number of entries in `conversions'. */

#define CAPABILITIES 310

extern struct conversion conversions[];
