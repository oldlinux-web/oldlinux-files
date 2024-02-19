/* The thing that makes children, remembers them, and contains wait loops. */

/* Copyright (C) 1989 Free Software Foundation, Inc.

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

/* Something that can be ignored. */
#define IGNORE_ARG (char *)0

#include "config.h"

#ifndef JOB_CONTROL
#include "nojobs.c"
#else

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/param.h>

/* Terminal handling stuff, to save and restore tty state. */
#define NEW_TTY_DRIVER

#if defined(SYSV) || defined(hpux) || defined(ALTOS)
#undef NEW_TTY_DRIVER
#endif /* SYSV || hpux || ALTOS */

#ifdef NEW_TTY_DRIVER
#include <sgtty.h>
#else
#include <termio.h>
#endif /* NEW_TTY_DRIVER */

/* For the TIOCGPGRP and TIOCSPGRP ioctl parameters on HP-UX */

#ifdef hpux
#include <bsdtty.h>
#endif /* hpux */

#include "shell.h"
#include "jobs.h"

/* Not all systems define errno in errno.h. */
extern int errno;

#ifndef sigmask
#define sigmask(x) (1 << ((x)-1))
#endif

#ifndef SIGABRT
#define SIGABRT SIGIOT
#endif

#ifndef SIGCHLD
#define SIGCHLD SIGCLD
#endif

/* The array of known jobs. */
JOB **jobs = (JOB **)NULL;

/* The number of slots currently allocated to JOBS. */
int job_slots = 0;

/* The number of additional slots to allocate when we run out. */
#define JOB_SLOTS 5

/* The controlling tty for this shell. */
int shell_tty;

/* The shell's process group. */
int shell_pgrp = -1;

/* The terminal's process group. */
int terminal_pgrp = -1;

/* The process group of the shell's parent. */
int original_pgrp = -1;

/* The process group of the pipeline currently being made. */
int pipeline_pgrp = 0;

/* The job which is current; i.e. the one that `%+' stands for. */
int current_job = NO_JOB;

/* The previous job; i.e. the one that `%-' stands for. */
int previous_job = NO_JOB;

/* Last child made by the shell.  */
int last_made_pid = -1;

/* Pid of the last asynchronous child. */
int last_asynchronous_pid = -1;

/* Non-zero allows asynchronous job notification.  If not set,
   then job state notification only takes place just before a
   prompt is printed. */
int asynchronous_notification = 0;

#ifndef hpux
/* The total amount of system time spent running processes for me. */
struct timeval total_systime = {0, 0};
long system_minutes_used = 0;
int system_seconds_used = 0;

/* The total amount of user time spent running processes for me. */
struct timeval total_usertime = {0, 0};
long user_minutes_used = 0;
int user_seconds_used = 0;
#endif /* hpux */

/* The pipeline currently being built. */
PROCESS *the_pipeline = (PROCESS *)NULL;

/* If this is non-zero, do job control. */
int job_control = 1;

/* Call this when you start making children. */
int already_making_children = 0;

/* These are definitions to map POSIX 1003.1 functions onto existing BSD
   library functions and system calls. */

#define setpgid(pid, pgrp)	setpgrp (pid, pgrp)
#define tcsetpgrp(fd, pgrp)	ioctl ((fd), TIOCSPGRP, &(pgrp))

tcgetpgrp (fd)
     int fd;
{
  int pgrp;

  /* ioctl will handle setting errno correctly. */
  if (ioctl (fd, TIOCGPGRP, &pgrp) < 0)
    return (-1);
  return (pgrp);
}

/* END of POISX 1003.1 definitions. */

making_children ()
{
  if (already_making_children)
    return;

  already_making_children = 1;
  start_pipeline ();
}

stop_making_children ()
{
  already_making_children = 0;
}

/* Start building a pipeline.  */
start_pipeline ()
{
  if (the_pipeline)
    {
      discard_pipeline (the_pipeline);
      the_pipeline = (PROCESS *)NULL;
      pipeline_pgrp = 0;
    }
}

/* Stop building a pipeline.  Install the process list in the job array.
   This returns the index of the newly installed job.
   DEFERRED is a command structure to be executed upon satisfactory
   execution exit of this pipeline. */
