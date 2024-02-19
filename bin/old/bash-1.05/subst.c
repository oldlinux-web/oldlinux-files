/* substitutions.c -- The part of the shell that does parameter,
   command, and globbing substitutions. */

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
#include <pwd.h>
#include <fcntl.h>
#include "shell.h"
#include "flags.h"
#include "alias.h"
#include <readline/history.h>

/* The size that strings change by. */
#define DEFAULT_ARRAY_SIZE 512

/* Some forward declarations. */

extern WORD_LIST *expand_string (), *expand_word (), *list_string ();
extern char *string_list ();
extern WORD_DESC *make_word ();


/* **************************************************************** */
/*								    */
/*			Utility Functions			    */
/*								    */
/* **************************************************************** */


/* Cons a new string from STRING starting at START and ending at END,
   not including END. */
char *
substring (string, start, end)
     char *string;
     int start, end;
{
  register int len = end - start;
  register char *result = (char *)xmalloc (len + 1);

  strncpy (result, string + start, len);
  result[len] = '\0';
  return (result);
}

/* Just like string_extract, but doesn't hack backslashes or any of
   that other stuff. */
char *
string_extract_verbatim (string, sindex, charlist)
     char *string, *charlist;
     int *sindex;
{
  register int i = *sindex;
  int c;
  char *temp;

  while ((c = string[i]) && (!member (c, charlist))) i++;
  temp = (char *)xmalloc (1 + (i - *sindex));
  strncpy (temp, string + (*sindex), i - (*sindex));
  temp[i - (*sindex)] = '\0';
  *sindex = i;
  return (temp);
}

/* Extract a substring from STRING, starting at INDEX and ending with
   one of the characters in CHARLIST.  Don't make the ending character
   part of the string.  Leave INDEX pointing at the ending character.
   Understand about backslashes in the string. */
char *
string_extract (string, sindex, charlist)
     char *string, *charlist;
     int *sindex;
{
  register int c, i = *sindex;
  char *temp;

  while (c = string[i]) {
    if (c == '\\')
      if (string[i + 1])
	i++;
      else
	break;
    else
      if (member (c, charlist))
	break;
    i++;
  }
  temp = (char *)xmalloc (1 + (i - *sindex));
  strncpy (temp, string + (*sindex), i - (*sindex));
  temp[i - (*sindex)] = '\0';
  *sindex = i;
  return (temp);
}

/* Remove backslashes which are quoting backquotes from STRING.  Modifies
   STRING, and returns a pointer to it. */
char *
de_backslash (string)
     char *string;
{
  register int i, l = strlen (string);

  for (i = 0; i < l; i++)
    if (string[i] == '\\' && (string[i + 1] == '`' || string[i] == '\\'))
      strcpy (&string[i], &string[i + 1]);
  return (string);
}

/* Remove instances of \! from a string. */
void
unquote_bang (string)
     char *string;
{
  register int i, j;
  register char *temp = (char *)alloca (1 + strlen (string));

  for (i = 0, j = 0; (temp[j] = string[i]); i++, j++)
    {
      if (string[i] == '\\' && string[i + 1] == '!')
	{
	  temp[j] = '!';
	  i++;
	}
    }
  strcpy (string, temp);
}

/* Extract the $( construct in STRING, and return a new string.
   Start extracting at (SINDEX) as if we had just seen "$(".
   Make (SINDEX) get the position just after the matching ")". */
char *
extract_command_subst (string, sindex)
     char *string;
     int *sindex;
{
  register int i, c, l;
  int pass_character, paren_level;
  int delimiter, delimited_paren_level;
  char *result;

  pass_character = delimiter = delimited_paren_level = 0;
  paren_level = 1;

  for (i = *sindex; c = string[i]; i++)
    {
      if (pass_character)
	{
	  pass_character = 0;
	  continue;
	}

      if (c == '\\')
	{
	  if ((delimiter == '"') &&
	      (member (string[i + 1], slashify_in_quotes)))
	    {
	    pass_next_character:
	      pass_character++;
	      continue;
	    }
	}

      if (!delimiter || delimiter == '"')
	{
	  if (c == '$' && (string[i + 1] == '('))
	    {
	      if (!delimiter)
		paren_level++;
	      else
		delimited_paren_level++;
	      goto pass_next_character;
	    }

	  if (c == ')')
	    {
	      if (delimiter && delimited_paren_level)
		delimited_paren_level--;

	      if (!delimiter)
		{
		  paren_level--;
		  if (paren_level == 0)
		    break;
		}
	    }
	}

      if (delimiter)
	{
	  if (c == delimiter)
	    delimiter = 0;
	  continue;
	}
      else
	{
	  if (c == '"' || c == '\'' || c == '\\')
	    delimiter = c;
	}
    }

  l = i - *sindex;
  result = (char *)xmalloc (1 + l);
  strncpy (result, &string[*sindex], l);
  result[l] = '\0';
  *sindex = i;

  if (!c && (delimiter || paren_level))
    {
      report_error ("Bad command substitution: `$(%s'", result);
      free (result);
      longjmp (top_level, DISCARD);
    }

  return (result);
}

/* An artifact for extracting the contents of a quoted string.  Since the
   string is about to be evaluated, we pass everything through, and only
   strip backslash before backslash or quote. */
char *
string_extract_double_quoted (string, sindex)
     char *string;
     int *sindex;
{
  register int c, j, i;
  char *temp = (char *)xmalloc (1 + strlen (string + (*sindex)));

  for (j = 0, i = *sindex; ((c = string[i]) && (c != '"')); i++)
    {
      if (c == '\\')
	{
	  c = string[++i];
	  if (c != '\\' && c != '"')
	    temp[j++] = '\\';
	}
      temp[j++] = c;
    }
  temp[j] = '\0';
  *sindex = i;
  return (temp);
}

/* Extract the name of the variable to bind to from the assignment string. */
char *
assignment_name (string)
     char *string;
{
  int offset = assignment (string);
  char *temp;
  if (!offset) return (char *)NULL;
  temp = (char *)xmalloc (offset + 1);
  strncpy (temp, string, offset);
  temp[offset] = '\0';
  return (temp);
}

/* Return a single string of all the words in LIST. */
char *
string_list (list)
     WORD_LIST *list;
{
  char *result = (char *)NULL;

  while (list)
    {
      if (!result)
	result = savestring ("");

      result =
	(char *)xrealloc (result, 3 + strlen (result) + strlen (list->word->word));
      strcat (result, list->word->word);
      if (list->next) strcat (result, " ");
      list = list->next;
    }
  return (result);
}

