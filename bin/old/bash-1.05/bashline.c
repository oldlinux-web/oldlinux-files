/* bashline.c -- Bash's interface to the readline library. */

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
#include <readline/readline.h>
#include "config.h"
#include "general.h"
#include "variables.h"
#include "builtins.h"
#include "quit.h"

/* Called once from parse.y if we are going to use readline. */
initialize_readline ()
{
  char **attempt_shell_completion (), *bash_tilde_expand ();
  int shell_expand_line (), insert_last_arg (), bash_symbolic_link_hook ();
  char *get_string_value ();

  rl_terminal_name = get_string_value ("TERM");
  rl_instream = stdin, rl_outstream = stderr;
  rl_special_prefixes = "$@%";

  /* Bind up our special shell functions. */
  rl_add_defun ("shell-expand-line", shell_expand_line, META(CTRL('E')));
  rl_add_defun ("insert-last-argument", insert_last_arg, META('.'));
  rl_add_defun ("insert-last-argument", insert_last_arg, META('_'));

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = (Function *)attempt_shell_completion;

  /* Tell the tilde expander that we want a crack if it fails. */
  rl_tilde_expander = (Function *)bash_tilde_expand;

  /* Tell the completer that we might want to follow symbolic links. */
  rl_symbolic_link_hook = (Function *)bash_symbolic_link_hook;

  /* And don't forget to allow conditional parsing of the ~/.inputrc
     file. */
  rl_readline_name = "Bash";
}

/* Contains the line to push into readline. */
char *push_to_readline = (char *)NULL;

/* Push the contents of push_to_readline into the
   readline buffer. */
bash_push_line ()
{
  if (push_to_readline)
    {
      rl_insert_text (push_to_readline);
      free (push_to_readline);
      push_to_readline = (char *)NULL;
    }
}

/* Call this to set the initial text for the next line to read
   from readline. */
bash_re_edit (line)
     char *line;
{
  if (push_to_readline)
    free (push_to_readline);

  push_to_readline = savestring (line);
  rl_startup_hook = bash_push_line;
}

/* **************************************************************** */
/*								    */
/*			     Readline Stuff  			    */
/*				        			    */
/* **************************************************************** */

/* If the user requests hostname completion, then simply build a list
   of hosts, and complete from that forever more. */
#ifndef ETCHOSTS
#define ETCHOSTS "/etc/hosts"
#endif

/* The kept list of hostnames. */
static char **hostname_list = (char **)NULL;

/* The physical size of the above list. */
static int hostname_list_size = 0;

/* The length of the above list. */
static int hostname_list_length = 0;

/* Whether or not HOSTNAME_LIST has been initialized. */
int hostname_list_initialized = 0;

/* Non-zero means that HOSTNAME_LIST needs to be sorted. */
static int hostname_list_needs_sorting = 0;

/* Initialize the hostname completion table. */
initialize_hostname_list ()
{
  char *temp = get_string_value ("hostname_completion_file");

  if (!temp)
    temp = ETCHOSTS;

  snarf_hosts_from_file (temp);
  sort_hostname_list ();
  if (hostname_list)
    hostname_list_initialized++;
}

/* Add NAME to the list of hosts. */
add_host_name (name)
     char *name;
{
  if (hostname_list_length + 2 > hostname_list_size)
    {
      if (!hostname_list)
	hostname_list = (char **)xmalloc (sizeof (char *));

      hostname_list = (char **)
	xrealloc (hostname_list,
		  (1 + (hostname_list_size += 100)) * sizeof (char *));
    }

  hostname_list[hostname_list_length] = savestring (name);
  hostname_list[++hostname_list_length] = (char *)NULL;
  hostname_list_needs_sorting++;
}

/* After you have added some names, you should sort the list of names. */
sort_hostname_list ()
{
  extern int qsort_string_compare ();
  if (hostname_list_needs_sorting && hostname_list)
    qsort (hostname_list, hostname_list_length,
	   sizeof (char *), qsort_string_compare);
  hostname_list_needs_sorting = 0;
}

#define cr_whitespace(c) ((c) == '\r' || (c) == '\n' || whitespace(c))

