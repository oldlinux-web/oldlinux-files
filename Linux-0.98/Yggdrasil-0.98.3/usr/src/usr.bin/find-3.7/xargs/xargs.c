/* xargs -- build and execute command lines from standard input
   Copyright (C) 1989, 1990, 1991 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Author:
	Mike Rendell			Department of Computer Science
	michael@garfield.mun.edu	Memorial University of Newfoundland
	..!uunet!garfield!michael	St. John's, Nfld., Canada
	(709) 737-4550			A1C 5S7

Usage: xargs [-0ptx] [-e[eof-str]] [-i[replace-str]] [-l[max-lines]]
       [-n max-args] [-s max-chars] [-P max-procs] [--null] [--eof[=eof-str]
       [--replace[=replace-str] [--max-lines[=max-lines] [--interactive]
       [--max-chars max-chars] [--verbose] [--exit] [--max-procs max-procs]
       [command [initial-arguments]]

   Read arguments from stdin, delimited by blanks (which can be quoted
   or escaped) or newlines, and execute the COMMAND (default is echo)
   one or more times with any INITIAL-ARGUMENTS plus arguments read
   from stdin.  Blank lines on stdin are ignored.

   Options:
   --null
   -0			Input filenames are terminated by a '\0' instead
			of by whitespace and no quoting characters
			are special (every character is taken literally).
			However, any eof string is still recognized.
			Useful with GNU find -print0 when filenames might
			contain newlines or quote marks or backslashes.

   --eof[=eof-str]
   -e[eof-str]		Set the eof string to EOF-STR.  If EOF-STR is
			omitted, there is no eof string.  If this option
			is not given, the eof string defaults to "_".

   --replace[=replace-str]
   -i[replace-str]	Replace occurences of REPLACE-STR in initial
			args with argument.  If REPLACE-STR is omitted, it
			defaults to "{}" (like for `find -exec').  Implies -x.
			Execute the command once per line of input.

   --max-lines[=max-lines]
   -l[max-lines]	Use at most MAX-LINES nonblank input lines per command
			line; MAX-LINES defaults to 1 if omitted.  Implies -x.
			Trailing blanks cause an input line to be logically
			continued on the next input line.

   --max-args max-args
   -n max-args		Use at most MAX-ARGS arguments per command line.  Fewer
			than MAX-ARGS arguments will be used if the size
			(see -s) is exceeded, unless the -x option is given,
			in which case xargs will exit.

   --interactive
   -p			Ask user about each command line before running it.
			Implies -t.

   --no-run-if-empty
   -r			If the standard input does not contain any nonblanks,
			do not run the command.  Normally, the command is run
			once even if there is no input.

   --max-chars max-chars
   -s max-chars		Use at most MAX-CHARS characters per command line,
			including command and initial arguments.
			Default is as large as possible.

   --verbose
   -t			Print command line string on stderr before executing.

   --exit
   -x			Exit if size (see -s) is exceeded.

   --max-procs max-procs
   -P max-procs		Run up to MAX-PROCS processes at a time (default is 1).
			If MAX-PROCS is 0 it will run as many as possible.
			Should use -n with -P; otherwise chances are that
			only one exec will be done. */

#define _GNU_SOURCE
#include <ctype.h>
#ifndef isblank
#define isblank(c) ((c) == ' ' || (c) == '\t')
#endif
#define ISSPACE(c) (isblank (c) || (c) == '\n' || (c) == '\r' || (c) == '\f' || (c) == '\v')
#include <stdio.h>
#include <errno.h>
#include <getopt.h>

/* COMPAT:  SYSV version defaults size (and has a max value of) to 470. We
   try to make it as large as possible. */
#if defined(USG) || defined(STDC_HEADERS)
#include <string.h>

#if !defined(STDC_HEADERS)
#include <memory.h>
#endif

#define bcopy(source, dest, count) (memcpy((dest), (source), (count)))
#else				/* USG || STDC_HEADERS */
#include <strings.h>
#endif				/* USG || STDC_HEADERS */

char *strstr ();

