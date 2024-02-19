/*
 *   This is dvips, a freely redistributable PostScript driver
 *   for dvi files.  It is (C) Copyright 1987 by Tomas Rokicki.
 *   You may modify and use this program to your heart's content,
 *   so long as you send modifications to Tomas Rokicki.  It can
 *   be included in any distribution, commercial or otherwise, so
 *   long as the banner string defined in structures.h is not
 *   modified (except for the version number) and this banner is
 *   printed on program invocation, or can be printed on program
 *   invocation with the -? option.
 */
#include <stdio.h>		/* for FILE and fopen */
#include <sys/param.h>		/* for MAXPATHLEN */
#include <sys/file.h>		/* for access and F_OK */
#include <strings.h>		/* for strcat, strlen and rindex */
#include "structures.h"
#include "debug.h"
#ifdef _POSIX_SOURCE
#include <unistd.h>
#endif /* _POSIX_SOURCE */

#ifdef DEBUG
extern integer debug_flag;
#endif	/* DEBUG */

/* directories are separated in the path by PATHSEP */
#define PATHSEP		':'
/* DIRSEP is the char that separates directories from files */
#define DIRSEP		'/'
extern void error() ;
/*
 *   search takes a directory list, separated by PATHSEP, and tries to
 *   open a file.  null directory components indicate current
 *   directory. if the file SUBDIR exists and the file is a font file,
 *   it checks for the file in a subdirectory named the same as the font name.
 *   returns the open file descriptor if ok, else NULL.
 *
 *   we hope MAXPATHLEN is enough -- rudimentary checking!
 */
FILE *
search(path, file)
	char *path, *file ;
{
   register char *nam ;			/* index into fname */
   register char *ext ;			/* point to extension(if any) */
   register char *tmp ;			/* tmp */
   register FILE *fd ;			/* file desc of file */
   char fname[MAXPATHLEN] ;		/* to store file name */

   do {
      /* copy the current directory into fname */
      nam = fname;
      /* copy till PATHSEP */
      while (*path != PATHSEP && *path) *nam++ = *path++;
      if (nam == fname) *nam++ = '.';	/* null component is current dir */
      *nam++ = DIRSEP;			/* add separator */

      /* check for SUBDIR */
      (void)strcpy(nam,"SUBDIR");
      if (access(fname,F_OK) == 0		/* if SUBDIR exists */
            && (ext=rindex(file, '.')) != NULL	/* and file has an extension */
	    && strncmp(ext,".tfm",4)) {		/* and it is not a tfm file */
	 /* add the stuff before the extension to the current directory */
	 for (tmp = file; tmp < ext; *nam++ = *tmp++);
	 *nam++ = DIRSEP;		/* add separator */
      }

      (void)strcpy(nam,file);			/* tack the file on */

      /* belated check -- bah! */
      if ((nam - fname) + strlen(file) + 1 > MAXPATHLEN)
	 error("! overran allocated storage in search()");

#ifdef DEBUG
      if (dd(D_PATHS))
      	 fprintf(stderr,"Trying to open %s\n", fname) ;
#endif
      if ((fd=fopen(fname,"r")) != NULL)
	 return(fd);

   /* skip over PATHSEP and try again */
   } while (*(path++));

   return(NULL);

}		/* end search */
