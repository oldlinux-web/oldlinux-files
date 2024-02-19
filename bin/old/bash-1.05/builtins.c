/* builtins.c -- the built in shell commands. */

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
#include <sys/param.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include "shell.h"

#ifndef SYSV
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include "builtins.h"
#include "trap.h"
#include "flags.h"
#include <readline/history.h>

#ifdef JOB_CONTROL
#include "jobs.h"
#endif

extern int errno;		/* Not always in <errno.h>.  Bogusness. */

#ifndef sigmask
#define sigmask(x) (1 << ((x)-1))
#endif

#ifdef SYSV
#include <fcntl.h>
#include <sys/times.h>
#endif

#if defined (HAVE_VPRINTF)
#include <varargs.h>
#endif

/* Yecch!  Who cares about this gross concept in the first place? */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

/* The command name of the currently running function. */
extern char *this_command_name;

/* Non-zero means running an interactive shell. */
extern int interactive;

/* The list of shell builtins.  Each element is name, function, enabled-p,
   short-doc, long-doc.  The long-doc field should contain a set of indented
   lines.  The function takes a WORD_LIST *, where the first word in the list
   is the first arg to the command.  The list has already been word expanded.

   Procedures which need to look at every simple command (like enable_builtin),
   should tree-walk looking for (array[i].function == (Function *)NULL).  The
   list of executable builtins (in the shell sense) ends there.  Then comes
   the control structure commands, like `if' and `while'.  */

struct builtin shell_builtins[] = {

  { ":", colon_builtin, 1, ":",
      "	No effect; the command does nothing.  A zero exit code is returned" },

  { ".", period_builtin, 1, ". [filename]",
      "	Read and execute commands from FILENAME and return.\n\
	Pathnames in $PATH are used to find the directory containing FILENAME" },

  { "alias", alias_builtin, 1, "alias [ name[=value] ... ]",
      "	Alias with no arguments prints the list of aliases in the form\n\
	name=value on standard output.  An alias is defined for each NAME\n\
	whose VALUE is given.  A trailing space in VALUE causes the next\n\
	word to be checked for alias substitution.  Alias returns true\n\
	unless a NAME is given for which no alias has been defined" },

#ifdef JOB_CONTROL
  { "bg", bg_builtin, 1, "bg [job_spec]",
      "	Place JOB_SPEC in the background, as if it had been started with\n\
	`&'.  If JOB_SPEC is not present, the shell's notion of the current\n\
	job is used" },
#endif

  { "break", break_builtin, 1, "break [n]",
      "	Exit from within a FOR, WHILE or UNTIL loop.  If N is specified,\n\
	break N levels" },

  { "builtin", builtin_builtin, 1, "builtin [shell-builtin [arg ...]]",
      "	Run a shell builtin.  This is useful when you wish to rename a\n\
	shell builtin to be a function, but need the functionality of the\n\
	builtin within the function itself" },

  { "bye", exit_builtin, 1, "bye [n]",
      "	Synonym for exit" },

  { "cd", cd_builtin, 1, "cd [dir]",
      "	Change the current directory to DIR.  The variable $HOME is the\n\
	default DIR.  The variable $CDPATH defines the search path for\n\
	the directory containing DIR.  Alternative directory names are\n\
	separated by a colon (:).  A null directory name is the same as\n\
	the current directory, i.e. `.'.  If DIR begins with a slash (/),\n\
	then $CDPATH is not used" },

  { "command", command_builtin, 1, "command [command [arg ...]]",
      "	Runs COMMAND with ARGS ignoring shell functions.  If you have a\n\
	shell function called `ls', and you wish to call the command\n\
	`ls', you can say \"command ls\"" },

  { "continue", continue_builtin, 1, "continue [n]",
      "	Resume the next iteration of the enclosing FOR, WHILE or UNTIL loop.\n\
	If N is specified, resume at the N-th enclosing loop" },

  { "declare", declare_builtin, 1, "declare [-[frx]] name[=value]",
      "	Declare variables and/or give them attributes.  If no NAMEs are\n\
	are given, then display the values of variables instead.  `-f'\n\
	says to use function names only.  `-r' says to make NAMEs readonly.\n\
	`-x' says to make NAMEs export.  Using `+' instead of `-' turns off\n\
	the attribute instead.  When used in a function, makes NAMEs local,\n\
	as with the `local' command" },

#ifdef PUSHD_AND_POPD
  { "dirs", dirs_builtin, 1, "dirs",
      "	Display the list of currently remembered directories.  Directories\n\
	find their way onto the list with the `pushd' command; you can get\n\
	back up through the list with the `popd' command" },
#endif

#ifndef V9_ECHO
  { "echo", echo_builtin, 1, "echo [-n] [arg]",
      " Output the ARGs.  If -n is specified, then suppress trailing\n\
        newline" },
#else
  { "echo", echo_builtin, 1, "echo [-n] [-e] [arg ...]",
      "	Output the ARGs.  if -n is specified, the trailing newline is\n\
	suppressed.  If the -e option is given, interpretation of the\n\
	following backslash-escaped characters is turned on:\n\
		\\b     backspace\n\
		\\c     suppress trailing newline\n\
		\\f     form feed\n\
		\\n     new line\n\
		\\r     carriage return\n\
		\\t     horizontal tab\n\
		\\v     vertical tab\n\
		\\num   the character whose ASCII code is NUM (octal)"},
#endif

  { "enable", enable_builtin, 1, "enable [-n] [name ...]",
      "	Enable and disable builtin shell commands.  This allows\n\
	you to use a disk command which has the same name as a shell\n\
	builtin.  If -n is used, the NAMEs become disabled.  Otherwise\n\
	NAMEs are enabled.  For example, to use the `test' found on your\n\
	path instead of the shell builtin version, you type `enable -n test'" },

  { "eval", eval_builtin, 1, "eval [arg ...]",
      "	Read ARGs as input to the shell and execute the resulting command(s)" },

  { "exec", exec_builtin, 1, "exec [ [-] file [redirections]]",
      "	Exec FILE, replacing this shell with the specified program.\n\
	If FILE is not specified, the redirections take effect in this\n\
	shell.  If the first argument is `-', then place a dash in the\n\
	zeroith arg passed to FILE.  This is what login does.  If the file\n\
	cannot be exec'ed for some reason, the shell exits, unless the\n\
	shell variable \"no_exit_on_failed_exec\" exists" },

  { "exit", exit_builtin, 1, "exit [n]",
      "	Exit the shell with a status of N.  If N is omitted, the exit status\n\
	is that of the last command executed" },

  { "export", export_builtin, 1, "export [-n] [name] ...",
      "	NAMEs are marked for automatic export to the environment of\n\
	subsequently executed commands.  If no NAMEs are given, a list\n\
	of all names that are exported in this shell is printed.  Note\n\
	that function names cannot be exported.  An argument of `-n' says\n\
	to remove the export property from subsequent NAMEs" },

#ifdef JOB_CONTROL
  { "fg", fg_builtin, 1, "fg [job_spec]",
      "	Place JOB_SPEC in the foreground, and make it the current job.  If\n\
	JOB_SPEC is not present, the shell's notion of the current job is\n\
	used" },
#endif

  { "hash", hash_builtin, 1, "hash [-r] [name]",
      "	For each NAME, the full pathname of the command is determined\n\
	and remembered.  The -r option causes the shell to forget all\n\
	remembered locations.  If no arguments are given, information\n\
	about remembered commands is presented" },

  { "help", help_builtin, 1, "help [pattern]",
     "	Display helpful information about builtin commands.  If\n\
	PATTERN is specified, gives detailed help on all commands\n\
	matching PATTERN, otherwise a list of the builtins is\n\
	printed" },

  { "history", history_builtin, 1, "history [n] [-s] [ [-w | -r] [filename]]",
      "	Display the history list with line numbers.  Lines listed with\n\
	with a `*' have been modified.  Argument of N says to list only\n\
	the last N lines.  Argument `-w' means write out the current\n\
	history file.  `-r' means to read it instead.  If FILENAME is\n\
	given, then use that file, else if $HISTFILE has a value, use\n\
	that, else use ~/.bash_history.  Argument -s oerforms history\n\
	substitution on the following args" },

#ifdef JOB_CONTROL
  { "jobs", jobs_builtin, 1, "jobs [-l]",
      "	Lists the active jobs; given the -l options lists process id's\n\
	in addition to the normal information" },

  { "kill", kill_builtin, 1, "kill [-sigspec -l] [pid | job] ...",
      "	Send the processes named by PID (or JOB) the signal SIGSPEC.\n\
	If SIGSPEC is not present, then SIGTERM is assumed.  An argument\n\
	of `-l' lists the signal names.  Kill is a builtin for two reasons;\n\
	it allows job id's to be used instead of pids, and if you run out of\n\
	processes, you can still kill them" },
#endif

  { "local", local_builtin, 1, "local name[=value]",
      "	Create a local variable called NAME, and give it VALUE.  LOCAL\n\
	can only be used within a function; it makes the variable NAME\n\
	have a visible scope restricted to that function and its children" },

  { "logout", logout_builtin, 1, "logout",
      "	Logout of a login shell" },

#ifdef PUSHD_AND_POPD
  { "popd", popd_builtin, 1, "popd [+n | -n]",
      "	Removes entries from the directory stack.  With no arguments,\n\
	removes the top directory from the stack, and cd's to the new\n\
	top directory.\n\
\n\
	+n    removes the Nth entry counting from the left of the list\n\
	      shown by `dirs', starting with zero.  For example: `popd +0'\n\
	      removes the first directory, `popd +1' the second.\n\
\n\
	-n    removes the Nth entry counting from the right of the list\n\
	      shown by `dirs', starting with zero.  For example: `popd -0'\n\
	      removes the last directory, `popd -1' the next to last.\n\
\n\
	You can see the directory stack with the `dirs' command.\n\
	If the variable 'pushd_silent' is not set and the popd command\n\
	was successful, a 'dirs' will be performed as well." },

  { "pushd", pushd_builtin, 1, "pushd [dir | +n | -n]",
      "	Adds a directory to the top of the directory stack, or rotates\n\
	the stack, making the new top of the stack the current working\n\
	directory.  With no arguments, exchanges the top two directories.\n\
\n\
	+n   Rotates the stack so that the Nth directory (counting\n\
	     from the left of the list shown by `dirs') is at the top.\n\
\n\
	-n   Rotates the stack so that the Nth directory (counting\n\
	     from the right) is at the top.\n\
\n\
	dir  adds DIR to the directory stack at the top, making it the\n\
	     new current working directory.\n\
\n\
	You can see the directory stack with the `dirs' command.\n\
	If the variable 'pushd_silent' is not set and the pushd command\n\
	was successful, a 'dirs' will be performed as well." },
#endif /* PUSHD_AND_POPD */

  { "pwd", pwd_builtin, 1, "pwd",
      "	Print the current working directory"},

  { "read", read_builtin, 1, "read [name ...]",
      "	One line is read from the standard input, and the first word\n\
	is assigned to the first NAME, the second word to the second NAME,\n\
	etc. with leftover words assigned to the last NAME.  Only the\n\
	characters in $IFS are recognized as word delimiters.  The return\n\
	code is zero, unless end-of-file is encountered" },

  { "readonly", readonly_builtin, 1, "readonly [name ...]",
      "	The given NAMEs are marked readonly and the values of these NAMEs\n\
	may not be changed by subsequent assignment.  If no arguments are\n\
	given, a list of all readonly names is printed" },

  { "return", return_builtin, 1,  "return [n]",
      "	Causes a function to exit with the return value specified by N.\n\
	If N is omitted, the return status is that of the last command" },

  { "set", set_builtin, 1, "set [-aefhkntuvx] [arg ...]]",
      "	-a  Mark variables which are modified or created for export\n\
	-e  Exit immediately if a command exits with a non-zero status\n\
	-f  Disable file name generation (globbing)\n\
	-h  Locate and remember function commands as functions are\n\
	    defined.  Functions commands are normally looked up when\n\
	    the function is executed)\n\
	-k  All keyword arguments are placed in the environment for a\n\
	    comand, not just those that precede the command name\n\
	-n  Read commands but do not execute them\n\
	-t  Exit after reading and executing one command\n\
	-u  Treat unset variables as an error when substituting\n\
	-v  Print shell input lines as they are read\n\
	-x  Print commands and their arguments as they are executed\n\
	-l  Save and restore the binding of the NAME in a FOR command.\n\
	-d  Disable the hashing of commands that are looked up for execution.\n\
	    Normally, commands are remembered in a hash table, and once\n\
	    found, do not have to be looked up again\n\
	-o  Enable ! style history substitution.  This flag is on by\n\
	    by default.\n\
\n\
	Using + rather than - causes these flags to be turned off.  The\n\
	flags can also be used upon invocation of the shell.  The current\n\
	set of flags may be found in $-.  The remaining ARGs are positional\n\
	parameters and are assigned, in order, to $1, $2, .. $9.  If no\n\
	ARGs are given, all shell variables are printed" },

