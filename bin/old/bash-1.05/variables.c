/* variables.c -- Functions for hacking shell variables. */

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
#include <ctype.h>
#include <pwd.h>

#include "shell.h"
#include "flags.h"
#include "version.h"

#ifdef SYSV
struct passwd *getpwuid (), *getpwent ();
#endif

/* The list of shell variables that the user has created, or that came from
   the environment. */
SHELL_VAR *variable_list;

/* The current variable context.  This is really a count of how deep into
   executing functions we are. */
int variable_context = 0;

/* The array of shell assignments which are made only in the environment
   for a single command. */
char **temporary_env = (char **)NULL;

/* Some funky variables which are known about specially.  Here is where
   "$*", "$1", and all the cruft is kept. */
char *dollar_vars[10];
WORD_LIST *rest_of_args = (WORD_LIST *)NULL;

/* The value of $$. */
int dollar_dollar_pid;

/* An array which is passed to commands as their environment.  It is
   manufactured from the overlap of the initial environment and the
   shell variables that are marked for export. */
char **export_env = (char **)NULL;

/* Non-zero means that we have to remake EXPORT_ENV. */
int array_needs_making = 1;

/* The list of variables that may not be unset in this shell. */
char **non_unsettable_vars = (char **)NULL;

#ifdef SYSV
#define DEFAULT_MAIL_PATH "/usr/mail/"
#else
#define DEFAULT_MAIL_PATH "/usr/spool/mail/"
#endif

