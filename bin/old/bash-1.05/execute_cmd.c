/* execute_command.c -- Execute a COMMAND structure. */

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

#include <stdio.h>
#include <sys/types.h>
#ifndef SONY
#include <fcntl.h>
#endif
#include <sys/file.h>
#include <sys/stat.h>
#include <signal.h>

#ifndef SIGABRT
#define SIGABRT SIGIOT
#endif

#include <sys/param.h>
#include <errno.h>

#include "shell.h"
#include "y.tab.h"
#include "builtins.h"
#include "flags.h"
#include "hash.h"

#ifdef JOB_CONTROL
#include "jobs.h"
#endif

#ifdef ALIAS
#include "alias.h"
#endif

extern int breaking, continuing, loop_level;
extern int errno, sys_nerr;
extern char *sys_errlist[];

#ifdef SYSV
extern int last_made_pid;
#endif

extern WORD_LIST *expand_words (), *expand_word ();

/* The value returned by the last synchronous command. */
int last_command_exit_value = 0;

/* The list of redirections to preform which will undo the redirections
   that I made in the shell. */
REDIRECT *redirection_undo_list = (REDIRECT *)NULL;

/* Execute the command passed in COMMAND.  COMMAND is exactly what
   read_command () places into GLOBAL_COMMAND.  See "shell.h" for the
   details of the command structure.

   EXECUTION_SUCCESS or EXECUTION_FAILURE are the only possible
   return values.  Executing a command with nothing in it returns
   success. */
execute_command (command)
     COMMAND *command;
{
  /* Just do the command, but not asynchronously. */
  return (execute_command_internal (command, 0, NO_PIPE, NO_PIPE));
}

/* Returns 1 if TYPE is a shell control structure type. */
int
shell_control_structure (type)
     enum command_type type;
{
  switch (type)
    {
    case cm_for:
    case cm_case:
    case cm_while:
    case cm_until:
    case cm_if:
    case cm_group:
      return (1);

    default:
      return (0);
    }
}