  { "shift", shift_builtin, 1, "shift [n]",
      "	The positional parameters from $N+1 ... are renamed to $1 ....  If\n\
	N is not given, it is assumed to be 1" },

  { "source", period_builtin, 1, "source <file>",
      " An alias for the `.' builtin" },

#ifdef JOB_CONTROL
  { "suspend", suspend_builtin, 1, "suspend [-f]",
      "	Suspend the execution of this shell until it receives a SIGCONT\n\
	signal.  The `-f' if specified says not to complain about this\n\
	being a login shell if it is; just suspend anyway" },
#endif

  { "[", test_builtin, 1, "[ args ]",
      "	Synonym for `test'" },

  { "test", test_builtin, 1, "test [expr]",
      "	Exits with a status of 0 (trueness) or 1 (falseness) depending on\n\
	the evaluation of EXPR.  Expressions may be unary or binary.  Unary\n\
	expressions are often used to examine the status of a file.  There\n\
	are string operators as well, and numeric comparison operators.\n\
\n\
	File operators:\n\
\n\
	-b FILE		True if file is block special.\n\
	-c FILE		True if file is character special.\n\
	-d FILE		True if file is a directory.\n\
	-ef FILE	True if file is a hard link.\n\
	-f FILE		True if file is a plain file.\n\
	-g FILE		True if file is set-group-id.\n\
	-L FILE		True if file is a symbolic link.\n\
	-k FILE		True if file has its \"sticky\" bit set.\n\
	-p FILE		True if file is a named pipe.\n\
	-r FILE		True if file is readable by you.\n\
	-s FILE		True if file is not empty.\n\
	-S FILE		True if file is a socket.\n\
	-t [FD]		True if FD is opened on a terminal.  If FD\n\
			is omitted, it defaults to 1 (stdout).\n\
	-u FILE		True if the file is set-user-id.\n\
	-w FILE		True if the file is writable by you.\n\
	-x FILE		True if the file is executable by you.\n\
	-O FILE		True if the file is effectively owned by you.\n\
	-G FILE		True if the file is effectively owned by your group.\n\
\n\
	FILE1 -nt FILE2	True if file1 is newer than (according to\n\
			modification date) file2.\n\
\n\
	FILE1 -ot FILE2 True if file1 is older than file2.\n\
\n\
	String operators:\n\
\n\
	-z STRING	True if string is empty.\n\
	-n STRING\n\
   or			True if string is not empty.\n\
	STRING\n\
	STRING1 = STRING2\n\
			True if the strings are equal.\n\
	STRING1 != STRING2\n\
			True if the strings are not equal.\n\
\n\
	Other operators:\n\
\n\
	! EXPR		True if expr is false.\n\
	EXPR1 -a EXPR2	True if both expr1 AND expr2 are true.\n\
	EXPR1 -o EXPR2	True if either expr1 OR expr2 is true.\n\
\n\
	arg1 OP arg2\n\
	OP is one of -eq, -ne, -lt, -le, -gt, ge.\n\
			Arithmetic binary operators return true if ARG1\n\
			is equal, not-equal, less-than, less-than-or-equal,\n\
			greater-than, or greater-than-or-equal than arg2" },

  { "times", times_builtin, 1, "times",
      "	Print the accumulated user and system times for processes run from\n\
	the shell" },

  { "trap", trap_builtin, 1, "trap [arg] [signal_spec]",
      "	The command ARG is to be read and executed when the shell receives\n\
	signal(s) SIGNAL_SPEC.  If ARG is absent all specified signals are\n\
	are reset to their original values.  If ARG is the null string this\n\
	signal is ignored by the shell and by the commands it invokes.  If\n\
	SIGNAL_SPEC is ON_EXIT (0) the command ARG is executed on exit from\n\
	the shell.  The trap command with no arguments prints the list of\n\
	commands associated with each signal number.  SIGNAL_SPEC is either\n\
	a signal name in <signal.h>, or a signal number.  The syntax `trap -l'\n\
	prints a list of signal names and their corresponding numbers.\n\
	Note that a signal can be sent to the shell with \"kill -signal $$\"" },

  { "type", type_builtin, 1, "type [-all] [-type | -path] [name ...]",
      "	For each NAME, indicate how it would be interpreted if used as a\n\
	command name.\n\
\n\
	If the -type flag is used, returns a single word which is one of\n\
	`alias', `function', `builtin', `file' or `', if NAME is an\n\
	alias, shell function, shell builtin, disk file, or unfound,\n\
	respectively.\n\
\n\
	If the -path flag is used, either returns the name of the disk file\n\
	that would be exec'ed, or nothing if -type wouldn't return `file'.\n\
\n\
	If the -all flag is used, returns all of the places that contain\n\
	an executable named `file'.  This includes aliases and functions,\n\
	if and only if the -path flag is not also used" },

  { "typeset", declare_builtin, 1, "typeset [-frx] [name[=word]]",
      "	Obsolete.  See `declare'" },

  { "ulimit", ulimit_builtin, 1, "ulimit [-cdmstf [limit]]",
      "	Ulimit provides control over the resources available to processes\n\
        started by the shell, on systems that allow such control.  If an\n\
        option is given, it is interpreted as follows:\n\
\n\
                -c      the maximum size of core files created\n\
                -d      the maximum size of a process's data segment\n\
                -m      the maximum resident set size\n\
                -s      the maximum stack size\n\
                -t      the maximum amount of cpu time in seconds\n\
                -f      the maximum size of files created by the shell\n\
\n\
        If LIMIT is given, it is the new value of the specified resource.\n\
	Otherwise, the current value of the specified resource is printed.\n\
	If no option is given, then -f is assumed.  Values are in 1k\n\
	increments, except for -t, which is in seconds" },

  { "umask", umask_builtin, 1, "umask [nnn]",
      "	The user file-creation mask is set to NNN.  If NNN is omitted, the\n\
	current value of the mask is printed.  NNN is read as an octal\n\
	number" },

  { "unalias", unalias_builtin, 1, "unalias [name ...]",
      "	Remove NAMEs from the list of defined aliases" },

  { "unset", unset_builtin, 1, "unset [name ...]",
      "	For each NAME, remove the corresponding variable or function.  Note\n\
	that PATH and IFS cannot be unset" },

  { "wait", wait_builtin, 1, "wait [n]",
      "	Wait for the specified process and report its termination\n\
	status.  If N is not given, all currently active child processes\n\
	are waited for, and the return code is zero" },

  /* This marks the end of the functions which are builtins per-se.  The
     following are actually parser constructs. */
  { "for", (Function *)0x0, 1, "for NAME [in WORDS ...] ; do COMMANDS ; done",
      "	The `for' loop executes a sequence of commands for each member in a\n\
	list of items.  If \"in WORDS ...\" is not present, then \"in $*\" is\n\
	assumed.  For each element in WORDS, NAME is set to that element, and\n\
	the COMMANDS are executed" },

  { "case", (Function *)0x0, 1, "case WORD in [PATTERN [| PATTERN]...) COMMANDS ;;]... esac",
      "	Selectively execute COMMANDS based upon WORD matching PATTERN.  The\n\
	`|' is used to separate multiple patterns" },

  { "if", (Function *)0x0, 1, "if COMMANDS then COMMANDS [else COMMANDS] fi",
      "	`if' executes the `then' COMMANDS only if the final command in the `if'\n\
	COMMANDS has an exit status of zero" },

  { "while", (Function *)0x0, 1, "while COMMANDS do COMMANDS done",
      "	Expand and execute COMMANDS as long as the final command in the `while'\n\
	COMMANDS has an exit status of zero" },

  { "until", (Function *)0x0, 1, "until COMMAND do COMMANDS done",
      "	Expand and execute COMMANDS as long as the final command in the `until'\n\
	COMMANDS has an exit status which is not zero" },

  { "function", (Function *)0x0, 1, "function NAME { COMMANDS ; } or NAME () { COMMANDS ; }",
      "	Create a simple command invoked by NAME which runs COMMANDS.  Arguments\n\
	on the command line along with NAME are passed to the function as\n\
	$0 .. $n" },
  { "{ ... }", (Function *)0x0, 1, "{ COMMANDS }",
      "	Run a set of commands in a group.  This is one way to redirect an\n\
	entire set of commands" },

#ifdef JOB_CONTROL
  { "%", (Function *)0x0, 1, "%[DIGITS | WORD] [&]",
      "	This is similar to the `fg' command.  Resume a stopped or background\n\
	job.  If you specifiy DIGITS, then that job is used.  If you specify\n\
	WORD, then the job whose name begins with WORD is used.  Following\n\
	the job specification with a `&' places the job in the background" },
#endif

  { (char *)0x0, (Function *)0x0, 0, (char *)0x0, (char *)0x0 }

};

/* Enable the shell command NAME.  If DISABLE_P is non-zero, then
   disable NAME instead. */
enable_shell_command (name, disable_p)
     char *name;
     int disable_p;
{
  register int i;
  int found = 0;

  for (i = 0; shell_builtins[i].function; i++)
    if (strcmp (name, shell_builtins[i].name) == 0)
      {
	found++;
	shell_builtins[i].enabled = !disable_p;
      }

  return (found);
}

/* Enable/disable shell commands present in LIST.  If list is not specified,
   then print out a list of shell commands showing which are enabled and
   which are disabled. */
enable_builtin (list)
     WORD_LIST *list;
{
  int result = 0;

  if (!list)
    {
      register int i;

      for (i = 0; shell_builtins[i].function; i++)
	printf ("enable %s%s\n", shell_builtins[i].enabled ? "" : "-n ",
		shell_builtins[i].name);
    }
  else
    {
      int disable_p = (strcmp (list->word->word, "-n") == 0);

      if (disable_p)
	list = list->next;

      while (list)
	{
	  result = enable_shell_command (list->word->word, disable_p);
	  list = list->next;
	}
    }
  return (result ? EXECUTION_FAILURE : EXECUTION_SUCCESS);
}

/* Print out a list of the known functions in the shell, and what they do.
   If LIST is supplied, print out the list which matches for each pattern
   specified. */
help_builtin (list)
     WORD_LIST *list;
{
  if (!list)
    {
      register int j, i = 0;
      char blurb[256];

      show_shell_version ();
      printf (
"Shell commands that are defined internally.  Type `help' to see this list.\n\
Type `help name' to find out more about the function `name'.\n\
Use `info bash' to find out more about the shell in general.\n\
\n\
A star (*) next to a name means that the command is disabled.\n\
\n");

      for (i = 0; shell_builtins[i].name; i++)
	{
	  sprintf (blurb, "%c%s", shell_builtins[i].enabled ? ' ' : '*',
		   shell_builtins[i].short_doc);

	  blurb[35] = '\0';
	  printf ("%s", blurb);

	  if (i % 2)
	    printf ("\n");
	  else
	    for (j = strlen (blurb); j < 35; j++)
	      putc (' ', stdout);
	    
	}
      if (i % 2)
	printf ("\n");
    }
  else
    {
      int match_found = 0;
      char *pattern = "";
      
      if (glob_pattern_p (list->word->word))
	{
	  printf ("Shell commands matching keyword%s `",
		  list->next ? "s" : "");
	  print_word_list (list, ", ");
	  printf ("'\n\n");
	}
      while (list)
	{
	  register int i = 0, plen;

	  pattern = list->word->word;
	  plen = strlen (pattern);

	  while (shell_builtins[i].name)
	    {
	      if (glob_match (pattern, shell_builtins[i].name, 0) ||
		  strnicmp (pattern, shell_builtins[i].name, plen) == 0)
		{
		  printf ("%s: %s\n%s.\n",
			  shell_builtins[i].name, shell_builtins[i].short_doc,
			  shell_builtins[i].long_doc);
		  match_found++;
		}
	      i++;
	    }
	  list = list->next;
	}
      if (!match_found)
	{
	  printf ("No help topics match `%s'.  Try `help help'.\n", pattern);
	  fflush (stdout);
	  return (EXECUTION_FAILURE);
	}
    }
  fflush (stdout);
  return (EXECUTION_SUCCESS);
}