/* Initialize the shell variables from the current environment. */
initialize_shell_variables (env)
     char *env[];
{
  extern char *primary_prompt, *secondary_prompt;
  char *name, *string;
  int c, char_index;
  int string_index = 0;
  SHELL_VAR *temp_var;

  while (string = env[string_index++])
    {
      char_index = 0;

      name = (char *)alloca (1 + strlen (string));

      while ((c = *string++) && c != '=')
	name[char_index++] = c;

      name[char_index] = '\0';

      /* If exported function, define it now.  Sigh. */
      if (strncmp ("() {", string, 4) == 0)
	{
	  char *eval_string =
	    (char *)xmalloc (3 + strlen (string) + strlen (name));
	  sprintf (eval_string, "%s %s", name, string);
	  parse_and_execute (eval_string, name);
	}
      else
	bind_variable (name, string);

      set_var_auto_export (name);
    }

  /* Remember this pid. */
  dollar_dollar_pid = getpid ();

  /* Now make our own defaults in case the vars that we think are
     important are missing. */
  set_if_not ("PATH", DEFAULT_PATH_VALUE);
  set_var_auto_export ("PATH");

  set_if_not ("TERM", "dumb");
  set_var_auto_export ("TERM");
	
  set_if_not ("PS1", primary_prompt);
  set_if_not ("PS2", secondary_prompt);
  set_if_not ("IFS", " \t\n");

  /* Magic machine types.  Pretty convenient. */
  bind_variable ("HOSTTYPE", HOSTTYPE);

  /* Default MAILPATH, and MAILCHECK. */
  set_if_not ("MAILCHECK", "60");
  if ((get_string_value ("MAIL") == (char *)NULL) &&
      (get_string_value ("MAILPATH") == (char *)NULL))
    {
      extern char *current_user_name;
      char *tem;

      tem = (char *)xmalloc (1 + sizeof (DEFAULT_MAIL_PATH)
			   + strlen (current_user_name));
      strcpy (tem, DEFAULT_MAIL_PATH);
      strcat (tem, current_user_name);

      bind_variable ("MAILPATH", tem);
      free (tem);
    }

  /* Set up $PWD. */
  {
    char *get_working_directory (), *cd;

    cd = get_working_directory ("shell-init");
    if (cd)
      {
	bind_variable ("PWD", cd);
	free (cd);
      }
  }
  
  /* Do some things with shell level. */
  {
    extern int shell_level;
    char new_level[10];
    int old_level;

    set_if_not ("SHLVL", "0");
    set_var_auto_export ("SHLVL");

    sscanf (get_string_value ("SHLVL"), "%d", &old_level);
    shell_level = old_level + 1;
    sprintf (new_level, "%d", shell_level);
    bind_variable ("SHLVL", new_level);
  }

  /* Get the full pathname to THIS shell, and set the BASH variable
     to it. */
  {
    extern char *shell_name, *find_user_command (), *full_pathname ();
    extern int login_shell;
    char *tname = find_user_command (shell_name);
    
    if ((login_shell == 1) && (*shell_name != '/'))
      {
	struct passwd *entry = getpwuid (getuid ());

	if (entry)
	  {
	    /* If HOME doesn't exist, set it. */
	    temp_var = (SHELL_VAR *)find_variable ("HOME");
	    if (!temp_var)
	      {
		temp_var = bind_variable ("HOME", entry->pw_dir);
		temp_var->attributes |= att_exported;
	      }
	    name = savestring (entry->pw_shell);
	  }
	else
	  name = savestring ("a.out");
      }
    else
      {
	if (!tname)
	  {
	    char *make_absolute ();
	    name = make_absolute (shell_name, get_string_value ("PWD"));
	  }
	else
	  {
	    name = full_pathname (tname);
	    free (tname);
	  }
      }

    /* Make the exported environment variable SHELL be whatever the name of
       this shell is.  Note that the `tset' command looks at this variable
       to determine what style of commands to output; if it ends in "csh",
       then C-shell commands are output, else Bourne shell commands. */
    set_if_not ("SHELL", name);
    set_var_auto_export ("SHELL");

    /* Make a variable called BASH, which is the name of THIS shell. */
    temp_var = bind_variable ("BASH", name);
    temp_var->attributes |= att_exported;

    free (name);
  }

  /* Make a variable called BASH_VERSION which contains the version info. */
  {
    char tt[12];
    extern char *dist_version;
    extern int build_version;

    sprintf (tt, "%s.%d", dist_version, build_version);
    bind_variable ("BASH_VERSION", tt);
  }

  /* Set history variables to defaults, and then do whatever we would
     do if the variable had just been set. */
  {
    char *tilde_expand ();
    char *tem = tilde_expand ("~/.bash_history");

    set_if_not ("HISTFILE", tem);
    free (tem);

    set_if_not ("HISTSIZE", "500");
    sv_histsize ("HISTSIZE");
  }

  /* Gee, might as well get parent pid. */
  {
    char aval[10];

    sprintf (aval, "%d", getppid ());
    bind_variable ("PPID", aval);
  }

  non_unsettable ("PATH");
  non_unsettable ("PS1");
  non_unsettable ("PS2");
  non_unsettable ("IFS");

  /* Get the users real user id, and save that in an readonly variable.
     To make the variable *really* readonly, we have added it to a special
     list of vars. */

  sv_uids ();
  set_var_read_only ("UID");
  set_var_read_only ("EUID");

  non_unsettable ("EUID");
  non_unsettable ("UID");
}

/* Add NAME to the list of variables that cannot be unset
   if it isn't already there. */
non_unsettable (name)
     char *name;
{
  register int i;

  if (!non_unsettable_vars)
    {
      non_unsettable_vars = (char **)xmalloc (1 * sizeof (char *));
      non_unsettable_vars[0] = (char *)NULL;
    }

  for (i = 0; non_unsettable_vars[i]; i++)
    if (strcmp (non_unsettable_vars[i], name) == 0)
      return;

  non_unsettable_vars =
    (char **)xrealloc (non_unsettable_vars, (2 + i) * sizeof (char *));
  non_unsettable_vars[i] = savestring (name);
  non_unsettable_vars[i + 1] = (char *)NULL;
}

/* Set NAME to VALUE if NAME has no value. */
set_if_not (name, value)
     char *name, *value;
{
  char *temp = get_string_value (name);

  if (!temp)
    bind_variable (name, value);
}

/* Print LIST (a linked list of shell variables) to stdout
   in such a way that they can be read back in. */
print_var_list (list)
     register SHELL_VAR *list;
{
  while (list)
    {
      if (!invisible_p (list))
	print_assignment (list);
      list = list->next;
    }
}

