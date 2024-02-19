/*
$Header: /usr/src/linux/lp/lp.c,v 1.2 1992/01/12 18:21:10 james_r_wiegand Exp $

  - linux/lp/lp.c main routine for printer job tool
  c. 1992 James Wiegand
*/

#include <fcntl.h>
#include <getopt.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "lpjob.h"

#define FAILURE -1
int verbose = 0;
int atoi( char * );


void message( char *format, ...)
{
va_list args;

	if( verbose )
		{
		va_start( args, format );
		vfprintf( stderr, format, args );
		va_end( args );
		}
}

void usage( void )
{
  fprintf( stderr,
           "\n\r\tusage: lp [-c] [-d destination] [-n number of copies]\n\r");
}

int main( int argc, char **argv )
{
struct passwd *pwdptr;
struct group  *grpptr;
int result;
int filein, fileout;
FILE *qfile;
time_t timenum;
char spoolname[ 1024 ];
char *spool[ 30 ];
char temp[ 128 ];
int spoolcount = 0;
char dest[ 128 ];
unsigned int copies, hold = 0, stdinput = 0;
int count;
char base[ 15 ];

char *strdup( char * );

  dest[ 0 ] = 0;
 
  while( 1 )
    {
    result = getopt( argc, argv, "cd:n:v" );
    if( result == FAILURE )
      break;
    switch( result )
      {
      case 'd':
              strcpy( dest, optarg );
              message( "using job setup %s\n\r", dest );
              break;
			case 'v':
							verbose = 1;
							break;							
      case 'c':
              hold = 1;
              printf( "job hold on\n\r" );
              break;
      case 'n':
              copies = atoi( optarg );
              if( !copies || copies > 200 )
                {
                fprintf( stderr, "can't have %d copies\n\r", copies );
                exit( 1 );
                }
              else
                message( "%d copies\n\r", copies );
              break;
      default:
        usage();
        exit( 1 );
      }
    }
    
  if( optind < argc )
    message( "files submitted on command line\n\r" );
  else
    {
    message( "input from (stdin)\n\r" );
    stdinput = 1;
    optind --;
    }

  pwdptr = getpwuid( getuid() );
  grpptr = getgrgid ( pwdptr->pw_gid  ); 
  strcpy( pusername, pwdptr->pw_name );
  strcpy( pgroup,    grpptr->gr_name );
  message( "username %s\n\r", pusername );
  message( "group    %s\n\r", pgroup    );

  if( dest[ 0 ])
    defaultjob = dest; 
  lpjobinit();

  pwdptr = getpwnam( "daemon" );
  message( "daemon uid %d\n\r", pwdptr->pw_uid );
  if( setuid( pwdptr->pw_uid ) == FAILURE )
    {
    printf( "no setuid\n\r" );
    exit( 1 );
    }
  message( "%s\n\r", argv[ optind ]);
  
  while( optind < argc )
  {
  if( !stdinput ) 
    {
    message( "%s\n\r", argv[ optind ]);
    filein = open( argv[ optind ], O_RDONLY );
    if( filein == FAILURE )
      {
      printf( "cannot open file %s\n\r", argv[ optind ]);
      exit( 1 );
      }
    else
      {
      message( "submitting file %s\n\r", argv[ optind ]);
      strcpy( pfilename, argv[ optind ]);
      }
    }
  else
    { 
    message( "stdin setup\n\r" );
    filein = STDIN_FILENO;
    strcpy( pfilename, "(stdin)" );
    }
  sprintf( base, "%x", time( &timenum ));
  sprintf( spoolname, "%s/%s", SSV(SD), base );
  message( "spooling to file %s\n\r", spoolname );

	sprintf( temp, "%s/.%s", SSV(SD), base );
  message( "queue file %s\n\r", temp );

  qfile = fopen( temp, "w" );
  fprintf( qfile, "%s:%s:%s\n", pfilename, pusername, pgroup );
  fclose( qfile );
  
  fileout = open( spoolname, O_WRONLY | O_CREAT, S_IWUSR );

  if( fileout == FAILURE )
    {
    printf( "no spool file \n\r" );
    exit( 1 );
    }
  
  /* do the banner page */
  if( !NSV(SH) )
    {
    lpbanner( fileout );
    write( fileout, SSV(FF), 1 );
    }
 
  /* process text */
  processtext( filein, fileout ); 

  close( fileout );
  /* needed to get a unique filename :-) */
  sleep( 1 );
  /* make file visible to lp daemon */

  spool[ spoolcount ] = strdup( spoolname );

  spoolcount ++;
  optind ++;
  }

  for( count = 0; count < spoolcount; count ++ )
    {
    message( "releasing file %s\n\r", spool[ count ]);
    chmod( spool[ count ], S_IRUSR | S_IWUSR ); 
    }


  exit( 0 );
}