int
stop_pipeline (async, deferred)
     int async;
     COMMAND *deferred;
{
  register int i, j;
  int oldmask;
  JOB *newjob = (JOB *)NULL;
  char *get_string_value ();

  oldmask = sigblock (sigmask (SIGCHLD));

  cleanup_dead_jobs ();

  if (!job_slots)
    {
      jobs =
	(JOB **)xmalloc ((1 + (job_slots = JOB_SLOTS)) * sizeof (JOB *));

      /* Now blank out these new entries. */
      for (i = 0; i < job_slots; i++)
	jobs[i] = (JOB *)NULL;
    }

  /* Scan from the last slot backward, looking for the next free one. */
  for (i = job_slots; i; i--)
    if (jobs[i - 1])
      break;

  /* Do we need more room? */
  if (i == job_slots)
    {
      jobs = (JOB **)realloc
	(jobs, (1 + (job_slots += JOB_SLOTS)) * sizeof (JOB *));

      for (j = i; j < job_slots; j++)
	jobs[j] = (JOB *)NULL;
    }

  /* Add the current pipeline to the job list. */
  if (the_pipeline)
    {
      extern int errno, sys_nerr;
      extern char *sys_errlist[];
      register PROCESS *p;

      newjob = (JOB *)xmalloc (sizeof (JOB));

      for (p = the_pipeline; p->next != the_pipeline; p = p->next);
      p->next = (PROCESS *)NULL;
      newjob->pipe = (PROCESS *)reverse_list (the_pipeline);
      for (p = newjob->pipe; p->next; p = p->next);
      p->next = newjob->pipe;

      the_pipeline = (PROCESS *)NULL;
      newjob->pgrp = pipeline_pgrp;
      pipeline_pgrp = 0;

      /* Flag to see if in another pgrp. */
      newjob->job_control = job_control;

      /* Set the state of this pipeline. */
      {
	register PROCESS *p = newjob->pipe;
	register int any_alive = 0;
	register int any_stopped = 0;

	do
	  {
	    any_alive |= p->running;
	    any_stopped |= WIFSTOPPED (p->status);
	    p = p->next;
	  }
	while (p != newjob->pipe);

	if (any_alive)
	  {
	    newjob->state = JRUNNING;
	  }
	else
	  {
	    if (any_stopped)
	      newjob->state = JSTOPPED;
	    else
	      newjob->state = JDEAD;
	  }
      }

      newjob->notified = 0;

      newjob->wd = get_string_value ("PWD");

      if (newjob->wd)
	newjob->wd = savestring (newjob->wd);
      else
	newjob->wd = (char *)get_working_directory ("");

      if (!(newjob->wd))
	newjob->wd = savestring ("<no directory>");

      newjob->deferred = deferred;

      jobs[i] = newjob;
    }

  if (async)
    {
      if (newjob)
	newjob->foreground = 0;
      reset_current ();
    }
  else
    {
      if (newjob)
	{
	  newjob->foreground = 1;
	  /*
	   *            !!!!! NOTE !!!!!  (chet@ins.cwru.edu)
	   *
	   * The currently-accepted job control wisdom says to set the
	   * terminal's process group n+1 times in an n-step pipeline:
	   * once in the parent and once in each child.  This is where
	   * the parent gives it away.
	   *
	   */
	  if (job_control && pipeline_pgrp)
	    give_terminal_to (pipeline_pgrp);
	}
    }

  stop_making_children ();
  sigsetmask (oldmask);
  return (current_job);
}

/* Delete all DEAD jobs that the user had received notification about. */
cleanup_dead_jobs ()
{
  int oldmask = sigblock (sigmask (SIGCHLD));
  register int i;

  for (i = 0; i < job_slots; i++)
    if (jobs[i] && JOBSTATE (i) == JDEAD && jobs[i]->notified)
      delete_job (i);

  sigsetmask (oldmask);
}

/* Delete the job at INDEX from the job list. */
delete_job (index)
     int index;
{
  register JOB *temp = jobs[index];

  if (index == current_job || index == previous_job)
    reset_current ();

  jobs[index] = (JOB *)NULL;

  free (temp->wd);
  discard_pipeline (temp->pipe);

  if (temp->deferred)
    dispose_command (temp->deferred);

  free (temp);
}

/* Get rid of the data structure associated with a process chain. */
discard_pipeline (chain)
     register PROCESS *chain;
{
  register PROCESS *this, *next;

  this = chain;
  do
    {
      next = this->next;
      if (this->command)
	free (this->command);
      free (this);
      this = next;
    }
  while (this != chain);
}

/* Add this process to the chain being built in the_pipeline.
   NAME is the command string that will be exec'ed later.
   PID is the process id of the child. */
add_process (name, pid)
     char *name;
     int pid;
{
  PROCESS *t = (PROCESS *)xmalloc (sizeof (PROCESS));

  t->next = the_pipeline;
  t->pid = pid;
  t->status.w_status = 0;
  t->running = 1;
  t->command = name;
  the_pipeline = t;

  if (!(t->next))
    {
      t->next = t;
    }
  else
    {
      register PROCESS *p = t->next;

      while (p->next != t->next) p = p->next;
      p->next = t;
    }
}

/* Map FUNC over the list of jobs.  If FUNC returns non-zero,
   then it is time to stop mapping, and that is the return value
   for map_over_jobs.  FUNC is called with a JOB, arg1, arg2,
   and INDEX. */
map_over_jobs (func, arg1, arg2)
     Function *func;
{
  register int i;

  for (i = 0; i < job_slots; i++)
    {
      if (jobs[i])
	{
	  int result = (*func)(jobs[i], arg1, arg2, i);
	  if (result)
	    return (result);
	}
    }
  return (0);
}

/* Return the pipeline that PID belongs to.  Note that the pipeline
   doesn't have to belong to a job. */
PROCESS *
find_pipeline (pid)
     int pid;
{
  int job;

  /* See if this process is in the pipeline that we are building. */
  if (the_pipeline)
    {
      register PROCESS *p = the_pipeline;

      do
	{
	  /* Return it if we found it. */
	  if (p->pid == pid)
	    return (p);

	  p = p->next;
	}
      while (p != the_pipeline);
    }

  job = find_job (pid);

  if (job == NO_JOB)
    return ((PROCESS *)NULL);
  else
    return (jobs[job]->pipe);
}

/* Return the job index that PID belongs to, or NO_JOB if it doesn't
   belong to any job. */
int
find_job (pid)
     int pid;
{
  register int i;
  register PROCESS *p;

  for (i = 0; i < job_slots; i++)
    {
      if (jobs[i])
	{
	  p = jobs[i]->pipe;

	  do
	    {
	      if (p->pid == pid)
		return (i);

	      p = p->next;
	    }
	  while (p != jobs[i]->pipe);
	}
    }

  return (NO_JOB);
}