#ifndef _POSIX_SOURCE
#include <sys/param.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _POSIX_VERSION
#ifdef ARG_MAX
#undef ARG_MAX
#endif /* ARG_MAX */
#define ARG_MAX sysconf (_SC_ARG_MAX)
#endif /* _POSIX_VERSION */

#ifndef ARG_MAX
#define ARG_MAX NCARGS
#endif

#include "wait.h"

/* States for read_line. */
#define NORM 0
#define SPACE 1
#define QUOTE 2
#define BACKSLASH 3

/* A linked list of processes that we have forked off. */
struct pid_list
{
  int pl_pid;
  struct pid_list *pl_next;
};

#ifdef STDC_HEADERS
#include <stdlib.h>
#else
char *malloc ();
void exit ();
void free ();
long strtol ();

extern int errno;
#endif

char *strdup ();

char *xmalloc ();
int print_args ();
int read_line ();
int read_string ();
long env_size ();
long parse_num ();
void add_proc ();
void do_exec ();
void do_insert ();
void error ();
void push_arg ();
void usage ();
void wait_for_proc ();

extern char **environ;

/* The name this program was run with. */
char *program_name;

/* If nonzero, then instead of putting the args from stdin
   at the end of the command argument list, they are each stuck into the
   initial args, replacing each occurrence of the `replace_pat' in the
   initial args. */
char *replace_pat = 0;

/* If nonzero, when this string is read on stdin it is treated as end of file.
   I don't like this - it should default to NULL. */
char *eof_str = "_";

/* If nonzero, the maximum number of nonblank lines from stdin to use
   per command line. */
long lines_per_exec = 0;

/* The maximum number of arguments to use per command line.
   The default is specified by POSIX. */
long nargs_per_exec = 255;

/* The maximum number of characters that can be used per command line. */
long arg_max;

/* If nonzero, print each command on stderr before executing it. */
int print_command = 0;

/* If nonzero, exit if lines_per_exec or nargs_per_exec is exceeded. */
int exit_if_size_exceeded = 0;

/* If nonzero, query the user before executing each command, and only
   execute the command if the user responds affirmatively. */
int query_before_executing = 0;

/* If nonzero, run the command at least once, even if there is no input. */
int always_run_command = 1;

/* If nonzero, the maximum number of child processes that can be running
   at once. */
int proc_max = 1;

/* Total number of child processes that have been executed. */
int processes_executed = 0;

/* List of child processes currently executing. */
struct pid_list *pid_list = 0;

/* The number of elements in `pid_list'. */
int procs_executing = 0;

/* Default program to run. */
char def_prog[] = "/bin/echo";

/* Buffer for reading arguments from stdin. */
char *linebuf;

/* Temporary copy of each arg with the replace pattern replaced by the
   real arg. */
char *insertbuf;

/* Line number in stdin since the last command was executed. */
int lineno = 0;

/* The list of args being built. */
char **cmd_argv = 0;

/* Number of elements allocated for `cmd_argv'. */
int cmd_nargv = 0;

/* Number of valid elements in `cmd_argv'. */
int cmd_argc = 0;

/* Number of chars in `cmd_argv'. */
int cmd_ncargs = 0;

/* Number of initial arguments given on the command line. */
int cmd_initial_argc = 0;

/* Number of chars in the initial args. */
int cmd_initial_ncargs = 0;

/* Nonzero when building up initial arguments in `cmd_argv'. */
int static_args = 1;

/* Nonzero (exit status) if any child process exited with a status of 1-125. */
int child_error = 0;

/* For reading user response to prompting from /dev/tty. */
FILE *tty_stream;

struct option longopts[] =
{
  {"null", 0, NULL, '0'},
  {"eof", 2, NULL, 'e'},
  {"replace", 2, NULL, 'i'},
  {"max-lines", 2, NULL, 'l'},
  {"max-args", 1, NULL, 'n'},
  {"interactive", 0, NULL, 'p'},
  {"no-run-if-empty", 0, NULL, 'r'},
  {"max-chars", 1, NULL, 's'},
  {"verbose", 0, NULL, 't'},
  {"exit", 0, NULL, 'x'},
  {"max-procs", 1, NULL, 'P'},
  {NULL, 0, NULL, 0}
};

