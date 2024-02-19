/* ulimit.c -- Setting resource usage for children of the shell. */

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
#include "shell.h"

#include <sys/types.h>
#include <sys/errno.h>
extern int errno;

#ifndef SYSV
#include <sys/time.h>
#include <sys/resource.h>
#else
#include <sys/times.h>
#endif

/* **************************************************************** */
/*								    */
/*			Ulimit builtin and Hacks.       	    */
/*								    */
/* **************************************************************** */

/* Block size for ulimit operations. */
#define ULIMIT_BLOCK_SIZE ((long)1024)

#define u_FILE_SIZE 1
#define u_MAX_BREAK_VAL 3
#define u_PIPE_SIZE 4
#define u_CORE_FILE_SIZE 6
#define u_DATA_SEG_SIZE 8
#define u_PHYS_MEM_SIZE 10
#define u_CPU_TIME_LIMIT 12
#define u_STACK_SIZE 14

#ifndef RLIM_INFINITY
#define RLIM_INFINITY  0x7fffffff
#endif

typedef struct ResourceLimit {
  struct ResourceLimit *next;
  int cmd;
  long limit;
} RESOURCE_LIMIT;

RESOURCE_LIMIT *resource_limits = (RESOURCE_LIMIT *)NULL;

/* Called after we fork.  Sets the process resource limits to the
   values saved in RESOURCE_LIMITS. */
set_process_resource_limits ()
{
  long shell_ulimit ();
  register RESOURCE_LIMIT *limits = resource_limits;

  while (limits)
    {
      shell_ulimit (limits->cmd, limits->limit, 1);
      limits = limits->next;
    }
}

/* Report or set limits associated with certain per-process resources.
   See the help documentation in builtins.c for a full description.
   Chet Ramey & Brian Fox 3/13/89 */
ulimit_builtin (list)
     register WORD_LIST *list;
{
  long shell_ulimit ();
  register char *s;
  int c, setting, cmd, r = EXECUTION_SUCCESS;
  long current_limit, real_limit, limit = -1L;
  long block_factor;

  do
    {
      cmd = u_FILE_SIZE;
      setting = 0;
      block_factor = ULIMIT_BLOCK_SIZE;

      if (list)
	{
	  s = list->word->word;
	  if (s && (*s == '-'))
	    {
	      c = *++s;

	      if (!c || *++s)
		goto error_case;

	      list = list->next;

	      switch (c)
		{
		case 'f': cmd = u_FILE_SIZE;
		  break;
#ifndef SYSV
		case 'a':
		  print_all_limits ();
		  return (EXECUTION_SUCCESS);

		case 'c': cmd = u_CORE_FILE_SIZE;
		  break;

		case 'd': cmd = u_DATA_SEG_SIZE;
		  break;

		case 'm': cmd = u_PHYS_MEM_SIZE;
		  break;

		case 't':
		  cmd = u_CPU_TIME_LIMIT;
		  block_factor = 1000;
		  break;

		case 's': cmd = u_STACK_SIZE;
		  break;
#endif
		default:
		error_case:
#ifdef SYSV
		  report_error("usage: ulimit [-f] [new limit]");
#else
		  report_error("usage: ulimit [-cmdstf] [new limit]");
#endif
		  return EXECUTION_FAILURE;
		}
	    }
      
	  /* If an argument was supplied for the command, then we want to
	     set the limit. */
	  if (list)
	    {
	      s = list->word->word;
	      list = list->next;
	      if (sscanf (s, "%ld", &limit) != 1)
		{
		  if (strcmp (s, "unlimited") == 0)
		    limit = RLIM_INFINITY;
		  else
		    {
		      report_error ("ulimit: bad non-numeric arg `%s'", s);
		      return (EXECUTION_FAILURE);
		    }
		}
	      setting++;
	    }
	}

      if (limit == RLIM_INFINITY)
	block_factor = 1;

      real_limit = limit * block_factor;

      current_limit = shell_ulimit (cmd, real_limit, 0);

      if (setting)
	{
	  /* If we wish to set the limit, we have to make sure that it is
	     a "legal" change. */
	  if (current_limit < (real_limit))
	    {
	      report_error("bad ulimit");
	      return (EXECUTION_FAILURE);
	    }

	  /* Since the change would be "legal", remember the new child
	     resource limit. */
	  {
	    RESOURCE_LIMIT *newlimit, *limits = resource_limits;

	    while (limits)
	      {
		if (limits->cmd == cmd)
		  {
		    limits->limit = real_limit;
		    return (EXECUTION_SUCCESS);
		  }
		limits = limits->next;
	      }
		
	    newlimit = (RESOURCE_LIMIT *)xmalloc (sizeof (RESOURCE_LIMIT));

	    newlimit->cmd = cmd;
	    newlimit->limit = real_limit;
	    newlimit->next = resource_limits;
	    resource_limits = newlimit;
	    return (EXECUTION_SUCCESS);
	  }
	}
      else
	{
	  register RESOURCE_LIMIT *limits = resource_limits;

	  /* If we aren't setting the limit, then we are getting the limit.
	     So print the results. */
	  while (limits)
	    {
	      if (limits->cmd == cmd)
		{
		  current_limit = resource_limits->limit;
		  break;
		}

	      limits = limits->next;
	    }
	  
	  if (current_limit != RLIM_INFINITY)
	    printf ("%ld\n", (current_limit / block_factor));
	  else
	    printf ("unlimited\n");
	}
    }
  while (list);
  return (EXECUTION_SUCCESS);
}

