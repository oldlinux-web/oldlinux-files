/* trap.c -- Not the trap command, but useful functions
   for manipulating those objects.  The trap command is
   in builtins.c */

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

#include "trap.h"
#include "shell.h"

/* The list of things to do originally, before we started trapping. */
SigHandler *original_signals[NSIG];

/* For each signal, a slot for a string, which is a command to be
   executed when that signal is recieved.  The slot can also contain
   DEFAULT_SIG, which means do whatever you were going to do before
   you were so rudely interrupted, or IGNORE_SIG, which says ignore
   this signal. */
char *trap_list[NSIG];

/* A translation list so we can be polite to our users. */
char *signal_names[NSIG];

static int signal_names_initialized = 0;


initialize_traps ()
{
  register int i;

  if (!signal_names_initialized)
    {
      for (i = 0; i < NSIG; i++)
	signal_names[i] = (char *)NULL;

      /* `signal' 0 is what we do on exit. */
      signal_names[0] = "ON_EXIT";

#if defined (SIGHUP)		/* hangup */
      signal_names[SIGHUP] = "SIGHUP";
#endif

#if defined (SIGINT)		/* interrupt */
      signal_names[SIGINT] = "SIGINT";
#endif

#if defined (SIGQUIT)		/* quit */
      signal_names[SIGQUIT] = "SIGQUIT";
#endif

#if defined (SIGILL)		/* illegal instruction (not reset when caught) */
      signal_names[SIGILL] = "SIGILL";
#endif

#if defined (SIGTRAP)		/* trace trap (not reset when caught) */
      signal_names[SIGTRAP] = "SIGTRAP";
#endif

#if defined (SIGABRT)		/*  */
      signal_names[SIGABRT] = "SIGABRT";
#endif

#if defined (SIGIOT)		/* IOT instruction */
      signal_names[SIGIOT] = "SIGIOT";
#endif

#if defined (SIGEMT)		/* EMT instruction */
      signal_names[SIGEMT] = "SIGEMT";
#endif

#if defined (SIGFPE)		/* floating point exception */
      signal_names[SIGFPE] = "SIGFPE";
#endif

#if defined (SIGKILL)		/* kill (cannot be caught or ignored) */
      signal_names[SIGKILL] = "SIGKILL";
#endif

#if defined (SIGBUS)		/* bus error */
      signal_names[SIGBUS] = "SIGBUS";
#endif

#if defined (SIGSEGV)		/* segmentation violation */
      signal_names[SIGSEGV] = "SIGSEGV";
#endif

#if defined (SIGSYS)		/* bad argument to system call */
      signal_names[SIGSYS] = "SIGSYS";
#endif

#if defined (SIGPIPE)		/* write on a pipe with no one to read it */
      signal_names[SIGPIPE] = "SIGPIPE";
#endif

#if defined (SIGALRM)		/* alarm clock */
      signal_names[SIGALRM] = "SIGALRM";
#endif

#if defined (SIGTERM)		/* software termination signal from kill */
      signal_names[SIGTERM] = "SIGTERM";
#endif

#if defined (SIGCLD)		/* Like SIGCHLD.  */
      signal_names[SIGCLD] = "SIGCLD";
#endif

#if defined (SIGPWR)		/* Magic thing for some machines. */
      signal_names[SIGPWR] = "SIGPWR";
#endif

#if defined (SIGPOLL)		/* For keyboard input?  */
      signal_names[SIGPOLL] = "SIGPOLL";
#endif

#if defined (SIGURG)		/* urgent condition on IO channel */
      signal_names[SIGURG] = "SIGURG";
#endif

#if defined (SIGSTOP)		/* sendable stop signal not from tty */
      signal_names[SIGSTOP] = "SIGSTOP";
#endif

#if defined (SIGTSTP)		/* stop signal from tty */
      signal_names[SIGTSTP] = "SIGTSTP";
#endif

#if defined (SIGCONT)		/* continue a stopped process */
      signal_names[SIGCONT] = "SIGCONT";
#endif

#if defined (SIGCHLD)		/* to parent on child stop or exit */
      signal_names[SIGCHLD] = "SIGCHLD";
#endif

#if defined (SIGTTIN)		/* to readers pgrp upon background tty read */
      signal_names[SIGTTIN] = "SIGTTIN";
#endif

#if defined (SIGTTOU)		/* like TTIN for output if (tp->t_local&LTOSTOP) */
      signal_names[SIGTTOU] = "SIGTTOU";
#endif

#if defined (SIGIO)		/* input/output possible signal */
      signal_names[SIGIO] = "SIGIO";
#endif

#if defined (SIGXCPU)		/* exceeded CPU time limit */
      signal_names[SIGXCPU] = "SIGXCPU";
#endif

#if defined (SIGXFSZ)		/* exceeded file size limit */
      signal_names[SIGXFSZ] = "SIGXFSZ";
#endif

#if defined (SIGVTALRM)		/* virtual time alarm */
      signal_names[SIGVTALRM] = "SIGVTALRM";
#endif

#if defined (SIGPROF)		/* profiling time alarm */
      signal_names[SIGPROF] = "SIGPROF";
#endif

#if defined (SIGWINCH)		/* window changed */
      signal_names[SIGWINCH] = "SIGWINCH";
#endif

#if defined (SIGLOST)		/* resource lost (eg, record-lock lost) */
      signal_names[SIGLOST] = "SIGLOST";
#endif

#if defined (SIGUSR1)		/* user defined signal 1 */
      signal_names[SIGUSR1] = "SIGUSR1";
#endif

#if defined (SIGUSR2)		/* user defined signal 2 */
      signal_names[SIGUSR2] = "SIGUSR2";
#endif

      for (i = 0; i < NSIG; i++)
	if (signal_names[i] == (char *)NULL)
	  {
	    signal_names[i] = (char *)xmalloc (10 + strlen ("SIGJUNK"));
	    sprintf (signal_names[i], "SIGJUNK(%d)", i);
	  }
    }


  for (i = 0; i < NSIG; i++)
    {
      trap_list[i] = (char *)DEFAULT_SIG;
      original_signals[i] = (SigHandler *)signal (i, SIG_DFL);
      signal (i, original_signals[i]);
    }
}