/* Print descriptive information about the job with leader pid PID. */
describe_pid (pid)
     int pid;
{
  int job;
  int oldmask = sigblock (sigmask (SIGCHLD));

  job = find_job (pid);

  if (job != NO_JOB)
    printf ("[%d] %d\n", job + 1, pid);
  else
    programming_error ("describe_pid: No such pid (%d)!\n", pid);

  sigsetmask (oldmask);
}


/* This is the way to print out information on a job if you
   know the index.  FORMAT is:

    0)   [1]+ Running		emacs
    1)   [1]+ 2378 Running	emacs
   -1)   [1]+ 2378		emacs

    0)   [1]+ Stopped		ls | more
    1)   [1]+ 2369 Stopped	ls
	      2367	      | more
 */
pretty_print_job (index, format, stream)
     int index, format;
     FILE *stream;
{
  register PROCESS *p;
  int first, oldmask;
  union wait first_job_cond;
  int name_padding;

  oldmask = sigblock (sigmask (SIGCHLD));

  fprintf (stream, "[%d]%c ", index + 1,
	   (index == current_job) ? '+':
	   (index == previous_job) ? '-' : ' ');
  
  first = 1;
  p = jobs[index]->pipe;

  do
    {
      if (!first && !format)
	fprintf (stream, " |");
      else if (!first)
	fprintf (stream, "     ");

      if (format)
	fprintf (stream, "%d", p->pid);

      fprintf (stream, " ");

      if (format > -1)
	{
	  extern char *sys_siglist[];
	  union wait status;
	  char *temp = "Done";

	  if (JOBSTATE (index) == JSTOPPED && !format)
	    temp = "Stopped";

	  status = p->status;
	  if (p->running)
	    {
	      temp = "Running";
	    }
	  else
	    {
	      if (status.w_termsig)
		if (status.w_termsig == WSTOPPED)
		  temp = sys_siglist[status.w_stopsig];
		else
		  temp = sys_siglist[status.w_termsig];
	    }

	  if (first)
	    first_job_cond = status;
	  else
	    if (format)
	      {
		if (status.w_status == first_job_cond.w_status)
		  temp = "";
	      }
	    else
	      temp = (char *)NULL;

	  if (temp)
	    {
	      fprintf (stream, "%s", temp);

	      if (strlen (temp))
		name_padding = LONGEST_SIGNAL_DESC - strlen (temp);
	      else
		name_padding = LONGEST_SIGNAL_DESC - 2; /* strlen ("| ") */

	      fprintf (stream, "%*s", name_padding, "");

	      if ((status.w_termsig != WSTOPPED) && (status.w_coredump))
		fprintf (stream, "(core dumped) ");
	    }
	}

      if (first)
	{
	  /* fprintf (stream, "  "); */
	}
      else
	{
	  if (format)
	    fprintf (stream, "| ");
	}

      fprintf (stream, "%s", p->command);

      if (p->next == jobs[index]->pipe) 
	{
	  if (JOBSTATE (index) == JRUNNING && jobs[index]->foreground == 0)
	    fprintf (stream, " &");

	  if (strcmp (get_string_value ("PWD"), jobs[index]->wd) != 0)
	    fprintf (stream,
		     "  (wd: %s)", polite_directory_format (jobs[index]->wd));
	}

      if (format || (p->next == jobs[index]->pipe))
	fprintf (stream, "\r\n");
      
      first = 0;
      p = p->next;
    } while (p != jobs[index]->pipe);
  fflush (stream);
  sigsetmask (oldmask);
}

list_one_job (job, format, ignore, index)
     JOB *job;
     int format, ignore, index;
{
  pretty_print_job (index, format, stdout);
  return (0);
}

/* List jobs.  If FORMAT is non-zero, then the long form of the information
   is printed, else just a short version. */
list_jobs (format)
     int format;
{
  cleanup_dead_jobs ();
  map_over_jobs (list_one_job, format, (int) IGNORE_ARG);
}

/* Fork, handling errors.  Returns the pid of the newly made child, or 0.
   COMMAND is just for remembering the name of the command; we don't do
   anything else with it.  ASYNC_P says what to do with the tty.  If
   non-zero, then don't give it away. */