snarf_hosts_from_file (filename)
     char *filename;
{
  FILE *file = fopen (filename, "r");
  char *temp, buffer[256], name[256];
  register int i, start;

  if (!file)
    return;

  while (temp = fgets (buffer, 255, file))
    {
      /* Skip to first character. */
      for (i = 0; buffer[i] && cr_whitespace (buffer[i]); i++);

      /* If comment, ignore. */
      if (buffer[i] ==  '#')
	continue;

      /* Skip internet address. */
      for (; buffer[i] && !cr_whitespace (buffer[i]); i++);

      /* Gobble up names.  Each name is separated with whitespace. */
      while (buffer[i] && buffer[i] != '#')
	{
	  for (; i && cr_whitespace (buffer[i]); i++);
	  if (buffer[i] ==  '#')
	    continue;
	  for (start = i; buffer[i] && !cr_whitespace (buffer[i]); i++);
	  if ((i - start) == 0)
	    continue;
	  strncpy (name, buffer + start, i - start);
	  name[i - start] = '\0';
	  add_host_name (name);
	}
    }
  fclose (file);
}

/* Return a NULL terminated list of hostnames which begin with TEXT.
   Initialize the hostname list the first time if neccessary.
   The array is malloc ()'ed, but not the individual strings. */
char **
hostnames_matching (text)
     char *text;
{
  register int i, len = strlen (text);
  register int begin, end;
  int last_search = -1;
  char **result = (char **)NULL;

  if (!hostname_list_initialized)
    {
      initialize_hostname_list ();

      if (!hostname_list_initialized)
	return ((char **)NULL);
    }

  sort_hostname_list ();

  /* The list is sorted.  Do a binary search on it for the first character
     in TEXT, and then grovel the names of interest. */
  begin = 0; end = hostname_list_length;

  /* Special case.  If TEXT consists of nothing, then the whole list is
     what is desired. */
  if (!*text)
    {
      result = (char **)xmalloc ((1 + hostname_list_length) * sizeof (char *));
      for (i = 0; i < hostname_list_length; i++)
	result[i] = hostname_list[i];
      result[i] = (char *)NULL;
      return (result);
    }

  /* Scan until found, or failure. */
  while (end != begin)
    {
      int r;

      i = ((end - begin) / 2) + begin;
      if (i == last_search)
	break;

      if (hostname_list[i] &&
	  (r = strncmp (hostname_list[i], text, len)) == 0)
	{
	  while (strncmp (hostname_list[i], text, len) == 0 && i) i--;
	  if (strncmp (hostname_list[i], text, len) != 0) i++;

	  begin = i;
	  while (hostname_list[i] &&
		 strncmp (hostname_list[i], text, len) == 0) i++;
	  end = i;

	  result = (char **)xmalloc ((1 + (end - begin)) * sizeof (char *));
	  for (i = 0; i + begin < end; i++)
	    result[i] = hostname_list[begin + i];
	  result[i] = (char *)NULL;
	  return (result);
	}

      last_search = i;

      if (r < 0)
	begin = i;
      else
	end = i;
    }
  return ((char **)NULL);
}

/* This is a ksh-style insert-last-arg function.  The difference is that bash
   puts stuff into the history file before expansion and file name generation,
   so we deal with exactly what the user typed.  Those wanting the other
   behavior, at least for the last arg, can use `$_'.  This also `knows' about
   how rl_yank_nth_arg treats `$'. */
insert_last_arg(count, c)
     int count, c;
{
  extern int rl_explicit_arg;

  if (rl_explicit_arg)
    return (rl_yank_nth_arg (count, c));
  else
    return (rl_yank_nth_arg ('$', c));
}

/* **************************************************************** */
/*								    */
/*			How To Do Shell Completion      	    */
/*								    */
/* **************************************************************** */

/* Do some completion on TEXT.  The indices of TEXT in RL_LINE_BUFFER are
   at START and END.  Return an array of matches, or NULL if none. */
char **
attempt_shell_completion (text, start, end)
     char *text;
     int start, end;
{
  int in_command_position = 0;
  char **matches = (char **)NULL;
  char *command_separator_chars = ";|&{(";

  /* Determine if this could be a command word. */
  if (start == 0)
    in_command_position++;
  else
    {
      register int ti = start - 1;

      while (whitespace (rl_line_buffer[ti]) && ti > -1)
	ti--;

      if (ti < 0)
	in_command_position++;
      else
	{
	  if (member (rl_line_buffer[ti], command_separator_chars) ||
	      member (*text, command_separator_chars))
	    in_command_position++;
	}
    }

  /* Variable name? */
  if (*text == '$')
    {
      char *variable_completion_function ();
      matches = completion_matches (text, variable_completion_function);
    }

  /* If the word starts in `~', and there is no slash in the word, then
     try completing this word as a username. */
  if (!matches && *text == '~' && !index (text, '/'))
    {
      char *username_completion_function ();
      matches = completion_matches (text, username_completion_function);
    }

  /* Another one.  Why not?  If the word starts in '@', then look through
     the world of known hostnames for completion first. */
  if (!matches && *text == '@')
    {
      char *hostname_completion_function ();
      matches = completion_matches (text, hostname_completion_function);
    }

  /* And last, (but not least) if this word is in a command position, then
     complete over possible command names, including aliases, functions,
     and command names. */

  if (!matches && in_command_position)
    {
      char *command_word_completion_function ();
      int text_offset = 0;

      if (start && member (*text, command_separator_chars))
	text_offset++, start++;

      if (*text != '/')
	matches = completion_matches (&text[text_offset],
				      command_word_completion_function);
    }
  return (matches);
}

