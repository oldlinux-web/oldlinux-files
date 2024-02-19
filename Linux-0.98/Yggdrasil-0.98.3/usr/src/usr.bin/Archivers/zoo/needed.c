#ifndef LINT
static char sccsid[]="@(#) needed.c 2.16 88/01/31 15:54:37";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/

#define	STRCMP(s1,op,s2)		(strcmp(s1,s2) op 0)

#include "options.h"
/* Accepts a filename from an archive and returns 1 if a command-line
   argument filename matches it.  Otherwise returns 0. Returns
   1 if no arguments were supplied (so by default, all files will
   be extracted */

#include "zoo.h"

#ifdef NEEDCTYP
#include <ctype.h>              /* for tolower() */
#else
#include "portable.h"
#endif

#include "zooio.h"
#include "zoofns.h"
#include "various.h"
#include "debug.h"

extern int next_arg;          /* filenames start at this position */
extern int arg_count;         /* count of arguments supplied to program */
extern char **arg_vector;     /* vector of arguments supplied to program */
/* Uses FIRST_ARG in zoo.h, so must be recompiled when switching
   between Ooz and Zoo */

int needed(pathname, direntry, header)
char *pathname;
struct direntry *direntry;
struct zoo_header *header;
{
   register int i;
   register char *arg;
   char *justname;
	char arg_copy[PATHSIZE];				/* working copy of an argument */
	char path_copy[PATHSIZE];				/* working copy of pathname */
	char *p;										/* a temporary pointer */
	char *q;										/* a temporary pointer */

	/* if no filenames supplied, match latest version of each 
		(but match any version if versions not enabled) */
   if (arg_count <= FIRST_ARG &&
			(!(header->vdata & VFL_ON) || 
			!(direntry->vflag & VFL_ON) || (direntry->vflag & VFL_LAST))) {
		 
      return (1);                   	/* .. then all files are needed */
	}

