/* shell.h -- The data structures used by the shell */

#include "config.h"
#include "general.h"
#include "variables.h"
#include "quit.h"

#ifdef SYSV
#define MAXPATHLEN 1024
#endif

extern int EOF_Reached;

#define NO_PIPE -1
#define REDIRECT_BOTH -2
#define IS_DESCRIPTOR -1

#define NO_VARIABLE -1

/* A bunch of stuff for flow of control using setjmp () and longjmp (). */

#include <setjmp.h>
extern jmp_buf top_level, catch;

#define NOT_JUMPED 0		/* Not returning from a longjmp. */
#define FORCE_EOF 1		/* We want to stop parsing. */
#define DISCARD 2		/* Discard current command. */
#define EXITPROG 3		/* Unconditionally exit the program now. */

/* Values that can be returned by execute_command (). */
#define EXECUTION_FAILURE 1
#define EXECUTION_SUCCESS 0


/* The list of characters that are quoted in double-quotes with a
   backslash.  Other characters following a backslash cause nothing
   special to happen. */
#define slashify_in_quotes "\\`\"$"

/* All structs which contain a `next' field should have that field
   as the first field in the struct.  This means that functions
   can be written to handle the general case for linked lists. */
typedef struct g_list {
  struct g_list *next;
} GENERIC_LIST;

/* Instructions describing what kind of thing to do for a redirection. */
enum r_instruction { r_output_direction, r_input_direction, r_inputa_direction,
		     r_appending_to, r_reading_until, r_duplicating,
		     r_deblank_reading_until, r_close_this, r_err_and_out };

/* Command Types: */
enum command_type { cm_for, cm_case, cm_while, cm_if, cm_simple,
		    cm_connection, cm_function_def, cm_until, cm_group };

/* A structure which represents a word. */
typedef struct word_desc {
  char *word;			/* Zero terminated string. */
  int dollar_present;		/* Non-zero means dollar sign present. */
  int quoted;			/* Non-zero means single, double, or back quote
				   or backslash is present. */
  int assignment;		/* Non-zero means that this word contains an assignment. */
} WORD_DESC;

/* A linked list of words. */
typedef struct word_list {
  struct word_list *next;
  WORD_DESC *word;
} WORD_LIST;


/* **************************************************************** */
/*								    */
/*			Shell Command Structs			    */
/*								    */
/* **************************************************************** */

/* What a redirection descriptor looks like.  If FLAGS is IS_DESCRIPTOR,
   then we use REDIRECTEE.DEST, else we use the file specified. */
typedef struct redirect {
  struct redirect *next;	/* Next element, or NULL. */
  int redirector;		/* Descriptor to be redirected. */
  int flags;			/* Flag value for `open'. */
  enum r_instruction  instruction; /* What to do with the information. */
  union {
    int dest;			/* Place to redirect REDIRECTOR to, or ... */
    WORD_DESC *filename;	/* filename to redirect to. */
  } redirectee;
  char *here_doc_eof;		/* The word that appeared in <<foo. */
} REDIRECT;

/* An element used in parsing.  A single word or a single redirection.
   This is an ephemeral construct. */
typedef struct element {
  WORD_DESC *word;
  REDIRECT *redirect;
} ELEMENT;

/* What a command looks like. */
typedef struct command {
  enum command_type type;	/* FOR CASE WHILE IF CONNECTION or SIMPLE. */
  int subshell;			/* Non-zero means execute in a subshell. */
  REDIRECT *redirects;		/* Special redirects for FOR CASE, etc. */
  union {
    struct for_com *For;
    struct case_com *Case;
    struct while_com *While;
    struct if_com *If;
    struct connection *Connection;
    struct simple_com *Simple;
    struct function_def *Function_def;
    struct group_com *Group;
  } value;
} COMMAND;

/* Structure used to represent the CONNECTION type. */
typedef struct connection {
  COMMAND *first;		/* Pointer to the first command. */
  COMMAND *second;		/* Pointer to the second command. */
  int connector;		/* What separates this command from others. */
} CONNECTION;

/* Structures used to represent the CASE command. */

/* Pattern/action structure for CASE_COM. */
typedef struct pattern_list {
  struct pattern_list *next;	/* The next clause to try in case this one failed. */
  WORD_LIST *patterns;		/* Linked list of patterns to test, one after each other. */
  COMMAND *action;		/* Thing to execute if one of the patterns match. */
} PATTERN_LIST;

/* The CASE command. */
typedef struct case_com {
  WORD_DESC *word;		/* the thing to test. */
  PATTERN_LIST *clauses;	/* the clauses to test against, or NULL. */
} CASE_COM;

/* FOR command. */
typedef struct for_com {
  WORD_DESC *name;		/* The variable name to get mapped over. */
  WORD_LIST *map_list;		/* The things to map over.  This is never NULL. */
  COMMAND *action;		/* The action to execute.
				   During execution, NAME is bound to successive
				   members of MAP_LIST. */
} FOR_COM;

/* IF command. */
typedef struct if_com {
  COMMAND *test;		/* Thing to test. */
  COMMAND *true_case;		/* What to do if the test returned non-zero. */
  COMMAND *false_case;		/* What to do if the test returned zero. */
} IF_COM;

/* WHILE command. */
typedef struct while_com {
  COMMAND *test;		/* Thing to test. */
  COMMAND *action;		/* Thing to do while test is non-zero. */
} WHILE_COM;

/* The "simple" command.  Just a collection of words and redirects. */
typedef struct simple_com {
  WORD_LIST *words;		/* The program name, the arguments, variable assignments, etc. */
  REDIRECT *redirects;		/* Redirections to perform. */
} SIMPLE_COM;

/* The "function_def" command.  This isn't really a command, but it is
   represented as such for now.  If the function def appears within 
   `(' `)' the parser tries to set the SUBSHELL bit of the command.  That
   means that FUNCTION_DEF has to be run through the executor.  Maybe this
   command should be defined in a subshell.  Who knows or cares. */
typedef struct function_def {
  WORD_DESC *name;
  COMMAND *command;
} FUNCTION_DEF;

/* A command that is `grouped' allows pipes to take effect over
   the entire command structure. */
typedef struct group_com {
  COMMAND *command;
} GROUP_COM;
  
/* Forward declarations of functions called by the grammer. */
extern REDIRECT *make_redirection ();
extern WORD_LIST *make_word_list ();
extern WORD_DESC *make_word ();

extern COMMAND
  *make_for_command (), *make_case_command (), *make_if_command (),
  *make_while_command (), *command_connect (), *make_simple_command (),
  *make_function_def (), *clean_simple_command (), *make_until_command (),
  *make_group_command ();


extern PATTERN_LIST *make_pattern_list ();
extern COMMAND *global_command, *copy_command ();

extern char **shell_environment;
extern WORD_LIST *rest_of_args;

/* Generalized global variables. */
extern int executing, login_shell;
