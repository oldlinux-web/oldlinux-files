/* Yacc grammar for bash. */

/*  Copyright (C) 1989 Free Software Foundation, Inc.

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
with Bash; see the file LICENSE.  If not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

%{
#include <stdio.h>
#include <signal.h>
#include "shell.h"
#include "flags.h"

#ifdef READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#define YYDEBUG 1
extern int eof_encountered;
extern int no_line_editing;
extern int interactive;
%}

%union {
  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT element;
  PATTERN_LIST *pattern;
}

/* Reserved words.  Members of the first group are only recognized
   in the case that they are preceded by a list_terminator.  Members
   of the second group are recognized only under special circumstances. */
%token IF THEN ELSE ELIF FI CASE ESAC FOR WHILE UNTIL DO DONE FUNCTION
%token IN

/* More general tokens. yylex () knows how to make these. */
%token <word> WORD
%token <number> NUMBER
%token AND_AND OR_OR GREATER_GREATER LESS_LESS LESS_AND
%token GREATER_AND SEMI_SEMI LESS_LESS_MINUS AND_GREATER
%token DOUBLE_OPEN DOUBLE_CLOSE

/* The types that the various syntactical units return. */

%type <command> inputunit command list list0 list1 simple_list simple_list1 simple_command shell_command group_command
%type <command> elif_clause
%type <redirect> redirection redirections
%type <element> simple_command_element
%type <word_list> words pattern 
%type <pattern> pattern_list case_clause_sequence case_clause_1 pattern_list_1

%start inputunit

%left '&' ';' '\n' yacc_EOF
%left AND_AND OR_OR
%left '|'
%%

inputunit:	simple_list '\n'
			{
			  /* Case of regular command.  Discard the error
			     safety net,and return the command just parsed. */
			  global_command = $1;
			  eof_encountered = 0;
			  discard_parser_constructs (0);
			  YYACCEPT;
			}
	|	'\n'
			{
			  /* Case of regular command, but not a very
			     interesting one.  Return a NULL command. */
			  global_command = (COMMAND *)NULL;
			  YYACCEPT;
			}
	|
		error '\n'
			{
			  /* Error during parsing.  Return NULL command. */
			  global_command = (COMMAND *)NULL;
			  eof_encountered = 0;
			  discard_parser_constructs (1);
			  if (interactive)
			    YYACCEPT;
			  else
			    YYABORT;
			}
	|
		yacc_EOF
			{
			  /* Case of EOF seen by itself.  Do ignoreeof or 
			     not. */
			  global_command = (COMMAND *)NULL;
			  handle_eof_input_unit ();
			  YYACCEPT;
			}
	;

words:	
			{ $$ = NULL; }
	|	words WORD
			{ $$ = make_word_list ($2, $1); }
	;

redirection:	'>' WORD
			{ $$ = make_redirection ( 1, r_output_direction, $2); }
	|	'<' WORD
			{ $$ = make_redirection ( 0, r_input_direction, $2); }
	|	NUMBER '>' WORD
			{ $$ = make_redirection ($1, r_output_direction, $3); }
	|	NUMBER '<' WORD
			{ $$ = make_redirection ($1, r_input_direction, $3); }
	|	GREATER_GREATER WORD
			{ $$ = make_redirection ( 1, r_appending_to, $2); }
	|	NUMBER GREATER_GREATER WORD
			{ $$ = make_redirection ($1, r_appending_to, $3); }
	|	LESS_LESS WORD
			{ $$ = make_redirection ( 0, r_reading_until, $2); }
	|	NUMBER LESS_LESS WORD
			{ $$ = make_redirection ($1, r_reading_until, $3); }
	|	LESS_AND NUMBER
			{ $$ = make_redirection ( 0, r_duplicating, $2); }
	|	NUMBER LESS_AND NUMBER
			{ $$ = make_redirection ($1, r_duplicating, $3); }
	|	GREATER_AND NUMBER
			{ $$ = make_redirection ( 1, r_duplicating, $2); }
	|	NUMBER GREATER_AND NUMBER
			{ $$ = make_redirection ($1, r_duplicating, $3); }
	|	LESS_LESS_MINUS WORD
			{ $$ = make_redirection ( 0, r_deblank_reading_until, $2); }
	|	NUMBER LESS_LESS_MINUS WORD
			{ $$ = make_redirection ($1, r_deblank_reading_until, $3); }
	|	GREATER_AND '-'
			{ $$ = make_redirection ( 1, r_close_this, 0); }

	|	NUMBER GREATER_AND '-'
			{ $$ = make_redirection ($1, r_close_this, 0); }
	|	LESS_AND '-'
			{ $$ = make_redirection ( 0, r_close_this, 0); }
	|	NUMBER LESS_AND '-'
			{ $$ = make_redirection ($1, r_close_this, 0); }
	|	AND_GREATER WORD
			{ $$ = make_redirection ( 1, r_err_and_out, $2); }
	|	GREATER_AND WORD
			{ $$ = make_redirection ( 1, r_err_and_out, $2); }
	;

simple_command_element: WORD
 			{ $$.word = $1; $$.redirect = 0; }
	|	redirection
			{ $$.redirect = $1; $$.word = 0; }
	;

redirections:	redirection
	|	redirections redirection
			{ $1->next = $2; $$ = $1; }
	;

simple_command:	simple_command_element
			{ $$ = make_simple_command ($1, NULL); }
	|	simple_command simple_command_element
			{ $$ = make_simple_command ($2, $1); }
	;

command:	simple_command
			{ $$ = clean_simple_command ($1); }
	|	shell_command
			{ $$ = $1; }

	|	shell_command redirections
			{ $$->redirects = $2; $$ = $1; }
	;