/* Return the list of words present in STRING.  Separate the string into
   words at any of the characters found in SEPARATORS.  If QUOTED is
   non-zero then word in the list will have its quoted flag set, otherwise
   the quoted flag is left as make_word () deemed fit. */
WORD_LIST *
list_string (string, separators, quoted)
     register char *string, *separators;
     int quoted;
{
  WORD_LIST *result = (WORD_LIST *)NULL;
  char *current_word = (char *)NULL;
  int sindex =0;

  while (string[sindex] &&
	 (current_word =
	  string_extract_verbatim (string, &sindex, separators)))
    {
      if (strlen (current_word))
	{
	  register char *temp_string;

 	  for (temp_string = current_word; *temp_string; temp_string++)
	    if ((unsigned char)(*temp_string) == (unsigned char)0x80)
	      {
		strcpy (temp_string, temp_string + 1);
		temp_string--;
	      }

	  result = make_word_list (make_word (current_word), result);
	  if (quoted)
	    result->word->quoted++;
	}
      free (current_word);
      if (string[sindex]) sindex++;
    }
  return (WORD_LIST *)reverse_list (result);
}
  
/* Given STRING, an assignment string, get the value of the right side
   of the `=', and bind it to the left side. */
do_assignment (string)
     char *string;
{
  int offset = assignment (string);
  char *name = savestring (string);
  char *value = (char *)NULL;
  SHELL_VAR *entry = (SHELL_VAR *)NULL;

  if (name[offset] == '=')
    {
      char *temp, *tilde_expand (), *string_list ();
      WORD_LIST *list, *expand_string ();
      extern int disallow_filename_globbing;

      name[offset] = 0;
      temp = name + offset + 1;
      if (!disallow_filename_globbing)
	temp = tilde_expand (temp);
      else
	temp = savestring (temp);

      list = expand_string (temp, 0);
      if (list)
	{
	  value = string_list (list);
	  dispose_words (list);
	}
      free (temp);
    }

  if (!value) value = savestring ("");
  entry = bind_variable (name, value);

  /* Yes, here is where the special shell variables get tested for.
     Don't ask me, I just work here.  This is really stupid.  I would
     swear, but I've decided that that is an impolite thing to do in
     source that is to be distributed around the net, even if this code
     is totally brain-damaged. */

  /* if (strcmp (name, "PATH") == 0) Yeeecchhh!!!*/
  stupidly_hack_special_variables (name);

  if (entry)
    entry->attributes &= ~att_invisible;
  if (value) free (value);
  free (name);
}

/* Most of the substitutions must be done in parallel.  In order
   to avoid using tons of unclear goto's, I have some functions
   for manipulating malloc'ed strings.  They all take INDEX, a
   pointer to an integer which is the offset into the string
   where manipulation is taking place.  They also take SIZE, a
   pointer to an integer which is the current length of the
   character array for this string. */

/* Append SOURCE to TARGET at INDEX.  SIZE is the current amount
   of space allocated to TARGET.  SOURCE can be NULL, in which
   case nothing happens.  Gets rid of SOURCE by free ()ing it.
   Returns TARGET in case the location has changed. */
char *
sub_append_string (source, target, index, size)
     char *source, *target;
     int *index, *size;
{
  if (source)
    {
      if (!target)
	target = (char *)xmalloc (*size = DEFAULT_ARRAY_SIZE);

      while (strlen (source) >= (*size - *index))
	target = (char *)xrealloc (target, *size += DEFAULT_ARRAY_SIZE);

      strcat (target, source);
      *index += strlen (source);

      free (source);
    }
  return (target);
}

/* Append the textual representation of NUMBER to TARGET.
   INDEX and SIZE are as in SUB_APPEND_STRING. */
char *
sub_append_number (number, target, index, size)
     int number, *index, *size;
     char *target;
{
  char *temp = (char *)xmalloc (10);
  sprintf (temp, "%d", number);
  return (sub_append_string (temp, target, index, size));
}

/* Return the word list that corresponds to `$*'. */
WORD_LIST *
list_rest_of_args ()
{
  register WORD_LIST *list = (WORD_LIST *)NULL;
  register WORD_LIST *args = rest_of_args;
  int i;

  for (i = 1; i < 10; i++)
    if (dollar_vars[i])
      list = make_word_list (make_word (dollar_vars[i]), list);
  while (args)
    {
      list = make_word_list (make_word (args->word->word), list);
      args = args->next;
    }
  return ((WORD_LIST *)reverse_list (list));
}

/* Make a single large string out of the dollar digit variables,
   and the rest_of_args. */
char *
string_rest_of_args ()
{
  register WORD_LIST *list = list_rest_of_args ();
  char *string = string_list (list);

  dispose_words (list);
  return (string);
}

/* Expand STRING just as if you were expanding a word.  This also returns
   a list of words.  Note that filename globbing is *NOT* done for word
   or string expansion, just when the shell is expanding a command. */
WORD_LIST *
expand_string (string, quoted)
     char *string;
     int quoted;
{
  WORD_DESC *make_word (), *temp = make_word (string);
  WORD_LIST *expand_word ();
  WORD_LIST *value = expand_word (temp, quoted);

  dispose_word (temp);
  dequote (value);
  return (value);
}

/* I'm going to have to rewrite expansion because filename globbing is
   beginning to make the entire arrangement ugly.  I'll do this soon. */
dequote (list)
     register WORD_LIST *list;
{
  register char *s;

  while (list)
    {
      s = list->word->word;
      while (*s)
	{
	  *s &= 0x7f;
	  s++;
	}
      list = list->next;
    }
}

/* How to quote and dequote the character C. */
#define QUOTE_CHAR(c)	((unsigned char)(c) | 0x80)
#define DEQUOTE_CHAR(c) ((unsigned char)(c) & 0x7f)

/* Quote the string s. */
quote_string (s)
     char *s;
{
  register unsigned char *t = (unsigned char *) s;

  for ( ; t && *t ; t++)
    *t |= 0x80;
}

/* Quote the entire WORD_LIST list. */
quote_list (list)
     WORD_LIST *list;
{
  register WORD_LIST *w;

  for (w = list; w; w = w->next)
    {
      quote_string (w->word->word);
      w->word->quoted = 1;
    }
}

