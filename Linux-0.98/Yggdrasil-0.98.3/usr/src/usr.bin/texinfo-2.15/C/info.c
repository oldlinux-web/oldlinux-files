/* info -- a stand-alone Info program.

   Copyright (C) 1987, 1991 Free Software Foundation, Inc.

   This file is part of GNU Info.

   GNU Info is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.  No author or distributor accepts
   responsibility to anyone for the consequences of using it or for
   whether it serves any particular purpose or works at all, unless he
   says so in writing.  Refer to the GNU Emacs General Public License
   for full details.

   Everyone is granted permission to copy, modify and redistribute
   GNU Info, but only under the conditions described in the GNU Emacs
   General Public License.   A copy of this license is supposed to
   have been given to you along with GNU Emacs so you can know your
   rights and responsibilities.  It should be in a file named COPYING.
   Among other things, the copyright notice and this notice must be
   preserved on all copies.  */

/* This is GNU Info:

   Version 1.44  (Change "major_version" and "minor_version" below.)
   Fri Feb  7 1992
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>
#include <errno.h>

#if !defined (errno)
extern int errno;
#endif /* !errno */

#include <ctype.h>
#include "getopt.h"

#if defined (USG)
   struct passwd *getpwnam ();
#  include <fcntl.h>
#  include <termio.h>
#  include <string.h>

#  if defined (USGr3)
#    if !defined (M_XENIX)
#      include <sys/stream.h>
#      include <sys/ptem.h>
#      undef TIOCGETC
#    else /* M_XENIX */
#      define tchars tc
#      include <sys/ttold.h>
#    endif /* M_XENIX */
#  endif /* USGr3 */
#  define bcopy(source, dest, count) memcpy(dest, source, count)
   char *index(s,c) char *s; { char *strchr(); return strchr(s,c); }
   char *rindex(s,c) char *s; { char *strrchr(); return strrchr(s,c); }
#else /* !USG */
#  include <sys/file.h>
#  include <sgtty.h>
#  include <strings.h>
#endif /* USG */

#if !defined (DEFAULT_INFOPATH)
#  define DEFAULT_INFOPATH \
	".:/usr/gnu/info:/usr/local/emacs/info:/usr/local/lib/emacs/info"
#endif /* !DEFAULT_INFOPATH */

typedef struct nodeinfo {
  char *filename;
  char *nodename;
  int pagetop;
  int nodetop;
  struct nodeinfo *next;
} NODEINFO;

typedef struct indirectinfo {
  char *filename;
  int first_byte;
} INDIRECT_INFO;

typedef int Function ();
#define VOID_SIGHANDLER
#if defined (VOID_SIGHANDLER)
#  define SigHandler void
#else
#  define SigHandler int
#endif /* !VOID_SIGHANDLER */
#define barf(msg) fprintf(stderr, "%s\n", msg)

/* Some character stuff. */
#define control_character_threshold 0x020 /* smaller than this is control */
#define meta_character_threshold 0x07f	/* larger than this is Meta. */
#define control_character_bit 0x40	/* 0x000000, must be off. */
#define meta_character_bit 0x080	/* x0000000, must be on. */

#define info_separator_char '\037'
#define start_of_node_string "\037"

#ifdef CTRL
#undef CTRL
#endif

#define CTRL(c) ((c) & (~control_character_bit))
#define META(c) ((c) | meta_character_bit)

#define UNMETA(c) ((c) & (~meta_character_bit))
#define UNCTRL(c) to_upper(((c)|control_character_bit))

#ifndef to_upper
#define to_upper(c) (((c) < 'a' || (c) > 'z') ? (c) : (c) - 32)
#define to_lower(c) (((c) < 'A' || (c) > 'Z') ? (c) : (c) + 32)
#endif

#define CTRL_P(c) ((unsigned char) (c) < control_character_threshold)
#define META_P(c) ((unsigned char) (c) > meta_character_threshold)

#define NEWLINE '\n'
#define RETURN CTRL('M')
#define DELETE 0x07f
#define TAB '\t'
#define ABORT_CHAR CTRL('G')
#define PAGE CTRL('L')
#define SPACE 0x020
#define ESC CTRL('[')
#define control_display_prefix '^'

#define TAG_TABLE_END_STRING "\037\nEND TAG TABLE"
#define TAG_TABLE_BEG_STRING "\nTAG TABLE:\n"
#define NODE_ID "Node:"
#define NNODENAME 4		/* Default amount to grow nodename list by. */
#define FILENAME_LEN 256
#define NODENAME_LEN 256
#define STRING_SIZE 256
#define nodeend_sequence "\n\037"

#define whitespace(c) ((c) == ' ' || (c) == '\t')
#define cr_whitespace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')

/* All right, some windows stuff. */

typedef struct {
  /* Absolute x and y coordinates for usable portion of this window. */
  int left, top, right, bottom;
  /* Absolute cursor position in this window. */
  int ch, cv;
} WINDOW;

typedef struct _wind_list {
  int left, top, right, bottom;
  int ch, cv;
  struct _wind_list *next_window;
} WINDOW_LIST;

WINDOW the_window = {0, 0, 80, 24, 0, 0};
WINDOW_LIST *window_stack = (WINDOW_LIST *)NULL;
WINDOW terminal_window = {0, 0, 80, 24, 0, 0};

/* Not really extern, but defined later in this file. */
extern WINDOW echo_area;
void *xmalloc (), *xrealloc ();
char *getenv (), *next_info_file (), *opsys_filename ();
int build_menu (), find_menu_node ();
void swap_filestack (), pop_filestack ();

/* A crock, this should be done in a different way. */
#define MAX_INDIRECT_FILES 100 

/* The info history list. */
NODEINFO *Info_History = NULL;

/* ?Can't have more than xx files in the indirect list? */
INDIRECT_INFO indirect_list[MAX_INDIRECT_FILES];

/* The filename of the currently loaded info file. */
char current_info_file[FILENAME_LEN];

/* The nodename of the node the user is looking at. */
char current_info_node[NODENAME_LEN];

/* The last file actually loaded.  Not the same as current info file. */
char last_loaded_info_file[FILENAME_LEN];

/* Offsets in info_file of top and bottom of current_info_node. */
int nodetop, nodebot;

/* Number of lines in this node. */
int nodelines;

/* Buffer for the info file. */
char *info_file = NULL;

/* Length of the above buffer. */
int info_buffer_len;

/* Pointer to the start of a tag table, or NULL to show none. */
char *tag_table = NULL;

/* Length of the above buffer. */
int tag_buffer_len;		

/* Non-zero means that the tag table is indirect. */
int indirect = 0;
int indirect_top;

/* Offset in the buffer of the current pagetop. */
int pagetop;

/* Offset in the buffer of the last displayed character. */
int pagebot = 0;

/* If non-NULL, this is a colon separated list of directories to search
   for a specific info file.  The user places this variable into his or
   her environment. */
char *infopath = NULL;

/* If filled, the name of a file to write to. */
char dumpfile[FILENAME_LEN] = "";

/* This is the command to print a node. A default value is compiled in,
   or it can be found from the environment as $INFO_PRINT_COMMAND. */
char *print_command;

/* Non-zero means forst redisplay before prompt for the next command. */
int window_bashed = 0;

/* **************************************************************** */
/*								    */
/*			Getting Started.			    */
/*								    */
/* **************************************************************** */

/* Begin the Info session. */

/* Global is on until we are out of trouble. */
int totally_inhibit_errors = 1;

/* Non-zero means print version info only. */
int version_flag = 0;

/* The major and minor versions of Info. */
int major_version = 1;
int minor_version = 44;

struct option long_options[] = {
  { "directory", 1, 0, 'd' },
  { "node", 1, 0, 'n' },
  { "file", 1, 0, 'f' },
  { "output", 1, 0, 'o' },
  { "version", 0, &version_flag, 1 },
  {NULL, 0, NULL, 0}
};

#define savestring(x) (char *) strcpy ((char *) xmalloc (1 + strlen (x)), (x))

main (argc, argv)
     int argc;
     char **argv;
{
  int c, ind, no_such_node = 0;
  char filename[FILENAME_LEN];
  char *nodename;
  char **nodenames;
  int nodenames_size, nodenames_index;
  char *ptr, *env_infopath, *env_print_command;

  nodenames_index = 0;
  nodenames = (char **) xmalloc ((nodenames_size = 1) * sizeof (char *));
  nodenames[0] = (char *)NULL;

  env_infopath = getenv ("INFOPATH");
  env_print_command = getenv ("INFO_PRINT_COMMAND");

  filename[0] = '\0';

  if (env_infopath && *env_infopath)
    infopath = savestring (env_infopath);
  else
    infopath = savestring (DEFAULT_INFOPATH);

  if (env_print_command && *env_print_command)
    print_command = savestring (env_print_command);
  else
    print_command = savestring (INFO_PRINT_COMMAND);

  while ((c = getopt_long (argc, argv, "d:n:f:o:", long_options, &ind)) != EOF)
    {
      if (c == 0 && long_options[ind].flag == 0)
	c = long_options[ind].val;
      switch (c)
	{
	case 0:
	  break;
	  
	case 'd':
	  free (infopath);
	  infopath = savestring (optarg);
	  break;
	  
	case 'n':

	  if (nodenames_index + 2 > nodenames_size)
	    nodenames = (char **)
	      xrealloc (nodenames, (nodenames_size += 10) * sizeof (char *));

	  nodenames[nodenames_index++] = optarg;
	  nodenames[nodenames_index] = (char *)NULL;
	  break;
	  
	case 'f':
	  strncpy (filename, optarg, FILENAME_LEN);
	  break;
	  
	case 'o':
	  strncpy (dumpfile, optarg, FILENAME_LEN);
	  break;
	  
	default:
	  usage ();
	}
    }

  /* If the user specified `--version' then simply show the version
     info at this time and exit. */
  if (version_flag)
    {
      show_version_info (stdout);
      exit (0);
    }

  /* Okay, flags are parsed.  Get possible Info menuname. */

  if (*filename && (ptr = rindex (filename,'/')) != NULL )
    {
      /* Add filename's directory to path. */
      char *temp;

      temp = (char *) xmalloc (2 + strlen (filename) + strlen (infopath));
      strncpy (temp, filename, ptr - filename);
      sprintf (temp + (ptr - filename), ":%s", infopath);
      free (infopath);
      infopath = temp;
    }

  /* Start with DIR or whatever was specified. */
  if (!get_node (filename, (nodenames[0] == NULL) ? "Top" : nodenames[0], 0))
    {
      if (filename[0])
	{
	  fprintf (stderr, "Try just plain `info'.\n");
	  exit (1);
	}
      else
	{
	  strcpy (filename, "DIR");

	  if (!get_node ((char *)NULL, (char *)NULL, 1))
	    {
	      fprintf (stderr,
		       "%s: Cannot find \"%s\", anywhere along the search ",
		       argv[0], filename);
	      fprintf (stderr, "path of\n\"%s\".\n", infopath);
	      exit (1);
	    }
	}
    }

  totally_inhibit_errors = 0;

  for (ind = 1 ; ind < nodenames_index ; ind++)
    get_node (filename, nodenames[ind], 0);

  nodename = nodenames[nodenames_index > 0 ? nodenames_index - 1 : 0];
  if (!nodename)
    {
      nodename = (char *) xmalloc (NODENAME_LEN);
      *nodename = '\0';
    }
  
  if (optind != argc)
    {
      while (optind != argc)
	{
	  if (!build_menu ())
  	    {
	      display_error ("There is no menu in node \"%s\"",
			     current_info_node );
	      no_such_node++;
	      break;
	    }
	  else if (!find_menu_node (argv[optind], nodename))
	    {
	      display_error
		("There is no menu entry for \"%s\" in node \"%s\"",
		 argv[optind], nodename );
	      no_such_node++;
	      break;
	    }
	  else if (!get_node ((char *)NULL, nodename, 0))
	    {
	      no_such_node++;
	      break;
	    }
	  else
 	    {
#if 1
	      /* RMS says not to type this stuff out because he expects
		 programs to call Info instead of interactive users. */
	      printf ("%s.. ",argv[optind]);
	      fflush (stdout);
#endif
	      optind++;
	    }
	}
    }

  /* If we are outputting to a file, and the node was not found, exit. */
  if (no_such_node && dumpfile[0])
    exit (1);
  else
    begin_info_session ();

  exit (0);
}

usage ()
{
  fprintf (stderr,"%s\n%s\n%s\n%s\n",
"Usage: info [-d dir-path] [-f info-file] [-n node-name -n node-name ...]",
"            [-o output-file] [--directory dir-path] [--file info-file]",
"            [--node node-name --node node-name ...] [--version]",
"            [--output output-file] [menu-selection...]");
  exit (1);
}

/* Print the version of info to standard output. */
show_version_info (stream)
     FILE *stream;
{
  fprintf (stream, "GNU Info version %d.%d\n", major_version, minor_version);
  fflush (stream);
}

#if defined (SIGTSTP)
Function *old_tstp;
Function *old_ttou, *old_ttin;
#endif /* SIGTSTP */

#if defined (SIGWINCH)
Function *old_winch;
#endif /* SIGWINCH */

/* Start using Info. */
begin_info_session ()
{
  int info_signal_handler ();

  /* If the user just wants to dump the node, then do that. */
  if (dumpfile[0])
    {
      dump_current_node (dumpfile);
      exit (0);
    }

  init_terminal_io ();

  /* Install handlers for restoring/breaking the screen. */

  install_signals ();
  new_echo_area ();

  print_string ("Welcome to Info!  Type \"?\" for help. ");
  close_echo_area ();
  toploop ();
  goto_xy (the_window.left, the_window.bottom + 1);
  restore_io ();
}

/* What to do before processing a stop signal. */
before_stop_signal ()
{
  restore_io ();
}

/* What to do after processing a stop signal. */
after_stop_signal ()
{
  clear_screen ();
  display_page ();
  goto_xy (the_window.ch, the_window.cv);
  opsys_init_terminal ();
}