void
main (argc, argv)
     int argc;
     char **argv;
{
  int optc;
  long orig_arg_max;
  char *dummy;
  int (*read_args) () = read_line;

  program_name = argv[0];

  /* Sanity check for systems with huge ARG_MAX defines (e.g., Suns which
     have it at 1 meg).  Things will work fine with a large ARG_MAX but it
     will probably hurt the system more than it needs to; an array of this
     size is allocated.  */
  arg_max = ARG_MAX;
  if (arg_max > 20 * 1024)
    arg_max = 20 * 1024;

  /* Adjust arg_max to take the size of the environment into account. */
  arg_max -= env_size (environ);
  if (arg_max <= 0)
    error (1, 0, "environment too large for exec");
  orig_arg_max = arg_max;

  while ((optc = getopt_long (argc, argv, "+0e::i::l::n:prs:txP:",
			      longopts, (int *) 0)) != EOF)
    {
      switch (optc)
	{
	case '0':
	  read_args = read_string;
	  break;

	case 'e':
	  if (optarg)
	    eof_str = optarg;
	  else
	    eof_str = 0;
	  break;

	case 'i':
	  if (optarg)
	    replace_pat = optarg;
	  else
	    replace_pat = "{}";
	  /* -i excludes -n -l. */
	  nargs_per_exec = 0;
	  lines_per_exec = 0;
	  break;

	case 'l':
	  if (optarg)
	    lines_per_exec = parse_num (optarg, 'l', 1L, -1L);
	  else
	    lines_per_exec = 1;
	  /* -l excludes -i -n. */
	  nargs_per_exec = 0;
	  replace_pat = 0;
	  break;
	  
	case 'n':
	  nargs_per_exec = parse_num (optarg, 'n', 1L, -1L);
	  /* -n excludes -i -l. */
	  lines_per_exec = 0;
	  replace_pat = 0;
	  break;
	  
	case 's':
	  arg_max = parse_num (optarg, 's', 1L, orig_arg_max);
	  break;
	  
	case 't':
	  print_command++;
	  break;
	  
	case 'x':
	  exit_if_size_exceeded++;
	  break;
	  
	case 'p':
	  query_before_executing++;
	  break;
	  
	case 'r':
	  always_run_command = 0;
	  break;

	case 'P':
	  proc_max = parse_num (optarg, 'P', 0L, -1L);
	  break;
	  
	default:
	  usage ();
	}
    }

  if (query_before_executing)
    {
      print_command++;
      tty_stream = fopen ("/dev/tty", "r");
      if (!tty_stream)
	error (1, errno, "/dev/tty");
    }
  if (replace_pat || lines_per_exec)
    exit_if_size_exceeded++;

  linebuf = xmalloc ((unsigned) arg_max + 1);

  if (optind == argc)
    {
      optind = 0;
      argc = 1;
      dummy = def_prog;
      argv = &dummy;
    }

  if (!replace_pat)
    {
      while (optind < argc)
	push_arg (argv[optind++]);
      static_args = 0;

      cmd_initial_argc = cmd_argc;
      cmd_initial_ncargs = cmd_ncargs;

      while ((*read_args) () == 0)
	if (lines_per_exec && lineno >= lines_per_exec)
	  {
	    do_exec ();
	    lineno = 0;
	  }

      /* SYSV xargs seems to do at least one exec, even if the
         input is empty. */
      if (cmd_argc != cmd_initial_argc
	  || (always_run_command && processes_executed == 0))
	do_exec ();
    }
  else
    {
      int ac;
      char **av;

      insertbuf = xmalloc ((unsigned) arg_max + 1);
      while ((*read_args) () == 0)
	{
	  push_arg (argv[optind]);	/* Don't do insert on command name. */
	  for (ac = argc - optind, av = argv + optind; --ac > 0;)
	    do_insert (*++av, linebuf);
	  do_exec ();
	}
    }
  wait_for_proc (1);
  exit (child_error);
}

/* Read a line of arguments from stdin and add them to the list of
   arguments to pass to the command.  Ignore blank lines and initial blanks.
   Single and double quotes and backslashes quote metacharacters and blanks
   as they do in the shell.
   Return -1 if eof (either physical or logical) is reached, 0 otherwise. */

