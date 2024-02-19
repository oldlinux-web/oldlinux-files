/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	POSIX Standard: 9.2.2 User Database Access	<pwd.h>
 */

#ifndef	_PWD_H
#define	_PWD_H	1

#include <traditional.h>
#include <features.h>
#include <sys/types.h>

/* The passwd structure.  */
struct passwd
  {
    char *pw_name;	/* Username.  */
    char *pw_passwd;	/* Password.  */
    uid_t pw_uid;	/* User ID.  */
    gid_t pw_gid;	/* Group ID.  */
    char *pw_gecos;	/* Real name.  */
    char *pw_dir;	/* Home directory.  */
    char *pw_shell;	/* Shell program.  */
  };


#if defined(__USE_SVID) || defined(__USE_GNU)
#define	__need_FILE
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __USE_GNU
/* Return a new stream open on the password file.  */
extern FILE *__pwdopen _ARGS ((void));

/* Read a password entry from STREAM, filling in P.
   Return the `struct passwd' of P if successful, NULL on failure.  */
extern struct passwd *__pwdread _ARGS ((FILE *__stream, void *__p));

/* Return a chunk of memory containing pre-initialized data for __pwdread.  */
extern void *__pwdalloc _ARGS ((void));
#endif


#if defined (__USE_SVID) || defined(__USE_MISC)
/* Rewind the password-file stream.  */
extern void setpwent _ARGS ((void));

/* Close the password-file stream.  */
extern void endpwent _ARGS ((void));

/* Read an entry from the password-file stream, opening it if necessary.  */
extern struct passwd *getpwent _ARGS ((void));
#endif

#ifdef	__USE_SVID
/* Read an entry from STREAM.  */
extern struct passwd *fgetpwent _ARGS ((FILE *__stream));

/* Write the given entry onto the given stream.  */
extern int putpwent _ARGS ((const struct passwd *__p, FILE *__f));
#endif

/* Search for an entry with a matching user ID.  */
extern struct passwd *getpwuid _ARGS ((uid_t __uid));

/* Search for an entry with a matching username.  */
extern struct passwd *getpwnam _ARGS ((const char *__name));

#ifdef __cplusplus
}
#endif

#endif	/* pwd.h  */