/* This is the function to call when the word to complete is at the start
   of a line.  It grovels $PATH, looking for commands that match.  It also
   scans for aliases, function names, and the shell_builtin table. */
char *
command_word_completion_function (hint_text, state)
     char *hint_text;
     int state;
{
  static char *hint = (char *)NULL;
  static char *path = (char *)NULL;
  static char *val = (char *)NULL;
  static int path_index, hint_len, istate;
  static char *filename_hint = (char *)NULL;
  static int mapping_over = 0;

  static int local_index;
  static SHELL_VAR *varlist;

  char *filename_completion_function ();
  char *extract_colon_unit ();

  /* We have to map over the possibilities for command words.  If we have
     no state, then make one just for that purpose. */

  if (!state)
    {
      if (hint) free (hint);

      path = get_string_value ("PATH");
      path_index = 0;

      hint = savestring (hint_text);
      hint_len = strlen (hint);

      mapping_over = 0;
      val = (char *)NULL;

      /* Initialize the variables for each type of command word. */
      local_index = 0;
      varlist = variable_list;
    }

  /* mapping_over says what we are currently hacking.  Note that every case
     in this list must fall through when there are no more possibilities. */

  switch (mapping_over)
    {
    case 0:			/* aliases come first. */
#ifdef ALIAS
      while (aliases && aliases[local_index])
	{
	  local_index++;
	  if (strncmp (aliases[local_index - 1]->name, hint, hint_len) == 0)
	    return (savestring (aliases[local_index - 1]->name));
	}
      local_index = 0;
#endif				/* ALIAS */
      mapping_over++;

    case 1:			/* Then function names. */
      while (varlist)
	{
	  if (function_p (varlist) && !invisible_p (varlist) &&
	      strncmp (varlist->name, hint, hint_len) == 0)
	    {
	      char *temp = savestring (varlist->name);
	      varlist = varlist->next;
	      return (temp);
	    }
	  else
	    varlist = varlist->next;
	}
      mapping_over++;

    case 2:			/* Then shell builtins. */
      while (shell_builtins[local_index].function)
	{
	  local_index++;
	  if (strncmp (shell_builtins[local_index - 1].name,
		       hint, hint_len) == 0)
	    return (savestring (shell_builtins[local_index - 1].name));
	}
      mapping_over++;
      local_index = 0;
    }

  /* Repeatadly call filename_completion_function while we have
     members of PATH left.  Question:  should we stat each file?
     Answer: we call executable_file () on each file. */
 outer:

  istate = (val != (char *)NULL);

  if (!istate)
    {
      char *current_path;

      /* Get the next directory from the path.  If there is none, then we
	 are all done. */
      if (!path ||
	  !path[path_index] ||
	  !(current_path = extract_colon_unit (path, &path_index)))
	return ((char *)NULL);

      if (!*current_path)
	{
	  free (current_path);
	  current_path = savestring (".");
	}

      if (filename_hint)
	free (filename_hint);

      filename_hint =
	(char *)xmalloc (2 + strlen (current_path)
		         + strlen (hint));
      sprintf (filename_hint, "%s/%s", current_path, hint);

      free (current_path);
    }

 inner:
  val = filename_completion_function (filename_hint, istate);
  istate = 1;

  if (!val)
    {
      goto outer;
    }
  else
    {
      char *rindex (), *temp = rindex (val, '/');
      temp++;
      if ((strncmp (hint, temp, hint_len) == 0) && executable_file (val))
	{
	  temp = (savestring (temp));
	  free (val);
	  val = "";		/* So it won't be NULL */
	  return (temp);
	}
      else
	{
	  free (val);
	  goto inner;
	}
    }
}