/* Remove the portion of PARAM matched by PATTERN according to OP, where OP
   can have one of 4 values:
	RP_LONG_LEFT	remove longest matching portion at start of PARAM
	RP_SHORT_LEFT	remove shortest matching portion at start of PARAM
	RP_LONG_RIGHT	remove longest matching portion at end of PARAM
	RP_SHORT_RIGHT	remove shortest matching portion at end of PARAM
*/

#define RP_LONG_LEFT	1
#define RP_SHORT_LEFT	2
#define RP_LONG_RIGHT	3
#define RP_SHORT_RIGHT	4

static char *
remove_pattern (param, pattern, op)
     char *param, *pattern;
     int op;
{
  register int len = param ? strlen (param) : 0;
  register char *end = param + len;
  register char *p, *ret, c;

  if (pattern == NULL || *pattern == '\0')	/* minor optimization */
    return (savestring (param));

  if (param == NULL || *param == '\0')
    return (param);

  switch (op)
    {
      case RP_LONG_LEFT:	/* remove longest match at start */
	for (p = end; p >= param; p--)
	  {
	    c = *p; *p = '\0';
	    if (glob_match (pattern, param, 0))
	      {
		*p = c;
		return (savestring (p));
	      }
	    *p = c;
	  }
	break;

      case RP_SHORT_LEFT:	/* remove shortest match at start */
	for (p = param; p <= end; p++)
	  {
	    c = *p; *p = '\0';
	    if (glob_match (pattern, param, 0))
	      {
		*p = c;
		return (savestring (p));
	      }
	    *p = c;
	  }
	break;

      case RP_LONG_RIGHT:	/* remove longest match at end */
	for (p = param; p <= end; p++)
	  {
	    if (glob_match (pattern, p, 0))
	      {
		c = *p;
		*p = '\0';
		ret = savestring (param);
		*p = c;
		return (ret);
	      }
	  }
	break;

      case RP_SHORT_RIGHT:	/* remove shortest match at end */
	for (p = end; p >= param; p--)
	  {
	    if (glob_match (pattern, p, 0))
	      {
		c = *p;
		*p = '\0';
		ret = savestring (param);
		*p = c;
		return (ret);
	      }
	  }
	break;
    }
  return (savestring (param));	/* no match, return original string */
}
	
WORD_LIST *
expand_word (word, quoted)
     WORD_DESC *word;
     int quoted;
{
  WORD_LIST *expand_word_internal ();
  WORD_LIST *result = expand_word_internal (word, quoted);
  if (result)
    dequote (result);
  return (result);
}

/* Make a word list which is the expansion of the word passed in WORD.
   This returns a WORD_LIST * if the expansion expands to something,
   else it returns NULL.  If QUOTED is non-zero, then the text of WORD is
   treated as if it was surrounded by double-quotes.  */
