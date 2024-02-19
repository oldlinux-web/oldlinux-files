/* shell.c -- GNU's idea of the POSIX shell specification.
   Hopefully, this shell will contain significant enhancements.

   This file is part of Bash, the Bourne Again SHell.
   Bash is free software; no one can prevent you from reading the source
   code, or giving it to someone else.
   This file is copyrighted under the GNU General Public License, which
   can be found in the file called COPYING.

   Copyright (C) 1989 Free Software Foundation, Inc.

   This file is part of GNU Bash.

   Bash is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY.  No author or distributor accepts
   responsibility to anyone for the consequences of using it or for
   whether it serves any particular purpose or works at all, unless he
   says so in writing.  Refer to the GNU Emacs General Public License
   for full details.

   Everyone is granted permission to copy, modify and redistribute
   Bash, but only under the conditions described in the GNU General Public
   License.   A copy of this license is supposed to have been given to you
   along with GNU Emacs so you can know your rights and responsibilities.
   It should be in a file named COPYING.

   Among other things, the copyright notice and this notice must be
   preserved on all copies.

  Modification history:

  Sunday, January 10th, 1988.
  Initial author: Brian Fox

*/

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#ifndef SONY
#include <fcntl.h>
#endif
#include <sys/file.h>
#include <sys/stat.h>
#include <pwd.h>

#if defined (HAVE_VPRINTF)
#include <varargs.h>
#endif

#include "shell.h"
#include "flags.h"

#ifdef SYSV
struct passwd *getpwuid();
#endif

extern char *dist_version;
extern int build_version;

extern int yydebug;

/* Non-zero means that this shell has already been run; i.e. you should
   call shell_reinitialize () if you need to start afresh. */
int shell_initialized = 0;

/* The current maintainer of the shell.  You change this in the
   Makefile. */
#ifndef MAINTAINER
#define MAINTAINER "deliberately-anonymous"
#endif
char *the_current_maintainer = MAINTAINER;

#ifndef PPROMPT
#define PPROMPT "bash\\$ "
#endif
char *primary_prompt = PPROMPT;

#ifndef SPROMPT
#define SPROMPT "bash> "
#endif
char *secondary_prompt = SPROMPT;

COMMAND *global_command = (COMMAND *)NULL;

/* Non-zero after SIGINT. */
int interrupt_state = 0;

/* The current user's name. */
char *current_user_name = (char *)NULL;

/* The current host's name. */
char *current_host_name = (char *)NULL;

/* Non-zero means that this shell is a login shell.
   Specifically:
   0 = not login shell.
   1 = login shell from getty (or equivalent fake out)
  -1 = login shell from "-login" flag.
  -2 = both from getty, and from flag.
 */
int login_shell = 0;

/* Non-zero means this shell is running interactively. */
int interactive = 0;

/* Non-zero means to remember lines typed to the shell on the history
   list.  This is different than the user-controlled behaviour; this
   becomes zero when we read lines from a file, for example. */
int remember_on_history = 1;

/* Non-zero means this shell is restricted. */
int restricted = 0;

/* Special debugging helper. */
int debugging_login_shell = 0;

/* The environment that the shell passes to other commands. */
char **shell_environment;

/* Non-zero when we are executing a top-level command. */
int executing = 0;

/* The number of commands executed so far. */
int current_command_number = 1;

/* The environment at the top-level REP loop.  We use this in the case of
   error return. */
jmp_buf top_level, catch;

/* Non-zero is the recursion depth for commands. */
int indirection_level = 0;

/* The number of times BASH has been executed.  This is set
   by initialize_variables () in variables.c. */
int shell_level = 0;

/* The name of this shell, as taken from argv[0]. */
char *shell_name;

/* The name of the .(shell)rc file. */
char *bashrc_file = "~/.bashrc";

/* Non-zero means to act more like the Bourne shell on startup. */
int act_like_sh = 0;