execute_command_internal (command, asynchronous, pipe_in, pipe_out)
     COMMAND *command;
     int asynchronous;
     int pipe_in, pipe_out;
{
  int exec_result;
  REDIRECT *my_undo_list = (REDIRECT *)NULL;

  if (!command || breaking || continuing)
    return (EXECUTION_SUCCESS);

  /* If a command was being explicitly run in a subshell, or if it is
     a shell control-structure, and it has a pipe, then we do the command
     in a subshell. */

  if (command->subshell ||
      (shell_control_structure (command->type) &&
       (pipe_out != NO_PIPE || pipe_in != NO_PIPE || asynchronous)))
    {
      int paren_pid;

      /* Fork a subshell, turn off the subshell bit, turn off job
	 control and call execute_command () on the command again. */
      paren_pid = make_child (savestring (make_command_string (command)),
			      asynchronous);
      if (paren_pid == 0)
	{
	  extern int interactive, login_shell;

	  command->subshell = 0;

	  /* Don't fork again, we are already in a subshell. */
	  asynchronous = 0;

	  /* Subshells are neither login nor interactive. */
	  login_shell = interactive = 0;

#ifdef JOB_CONTROL
	  /* Delete all traces that there were any jobs running.  This is
	     only for subshells. */
	  without_job_control ();
#endif
	  do_piping (pipe_in, pipe_out);
	  if (command->redirects)
	    if (!(do_redirections (command->redirects, 1, 0) == 0))
	      exit (EXECUTION_FAILURE);
	  exit (execute_command_internal
		 (command, asynchronous, NO_PIPE, NO_PIPE));
	}
      else
	{
	  close_pipes (pipe_in, pipe_out);

	  /* If we are part of a pipeline, and not the end of the pipeline,
	     then we should simply return and let the last command in the
	     pipe be waited for.  If we are not in a pipeline, or are the
	     last command in the pipeline, then we wait for the subshell
	     and return its exit status as usual. */
	  if (pipe_out != NO_PIPE)
	    return (EXECUTION_SUCCESS);
	  
	  stop_pipeline (asynchronous, (COMMAND *)NULL);

	  if (!asynchronous)
	    return (last_command_exit_value = wait_for (paren_pid));
	  else
	    {
	      extern int interactive;
	      if (interactive)
		describe_pid (paren_pid);
	      return (EXECUTION_SUCCESS);
	    }
	}
    }

  /* Handle WHILE FOR CASE etc. with redirections.  (Also '&' input
     redirection.)  */
  do_redirections (command->redirects, 1, 1);
  my_undo_list = (REDIRECT *)copy_redirects (redirection_undo_list);

  switch (command->type)
    {
    case cm_for:
      exec_result = execute_for_command (command->value.For);
      break;

    case cm_case:
      exec_result = execute_case_command (command->value.Case);
      break;

    case cm_while:
      exec_result = execute_while_command (command->value.While);
      break;

    case cm_until:
      exec_result = execute_until_command (command->value.While);
      break;

    case cm_if:
      exec_result = execute_if_command (command->value.If);
      break;

    case cm_group:
      if (asynchronous)
	{
	  command->subshell = 1;
	  exec_result =
	    execute_command_internal (command, 1, pipe_in, pipe_out);
	}
      else
	{
	  exec_result =
	    execute_command_internal (command->value.Group->command,
				      asynchronous, pipe_in, pipe_out);
	}
      break;

    case cm_simple:
      {
	extern int last_asynchronous_pid, last_made_pid;
	int last_pid = last_made_pid;
	
#ifdef JOB_CONTROL
	extern int already_making_children;
#endif 
	exec_result =
	  execute_simple_command (command->value.Simple, pipe_in, pipe_out,
				  asynchronous);

	/* If we forked to do the command, then we must
	   wait_for() the child. */
#ifdef JOB_CONTROL
	if (already_making_children && pipe_out == NO_PIPE)
#else
	  if (pipe_out == NO_PIPE)
#endif
	    {
	      if (last_pid != last_made_pid)
		{
		  stop_pipeline (asynchronous, (COMMAND *)NULL);

		  if (asynchronous)
		    {
		      extern int interactive;

		      if (interactive)
			describe_pid (last_made_pid);
		    }
		  else
		    exec_result = wait_for (last_made_pid);
		}
	    }
      }
      break;

    case cm_connection:
      switch (command->value.Connection->connector)
	{
	  /* Do the first command asynchronously. */
	case '&':
	  {
	    COMMAND *tc = command->value.Connection->first;
#ifndef JOB_CONTROL
	    {
	      REDIRECT *tr = 
		make_redirection (0, r_inputa_direction,
				  make_word ("/dev/null"));
	      tr->next = tc->redirects;
	      tc->redirects = tr;
	    }
#endif				/* !JOB_CONTROL */
	    exec_result = execute_command_internal (tc, 1, pipe_in, pipe_out);
	    if (command->value.Connection->second)
	      exec_result =
		execute_command_internal (command->value.Connection->second,
					  asynchronous, pipe_in, pipe_out);
	  }
	  break;

	case ';':
	  /* Just call execute command on both of them. */
	  execute_command (command->value.Connection->first);
	  exec_result =
	    execute_command_internal (command->value.Connection->second,
				      asynchronous, pipe_in, pipe_out);
	  break;

	case '|':
	  {
	    /* Make a pipeline between the two commands. */
	    int fildes[2];
	    if (pipe (fildes) < 0)
	      {
		report_error ("Pipe error %d", errno);
		exec_result = EXECUTION_FAILURE;
	      }
	    else
	      {
		execute_command_internal (command->value.Connection->first,
					  asynchronous, pipe_in, fildes[1]);
		exec_result =
		  execute_command_internal (command->value.Connection->second,
					    asynchronous, fildes[0], pipe_out);
	      }
	  }
	  break;

	case AND_AND:
	  /* Execute the first command.  If the result of that is successful,
	     then execute the second command, otherwise return. */
	  if (execute_command (command->value.Connection->first)
	      != EXECUTION_FAILURE)
	    exec_result = execute_command (command->value.Connection->second);
	  else exec_result = EXECUTION_FAILURE;
	  break;

	case OR_OR:
	  /* Execute the first command.  If the result of that is successfull,
	     then return, otherwise execute the second command. */
	  if (execute_command (command->value.Connection->first)
	      == EXECUTION_FAILURE)
	    exec_result =  execute_command (command->value.Connection->second);
	  else exec_result = EXECUTION_SUCCESS;
	  break;
	
	default:
	  programming_error ("Bad connector `%d'!",
			     command->value.Connection->connector);
	  longjmp (top_level, DISCARD);
	  break;
	}
      break;
      
    case cm_function_def:
      exec_result = intern_function (command->value.Function_def->name,
				     command->value.Function_def->command);
      break;

    default:
      programming_error ("execute_command: Bad command type `%d'!",
			 command->type);
    }

  if (my_undo_list)
    {
      do_redirections (my_undo_list, 1, 0);
      dispose_redirects (my_undo_list);
    }
  return (last_command_exit_value = exec_result);
}

/* Execute a FOR command.  The syntax is: FOR word_desc IN word_list;
   DO command; DONE */
execute_for_command (for_command)
     FOR_COM *for_command;
{
  /* I just noticed that the Bourne shell leaves word_desc bound to the
     last name in word_list after the FOR statement is done.  This seems
     wrong to me; I thought that the variable binding should be lexically
     scoped, i.e. only would last the duration of the FOR command.  This
     behaviour can be gotten by turning on the lexical_scoping switch. */

  extern int breaking, continuing;
  register WORD_LIST *releaser, *list;
  WORD_DESC *temp = for_command->name;
  char *identifier;
  SHELL_VAR *old_value;		/* Remember the old value of x. */

  if (!check_identifier (temp))
    return (EXECUTION_FAILURE);

  loop_level++;
  identifier = temp->word;

  list = releaser = expand_words (for_command->map_list, 0);

  if (lexical_scoping)
    old_value = copy_variable (find_variable (identifier));

  while (list)
    {
      QUIT;
      bind_variable (identifier, list->word->word);
      execute_command (for_command->action);
      QUIT;

      if (breaking)
	{
	  breaking--; 
	  break;
	}

      if (continuing)
	{
	  continuing--;
	  if (continuing)
	    break;
	}

      list = list->next;
    }
  dispose_words (releaser);

  loop_level--;
  
  if (lexical_scoping)
    {
      if (!old_value)
	{
	  makunbound (identifier);
	}
      else
	{
	  SHELL_VAR *new_value;
	  if (function_p (old_value))
	    bind_function (identifier, old_value->function);
	  else
	    bind_variable (identifier, old_value->value);
    
	  new_value = find_variable (identifier);
	  new_value->attributes = old_value->attributes;
	}
      dispose_variable (old_value);
    }
  return (EXECUTION_SUCCESS);
}