int
read_line ()
{
  static int eof = 0;
  /* Start out in mode SPACE to always strip leading spaces (even with -i). */
  int state = SPACE;		/* The type of character we last read. */
  int lastc;			/* The previous value of c. */
  int quotc;			/* The last quote character read. */
  int c = EOF;
  int first = 1;		/* Nonzero if reading first arg on the line. */
  char *p = linebuf;
  char *endbuf = linebuf + arg_max - 1;

  if (eof)
    return -1;
  while (1)
    {
      lastc = c;
      c = getc (stdin);
      if (c == EOF)
	{
	  /* COMPAT: SYSV seems to ignore stuff on a line that
	     ends without a \n; we don't.  */
	  eof = 1;
	  if (p == linebuf)
	    return -1;
	  *p = '\0';
	  if (eof_str && first && !strcmp (eof_str, linebuf))
	    return -1;
	  if (!replace_pat)
	    push_arg (linebuf);
	  return 0;
	}
      switch (state)
	{
	case SPACE:
	  if (ISSPACE (c))
	    continue;
	  state = NORM;
	  /* aaahhhh.... */

	case NORM:
	  if (c == '\n')
	    {
	      if (!isblank (lastc))
		lineno++;	/* For -l. */
	      if (p == linebuf)
		{
		  state = SPACE;
		  continue;
		}
	      *p = '\0';
	      if (eof_str && !strcmp (eof_str, linebuf))
		{
		  eof = 1;
		  return first ? -1 : 0;
		}
	      if (!replace_pat)
		push_arg (linebuf);
	      return 0;
	    }
	  if (!replace_pat && ISSPACE (c))
	    {
	      *p = '\0';
	      if (eof_str && !strcmp (eof_str, linebuf))
		{
		  eof = 1;
		  return first ? -1 : 0;
		}
	      p = linebuf;
	      push_arg (p);
	      state = SPACE;
	      first = 0;
	      continue;
	    }
	  switch (c)
	    {
	    case '\\':
	      state = BACKSLASH;
	      continue;
	      
	    case '\'':
	    case '"':
	      state = QUOTE;
	      quotc = c;
	      continue;
	    }
	  break;

	case QUOTE:
	  if (c == '\n')
	    error (1, 0, "%s quote did not end before line did",
		   quotc == '"' ? "double" : "single");
	  if (c == quotc)
	    {
	      state = NORM;
	      continue;
	    }
	  break;

	case BACKSLASH:
	  state = NORM;
	  break;
	}
      if (p >= endbuf)
	error (1, 0, "argument line too long");
      *p++ = c;
    }
}

/* Read a null-terminated string from stdin and add it to the list of
   arguments to pass to the command.
   Return -1 if eof (either physical or logical) is reached, 0 otherwise. */

int
read_string ()
{
  static int eof = 0;
  int c;
  char *p = linebuf;
  char *endbuf = linebuf + arg_max - 1;

  if (eof)
    return -1;
  while (1)
    {
      c = getc (stdin);
      if (c == EOF)
	{
	  eof = 1;
	  if (p == linebuf)
	    return -1;
	  *p = '\0';
	  if (eof_str && !strcmp (eof_str, linebuf))
	    return -1;
	  if (!replace_pat)
	    push_arg (linebuf);
	  return 0;
	}
      if (c == '\0')
	{
	  lineno++;	/* For -l. */
	  *p = '\0';
	  if (eof_str && !strcmp (eof_str, linebuf))
	    {
	      eof = 1;
	      return -1;
	    }
	  if (!replace_pat)
	    push_arg (linebuf);
	  return 0;
	}
      if (p >= endbuf)
	error (1, 0, "argument line too long");
      *p++ = c;
    }
}

/* Print the arguments of the command to execute.
   If ASK is nonzero, prompt the user for a response, and
   if the user responds affirmatively, return 1.
   Otherwise, return 0. */

