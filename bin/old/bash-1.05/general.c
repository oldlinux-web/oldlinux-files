/* general.c -- Stuff that is used by all files. */

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

#include <stdio.h>
#include <errno.h>

#include "shell.h"
#ifdef SYSV
#include <string.h>
#else
#include <strings.h>
#endif  /* SYSV */

#ifndef NULL
#define NULL 0x0
#endif


#if !defined (rindex)
extern char *rindex ();
#endif

/* **************************************************************** */
/*								    */
/*		   Memory Allocation and Deallocation.		    */
/*								    */
/* **************************************************************** */

char *
xmalloc (size)
     int size;
{
  register char *temp = (char *)malloc (size);
  if (!temp)
    fatal_error ("Out of virtual memory!");
  return (temp);
}

char *
xrealloc (pointer, size)
     register char *pointer;
     int size;
{
  pointer = (char *)realloc (pointer, size);
  if (!pointer)
    fatal_error ("Out of virtual memory!");
  return (pointer);
}


/* **************************************************************** */
/*								    */
/*			Generic List Functions			    */
/*								    */
/* **************************************************************** */

/* Call FUNCTION on every member of LIST, a generic list. */
map_over_list (list, function)
     GENERIC_LIST *list;
     Function *function;
{
  while (list) {
    (*function) (list);
    list = list->next;
  }
}

/* Call FUNCTION on every string in WORDS. */
map_over_words (words, function)
     WORD_LIST *words;
     Function *function;
{
  while (words) {
    (*function)(words->word->word);
    words = words->next;
  }
}

/* Reverse the chain of structures in LIST.  Output the new head
   of the chain.  You should always assign the output value of this
   function to something, or you will lose the chain. */
GENERIC_LIST *
reverse_list (list)
     register GENERIC_LIST *list;
{
  register GENERIC_LIST *next, *prev = (GENERIC_LIST *)NULL;

  while (list) {
    next = list->next;
    list->next = prev;
    prev = list;
    list = next;
  }
  return (prev);
}

/* Return the number of elements in LIST, a generic list. */
list_length (list)
     register GENERIC_LIST *list;
{
  register int i;

  for (i = 0; list; list = list->next, i++);
  return (i);
}

/* Delete the element of LIST which satisfies the predicate function COMPARER.
   Returns the element that was deleted, so you can dispose of it, or -1 if
   the element wasn't found.  COMPARER is called with the list element and
   then ARG.  Note that LIST contains the address of a variable which points
   to the list.  You might call this function like this:
   
   SHELL_VAR *elt = delete_element (&variable_list, check_var_has_name, "foo");
   dispose_variable (elt);
*/
GENERIC_LIST *
delete_element (list, comparer, arg)
     GENERIC_LIST **list;
     Function *comparer;
{
  register GENERIC_LIST *prev = (GENERIC_LIST *)NULL;
  register GENERIC_LIST *temp = *list;

  while (temp) {
    if ((*comparer) (temp, arg)) {
      if (prev) prev->next = temp->next;
      else *list = temp->next;
      return (temp);
    }
    prev = temp;
    temp = temp->next;
  }
  return ((GENERIC_LIST *)-1);
}

/* Find NAME in ARRAY.  Return the index of NAME, or -1 if not present.
   ARRAY shoudl be NULL terminated. */
find_name_in_list (name, array)
     char *name, *array[];
{
  int i;

  for (i=0; array[i]; i++)
    if (strcmp (name, array[i]) == 0)
      return (i);

  return (-1);
}

/* Return the length of ARRAY, a NULL terminated array of char *. */
array_len (array)
     register char **array;
{
  register int i;
  for (i=0; array[i]; i++);
  return (i);
}

/* Free the contents of ARRAY, a NULL terminated array of char *. */
free_array (array)
     register char **array;
{
  register int i = 0;

  if (!array) return;

  while (array[i]) free (array[i++]);
  free (array);
}

/* Append LIST2 to LIST1.  Return the header of the list. */
GENERIC_LIST *
list_append (head, tail)
     GENERIC_LIST *head, *tail;
{
  register GENERIC_LIST *t_head = head;

  if (!t_head)
    return (tail);

  while (t_head->next) t_head = t_head->next;
  t_head->next = tail;
  return (head);
}

/* Some random string stuff. */

/* Remove all leading whitespace from STRING.  This includes
   newlines.  STRING should be terminated with a zero. */
strip_leading (string)
     char *string;
{
  char *start = string;

  while (*string && (whitespace (*string) || *string == '\n')) string++;

  if (string != start) {
    int len = strlen (string);
    bcopy (string, start, len);
    start[len] = '\0';
  }
}

/* Remove all trailing whitespace from STRING.  This includes
   newlines.  STRING should be terminated with a zero. */
strip_trailing (string)
     char *string;
{
  int len = strlen (string);

  while (len--)
    if (!whitespace (string[len]) && string[len] != '\n') {
      string[len + 1] = '\0';
      return;
    }
}

/* Turn STRING (a pathname) into an absolute pathname, assuming that
   DOT_PATH contains the symbolic location of '.'.  This always
   returns a new string, even if STRING was an absolute pathname to
   begin with. */

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

static char current_path[MAXPATHLEN];