/* Do nothing.  This command is a no-op. */
colon_builtin ()
{
  return (EXECUTION_SUCCESS);
}

/* Read and execute commands from the file passed as argument.  Guess what.
   This cannot be done in a subshell, since things like variable assignments
   take place in there.  So, I open the file, place it into a large string,
   close the file, and then execute the string. */
period_builtin (list)
     WORD_LIST *list;
{
  int result = (EXECUTION_SUCCESS);
  int push_dollar_vars (), pop_dollar_vars ();
    
  if (list)
    {
      int fd;			/* File descriptor. */
      int tt;			/* Temporary result. */
      char *string;		/* String to execute. */
      char *filename, *tempfile;
      extern char *find_path_file ();
      struct stat finfo;

      tempfile = find_path_file (list->word->word);

      if (!tempfile)
	tempfile = savestring (list->word->word);

      filename = (char *)alloca (1 + strlen (tempfile));
      strcpy (filename, tempfile);
      free (tempfile);

      if (stat (filename, &finfo) == -1 ||
	  (fd = open (filename, O_RDONLY)) == -1)
	goto file_error_exit;

      string = (char *)xmalloc  (1 + finfo.st_size);
      tt = read (fd, string, finfo.st_size);

      /* Close the open file, preserving the state of errno. */
      { int temp = errno; close (fd); errno = temp; }

      if (tt != finfo.st_size)
	{
	  free (string);
	file_error_exit:
	  file_error (filename);
	  return (EXECUTION_FAILURE);
	}

      push_dollar_vars ();
      remember_args (list->next, 1);
      begin_unwind_frame ("file_sourceing");
      add_unwind_protect (pop_dollar_vars, (char *)NULL);
      string[finfo.st_size] = '\0';
      result = parse_and_execute (string, filename);
      run_unwind_frame ("file_sourceing");
    }
  return (result);
}

/* Parse and execute the commands in STRING.  Returns whatever
   execute_command () returns.  This frees STRING. */
int
parse_and_execute (string, from_file)
     char *string;
     char *from_file;
{
  extern int remember_on_history;
  extern int history_expansion_inhibited;
  extern int indirection_level;
  char *indirection_level_string ();

  int old_interactive = interactive;
  int old_remember_on_history = remember_on_history;
  int old_history_expansion_inhibited = history_expansion_inhibited;
  int last_result = EXECUTION_SUCCESS;
  char *orig_string = string;
  extern COMMAND *global_command;

  push_stream ();
  interactive = 0;
  indirection_level++;

  /* We don't remember text read by the shell this way on
     the history list, and we don't use !$ in shell scripts. */
  remember_on_history = 0;
  history_expansion_inhibited = 1;
  
  with_input_from_string (string, from_file);
  {
    extern char *yy_input_dev;
    COMMAND *command;

    while (*yy_input_dev)
      {
	if (interrupt_state)
	  {
	    last_result = EXECUTION_FAILURE;
	    break;
	  }

	if (yyparse () == 0)
	  {
	    if ((command = global_command) != (COMMAND *)NULL)
	      {
		global_command = (COMMAND *)NULL;

		last_result = execute_command (command);
		dispose_command (command);
	      }
	  }
	else
	  {
	    last_result = EXECUTION_FAILURE;

	    /* Since we are shell compatible, syntax errors in a script
	       abort the execution of the script.  Right? */
	    break;
	  }
      }
  }

  remember_on_history = old_remember_on_history;
  history_expansion_inhibited = old_history_expansion_inhibited;
  interactive = old_interactive;
  pop_stream ();
  indirection_level--;
  if (orig_string)
    free (orig_string);

  return (last_result);
}

/* Set up to break x levels, where x defaults to 1, but can be specified
   as the first argument. */

/* The depth of while's and until's. */
int loop_level = 0;

/* Non-zero when a "break" instruction is encountered. */
int breaking = 0;

/* Return non-zero if a break or continue command would be okay.
   Print an error message if break or continue is meaningless here. */
check_loop_level ()
{
  extern char *this_command_name;

#ifdef BREAK_COMPLAINS
  if (!loop_level)
    builtin_error ("Only meaningful in a `for', `while', or `until' loop");
#endif
  return (loop_level);
}

break_builtin (list)
     WORD_LIST *list;
{

  if (!check_loop_level ())
    return (EXECUTION_FAILURE);

  breaking = get_numeric_arg (list);

  if (breaking < 0)
    breaking = 0;

  if (breaking > loop_level)
    breaking = loop_level;

  return (EXECUTION_SUCCESS);
}

/* Set up to continue x levels, where x defaults to 1, but can be specified
   as the first argument. */

/* Non-zero when we have encountered a continue instruction. */
int continuing = 0;

continue_builtin (list)
     WORD_LIST *list;
{

  if (!check_loop_level ())
    return (EXECUTION_FAILURE);

  continuing = get_numeric_arg (list);

  if (continuing < 0)
    continuing = 0;

  if (continuing > loop_level)
    continuing = loop_level;
  
  return (EXECUTION_SUCCESS);
}

/* Read a numeric arg for this_command_name, the name of the shell builtin
   that wants it.  LIST is the word list that the arg is to come from. */
get_numeric_arg (list)
     WORD_LIST *list;
{
  int count = 1;

  if (list)
    {
      if (sscanf (list->word->word, "%d", &count) != 1)
	{
	  builtin_error ("bad non-numeric arg `%s'", list->word->word);
	  longjmp (top_level, DISCARD);
	}
      no_args (list->next);
    }
  return (count);
}

/* Change the current working directory to the first word in LIST, or
   to $HOME if there is no LIST.  Do nothing in the case that there is
   no $HOME nor LIST. If the variable CDPATH exists, use that as the search
   path for finding the directory.  If all that fails, and the variable
   `cdable_vars' exists, then try the word as a variable name.  If that
   variable has a value, then cd to the value of that variable. */

/* By default, follow the symbolic links as if they were real directories
   while hacking the `cd' command.  This means that `cd ..' moves up in
   the string of symbolic links that make up the current directory, instead
   of the absolute directory.  The shell variable `nolinks' controls this
   flag. */
int follow_symbolic_links = 1;

/* In order to keep track of the working directory, we have this static
   variable hanging around. */
static char *the_current_working_directory = (char *)NULL;

cd_builtin (list)
     WORD_LIST *list;
{
  char *dirname;

#ifdef FACIST
  {
    extern int restricted;
    if (restricted)
      {
	builtin_error ("Privileged command");
	return (EXECUTION_FAILURE);
      }
  }
#endif

  if (list)
    {
      char *extract_colon_unit ();
      char *path_string = get_string_value ("CDPATH");
      char *path;
      int index = 0;

      dirname = list->word->word;

      if (path_string && !absolute_pathname (dirname))
	{
	  while ((path = extract_colon_unit (path_string, &index)))
	    {
	      char *dir;

	      if (!disallow_filename_globbing && *path)
		{
		  char *tilde_expand (), *te_string = tilde_expand (path);

		  free (path);
		  path = te_string;
		}

	      dir = (char *)alloca (2 + strlen (dirname) + strlen (path));

	      if (!dir)
		abort ();

	      if (!*path)
		{
		  free (path);
		  path = savestring ("."); /* by definition. */
		}

	      strcpy (dir, path);
	      if (path[strlen (path) - 1] != '/')
		strcat (dir, "/");
	      strcat (dir, dirname);
	      free (path);

	      if (change_to_directory (dir))
		{
		  if (strncmp (dir, "./", 2) != 0)
		    printf ("%s\n", dir);
		  dirname = dir;

		  goto bind_and_exit;
		}
	    }
	}

      if (!change_to_directory (dirname))
	{
	  /* Maybe this is `cd -', equivalent to `cd $OLDPWD' */
	  if (dirname[0] == '-' && dirname[1] == '\0')
	    {
	      char *t = get_string_value ("OLDPWD");

	      if (t && change_to_directory (t))
		goto bind_and_exit;
	    }

	  /* If the user requests it, then perhaps this is the name of
	     a shell variable, whose value contains the directory to
	     change to.  If that is the case, then change to that
	     directory. */
	  if (find_variable ("cdable_vars"))
	    {
	      char *t = get_string_value (dirname);

	      if (t && change_to_directory (t))
		{
		  printf ("%s\n", t);
		  goto bind_and_exit;
		}
	    }

	  file_error (dirname);
	  return (EXECUTION_FAILURE);
	}
      goto bind_and_exit;
    }
  else
    {
      dirname = get_string_value ("HOME");

      if (!dirname)
	return (EXECUTION_FAILURE);

      if (!change_to_directory (dirname))
	{
	  file_error (dirname);
	  return (EXECUTION_FAILURE);
	}

    bind_and_exit:
      {
	char *get_working_directory (), *get_string_value ();
	char *directory = get_working_directory ("cd");

	bind_variable ("OLDPWD", get_string_value ("PWD"));
	bind_variable ("PWD", directory);

	if (directory)
	  free (directory);
      }
      return (EXECUTION_SUCCESS);
    }
}

/* Do the work of changing to the directory NEWDIR.  Handle symbolic
   link following, etc. */
change_to_directory (newdir)
     char *newdir;
{
  char *get_working_directory (), *make_absolute ();
  char *t;

  if (follow_symbolic_links)
    {
      if (!the_current_working_directory)
	{
	  t = get_working_directory ("cd_links");
	  if (t)
	    free (t);
	}

      if (the_current_working_directory)
	t = make_absolute (newdir, the_current_working_directory);
      else
	t = savestring (newdir);

      /* Get rid of trailing `/'. */
      {
	register int len_t = strlen (t);
	if (len_t > 1)
	  {
	    --len_t;
	    if (t[len_t] == '/')
	      t[len_t] = '\0';
	  }
      }

      if (chdir (t) < 0)
	{
	  free (t);
	  return (0);
	}

      if (the_current_working_directory)
	strcpy (the_current_working_directory, t);

      free (t);
      return (1);
    }
  else
    {
      if (chdir (newdir) < 0)
	return (0);
      else
	return (1);
    }
}

/* Return a consed string which is the current working directory.
   FOR_WHOM is the name of the caller for error printing.  */
char *
get_working_directory (for_whom)
     char *for_whom;
{
  if (!follow_symbolic_links)
    {
      if (the_current_working_directory)
	free (the_current_working_directory);

      the_current_working_directory = (char *)NULL;
    }

  if (!the_current_working_directory)
    {
      char *directory, *getwd ();

      the_current_working_directory = (char *)xmalloc (MAXPATHLEN);
      directory = getwd (the_current_working_directory);
      if (!directory)
	{
	  fprintf (stderr, "%s: %s\n",
		   for_whom, the_current_working_directory);
	  free (the_current_working_directory);
	  the_current_working_directory = (char *)NULL;
	  return (char *)NULL;
	}
    }

  return (savestring (the_current_working_directory));
}


/* Print the words in LIST to standard output.  If the first word is
   `-n', then don't print a trailing newline.  We also support the
   echo syntax from Version 9 unix systems. */
echo_builtin (list)
     WORD_LIST *list;
{
  int display_return = 1, do_v9 = 0;

/* System V machines already have a /bin/sh with a v9 behaviour.  We
   give Bash the identical behaviour for these machines so that the
   existing system shells won't barf. */
#if defined (V9_ECHO) && defined (SYSV)
	     do_v9 = 1;
#endif

  while (list && list->word->word[0] == '-')
    {
      char *temp = &(list->word->word[1]);

      if (!*temp)
	goto just_echo;

      while (*temp)
	{
	  if (*temp == 'n')
	    display_return = 0;
#ifdef V9_ECHO
	  else if (*temp == 'e')
	    do_v9 = 1;
#ifdef SYSV
	  else if (*temp == 'E')
	    do_v9 = 0;
#endif
#endif
	  else
	    goto just_echo;
	  
	  temp++;
	}
      list = list->next;
    }

just_echo:

  if (list)
    {
#ifdef V9_ECHO
      if (do_v9)
	{
	  while (list)
	    {
	      register char *s = list->word->word;
	      register int c;

	      while (c = *s++)
		{
		  if (c == '\\' && *s)
		    {
		      switch (c = *s++)
			{
			case 'b': c = '\b'; break;
			case 'c': display_return = 0; continue;
			case 'f': c = '\f'; break;
			case 'n': c = '\n'; break;
			case 'r': c = '\r'; break;
			case 't': c = '\t'; break;
			case 'v': c = (int) 0x0B; break;
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			  c -= '0';
			  if (*s >= '0' && *s <= '7')
			    c = c * 8 + (*s++ - '0');
			  if (*s >= '0' && *s <= '7')
			    c = c * 8 + (*s++ - '0');
			  break;
			case '\\': break;
			default:  putchar ('\\'); break;
			}
		    }
		  putchar(c);
		}
	      list = list->next;
	      if (list)
		putchar(' ');
	    }
	}
      else
#endif  /* V9_ECHO */	
	print_word_list (list, " ");
    }
  if (display_return)
    printf ("\n");
  fflush (stdout);
  return (EXECUTION_SUCCESS);
}