/* Do the right thing with this signal. */
SigHandler
info_signal_handler (sig)
     int sig;
{
  switch (sig)
    {
#if defined (SIGTSTP)
    case SIGTSTP:
    case SIGTTOU:
    case SIGTTIN:
      before_stop_signal ();
      signal (sig, SIG_DFL);
#if !defined (USG)
      sigsetmask (sigblock (0) & ~sigmask (sig));
#endif /* !USG */
      kill (getpid (), sig);
      after_stop_signal ();
      signal (sig, info_signal_handler);
      break;
#endif /* SIGTSTP */

#if defined (SIGWINCH)
    case SIGWINCH:
      /* Window has changed.  Get the size of the new window, and rebuild our
         window chain. */
      {
	int display_page ();
	extern char *widest_line;
	extern WINDOW terminal_window;
	extern WINDOW_LIST *window_stack;
	extern int terminal_rows, terminal_columns;
	int delta_width, delta_height, right, bottom;

	right = get_terminal_columns ();
	bottom = get_terminal_rows ();

	delta_width = right - terminal_columns;
	delta_height = bottom - terminal_rows;

	terminal_columns = right;
	terminal_rows = bottom;

	/* Save current window, whatever it is. */
	push_window ();

	/* Change the value of the widest_line. */
	free (widest_line);
	widest_line = (char *) xmalloc (right);

	/* Make the new window.  Map over all windows in window list. */
	{
	  WINDOW_LIST *wind = window_stack;
	  extern WINDOW modeline_window;

	  while (wind != (WINDOW_LIST *)NULL)
	    {
	      adjust_wind ((WINDOW *)wind, delta_width, delta_height);
	      wind = wind->next_window;
	    }

	  /* Adjust the other windows that we know about. */
	  adjust_wind (&terminal_window, delta_width, delta_height);
	  adjust_wind (&echo_area, delta_width, delta_height);
	  adjust_wind (&modeline_window, delta_width, delta_height);
	}

	/* Clear and redisplay the entire terminal window. */
	set_window (&terminal_window);
	clear_screen ();

	/* Redisplay the contents of the screen. */
	with_output_to_window (&terminal_window, display_page);

	/* Get back the current window. */
	pop_window ();
      }
      break;
#endif /* SIGWINCH */

    case SIGINT:
      restore_io ();
      exit (1);
      break;
    }
}

install_signals ()
{
#if defined (SIGTSTP)
  old_tstp = (Function *) signal (SIGTSTP, info_signal_handler);
  old_ttou = (Function *) signal (SIGTTOU, info_signal_handler);
  old_ttin = (Function *) signal (SIGTTIN, info_signal_handler);
#endif /* SIGTSTP */

#if defined (SIGWINCH)
  old_winch = (Function *) signal (SIGWINCH, info_signal_handler);
#endif /* SIGWINCH */

  signal (SIGINT, info_signal_handler);
}

adjust_wind (wind, delta_width, delta_height)
     WINDOW *wind;
     int delta_width, delta_height;
{
  wind->right += delta_width;
  wind->bottom += delta_height;
  wind->ch += delta_width;
  wind->cv += delta_height;

  /* Ugly hack to fix busted windows code.  If the window we are adjusting
     already has a TOP offset, then adjust that also. */
  if (wind->top)
    wind->top += delta_height;
}

/* **************************************************************** */
/*								    */
/*			Completing Things			    */
/*								    */
/* **************************************************************** */

typedef struct completion_entry
{
  char *identifier;
  char *data;
  struct completion_entry *next;
}                COMP_ENTRY;

/* The linked list of COMP_ENTRY structures that you create. */
COMP_ENTRY *completion_list = (COMP_ENTRY *) NULL;

/* The vector of COMP_ENTRY pointers that COMPLETE returns. */
COMP_ENTRY **completions = NULL;

/* The number of elements in the above vector. */
int completion_count;

/* Initial size of COMPLETIONS array. */
#define INITIAL_COMPLETIONS_CORE_SIZE 200

/* Current size of the completion array in core. */
int completions_core_size = 0;

/* Ease the typing task.  Another name for the I'th
   IDENTIFIER of COMPLETIONS. */
#define completion_id(i) ((completions[(i)])->identifier)

/* The number of completions that can be present before the help
   function starts asking you about whether it should print them
   all or not. */
int completion_query_threshold = 100;

free_completion_list ()
{
  COMP_ENTRY *temp;
  while (completion_list)
    {
      temp = completion_list;

      if (completion_list->identifier)
	free (completion_list->identifier);

      if (completion_list->data)
	free (completion_list->data);

      completion_list = completion_list->next;
      free (temp);
    }
}

/* Add a single completion to COMPLETION_LIST.
   IDENTIFIER is the string that the user should type.
   DATA should just be a pointer to some random data that you wish to
   have associated with the identifier, but I'm too stupid for that, so
   it must be a string as well.  This allocates the space for the strings
   so you don't necessarily have to. */
add_completion (identifier, data)
     char *identifier, *data;
{
  COMP_ENTRY *temp = (COMP_ENTRY *) xmalloc (sizeof (COMP_ENTRY));

  temp->identifier = (char *) xmalloc (strlen (identifier) + 1);
  strcpy (temp->identifier, identifier);

  temp->data = (char *) xmalloc (strlen (data) + 1);
  strcpy (temp->data, data);

  temp->next = completion_list;
  completion_list = temp;
}

/* Function for reading a line.  Supports completion on COMPLETION_LIST
   if you pass COMPLETING as non-zero.  Prompt is either a prompt or
   NULL, LINE is the place to store the characters that are read.
   LINE may start out already containing some characters; if so, they
   are printed.  MAXCHARS tells how many characters can fit in the
   buffer at LINE.  readline () returns zero if the user types the
   abort character.  LINE is returned with a '\0' at the end, not a '\n'. */
int
readline (prompt, line, maxchars, completing)
     char *prompt, *line;
     int maxchars;
     int completing;
{
  int character;
  int readline_ch, readline_cv;
  int current_len = strlen (line);
  int just_completed = 0;		/* Have we just done a completion? */
  int meta_flag = 0;

  new_echo_area ();

  if (prompt)
    print_string ("%s", prompt);

  readline_ch = the_window.ch;
  readline_cv = the_window.cv;

  print_string ("%s", line);

  while (1)
    {
      line[current_len] = '\0';
      goto_xy (readline_ch, readline_cv);
      print_string ("%s", line);
      clear_eol ();

      if (just_completed)
	just_completed--;

      character = blink_cursor ();
      if (meta_flag)
	{
	  character = META (character);
	  meta_flag = 0;
	}

      if (META_P (character))
	character = META (to_upper (UNMETA (character)));

      switch (character)
	{
	case EOF:
	  character = '\n';

	case ESC:
	  meta_flag++;
	  break;

	case META (DELETE):
	case CTRL ('W'):
	  while (current_len && line[current_len] == SPACE)
	    current_len--;

	  if (!current_len)
	    break;

	  while (current_len && line[current_len] != SPACE)
	    current_len--;

	  break;

	case CTRL ('U'):
	  current_len = 0;
	  break;

	case '\b':
	case 0x07f:
	  if (current_len)
	    current_len--;
	  else
	    ding ();
	  break;

	case '\n':
	case '\r':
	  if (completing)
	    {
	      extern int completion_count;

	      try_complete (line);

	      if (completion_count >= 1)
		{
		  close_echo_area ();
		  return (1);
		}
	      else
		{
		  current_len = strlen (line);
		  break;
		}
	    }
	  else
	    {
	      close_echo_area ();
	      return (1);
	    }

	case ABORT_CHAR:
	  ding ();

	  if (current_len)
	    {
	      current_len = 0;
	    }
	  else
	    {
	      close_echo_area ();
	      clear_echo_area ();
	      return (0);
	    }
	  break;

	case ' ':
	case '\t':
	case '?':
	  if (completing)
	    {
	      extern int completion_count;

	      if (character == '?' || just_completed)
		{
		  help_possible_completions (line);
		  break;
		}
	      else
		{
		  char temp_line[NODENAME_LEN];
		  strcpy (temp_line, line);
		  try_complete (line); just_completed = 2;
		  if (completion_count != 1 && character == SPACE)
		    {
		      if (strcmp (temp_line, line) == 0)
			{
			  line[current_len] = SPACE;
			  line[current_len + 1] = '\0';
			  strcpy (temp_line, line);
			  try_complete (line);
			  if (completion_count == 0)
			    {
			      line[current_len] = '\0';
			      ding ();
			    }
			}
		    }
		  current_len = strlen (line);
		  if (completion_count == 0)
		    ding ();
		  break;
		}
	    }
	  /* Do *NOT* put anything in-between the completing cases and
	     the default: case.  No.  Because the SPC, TAB and `?' get
	     treated as normal characters by falling through the
	     "if (completing)" test above. */
	default:
	  if (!CTRL_P (character) &&
	      !META_P (character) &&
	      current_len < maxchars)
	    line[current_len++] = character;
	  else
	    ding ();
	}
    }
}

/* Initialize whatever the completer is using. */
init_completer ()
{
  if (completions_core_size != INITIAL_COMPLETIONS_CORE_SIZE)
    {
      if (completions)
	free (completions);

      completions = (COMP_ENTRY **)
	xmalloc ((sizeof (COMP_ENTRY *))
		 * (completions_core_size = INITIAL_COMPLETIONS_CORE_SIZE));
    }
  completion_count = 0;
}

/* Reverse the completion list passed in LIST, and
   return a pointer to the new head. */
COMP_ENTRY *
reverse_list (list)
     COMP_ENTRY *list;
{
  COMP_ENTRY *next;
  COMP_ENTRY *prev = (COMP_ENTRY *) NULL;

  while (list)
    {
      next = list->next;
      list->next = prev;
      prev = list;
      list = next;
    }
  return (prev);
}

/* Remember the possible completion passed in POINTER on the
   completions list. */
remember_completion (pointer)
     COMP_ENTRY *pointer;
{
  if (completion_count == completions_core_size)
    {
      COMP_ENTRY **temp = (COMP_ENTRY **)
	realloc (completions, ((sizeof (COMP_ENTRY *))
			       * (completions_core_size +=
				  INITIAL_COMPLETIONS_CORE_SIZE)));
      if (!temp)
	{
	  display_error ("Too many completions (~d)!  Out of core!",
			 completion_count);
	  return;
	}
      else
	completions = temp;
    }
  completions[completion_count++] = pointer;
}

/* Complete TEXT from identifiers in LIST.  Place the resultant
   completions in COMPLETIONS, and the number of completions in
   COMPLETION_COUNT. Modify TEXT to contain the least common
   denominator of all the completions found. */
int
complete (text, list)
     char *text;
     COMP_ENTRY *list;
{
  int low_match, i, idx;
  int string_length = strlen (text);

  init_completer ();
  low_match = 100000;		/* Some large number. */

  while (list)
    {
      if (strnicmp (text, list->identifier, string_length) == 0)
	remember_completion (list);
      list = list->next;
    }

  if (completion_count == 0)
    return (0);

  if (completion_count == 1)
    {				/* One completion */
      strcpy (text, completion_id (0));
      return (1);
    }

  /* Else find the least common denominator */

  idx = 1;

  while (idx < completion_count)
    {
      int c1, c2;
      for (i = 0;
	   (c1 = to_lower (completion_id (idx - 1)[i])) &&
	   (c2 = to_lower (completion_id (idx)[i]));
	   i++)
	if (c1 != c2)
	  break;

      if (low_match > i)
	low_match = i;
      idx++;
    }

  strncpy (text, completion_id (0), low_match);
  text[low_match] = '\0';
  return (1);
}

/* Complete TEXT from the completion structures in COMPLETION_LIST. */
int
try_complete (text)
     char *text;
{
  return (complete (text, completion_list));
}

/* The function that prints out the possible completions. */
help_possible_completions (text)
     char *text;
{
  char temp_string[2000];

  goto_xy (the_window.left, the_window.top);
  strcpy (temp_string, text);
  try_complete (temp_string);

  open_typeout ();

  if (completion_count == 0)
    {
      print_string ("There are no possible completions.\n");
      goto print_done;
    }

  if (completion_count == 1)
    {
      print_string
	("The only possible completion of what you have typed is:\n\n");
      print_string ("%s", completion_id(0));
      goto print_done;
    }

  if (completion_count >= completion_query_threshold)
    {
      print_string
	("\nThere are %d completions.  Do you really want to see them all",
	 completion_count);

      if (!get_y_or_n_p ())
	return;
    }

  print_string ("\nThe %d completions of what you have typed are:\n\n",
		completion_count);

  {
    int idx = 0;
    int counter = 0;
    int columns = (the_window.right - the_window.left) / 30;

    while (idx < completion_count)
      {
	if (counter == columns)
	  {
	    charout ('\n');
	    counter = 0;
	  }
	indent_to (counter * 30);
	print_string ("%s", completion_id (idx));
	counter++;
	idx++;
      }
  }

print_done:
  print_string ("\n\n-----------------\n");
  close_typeout ();
}

/* Return the next file that should be searched, or NULL if we are at the end
   of the info file. If the FILE argument is provided, begin the search there,
   if REWIND is non-zero start the search at the beginning of the list.

   The list is the one built by an indirect tag table, on the supposition
   that those files form a logical set to search if we are in one of them.
   If no such list is current (either it doesn't exist, or FILE isn't on
   it) the search list is set to be last_loaded_info_file */
char *
next_info_file (file, rewind)
     char *file;		/* file to set `next' to. May be NULL. */
     int rewind;		/* should I rewind the file list?  */
{
  static int index = -1;

  if (file != NULL)
    {
      char *ptr = rindex (file,'/');

      if (ptr != NULL)
	file = ptr + 1;

      for (index = 0;
	   index < MAX_INDIRECT_FILES &&
	   indirect_list[index].filename != (char *)NULL;
	   index++)
	{
	  if (strcmp (file, indirect_list[index].filename) == 0)
	    return (file);
	}

      /* OK, we are not on the current indirect_list. This means that
	 we have switched to another node that has no indirect list,
	 so forget the old one. */
      for (index = 0;
	   index < MAX_INDIRECT_FILES &&
	   indirect_list[index].filename != (char *)NULL;
	   index++)
	{
	  free (indirect_list[index].filename);
	  indirect_list[index].filename = (char *)NULL;
	}
      return (indirect_list[0].filename = savestring (file));
    }
  else if (rewind)
    {
      index = 0;

      if (indirect_list[0].filename == (char *)NULL)
	indirect_list[0].filename = savestring (last_loaded_info_file);
    }
  else
    index++;
  
  if (index < MAX_INDIRECT_FILES &&
      indirect_list[index].filename != (char *)NULL)
    return (indirect_list[index].filename);

  index = -1;
  return (NULL);
}

/* **************************************************************** */
/*								    */
/*			Getting Nodes				    */
/*								    */
/* **************************************************************** */

/* A node name looks like:
   Node: nodename with spaces but not a comma,
or Node: (filename-containing-node)node-within-file
or Node: (filename)

   The latter case implies a nodename of "Top".  All files are
   supposed to have one.

   Lastly, the nodename specified could be "*", which specifies the
   entire file. */

