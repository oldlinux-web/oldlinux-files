/* alias.c -- Not a full alias, but just the kind that we use in the
   shell.  Csh style alias is somewhere else. */

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

#include "config.h"
#include "general.h"
#include "alias.h"

/* The number of slots to allocate when we need new slots. */
#define alias_list_grow_amount 50

/* Non-zero means expand all words on the line.  Otherwise, expand
   after first expansion if the expansion ends in a space. */
int alias_expand_all = 0;

/* The list of aliases that we have. */
ASSOC **aliases = (ASSOC **)NULL;

/* The number of slots in the above list. */
static int aliases_size = 0;

/* The number of aliases that are in existence. */
static int aliases_length = 0;

/* The last alias index found with find_alias (). */
static int last_alias_index = 0;

/* Scan the list of aliases looking for one with NAME.  Return NULL
   if the alias doesn't exist, else a pointer to the assoc. */
ASSOC *
find_alias (name)
     char *name;
{
  register int i;

  for (i = 0; i < aliases_length; i++)
    if (strcmp (name, aliases[i]->name) == 0)
      return (aliases[last_alias_index = i]);

  return ((ASSOC *)NULL);
}

/* Return the value of the alias for NAME, or NULL if there is none. */
char *
get_alias_value (name)
     char *name;
{
  ASSOC *alias = find_alias (name);
  if (alias)
    return (alias->value);
  else
    return ((char *)NULL);
}

/* Make a new alias from NAME and VALUE.  If NAME can be found,
   then replace its value. */
void
add_alias (name, value)
     char *name, *value;
{
  ASSOC *temp = find_alias (name);

  if (temp)
    {
      free (temp->value);
      temp->value = savestring (value);
    }
  else
    {
      temp = (ASSOC *)xmalloc (sizeof (ASSOC));
      temp->name = savestring (name);
      temp->value = savestring (value);

      if ((aliases_length + 1) >= aliases_size)
	{
	  if (!aliases)
	    aliases =
	      (ASSOC **)xmalloc ((aliases_size = alias_list_grow_amount)
				 * sizeof (ASSOC *));
	  else
	    aliases =
	      (ASSOC **)xrealloc (aliases,
				  (aliases_size += alias_list_grow_amount)
				  * sizeof (ASSOC *));
	}
      aliases[aliases_length++] = temp;
      aliases[aliases_length] = (ASSOC *)NULL;
    }
}

/* Remove the alias with name NAME from the alias list.  Returns
   the index of the removed alias, or -1 if the alias didn't exist. */
int
remove_alias (name)
     char *name;
{
  register int i;

  if (!find_alias (name))
    return (-1);

  i = last_alias_index;
  free (aliases[i]->name);
  free (aliases[i]->value);
  free (aliases[i]);

  for (; i < aliases_length; i++)
    aliases[i] = aliases[i + 1];

  aliases_length--;
  aliases[aliases_length] = (ASSOC *)NULL;

  return (last_alias_index);
}

/* Delete all aliases. */
delete_all_aliases ()
{
  register int i;

  for (i = 0; i < aliases_length; i++)
    {
      free (aliases[i]->name);
      free (aliases[i]->value);
      free (aliases[i]);
      aliases[i] = (ASSOC *)NULL;
    }

  aliases_length = 0;
}

/* Return non-zero if CHARACTER is a member of the class of characters
   that are self-delimiting in the shell. */
self_delimiting (character)
     int character;
{
  return (member (character, " \t\n\r;|&("));
}

/* Return a new line, with any aliases substituted. */
char *
alias_expand (string)
     char *string;
{
  int line_len = 1 + strlen (string);
  char *line = (char *)xmalloc (line_len);
  register int i, j, start, delimiter;
  char *token = (char *)alloca (line_len);
  int tl, real_start, in_command_position;
  int expand_next = 1;
  ASSOC *alias;

  line[0] = i = 0;

  /* Find the next word in line.  If it has an alias, substitute
     the alias value.  If the value ends in ` ', then try again
     with the next word.  Else, if there is no value, or if
     the value does not end in space, we are done. */

 next_word:

  token[0] = 0;
  /* Skip leading whitespace (or separator characters).
     But save it in the output.  */
  for (start = i; string[i]; i++)
    {
      if (whitespace (string[i]))
	continue;

      if (self_delimiting (string[i]))
	{
	  expand_next++;
	  continue;
	}
      break;
    }

  if (start == i && string[i] == '\0')
    return (line);

  j = strlen (line);
  if (1 + j + (i - start) >= line_len)
    line = (char *)xrealloc (line, line_len += (50 + (i - start)));
  strncpy (line + j, string + start, i - start);
  line[j + (i - start)] = '\0';

  real_start = i;

  in_command_position = (self_delimiting (string[i]) || expand_next);
  expand_next = 0;

  /* From here to next separator character is a token. */
  delimiter = 0;

  for (start = i; string[i]; i++)
    {
      switch (string[i])
	{
	case '\\':
	  if (string[i + 1])
	    {
	      i++;
	      continue;
	    }
	  break;

	case '"':
	case '\'':
	  if (!delimiter)
	    delimiter = string[i];
	  else if (delimiter == string[i])
	    delimiter = 0;
	  break;
	}

      if (!delimiter &&
	  (whitespace (string[i]) || self_delimiting (string[i])))
	break;
    }

  tl = strlen (token);
  strncpy (token + tl, string + start, i - start);
  token [tl += (i - start)] = '\0';

  /* See if this word has a substitution.  If it does, do the expansion,
     but only if we are expanding all words, or if we are in a location
     where an expansion is supposed to take place. */
  alias = find_alias (token);

  if (alias && (in_command_position || alias_expand_all))
    {
      char *v = alias->value;
      int l = strlen (v);
      
      /* +3 because we possibly add one more character below. */
      if ((l + 3) > line_len - strlen (line))
	line = (char *)xrealloc (line, line_len += (50 + l));

      strcat (line, v);

      if ((l && whitespace (v[l - 1])) || alias_expand_all)
	{
	  if (l && whitespace (v[l -1]))
	    line[strlen (line) - 1] = '\0';
	  
	  expand_next = 1;
	}
    }
  else
    {
      int ll = strlen (line), tl = i - real_start;
      /* int tl = strlen (token); */

      if (ll + tl + 2 > line_len)
	line = (char *)xrealloc (line, line_len += 50 + ll + tl);

      strncpy (line + ll, string + real_start, tl);
      line[ll + tl] = '\0';
      /* strcat (line, token); */
    }
  goto next_word;
}