char *
make_absolute (string, dot_path)
     char *string, *dot_path;
{
  register char *cp;

  if (!dot_path || *string == '/')
    return (savestring (string));

  strcpy (current_path, dot_path);

  if (!current_path[0])
    strcpy (current_path, "./");

  cp = current_path + (strlen (current_path) - 1);

  if (*cp++ != '/')
    *cp++ = '/';

  *cp = '\0';

  while (*string)
    {
      if (*string == '.')
	{
	  if (!string[1])
	    return (savestring (current_path));

	  if (string[1] == '/')
	    {
	      string += 2;
	      continue;
	    }

	  if (string[1] == '.' && (string[2] == '/' || !string[2]))
	    {
	      string += 2;

	      if (*string)
		string++;

	      pathname_backup (current_path, 1);
	      cp = current_path + strlen (current_path);
	      continue;
	    }
	}

      while (*string && *string != '/')
	*cp++ = *string++;

      if (*string)
	*cp++ = *string++;

      *cp = '\0';
    }
  return (savestring (current_path));
}

/* Remove the last N directories from PATH.  Do not leave a blank path.
   PATH must contain enough space for MAXPATHLEN characters. */
pathname_backup (path, n)
     char *path;
     int n;
{
  register char *p = path + strlen (path);

  if (*path)
    p--;

  while (n--)
    {
      while (*p == '/')
	p--;

      while (*p != '/')
	p--;

      *++p = '\0';
    }
}

/* Return 1 if STRING contains an absolute pathname, else 0. */
absolute_pathname (string)
     char *string;
{
  if (!string || !strlen (string))
    return (0);

  if (*string == '/')
    return (1);

  if (*string++ == '.')
    {
      if ((!*string) || *string == '/')
	return (1);

      if (*string++ == '.')
	if (!*string || *string == '/')
	  return (1);
    }
  return (0);
}

/* Return the `basename' of the pathname in STRING (the stuff after the
   last '/').  If STRING is not a full pathname, simply return it. */
char *
base_pathname (string)
     char *string;
{
  char *p = rindex (string, '/');

  if (*string != '/')
    return (string);

  if (p)
    return (++p);
  else
    return (string);
}

/* Determine if s2 occurs in s1.  If so, return a pointer to the
   match in s1.  The compare is case insensitive. */
char *
strindex (s1, s2)
     register char *s1, *s2;
{
  register int i, l = strlen (s2);
  register int len = strlen (s1);

  for (i = 0; (len - i) >= l; i++)
    if (strnicmp (&s1[i], s2, l) == 0)
      return (s1 + i);
  return ((char *)NULL);
}


#ifndef to_upper
#define lowercase_p(c) (((c) > ('a' - 1) && (c) < ('z' + 1)))
#define uppercase_p(c) (((c) > ('A' - 1) && (c) < ('Z' + 1)))
#define pure_alphabetic(c) (lowercase_p(c) || uppercase_p(c))
#define to_upper(c) (lowercase_p(c) ? ((c) - 32) : (c))
#define to_lower(c) (uppercase_p(c) ? ((c) + 32) : (c))
#endif

/* Compare at most COUNT characters from string1 to string2.  Case
   doesn't matter. */
int
strnicmp (string1, string2, count)
     char *string1, *string2;
{
  register char ch1, ch2;

  while (count) {
    ch1 = *string1++;
    ch2 = *string2++;
    if (to_upper(ch1) == to_upper(ch2))
      count--;
    else break;
  }
  return (count);
}

/* strcmp (), but caseless. */
int
stricmp (string1, string2)
     char *string1, *string2;
{
  register char ch1, ch2;

  while (*string1 && *string2) {
    ch1 = *string1++;
    ch2 = *string2++;
    if (to_upper(ch1) != to_upper(ch2))
      return (1);
  }
  return (*string1 | *string2);
}

#if defined (SONY)

char *
strchr (s, c)
     char *s;
     int c;
{
  char *index ();
  return (index (s, c));
}

#endif /* SONY */

extern int errno;
#include <errno.h>
#include <fcntl.h>

#ifdef NO_DUP2
dup2 (fd1, fd2)
     int fd1, fd2;
{
  if (fcntl (fd1, F_GETFL, 0) == -1)	/* fd1 is an invalid fd */
    return (-1);
  if (fd2 < 0 || fd2 >= NOFILE)
    {
      errno = EBADF;
      return (-1);
    }
  if (fd1 == fd2)
    return (0);
  (void) close (fd2);
  return (fcntl (fd1, F_DUPFD, fd2));
}
#endif /* NO_DUP */

#ifdef SYSV
#include <sys/utsname.h>
bcopy(s,d,n) char *d,*s; { while(n--) *d++ = *s++; }
char *getwd(s) char *s; { getcwd(s,MAXPATHLEN); return s; }
char *index(s,c) char *s; { char *strchr(); return strchr(s,c); }
char *rindex(s,c) char *s; { char *strrchr(); return strrchr(s,c); }
     
gethostname (name, namelen) 
     char *name;
     int namelen;
{
  int i;
  struct utsname uts;

  --namelen;

  uname (&uts);
  i = strlen (uts.nodename) + 1;
  strncpy (name, uts.nodename, i < namelen ? i : namelen);
  name[namelen] = '\0';
  return (0);
}

int
sysv_getc (stream)
     FILE *stream;
{
  int result;
  char c;

  while (1)
    {
      result = read (fileno (stream), &c, sizeof (char));
      if (result == sizeof (char))
	return (c);

      if (errno != EINTR)
	return (EOF);
    }
}
#endif  /* SYSV */