/* Return the directory portion of FILENAME, i.e., everything before the
   last slash. */
static char *
file_directory (filename)
     char *filename;
{
  register char *scan;
  register int length;
  char *result;

  scan = filename;

  while (*scan++ != '\0');

  while (1)
    {
      if (scan == filename)
	break;

      if ((*--scan) == '/')
	{
	  scan++;
	  break;
	}
    }

  length = scan - filename;
  result = (char *) xmalloc (length + 1);
  strncpy (result, filename, length);
  result[length] = '\0';

  return (result);
}

/* Given FILENAME and DIRECTORY return a newly allocated string which
   is either the two concatenated, or simply FILENAME if it is absolute
   already. */
static char *
file_absolutize (filename, directory)
     char *filename, *directory;
{
  register int filename_len, directory_len;
  char *result;

  if (filename[0] == '/')
    return (savestring (filename));

  filename_len = strlen (filename);
  directory_len = strlen (directory);
  result = (char *) xmalloc (directory_len + filename_len + 1);

  strcpy (result, directory);
  strcat (result, filename);

  return (result);
}

/* Load FILENAME.  If REMEMBER_NAME is non-zero, then remember the
   loaded filename in CURRENT_INFO_FILE.  In either case, remember
   the name of this file in LAST_LOADED_INFO_FILE. */
int
get_info_file (filename, remember_name)
     char *filename;
     int remember_name;
{
  FILE *input_stream;
  struct stat file_info;
  int pointer, result;
  char tempname[FILENAME_LEN];

  /* Get real filename. */
  strcpy (tempname, opsys_filename (filename));

  /* If the file doesn't exist, try again with the name in lower case. */
  result = stat (tempname, &file_info);

  if (result < 0)
    {
      register int i;
      char *lowered_name;

      lowered_name = (char *)xmalloc (1 + strlen (filename));

      for (i = 0; lowered_name[i] = to_lower (filename[i]); i++)
	;

      strcpy (tempname, opsys_filename (lowered_name));
      result = stat (tempname, &file_info);
    }

  /* See if this file is the last loaded one. */
  if (!result && (strcmp (last_loaded_info_file, tempname) == 0))
    return (1);

  /* Now try to open the file. */
  if (result || (input_stream = fopen (tempname, "r")) == NULL)
    {
      file_error (tempname);
      return (0);
    }

  /* If we already have a file loaded, then free it first. */
  if (info_file)
    {
      free (info_file);

      if (!indirect)
	{
	  /* Then the tag table is also no longer valid. */
	  tag_table = (char *) NULL;
	}
    }

  /* Read the contents of the file into a new buffer. */

  info_file = (char *) xmalloc (info_buffer_len = file_info.st_size);
  fread (info_file, 1, info_buffer_len, input_stream);
  fclose (input_stream);
  strcpy (last_loaded_info_file, tempname);
  if (remember_name)
    {
      strcpy (current_info_file, tempname);
      if (indirect)
	{
	  int idx;
	  indirect = 0;
	  free (tag_table);
	}
    }
  else
    return (1);

  /* Force redisplay, since we are looking at a new file. */
  window_bashed = 1;

  /* The file has been read, and we don't know anything about it.
     Find out if it contains a tag table. */

  tag_table = NULL;		/* assume none. */
  indirect = 0;
  tag_buffer_len = 0;

  set_search_constraints (info_file, info_buffer_len);

  /* Go to the last few lines in the file. */
  pointer = back_lines (8, info_buffer_len);
  pointer = search_forward (TAG_TABLE_END_STRING, pointer);

  if (pointer > -1)
    {
      /* Then there is a tag table.  Find the start of it,
	 and remember that. */
      pointer = search_backward (TAG_TABLE_BEG_STRING, pointer);

      /* Handle error for malformed info file. */
      if (pointer < 0)
	display_error ("Start of tag table not found!");
      else
	{
	  /* No problem.  If this file is an indirect file, then the contents
	     of the tag table must remain in RAM the entire time.  Otherwise,
	     we can flush the tag table with the file when the file is flushed.
	     So, if indirect, remember that, and copy the table to another
	     place.*/

	  int indirect_check = forward_lines (2, pointer);

	  tag_table = info_file + pointer;
	  tag_buffer_len = info_buffer_len - pointer;

	  /* Shorten the search constraints. */
	  info_buffer_len = pointer;

	  if (looking_at ("(Indirect)\n", indirect_check))
	    {
	      /* We have to find the start of the indirect file's
		 information. */
	      tag_table = (char *) xmalloc (tag_buffer_len);

	      bcopy (&info_file[indirect_check], tag_table, tag_buffer_len);

	      /* Find the list of filenames. */
	      indirect_top = search_backward ("Indirect:\n", indirect_check);
	      if (indirect_top < 0)
		{
		  free (tag_table);
		  tag_table = (char *) NULL;
		  display_error ("Start of INDIRECT tag table not found!");
		  return (0);
		}

	      /* Remember the filenames, and their byte offsets. */
	      {
		/* Index into the filename/offsets array. */
		int idx, temp_first_byte;
		char temp_filename[FILENAME_LEN];
		char *directory = file_directory (tempname);

		info_buffer_len = indirect_top;

		/* For each line, scan the info into globals.  Then save
	           the information in the INDIRECT_INFO structure. */

		for (idx = 0; idx < MAX_INDIRECT_FILES &&
		     indirect_list[idx].filename != (char *) NULL;
		     idx++)
  		  {
		     free (indirect_list[idx].filename);
		     indirect_list[idx].filename = (char *) NULL;
		  }
		
		for (idx = 0;info_file[indirect_top] != info_separator_char &&
		     idx < MAX_INDIRECT_FILES;)
		  {
		    indirect_top = forward_lines (1, indirect_top);
		    if (info_file[indirect_top] == info_separator_char)
		      break;

		    /* Ignore blank lines. */
		    if (info_file[indirect_top] == '\n')
		      continue;

		    sscanf (&info_file[indirect_top], "%s%d",
			    temp_filename, &temp_first_byte);

		    if (strlen (temp_filename))
		      {
			temp_filename[strlen (temp_filename) - 1] = '\0';
			indirect_list[idx].filename =
			  file_absolutize (temp_filename, directory);
			indirect_list[idx].first_byte = temp_first_byte;
			idx++;
		      }
		  }

		free (directory);

		/* Terminate the table. */
		if (idx == MAX_INDIRECT_FILES)
		  {
		    display_error
		      ("Sorry, the INDIRECT file array isn't large enough.");
		    idx--;
		  }
		indirect_list[idx].filename = (char *) NULL;
	      }
	      indirect = 1;
	   } else {
	      ;
	   }
	}
    }
  return (1);
}

/* Make current_info_node be NODENAME.  This could involve loading
   a file, etc.  POPPING is non-zero if we got here because we are
   popping one level. */
int
get_node (filename, nodename, popping)
     char *nodename, *filename;
     int popping;
{
  int pointer;
  char internal_filename[FILENAME_LEN];
  char internal_nodename[NODENAME_LEN];

  if (nodename && *nodename)
    {
      /* Maybe nodename looks like: (filename)nodename, or worse: (filename).
         If so, extract the stuff out. */
      if (*nodename == '(')
	{
	  int temp = 1, temp1 = 0;
	  char character;

	  filename = internal_filename;

	  while ((character = nodename[temp]) && character != ')')
	    {
	      filename[temp - 1] = character;
	      temp++;
	    }
	  filename[temp - 1] = '\0';
	  temp++;			/* skip the closing ')' */

	  /* We have the filename now.  The nodename follows. */
	  internal_nodename[0] = '\0';

	  while (nodename[temp] == ' ' ||
		 nodename[temp] == '\t' ||
		 nodename[temp] == '\n')
	    temp++;

	  if (nodename[temp])
	    while (internal_nodename[temp1++] = nodename[temp++])
	      ;
	  else if (*filename != '\0')
	    strcpy (internal_nodename,"Top");

	  nodename = internal_nodename;
	}
    }

  if (!popping)
    push_node (current_info_file, current_info_node, pagetop, nodetop);

  if (!nodename || !*nodename)
    {
      nodename = internal_nodename;
      strcpy (nodename, "Top");
    }

  if (!filename || !*filename)
    {
      filename = internal_filename;
      strcpy (filename, current_info_file);
    }

  if (!*filename)
    strcpy (filename, "DIR");

  if (!get_info_file (filename, 1))
    goto node_not_found;

  if (strcmp (nodename, "*") == 0)
    {
      /* The "node" that we want is the entire file. */
      pointer = 0;
      goto found_node;
    }
  
  /* If we are using a tag table, see if we can find the nodename in it. */
  if (tag_table)
    {
      pointer = find_node_in_tag_table (nodename, 0);
      if (pointer < 1)
	{
	  int pop_node ();

	  /* The search through the tag table failed.  Maybe we
	     should try searching the buffer?  Nahh, just barf. */
	node_not_found:
	  if (popping)
	    return (0);	/* Second time through. */

	  {
	     int save_inhibit = totally_inhibit_errors;

	     totally_inhibit_errors = 0;
	     display_error
	       ("Sorry, unable to find the node \"%s\" in the file \"%s\".",
		nodename, filename);
	     totally_inhibit_errors = save_inhibit;
	  }

	  current_info_file[0] = '\0';
	  current_info_node[0] = '\0';
	  last_loaded_info_file[0] = '\0';
	  pop_node (internal_filename, internal_nodename, &nodetop, &pagetop);
	  get_node (internal_filename, internal_nodename, 1);
	  return (0);
	}

      /* Otherwise, the desired location is right here.
         Scarf the position byte. */
      while (tag_table[pointer] != '\177')
	pointer++;

      sscanf (&tag_table[pointer + 1], "%d", &pointer);

      /* Okay, we have a position pointer.  If this is an indirect file,
         then we should look through the indirect_list for the first
         element.first_byte which is larger than this.  Then we can load
         the specified file, and win. */
      if (indirect)
	{
	  /* Find the filename for this node. */
	  int idx;
	  for (idx = 0; idx < MAX_INDIRECT_FILES &&
	       indirect_list[idx].filename != (char *) NULL; idx++)
	    {
	      if (indirect_list[idx].first_byte > pointer)
		{
		  /* We found it. */
		  break;
		}
	    }
	  if (!get_info_file (indirect_list[idx - 1].filename, 1))
	    goto node_not_found;
	  pointer -= indirect_list[idx - 1].first_byte;

	  /* Here is code to compensate for the header of an indirect file. */
	  {
	    int tt = find_node_start (0);
	    if (tt > -1)
	      pointer += tt;
	  }
	}
      else
	{
	  /* This tag table is *not* indirect.  The filename of the file
	     containing this node is the same as the current file.  The
	     line probably looks like:
	     File: info,  Node: Checking25796 */
	}
    }
  else
    {
#if defined (NOTDEF)
      /* We don't have a tag table.  The node can only be found by
         searching this file in its entirety.  */
      if (!get_info_file (filename, 1))
	return (0);
#endif /* NOTDEF */
      pointer = 0;
    }

  /* Search this file, using pointer as a good guess where to start. */
  /* This is the same number that RMS used.  It might be right or wrong. */
  pointer -= 1000;
  if (pointer < 0)
    pointer = 0;

  pointer = find_node_in_file (nodename, pointer);
  if (pointer < 0)
    goto node_not_found;

  /* We found the node in its file.  Remember exciting information. */

found_node:
  back_lines (0, pointer);
  nodetop = pagetop = pointer;
  strcpy (current_info_node, nodename);
  strcpy (current_info_file, filename);
  get_node_extent ();
  return (1);
}

/* Get the bounds for this node.  NODETOP points to the start of the
   node. Scan forward looking for info_separator_char, and remember
   that in NODEBOT. */
get_node_extent ()
{
  int idx = nodetop;
  int character;
  int do_it_till_end = (strcmp (current_info_node, "*") == 0);

  nodelines = 0;

again:
  while ((idx < info_buffer_len) &&
	 ((character = info_file[idx]) != info_separator_char))
    {
      if (character == '\n')
	nodelines++;
      idx++;
    }
  if (do_it_till_end && idx != info_buffer_len)
    {
      idx++;
      goto again;
    }
  nodebot = idx;
}

/* Locate the start of a node in the current search_buffer.  Return
   the offset to the node start, or minus one.  START is the place in
   the file at where to begin the search. */
find_node_start (start)
     int start;
{
  return (search_forward (start_of_node_string, start));
}

/* Find NODENAME in TAG_TABLE. */
find_node_in_tag_table (nodename, offset)
     char *nodename;
     int offset;
{
  int temp;

  set_search_constraints (tag_table, tag_buffer_len);

  temp = offset;
  while (1)
    {
      offset = search_forward (NODE_ID, temp);

      if (offset < 0)
	return (offset);

      temp = skip_whitespace (offset + strlen (NODE_ID));

      if (strnicmp (tag_table + temp, nodename, strlen (nodename)) == 0)
	if (*(tag_table + temp + strlen (nodename)) == '\177')
	  return (temp);
    }
}

/* Find NODENAME in INFO_FILE. */
find_node_in_file (nodename, offset)
     char *nodename;
     int offset;
{
  int temp, last_offset = -1;

  set_search_constraints (info_file, info_buffer_len);

  while (1)
    {
      offset = find_node_start (offset);

      if (offset == last_offset)
	offset = -1;
      else
	last_offset = offset;

      if (offset < 0)
	return (offset);
      else
	temp = forward_lines (1, offset);

      if (temp == offset)
	return (-1);		/* At last line now, just a node start. */
      else
	offset = temp;

      temp = string_in_line (NODE_ID, offset);

      if (temp > -1)
	{
	  temp = skip_whitespace (temp + strlen (NODE_ID));
	  if (strnicmp (info_file + temp, nodename, strlen (nodename)) == 0)
	    {
	      int check_exact = *(info_file + temp + strlen (nodename));

	      if (check_exact == '\t' ||
		  check_exact == ',' ||
		  check_exact == '.' ||
		  check_exact == '\n')
		return (offset);
	    }
	}
    }
}


/* **************************************************************** */
/*								    */
/*		    Dumping and Printing Nodes			    */
/*								    */
/* **************************************************************** */

/* Make a temporary filename based on STARTER and the PID of this Info. */
char *
make_temp_filename (starter)
     char *starter;
{
  register int i;
  char *temp;

  temp = (char *) xmalloc (strlen (starter) + 10);
  sprintf (temp, "%s-%d", starter, getpid ());

  for (i = 0; temp[i]; i++)
    if (!isalnum (temp[i]))
      temp[i] = '-';

  return (temp);
}