/* Values for the long-winded argument names. */
int debugging = 0;		/* Do debugging things. */
int no_rc = 0;			/* Don't execute ~/.bashrc */
int no_profile = 0;		/* Don't execute .profile */
int do_version = 0;		/* Display interesting version info. */
int quiet = 0;			/* Be quiet when starting up. */
int make_login_shell = 0;	/* Make this shell be a `-bash' shell. */
int no_line_editing = 0;	/* Don't do fancy line editing. */
int no_brace_expansion = 0;	/* Non-zero means no foo{a,b} -> fooa fooa. */

/* Some long-winded argument names.  These are obviously new. */
#define Int 1
#define Charp 2
struct {
  char *name;
  int *value;
  int type;
} long_args[] = {
  { "debug", &debugging, Int },
  { "norc", &no_rc, Int },
  { "noprofile", &no_profile, Int },
  { "rcfile", (int *)&bashrc_file, Charp},
  { "version", &do_version, Int},
  { "quiet", &quiet, Int},
  { "login", &make_login_shell, Int},
  { "nolineediting", &no_line_editing, Int},
  { "nobraceexpansion", &no_brace_expansion, Int},
  { (char *)NULL, (int *)0x0, 0 }
};

main (argc, argv, env)
     int argc;
     char **argv, **env;
{
  int i, arg_index = 1;
  extern int yydebug;
  FILE *default_input = stdin;
  char *local_pending_command = (char *)NULL;
  extern int last_command_exit_value;
  int locally_skip_execution = 0, top_level_arg_index;
  extern char *base_pathname ();
#ifdef JOB_CONTROL
  extern int job_control;
#endif

  /* Wait forever if we are debugging a login shell. */
  while (debugging_login_shell);
      
  /* If this shell has already been run, then reinitialize it to a
     vanilla state. */
  if (shell_initialized)
    {
      shell_reinitialize ();
      if (setjmp (top_level))
	exit (2);
    }

  /* Here's a hack.  If the name of this shell is "sh", then don't do
     any startup files; just try to be more like /bin/sh. */
  {
    char *tshell_name = base_pathname (argv[0]);

    if (*tshell_name == '-')
      tshell_name++;

    if (strcmp (tshell_name, "sh") == 0)
      act_like_sh++;
  }

  yydebug = 0;

  shell_environment = env;
  shell_name = argv[0];
  if (*shell_name == '-')
    {
      shell_name++;
      login_shell++;
    }

#ifdef JOB_CONTROL
  if (act_like_sh)
    job_control = 0;
#endif

  dollar_vars[0] = savestring (argv[0]);

  /* Parse argument flags from the input line. */

  /* Find full word arguments first. */
  while ((arg_index != argc) && *(argv[arg_index]) == '-')
    {
      for (i = 0; long_args[i].name; i++)
	{
	  if (strcmp (&(argv[arg_index][1]), long_args[i].name) == 0)
	    {
	      if (long_args[i].type == Int)
		*(long_args[i].value) = 1;
	      else
		{
		  if (!argv[++arg_index])
		    {
		      report_error ("%s: Flag `%s' expected an argument",
				    shell_name, long_args[i].name);
		      exit (1);
		    }
		  else
		    *long_args[i].value = (int)argv[arg_index];
		}
	      goto next_arg;
	    }
	}
      break;			/* No such argument.  Maybe flag arg. */
    next_arg:
      arg_index++;
    }

  /* If user supplied the "-login" flag, then set and invert LOGIN_SHELL. */
  if (make_login_shell)
    login_shell = -++login_shell;

  /* All done with full word args; do standard shell arg parsing.*/
  while (arg_index != argc && argv[arg_index] &&
	 (*(argv[arg_index]) == '-' || (*argv[arg_index] == '+')))
    {
      /* There are flag arguments, so parse them. */
      int arg_character;
      int on_or_off = (*argv[arg_index]);
      int  i = 1;

      while (arg_character = (argv[arg_index])[i++])
	{
	  switch (arg_character)
	    {
	    case 'c':
	      /* The next arg is a command to execute, and the following args
		 are $1 .. $n respectively. */
	      local_pending_command = argv[++arg_index];
	      if (!local_pending_command)
		{
		  report_error ("`%cc' requires an argument", on_or_off);
		  exit (1);
		}

	      arg_index++;
	      goto after_flags;
	      break;

	    default:
	      if (change_flag_char (arg_character, on_or_off) == FLAG_ERROR)
		{
		  report_error ("%c%c: bad option", on_or_off, arg_character);
		  exit (1);
		}

	    }
	}
      arg_index++;
    }

 after_flags:

  /* First, let the outside world know about our interactive status. */
  if (forced_interactive ||
      (!local_pending_command &&
       arg_index == argc &&
       isatty (fileno (stdin)) &&
       isatty (fileno (stdout))))
    interactive = 1;
  else
    {
      interactive = 0;
#ifdef JOB_CONTROL
      job_control = 0;
#endif
    }

  /* From here on in, the shell must be a normal functioning shell.
     Variables from the environment are expected to be set, etc. */
  shell_initialize ();

  if (interactive)
    {
      char *emacs = (char *)getenv ("EMACS");
      if (emacs && (strcmp (emacs, "t") == 0))
	no_line_editing = 1;
    }

  top_level_arg_index = arg_index;

  if (!quiet && do_version)
    show_shell_version ();

  /* Give this shell a place to longjmp to before executing the
     startup files.  This allows users to press C-c to abort the
     lengthy startup. */
  if (setjmp (top_level))
    {
      if (!interactive)
	exit (2);
      else
	locally_skip_execution++;
    }

  arg_index = top_level_arg_index;

  /* Execute the start-up scripts. */

  if (!interactive)
    {
      makunbound ("PS1");
      makunbound ("PS2");
    }

  if (!locally_skip_execution)
    {
      if (login_shell)
	maybe_execute_file ("/etc/profile");

      if (login_shell && !no_profile)
	{
	  /* If we are doing the .bash_profile, then don't do the .bashrc. */
	  no_rc++;

	  if (act_like_sh)
	    maybe_execute_file ("~/.profile");
	  else
	    {
	      if (maybe_execute_file ("~/.bash_profile") == 0)
		if (maybe_execute_file ("~/.bash_login") == 0)
		  maybe_execute_file ("~/.profile");
	    }

	  /* I turn on the restrictions afterwards because it is explictly
	     stated in the POSIX spec that PATH cannot be set in a restricted
	     shell, except in .profile. */
	  if (*++(argv[0]) == 'r')
	    {
	      set_var_read_only ("PATH");
	      restricted++;
	    }
	}

      /* Execute ~/.bashrc for all shells except direct script shells,
	 and shells that are doing -c "command". */

      if (arg_index == argc && !no_rc && !act_like_sh &&
	  (!local_pending_command || shell_level < 2))
	maybe_execute_file (bashrc_file);

      /* Try a TMB suggestion.  If running a script, then execute the
	 file mentioned in the ENV variable. */
      if (!interactive)
	{
	  char *env_file = (char *)getenv ("ENV");
	  if (env_file && *env_file)
	    maybe_execute_file (env_file);
	}

      if (local_pending_command)
	{
	  with_input_from_string (local_pending_command, "-c");
	  goto read_and_execute;
	}
    }
  /* Do the things that should be done only for interactive shells. */
  if (interactive)
    {
      /* Set up for checking for presence of mail. */
#ifdef SYSV
      /* Under SYSV, we can only tell if you have mail if the
	 modification date has changed.  So remember the current
	 modification dates. */
      remember_mail_dates ();
#else
      /* Under 4.x, you have mail if there is something in your inbox.
	 I set the remembered mail dates to 1900.  */
      reset_mail_files ();
#endif /* SYSV */

      /* If this was a login shell, then assume that /bin/login has already
	 taken care of informing the user that they have new mail.  Otherwise,
	 we want to check right away. */
      if (login_shell == 1)
	{
#ifndef SYSV
	  remember_mail_dates ();
#endif  /* SYSV */
	}

      reset_mail_timer ();

      /* Initialize the interactive history stuff. */

      if (!shell_initialized)
	{
	  char *hf = get_string_value ("HISTFILE");
	if (hf)
	  read_history (hf);
	}
    }

  /* Get possible input filename. */
 get_input_filename:
  if (arg_index != argc)
    {
      int fd;

      free (dollar_vars[0]);
      dollar_vars[0] = savestring (argv[arg_index]);

      fd = open (argv[arg_index++], O_RDONLY);
      if (fd < 0)
	{
	  file_error (dollar_vars[0]);
	  exit (1);
	}
      else
	{
	  default_input = fdopen (fd, "r");
	  if (!default_input)
	    {
	      file_error (dollar_vars[0]);
	      exit (1);
	    }
	}

      if (!interactive || (!isatty (fd)))
	{
	  extern int history_expansion;
	  history_expansion = interactive = 0;
#ifdef JOB_CONTROL
	  set_job_control (0);
#endif
	}
      else
	{
	  dup2 (fd, 0);
	  close (fd);
	  fclose (default_input);
	}
    }

  /* Bind remaining args to $1 ... $n */
  {
    WORD_LIST *args = (WORD_LIST *)NULL;
    while (arg_index != argc)
      args = make_word_list (make_word (argv[arg_index++]), args);
    args = (WORD_LIST *)reverse_list (args);
    remember_args (args, 1);
    dispose_words (args);
  }

  if (interactive && !no_line_editing)
    with_input_from_stdin ();
  else
    with_input_from_stream (default_input, dollar_vars[0]);

 read_and_execute:

  shell_initialized = 1;

  /* Read commands until exit condition. */
  reader_loop ();

  /* Do trap[0] if defined. */
  run_exit_trap ();
  
  /* Save the history of executed commands. */
  if (interactive)
    {
      char *hf = get_string_value ("HISTFILE");
      if (hf)
	write_history (hf);
    }

  /* Always return the exit status of the last command to our parent. */
  exit (last_command_exit_value);
}