/* Return the print name of this signal. */
char *
signal_name (signal)
     int signal;
{
  if (signal > NSIG)
     return ("bad signal number");
  else return (signal_names[signal]);
}

/* Turn a string into a signal number, or a number into
   a signal number.  If STRING was "2", "SIGINT", or "INT",
   then (int)2 would be returned. */
int
decode_signal (string)
     char *string;
{
  int sig;

  if (sscanf (string, "%d", &sig) == 1) {
    if (sig < NSIG && sig >= 0)
      return (sig);
    else
      return (NO_SIG);
  }
      
  for (sig = 0; sig < NSIG; sig++)
     if ((strcmp (string, signal_names[sig]) == 0) ||
	 (strcmp (string, &(signal_names[sig])[3]) == 0))
       return (sig);
  return (NO_SIG);
}

sighandler
trap_handler (sig)
     int sig;
{
  if ((sig >= NSIG) || (((int)trap_list[sig]) == 0))
     programming_error ("trap_handler: Bad signal %d", sig);
  else
     parse_and_execute (savestring (trap_list[sig]), "trap");
}

/* Set SIG to call STRING as a command. */
void
set_signal (sig, string)
     int sig;
     char *string;
{
  void change_signal ();

  if (sig)
    signal (sig, SIG_IGN);

  change_signal (sig, savestring (string));

  if (sig)
    signal (sig, trap_handler);
}

/* If SIG has a string assigned to it, get rid of it.  Then give it
   VALUE. */
void
change_signal (sig, value)
     int sig;
     char *value;
{
  if (((int)trap_list[sig]) > 0)
     free (trap_list[sig]);
  trap_list[sig] = value;
}

/* Restore the default action for SIG; i.e., the action the shell
   would have taken before you used the trap command. */
void
restore_default_signal (sig)
     int sig;
{
  signal (sig, original_signals[sig]);
  change_signal (sig, (char *)DEFAULT_SIG);
}

/* Make this signal be ignored. */
void
ignore_signal (sig)
     int sig;
{
  signal (sig, SIG_IGN);
  change_signal (sig, (char *)IGNORE_SIG);
}

/* Handle the calling of "trap 0".  The only sticky situation is when
   the command to be executed includes an "exit". */
void
run_exit_trap ()
{
  if (((int)trap_list[0]) > 0)
    {
      char *trap_command = savestring (trap_list[0]);

      change_signal (0, (char *)NULL);
      parse_and_execute (trap_command, "trap");
    }
}
      