shell_command:	FOR WORD newlines DO list DONE
			{ $$ = make_for_command ($2, (WORD_LIST *)add_string_to_list ("$@", NULL), $5); }
	|	FOR WORD ';' newlines DO list DONE
			{ $$ = make_for_command ($2, (WORD_LIST *)add_string_to_list ("$@", NULL), $6); }
	|	FOR WORD ';' newlines '{' list '}'
			{ $$ = make_for_command ($2, (WORD_LIST *)add_string_to_list ("$@", NULL), $6); }
	|	FOR WORD newlines IN words list_terminator newlines DO list DONE
			{ $$ = make_for_command ($2, (WORD_LIST *)reverse_list ($5), $9); }
	|	FOR WORD newlines IN words list_terminator newlines '{' list '}'
			{ $$ = make_for_command ($2, (WORD_LIST *)reverse_list ($5), $9); }

	|	CASE WORD newlines IN newlines ESAC
			{ $$ = make_case_command ($2, NULL); }
	|	CASE WORD newlines IN case_clause_sequence newlines ESAC
			{ $$ = make_case_command ($2, $5); }
	|	CASE WORD newlines IN case_clause_1 ESAC
			{ report_syntax_error ("Inserted `;;'");
			  $$ = make_case_command ($2, $5); }
 
	|	IF list THEN list FI
	  		{ $$ = make_if_command ($2, $4, NULL); }
	|	IF list THEN list ELSE list FI
			{ $$ = make_if_command ($2, $4, $6); }
	|	IF list THEN list elif_clause FI
			{ $$ = make_if_command ($2, $4, $5); }

	|	WHILE list DO list DONE
			{ $$ = make_while_command ($2, $4); }
	|	UNTIL list DO list DONE
			{ $$ = make_until_command ($2, $4); }

	|	'(' list ')'
			{ $2->subshell = 1; $$ = $2; }

	|	group_command
			{ $$ = $1; }

	|	WORD '(' ')' newlines group_command
			{ $$ = make_function_def ($1, $5); }

	|	FUNCTION WORD '(' ')' newlines group_command
			{ $$ = make_function_def ($2, $6); }

	|	FUNCTION WORD '\n' newlines group_command
			{ $$ = make_function_def ($2, $5); }

	|	FUNCTION WORD group_command
			{ $$ = make_function_def ($2, $3); }
	;

group_command:	'{' list '}'
			{ $$ = make_group_command ($2); }
	;

elif_clause:	ELIF list THEN list
			{ $$ = make_if_command ($2, $4, NULL); }
	|	ELIF list THEN list ELSE list
			{ $$ = make_if_command ($2, $4, $6); }
	|	ELIF list THEN list elif_clause
			{ $$ = make_if_command ($2, $4, $5); }
	;


case_clause_1:	pattern_list_1
	|	case_clause_sequence pattern_list_1
  			{ $2->next = $1; $$ = $2; }
	;

pattern_list_1:	newlines pattern ')' list
			{ $$ = make_pattern_list ($2, $4); }
	|	newlines pattern ')' newlines
			{ $$ = make_pattern_list ($2, NULL); }
	;

case_clause_sequence:  pattern_list

  	|	case_clause_sequence pattern_list
			{ $2->next = $1; $$ = $2; }
	;

pattern_list:	newlines pattern ')' list SEMI_SEMI
			{ $$ = make_pattern_list ($2, $4); }
	|	newlines pattern ')' newlines SEMI_SEMI
			{ $$ = make_pattern_list ($2, NULL); }
	;

pattern:	WORD
			{ $$ = make_word_list ($1, NULL); }
	|	pattern '|' WORD
			{ $$ = make_word_list ($3, $1); }
	;

/* A list allows leading or trailing newlines and
   newlines as operators (equivalent to semicolons).
   It must end with a newline or semicolon.
   Lists are used within commands such as if, for, while.  */

list:		newlines list0
			{ $$ = $2; }
	;

list0:		list1
	|	list1 '\n' newlines
	|	list1 '&' newlines
			{ $$ = command_connect ($1, 0, '&'); }
	|	list1 ';' newlines

	;

list1:		list1 AND_AND newlines list1
			{ $$ = command_connect ($1, $4, AND_AND); }
	|	list1 OR_OR newlines list1
			{ $$ = command_connect ($1, $4, OR_OR); }
	|	list1 '&' newlines list1
			{ $$ = command_connect ($1, $4, '&'); }
	|	list1 ';' newlines list1
			{ $$ = command_connect ($1, $4, ';'); }
	|	list1 '\n' newlines list1
			{ $$ = command_connect ($1, $4, ';'); }
	|	list1 '|' newlines list1
			{ $$ = command_connect ($1, $4, '|'); }
	|	command
	;


list_terminator:'\n'
	|	';'
	|	yacc_EOF
	;

newlines:
	|	newlines '\n'
	;

/* A simple_list is a list that contains no significant newlines
   and no leading or trailing newlines.  Newlines are allowed
   only following operators, where they are not significant.

   This is what an inputunit consists of.  */

simple_list:	simple_list1
	|	simple_list1 '&'
			{ $$ = command_connect ($1, (COMMAND *)NULL, '&'); }
	|	simple_list1 ';'
	;

simple_list1:	simple_list1 AND_AND newlines simple_list1
			{ $$ = command_connect ($1, $4, AND_AND); }
	|	simple_list1 OR_OR newlines simple_list1
			{ $$ = command_connect ($1, $4, OR_OR); }
	|	simple_list1 '&' simple_list1
			{ $$ = command_connect ($1, $3, '&'); }
	|	simple_list1 ';' simple_list1
			{ $$ = command_connect ($1, $3, ';'); }
	|	simple_list1 '|' newlines simple_list1
			{ $$ = command_connect ($1, $4, '|'); }
	|	command
	;
%%

/* Initial size to allocate for tokens, and the
   amount to grow them by. */
#define TOKEN_DEFAULT_GROW_SIZE 512

/* The token currently being read. */
int current_token = 0;

/* The last read token, or NULL.  read_token () uses this for context
   checking. */
int last_read_token = 0;

/* The token read prior to last_read_token. */
int token_before_that = 0;

/* Global var is non-zero when end of file has been reached. */
int EOF_Reached = 0;

/* yy_getc () returns the next available character from input or EOF.
   yy_ungetc (c) makes `c' the next character to read.
   init_yy_io (get, unget), makes the function `get' the installed function
   for getting the next character, and makes `unget' the installed function
   for un-getting a character. */
return_EOF ()			/* does nothing good. */
{
  return (EOF);
}

/* Variables containing the current get and unget functions. */

/* Some stream `types'. */
#define st_stream 0
#define st_string 1

Function *get_yy_char = return_EOF;
Function *unget_yy_char = return_EOF;
int yy_input_type = st_stream;
FILE *yy_input_dev = (FILE *)NULL;