/* Try to execute the contents of FNAME.  If FNAME doesn't exist,
   that is not an error, but other kinds of errors are.  Returns
   -1 in the case of an error, 0 in the case that the file was not
   found, and 1 if the file was found and executed. */
maybe_execute_file (fname)
     char *fname;
{
  extern int errno;
  char *tilde_expand ();
  char *filename = tilde_expand (fname);

  struct stat file_info;

  if (stat (filename, &file_info) == -1)
    {
      if (errno != ENOENT)
	{
	file_error_and_exit:
	  file_error (filename);
	  free (filename);
	  return (-1);
	}
      free (filename);
      return (0);
    }
  else
    {
      int tt, tresult;
      char *string = (char *)xmalloc (1 + file_info.st_size);
      int fd = open (filename, O_RDONLY);

      if (fd < 0)
	{
	hack_file_error:
	  free (string);
	  goto file_error_and_exit;
	}

      tresult = read (fd, string, file_info.st_size);
      tt = errno;
      close (fd);
      errno = tt;
      if (tresult != file_info.st_size)
	goto hack_file_error;
      string[file_info.st_size] = '\0';

      parse_and_execute (string, filename);
      free (filename);

      return (1);
    }
}

reader_loop ()
{
  extern int indirection_level;
  int our_indirection_level;
  COMMAND *current_command = (COMMAND *)NULL;

  our_indirection_level = ++indirection_level;

  while (!EOF_Reached)
    {
      sighandler sigint_sighandler ();
      int code = setjmp (top_level);

      signal (SIGINT, sigint_sighandler);

      if (code != NOT_JUMPED)
	{
	  indirection_level = our_indirection_level;

	  switch (code)
	    {
	      /* Some kind of throw to top_level has occured. */
	    case FORCE_EOF:
	    case EXITPROG:
	      current_command = (COMMAND *)NULL;
	      EOF_Reached = EOF;
	      goto exec_done;

	    case DISCARD:
	      /* Obstack free command elements, etc. */
	      break;

	    default:
	      programming_error ("Bad jump %d", code);
	    }
	}

      executing = 0;
      dispose_used_env_vars ();

      if (read_command () == 0)
	{
	  if (global_command) {
	    current_command = global_command;

	    current_command_number++;

	    /* POSIX spec: "-n  The shell reads commands but does
	       not execute them; this can be used to check for shell
	       script syntax errors.  The shell ignores the -n option
	       for interactive shells. " */

	    if (interactive || !read_but_dont_execute)
	      {
		executing = 1;
		execute_command (current_command);
	      }

	  exec_done:
	    if (current_command)
	      dispose_command (current_command);
	    QUIT;
	  }
	}
      else
	{
	  /* Parse error, maybe discard rest of stream if not interactive. */
	  if (!interactive)
	    EOF_Reached = EOF;
	}
      if (just_one_command)
	EOF_Reached = EOF;
    }
  indirection_level--;
}