/* Print the value of a single SHELL_VAR.  No newline is
   output, but the variable is printed in such a way that
   it can be read back in. */
print_assignment (var)
     SHELL_VAR *var;
{
  if (var->value)
    {
      printf ("%s=", var->name);
      print_var_value (var);
      printf ("\n");
    }

  if (var->function)
    {
      printf ("%s=", var->name);
      print_var_function (var);
      printf ("\n");
    }
}

/* Print the value cell of VAR, a shell variable.  Do not print
   the name, nor leading/trailing newline. */
print_var_value (var)
     SHELL_VAR *var;
{
  if (var->value)
    printf ("%s", var->value);
}

/* Print the function cell of VAR, a shell variable.  Do not
   print the name, nor leading/trailing newline. */
print_var_function (var)
     SHELL_VAR *var;
{
  char *named_function_string ();

  if (var->function)
    printf ("%s", named_function_string ((char *)NULL, var->function, 1));
}

/* Look up the variable entry whose name matches STRING.
   Returns the entry or NULL. */
SHELL_VAR *
find_variable (string)
     char *string;
{
  register SHELL_VAR *list = variable_list;

  while (list)
    {
      if (strcmp (string, list->name) == 0)
	return (list);

      list = list->next;
    }
  return ((SHELL_VAR *)NULL);
}

/* Return the string value of a variable.  Return NULL if the variable
   doesn't exist, or only has a function as a value.  Don't cons a new
   string. */
char *
get_string_value (var_name)
     char *var_name;
{
  SHELL_VAR *var = find_variable (var_name);

  if (!var)
    return (char *)NULL;
  else
    return (var->value);
}

/* Create a local variable referenced by NAME. */
SHELL_VAR *
make_local_variable (name)
     char *name;
{
  SHELL_VAR *new_var, *old_var, *bind_variable ();
  int var_has_name ();

  /* local foo; local foo;  is a no-op. */
  {
    old_var = find_variable (name);
    if (old_var && old_var->context == variable_context)
      return (old_var);
  }

  old_var = (SHELL_VAR *)delete_element (&variable_list, var_has_name, name);

  /* If a variable does not already exist with this name, then
     just make a new one. */
  if ((int)old_var == -1)
    {
      new_var = bind_variable (name, "");
    }
  else
    {
      new_var = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));

      new_var->name = savestring (name);
      new_var->value = savestring ("");
      new_var->function = (char *)copy_command (old_var->function);

      new_var->attributes = 0;

      if (exported_p (old_var))
	new_var->attributes |= att_exported;

      new_var->prev_context = old_var;
      new_var->next = variable_list;
      variable_list = new_var;
    }

  new_var->context = variable_context;
  return (new_var);
}

/* Bind a variable name to some string.  This conses up the name
   and value strings. */
SHELL_VAR *
bind_variable (name, value)
     char *name, *value;
{
  SHELL_VAR *entry = find_variable (name);

  
  if (!entry)
    {
      /* Make a new entry for this variable.  Then do the binding. */
      entry = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));

      entry->attributes = 0;

      entry->name = savestring (name);

      if (value)
	entry->value = savestring (value);
      else
	entry->value = (char *)NULL;

      entry->function = (char *)NULL;

      entry->next = variable_list;

      /* Always assume variables are to be made at toplevel!
	 make_local_variable has the responsibilty of changing the
	 variable context. */
      entry->context = 0;
      entry->prev_context = (SHELL_VAR *)NULL;
      variable_list = entry;
    }
  else
    {
      if (readonly_p (entry))
	{
	  report_error ("%s: read-only variable", name);
	  return (entry);
	}

      if (entry->value)
	free (entry->value);

      if (value)
	entry->value = savestring (value);
      else
	entry->value = (char *)NULL;
    }

  if (mark_modified_vars)
    entry->attributes |= att_exported;

  if (exported_p (entry))
    array_needs_making = 1;

  return (entry);
}

/* Dispose of the information attached to a variable. */
dispose_variable (var)
     SHELL_VAR *var;
{
  if (!var)
    return;

  if (var->function)
    dispose_command (var->function);

  if (var->value)
    free (var->value);

  free (var->name);

  if (exported_p (var))
    array_needs_making = 1;

  free (var);
}