/* Parse the string that these words make, and execute the command found. */
eval_builtin (list)
     WORD_LIST *list;
{
  char *string_list ();
  int result;

  /* Note that parse_and_execute () free ()'s what it is passed. */
  if (list)
    result = parse_and_execute (string_list (list), "eval");
  else
    result = EXECUTION_SUCCESS;
  return (result);
}

/* Defined in execute_cmd.c */
extern REDIRECT *redirection_undo_list;

exec_builtin (list)
     WORD_LIST *list;
{
  extern char *find_user_command ();

  maybe_make_export_env ();

  /* First, let the redirections remain. */
  dispose_redirects (redirection_undo_list);
  redirection_undo_list = (REDIRECT *)NULL;

  if (!list)
    return (EXECUTION_SUCCESS);
  else
    {
      /* Otherwise, execve the new command with args. */
      char *command, **args;
      int dash_name = 0;

      if (strcmp (list->word->word, "-") == 0)
	{
	  /* The user would like to exec this command as if it was a
	     login command.  Do so. */
	  list = list->next;
	  dash_name++;
	}

      args = (char **)make_word_array (list);
      command = find_user_command (args[0]);
      if (!command)
	{
	  builtin_error ("%s: not found", args[0]);
	  goto failed_exec;
	}

      command = (char *)full_pathname (command);
      /* If the user wants this to look like a login shell, then
	 pass the full pathname in argv[0]. */
      if (dash_name)
	{
	  char *new_name = (char *)xmalloc (1 + strlen (command));
	  strcpy (new_name, "-");
	  strcat (new_name, command);
	  args[0] = new_name;
	}

      write_history (get_string_value ("HISTFILE"));

      signal (SIGINT, SIG_DFL);
      signal (SIGQUIT, SIG_DFL);
      execve (command, args, export_env);

      if (!executable_file (command))
	builtin_error ("%s: cannot execute", command);
      else
	file_error (command);

    failed_exec:
      if (!interactive && !find_variable ("no_exit_on_failed_exec"))
	exit (EXECUTION_FAILURE);
      return (EXECUTION_FAILURE);
    }
}

exit_builtin (list)
     WORD_LIST *list;
{
  extern int login_shell;

  if (interactive && login_shell)
    {
      fprintf (stderr, "logout\n");
      fflush (stderr);
    }

  exit_or_logout (list);
}

/* How to logout. */
logout_builtin (list)
     WORD_LIST *list;
{
  if (!login_shell && interactive)
    {
      builtin_error ("Not login shell: use `exit' or `bye'");
      return (EXECUTION_FAILURE);
    }
  else
    exit_or_logout (list);
}

/* Clean up work for exiting or logging out. */
Function *last_shell_builtin = (Function *)NULL;
Function *this_shell_builtin = (Function *)NULL;

exit_or_logout (list)
     WORD_LIST *list;
{
  extern int last_command_exit_value;

#ifdef JOB_CONTROL
  int exit_immediate_okay;

  exit_immediate_okay = (!interactive ||
			 last_shell_builtin == exit_builtin ||
			 last_shell_builtin == logout_builtin ||
			 last_shell_builtin == jobs_builtin);

  /* Check for stopped jobs if the user wants to. */
  if (1 && !exit_immediate_okay)
    {
      register int i;
      for (i = 0; i < job_slots; i++)
	if (jobs[i] && (jobs[i]->state == JSTOPPED))
	  {
	    fprintf (stderr, "There are stopped jobs.\n");

	    /* This is NOT superfluous because EOF can get here without
	       going through the command parser. */
	    last_shell_builtin = exit_builtin;

	    longjmp (top_level, DISCARD);
	  }
    }
#endif

  /* Get return value if present.  This means that you can type
     `logout 5' to a shell, and it returns 5. */

  if (list)
    last_command_exit_value = get_numeric_arg (list);

  /* Run our `~/.bash_logout' file if it exists, and this is a login shell. */
  if (login_shell)
    maybe_execute_file ("~/.bash_logout");

  /* Exit the program. */
  longjmp (top_level, EXITPROG);
}

/* For each variable name in LIST, make that variable appear in the
   environment passed to simple commands.  If there is no LIST, then
   print all such variables.  An argument of `-n' says to remove the
   exported attribute from variables named in LIST. */
export_builtin (list)
     register WORD_LIST *list;
{
  return (set_or_show_attributes (list, att_exported));
}

/* For each variable name in LIST, make that variable have the specified
   ATTRIBUTE.  An arg of `-n' says to remove the attribute from the the
   remaining names in LIST.  */
set_or_show_attributes (list, attribute)
     register WORD_LIST *list;
     int attribute;
{
  register SHELL_VAR *var;
  int assign, undo = 0;
  extern int array_needs_making;

  if (list)
    {
      while (list)
	{
	  register char *name = list->word->word;

	  if (strcmp (name, "-n") == 0)
	    {
	      undo++;
	      list = list->next;
	      continue;
	    }

	  var = find_variable (name);

	  if ((assign = assignment (name)) != 0)
	    {
	      do_assignment (name);
	      name[assign] = '\0';
	    }

	  if (undo)
	    {
	      var = find_variable (name);
	      if (var)
		var->attributes &= ~attribute;
	    }
	  else
	    {
	      var = find_variable (name);

	      if (!var)
		{
		  SHELL_VAR *find_tempenv_variable ();

		  if ((var = find_tempenv_variable (name)))
		    {
		      SHELL_VAR *disposer = var;
		      var = bind_variable (disposer->name, disposer->value);
		      dispose_variable (disposer);
		    }
		  else
		    {
		      var = bind_variable (name, (char *)NULL);
		      var->attributes |= att_invisible;
		    }
		}

	      var->attributes |= attribute;
	    }
	  array_needs_making++;

	  list = list->next;
	}
    }
  else
    {
      var = variable_list;

      while (var)
	{
	  if ((var->attributes & attribute) && !invisible_p (var))
	    {
	      char flags[5];

	      flags[0] = '\0';

	      if (var->attributes & att_exported)
		strcat (flags, "x");

	      if (var->attributes & att_readonly)
		strcat (flags, "r");

	      if (function_p (var))
		 strcat (flags, "f");

	      if (flags[0])
		{
		  printf ("declare -%s ", flags);

		  if (!function_p (var))
		    printf ("%s=%s\n", var->name, value_cell (var));
		  else
		    {
		      char *named_function_string ();

		      printf ("%s\n",
			      named_function_string (var->name,
						     function_cell (var), 1));
		    }
		}
	    }
	  var = var->next;
	}
    }
  return (EXECUTION_SUCCESS);
}

/* Hashing filenames in the shell. */

#include "hash.h"

#define FILENAME_HASH_BUCKETS 631

HASH_TABLE *hashed_filenames;

typedef struct {
  /* The full pathname of the file. */
  char *path;

  /* Whether `.' appeared before this one in $PATHS. */
  int check_dot;
} PATH_DATA;

#define pathdata(x) ((PATH_DATA *)(x)->data)

initialize_filename_hashing ()
{
  hashed_filenames = make_hash_table (FILENAME_HASH_BUCKETS);
}

/* Place FILENAME (key) and FULL_PATHNAME (data->path) into the
   hash table.  CHECK_DOT if non-null is for future calls to
   find_hashed_filename (). */
remember_filename (filename, full_pathname, check_dot)
     char *filename, *full_pathname;
     int check_dot;
{
  register BUCKET_CONTENTS *item;

  if (hashing_disabled)
    return;
  item = add_hash_item (filename, hashed_filenames);
  if (item->data)
    free (pathdata(item)->path);
  else
    item->data = (char *)xmalloc (sizeof (PATH_DATA));

  item->key = savestring (filename);
  pathdata(item)->path = savestring (full_pathname);
  pathdata(item)->check_dot = check_dot;
  item->times_found = 0;
}

/* Temporary static. */
char *dotted_filename = (char *)NULL;

/* Return the full pathname that FILENAME hashes to.  If FILENAME
   is hashed, but data->check_dot is non-zero, check ./FILENAME
   and return that if it is executable. */
char *
find_hashed_filename (filename)
     char *filename;
{
  register BUCKET_CONTENTS *item;

  if (hashing_disabled)
    return ((char *)NULL);

  item = find_hash_item (filename, hashed_filenames);
  if (item) {

    /* If this filename is hashed, but `.' comes before it in the path,
       then see if `./filename' is an executable. */
    if (pathdata(item)->check_dot) {

      if (dotted_filename)
	free (dotted_filename);

      dotted_filename = (char *)xmalloc (3 + strlen (filename));
      strcpy (dotted_filename, "./");
      strcat (dotted_filename, filename);

      if (executable_file (dotted_filename))
	return (dotted_filename);

      /* Watch out.  If this file was hashed to "./filename", and
	 "./filename" is not executable, then return NULL. */
      if (strcmp (pathdata(item)->path, dotted_filename) == 0)
	return ((char *)NULL);
    }
    return (pathdata(item)->path);
  } else {
    return ((char *)NULL);
  }
}

/* Print statistics on the current state of hashed commands.  If LIST is
   not empty, then rehash (or hash in the first place) the specified
   commands. */
hash_builtin (list)
     WORD_LIST *list;
{
  extern Function *find_shell_builtin ();
  extern char *find_user_command ();
  int any_failed = 0;

  if (hashing_disabled)
    {
      builtin_error ("Hashing is disabled");
      return (EXECUTION_FAILURE);
    }

  if (!list)
    {
      /* Print information about current hashed info. */
      int any_printed = 0;
      int bucket = 0;
      register BUCKET_CONTENTS *item_list;

      while (bucket < hashed_filenames->nbuckets)
	{
	  item_list = get_hash_bucket (bucket, hashed_filenames);
	  if (item_list)
	    {
	      if (!any_printed)
		{
		  printf ("hits\tcommand\n");
		  any_printed++;
		}
	      while (item_list)
		{
		  printf ("%4d\t%s\n",
			  item_list->times_found, pathdata(item_list)->path);
		  item_list = item_list->next;
		}
	    }
	  bucket++;
	}
      if (!any_printed)
	{
	  printf ("No commands in hash table.\n");
	}
    }
  else
    {
      /* Add or rehash the specified commands. */
      char *word;
      char *full_path;
      SHELL_VAR *var;
      int any_failed = 0;

      if (strcmp (list->word->word, "-r") == 0)
	{
	  int bucket = 0;
	  register BUCKET_CONTENTS *item_list, *prev;

	  while (bucket < hashed_filenames->nbuckets)
	    {
	      item_list = get_hash_bucket (bucket, hashed_filenames);
	      if (item_list)
		{
		  while (item_list)
		    {
		      prev = item_list;
		      free (item_list->key);
		      free (pathdata(item_list)->path);
		      free (item_list->data);
		      item_list = item_list->next;
		      free (prev);
		    }
		  hashed_filenames->bucket_array[bucket] = (BUCKET_CONTENTS *)NULL;
		}
	      bucket++;
	    }
	  list = list->next;
	}
      while (list)
	{
	  word = list->word->word;
	  if (absolute_pathname (word))
	    goto next;
	  full_path = find_user_command (word);
	  var = find_variable (word);

	  if (!find_shell_builtin (word) &&
	      (!var || !function_p (var)))
	    {
	      if (full_path && executable_file (full_path))
		{
		  extern int dot_found_in_search;
		  remember_filename (word, full_path, dot_found_in_search);
		}
	      else
		{
		  builtin_error ("%s: not found", word);
		  any_failed++;
		}
	  }
	next:
	  list = list->next;
	}
    }
  return (any_failed ? EXECUTION_FAILURE : EXECUTION_SUCCESS);
}

/* History.  Arg of -w FILENAME means write file, arg of -r FILENAME
   means read file.  Arg of N means only display that many items. */