/* Return a string denoting what our indirection level is. */
static char indirection_string[100];

char *
indirection_level_string ()
{
  register int i;
  char *get_string_value (), *ps4 = get_string_value ("PS4");

  if (!ps4)
    ps4 = "+";

  for (i = 0; i < 100 && i < indirection_level; i++)
    indirection_string[i] = *ps4;

  indirection_string[i] = '\0';
  return (indirection_string);
}

read_command ()
{
  extern char *ps1_prompt, **prompt_string_pointer;

  prompt_string_pointer = &ps1_prompt;
  global_command = (COMMAND *)NULL;
  return (yyparse ());
}

/* Do whatever is necessary to initialize the shell.
   Put new initializations in here. */
shell_initialize ()
{
  /* Line buffer output for stderr. 
     If your machine doesn't have either of setlinebuf or setvbuf,
     you can just comment out the buffering commands, and the shell
     will still work.  It will take more cycles, though. */
#if defined (HAVE_SETLINEBUF)
  setlinebuf (stderr);
  setlinebuf (stdout);
#else
# if defined (_IOLBF)
  setvbuf (stderr, (char *)NULL, _IOLBF, BUFSIZ);
  setvbuf (stdout, (char *)NULL, _IOLBF, BUFSIZ);
# endif
#endif /* HAVE_SETLINEBUF */

  /* The initialization of the basic underlying signal handlers must
     happen before we initialize_traps ().  */
  initialize_signals ();
  initialize_traps ();

  /* Initialize current_user_name and current_host_name. */
  {
    struct passwd *entry = getpwuid (getuid ());
    char hostname[256];

    if (gethostname (hostname, 255) < 0)
      current_host_name = "??host??";
    else
      current_host_name = savestring (hostname);

    if (entry)
      current_user_name = savestring (entry->pw_name);
    else
      current_user_name = savestring ("I have no name!");
  }

  initialize_shell_variables (shell_environment);
  initialize_filename_hashing ();
  initialize_jobs ();
}

