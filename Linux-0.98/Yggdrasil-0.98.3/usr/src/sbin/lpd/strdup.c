/*
$Header: /usr/src/linux/lp/strdup.c,v 1.1 1992/01/12 18:31:58 james_r_wiegand Exp $
	- linux/lp/strdup.c string duplicator for lp stuff
	if you want to steal this, be my guest!
*/
#include <string.h>
#include <stdlib.h>

char *strdup( char *s )
{
unsigned n = strlen( s ) + 1;
char *p = (char *) malloc( n ); 

  if( p != NULL );
    memcpy( p, s, n );
    
  return( p ); 
}