history_builtin (list)
     WORD_LIST *list;
{
  HIST_ENTRY **hlist = history_list ();
  int limited = (list != (WORD_LIST *)NULL);
  int limit;
  register int i = 0;

  if (hlist)
    while (hlist[i]) i++;

  if (list)
    {
      if ((strcmp (list->word->word, "-w") == 0) ||
	  (strcmp (list->word->word, "-r") == 0)) {
	int writing = (strcmp (list->word->word, "-w") == 0);
	char *file;
	int result;

	if (list->next)
	  file = list->next->word->word;
	else
	  file = get_string_value ("HISTFILE");

	if (writing)
	  result = write_history (file);
	else
	  result = read_history (file);

	return (result ? EXECUTION_FAILURE : EXECUTION_SUCCESS);
      }
    }

  if (strcmp (list->word->word, "-s") == 0)
    {
      extern int history_expand ();
      char *expanded;
      int rval;

      list = list->next;

      while (list)
	{
	  rval = history_expand (list->word->word, &expanded);
	  printf ("%s", expanded);
	  fflush (stdout);

	  if (rval == -1)
	    return (EXECUTION_FAILURE);

	  free (expanded);

	  list = list->next;
	}
    }
  
  limit = get_numeric_arg (list);
  if (limit < 0)
    limit = -limit;
  
  if (!limited)
    i = 0;
  else
    if ((i -= limit) < 0)
      i = 0;

  while (hlist[i])
    {
      fprintf (stdout,
	       "%5d%c %s\n", i + history_base, hlist[i]->data ? '*' : ' ',
	       hlist[i]->line);
      i++;
    }
  return (EXECUTION_SUCCESS);
}

/* Non-zero means that pwd always give verbatim directory, regardless of
   symbolic link following. */
int verbatim_pwd = 1;

/* Print the name of the current working directory. */
pwd_builtin (list)
     WORD_LIST *list;
{
  char *get_working_directory (), *getwd (), *directory;

  no_args (list);

  if (verbatim_pwd)
    {
      char *buffer = (char *)xmalloc (MAXPATHLEN);
      directory = getwd (buffer);

      if (!directory)
	{
	  builtin_error ("%s", buffer);
	  free (buffer);
	}
    }
  else
    {
      directory = get_working_directory ("pwd");
    }

  if (directory)
    {
      printf ("%s\n", directory);
      fflush (stdout);
      free (directory);
      return (EXECUTION_SUCCESS);
    }
  else
    {
      return (EXECUTION_FAILURE);
    }
}

/* Read the value of the shell variables whose names follow.
   The reading is done from the current input stream, whatever
   that may be.  Successive words of the input line are assigned
   to the variables mentioned in LIST.  The last variable in LIST
   gets the remainder of the words on the line.  If no variables
   are mentioned in LIST, then the default variable is $REPLY.

   S. R. Bourne's shell complains if you don't name a variable
   to receive the stuff that is read.  GNU's shell doesn't.  This
   allows you to let the user type random things. */
read_builtin (list)
     WORD_LIST *list;
{
  extern int interrupt_immediately, free ();
  register char *varname;
  int size, c, i = 0, fildes;
  char *input_string, *ifs_chars;
  WORD_LIST *words, *rwords, *list_string ();
  FILE *input_stream;

  ifs_chars = get_string_value ("IFS");
  input_string = (char *)xmalloc (size = 128);

  /* We need unbuffered input from stdin.  So we make a new
     unbuffered stream with the same file descriptor, then
     unbuffer that one. */
  fildes = dup (fileno (stdin));

  if (fildes == -1)
    return (EXECUTION_FAILURE);

  input_stream = fdopen (fildes, "r");

  if (!input_stream)
    {
      close (fildes);
      return (EXECUTION_FAILURE);
    }

  setbuf (input_stream, (char *)NULL);

  {
    int stream_close ();
    begin_unwind_frame ("read_builtin");
    add_unwind_protect (free, input_string);
    add_unwind_protect (stream_close, input_stream);
    interrupt_immediately++;
  }

  while ((c = getc (input_stream)) != EOF)
    {
      if (i + 1 >= size)
	input_string = (char *)xrealloc (input_string, size += 128);

      input_string[i++] = c;

      if (c == '\n')
	{
	  if ((i >= 2) && (input_string[i - 2] == '\\'))
	    {
	      i -= 2;
	      continue;
	    }
	  break;
	}
    }
  input_string[i] = '\0';

  interrupt_immediately--;
  discard_unwind_frame ("read_builtin");

  fclose (input_stream);

  if (!i)
    return (EXECUTION_FAILURE);    

  if (!list)
    {
      bind_variable ("REPLY", input_string);
      free (input_string);
    }
  else
    {
      words = list_string (input_string, ifs_chars, 0);
      rwords = words;

      free (input_string);

      while (list)
	{
	  varname = list->word->word;

	  if (!list->next)
	    bind_variable (varname, words ? (char *)string_list (words) : "");
	  else
	    bind_variable (varname, words ? words->word->word : "");
	  stupidly_hack_special_variables (varname);
	  list = list->next;
	  if (words)
	    words = words->next;
	}
      if (rwords)
	dispose_words (rwords);
    }

  return (EXECUTION_SUCCESS);
}

/* This way I don't have to know whether fclose is a function or a macro. */
int
stream_close (file)
     FILE *file;
{
  return (fclose (file));
}

/* For each variable name in LIST, make that variable read_only. */
readonly_builtin (list)
     register WORD_LIST *list;
{
  return (set_or_show_attributes (list, att_readonly));
}

/* If we are executing a user-defined function then exit with the value
   specified as an argument.  if no argument is given, then the last
   exit status is used. */
return_builtin (list)
     WORD_LIST *list;
{
  extern int last_command_exit_value;
  extern int return_catch_flag, return_catch_value;
  extern jmp_buf return_catch;

  return_catch_value = get_numeric_arg (list);

  if (!list)
    return_catch_value = last_command_exit_value;

  if (return_catch_flag)
    longjmp (return_catch, 1);
  else
    {
      builtin_error ("Can only `return' from a function");
      longjmp (top_level, DISCARD);
    }
}

/* Set some flags from the word values in the input list.  If LIST is empty,
   then print out the values of the variables instead.  If LIST contains
   non-flags, then set $1 - $9 to the successive words of LIST. */
set_builtin (list)
     WORD_LIST *list;
{
  int on_or_off, flag_name;

  if (!list)
    {
      print_var_list (variable_list);
      return (EXECUTION_SUCCESS);
    }

  /* Do the set command.  While the list consists of words starting with
     '-' or '+' treat them as flags, otherwise, start assigning them to
     $1 ... $n. */
  while (list)
    {
      char *string = list->word->word;
#if defined (NEVER)
      if (strcmp (string, "-") == 0)
	{
	  WORD_LIST *t =
	    (WORD_LIST *)make_word_list (make_word ("-"), NULL);
	  remember_args (t, 1);
	  dispose_words (t);
	  return (EXECUTION_SUCCESS);
	}
#endif /* NEVER */

      /* If the argument is `--' then signal the end of the list and
	 remember the remaining arguments. */
      if (strcmp (string, "--") == 0)
	{
	  list = list->next;
	  break;
	}

      if ((on_or_off = *string) &&
	  (on_or_off == '-' || on_or_off == '+'))
	{
	  int i = 1;
	  while (flag_name = string[i++])
	    {
	      if (flag_name == '?')
		{
		  /* Print all the possible flags. */
		}
	      else
		{
		  if (change_flag_char (flag_name, on_or_off) == FLAG_ERROR)
		    {
		      builtin_error ("%c%c: bad option", on_or_off, flag_name);
		      return (EXECUTION_FAILURE);
		    }
		}
	    }
	}
      else
	{
	  break;
	}
      list = list->next;
    }

  /* Assigning $1 ... $n */
  if (list)
    remember_args (list, 1);
  return (EXECUTION_SUCCESS);
}


/* **************************************************************** */
/*								    */
/*		    Pushing and Popping a Context		    */
/*								    */
/* **************************************************************** */

WORD_LIST **dollar_arg_stack = (WORD_LIST **)NULL;
int dollar_arg_stack_slots = 0;
int dollar_arg_stack_index = 0;

push_context ()
{
  extern int variable_context;

  push_dollar_vars ();
  variable_context++;
}

pop_context ()
{
  extern int variable_context;

  pop_dollar_vars ();
  kill_all_local_variables ();
  variable_context--;
}

/* Save the existing arguments on a stack. */
push_dollar_vars ()
{
  extern WORD_LIST *list_rest_of_args ();

  if (dollar_arg_stack_index + 2 > dollar_arg_stack_slots)
    {
      if (!dollar_arg_stack)
	{
	  dollar_arg_stack_slots = 10;
	  dollar_arg_stack =
	    (WORD_LIST **)xmalloc (dollar_arg_stack_slots * sizeof (WORD_LIST *));
	}
      else
	{
	  dollar_arg_stack_slots += 10;
	  dollar_arg_stack =
	    (WORD_LIST **)xrealloc (dollar_arg_stack,
				    dollar_arg_stack_slots * sizeof (WORD_LIST **));
	}
    }
  dollar_arg_stack[dollar_arg_stack_index] = list_rest_of_args ();
  dollar_arg_stack[++dollar_arg_stack_index] = (WORD_LIST *)NULL;
}

pop_dollar_vars ()
{
  if (!dollar_arg_stack || !dollar_arg_stack_index)
    return;

  remember_args (dollar_arg_stack[--dollar_arg_stack_index], 1);
  dispose_words (dollar_arg_stack[dollar_arg_stack_index]);
  dollar_arg_stack[dollar_arg_stack_index] = (WORD_LIST *)NULL;
}

/* Remember LIST in $0 ... $9, and REST_OF_ARGS.  If DESTRUCTIVE is
   non-zero, then discard whatever the existing arguments are, else
   only discard the ones that are to be replaced. */
remember_args (list, destructive)
     WORD_LIST *list;
     int destructive;
{
  register int i;

  for (i = 1; i < 10; i++)
    {
      if (destructive && dollar_vars[i])
	{
	  free (dollar_vars[i]);
	  dollar_vars[i] = (char *)NULL;
	}

      if (list)
	{
	  if (!destructive && dollar_vars[i])
	    free (dollar_vars[i]);

	  dollar_vars[i] = savestring (list->word->word);
	  list = list->next;
	}
    }

  /* If arguments remain, assign them to REST_OF_ARGS. */
  if (!list)
    {
      dispose_words (rest_of_args);
      rest_of_args = NULL;
    }
  else
    {
      rest_of_args = (WORD_LIST *)copy_word_list (list);
    }
}

/* Return if LIST is NULL else barf and jump to top_level. */
no_args (list)
     WORD_LIST *list;
{
  if (list)
    {
      builtin_error ("extra arguments");
      longjmp (top_level, DISCARD);
    }
}

/* Shift the arguments ``left''.  Shift DOLLAR_VARS down then take one
   off of REST_OF_ARGS and place it into DOLLAR_VARS[9].  If LIST has
   anything in it, it is a number which says where to start the shifting. */
shift_builtin (list)
     WORD_LIST *list;
{
  int times = get_numeric_arg (list);

  while (times-- > 0) {
    register int count;

    if (dollar_vars[1]) free (dollar_vars[1]);

    for (count = 1; count < 9; count++)
      dollar_vars[count] = dollar_vars[count + 1];

    if (rest_of_args) {
      WORD_LIST *temp = rest_of_args;

      dollar_vars[9] = savestring (temp->word->word);
      rest_of_args = rest_of_args->next;
      dispose_word (temp->word);
    } else {
      dollar_vars[9] = (char *)NULL;
    }
  }
}

/* TEST/[ builtin. */
test_builtin (list)
     WORD_LIST *list;
{
  char **argv;
  int argc, result;
  WORD_LIST *t = list;

  /* We let Matthew Bradburn and Kevin Braunsdorf's code do the
     actual test command.  So turn the list of args into an array
     of strings, since that is what his code wants. */

  if (!list)
    {
      if (strcmp (this_command_name, "[") == 0)
	fprintf (stderr, "[: missing `]'\n");

      return (EXECUTION_FAILURE);
    }

  /* Get the length of the argument list. */
  for (argc = 0; t; t = t->next, argc++);

  /* Account for argv[0] being a command name.  This makes our life easier. */
  argc++;
  argv = (char **)xmalloc ((1 + argc) * sizeof (char *));
  argv[argc] = (char *)NULL;

  /* this_command_name is the name of the command that invoked this
     function.  So you can't call test_builtin () directly from
     within this code, there are too many things to worry about. */
  argv[0] = savestring (this_command_name);

  for (t = list, argc = 1; t; t = t->next, argc++)
    argv[argc] = savestring (t->word->word);

  result = test_command (argc, argv);
  free_array (argv);
  return (result);
}

