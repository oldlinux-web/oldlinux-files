/* parser.c -- convert the command line args into an expression tree.
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

#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(USG) || defined(STDC_HEADERS)
#include <string.h>
#define index strchr
#define rindex strrchr
#else /* not (USG or STDC_HEADERS) */
#include <strings.h>
#endif /* USG or STDC_HEADERS */

#ifndef _POSIX_VERSION
struct group *getgrnam ();
struct group *getgrent ();
struct passwd *getpwnam ();
struct passwd *getpwent ();
#endif

#include "modechange.h"
#include "defs.h"
#include "modetype.h"

#ifndef S_IFLNK
#define lstat stat
#endif

char *strstr ();
int lstat ();
int stat ();
long atol ();
struct tm *localtime ();
#ifdef _POSIX_SOURCE
#define endgrent()
#define endpwent()
#else
void endgrent ();
void endpwent ();
#endif

#if !__STDC__
#define const
#endif

boolean parse_amin ();
boolean parse_and ();
boolean parse_anewer ();
boolean parse_atime ();
boolean parse_close ();
boolean parse_cmin ();
boolean parse_cnewer ();
boolean parse_comma ();
boolean parse_ctime ();
boolean parse_daystart ();
boolean parse_depth ();
boolean parse_empty ();
boolean parse_exec ();
boolean parse_false ();
boolean parse_follow ();
boolean parse_fprint ();
boolean parse_fprint0 ();
boolean parse_fprintf ();
boolean parse_fstype ();
boolean parse_gid ();
boolean parse_group ();
boolean parse_inum ();
boolean parse_links ();
boolean parse_lname ();
boolean parse_ls ();
boolean parse_maxdepth ();
boolean parse_mindepth ();
boolean parse_mmin ();
boolean parse_mtime ();
boolean parse_name ();
boolean parse_negate ();
boolean parse_newer ();
boolean parse_noleaf ();
boolean parse_nogroup ();
boolean parse_nouser ();
boolean parse_ok ();
boolean parse_open ();
boolean parse_or ();
boolean parse_path ();
boolean parse_perm ();
boolean parse_print ();
boolean parse_print0 ();
boolean parse_printf ();
boolean parse_prune ();
boolean parse_regex ();
boolean parse_size ();
boolean parse_true ();
boolean parse_type ();
boolean parse_uid ();
boolean parse_used ();
boolean parse_user ();
boolean parse_version ();
boolean parse_xdev ();
boolean parse_xtype ();

boolean pred_amin ();
boolean pred_and ();
boolean pred_anewer ();
boolean pred_atime ();
boolean pred_close ();
boolean pred_cmin ();
boolean pred_cnewer ();
boolean pred_comma ();
boolean pred_ctime ();
/* no pred_daystart */
/* no pred_depth */
boolean pred_empty ();
boolean pred_exec ();
boolean pred_false ();
/* no pred_follow */
boolean pred_fprint ();
boolean pred_fprint0 ();
boolean pred_fprintf ();
boolean pred_fstype ();
boolean pred_gid ();
boolean pred_group ();
boolean pred_inum ();
boolean pred_links ();
boolean pred_lname ();
boolean pred_ls ();
/* no pred_maxdepth */
/* no pred_mindepth */
boolean pred_mmin ();
boolean pred_mtime ();
boolean pred_name ();
boolean pred_negate ();
boolean pred_newer ();
/* no pred_noleaf */
boolean pred_nogroup ();
boolean pred_nouser ();
boolean pred_ok ();
boolean pred_open ();
boolean pred_or ();
boolean pred_path ();
boolean pred_perm ();
boolean pred_print ();
boolean pred_print0 ();
/* no pred_printf */
boolean pred_prune ();
boolean pred_regex ();
boolean pred_size ();
boolean pred_true ();
boolean pred_type ();
boolean pred_uid ();
boolean pred_used ();
boolean pred_user ();
/* no pred_version */
/* no pred_xdev */
boolean pred_xtype ();

boolean get_num ();
boolean get_num_days ();
boolean insert_exec_ok ();
boolean insert_fprintf ();
boolean insert_num ();
boolean insert_time ();
boolean insert_type ();
FILE *open_output_file ();
struct segment **make_segment ();

#ifdef	DEBUG
char *find_pred_name ();
#endif	/* DEBUG */

struct parser_table_t
{
  char *parser_name;
  PFB parser_func;
};

/* GNU find predicates that are not mentioned in POSIX.2 are marked `GNU'.
   If they are in some Unix versions of find, they are marked `Unix'. */