/* The current stream name.  In the case of a file, this is a filename. */
char *stream_name = (char *)NULL;

/* Function to set get_yy_char and unget_yy_char. */
init_yy_io (get_function, unget_function, type, location)
     Function *get_function, *unget_function;
     int type;
     FILE *location;
{
  get_yy_char = get_function;
  unget_yy_char = unget_function;
  yy_input_type = type;
  yy_input_dev = location;
}

/* Call this to get the next character of input. */
yy_getc ()
{
  return (*get_yy_char) ();
}

/* Call this to unget C.  That is, to make C the next character
   to be read. */
yy_ungetc (c)
{
  return (*unget_yy_char) (c);
}

/* **************************************************************** */
/*								    */
/*		  Let input be read from readline ().		    */
/*								    */
/* **************************************************************** */

#ifdef READLINE
char *current_readline_prompt = (char *)NULL;
char *current_readline_line = (char *)NULL;
int current_readline_line_index = 0;

static int readline_initialized_yet = 0;
yy_readline_get ()
{
  if (!current_readline_line)
    {
      char *readline ();
      SigHandler *old_sigint;
      extern sighandler throw_to_top_level ();
      
      if (!readline_initialized_yet)
	{
	  initialize_readline ();
	  readline_initialized_yet = 1;
	}

      old_sigint = (SigHandler *)signal (SIGINT, throw_to_top_level);
	  if (!current_readline_prompt)
	    current_readline_line = readline ("");
	  else
	    current_readline_line = readline (current_readline_prompt);
      signal (SIGINT, old_sigint);

      current_readline_line_index = 0;

      if (!current_readline_line)
	{
	  current_readline_line_index = 0;
	  return (EOF);
	}

      current_readline_line =
	(char *)xrealloc (current_readline_line,
			  2 + strlen (current_readline_line));
      strcat (current_readline_line, "\n");
    }

  if (!current_readline_line[current_readline_line_index])
    {
      free (current_readline_line);
      current_readline_line = (char *)NULL;
      return (yy_readline_get ());
    }
  else
    {
      int c = current_readline_line[current_readline_line_index++];
      return (c);
    }
}

yy_readline_unget (c)
{
  if (current_readline_line_index && current_readline_line)
    current_readline_line[--current_readline_line_index] = c;
}
  
with_input_from_stdin ()
{
  init_yy_io (yy_readline_get, yy_readline_unget,
	      st_string, current_readline_line);
  stream_name = savestring ("readline stdin");
}

#else  /* READLINE */

with_input_from_stdin ()
{
  with_input_from_stream (stdin, "stdin");
}
#endif  /* READLINE */

/* **************************************************************** */
/*								    */
/*   Let input come from STRING.  STRING is zero terminated.	    */
/*								    */
/* **************************************************************** */

yy_string_get ()
{
  /* If the string doesn't exist, or is empty, EOF found. */
  if (!(char *)yy_input_dev || !*(char *)yy_input_dev)
    return (EOF);
  else {
    register char *temp = (char *)yy_input_dev;
    int c = *temp++;
    yy_input_dev = (FILE *)temp;
    return (c);
  }
}

yy_string_unget (c)
     int c;
{
  register char *temp = (char *)yy_input_dev;
  *(--temp) = c;
  yy_input_dev = (FILE *)temp;
  return (c);
}

with_input_from_string (string, name)
     char *string;
     char *name;
{
  init_yy_io (yy_string_get, yy_string_unget, st_string, string);
  stream_name = savestring (name);
}

/* **************************************************************** */
/*								    */
/*		     Let input come from STREAM.		    */
/*								    */
/* **************************************************************** */

yy_stream_get ()
{
  if (yy_input_dev)
#ifdef SYSV
    return (sysv_getc (yy_input_dev));
#else
    return (getc (yy_input_dev));
#endif  /* SYSV */
  else return (EOF);
}

yy_stream_unget (c)
     int c;
{
  ungetc (c, yy_input_dev);
}

with_input_from_stream (stream, name)
     FILE *stream;
     char *name;
{
  init_yy_io (yy_stream_get, yy_stream_unget, st_stream, stream);
  stream_name = savestring (name);
}

typedef struct stream_saver {
  struct stream_saver *next;
  Function *getter, *putter;
  int type, line;
  char *location, *name;
} STREAM_SAVER;

/* The globally known line number. */
int line_number = 0;

STREAM_SAVER *stream_list = (STREAM_SAVER *)NULL;

push_stream ()
{
  STREAM_SAVER *temp = (STREAM_SAVER *)xmalloc (sizeof (STREAM_SAVER));
  temp->type = yy_input_type;
  temp->location = (char *)yy_input_dev;
  temp->getter = get_yy_char;
  temp->putter = unget_yy_char;
  temp->line = line_number;
  temp->name = stream_name; stream_name = (char *)NULL;
  temp->next = stream_list;
  stream_list = temp;
  EOF_Reached = line_number = 0;
}

pop_stream ()
{
  if (!stream_list)
    {
      EOF_Reached = 1;
    }
  else
    {
      STREAM_SAVER *temp = stream_list;
    
      EOF_Reached = 0;
      stream_list = stream_list->next;

      if (stream_name)
	free (stream_name);
      stream_name = temp->name;

      init_yy_io (temp->getter, temp->putter, temp->type, temp->location);
      line_number = temp->line;
      free (temp);
    }
}


/* Return a line of text, taken from wherever yylex () reads input.
   If there is no more input, then we return NULL. */
char *
read_a_line ()
{
  char *line_buffer = (char *)NULL;
  int index = 0, buffer_size = 0;
  int c;

  while (c = yy_getc ())
    {
      /* If there is no more input, then we return NULL. */
      if (c == EOF)
	{
	  c = '\n';
	  if (!line_buffer)
	    return ((char *)NULL);
	}

      if (index + 1 > buffer_size)
	if (!buffer_size)
	  line_buffer = (char *)xmalloc (buffer_size = 200);
	else
	  line_buffer = (char *)xrealloc (line_buffer, buffer_size += 200);

      line_buffer[index++] = c;
      if (c == '\n')
	{
	  line_buffer[index] = '\0';
	  return (line_buffer);
	}
    }
}