/* Function called by main () when it appears that the shell has already
   had some initialization preformed.  This is supposed to reset the world
   back to a pristine state, as if we had been exec'ed. */
shell_reinitialize ()
{
  extern int line_number, last_command_exit_value;

  /* The default shell prompts. */
  primary_prompt = PPROMPT;
  secondary_prompt = SPROMPT;

  /* Things that get 1. */
  remember_on_history = current_command_number = 1;

  /* We have decided that the ~/.bashrc file should not be executed
     for the invocation of each shell script.  Perhaps some other file
     should.  */
  act_like_sh = 1;

  /* Things that get 0. */
  login_shell = make_login_shell = interactive = restricted = executing = 0;
  debugging = no_rc = no_profile = do_version = line_number = 0;
  last_command_exit_value = 0;

  /* Ensure that the default startup file is used. */
  bashrc_file = "~/.bashrc";

  /* Delete all shell variables, except for functions. */
  {
    register SHELL_VAR *list = variable_list;
    register SHELL_VAR *new_list = (SHELL_VAR *)NULL;
    register SHELL_VAR *temp;

    variable_list = (SHELL_VAR *)NULL;

    while (list)
      {
	if (function_p (list))
	  {
	    temp = list->next;
	    list->next = new_list;
	    new_list = list;
	    list = temp;
	  }
	else
	  {
	    temp = list;
	    list = list->next;
	    dispose_variable (temp);
	  }
      }
    variable_list = new_list;
  }

  /* Pretend the PATH variable has changed. */
  sv_path ("PATH");
}