int
make_child (command, async_p)
     char *command;
     int async_p;
{
  int pid, oldmask;
  
  oldmask = sigblock (sigmask (SIGINT) | sigmask (SIGCHLD));

  making_children ();

  /* Make new environment array if neccessary. */
  maybe_make_export_env ();

  /* Create the child, handle severe errors. */
  if ((pid = fork ()) < 0)
    {
      extern sighandler throw_to_top_level ();

      sigsetmask (oldmask);
      report_error ("Memory exhausted or process overflow!");
      throw_to_top_level ();
    }
 
  if (!pid)
    {
      /* In the child.  Give this child the right process group, set the
	 signals to the default state for a new process. */
      signal (SIGINT, SIG_DFL);
      signal (SIGQUIT, SIG_DFL);
      signal (SIGTERM, SIG_DFL);

      /* Set the resource limits for this child. (In ulimit.c). */
      set_process_resource_limits ();

      /* Restore the sigmask before changing the tty pgrp, since a
	 SIGINT may have occurred in fork (), and we don't want to
	 surprise read (). */
/*      sigsetmask (oldmask); */

      if (job_control)
	{
	  /* All processes in this pipeline belong in the same
	     process group. */

	  if (!pipeline_pgrp)	/* Then this is the first child. */
	    pipeline_pgrp = getpid ();

	  /* Check for running command in backquotes. */
	  if (pipeline_pgrp == shell_pgrp)
	    {
	      signal (SIGTSTP, SIG_IGN);
	      signal (SIGTTOU, SIG_IGN);
	      signal (SIGTTIN, SIG_IGN);
	    }
	  else
	    {
	      signal (SIGTSTP, SIG_DFL);
	      signal (SIGTTOU, SIG_DFL);
	      signal (SIGTTIN, SIG_DFL);
	    }
	
	  if (!async_p)
	    give_terminal_to (pipeline_pgrp);

	  setpgrp (0, pipeline_pgrp);
	}
      else			/* Without job control... */
	{
	  if (!pipeline_pgrp)
	    pipeline_pgrp = shell_pgrp;

	  signal (SIGTSTP, SIG_IGN);
	  signal (SIGTTOU, SIG_IGN);
	  signal (SIGTTIN, SIG_IGN);

	  if (async_p)
	    {
	      signal (SIGINT, SIG_IGN);
	      signal (SIGQUIT, SIG_IGN);
	    }
	}

      if (async_p)
	last_asynchronous_pid = getpid ();
    }
  else
    {
      /* In the parent.  Remember the pid of the child just created
	 as the proper pgrp if this is the first child. */

      if (job_control)
	{
	  if (!pipeline_pgrp)
	    {
	      pipeline_pgrp = pid;
	      /* Don't twiddle terminal pgrps in the parent!  This is the bug,
		 not the good thing of twiddling them in the child! */
	      /* give_terminal_to (pipeline_pgrp); */
	    }
	  setpgid (pid, pipeline_pgrp);
	}
      else
	{
	  if (!pipeline_pgrp)
	    pipeline_pgrp = shell_pgrp;
	}

      /* Place all processes into the jobs array regardless of the
	 state of job_control.  */
      add_process (command, pid);

      if (async_p)
	last_asynchronous_pid = pid;

      last_made_pid = pid;
    }
  sigsetmask (oldmask);
  return (pid);
}

/* When we end a job abnormally, or if we stop a job, we set the tty to the
   state kept in here.  When a job ends normally, we set the state in here
   to the state of the tty. */

#ifdef NEW_TTY_DRIVER
static struct sgttyb shell_tty_info;
static struct tchars shell_tchars;
static struct ltchars shell_ltchars;
#else
static struct termio shell_tty_info;
#endif

/* Fill the contents of shell_tty_info with the current tty info. */
get_tty_state ()
{
  int tty = open ("/dev/tty", O_RDONLY);
  if (tty != -1)
    {
#ifdef NEW_TTY_DRIVER
      ioctl (tty, TIOCGETP, &shell_tty_info);
      ioctl (tty, TIOCGETC, &shell_tchars);
      ioctl (tty, TIOCGLTC, &shell_ltchars);
#else
      ioctl (tty, TCGETA, &shell_tty_info);
#endif /* NEW_TTY_DRIVER */
      close (tty);
    }
}

/* Make the current tty use the state in shell_tty_info. */
set_tty_state ()
{
  int tty = open ("/dev/tty", O_RDONLY);
  if (tty != -1)
    {
#ifdef NEW_TTY_DRIVER
      ioctl (tty, TIOCSETN, &shell_tty_info);
      ioctl (tty, TIOCSETC, &shell_tchars);
      ioctl (tty, TIOCSLTC, &shell_ltchars);
#else
      ioctl (tty, TCSETAW, &shell_tty_info);
#endif /* NEW_TTY_DRIVER */
      close (tty);
    }
}

/* Given an index into the jobs array JOB, return the pid of the last process
   in that job's pipeline.  This is the one whose exit status counts. */
int
lastproc (job)
     int job;
{
  int oldmask = sigblock (sigmask (SIGCHLD));
  register PROCESS *p;

  p = jobs[job]->pipe;
  while (p->next != jobs[job]->pipe)
    p = p->next;

  sigsetmask (oldmask);
  return (p->pid);
}

/* Wait for a particular child of the shell to finish executing.
   This low-level function prints an error message if PID is not
   a child of this shell.  It returns -1 if it fails, or 0 if not. */
int
wait_for_single_pid (pid)
     int pid;
{
  register PROCESS *child;

  child = find_pipeline (pid);

  if (!child)
    {
      report_error ("wait: pid %d is not a child of this shell", pid);
      return (-1);
    }

  return (wait_for (pid));
}

/* Wait for all of the backgrounds of this shell to finish. */
wait_for_background_pids ()
{
  while (1)
    {
      register int i, count = 0;
      int oldmask = sigblock (sigmask (SIGCHLD));

      for (i = 0; i < job_slots; i++)
	if (jobs[i] && (JOBSTATE (i) == JRUNNING) && !(jobs[i]->foreground))
	  {
	    count++;
	    break;
	  }

      if (!count)
	{
	  sigsetmask (oldmask);
	  break;
	}

      for (i = 0; i < job_slots; i++)
	if (jobs[i] && (JOBSTATE (i) == JRUNNING) && !jobs[i]->foreground)
	  {
	    int pid = jobs[i]->pgrp;
	    sigsetmask (oldmask);
	    QUIT;
	    wait_for_single_pid (pid);
	    break;
	  }
    }
}