/* Okay, now we write the entry_function for variable completion. */
char *
variable_completion_function (text, state)
     int state;
     char *text;
{
  static char *varname = (char *)NULL;
  static SHELL_VAR *list;
  static int namelen;

  if (!state)
    {
      if (varname)
	free (varname);
      varname = savestring (&text[1]);
      namelen = strlen (varname);
      list = variable_list;
    }

  while (list)
    {
      /* Compare.  You can't do better than Zayre.  No text is also
	 a match.  */
      if (!invisible_p (list) &&
	  (!*varname || (strncmp (varname, list->name, namelen) == 0)))
	break;
      list = list->next;
    }

  if (!list)
    {
      /* Then we are done. */
      return ((char *)NULL);
    }
  else
    {
      char *value = (char *)xmalloc (2 + strlen (list->name));
      *value = '$';
      strcpy (&value[1], list->name);
      list = list->next;
      return (value);
    }
}

/* How about a completion function for hostnames? */
char *
hostname_completion_function (text, state)
     int state;
     char *text;
{
  static char **list = (char **)NULL;
  static int list_index = 0;

  /* If we don't have any state, make some. */
  if (!state)
    {
      extern char **hostnames_matching ();

      if (list)
	free (list);
      list = (char **)NULL;

      list = hostnames_matching (*text ? &text[1] : &text[0]);
      list_index = 0;
    }

  if (list && list[list_index])
    {
      char *t = (char *)xmalloc (2 + strlen (list[list_index]));

      *t = *text;
      strcpy (t + 1, list[list_index]);
      list_index++;
      return (t);
    }
  else
    return ((char *)NULL);
}

/* History and alias expand the line.  But maybe do more?  This
   is a test to see what users like.  Do expand_string on the string. */
shell_expand_line (ignore)
     int ignore;
{
  char *pre_process_line (), *new_line;

  new_line = pre_process_line (rl_line_buffer, 0, 0);

  if (new_line)
    {
      int old_point = rl_point;
      int at_end = rl_point == rl_end;

      /* If the line was history and alias expanded, then make that
	 be one thing to undo. */

      if (strcmp (new_line, rl_line_buffer) != 0)
	{
	  rl_point = rl_end;

	  rl_add_undo (UNDO_BEGIN, 0, 0, 0);
	  rl_kill_text (0, rl_point);
	  rl_point = rl_end = 0;
	  rl_insert_text (new_line);
	  rl_add_undo (UNDO_END, 0, 0, 0);
	}

      free (new_line);

      /* If there is variable expansion to perform, do that as a separate
	 operation to be undone. */
      {
	char *expand_string (), *expanded_string;
	char *string_list ();

	expanded_string = expand_string (rl_line_buffer, 0);
	if (!expanded_string)
	  new_line = savestring ("");
	else
	  {
	    new_line = string_list (expanded_string);
	    dispose_words (expanded_string);
	  }

      if (strcmp (new_line, rl_line_buffer) != 0)
	{
	  rl_add_undo (UNDO_BEGIN, 0, 0 ,0);
	  rl_kill_text (0, rl_end);
	  rl_point = rl_end = 0;
	  rl_insert_text (new_line);
	  rl_add_undo (UNDO_END, 0, 0, 0);
	}

      free (new_line);

      /* Place rl_point where we think it should go. */
      if (at_end)
	rl_point = rl_end;
      else if (old_point < rl_end)
	{
	  rl_point = old_point;
	  if (!whitespace (rl_line_buffer[rl_point]))
	    rl_forward_word (1);
	}
      }
    }
  else
    {
      /* There was an error in expansion.  Let the preprocessor print
	 the error here.  Note that we know that pre_process_line ()
	 will return NULL, since it just did. */
      fprintf (rl_outstream, "\n\r");
      pre_process_line (rl_line_buffer, 1, 0);
      rl_forced_update_display ();
    }
}

/* If tilde_expand hasn't been able to expand the text, perhaps it
   is a special shell expansion.  We handle that here. */
char *
bash_tilde_expand (text)
     char *text;
{
  char *result = (char *)NULL;

  if (strcmp (text, "-") == 0)
    result = get_string_value ("OLDPWD");
  else if (strcmp (text, "+") == 0)
    result = get_string_value ("PWD");

  if (result)
    result = savestring (result);

  return (result);
}

/* Handle symbolic link references while hacking completion. */
bash_symbolic_link_hook (dirname)
     char **dirname;
{
  extern int follow_symbolic_links;
  char *make_absolute (), *temp_dirname;

  if (follow_symbolic_links && (strcmp (*dirname, ".") != 0))
    {
      temp_dirname = make_absolute (*dirname, get_working_directory (""));

      if (temp_dirname)
	{
	  free (*dirname);
	  *dirname = temp_dirname;
	}
    }
}
