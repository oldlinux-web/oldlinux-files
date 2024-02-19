#ifndef LINT
/* derived from: misc.c 2.6 88/08/15 16:17:23 */
static char sccsid[]="$Source: /usr/home/dhesi/zoo/RCS/misc.c,v $\n\
$Id: misc.c,v 1.8 91/07/09 01:54:08 dhesi Exp $";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/
#include "options.h"
/* Miscellaneous functions needed by Zoo but not by Ooz */

#include "zoo.h"
#include "zooio.h"
#include "various.h"

#include "errors.i"
#include "zoofns.h"
#ifndef NOSIGNAL
#include <signal.h>
#endif

#ifdef NEEDCTYP
#include <ctype.h>
#else
#include "portable.h"
#endif

int ver_too_high PARMS((struct zoo_header *));

/*
calc_ofs() is given a string that (supposedly) begins with a string
of digits.  It returns a corresponding numeric value.  If no such
string, it aborts the program with a fatal error message.
*/
long calc_ofs(str)
char *str;
{
   long retval;
   char *p;
   retval = 0L;
   p = str; /* save for error message */
   while (isdigit(*str)) {
      retval = retval * 10L + (*str-'0');
      str++;
   }
   if (*str != '\0')
      prterror ('f', "Invalid number %s\n", p);
   return (retval);
}

/*
choosefname() decides which filename to use.  If a long filename is present,
and if the syntax is that of UNIX, MS-DOS or the portable form, we use it;
else we use the short filename. 
*/

char *choosefname(direntry)
struct direntry *direntry;
{
   char *retptr;                 /* pointer to name that we will return */
   switch (direntry->system_id) {
      case SYSID_NIX:
      case SYSID_PORTABLE:
      case SYSID_MS:
         retptr = (direntry->namlen != 0) ? direntry->lfname : direntry->fname;
         break;
      default:
         retptr = direntry->fname;
         break;
   }
   return (retptr);
} /* choosefname() */

/* 
combine() combines a directory name and a filename, making sure the
two are separated by a path separator 
*/
char *combine(result, dirname, fname)
char result[], *dirname, *fname;
{
   *result = '\0';
   if (*dirname != '\0') {
#ifdef DIR_LBRACK /* hack for VMS */
      strcat (result, DIR_LBRACK);

      /*  "/" => "[",  "./" => "[."  others => "[." */

      if (dirname[0] == '/') {        /* absolute path => "[" */
         strcat (result, dirname + 1);
      } else if (dirname[0] == '.' && dirname[1] == '/') {
         strcat (result, CUR_DIR);
         strcat (result, dirname + 2);
      } else {
         strcat (result, CUR_DIR);
         strcat (result, dirname);
      }

/* folowing #ifdef block ought to be outside #ifdef DIR_LBRACK, and
   for loop should then start with p=result.  This is currently
   just a hack for VMS.
*/
#ifdef DIR_SEP
   if (DIR_SEP != '/') {   /* if char separating dirs is not "/",  */
      char *p;
      for (p = result+2;  *p != '\0';  p++) /* change it to underscore */
        if (*p == DIR_SEP)
           *p = '_';
   }
#endif

      {
         char *p;
         for (p = result; *p != '\0';  p++)
         if (*p == '/')
            *p = '.';
      }
#else
      strcat (result, dirname);
#endif
      if (*lastptr(result) != *PATH_CH)
         strcat(result, PATH_CH);
   }

   strcat(result, fname);
   return (result);
}

/*
fullpath() accepts a pointer to a directory entry and returns the
combined directory name + filename.  The long filename is used
if available, else the short filename is used.
*/
char *fullpath (direntry)
struct direntry *direntry;
{
	static char result[PATHSIZE];
	combine (result,
				direntry->dirlen != 0 ? direntry->dirname : "", 
				(direntry->namlen != 0) ? direntry->lfname : direntry->fname
			  );
	return (result);
}

/* 
ver_too_high returns true if version of provided archive header is
too high for us to manipulate archive
*/

int ver_too_high (header)
struct zoo_header *header;
{
   return (header->major_ver > MAJOR_VER ||
            (header->major_ver == MAJOR_VER &&
             header->minor_ver > MINOR_VER));
}

/* 
rwheader() reads archive header, checks consistency, makes sure its
version number is not too high, updates it if too low, and seeks to
beginning of first directory entr.  If `preserve' is 1, it preserves
the header type;  if `preserve' is 0, it gives a fatal error message
if type is 0.
*/

void rwheader (header, zoo_file, preserve)
register struct zoo_header *header;
ZOOFILE zoo_file;
int preserve;
{

   frd_zooh (header, zoo_file);

   if ((header->zoo_start + header->zoo_minus) != 0L)
      prterror ('f', failed_consistency);
   if (ver_too_high (header))
      prterror ('f', wrong_version, header->major_ver, header->minor_ver);

	if (preserve == 0 && header->type == 0)
		prterror ('f', packfirst);

   /* We reach here if the archive version is not too high.  Now, if it
   isn't the same as ours, we bring it up to ours so the modified archive
   will be safe from previous versions of Zoo */

   if (header->major_ver != MAJOR_VER || header->minor_ver != MINOR_VER) {
      header->major_ver = MAJOR_VER;
      header->minor_ver = MINOR_VER;
      zooseek (zoo_file, 0L, 0);            /* seek to beginning */
      fwr_zooh (header, zoo_file);
   }
   zooseek (zoo_file, header->zoo_start, 0); /* seek to where data begins */
} /* rwheader */