initialize_signals ()
{
  initialize_terminating_signals ();
  initialize_job_signals ();
#ifdef INITIALIZE_SIGLIST
  initialize_siglist ();
#endif
}

/* The list of signals that would terminate the shell if not caught.
   We catch them, but just so that we can write the history file,
   and so forth. */
int terminating_signals[] = {
  SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGIOT,
#ifdef SIGDANGER
  SIGDANGER,
#endif
#ifdef SIGEMT
  SIGEMT,
#endif
  SIGFPE, SIGKILL, SIGBUS, SIGSEGV, SIGSYS, SIGPIPE, SIGALRM, SIGTERM,
#ifdef SIGXCPU
  SIGXCPU,
#endif
#ifdef SIGXFSZ
  SIGXFSZ,
#endif
#ifdef SIGVTALRM
  SIGVTALRM,
#endif
#ifdef SIGPROF
  SIGPROF,
#endif
#ifdef SIGLOST
  SIGLOST,
#endif
#ifdef SIGUSR1
  SIGUSR1, SIGUSR2
#endif
    };

#define TERMSIGS_LENGTH (sizeof (terminating_signals) / sizeof (int))

/* This function belongs here? */
sighandler
termination_unwind_protect (sig)
     int sig;
{
  save_history ();
  signal (sig, SIG_DFL);
  kill (getpid (), sig);
}

/* Initialize signals that will terminate the shell to do some
   unwind protection. */
initialize_terminating_signals ()
{
  register int i;

  for (i = 0; i < TERMSIGS_LENGTH; i++)
    signal (terminating_signals[i], termination_unwind_protect);

  /* And, some signals that are specifically ignored by the shell. */
  signal (SIGQUIT, SIG_IGN);

  if (login_shell)
    signal (SIGTERM, SIG_IGN);
}

/* What to do when we've been interrupted, and it is safe to handle it. */
sighandler
throw_to_top_level ()
{
  extern int last_command_exit_value, loop_level, continuing, breaking;
  extern int return_catch_flag;

  if (interrupt_state)
    interrupt_state --;

  if (interrupt_state)
    return;

#ifdef JOB_CONTROL
  {
    extern int shell_pgrp;
    give_terminal_to (shell_pgrp);
  }
#endif  /* JOB_CONTROL */

  run_unwind_protects ();
  loop_level = continuing = breaking = 0;
  return_catch_flag = 0;

  reset_parser ();
  if (interactive)
    {
      fflush (stdout);
      fprintf (stderr, "\n");
    }

  last_command_exit_value |= 128;

  if (interactive)
    longjmp (top_level, DISCARD);
  else
    longjmp (top_level, EXITPROG);
}

/* When non-zero, we throw_to_top_level (). */
int interrupt_immediately = 0;

