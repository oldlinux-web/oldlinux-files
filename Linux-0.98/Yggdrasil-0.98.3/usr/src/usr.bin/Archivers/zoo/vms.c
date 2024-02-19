#ifndef LINT
/* derived from: @(#) vms.c 2.2 88/01/09 03:47:52 */
static char vmsid[]="$Source: /usr/home/dhesi/zoo/RCS/vms.c,v $\n\
$Id: vms.c,v 1.10 91/07/07 14:41:17 dhesi Exp $";
#endif /* LINT */

/* Support routines for VAX/VMS. */

#include <stat.h>
#include <time.h>

/* our own version of NULL; avoid any interaction with system defn.
but don't require inclusion of stdio.h */
#define  NILPTR	0

/* Function isuadir() returns 1 if the supplied filename is a directory, 
else it returns 0.  
*/

int isuadir (file)
char *file;
{
   struct stat buf;           /* buffer to hold file information */
   if (stat (file, &buf) == -1) {
      return (0);             /* inaccessible -- assume not dir */
   } else {
      if (buf.st_mode & S_IFDIR)
         return (1);
      else
         return (0);
   }
}

/****************
Function fixfname() converts the supplied filename to a syntax
legal for the host system.  It is used during extraction.  We
allow a maximum of one dot in the filename, and it must not
be at the beginning.  We also truncate the number of charac-
ters preceding and following the dot to at most 39.
*/

char *strchr();

char *fixfname(fname)
char *fname;
{
   char *p;
   char *dotpos;
   static char legal[] = 
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$_.0123456789";

   /* convert all characters to legal characters */
   for (p = fname;  *p != '\0';  p++)
      if (strchr (legal, *p) == 0) {
         if (*p == '-' || *p == ' ')
            *p = '_';
         else
            *p = legal [ *p % 26 ];
      }

   /* first char can't be dot */
   if (*fname == '.')
      *fname = 'X';

   /* find embedded dot if any */
   dotpos = strchr (fname, '.');

   if (dotpos != NILPTR) {
      for (p = dotpos+1;  *p != '\0';  p++)  /* remove 2nd dot onwards */
         if (*p == '.')
            *p = '_';
      if (dotpos - fname + 1 > 39) {  /* more than 39 before dot not allowed */
         char *q;
         p = fname + 39;
         q = dotpos;
         while (*p++ = *q++)  /* the notational convenience is considerable */
            ;
         dotpos = strchr (fname, '.');
      }
      if (strlen (dotpos + 1) > 39)  /* more than 39 after dot not allowed */
         *(dotpos + 39 + 1) = '\0';
   } else
      *(fname + 39) = '\0'; /* no dots, just truncate to 39 characters */
   return (fname);
}

/*
Function gettz(), returns the offset from GMT in seconds of the
local time, taking into account daylight savings time -- or it
would, if VAX/VMS knew about timezones!  It's just a no-op.
*/
long gettz()
{
   return 0L;
}

struct tm *localtime();

/*****************
Function gettime() gets the date and time of the file handle supplied.
Date and time is in MSDOS format.  This function duplicated from nixtime.i.
*/

int gettime (file, date, time)
ZOOFILE file;
unsigned *date, *time;
{
   struct stat buf;           /* buffer to hold file information */
   struct tm *tm;             /* will hold year/month/day etc. */
	int handle;
	handle = fileno(file);
   if (fstat (handle, &buf) == -1) {
      prterror ('w', "Could not get file time\n");
      *date = *time = 0;
   } else {
      tm = localtime (&buf.st_mtime); /* get info about file mod time */
      *date = tm->tm_mday + ((tm->tm_mon + 1) << 5) +
         ((tm->tm_year - 80) << 9);
      *time = tm->tm_sec / 2 + (tm->tm_min << 5) +
         (tm->tm_hour << 11);
   }

}

/*
Function unlink() will delete a file using the VMS C delete()
function.
*/
int unlink (fname)
char *fname;
{
   return (delete (fname));
}