/* The ulimit that we call from within Bash.  Extended to handle
   more resources by Chet Ramey (chet@cwjcc.cwru.edu).
   WHICH says which limit to twiddle; SETTING is non-zero if NEWLIM
   contains the desired new limit.  Otherwise, the existing limit is
   returned. */
long
shell_ulimit (which, newlim, setting)
     int which, setting;
     long newlim;
{
#ifndef SYSV
  struct rlimit limit;
#endif

  switch (which)
    {

    case u_FILE_SIZE:
      if (!setting)
	{
#ifdef SYSV
	  return (ulimit (1, 0l));
#else
	  if (getrlimit (RLIMIT_FSIZE, &limit) != 0 )
	    return ((long) -1);
	  return (limit.rlim_cur);
#endif
	}
      else
	{
#ifdef SYSV
	  return (ulimit (2, newlim));
#else
	  limit.rlim_cur = newlim;
	  return (setrlimit (RLIMIT_FSIZE, &limit));
#endif
	}

#ifndef SYSV
    case u_MAX_BREAK_VAL:
      if (setting || (getrlimit (RLIMIT_DATA, &limit) != 0))
	return ((long) -1);
      return (limit.rlim_cur);

      /* You can't get or set the pipe size with getrlimit. */
    case u_PIPE_SIZE:
      errno = EINVAL;
      return ((long) -1);

    case u_CORE_FILE_SIZE:
      if (!setting)
	{
	  if (getrlimit (RLIMIT_CORE, &limit) != 0)
	    return ((long) -1);
	  return (limit.rlim_cur);
	}
      else
	{
	  limit.rlim_cur = newlim;
	  return (setrlimit (RLIMIT_CORE, &limit));
	}

    case u_DATA_SEG_SIZE:
      if (!setting)
	{
	  if (getrlimit (RLIMIT_DATA, &limit) != 0)
	    return (long) -1;
	  return (limit.rlim_cur);
	}
      else
	{
	  limit.rlim_cur = newlim;
	  return (setrlimit (RLIMIT_DATA, &limit));
	}

    case u_PHYS_MEM_SIZE:
      if (!setting)
	{
	  if (getrlimit (RLIMIT_RSS, &limit) != 0)
	    return ((long) -1);
	  return (limit.rlim_cur);
	}
      else
	{
	  limit.rlim_cur = newlim;
	  return (setrlimit (RLIMIT_RSS, &limit));
	}

    case u_CPU_TIME_LIMIT:
      if (!setting)
	{
	  if (getrlimit (RLIMIT_CPU, &limit) != 0)
	    return ((long) -1);
	  return (limit.rlim_cur);
	}
      else
	{
	  limit.rlim_cur = newlim;
	  return (setrlimit (RLIMIT_CPU, &limit));
	}

    case u_STACK_SIZE:
      if (!setting)
	{
	  if (getrlimit (RLIMIT_STACK, &limit) != 0)
	    return ((long) -1);
	  return (limit.rlim_cur);
	}
      else
	{
	  limit.rlim_cur = newlim;
	  return (setrlimit(RLIMIT_STACK, &limit));
	}
#endif

    default:
      errno = EINVAL;
      return ((long) -1);
    }
}

#ifndef SYSV

#ifndef RLIM_NLIMITS
#define RLIM_NLIMITS	6		/* Number of resource limits */
#endif

struct bsd_resource_limits {
	int	limit_param;		/* param to pass to getrlimit() */
	char	*limit_string;		/* descriptive string to print */
	int	limit_block_factor;
} limits[RLIM_NLIMITS] = {           /* arbitrary order (same as ksh) */
	RLIMIT_CPU,	"cpu time (seconds)",		1000,
	RLIMIT_RSS,	"max memory size (kbytes)",	1024,
	RLIMIT_DATA,	"data seg size (kbytes)",	1024,
	RLIMIT_STACK,	"stack size (kbytes)",		1024,
	RLIMIT_FSIZE,	"file size (blocks)",		1024,
	RLIMIT_CORE,	"core file size (blocks)",	1024,
};

print_all_limits ()
{
  register int i;
  struct rlimit rl;
  long limit;

  for (i = 0; i < RLIM_NLIMITS; i++)             /* for all limits */
    {
      getrlimit (limits[i].limit_param, &rl);
      limit = rl.rlim_cur;
      printf("%-25s",limits[i].limit_string);
      if (limit == RLIM_INFINITY)
	printf("unlimited\n");
      else
	printf("%ld\n", limit / limits[i].limit_block_factor);
    }
}

#endif /* !SYSV */

