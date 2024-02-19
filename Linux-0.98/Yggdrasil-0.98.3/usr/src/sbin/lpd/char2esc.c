/*
$Header: /usr/src/linux/lp/char2esc.c,v 1.2 1992/01/12 18:20:40 james_r_wiegand Exp $

  - linux/lp/char2esc.c translate strings into hexy-decimal escape sequences
  c. 1992 James Wiegand
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *char2esc( char *cptr )
{
char buffer[1024];
char *bufptr = buffer;
char escape = '\\';

  buffer[ 0 ] = ' ';
  buffer[ 1 ] = 0x0;
  
  /* if string is null string just return a blank */
  if( !cptr )
    return buffer;
    
  while( *cptr )
    {
    *bufptr = escape;
    bufptr ++;
    *bufptr = (( *cptr & 0xf0 ) > 0x90 )
            ? ((*cptr & 0xf0) >> 4) + 'A' - 10 : ((*cptr & 0xf0) >> 4) +'0';
                        
    bufptr ++;
    *bufptr = (( *cptr & 0x0f ) > 0x09 )
                  ? (*cptr & 0x0f) + 'A' - 10 : (*cptr & 0x0f) +'0';
    
    bufptr ++;
    cptr ++;
    }
  *bufptr = 0x0;
  return buffer;  
} 
/*
void main( void )
{
char *foo = "01?o5wxyz\x7f\x8f\x9f\xaf\xbf\xcf\xdf";

  printf( "%s is escaped as %s\n\r", foo, char2esc( foo ));
}
*/