/* Return a line as in read_a_line (), but insure that the prompt is
   the secondary prompt. */
char *
read_secondary_line ()
{
  char *decode_prompt_string ();
  char *temp_prompt = get_string_value ("PS2");

#ifdef READLINE
  if (!no_line_editing)
    {
      extern char *current_readline_prompt;

      if (current_readline_prompt)
	{
	  free (current_readline_prompt);
	  current_readline_prompt = (char *)NULL;
	}

      if (temp_prompt)
	current_readline_prompt = decode_prompt_string (temp_prompt);
      else
	current_readline_prompt = (savestring (""));
    }
  else
#endif  /* READLINE */
    {
      printf ("%s", temp_prompt ? temp_prompt : "");
      fflush (stdout);
    }

  return (read_a_line ());
}

/* **************************************************************** */
/*								    */
/*				YYLEX ()			    */
/*								    */
/* **************************************************************** */

/* Reserved words.  These are only recognized as the first word of a
   command.  TOKEN_WORD_ALIST. */
struct {
  char *word;
  int token;
} token_word_alist[] = {
  {"if", IF},
  {"then", THEN},
  {"else", ELSE},
  {"elif", ELIF},
  {"fi", FI},
  {"case", CASE},
  {"esac", ESAC},
  {"for", FOR},
  {"while", WHILE},
  {"until", UNTIL},
  {"do", DO},
  {"done", DONE},
  {"in", IN},
  {"function", FUNCTION},
  {"{", '{'},
  {"}", '}'},
  {(char *)NULL, 0}
};

/* Where shell input comes from.  For each line that we read, alias
   and history expansion are done. */
char *shell_input_line = (char *)NULL;
int shell_input_line_index = 0;
int shell_input_line_size = 0;

/* Either zero, or EOF. */
int shell_input_line_terminator = 0;

/* Return the next shell input character.  This always reads characters
   from shell_input_line; when that line is exhausted, it is time to
   read the next line. */
int
shell_getc (remove_quoted_newline)
     int remove_quoted_newline;
{
  extern int login_shell;
  int c;

  if (!shell_input_line || !shell_input_line[shell_input_line_index])
    {
      register int i, l;
      char *pre_process_line (), *expansions;

      restart_read_next_line:

      line_number++;

    restart_read:

      i = 0;
      shell_input_line_terminator = 0;

#ifdef JOB_CONTROL
      notify_and_cleanup ();
#endif

      clearerr (stdin);
      while (c = yy_getc ())
	{
	  if (i + 2 > shell_input_line_size)
	    if (!shell_input_line)
	      shell_input_line = (char *)xmalloc (shell_input_line_size = 256);
	    else
	      shell_input_line =
		(char *)xrealloc (shell_input_line, shell_input_line_size += 256);

	  if (c == EOF)
	    {
	      clearerr (stdin);

	      if (!i)
		shell_input_line_terminator = EOF;

	      shell_input_line[i] = '\0';
	      break;
	    }

	  shell_input_line[i++] = c;

	  if (c == '\n')
	    {
	      shell_input_line[--i] = '\0';
	      break;
	    }
	}
      shell_input_line_index = 0;

      if (!shell_input_line[0] || shell_input_line[0] == '#')
	goto after_pre_process;

      expansions = pre_process_line (shell_input_line, 1, 1);

      free (shell_input_line);
      shell_input_line = expansions;

      if (shell_input_line)
	{
	  if (echo_input_at_read)
	    fprintf (stderr, "%s\n", shell_input_line);

	  shell_input_line_size = strlen (expansions);

	}
      else
	{
	  shell_input_line_size = 0;
	  prompt_again ();
	  goto restart_read;
	}

    after_pre_process:
      /* Add the newline to the end of this string, iff the string does
	 not already end in an EOF character.  */
      if (shell_input_line_terminator != EOF)
	{
	  l = strlen (shell_input_line);

	  if (l + 3 > shell_input_line_size)
	    shell_input_line =
	      (char *)xrealloc (shell_input_line,
				1 + (shell_input_line_size += 2));
	  strcpy (shell_input_line + l, "\n");
	}
    }
  
  c = shell_input_line[shell_input_line_index];

  if (c)
    shell_input_line_index++;

  if (c == '\\' && remove_quoted_newline &&
      shell_input_line[shell_input_line_index] == '\n')
    {
      prompt_again ();
      goto restart_read_next_line;
    }

  if (!c && shell_input_line_terminator == EOF)
    {
      if (shell_input_line_index != 0)
	return ('\n');
      else
	return (EOF);
    }

  return (c);
}

/* Put C back into the input for the shell. */
shell_ungetc (c)
     int c;
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line[--shell_input_line_index] = c;
}

/* Discard input until CHARACTER is seen. */
discard_until (character)
     int character;
{
  int c;
  while ((c = shell_getc (0)) != EOF && c != character)
    ;
  if (c != EOF )
    shell_ungetc (c);
}

/* Tell readline () that we have some text for it to edit. */
re_edit (text)
     char *text;
{
#ifdef READLINE
  if (strcmp (stream_name, "readline stdin") == 0)
    bash_re_edit (text);
#endif
}

/* Non-zero means do no history expansion on this line, regardless
   of what history_expansion says. */
int history_expansion_inhibited = 0;

/* Do pre-processing on LINE.  If PRINT_CHANGES is non-zero, then
   print the results of expanding the line if there were any changes.
   If there is an error, return NULL, otherwise the expanded line is
   returned.  If ADDIT is non-zero the line is added to the history
   list after history expansion, but before alias expansion.  ADDIT
   is just a suggestion; REMEMBER_ON_HISTORY can veto, and does.
   Right now this does history and alias expansion. */