/*
writedir() write a directory entry with keyboard interrupt disabled
*/
void writedir (direntry, zoo_file)
struct direntry *direntry;
ZOOFILE zoo_file;
{
#ifndef NOSIGNAL  
   T_SIGNAL (*oldsignal)();
   oldsignal = signal (SIGINT, SIG_IGN);
#endif
   if (fwr_dir (direntry, zoo_file) == -1)
      prterror ('f', disk_full);
#ifndef NOSIGNAL
   signal (SIGINT, oldsignal);
#endif
}

/* 
readdir() reads a directory entry from an archive.  If the directory
entry is invalid and if fail is 1, it causes a fatal error;
else it returns.  Return value is 0 if no error else -1;
*/

int readdir (direntry, zoo_file, fail)    /* read directory entry */
register struct direntry *direntry;
ZOOFILE zoo_file;
int fail;                              /* 0 -> return, 1 -> abort on error */
{
   if (frd_dir (direntry, zoo_file) < 0) {
      if (fail) {
         prterror ('f', bad_directory);
      } else
         return (-1);
   }
   if (direntry->zoo_tag != ZOO_TAG) {
      if (fail)
         prterror ('f', bad_directory);
      else
         return (-1);
   }
   return (0);
}

/* use pointer version below */
#ifdef COMMENT
/* instr() searches a string for a substring */
instr (s, t)      /* return index of string t in string s, -1 if none */
char s[], t[];    /*  .. from K&R page 67 */
{
   int i;
   register int j, k;
   for (i = 0; s[i] != '\0'; i++) {
      for (j = i, k = 0; t[k] != '\0' && s[j]==t[k]; j++, k++)
         ;
      if (t[k] == '\0')
         return (i);
   }
   return (-1);
}
#endif /* COMMENT */

/* instr() searches a string for a substring */
/* from J. Brian Waters */
int instr (s, t)   		/* return the position of t in s, -1 if none */
char *s, *t;				/*  a pointer version of K&R index function p.67 */
{               /* renamed to instr() to avoid conflicts with C RTL - JBW */

   register char *i, *j, *k;

   for (i = s; *i; i++) {
      for (j = i, k = t; (*k) && (*j++ == *k); k++)
         ;
       if (!*k)
         return ((int) (i - s));
   }
   return(-1);
}

/* cfactor() calculates the compression factor given a directory entry */
int cfactor (org_size, size_now)
long org_size, size_now;
{
   register int size_factor;
   while ((unsigned long) org_size > 32000) { /* avoid later overflow */
      org_size = (unsigned long) org_size / 1024;
      size_now = (unsigned long) size_now / 1024;
   }
   if (org_size == 0)         /* avoid division by zero */
      size_factor = 0;
   else {
      size_factor = (int)
         (
            (1000 * 
               ((unsigned long) org_size - (unsigned long) size_now)
            ) / org_size + 5
         ) / 10;
   }
   return (size_factor);
}

/***********
str_dup() duplicates a string using dynamic memory.
*/

char *str_dup (str)
register char *str;
{
   return (strcpy (emalloc (strlen(str)+1), str));
}

/**************
cmpnum() compares two pairs of unsigned integers and returns a negative,
zero, or positive value as the comparison yields less than, equal, or
greater than result.  Each pair of unsigned integers is considered to be the
more significant and the less significant half of a longer unsigned number.

Note:  cmpnum is used to compare dates and times.
*/

int cmpnum (hi1, lo1, hi2, lo2)
register unsigned int hi1, hi2;
unsigned int lo1, lo2;
{
   if (hi1 != hi2)
      return (hi1 > hi2 ? 1 : -1);
   else {
		if (lo1 == lo2)
			return (0);
		else
			return (lo1 > lo2 ? 1 : -1);
	}
}

/*******************/
/* writenull() */
/* writes a null directory entry to an open archive */
void writenull (file, length)
ZOOFILE file;
int length;
{
#ifndef NOSIGNAL
   T_SIGNAL (*oldsignal)();
#endif
   struct direntry newentry;
   memset ((char *) &newentry, 0, sizeof (newentry));
   newentry.zoo_tag = ZOO_TAG;
   newentry.type = 2;
   /* Force entry to be the required length plus possibly 2 stray bytes
   by dividing up the needed padding into dirlen and namlen. */
   if (length > SIZ_DIRL)
      newentry.dirlen = newentry.namlen = (length-SIZ_DIRL)/2 + 2;
   else
      newentry.dirlen = newentry.namlen = 0;
#ifndef NOSIGNAL
   oldsignal = signal (SIGINT, SIG_IGN);
#endif
   if (fwr_dir (&newentry, file) == -1)
      prterror ('f', disk_full);
#ifndef NOSIGNAL
   signal (SIGINT, oldsignal);
#endif
}

#ifdef FORCESLASH
/*******************/
/*
fixslash() changes all "\" characters in the supplied string to "/".
*/

void fixslash (str)
char *str;
{
   register char *p;
   for (p = str; *p != '\0'; p++)
      if (*p == '\\')
         *p = '/';
}
#endif /* FORCESLASH */
