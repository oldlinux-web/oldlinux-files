#ifndef LINT
static char sccsid[]="@(#) parse.c 2.1 87/12/25 12:24:10";
#endif /* LINT */

/*
The contents of this file are hereby released to the public domain.

                                 -- Rahul Dhesi 1986/11/14

*/

#include "options.h"
#include "zoo.h"
#include "zooio.h"
#include "various.h"
#include "zoofns.h"

#include "parse.h"
#include "assert.h"

/*
parse() accepts a filename and return its component parts in a structure.
The component parts are:  disk drive, path prefix, root name of filename, 
and extension.  

If DISK_CH is not defined, it is assumed that filenames may be
preceded with a disk prefix terminated by the character DISK_CH.
The first character of the disk prefix, followed by DISK_CH,
is returned in the drive field.

If the symbol DISK_CH is defined, a null string is returned in the
disk field.
*/
void parse (path_st, fname)
register struct path_st *path_st;
char *fname;
{
   char tempname[LFNAMESIZE];       /* working copy of supplied fname */
   char *namep;                   /* points to relevant part of tempname */

   char *p;
   strcpy (tempname, fname);

#ifdef DEBUG
printf ("parse:  supplied name is [%s].\n", tempname);
#endif

#ifndef DISK_CH
   path_st->drive[0] = '\0';
   namep = tempname;           /* points to pathname+filename */
#else
   path_st->drive[0] = '\0';
   p = strchr (tempname, DISK_CH);      /* point to first ':' */

   if (p != NULL) {
      path_st->drive[0] = *tempname;/* use only first char of drive name */
      path_st->drive[1] = DISK_CH;
      path_st->drive[2] = '\0';
      namep = ++p;                /* point to pathname+filename */
   } else {
      path_st->drive[0] = '\0';
      namep = tempname;           /* points to pathname+filename */
   }
#endif /* end of not DISK_CH */
   
   /* Note:  findlast() finds last occurrence in the subject string of 
      any one of a set of chars */

   /* save the long filename */
   p = findlast (namep, PATH_SEP);

   /* if path separator found, copy next char onwards; else entire string */
   strncpy (path_st->lfname,
               (p != NULL) ? p+1 : namep,
               LFNAMESIZE);
   path_st->lfname[LFNAMESIZE-1] = '\0';     /* force null termination */

#ifdef DEBUG
printf ("parse:  path = [%s] long filename = [%s]\n", 
         namep, path_st->lfname);
#endif

/* Separate out the extension */
p = findlast (namep, EXT_SEP);						/* look for . or /		*/
if (p != NULL && *p != EXT_CH)						/* found .?					*/
	p = NULL;												/* ... if not, ignore / */

#ifdef DEBUG
if (p == NULL)
   printf ("parse:  no extension found for [%s]\n", namep);
else
   printf ("parse:  extension for [%s] is [%s]\n", namep, p);
#endif
   
   path_st->ext[0] = '\0';                      /* assume no extension  */
   if (p != NULL) {                             /* found extension      */
      strncpy (path_st->ext, (p+1), EXTLEN);    /* save extension       */
      path_st->ext[EXTLEN] = '\0';              /* force termination    */
      *p = '\0';                                /* null out extension   */
   }

   /* separate out root of filename if any */
   p = findlast (namep, PATH_SEP);

   if (p != NULL) {
      ++p;
      strncpy (path_st->fname, p, ROOTSIZE);  /* save filename        */
      *p = '\0';               /* null out filename */
   } else {
      strncpy (path_st->fname, namep, ROOTSIZE);
      *namep = '\0';                   /* null out filename    */
   }
   path_st->fname[ROOTSIZE] = '\0';           /* force termination    */

   /* what remains, whether null or not, is the path prefix */
   path_st->dir[0] = '\0';             /* in case *namep is '\0' */

   strncpy (path_st->dir, namep, PATHSIZE);

   /* remove trailing path-separater from directory name, but don't
      remove it if it is also the leading separater */
   { 
      int n;
      n = strlen(path_st->dir);
      if (n != 1)
         path_st->dir[n-1] = '\0';
   }

#ifdef DEBUG
printf ("parse:  path prefix = [%s].\n", namep);
#endif
   /* if extension is null, and if long filename contains more than
      ROOTSIZE  characters, transfer some of them to extension */
   if (path_st->ext[0] == '\0' && strlen(path_st->lfname) > ROOTSIZE) {
      strncpy(path_st->ext, &path_st->lfname[ROOTSIZE], EXTLEN);
      path_st->ext[3] = '\0';
   }
}

/*******************/
/* 
findlast() finds last occurrence in provided string of any of the characters
except the null character in the provided set.

If found, return value is pointer to character found, else it is NULL.
*/

char *findlast (str, set)
register char *str;        /* subject string     */
char *set;                 /* set of characters to look for */

{
   register char *p;

   if (str == NULL || set == NULL || *str == '\0' || *set == '\0')
      return (NULL);

   p = lastptr (str);   /* pointer to last char of string */
   assert(p != NULL);

   while (p != str && strchr (set, *p) == NULL) {
      --p;
   }                 

   /* either p == str or we found a character or both */
   if (strchr (set, *p) == NULL)
      return (NULL);
   else
      return (p);
}

/*******************/
/*
lastptr() returns a pointer to the last non-null character in the string, if
any.  If the string is null it returns NULL
*/

char *lastptr (str)
register char *str;                 /* string in which to find last char */
{
   register char *p;
   if (str == NULL)
      prterror ('f', "lastptr:  received null pointer\n");
   if (*str == '\0')
      return (NULL);
   p = str;
   while (*p != '\0')            /* find trailing null char */
      ++p;
   --p;                          /* point to just before it */
   return (p);
}