char *
pre_process_line (line, print_changes, addit)
     char *line;
     int print_changes, addit;
{
  extern int history_expansion;
  extern int remember_on_history;
  int history_expand ();
  char *history_value;
  char *return_value = line;
  int expanded = 0;

#ifdef ALIAS
  char *alias_expand (), *alias_value;
#endif

  /* History expand the line.  If this results in no errors, then
     add that line to the history if ADDIT is non-zero. */
  if (!history_expansion_inhibited && history_expansion)
    {
      expanded = history_expand (line, &history_value);

      if (expanded)
	{
	  if (print_changes)
	    fprintf (stderr, "%s\n", history_value);

	  /* If there was an error, return NULL. */
	  if (expanded < 0)
	    {
	      free (history_value);

	      /* New hack.  We can allow the user to edit the
		 failed history expansion. */
	      re_edit (line);

	      return ((char *)NULL);
	    }
	}

      /* Let other expansions know that return_value can be free'ed,
	 and that a line has been added to the history list. */
      expanded = 1;
      return_value = history_value;
    }

  if (addit && remember_on_history)
    {
      extern int history_control;

      switch (history_control)
	{
	case 0:
	  add_history (return_value);
	  break;
	case 1:
	  if (*return_value != ' ')
	    add_history (return_value);
	  break;
	case 2:
	  {
	    HIST_ENTRY *temp = previous_history ();
	    if (!temp || (strcmp (temp->line, return_value) != 0))
	      add_history (return_value);
	    using_history ();
	  }
	  break;
	}
    }

#ifdef ALIAS
  alias_value = alias_expand (return_value);

  if (expanded)
    {
      expanded = 0;
      free (return_value);
    }

  return_value = alias_value;

#else
  return_value = savestring (line);
#endif /* ALIAS */

#ifdef ALIAS
#ifdef NEVER  /* Expanding history-wise here is sematically incorrect
		 for this shell, and should never be done.  I figured
		 it out, so just trust me, okay? */
  /* History expand the alias.  This is a special hack.  Don't you
     just hate this? */

  if (!history_expansion_inhibited && history_expansion)
    {
      expanded = history_expand (return_value, &history_value);

      if (expanded < 0)
	{
	  free (history_value);
	  free (return_value);
	  return ((char *)NULL);
	}

      if (expanded)
	{
	  free (return_value);
	  return_value = history_value;
	}
    }
#endif  /* NEVER */
#endif  /* ALIAS */
  return (return_value);
}


/* Place to remember the token.  We try to keep the buffer
   at a reasonable size, but it can grow. */
char *token = NULL;

/* Current size of the token buffer. */
int token_buffer_size = 0;

/* Command to read_token () explaining what we want it to do. */
#define READ 0
#define RESET 1

/* prompt_string_pointer points to one of these,
   never to an actual string. */
char *ps1_prompt, *ps2_prompt;

/* Handle on the current prompt string.  Indirectly points through
   ps1_ or ps2_prompt. */
char **prompt_string_pointer;

/* Function for yyparse to call.  yylex keeps track of
   the last two tokens read, and calls read_token.  */
yylex ()
{
  if (interactive && (!current_token || current_token == '\n'))
    {
      /* Before we print a prompt, we might have to check mailboxes.
	 We do this only if it is time to do so. Notice that only here
	 is the mail alarm reset; nothing takes place in check_mail ()
	 except the checking of mail.  Please don't change this. */
      if (time_to_check_mail ())
	{
	  check_mail ();
	  reset_mail_timer ();
	}

      /* Allow the execution of a random command just before the printing
	 of each prompt.  If the shell variable PROMPT_COMMAND
	 is set then the value of it is the command to execute. */
      {
	char *command_to_execute = get_string_value ("PROMPT_COMMAND");

	if (command_to_execute)
	  {
	    extern Function *last_shell_builtin, *this_shell_builtin;
	    Function *temp_last, *temp_this;

	    temp_last = last_shell_builtin;
	    temp_this = this_shell_builtin;

	    parse_and_execute (savestring (command_to_execute),
			       "PROMPT_COMMAND");
	    last_shell_builtin = temp_last;
	    this_shell_builtin = temp_this;
	  }
      }

      prompt_again ();
      prompt_string_pointer = &ps2_prompt;
    }

  token_before_that = last_read_token;
  last_read_token = current_token;
  current_token = read_token (READ);
  return (current_token);
}

/* Called from shell.c when Control-C is typed at top level.  Or
   by the error rule at top level. */
reset_parser ()
{
  read_token (RESET);
}
  
/* When non-zero, we have read the required tokens
   which allow ESAC to be the next one read. */
static int allow_esac_as_next = 0;

/* When non-zero, accept single '{' as a token itself. */
static int allow_open_brace = 0;

/* DELIMITER is the value of the delimiter that is currently
   enclosing, or zero for none. */
static int delimiter = 0;

/* When non-zero, an open-brace used to create a group is awaiting a close
   brace partner. */
static int open_brace_awaiting_satisfaction = 0;

/* If non-zero, it is the token that we want read_token to return regardless
   of what text is (or isn't) present to be read.  read_token resets this. */
int token_to_read = 0;

/* Read the next token.  Command can be READ (normal operation) or 
   RESET (to normalize state. */