/* Execute a CASE command.  The syntax is: CASE word_desc IN pattern_list ESAC.
   The pattern_list is a linked list of pattern clauses; each clause contains
   some patterns to compare word_desc against, and an associated command to
   execute. */
execute_case_command (case_command)
     CASE_COM *case_command;
{
  extern dispose_words ();
  WORD_LIST *wlist = expand_word (case_command->word, 0);
  PATTERN_LIST *clauses = case_command->clauses;
  register WORD_LIST *list;
  char *word = (wlist) ? wlist->word->word : "";

  add_unwind_protect (dispose_words, wlist);
  while (clauses)
    {
      QUIT;
      list = clauses->patterns;
      while (list)
	{
	  WORD_LIST *es = expand_word (list->word, 0);
	  char *pattern = (es) ? es->word->word : "";

	  if (glob_match (pattern, word, 0))
	    {
	      dispose_words (es);
	      execute_command (clauses->action);
	      goto exit_command;
	    }
	  dispose_words (es);
	  list = list->next;
	  QUIT;
	}
      clauses = clauses->next;
    }
 exit_command:
  remove_unwind_protect ();
  dispose_words (wlist);
  return (EXECUTION_SUCCESS);
}

/* The WHILE command.  Syntax: WHILE test DO action; DONE.
   Repeatedly execute action while executing test produces
   EXECUTION_SUCCESS. */
execute_while_command (while_command)
     WHILE_COM *while_command;
{
  extern int breaking;
  extern int continuing;
  int commands_executed = 0;

  loop_level++;

  while (execute_command (while_command->test) == EXECUTION_SUCCESS)
    {
      QUIT;
      commands_executed = 1;
      execute_command (while_command->action);
      QUIT;

      if (breaking)
	{
	  breaking--;
	  break;
	}

      if (continuing)
	{
	  continuing--;
	  if (continuing)
	    break;
	}
    }
  loop_level--;
  return ((commands_executed == 1) ? EXECUTION_SUCCESS : EXECUTION_FAILURE);
}

/* UNTIL is just like WHILE except that the test result is negated. */
execute_until_command (while_command)
     WHILE_COM *while_command;
{
  extern int breaking;
  extern int continuing;
  int commands_executed = 0;

  loop_level++;
  while (execute_command (while_command->test) != EXECUTION_SUCCESS)
    {
      QUIT;
      commands_executed = 1;
      execute_command (while_command->action);
      QUIT;

      if (breaking)
	{
	  breaking--;
	  break;
	}

      if (continuing)
	{
	  continuing--;
	  if (continuing)
	    break;
	}

    }
  loop_level--;
  return (commands_executed);
}

/* IF test THEN command [ELSE command].
   IF also allows ELIF in the place of ELSE IF, but
   the parser makes *that* stupidity transparent. */
execute_if_command (if_command)
     IF_COM *if_command;
{
  if (execute_command (if_command->test) == EXECUTION_SUCCESS)
    {
      QUIT;
      return (execute_command (if_command->true_case));
    }
  else
    {
      QUIT;
      return (execute_command (if_command->false_case));
    }
}

Function *
find_shell_builtin (string)
     char *string;
{
  int i = 0;
  while (shell_builtins[i].name)
    {
      if (shell_builtins[i].enabled &&
	  strcmp (shell_builtins[i].name, string) == 0)
	return (shell_builtins[i].function);
      i++;
    }
  return ((Function *)NULL);
}

/* The name of the command that is currently being executed.
   `test' needs this, for example. */
char *this_command_name;

/* For catching RETURN in a function. */
int return_catch_flag = 0;
int return_catch_value;
jmp_buf return_catch;

/* The meaty part of all the executions.  We have to start
   hacking the real execution of commands here.  Fork a process,
   set things up, execute the command. */