int
print_args (ask)
     int ask;
{
  int i;

  for (i = 0; i < cmd_argc - 1; i++)
    fprintf (stderr, "%s ", cmd_argv[i]);
  if (ask)
    {
      int c, savec;

      fputs ("?...", stderr);
      fflush (stderr);
      c = savec = getc (tty_stream);
      while (c != EOF && c != '\n')
	c = getc (tty_stream);
      if (savec == 'y' || savec == 'Y')
	return 1;
    }
  else
    putc ('\n', stderr);

  return 0;
}

/* Return the value of the number represented in STR.
   OPTION is the command line option to which STR is the argument.
   If the value does not fall within the boundaries MIN and MAX,
   Print an error message mentioning OPTION and exit. */

long
parse_num (str, option, min, max)
     char *str;
     int option;
     long min;
     long max;
{
  char *eptr;
  long val;

  val = strtol (str, &eptr, 10);
  if (eptr == str || *eptr)
    {
      fprintf (stderr, "%s: invalid number for -%c option\n",
	       program_name, option);
      usage ();
    }
  else if (val < min)
    {
      fprintf (stderr, "%s: value for -%c option must be >= %d\n",
	       program_name, option, min);
      usage ();
    }
  else if (max >= 0 && val > max)
    {
      fprintf (stderr, "%s: value for -%c option must be < %ld\n",
	       program_name, option, max);
      usage ();
    }
  return val;
}

/* Add ARG to the end of the list of arguments `cmd_argv' to pass
   to the command.
   If this brings the list up to its maximum size, execute the command. */

void
push_arg (arg)
     char *arg;
{
  if (arg)
    {
      int len = strlen (arg) + 1;

      if (cmd_ncargs + len > arg_max)
	{
	  if (static_args || cmd_argc == cmd_initial_argc)
	    error (1, 0, "can not fit single argument within argument list size limit");
	  if (replace_pat
	      || exit_if_size_exceeded && (lines_per_exec || nargs_per_exec))
	    error (1, 0, "argument list too long");
	  do_exec ();
	}
      if (!static_args && nargs_per_exec &&
	  cmd_argc - cmd_initial_argc == nargs_per_exec)
	do_exec ();
      cmd_ncargs += len;
    }
  if (cmd_argc >= cmd_nargv)
    {
      char **new_argv;

      if (!cmd_argv)
	cmd_nargv = 50;
      new_argv = (char **)
	    xmalloc ((unsigned) (sizeof (char *) * cmd_nargv * 2));
      cmd_nargv *= 2;
      if (cmd_argv)
	{
	  bcopy ((char *) cmd_argv, (char *) new_argv,
		 sizeof (char *) * cmd_argc);
	  free ((char *) cmd_argv);
	}
      cmd_argv = new_argv;
    }
  if (!arg)
    cmd_argv[cmd_argc++] = 0;
  else
    {
      cmd_argv[cmd_argc] = strdup (arg);
      if (!cmd_argv[cmd_argc++])
	error (1, 0, "virtual memory exhausted");
    }
}

/* Execute the command that has been built in `cmd_argv'.  This may involve
   waiting for processes that were previously executed.
   Also free the memory used by the command's arguments.

   COMPAT: since we don't wait for the process to complete before returning,
   it is possible that we won't notice a command failing until the next
   time do_exec is called.  This is only noticeable when xargs is run
   interactivly.  It can be fixed by adding
   if (proc_max == 1)
     wait_for_proc (0)
   after the add_proc call. */

void
do_exec ()
{
  int child;

  push_arg ((char *) 0);	/* Null terminate the arg list. */
  if (!query_before_executing || print_args (1))
    {
      if (proc_max && procs_executing >= proc_max)
	wait_for_proc (0);
      if (!query_before_executing && print_command)
	print_args (0);
      /* If we run out of processes, wait for a child to return and
         try again.  */
      while ((child = fork ()) < 0 && errno == EAGAIN && procs_executing)
	wait_for_proc (0);
      switch (child)
	{
	case -1:
	  error (1, errno, "cannot fork");

	case 0:			/* Child. */
	  execvp (cmd_argv[0], cmd_argv);
	  error (0, errno, "%s", cmd_argv[0]);
	  _exit (errno == ENOENT ? 127 : 126);
	}
      add_proc (child);
      processes_executed++;
    }

  --cmd_argc;			/* For the trailing null. */
  while (--cmd_argc > cmd_initial_argc)
    free (cmd_argv[cmd_argc]);
  cmd_ncargs = cmd_initial_ncargs;
}

