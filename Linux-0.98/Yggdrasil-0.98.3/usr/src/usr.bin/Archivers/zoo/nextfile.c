#ifndef LINT
static char sccsid[]="@(#) nextfile.c 2.2 87/12/26 12:23:43";
#endif /* LINT */

#include "options.h"
/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
*/
/*
Functions to collect filenames from command line etc.  Nextfile() is
used by both Atoz and Zoo.  Wildcard expansion by nextfile() is specific to 
MS-DOS and this implementation is specific to Microsoft C.  If the symbol 
PORTABLE is defined, nextfile() becomes effectively a no-op that will return
the original filespec the first time and NULL subsequently.
*/

#define  FMAX  3        /* Number of different filename patterns */

#ifndef	OK_STDIO
#include <stdio.h>
#define	OK_STDIO
#endif
#include "various.h"
#include "zoo.h"        /* solely to define PATHSIZE */

#ifdef PORTABLE
#ifndef SPECNEXT
/* If portable version, nextfile() is effectively a no-op and any wildcard
expansion must be done by the runtime system before the command line
is received by this program
*/
char *nextfile (what, filespec, fileset)
int what;                        /* whether to initialize or match      */
register char *filespec;         /* filespec to match if initializing   */
register int fileset;            /* which set of files                  */
{
   static int first_time [FMAX+1];
   static char saved_fspec [FMAX+1][PATHSIZE];  /* our own copy of filespec */

   if (what == 0) {
      strcpy (saved_fspec[fileset], filespec);  /* save the filespec */
      first_time[fileset] = 1;
      return NULL;
   }

   if (first_time[fileset]) {
      first_time[fileset] = 0;
      return saved_fspec[fileset];
   } else {
      return NULL;
   }
}
#endif /* SPECNEXT */
#else
/* if not PORTABLE  then */

#include <dir.h>
#include <dos.h>
#include "assert.h"     /* macro definition:  assert() macro            */

void fcbpath PARMS((struct ffblk *, char *, char *));


/*******************/
/*
nextfile() returns the name of the next source file matching a filespec.

INPUT
   what: A flag specifying what to do.  If "what" is 0, nextfile() 
      initializes itself.  If "what" is 1, nextfile() returns the next 
      matching filename.  
   filespec:  The filespec, usually containing wildcard characters, that 
      specifies which files are needed.  If "what" is 0, filespec must be 
      the filespec for which matching filenames are needed.  If "what" is 1, 
      nextfile() does not use "filespec" and "filespec" should be NULL to 
      avoid an assertion error during debugging.
   fileset:  nextfile() can keep track of more than one set of filespecs.
      The fileset specifies which filespec is being matched and therefore
      which set of files is being considered.  "fileset" can be in the
      range 0:FMAX.  Initialization of one fileset does not affect the
      other filesets.

OUTPUT
   IF what == 0 THEN
      return value is NULL
   ELSE IF what == 1 THEN
      IF a matching filename is found THEN
         return value is pointer to matching filename including supplied path
      ELSE
         IF at least one file matched previously but no more match THEN
            return value is NULL
         ELSE IF supplied filespec never matched any filename THEN
            IF this is the first call with what == 1 THEN
               return value is pointer to original filespec
            ELSE
               return value is NULL
            END IF
         END IF
      END IF
   END IF

NOTE

   Initialization done when "what"=0 is not dependent on the correctness
   of the supplied filespec but simply initializes internal variables
   and makes a local copy of the supplied filespec.  If the supplied
   filespec was illegal, the only effect is that the first time that
   nextfile() is called with "what"=1, it will return the original 
   filespec instead of a matching filename.  That the filespec was
   illegal will become obvious when the caller attempts to open the
   returned filename for input/output and the open attempt fails.

USAGE HINTS

nextfile() can be used in the following manner:

      char *filespec;                  -- will point to filespec
      char *this_file;                 -- will point to matching filename
      filespec = parse_command_line(); -- may contain wildcards
      FILE *stream;
   
      nextfile (0, filespec, 0);          -- initialize fileset 0
      while ((this_file = nextfile(1, (char *) NULL, 0)) != NULL) {
         stream = fopen (this_file, "whatever");
         if (stream == NULL)
            printf ("could not open %s\n", this_file);
         else
            perform_operations (stream);
      }
*/             
               
char *nextfile (what, filespec, fileset)
int what;                        /* whether to initialize or match      */
register char *filespec;         /* filespec to match if initializing   */
register int fileset;            /* which set of files                  */
{
   static struct ffblk ffblk[FMAX+1];
   static int first_time [FMAX+1];
   static char pathholder [FMAX+1][PATHSIZE]; /* holds a pathname to return */
   static char saved_fspec [FMAX+1][PATHSIZE];/* our own copy of filespec   */
	int ffretval;	/* return value from findfirst() or findnext() */

   assert(fileset >= 0 && fileset <= FMAX);
   if (what == 0) {
      assert(filespec != NULL);
      strcpy (saved_fspec[fileset], filespec);  /* save the filespec */
      first_time[fileset] = 1;
      return (NULL);
   }

   assert(what == 1);
   assert(filespec == NULL);
   assert(first_time[fileset] == 0 || first_time[fileset] == 1);

   if (first_time[fileset])              /* first time -- initialize etc. */
		ffretval = findfirst(saved_fspec[fileset], &ffblk[fileset],  0);
   else
		ffretval = findnext(&ffblk[fileset]);

   if (ffretval != 0) {            /* if error status                  */
      if (first_time[fileset]) {       /*   if file never matched then     */
         first_time[fileset] = 0;
         return (saved_fspec[fileset]);/*      return original filespec    */
      } else {                         /*   else                           */
         first_time[fileset] = 0;      /*                                  */
         return (NULL);                /*      return (NULL) for no more   */
      }
   } else {                                        /* a file matched */
      first_time[fileset] = 0;         
      /* add path info  */
      fcbpath (&ffblk[fileset], saved_fspec[fileset], pathholder[fileset]); 
      return (pathholder[fileset]);                /* matching path  */
   }
} /* nextfile */

/*******************/
/* 
fcbpath() accepts a pointer to an ffblk structure, a character pointer 
to a pathname that may contain wildcards, and a character pointer to a
buffer.  Copies into buffer the path prefix from the pathname and the
filename prefix from the ffblk so that it forms a complete path.
*/

void fcbpath (ffblk, old_path, new_path)
struct ffblk *ffblk;
char *old_path;
register char *new_path;
{
   register int i;
   int length, start_pos;
      
   strcpy(new_path, old_path);               /* copy the whole thing first */
   length = strlen(new_path);
   i = length - 1;                           /* i points to end of path */
   while (i >= 0 && new_path[i] != '/' && new_path[i] != '\\' && new_path[i] != ':')
      i--;
   /* either we found a "/", "\", or ":", or we reached the beginning of
      the name.  In any case, i points to the last character of the
      path part. */
   start_pos = i + 1;
   for (i = 0; i < 13; i++)
      new_path[start_pos+i] = ffblk->ff_name[i];
   new_path[start_pos+13] = '\0';
}
#endif /* PORTABLE */