   /* count backwards and stop if '+' is encountered */
   for (i = arg_count-1;  i >= next_arg; i--) {
      arg = arg_vector[i];
#ifdef FOLD
      str_lwr(pathname); str_lwr(arg);
#endif
#ifdef DEBUG
printf("needed: testing [%s] and [%s]\n", pathname, arg);
#endif
      if (STRCMP(arg,==,"+"))
         return (0);

      /* If the argument contains a slash, the match fails if the
         path prefixes don't match */
      if (strchr(arg, *(char *)PATH_CH) != NULL) {      /* found slash */
         strcpy(arg_copy,arg);
         strcpy(path_copy,pathname);
         p = findlast(arg_copy, PATH_CH);
         if (p != NULL)
            *p = '\0';
			else {
				p = findlast (arg_copy, VER_INPUT);
				if (p != NULL)
					*p = '\0';
			}
         p = findlast(path_copy, PATH_CH);
         if (p != NULL)
            *p = '\0';
			else {
				p = findlast (path_copy, VER_DISPLAY);
				if (p != NULL)
					*p = '\0';
			}
         if (!match_half(path_copy, arg_copy)) {
#ifdef DEBUG
printf ("needed(): match failed for [%s] and [%s]\n",
	path_copy, arg_copy);
#endif
            continue;                     /* no match this time in loop */
         }
      }

      /*
      We reach here either if the pattern had no slashes, or if it had a 
		slash but the path prefixes matched.  Now we will test to see if the 
		filename parts match.  If the argument contains VER_INPUT character, 
		then this separates the filename from a version number, and only that 
		specific version will match.  If not, then only the latest version 
		will match.  However, if the argument has a version character but
		nothing following it, that matches all versions.  Also, version
		0 matches only the latest version and version ^0 matches all
		versions except the latest one.
      */
		strcpy (arg_copy, arg);							/* local copy of argument */
		strcpy (path_copy, pathname);					/* local copy of pathname */
		p = findlast(arg_copy, VER_INPUT);			/* p is version in argument */
		q = findlast(path_copy, VER_DISPLAY);		/* q is version in archive */
		if (p != NULL && p != lastptr(arg_copy)) {/* nonnull version in arg */
			if (q != NULL) {								/* nonnull ver. in archive */
				char *pp = p+1;							/* point to actual version */
				char *qq = q+1;
				if (STRCMP(pp, ==, "0") && !(direntry->vflag & VFL_LAST) ||
				 	STRCMP(pp, ==, "^0") && (direntry->vflag & VFL_LAST)) {
						debug(("needed:  no match versions [%s] and [%s]\n", qq, pp))
						continue;
				}
				if (STRCMP(pp, !=, "0") && STRCMP(pp, !=, "^0") &&
					!match_half (qq, pp)) {
					debug(("needed:  no match versions [%s] and [%s]\n", qq, pp))
					continue;									/* no match this loop */
				}
			}
		}
		/* Special case test:  If argument has version but no filename,
			then filename is assumed to match */
		if (p == arg_copy) {								/* 1st char is version char */
			return (1);										/* .. so declare a match */
		}

		/* 
		Reach here if argument has no version character, or if argument has 
		version character and it matches version in pathname.  Now we check to 
		see if argument has no version character and if pathname is latest 
		version.  If so, the versions do match;  if not, then the match fails.  
		But if version numbering is not enabled, then versions always match.
		If the match fails, we do a "continue", else we fall through and
		proceed to test the filenames.  (Note:  It is intuitively better
		to first compare filenames and then see if versions match, but
		since they are both just independent fields, it's equally correct
		to compare versions first, as we are doing here, and then see if
		filenames match.  It may even be more efficient.)
		*/

		if (p == NULL && 										/* no version char typed */
				!(													/* NOT */
					(direntry->vflag & VFL_ON) == 0 || 	/*  no versions */
					(direntry->vflag & VFL_LAST) ||		/*  .. or latest version */
					q == NULL									/*  .. or no version char */
				)
			)
		{
#ifdef DEBUG
printf("needed: fail--no version typed and not latest version\n");
#endif
			continue;											/* match fails */
		} 
		/* versions match and we fall through */;

		/* reach here if versions match -- so strip them and compare rest */
		if (p != NULL)
			*p = '\0';							/* strips version from arg_copy */
		if (q != NULL)
			*q = '\0';							/* strips version from path_copy */

		justname = nameptr(path_copy);		/* filename without any pathname */
      if (match_half (justname, nameptr(arg_copy)))
         return (1);
#ifdef DEBUG
printf ("needed: fname-only match failed [%s] and [%s]\n",
 justname, nameptr(arg_copy));
#endif

      /* try for a character range */
      if (match_half (arg, "?-?")) { 			/* character range given */
         if (arg[0] <= *justname && arg[2] >= *justname)
            return (1);
      }
   }
   return (0);

} /* needed */

/***********************/
/*
match_half() compares a pattern with a string.  Wildcards accepted in
the pattern are:  "*" for zero or more arbitrary characters;  "?"
for any one characters.  Unlike the MS-DOS wildcard match, "*" is
correctly handled even if it isn't at the end of the pattern. ".'
is not special.

Originally written by Jeff Damens of Columbia University Center for
Computing Activities.  Taken from the source code for C-Kermit version
4C.
*/

int match_half (string, pattern) 
register char *string, *pattern;
{
   char *psave,*ssave;        /* back up pointers for failure */
   psave = ssave = NULL;
   while (1) {
#ifdef IGNORECASE
      for (; 
         tolower(*pattern) == tolower(*string); 
         pattern++,string++                        )  /* skip first */
#else
      for (; *pattern == *string; pattern++,string++)  /* skip first */
#endif /* IGNORECASE */

         if (*string == '\0') 
            return(1);                          /* end of strings, succeed */
      if (*string != '\0' && *pattern == '?') {
         pattern++;                             /* '?', let it match */
         string++;
      } else if (*pattern == '*') {             /* '*' ... */
         psave = ++pattern;                     /* remember where we saw it */
         ssave = string;                        /* let it match 0 chars */
      } else if (ssave != NULL && *ssave != '\0') {   /* if not at end  */
         /* ...have seen a star */
         string = ++ssave;                      /* skip 1 char from string */
         pattern = psave;                       /* and back up pattern */
      } else 
         return(0);                             /* otherwise just fail */
   }
}

