/* YOCS #1 - install(1) - install files */

#ifndef lint
static char *RCSid = "$Id: install.c,v 1.2 1992/01/01 15:26:37 yocs Exp $" ;
#endif

/*
 * $Log: install.c,v $
 * Revision 1.2  1992/01/01  15:26:37  yocs
 * Fixed option handling to work correctly if a '-g' is given
 * before a '-o'. Previously is forgot the '-g' setting in favour
 * of the '-o' user's group.
 *
 * Revision 1.1  1991/04/09  22:54:27  yocs
 * Initial revision
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <ansi.h>


/* For getopt(3): */
extern char *optarg;
extern int   optind;


#define	CPBUFSIZE	16384	/* Copy in blocks of CPBUSIZE bytes */
#define	err(func)	fputs( func, stderr );
#define	STRIP		"strip %s"


char
   cpbuf [CPBUFSIZE],
  *prog;

uid_t
  uid		= -1;		/* Owner to be, default: same as user */
gid_t
  gid		= -1;		/* Group to be, default: same as user */
unsigned int
  mode		= 0755;		/* File mode, default: executable/dir */
int
  dostrip	= 0;		/* Strip the file after copying? */


_PROTOTYPE( int  omode		, ( char *arg ) );
_PROTOTYPE( void Usage		, ( void ) );
_PROTOTYPE( void Fixup		, ( char *name ) );
_PROTOTYPE( void Makedir	, ( char *name ) );
_PROTOTYPE( void CopyFile	, ( char *from, char *to ) );
_PROTOTYPE( void CopyToDir	, ( int argc, char *argv[], char *dir ) );



/* omode( arg ) - compute octal file mode
 *
 *   omode returns the mode value given in arg.
 *
 * Parameter:
 *	arg	- String to parse
 * Returns:
 *	-1 if arg contains invalid characters
 *	the value of arg otherwise
 * Error exits:
 *     none.
 */
int omode( arg )
  char *arg;
{ int m = 0;

#ifdef	DEBUG
  fprintf( stderr, "omode( \"%s\" );\n", arg );
#endif

  if (*arg == '\0')
    return -1;
  while ((*arg >= '0') && (*arg < '8'))
    m = (m << 3) + *arg++ - '0';
  if (*arg != '\0')
    return -1;

    return m;
}


void Usage( )
{
#ifdef	DEBUG
  fprintf( stderr, "Usage();\n" );
#endif

  fprintf( stderr, "Usage:\n" );
  fprintf( stderr
	 , "  %s [-cs] [-g group] [-o owner] [-m omode] f1 f2\n", prog );
  fprintf( stderr
	 , "  %s [-cs] [-g group] [-o owner] [-m omode] file ... dir\n"
	 , prog );
  fprintf( stderr, "  %s -d [-g group] [-o owner] [-m omode] dir\n", prog );
  exit( 1 );
}


/* Fixup( name ) - Fix ownership and mode
 *
 *   Fixup tries to set the required mode bits, and changes the owner-
 * ship if requested by -o/-g.
 *
 * Parameter:
 *	name	- name of the file to fix.
 * Returns:
 *	(void)
 * Error exits:
 *     Failed calls to chmod(2) or chown(2) will cause the program to
 *   abort with return value 1.
 */
void Fixup( name )
  char *name;
{
#ifdef	DEBUG
  fprintf( stderr, "Fixup( \"%s\" );\n", name );
#endif

  if (chmod( name, mode ))	/* Change mode bits */
  { err( "chmod: " ); perror( name );
    exit( 1 );
  }
  if ((uid != -1) &&		/*  and ownership */
    chown( name, uid, gid ))
  { err( "chown: " ); perror( name );
    exit( 1 );
  }
}


/* Makedir( name ) - make directories in given path
 *
 *   Makedir scans the given pathname, making the missing directories
 * along the way. Newly created directories, or else the one named by the
 * pathname if it already exists, are optionally chmodded and chowned, as
 * directed by the -m, -g, and -o parameters.
 *
 * Parameter:
 *	name	- Pathname to scan/create
 * Returns:
 *	(void)
 * Error exits:
 *     Failed calls to mkdir(2), chmod(2), or chown(2) will cause the
 *   program to abort with return value 1.
 */
void Makedir( name )
  char *name;
{ char *p = name,	/* pointer te end of pathname being checked */
	c,		/* character at that point, '/' or '\0' */
	isnew;		/* -1 if directory was newly created */

#ifdef	DEBUG
  fprintf( stderr, "Makedir( \"%s\" );\n", name );
#endif

  while (*p != '\0')
  { do { p++;			/* Skip to next '/' or end-of-string */
    } while ((*p != '/') && (*p != '\0'));
    c = *p;			/* Save character */
    *p = '\0';			/* Terminate pathname temporarily */
    if ((isnew = access( name, F_OK )) != 0)	/* exists? */
    { if (mkdir( name ))	/* Create it if not */
      { err( "mkdir: " ); perror( name );
	exit( 1 );
      }
    }
    if (isnew || (c == '\0'))
      Fixup( name );
    *p = c;			/* Restore string */
  }
}


/* CopyToDir( argc, argv, dir ) - Copy files to a directory
 *
 *   CopyToDir copies the argc files given in argv to the directory
 * dir. The files' indiviual names will stay the same. All destination
 * files are stripped, chmodded, or chowned if requested.
 *
 * Parameters:
 *	argc	- Number of files to copy
 *	argv	- Array of file names
 *	dir	- Directory to copy to
 * Returns:
 *	(void)
 * Error exits:
 *     Failure to copy - and subsequently do things to - a file will
 *   abort the program. No attempt is made to undo the copying if failure
 *   occurs halfway.
 */