/* Wait for pid (one of our children) to terminate. */
int
wait_for (pid)
     int pid;
{
  int oldmask, job, termination_state;
  register PROCESS *child;
  extern char *sys_siglist[];
  extern int interactive;

  oldmask = sigblock (sigmask (SIGCHLD));

  /* If we say wait_for (), then we have a record of this child somewhere.
     If this child and all of its peers are not running, then don't
     sigpause (), since there is no need to. */
 wait_loop:

  /* If the shell is running interactively, then let the user C-c out. */
  if (interactive)
    QUIT;

  child = find_pipeline (pid);

  if (!child)
    {
      give_terminal_to (shell_pgrp);
      programming_error ("wait_for: No record of pid %d", pid);
    }

  /* If this child is part of a job, then we are really waiting for the
     job to finish.  Otherwise, we are waiting for the child to finish. */

  job = find_job (pid);

  if (job != NO_JOB)
    {
      register int job_state = 0, any_stopped = 0;
      register PROCESS *p = jobs[job]->pipe;

      do
	{
	  job_state |= p->running;
	  if (!p->running)
	    any_stopped |= WIFSTOPPED (p->status);
	  p = p->next;
	}
      while (p != jobs[job]->pipe);

      if (job_state == 0)
	{
	  if (any_stopped)
	    jobs[job]->state = JSTOPPED;
	  else
	    jobs[job]->state = JDEAD;
	}
    }

  if (child->running ||
      ((job != NO_JOB) && (JOBSTATE (job) == JRUNNING)))
    {
      sigpause ((long)0);
      goto wait_loop;
    }

  /* The exit state of the command is either the termination state of the
     child, or the termination state of the job.  If a job, the status
     of the last child in the pipeline is the significant one. */

  if (job != NO_JOB)
    {
      register PROCESS *p = jobs[job]->pipe;

      while (p->next != jobs[job]->pipe)
	p = p->next;
      termination_state = p->status.w_retcode;
    }
  else
    termination_state = child->status.w_retcode;

  if (job == NO_JOB || jobs[job]->job_control)
    give_terminal_to (shell_pgrp);

  /* If the command did not exit cleanly, or the job is just
     being stopped, then reset the tty state back to what it
     was before this command. */
  if ((child->status.w_termsig != 0 || (WIFSTOPPED (child->status))))
    set_tty_state ();
  else
    get_tty_state ();

  if (job != NO_JOB)
    notify_and_cleanup ();

 wait_exit:
  sigsetmask (oldmask);
  return (termination_state);
}

/* Wait for the last process in the pipeline for JOB. */
int
wait_for_job (job)
     int job;
{
  int pid = lastproc (job);
  return (wait_for (pid));
}

/* Print info about dead jobs, and then delete them from the list
   of known jobs. */
notify_and_cleanup ()
{
  notify_of_job_status ();
  cleanup_dead_jobs ();
}

/* Return the next closest (chronologically) job to JOB which is in
   STATE.  STATE can be JSTOPPED, JRUNNING.  NO_JOB is returned if
   there is no next recent job. */
static int
most_recent_job_in_state (job, state)
     int job;
     JOB_STATE state;
{
  register int i;
  int oldmask = sigblock (sigmask (SIGCHLD));

  for (i = job - 1; i >= 0; i--)
    {
      if (jobs[i])
	{
	  if (JOBSTATE (i) == state)
	    {
	      /* Found it! */
	      sigsetmask (oldmask);
	      return (i);
	    }
	}
    }
  sigsetmask (oldmask);
  return (NO_JOB);
}

/* Return the newest *stopped* job older than JOB, or NO_JOB if not
   found. */
static int
last_stopped_job (job)
     int job;
{
  return (most_recent_job_in_state (job, JSTOPPED));
}

/* Return the newest *running* job older than JOB, or NO_JOB if not
   found. */
static int
last_running_job (job)
     int job;
{
  return (most_recent_job_in_state (job, JRUNNING));
}

/* Make JOB be the current job, and make previous be useful. */
set_current_job (job)
     int job;
{
  int candidate = NO_JOB;

  if (current_job != job)
    {
      previous_job = current_job;
      current_job = job;
    }

  /* First choice for previous_job is the old current_job. */
  if (previous_job != current_job &&
      previous_job != NO_JOB &&
      JOBSTATE (previous_job) == JSTOPPED)
    return;

  /* Second choice:  Newest stopped job that is older than
     the current job. */
  if (JOBSTATE (current_job) == JSTOPPED)
    {
      candidate = last_stopped_job (current_job);

      if (candidate != NO_JOB)
	{
	  previous_job = candidate;
	  return;
	}
    }

  if (JOBSTATE (current_job) == JRUNNING)
    candidate = last_running_job (current_job);
  else
    candidate = last_running_job (job_slots);

  if (candidate != NO_JOB)
    {
      previous_job = candidate;
      return;
    }

  /* There is only a single job, and it is both `+' and `-'. */
  previous_job = current_job;
}

/* Make current_job be something useful, if it isn't already. */
reset_current ()
{
  int candidate = NO_JOB;

  if (current_job != NO_JOB &&
      job_slots && jobs[current_job] &&
      JOBSTATE (current_job) == JSTOPPED)
    {
      candidate = current_job;
    }
  else
    {
      /* First choice:  the previous job! */
      if (previous_job != NO_JOB && jobs[previous_job] &&
	  JOBSTATE (previous_job) == JSTOPPED)
	candidate = previous_job;

      /* Second choice: the most recently stopped job. */
      candidate = last_stopped_job (job_slots);

      if (candidate == NO_JOB)
	{
	  /* Third choice: the newest running job. */
	  candidate = last_running_job (job_slots);
	}
    }

  /* If we found a job to use, then use it.  Otherwise, there
     are no jobs period. */
  if (candidate != NO_JOB)
    set_current_job (candidate);
  else
    current_job = previous_job = NO_JOB;
}