WORD_LIST *
expand_word_internal (word, quoted)
     WORD_DESC *word;
     int quoted;
{
  extern int last_command_exit_value;

  /* The thing that we finally output. */
  WORD_LIST *result = (WORD_LIST *)NULL;

  /* The intermediate string that we build while expanding. */
  char *istring = (char *)xmalloc (DEFAULT_ARRAY_SIZE);

  /* The current size of the above object. */
  int istring_size = DEFAULT_ARRAY_SIZE;

  /* Index into ISTRING. */
  int istring_index = 0;

  /* Temporary string storage. */
  char *temp = (char *)NULL;

  /* The text of WORD. */
  register char *string = word->word;

  /* The index into STRING. */
  register int sindex = 0;

  register int c;		/* Current character. */
  int number;			/* Temporary number value. */
  int t_index;			/* For calls to string_extract_xxx. */
  extern int interactive;

  istring[0] = '\0';

  if (!string) goto final_exit;

  /* Begin the expansion. */

  for (;;) {

    c = string[sindex];

    switch (c) {		/* Case on toplevel character. */

    case '\0':
      goto finished_with_string;

    case '$':

      c = string[++sindex];

      /* Do simple cases first... */

      switch (c) {		/* Case on what follows '$'. */

      case '0':			/* $0 .. $9? */
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
#define SINGLE_DIGIT_COMPATIBILITY /* I'm beginning to hate Bourne. */
#ifdef SINGLE_DIGIT_COMPATIBILITY
	if (dollar_vars[digit_value (c)])
	  temp = savestring (dollar_vars[digit_value (c)]);
	else
	  temp = (char *)NULL;
#else /* We read the whole number, not just the first digit. */
#endif
	goto dollar_add_string;

      case '$':			/* $$ -- pid of the invoking shell. */
	{
	  extern int dollar_dollar_pid;
	  number = dollar_dollar_pid;
	}
add_number:
	temp = (char *)xmalloc (10);
	sprintf (temp, "%d", number);
dollar_add_string:
	if (string[sindex]) sindex++;

	/* Add TEMP to ISTRING. */
add_string:
	istring = sub_append_string (temp, istring,
				     &istring_index, &istring_size);
	break;
	
      case '#':			/* $# -- number of positional parameters. */
	{
	  WORD_LIST *list = list_rest_of_args ();
	  number = list_length (list);
	  dispose_words (list);
	  goto add_number;
	}
	break;
      
      case '?':			/* $? -- return value of the last synchronous command. */
	{
	  number = last_command_exit_value;
	  goto add_number;
	}
	break;
      
      case '-':			/* $- -- flags supplied to the shell on
				   invocation or by the `set' builtin. */
	temp = (char *)which_set_flags ();
	goto dollar_add_string;
	break;
      
      case '!':			/* $! -- Pid of the last asynchronous command. */
	{
	  extern int last_asynchronous_pid;
	  number = last_asynchronous_pid;
	  goto add_number;
	}
	break;
      
	/* The only difference between this and $@ is when the arg is
	   quoted. */
      case '*':			/* `$*' */
	temp = string_rest_of_args ();

	/* In the case of a quoted string, quote the entire arg-list.
	   "$1 $2 $3". */
	if (quoted && temp)
	  quote_string (temp);
	goto dollar_add_string;
	break;
	
	/* When we have "$@" what we want is "$1" "$2" "$3" ... This 
	   means that we have to turn quoting off after we split into
	   the individually quoted arguments so that the final split
	   on the first character of $IFS is still done.  */
      case '@':			/* `$@' */
	{
	  WORD_LIST *tlist = list_rest_of_args ();
	  if (quoted && tlist)
	    {
	      quote_list (tlist);
	      quoted = 0;	/* This *should* be safe */
	    }
	  temp = string_list (tlist);
	  goto dollar_add_string;
	  break;
	}

      case '{':			/* ${[#]name[[:]#[#]%[%]-=?+[word]]} */
	{
	  int check_nullness = 0;
	  int var_is_set = 0;
	  int var_is_null = 0;
	  int var_is_special = 0;
	  char *name, *value, *string_extract ();

	  sindex++;
	  t_index = sindex;
	  name = string_extract (string, &t_index, "#%:-=?+}");

	  /* If the name really consists of a special variable, then
	     make sure that we have the entire name. */
	  if (sindex == t_index &&
	      (string[sindex] == '-' ||
	       string[sindex] == '?' ||
	       string[sindex] == '#'))
	    {
	      char *tt;
	      t_index++;
	      free (name);
	      tt = (string_extract (string, &t_index, "#%:-=?+}"));
	      name = (char *)xmalloc (2 + (strlen (tt)));
	      *name = string[sindex];
	      strcpy (name + 1, tt);
	      free (tt);
	    }
	  sindex = t_index;

	  /* Find out what character ended the variable name.  Then
	     do the appropriate thing. */
	  
	  if (c = string[sindex])
	    sindex++;

	  if (c == ':')
	    {
	      check_nullness++;
	      if (c = string[sindex])
		sindex++;
	    }
	  
	  /* Determine the value of this variable. */
	  if (strlen (name) == 1 && (digit(*name) || member(*name, "#-?$!@*")))
	    var_is_special++;
	    
	  /* Check for special expansion things. */
	  if (*name == '#')
	    {
	      if (name[1] != '*' && name[1] != '@')
		{
		  char *tt = get_string_value (&name[1]);
		  if (tt)
		    number = strlen (tt);
		  else
		    number = 0;
		  goto add_number;
		}
	      else
		name[1] = '\0';	/* ${#@} is the same as $#. */
	    }
	  
	  if (var_is_special)
	    {
	      char *tt = (char *)alloca (2 + strlen (name));
	      WORD_LIST *l;
	      tt[0] = '$'; tt[1] = '\0';
	      strcat (tt, name);
	      l = expand_string (tt, 0);
	      temp = string_list (l);
	      dispose_words (l);

	      if (temp)
		var_is_set++;
	    }
	  else
	    {
	      char *get_string_value ();
	      SHELL_VAR *f = find_variable (name);

	      if (f && !invisible_p (f) &&
		  ((temp = get_string_value (name)) != (char *)NULL))
		{
		  temp = savestring (temp);
		  var_is_set++;
		}
	      else
		temp = (char *)NULL;
	    }

	  if (!var_is_set || !temp || !*temp)
	    var_is_null++;
	  
	  if (!check_nullness)
	    var_is_null = 0;

	  /* Get the rest of the stuff inside the braces. */
	  if (c && c != '}')
	    {
	      /* Scan forward searching for last `{'.  This is a hack,
		 it will always be a hack, and it always has been a hack. */
	      {
		int braces_found = 0;

		for (t_index = sindex; string[t_index]; t_index++)
		  {
		    if (string[t_index] == '{')
		      braces_found++;
		    else
		      if (string[t_index] == '}')
			if (!braces_found)
			  break;
			else
			  braces_found--;
		  }

		value = (char *)xmalloc (1 + (t_index - sindex));
		strncpy (value, &string[sindex], t_index - sindex);
		value[t_index - sindex] = '\0';
		sindex = t_index;
	      }

	      if (string[sindex] == '}') sindex++;
	      else goto bad_substitution;
	    
	    }
	  else
	    {
	      value = (char *)NULL;
	    }
	  
	  /* Do the right thing based on which character ended the variable
	     name. */
	  switch (c)
	    {
	    case '\0':
	    bad_substitution:
	      report_error ("%s: bad substitution", name ? name : "??");
	      longjmp (top_level, DISCARD);

	    case '}':
	      break;
	    
	    case '#':	/* ${param#[#]pattern} */
	      if (!value || !*value || !temp || !*temp)
		break;

	      if (*value == '#')
		{
		  WORD_LIST *l = expand_string (++value, 0);
		  char *pattern = (char *)string_list (l);
		  char *t = temp;
		  dispose_words (l);
		  temp = remove_pattern (t, pattern, RP_LONG_LEFT);
		  free (t);
		  free (pattern);
		}
	      else
		{
		  WORD_LIST *l = expand_string (value, 0);
		  char *pattern = string_list (l);
		  char *t = temp;
		  dispose_words (l);
		  temp = remove_pattern (t, pattern, RP_SHORT_LEFT);
		  free (t);
		  free (pattern);
		}
	      break;

	    case '%':	/* ${param%[%]pattern} */
	      if (!value || !*value || !temp || !*temp)
		break;

	      if (*value == '%')
		{
		  WORD_LIST *l = expand_string (++value, 0);
		  char *pattern = string_list (l);
		  char *t = temp;
		  dispose_words (l);
		  temp = remove_pattern (t, pattern, RP_LONG_RIGHT);
		  free (t);
		  free (pattern);
		}
	      else
		{
		  WORD_LIST *l = expand_string (value, 0);
		  char *pattern = string_list (l);
		  char *t = temp;
		  dispose_words (l);
		  temp = remove_pattern (t, pattern, RP_SHORT_RIGHT);
		  free (t);
		  free (pattern);
		}
	      break;

	    case '-':
	      if (var_is_set && !var_is_null)
		{
		  /* Do nothing.  Just use the value in temp. */
		}
	      else
		{
		  WORD_LIST *l = expand_string (value, 0);
		  if (temp)
		    free (temp);
		  temp = (char *)string_list (l);
		  dispose_words (l);
		  free (value);
		}
	      break;
	    
	    case '=':
	      if (var_is_set && !var_is_null)
		{
		  /* Do nothing.  The value of temp is desired. */
		}
	      else
		{
		  if (var_is_special)
		    {
		      report_error ("$%s: cannot assign in this way", name);
		      if (temp)
			free (temp);
		      temp = (char *)NULL;
		    }
		  else
		    {
		      WORD_LIST *l = expand_string (value, 0);
		      if (temp)
			free (temp);
		      temp = (char *)string_list (l);
		      dispose_words (l);
		      bind_variable (name, temp);
		    }
		}
	      break;
	    
	    case '?':
	      if (var_is_set && !var_is_null)
		{
		  /* Do nothing.  The value in temp is desired. */
		}
	      else
		{
		  /* The spec says to "print `word' [the right hand
		     side], and exit from the shell", but I think that
		     is brain-damaged beyond all belief.  I also haven't
		     found another shell that does that yet.  I don't
		     think that I ever will find one.  The spec goes on
		     to then say "If `word' is omitted, the message
		     ``parameter null or not set'' is printed", but that
		     is so stupid it is clearly to be ignored.  I just
		     print whatever was found on the right-hand of the
		     statement.  If nothing is found on the right hand
		     side, I am told that I must print a default error
		     message and exit from the shell.  Remember, this
		     isn't my idea.  */

		  if (value && *value)
		    {
		      WORD_LIST *l = expand_string (value, 0);
		      char *temp1 =  string_list (l);
		      fprintf (stderr, "%s: %s\n", temp, temp1 ? temp1 : "");
		      if (temp1)
			free (temp1);
		      dispose_words (l);
		    }
		  else
		    {
		      if (var_is_special)
			report_error ("%s: Too few arguments", dollar_vars[0]);
		      else
			report_error ("%s: parameter not set", name);
		    }

		  if (temp)
		    free (temp);

		  if (!interactive)
		    longjmp (top_level, FORCE_EOF);
		  else
		    longjmp (top_level, DISCARD);
		}
	      break;
	    
	    case '+':
	      /* We don't want the value of the named variable for anything. */
	      if (temp)
		free (temp);
	      temp = (char *)NULL;
	    
	      if (var_is_set && !var_is_null)
		{
		  /* Use the right-hand side.  Pretty weird. */
		  if (value)
		    {
		      WORD_LIST *l = expand_string (value, 0);
		      temp = (char *)string_list (l);
		      dispose_words (l);
		    }
		}
	      break;
	    }			/* end case on closing character. */
	  free (name);
	  goto add_string;
	}			/* end case '{' */
      break;
    
      case '(':			/* Do command substitution. */
	/* We have to extract the contents of this paren substitution. */
	{
	  char *extract_command_subst ();
	  int old_index = ++sindex;

	  temp = extract_command_subst (string, &old_index);
	  sindex = old_index;

	  goto command_substitution;
	}

      default:
	{
	  /* Find the variable in VARIABLE_LIST. */
	  int old_index = sindex;
	  char *name;
	  SHELL_VAR *var;

	  temp = (char *)NULL;

	  for (;
	       (c = string[sindex]) && (isletter (c) || digit (c) || c == '_');
	       sindex++);
	  name = (char *)substring (string, old_index, sindex);

	  /* If this isn't a variable name, then just output the `$'. */
	  if (!name || !*name) {
	    free (name);
	    temp = savestring ("$");
	    goto add_string;
	  }

	  /* If the variable exists, return its value cell. */
	  var = find_variable (name);

	  if (var && value_cell (var))
	    {
	      temp = savestring (value_cell (var));
	      free (name);
	      goto add_string;
	    }
	  else
	    temp = (char *)NULL;

	  if (var && !temp && function_cell (var))
	    {
	      report_error ("%s: cannot substitute function", name);
	    }
	  else
	    {
	      if (unbound_vars_is_error)
		{
		  report_error ("%s: unbound variable", name);
		}
	      else
		goto add_string;
	    }
	  free (name);
	  longjmp (top_level, DISCARD);
	}
      }
      break;			/* End case '$': */

    case '`':
      {
	sindex++;

	t_index = sindex;
	temp = string_extract (string, &t_index, "`");
	sindex = t_index;
	de_backslash (temp);
	
      command_substitution:
	/* Pipe the output of executing TEMP into the current shell.
	   Make any and all whitespace (including newlines) be just
	   one space character. */
	{
	  extern int last_made_pid;
	  int pid, old_pid, fildes[2];
	  
	  if (pipe (fildes) < 0) {
	    report_error ("Can't make pipes for backquote substitution!");
	    goto error_exit;
	  }
	  
	  old_pid = last_made_pid;
#ifdef JOB_CONTROL
	  {
	    extern int pipeline_pgrp, shell_pgrp;
	    pipeline_pgrp = shell_pgrp;
	    pid = make_child (savestring ("backquote"), 0);

	    stop_making_children ();
	    pipeline_pgrp = 0;
	  }
#else   /* JOB_CONTROL */
	  pid = make_child (savestring ("backquote"), 0);
#endif  /* JOB_CONTROL */

	  if (pid < 0) {
	    report_error ("Can't make a child for backquote substitution!");
	  error_exit:
	    if (istring)
	      free (istring);
	    dispose_words (result);
	    return ((WORD_LIST *)NULL);
	  }

	  if (pid == 0)
	    {
#ifdef JOB_CONTROL
	      set_job_control (0);
#endif
	      dup2 (fildes[1], 1);
	      close (fildes[1]);
	      close (fildes[0]);

	      exit (parse_and_execute (temp, "backquote"));
	    }
	  else
	    {
	      FILE *istream = fdopen (fildes[0], "r");
	      int start_index = istring_index;
	      int lastc;
	    
	      close (fildes[1]);
	    
	      if (!istream)
		{
		  report_error ("Can't reopen pipe to command substitution");
		  goto error_exit;
		}

	      while ((lastc = c) &&
#ifdef SYSV
		     ((c = sysv_getc (istream)) != EOF)
#else
		     ((c = getc (istream)) != EOF)
#endif
		     )
		{

		  /* Duplicated from `add_character:' */
		  while (istring_index + 1 >= istring_size)
		    istring =
		      (char *)xrealloc (istring,
					istring_size += DEFAULT_ARRAY_SIZE);

		  if (quoted)
		    {
		      istring[istring_index++] = QUOTE_CHAR (c);
		    }
		  else if (whitespace (c) || c == '\n')
		    {
		      if (!whitespace (lastc) && lastc != '\n')
			istring[istring_index++] = ' ';
		    }
		  else
		    {
		      istring[istring_index++] = c;
		    }
		  istring[istring_index] = '\0';
		}
	      fclose (istream);
	      close (fildes[0]);

	      last_command_exit_value = wait_for (pid);
	      last_made_pid = old_pid;

	      if (temp)
		free (temp);

	      if (quoted)
		{
		  if (istring_index > 0 &&
		      DEQUOTE_CHAR (istring[istring_index - 1]) == '\n')
		    istring[--istring_index] = '\0';
		}
	      else
		{
		  strip_leading (istring + start_index);
		  strip_trailing (istring + start_index);
		  istring_index = strlen (istring);
		}
	      if (string[sindex])
		goto next_character;
	      else
		continue;
	    }
	}
      }
      
    case '\\':
      if (string[sindex + 1] == '\n')
	{
	  sindex += 2;
	  continue;
	}
      else
	{
	  c = (string[++sindex]);
	  if (quoted && !member (c, slashify_in_quotes))
	    {
	      temp = (char *)xmalloc (3);
	      temp[0] = '\\'; temp[1] = c; temp[2] = '\0';
	      if (c)
		sindex++;
	      goto add_string;
	    }
	  else
	    {
	      /* This character is quoted, so add it it in quoted mode. */
	      c = QUOTE_CHAR (c);
	      goto add_character;
	    }
	}
      
    case '"':
      if (quoted)
	goto add_character;
      sindex++;
      {
	WORD_LIST *tresult = (WORD_LIST *)NULL;

	t_index = sindex;
	temp = string_extract_double_quoted (string, &t_index);
	sindex = t_index;

	if (string[sindex])
	  sindex++;

	tresult = expand_string (temp, 1);
	free (temp);

	/* expand_string *might* return a list (consider the case of "$@",
	   where it returns "$1", "$2", and so on).  We can't throw away
	   the rest of the list, and we have to make sure each word gets
	   added as quoted.  We test on tresult->next:  if it is non-NULL,
	   we  quote the whole list, save it to a string with string_list,
	   and add that string. We don't need to quote the results of this
	   (and it would be wrong, since that would quote the separators
	   as well), so we go directly to add_string. */

	if (tresult)
	  {
	    if (tresult->next)
	      {
		quote_list (tresult);
		temp = string_list (tresult);
		dispose_words (tresult);
		goto add_string;
	      }
	    else
	      {
		temp = savestring (tresult->word->word);
		dispose_words (tresult);
	      }
	  }
	else
	  temp = (char *)NULL;

      add_quoted_string:

	if (temp)
	  quote_string (temp);
	else
	  {
	  add_null_arg:
	    temp = savestring (" ");
	    temp[0] = (unsigned char)QUOTE_CHAR ('\0');
	  }
	goto add_string;
      }
      break;
      
    case '\'':
      if (!quoted)
	{
	  sindex++;

	  t_index = sindex;
	  temp = string_extract_verbatim (string, &t_index, "'");
	  if (history_expansion)
	    unquote_bang (temp);
	  sindex = t_index;

	  if (string[sindex]) sindex++;

	  if (!*temp)
	    {
	      free (temp);
	      temp = (char *)NULL;
	    }

	  goto add_quoted_string;
	}
      else
	{
	  goto add_character;
	}
      break;

    default:

add_character:
      while (istring_index + 1 >= istring_size)
	istring = (char *)xrealloc (istring,
				    istring_size += DEFAULT_ARRAY_SIZE);
      istring[istring_index++] = c;
      istring[istring_index] = '\0';

next_character:
      sindex++;
    }
  }

finished_with_string:
final_exit:
  if (istring)
    {
      WORD_LIST *temp_list;
      char *ifs_chars = get_string_value ("IFS");

      if (quoted || !ifs_chars) ifs_chars = "";

      temp_list = list_string (istring, ifs_chars, quoted);
      free (istring);
      result = (WORD_LIST *)list_append (reverse_list (result), temp_list);
    }
  else
    result = (WORD_LIST *)NULL;
  return (result);
}