execute_simple_command (simple_command, pipe_in, pipe_out, async)
     SIMPLE_COM *simple_command;
     int pipe_in, pipe_out;
{
  WORD_LIST *expand_words ();
  WORD_LIST *words;

  /* Remember what this command line looks like at invocation. */
  extern int command_string_index;
  extern char *the_printed_command;
  char *command_line;
  int first_word_quoted;

  command_string_index = 0;
  print_simple_command (simple_command);
  command_line = (char *)alloca (1 + strlen (the_printed_command));
  strcpy (command_line, the_printed_command);

  first_word_quoted =
    (simple_command->words? simple_command->words->word->quoted : 0);

  words = expand_words (simple_command->words);

  /* It is possible for WORDS not to have anything left in it.
     Perhaps all the words consisted of `$foo', and there was
     no variable `$foo'. */
  if (words)
    {
      extern Function *last_shell_builtin, *this_shell_builtin;
      extern int ignore_function_references;
      Function *builtin;
      SHELL_VAR *var = find_variable (words->word->word);
      char *auto_resume_value;

      if (echo_command_at_execute)
	{
	  extern char *string_list (), *indirection_level_string ();
	  char *line = string_list (words);

	  if (line && *line)
	    fprintf (stderr, "%s%s\n", indirection_level_string (), line);

	  if (line)
	    free (line);
	}

      if (ignore_function_references)
	var = (SHELL_VAR *)NULL;

      QUIT;
#ifdef JOB_CONTROL
      /* Is this command a job control related thing? */
      if (words->word->word[0] == '%')
	{
	  this_command_name = "fg";
	  return (fg_builtin (words));
	}

      /* One other possiblilty.  The user may want to resume an existing job.
	 If they do, find out whether this word is a candidate for a running
	 job. */
      if ((auto_resume_value = get_string_value ("auto_resume")) &&
	  !first_word_quoted &&
	  !words->next &&
	  words->word->word[0] &&
	  !simple_command->redirects &&
	  pipe_in == NO_PIPE &&
	  pipe_out == NO_PIPE &&
	  !async)
	{
	  char *word = words->word->word;
	  register int i, wl = strlen (word), exact;

	  exact = strcmp (auto_resume_value, "exact") == 0;
	  for (i = job_slots - 1; i > -1; i--)
	    {
	      if (jobs[i])
		{
		  register PROCESS *p = jobs[i]->pipe;
		  do
		    {
		      if ((exact && strcmp (p->command, word) == 0) ||
			  strncmp (p->command, word, wl) == 0)
			{
			  dispose_words (words);
			  return (start_job (i, 1));
			}
		      p = p->next;
		    }
		  while (p != jobs[i]->pipe);
		}
	    }
	}
#endif

      /* Not a running job.  Do normal command processing. */
      maybe_make_export_env ();
      QUIT;

      /* Remember the name of this command globally. */
      this_command_name = words->word->word;

      /* This command could be a shell builtin or a user-defined function.
	 If so, and we have pipes, then fork a subshell in here.  Else, just
	 do the command. */

      if (var && function_p (var))
	builtin = (Function *)NULL;
      else
	{
	  builtin = find_shell_builtin (words->word->word);
	  last_shell_builtin = this_shell_builtin;
	  this_shell_builtin = builtin;
	}

      if (builtin || (var && function_p (var)))
	{
	  if ((pipe_in != NO_PIPE) || (pipe_out != NO_PIPE) || async)
	    {
#ifdef JOB_CONTROL
	      extern int job_control;
	      int old_job_control = job_control;

	      /* Turn off job control before we fork the subshell. */
	      set_job_control (0);
#endif /* JOB_CONTROL */
	      if (make_child (savestring (command_line), async) == 0)
		{
		  do_piping (pipe_in, pipe_out);

		  if (do_redirections (simple_command->redirects, 1, 0) == 0)
		    {
		      if (builtin)
			exit ((*builtin) (words->next));
		      else
			{
			  COMMAND *tc = (COMMAND *)copy_command (function_cell (var));
			  int result;
			  extern int variable_context;

			  remember_args (words->next, 1);
#ifdef JOB_CONTROL
			  stop_pipeline (async, (COMMAND *)NULL);
#endif
			  variable_context++;
			  return_catch_flag++;
			  result = execute_command (tc);
			  dispose_command (tc);
			  variable_context--;
			  exit (result);
			}
		    }
		  else
		    {
		      exit (EXECUTION_FAILURE);
		    }
		}
	      else
		{
		  close_pipes (pipe_in, pipe_out);
#ifdef JOB_CONTROL
		  set_job_control (old_job_control);
#endif
		  return (EXECUTION_SUCCESS);
		}
	    }
	  else
	    {
	      int result = EXECUTION_FAILURE;

 	      if (do_redirections (simple_command->redirects, 1, 1) == 0)
 		{
		  REDIRECT *saved_undo_list = redirection_undo_list;

		  redirection_undo_list = (REDIRECT *)NULL;

 		  if (builtin)
 		    result = ((*builtin) (words->next));
 		  else
 		    {
 		      int return_val;
 		      extern int dispose_command (), pop_context ();
		      jmp_buf old_return_catch;
 		      COMMAND *tc;
		      
		      tc = (COMMAND *)copy_command (function_cell (var));
 
 		      push_context ();
 		      begin_unwind_frame ("function_calling");
 		      add_unwind_protect (pop_context, (char *)NULL);
 		      add_unwind_protect (dispose_command, (char *)tc);
 
		      /* Note the second argument of "1", meaning that
 			 we discard the current value of "$*"!  This
 			 is apparently the right thing. */
		      remember_args (words->next, 1);

 		      return_catch_flag++;
		      bcopy ((char *)return_catch, (char *)old_return_catch,
			     sizeof (jmp_buf));
 		      return_val =  setjmp (return_catch);
 
 		      if (return_val)
			result = return_catch_value;
		      else
			result = execute_command (tc);

 		      run_unwind_frame ("function_calling");
 		      return_catch_flag--;
		      bcopy ((char *)old_return_catch, (char *)return_catch,
			     sizeof (jmp_buf));
 		    }
		  redirection_undo_list = saved_undo_list;
 		}
	      do_redirections (redirection_undo_list, 1, 0);
	      dispose_words (words);
	      return (result);
	    }
	}

      {
	/* Hopefully this command is defined in a disk file somewhere.
	   
	   1) fork ()
	   2) connect pipes
	   3) close file descriptors 3-NOFILE
	   4) look up the command
	   5) do redirections
	   6) execve ()
	   7) If the execve failed, see if the file has executable mode set.
	   If so, and it isn't a directory, then execute its contents as
	   a shell script.
	   
	   Note that the filename hashing stuff has to take place up here,
	   in the parent.  This is probably why the Bourne style shells
	   don't handle it, since that would require them to go through
	   this gnarly hair, for no good reason.
	   */

	char **make_word_array (), *find_user_command (),
	*find_hashed_filename ();

	char *hashed_file, *command, **args;

	hashed_file = find_hashed_filename (words->word->word);

	if (hashed_file)
	  command = savestring (hashed_file);
	else
	  {
	    command = find_user_command (words->word->word);
	    if (command && !hashing_disabled)
	      {
		extern int dot_found_in_search;
		if (!absolute_pathname (words->word->word))
		  remember_filename (words->word->word,
				     command, dot_found_in_search);
		/* Increase the number of hits to 1. */
		find_hashed_filename (words->word->word);
	      }
	  }

	/* We have to make the child before we check for the non-existance
	   of COMMAND, since we want the error messages to be redirected. */
      
	if (make_child (savestring (command_line), async) == 0)
	  {
	    do_piping (pipe_in, pipe_out);
	    {
	      register int i;
	      for (i = 3; i < NOFILE; i++)
		close (i);
	    }
      
	    /* Execve expects the command name to be in args[0].  So we
	       leave it there, in the same format that the user used to
	       type it in. */
	    args = make_word_array (words);

	    if (!command)
	      {
		report_error ("%s: command not found", args[0]);
		exit (EXECUTION_FAILURE);
	      }

	    if (do_redirections (simple_command->redirects, 1, 0) == 0)
	      {
		execve (command, args, export_env);

		/* If we get to this point, then start checking out the file.
		   Maybe it is something we can hack ourselves. */
		{
		  struct stat finfo;
		  extern int errno;

		  if (errno != ENOEXEC)
		    {
		      if ((stat (command, &finfo) == 0) &&
			  ((finfo.st_mode & S_IFMT) == S_IFDIR))
			report_error ("%s: is a directory", args[0]);
		      else
			file_error (command);
		
		      exit (EXECUTION_FAILURE);
		    }
		  else
		    {
		      /* This file is executable.
			 If it begins with #!, then help out people
			 with losing operating systems.  Otherwise,
			 check to see if it is a binary file by seeing
			 if the first line (or upto 30 characters) are
			 in the ASCII set.
			 Execute the contents as shell commands. */
		      extern char *shell_name;
		      int larry = array_len (args) + 1;
		      int i, should_exec = 0;

		      {
			int fd = open (command, O_RDONLY);
			if (fd != -1)
			  {
			    unsigned char sample[80];
			    int sample_len = read (fd, &sample[0], 80);

			    /* Is this supposed to be an executable script? */
			    if (strncmp (sample, "#!", 2) == 0)
			      {
				char *execname;
				int start;

				for (i = 2;
				     whitespace (sample[i]) && i < 80; i++);
				start = i;
				for (; !whitespace (sample[i]) &&
				     sample[i] != '\n' && i < 80;
				     i++);

				execname = (char *)xmalloc (1 + (i - start));
				strncpy (execname, sample + start, i - start);
				execname[i - start] = '\0';

				should_exec = 1;
				shell_name = execname;
			      }
			    else
			      {
				if (sample_len != -1)
				  {
				    for (i = 0; i < sample_len; i++)
				      {
					if (sample[i] == '\n')
					  break;
					if (sample[i] > 128 || sample[i] < ' ')
					  {
					    if (sample[i] == '\t')
					      continue;
					    
					    report_error ("%s: Cannot execute binary file", shell_name);
					    exit (EXECUTION_FAILURE);
					  }
				      }
				  }
			      }
			    close (fd);
			  }
		      }
#ifdef JOB_CONTROL
		      /* Forget about the way that job control was working.
			 We are in a subshell. */
		      without_job_control ();
#endif
#ifdef ALIAS
		      /* Forget about any aliases that we knew of.
			 We are in a subshell. */
		      delete_all_aliases ();
#endif

		      /* Insert the name of this shell into the argument
			 list. */
		      args =
			(char **)xrealloc (args, (1 + larry) * sizeof (char *));
		      for (i = larry - 1; i; i--)
			args[i] = args[i - 1];

		      args[0] = shell_name;
		      args[1] = command;
		      args[larry] = (char *)NULL;

		      if (args[0][0] == '-')
			args[0]++;

		      if (should_exec)
			{
			  struct stat finfo;
			  extern int errno;

			  execve (shell_name, args, export_env);

			  /* Oh, no!  We couldn't even exec this! */

			  if ((stat (shell_name, &finfo) == 0) &&
			      ((finfo.st_mode & S_IFMT) == S_IFDIR))
			    report_error ("%s: is a directory", args[0]);
			  else
			    file_error (shell_name);
		
			  exit (EXECUTION_FAILURE);
			}
		      else
			exit (main (larry, args, export_env));
		    }
		}
	      }
	    else
	      {
		exit (EXECUTION_FAILURE);
	      }
	  }
	else
	  {
	    /* Make sure that the pipes are closed in the parent. */
	    close_pipes (pipe_in, pipe_out);
	    if (command)
	      free (command);
	  }
      }
      dispose_words (words);
      return (EXECUTION_SUCCESS);
    }
  else
    {
      /* Even if there aren't any command names, pretend to do the
	 redirections that are specified.  The user expects the side
	 effects to take place. */
      if (do_redirections (simple_command->redirects, 0, 0) == 0)
	return (last_command_exit_value);
      else
	return (EXECUTION_FAILURE);
    }
}