/* Print the totals for system and user time used.  The
   information comes from variables in jobs.c used to keep
   track of this stuff. */

#if !defined (SYSV)
static long
scale60 (tvalp)
     struct timeval *tvalp;
{
  return (tvalp->tv_sec * 60 + tvalp->tv_usec / 16667);
}
#endif /* !SYSV */

times_builtin (list)
     WORD_LIST *list;
{
  int system_minutes_used, user_minutes_used;
  long system_seconds_used, user_seconds_used;
#ifndef SYSV
  struct rusage self, kids;

  no_args (list);

  getrusage (RUSAGE_SELF, &self);
  getrusage (RUSAGE_CHILDREN, &kids);	/* terminated child processes */

  user_seconds_used = scale60(&self.ru_utime) + scale60(&kids.ru_utime);
  system_seconds_used = scale60(&self.ru_stime) + scale60(&kids.ru_stime);

#else /* SYSV */

#ifndef HZ
#define HZ 100		/* From my Sys V.3.2 manual for times(2) */
#endif /* !HZ */

  struct tms t;
  int sys_seconds, user_seconds;

  no_args (list);

  times (&t);

  /* As of System V.3, HP-UX 6.5, and other ATT-like systems, this stuff is
     returned in terms of clock ticks (HZ from sys/param.h).  C'mon, guys.
     This kind of stupid clock-dependent stuff is exactly the reason 4.2BSD
     introduced the `timeval' struct. */

  system_seconds_used = ((t.tms_stime + HZ - 1) / HZ) +
			((t.tms_cstime + HZ - 1) / HZ);

  user_seconds_used   = ((t.tms_utime + HZ - 1) / HZ) +
			((t.tms_cutime + HZ - 1) / HZ);

#endif /* SYSV */

  system_minutes_used = system_seconds_used / 60;
  system_seconds_used %= 60;

  user_minutes_used = user_seconds_used / 60;
  user_seconds_used %= 60;

  printf ("%0dm%0ds %0dm%0ds\n", system_minutes_used, system_seconds_used,
	  			 user_minutes_used, user_seconds_used);
  return (EXECUTION_SUCCESS);
}

/* The trap command:

   trap <arg> <signal ...>
   trap <signal ...>
   trap -l
   trap

   Set things up so that ARG is executed when SIGNAL(s) N is recieved.
   If ARG is the empty string, then ignore the SIGNAL(s).  If there is
   no ARG, then set the trap for SIGNAL(s) to its original value.  Just
   plain "trap" means to print out the list of commands associated with
   each signal number.  Single arg of "-l" means list the signal names. */

/* Possible operations to perform on the list of signals.*/
#define SET 0			/* Set this signal to first_arg. */
#define REVERT 1		/* Revert to this signals original value. */
#define IGNORE 2		/* Ignore this signal. */

trap_builtin (list)
     WORD_LIST *list;
{
  register int i;

  if (!list)
    {
      for (i = 0; i < NSIG; i++)
	if ((int)(trap_list[i]) > 0)
	  printf ("%d:%s: #(%s)\n", i, trap_list[i], signal_name (i));
      return (EXECUTION_SUCCESS);
    }
  else
    {
      char *first_arg = list->word->word;
      int operation = SET, any_failed = 0;

      if (strcmp ("-l", first_arg) == 0)
	{
	  int column = 0;

	  for (i = 0; i < NSIG; i++)
	    {
	      printf ("%d) %s", i, signal_name (i));
	      if (++column < 4)
		printf ("\t");
	      else
		{
		  printf ("\n");
		  column = 0;
		}
	    }
	  if (column != 0)
	    printf ("\n");
	  return (EXECUTION_SUCCESS);
	}

      if (signal_object_p (first_arg))
	operation = REVERT;
      else
	{
	  list = list->next;
	  if (*first_arg == '\0')
	    operation = IGNORE;
	}

      while (list)
	{
	  int signal = decode_signal (list->word->word);

	  if (signal == NO_SIG)
	    {
	      builtin_error ("%s: not a signal specification", list->word->word);
	      any_failed++;
	    }
	  else
	    {
	      switch (operation)
		{
		case SET:
		  set_signal (signal, first_arg);
		  break;

		case REVERT:
		  restore_default_signal (signal);
		  break;

		case IGNORE:
		  ignore_signal (signal);
		  break;
		}
	    }
	  list = list->next;
	}
      return ((!any_failed) ? EXECUTION_SUCCESS : EXECUTION_FAILURE);
    }
}

/* For each word in LIST, find out what the shell is going to do with
   it as a simple command. i.e., which file would this shell use to
   execve, or if it is a builtin command, or an alias.  Possible flag
   arguments:
   	-type		Returns the "type" of the object, one of
			`alias', `function', `builtin', or `file'.

	-path		Returns the pathname of the file if -type is
			a file.

	-all		Returns all occurrences of words, whether they
	 		be a filename in the path, alias, function,
			or builtin.
   Order of evaluation:
   	alias
	keyword
	function
	builtin
	file
 */
type_builtin (list)
     WORD_LIST *list;
{
  extern Function *find_shell_builtin ();
  extern char *user_command_matches (), *find_user_command ();
  extern struct { char *word; int token; } token_word_alist[];
  int path_only, type_only, all;
  int found_something = 0;
  int found_file = 0;

  path_only = type_only = all = 0;

  while (list && *(list->word->word) == '-')
    {
      char *flag = &(list->word->word[1]);

      if (strcmp (flag, "type") == 0)
	{
	  type_only = 1;
	  path_only = 0;
	}
      else if (strcmp (flag, "path") == 0 || strcmp (flag, "n") == 0)
	{
	  path_only = 1;
	  type_only = 0;
	}
      else if (strcmp (flag, "all") == 0)
	{
	  all = 1;
	}
      else
	{
	  builtin_error ("Bad flag `%s'", flag);
	  return (EXECUTION_FAILURE);
	}
      list = list->next;
    }

  while (list)
    {
      char *command = list->word->word;
      SHELL_VAR *var = find_variable (command);
      int hashed = 0;
      char *full_path = (char *)NULL;

#ifdef ALIAS
      /* Command is an alias? */
      ASSOC *alias = find_alias (command);
      if (alias)
	{
	  if (type_only)
	    printf ("alias\n");
	  else if (!path_only)
	    printf ("%s is aliased to `%s'\n", command, alias->value);
	  found_something++;
	  if (!all)
	    goto next_item;
	}
#endif

      /* Command is a shell reserved word? */
      {
	register int i;

	for (i = 0; token_word_alist[i].word; i++)
	  {
	    if (strcmp (token_word_alist[i].word, command) == 0)
	      {
		if (type_only)
		  printf ("reserved word\n");
		else if (!path_only)
		  printf ("%s is a shell reserved word\n", command);

		found_something++;

		if (!all)
		  goto next_item;

		break;
	      }
	  }
      }
		
      /* Command is a function? */
      if (var && function_p (var))
	{
	  if (type_only)
	    printf ("function\n");
	  else if (!path_only)
	    {
	      printf ("%s is a function\n", command);
	      printf ("%s () ", command);
	      print_command (function_cell (var));
	      printf ("\n");
	    }
	  found_something++;
	  if (!all)
	    goto next_item;
	}

      /* Command is a builtin? */
      if (find_shell_builtin (command))
	{
	  if (type_only)
	    printf ("builtin\n");
	  else if (!path_only)
	    printf ("%s is a shell builtin\n", command);
	  found_something++;
	  if (!all)
	    goto next_item;
	}

      full_path = (char *)NULL;

      /* Command is a disk file? */
      if (absolute_pathname (command))
	{
	  full_path = savestring (command);
	}
      else
	{
	  /* If the user isn't doing "-all", then we might care about
	     whether the file is present in our hash table. */
	  if (!all)
	    {
	      if ((full_path = find_hashed_filename (command)) != (char *)NULL)
		{
		  hashed++;
		  full_path = savestring (full_path);
		}
	      else
		{
		  full_path = find_user_command (command);
		}
	    }
	}
      if (all)
	{
	  /* If full_path was set then it is an absolute path name. */

	  if (full_path)
	    {
	      found_something++;
	      if (type_only)
		printf ("file\n");
	      else if (path_only)
		printf ("%s\n", full_path);
	      else
		printf ("%s is %s\n", command, full_path);
	      
	      free (full_path);
	    }
	  else
	    while (full_path = user_command_matches (command, 1, found_file))
	      {
		found_something++;
		found_file++;
		
		if (type_only)
		  printf ("file\n");
		else if (path_only)
		  printf ("%s\n", full_path);
		else
		  printf ("%s is %s\n", command, full_path);
		
		free (full_path);
	      }
	  
	  if (!found_something && !path_only)
	    printf ("%s not found\n", command);

	  goto next_item;
	}

      /* We can only get here if "-all" was not specified. */
      if (!full_path)
	{
	  if (!type_only && !path_only)
	    printf ("%s not found\n", command);
	}
      else
	{
	  if (type_only)
	    printf ("file\n");
	  else if (path_only)
	    printf ("%s\n", full_path);
	  else
	    {
	      if (hashed)
		printf ("%s is hashed (%s)\n", command, full_path);
	      else
		printf ("%s is %s\n", command, full_path);
	    }

	  free (full_path);
	}
    next_item:
      list = list->next;
      }
  fflush (stdout);
  return (found_something ? EXECUTION_SUCCESS : EXECUTION_FAILURE);
}

/* Declare or change variable attributes. */
declare_builtin (list)
     register WORD_LIST *list;
{
  return (declare_internal (list, 0));
}

local_builtin (list)
     register WORD_LIST *list;
{
  extern int variable_context;

  if (variable_context)
    return (declare_internal (list, 1));
  else
    {
      builtin_error ("Can only be used in a function");
      return (EXECUTION_FAILURE);
    }
}

declare_internal (list, no_modifiers)
     register WORD_LIST *list;
     int no_modifiers;
{
  extern int variable_context, array_needs_making;
  int flags_on = 0, flags_off = 0;
  int any_failed = 0;

  while (list)
    {
      register char *t = list->word->word;
      int *flags;

      if (*t != '+' && *t != '-')
	break;

      if (no_modifiers)
	{
	  builtin_error ("Modifiers not allowed");
	  return (EXECUTION_FAILURE);
	}

      if (*t == '+')
	flags = &flags_off;
      else
	flags = &flags_on;

      t++;

      while (*t)
	{
	  if (*t == 'f')
	    *flags |= att_function, t++;
	  else if (*t == 'x')
	    *flags |= att_exported, t++, array_needs_making = 1;
	  else if (*t == 'r')
	    *flags |= att_readonly, t++;
	  else
	    {
	      builtin_error ("Bad flag `-%c'", *t);
	      return (EXECUTION_FAILURE);
	    }
	}

      list = list->next;
    }

  /* If there are no more arguments left, then we just want to show
     some variables. */
  if (!list)
    {
      /* If we didn't allow modifiers, then this is the `local' command.
	 Perhaps the flag should be called `local_command' instead of
	 `no_modifiers'.  At any rate, only show local variables defined
	 at this context level. */
      if (no_modifiers)
	{
	  extern SHELL_VAR *variable_list;
	  register SHELL_VAR *vlist = variable_list;

	  while (vlist)
	    {
	      if (!invisible_p (vlist) && vlist->context == variable_context)
		print_assignment (vlist);
	      vlist = vlist->next;
	    }
	}
      else
	{
	  if (!flags_on)
	    set_builtin ((WORD_LIST *)NULL);
	  else
	    set_or_show_attributes ((WORD_LIST *)NULL, flags_on);
	}

      fflush (stdout);
      return (EXECUTION_SUCCESS);
    }

  /* There are arguments left, so we are making variables. */
  while (list)
    {
      char *value, *name = savestring (list->word->word);
      int offset = assignment (name);

      if (offset)
	{
	  name[offset] = '\0';
	  value = name + offset + 1;
	}
      else
	{
	  value = "";
	}

      /* If VARIABLE_CONTEXT has a non-zero value, then we are executing
	 inside of a function.  This means we should make local variables,
	 not global ones. */

      if (variable_context)
	make_local_variable (name);

      /* If we are declaring a function, then complain about it in some way.
	 We don't let people make functions by saying `typeset -f foo=bar'. */

      if (flags_on & att_function)
	{
	  builtin_error ("Can't use `-f' to make functions");
	  return (EXECUTION_FAILURE);
	}
      else
	{
	  SHELL_VAR *bind_variable (), *find_variable (), *v;

	  if ((v = find_variable (name)) == (SHELL_VAR *)NULL)
	    v = bind_variable (name, "");

	  /* We are not allowed to rebind readonly variables that
	     already are readonly unless we are turning off the
	     readonly bit. */
	  if (flags_off & att_readonly)
	    flags_on &= ~att_readonly;

	  if (value && readonly_p (v) && (!(flags_off & att_readonly)))
	    {
	      builtin_error ("%s: readonly variable", name);
	      any_failed++;
	      goto hack_next_variable;
	    }
	    
	  v->attributes |= flags_on;
	  v->attributes &= ~flags_off;

	  if (offset)
	    {
	      free (v->value);
	      v->value = savestring (value);
	    }
	}

      stupidly_hack_special_variables (name);

    hack_next_variable:
      free (name);
      list = list->next;
    }
  return ((!any_failed) ? EXECUTION_SUCCESS : EXECUTION_FAILURE);
}