/* Do all of the assignments in LIST upto a word which isn't an
   assignment. */
WORD_LIST *
get_rid_of_variable_assignments (list)
     WORD_LIST *list;
{
  WORD_LIST *copy_word_list ();
  WORD_LIST *orig = list;

  while (list)
    if (!list->word->assignment)
      {
	WORD_LIST *new_list = copy_word_list (list);
	dispose_words (orig);
	return (new_list);
      }
    else
      {
	do_assignment (list->word->word);
	list = list->next;
      }
  dispose_words (orig);
  return ((WORD_LIST *)NULL);
}

/* Check and handle the case where there are some variable assignments
   in LIST which go into the environment for this command. */
WORD_LIST *
get_rid_of_environment_assignments (list)
     WORD_LIST *list;
{
  register WORD_LIST *tlist = list;
  register WORD_LIST *new_list;
  WORD_LIST *copy_word_list (), *copy_word ();

  while (tlist)
    {
      if (!tlist->word->assignment) goto make_assignments;
      tlist = tlist->next;
    }
  /* Since all of the assignments are variable assignments. */
  return (list);

make_assignments:
  tlist = list;
  while (tlist)
    {
      if (tlist->word->assignment)
	assign_in_env (tlist->word->word);
      else
	{
	  if (!place_keywords_in_env)
	    {
	      new_list = copy_word_list (tlist);
	      dispose_words (list);
	      return (new_list);
	    }
	}
      tlist = tlist->next;
    }

  /* We got all of the keywords assigned.  Now return the remainder
     of the words. */
  {
    register WORD_LIST *new_list = (WORD_LIST *)NULL;

    tlist = list;

    /* Skip the ones at the start. */
    while (tlist && tlist->word->assignment)
      tlist = tlist->next;

    /* If we placed all the keywords in the list into the environment,
       then remove them from the output list. */
    if (place_keywords_in_env)
      {
	while (tlist)
	  {
	    if (!tlist->word->assignment)
	      new_list = make_word_list (copy_word (tlist->word), new_list);
	    tlist = tlist->next;
	  }
	new_list = (WORD_LIST *)reverse_list (new_list);
      }
    else
      {
	/* Just copy the list. */
	new_list = copy_word_list (tlist);
      }
    dispose_words (list);
    return (new_list);
  }
}