close_pipes (in, out)
     int in, out;
{
  if (in >= 0) close (in);
  if (out >= 0) close (out);
}
    

/* Redirect input and output to be from and to the specified pipes.
   NO_PIPE and REDIRECT_BOTH are handled correctly. */
do_piping (pipe_in, pipe_out)
     int pipe_in, pipe_out;
{
  if (pipe_in != NO_PIPE) {
    dup2 (pipe_in, 0);
    close (pipe_in);
  }
  if (pipe_out != NO_PIPE) {
    dup2 (pipe_out, 1);
    close (pipe_out);
    if (pipe_out == REDIRECT_BOTH)
      dup2 (1, 2);
  }
}

/* Non-zero means don't overwrite existing files. */
int noclobber = 0;

#define AMBIGUOUS_REDIRECT -1
#define NOCLOBBER_REDIRECT -2
/* Perform the redirections on LIST.  If FOR_REAL, then actually make
   input and output file descriptors, otherwise just do whatever is
   neccessary for side effecting.  INTERNAL says to remember how to
   undo the redirections later, if non-zero.  */
do_redirections (list, for_real, internal)
     REDIRECT *list;
     int for_real, internal;
{
  register int error;
  register REDIRECT *temp = list;

  if (internal && redirection_undo_list)
    {
      dispose_redirects (redirection_undo_list);
      redirection_undo_list = (REDIRECT *)NULL;
    }

  while (temp)
    {
      error = do_redirection (temp, for_real, internal);
      if (error)
	{
	  if (error == AMBIGUOUS_REDIRECT)
	    report_error ("%s: Ambiguous redirect",
			  temp->redirectee.filename->word);
	  else if (error == NOCLOBBER_REDIRECT)
	    report_error ("%s: Cannot clobber existing file\n",
			  temp->redirectee.filename->word);
	  else
	    report_error ("%s: %s",
			  temp->redirectee.filename->word,
			  sys_errlist[error]);
	  return (error);
	}

      temp = temp->next;
    }
  return (0);
}



