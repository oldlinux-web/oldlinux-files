/* Our little helpers.  Exported functions are prototyped in sundries.h.
 * $Header: /usr/src/mount/RCS/sundries.c,v 1.1 1992/09/06 13:30:53 root Exp root $
 */

#include "sundries.h"


/* File pointer for /etc/mtab.  */
FILE *F_mtab;

/* File pointer for lock.  */
FILE *F_lock;

/* File descriptor for lock.  Value tested in unlock_mtab() to remove race.  */
int lock = -1;

/* String list constructor.  (car() and cdr() are defined in "sundries.h").  */
string_list
cons (char * const a, const string_list b)
{
  string_list p;

  p = xmalloc (sizeof *p);

  car (p) = a;
  cdr (p) = b;
  return p;
}

void *
xmalloc (size_t size)
{
  void *t;

  if (size == 0)
    return NULL;

  t = malloc (size);
  if (t == NULL)
    die (2, "not enough memory");
  
  return t;
}

char *
xstrdup (const char *s)
{
  char *t;

  if (s == NULL)
    return NULL;
 
  t = strdup (s);

  if (t == NULL)
    die (2, "not enough memory");

  return t;
}

/* Call this with SIG_BLOCK to block and SIG_UNBLOCK to unblock.  */
void
block_signals (int how)
{
  sigset_t sigs;

  sigfillset (&sigs);
  sigprocmask (how, &sigs, (sigset_t *) 0);
}


/* Non-fatal error.  Print message and return.  */
void
error (const char *fmt, ...)
{
  va_list args;

  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  fprintf (stderr, "\n");
  va_end (args);
}

/* Fatal error.  Print message and exit.  */
void volatile
die (int err, const char *fmt, ...)
{
  va_list args;

  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  fprintf (stderr, "\n");
  va_end (args);

  exit (err);
}

/* Ensure that the lock is released if we are interrupted.  */
static void volatile
handler (int sig)
{
  die (2, "%s", sys_siglist[sig]);
}

/* Create the lock file.  The lock file will be removed if we catch a signal
   or when we exit.  The value of lock is tested to remove the race.  */
void
lock_mtab (void)
{
  int sig = 0;
  struct sigaction sa;

  /* If this is the first time, ensure that the lock will be removed.  */
  if (lock < 0)
    {
      sa.sa_handler = handler;
      sigfillset (&sa.sa_mask);
  
      while (sig++ < 32)
	sigaction (sig, &sa, (struct sigaction *) 0);
    }

  if ((lock = open (MOUNTED_LOCK, O_WRONLY|O_CREAT|O_EXCL, 0644)) < 0)
    die (2, "can't create lock file %s: %s", MOUNTED_LOCK, strerror (errno));

  F_lock = fdopen (lock, "w");
}

/* Remove lock file.  */
void
unlock_mtab (void)
{
  endmntent (F_lock);
  if (lock != -1)
    unlink (MOUNTED_LOCK);
}


/* Create mtab with a root entry.  */
static void
create_mtab (void)
{
  struct mntent *fstab;

  if ((F_mtab = setmntent (MOUNTED, "a+")) == NULL)
    die (1, "mount: can't open %s for writing: %s", MOUNTED, strerror (errno));

  /* Find the root entry by looking it up in fstab, which might be wrong.
     We could statfs "/" followed by a slew of stats on /dev/ but then
     we'd have to unparse the mount options as well....  */
  if ((fstab = getfsfile ("/")) || (fstab = getfsfile ("root")))
    {
      if (addmntent (F_mtab, fstab) == 1)
	die (1, "mount: error writing %s: %s", MOUNTED, strerror (errno));
    }
  endmntent (F_mtab);
}

/* Open mtab.  */
void
open_mtab (const char *mode)
{
  if (fopen (MOUNTED, "r") == NULL)
    create_mtab ();
  if ((F_mtab = setmntent (MOUNTED, mode)) == NULL)
    die (2, "can't open %s: %s", MOUNTED, strerror (errno));
}

/* Close mtab.  */
void
close_mtab (void)
{
  endmntent (F_mtab);
}


/* Parse a -t typelist into a string list.  */
string_list
parse_types (char *types)
{
  string_list list;
  char *t;

  if (types == NULL)
    return NULL;

  list = cons (strtok (types, ","), NULL);

  while ((t = strtok (NULL, ",")) != NULL)
    list = cons (t, list);

  return list;
}

/* True if fstypes match.  Null *TYPES means match anything,
   except that swap types always return false.  This routine
   has some ugliness to deal with ``no'' types.  */
int
matching_type (const char *type, string_list types)
{
  char *notype;
  int no;			/* true if a "no" type match, ie -t nominix */

  if (streq (type, MNTTYPE_SWAP))
    return 0;
  if (types == NULL)
    return 1;

  if ((notype = alloca (strlen (type) + 3)) == NULL)
    die (2, "mount: out of memory");
  sprintf (notype, "no%s", type);
  no = (car (types)[0] == 'n') && (car (types)[1] == 'o');

  /* If we get a match and the user specified a positive match type (e.g.
     "minix") we return true.  If we match and a negative match type (e.g.
     "nominix") was specified we return false.  */
  while (types != NULL)
    if (streq (type, car (types)))
      return !no;
    else if (streq (notype, car (types)))
      return 0;			/* match with "nofoo" always returns false */
    else
      types = cdr (types);

  /* No matches, so if the user specified a positive match type return false,
     if a negative match type was specified, return true.  */
  return no;
}

/* Make a canonical pathname from PATH.  Returns a freshly malloced string.
   It is up the *caller* to ensure that the PATH is sensible.  i.e.
   canonicalize ("/dev/fd0/.") returns "/dev/fd0" even though ``/dev/fd0/.''
   is not a legal pathname for ``/dev/fd0.''  */
char *
canonicalize (const char *path)
{
  char *canonical = xmalloc (PATH_MAX + 1);
  char *p = canonical;
  
  if (path == NULL)
    return NULL;
  
  if (*path == '/')
    {
      /* We've already got an absolute path in PATH, but we need at
	 least one char in canonical[] on entry to the loop below.  */
      *p = *path++;
    }
  else
    {
      getcwd (canonical, PATH_MAX);
      p = canonical + strlen (canonical) - 1;
      if (*p != '/')
	*++p = '/';
    }
  
  /* There is at least one character in canonical[],
     and the last char in canonical[], *p, is '/'.  */
  while ((*path != '\0') && (p < canonical + PATH_MAX))
    if (*p != '/')
      {
	*++p = *path++;
      }
    else
      {
	if (path[0] == '/')
	  {
	    path++;		/* eliminate duplicate slashes (``//'') */
	  }
	else if ((path[0] == '.') && ((path[1] == '\0') || (path[1] == '/')))
	{
	  path++;		/* eliminate ``/.'' */
	}
	else if ((path[0] == '.') && (path[1] == '.')
		 && ((path[2] == '\0') || (path[2] == '/')))
	  {
	    while ((p > canonical) && (*--p != '/'))
	      /* ascend on ``/../'' */
	      ;
	    path += 2;
	  }
	else
	  {
	    *++p = *path++;	/* not a special case, just copy char */
	  }
      }
  if (p >= (canonical + PATH_MAX))
    die (2, "mount: path too long");

  if (*p == '/')
    --p;			/* eliminate trailing slash */

  *++p = '\0';
  
  return canonical;
}