/* Start a job.  FOREGROUND if non-zero says to do that.  Otherwise,
   start the job in the background.  JOB is a zero-based index into
   JOBS.  Returns zero if it is unable to start a job. */
int
start_job (job, foreground)
     int job, foreground;
{
  int oldmask = sigblock (sigmask (SIGCHLD));
  int already_running = (JOBSTATE (job) == JRUNNING);
  register PROCESS *p;

  if (!foreground && already_running)
    {
      extern char *this_command_name;

      report_error ("%s: bg background job?", this_command_name);
      return (0);
    }

  /* You don't know about the state of this job.  Do you? */
  jobs[job]->notified = 0;

  if (foreground)
    {
      set_current_job (job);
      jobs[job]->foreground = 1;
    }

  /* Tell the outside world what we're doing. */
  p = jobs[job]->pipe;

  do
    {
      fprintf (stderr, "%s%s",
	       p->command, p->next != jobs[job]->pipe? " | " : "");
      p = p->next;
    }
  while (p != jobs[job]->pipe);

  if (!foreground)
    fprintf (stderr, " &");
      
  if (strcmp (get_string_value ("PWD"), jobs[job]->wd) != 0)
    fprintf (stderr, "  (wd: %s)", polite_directory_format (jobs[job]->wd));

  fprintf (stderr, "\n");
  
  /* Run the job. */

  if (!already_running)
    {
      /* Each member of the pipeline is now running. */
      p = jobs[job]->pipe;

      do
	{
	  if (WIFSTOPPED (p->status))
	    p->running = 1;
	  p = p->next;
	}
      while (p != jobs[job]->pipe);

    /* This means that the job is running. */
    JOBSTATE (job) = JRUNNING;
  }

  /* Give the terminal to this job. */
  if (foreground)
    {
      if (jobs[job]->job_control)
	give_terminal_to (jobs[job]->pgrp);
    }
  else
    jobs[job]->foreground = 0;

  /* If the job is already running, then don't bother jump-starting it. */
  if (!already_running)
    {
      jobs[job]->notified = 1;
      killpg (jobs[job]->pgrp, SIGCONT);
    }

  sigsetmask (oldmask);

  if (foreground)
    {
      int pid = lastproc (job);

      return (!wait_for (pid));
    }
  else
    reset_current ();

  return (1);
}

/* Give PID SIGNAL.  This determines what job the pid belongs to (if any).
   If PID does belong to a job, and the job is stopped, then CONTinue the
   job after giving it SIGNAL.  Returns -1 on failure.  If GROUP is non-null,
   then kill the process group associated with PID. */
int
kill_pid (pid, signal, group)
     int pid, signal, group;
{
  int old_mask = sigblock (sigmask (SIGCHLD));
  register PROCESS *p = find_pipeline (pid);
  int job = find_job (pid);
  int result = EXECUTION_SUCCESS;

  if (group)
    {
      if (job != NO_JOB)
	{
	  jobs[job]->notified = 0;

	  /* Kill process in backquotes or one started with job control? */
	  if (jobs[job]->pgrp == shell_pgrp)
	    {
	      p = jobs[job]->pipe;

	      do
		{
		  if (!p->running && (signal == SIGTERM || signal == SIGHUP))
		    kill (pid, SIGCONT);
		  kill (pid, signal);
		  p = p->next;
		} while (p != jobs[job]->pipe);
	    }
	  else
	    {
	      if (p && (JOBSTATE (job) == JSTOPPED) &&
		  (signal == SIGTERM || signal == SIGHUP))
		killpg (jobs[job]->pgrp, SIGCONT);
	      result = killpg (jobs[job]->pgrp, signal);
	    }
	}
      else
	{
	  result = killpg (pid, signal);
	}
    }
  else
    {
      result = kill (pid, signal);
    }
  sigsetmask (old_mask);
  return (result);
}

/* Flush_child () flushes at least one of the children that we are waiting for.
   It gets run when we have gotten a SIGCHLD signal, and stops when there
   aren't any children terminating any more. */