/* Set or display the mask used by the system when creating files. */
umask_builtin (list)
     WORD_LIST *list;
{
  if (list)
    {
      int new_umask;
      new_umask = read_octal (list->word->word);

      /* Note that other shells just let you set the umask to zero
	 by specifying a number out of range.  This is a problem
	 with those shells.  We don't change the umask if the input
	 is lousy. */
      if (new_umask == -1)
	{
	  builtin_error ("`%s' is not an octal number from 000 to 777",
			 list->word->word);
	  return (EXECUTION_FAILURE);
	}
      umask (new_umask);
    }
  else
    {
      /* Display the UMASK for this user. */
      int old_umask = umask (022);
      umask (old_umask);
      printf ("%03o\n", old_umask);
    }
  fflush (stdout);
  return (EXECUTION_SUCCESS);
}

/* Return the octal number parsed from STRING, or -1 to indicate
   that the string contained a bad number. */
int
read_octal (string)
     char *string;
{
  int result = 0;
  int digits = 0;
  while (*string && *string >= '0' && *string < '8')
    {
      digits++;
      result = (result * 8) + *string++ - '0';
    }

  if (!digits || result > 0777 || *string)
    result = -1;

  return (result);
}

/* Remove the variables specified in LIST from VARIABLE_LIST. */
unset_builtin (list)
  WORD_LIST *list;
{
  extern char **non_unsettable_vars;
  int unset_function = 0;
  int any_failed = 0;

  while (list)
    {
      char *name = list->word->word;

      if (strcmp (name, "-f") == 0)
	{
	  list = list->next;
	  unset_function++;
	  continue;
	}

      if (find_name_in_list (name, non_unsettable_vars) > -1)
	{
	  builtin_error ("%s: cannot unset", name);
	  any_failed++;
	}
      else
	{
	  int tem;

	  if (unset_function)
	    tem = unbind_function (list->word->word);
	  else
	    tem = unbind_variable (list->word->word);

	  if (tem == -1)
	    any_failed++;
	  else
	    stupidly_hack_special_variables (list->word->word);
	}
      list = list->next;
    }
  return (any_failed ? EXECUTION_FAILURE : EXECUTION_SUCCESS);
}

/* Run the command mentioned in list directly, without going through the
   normal alias/function/builtin/filename lookup process. */
builtin_builtin (list)
     WORD_LIST *list;
{
  extern Function *find_shell_builtin ();
  Function *function;
  register char *command;

  if (!list)
    return (EXECUTION_SUCCESS);

  command = (list->word->word);
  function = find_shell_builtin (command);

  if (!function)
    {
      builtin_error ("%s: Not a shell builtin!", command);
      return (EXECUTION_FAILURE);
    }
  else
    {
      this_command_name = command;
      list = list->next;
      return ((*function) (list));
    }
}

decrement_variable (var)
     int *var;
{
  *var = *var - 1;
}

/* Run the commands mentioned in LIST without paying attention to shell
   functions. */
int ignore_function_references = 0;

command_builtin (list)
     WORD_LIST *list;
{
  char *string, *string_list ();
  int result;

  begin_unwind_frame ("command_builtin");
  add_unwind_protect (decrement_variable, &ignore_function_references);
  ignore_function_references++;

  string = string_list (list);

  if (!string)
    result = EXECUTION_SUCCESS;
  else
    result = parse_and_execute (string, "command");

  run_unwind_frame ("command_builtin");

  return (result);
}

char tdir[MAXPATHLEN];
/* Return a pretty pathname.  If the first part of the pathname is
   the same as $HOME, then replace that with `~'.  */
char *
polite_directory_format (name)
     char *name;
{
  char *home = get_string_value ("HOME");
  int l = home ? strlen (home) : 0;

  if (l > 1 && strncmp (home, name, l) == 0 && (!name[l] || name[l] == '/'))
    {
      strcpy (tdir + 1, name + l);
      tdir[0] = '~';
      return (tdir);
    }
  else
    return (name);
}

#ifdef PUSHD_AND_POPD

/* Some useful commands whose behaviour has been observed in csh. */

/* The list of remembered directories. */
char **pushd_directory_list = (char **)NULL;

/* Number of existing slots in this list. */
int directory_list_size = 0;

/* Offset to the end of the list. */
int directory_list_offset = 0;

pushd_builtin (list)
     WORD_LIST *list;
{
  char *temp, *current_directory, *get_working_directory ();
  int j = directory_list_offset - 1;
  char direction = '+';

  /* If there is no argument list then switch current and
     top of list. */
  if (!list)
    {
      if (!directory_list_offset)
	{
	  builtin_error ("No other directory");
	  return (EXECUTION_FAILURE);
	}

      current_directory = get_working_directory ("pushd");
      if (!current_directory)
	return (EXECUTION_FAILURE);

      temp = pushd_directory_list[j];
      pushd_directory_list[j] = current_directory;
      goto change_to_temp;
    }
  else
    {
      direction = *(list->word->word);
      if (direction == '+' || direction == '-')
	{
	  int num;
	  if (1 == sscanf (&(list->word->word)[1], "%d", &num))
	    {
	      if (direction == '-')
		num = directory_list_offset - num;

	      if (num > directory_list_offset)
		{
		  if (!directory_list_offset)
		    builtin_error ("Directory stack empty");
		  else
		    builtin_error ("Stack contains only %d directories",
				  directory_list_offset + 1);
		  return (EXECUTION_FAILURE);
		}
	      else
		{
		  /* Rotate the stack num times.  Remember, the
		     current directory acts like it is part of the
		     stack. */
		  temp = get_working_directory ("pushd");

		  if (!num)
		    goto change_to_temp;

		  do
		    {
		      char *top =
			pushd_directory_list[directory_list_offset - 1];

		      for (j = directory_list_offset - 2; j > -1; j--)
			pushd_directory_list[j + 1] = pushd_directory_list[j];

		      pushd_directory_list[j + 1] = temp;

		      temp = top;
		      num--;
		    }
		  while (num);

		  temp = savestring (temp);
		change_to_temp:
		  {
		    int tt = EXECUTION_FAILURE;

		    if (temp)
		      {
			tt = cd_to_string (temp);
			free (temp);
		      }

		    if ((tt == EXECUTION_SUCCESS) &&
			(!find_variable ("pushd_silent")))
		      dirs_builtin ((WORD_LIST *)NULL);

		    return (tt);
		  }
		}
	    }
	}

      /* Change to the directory in list->word->word.  Save the current
	 directory on the top of the stack. */
      current_directory = get_working_directory ("pushd");
      if (!current_directory)
	return (EXECUTION_FAILURE);

      if (cd_builtin (list) == EXECUTION_SUCCESS)
	{
	  if (directory_list_offset == directory_list_size)
	    {
	      if (!pushd_directory_list)
		pushd_directory_list =
		  (char **)xmalloc ((directory_list_size = 10) * sizeof (char *));
	      else
		pushd_directory_list =
		  (char **)xrealloc (pushd_directory_list,
				     (directory_list_size += 10) * sizeof (char *));
	    }
	  pushd_directory_list[directory_list_offset++] = current_directory;

	  if (!find_variable ("pushd_silent"))
	    dirs_builtin ((WORD_LIST *)NULL);

	  return (EXECUTION_SUCCESS);
	}
      else
	{
	  free (current_directory);
	  return (EXECUTION_FAILURE);
	}
    }
}

/* Print the current list of directories on the directory stack. */
dirs_builtin (list)
     WORD_LIST *list;
{
  register int i, format = 0;
  char *temp, *polite_directory_format (), *get_working_directory ();

  /* Maybe do long form? */
  if (list)
    {
      if (strcmp (list->word->word, "-l") == 0)
	format++;

      if (!format || list->next)
	{
	  builtin_error ("usage: dirs [ -l ]");
	  return (EXECUTION_FAILURE);
	}
    }

  /* The first directory printed is always the current working directory. */
  temp = get_working_directory ("dirs");
  if (!temp)
    temp = savestring ("<no directory>");
  printf ("%s ", format ? temp : polite_directory_format (temp));
  free (temp);

  /* Now print any directories in the array. */
  for (i = (directory_list_offset - 1); i > -1; i--)
    printf ("%s ", format ? pushd_directory_list[i] :
	    polite_directory_format (pushd_directory_list[i]));

  printf ("\n");
  fflush (stdout);
  return (EXECUTION_SUCCESS);
}

/* Switch to the directory in NAME.  This uses the cd_builtin to do the work,
   so if the result is EXECUTION_FAILURE then an error message has already
   been printed. */
cd_to_string (name)
     char *name;
{
  WORD_LIST *tlist = make_word_list (make_word (name), NULL);
  int result = (cd_builtin (tlist));
  dispose_words (tlist);
  return (result);
}

/* Pop the directory stack, and then change to the new top of the stack.
   If LIST is non-null it should consist of a word +N or -N, which says
   what element to delete from the stack.  The default is the top one. */
popd_builtin (list)
     WORD_LIST *list;
{
  register int i;
  int which = 0;
  char direction = '+';

  if (list)
    {
      direction = *(list->word->word);

      if ((direction != '+' && direction != '-') ||
	  (1 != sscanf (&((list->word->word)[1]), "%d", &which)))
	{
	  builtin_error ("Bad arg `%s'", list->word->word);
	  return (EXECUTION_FAILURE);
	}
    }

  if (which > directory_list_offset || (!directory_list_offset && !which))
    {
      if (!directory_list_offset)
	builtin_error ("Directory stack empty");
      else
	builtin_error ("Stack contains only %d directories",
		       directory_list_offset + 1);
      return (EXECUTION_FAILURE);
    }

  /* Handle case of no specification, or top of stack specification. */
   if ((direction == '+' && which == 0) ||
       (direction == '-' && which == directory_list_offset))
    {
      i = cd_to_string (pushd_directory_list[directory_list_offset - 1]);
      if (i != EXECUTION_SUCCESS)
	return (i);
      free (pushd_directory_list[--directory_list_offset]);
    }
  else
    {
      /* Since an offset other than the top directory was specified,
	 remove that directory from the list and shift the remainder
	 of the list into place. */

      if (direction == '+')
	i = directory_list_offset - which;
      else
	i = which;

      free (pushd_directory_list[i]);
      directory_list_offset--;

      /* Shift the remainder of the list into place. */
      for (; i < directory_list_offset; i++)
	pushd_directory_list[i] = pushd_directory_list[i + 1];
    }

  if (!find_variable ("pushd_silent"))
    dirs_builtin ((WORD_LIST *)NULL);

  return (EXECUTION_SUCCESS);
}

#endif  /* PUSHD_AND_POPD */

#ifdef ALIAS

/* Hack the alias command in a Korn shell way. */
alias_builtin (list)
     WORD_LIST *list;
{
  int any_failed = 0;

  if (!list)
    {
      register int i;

      if (!aliases)
	return (EXECUTION_FAILURE);

      for (i = 0; aliases[i]; i++)
	print_alias (aliases[i]);
    }
  else
    {
      while (list)
	{
	  register char *value, *name = list->word->word;
	  register int offset;

	  for (offset = 0; name[offset] && name[offset] != '='; offset++);

	  if (offset && name[offset] == '=')
	    {
	      name[offset] = '\0';
	      value = name + offset + 1;

	      add_alias (name, value);
	    }
	  else
	    {
	      ASSOC *t = find_alias (name);
	      if (t)
		print_alias (t);
	      else
		{
		  if (interactive)
		    builtin_error ("`%s' not found", name);
		  any_failed++;
		}
	    }
	  list = list->next;
	}
    }
  return (any_failed ? EXECUTION_FAILURE : EXECUTION_SUCCESS);
}

