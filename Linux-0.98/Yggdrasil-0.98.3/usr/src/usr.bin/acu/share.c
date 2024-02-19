#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define LCKF "/usr/spool/uucp/"
#define PREFIX "LCK.."
#define PORT "/dev/tty65"
#define ON  1
#define OFF 0

int quiet = 0;

void xprintf( char *, ... );

int main (int argc, char **argv )
{
int pid; 
char buffer[ 128 ], buffer2[ 128 ];
FILE *fp;
int result;
int gswitch = ON;
register int c;
char *port;
int help = 0;
char *cptr;
char *portdev;
char *usage = "share: usage: share [ -0 | -1 ] -q -d port\n\r";
	
  for (;;)
    {
    if((c = getopt(argc, argv, "01d:hq")) == -1)
			break;
    else
			switch (c)
		  {
	  case '0':
				gswitch = OFF;
   			break;
	  case '1':
				gswitch = ON;
   			break;
		case 'q':
			quiet = 1;
			break;
	  case 'd':
	    port = optarg;
	    break;
	  case '?':
		case 'h':
	    help = 1;
	    break;
	  default:
	    break;
		  }
		}
  if (optind < argc || help)
    {
		fprintf( stderr, usage );
		exit(1);
    }

  if( !port )
  	port = PORT;
  	
	if( port )
		{
		cptr = strrchr( port, '/' );
		if( cptr )
			{
			cptr ++;
			portdev = cptr;
			}
		}
		
	xprintf( "Turning %s getty %s\n", port, (gswitch == ON) ? "on" : "off" );
	
	if( setuid(0) == -1 )
		perror( "setuid" );

	strcpy( buffer, LCKF );
	if( gswitch == ON )
		strcat( buffer, "." );
	strcat( buffer, PREFIX );
	strcat( buffer, portdev );

	strcpy( buffer2, LCKF );
	if( gswitch == OFF )
		strcat( buffer2, "." );
	strcat( buffer2, PREFIX );
	strcat( buffer2, portdev );
	
	
	fp = fopen( buffer, "r" );
	if( !fp )
		{
		xprintf( "filename %s: %s\n", buffer, strerror(errno) );
		xprintf( "port is probably already %s\n", (gswitch==ON) ? "ON":"OFF");
		exit(1);
		}
	result = fscanf( fp, "%d", &pid );
	fclose( fp );
	xprintf( "pid %d\n", pid );

	xprintf( "Renaming %s to %s\n", buffer, buffer2 );
	rename( buffer, buffer2 );	
	
	if( gswitch == ON )
		{
		chmod( port, 0622 );
		kill( pid, SIGCONT );		
		}
	else
		{
		chmod( port, 0666 );
		kill( pid, SIGSTOP );
		}

	return 0;	
}
	
void xprintf( char *fmt, ... )
{
va_list ap;

	if( !quiet )
	{
	va_start( ap, fmt );
	vfprintf( stdout, fmt, ap );
	va_end( ap );
	}
	
	return;
}