/* Take the list of words in LIST and do the various substitutions.  Return
   a new list of words which is the expanded list, and without things like
   variable assignments. */
WORD_LIST *
expand_words (list)
     WORD_LIST *list;
{
  WORD_LIST *expand_words_1 ();
  return (expand_words_1 (list, 1));
}

/* Same as expand_words (), but doesn't hack variable or environment
   variables. */
WORD_LIST *
expand_words_no_vars (list)
     WORD_LIST *list;
{
  WORD_LIST *expand_words_1 ();
  return (expand_words_1 (list, 0));
}

/* Non-zero means to allow unmatched globbed filenames to expand to
   a null file. */
static int allow_null_glob_expansion = 0;

/* The workhorse for expand_words () and expand_words_no_var ().
   First arg is LIST, a WORD_LIST of words.
   Second arg DO_VARS is non-zero if you want to do environment and
   variable assignments, else zero. */
WORD_LIST *
expand_words_1 (list, do_vars)
     WORD_LIST *list;
     int do_vars;
{
  register WORD_LIST *tlist, *new_list = (WORD_LIST *)NULL;
  WORD_LIST *orig_list;
  extern int no_brace_expansion;

  tlist = (WORD_LIST *)copy_word_list (list);

  if (do_vars)
    {
      /* Handle the case where the arguments are assignments for
	 the environment of this command. */
      tlist = get_rid_of_environment_assignments (tlist);

      /* Handle the case where the arguments are all variable assignments. */
      tlist = get_rid_of_variable_assignments (tlist);
    }

  /* Begin expanding the words that remain.  The expansions take place on
     things that aren't really variable assignments. */

  if (!tlist)
    return ((WORD_LIST *)NULL);

  /* Do brace expansion on this word if there are any brace characters
     in the string. */
  if (!no_brace_expansion)
    {
      extern char **brace_expand ();
      register char **expansions;
      WORD_LIST *braces = (WORD_LIST *)NULL;
      int index;
    
      orig_list = tlist;

      while (tlist)
	{
	  expansions = brace_expand (tlist->word->word);

	  for (index = 0; expansions[index]; index++)
	    {
	      braces = make_word_list (make_word (expansions[index]), braces);
	      free (expansions[index]);
	    }
	  free (expansions);

	  tlist = tlist->next;
	}
      dispose_words (orig_list);
      tlist = (WORD_LIST *)reverse_list (braces);
    }

  orig_list = tlist;

  /* We do tilde expansion, but only if globbing is enabled.
     I guess this is the right thing. */
  while (tlist)
    {
      if (!disallow_filename_globbing && !tlist->word->quoted &&
	  (*(tlist->word->word) == '~'))
	{
	  char *tilde_expand (), *tt = tlist->word->word;
	  tlist->word->word = tilde_expand (tlist->word->word);
	  free (tt);
	}

      new_list =
	(WORD_LIST *)list_append
	  (reverse_list (expand_word_internal (tlist->word, 0)), new_list);

      tlist = tlist->next;
    }

  new_list = (WORD_LIST *)reverse_list (new_list);

  dispose_words (orig_list);

  /* Okay, we're almost done.  Now let's just do some filename
     globbing. */
  {
    char **shell_glob_filename (), **temp_list = (char **)NULL;
    register int list_index;
    WORD_LIST *glob_list;

    orig_list = (WORD_LIST *)NULL;
    tlist = new_list;

    if (!disallow_filename_globbing)
      {
	while (tlist)
	  {
	    /* If the word isn't quoted, then glob it. */
	    if (!tlist->word->quoted && glob_pattern_p (tlist->word->word))
	      {
		temp_list = shell_glob_filename (tlist->word->word);

		/* Fix the hi-bits. (This is how we quoted
		   special characters.) */
		{
		  register char *t = tlist->word->word;
		  while (*t) *t++ &= 0x7f;
		}

		/* Handle error cases.
		   I don't think we should report errors like "No such file
		   or directory".  However, I would like to report errors
		   like "Read failed". */
		 
		if (temp_list == (char **)-1)
		  {
		    /* file_error (tlist->word->word); */
		    return (new_list);
		  }

		if (!temp_list) abort ();

		/* Sort the returned names.  Maybe this should be done in
		   glob_filename (). */
		{
		  int qsort_string_compare ();
		  qsort (temp_list, array_len (temp_list),
			 sizeof (char *), qsort_string_compare);
		}

		/* Make the array into a word list. */
		glob_list = (WORD_LIST *)NULL;
		for (list_index = 0; temp_list[list_index]; list_index++)
		  glob_list =
		    make_word_list (make_word (temp_list[list_index]), glob_list);
	  
		if (glob_list)
		  orig_list = (WORD_LIST *)list_append (glob_list, orig_list);
		else
		  if (!allow_null_glob_expansion)
		    orig_list =
		      make_word_list (copy_word (tlist->word), orig_list);
	      }
	    else
	      {
		/* Fix the hi-bits. (This is how we quoted special
		   characters.) */
		register char *t = tlist->word->word;
		while (*t) *t++ &= 0x7f;
		orig_list = make_word_list (copy_word (tlist->word), orig_list);
	      }

	    free_array (temp_list);
	    temp_list = (char **)NULL;

	    tlist = tlist->next;
	  }
	dispose_words (new_list);
	new_list = orig_list;
      }
    else
      {
	/* Fix the hi-bits. (This is how we quoted special characters.) */
	register WORD_LIST *wl = new_list;
	register char *wp;
	while (wl) {
	  wp = wl->word->word;
	  while (*wp) *wp++ &= 0x7f;
	  wl = wl->next;
	}
	return (new_list);
      }
  }
  return (WORD_LIST *)(reverse_list (new_list));
}

