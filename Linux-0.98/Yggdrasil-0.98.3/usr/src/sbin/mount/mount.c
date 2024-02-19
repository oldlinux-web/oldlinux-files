/*
 * A mount(8) for Linux 0.97.
 * $Header: /usr/src/mount/RCS/mount.c,v 1.1 1992/09/06 13:30:53 root Exp root $
 */ 

/* The interface to mount(2) changed from 4 arguments to 5 in 0.96c-pl1.
   The 2.2.2 libraries predate the change, so we need to fix the syscall.
   The 2.2.2d libc and later have the new mount(2), but it's simpler
   to do this for everybody who has a 5 arg mount(2).  */

#ifdef HAVE_MOUNT5

#define __LIBRARY__
#include <sys/mount.h>
#include <unistd.h>

_syscall5(int, mount, const char *, special, const char *, dir,
	   const char *, type, unsigned long, flags, const void *, data);
#endif


#include "sundries.h"


const char *usage = "\
Usage: mount [-afrvw] [-t vfstypes]
       mount [-frvw] special | node
       mount [-frvw] [-t vfstype] [-o options] special node";

#ifdef notyet
/* True for change options of an already mounted fs (-u).  */
int update = 0;
#endif

/* True for fake mount (-f).  */
int fake = 0;

/* True for readonly (-r).  */
int readonly = 0;

/* Nonzero for chatty (-v).  */
int verbose = 0;

/* True for read/write (-w).  */
int readwrite = 0;

/* Map from -o and fstab option strings to the flag argument to mount(2).  */
struct opt_map
{
  const char *opt;		/* option name */
  int  inv;			/* true if flag value should be inverted */
  int  mask;			/* flag mask value */
};

const struct opt_map opt_map[] =
{
  {"defaults",	0, MNT_DEFAULT},	/* default options */
  {"ro",	0, MNT_RDONLY},		/* read-only */
  {"rw",	1, MNT_RDONLY},		/* read-write */
  {"exec",	1, MNT_NOEXEC},		/* permit execution of binaries */
  {"noexec",	0, MNT_NOEXEC},		/* don't execute binaries */
  {"suid",	1, MNT_NOSUID},		/* honor suid executables */
  {"nosuid",	0, MNT_NOSUID},		/* don't honor suid executables */
  {"dev",	1, MNT_NODEV},		/* interpret device files  */
  {"nodev",	0, MNT_NODEV},		/* don't interpret devices */
#ifdef notyet
  {"synchronous", 0, MNT_SYNCHRONOUS},	/* synchronous I/O */
  {"sub",	1, MNT_NOSUB},		/* allow submounts */
  {"nosub",	0, MNT_NOSUB},		/* don't allow submounts */
#endif
  /* add new options here */
  {NULL,	0, 0}
};


/* Report on a single mount.  */
static void
print_one (const struct mntent *mnt)
{
  printf ("%s on %s", mnt->mnt_fsname, mnt->mnt_dir);
  if ((mnt->mnt_type != NULL) && *mnt->mnt_type != '\0')
    printf (" type %s", mnt->mnt_type);
  if (mnt->mnt_opts != NULL)
    printf (" (%s)", mnt->mnt_opts);
  printf ("\n");
}

/* Report on everything in mtab (of the specified types if any).  */
static volatile int
print_all (string_list types)
{
  struct mntent *mnt;
  
  open_mtab ("r");

  while ((mnt = getmntent (F_mtab)) != NULL)
    if (matching_type (mnt->mnt_type, types))
      print_one (mnt);

  if (ferror (F_mtab))
    die (1, "mount: error reading %s: %s", MOUNTED, strerror (errno));

  exit (0);
}


/* Look for OPT in opt_map table and return mask value.  If OPT isn't found,
   tack it onto extra_opts.  Due to laziness, the first char of extra_opts
   will be ',' if any extra options are found.  */
static void inline
parse_opt (const char *opt, int *mask, char *extra_opts)
{
  const struct opt_map *om;

  for (om = opt_map; om->opt != NULL; ++om)
    if (streq (opt, om->opt))
      {
	if (om->inv)
	  *mask &= ~om->mask;
	else
	  *mask |= om->mask;
	return;
      }
  strcat (extra_opts, ",");
  strcat (extra_opts, opt);
}
  
/* Take -o options list and compute 4th and 5th args to mount(2).  flags
   gets the standard options and extra_opts anything we don't recognize.  */
static void
parse_opts (char *opts, int *flags, char **extra_opts)
{
  char *opt;

  if (opts != NULL)
    {
      *extra_opts = xmalloc (strlen (opts) + 1); 
      **extra_opts = '\0';

      for (opt = strtok (opts, ",");
	   opt != NULL;
	   opt = strtok (NULL, ","))
	parse_opt (opt, flags, *extra_opts);
    }

  if (readonly)
    *flags |= MNT_RDONLY;
  if (readwrite)
    *flags &= ~MNT_RDONLY;
}

/* Fix opts_string to defaults if NULL, else merge readonly and readwrite.
   Simplistic, merging ``-r'' with ``-o rw''a will give ``ro,rw''.  */