sighandler
flush_child (sig, code)
     int sig, code;
{
  union wait status;
#ifndef hpux
  struct rusage rusage;
#endif
  PROCESS *child;
  int pid, call_set_current = 0, last_stopped_job = NO_JOB;

  do
    {
#ifdef hpux
      pid = wait3 (&status, (WNOHANG | WUNTRACED), (int *)0);
#else
      pid = wait3 (&status, (WNOHANG | WUNTRACED), &rusage);
#endif /* hpux */

      if (pid > 0)
	{
#ifdef hpux
	  /* Reinstall the signal handler.  That's what HPUX makes us do. */
	  signal (SIGCHLD, flush_child);
#endif

#ifdef NEVER /* Claim is made that times_builtin can handle it. */
	  /* Keep track of total time used. */
	  if (! WIFSTOPPED (status))
	    add_times (&rusage);
#endif /* NEVER */

	  /* Locate our PROCESS for this pid. */
	  child = find_pipeline (pid);

	  /* It is not an error to have a child terminate that we did
	     not have a record of.  This child could have been part of
	     a pipeline in backquote substitution. */
	  if (child)
	    {
	      int job = find_job (pid);

	      while (child->pid != pid)
		child = child->next;

	      /* Remember status, and fact that process is not running. */
	      child->status = status;
	      child->running = 0;

	      if (job != NO_JOB)
		{
		  int job_state = 0;
		  int any_stopped = 0;

		  child = jobs[job]->pipe;
		  jobs[job]->notified = 0;
		  
		  /* If all children are not running, but any of them is
		     stopped, then the job is stopped, not dead. */
		  do
		    {
		      job_state |= child->running;
		      if (!child->running)
			any_stopped |= (WIFSTOPPED (child->status));
		      child = child->next;
		    }
		  while (child != jobs[job]->pipe);

		  if (job_state == 0)
		    {
		      if (any_stopped)
			{
			  jobs[job]->state = JSTOPPED;
			  jobs[job]->foreground = 0;
			  call_set_current++;
			  last_stopped_job = job;
			}
		      else
			{
			  jobs[job]->state = JDEAD;

			  if (job == last_stopped_job)
			    last_stopped_job = NO_JOB;

			  /* If this job was not started with job control,
			     then the shell has already seen the SIGINT, since
			     the process groups are the same.  In that case,
			     don't send the SIGINT to the shell; it will
			     surprise people to have a stray interrupt
			     arriving some time after they killed the job. */

			  if (jobs[job]->foreground &&
			      jobs[job]->job_control &&
			      jobs[job]->pipe->status.w_termsig == SIGINT)
			    kill (getpid (), SIGINT);
			}
		    }
		}
	    }
	}
    }
  while (pid > 0);

  /* If a job was running and became stopped, then set the current
     job.  Otherwise, don't change a thing. */
  if (call_set_current)
    if (last_stopped_job != NO_JOB)
      set_current_job (last_stopped_job);
    else
      reset_current ();

  /* We have successfully recorded the useful information about this process
     that has just changed state.  If we notify asynchronously, and the job
     that this process belongs to is no longer running, then notify the user
     of that fact now. */
  if (asynchronous_notification)
    notify_of_job_status ();
}

/* Function to call when you want to notify people of changes
   in job status.  This prints out all jobs which are pending
   notification to stderr, and marks those printed as already
   notified, thus making them candidates for cleanup. */
notify_of_job_status ()
{
  extern char *sys_siglist[];
  register int job, termsig;
  char *dir = (char *)get_string_value ("PWD");
  int oldmask = sigblock (sigmask (SIGCHLD) | sigmask (SIGTTOU));

  for (job = 0; job < job_slots; job++)
    {
      if (jobs[job] && jobs[job]->notified == 0)
	{
	  termsig = jobs[job]->pipe->status.w_termsig;

	  switch (JOBSTATE (job))
	    {
	      /* Print info on jobs that are running in the background,
		 and on foreground jobs that were killed by anything
		 except SIGINT. */

	    case JDEAD:

	      if (jobs[job]->foreground)
		{
		  if (termsig && termsig != WSTOPPED && termsig != SIGINT)
		    {
		      fprintf (stderr, "%s", sys_siglist[termsig]);

		      if (jobs[job]->pipe->status.w_coredump)
			fprintf (stderr, " (core dumped)");

		      fprintf (stderr, "\n");
		    }
		}
	      else
		{
		  pretty_print_job (job, 0, stderr);
		  if (dir && strcmp (dir, jobs[job]->wd) != 0)
		    fprintf (stderr,
			     "(wd now: %s)\n", polite_directory_format (dir));
		}
	      jobs[job]->notified = 1;
	      break;

	    case JSTOPPED:
	      fprintf (stderr, "\n");
	      pretty_print_job (job, 0, stderr);
	      if (dir && (strcmp (dir, jobs[job]->wd) != 0))
		fprintf (stderr,
			 "(wd now: %s)\n", polite_directory_format (dir));
	      jobs[job]->notified = 1;
	      break;

	    case JRUNNING:
	    case JMIXED:
	      break;

	    default:
	      programming_error ("notify_of_job_status");
	    }
	}
    }
  sigsetmask (oldmask);
}

#ifndef hpux
add_times (rused)
     struct rusage *rused;
{
  total_systime.tv_usec += rused->ru_stime.tv_usec;
  total_systime.tv_sec += rused->ru_stime.tv_sec;

  if (total_systime.tv_usec > 1000000)
    {
      total_systime.tv_sec++;
      total_systime.tv_usec -= 1000000;
    }

  total_usertime.tv_usec += rused->ru_utime.tv_usec;
  total_usertime.tv_sec += rused->ru_utime.tv_sec;

  if (total_usertime.tv_usec > 1000000)
    {
      total_usertime.tv_sec++;
      total_usertime.tv_usec -= 1000000;
    }

  if (total_systime.tv_sec)
    {
      system_minutes_used = (total_systime.tv_sec / 60);
      system_seconds_used = (total_systime.tv_sec % 60);
    }

  if (total_usertime.tv_sec)
    {
      user_minutes_used = (total_usertime.tv_sec / 60);
      user_seconds_used = (total_usertime.tv_sec % 60);
    }
}
#endif /* hpux */

#ifdef hpux
getdtablesize ()
{
  return (NOFILE);
}
#endif /* hpux */