/* Call the glob library to do globbing on PATHNAME.
   PATHNAME can contain characters with the hi bit set; this indicates
   that the character is to be quoted.  We quote it here. */
char **
shell_glob_filename (pathname)
     char *pathname;
{
  extern char **glob_filename ();
  register int i, j;
  char *temp = (char *)alloca (2 * strlen (pathname) + 1);

  for (i = j = 0; pathname[i]; i++, j++)
    {
      if ((unsigned char)pathname[i] > 0x7f)
	temp[j++] = '\\';
  
      temp[j] = (unsigned char)pathname[i] & 0x7f;
    }
  temp[j] = '\0';

  return (glob_filename (temp));
}

/* An alist of name.function for each special variable.  Most of the functions
   don't do much, and in fact, this would be faster with a switch statement,
   but by the end of this file, I am sick of switch statements. */

/* The functions that get called. */
int sv_path (), sv_mail (), sv_terminal (), sv_histsize (), sv_uids (),
  sv_ignoreeof (), sv_glob_dot_filenames (), sv_histchars (), sv_nolinks (),
  sv_hostname_completion_file (), sv_allow_null_glob_expansion (),
  sv_history_control (), sv_noclobber ();

#ifdef JOB_CONTROL
extern int sv_notify ();
#endif


struct name_and_function {
  char *name;
  Function *function;
} special_vars[] = {
  { "PATH", sv_path },
  { "MAIL", sv_mail },
  { "MAILPATH", sv_mail },
  { "MAILCHECK", sv_mail },
  { "TERMCAP", sv_terminal },
  { "TERM", sv_terminal },
  { "HISTSIZE", sv_histsize },
  { "EUID", sv_uids},
  { "UID", sv_uids},
  { "ignoreeof", sv_ignoreeof },
#ifdef JOB_CONTROL
  { "notify", sv_notify },
#endif  /* JOB_CONTROL */
  { "glob_dot_filenames", sv_glob_dot_filenames },
  { "allow_null_glob_expansion", sv_allow_null_glob_expansion },
  { "histchars", sv_histchars },
  { "nolinks", sv_nolinks },
  { "hostname_completion_file", sv_hostname_completion_file },
  { "history_control", sv_history_control },
  { "noclobber", sv_noclobber },
  { (char *)0x00, (Function *)0x00 }
};

