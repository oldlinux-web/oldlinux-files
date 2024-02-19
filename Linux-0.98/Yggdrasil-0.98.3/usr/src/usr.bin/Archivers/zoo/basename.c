#ifndef LINT
/* @(#) basename.c 2.2 87/12/27 13:42:40 */
static char sccsid[]="@(#) basename.c 2.2 87/12/27 13:42:40";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
*/

#include "options.h"
#include "zooio.h"
#include "zoo.h"
#include "parse.h"
#include "various.h"
#include "zoofns.h"
#include "debug.h"
#include "assert.h"

/* This function strips device/directory information from
a pathname and returns just the plain filename */
void basename (pathname, fname)
char *pathname;
char fname[];
{
   strcpy (fname, nameptr (pathname));
}

/* Set of legal MSDOS filename characters.  The working of cvtchr() depends
on the order of the first few characters here.  In particular, '_' is
positioned so '.' gets converted to it. */
static char legal[] = 
"tabcdefghijklmnopqrs_uvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ@^`{}~!#$%&'()-";

/****************
cvtchr() converts a character to a lowercase alphabetic character in
a somewhat random way.  
*/
#define  cvtchr(ch)        legal[(ch & 0xff) % 26]

/***************
cleanup() cleans up a string so it contains only legal MSDOS filename
characters.  Any other characters are converted to an underscore.
If the filename is null or if it begins with a dot, it is fixed.
All dots are also converted.
*/

void cleanup (p)
char *p;
{
   assert(p != NULL);
   if (*p == '\0')
      strcpy (p, "X");
   if (*p == '.')
      *p = '_';
   while (*p != '\0') {
      if (strchr (legal, *p) == NULL) {   /* if invalid character */
         *p = cvtchr(*p);
      }
      p++;
   }
}
/* This function strips device/directory information from a pathname,
forces the remaining filename to MSDOS format, and returns it.  Any
illegal characters are fixed.
*/
void dosname (pathname, fname)
char *pathname;
char fname[];
{
   struct path_st path_st;
   parse (&path_st, pathname);
   strcpy (fname, path_st.fname);
   cleanup (fname);

#ifdef VER_CH  /* remove any trailing extension field */
   if (path_st.ext[0] != '\0')
      strip_ver (path_st.ext);
#endif

   /* extension could have been nulled, so we test again */
   if (path_st.ext[0] != '\0') {
      cleanup (path_st.ext);
      strcat (fname, ".");
      strcat (fname, path_st.ext);
   }

#ifdef SPECMOD
   specfname (fname);
#endif
}

/* rootname() */
/* Accepts a pathname.  Returns the root filename, i.e., with both the
directory path and the extension stripped. */

void rootname (path, root)
char *path, *root;
{
   char *p;
   static char dot[] = {EXT_CH, '\0'};
   strcpy(root, nameptr(path));           /* copy all but path prefix */
   p = findlast(root, dot);               /* find last dot */
   if (p != NULL)                         /* if found ... */
      *p = '\0';                          /* ... null it out */
}

/* nameptr() */
/* Accepts a pathname.  Returns a pointer to the filename within
that pathname.
*/

char *nameptr (path)
char *path;
{
   char *t;
   t = findlast (path, PATH_SEP);   /* last char separating device/directory */
   debug ((printf ("nameptr:  findlast returned ptr to string [%s].\n",t)))
   if (t == NULL)                /* no separator */
      return (path);
   else {
      return (t+1);
   }
}