/* Delete a file.  Print errors if necessary. */
deletefile (filename)
     char *filename;
{
  if (unlink (filename) != 0)
    {
      file_error (filename);
      return (1);
    }
  return (0);
}

printfile (filename)
     char *filename;
{
  int length = strlen (print_command) + strlen (filename) + strlen ("\n") + 1;
  char *command = (char *) xmalloc (length);
  int error;

  display_error ("Printing file `%s'...\n", filename);
  sprintf (command, "%s %s", print_command, filename);
  error = system (command);
  if (error)
    display_error ("Can't invoke `%s'", command);
  free (command);
  return (error);
}

/* Dump the current node into a file named FILENAME.
   Return 0 if the dump was successful, otherwise,
   print error and exit. */
dump_current_node (filename)
     char *filename;
{
  int c, i = nodetop;
  FILE *output_stream;

  if (strcmp (filename, "-") == 0)
    output_stream = stdout;
  else
    output_stream = fopen (filename, "w");

  if (output_stream == (FILE *) NULL)
    {
      file_error (filename);
      return (1);
    }

  while (i < nodebot && i < info_buffer_len)
    {
      c = info_file[i];
      if (CTRL_P (c) && !(index ("\n\t\f", c)))
	{
	  putc ('^', output_stream);
	  c = UNCTRL (c);
	}

      if (putc (c, output_stream) == EOF)
	{
	  if (output_stream != stdout)
	    fclose (output_stream);

	  file_error (filename);
	  return (1);
	}
      i++;
    }

  if (output_stream != stdout)
    fclose (output_stream);

  return (0);
}

/* **************************************************************** */
/*								    */
/*			 Toplevel eval loop. 			    */
/*								    */
/* **************************************************************** */

#define MENU_HEADER "\n* Menu:"
#define MENU_ID "\n* "
#define FOOTNOTE_HEADER "*Note"

/* Number of items that the current menu has. */
int the_menu_size = 0;

/* The node that last made a menus completion list. */
char menus_nodename[NODENAME_LEN];
char menus_filename[NODENAME_LEN];

static int search_start = 0;

/* The default prompt string for the Follow Reference command. */
char *visible_footnote = (char *)NULL;
toploop ()
{
  int done, inhibit_display;
  int command, last_command;
  int last_pointer, count, new_ypos, last_pagetop;
  char nodename[NODENAME_LEN];

  done = inhibit_display = 0;
  command = last_command = 0;
  new_ypos = last_pagetop = -1;

  while (!done)
    {
      if (!inhibit_display &&
	  (window_bashed || (pagetop != last_pagetop)))
	display_page ();

      inhibit_display = window_bashed = 0;
      last_pagetop = pagetop;

      nodename[0] = '\0';	/* Don't display old text in input line. */

      last_command = command;

      if (last_command == 'S')
	cursor_to (search_start);
      else
	goto_xy (echo_area.left, echo_area.top);

      command = blink_cursor ();
      clear_echo_area ();

      if (command == EOF)
	{
	  done = 1;
	  continue;
	}
      command = to_upper (command);

      switch (command)
	{
	case 'D':
	  get_node ((char *) NULL, "(dir)Top", 0);
	  break;

	case 'H':
	  if ((the_window.bottom - the_window.top) < 24)
	    get_node ((char *) NULL, "(info)Help-Small-Screen", 0);
	  else
	    get_node ((char *) NULL, "(info)Help", 0);
	  break;

	case 'N':
	  if (!next_node ())
	    {
	      display_error ("No NEXT for this node!");
	      inhibit_display = 1;
	    }
	  break;

	case 'P':
	  if (!prev_node ())
	    {
	      display_error ("No PREV for this node!");
	      inhibit_display = 1;
	    }
	  break;

	case 'U':
	  {
	    int savetop = pagetop;

	    if (!up_node ())
	      {
		display_error ("No UP for this node!");
		inhibit_display = 1;
		pagetop = savetop;
	      }
	    break;
	  }

	case 'M':
	  if (!build_menu ())
	    {
	      display_error ("No menu in this node!");
	      inhibit_display = 1;
	      break;
	    }

	  if (!readline ("Menu item: ", nodename, NODENAME_LEN, 1))
	    {
	      clear_echo_area ();
	      inhibit_display = 1;
	      break;
	    }

	  I_goto_xy (echo_area.left, echo_area.top);
	  if (!find_menu_node (nodename, nodename))
	    {
	      display_error ("\"%s\" is not a menu item!", nodename);
	      inhibit_display = 1;
	      break;
	    }

	  if (get_node ((char *) NULL, nodename, 0))
	    clear_echo_area ();
	  break;

	case 'F':
	  {
	    char footnote[NODENAME_LEN];

	    if (!build_notes ())
	      {
		display_error ("No cross-references in this node!");
		inhibit_display = 1;
		break;
	      }

	    strcpy (footnote, visible_footnote);
	    if (!readline ("Follow reference: ", footnote, NODENAME_LEN, 1))
	      {
		inhibit_display = 1;
		break;
	      }

	    I_goto_xy (echo_area.left, echo_area.top);
	    if (!find_note_node (footnote, nodename))
	      {
		display_error ("\"%s\" is not a cross-reference in this node!",
			       footnote);
		inhibit_display = 1;
		break;
	      }

	    if (get_node ((char *)NULL, nodename, 0))
	      clear_echo_area ();
	    break;
	  }

	case 'L':
	  {
	    char filename[FILENAME_LEN], nodename[NODENAME_LEN];
	    int ptop, ntop;
	    if (pop_node (filename, nodename, &ntop, &ptop) &&
		get_node (filename, nodename, 1))
	      {
		pagetop = ptop;
	      }
	    else
	      inhibit_display = 1;
	    break;
	  }

	case SPACE:
	case CTRL ('V'):
	  if (!next_page ())
	    {
	      display_error ("At last page of this node now!");
	      inhibit_display = 1;
	    }
	  break;

	case META ('V'):
	case DELETE:
	  if (!prev_page ())
	    {
	      display_error ("At first page of this node now!");
	      inhibit_display = 1;
	    }
	  break;

	case 'B':
	  if (pagetop == nodetop)
	    {
	      display_error ("Already at beginning of this node!");
	      inhibit_display = 1;
	    }
	  else
	    pagetop = nodetop;
	  break;

	  /* I don't want to do this this way, but the documentation
	     clearly states that '6' doesn't work.  It states this for a
	     reason, and ours is not to wonder why... */
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	  {
	    int item = command - '0';

	    if (!build_menu ())
	      {
		display_error ("No menu in this node!");
		inhibit_display = 1;
		break;
	      }

	    if (item > the_menu_size)
	      {
		display_error ("There are only %d items in the menu!",
			       the_menu_size);
		inhibit_display = 1;
		break;
	      }

	    if (!get_menu (item))
	      inhibit_display = 1;
	  }
	  break;

	case 'G':
	  if (!readline ("Goto node: ", nodename, NODENAME_LEN, 0))
	    {
	      inhibit_display = 1;
	      break;
	    }

	  if (get_node ((char *) NULL, nodename, 0))
	    clear_echo_area ();
	  break;

	  /* Search from the starting position forward for a string.
	     Select the node containing the desired string.  Put the
	     top of the page screen_lines / 2 lines behind it, but not
	     before nodetop. */
	case 'S':
	  {
	    int pointer, temp;
 	    char prompt[21 + NODENAME_LEN + 1];
 	    static char search_string[NODENAME_LEN] = "";
 	    static char *starting_filename = NULL,
	    		*starting_nodename = NULL;
	    static int starting_pagetop = 0;
	    static int wrap_search = 0;

 	    sprintf (prompt, "Search for string [%s]: ", search_string);

 	    if (!readline (prompt , nodename, NODENAME_LEN, 0))
	      {
		inhibit_display = 1;
		break;
	      }

	    /* If the user defaulted the search string, and the previous
	       command was search, then this is a continuation of the
	       previous search. */
	    if (((strcmp (nodename, search_string) == 0) ||
		 (!*nodename && *search_string)) &&
		(last_command == 'S'))
	      {
		search_start++;
	      }
	    else
	      {
		/* Initialize the start of a new search. */
		if (starting_filename)
		  free (starting_filename);

		starting_filename = savestring (last_loaded_info_file);

		if (starting_nodename)
		  free (starting_nodename);

		starting_nodename = savestring (nodename ? nodename : "");

		starting_pagetop = pagetop;
		search_start = pagetop;
		wrap_search = 0;

		if (*nodename != '\0')
		  strcpy (search_string, nodename);
	      }

	    I_goto_xy (echo_area.left, echo_area.top);

	    {
	      static int pushed = 0; /* How many files are pushed? */
	      int found_string = 0;  /* Did we find our string? */

	      if (wrap_search)
		{
		  push_filestack (next_info_file ((char *)NULL, 1), 0);
		  pushed++;
		  search_start = 0;
		}

	      for (;;)
		{
		  set_search_constraints (info_file, info_buffer_len);
		  pointer = search_forward (search_string, search_start);

		  if (pointer != -1)
		    {
		      found_string = 1;
		      break;
		    }
		  else
		    {
		      char *next_file;

		      next_file = next_info_file ((char *)NULL, 0);

		      if (next_file != NULL)
			{
			  if (pushed)
			    {
			      pop_filestack ();
			      pushed--;
			    }

			  push_filestack (next_file, 0);
			  pushed++;
			  search_start = 0;
#if 1
			  I_goto_xy (echo_area.left, echo_area.top);
			  print_string ("Searching file %s...\n", next_file);
#endif
			  continue;
			}

		      if (wrap_search)
			{
			  display_error
			    ("\"%s\" not found!",
			     search_string);

			  inhibit_display = 1;
			  wrap_search = 0;

			  if (pushed)
			    {
			      pop_filestack ();
			      pushed--;
			    }
			  break;
			}
		      else
			{
			  display_error ("Search: End of file");
			  inhibit_display = 1;
			  wrap_search = 1;

			  if (pushed)
			    {
			      pop_filestack ();
			      pushed--;
			    }
			  break;
			}
		    }
		}

	      if (pushed)
		{
		  swap_filestack ();
		  pop_filestack ();
		  pushed--;
		}

	      if (!found_string)
		break;

	      wrap_search = 0;
	      temp = search_backward (start_of_node_string, pointer);

	      if (temp != -1)
 		{
		  search_start = pointer;
		  pointer = forward_lines (1, temp);
		}

	      if (temp == -1 || !extract_field ("Node:", nodename, pointer))
		{
		  display_error
		    ("There doesn't appear to be a nodename for this node.");
		  get_node ((char *)NULL, "*", 0);
		  pagetop = pointer;
		  break;
		}
 		
	      /* Get the node if it is different than the one already
		 loaded. */
	      if (strcmp (nodename, starting_nodename) != 0)
		{
		  free (starting_nodename);
		  starting_nodename = savestring (nodename);

		  if (get_node ((char *) NULL, nodename, 0))
		    clear_echo_area ();
		}
 		
	      /* Reset the top of page if necessary. */
	      {
		if ((strcmp (last_loaded_info_file, starting_filename) != 0) ||
		    (starting_pagetop != pagetop) ||
		    (search_start > pagebot))
		  {
		    pointer =
		      back_lines ((the_window.bottom - the_window.top) / 2,
				  forward_lines (1, search_start));

		    if (pointer < nodetop)
		      pointer = nodetop;
 		  
		    pagetop = pointer;
		    window_bashed = 1;
		  }
		else
		  inhibit_display = 1;
	      }
	      break;
	    }
   	  }

	case CTRL ('H'):
	case '?':
	  help_use_info ();
	  last_pagetop = -1;
	  break;

	case 'Q':
	  done = 1;
	  break;

	case CTRL ('L'):	/* Control-l is redisplay. */
	  window_bashed = 1;
	  if (last_command == 'S')
	    command = 'S';
	  break;

	case '(':    /* You *must* be trying to type a complete nodename. */
	  strcpy (nodename, "(");
	  if (!readline ("Goto node: ", nodename, NODENAME_LEN, 0))
	    {
	      inhibit_display = 1;
	      clear_echo_area ();
	      break;
	    }
	  I_goto_xy (echo_area.left, echo_area.top);
	  if (get_node ((char *) NULL, nodename, 0))
	    clear_echo_area ();
	  break;

	case CTRL ('P'):
	  /* Print the contents of this node on the default printer.  We
	     would like to let the user specify the printer, but we don't
	     want to ask her each time which printer to use.  Besides, he
	     might not know, which is why it (the user) is in the need of
	     Info. */
	  {
	    char *tempname = make_temp_filename (current_info_node);
	    if (dump_current_node (tempname) == 0 &&
		printfile (tempname) == 0 &&
		deletefile (tempname) == 0)
	      {
		display_error ("Printed node.  Go pick up your output.\n");
	      }
	    inhibit_display = 1;
	    free (tempname);
	  }
	  break;

	default:
	  inhibit_display = 1;
	  display_error ("Unknown command! Press '?' for help.");

	}
    }
}

/* Return the screen column width that the line from START to END
   requires to display. */
line_length (start, end)
     int start, end;
{
  int count = 0;

  while (start < end)
    {
      if (info_file[start] == '\t')
	count += 7 - (count % 8);
      else if (CTRL_P (info_file[start]))
	count += 2;
      else
	count++;

      start++;
    }

  return (count);
}

/* Tell this person how to use Info. */
help_use_info ()
{
  open_typeout ();
  clear_screen ();
  print_string ("\n\
          Commands in Info\n\
\n\
h	Invoke the Info tutorial.\n\
\n\
Selecting other nodes:\n\
n	Move to the \"next\" node of this node.\n\
p	Move to the \"previous\" node of this node.\n\
u	Move \"up\" from this node.\n\
m	Pick menu item specified by name.\n\
	Picking a menu item causes another node to be selected.\n\
f	Follow a cross reference.  Reads name of reference.\n\
l	Move to the last node you were at.\n\
d	Move to the `directory' node.  Equivalent to `gDIR'.\n\
\n\
Moving within a node:\n\
Space	Scroll forward a page.\n\
DEL	Scroll backward a page.\n\
b	Go to the beginning of this node.\n\
\n\
Advanced commands:\n\
q	Quit Info.\n\
1	Pick first item in node's menu.\n\
2 - 5   Pick second ... fifth item in node's menu.\n\
g	Move to node specified by name.\n\
	You may include a filename as well, as (FILENAME)NODENAME.\n\
s	Search through this Info file for a specified string,\n\
	and select the node in which the next occurrence is found.\n\
Ctl-p   Print the contents of this node using `%s'.\n\
\n\
Done.\n\n",print_command);
  close_typeout ();
}