/* What we really do when SIGINT occurs. */
sighandler
sigint_sighandler ()
{
  if (interrupt_immediately)
    {
      interrupt_immediately = 0;
      throw_to_top_level ();
    }
  if (!interrupt_state)
    interrupt_state++;
}
    
/* Write the existing history out to the history file. */
save_history ()
{
  void using_history ();

  using_history ();
  write_history (get_string_value ("HISTFILE"));
}

/* Make a bug report, even to the extent of mailing it.  Hope that it
   gets where it is supposed to go.  If not, maybe the user will send
   it back to me. */
#include <readline/history.h>
/* Number of commands to place in the bug report. */
#define LAST_INTERESTING_HISTORY_COUNT 6

#if defined (HAVE_VPRINTF)
make_bug_report (va_alist)
     va_dcl
#else
make_bug_report (reason, arg1, arg2)
     char *reason;
#endif /* HAVE_VPRINTF */
{
  extern char *current_host_name, *current_user_name, *the_current_maintainer;
  extern int interactive, login_shell;
  register int len = where_history ();
  register int i = len - LAST_INTERESTING_HISTORY_COUNT;
  FILE *stream, *popen ();
  HIST_ENTRY **list = history_list ();

#if defined (HAVE_VPRINTF)
  char *reason;
  va_list args;
#endif /* HAVE_VPRINTF */

  stream = popen ("/bin/rmail bash-maintainers@ai.mit.edu", "w");

  save_history ();
  if (i < 0) i = 0;

  if (stream)
    {
      fprintf (stream, "To: bash-maintainers@ai.mit.edu\n");
      fprintf (stream, "Subject: Bash-%s.%d bug-report: ",
	       dist_version, build_version);

#if defined (HAVE_VPRINTF)
      va_start (args);
      reason = va_arg (args, char *);
      vfprintf (stream, reason, args);
      va_end (args);
#else
      fprintf (stream, reason, arg1, arg2);
#endif /* HAVE_VPRINTF */

      fprintf (stream, "\n");

      /* Write the history into the mail file.  Maybe we can recreate
	 the bug? */
      fprintf (stream,
"This is a Bash bug report.  Bash maintainers should be getting this report.\n\
If this mail has bounced, for right now please send it to:\n\
\n\
	%s\n\
\n\
since he is the current maintainer of this version of the shell.\n\
\n\
This is %s (invoked as `%s'), version %s.%d, on host %s, used by %s.\n\
This shell is %sinteractive, and it is %sa login shell.\n\
\n\
The host is a %s running %s.\n\
\n\
The current environment is:\n",
	       the_current_maintainer,
	       get_string_value ("BASH"), full_pathname (dollar_vars[0]),
	       dist_version, build_version, current_host_name,
	       current_user_name, interactive ? "" : "not ",
	       login_shell ? "" : "not ", SYSTEM_NAME, OS_NAME);

      {
	SHELL_VAR *v = variable_list;

	while (v)
	  {
	    if (!invisible_p (v))
	      {
		if (function_p (v))
		  {
		    fprintf (stream, "function %s () {\n", v->name);
		    fprintf (stream, "%s\n}",
			     make_command_string (function_cell (v)));
		  }
		else
		  fprintf (stream, "%s=%s", v->name, value_cell (v));

		fprintf (stream, "\n");
	      }
	    v = v->next;
	  }
      }

      fprintf (stream, "\nAnd here are the last %d commands.\n\n",
	       LAST_INTERESTING_HISTORY_COUNT);

      for (; i < len; i++)
	fprintf (stream, "%s\n", list[i]->line);

      pclose (stream);
    } else {
      fprintf (stderr, "Can't mail bug report!\n");
    }
}

/* Give version information about this shell. */
show_shell_version ()
{
  extern char *shell_name;
  extern int version;

  printf ("GNU %s, version %s.%d\n", base_pathname (shell_name),
	  dist_version, build_version);
}