struct parser_table_t parse_table[] =
{
  {"!", parse_negate},
  {"not", parse_negate},	/* GNU */
  {"(", parse_open},
  {")", parse_close},
  {",", parse_comma},		/* GNU */
  {"a", parse_and},
  {"amin", parse_amin},		/* GNU */
  {"and", parse_and},		/* GNU */
  {"anewer", parse_anewer},	/* GNU */
  {"atime", parse_atime},
  {"cmin", parse_cmin},		/* GNU */
  {"cnewer", parse_cnewer},	/* GNU */
#ifdef UNIMPLEMENTED_UNIX
  /* It's pretty ugly for find to know about archive formats.
     Plus what it could do with cpio archives is very limited.
     Better to leave it out.  */
  {"cpio", parse_cpio},		/* Unix */
#endif
  {"ctime", parse_ctime},
  {"daystart", parse_daystart},	/* GNU */
  {"depth", parse_depth},
  {"empty", parse_empty},	/* GNU */
  {"exec", parse_exec},
  {"false", parse_false},	/* GNU */
  {"follow", parse_follow},	/* GNU, Unix */
  {"fprint", parse_fprint},	/* GNU */
  {"fprint0", parse_fprint0},	/* GNU */
  {"fprintf", parse_fprintf},	/* GNU */
  {"fstype", parse_fstype},	/* GNU, Unix */
  {"gid", parse_gid},		/* GNU */
  {"group", parse_group},
  {"inum", parse_inum},		/* GNU, Unix */
  {"links", parse_links},
  {"lname", parse_lname},	/* GNU */
  {"ls", parse_ls},		/* GNU, Unix */
  {"maxdepth", parse_maxdepth},	/* GNU */
  {"mindepth", parse_mindepth},	/* GNU */
  {"mmin", parse_mmin},		/* GNU */
  {"mtime", parse_mtime},
  {"name", parse_name},
#ifdef UNIMPLEMENTED_UNIX
  {"ncpio", parse_ncpio},	/* Unix */
#endif
  {"newer", parse_newer},
  {"noleaf", parse_noleaf},	/* GNU */
  {"nogroup", parse_nogroup},
  {"nouser", parse_nouser},
  {"o", parse_or},
  {"or", parse_or},		/* GNU */
  {"ok", parse_ok},
  {"path", parse_path},		/* GNU, HP-UX */
  {"perm", parse_perm},
  {"print", parse_print},
  {"print0", parse_print0},	/* GNU */
  {"printf", parse_printf},	/* GNU */
  {"prune", parse_prune},
  {"regex", parse_regex},	/* GNU */
  {"size", parse_size},
  {"true", parse_true},		/* GNU */
  {"type", parse_type},
  {"uid", parse_uid},		/* GNU */
  {"used", parse_used},		/* GNU */
  {"user", parse_user},
  {"version", parse_version},	/* GNU */
  {"xdev", parse_xdev},
  {"xtype", parse_xtype},	/* GNU */
  {0, 0}
};

/* Return a pointer to the parser function to invoke for predicate
   SEARCH_NAME.
   Return NULL if SEARCH_NAME is not a valid predicate name. */

PFB
find_parser (search_name)
     char *search_name;
{
  int i;

  if (*search_name == '-')
    search_name++;
  for (i = 0; parse_table[i].parser_name != 0; i++)
    if (strcmp (parse_table[i].parser_name, search_name) == 0)
      return (parse_table[i].parser_func);
  return (NULL);
}

/* The parsers are responsible to continue scanning ARGV for
   their arguments.  Each parser knows what is and isn't
   allowed for itself.
   
   ARGV is the argument array.
   *ARG_PTR is the index to start at in ARGV,
   updated to point beyond the last element consumed.
 
   The predicate structure is updated with the new information. */

