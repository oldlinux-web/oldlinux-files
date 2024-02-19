/* I can't stand it anymore!  Please can't we just write the
   whole Unix system in lisp or something? */

/* Copyright (C) 1987,1989 Free Software Foundation, Inc.

This file is part of GNU Bash, the Bourne Again SHell.

Bash is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 1, or (at your option) any later
version.

Bash is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with Bash; see the file COPYING.  If not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

/* **************************************************************** */
/*								    */
/*		      Unwind Protection Scheme for Bash		    */
/*								    */
/* **************************************************************** */

#include <sys/signal.h>
#include "config.h"
#include "general.h"

/* If CLEANUP is null, then ARG contains a tag to throw back to. */
typedef struct _uwp {
  struct _uwp *next;
  Function *cleanup;
  char *arg;
} UNWIND_ELT;

static UNWIND_ELT *unwind_protect_list = (UNWIND_ELT *)NULL;

/* Run a function without interrupts. */
without_interrupts (function, arg1, arg2)
     Function *function;
     char *arg1, *arg2;
{
  SigHandler *old_int;

  old_int = (SigHandler *)signal (SIGINT, SIG_IGN);
  (*function)(arg1, arg2);
  signal (SIGINT, old_int);
}

/* Start the beginning of a region. */
begin_unwind_frame (tag)
     char *tag;
{
  add_unwind_protect ((Function *)NULL, tag);
}

/* Discard the unwind protects back to TAG. */
discard_unwind_frame (tag)
     char *tag;
{
  int unwind_frame_discard_internal ();

  without_interrupts (unwind_frame_discard_internal, tag);
}

/* Run the unwind protects back to TAG. */
run_unwind_frame (tag)
     char *tag;
{
  int unwind_frame_run_internal ();
  without_interrupts (unwind_frame_run_internal, tag);
}

/* Add the function CLEANUP with ARG to the list of unwindable things. */
add_unwind_protect (cleanup, arg)
     Function *cleanup;
     char *arg;
{
  int add_unwind_protect_internal ();
  without_interrupts (add_unwind_protect_internal, cleanup, arg);
}

/* Remove the top unwind protect from the list. */
remove_unwind_protect ()
{
  int remove_unwind_protect_internal ();
  without_interrupts (remove_unwind_protect_internal);
}

/* Run the list of cleanup functions in unwind_protect_list. */
run_unwind_protects ()
{
  int run_unwind_protects_internal ();
  without_interrupts (run_unwind_protects_internal);
}

/* **************************************************************** */
/*								    */
/*                        The Actual Functions                 	    */
/*								    */
/* **************************************************************** */

add_unwind_protect_internal (cleanup, arg)
     Function *cleanup;
     char *arg;
{
  UNWIND_ELT *elt;

  elt = (UNWIND_ELT *)xmalloc (sizeof (UNWIND_ELT));
  elt->cleanup = cleanup;
  elt->arg = arg;
  elt->next = unwind_protect_list;
  unwind_protect_list = elt;
}

remove_unwind_protect_internal ()
{
  UNWIND_ELT *elt = unwind_protect_list;

  if (elt)
    {
      unwind_protect_list = unwind_protect_list->next;
      free (elt);
    }
}

run_unwind_protects_internal ()
{
  UNWIND_ELT *t, *elt = unwind_protect_list;

  while (elt)
    {
      /* This function can be run at strange times, like when unwinding
	 the entire world of unwind protects.  Thus, we may come across
	 an element which is simply a label for a catch frame.  Don't call
	 the non-existant function. */
      if (elt->cleanup)
	(*(elt->cleanup)) (elt->arg);

      t = elt;
      elt = elt->next;
      free (t);
    }
  unwind_protect_list = elt;
}

unwind_frame_discard_internal (tag)
     char *tag;
{
  UNWIND_ELT *elt;

  while (elt = unwind_protect_list)
    {
      unwind_protect_list = unwind_protect_list->next;
      if (!elt->cleanup && strcmp (elt->arg, tag) == 0)
	{
	  free (elt);
	  break;
	}
      else
	free (elt);
    }
}

unwind_frame_run_internal (tag)
     char *tag;
{
  UNWIND_ELT *elt;

  while (elt = unwind_protect_list)
    {
      unwind_protect_list = elt->next;

      /* If tag, then compare. */
      if (!elt->cleanup)
	{
	  if (strcmp (elt->arg, tag) == 0)
	    {
	      free (elt);
	      break;
	    }
	  free (elt);
	  continue;
	}
      else
	{
	  (*(elt->cleanup)) (elt->arg);
	  free (elt);
	}
    }
}