/* Move to the node specified in the NEXT field. */
int
next_node ()
{
  char nodename[NODENAME_LEN];

  if (!extract_field ("Next:", nodename, nodetop))
    return (0);
  return (get_node ((char *) NULL, nodename, 0));
}

/* Move to the node specified in the PREVIOUS field. */
int
prev_node ()
{
  char nodename[NODENAME_LEN];

  if (!extract_field ("Previous:", nodename, nodetop)
      && !extract_field ("Prev:", nodename, nodetop))
    return (0);
  return (get_node ((char *) NULL, nodename, 0));
}

/* Move to the node specified in the UP field. */
int
up_node ()
{
  char nodename[NODENAME_LEN];

  if (!extract_field ("Up:", nodename, nodetop))
    return (0);
  return (get_node ((char *) NULL, nodename, 0));
}

/* Build a completion list of menuname/nodename for each
   line in this node that is a menu item. */
int
build_menu ()
{
  int pointer = nodetop;
  char menuname[NODENAME_LEN];
  char nodename[NODENAME_LEN];

  if (strcmp (menus_nodename, current_info_node) == 0 &&
      strcmp (menus_filename, current_info_file) == 0)
    return (the_menu_size != 0);

  strcpy (menus_nodename, current_info_node);
  strcpy (menus_filename, current_info_file);
  free_completion_list ();
  the_menu_size = 0;

  set_search_constraints (info_file, nodebot);
  if ((pointer = search_forward (MENU_HEADER, nodetop)) < 0)
    return (0);

  /* There is a menu here.  Look for members of it. */
  pointer += strlen (MENU_HEADER);

  while (1)
    {
      int idx;

      pointer = search_forward (MENU_ID, pointer);
      if (pointer < 0)
	break;			/* No more menus in this node. */

      pointer = (skip_whitespace (pointer + strlen (MENU_ID)));

      idx = 0;
      while ((menuname[idx] = info_file[pointer]) && menuname[idx] != ':')
	idx++, pointer++;

      menuname[idx] = '\0';
      pointer++;

      if (info_file[pointer] == ':')
	{
	  strcpy (nodename, menuname);
	}
      else
	{
	  int in_parens;

	  pointer = skip_whitespace (pointer);
	  idx = in_parens = 0;

	  while ((nodename[idx] = info_file[pointer]) &&
		 (in_parens ||
		  (nodename[idx] != '\t' &&
		   nodename[idx] != '.' &&
		   nodename[idx] != ',')))
	    {
	      if (nodename[idx] == '(')
		in_parens++;
	      else if (nodename[idx] == ')')
		in_parens--;

	      idx++, pointer++;
	    }
	  nodename[idx] = '\0';
	}
      add_completion (menuname, nodename);
      the_menu_size++;
    }
  if (the_menu_size)
    completion_list = reverse_list (completion_list);
  return (the_menu_size != 0);
}

/* Select ITEMth item from a list built by build_menu (). */
int
get_menu (item)
     int item;
{
  if (!build_menu ())
    return (0);

  if (item > the_menu_size)
    return (0);
  else
    {
      COMP_ENTRY *temp = completion_list;

      while (--item && temp)
	temp = temp->next;

      return (get_node ((char *) NULL, temp->data, 0));
    }
}

/* Scan through the ?already? built menu list looking
   for STRING.  If you find it, put the corresponding nodes
   name in NODENAME. */
int
find_menu_node (string, nodename)
     char *string, *nodename;
{
  return (scan_list (string, nodename));
}

/* The work part of find_menu_node and find_note_node. */
int
scan_list (string, nodename)
     char *string, *nodename;
{
  COMP_ENTRY *temp = completion_list;

  while (temp)
    {
      if (stricmp (string, temp->identifier, strlen (string)) == 0)
	{
	  strcpy (nodename, temp->data);
	  return (1);
	}
      temp = temp->next;
    }
  return (0);
}

/* Remove <CR> and whitespace from string, replacing them with
   only one space.  Exception:  <CR> at end of string disappears. */
clean_up (string)
     char *string;
{
  char *to;

  /* Skip all whitespace characters found at the start of STRING. */
  while (whitespace (*string))
    string++;

  to = string;

  while (*to = *string++)
    {
      if (*to == '\n' || *to == ' ')
	{
	  *to = ' ';

	  while (*string == ' ' || *string == '\t')
	    string++;
	}
	to++;
    }
}

/* Find a reference to "*Note".  Return the offset of the start
   of that reference, or -1. */
find_footnote_ref (from)
     int from;
{
  while (1)
    {
      from = search_forward (FOOTNOTE_HEADER, from);
      if (from < 0)
	return (from);
      else
	from += strlen (FOOTNOTE_HEADER);
      if (info_file[from] == ' ' ||
	  info_file[from] == '\n' ||
	  info_file[from] == '\t')
	return (from);
    }
}

/* Build an array of (footnote.nodename) for each footnote in this node. */
int
build_notes ()
{
  int pointer;
  char notename[NODENAME_LEN];
  char nodename[NODENAME_LEN];

  set_search_constraints (info_file, nodebot);

  if ((find_footnote_ref (nodetop)) < 0)
    return (0);
  pointer = nodetop;

  menus_filename[0] = menus_nodename[0] = '\0';
  visible_footnote = "";
  free_completion_list ();

  while (1)
    {
      int idx;

      pointer = find_footnote_ref (pointer);
      if (pointer < 0)
	break;			/* no more footnotes in this node. */

      pointer = skip_whitespace_and_cr (pointer);
      idx = 0;

      while ((notename[idx] = info_file[pointer]) && notename[idx] != ':')
	{
	  idx++, pointer++;
	}

      notename[idx] = '\0';
      clean_up (notename);
      pointer++;
      if (info_file[pointer] == ':')
	{
	  strcpy (nodename, notename);
	}
      else
	{
	  int in_parens = 0;

	  pointer = skip_whitespace (pointer);
	  idx = 0;

	  while ((nodename[idx] = info_file[pointer]) &&
		 (in_parens ||
		  (nodename[idx] != '\t' &&
		   nodename[idx] != '.' &&
		   nodename[idx] != ',')))
	    {
	      if (nodename[idx] == '(')
		in_parens++;
	      else if (nodename[idx] == ')')
		in_parens--;

	      idx++, pointer++;
	    }
	  nodename[idx] = '\0';
	  clean_up (nodename);
	}
      /* Add the notename/nodename to the list. */
      add_completion (notename, nodename);
      the_menu_size++;

      /* Remember this identifier as the default if it is the first one in the
         page. */
      if (!(*visible_footnote) &&
	  pointer > pagetop &&
      pointer < forward_lines (the_window.bottom - the_window.top, pointer))
	visible_footnote = completion_list->identifier;
    }
  if (the_menu_size)
    completion_list = reverse_list (completion_list);
  return (the_menu_size != 0);
}

/* Scan through the ?already? built footnote list looking for STRING.
   If found, place the corresponding node name in NODENAME. */
int
find_note_node (string, nodename)
     char *string, *nodename;
{
  return (scan_list (string, nodename));
}

/* **************************************************************** */
/*								    */
/*			Page Display 				    */
/*								    */
/* **************************************************************** */


/* The display functions for GNU Info. */
int display_ch, display_cv;
int display_point;

/* Display the current page from pagetop down to the bottom of the
   page or the bottom of the node, whichever comes first. */
display_page ()
{
  display_point = pagetop;
  display_ch = the_window.left;
  display_cv = the_window.top;
  generic_page_display ();
}

/* Print the page from display_point to bottom of node, or window,
   whichever comes first.  Start printing at display_ch, display_cv. */
generic_page_display ()
{
  int done_with_display = 0;
  int character;

  goto_xy (display_ch, display_cv);

  while (!done_with_display)
    {
      if (display_point == nodebot)
	{
	  clear_eop ();
	  goto display_finish;
	}

      character = info_file[display_point];

      if ((display_width (character, the_window.ch) + the_window.ch)
	  >= the_window.right)
	display_carefully (character);
      else
	charout (character);

      if ((the_window.cv >= the_window.bottom)
	  || (the_window.cv == the_window.top
	      && the_window.ch == the_window.left))
	{
	display_finish:
	  pagebot = display_point;
	  make_modeline ();
	  done_with_display++;
	  continue;
	}
      else
	display_point++;
    }
  fflush (stdout);
}

/* Display character carefully, ensuring that no scrolling takes
   place, even in the case of funky control characters. */
display_carefully (character)
     int character;
{
  if (CTRL_P (character))
    {
      switch (character)
	{
	case RETURN:
	case NEWLINE:
	case TAB:
	  clear_eol ();
	  advance (the_window.right - the_window.ch);
	  break;
	default:
	  charout ('^');
	  if (the_window.cv == the_window.bottom)
	    break;
	  else
	    charout (UNCTRL (character));
	}
    }
  else
    charout (character);
}

/* Move the cursor to POSITION in page.  Return non-zero if successful. */
cursor_to (position)
     int position;
{
  int ch, cv, character;
  int point;

  if (position > pagebot || position < pagetop)
    return (0);

  point = pagetop;
  ch = the_window.left;
  cv = the_window.top;

  while (point < position)
    {
      character = info_file[point++];

      ch += display_width (character, ch);

      if (ch >= the_window.right)
	{
	  ch = ch - the_window.right;
	  cv++;

	  if (cv >= the_window.bottom)
	    return (0);
	}
    }
  goto_xy (ch, cv);
  return (1);
}

/* Move to the next page in this node.  Return 0 if
   we can't get to the next page. */
int
next_page ()
{
  int pointer;

  pointer =
    forward_lines ((the_window.bottom - the_window.top) - 2, pagetop);

  if (pointer >= nodebot)
    return (0);

  /* Hack for screens smaller than displayed line width. */
  if (pointer > display_point)
    {
      pointer = display_point;
      back_lines (1);
    }
  pagetop = pointer;
  return (1);
}

/* Move to the previous page in this node.  Return zero if
   there is no previous page. */
int
prev_page ()
{
  int pointer =
  back_lines ((the_window.bottom - the_window.top) - 2, pagetop);

  if (pagetop == nodetop)
    return (0);

  if (pointer < nodetop)
    pointer = nodetop;

  pagetop = pointer;
  return (1);
}


/* **************************************************************** */
/*								    */
/*			Utility Functions			    */
/*								    */
/* **************************************************************** */

char *search_buffer;		/* area in ram to scan through. */
int buffer_bottom;		/* Length of this area. */

/* Set the global variables that all of these routines use. */
set_search_constraints (buffer, extent)
     char *buffer;
     int extent;
{
  search_buffer = buffer;
  buffer_bottom = extent;
}

/* Move back to the start of this line. */
to_beg_line (from)
     int from;
{
  while (from && search_buffer[from - 1] != '\n')
    from--;
  return (from);
}

/* Move forward to the end of this line. */
to_end_line (from)
{
  while (from < buffer_bottom && search_buffer[from] != '\n')
    from++;
  return (from);
}

/* Move back count lines in search_buffer starting at starting_pos.
   Returns the start of that line. */
back_lines (count, starting_pos)
     int count, starting_pos;
{
  starting_pos = to_beg_line (starting_pos);
  while (starting_pos && count)
    {
      starting_pos = to_beg_line (starting_pos - 1);
      count--;
    }
  return (starting_pos);
}

/* Move forward count lines starting at starting_pos.
   Returns the start of that line. */
forward_lines (count, starting_pos)
     int count, starting_pos;
{
  starting_pos = to_end_line (starting_pos);
  while (starting_pos < buffer_bottom && count)
    {
      starting_pos = to_end_line (starting_pos + 1);
      count--;
    }
  return (to_beg_line (starting_pos));
}

/* Search for STRING in SEARCH_BUFFER starting at STARTING_POS.
   Return the location of the string, or -1 if not found. */
search_forward (string, starting_pos)
     char *string;
     int starting_pos;
{
  register int c, i, len;
  register char *buff, *end;
  char *alternate;


  /* We match characters in SEARCH_BUFFER against STRING and ALTERNATE.
     ALTERNATE is a case reversed version of STRING; this is cheaper than
     case folding each character before comparison. */

  /* Build the alternate string. */
  alternate = savestring (string);
  len = strlen (string);

  for (i = 0; i < len; i++)
    {
      c = alternate[i];

      if (c >= 'a' && c <= 'z')
	alternate[i] = c - 32;
      else if (c >= 'A' && c <= 'Z')
	alternate[i] = c + 32;
    }

  buff = search_buffer + starting_pos;
  end = search_buffer + buffer_bottom + 1;

  while (buff < end)
    {
      for (i = 0; i < len; i++)
	{
	  c = buff[i];

	  if (c != string[i] && c != alternate[i])
	    break;
	}

      if (!string[i])
	{
	  free (alternate);
	  return (buff - search_buffer);
	}

      buff++;
    }

  free (alternate);
  return (-1);
}

/* Search for STRING in SEARCH_BUFFER starting at STARTING_POS.
   Return the location of the string, or -1 if not found. */
search_backward (string, starting_pos)
     char *string;
     int starting_pos;
{
  int len = strlen (string);
  while (starting_pos - len > -1)
    {
      if (strnicmp (search_buffer + (starting_pos - len), string, len) == 0)
	return (starting_pos - len);
      else
	starting_pos--;
    }
  return (-1);
}

/* Only search for STRING from POINTER to end of line.  Return offset
   of string, or -1 if not found. */
string_in_line (string, pointer)
     char *string;
     int pointer;
{
  int old_buffer_bottom = buffer_bottom;

  set_search_constraints (search_buffer, to_end_line (pointer));
  pointer = search_forward (string, pointer);
  buffer_bottom = old_buffer_bottom;
  return (pointer);
}

/* Skip whitespace characters at OFFSET in SEARCH_BUFFER.
   Return the next non-whitespace character or -1 if BUFFER_BOTTOM
   is reached. */
skip_whitespace (offset)
     int offset;
{
  int character;

  while (offset < buffer_bottom)
    {
      character = search_buffer[offset];
      if (character == ' ' || character == '\t')
	offset++;
      else
	return (offset);
    }
  return (-1);
}

/* Skip whitespace characters including <CR> at OFFSET in
   SEARCH_BUFFER.  Return the position of the next non-whitespace
   character, or -1 if BUFFER_BOTTOM is reached. */
