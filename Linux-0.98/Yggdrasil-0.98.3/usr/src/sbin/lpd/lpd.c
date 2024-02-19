
/*
$Header: /usr/src/linux/lp/lpd.c,v 1.2 1992/01/12 Exp james_r_wiegand $

  - linux/lp/lpd.c line printer daemon
  c. 1992 James Wiegand
*/

#include "pcap.h"
#include <dirent.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>  
#include <sys/types.h>

#define FAILURE -1
#define SUCCESS  0
#define DEBUG
#define MAX_JOBS 30

#define PANIC( msg ) {fprintf( stderr, msg); exit(2);}

extern int errno;
int lp_child = 0;
int runningat = 0;
char *logdefault = "/usr/bin/mail";

enum { TERSE, CHATTY, VERBOSE };

struct 
  {
  char dir[ _POSIX_PATH_MAX ];
  char dev[ _POSIX_PATH_MAX ];
  char log[ _POSIX_PATH_MAX ];
  char current[ _POSIX_NAME_MAX ];  
  struct
    {
    char list[ _POSIX_NAME_MAX ];
    int order;
    } jobs[ MAX_JOBS ];
  int index, currentindex;
  } lpd_data;

void log_entry( int runlevel, char *message, ... )
{
va_list args;
FILE *fp;

  fp = fopen( lpd_data.log, "a" ); 
  if( runlevel >= runningat )
    {
    va_start( args, message );
    vfprintf( fp, message, args );
    va_end( args );
    }

  fclose( fp );
  return;
}
  
void lp_read( void )
{
DIR *dirptr;
struct dirent *direntptr;
int count;
struct stat buf;

  for( count = 0; count < MAX_JOBS; count ++ )
    lpd_data.jobs[ count ].list[ 0 ] = 0x0;
    
  dirptr = opendir( "." );
  lpd_data.index = 0;
  sleep( 5 );
  if( !dirptr )
    {
    closedir( dirptr );
    return;
    }
  while( 1 )
    {
    direntptr = readdir( dirptr );
    if( !direntptr )
      break;

    if( *(direntptr->d_name) == '.' )
      continue;

    stat( direntptr->d_name, &buf );
    if( buf.st_mode & S_IRUSR )
      {
      log_entry( VERBOSE, "found file: %s\n", direntptr->d_name );
      if( lpd_data.index < MAX_JOBS )
        {
        strcpy( lpd_data.jobs[ lpd_data.index ].list, direntptr->d_name );
        lpd_data.index ++;
        }
      else
        {
        log_entry( TERSE, "maximum number of jobs taken\n" );
        break;
        }
      }
    }
  closedir( dirptr );
}

void lp_lock( char *jobname )
{
FILE *fp;

  fp = fopen( ".daemon", "w" );
  if( !fp )
    PANIC( "cannot create lock file\n\r" );

  fprintf( fp, "%d\n", getpid() );

  if( !jobname )
    fprintf( fp, "idle\n" );
  else
    fprintf( fp, "%s\n", jobname );

  fclose( fp );
  
}

void lp_dir( void )
{
/*
  change to daemon directory, abort on failure
  abort if blocked by lock file
*/
struct stat buf;

  log_entry( VERBOSE, "changing to %s \n", lpd_data.dir );
  
  if( chdir( lpd_data.dir ) == FAILURE )
    PANIC( "Unable to set directory\n\r" );

  if( stat( ".daemon", &buf ) == SUCCESS )
    {
    errno = EAGAIN;    
    PANIC( "Would block running daemon: lock file exist\n\r" );
    }
  
}
 
void cleanup( int dummy )
{

  /* bounce the signal to child */
  if( lp_child )
    {
    kill( lp_child, SIGKILL );
    }
    
  unlink( ".daemon" );
  log_entry( CHATTY, "TERM signal received\n" );
  exit( 2 );
  
}