/* Expand the word in WORD returning a string.  If WORD expands to
   multiple words (or no words), then return NULL. */
char *
redirection_expand (word)
     WORD_DESC *word;
{
  char *string_list (), *result;
  WORD_LIST *make_word_list (), *expand_words_no_vars ();
  WORD_LIST *tlist1, *tlist2;

  tlist1 = make_word_list (copy_word (word), (WORD_LIST *)NULL);
  tlist2 = expand_words_no_vars (tlist1);
  dispose_words (tlist1);

  result = string_list (tlist2);
  dispose_words (tlist2);
  return (result);
}

/* Do the specific redirection requested.  Returns errno in case of error.
   If FOR_REAL is zero, then just do whatever is neccessary to produce the
   appropriate side effects.   REMEMBERING, if non-zero, says to remember
   how to undo each redirection.  */
do_redirection (redirect, for_real, remembering)
     REDIRECT *redirect;
     int for_real, remembering;
{
  WORD_DESC *redirectee = redirect->redirectee.filename;
  int redirector = redirect->redirector;
  char *redirectee_word = 0;
  enum r_instruction ri = redirect->instruction;

  int fd;

  switch (ri)
    {
    case r_output_direction:
    case r_appending_to:
    case r_input_direction:
    case r_inputa_direction:
    case r_err_and_out:		/* command &>filename */

      if (!(redirectee_word = redirection_expand (redirectee)))
	return (AMBIGUOUS_REDIRECT);

      /* If we are in noclobber mode, you are not allowed to overwrite
	 existing files.  Check first. */
      if (noclobber && (ri == r_output_direction ||
			ri == r_appending_to ||
			ri == r_err_and_out))
	{
	  struct stat buf;
	  if (stat (redirectee_word, &buf) == 0)
	    return (NOCLOBBER_REDIRECT);
	}

      fd = open (redirectee_word, redirect->flags, 0666);
      free (redirectee_word);

      if (fd < 0 )
	return (errno);

      if (for_real)
	{
	  struct stat buf;

	  if (remembering)
	    /* Only setup to undo it if the thing to undo is active. */
	    if (fstat (redirector, &buf) == 0)
	      add_undo_redirect (redirector);

	  if (fd != redirector && dup2 (fd, redirector) < 0)
	    return (errno);
	}
      if (fd != redirector)
	close (fd);

      /* If we are hacking both stdout and stderr, do the stderr
	 redirection here. */
      if (redirect->instruction == r_err_and_out)
	{
	  if (for_real)
	    {
	      if (remembering)
		add_undo_redirect (2);
	      dup2 (1, 2);
	    }
	}
      break;

    case r_reading_until:
    case r_deblank_reading_until:
      {
	/* REDIRECTEE is a pointer to a WORD_DESC containing the text of
	   the new input.  Place it in a temporary file. */
	char *document = (char *)NULL;
	int document_index = 0;

	/* Expand the text if the word that was specified had no quoting.
	   Note that the text that we expand is treated exactly as if it
	   were surrounded by double-quotes.  */
	
	if (!redirectee)
	  document = savestring ("");
	else
	  {
	    if (!redirectee->quoted)
	      {
		WORD_LIST *temp_word_list =
		  (WORD_LIST *)expand_string (redirectee->word, 1);

		document = (char *)string_list (temp_word_list);
		if (!document)
		  document = savestring ("");
		dispose_words (temp_word_list);
	      }
	    else
	      {
		document = redirectee->word;
	      }
	    document_index = strlen (document);

	    {
	      char filename[40];
	      int pid = getpid ();

	      /* Make the filename for the temp file. */
	      sprintf (filename, "/tmp/t%d-sh", pid);
	
	      fd = open (filename, O_TRUNC | O_WRONLY | O_CREAT, 0666);
	      if (fd < 0)
		{
		  if (!redirectee->quoted)
		    free (document);
		  return (errno);
		}
	      write (fd, document, document_index);
	      close (fd);
	      if (!redirectee->quoted)
		free (document);

	      /* Make the document really temporary.  Also make it the
		 input. */
	      fd = open (filename, O_RDONLY, 0666);

	      if (unlink (filename) < 0 || fd < 0)
		return (errno);

	      if (for_real)
		{
		  if (remembering)
		    add_undo_redirect (redirector);

		  if (dup2 (fd, redirector) < 0)
		    return (errno);
		}
	      close (fd);
	    }
	  }
      }
      break;

    case r_duplicating:
      if (for_real)
	{
	  if (remembering)
	    add_undo_redirect (redirector);

	  /* This is correct.  2>&1 means dup2 (1, 2); */
	  dup2 ((int)redirectee, redirector);
	}
      break;

    case r_close_this:
      if (for_real)
	{
	  if (remembering)
	    add_undo_redirect (redirector);
	  close (redirector);
	}
      break;
    }
  return (0);
}