/* If ALL is nonzero, wait for all child processes to finish;
   otherwise, wait for one child process to finish.
   Remove the processes that finish from the list of executing processes. */

void
wait_for_proc (all)
     int all;
{
  struct pid_list **pl_prev;
  struct pid_list *pl;
  int pid;
  int status;

  while (procs_executing)
    {
      do
	{
	  pid = wait (&status);
	  if (pid < 0)
	    error (1, 0, "error waiting for child process");

	  /* Find the entry in `pid_list' for the child process that exited. */
	  pl_prev = &pid_list;
	  pl = pid_list;
	  while (pl != 0 && pl->pl_pid != pid)
	    {
	      pl_prev = &pl->pl_next;
	      pl = pl->pl_next;
	    }
	}
      while (pl == 0);		/* A child died that we didn't start? */

      /* Remove the child from the list. */
      *pl_prev = pl->pl_next;
      free ((char *) pl);
      procs_executing--;

      if (WEXITSTATUS (status) == 126 || WEXITSTATUS (status) == 127)
	exit (status);		/* Can't find or run the command. */
      if (WEXITSTATUS (status) == 255)
	error (124, 0, "%s: exited with status 255; aborting", cmd_argv[0]);
      if (WIFSTOPPED (status))
	error (125, 0, "%s: stopped by signal %d", cmd_argv[0], WSTOPSIG (status));
      if (WIFSIGNALED (status))
	error (125, 0, "%s: terminated by signal %d", cmd_argv[0], WTERMSIG (status));
      if (WEXITSTATUS (status) != 0)
	child_error = 123;

      if (!all)
	break;
    }
}

/* Add the process with id PID to the list of processes that have
   been executed. */

void
add_proc (pid)
     int pid;
{
  struct pid_list *pl;

  pl = (struct pid_list *) xmalloc (sizeof (struct pid_list));
  pl->pl_pid = pid;
  pl->pl_next = pid_list;
  pid_list = pl;
  procs_executing++;
}

/* Replace all instances of `replace_pat' in ARG with LINE, and add the
   resulting string to the list of arguments for the command to execute.

   COMPAT: insertions on the SYSV version are limited to 255 chars per line,
   and a max of 5 occurences of replace_pat in the initial-arguments.  These
   restristrions do not exist here.  */

void
do_insert (arg, line)
     char *arg;
     char *line;
{
  char *p = insertbuf;
  char *s;
  int cnt = arg_max - 1;	/* Bytes left on the command line. */
  int len;			/* Length of ARG before `replace_pat'. */
  int ilen = strlen (replace_pat);
  int llen = strlen (line);

  do
    {
      s = strstr (arg, replace_pat);
      if (s)
	len = s - arg;
      else
	len = strlen (arg);
      cnt -= len;
      if (cnt <= 0)
	break;
      strncpy (p, arg, len);
      p += len;
      arg += len;
      if (s)
	{
	  cnt -= llen;
	  if (cnt <= 0)
	    break;
	  strcpy (p, line);
	  arg += ilen;
	  p += llen;
	}
    }
  while (*arg);
  if (*arg)
    error (1, 0, "command too long");
  *p = '\0';
  push_arg (insertbuf);
}

/* Return how much of ARG_MAX is used by the environment. */

long
env_size (envp)
     char **envp;
{
  long len = 0;

  while (*envp)
    len += strlen (*envp++) + 1;

  return len;
}

void
usage ()
{
  fprintf (stderr, "\
Usage: %s [-0prtx] [-e[eof-str]] [-i[replace-str]] [-l[max-lines]]\n\
       [-n max-args] [-s max-chars] [-P max-procs] [--null] [--eof[=eof-str]\n\
       [--replace[=replace-str] [--max-lines[=max-lines] [--interactive]\n\
       [--max-chars max-chars] [--verbose] [--exit] [--max-procs max-procs]\n\
       [--no-run-if-empty] [command [initial-arguments]]\n",
	   program_name);
  exit (1);
}