skip_whitespace_and_cr (offset)
     int offset;
{
  while (1)
    {
      offset = skip_whitespace (offset);
      if (offset > 0 && search_buffer[offset] != '\n')
	return (offset);
      else
	offset++;
    }
}

/* Extract the node name part of the of the text after the FIELD.
   Place the node name into NODENAME.  Assume the line starts at
   OFFSET in SEARCH_BUFFER. */
int
extract_field (field_name, nodename, offset)
     char *field_name, *nodename;
     int offset;
{
  int temp, character;

  temp = string_in_line (field_name, offset);
  if (temp < 0)
    return (0);

  temp += strlen (field_name);
  temp = skip_whitespace (temp);

  /* Okay, place the following text into NODENAME. */

  while ((character = search_buffer[temp]) != ','
	 && character != '\n'
	 && character != '\t')
    {
      *nodename = character;
      nodename++;
      temp++;
    }
  *nodename = '\0';
  return (1);
}

/* Return non-zero if pointer is exactly at string, else zero. */
int
looking_at (string, pointer)
     char *string;
     int pointer;
{
  if (strnicmp (search_buffer + pointer, string, strlen (string)) == 0)
    return (1);
  else
    return (0);
}

/* File stack stuff. This is currently only used to push one file while
   searching indirect files, but we may as well write it in full
   generality. */
typedef struct filestack
{
  struct filestack *next;
  char filename[FILENAME_LEN];
  char current_filename[FILENAME_LEN];
  char *tag_table;
  char *info_file;
  int info_buffer_len;
} FILESTACK;

FILESTACK *filestack = NULL;

int
push_filestack (filename, remember_name)
     char *filename;
     int remember_name;
{
  FILESTACK *element = (FILESTACK *) xmalloc (sizeof (FILESTACK));

  element->next = filestack;
  filestack = element;

  strcpy (filestack->filename, last_loaded_info_file);
  strcpy (filestack->current_filename, current_info_file);
  filestack->tag_table = tag_table;
  filestack->info_file = info_file;
  filestack->info_buffer_len = info_buffer_len;
  
  *last_loaded_info_file = '\0';	/* force the file to be read */
  info_file = (char *)NULL;	/* Pretend we have no buffer. */
  if (get_info_file (filename, remember_name))
    {
      return (1);
    }
  else
    {
      pop_filestack ();
      return (0);
    }
}

void
pop_filestack ()
{
  FILESTACK *temp;
   
  if (filestack == NULL)
    {
       fprintf (stderr , "File stack is empty and can't be popped\n");
       brians_error ();
       return;
    }

  free (info_file);

  strcpy (last_loaded_info_file, filestack->filename);
  strcpy (current_info_file, filestack->current_filename);
  tag_table = filestack->tag_table;
  info_file = filestack->info_file;
  info_buffer_len = filestack->info_buffer_len;
  set_search_constraints (info_file, info_buffer_len);

  temp = filestack;
  filestack = filestack->next;
  free ((char *)temp);
}

/* Swap the current info file with the bottom of the filestack */
void
swap_filestack ()
{
  char t_last_loaded_info_file[FILENAME_LEN];
  char t_current_info_file[FILENAME_LEN];
  char *t_tag_table;
  char *t_info_file;
  int t_info_buffer_len;

  if (filestack == NULL)
    {
       fprintf (stderr , "File stack is empty and can't be swapped\n");
       brians_error ();
       return;
    }

  strcpy (t_last_loaded_info_file, filestack->filename);
  strcpy (t_current_info_file, filestack->current_filename);
  t_tag_table = filestack->tag_table;
  t_info_file = info_file;
  t_info_buffer_len = info_buffer_len;

  strcpy (filestack->filename, last_loaded_info_file);
  strcpy (filestack->current_filename, current_info_file);
  filestack->tag_table = tag_table;
  filestack->info_file = info_file;
  filestack->info_buffer_len = info_buffer_len;

  strcpy (last_loaded_info_file, t_last_loaded_info_file);
  strcpy (current_info_file, t_current_info_file);
  tag_table = t_tag_table;
  info_file = t_info_file;
  info_buffer_len = t_info_buffer_len;
}

/* Now the node history stack */

extern NODEINFO *Info_History;

/* Save the current filename, nodename, and position on the history list.
   We prepend. */
int
push_node (filename, nodename, page_position, node_position)
     char *filename, *nodename;
     int page_position, node_position;
{
  NODEINFO *newnode = (NODEINFO *) xmalloc (sizeof (NODEINFO));

  newnode->next = Info_History;

  newnode->filename = (char *) xmalloc (strlen (filename) + 1);
  strcpy (newnode->filename, filename);

  newnode->nodename = (char *) xmalloc (strlen (nodename) + 1);
  strcpy (newnode->nodename, nodename);

  newnode->pagetop = page_position;
  newnode->nodetop = node_position;

  Info_History = newnode;
  return (1);
}

/* Pop one node from the node list, leaving the values in
   passed variables. */
int
pop_node (filename, nodename, nodetop, pagetop)
     char *filename, *nodename;
     int *nodetop, *pagetop;
{
  if (Info_History->next == (NODEINFO *) NULL)
    {
      display_error ("At beginning of history now!");
      return (0);
    }
  else
    {
      NODEINFO *releaser = Info_History;

	/* If the popped file is not the current file, then force
	   the popped file to be loaded. */
      if (strcmp (Info_History->filename, last_loaded_info_file) != 0)
	last_loaded_info_file[0] = '\0';

      strcpy (filename, Info_History->filename);
      strcpy (nodename, Info_History->nodename);
      *pagetop = Info_History->pagetop;
      *nodetop = Info_History->nodetop;
      free (Info_History->nodename);
      free (Info_History->filename);
      Info_History = Info_History->next;
      free (releaser);
      return (1);
    }
}

/* Whoops, Unix doesn't have strnicmp. */

/* Compare at most COUNT characters from string1 to string2.  Case
   doesn't matter. */
int
strnicmp (string1, string2, count)
     char *string1, *string2;
{
  char ch1, ch2;

  while (count)
    {
      ch1 = *string1++;
      ch2 = *string2++;
      if (to_upper (ch1) == to_upper (ch2))
	count--;
      else
	break;
    }
  return (count);
}

/* Compare string1 to string2.  Case doesn't matter. */
int
stricmp (string1, string2)
     char *string1, *string2;
{
  char ch1, ch2;

  while (1)
    {
      ch1 = *string1++;
      ch2 = *string2++;

      if (ch1 == '\0')
	return (ch2 != '\0');

      if ((ch2 == '\0') ||
	  (to_upper (ch1) != to_upper (ch2)))
	return (1);
    }
}

/* Make the user type "Y" or "N". */
int 
get_y_or_n_p ()
{
  int character;
  print_string (" (Y or N)?");
  clear_eol ();

until_we_like_it:

  character = blink_cursor ();
  if (character == EOF)
    return (0);
  if (to_upper (character) == 'Y')
    {
      charout (character);
      return (1);
    }

  if (to_upper (character) == 'N')
    {
      charout (character);
      return (0);
    }

  if (character == ABORT_CHAR)
    {
      ding ();
      return (0);
    }

  goto until_we_like_it;
}

/* Move the cursor to the desired column in the window. */
indent_to (screen_column)
     int screen_column;
{
  int counter = screen_column - the_window.ch;
  if (counter > 0)
    {
      while (counter--)
	charout (' ');
    }
  else if (screen_column != 0)
    charout (' ');
}


/* **************************************************************** */
/*								    */
/*			Error output/handling.			    */
/*								    */
/* **************************************************************** */

/* Display specific error from known file error table. */
file_error (file)
     char *file;
{
  extern int errno;
  extern int sys_nerr;
  extern char *sys_errlist[];

  if (errno < sys_nerr)
    display_error ("%s: %s", file, sys_errlist[errno]);
  else
    display_error ("%s: Unknown error %d", file, errno);
}

/* Display the error in the echo-area using format_string and args.
   This is a specialized interface to printf. */
display_error (format_string, arg1, arg2)
     char *format_string;
{
  extern int terminal_inited_p;
  char output_buffer[1024];

  if (totally_inhibit_errors)
    return;

  sprintf (output_buffer, format_string, arg1, arg2);
  if (terminal_inited_p)
    {
      new_echo_area ();
      ding ();
      print_string ("%s", output_buffer);
      close_echo_area ();
    }
  else
    {
      fprintf (stderr, "%s\n", output_buffer);
    }
}

/* Tell everybody what a loser I am.  If you see this error,
   send me a bug report. */
brians_error ()
{
  display_error ("You are never supposed to see this error.\n");
  display_error ("Tell bfox@ai.mit.edu to fix this someday.\n");
  return (-1);
}

/* **************************************************************** */
/*								    */
/*			Terminal IO, and Driver			    */
/*								    */
/* **************************************************************** */

/* The Unix termcap interface code. */

#define NO_ERROR 0
#define GENERIC_ERROR 1
#define NO_TERMINAL_DESCRIPTOR 2
#define OUT_OF_MEMORY 3
#define BAD_TERMINAL 4

#define FERROR(msg)	fprintf (stderr, msg); exit (GENERIC_ERROR)

extern int tgetnum (), tgetflag ();
extern char *tgetstr ();
extern char *tgoto ();

#define Certainly_enough_space 2048	/* page 3, Section 1.1, para 4 */

#if defined (unix)
char termcap_buffer[Certainly_enough_space];
#else /* !unix */
#define termcap_buffer NULL
#endif /* !unix */

/* You CANNOT remove these next four vars.  TERMCAP needs them to operate. */
char PC;
char *BC;
char *UP;

/* A huge array of stuff to get from termcap initialization. */

#define tc_int 0
#define tc_char tc_int+1
#define tc_flag tc_char+1
#define tc_last tc_flag+1

typedef int flag;

/* First, the variables which this array refers to */

/* Capabilities */

int terminal_columns;		/* {tc_int, "co" */
int terminal_rows;		/* {tc_int, "li" */
flag terminal_is_generic;	/* {tc_flag,"gn" */

 /* Cursor Motion */

char *terminal_goto;		/* {tc_char,"cm" */
char *terminal_home;		/* {tc_char,"ho" */

char *terminal_cursor_left;	/* {tc_char,"le" */
char *terminal_cursor_right;	/* {tc_char,"nd" */
char *terminal_cursor_up;	/* {tc_char,"up" */
char *terminal_cursor_down;	/* {tc_char,"do" */

/* Screen Clearing */

char *terminal_clearpage;	/* {tc_char,"cl" */
char *terminal_clearEOP;	/* {tc_char,"cd" */
char *terminal_clearEOL;	/* {tc_char,"ce" */

/* "Standout" */
char *terminal_standout_begin;	/* {tc_char,"so" */
char *terminal_standout_end;	/* {tc_char,"se" */

/* Reverse Video */
char *terminal_inverse_begin;	/* {tc_char,"mr" */
char *terminal_end_attributes;	/* {tc_char,"me" */

/* Ding! */

char *terminal_ear_bell;	/* {tc_char,"bl" */

/* Terminal Initialization */

char *terminal_use_begin;	/* {tc_char,"ti" */
char *terminal_use_end;		/* {tc_char,"te" */

/* Padding Stuff */

char *terminal_padding;		/* {tc_char,"pc" */

/* Now the whopping big array */

typedef struct {
  char type;
  char *name;
  char *value;
} termcap_capability_struct;

termcap_capability_struct capabilities[] = {

/* Capabilities */
  
  {tc_int, "co", (char *) &terminal_columns},
  {tc_int, "li", (char *) &terminal_rows},
  {tc_flag, "gn", (char *) &terminal_is_generic},

/* Cursor Motion */

  {tc_char, "cm", (char *) &terminal_goto},
  {tc_char, "ho", (char *) &terminal_home},

  {tc_char, "le", (char *) &terminal_cursor_left},
  {tc_char, "nd", (char *) &terminal_cursor_right},
  {tc_char, "up", (char *) &terminal_cursor_up},
  {tc_char, "do", (char *) &terminal_cursor_down},

/* Screen Clearing */
  
  {tc_char, "cl", (char *) &terminal_clearpage},
  {tc_char, "cd", (char *) &terminal_clearEOP},
  {tc_char, "ce", (char *) &terminal_clearEOL},
  
/* "Standout" */
  {tc_char, "so", (char *) &terminal_standout_begin},
  {tc_char, "se", (char *) &terminal_standout_end},

/* Reverse Video */
  {tc_char, "mr", (char *) &terminal_inverse_begin},
  {tc_char, "me", (char *) &terminal_end_attributes},

/* Ding! */

  {tc_char, "bl", (char *) &terminal_ear_bell},
  
/* Terminal Initialization */

  {tc_char, "ti", (char *) &terminal_use_begin},
  {tc_char, "te", (char *) &terminal_use_end},

/* Padding Stuff */
  
  {tc_char, "pc", (char *) &terminal_padding},

/* Terminate this array with a var of type tc_last */
  {tc_last, NULL, NULL}

};

int terminal_opened_p = 0;

open_terminal_io ()
{
  int error;

  if (terminal_opened_p)
    return (NO_ERROR);

  if ((error = get_terminal_info ()) != NO_ERROR)
    return (error);

  if ((error = get_terminal_vars (capabilities)) != NO_ERROR)
    return (error);

  /* Now, make sure we have the capabilites that we need. */
  if (terminal_is_generic)
    return (BAD_TERMINAL);

  terminal_opened_p++;
  return (NO_ERROR);
}

get_terminal_info ()
{
  char temp_string_buffer[256];
  int result;

  char *terminal_name = getenv ("TERM");

  if (terminal_name == NULL || *terminal_name == 0
      || (strcmp (terminal_name, "dialup") == 0))
    {
      terminal_name = temp_string_buffer;
      printf ("\nTerminal Type:");
      fflush (stdout);
      fgets (terminal_name, 256, stdin);
      if (!(*terminal_name))
	return (NO_TERMINAL_DESCRIPTOR);
    }

/* #define VERBOSE_GET_TERMINAL 1 */
#ifdef VERBOSE_GET_TERMINAL

#define buffer_limit 256
#define opsys_termcap_filename "/etc/termcap"

  /* We hack question mark if that is what the user typed.  All this means
     is we read /etc/termcap, and prettily print out the names of terminals
     that we find. */

  if (terminal_name[0] == '?' && !terminal_name[1])
    {
      FILE *termcap_file;
      if ((termcap_file = fopen (opsys_termcap_filename, "r")) != NULL)
	{
	  int result;
	  char line_buffer[buffer_limit];
	  int terminal_count = 0;

	  while ((readline_termcap (termcap_file, line_buffer)) != EOF)
	    {
	      char first_char = *line_buffer;
	      if (first_char == '#' || first_char == ' '
		  || first_char == '\t' || first_char == '\n')
		;
	      else
		{
		  /* Print the names the pretty way. */
		  printf ("\n%s", line_buffer);	/* liar */
		  terminal_count++;
		}
	    }
	  fclose (termcap_file);

	  if (terminal_count)
	    printf ("\n%d terminals listed.\n", terminal_count);
	  else
	    printf ("\nNo terminals were listed.  Brian's mistake.\n");
	}
      else
	{
	  fprintf (stderr,
		   "\nNo such system file as %s!\nWe lose badly.\n",
		   opsys_termcap_filename);
	  return (NO_TERMINAL_DESCRIPTOR);
	}
      return (get_terminal_info ());
    }
#endif /* VERBOSE_GET_TERMINAL */

  result = tgetent (termcap_buffer, terminal_name);

  if (!result)
    return (NO_TERMINAL_DESCRIPTOR);
  else
    return (NO_ERROR);
}