/* Remember the file descriptor associated with the slot FD,
   on REDIRECTION_UNDO_LIST.  Note that the list will be reversed
   before it is executed. */
add_undo_redirect (fd)
     int fd;
{
  int new_fd = dup (fd);
  REDIRECT *new_redirect, *closer;

  if (new_fd < 0)
    {
      file_error ("redirection error");
      return (-1);
    }
  else
    {
      closer = make_redirection (new_fd, r_close_this, 0);
      new_redirect = make_redirection (fd, r_duplicating, new_fd);
      new_redirect->next = closer;
      closer->next = redirection_undo_list;
      redirection_undo_list = new_redirect;
    }
  return (0);
}
  
intern_function (name, function)
     WORD_DESC *name;
     COMMAND *function;
{
  if (!check_identifier (name))
    return (EXECUTION_FAILURE);
  bind_function (name->word, function);
  return (EXECUTION_SUCCESS);
}

/* Make sure that identifier is a valid shell identifier, i.e.
   does not contain a dollar sign, nor is quoted in any way.  Nor
   does it consist of all digits. */
check_identifier (word)
     WORD_DESC *word;
{
  if (word->dollar_present || word->quoted || all_digits (word->word)) {
    report_error ("`%s' is not a valid identifier", word->word);
    return (0);
  } else return (1);
}

all_digits (string)
     char *string;
{
  while (*string) {
    if (!digit (*string)) return (0);
    else string++;
  }
  return (1);
}

#define u_mode_bits(x) (((x) & 0000700) >> 6)
#define g_mode_bits(x) (((x) & 0000070) >> 3)
#define o_mode_bits(x) (((x) & 0000007) >> 0)
#define X_BIT(x) (x & 1)

/* Non-zero if the last call to executable_file () found
   the file, but stated that it wasn't executable. */
int file_exists_p = 0;

/* Return non-zero if FILE is an executable file, otherwise 0.
   Note that this function is the definition of what an
   executable file is; do not change this unless YOU know
   what an executable file is. */
executable_file (file)
     char *file;
{
  struct stat finfo;
  int user_id;

  /* If the file doesn't exist, or is a directory, then we are
     not interested. */
  file_exists_p = !stat (file, &finfo);
  if (!file_exists_p || (finfo.st_mode & S_IFDIR))
    return (0);

  /* By definition, the only other criteria is that the file has
     an execute bit set that we can use. */
  user_id = geteuid ();

  /* If we are the owner of the file, the owner execute bit applies. */
  if (user_id == finfo.st_uid)
    return (X_BIT (u_mode_bits (finfo.st_mode)));

  /* If we are in the owning group, the group permissions apply. */
  if (group_member (finfo.st_gid))
    return (X_BIT (g_mode_bits (finfo.st_mode)));

  /* If `others' have execute permission to the file, then so do we,
     since we are also `others'. */
  return (X_BIT (o_mode_bits (finfo.st_mode)));
}

#ifndef SYSV
/* The number of groups (within 64) that this user is a member of. */
static int default_group_array_size = 0;
static int ngroups = 0;
static int *group_array = (int *)NULL;
#endif

/* Return non-zero if GID is one that we have in our groups list. */
group_member (gid)
     int gid;
{
#ifdef SYSV
  return ((gid == getgid ()) || (gid == geteuid ()));
#else

  register int i;

  /* getgroups () returns the number of elements that it was able to
     place into the array.  We simply continue to call getgroups ()
     until the number of elements placed into the array is smaller than
     the physical size of the array. */

  while (ngroups == default_group_array_size)
    {
      default_group_array_size += 64;

      if (!group_array)
	group_array = (int *)xmalloc (default_group_array_size * sizeof (int));
      else
	group_array =
	  (int *)xrealloc (group_array,
			   default_group_array_size * sizeof (int));

      ngroups = getgroups (default_group_array_size, group_array);
    }

  /* In case of error, the user loses. */
  if (ngroups < 0)
    return (0);

  /* Search through the list looking for GID. */
  for (i = 0; i < ngroups; i++)
    if (gid == group_array[i])
      return (1);

  return (0);
#endif  /* SYSV */
}

/* DOT_FOUND_IN_SEARCH becomes non-zero when find_user_command ()
   encounters a `.' as the directory pathname while scanning the
   list of possible pathnames; i.e., if `.' comes before the directory
   containing the file of interest. */
int dot_found_in_search = 0;

/* Locate the executable file referenced by NAME, searching along
   the contents of the shell PATH variable.
   Return a new string which is the full pathname to the file,
   or NULL if the file couldn't be found.
   If a file is found that isn't executable, and that is the only
   match, then return that. */
