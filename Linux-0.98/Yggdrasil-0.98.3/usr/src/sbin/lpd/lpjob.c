/*
$Header: /usr/src/linux/lp/lpjob.c,v 1.2 1992/01/12 18:22:37 james_r_wiegand Exp $

  linux/lp/lpjob.h  printer job setup data for lp
  c. 1992 James Wiegand

*/
#define __LP_JOB__
#include "lpjob.h"

#include <stdarg.h>
#include <unistd.h>
#include <string.h>

/*
if header and footer lines are less that 2 each they will not be printed even if
the string values are non-null.  
*/

void numericread( void )
{
/* read numeric values */
int count, temp;

  /* read flags */
  for( count = 2; count < 7; count ++ )
    {
    numeric_set[ count ].nvalue = pgetflag( numeric_set[ count ].npcapname );
    if( numeric_set[ count ].nvalue == FAILURE )
      {
      fprintf( stderr, "bad printcap or bad hardcode\n\r" );
      exit( 1 );
      }
    }

  /* numerical values */
  for( count = 7; count < 16; count ++ )
    {
    temp = pgetnum( numeric_set[ count ].npcapname );
    if( temp == FAILURE )
      {
      fprintf( stderr, "bad printcap or bad hardcode\n\r" );
      exit( 1 );
      }

    if( temp )
      numeric_set[ count ].nvalue = temp;
    }
}

void stringread( void )
{
int count;
char *result;
char *bufptr;
char *work, buffer[ 240 ];

  /* read string values */
  /* numerical values */
  
  for( count = 1; count < 9; count ++ )
    {
    work = buffer;
    result = pgetstr( string_set[ count ].spcapname, &work );
    if( result )
      {
      bufptr = strdup( result );
      if( bufptr )
        {
        message( "setting %s to %s == %s\n\r",
                 string_set[ count ].spcapname, result, bufptr );
        string_set[ count ].svalue = bufptr;
        }
      else
        {
        fprintf( stderr, "oudda da mem'ry, time ta bi mo'\n\r" );
        exit( 1 );
        }
      }   
    }
}/* makes me just want to cry! */

void lpjobinit( void )
{
 
/* read them damn pcap entries */
  if( pgetent( pcapbuffer, defaultjob ) == FAILURE )
    {
    fprintf( stderr, "missing printcap or bad job configuration\n\r" );
    exit( 1 );
    }
    
  numericread();
  stringread();
}

void lpbanner( int fptr )
{
/*
ok, to simplify things we assume the form width is at least 40 char.
otherwise, just blast out a blank page
*/
/*
need to get the file owner information from an fstat of the input file
if stdin skip all that.
(not done here)
*/
char buffer[ 128 ];
char *line1  = "du: %3d uu: %3d nc: %1d nf: %1d sb: %1d wr: %1d\n\r";
char *line2  = "mc: %3d pw: %3d pl: %3d px: %4d\n\r";
char *line3  = "py: %4d hl: %2d fl: %2d ex: %2d\n\r";
char *line4  = "lf: %14s ff: %2s ";
char *line4a = "tr: %8s\n\r";
char *line5  = "pr: %8s  ";
char *line5a = "ls: %8s\n\r";
char *line6  =  "ht: %35s\n\r";
char *line7  =  "ft: %35s\n\r";
char *line7a =  "sd: %35s\n\r";
char *line7b =  "lf: %35s\n\r";

char *line8  =  "username: %s\n\r";
char *line9  =  "group:    %s\n\r";
char *linea  =  "filename: %s\n\r";
char *lineb  =  "date:     %s time: %s\n\r";

sprintf( buffer, line1, NSV(0),  NSV(1),  NSV(2),  NSV(3),  NSV(4), NSV(5));
write  ( fptr, buffer, strlen( buffer ));

sprintf( buffer, line2, NSV(6),  NSV(7),  NSV(8),  NSV(9));
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, line3, NSV(10), NSV(11), NSV(12), NSV(13));
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, line4, SSV(0),  char2esc(SSV(1)) );
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, line4a, char2esc(SSV(2)) );
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, line5,  char2esc(SSV(3)) );
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, line5a, char2esc(SSV(4)) );
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, line6, IFIS( SSV(5) ));
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, line7, IFIS( SSV(6) ));
write  ( fptr, buffer, strlen( buffer ));

sprintf( buffer, line7a, IFIS( SSV(7) ));
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, line7b, IFIS( SSV(8) ));
write  ( fptr, buffer, strlen( buffer ));

sprintf( buffer, line8, pusername    );
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, line9, pgroup       );
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, linea, pfilename    );
write  ( fptr, buffer, strlen( buffer ));
sprintf( buffer, lineb, pdate, ptime );
write  ( fptr, buffer, strlen( buffer ));

}

#ifdef TEST
void main( void )
{
  lpjobinit();
  lpbanner( stdout );
}
#endif