int main( int argc, char *argv[] )
{
char *optstring = "d:r:";
int result;
char devname[ 20 ];
char pcapentry[ 1024 ];
char *cptr;
char temp[ 1024 ];
char *pstr= temp;
int filein, deviceout;
int status = 0, test;
char buf[ 512 ];

  while( 1 )
    {
    result = getopt( argc, argv, optstring );
    if( result == FAILURE )
      break;
    switch( result )
      {
      case 'd':
              if( optarg )
                strcpy( devname, optarg );
                status = 1;
                break;
      case 'r':
              if( optarg )
                runningat = atoi( optarg );
              break;
      default:
                break;
      }
    }

  /* make sure a device name was said */
  if( !status )
    PANIC( "usage: /usr/lib/lpd -d printer_name \n\r" );

  /* make default log file - /usr/bin/mail/<devicename> */
  strcpy( lpd_data.log, logdefault );
  strcat( lpd_data.log, "/" );
  strcat( lpd_data.log, devname );
  
  log_entry( TERSE, "lpd: trying printer %s\n", devname );
  log_entry( TERSE, "lpd: running at %d\n", runningat );

  result = pgetent( pcapentry, devname );
  log_entry( VERBOSE, "result=%d, %s\n", result, pcapentry );

  if( result != 1 )
    PANIC( "unable to read /etc/printcap or PRINTCAP=\n\r" );

  cptr = pgetstr( "lp", &pstr );
  if( cptr )
    {
    log_entry( VERBOSE, "dev = %s\n", cptr );
    strcpy( lpd_data.dev, cptr );
    }
  else
    PANIC( "no device for printer\n\r" );
    
  cptr = pgetstr( "sd", &pstr );
  if( cptr )
    {
    log_entry( VERBOSE, "dir = %s\n", cptr );
    strcpy( lpd_data.dir, cptr );
    }
  else
    PANIC( "no spool dir\n\r" );
    
  cptr = pgetstr( "lf", &pstr );
  if( cptr )
    {
    strcpy( lpd_data.log, cptr );
    strcat( lpd_data.log, "/" );
    strcat( lpd_data.log, devname ); 
    log_entry( VERBOSE, "log = %s\n", lpd_data.log );
    }

  lp_dir();

  signal( SIGTERM, cleanup );
  
  lp_lock( (char *) 0 ); 
  lp_read();  
  for( result = 0; result < MAX_JOBS; result ++)
    {
    if( !lpd_data.jobs[ result ].list[ 0 ] )
      break;
    log_entry(CHATTY, "Job %d: %s\n",
              result, lpd_data.jobs[ result ].list );
    }

  lpd_data.index = 0;

  while( 1 )
  {
  if( !lpd_data.jobs[ lpd_data.index ].list[ 0 ] )
    {
    while( 1 )
      {
      lp_read();
      if( lpd_data.index > 0 )
        {
        lpd_data.index = 0;
        break;
        }
      }
    }

  deviceout = open( lpd_data.dev, O_WRONLY );
  if( deviceout == FAILURE )
    PANIC( "lpd: device open" );
    
  filein  = open( lpd_data.jobs[ lpd_data.index ].list, O_RDONLY );
  if( filein == FAILURE )
    perror( "lpd: file open: ");
    
  log_entry( CHATTY, "printing job: %s\n",
             lpd_data.jobs[ lpd_data.index ].list );
  lp_lock( lpd_data.jobs[ lpd_data.index ].list );
  if(( lp_child = fork()) == 0 )
    {
    while( 1 )
      {
      result = read( filein, buf, 512 );
      if( result == FAILURE )
        {
        perror( "lpd: file read: ");
        break;
        }
        
      test = write( deviceout, buf, result );
      /* should do error recovery here */
      if( test < 0 )
        perror( "lpd: " );
      if( result < 512 )
        break;
      }
    exit( 0 ); /* child dies after write is completed */
    }

  log_entry( VERBOSE, "child is pid %d\n", lp_child );
  wait( &status ); /* parent picks up loop */
  /* no child */
  lp_child = 0;
  close( filein );
  close( deviceout );
  log_entry( CHATTY, "discarding file: %s\n",
             lpd_data.jobs[ lpd_data.index ].list );
  unlink( lpd_data.jobs[ lpd_data.index ].list );
	sprintf( temp, ".%s", lpd_data.jobs[ lpd_data.index ].list );
  log_entry( CHATTY, "discarding file: %s\n", temp );
  unlink( temp );
	
  lpd_data.index ++;
  }
  exit( 0 );
}