static char *
fix_opts_string (char *opts)
{
  char *fixed;

  if (readonly || readwrite)
    {
      if (opts == NULL)
	return (readonly ? "ro" : "rw");

      fixed = xmalloc (strlen (opts) + 4);
	
      strcpy (fixed, readonly ? "ro," : "rw,");
      strcat (fixed, opts);
      return fixed;
    }
  return (opts == NULL ? "defaults" : opts);
}

/* Mount a single file system.  Return status,
   so don't exit on non-fatal errors.  */
static int
mount_one (char *spec, char *node, char *type, char *opts)
{
  struct mntent mnt = {0};
  int flags = 0;
  char *extra_opts = NULL;

  if (type == NULL)
    type = FSTYPE_DEFAULT;

  parse_opts (xstrdup (opts), &flags, &extra_opts);

  block_signals (SIG_BLOCK);

  if (fake || (mount5 (spec, node, type, flags, extra_opts)) == 0)
    /* Mount succeeded, write mtab entry.  */
    {
      mnt.mnt_fsname = canonicalize (spec);
      mnt.mnt_dir = canonicalize (node);
      mnt.mnt_type = type;
      mnt.mnt_opts = fix_opts_string (opts);
      
      /* We get chatty now rather than after the update to mtab since the
	 mount succeeded, even if the write to /etc/mtab should fail.  */
      if (verbose)
	print_one (&mnt);

      if (!fake && (addmntent (F_mtab, &mnt)) == 1)
	die (1, "mount: error writing %s: %s", MOUNTED, strerror(errno));

      block_signals (SIG_UNBLOCK);
      return 0;
    }

  block_signals (SIG_UNBLOCK);

  /* Mount failed, complain, but don't die.  */
  switch (errno)
    {
    case EPERM:
      error ("mount: must be superuser"); break;
    case EBUSY:
      error ("mount: %s already mounted or %s busy", spec, node); break;
    case EINVAL:
      error ("mount: %s: bad superblock", spec); break;
    case EMFILE:
      error ("mount table full"); break;
    case EIO:
      error ("mount: %s: can't read superblock", spec); break;
    default:
      error ("mount: %s", strerror (errno)); break;
    }
  return 1;
}

/* Mount all filesystems of the specified types except swap and root.  */
static int
mount_all (string_list types)
{
  struct mntent *fstab;
  int status;

  lock_mtab ();
  open_mtab ("a+");
  unlink (MOUNTED_LOCK);

  status = 0;
  while ((fstab = getfsent ()) != NULL)
    if 	((matching_type (fstab->mnt_type, types))
	 && !streq (fstab->mnt_dir, "/")
	 && !streq (fstab->mnt_dir, "root"))
      status |= mount_one (fstab->mnt_fsname,
			   fstab->mnt_dir,
			   fstab->mnt_type,
			   fstab->mnt_opts);

  return status;
}

int
main (int argc, char **argv)
{
  int opt;
  int all = 0;
  char *options = NULL;
  string_list types = NULL;
  struct mntent *fs;
  char *spec;

  while ((opt = getopt (argc, argv, "aft:o:ruvw")) != EOF)
    switch (opt)
      {
      case 'a':			/* mount everything in fstab */
	++all;
	break;
      case 'f':			/* fake (don't actually do mount(2) call) */
	++fake;
	break;
      case 't':			/* specify file system types */
	types = parse_types (optarg);
	break;
      case 'o':			/* specify mount options */
	options = optarg;
	break;
      case 'r':			/* mount readonly */
	++readonly;
	break;
#ifdef notyet
/* Needs kernel support that we don't have yet, & some rewrites here also.  */
      case 'u':			/* change options on an already mounted fs */
	++change; 
	break;
#endif
      case 'v':			/* be chatty */
	++verbose;
	break;
      case 'w':			/* mount read/write */
	++readwrite;
	break;
      default:
	die (2, usage);
      }

  argc -= optind;
  argv += optind;

  if (readonly && readwrite)
    die (2, "mount: the -r and -w options conflict\n%s", usage);

  if (argc == 0)
    if (options)
      die (2, usage);
    else if (all)
      return mount_all (types);
    else
      return print_all (types);

  lock_mtab ();
  open_mtab ("a+");
  unlink (MOUNTED_LOCK);

  switch (argc)
    {
    case 1:
      /* mount [-frvw] special | node */
      if ((types != NULL) || (options != NULL))
	die (2, usage);
      /* Try to find the other pathname in fstab.  */ 
      spec = canonicalize (*argv);
      if (!(fs = getfsspec (spec)) && !(fs = getfsfile (spec)))
	die (2, "mount: can't find %s in %s\n", spec, _PATH_FSTAB);
      return mount_one (fs->mnt_fsname, fs->mnt_dir,
			fs->mnt_type, fs->mnt_opts);
    case 2:
      /* mount [-frvw] [-t vfstype] [-o options] special node */
      if (types == NULL)
	return mount_one (argv[0], argv[1], NULL, options);
      if (cdr (types) == NULL)
	return mount_one (argv[0], argv[1], car (types), options);
      die (2, usage);
      
    default:
      die (2, usage);
    }
}