/*
Function zooexit() receives attempts at exit.  It always invokes
exit() with status=1.
*/
void zooexit (status)
int status;
{
   exit (1);
}


/*
Function rename() renames a file.  
Thanks to Owen Anthony (was <anthony@bsu-cs.bsu.edu> at one time).
*/

#include <descrip.h>

#ifdef NEED_VMS_RENAME /* if not using VMS 4.6 library rename() */
int rename (old_name, new_name)
char *old_name;
char *new_name;
{
   int status;
   struct dsc$descriptor_s file1, file2;
   file1.dsc$w_length = strlen (old_name);  /* descriptor for old name */
   file1.dsc$a_pointer = old_name;
   file1.dsc$b_class = DSC$K_CLASS_S;
   file1.dsc$b_dtype = DSC$K_DTYPE_T;
   file2.dsc$w_length = strlen (new_name);  /* descriptor for new name */
   file2.dsc$a_pointer = new_name;
   file2.dsc$b_class = DSC$K_CLASS_S;
   file2.dsc$b_dtype = DSC$K_DTYPE_T;

   status = LIB$RENAME_FILE (&file1, &file2);

   return ((status & ~1) == 1);
}
#endif /* VMS_RENAME */

/*
Function specfname() modifies filenames before they are stored
in an archive.  Currently we remove any trailing version field,
and then any trailing dot.
*/
char *specfname (fname)
char *fname;
{
   char *p;
   p = strchr (fname, ';');
   if (p != NILPTR)
      *p = '\0';
   if (*fname != '\0') {
      p = fname + strlen (fname) - 1; /* point to last char */
      if (*p == '.')                  /* remove any trailing dot */
         *p = '\0';
   }
   return (fname);
}

/* 
Function specdir() modifies directory names before they are stored
in an archive.  We remove any leading device name or logical
name and and the [ and ] that bracket any directory name.
Then we change any dots in the directory name to slashes.
*/

#if 0
/* test stub that just truncates dir to null string */
char *specdir (fname) char *fname;
{ *fname = '\0'; return (fname); }
#else

char *specdir (fname)
char *fname;
{
   char *p;
   char tmpstr[LFNAMESIZE];

   p = strchr (fname, ':');      /* remove chars upto and including : */
   if (p != NILPTR) {
      strcpy (tmpstr, p+1);
      strcpy (fname, tmpstr);
   }

   p = strchr (fname, '[');      /* remove chars upto and including [ */
   if (p != NILPTR) {
      strcpy (tmpstr, p+1);
      strcpy (fname, tmpstr);
   }

   p = strchr (fname, ']');      /* truncate at ] */
   if (p != NILPTR) {
      if (*(p+1) != '\0')
         prterror ('w', "Trailing garbage in directory name\n");
      *p = '\0';
   }

   for (p = fname;  *p != '\0';  p++)   /* change dots to slashes */
      if (*p == '.')
         *p = '/';

   /* make sure there is a leading slash -- just a hack for now */
   if (*fname != '/') {
      strcpy (tmpstr, fname);
      strcpy (fname, "/");
      strcat (fname, tmpstr);
   }

#ifdef DEBUG
printf ("dir name transformed to \"%s\"\n", fname);
#endif
}
#endif

#define  FMAX  3        /* Number of different filename patterns */