#ifdef VERBOSE_GET_TERMINAL
readline_termcap (stream, buffer)
     FILE *stream;
     char *buffer;
{
  int c;
  int buffer_index = 0;

  while ((c = getc (stream)) != EOF && c != '\n')
    {
      if (buffer_index != buffer_limit - 1)
	buffer[buffer_index++] = c;
    }

  buffer[buffer_index] = 0;

  if (c == EOF)
    return ((buffer_index) ? 0 : EOF);
  else
    return (0);
}
#endif /* VERBOSE_GET_TERMINAL */

/* For each element of "from_array", read the corresponding variable's
   value into the right place. */
get_terminal_vars (from_array)
     termcap_capability_struct from_array[];
{
  int i;
  register termcap_capability_struct *item;
  char *buffer;

#if !defined (GNU_TERMCAP)
  buffer = (char *) xmalloc (sizeof (termcap_buffer) + 1);
# define buffer_space &buffer
#else
# define buffer_space 0
#endif

  for (i = 0; (item = &from_array[i]) && (item->type != tc_last); i++)
    {
      switch (item->type)
	{
	case tc_int:
	  *((int *) (item->value)) = tgetnum (item->name);
	  break;

	case tc_flag:
	  *((int *) item->value) = tgetflag (item->name);
	  break;

	case tc_char:
	  *((char **) item->value) = tgetstr (item->name, buffer_space);
	  break;

	default:
	  FERROR ("Bad entry scanned in tc_struct[].\n \
	       Ask bfox@ai.mit.edu to fix this someday.\n");
	}
    }

  PC = terminal_padding ? terminal_padding[0] : 0;
  BC = terminal_cursor_left;
  UP = terminal_cursor_up;
  return (NO_ERROR);
}

/* Return the number of rows this terminal has. */
int
get_terminal_rows ()
{
  int rows = 0;

#if defined (TIOCGWINSZ)
  {
    int tty;
    struct winsize size;

    tty = fileno (stdin);

    if (ioctl (tty, TIOCGWINSZ, &size) != -1)
      rows = size.ws_row;
  }
#endif /* TIOCGWINSZ */

  if (!rows)
    rows = tgetnum ("li");

  if (rows <= 0)
    rows = 24;

  return (rows);
}

/* Return the number of columns this terminal has. */
get_terminal_columns ()
{
  int columns = 0;

#if defined (TIOCGWINSZ)
  {
    int tty;
    struct winsize size;

    tty = fileno (stdin);

    if (ioctl (tty, TIOCGWINSZ, &size) != -1)
      columns = size.ws_col;
  }
#endif /* TIOCGWINSZ */

  if (!columns)
    columns = tgetnum ("co");

  if (columns <= 0)
    columns = 80;

  return (columns);
}

/* #define TERMINAL_INFO_PRINTING */
#ifdef TERMINAL_INFO_PRINTING

/* Scan this (already "get_terminal_vars"ed) array, printing out the
   capability name, and value for each entry.  Pretty print the value
   so that the terminal doesn't actually do anything, shitbrain. */
show_terminal_info (from_array)
     termcap_capability_struct from_array[];
{
  register int i;
  register termcap_capability_struct *item;

  for (i = 0; ((item = &from_array[i]) && ((item->type) != tc_last)); i++)
    {

      char *type_name;
      switch (item->type)
	{
	case tc_int:
	  type_name = "int ";
	  break;
	case tc_flag:
	  type_name = "flag";
	  break;
	case tc_char:
	  type_name = "char";
	  break;
	default:
	  type_name = "Broken";
	}

      printf ("\t%s\t%s = ", type_name, item->name);

      switch (item->type)
	{
	case tc_int:
	case tc_flag:
	  printf ("%d", *((int *) item->value));
	  break;
	case tc_char:
	  tc_pretty_print (*((char **) item->value));
	  break;
	}
      printf ("\n");
    }
}

/* Print the contents of string without sending anything that isn't
   a normal printing ASCII character. */
tc_pretty_print (string)
     register char *string;
{
  register char c;

  while (c = *string++)
    {
      if (CTRLP (c))
	{
	  putchar ('^');
	  c += 64;
	}
      putchar (c);
    }
}
#endif TERMINAL_INFO_PRINTING


/* **************************************************************** */
/*								    */
/*			Character IO, and driver		    */
/*								    */
/* **************************************************************** */

char *widest_line;
int terminal_inited_p = 0;

/* Start up the character io stuff. */
init_terminal_io ()
{
  if (!terminal_inited_p)
    {
      opsys_init_terminal ();
      terminal_rows = get_terminal_rows ();
      terminal_columns = get_terminal_columns ();

      widest_line = (char *) xmalloc (terminal_columns);

      terminal_inited_p = 1;
    }

  terminal_window.left = 0;
  terminal_window.top = 0;
  terminal_window.right = terminal_columns;
  terminal_window.bottom = terminal_rows;

  set_window (&terminal_window);

  terminal_window.bottom -= 2;

  set_window (&terminal_window);

  init_echo_area (the_window.left, the_window.bottom + 1,
		  the_window.right, terminal_rows);

  /* Here is a list of things that the terminal has to be able to do. Do
     you think that this is too harsh? */
  if (!terminal_goto ||		/* We can't move the cursor. */
      !terminal_rows)		/* We don't how many lines it has. */
    {
      fprintf (stderr,
	       "Your terminal is not clever enough to run info. Sorry.\n");
       exit (1);
    }
}

/* Ring the terminal bell. */
ding ()
{
  extern char *terminal_ear_bell;

  if (terminal_ear_bell)
    do_term (terminal_ear_bell);
  else
    putchar (CTRL ('G'));

  fflush (stdout);
}

int untyi_char = 0;
int inhibit_output = 0;

/* Return a character from stdin, or the last unread character
   if there is one available. */
blink_cursor ()
{
  int character;

  fflush (stdout);
  if (untyi_char)
    {
      character = untyi_char;
      untyi_char = 0;
    }
  else
    do { character = getc (stdin); } while (character == -1 && errno == EINTR);

  return (character);
}

/* Display single character on the terminal screen.  If the
   character would run off the right hand edge of the screen,
   advance the cursor to the next line. */
charout (character)
     int character;
{
  if (inhibit_output)
    return;

  /* This character may need special treatment if it is
     a control character. */
  if (CTRL_P (character))
    {
      switch (character)
	{
	case NEWLINE:
	case RETURN:
	  print_cr ();
	  break;

	case TAB:
	  print_tab ();
	  break;

	default:
	  charout ('^');
	  charout (UNCTRL (character));
	}
    }
  else
    {
      putchar (character);
      advance (1);
    }
}

/* Move the cursor AMOUNT character positions. */
advance (amount)
     int amount;
{
  int old_window_cv = the_window.cv;

  while (amount-- > 0)
    {
      the_window.ch++;
      if (the_window.ch >= the_window.right)
	{
	  the_window.ch = (the_window.ch - the_window.right) + the_window.left;
	  the_window.cv++;

	  if (the_window.cv >= the_window.bottom)
	    the_window.cv = the_window.top;
	}
    }

  if (the_window.cv != old_window_cv)
    goto_xy (the_window.ch, the_window.cv);
}

/* Print STRING and args using charout */
print_string (string, a1, a2, a3, a4, a5)
     char *string;
{
  int character;
  char buffer[2048];
  int idx = 0;

  sprintf (buffer, string, a1, a2, a3, a4, a5);

  while (character = buffer[idx++])
    charout (character);

  fflush (stdout);
}

/* Display a carriage return.
   Clears to the end of the line first. */
print_cr ()
{
  extern int typing_out;
  clear_eol ();

  if (typing_out)
    {				/* Do the "MORE" stuff. */
      int response;

      if (the_window.cv + 2 == the_window.bottom)
	{
	  goto_xy (the_window.left, the_window.cv + 1);
	  clear_eol ();
	  print_string ("[More]");
	  response = blink_cursor ();
	  if (response != SPACE)
	    {
	      untyi_char = response;
	      inhibit_output = 1;
	      return;
	    }
	  else
	    {
	      goto_xy (the_window.left, the_window.cv);
	      clear_eol ();
	      goto_xy (the_window.left, the_window.top);
	      return;
	    }
	}
    }
  advance (the_window.right - the_window.ch);
}

/* Move the cursor to the next tab stop, blanking the intervening
   spaces along the way. */
print_tab ()
{
  int hpos, width, destination;

  hpos = the_window.ch - the_window.left;
  width = ((hpos + 8) & 0xf8) - hpos;

  destination = hpos + width + the_window.left;

  if (destination >= the_window.right)
    destination -= the_window.right;

  while (the_window.ch != destination)
    charout (SPACE);
}

display_width (character, hpos)
     int character, hpos;
{
  int width = 1;

  if (CTRL_P (character))
    {
      switch (character)
	{
	case RETURN:
	case NEWLINE:
	  width = the_window.right - hpos;
	  break;
	case TAB:
	  width = ((hpos + 8) & 0xf8) - hpos;
	  break;
	default:
	  width = 2;
	}
    }
  return (width);
}

/* Like GOTO_XY, but do it right away. */
I_goto_xy (xpos, ypos)
     int xpos, ypos;
{
  goto_xy (xpos, ypos);
  fflush (stdout);
}

/* Move the cursor, (and cursor variables) to xpos, ypos. */
goto_xy (xpos, ypos)
     int xpos, ypos;
{
  the_window.ch = xpos;
  the_window.cv = ypos;
  opsys_goto_pos (xpos, ypos);
}

/* Clear the screen, leaving ch and cv at the top of the window. */
clear_screen ()
{
  goto_xy (the_window.left, the_window.top);
  clear_eop_slowly ();
}

clear_eop_slowly ()
{
  int temp_ch = the_window.ch;
  int temp_cv = the_window.cv;

  clear_eol ();

  while (++the_window.cv < the_window.bottom)
    {
      goto_xy (the_window.left, the_window.cv);
      clear_eol ();
    }
  goto_xy (temp_ch, temp_cv);
}

/* Clear from current cursor position to end of page. */
clear_eop ()
{
  if (terminal_clearEOP)
    do_term (terminal_clearEOP);
  else
    clear_eop_slowly ();
}

/* Clear from current cursor position to end of screen line */
clear_eol ()
{
  int temp_ch = the_window.ch;

  if (terminal_clearEOL)
    do_term (terminal_clearEOL);
  else
    {
      char *line = widest_line;
      int i;

      for (i = 0; i < the_window.right - the_window.ch; i++)
	line[i] = ' ';
      line[i] = '\0';

      printf ("%s", line);
    }
  goto_xy (temp_ch, the_window.cv);
}

/* Call FUNCTION with WINDOW active.  You can pass upto 5 args to the
   function.  This returns whatever FUNCTION returns. */
int
with_output_to_window (window, function, arg1, arg2, arg3, arg4, arg5)
     WINDOW *window;
     Function *function;
{
  int result;

  push_window ();
  set_window (window);
  result = (*function) (arg1, arg2, arg3, arg4, arg5);
  pop_window ();
  return (result);
}

/* Given a pointer to a window data structure, make that
   the current window. */
set_window (window)
     WINDOW *window;
{
  bcopy (window, &the_window, sizeof (WINDOW));
}

/* Save the current window on the window stack. */
push_window ()
{
  WINDOW_LIST *new_window = (WINDOW_LIST *) xmalloc (sizeof (WINDOW_LIST));

  new_window->next_window = window_stack;
  window_stack = new_window;
  new_window->ch = the_window.ch;
  new_window->cv = the_window.cv;
  new_window->top = the_window.top;
  new_window->bottom = the_window.bottom;
  new_window->left = the_window.left;
  new_window->right = the_window.right;
}

/* Pop the top of the window_stack into the_window. */
pop_window ()
{
  set_window ((WINDOW *)window_stack);

  if (window_stack->next_window)
    {
      WINDOW_LIST *thing_to_free = window_stack;
      window_stack = window_stack->next_window;
      free (thing_to_free);
    }

  goto_xy (the_window.ch, the_window.cv);
}

/* **************************************************************** */
/*								    */
/*			"Opsys" functions.			    */
/*								    */
/* **************************************************************** */

/* The lowlevel terminal/file interface.  Nothing ever really gets
   low level when you're writing in C, though.

   This file contains all of the "opsys" labels.  You have to make
   a different one if you want GNU Info to run on machines that don't
   have Unix.  */

extern char *terminal_use_begin, *terminal_use_end, *terminal_goto;

#if defined (TIOCGETC)
struct tchars original_tchars;
#endif

#if defined (TIOCGLTC)
struct ltchars original_ltchars;
#endif

#if defined (USG)
struct termio original_termio, ttybuff;
#else
int original_tty_flags = 0;
int original_lmode;
struct sgttyb ttybuff;
#endif /* !USG */

/* Yes, that's right, do things that the machine needs to get
   the terminal into a usable mode. */