/* Initialize the job control mechanism, and set up the tty stuff. */
initialize_jobs ()
{
  extern int interactive;

  shell_pgrp = getpgrp (0);

  /* We can only have job control if we are interactive?
     I guess that makes sense. */

  if (!job_control || !interactive)
    {
      job_control = 0;
    }
  else
    {
      char *err_string = "get";

      /* Make sure that we are using the new line discipline. */
      int ldisc;

      /* Get our controlling terminal.  If job_control is set, or
	 interactive is set, then this is an interactive shell no
	 matter what opening /dev/tty returns.  (It sometimes says
	 the wrong thing.) */
      shell_tty = open ("/dev/tty", O_RDWR, 0666);
      if (shell_tty < 0)
	shell_tty = dup (fileno (stdin));

      /* Find the highest unused file descriptor we can. */
      {
	int ignore, nds = getdtablesize ();
	
	while (--nds > 3)
	  {
	    if (fcntl (nds, F_GETFD, &ignore) == -1)
	      break;
	  }

	if (shell_tty != nds && (dup2 (shell_tty, nds) != -1))
	  {
	    if (shell_tty != fileno (stdin))
	      close (shell_tty);
	    shell_tty = nds;
	  }
      }

      while ((terminal_pgrp = tcgetpgrp (shell_tty)) != -1)
	{
	  if (shell_pgrp != terminal_pgrp)
	    {
	      SigHandler *old_ttin = (SigHandler *)signal (SIGTTIN, SIG_DFL);
	      kill (0, SIGTTIN);
	      signal (SIGTTIN, old_ttin);
	      continue;
	    }
	  break;
	}

#ifdef NEW_TTY_DRIVER
      if (ioctl (shell_tty, TIOCGETD, &ldisc) < 0)
	goto bad_ioctl;

      if (ldisc != NTTYDISC)
	{
	  ldisc = NTTYDISC;
	  err_string = "set";
	  if (ioctl (shell_tty, TIOCSETD, &ldisc) < 0)
	    {
	    bad_ioctl:
	      fprintf (stderr, "initialize_jobs: %s line disc: ", err_string);
	      job_control = 0;
	      file_error ("jobs.c");
	    }
	}
#endif /* NEW_TTY_DRIVER */

      original_pgrp = shell_pgrp;
      shell_pgrp = getpid ();
      give_terminal_to (shell_pgrp);
      setpgid (0, shell_pgrp);

#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

      if (shell_tty != fileno (stdin))
	fcntl (shell_tty, F_SETFD, FD_CLOEXEC);

      job_control = 1;
    }

  signal (SIGCHLD, flush_child);
  /* We don't call set_job_control here, because change_flag_char ()
     does that for us. */
  /* set_job_control (job_control); */
  change_flag_char ('m', job_control ? '-' : '+');

  get_tty_state ();
}

/* Allow or disallow job control to take place. */
set_job_control (arg)
     int arg;
{
  job_control = arg;
}

static SigHandler *old_tstp, *old_ttou, *old_ttin;
static SigHandler *old_cont = (SigHandler *)SIG_DFL;

/* Setup this shell to handle C-C, etc. */
initialize_job_signals ()
{
  extern int login_shell;
  sighandler sigint_sighandler ();

  signal (SIGINT, sigint_sighandler);
  signal (SIGQUIT, SIG_IGN);

  if (login_shell)
    {
      signal (SIGTSTP, SIG_IGN);
      signal (SIGTTOU, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
    }
  else
    {
      static sighandler stop_signal_handler ();

      old_tstp = (SigHandler *)signal (SIGTSTP, stop_signal_handler);
      old_ttou = (SigHandler *)signal (SIGTTOU, stop_signal_handler);
      old_ttin = (SigHandler *)signal (SIGTTIN, stop_signal_handler);
    }
}

/* Here we handle CONT signals. */
static sighandler
cont_signal_handler (sig, code)
     int sig, code;
{
  initialize_job_signals ();
  signal (SIGCONT, old_cont);
  kill (getpid (), SIGCONT);
}

/* Here we handle stop signals while we are running not as a login shell. */
static sighandler
stop_signal_handler (sig, code)
     int sig, code;
{
  signal (SIGTSTP, old_tstp);
  signal (SIGTTOU, old_ttou);
  signal (SIGTTIN, old_ttin);

  old_cont = (SigHandler *)signal (SIGCONT, cont_signal_handler);

  give_terminal_to (shell_pgrp);

  kill (getpid (), sig);
}

/* Give the terminal to PGRP.  */
give_terminal_to (pgrp)
     int pgrp;
{
  int oldmask;

  if (job_control)
    {
      oldmask = sigblock (sigmask (SIGTTOU) |
			  sigmask (SIGTTIN) |
			  sigmask (SIGTSTP) |
			  sigmask (SIGCHLD));

      terminal_pgrp = pgrp;
      tcsetpgrp (shell_tty, terminal_pgrp);
      sigsetmask (oldmask);
    }
}

/* Clear out any jobs in the job array.  This is intended to be used by
   children of the shell, who should not have any job structures as baggage
   when they start executing (forking subshells for parenthesized execution
   and functions with pipes are the two that spring to mind). */

delete_all_jobs ()
{
  if (job_slots)
    {
      register int i;

      for (i = 0; i < job_slots; i++)
	if (jobs[i] != (JOB *) NULL)
	  delete_job (i);

      free ((char *)jobs);
      job_slots = 0;
    }
}

/* Turn off all traces of job control.  This is run by children of the shell
   which are going to do shellsy things, like wait (), etc. */
without_job_control ()
{
  stop_making_children ();
  start_pipeline ();
  delete_all_jobs ();
  set_job_control (0);
}
#endif  /* JOB_CONTROL */

