#ifndef LINT
/* @(#) misc2.c 2.7 88/01/24 12:47:36 */
static char sccsid[]="@(#) misc2.c 2.7 88/01/24 12:47:36";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/
#include "options.h"
/* Miscellaneous routines */
#include "portable.h"
#include "zooio.h"
#include "various.h"
#include "errors.i"
#include "zoomem.h"
#include "zoo.h"
#include "zoofns.h"     /* only for malloc */

void makepath PARMS((char *));

/**********************/
/* memerr() */
/* Give error message on memory error and abort */
void memerr(i)
unsigned int i;
{
#ifdef OOZ
   prterror ('f', no_memory, "", "");
#else
	if (i != 0)
		prterror ('f', no_memory);
	else
		prterror('f', "needed %u bytes: %s", no_memory);
#endif
}

/**********************/
/*
emalloc() allocates memory like malloc() does (and it calls malloc).  However,
it automatically calls the error function memerr() if memory couldn't be
allocated.  It also assumes that memory will never be freed and conserves it
by allocating memory in large chunks and then partitioning it out with no
administrative overhead.  This avoids many problems due to various bugs in
various implementations of malloc, and also avoids a very high amount of
malloc overhead for repeated allocations to store numerous filenames etc.
The down side is that we can't use free().

WARNING:  No alignment of allocated memory is attempted, so memory
allocated through emalloc should be used only for storing strings.
For allocating memory for other data types use ealloc().  */

VOIDPTR emalloc (size)
unsigned int size;
{
#define  BLOCK_SIZE  512      /* memory allocation granularity */

#ifdef USE_MALLOC
/* Pass on memory requests to malloc() */
   char *ptr;
   if ((ptr = malloc (size)) == NULL)
      memerr(size);
   return (ptr);
#else
   static char *memptr;
   static unsigned avail = 0;
   unsigned malloc_incr;
   char *retval;

   if (size == 0)
      return (NULL);

   /* if not enough space avail get some more */
   if (avail < size) {
      malloc_incr = BLOCK_SIZE;
      if (malloc_incr < size)
         malloc_incr = size;
      while (malloc_incr >= size && (memptr = malloc (malloc_incr)) == NULL)
         malloc_incr = (malloc_incr / 6) * 5;
      avail = malloc_incr;
   }

   if (avail < size)
      memerr(size);                             /* no return from this */
   retval = memptr;
   memptr += size;
   avail -= size;
   return (retval);
#endif  /* end of not USE_MALLOC */
}

/**********************/
/*
ealloc() is just a wrapper around malloc(), which causes memerr() to be
called if memory cannot be allocated.  All memory requests are passed on
to malloc.   Allocated memory can later be freed.  */

VOIDPTR ealloc(size)
unsigned int size;
{
   char *ptr;
   if ((ptr = malloc (size)) == NULL)
      memerr(size);
   return ptr;
}

/**********************/
/* erealloc() is a wrapper around realloc() the way ealloc is a wrapper
around malloc().  It calls memerr() on error. */

VOIDPTR erealloc(p, size)
VOIDPTR p;
unsigned int size;
{
   char *ptr;
   if ((ptr = realloc (p, size)) == NULL)
      memerr(size);
   return ptr;
}

/**********************/
/* putstr()
This function prints a string to standard output.  If the received
string pointer is NULL, it is handled safely.  This function is here
for historical reasons:  Ooz was once coded to not use printf under
MSDOS to save space, and at that time putstr() printed a string
without using printf.  It should eventually be eliminated and all
calls to it replaced with calls to printf directly.
*/
void putstr (str)
register char *str;
{
   if (str == NULL)
      return;
	printf ("%s", str);
}

/**********************/
/* exists()
This function checks the existence of a file.  

If the symbol EXISTS is defined, that is called as a macro and
supplied the filename.  It must return 1 if the file exists and
0 if it does not.

If EXISTS is not defined, exists() tests to see if the file can be 
opened for reading or writing;  if so, it returns 1 else it returns 0. 

Because of the delay between the time existence is checked and the time Zoo
creates a files, a race condition exists.  It would be better to
use open() with the O_EXCL flag but that will not work for many
systems.
*/

int exists (fname)
char *fname;
{
#ifdef EXISTS
	return EXISTS(fname);
#else
   ZOOFILE f;

   if ( (f = zooopen (fname, Z_READ )) != NOFILE ||
		  (f = zooopen (fname, Z_WRITE)) != NOFILE ) {
      zooclose (f);
      return (1);
   } else
      return (0);
#endif /* ifdef EXISTS */
}