opsys_init_terminal ()
{
  int tty = fileno (stdin);

#if defined (USG)
  ioctl (tty, TCGETA, &original_termio);
  ioctl (tty, TCGETA, &ttybuff);
  ttybuff.c_iflag &= (~ISTRIP & ~INLCR & ~IGNCR & ~ICRNL &~IXON);
  ttybuff.c_oflag &= (~ONLCR & ~OCRNL);
  ttybuff.c_lflag &= (~ICANON & ~ECHO);

  ttybuff.c_cc[VMIN] = 1;
  ttybuff.c_cc[VTIME] = 0;

  if (ttybuff.c_cc[VINTR] = DELETE)
    ttybuff.c_cc[VINTR] = -1;

  if (ttybuff.c_cc[VQUIT] = DELETE)
    ttybuff.c_cc[VQUIT] = -1;

  ioctl (tty, TCSETA, &ttybuff);
#else /* !USG */

  ioctl (tty, TIOCGETP, &ttybuff);

  if (!original_tty_flags)
    original_tty_flags = ttybuff.sg_flags;

  /* Make this terminal pass 8 bits around while we are using it. */
#ifdef PASS8
  ttybuff.sg_flags |= PASS8;
#endif

#if defined (TIOCLGET) && defined (LPASS8)
  {
    int flags;
    ioctl (tty, TIOCLGET, &flags);
    original_lmode = flags;
    flags |= LPASS8;
    ioctl (tty, TIOCLSET, &flags);
  }
#endif

#ifdef TIOCGETC
  {
    struct tchars temp;

    ioctl (tty, TIOCGETC, &original_tchars);
    bcopy (&original_tchars, &temp, sizeof (struct tchars));

    temp.t_startc = temp.t_stopc = -1;

    /* If the quit character conflicts with one of our commands, then
       make it go away. */
    if (temp.t_intrc == DELETE)
      temp.t_intrc == -1;

    if (temp.t_quitc == DELETE)
      temp.t_quitc == -1;

    ioctl (tty, TIOCSETC, &temp);
  }
#endif /* TIOCGETC */

#ifdef TIOCGLTC
  {
    struct ltchars temp;

    ioctl (tty, TIOCGLTC, &original_ltchars);
    bcopy (&original_ltchars, &temp, sizeof (struct ltchars));

    /* Make the interrupt keys go away.  Just enough to make people happy. */
    temp.t_lnextc = -1;		/* C-v */

    ioctl (tty, TIOCSLTC, &temp);
  }
#endif /* TIOCGLTC */

  ttybuff.sg_flags &= ~ECHO;
  ttybuff.sg_flags |= CBREAK;
  ioctl (tty, TIOCSETN, &ttybuff);
#endif /* !USG */

  open_terminal_io ();
  do_term (terminal_use_begin);
}

/* Fix the terminal that I broke. */
restore_io ()
{
  int tty = fileno (stdin);

#if defined (USG)
  ioctl (tty, TCSETA, &original_termio);
#else
  ioctl (tty, TIOCGETP, &ttybuff);
  ttybuff.sg_flags = original_tty_flags;
  ioctl (tty, TIOCSETN, &ttybuff);

#ifdef TIOCGETC
  ioctl (tty, TIOCSETC, &original_tchars);
#endif /* TIOCGETC */

#ifdef TIOCGLTC
  ioctl (tty, TIOCSLTC, &original_ltchars);
#endif /* TIOCGLTC */

#if defined (TIOCLGET) && defined (LPASS8)
  ioctl (tty, TIOCLSET, &original_lmode);
#endif

#endif /* !USG */
  do_term (terminal_use_end);
}

opsys_goto_pos (xpos, ypos)
     int xpos, ypos;
{
  do_term (tgoto (terminal_goto, xpos, ypos));
}

character_output_function (character)
     char character;
{
  putchar (character);
}

/* Generic interface to tputs. */
do_term (command)
     char *command;
{
  /* Send command to the terminal, with appropriate padding. */
  if (command)
    tputs (command, 1, character_output_function);
}

/* Filename manipulators, and the like. */
char local_temp_filename[FILENAME_LEN];

char *info_suffixes[] = {
  "",
  ".info",
  "-info",
  (char *)NULL
};

/* A structure which associates the argument passed into a function with
   the result from that function. */
typedef struct {
  char *called;
  char *result;
} CALLED_RESULTS;

/* An array of remembered arguments and results. */
static CALLED_RESULTS **opsys_callers_and_results = (CALLED_RESULTS **)NULL;
static int next_caller_and_result = 0;
static int callers_and_results_size = 0;

/* Find the result for having already called opsys_filename () with CALLER. */
char *
find_opsys_filename_result (caller)
     char *caller;
{
  if (caller && opsys_callers_and_results)
    {
      register int i;
      for (i = 0; opsys_callers_and_results[i]; i++)
	{
	  if (strcmp (opsys_callers_and_results[i]->called, caller) == 0)
	    return (opsys_callers_and_results[i]->result);
	}
    }
  return (char *)NULL;;
}
  
/* Add an argument and result to our list. */
void
add_caller_and_result (caller, result)
     char *caller, *result;
{
  CALLED_RESULTS *new;

  if (next_caller_and_result + 2 > callers_and_results_size)
    {
      int alloc_size;
      callers_and_results_size += 10;

      alloc_size = callers_and_results_size * sizeof (CALLED_RESULTS *);

      opsys_callers_and_results = (CALLED_RESULTS **)
	xrealloc (opsys_callers_and_results, alloc_size);
    }

  new = (CALLED_RESULTS *)xmalloc (sizeof (CALLED_RESULTS));
  new->called = savestring (caller);
  new->result = result ? savestring (result) : (char *)NULL;

  opsys_callers_and_results[next_caller_and_result++] = new;
  opsys_callers_and_results[next_caller_and_result] = (CALLED_RESULTS *)NULL;
}

/* Expand the filename in partial to make a real name for
   this operating system.  This looks in INFO_PATHS in order to
   find the correct file.  If it can't find the file, it just
   returns the path as you gave it. */
char *
opsys_filename (partial)
     char *partial;
{
  int initial_character;
  char *my_infopath;

  if (partial && (initial_character = *partial))
    {
      char *possible_result;

      if (initial_character == '/')
	return (partial);

      possible_result = find_opsys_filename_result (partial);
      if (possible_result)
	return (possible_result);

      if (initial_character == '~')
	{
	  if (partial[1] == '/')
	    {
	      /* Return the concatenation of HOME and the rest
		 of the string. */
	      strcpy (local_temp_filename, getenv ("HOME"));
	      strcat (local_temp_filename, &partial[2]);
	      return (local_temp_filename);
	    }
	  else
	    {
	      struct passwd *user_entry;
	      int i, c;
	      char username[257];

	      for (i = 1; c = partial[i]; i++)
		{
		  if (c == '/')
		    break;
		  else
		    username[i - 1] = c;
		}
	      username[i - 1] = '\0';

	      if (!(user_entry = getpwnam (username)))
		{
		  display_error ("Not a registered user!");
		  return (partial);
		}
	      strcpy (local_temp_filename, user_entry->pw_dir);
	      strcat (local_temp_filename, &partial[i]);
	      return (local_temp_filename);
	    }
	}

      if (initial_character == '.' &&
	  (partial[1]) == '/' || (partial[1] == '.' && partial[2] == '/'))
	{
#if defined (USG)
	  if (!getcwd (local_temp_filename, FILENAME_LEN))
#else
	  if (!getwd (local_temp_filename))
#endif
	    {
	      display_error (local_temp_filename);
	      return (partial);
	    }

	  strcat (local_temp_filename, "/");
	  strcat (local_temp_filename, partial);
	  return (local_temp_filename);
	}

      /* If the current info file has a directory, then search that directory
	 first. */
      {
	register char *ptr, *file;

	file = find_opsys_filename_result (current_info_file);

	if (file && (ptr = (char *)rindex (file, '/')) != (char *)NULL)
	  {
	    register int len = (ptr - file);

	    my_infopath = (char *)
	      xmalloc (2 + strlen (file) + strlen (infopath));

	    strncpy (my_infopath, file, len);
	    sprintf (my_infopath + len, ":%s", infopath);
	  }
	else
	  my_infopath = savestring (infopath);
      }

      if (opsys_info_file_in_path (partial, my_infopath, local_temp_filename))
	{
	  free (my_infopath);
	  add_caller_and_result (partial, local_temp_filename);
	  return (local_temp_filename);
	}

      free (my_infopath);
    }
  return (partial);
}

#if !defined (S_ISREG) && defined (S_IFREG)
#  define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif /* !S_ISREG && S_IFREG */

#if !defined (S_ISDIR) && defined (S_IFDIR)
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif /* !S_ISDIR && S_IFDIR */

/* Scan the list of directories in PATH looking for FILENAME.
   If we find one that is a regular file, stuff the answer into
   RETURN_STRING.  Return non-zero if a file was found, zero otherwise. */
int
opsys_info_file_in_path (filename, path, return_string)
     char *filename, *path;
     char *return_string;
{
  struct stat finfo;
  char *temp_dirname, *extract_colon_unit ();
  char *test_file = return_string;
  int statable, dirname_index = 0;

  while (temp_dirname = extract_colon_unit (path, &dirname_index))
    {
      register int i, pre_suffix_length;

      strcpy (test_file, temp_dirname);
      if (temp_dirname[(strlen (temp_dirname)) - 1] != '/')
	strcat (test_file, "/");
      strcat (test_file, filename);

      pre_suffix_length = strlen (test_file);

      free (temp_dirname);

      for (i = 0; info_suffixes[i]; i++)
	{
	  strcpy (test_file + pre_suffix_length, info_suffixes[i]);

	  statable = (stat (test_file, &finfo) == 0);

	  /* If we have found a regular file, then use that.  Else, if we
	     have found a directory, look in that directory for this file. */
	  if (statable)
	    {
	      if (S_ISREG (finfo.st_mode))
		return (1);
	      else if (S_ISDIR (finfo.st_mode))
		{
		  char *newpath = savestring (test_file);
		  char *filename_only = (char *)rindex (filename, '/');

		  if (opsys_info_file_in_path
		      (filename_only ? filename_only : filename,
		       newpath, return_string))
		    {
		      free (newpath);
		      return (1);
		    }
		}
	    }
	}
    }
  return (0);
}

/* Given a string containing units of information separated by colons,
   return the next one pointed to by IDX, or NULL if there are no more.
   Advance IDX to the character after the colon. */
char *
extract_colon_unit (string, idx)
     char *string;
     int *idx;
{
  register int i, start;

  i = start = *idx;
  if ((i >= strlen (string)) || !string)
    return ((char *) NULL);

  while (string[i] && string[i] != ':')
    i++;
  if (i == start)
    {
      return ((char *) NULL);
    }
  else
    {
      char *value = (char *) xmalloc (1 + (i - start));
      strncpy (value, &string[start], (i - start));
      value[i - start] = '\0';
      if (string[i])
	++i;
      *idx = i;
      return (value);
    }
}

/* **************************************************************** */
/*								    */
/*			The echo area.				    */
/*								    */
/* **************************************************************** */

WINDOW echo_area = {0, 0, 0, 0, 0, 0};
int echo_area_open_p = 0;
char modeline[256];
WINDOW modeline_window = {0, 0, 0, 0, 0, 0};

/* Define the location of the echo area. Also inits the
   modeline as well. */
init_echo_area (left, top, right, bottom)
     int left, top, right, bottom;
{
  echo_area.left = modeline_window.left = left;
  echo_area.top = top;
  modeline_window.top = top - 1;
  echo_area.right = modeline_window.right = right;
  echo_area.bottom = bottom;
  modeline_window.bottom = modeline_window.top;
}

/* Make the echo_area_window be the current window, and only allow
   output in there.  Clear the window to start. */
new_echo_area ()
{
  if (!echo_area_open_p)
    {
      push_window ();
      set_window (&echo_area);
      echo_area_open_p = 1;
    }
  goto_xy (the_window.left, the_window.top);
  clear_eop ();
}

/* Return output to the previous window. */
close_echo_area ()
{
  if (!echo_area_open_p)
    return;

  pop_window ();
  echo_area_open_p = 0;
}

/* Clear the contents of the echo area. */
clear_echo_area ()
{
  new_echo_area ();
  close_echo_area ();
}

/* Create and display the modeline. */
make_modeline ()
{
  int width = modeline_window.right - modeline_window.left;
  char textual_position[6];

  if (pagetop == nodetop)
    {
      if (pagebot == nodebot)
	sprintf (textual_position, "All");
      else
	sprintf (textual_position, "Top");
    }
  else
    {
      if (pagebot == nodebot)
	sprintf (textual_position, "Bot");
      else
	sprintf (textual_position, "%2d%%",
		 100 * (pagetop - nodetop) / (nodebot - nodetop));
    }
    
  sprintf (modeline, "Info: (%s)%s, %d lines ---%s",
	   current_info_file, current_info_node, nodelines, textual_position);

  if (strnicmp
      (opsys_filename (current_info_file), last_loaded_info_file,
       strlen (last_loaded_info_file)) != 0)
    {
      sprintf (&modeline[strlen (modeline)], ", Subfile: %s",
	       last_loaded_info_file);
    }

  if (strlen (modeline) < width)
    {
      int idx = strlen (modeline);
      while (idx != width)
	modeline[idx++] = '-';
      modeline[idx] = '\0';
    }

  if (strlen (modeline) > width)
    modeline[width] = '\0';
  push_window ();
  set_window (&modeline_window);
  goto_xy (the_window.left, the_window.top);

  if (terminal_inverse_begin)
    do_term (terminal_inverse_begin);
  print_string ("%s", modeline);
  if (terminal_inverse_begin)
    do_term (terminal_end_attributes);

  pop_window ();
}

int typing_out = 0;

/* Prepare to do output to the typeout window.  If the
   typeout window is already open, ignore this clown. */
open_typeout ()
{
  if (typing_out)
    return;

  push_window ();
  set_window (&terminal_window);
  goto_xy (the_window.ch, the_window.cv);
  typing_out = window_bashed = 1;
}

/* Close the currently open typeout window. */
close_typeout ()
{
  if (inhibit_output)
    inhibit_output = 0;
  else
    {
      do { untyi_char = getc (stdin); }
        while (untyi_char == -1 && errno == EINTR);

      if (untyi_char == SPACE)
	untyi_char = 0;
    }
  pop_window ();
  typing_out = 0;
}

void *
xrealloc (pointer, bytes)
     char *pointer;
     int bytes;
{
  void *temp;

  if (pointer == (char *)NULL)
    temp = (void *) xmalloc (bytes);
  else
    temp = (void *) realloc (pointer, bytes);

  if (temp == (void *)NULL)
    {
      fprintf (stderr, "Virtual memory exhausted\n");
      restore_io ();
      exit (2);
    }
  return (temp);
}

void *
xmalloc (bytes)
     int bytes;
{
  void *temp = (void *) malloc (bytes);

  if (temp == (void *) NULL)
    {
      fprintf (stderr, "Virtual memory exhausted\n");
      restore_io ();
      exit (2);
    }
  return (temp);
}