read_token (command)
     int command;
{
  int character;		/* Current character. */
  int peek_char;		/* Temporary look-ahead character. */
  int result;			/* The thing to return. */
  WORD_DESC *the_word;		/* The value for YYLVAL when a WORD is read. */

  if (token_buffer_size < TOKEN_DEFAULT_GROW_SIZE)
    {
      if (token)
	free (token);
      token = (char *)xmalloc (token_buffer_size = TOKEN_DEFAULT_GROW_SIZE);
    }

  if (command == RESET)
    {
      delimiter = 0;
      open_brace_awaiting_satisfaction = 0;
      if (shell_input_line)
	{
	  free (shell_input_line);
	  shell_input_line = (char *)NULL;
	  shell_input_line_size = 0;
	}
      last_read_token = '\n';
      token_to_read = '\n';
      return;
    }

  if (token_to_read)
    {
      int rt = token_to_read;
      token_to_read = 0;
      return (rt);
    }

  /* Read a single word from input.  Start by skipping blanks. */
  while ((character = shell_getc (1)) != EOF && whitespace (character));

  if (character == EOF)
    return (yacc_EOF);

  if (character == '#' && !interactive)
    {
      /* A comment.  Discard until EOL or EOF, and then return a newline. */
      discard_until ('\n');
      shell_getc (0);
      return ('\n');
    }

  if (character == '\n')
    return (character);

  if (member (character, "()<>;&|"))
    {
      /* Please note that the shell does not allow whitespace to
	 appear in between tokens which are character pairs, such as
	 "<<" or ">>".  I believe this is the correct behaviour. */

      if (character == (peek_char = shell_getc (1)))
	{
	  switch (character)
	    {
	      /* If '<' then we could be at "<<" or at "<<-".  We have to
		 look ahead one more character. */
	    case '<':
	      peek_char = shell_getc (1);
	      if (peek_char == '-')
		return (LESS_LESS_MINUS);
	      else
		{
		  shell_ungetc (peek_char);
		  return (LESS_LESS);
		}

	    case '(': return (DOUBLE_OPEN);
	    case ')': return (DOUBLE_CLOSE);
	    case '>': return (GREATER_GREATER);
	    case ';':	return (SEMI_SEMI);
	    case '&': return (AND_AND);
	    case '|': return (OR_OR);
	    }
	}
      else
	{
	  if (peek_char == '&')
	    {
	      switch (character)
		{
		case '<': return (LESS_AND);
		case '>': return (GREATER_AND);
		}
	    }
	  if (peek_char == '>' && character == '&')
	    return (AND_GREATER);
	}
      shell_ungetc (peek_char);

      /* If we look like we are reading the start of a function
	 definition, then let the reader know about it so that
	 we will do the right thing with `{'. */
      if (character == ')' &&
	  last_read_token == '(' && token_before_that == WORD)
	allow_open_brace = 1;

      return (character);
    }

  /* Hack <&- (close stdin) case. */
  if (character == '-')
    {
      switch (last_read_token)
	{
	case LESS_AND:
	case GREATER_AND:
	  return (character);
	}
    }
  
  /* Okay, if we got this far, we have to read a word.  Read one,
     and then check it against the known ones. */
  {
    /* Index into the token that we are building. */
    int token_index = 0;

    /* ALL_DIGITS becomes zero when we see a non-digit. */
    int all_digits = digit (character);

    /* DOLLAR_PRESENT becomes non-zero if we see a `$'. */
    int dollar_present = 0;

    /* QUOTED becomes non-zero if we see one of ("), ('), (`), or (\). */
    int quoted = 0;

    /* Non-zero means to ignore the value of the next character, and just
       to add it no matter what. */
    int pass_next_character = 0;

    /* Non-zero means parsing a dollar-paren construct.  It is the count of
       un-quoted closes we need to see. */
    int dollar_paren_level = 0;

    /* Another level variable.  This one is for dollar_parens inside of
       double-quotes. */
    int delimited_paren_level = 0;

    for (;;)
      {
	if (character == EOF)
	  goto got_token;

	if (pass_next_character)
	  {
	    pass_next_character = 0;
	    goto got_character;
	  }

	/* Handle double backslash.  These are always magic.  The
	   second backslash does not cause a trailing newline to be
	   eaten. */

	if (character == '\\')
	  {
	    peek_char = shell_getc (0);
	    if (peek_char != '\\')
	      shell_ungetc (peek_char);
	    else
	      {
		token[token_index++] = character;
		goto got_character;
	      }
	  }

	/* Handle backslashes.  Quote lots of things when not inside of
	   double-quotes, quote some things inside of double-quotes. */

	if (character == '\\' && delimiter != '\'')
	  {
	    peek_char = shell_getc (0);

	    /* Backslash-newline is ignored in all other cases. */
	    if (peek_char == '\n')
	      {
		character = '\n';
		goto next_character;
	      }
	    else
	      {
		shell_ungetc (peek_char);

		/* If the next character is to be quoted, do it now. */
		if (!delimiter || delimiter == '`' ||
		    ((delimiter == '"' ) &&
		     (member (peek_char, slashify_in_quotes))))
		  {
		    pass_next_character++;
		    quoted = 1;
		    goto got_character;
		  }
	      }
	  }

	if (delimiter)
	  {
	    if (character == delimiter)
	      {
		delimiter = 0;
		if (delimited_paren_level)
		  {
		    report_error ("Expected ')' before %c", character);
		    return ('\n');
		  }
		goto got_character;
	      }
	  }

	if (!delimiter || delimiter == '`' || delimiter == '"')
	  {
	    if (character == '$')
	      {
		peek_char = shell_getc (1);
		shell_ungetc (peek_char);
		if (peek_char == '(')
		  {
		    if (!delimiter)
		      dollar_paren_level++;
		    else
		      delimited_paren_level++;

		    pass_next_character++;
		    goto got_character;
		  }
	      }
		
	    if (character == ')')
	      {
		if (delimiter && delimited_paren_level)
		  delimited_paren_level--;

		if (!delimiter && dollar_paren_level)
		  {
		    dollar_paren_level--;
		    goto got_character;
		  }
	      }
	  }

	if (!dollar_paren_level && !delimiter &&
	    member (character, " \t\n;&()|<>"))
	  {
	    shell_ungetc (character);
	    goto got_token;
	  }
    
	if (!delimiter)
	  {
	    if (character == '"' || character == '`' || character == '\'')
	      {
		quoted = 1;
		delimiter = character;
		goto got_character;
	      }
	  }

	if (all_digits) all_digits = digit (character);
	if (character == '$') dollar_present = 1;

      got_character:

	token[token_index++] = character;

	if (token_index == (token_buffer_size - 1))
	  token = (char *)xrealloc (token, (token_buffer_size
					    += TOKEN_DEFAULT_GROW_SIZE));
	{
	  char *decode_prompt_string ();

	next_character:
	  if (character == '\n' && interactive)
	    prompt_again ();
	}
	character = shell_getc ((delimiter != '\''));
      }

got_token:

    token[token_index] = '\0';
	
    if ((delimiter || dollar_paren_level) && character == EOF)
      {
	if (dollar_paren_level && !delimiter)
	  delimiter = ')';

	report_error ("Unexpected EOF.  Looking for `%c'.", delimiter);
	return (-1);
      }

    if (all_digits)
      {
	/* Check to see what thing we should return.  If the last_read_token
	   is a `<', or a `&', or the character which ended this token is
	   a '>' or '<', then, and ONLY then, is this input token a NUMBER.
	   Otherwise, it is just a word, and should be returned as such. */

	if ((character == '<' || character == '>') ||
	    (last_read_token == LESS_AND ||
	     last_read_token == GREATER_AND))
	  {
	    sscanf (token, "%d", &(yylval.number));
	    return (NUMBER);
	  }
      }

    /* Handle special case.  IN is recognized if the last token
       was WORD and the token before that was FOR or CASE. */
    if ((strcmp (token, "in") == 0) &&
	(last_read_token == WORD) &&
	((token_before_that == FOR) ||
	 (token_before_that == CASE)))
      {
	if (token_before_that == CASE) allow_esac_as_next++;
	return (IN);
      }

    /* Ditto for DO in the FOR case. */
    if ((strcmp (token, "do") == 0) &&
	(last_read_token == WORD) &&
	(token_before_that == FOR))
      return (DO);

    /* Ditto for ESAC in the CASE case. 
       Specifically, this handles "case word in esac", which is a legal
       construct, certainly because someone will pass an empty arg to the
       case construct, and we don't want it to barf.  Of course, we should
       insist that the case construct has at least one pattern in it, but
       the designers disagree. */
    if (allow_esac_as_next)
      {
	allow_esac_as_next--;
	if (strcmp (token, "esac") == 0)
	  return (ESAC);
      }

    /* Ditto for `{' in the FUNCTION case. */
    if (allow_open_brace)
      {
	allow_open_brace = 0;
	if (strcmp (token, "{") == 0)
	  {
	    open_brace_awaiting_satisfaction++;
	    return ('{');
	  }
      }

    /* Check to see if it is a reserved word. */
    if (!dollar_present && !quoted &&
	reserved_word_acceptable (last_read_token))
      {
	int i;
	for (i = 0; token_word_alist[i].word != (char *)NULL; i++)
	  if (strcmp (token, token_word_alist[i].word) == 0)
	    {
	      if (token_word_alist[i].token == '{')
		open_brace_awaiting_satisfaction++;

	      return (token_word_alist[i].token);
	    }
      }

    /* What if we are attempting to satisfy an open-brace grouper? */
    if (open_brace_awaiting_satisfaction && strcmp (token, "}") == 0)
      {
	open_brace_awaiting_satisfaction--;
	return ('}');
      }
      
    the_word = (WORD_DESC *)xmalloc (sizeof (WORD_DESC));
    the_word->word = (char *)xmalloc (1 + strlen (token));
    strcpy (the_word->word, token);
    the_word->dollar_present = dollar_present;
    the_word->quoted = quoted;
    the_word->assignment = assignment (token);

    yylval.word = the_word;
    result = WORD;
    if (last_read_token == FUNCTION)
      allow_open_brace = 1;
  }
  return (result);
}