/* Remove aliases named in LIST from the aliases database. */
unalias_builtin (list)
     register WORD_LIST *list;
{
  register ASSOC *alias;
  int any_failed = 0;

  while (list)
    {
      alias = find_alias (list->word->word);

      if (alias)
	remove_alias (alias->name);
      else
	{
	  if (interactive)
	    builtin_error ("`%s' not an alias", list->word->word);

	  any_failed++;
	}

      list = list->next;
    }
  return (any_failed ? EXECUTION_FAILURE : EXECUTION_SUCCESS);
}

/* Output ALIAS in such a way as to allow it to be read back in. */
print_alias (alias)
     ASSOC *alias;
{
  register int i;

  for (i = 0; alias->value[i] && !whitespace (alias->value[i]); i++);

  if (alias->value[i])
    printf ("alias %s=\"%s\"\n", alias->name, alias->value);
  else
    printf ("alias %s=%s\n", alias->name, alias->value);

  fflush (stdout);
}

#endif  /* ALIAS */

/* Wait for the pid in LIST to stop or die.  If no arguments are given, then
   wait for all of the active background processes of the shell and return
   0.  If a list of pids or job specs are given, return the exit status of
   the last one waited for. */
wait_builtin (list)
     WORD_LIST *list;
{
  extern wait_for_background_pids ();
  extern wait_for_single_pid ();
  extern int job_control;
  int status = EXECUTION_SUCCESS;

  /* We support jobs or pids.
     wait <pid-or-job> [pid-or-job ...] */

  /* But wait without any arguments means to wait for all of the shell's
     currently active background processes. */
  if (!list)
    {
      wait_for_background_pids ();
      return (EXECUTION_SUCCESS);
    }

  while (list)
    {
      int pid;
      if (digit (*(list->word->word)))
	{
	  if (sscanf (list->word->word, "%d", &pid) == 1)
	    status = wait_for_single_pid (pid);
	  else
	    {
	      builtin_error ("`%s' is not a pid or job spec", list->word->word);
	      return (EXECUTION_FAILURE);
	    }
	}
#ifdef JOB_CONTROL
      else if (job_control)
	/* Must be a job spec.  Check it out. */
	{
	  int oldmask = sigblock (sigmask (SIGCHLD));
	  int job = get_job_spec (list);

	  if (job < 0 || job >= job_slots || !jobs[job])
	    {
	      if (job != DUP_JOB)
		builtin_error ("No such job %s", list->word->word);
	      sigsetmask (oldmask);
	      return (EXECUTION_FAILURE);
	    }

	  /* Job spec used.  Wait for the last pid in the pipeline. */

	  sigsetmask (oldmask);

	  status = wait_for_job (job);
	}
#endif /* JOB_CONTROL */
      else
	{
	  builtin_error ("%s is not a pid or legal job spec", list->word->word);
	  status = EXECUTION_FAILURE;
	}
      list = list->next;
    }
  return (status);
}

/* This is a lot like report_error (), but it is for shell builtins instead
   of shell control structures, and it won't ever exit the shell. */
#if defined (HAVE_VPRINTF)
/* VARARGS */
builtin_error (va_alist)
     va_dcl
{
  extern char *this_command_name;
  char *format;
  va_list args;

  fprintf (stderr, "%s: ", this_command_name);
  va_start (args);
  format = va_arg (args, char *);
  vfprintf (stderr, format, args);
  va_end (args);
  fprintf (stderr, "\n");
}

#else

builtin_error (format, arg1, arg2)
     char *format, *arg1, *arg2;
{
  extern char *this_command_name;

  fprintf (stderr, "%s: ", this_command_name);
  fprintf (stderr, format, arg1, arg2);
  fprintf (stderr, "\n");
  fflush (stderr);
}
#endif /* HAVE_VPRINTF */

#ifdef JOB_CONTROL
/* **************************************************************** */
/*								    */
/*			Job Control!				    */
/*								    */
/* **************************************************************** */

extern int job_control;

/* The `jobs' command.  Prints outs a list of active jobs.  If the
   first argument is `-l', then the process id's are printed also. */
jobs_builtin (list)
     WORD_LIST *list;
{
  int long_form = 0;

  if (!job_control)
    return (EXECUTION_SUCCESS);

  if (list && (strcmp (list->word->word, "-l") == 0))
    long_form = 1;

  list_jobs (long_form);
  return (EXECUTION_SUCCESS);
}

/* Suspend the current shell.  Not hard to do. */

static SigHandler *old_cont, *old_tstp;

/* Continue handler. */
sighandler
suspend_continue ()
{
  signal (SIGCONT, old_cont);
  signal (SIGTSTP, old_tstp);
}

/* Suspending the shell.  If -f is the arg, then do the suspend
   no matter what.  Otherwise, complain if a login shell. */
suspend_builtin (list)
     WORD_LIST *list;
{
  extern int shell_pgrp;

  if (!job_control)
    {
       builtin_error ("Cannot suspend a shell without job control");
       return (EXECUTION_FAILURE);
    }

  if (list)
    if (strcmp (list->word->word, "-f") == 0)
      goto do_suspend;

  no_args (list);

  if (login_shell)
    {
      builtin_error ("Can't suspend a login shell");
      longjmp (top_level, DISCARD);
    }

do_suspend:
  old_cont = (SigHandler *)signal (SIGCONT, suspend_continue);
  old_tstp = (SigHandler *)signal (SIGTSTP, SIG_DFL);
  killpg (shell_pgrp, SIGTSTP);
  return (EXECUTION_SUCCESS);
}

/* How to bring a job into the foreground. */
fg_builtin (list)
     WORD_LIST *list;
{
  int fg_bit = 1;
  register WORD_LIST *t = list;

  if (!job_control)
    return (EXECUTION_SUCCESS);

  /* If the last arg on the line is '&', then start this job in the
     background.  Else, fg the job. */

  while (t && t->next)
    t = t->next;

  if (t && strcmp (t->word->word, "&") == 0)
    fg_bit = 0;

  return (fg_bg (list, fg_bit));
}

/* How to put a job into the background. */
bg_builtin (list)
     WORD_LIST *list;
{
  if (!job_control)
    return (EXECUTION_SUCCESS);

  return (fg_bg (list, 0));
}

/* How to put a job into the foreground/background. */
fg_bg (list, foreground)
     WORD_LIST *list;
     int foreground;
{
  int job = get_job_spec (list);
  extern char *this_command_name;

  if (job < 0 || job >= job_slots || !jobs[job])
    {
      if (job != DUP_JOB)
	builtin_error ("No such job %s", list ? list->word->word : "");
      return (EXECUTION_FAILURE);
    }

  /* Or if jobs[job]->pgrp == shell_pgrp. */
  if (jobs[job]->job_control == 0)
    {
      builtin_error ("job %%%d started without job control", job + 1);
      return (EXECUTION_FAILURE);
    }

  if (start_job (job, foreground))
    return (EXECUTION_SUCCESS);
  else
    return (EXECUTION_FAILURE);
}

/* Return the job spec found in LIST. */
get_job_spec (list)
     WORD_LIST *list;
{
  register char *word;
  int job = NO_JOB;
  int substring = 0;

  if (!list)
    return (current_job);

  word = list->word->word;

  if (!*word)
    return (current_job);

  if (*word == '%')
    word++;

  if (digit (*word) && (sscanf (word, "%d", &job) == 1))
    return (job - 1);

  switch (*word) {

  case 0:
  case '%':
  case '+':
    return (current_job);

  case '-':
    return (previous_job);

  case '?':			/* Substring search requested. */
    substring++;
    word++;
    goto find_string;

  default:
  find_string:
    {
      register int i, wl = strlen (word);
      for (i = 0; i < job_slots; i++)
	{
	  if (jobs[i])
	    {
	      register PROCESS *p = jobs[i]->pipe;
	      extern char *strindex ();
	      do
		{
		  if ((substring && strindex (p->command, word)) ||
		      (strncmp (p->command, word, wl) == 0))
		    if (job != NO_JOB)
		      {
			builtin_error ("ambigious job spec: %s", word);
			return (DUP_JOB);
		      }
		    else
		      job = i;

		  p = p->next;
		}
	      while (p != jobs[i]->pipe);
	    }
	}
      return (job);
    }
  }
}

#ifndef CONTINUE_AFTER_KILL_ERROR
#define CONTINUE_OR_FAIL return (EXECUTION_FAILURE)
#else
#define CONTINUE_OR_FAIL goto continue_killing
#endif

/* Here is the kill builtin.  We only have it so that people can type
   kill -KILL %1?  No, if you fill up the process table this way you
   can kill some. */
kill_builtin (list)
     WORD_LIST *list;
{
  int signal = SIGTERM;
  int pid;

  if (!list)
    return (EXECUTION_SUCCESS);

  if (strcmp (list->word->word, "-l") == 0)
    {
      register int i, column = 0;
      for (i = 1; i < NSIG; i++)
	{
	  printf ("%d) %s", i, signal_name (i));
	  if (++column < 4)
	    printf ("\t");
	  else {
	    printf ("\n");
	    column = 0;
	  }
	}
      if (column != 0)
	printf ("\n");
      return (EXECUTION_SUCCESS);
    }

  /* If the user specified a signal, use that. */
  if (*(list->word->word) == '-')
    {
      signal = decode_signal (&(list->word->word)[1]);
      if (signal == NO_SIG)
	{
	  builtin_error ("bad signal spec `%s'", &(list->word->word)[1]);
	  return (EXECUTION_FAILURE);
	}
      else
	{
	  list = list->next;
	}
    }

  while (list)
    {
      if (digit (*(list->word->word)))
	{
	  if (sscanf (list->word->word, "%d", &pid) == 1)
	    {
	      if (kill_pid (pid, signal, 0) < 0)
		goto signal_error;
	    }
	  else
	    {
	      builtin_error ("No such pid %d", pid);
	      CONTINUE_OR_FAIL;
	    }
	}
      else if (job_control)     /* can't kill jobs if not using job control */
	{			/* Must be a job spec.  Check it out. */
	  int oldmask = sigblock (sigmask (SIGCHLD));
	  int job = get_job_spec (list);

	  if (job < 0 || job >= job_slots || !jobs[job])
	    {
	      if (job != DUP_JOB)
		builtin_error ("No such job %s", list->word->word);
	      sigsetmask (oldmask);
	      CONTINUE_OR_FAIL;
	    }

	  /* Job spec used.  Kill the process group. If the job was started
	     without job control, then its pgrp == shell_pgrp, so we have
	     to be careful.  We take the pid of the first job in the pipeline
	     in that case. */
	  if (jobs[job]->job_control)
	    pid = jobs[job]->pgrp;
	  else
	    pid = jobs[job]->pipe->pid;

	  sigsetmask (oldmask);

	  if (kill_pid (pid, signal, 1) < 0)
	    {
	    signal_error:
	      if (errno == EPERM)
		builtin_error ("(%d) - Not owner", pid);
	      else if (errno == ESRCH)
		builtin_error ("(%d) - No such pid", pid);
	      else
		builtin_error ("Invalid signal %d", signal);
	      CONTINUE_OR_FAIL;
	    }
	}
      else
	{
	  builtin_error ("bad process specification `%s'", list->word->word);
	  CONTINUE_OR_FAIL;
	}
    continue_killing:
      list = list->next;
    }
  return (EXECUTION_SUCCESS);
}

#define DETACH
#ifdef DETACH
detach_builtin (list)
     WORD_LIST *list;
{
  int job = NO_JOB;
  JOB_STATE job_state;
  int oldmask;
  PROCESS *process, *pipeline;

  if (!job_control)
    return (EXECUTION_SUCCESS);

  job = get_job_spec (list);

  oldmask = sigblock (sigmask (SIGCHLD));
  job_state = JOBSTATE(job);

  killpg (jobs[job]->pgrp, SIGSTOP);
  while (jobs[job] && JOBSTATE (job) == JRUNNING)
    sigpause (0);

  /* Make the process group of the pipeline the same as init's.  */
  process = pipeline = jobs[job]->pipe;
  do
    {                              
      setpgrp (process->pid, 1);
      process = process -> next;
    }
  while (process != pipeline);

  /* Return the pipeline to the state it was in before we stopped it. */
  if (job_state == JRUNNING)
    {
      process = pipeline;         
      do
	{
	  kill (process->pid, SIGCONT);
	  process = process -> next;
	}
      while (process != pipeline);
    }
      
  /* Remove the job from the job list. */
  delete_job (job);

  sigsetmask (oldmask);
}
#endif /* DETACH */
#endif  /* JOB_CONTROL */
