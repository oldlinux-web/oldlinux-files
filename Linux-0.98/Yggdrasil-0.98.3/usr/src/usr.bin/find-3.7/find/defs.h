/* defs.h -- data types and declarations.
   Copyright (C) 1987, 1990, 1991 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <errno.h>
#ifndef STDC_HEADERS
extern int errno;
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#endif
#include <time.h>
#include "regex.h"

typedef char boolean;
#define		true    1
#define		false	0

/* Pointer to function returning boolean. */
typedef boolean (*PFB)();

PFB find_parser ();
boolean no_side_effects ();
boolean parse_print ();
char *xmalloc ();
char *xrealloc ();
int process_path ();
struct predicate *get_expr ();
struct predicate *get_new_pred ();
struct predicate *get_new_pred_chk_op ();
struct predicate *insert_victim ();
void error ();
void usage ();

#ifdef	DEBUG
void print_tree ();
void print_list ();
#endif	/* DEBUG */

/* Argument structures for predicates. */

enum comparison_type
{
  COMP_GT,
  COMP_LT,
  COMP_EQ
};

enum predicate_type
{
  NO_TYPE,
  VICTIM_TYPE,
  UNI_OP,
  BI_OP,
  OPEN_PAREN,
  CLOSE_PAREN
};

enum predicate_precedence
{
  NO_PREC,
  COMMA_PREC,
  OR_PREC,
  AND_PREC,
  NEGATE_PREC,
  MAX_PREC
};

struct long_val
{
  enum comparison_type kind;
  unsigned long l_val;
};

struct size_val
{
  enum comparison_type kind;
  int blocksize;
  unsigned long size;
};

struct path_arg
{
  short offset;			/* Offset in `vec' of this arg. */
  short count;			/* Number of path replacements in this arg. */
  char *origarg;		/* Arg with "{}" intact. */
};

struct exec_val
{
  struct path_arg *paths;	/* Array of args with path replacements. */
  char **vec;			/* Array of args to pass to program. */
};

/* The format string for a -printf or -fprintf is chopped into one or
   more `struct segment', linked together into a list.
   Each stretch of plain text is a segment, and
   each \c and `%' conversion is a segment. */

/* Special values for the `kind' field of `struct segment'. */
#define KIND_PLAIN 0		/* This segment is just plain text. */
#define KIND_STOP 1		/* \c -- stop printing from this fmt string. */

struct segment
{
  int kind;			/* Format chars or KIND_{PLAIN,STOP}. */
  char *text;			/* Plain text or `%' format string. */
  int text_len;			/* Length of `text'. */
  struct segment *next;		/* Next segment for this predicate. */
};

struct format_val
{
  struct segment *segment;	/* Linked list of segments. */
  FILE *stream;			/* Output stream to print on. */
};

struct predicate
{
  /* Pointer to the function that implements this predicate.  */
  PFB pred_func;

  /* Only used for debugging, but defined unconditionally so individual
     modules can be compiled with -DDEBUG.  */
  char *p_name;

  /* The type of this node.  There are two kinds.  The first is real
     predicates ("victims") such as -perm, -print, or -exec.  The
     other kind is operators for combining predicates. */
  enum predicate_type p_type;

  /* The precedence of this node.  Only has meaning for operators. */
  enum predicate_precedence p_prec;

  /* True if this predicate node produces side effects. */
  boolean side_effects;

  /* True if this predicate node requires a stat system call to execute. */
  boolean need_stat;

  /* Information needed by the predicate processor.
     Next to each member are listed the predicates that use it. */
  union
  {
    char *str;			/* name fstype */
    struct re_pattern_buffer *regex; /* regex */
    struct exec_val exec_vec;	/* exec ok */
    struct long_val info;	/* atime ctime mtime inum links */
    struct size_val size;	/* size */
    unsigned short uid;		/* user */
    unsigned short gid;		/* group */
    time_t time;		/* newer */
    unsigned long perm;		/* perm */
    unsigned long type;		/* type */
    FILE *stream;		/* fprint fprint0 */
    struct format_val printf_vec; /* printf fprintf */
  } args;

  /* The next predicate in the user input sequence,
     which repesents the order in which the user supplied the
     predicates on the command line. */
  struct predicate *pred_next;

  /* The right and left branches from this node in the expression
     tree, which represents the order in which the nodes should be
     processed. */
  struct predicate *pred_left;
  struct predicate *pred_right;
};

/* The number of seconds in a day. */
#define		DAYSECS	    86400

extern char *program_name;
extern struct predicate *predicates;
extern struct predicate *last_pred;
extern boolean do_dir_first;
extern int maxdepth;
extern int mindepth;
extern int curdepth;
extern time_t cur_day_start;
extern boolean full_days;
extern boolean no_leaf_check;
extern boolean stay_on_filesystem;
extern boolean stop_at_current_level;
extern boolean have_stat;
extern int exit_status;
extern int path_length;
extern int (*xstat) ();