/****************
newcat() allocates enough space to concatenate two strings then returns
a pointer to the concatenated result */

char *newcat (r, s)
char *r, *s;
{
   char *temp = emalloc (strlen (r) + strlen (s) + 2); /* 1 spare */
   strcpy (temp, r);
   strcat (temp, s);
   return (temp);
}


/* Creates a path */
void makepath(path)
char *path;
{
   char tmppath[PATHSIZE];
   char *slashpos;
   if (path == NULL)
      return;
   while (*lastptr(path) == *(char *) PATH_CH)  /* remove trailing slashes */
      *lastptr(path) = '\0';
   if (*path == '\0')
      return;

   slashpos = findlast(path, PATH_CH);    /* find last slash */
   if (slashpos == NULL) {                /* if not, just create dir. */
      MKDIR(path);
      return;
   } else {                               /* otherwise...         */
      if (slashpos == path) {             /* if leading slash */
         MKDIR(slashpos);                 /* make that directory */
         return;                          /* and done */
      } else {
         strcpy(tmppath,path);            /* save path */
         *slashpos = '\0';                /* split into prefix & suffix */
#ifdef DEBUG
         printf("making path from [%s]\n", path);
#endif
         makepath(path);                     /* make path from prefix */
#ifdef DEBUG
         printf("making dir from [%s]\n", tmppath);
#endif
         MKDIR(tmppath);                  /* make dir from suffix */
      }
   }
} /* makepath() */

/*
If no extension in filename add supplied extension
*/
char *addext (fname, ext)
char *fname;
char *ext;
{
   if (strchr (nameptr (fname), EXT_CH) == NULL)
      return (newcat (fname, ext));
   else
      return (fname);
}

#ifdef VER_CH       /* remove any trailing extension field */
char *strip_ver (fname)
char *fname;
{
   char *p = strchr (fname, VER_CH);
   if (p != NULL)
      *p = '\0';
}
#endif

/*
Function samefile() compares two filenames to see if they are the
same file.  Just strcmp() or str_icmp() could have been used, except
that if the filenames have trailing version fields, we want to
compare those always equal.  samefile() is called by routines
that want to avoid adding an archive to itself.
*/
int samefile (f1, f2)
char *f1;
char *f2;
{
#ifdef IGNORECASE
#define COMPARE str_icmp
#else
#define COMPARE strcmp
#endif

#ifdef VER_CH
   char tf1[LFNAMESIZE];
   char tf2[LFNAMESIZE];
   strcpy (tf1, f1);
   strcpy (tf2, f2);
   strip_ver (tf1);   /* strip version fields */
   strip_ver (tf2);
   return (COMPARE (tf1, tf2) == 0);
#else
/* if no version fields, just use strcmp(i) */
   return (COMPARE (f1, f2) == 0);
#endif
}

#ifdef USE_ASCII
int isdigit (c)
int c;
{
	return (c >= '0' && c <= '9');
}
int isupper (c)
int c;
{
	return (c >= 'A' && c <= 'Z');
}

int toascii (c)
int c;
{
	return (c & 0x7f);
}

int tolower (c)
int c;
{
	return (isupper(c) ? (c | 0x20) : c);
}
#endif

#ifdef GETTZ
/****************
Function tzadj() accepts a directory entry and adjusts its timestamp
to reflect its timezone.  Uses function mstime() from mstime.i
and mstonix() from nixtime.i.
*/

long mstonix();
long gettz();
#include "mstime.i"	/* get mstime() */

void tzadj (direntry)
struct direntry *direntry;
{
	long diff_tz;
	long longtime;
	if (direntry->tz == NO_TZ)		/* none stored */
		return;
	diff_tz = (long) direntry->tz * (3600/4) - gettz(); /* diff. in seconds */
	longtime = mstonix (direntry->date, direntry->time) + diff_tz; /* adj tz */
	mstime (longtime, &direntry->date, &direntry->time);
}
#endif /* GETTZ */

/* how long an int can be in text form -- allow 64-bit ints */
#define INT_TEXT 21

/* Function add_version adds a version suffix to a filename, given
the directory entry corresponding to the file */
void add_version (fname, direntry)
char *fname;
struct direntry *direntry;
{
	char verstr[INT_TEXT];	/* string buffer for conversion to text */
	if (direntry->vflag & VFL_ON) {
		sprintf (verstr, "%u", direntry->version_no);
		strcat (fname, VER_DISPLAY);
		strcat (fname, verstr);
	}
}