boolean
parse_amin (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;
  unsigned long num;
  enum comparison_type c_type;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  if (!get_num_days (argv[*arg_ptr], &num, &c_type))
    return (false);
  our_pred = insert_victim (pred_amin);
  our_pred->args.info.kind = c_type;
  our_pred->args.info.l_val = cur_day_start + DAYSECS - num * 60;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_and (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = get_new_pred ();
  our_pred->pred_func = pred_and;
#ifdef	DEBUG
  our_pred->p_name = find_pred_name (pred_and);
#endif	/* DEBUG */
  our_pred->p_type = BI_OP;
  our_pred->p_prec = AND_PREC;
  our_pred->need_stat = false;
  return (true);
}

boolean
parse_anewer (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;
  struct stat stat_newer;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  if ((*xstat) (argv[*arg_ptr], &stat_newer))
    error (1, errno, "%s", argv[*arg_ptr]);
  our_pred = insert_victim (pred_anewer);
  our_pred->args.time = stat_newer.st_mtime;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_atime (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return (insert_time (argv, arg_ptr, pred_atime));
}

boolean
parse_close (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = get_new_pred ();
  our_pred->pred_func = pred_close;
#ifdef	DEBUG
  our_pred->p_name = find_pred_name (pred_close);
#endif	/* DEBUG */
  our_pred->p_type = CLOSE_PAREN;
  our_pred->p_prec = NO_PREC;
  our_pred->need_stat = false;
  return (true);
}

boolean
parse_cmin (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;
  unsigned long num;
  enum comparison_type c_type;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  if (!get_num_days (argv[*arg_ptr], &num, &c_type))
    return (false);
  our_pred = insert_victim (pred_cmin);
  our_pred->args.info.kind = c_type;
  our_pred->args.info.l_val = cur_day_start + DAYSECS - num * 60;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_cnewer (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;
  struct stat stat_newer;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  if ((*xstat) (argv[*arg_ptr], &stat_newer))
    error (1, errno, "%s", argv[*arg_ptr]);
  our_pred = insert_victim (pred_cnewer);
  our_pred->args.time = stat_newer.st_mtime;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_comma (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = get_new_pred ();
  our_pred->pred_func = pred_comma;
#ifdef	DEBUG
  our_pred->p_name = find_pred_name (pred_comma);
#endif /* DEBUG */
  our_pred->p_type = BI_OP;
  our_pred->p_prec = COMMA_PREC;
  our_pred->need_stat = false;
  return (true);
}

boolean
parse_ctime (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return (insert_time (argv, arg_ptr, pred_ctime));
}

boolean
parse_daystart (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct tm *local;

  if (full_days == false)
    {
      cur_day_start += DAYSECS;
      local = localtime (&cur_day_start);
      cur_day_start -= local->tm_sec + local->tm_min * 60
	+ local->tm_hour * 3600;
      full_days = true;
    }
  return (true);
}

boolean
parse_depth (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  do_dir_first = false;
  return (true);
}
 
boolean
parse_empty (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  insert_victim (pred_empty);
  return (true);
}

boolean
parse_exec (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return (insert_exec_ok (pred_exec, argv, arg_ptr));
}

boolean
parse_false (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = insert_victim (pred_false);
  our_pred->need_stat = false;
  return (true);
}

boolean 
parse_fprintf (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  FILE *fp;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  fp = open_output_file (argv[*arg_ptr]);
  (*arg_ptr)++;
  return (insert_fprintf (fp, pred_fprintf, argv, arg_ptr));
}

boolean
parse_follow (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  xstat = stat;
  no_leaf_check = true;
  return (true);
}

boolean
parse_fprint (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  our_pred = insert_victim (pred_fprint);
  our_pred->args.stream = open_output_file (argv[*arg_ptr]);
  our_pred->side_effects = true;
  our_pred->need_stat = false;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_fprint0 (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  our_pred = insert_victim (pred_fprint0);
  our_pred->args.stream = open_output_file (argv[*arg_ptr]);
  our_pred->side_effects = true;
  our_pred->need_stat = false;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_fstype (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  our_pred = insert_victim (pred_fstype);
  our_pred->args.str = argv[*arg_ptr];
  (*arg_ptr)++;
  return (true);
}

boolean
parse_gid (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return (insert_num (argv, arg_ptr, pred_gid));
}

boolean
parse_group (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct group *cur_gr;
  struct predicate *our_pred;
  int gid, gid_len;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  cur_gr = getgrnam (argv[*arg_ptr]);
  endgrent ();
  if (cur_gr != NULL)
    gid = cur_gr->gr_gid;
  else
    {
      gid_len = strspn (argv[*arg_ptr], "0123456789");
      if ((gid_len == 0) || (argv[*arg_ptr][gid_len] != '\0'))
	return (false);
      gid = atoi (argv[*arg_ptr]);
    }
  our_pred = insert_victim (pred_group);
  our_pred->args.gid = (short) gid;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_inum (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return (insert_num (argv, arg_ptr, pred_inum));
}

boolean
parse_links (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return (insert_num (argv, arg_ptr, pred_links));
}

boolean
parse_lname (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  our_pred = insert_victim (pred_lname);
  our_pred->args.str = argv[*arg_ptr];
  (*arg_ptr)++;
  return (true);
}

boolean
parse_ls (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = insert_victim (pred_ls);
  our_pred->side_effects = true;
  return (true);
}

boolean
parse_maxdepth (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  int depth_len;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  depth_len = strspn (argv[*arg_ptr], "0123456789");
  if ((depth_len == 0) || (argv[*arg_ptr][depth_len] != '\0'))
    return (false);
  maxdepth = atoi (argv[*arg_ptr]);
  if (maxdepth < 0)
    return (false);
  (*arg_ptr)++;
  return (true);
}

boolean
parse_mindepth (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  int depth_len;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  depth_len = strspn (argv[*arg_ptr], "0123456789");
  if ((depth_len == 0) || (argv[*arg_ptr][depth_len] != '\0'))
    return (false);
  mindepth = atoi (argv[*arg_ptr]);
  if (mindepth < 0)
    return (false);
  (*arg_ptr)++;
  return (true);
}

boolean
parse_mmin (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;
  unsigned long num;
  enum comparison_type c_type;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  if (!get_num_days (argv[*arg_ptr], &num, &c_type))
    return (false);
  our_pred = insert_victim (pred_mmin);
  our_pred->args.info.kind = c_type;
  our_pred->args.info.l_val = cur_day_start + DAYSECS - num * 60;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_mtime (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return (insert_time (argv, arg_ptr, pred_mtime));
}

boolean
parse_name (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  our_pred = insert_victim (pred_name);
  our_pred->need_stat = false;
  our_pred->args.str = argv[*arg_ptr];
  (*arg_ptr)++;
  return (true);
}

boolean
parse_negate (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = get_new_pred_chk_op ();
  our_pred->pred_func = pred_negate;
#ifdef	DEBUG
  our_pred->p_name = find_pred_name (pred_negate);
#endif	/* DEBUG */
  our_pred->p_type = UNI_OP;
  our_pred->p_prec = NEGATE_PREC;
  our_pred->need_stat = false;
  return (true);
}

boolean
parse_newer (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;
  struct stat stat_newer;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  if ((*xstat) (argv[*arg_ptr], &stat_newer))
    error (1, errno, "%s", argv[*arg_ptr]);
  our_pred = insert_victim (pred_newer);
  our_pred->args.time = stat_newer.st_mtime;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_noleaf (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  no_leaf_check = true;
  return true;
}

#ifdef CACHE_IDS
/* Arbitrary amount by which to increase size
   of `uid_unused' and `gid_unused'. */
#define ALLOC_STEP 2048

/* Boolean: if uid_unused[n] is nonzero, then UID n has no passwd entry. */
char *uid_unused = NULL;

/* Number of elements in `uid_unused'. */
unsigned uid_allocated;

/* Similar for GIDs and group entries. */
char *gid_unused = NULL;
unsigned gid_allocated;
#endif

boolean
parse_nogroup (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = insert_victim (pred_nogroup);
#ifdef CACHE_IDS
  if (gid_unused == NULL)
    {
      struct group *gr;

      gid_allocated = ALLOC_STEP;
      gid_unused = xmalloc (gid_allocated);
      memset (gid_unused, 1, gid_allocated);
      setgrent ();
      while ((gr = getgrent ()) != NULL)
	{
	  if ((unsigned) gr->gr_gid >= gid_allocated)
	    {
	      unsigned new_allocated = gr->gr_gid + ALLOC_STEP;
	      gid_unused = xrealloc (gid_unused, new_allocated);
	      memset (gid_unused + gid_allocated, 1,
		      new_allocated - gid_allocated);
	      gid_allocated = new_allocated;
	    }
	  gid_unused[(unsigned) gr->gr_gid] = 0;
	}
      endgrent ();
    }
#endif
  return (true);
}

boolean
parse_nouser (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = insert_victim (pred_nouser);
#ifdef CACHE_IDS
  if (uid_unused == NULL)
    {
      struct passwd *pw;

      uid_allocated = ALLOC_STEP;
      uid_unused = xmalloc (uid_allocated);
      memset (uid_unused, 1, uid_allocated);
      setpwent ();
      while ((pw = getpwent ()) != NULL)
	{
	  if ((unsigned) pw->pw_uid >= uid_allocated)
	    {
	      unsigned new_allocated = pw->pw_uid + ALLOC_STEP;
	      uid_unused = xrealloc (uid_unused, new_allocated);
	      memset (uid_unused + uid_allocated, 1,
		      new_allocated - uid_allocated);
	      uid_allocated = new_allocated;
	    }
	  uid_unused[(unsigned) pw->pw_uid] = 0;
	}
      endpwent ();
    }
#endif
  return (true);
}

boolean
parse_ok (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return (insert_exec_ok (pred_ok, argv, arg_ptr));
}

boolean
parse_open (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = get_new_pred_chk_op ();
  our_pred->pred_func = pred_open;
#ifdef	DEBUG
  our_pred->p_name = find_pred_name (pred_open);
#endif	/* DEBUG */
  our_pred->p_type = OPEN_PAREN;
  our_pred->p_prec = NO_PREC;
  our_pred->need_stat = false;
  return (true);
}

boolean
parse_or (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = get_new_pred ();
  our_pred->pred_func = pred_or;
#ifdef	DEBUG
  our_pred->p_name = find_pred_name (pred_or);
#endif	/* DEBUG */
  our_pred->p_type = BI_OP;
  our_pred->p_prec = OR_PREC;
  our_pred->need_stat = false;
  return (true);
}

boolean
parse_path (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  our_pred = insert_victim (pred_path);
  our_pred->need_stat = false;
  our_pred->args.str = argv[*arg_ptr];
  (*arg_ptr)++;
  return (true);
}

boolean
parse_perm (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  unsigned long perm_val;
  int mode_start = 0;
  struct mode_change *change;
  struct predicate *our_pred;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);

  switch (argv[*arg_ptr][0])
    {
    case '-':
    case '+':
      mode_start = 1;
      break;
    default:
      /* empty */
      break;
    }

  change = mode_compile (argv[*arg_ptr] + mode_start, MODE_MASK_PLUS);
  if (change == MODE_INVALID)
    error (1, 0, "invalid mode `%s'", argv[*arg_ptr]);
  else if (change == MODE_MEMORY_EXHAUSTED)
    error (1, 0, "virtual memory exhausted");
  perm_val = mode_adjust (0, change);
  mode_free (change);

  our_pred = insert_victim (pred_perm);

  switch (argv[*arg_ptr][0])
    {
    case '-':
      /* Set magic flag to indicate true if at least the given bits are set. */
      our_pred->args.perm = (perm_val & 07777) | 010000;
      break;
    case '+':
      /* Set magic flag to indicate true if any of the given bits are set. */
      our_pred->args.perm = (perm_val & 07777) | 020000;
      break;
    default:
      /* True if exactly the given bits are set. */
      our_pred->args.perm = (perm_val & 07777);
      break;
    }
  (*arg_ptr)++;
  return (true);
}

boolean
parse_print (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = insert_victim (pred_print);
  /* -print has the side effect of printing.  This prevents us
     from doing undesired multiple printing when the user has
     already specified -print. */
  our_pred->side_effects = true;
  our_pred->need_stat = false;
  return (true);
}

boolean
parse_print0 (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = insert_victim (pred_print0);
  /* -print0 has the side effect of printing.  This prevents us
     from doing undesired multiple printing when the user has
     already specified -print0. */
  our_pred->side_effects = true;
  our_pred->need_stat = false;
  return (true);
}

boolean
parse_printf (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return (insert_fprintf (stdout, pred_fprintf, argv, arg_ptr));
}

boolean
parse_prune (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = insert_victim (pred_prune);
  our_pred->need_stat = false;
  return (true);
}

boolean
parse_regex (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;
  struct re_pattern_buffer *re;
  const char *error_message;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  our_pred = insert_victim (pred_regex);
  our_pred->need_stat = false;
  re = (struct re_pattern_buffer *)
    xmalloc (sizeof (struct re_pattern_buffer));
  our_pred->args.regex = re;
  re->allocated = 100;
  re->buffer = (unsigned char *) xmalloc (re->allocated);
  re->fastmap = NULL;
  re->translate = NULL;
  error_message = re_compile_pattern (argv[*arg_ptr], strlen (argv[*arg_ptr]),
				      re);
  if (error_message)
    error (1, 0, "%s", error_message);
  (*arg_ptr)++;
  return (true);
}

boolean
parse_size (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;
  unsigned long num;
  enum comparison_type c_type;
  int blksize = 512;
  int len;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  len = strlen (argv[*arg_ptr]);
  if (len == 0)
    error (1, 0, "invalid null argument to -size");
  switch (argv[*arg_ptr][len - 1])
    {
    case 'c':
      blksize = 1;
      argv[*arg_ptr][len - 1] = '\0';
      break;

    case 'k':
      blksize = 1024;
      argv[*arg_ptr][len - 1] = '\0';
      break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      break;

    default:
      error (1, 0, "invalid -size type `%c'", argv[*arg_ptr][len - 1]);
    }
  if (!get_num (argv[*arg_ptr], &num, &c_type))
    return (false);
  our_pred = insert_victim (pred_size);
  our_pred->args.size.kind = c_type;
  our_pred->args.size.blocksize = blksize;
  our_pred->args.size.size = num;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_true (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct predicate *our_pred;

  our_pred = insert_victim (pred_true);
  our_pred->need_stat = false;
  return (true);
}

boolean
parse_type (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return insert_type (argv, arg_ptr, pred_type);
}

boolean
parse_uid (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return (insert_num (argv, arg_ptr, pred_uid));
}

boolean
parse_used (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;

{
  struct predicate *our_pred;
  unsigned long num_days;
  enum comparison_type c_type;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  if (!get_num (argv[*arg_ptr], &num_days, &c_type))
    return (false);
  our_pred = insert_victim (pred_used);
  our_pred->args.info.kind = c_type;
  our_pred->args.info.l_val = num_days * DAYSECS;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_user (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  struct passwd *cur_pwd;
  struct predicate *our_pred;
  int uid, uid_len;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  cur_pwd = getpwnam (argv[*arg_ptr]);
  endpwent ();
  if (cur_pwd != NULL)
    uid = cur_pwd->pw_uid;
  else
    {
      uid_len = strspn (argv[*arg_ptr], "0123456789");
      if ((uid_len == 0) || (argv[*arg_ptr][uid_len] != '\0'))
	return (false);
      uid = atoi (argv[*arg_ptr]);
    }
  our_pred = insert_victim (pred_user);
  our_pred->args.uid = (short) uid;
  (*arg_ptr)++;
  return (true);
}

boolean
parse_version (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  extern char *version_string;

  fflush (stdout);
  fprintf (stderr, "%s", version_string);
  fflush (stderr);
  return true;
}

boolean
parse_xdev (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  stay_on_filesystem = true;
  return true;
}

boolean
parse_xtype (argv, arg_ptr)
     char *argv[];
     int *arg_ptr;
{
  return insert_type (argv, arg_ptr, pred_xtype);
}

boolean
insert_type (argv, arg_ptr, which_pred)
     char *argv[];
     int *arg_ptr;
     boolean (*which_pred) ();
{
  unsigned long type_cell;
  struct predicate *our_pred;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL)
      || (strlen (argv[*arg_ptr]) != 1))
    return (false);
  switch (argv[*arg_ptr][0])
    {
    case 'b':			/* block special */
      type_cell = S_IFBLK;
      break;
    case 'c':			/* character special */
      type_cell = S_IFCHR;
      break;
    case 'd':			/* directory */
      type_cell = S_IFDIR;
      break;
    case 'f':			/* regular file */
      type_cell = S_IFREG;
      break;
#ifdef S_IFLNK
    case 'l':			/* symbolic link */
      type_cell = S_IFLNK;
      break;
#endif
#ifdef S_IFIFO
    case 'p':			/* pipe */
      type_cell = S_IFIFO;
      break;
#endif
#ifdef S_IFSOCK
    case 's':			/* socket */
      type_cell = S_IFSOCK;
      break;
#endif
    default:			/* None of the above ... nuke 'em. */
      return (false);
    }
  our_pred = insert_victim (which_pred);
  our_pred->args.type = type_cell;
  (*arg_ptr)++;			/* Move on to next argument. */
  return (true);
}

/* If true, we've determined that the current fprintf predicate
   uses stat information. */
static boolean fprintf_stat_needed;

boolean
insert_fprintf (fp, func, argv, arg_ptr)
     FILE *fp;
     boolean (*func) ();
     char *argv[];
     int *arg_ptr;
{
  char *format;			/* Beginning of unprocessed format string. */
  register char *scan;		/* Current address in scanning `format'. */
  register char *scan2;		/* Address inside of element being scanned. */
  struct segment **segmentp;	/* Address of current segment. */
  struct predicate *our_pred;

  format = argv[(*arg_ptr)++];

  fprintf_stat_needed = false;	/* Might be overridden later. */
  our_pred = insert_victim (func);
  our_pred->side_effects = true;
  our_pred->args.printf_vec.stream = fp;
  segmentp = &our_pred->args.printf_vec.segment;
  *segmentp = NULL;

  for (scan = format; *scan; scan++)
    {
      if (*scan == '\\')
	{
	  scan2 = scan + 1;
	  if (*scan2 >= '0' && *scan2 <= '7')
	    {
	      register int n, i;

	      for (i = n = 0; i < 3 && (*scan2 >= '0' && *scan2 <= '7');
		   i++, scan2++)
		n = 8 * n + *scan2 - '0';
	      scan2--;
	      *scan = n;
	    }
	  else
	    {
	      switch (*scan2)
		{
		case 'a':
		  *scan = 7;
		  break;
		case 'b':
		  *scan = '\b';
		  break;
		case 'c':
		  make_segment (segmentp, format, scan - format, KIND_STOP);
		  return (true);
		case 'f':
		  *scan = '\f';
		  break;
		case 'n':
		  *scan = '\n';
		  break;
		case 'r':
		  *scan = '\r';
		  break;
		case 't':
		  *scan = '\t';
		  break;
		case 'v':
		  *scan = '\v';
		  break;
		case '\\':
		  /* *scan = '\\'; * it already is */
		  break;
		default:
		  scan++;
		  continue;
		}
	    }
	  segmentp = make_segment (segmentp, format, scan - format + 1,
				   KIND_PLAIN);
	  format = scan2 + 1;	/* Move past the escape. */
	  scan = scan2;		/* Incremented immediately by `for'. */
	}
      else if (*scan == '%')
	{
	  if (scan[1] == '%')
	    {
	      segmentp = make_segment (segmentp, format, scan - format + 1,
				       KIND_PLAIN);
	      scan++;
	      format = scan + 1;
	      continue;
	    }
	  /* Scan past flags, width and precision, to verify kind. */
	  for (scan2 = scan; *++scan2 && index ("-+ #", *scan2);)
	    /* Do nothing. */ ;
	  while (isdigit (*scan2))
	    scan2++;
	  if (*scan2 == '.')
	    for (scan2++; isdigit (*scan2); scan2++)
	      /* Do nothing. */ ;
	  if (index ("abcdfgGhHiklmnpPstuU", *scan2))
	    {
	      segmentp = make_segment (segmentp, format, scan2 - format,
				       (int) *scan2);
	      scan = scan2;
	      format = scan + 1;
	    }
	  else if (index ("ACT", *scan2) && scan2[1])
	    {
	      segmentp = make_segment (segmentp, format, scan2 - format,
				       *scan2 | (scan2[1] << 8));
	      scan = scan2 + 1;
	      format = scan + 1;
	      continue;
	    }
	  else
	    {
	      /* An unrecognized % escape.  Print the char after the %. */
	      segmentp = make_segment (segmentp, format, scan - format,
				       KIND_PLAIN);
	      format = scan + 1;
	      continue;
	    }
	}
    }

  if (scan > format)
    make_segment (segmentp, format, scan - format, KIND_PLAIN);
  our_pred->need_stat = fprintf_stat_needed;
  return (true);
}

/* Create a new fprintf segment in *SEGMENT, with type KIND,
   from the text in FORMAT, which has length LEN.
   Return the address of the `next' pointer of the new segment. */

struct segment **
make_segment (segment, format, len, kind)
     struct segment **segment;
     char *format;
     int len, kind;
{
  char *fmt;

  *segment = (struct segment *) xmalloc (sizeof (struct segment));

  (*segment)->kind = kind;
  (*segment)->next = NULL;
  (*segment)->text_len = len;

  fmt = (*segment)->text = xmalloc (len + 3);	/* room for "ld\0" */
  strncpy (fmt, format, len);
  fmt += len;

  switch (kind & 0xff)
    {
    case KIND_PLAIN:		/* Plain text string, no % conversion. */
    case KIND_STOP:		/* Terminate argument, no newline. */
      break;

    case 'a':			/* atime in `ctime' format */
    case 'c':			/* ctime in `ctime' format */
    case 'g':			/* group name */
    case 'l':			/* object of symlink */
    case 't':			/* mtime in `ctime' format */
    case 'u':			/* user name */
    case 'A':			/* atime in user-specified strftime format */
    case 'C':			/* ctime in user-specified strftime format */
    case 'T':			/* mtime in user-specified strftime format */
      fprintf_stat_needed = true;
      /* FALLTHROUGH */
    case 'f':			/* basename of path */
    case 'h':			/* leading directories part of path */
    case 'H':			/* ARGV element file was found under */
    case 'p':			/* pathname */
    case 'P':			/* pathname with ARGV element stripped */
      *fmt++ = 's';
      break;

    case 'b':			/* size in 512-byte blocks */
    case 'k':			/* size in 1K blocks */
    case 's':			/* size in bytes */
      *fmt++ = 'l';
      /*FALLTHROUGH*/
    case 'n':			/* number of links */
      fprintf_stat_needed = true;
      /* FALLTHROUGH */
    case 'd':			/* depth in search tree (0 = ARGV element) */
      *fmt++ = 'd';
      break;

    case 'i':			/* inode number */
      *fmt++ = 'l';
      /*FALLTHROUGH*/
    case 'G':			/* GID number */
    case 'U':			/* UID number */
      *fmt++ = 'u';
      fprintf_stat_needed = true;
      break;

    case 'm':			/* mode as octal number (perms only) */
      *fmt++ = 'o';
      fprintf_stat_needed = true;
      break;
    }
  *fmt = '\0';

  return (&(*segment)->next);
}

boolean
insert_exec_ok (func, argv, arg_ptr)
     boolean (*func) ();
     char *argv[];
     int *arg_ptr;
{
  int start, end;		/* Indexes in ARGV of start & end of cmd. */
  int num_paths;		/* Number of args with path replacements. */
  int path_pos;			/* Index in array of path replacements. */
  int vec_pos;			/* Index in array of args. */
  struct predicate *our_pred;
  struct exec_val *execp;	/* Pointer for efficiency. */

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);

  /* Count the number of args with path replacements, up until the ';'. */
  start = *arg_ptr;
  for (end = start, num_paths = 0;
       (argv[end] != NULL)
       && ((argv[end][0] != ';') || (argv[end][1] != '\0'));
       end++)
    if (strstr (argv[end], "{}"))
      num_paths++;
  /* Fail if no command given or no semicolon found. */
  if ((end == start) || (argv[end] == NULL))
    {
      *arg_ptr = end;
      return (false);
    }

  our_pred = insert_victim (func);
  our_pred->side_effects = true;
  execp = &our_pred->args.exec_vec;
  execp->paths =
    (struct path_arg *) xmalloc (sizeof (struct path_arg) * (num_paths + 1));
  execp->vec = (char **) xmalloc (sizeof (char *) * (end - start + 1));
  /* Record the positions of all args, and the args with path replacements. */
  for (end = start, path_pos = vec_pos = 0;
       (argv[end] != NULL)
       && ((argv[end][0] != ';') || (argv[end][1] != '\0'));
       end++)
    {
      register char *p;
      
      execp->paths[path_pos].count = 0;
      for (p = argv[end]; *p; ++p)
	if (p[0] == '{' && p[1] == '}')
	  {
	    execp->paths[path_pos].count++;
	    ++p;
	  }
      if (execp->paths[path_pos].count)
	{
	  execp->paths[path_pos].offset = vec_pos;
	  execp->paths[path_pos].origarg = argv[end];
	  path_pos++;
	}
      execp->vec[vec_pos++] = argv[end];
    }
  execp->paths[path_pos].offset = -1;
  execp->vec[vec_pos] = NULL;

  if (argv[end] == NULL)
    *arg_ptr = end;
  else
    *arg_ptr = end + 1;
  return (true);
}

/* Get a number of days and comparison type.
   STR is the ASCII representation.
   Set *NUM_DAYS to the number of days, taken as being from
   the current moment (or possibly midnight).  Thus the sense of the
   comparison type appears to be reversed.
   Set *COMP_TYPE to the kind of comparison that is requested.

   Return true if all okay, false if input error.

   Used by -atime, -ctime and -mtime (parsers) to
   get the appropriate information for a time predicate processor. */

boolean
get_num_days (str, num_days, comp_type)
     char *str;
     unsigned long *num_days;
     enum comparison_type *comp_type;
{
  int len_days;			/* length of field */

  if (str == NULL)
    return (false);
  switch (str[0])
    {
    case '+':
      *comp_type = COMP_LT;
      str++;
      break;
    case '-':
      *comp_type = COMP_GT;
      str++;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      *comp_type = COMP_EQ;
      break;
    default:
      return (false);
    }

  /* We know the first char has been reasonable.  Find the
     number of days to play with. */
  len_days = strspn (str, "0123456789");
  if ((len_days == 0) || (str[len_days] != '\0'))
    return (false);
  *num_days = (unsigned long) atol (str);
  return (true);
}

/* Insert a time predicate PRED.
   ARGV is a pointer to the argument array.
   ARG_PTR is a pointer to an index into the array, incremented if
   all went well.

   Return true if input is valid, false if not.

   A new predicate node is assigned, along with an argument node
   obtained with malloc.

   Used by -atime, -ctime, and -mtime parsers. */

boolean
insert_time (argv, arg_ptr, pred)
     char *argv[];
     int *arg_ptr;
     PFB pred;
{
  struct predicate *our_pred;
  unsigned long num_days;
  enum comparison_type c_type;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  if (!get_num_days (argv[*arg_ptr], &num_days, &c_type))
    return (false);
  our_pred = insert_victim (pred);
  our_pred->args.info.kind = c_type;
  our_pred->args.info.l_val = cur_day_start - num_days * DAYSECS
    + ((c_type == COMP_GT) ? DAYSECS - 1 : 0);
  (*arg_ptr)++;
#ifdef	DEBUG
  printf ("inserting %s\n", our_pred->p_name);
  printf ("    type: %s    %s  ",
	  (c_type == COMP_GT) ? "gt" :
	  ((c_type == COMP_LT) ? "lt" : ((c_type == COMP_EQ) ? "eq" : "?")),
	  (c_type == COMP_GT) ? " >" :
	  ((c_type == COMP_LT) ? " <" : ((c_type == COMP_EQ) ? ">=" : " ?")));
  printf ("%ld %s", our_pred->args.info.l_val,
	  ctime (&our_pred->args.info.l_val));
  if (c_type == COMP_EQ)
    {
      our_pred->args.info.l_val += DAYSECS;
      printf ("                 <  %ld %s", our_pred->args.info.l_val,
	      ctime (&our_pred->args.info.l_val));
      our_pred->args.info.l_val -= DAYSECS;
    }
#endif	/* DEBUG */
  return (true);
}

/* Get a number with comparision information.
   The sense of the comparision information is 'normal'; that is,
   '+' looks for inums or links > than the number and '-' less than.
   
   STR is the ASCII representation of the number.
   Set *NUM to the number.
   Set *COMP_TYPE to the kind of comparison that is requested.
 
   Return true if all okay, false if input error.

   Used by the -inum and -links predicate parsers. */

boolean
get_num (str, num, comp_type)
     char *str;
     unsigned long *num;
     enum comparison_type *comp_type;
{
  int len_num;			/* Length of field. */

  if (str == NULL)
    return (false);
  switch (str[0])
    {
    case '+':
      *comp_type = COMP_GT;
      str++;
      break;
    case '-':
      *comp_type = COMP_LT;
      str++;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      *comp_type = COMP_EQ;
      break;
    default:
      return (false);
    }

  /* We know the first char has been reasonable.  Find the number of
     days to play with. */
  len_num = strspn (str, "0123456789");
  if ((len_num == 0) || (str[len_num] != '\0'))
    return (false);
  *num = (unsigned long) atol (str);
  return (true);
}

/* Insert a number predicate.
   ARGV is a pointer to the argument array.
   *ARG_PTR is an index into ARGV, incremented if all went well.
   *PRED is the predicate processor to insert.

   Return true if input is valid, false if error.
   
   A new predicate node is assigned, along with an argument node
   obtained with malloc.

   Used by -inum and -links parsers. */

boolean
insert_num (argv, arg_ptr, pred)
     char *argv[];
     int *arg_ptr;
     PFB pred;
{
  struct predicate *our_pred;
  unsigned long num;
  enum comparison_type c_type;

  if ((argv == NULL) || (argv[*arg_ptr] == NULL))
    return (false);
  if (!get_num (argv[*arg_ptr], &num, &c_type))
    return (false);
  our_pred = insert_victim (pred);
  our_pred->args.info.kind = c_type;
  our_pred->args.info.l_val = num;
  (*arg_ptr)++;
#ifdef	DEBUG
  printf ("inserting %s\n", our_pred->p_name);
  printf ("    type: %s    %s  ",
	  (c_type == COMP_GT) ? "gt" :
	  ((c_type == COMP_LT) ? "lt" : ((c_type == COMP_EQ) ? "eq" : "?")),
	  (c_type == COMP_GT) ? " >" :
	  ((c_type == COMP_LT) ? " <" : ((c_type == COMP_EQ) ? " =" : " ?")));
  printf ("%ld\n", our_pred->args.info.l_val);
#endif	/* DEBUG */
  return (true);
}

FILE *
open_output_file (path)
     char *path;
{
  FILE *f;

  if (!strcmp (path, "/dev/stderr"))
    return (stderr);
  else if (!strcmp (path, "/dev/stdout"))
    return (stdout);
  f = fopen (path, "w");
  if (f == NULL)
    error (1, errno, "%s", path);
  return (f);
}