/* Return 1 if VAR (a variable) is named NAME (a string). */
var_has_name (var, name)
     SHELL_VAR *var;
     char *name;
{
  if (var)
    return ((strcmp (var->name, name) == 0));
  else
    return (0);
}

/* Unbind the function cell of a variable.  If the variable has
   no function cell, then discard it.  Return -1 if the variable
   couldn't be found. */
unbind_function (name)
     char *name;
{
  SHELL_VAR *var = find_variable (name);

  if (!var)
    return (-1);

  if (var->function)
    {
      dispose_command (var->function);
      var->function = (char *)NULL;
    }

  if (!var->value)
    makunbound (name);

  return (0);
}

/* Unbind the value cell of a variable.  If the variable has
   no value cell, then discard it.  Return -1 if the variable
   couldn't be found. */
unbind_variable (name)
     char *name;
{
  SHELL_VAR *var = find_variable (name);

  if (!var)
    return (-1);

  if (var->value)
    {
      free (var->value);
      var->value = (char *)NULL;
    }

  if (!var->function)
    makunbound (name);

  return (0);
}

/* Make the variable associated with NAME go away.
   Return non-zero if the variable couldn't be found. */
makunbound (name)
     char *name;
{
  SHELL_VAR *elt = 
    (SHELL_VAR *)delete_element (&variable_list, var_has_name, name);

  if ((int)elt != -1)
    {
      if (elt->prev_context)
	{
	  SHELL_VAR *new = elt->prev_context;
	  new->next = variable_list;
	  variable_list = new;
	}
      if (exported_p (elt))
	set_var_auto_export (elt->name);

      dispose_variable (elt);
      return (0);
    }
  return (-1);
}

/* Remove the variable with NAME if it is a local variable in the
   current context. */
kill_local_variable (name)
     char *name;
{
  SHELL_VAR *temp = find_variable (name);

  if (temp && (temp->context == variable_context))
    {
      makunbound (name);
      return (0);
    }
  return (-1);
}

/* Get rid of all of the variables in the current context. */
kill_all_local_variables ()
{
  register SHELL_VAR *list = variable_list;
  register SHELL_VAR *prev = (SHELL_VAR *)NULL;

  while (list)
    {
      if (list->context && list->context == variable_context)
	{

	  if (list->prev_context)
	    list->prev_context->next = list->next;
	  else
	    list->prev_context = list->next;

	  if (prev)
	    {
	      prev->next = list->prev_context;
	      dispose_variable (list);
	      list = prev;
	    }
	  else
	    {
	      variable_list = list->prev_context;
	      dispose_variable (list);
	      list = variable_list;
	    }
	}
      prev = list;
      list = list->next;
    }
}

/* Do a function binding to a variable.  You pass the name and
   the command to bind to.  This conses the name and command. */
SHELL_VAR *
bind_function (name, value)
     char *name;
     COMMAND *value;
{
  SHELL_VAR *entry = find_variable (name);

  if (!entry)
    {
      entry = bind_variable (name, "");
      free (entry->value);
      entry->value = (char *)NULL;
    }

  if (entry->function)
    dispose_command (entry->function);

  entry->function = (char *)copy_command (value);

  entry->attributes |= att_function;

  if (mark_modified_vars)
    entry->attributes |= att_exported;

  array_needs_making = 1;

  return (entry);
}

/* Copy VAR to a new data structure and return that structure. */
SHELL_VAR *
copy_variable (var)
     SHELL_VAR *var;
{
  SHELL_VAR *copy = (SHELL_VAR *)NULL;

  if (var)
    {
      copy = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));
  
      copy->attributes = var->attributes;
      copy->name = savestring (var->name);

      if (var->function)
	copy->function = (char *)copy_command (var->function);

      if (var->value)
	copy->value = savestring (var->value);
      else
	copy->value = (char *)NULL;

      copy->context = var->context;

      /* Don't bother copying previous contexts along with this variable. */
      copy->prev_context = (SHELL_VAR *)NULL;
    }

  return (copy);
}