/* Return 1 if TOKEN is a token that after being read would allow
   a reserved word to be seen, else 0. */
reserved_word_acceptable (token)
     int token;
{
  if (member (token, "\n;()|&{") ||
      token == AND_AND ||
      token == OR_OR ||
      token == SEMI_SEMI ||
      token == DO ||
      token == IF ||
      token == THEN ||
      token == ELSE ||
      token == ELIF ||
      token == 0)
    return (1);
  else
    return (0);
}

/* Issue a prompt, or prepare to issue a prompt when the next character
   is read. */
prompt_again ()
{
  char *decode_prompt_string ();
  char *temp_prompt;

  ps1_prompt = get_string_value ("PS1");
  ps2_prompt = get_string_value ("PS2");

  if (!prompt_string_pointer)
    prompt_string_pointer = &ps1_prompt;

  if (*prompt_string_pointer)
    temp_prompt = decode_prompt_string (*prompt_string_pointer);
  else
    temp_prompt = savestring ("");

#ifdef READLINE
  if (!no_line_editing)
    {
      if (current_readline_prompt)
	free (current_readline_prompt);
      
      current_readline_prompt = temp_prompt;
    }
  else
#endif  /* READLINE */
    {
      if (interactive)
	{
	  fprintf (stderr, "%s", temp_prompt);
	  fflush (stderr);
	}
      free (temp_prompt);
    }
}

/* This sucks. but it is just a crock for SYSV.  The whole idea of MAXPATHLEN
   is a crock if you ask me.  Why can't we just have dynamically defined
   sizes?  (UCSB crashes every 20 minutes on me.) */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif  /* MAXPATHLEN */

/* Return a string which will be printed as a prompt.  The string
   may contain special characters which are decoded as follows:
   
	\t	the time
	\d	the date
	\n	CRLF
	\s	the name of the shell
	\w	the current working directory
	\W	the last element of PWD
	\u	your username
	\h	the hostname
	\#	the command number of this command
	\!	the history number of this command
	\$	a $ or a # if you are root
	\<octal> character code in octal
	\\	a backslash
*/
#include <sys/param.h>
#include <time.h>