void CopyToDir( argc, argv, dir )
  int   argc;
  char *argv[];
  char *dir;
{ char dest [256], *base;

#ifdef	DEBUG
  fprintf( stderr, "CopyToDir( %d, [\"%s\", ...], \"%s\" );\n"
		 , argc, *argv, dir );
#endif

  while (argc-- > 0)
  { if ((base = strrchr( *argv, '/' )) == NULL)
      base = *argv;
    else
      base++;
    (void) strcpy( dest, dir );
    (void) strcat( dest, "/" );
    (void) strcat( dest, base );
    CopyFile( *argv, dest );
    argv++;
  }
}


/* CopyFile( from, to ) - Copy a file (on)to another
 *
 *   CopyFile copies the file specified by from, to the one specified
 * by to. If dostrip is true, strip(1) is used on the file after
 * copying. Also optionally calls are made to chmod(2) and chown(2)
 * as specified by the -m, -g, and -o flags.
 *
 * Parameters:
 *	from	- pathname of the sourcefile
 *	to	- pathname of the destination
 * Returns:
 *	(void)
 * Error exits:
 *     Failed calls to open(2), read(2), write(2), system(3), chmod(2),
 *   or chown(2) will cause the program to abort with return value 1.
 */
void CopyFile( from, to )
  char *from, *to;
{ int fin, fout;
  int n;
  char cmnd [256];

#ifdef	DEBUG
  fprintf( stderr, "CopyFile( \"%s\", \"%s\" );\n", from, to );
#endif

  if ((fin = open( from, O_RDONLY )) < 0)
  { perror( from );
    exit( 1 );
  }
  if ((fout = open( to, O_WRONLY|O_CREAT|O_TRUNC, mode )) < 0)
  { perror( to );
    exit( 1 );
  }
  while ((n = read( fin, cpbuf, CPBUFSIZE )) != 0)
  { if (n < 0)
    { perror( from );
      exit( 1 );
    }
    if (write( fout, cpbuf, n ) != n)
    { perror( to );
      exit( 1 );
    }
  }
  close( fin );
  close( fout );
  if (dostrip)
  { (void) sprintf( cmnd, STRIP, to );
    if (system( cmnd ))
    { fprintf( stderr, "strip failed for %s\n", to );
      exit( 1 );
    }
  }
  Fixup( to );
}


/* main */
void main( argc, argv )
  int		 argc;
  char		*argv[];
{ int		 dodir = 0;
  struct passwd	*p;
  struct group	*g;
  int		 opt;
  struct stat	 s;

#ifdef	DEBUG
  int i;
  fprintf( stderr, "main( %d", argc );
  for (i = 0; i < argc; i++) fprintf( stderr, ", \"%s\"", argv [i] );
  fprintf( stderr, " );\n" );
#endif

  prog = argv [0];
  while ((opt = getopt( argc, argv, "csdg:o:m:" )) != EOF)
  {
#ifdef	DEBUG
    if ((opt == 'g') || (opt == 'o') || (opt == 'm'))
      fprintf( stderr, "Option '%c', \"%s\"\n", opt, optarg );
    else
      fprintf( stderr, "Option '%c'\n", opt );
#endif
    switch (opt)
    { case 'c':			/* For compatibility; copy files */
	break;
      case 's':			/* strip files after copy */
	dostrip = 1;
	break;
      case 'd':			/* Install a directory */
	dodir = 1;
	break;
      case 'g':			/* Set group ownership */
	if (uid == -1)		/*  make sure we have a user id */
	  uid = getuid( );
	if ((g = getgrnam( optarg )) == NULL)
	{ fprintf( stderr, "%s: unknown group %s\n", prog, optarg );
	  exit( 1 );
	}
	gid = g->gr_gid;
	break;
      case 'o':			/* Set ownership */
	if ((p = getpwnam( optarg )) == NULL) /* find user id */
	{ fprintf( stderr, "%s: unknown user %s\n", prog, optarg );
	  exit( 1 );
	}
	uid = p->pw_uid;	/* copy uid & gid */
	if (gid == -1)		/*  copy gid only if no "-g" yet */
	  gid = p->pw_gid;
	break;
      case 'm':			/* Set file mode */
        if ((mode = omode( optarg )) == -1)
	{ fprintf( stderr, "%s: bad mode %s\n", prog, optarg );
	  exit( 1 );
	}
	break;
      default:
        fprintf( stderr, "%s: bad option '%c'\n", prog, opt );
	exit( 1 );
    }
  }

  if ((uid != -1) && (getuid( ) != 0))
  { fprintf( stderr, "%s: must be root to use -o/-g\n", prog );
    exit( 1 );
  }
  if (dodir)			/* Create a directory, */
  { if ((argc-optind) != 1) Usage( );
    Makedir( argv [optind] );
  }
  else				/* or else copy files */
  { if ((argc-optind) < 2) Usage( );
    if ((stat( argv [argc-1], &s) == 0) &&
	(s.st_mode & S_IFDIR))
      CopyToDir( argc-optind-1, &argv [optind], argv [argc-1] );
    else
    { if ((argc-optind) != 2) Usage( );
      CopyFile( argv [optind], argv [optind+1] );
    }
  }
  exit( 0 );
}