/* Make the variable associated with NAME be read-only.
   If NAME does not exist yet, create it. */
set_var_read_only (name)
     char *name;
{
  SHELL_VAR *entry = find_variable (name);

  if (!entry)
    {
      entry = bind_variable (name, "");
      if (!no_invisible_vars)
	entry->attributes |= att_invisible;
    }
  entry->attributes |= att_readonly;
}

/* Make the variable associated with NAME be auto-exported.
   If NAME does not exist yet, create it. */
set_var_auto_export (name)
     char *name;
{
  SHELL_VAR *entry = find_variable (name);
  
  if (!entry)
    {
      entry = bind_variable (name, "");
      if (!no_invisible_vars)
	entry->attributes |= att_invisible;
    }
  entry->attributes |= att_exported;
  array_needs_making = 1;
}

/* Returns non-zero if STRING is an assignment statement.  The returned value
   is the index of the `=' sign. */
assignment (string)
     char *string;
{
  register int c, index = 0;

  c = string[index];
  if (!isletter (c) && c != '_')
    return (0);

  while (c = string[index])
    {
      /* The following is safe.  Note that '=' at the start of a word
	 is not an assignment statement. */
      if (c == '=')
	return (index);

      if (!isletter (c) && !digit (c) && c != '_')
	return (0);

      index++;
    }
  return (0);
}

/* Make an array out of LIST, a list of SHELL_VAR.  Only visible
   variables which are marked for export are eligible. */
char **
make_var_array (list)
     SHELL_VAR *list;
{
  register int count = 0;
  register char **array;
  register SHELL_VAR *head = list;
  int hack_it_twice = 0;

  while (list)
    {
      if (exported_p (list) && !invisible_p (list))
	{
	  if (list->function)
	    count++;

	  if (list->value)
	    count++;
	}
      list = list->next;
    }

  array = (char **)xmalloc ((1 + count) * sizeof (char *));
  list = head;
  count = 0;

  while (list)
    {
      if (exported_p (list) && !invisible_p (list))
	{
	  char *value, *named_function_string ();

	  if (list->function && !hack_it_twice)
	    {
	      value =
		named_function_string ((char *)NULL,
				       (COMMAND *)list->function, 0);
	      hack_it_twice++;
	    }
	  else
	    {
	      value = list->value;
	      hack_it_twice = 0;
	    }

	  if (value)
	    {
	      array[count] =
		(char *)xmalloc (2 + strlen (list->name) + strlen (value));
	      sprintf (array[count], "%s=%s", list->name, value);
	      count++;
	    }
	}

      if (!hack_it_twice)
	list = list->next;
    }
  array[count] = (char *)NULL;
  return (array);
}

/* Add STRING to the array of foo=bar strings that we already
   have to add to the environment.  */
assign_in_env (string)
     char *string;
{
  int size;

  int offset = assignment (string);
  char *name = savestring (string);
  char *temp, *value = (char *)NULL;

  if (name[offset] == '=')
    {
      char *tilde_expand (), *string_list ();
      WORD_LIST *list, *expand_string ();
      extern int disallow_filename_globbing;

      name[offset] = 0;
      temp = name + offset + 1;
      if (!disallow_filename_globbing)
	temp = tilde_expand (temp);
      else
	temp = savestring (temp);

      list = expand_string (temp, 0);
      value = string_list (list);

      if (list)
	free (list);

      free (temp);
    }

  if (!value) value = savestring ("");

  temp = (char *)xmalloc (2 + strlen (name) + strlen (value));
  sprintf (temp, "%s=%s", name, value);
  free (name);

  if (!temporary_env)
    {
      temporary_env = (char **)xmalloc (sizeof (char *));
      temporary_env [0] = (char *)NULL;
    }

  size = array_len (temporary_env);
  temporary_env =
    (char **)xrealloc (temporary_env, (size + 2) * (sizeof (char *)));

  temporary_env[size] = (temp);
  temporary_env[size + 1] = (char *)NULL;
  array_needs_making = 1;
}

/* Find a variable in the temporary environment that is named NAME.
   Return a consed variable, or NULL if not found. */
