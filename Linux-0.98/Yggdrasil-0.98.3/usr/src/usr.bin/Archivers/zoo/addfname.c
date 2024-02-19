#ifndef LINT
static char sccsid[]="@(#) addfname.c 2.11 88/02/06 20:17:17";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
(C) Copyright 1988 Rahul Dhesi -- All rights reserved
*/
#include "options.h"

/* Adds a filename to global list.  (This global list will eventually
be searched by the inlist() function.)  The second and subsequent
parameters suppplied are stored with the name of the file and 
returned by inlist. */

#include "zooio.h"
#include "various.h"
#include "zoo.h"
#include "zoofns.h"
#include "zoomem.h" /* to get LIST_SIZE */

#define FENTRY_BSIZE		80		/* allocation granularity for fentry below */
static struct item **fentry;	/* array of ptrs to file information structs */
static unsigned sz_fentry;		/* its current size */
static int lastname = 0;                  /* index of last name */

struct item {              /* global filename list entry */
   char *fname;
   long position;
   unsigned int date;
   unsigned int time;
	unsigned vflag;
	unsigned version_no;
};

void addfname(fname, position, date, time, vflag, version_no)
char *fname;
long position;
unsigned int date, time;
unsigned vflag;
unsigned version_no;
{
   if (lastname == 0) {
		sz_fentry = FENTRY_BSIZE;
		fentry = (struct item **) ealloc(sizeof(struct item *) * sz_fentry);
      fentry[0] = (struct item *) ealloc (sizeof(struct item));
	}

	/* allocated more memory if needed */
   if (lastname >= sz_fentry - 3) {
		sz_fentry += FENTRY_BSIZE;
		fentry = (struct item **) 
			erealloc(fentry, sizeof(struct item *) * sz_fentry);
	}

   fentry[lastname]->fname = str_dup(fname);
   fentry[lastname]->position = position;
   fentry[lastname]->date = date;
   fentry[lastname]->time = time;
	fentry[lastname]->vflag = vflag;
	fentry[lastname]->version_no = version_no;
   lastname++;
   /* allocate memory for empty entry at end */
   fentry[lastname] = (struct item *) ealloc (sizeof(struct item)); 
} /* addfname */

/* inlist() */
/* Examines global list built by addfname() to see if supplied filename
is in the list.  

If found, returns the file's position within the archive as the function 
value and the date, time, version flag, and version number as parameters.
If not found, returns -1.  Also returns the highest version no. seen
for this filename and the vflag associated with that version.

A simple sequential search is done.

If justname is nonzero, then the search is for the filename only
without the directory prefix;  else it is for the full
pathname.
*/

long inlist (fname, date, time, this_version_no, high_vflag, 
					high_version_no, high_pos, justname)
char *fname;
unsigned int *date, *time;
unsigned *high_vflag;
unsigned *this_version_no;
unsigned *high_version_no;
long *high_pos;
int justname;
{
   register int i = 0;

	*high_version_no = 0;
	if (justname)
		fname = nameptr (fname);					/* if directory wanted */
   fentry[lastname]->fname = fname;          /* sentinel */
	fentry[lastname]->version_no = 0;

#ifdef IGNORECASE
#define	COMPARE	str_icmp
#else
#define	COMPARE	strcmp
#endif

   while (COMPARE(fname,
			(justname ? nameptr (fentry[i]->fname) : fentry[i]->fname)) != 0) {
      i++;
   }

   if (i == lastname)
      return (-1L);
   else {
		int j;
		*date = fentry[i]->date;
		*time = fentry[i]->time;
		*high_pos = fentry[i]->position;
		*high_vflag = fentry[i]->vflag;
		for (j = i; j < lastname; j++) {	/* find highest version no. for file */
			if (COMPARE(fname,
				(justname ? nameptr (fentry[j]->fname) : fentry[j]->fname)) == 0) {
				if (*high_version_no < fentry[j]->version_no) {
					*high_version_no = fentry[j]->version_no;
					*high_vflag = fentry[j]->vflag;
					*high_pos = fentry[j]->position;
					*date = fentry[j]->date;
					*time = fentry[j]->time;
				}
			}
		}
		*this_version_no = fentry[i]->version_no;
      return (fentry[i]->position);
   }
} /* inlist() */
