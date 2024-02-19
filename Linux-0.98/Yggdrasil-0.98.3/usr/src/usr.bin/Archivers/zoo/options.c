#ifndef LINT
static char sccsid[]="@(#) options.c 2.1 87/12/25 12:23:56";
#endif /* LINT */

/*
Copyright (C) 1986, 1987 Rahul Dhesi -- All rights reserved
*/
/*
Here we define routines specific to only a few systems.  Routines are
selected based on defined symbols.  Routines specific to only one
system are in machine.c for the appropriate system.
*/

#include "options.h"
#include "zooio.h"
#include "various.h"
#include "zoo.h"
#include "zoofns.h"
#include "errors.i"

#ifdef REN_LINK         
/* rename using link() followed by unlink() */
/* 
The following code assumes that if unlink() returns nonzero, then the
attempt to unlink failed.  If unlink() ever returns nonzero after actually
unlinking the file, then the file being renamed will be lost!!!  Test this 
thoroughly.  It is assumed that link() and unlink() return zero if no
error else nonzero.
*/
int chname (newname, oldname)
char *newname, *oldname;
{
   int status;
   if (link (oldname, newname) == 0) { /* if we can create new name */
      status = unlink (oldname);          /*   unlink old one */
      if (status != 0) {                  /*   if unlink of old name failed */
         unlink (newname);                /*     cancel new link */
         return (-1);                     /*     return error */
      } else
         return (0);
   }
   else                    /* couldn't create new link */
      return (-1);
}
#else
/* else not REN_LINK */

int chname (newname, oldname)
char *newname, *oldname;
{
#ifdef REN_STDC
   if (rename(oldname, newname) != 0)     /* ANSI standard */
#else
   if (rename(newname, oldname) != 0)     /* its reverse */
#endif
      return (-1);
   else
      return (0);
}
#endif /* end of not REN_LINK */

/*
Standard exit handler;  not used if specific system defines its
own.
*/
#ifndef SPECEXIT
void zooexit (status)
int status;
{
	exit (status);
}
#endif