SHELL_VAR *
find_tempenv_variable (name)
     char *name;
{
  register int i, l = strlen (name);

  if (!temporary_env)
    return ((SHELL_VAR *)NULL);

  for (i = 0; temporary_env[i]; i++)
    {
      if (strncmp (temporary_env[i], name, l) == 0 &&
	  temporary_env[i][l] == '=')
	{
	  SHELL_VAR *temp = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));

	  temp->next = (SHELL_VAR *)NULL;
	  temp->name = savestring (name);
	  if (temporary_env[i][l + 1])
	    temp->value = savestring (&temporary_env[i][l + 1]);
	  else
	    temp->value = savestring ("");
	  temp->function = (char *)NULL;
	  temp->attributes = att_exported;
	  temp->context = 0;
	  temp->prev_context = (SHELL_VAR *)NULL;

	  return (temp);
	}
    }
  return ((SHELL_VAR *)NULL);
}

/* Free the storage used in the variable array for temporary
   environment variables. */
dispose_used_env_vars ()
{
  if (!temporary_env)
    return;

  free_array (temporary_env);
  temporary_env = (char **)NULL;
  array_needs_making = 1;
}

/* Stupid comparison routine for qsort () ing strings. */
qsort_string_compare (s1, s2)
     register char **s1, **s2;
{
  return (strcmp (*s1, *s2));
}

/* Add ASSIGN to ARRAY, or supercede a previous assignment in the
   array with the same left-hand side.  Return the new array. */
char **
add_or_supercede (assign, array)
     char *assign;
     register char **array;
{
  register int i;
  int equal_offset = assignment (assign);

  if (!equal_offset)
    return (array);
  
  for (i = 0; array[i]; i++)
    {
      if (strncmp (assign, array[i], equal_offset + 1) == 0)
	{
	  free (array[i]);
	  array[i] = savestring (assign);
	  return (array);
	}
    }
  array = (char **)xrealloc (array, (2 + array_len (array)) * sizeof (char *));
  array[i++] = savestring (assign);
  array[i] = (char *)NULL;
  return (array);
}

/* Make the environment array for the command about to be executed.  If the
   array needs making.  Otherwise, do nothing.  If a shell action could
   change the array that commands receive for their environment, then the
   code should `array_needs_making++'. */
maybe_make_export_env ()
{
  register int i;
  register char **temp_array;

  if (array_needs_making)
    {
      if (export_env)
	free_array (export_env);

#ifdef SHADOWED_ENV
      export_env =
	(char **)xmalloc ((1 + array_len (shell_environment)) * sizeof (char *));

      for (i = 0; shell_environment[i]; i++)
	export_env[i] = savestring (shell_environment[i]);
      export_env[i] = (char *)NULL;

#else /* !SHADOWED_ENV */

      export_env = (char **)xmalloc (sizeof (char *));
      export_env[0] = (char *)NULL;

#endif /* SHADOWED_ENV */

      temp_array = make_var_array (variable_list);
      for (i = 0; temp_array[i]; i++)
	export_env = add_or_supercede (temp_array[i], export_env);
      free_array (temp_array);

      if (temporary_env)
	{
	  for (i = 0; temporary_env[i]; i++)
	    export_env = add_or_supercede (temporary_env[i], export_env);

	  /* Sort the array alphabetically. */
	  qsort (export_env, array_len (export_env),
		 sizeof (char *), qsort_string_compare);
	}
      array_needs_making = 0;
    }
}

/* We supply our own version of getenv () because we want library routines
   to get the changed values of exported variables. */
char *last_tempenv_value = (char *)NULL;

char *
getenv (name)
     char *name;
{
  SHELL_VAR *var = find_tempenv_variable (name);

  if (var)
    {
      if (last_tempenv_value)
	free (last_tempenv_value);
      
      last_tempenv_value = savestring (value_cell (var));
      dispose_variable (var);
      return (last_tempenv_value);
    }
  else
    {
      var = find_variable (name);
      if (var && exported_p (var))
	return (value_cell (var));
    }
  return ((char *)NULL);
}