char *
find_user_command (name)
     char *name;
{
  char *find_user_command_internal ();

  return (find_user_command_internal (name, 1));
}

/* Locate the file referenced by NAME, searching along
   the contents of the shell PATH variable.
   Return a new string which is the full pathname to the file,
   or NULL if the file couldn't be found.
   This returns the first file found. */
char *
find_path_file (name)
     char *name;
{
  char *find_user_command_internal ();

  return (find_user_command_internal (name, 0));
}

char *
find_user_command_internal (name, must_be_executable)
     char *name;
     int must_be_executable;
{
  char *path_list;
  char *find_user_command_in_path ();

  path_list = get_string_value ("PATH");
  if (!path_list) return (savestring (name));

  return (find_user_command_in_path (name, path_list, must_be_executable));
}

char *
user_command_matches (name, must_be_executable, state)
     char *name;
     int must_be_executable;
     int state;
{
  register int i;
  char *path_list;
  int  path_index;
  char *path_element;
  char *match;
  static char **match_list = NULL;
  static int match_list_size = 0;
  static int match_index = 0;
  char *extract_colon_unit ();
  
  if (!state)
    {
      /* Create the list of matches. */
      if (!match_list)
	{
	  match_list = 
	    (char **) xmalloc ((match_list_size = 5) * sizeof(char *));

	  for (i = 0; i < match_list_size; i++)
	    match_list[i] = 0;
	}
      
      /* Clear out the old match list. */
      for (i = 0; i < match_list_size; i++)
	match_list[i] = NULL;

      /* We haven't found any files yet. */
      match_index = 0;

      path_list = get_string_value ("PATH");
      path_index = 0;

      while (path_element = extract_colon_unit (path_list, &path_index))
	{
	  char *find_user_command_in_path ();

	  match =
	    find_user_command_in_path (name, path_element, must_be_executable);

	  free (path_element);

	  if (!match)
	    continue;
      
	  if (match_index + 1 == match_list_size)
	    match_list =
	      (char **)xrealloc (match_list,
				 ((match_list_size += 10) + 1) * sizeof (char *));
	  match_list[match_index++] = match;
	  match_list[match_index] = (char *)NULL;
	}

      /* We haven't returned any strings yet. */
      match_index = 0;
    }

  match = match_list[match_index];

  if (match)
    match_index++;

  return(match);
}

/* This does the dirty work for find_path_file ()
   and find_user_command (). */
char *
find_user_command_in_path (name, path_list, must_be_executable)
     char *name;
     char *path_list;
     int must_be_executable;
{
  extern char *extract_colon_unit ();
  extern int file_exists_p;
  char *full_path;
  char *path;
  int path_index = 0;

  /* The file name which we would try to execute, except that it isn't
     possible to execute it.  This is the first file that matches the
     name that we are looking for while we are searching $PATH for a
     suitable one to execute.  If we cannot find a suitable executable
     file, then we use this one. */
  char *file_to_lose_on = (char *)NULL;

  /* We haven't started looking, so we certainly haven't seen
     a `.' as the directory path yet. */
  dot_found_in_search = 0;

  if (absolute_pathname (name))
    {
      full_path = (char *)xmalloc (1 + strlen (name));
      strcpy (full_path, name);

      if (executable_file (full_path) || file_exists_p)
	{
	  return (full_path);
	}
      else
	{
	  free (full_path);
	  return ((char *)NULL);
	}
    }

  while (path_list && path_list[path_index])
    {
      path = extract_colon_unit (path_list, &path_index);
      if (!*path || !*path)
	{
	  free (path);
	  path = savestring ("."); /* by definition. */
	}

      if (!disallow_filename_globbing && *path == '~')
	{
	  char *tilde_expand ();
	  char *t = tilde_expand (path);
	  free (path);
	  path = t;
	}

      /* Remember the location of "." in the path. */
      if (strcmp (path, ".") == 0)
	dot_found_in_search = 1;

      full_path = (char *)xmalloc (2 + strlen (path) + strlen (name));
      sprintf (full_path, "%s/%s", path, name);
      free (path);

      if (executable_file (full_path) ||
	  (!must_be_executable && file_exists_p))
	{
	  if (file_to_lose_on)
	    free (file_to_lose_on);
	  return (full_path);
	}
      else
	{
	  if (file_exists_p && !file_to_lose_on)
	    file_to_lose_on = full_path;
	  else
	    free (full_path);
	}
    }
  /* If we found a file with the right name, but not one that is
     executable, then return the one with the right name. */
  if (file_to_lose_on)
    return (file_to_lose_on);
  else
    return (char *)NULL;
}

/* Given a string containing units of information separated by colons,
   return the next one pointed to by INDEX, or NULL if there are no more.
   Advance INDEX to the character after the colon. */
char *
extract_colon_unit (string, index)
     char *string;
     int *index;
{
  int i, start;

  i = *index;

  if ((i >= strlen (string)) || !string)
    return ((char *)NULL);

  if (string[i] == ':')
    i++;

  start = i;

  while (string[i] && string[i] != ':') i++;

  *index = i;

  if (i == start)
    {
      if (!string[i])
	return ((char *)NULL);

      (*index)++;

      return (savestring (""));
    }
  else
    {
      char *value;

      value = (char *)xmalloc (1 + (i - start));
      strncpy (value, &string[start], (i - start));
      value [i - start] = '\0';

      return (value);
    }
}