char *nextfile (what, filespec, fileset)
int what;                        /* whether to initialize or match      */
register char *filespec;         /* filespec to match if initializing   */
register int fileset;            /* which set of files                  */
{
   int status;
   char *p;                      /* temp ptr */
   struct dsc$descriptor_s d_fwild, d_ffound;
   static int first_time [FMAX+1];
   static char saved_fspec [FMAX+1][PATHSIZE];  /* our own copy of filespec */
   static char found_fspec [FMAX+1][PATHSIZE];  /* matched filename */
   static unsigned long context [FMAX+1];	/* needed by VMS */
   if (what == 0) {
      strcpy (saved_fspec[fileset], filespec);  /* save the filespec */
      first_time[fileset] = 1;
      return (0);
   }

   /* Reach here if what is not 0, so it must be 1 */

   /* Create a descriptor for the wildcarded filespec */
   d_fwild.dsc$w_length = strlen (saved_fspec[fileset]);
   d_fwild.dsc$a_pointer = saved_fspec[fileset];
   d_fwild.dsc$b_class = DSC$K_CLASS_S;
   d_fwild.dsc$b_dtype = DSC$K_DTYPE_T;

   d_ffound.dsc$w_length = sizeof (found_fspec[fileset]);
   d_ffound.dsc$a_pointer = found_fspec[fileset];
   d_ffound.dsc$b_class = DSC$K_CLASS_S;
   d_ffound.dsc$b_dtype = DSC$K_DTYPE_T;

   if (first_time[fileset]) {
      first_time[fileset] = 0;
      context[fileset] = 0L;   /* tell VMS this is first search */
   }
   status = LIB$FIND_FILE (&d_fwild, &d_ffound, &context[fileset]);
   status = status & 1;	/* use only lowest bit */

   if (status == 0) {
      LIB$FIND_FILE_END (&context[fileset]);
      return ((char *) 0);
   } else {
      found_fspec[fileset][d_ffound.dsc$w_length] = '\0'; /* just in case */
      p = found_fspec[fileset];
      while (*p != ' ' && *p != '\0')
         p++;
      if (*p != '\0')
         *p = '\0';
      return (found_fspec[fileset]);
   }
}

/*
Function vmsmkdir() converts the received directory name into VMS
format and then creates a directory.
*/
int vmsmkdir (subdir)
char *subdir;
{
   char *lastptr();
   char *p;
   char tmp[LFNAMESIZE];

   p = subdir;

   /* leading "/" => "[", otherwise => "[." */
   if (*p == '/') {
      strcpy (tmp, "[");
      p++;
   } else {
      strcpy (tmp, "[.");
      while (*p == '/' || *p == '.')
         p++;
   }

   strcat (tmp, p);

   /* 
   VMS doesn't like dots in directory names, so we convert them to
   underscores.  Leave first two characters untouched, because
   we don't want to corrupt a leading "[." into "[_".
   */
   for (p = tmp + 2;  *p != '\0';  p++)
      if (*p == '.')
         *p = '_';

   /* convert all slashes to dots */
   for (p = tmp; *p != '\0';  p++)
      if (*p == '/')
         *p = '.';

   /* Remove any trailing dot */
   p = lastptr (tmp);
   if (*p == '.')
      *p = '\0';

   /* append closing bracket */
   strcat (tmp, "]");
#if 0
   printf ("\nmaking directory \"%s\"\n", tmp);
#endif
   return (mkdir (tmp, 0));
}

/*
Function spec_wild() transforms a pattern supplied on the command line into one
suitable for wildcard expansion in the most efficient way possible.  We change
each "?" to "%" but let "*" remain unchanged.  We also append a ".*" if the
pattern contains no dot, so "*" will be interpreted as "*.*" for VMS globbing. 
*/
char *spec_wild (arg)
char *arg;
{
   char *p;
#ifdef DEBUG
   printf ("spec_wild: arg = [%s]\n", arg);
#endif
   if (*lastptr (arg) == ']')      /* add *.* if no filename */
      strcat (arg, "*.*");
   p = nameptr (arg);              /* point p to filename part */

   /* if no dot in name append ".*" */
   if (strchr (p, '.') == NILPTR)
      strcat (p, ".*");

   for ( ; *p != '\0';  p++)        /* change every "?" to "%" */
      if (*p == '?')
         *p = '%';
#ifdef DEBUG
   printf ("spec_wild: arg changed to [%s]\n", arg);
#endif
   return (arg);
}

int zootrunc(f) FILE *f; { return 0; }
