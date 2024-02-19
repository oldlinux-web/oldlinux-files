/* The thing that makes children, remembers them, and contains wait loops. */
/* this will probably only work with System V */

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
#include <sys/types.h>
#include <sys/ttold.h>
#include <fcntl.h>
#include <termio.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

#include "config.h"
#include "general.h"
#include "jobs.h"

#ifndef SIGABRT
#define SIGABRT SIGIOT
#endif

#ifdef SYSV
#define killpg(pg, sig)		kill(-(pg),(sig))
#endif /* SYSV */

#if defined(SYSV) || defined(HPUX)
int siginterrupt (sig, flag) int sig, flag; { return 0; }
#endif

int last_made_pid = -1;
int last_asynchronous_pid = -1;

/*
 * Initialize the job control mechanism, and set up the tty stuff.
 */
initialize_jobs ()
{
}

/*
 * Setup this shell to handle C-C, etc.
 */
initialize_job_signals ()
{
  extern int login_shell;
  extern sighandler throw_to_top_level ();

  signal (SIGINT, throw_to_top_level);
  signal (SIGQUIT, SIG_IGN);

  /* If this is a login shell we don't wish to be disturbed by
     stop signals. */
  if (login_shell)
    {
#ifdef SIGSTOP
      signal (SIGSTOP, SIG_IGN);
#endif
#ifdef SIGTSTP
      signal (SIGTSTP, SIG_IGN);
      signal (SIGTTOU, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
#endif
    }
}

/*
 * Fork, handling errors.  Returns the pid of the newly made child, or 0.
 * COMMAND is just for remembering the name of the command; we don't do
 * anything else with it.  ASYNC_P says what to do with the tty.  If
 * non-zero, then don't give it away.
 */
int
make_child (command, async_p)
     char *command;
     int async_p;
{
  int pid;

  /* Discard saved memory. */
  if (command)  
    free (command);

  /* Make new environment array if neccessary. */
  maybe_make_export_env ();

  /* Create the child, handle severe errors. */
  if ((pid = fork ()) < 0)
    {
      report_error ("Memory exhausted or process overflow!");
      throw_to_top_level ();
    }
 
  if (!pid)
    {
      /*
       * Ignore INT and QUIT in asynchronous children.
       */
      if (async_p)
	{
	  signal (SIGINT, SIG_IGN);
	  signal (SIGQUIT, SIG_IGN);
	  last_asynchronous_pid = getpid ();
	}
      else
	{
	  signal (SIGINT, SIG_DFL);
	  signal (SIGQUIT, SIG_DFL);
	}

      /* Children are easily terminated with SIGTERM. */
      signal (SIGTERM, SIG_DFL);

      /* Set the resource limits for this child. (In ulimit.c). */
      set_process_resource_limits ();
    }
  else
    {
      /*
       * In the parent.
       */
      last_made_pid = pid;

      if (async_p)
	last_asynchronous_pid = pid;
    }
  return (pid);
}

/* Wait for a single pid (PID) and return its exit status. */

wait_for_single_pid (pid)
     int pid;
{
  int got_pid;
  union wait status;

  while ((got_pid = wait (&status)) != pid)
    {
      if (got_pid < 0)
	{
	  if (errno != EINTR && errno != ECHILD)
	    file_error ("wait");
	  break;
	}
    }
  QUIT;
}

/* Wait for all of the shell's children to exit. */

wait_for_background_pids ()
{
  /* If we aren't using job control, we let the kernel take care of the
     bookkeeping for us.  wait () will return -1 and set errno to ECHILD 
     when there are no more unwaited-for child processes on both 4.2BSD-based
     and System V-based systems. */

  while (1)
    {
      int got_pid;
      union wait status;

      while ((got_pid = wait(&status)) != -1)  /* wait for ECHILD */
	;
      if (errno != EINTR && errno != ECHILD)
	file_error("wait");
      break;
    }

  QUIT;
}

/*
 * Wait for pid (one of our children) to terminate.
 */
int
wait_for (pid)
     int pid;
{
  extern int interactive;
  int got_pid, return_val, oldmask;
  union wait status;

  while ((got_pid = wait (&status)) != pid)
    {
      if (got_pid < 0 && errno == ECHILD)
	{
	  status.w_termsig = status.w_retcode = 0;
	  break;
	}
      else if (got_pid < 0 && errno != EINTR)
	programming_error ("got errno %d while waiting for %d", errno, pid);
    }

  if (interactive)	/* allow the user to ^C out of the builtin wait */
    QUIT;

  /* Default return value. */
  return_val = status.w_retcode & 0x7f;

  if (status.w_termsig != 0 && status.w_termsig != WSTOPPED)
    {
      extern char *sys_siglist[];
      fprintf (stderr, "%s", sys_siglist[status.w_termsig]);
      if (status.w_coredump)
	fprintf (stderr, " (core dumped)");
      fprintf (stderr, "\n");
      return_val = status.w_termsig + 128;
    }

  if (status.w_termsig != 0)
    set_tty_state ();
  else
    get_tty_state ();

  return (return_val);
}

/* Give PID SIGNAL.  This determines what job the pid belongs to (if any).
   If PID does belong to a job, and the job is stopped, then CONTinue the
   job after giving it SIGNAL.  Returns -1 on failure.  If GROUP is non-null,
   then kill the process group associated with PID. */
int
kill_pid (pid, signal, group)
     int pid, signal, group;
{
  int result;

  if (group)
    result = killpg (pid, signal);
  else
    result = kill (pid, signal);

  return (result);
}

static struct termio shell_tty_info;

/* Fill the contents of shell_tty_info with the current tty info. */
get_tty_state ()
{
  int tty = open ("/dev/tty", O_RDONLY);
  if (tty != -1)
    {
      ioctl (tty, TCGETA, &shell_tty_info);
      close (tty);
    }
}

/* Make the current tty use the state in shell_tty_info. */
set_tty_state ()
{
  int tty = open ("/dev/tty", O_RDONLY);
  if (tty != -1)
    {
      ioctl (tty, TCSETAW, &shell_tty_info);  /* Wait for output, no flush */
      close (tty);
    }
}

/* Give the terminal to PGRP.  */
give_terminal_to (pgrp)
     int pgrp;
{
}

/*
 * stop a pipeline
 */
stop_pipeline (async, ignore)
     int async;
     char *ignore;
{
}

/*
 * Print descriptive information about the job with leader pid PID.
 */

describe_pid (pid)
     int pid;
{
  fprintf (stderr, "<%d>\n", pid);
}