/* The variable in NAME has just had its state changed.  Check to see if it
   is one of the special ones where something special happens. */
stupidly_hack_special_variables (name)
     char *name;
{
  int i = 0;

  while (special_vars[i].name)
    {
      if (strcmp (special_vars[i].name, name) == 0)
	{
	  (*(special_vars[i].function)) (name);
	  return;
	}
      i++;
    }
}

/* Set/unset noclobber. */
sv_noclobber (name)
     char *name;
{
  extern int noclobber;

  if (find_variable (name))
    noclobber = 1;
  else
    noclobber = 0;
}

/* What to do just after the PATH variable has changed. */
sv_path ()
{
  /* hash -r */
  WORD_LIST *args;

  args = make_word_list (make_word ("-r"), NULL);
  hash_builtin (args);
  dispose_words (args);
}

/* What to do just after one of the MAILxxxx variables has changed.  NAME
   is the name of the variable.  */
sv_mail (name)
     char *name;
{
  /* If the time interval for checking the files has changed, then
     reset the mail timer.  Otherwise, one of the pathname vars
     to the users mailbox has changed, so rebuild the array of
     filenames. */
  if (strcmp (name, "MAILCHECK") == 0)
    reset_mail_timer ();
  else
    {
      if ((strcmp (name, "MAIL") == 0) || (strcmp (name, "MAILPATH") == 0))
	{
	  free_mail_files ();
	  remember_mail_dates ();
	}
    }
}

/* What to do just after one of the TERMxxx variables has changed.
   If we are an interactive shell, then try to reset the terminal
   information in readline. */
sv_terminal (name)
     char *name;
{
  extern int interactive;

  if (interactive)
    rl_reset_terminal (get_string_value ("TERM"));
}

/* What to do after the HISTSIZE variable changes.
   If there is a value for this variable (and it is numeric), then stifle
   the history.  Otherwise, if there is NO value for this variable,
   unstifle the history. */
sv_histsize (name)
     char *name;
{
  char *temp = get_string_value (name);
  
  if (temp)
    {
      int num;
      if (sscanf (temp, "%d", &num) == 1)
	stifle_history (num);
    }
  else
    unstifle_history ();
}

/* A nit for picking at history saving.
   Value of 0 means save all lines parsed by the shell on the history.
   Value of 1 means save all lines that do not start with a space.
   Value of 2 means save all lines that do not match the last line saved. */
int history_control = 0;

/* What to do after the HISTORY_CONTROL variable changes. */
sv_history_control (name)
     char *name;
{
  char *temp = get_string_value (name);

  history_control = 0;

  if (temp && *temp)
    {
      if (strcmp (temp, "ignorespace") == 0)
	history_control = 1;
      else if (strcmp (temp, "ignoredups") == 0)
	history_control = 2;
    }
}

/* If the variable exists, then the value of it can be the number
   of times we actually ignore the EOF.  The default is small,
   (smaller than csh, anyway). */
sv_ignoreeof (name)
     char *name;
{
  extern int eof_encountered, eof_encountered_limit;
  char *temp = get_string_value (name);
  int new_limit;

  eof_encountered = 0;

  if (temp && (sscanf (temp, "%d", &new_limit) == 1))
    eof_encountered_limit = new_limit;
  else
    eof_encountered_limit = 10; /* csh uses 26. */
}

/* Control whether * matches .files in globbing.  Yechh. */

sv_glob_dot_filenames (name)
     char *name;
{
  extern int noglob_dot_filenames;

  noglob_dot_filenames = !(find_variable (name));
}

/* Setting/unsetting of the history expansion character. */
char old_history_expansion_char = '!';
char old_history_comment_char = '#';
char old_history_subst_char = '^';

sv_histchars (name)
     char *name;
{
  extern int history_expansion_char;
  extern int history_comment_char;
  extern int history_subst_char;
  char *temp = get_string_value (name);

  if (temp)
    {
      old_history_expansion_char = history_expansion_char;
      history_expansion_char = *temp;

      if (temp[1])
	{
	  old_history_subst_char = history_subst_char;
	  history_subst_char = temp[1];

	  if (temp[2])
	    {
	      old_history_comment_char = history_comment_char;
	      history_comment_char = temp[2];
	    }
	}
    }
  else
    {
      history_expansion_char = '!';
      history_subst_char = '^';
      history_comment_char = '#';
    }
}

#ifdef JOB_CONTROL
/* Job notification feature desired? */
sv_notify (name)
     char *name;
{
  extern int asynchronous_notification;

  if (get_string_value (name))
    asynchronous_notification = 1;
  else
    asynchronous_notification = 0;
}
#endif  /* JOB_CONTROL */

/* If the variable `nolinks' exists, it specifies that symbolic links are
   not to be followed in `cd' commands. */
sv_nolinks (name)
     char *name;
{
  extern int follow_symbolic_links;

  follow_symbolic_links = !find_variable (name);
}

/* Don't let users hack the user id variables. */
sv_uids ()
{
  int uid = getuid ();
  int euid = geteuid ();
  char buff[10];
  register SHELL_VAR *v;

  sprintf (buff, "%d", uid);
  v = find_variable ("UID");
  if (v)
    v->attributes &= ~att_readonly;

  v = bind_variable ("UID", buff);
  v->attributes |= att_readonly;

  sprintf (buff, "%d", euid);
  v = find_variable ("EUID");
  if (v)
    v->attributes &= ~att_readonly;

  v = bind_variable ("EUID", buff);
  v->attributes |= att_readonly;
}

sv_hostname_completion_file ()
{
  extern int hostname_list_initialized;

  hostname_list_initialized = 0;
}

sv_allow_null_glob_expansion (name)
{
  allow_null_glob_expansion = (int)find_variable (name);
}