#define PROMPT_GROWTH 50
char *
decode_prompt_string (string)
     char *string;
{
  int result_size = PROMPT_GROWTH;
  int result_index = 0;
  char *result = (char *)xmalloc (PROMPT_GROWTH);
  int c;
  char *temp = (char *)NULL;

  result[0] = 0;
  while (c = *string++)
    {
      if (c == '\\')
	{
	  c = *string;

	  switch (c)
	    {

	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	      {
		char octal_string[4];
		int n;

		strncpy (octal_string, string, 3);
		octal_string[3] = '\0';

		n = read_octal (octal_string);

		temp = savestring ("\\");
		if (n != -1)
		  {
		    string += 3;
		    temp[0] = n;
		  }

		c = 0;
		goto add_string;
	      }
	  
	    case 't':
	    case 'd':

	      /* Make the current time/date into a string. */
	      {
		long the_time = time (0);
		char *ttemp = ctime (&the_time);
		temp = savestring (ttemp);

		if (c == 't')
		  {
		    strcpy (temp, temp + 11);
		    temp[8] = '\0';
		  }
		else
		  temp[10] = '\0';

		goto add_string;
	      }

	    case 'n':
	      temp = savestring ("\r\n");
	      goto add_string;

	    case 's':
	      {
		extern char *shell_name;
		temp = savestring (shell_name);
		goto add_string;
	      }
	
	    case 'w':
	    case 'W':
	      {
		/* Use the value of PWD because it is much more effecient. */
#define EFFICIENT
#ifdef EFFICIENT
		char *polite_directory_format (), t_string[MAXPATHLEN];

		temp = get_string_value ("PWD");

		if (!temp)
		  getwd (t_string);
		else
		  strcpy (t_string, temp);
#else
		getwd (t_string);
#endif  /* EFFICIENT */

		if (c == 'W')
		  {
		    char *rindex (), *dir = rindex (t_string, '/');
		    if (dir)
		      strcpy (t_string, dir + 1);
		    temp = savestring (t_string);
		  }
		else
		  temp = savestring (polite_directory_format (t_string));
		goto add_string;
	      }
      
	    case 'u':
	      {
		extern char *current_user_name;
		temp = savestring (current_user_name);

		goto add_string;
	      }

	    case 'h':
	      {
		extern char *current_host_name;
		char *t_string, *index ();

		temp = savestring (current_host_name);
		if (t_string = index (temp, '.'))
		  *t_string = '\0';
		
		goto add_string;
	      }

	    case '#':
	      {
		extern int current_command_number;
		char number_buffer[20];
		sprintf (number_buffer, "%d", current_command_number);
		temp = savestring (number_buffer);
		goto add_string;
	      }

	    case '!':
	      {
		extern int history_base, where_history ();
		char number_buffer[20];

		using_history ();
		if (get_string_value ("HISTSIZE"))
		  sprintf (number_buffer, "%d",
			   history_base + where_history ());
		else
		  strcpy (number_buffer, "!");
		temp = savestring (number_buffer);
		goto add_string;
	      }

	    case '$':
	      temp = savestring (geteuid () == 0 ? "#" : "$");
	      goto add_string;

	    case '\\':
	      temp = savestring ("\\");
	      goto add_string;

	    default:
	      temp = savestring ("\\ ");
	      temp[1] = c;

	    add_string:
	      if (c)
		string++;
	      result =
		(char *)sub_append_string (temp, result,
					   &result_index, &result_size);
	      temp = (char *)NULL; /* Free ()'ed in sub_append_string (). */
	      result[result_index] = '\0';
	      break;
	    }
	}
      else
	{
	  while (3 + result_index > result_size)
	    result = (char *)xrealloc (result, result_size += PROMPT_GROWTH);

	  result[result_index++] = c;
	  result[result_index] = '\0';
	}
    }

  /* I don't really think that this is a good idea.  Do you? */
  if (!find_variable ("NO_PROMPT_VARS"))
    {
      WORD_LIST *expand_string (), *list;
      char *string_list ();

      list = expand_string (result, 1);
      free (result);
      result = string_list (list);
      dispose_words (list);
    }

  return (result);
}

/* Report a syntax error, and restart the parser.  Call here for fatal
   errors. */
yyerror ()
{
  report_syntax_error ((char *)NULL);
  reset_parser ();
}

/* Report a syntax error with line numbers, etc.
   Call here for recoverable errors.  If you have a message to print,
   then place it in MESSAGE, otherwise pass NULL and this will figure
   out an appropriate message for you. */
report_syntax_error (message)
     char *message;
{
  if (message)
    {
      if (!interactive)
	{
	  char *name = stream_name ? stream_name : "stdin";
	  report_error ("%s:%d: `%s'", name, line_number, message);
	}
      else
	report_error ("%s", message);

      return;
    }

  if (shell_input_line && *shell_input_line)
    {
      char *error_token, *t = shell_input_line;
      register int i = shell_input_line_index;
      int token_end = 0;

      if (!t[i] && i)
	i--;

      while (i && t[i] == ' ' || t[i] == '\t' || t[i] == '\n')
	i--;

      if (i)
	token_end = i + 1;

      while (i && !member (t[i], " \n\t;|&"))
	i--;

      while (i != token_end && member (t[i], " \n\t"))
	i++;

      if (token_end)
	{
	  error_token = (char *)alloca (1 + (token_end - i));
	  strncpy (error_token, t + i, token_end - i);
	  error_token[token_end - i] = '\0';

	  report_error ("syntax error near `%s'", error_token);
	}
      else if ((i == 0) && (token_end == 0))    /* a 1-character token */
	{
	  error_token = (char *) alloca (2);
	  strncpy(error_token, t + i, 1);
	  error_token[1] = '\0';

	  report_error ("syntax error near `%s'", error_token);
	}

      if (!interactive)
	{
	  char *temp = savestring (shell_input_line);
	  char *name = stream_name ? stream_name : "stdin";
	  int l = strlen (temp);

	  while (l && temp[l - 1] == '\n')
	    temp[--l] = '\0';

	  report_error ("%s:%d: `%s'", name, line_number, temp);
	  free (temp);
	}
    }
  else
    report_error ("Syntax error");
}

/* ??? Needed function. ??? We have to be able to discard the constructs
   created during parsing.  In the case of error, we want to return
   allocated objects to the memory pool.  In the case of no error, we want
   to throw away the information about where the allocated objects live.
   (dispose_command () will actually free the command. */
discard_parser_constructs (error_p)
     int error_p;
{
/*   if (error_p) {
     fprintf (stderr, "*");
  } */
}
   
/* Do that silly `type "bye" to exit' stuff.  You know, "ignoreeof". */

/* The number of times that we have encountered an EOF character without
   another character intervening.  When this gets above the limit, the
   shell terminates. */
int eof_encountered = 0;

/* The limit for eof_encountered. */
int eof_encountered_limit = 10;

/* If we have EOF as the only input unit, this user wants to leave
   the shell.  If the shell is not interactive, then just leave.
   Otherwise, if ignoreeof is set, and we haven't done this the
   required number of times in a row, print a message. */
handle_eof_input_unit ()
{
  extern int login_shell, EOF_Reached;

  if (interactive)
    {
      /* If the user wants to "ignore" eof, then let her do so, kind of. */
      if (find_variable ("ignoreeof") || find_variable ("IGNOREEOF"))
	{
	  if (eof_encountered < eof_encountered_limit)
	    {
	      fprintf (stderr, "Use \"%s\" to leave the shell.\n",
		       login_shell ? "logout" : "exit");
	      eof_encountered++;
	      return;
	    } 
	}

      /* In this case EOF should exit the shell.  Do it now. */
      reset_parser ();
      exit_builtin ((WORD_LIST *)NULL);
    }
  else
    {
      /* We don't write history files, etc., for non-interactive shells. */
      EOF_Reached = 1;
    }
}
